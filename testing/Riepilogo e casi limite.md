# ft\_irc

Server IRC minimale scritto in C++98, conforme alle norme del progetto
(niente librerie esterne oltre alla libc, gestione I/O non bloccante con
`poll()`, nessun fork/thread per client).

> **Stato**: in sviluppo. Le sezioni marcate `TODO` vanno completate a mano
> a mano che le rispettive parti sono pronte — indicato tra parentesi chi
> se ne occupa.

## Indice

-   [Panoramica](#panoramica)
-   [Requisiti e build](#requisiti-e-build)
-   [Utilizzo](#utilizzo)
-   [Architettura](#architettura)
-   [Comandi supportati](#comandi-supportati)
-   [Testing](#testing)
-   [Limiti noti](#limiti-noti)

## Panoramica

Il server accetta connessioni TCP concorrenti da più client IRC (es.
`irssi`, `HexChat`, o semplicemente `nc`), gestendole tutte in un unico
processo/thread tramite multiplexing di I/O (`poll()`), senza mai bloccarsi
in attesa di un singolo client.

Supporta autenticazione con password, gestione nickname, canali con
operatori e modalità (invite-only, password di canale, limite utenti,
topic protetto), messaggistica privata e di canale.

## Requisiti e build

-   Compilatore C++ conforme a C++98 (`c++`/`g++`/`clang++`)
-   Nessuna dipendenza esterna oltre alla libc/POSIX sockets

```bash
make        # compila il server, produce l'eseguibile ircserv
make clean  # rimuove gli oggetti .o
make fclean # rimuove anche l'eseguibile
make re     # fclean + all
```

## Utilizzo

```bash
./ircserv <porta> <password>
```

-   `<porta>`: porta TCP su cui il server resta in ascolto
-   `<password>`: password richiesta a ogni client con il comando `PASS`
    prima di poter procedere con `NICK`/`USER`

Esempio di connessione con un client IRC qualsiasi:

```bash
nc localhost 6667
PASS segreta
NICK alice
USER alice 0 * :Alice Test
JOIN #general
PRIVMSG #general :ciao a tutti
```

## Architettura

Il codice è diviso in tre aree di responsabilità indipendenti:

| Area | File principali | Responsabilità |
| --- | --- | --- |
| Rete | `Server` (parte I/O), `poll()` loop | accept, read/send non bloccanti, ciclo eventi |
| Parser e comandi | `IrcMessage`, dispatcher comandi | tokenizzazione righe IRC, validazione, dispatch ai comandi |
| Core / dati | `Client`, `Channel`, `Server` (parte dati) | stato di utenti e canali, relazioni, regole |

Il flusso di un messaggio, dal client mittente al destinatario:

```
client scrive → read() dal socket → accumulo in recvBuffer → isolamento riga
   → parsing in IrcMessage → dispatch del comando → aggiornamento stato
   (Client/Channel) e accodamento in sendBuffer → send() dal socket
   → il destinatario riceve
```

Per il dettaglio di ogni classe (membri, metodi, chi chiama cosa) vedi
[`ARCHITETTURA.md`](./ARCHITETTURA.md).

## Comandi supportati

| Comando | Descrizione | Stato |
| --- | --- | --- |
| `PASS` | autenticazione iniziale | TODO |
| `NICK` | impostazione/cambio nickname | TODO |
| `USER` | registrazione dati utente | TODO |
| `JOIN` | ingresso in un canale (crea il canale se non esiste) | TODO |
| `PART` | uscita da un canale | TODO |
| `PRIVMSG` | messaggio privato o di canale | TODO |
| `TOPIC` | lettura/impostazione del topic | TODO |
| `INVITE` | invito a un canale invite-only | TODO |
| `KICK` | espulsione di un membro (solo operatori) | TODO |
| `MODE` | gestione modalità canale (`i`,`t`,`k`,`o`,`l`) | TODO |
| `QUIT` | disconnessione esplicita | TODO |

## Testing

Il core (`Client`/`Channel`/`Server`) ha una suite di test in isolamento,
senza rete né parser, in `srcs/test_main.cpp`:

```bash
c++ -Wall -Wextra -Werror -std=c++98 -Iincludes srcs/Client.cpp srcs/Channel.cpp srcs/Server.cpp srcs/test_main.cpp -o test_core
./test_core
```

Verifica: creazione client, join in canale, promozione a operatore,
broadcast con esclusione del mittente, rimozione client, distruzione
automatica dei canali vuoti, stringa delle modalità attive.

Controllo memory leak (da eseguire su Linux, non nel sandbox di sviluppo):

```bash
valgrind --leak-check=full --show-leak-kinds=all ./ircserv 6667 segreta
```

## Limiti noti

-   Nessuna persistenza: lo stato del server (client, canali) esiste solo
    in memoria e si perde al riavvio.
-   Nessun limite esplicito alla dimensione del buffer di invio per client:
    un client che non legge mai potrebbe far crescere `_sendBuffer`
    indefinitamente (nessuna protezione anti-DoS).
-   `servername`/`hostname` dichiarati dal client nel comando `USER` sono
    ignorati: l’hostname effettivo è ricavato dalla connessione TCP reale.

## Casi limite

### `Server`

13.  **`getClientByFd()` su un fd mai registrato** — deve restituire `0` (`NULL`), non un puntatore sporco o un crash.
14.  **`removeClient()` su un fd già rimosso (doppia rimozione)** — non deve fare doppio `delete` (il classico double-free). Con la tua guardia `if (it == _clients.end()) return;` dovrebbe essere già sicuro — verificalo con un test esplicito.
15.  **`createChannel()` chiamato due volte con lo stesso nome** — deve restituire **lo stesso puntatore**, non crearne uno nuovo (hai già il controllo `getChannel(name)` prima di creare — testalo).
16.  **`removeChannelIfEmpty()` su un canale che ha ancora membri** — non deve distruggerlo per errore.
17.  **`getClientByNickname()` con nickname duplicati** (bug altrove, ma testalo comunque) — cosa restituisce se per assurdo due client hanno lo stesso nickname? Con la tua implementazione restituirebbe il primo trovato scorrendo la mappa — è il comportamento voluto? Non dovrebbe succedere se `nicknameInUse()` viene sempre controllato prima, ma è bene sapere cosa fa se succede comunque.
18.  **`~Server()` con client/canali ancora presenti** — verifica con Valgrind che il distruttore liberi *tutto*, non solo una parte (hai già i due loop `delete`, ma è il test più importante di tutti per i memory leak).

###

### `Client`

1.  **`getPrefix()` con campi vuoti** — cosa restituisce se `_nickname`/`_username` non sono ancora stati impostati (client connesso ma non registrato)? Non deve crashare, ma il risultato (`"!@host"`) è “legale” solo se nessuno lo usa prima che il client sia registrato — verificalo.
2.  **`queueMessage()` chiamato più volte di fila** — il buffer deve accumulare correttamente, non sovrascrivere (`"msg1\r\nmsg2\r\n"`, non solo l’ultimo).
3.  **`clearSendBuffer()` su un buffer già vuoto** — non deve dare errori, deve solo restare vuoto.
4.  **Nickname con caratteri limite** — stringa vuota, o lunghissima: `Client` da solo non valida nulla (compito del parser), ma vale la pena verificare che accetti comunque qualsiasi stringa senza esplodere, così sai che la validazione va aggiunta *altrove* e non ti aspetti protezione qui.

### `Channel`

5.  **`removeClient()` su un client mai aggiunto** — non deve crashare né alterare lo stato; verifica che `_clients.size()` resti invariato.
6.  **`addClient()` chiamato due volte con lo stesso client** — non deve duplicarlo nel vettore (hai già la guardia `!contains(...)` in `addClient`, ma vale la pena un test esplicito che lo confermi).
7.  **`addOperator()` su un client che non è ancora membro** — deve fallire silenziosamente (non promuove), dato che hai il controllo `contains(_clients, client)` prima di aggiungere.
8.  **`broadcast()` su un canale con zero membri** — loop su vettore vuoto, non deve crashare, semplicemente non fa nulla.
9.  **`broadcast()` con `exclude` che non è nemmeno membro del canale** — dovrebbe comunque funzionare, mandando a tutti (visto che `exclude` è solo un filtro, non un requisito di appartenenza).
10.  **`removeClient()` dell’unico operatore rimasto** — il canale resta senza operatori. È un caso limite di design che avevamo accennato prima: verifica cosa succede concretamente (nessun crash, ma nessuno può più fare MODE/KICK — comportamento accettabile o da gestire?).
11.  **`getModeString()` senza nessuna modalità attiva** — deve restituire stringa vuota, non `"+"` da solo (controlla che il tuo `if (modes == "+") return ("");` funzioni).
12.  **Case-sensitivity del nome canale** — `Channel` da solo non lo gestisce (lo fa `Server` con `toLower`), ma verifica che due `Channel` con nomi diversi solo per maiuscole restino “diversi” a livello di oggetto, dato che la normalizzazione è responsabilità di `Server`, non sua.

     ### Il test più prezioso in assoluto

     19.  Sequenza QUIT-mid-broadcast: un client A manda un `broadcast()` a un canale, e nello stesso "istante logico" un client B di quello stesso canale viene rimosso da `Server::removeClient()` prima che il broadcast lo raggiunga. Non è un vero problema di concorrenza (il server è single-thread), ma verifica che l'ordine delle operazioni nel tuo dispatcher non lasci mai un puntatore penzolante (*dangling pointer*) verso un `Client` già distrutto ma ancora referenziato in un `Channel` — è il bug più subdolo possibile in questa architettura basata su puntatori grezzi.

## Team

| Ruolo | Responsabilità |
| --- | --- |
| Persona A | Rete: socket, `poll()`, I/O non bloccante |
| Persona B | Core: `Client`, `Channel`, `Server`, integrazione, test, README |
| Persona C | Parser e comandi: `IrcMessage`, dispatcher, logica dei comandi |