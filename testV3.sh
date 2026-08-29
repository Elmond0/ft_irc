#!/bin/bash
# ============================================================
#  IRC Server Test Suite (ft_irc) - v3 "Fantastico + Valgrind"
# ============================================================
#  Novità rispetto alla v2:
#   - Controlli di memoria con valgrind:
#       * se viene passato il path del binario del server (1° argomento
#         dello script, oppure variabile SERVER_BIN), lo script avvia
#         il server stesso sotto valgrind, esegue tutta la suite di
#         test come client esterno, e a fine esecuzione ferma il
#         server e analizza il log di valgrind (ERROR SUMMARY,
#         definitely/indirectly lost) aggiungendo i risultati al
#         conteggio PASS/FAIL finale.
#       * se non viene passato nessun binario, lo script si comporta
#         come prima: assume che il server sia già in ascolto su
#         SERVER:PORT e NON esegue controlli di memoria (perché non
#         ha modo di "agganciarsi" a un processo già avviato altrove).
#   - Timeout di attesa (wait_for) aumentati automaticamente quando
#     si esegue sotto valgrind, perché il server è molto più lento.
#
#  Uso:
#     ./testV3.sh                 # comportamento invariato (no valgrind)
#     ./testV3.sh ./ircserv       # avvia ./ircserv sotto valgrind e testa
#     SERVER_BIN=./ircserv ./testV3.sh
# ============================================================

set -u

# ---------------- Configurazione ----------------
SERVER="127.0.0.1"
PORT="6677"
PASSWORD="pass"

WAIT_TIMEOUT="2"     # secondi massimi di attesa per wait_for
POLL_INTERVAL="0.1"  # intervallo di polling

# ---------------- Colori ----------------
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m'

TMPDIR=$(mktemp -d)
PASSED=0
FAILED=0
declare -a FAILED_NAMES=()

# lista di pid dei client persistenti ancora vivi (per la pulizia finale)
declare -A CLIENT_PIDS=()

# ---------------- Valgrind ----------------
# Se SERVER_BIN è impostato (o passato come primo argomento allo script),
# il server viene avviato automaticamente sotto valgrind e il suo log
# di memory-check viene analizzato a fine suite.
SERVER_BIN="${1:-${SERVER_BIN:-}}"
USE_VALGRIND=1
VALGRIND_LOG="$TMPDIR/valgrind.log"
SERVER_PID=""

# Se stiamo usando valgrind il server è molto più lento: alziamo i timeout
# di default usati da wait_for/assert_eventually.
DEFAULT_MAX_STEPS=20
if [ -n "$SERVER_BIN" ]; then
    DEFAULT_MAX_STEPS=60
fi

# ============================================================
# PULIZIA
# ============================================================
cleanup() {
    for id in "${!CLIENT_PIDS[@]}"; do
        kill "${CLIENT_PIDS[$id]}" 2>/dev/null
    done
    wait 2>/dev/null

    if [ -n "$SERVER_PID" ]; then
        kill -TERM "$SERVER_PID" 2>/dev/null
        # piccola attesa per lasciare a valgrind il tempo di scrivere il report
        local tries=0
        while kill -0 "$SERVER_PID" 2>/dev/null && [ "$tries" -lt 30 ]; do
            sleep 0.1
            tries=$((tries + 1))
        done
        kill -KILL "$SERVER_PID" 2>/dev/null
    fi

    rm -rf "$TMPDIR"
}
trap cleanup EXIT INT TERM

# ============================================================
# FUNZIONI HELPER - client "usa e getta" (per test single-shot)
# ============================================================

run_client() {
    local input_file="$TMPDIR/input_$RANDOM$RANDOM"
    local output_file="$TMPDIR/output_$RANDOM$RANDOM"

    for cmd in "$@"; do
        printf "%s\r\n" "$cmd" >> "$input_file"
    done

    timeout 3 nc -w 2 "$SERVER" "$PORT" < "$input_file" > "$output_file" 2>/dev/null

    echo "$output_file"
}

# ============================================================
# FUNZIONI HELPER - client PERSISTENTI (via FIFO)
# ============================================================

# start_client <id>
# Apre una connessione nc che resta viva, alimentata da una FIFO.
# Dichiara array associativo (fuori dalle funzioni)
declare -A CLIENT_FDS=()

start_client() {
    local id="$1"
    local fifo="$TMPDIR/fifo_$id"
    local out="$TMPDIR/out_$id"

    mkfifo "$fifo"
    : > "$out"

    # 1) Avvia nc PRIMA, così il lettore è pronto
    nc "$SERVER" "$PORT" < "$fifo" > "$out" 2>/dev/null &
    local pid=$!
    CLIENT_PIDS[$id]=$pid

    # 2) Apre un nuovo fd in scrittura (fd auto-assegnato da bash)
    exec {fd}>"$fifo"                # il nuovo fd viene messo in $fd
    CLIENT_FDS[$id]=$fd              # memorizza il numero del descrittore

    # Rimuovi la vecchia eval non usata
}

# Tutti i send devono usare il fd corretto per il client
send_cmd() {
    local id="$1"
    local cmd="$2"
    printf "%s\r\n" "$cmd" >&${CLIENT_FDS[$id]}
}

send_raw() {
    local id="$1"
    local raw="$2"
    # %b (non %s) interpreta le sequenze di escape (\r\n) dentro $raw:
    # con %s i caratteri '\r\n' arrivano al server come backslash letterali,
    # non come un vero fine riga, e il comando non viene mai riconosciuto.
    printf "%b" "$raw" >&${CLIENT_FDS[$id]}
}

stop_client() {
    local id="$1"
    local fd=${CLIENT_FDS[$id]:-}
    # Chiude il descrittore di scrittura, nc riceverà EOF e terminerà
    if [[ -n "$fd" ]]; then
        exec {fd}>&- 2>/dev/null
    fi
    kill "${CLIENT_PIDS[$id]:-}" 2>/dev/null
    unset 'CLIENT_PIDS[$id]'
    unset 'CLIENT_FDS[$id]'
    rm -f "$TMPDIR/fifo_$id"
}

# wait_for <output_file> <pattern> [max_steps]
# Polling: ritorna 0 appena il pattern compare, 1 allo scadere del timeout.
# Non dipende da `bc`: usa un contatore intero di step da 0.1s
# (default DEFAULT_MAX_STEPS, alzato automaticamente sotto valgrind).
wait_for() {
    local file="$1"
    local pattern="$2"
    local max_steps="${3:-$DEFAULT_MAX_STEPS}"
    local step=0

    while [ "$step" -lt "$max_steps" ]; do
        if [ -f "$file" ] && grep -qF "$pattern" "$file" 2>/dev/null; then
            return 0
        fi
        sleep "$POLL_INTERVAL"
        step=$((step + 1))
    done
    grep -qF "$pattern" "$file" 2>/dev/null
}

# ============================================================
# ASSERZIONI
# ============================================================

assert_contains() {
    local test_name="$1"
    local output_file="$2"
    local expected="$3"

    if grep -qF "$expected" "$output_file" 2>/dev/null; then
        echo -e "${GREEN}[PASS]${NC} $test_name"
        ((PASSED++))
    else
        echo -e "${RED}[FAIL]${NC} $test_name"
        echo "   Atteso  : '$expected'"
        echo "   Ricevuto:"
        sed 's/^/      /' "$output_file" 2>/dev/null
        ((FAILED++))
        FAILED_NAMES+=("$test_name")
    fi
}

assert_not_contains() {
    local test_name="$1"
    local output_file="$2"
    local forbidden="$3"

    if ! grep -qF "$forbidden" "$output_file" 2>/dev/null; then
        echo -e "${GREEN}[PASS]${NC} $test_name"
        ((PASSED++))
    else
        echo -e "${RED}[FAIL]${NC} $test_name"
        echo "   Stringa indesiderata trovata: '$forbidden'"
        echo "   Ricevuto:"
        sed 's/^/      /' "$output_file" 2>/dev/null
        ((FAILED++))
        FAILED_NAMES+=("$test_name")
    fi
}

# Wrapper che attende con polling e poi asserisce (usato con client persistenti)
# Il 4o parametro opzionale alza il numero di step di wait_for (default
# DEFAULT_MAX_STEPS) per i test più lenti/delicati.
assert_eventually() {
    local test_name="$1"
    local output_file="$2"
    local expected="$3"
    local max_steps="${4:-$DEFAULT_MAX_STEPS}"
    wait_for "$output_file" "$expected" "$max_steps" >/dev/null
    assert_contains "$test_name" "$output_file" "$expected"
}

test_case() {
    local name="$1"; shift
    local expected="$1"; shift
    local output_file
    output_file=$(run_client "$@")
    assert_contains "$name" "$output_file" "$expected"
}

test_case_not() {
    local name="$1"; shift
    local forbidden="$1"; shift
    local output_file
    output_file=$(run_client "$@")
    assert_not_contains "$name" "$output_file" "$forbidden"
}

section() {
    echo
    echo -e "${YELLOW}--- $1 ---${NC}"
}

# ============================================================
# VALGRIND: avvio server e controllo finale
# ============================================================

start_server_with_valgrind() {
    if [ -z "$SERVER_BIN" ]; then
        echo -e "${YELLOW}Nessun binario del server specificato: i controlli valgrind sono disattivati.${NC}"
        echo -e "${YELLOW}(uso: $0 /path/to/ircserv   oppure  SERVER_BIN=/path/to/ircserv $0)${NC}"
        echo -e "${YELLOW}Si assume che il server sia già in ascolto su $SERVER:$PORT.${NC}"
        USE_VALGRIND=0
        return 0
    fi

    if [ ! -x "$SERVER_BIN" ]; then
        echo -e "${RED}Il binario '$SERVER_BIN' non esiste o non è eseguibile.${NC}"
        exit 1
    fi

    if ! command -v valgrind >/dev/null 2>&1; then
        echo -e "${RED}valgrind non è installato: impossibile eseguire i controlli di memoria.${NC}"
        exit 1
    fi

    echo -e "${YELLOW}Avvio di '$SERVER_BIN' sotto valgrind (log: $VALGRIND_LOG)...${NC}"

    valgrind \
        --leak-check=full \
        --show-leak-kinds=all \
        --track-origins=yes \
        --trace-children=yes \
        --error-exitcode=123 \
        --log-file="$VALGRIND_LOG" \
        "$SERVER_BIN" "$PORT" "$PASSWORD" &
    SERVER_PID=$!

    # Attende che il server sia pronto ad accettare connessioni
    # (valgrind rallenta molto l'avvio, quindi il timeout è più largo)
    local tries=0
    until (exec 3<>"/dev/tcp/$SERVER/$PORT") 2>/dev/null; do
        exec 3>&- 2>/dev/null; exec 3<&- 2>/dev/null

        if ! kill -0 "$SERVER_PID" 2>/dev/null; then
            echo -e "${RED}Il processo del server è terminato prematuramente. Controlla $VALGRIND_LOG.${NC}"
            [ -f "$VALGRIND_LOG" ] && sed 's/^/   /' "$VALGRIND_LOG"
            exit 1
        fi

        tries=$((tries + 1))
        if [ "$tries" -ge 100 ]; then
            echo -e "${RED}Il server non si è avviato entro il timeout.${NC}"
            exit 1
        fi
        sleep 0.2
    done
    exec 3>&- 2>/dev/null
    exec 3<&- 2>/dev/null
    echo -e "${GREEN}Server (PID $SERVER_PID) avviato e raggiungibile sotto valgrind.${NC}"
}

stop_server_and_check_valgrind() {
    [ "$USE_VALGRIND" != "1" ] && return 0
    [ -z "$SERVER_PID" ] && return 0

    section "Controlli di memoria (valgrind)"

    # SIGTERM (non SIGKILL): dà tempo a valgrind di scrivere il report finale.
    # Il server ft_irc dovrebbe gestire SIGTERM chiudendo i socket e uscendo
    # in modo pulito: questo è anche implicitamente un test che non fa
    # segfault/crashare sull'arresto.
    kill -TERM "$SERVER_PID" 2>/dev/null

    local tries=0
    while kill -0 "$SERVER_PID" 2>/dev/null; do
        tries=$((tries + 1))
        if [ "$tries" -ge 150 ]; then
            echo -e "${RED}Il server non termina entro il timeout dopo SIGTERM, invio SIGKILL.${NC}"
            echo "   (nota: SIGKILL non permette a valgrind di scrivere un report leak completo)"
            kill -KILL "$SERVER_PID" 2>/dev/null
            break
        fi
        sleep 0.1
    done
    wait "$SERVER_PID" 2>/dev/null
    SERVER_PID=""

    if [ ! -f "$VALGRIND_LOG" ]; then
        echo -e "${RED}[FAIL]${NC} Log di valgrind non trovato"
        ((FAILED++)); FAILED_NAMES+=("Valgrind: log mancante")
        return 0
    fi

    # 1) Errori rilevati da valgrind (invalid read/write, use-after-free, ecc.)
    local err_count
    err_count=$(grep -oP 'ERROR SUMMARY: \K[0-9]+' "$VALGRIND_LOG" | tail -1)
    err_count=${err_count:-0}
    if [ "$err_count" -eq 0 ]; then
        echo -e "${GREEN}[PASS]${NC} Nessun errore di memoria (ERROR SUMMARY: 0)"
        ((PASSED++))
    else
        echo -e "${RED}[FAIL]${NC} Valgrind ha rilevato $err_count errori"
        ((FAILED++)); FAILED_NAMES+=("Valgrind: $err_count errori di memoria")
    fi

    # 2) Leak "definitely lost"
    local def_lost
    def_lost=$(grep -oP 'definitely lost: \K[0-9,]+(?= bytes)' "$VALGRIND_LOG" | tail -1 | tr -d ',')
    def_lost=${def_lost:-0}
    if [ "$def_lost" -eq 0 ]; then
        echo -e "${GREEN}[PASS]${NC} Nessun leak 'definitely lost'"
        ((PASSED++))
    else
        echo -e "${RED}[FAIL]${NC} 'definitely lost': $def_lost byte"
        ((FAILED++)); FAILED_NAMES+=("Valgrind: definitely lost $def_lost byte")
    fi

    # 3) Leak "indirectly lost"
    local ind_lost
    ind_lost=$(grep -oP 'indirectly lost: \K[0-9,]+(?= bytes)' "$VALGRIND_LOG" | tail -1 | tr -d ',')
    ind_lost=${ind_lost:-0}
    if [ "$ind_lost" -eq 0 ]; then
        echo -e "${GREEN}[PASS]${NC} Nessun leak 'indirectly lost'"
        ((PASSED++))
    else
        echo -e "${RED}[FAIL]${NC} 'indirectly lost': $ind_lost byte"
        ((FAILED++)); FAILED_NAMES+=("Valgrind: indirectly lost $ind_lost byte")
    fi

    # 4) File descriptor rimasti aperti (con --trace-children/track-fds volendo
    #    si potrebbe estendere; qui ci limitiamo a segnalare se il pattern compare)
    if grep -q "FILE DESCRIPTORS" "$VALGRIND_LOG" 2>/dev/null; then
        local open_fds
        open_fds=$(grep -oP '\K[0-9]+(?= open at exit)' "$VALGRIND_LOG" | tail -1)
        if [ -n "$open_fds" ] && [ "$open_fds" -gt 3 ]; then
            echo -e "${YELLOW}[INFO]${NC} $open_fds file descriptor ancora aperti all'uscita (oltre a stdin/stdout/stderr)"
        fi
    fi

    echo -e "${YELLOW}Log completo di valgrind: $VALGRIND_LOG${NC}"
    cp "$VALGRIND_LOG" "./valgrind_ircserv.log" 2>/dev/null && \
        echo -e "${YELLOW}(copiato anche in ./valgrind_ircserv.log per ispezione dopo la pulizia)${NC}"
}

# ============================================================
# CONTROLLO PRELIMINARE: il server è raggiungibile?
# ============================================================
echo -e "${YELLOW}=== Test Server IRC ===${NC}"
echo "Server: $SERVER:$PORT  Password: $PASSWORD"

if [ -n "$SERVER_BIN" ]; then
    start_server_with_valgrind
else
    USE_VALGRIND=0
    if ! (exec 3<>"/dev/tcp/$SERVER/$PORT") 2>/dev/null; then
        echo -e "${RED}Impossibile connettersi a $SERVER:$PORT. Il server è avviato?${NC}"
        exit 1
    fi
    exec 3>&- 2>/dev/null
    exec 3<&- 2>/dev/null
    echo -e "${GREEN}Server raggiungibile.${NC}"
fi

# ============================================================
# 1. REGISTRAZIONE
# ============================================================
section "Registrazione"

test_case "Registrazione base" "001 test1" \
    "PASS $PASSWORD" "NICK test1" "USER test1 0 * :Test User One"

test_case "PASS sbagliata" "464" \
    "PASS wrongpass" "NICK testx" "USER testx 0 * :X"

test_case "USER senza parametri" "461" \
    "PASS $PASSWORD" "NICK test3" "USER test3"

test_case "PASS dopo registrazione" "462" \
    "PASS $PASSWORD" "NICK test4" "USER test4 0 * :Test4" "PASS $PASSWORD"

test_case "Registrazione con CAP" "001 test5" \
    "PASS $PASSWORD" "CAP LS" "NICK test5" "USER test5 0 * :Test5" "CAP END"

# 1g. NICK duplicato con client PERSISTENTE (il vero test di regressione:
#     qui il primo nick resta davvero occupato mentre il secondo si registra)
start_client "reg_c1"
send_cmd "reg_c1" "PASS $PASSWORD"
send_cmd "reg_c1" "NICK loStessoNick"
send_cmd "reg_c1" "USER u1 0 * :U1"
wait_for "$TMPDIR/out_reg_c1" "001" >/dev/null

start_client "reg_c2"
send_cmd "reg_c2" "PASS $PASSWORD"
send_cmd "reg_c2" "NICK loStessoNick"
send_cmd "reg_c2" "USER u2 0 * :U2"
assert_eventually "NICK duplicato (client persistente)" "$TMPDIR/out_reg_c2" "433"
stop_client "reg_c1"
stop_client "reg_c2"
sleep 0.3   # lascia che i socket dei client persistenti precedenti si chiudano del tutto

# ============================================================
# 2. DATI PARZIALI / BUFFERING
# ============================================================
section "Dati Parziali (frammentazione comandi)"

start_client "buf"
send_raw "buf" "NI"
sleep 0.2
send_raw "buf" "CK bu"
sleep 0.2
send_raw "buf" "ffer\r\n"
send_cmd "buf" "PASS $PASSWORD"
send_cmd "buf" "USER b 0 * :B"
assert_eventually "Comando NICK ricomposto da frammenti" "$TMPDIR/out_buf" "001 buffer" 40
stop_client "buf"
sleep 0.2

# Comando spezzato a metà di una singola riga USER
start_client "buf2"
send_cmd "buf2" "PASS $PASSWORD"
send_cmd "buf2" "NICK fraguser"
send_raw "buf2" "USER fr"
sleep 0.2
send_raw "buf2" "ag 0 * :Frag"
sleep 0.2
send_raw "buf2" "mented\r\n"
assert_eventually "Comando USER ricomposto da frammenti" "$TMPDIR/out_buf2" "001 fraguser" 40
stop_client "buf2"

# ============================================================
# 3. CANALI
# ============================================================
section "Canali"

test_case "JOIN nuovo canale" "353" \
    "PASS $PASSWORD" "NICK ch1" "USER ch1 0 * :Ch1" "JOIN #nuovo"

# JOIN con key sbagliata: il canale +k deve restare vivo (client persistente)
# mentre il secondo si connette, altrimenti si svuota e viene distrutto prima
# che il test possa verificarne la password.
start_client "op1"
send_cmd "op1" "PASS $PASSWORD"; send_cmd "op1" "NICK op1"; send_cmd "op1" "USER op1 0 * :Op1"
send_cmd "op1" "JOIN #locked"
send_cmd "op1" "MODE #locked +k segreta"
wait_for "$TMPDIR/out_op1" "+k segreta" >/dev/null
out=$(run_client "PASS $PASSWORD" "NICK guest" "USER guest 0 * :Guest" "JOIN #locked wrongkey")
assert_contains "JOIN con key sbagliata" "$out" "475"
stop_client "op1"

test_case "JOIN senza #" "403" \
    "PASS $PASSWORD" "NICK nochannel" "USER nochannel 0 * :NoCh" "JOIN brutto"

out=$(run_client "PASS $PASSWORD" "NICK p1" "USER p1 0 * :P1" "JOIN #parttest" "PART #parttest :me ne vado")
assert_contains "PART con motivo" "$out" "PART #parttest :me ne vado"

# PART da canale sbagliato: il canale deve esistere davvero (client persistente
# che resta dentro), altrimenti il PART riceve 403 "canale inesistente" invece
# del 442 "non sei nel canale" che il test vuole verificare.
start_client "chankeeper"
send_cmd "chankeeper" "PASS $PASSWORD"; send_cmd "chankeeper" "NICK keeper"; send_cmd "chankeeper" "USER k 0 * :Keeper"
send_cmd "chankeeper" "JOIN #parttest2"
wait_for "$TMPDIR/out_chankeeper" "JOIN #parttest2" >/dev/null
out=$(run_client "PASS $PASSWORD" "NICK p2" "USER p2 0 * :P2" "PART #parttest2")
assert_contains "PART da canale sbagliato" "$out" "442"
stop_client "chankeeper"

# ============================================================
# 4. PRIVMSG / NOTICE (inclusi test multi-client reali)
# ============================================================
section "Messaggistica"

test_case "PRIVMSG a utente inesistente" "401" \
    "PASS $PASSWORD" "NICK sender" "USER s 0 * :S" "PRIVMSG nobody :ciao"

test_case_not "NOTICE a inesistente silenzioso" "401" \
    "PASS $PASSWORD" "NICK sender2" "USER s2 0 * :S2" "NOTICE nobody :ciao"

# 4c. PRIVMSG privato reale tra due client persistenti
start_client "pm_a"
start_client "pm_b"
send_cmd "pm_a" "PASS $PASSWORD"; send_cmd "pm_a" "NICK alice"; send_cmd "pm_a" "USER a 0 * :Alice"
send_cmd "pm_b" "PASS $PASSWORD"; send_cmd "pm_b" "NICK bob";   send_cmd "pm_b" "USER b 0 * :Bob"
wait_for "$TMPDIR/out_pm_a" "001" >/dev/null
wait_for "$TMPDIR/out_pm_b" "001" >/dev/null
send_cmd "pm_a" "PRIVMSG bob :ciao privatamente"
assert_eventually "PRIVMSG privato ricevuto dal destinatario" "$TMPDIR/out_pm_b" "PRIVMSG bob :ciao privatamente"
stop_client "pm_a"
stop_client "pm_b"

# 4d. PRIVMSG broadcast su canale tra due client persistenti
start_client "ch_a"
start_client "ch_b"
send_cmd "ch_a" "PASS $PASSWORD"; send_cmd "ch_a" "NICK chatA"; send_cmd "ch_a" "USER ca 0 * :ChatA"
send_cmd "ch_a" "JOIN #chat"
send_cmd "ch_b" "PASS $PASSWORD"; send_cmd "ch_b" "NICK chatB"; send_cmd "ch_b" "USER cb 0 * :ChatB"
send_cmd "ch_b" "JOIN #chat"
wait_for "$TMPDIR/out_ch_b" "JOIN #chat" >/dev/null
send_cmd "ch_a" "PRIVMSG #chat :messaggio broadcast"
assert_eventually "PRIVMSG su canale ricevuto da altro utente" "$TMPDIR/out_ch_b" "PRIVMSG #chat :messaggio broadcast"
stop_client "ch_a"
stop_client "ch_b"

# ============================================================
# 5. OPERATORI E MODERAZIONE (multi-client reale)
# ============================================================
section "Operatori e Moderazione"

# 5a. KICK senza essere operatore: "victim" deve restare davvero nel canale
# (client persistente) mentre "normal" (membro normale, non operatore) prova
# a kickarlo, altrimenti il canale si svuota/ricrea e "normal" ne diventa operator.
start_client "victim2"
send_cmd "victim2" "PASS $PASSWORD"; send_cmd "victim2" "NICK victim2"; send_cmd "victim2" "USER v2 0 * :V2"
send_cmd "victim2" "JOIN #kicktest2"
wait_for "$TMPDIR/out_victim2" "JOIN #kicktest2" >/dev/null
out2=$(run_client "PASS $PASSWORD" "NICK normal" "USER n 0 * :N" "JOIN #kicktest2" "KICK #kicktest2 victim2")
assert_contains "KICK senza operatore" "$out2" "482"
stop_client "victim2"

# 5b. KICK reale: op caccia victim, verifichiamo che victim riceva il KICK
start_client "kick_op"
start_client "kick_victim"
send_cmd "kick_op" "PASS $PASSWORD"; send_cmd "kick_op" "NICK operatore"; send_cmd "kick_op" "USER op 0 * :Operator"
send_cmd "kick_op" "JOIN #realkick"
send_cmd "kick_victim" "PASS $PASSWORD"; send_cmd "kick_victim" "NICK vittima"; send_cmd "kick_victim" "USER vic 0 * :Victim"
send_cmd "kick_victim" "JOIN #realkick"
wait_for "$TMPDIR/out_kick_victim" "JOIN #realkick" >/dev/null
send_cmd "kick_op" "KICK #realkick vittima :Fuori!"
assert_eventually "KICK ricevuto dalla vittima" "$TMPDIR/out_kick_victim" "KICK #realkick vittima"
assert_eventually "KICK visibile all'operatore (broadcast)" "$TMPDIR/out_kick_op" "KICK #realkick vittima"
stop_client "kick_op"
stop_client "kick_victim"

# 5c. INVITE / +i: il canale +i deve restare vivo (client persistente) mentre
# il secondo client prova a entrare, altrimenti si ricrea senza +i.
start_client "inviter2"
send_cmd "inviter2" "PASS $PASSWORD"; send_cmd "inviter2" "NICK inviter2"; send_cmd "inviter2" "USER i2 0 * :I2"
send_cmd "inviter2" "JOIN #inviteonly2"
send_cmd "inviter2" "MODE #inviteonly2 +i"
wait_for "$TMPDIR/out_inviter2" "+i" >/dev/null
out3=$(run_client "PASS $PASSWORD" "NICK guesti" "USER g 0 * :G" "JOIN #inviteonly2")
assert_contains "Canale +i senza invito" "$out3" "473"
stop_client "inviter2"

# 5d. TOPIC con restrizione +t: stesso motivo, il canale +t deve restare vivo,
# e "nontop" deve entrarci davvero (da membro normale) prima di provare TOPIC.
start_client "top2"
send_cmd "top2" "PASS $PASSWORD"; send_cmd "top2" "NICK top2"; send_cmd "top2" "USER t2 0 * :T2"
send_cmd "top2" "JOIN #topic2"
send_cmd "top2" "TOPIC #topic2 :Primo topic"
send_cmd "top2" "MODE #topic2 +t"
wait_for "$TMPDIR/out_top2" "+t" >/dev/null
out4=$(run_client "PASS $PASSWORD" "NICK nontop" "USER nt 0 * :NT" "JOIN #topic2" "TOPIC #topic2 :Nuovo")
assert_contains "TOPIC con +t senza operatore" "$out4" "482"
stop_client "top2"

# ============================================================
# 6. MODE
# ============================================================
section "MODE"

# 6a. MODE +o reale: verifichiamo che il target riceva davvero il broadcast
start_client "mode_op"
start_client "mode_target"
send_cmd "mode_op" "PASS $PASSWORD"; send_cmd "mode_op" "NICK opmode"; send_cmd "mode_op" "USER om 0 * :Om"
send_cmd "mode_op" "JOIN #modecanale"
send_cmd "mode_target" "PASS $PASSWORD"; send_cmd "mode_target" "NICK target"; send_cmd "mode_target" "USER tg 0 * :Tg"
send_cmd "mode_target" "JOIN #modecanale"
wait_for "$TMPDIR/out_mode_target" "JOIN #modecanale" >/dev/null
send_cmd "mode_op" "MODE #modecanale +o target"
assert_eventually "MODE +o broadcast (operatore)" "$TMPDIR/out_mode_op" "+o target"
assert_eventually "MODE +o ricevuto dal target" "$TMPDIR/out_mode_target" "+o target"
stop_client "mode_op"
stop_client "mode_target"

# 6b. MODE +l con limite valido e non valido
out=$(run_client "PASS $PASSWORD" "NICK lim" "USER l 0 * :L" "JOIN #limit" "MODE #limit +l 2")
assert_contains "MODE +l 2" "$out" "+l 2"
out=$(run_client "PASS $PASSWORD" "NICK lim2" "USER l2 0 * :L2" "JOIN #limit2" "MODE #limit2 +l abc")
assert_contains "MODE +l non numerico" "$out" "461"

# 6c. MODE +l applicato davvero: il terzo utente non riesce a entrare (471)
start_client "lim_a"
start_client "lim_b"
send_cmd "lim_a" "PASS $PASSWORD"; send_cmd "lim_a" "NICK lima"; send_cmd "lim_a" "USER la 0 * :La"
send_cmd "lim_a" "JOIN #reallimit"
send_cmd "lim_a" "MODE #reallimit +l 1"
send_cmd "lim_b" "PASS $PASSWORD"; send_cmd "lim_b" "NICK limb"; send_cmd "lim_b" "USER lb 0 * :Lb"
send_cmd "lim_b" "JOIN #reallimit"
assert_eventually "MODE +l 1 applicato: secondo utente respinto (471)" "$TMPDIR/out_lim_b" "471"
stop_client "lim_a"
stop_client "lim_b"

# 6d. MODE +k con chiave
out=$(run_client "PASS $PASSWORD" "NICK key" "USER k 0 * :K" "JOIN #key" "MODE #key +k s3cret")
assert_contains "MODE +k" "$out" "+k s3cret"

# ============================================================
# 7. QUIT
# ============================================================
section "QUIT"
out=$(run_client "PASS $PASSWORD" "NICK quitter" "USER q 0 * :Q" "JOIN #quitchan" "QUIT :Arrivederci")
assert_contains "QUIT con motivo" "$out" "ERROR :Closing Link"

# 7b. QUIT reale: un altro membro del canale deve vedere il QUIT
start_client "quit_a"
start_client "quit_b"
send_cmd "quit_a" "PASS $PASSWORD"; send_cmd "quit_a" "NICK quita"; send_cmd "quit_a" "USER qa 0 * :Qa"
send_cmd "quit_a" "JOIN #quitwatch"
send_cmd "quit_b" "PASS $PASSWORD"; send_cmd "quit_b" "NICK quitb"; send_cmd "quit_b" "USER qb 0 * :Qb"
send_cmd "quit_b" "JOIN #quitwatch"
wait_for "$TMPDIR/out_quit_b" "JOIN #quitwatch" >/dev/null
send_cmd "quit_a" "QUIT :ciao a tutti"
assert_eventually "QUIT visto dagli altri membri del canale" "$TMPDIR/out_quit_b" "QUIT :ciao a tutti"
stop_client "quit_a"
stop_client "quit_b"

# ============================================================
# 8. PING / ERRORI
# ============================================================
section "PING e Comandi Sconosciuti"

test_case "PING" "PONG" "PING 12345"

test_case "Comando sconosciuto" "421" \
    "PASS $PASSWORD" "NICK err" "USER e 0 * :E" "FOOBAR"

# ============================================================
# VALGRIND: arresto server e verifica finale
# ============================================================
stop_server_and_check_valgrind

# ============================================================
# SOMMARIO
# ============================================================
echo
echo -e "${YELLOW}=== Risultati: ${GREEN}$PASSED passati${NC}, ${RED}$FAILED falliti${NC} su $((PASSED+FAILED)) test ===${NC}"

if [ "$FAILED" -gt 0 ]; then
    echo
    echo -e "${RED}Test falliti:${NC}"
    for name in "${FAILED_NAMES[@]}"; do
        echo "  - $name"
    done
    exit 1
fi

exit 0