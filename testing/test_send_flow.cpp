#include "Server.hpp"
#include "Client.hpp"
#include "Channel.hpp"
#include <iostream>
#include <cassert>
#include <algorithm>

/*
** Test del "contratto" tra Client::_sendBuffer e il ciclo poll()/send()
** che scrivera' Persona A. Non apriamo socket veri: simuliamo una send()
** che scrive solo N byte alla volta, come fa davvero un socket non
** bloccante quando il kernel non ha spazio per accettare tutto in un
** colpo solo. Verifichiamo che il buffer si svuoti in modo incrementale
** e corretto, e che la logica "serve POLLOUT?" resti sempre coerente.
*/

/* Simula una send() di rete che accetta al massimo maxBytes per chiamata.
** Ritorna quanti byte sono stati "spediti" in questa chiamata. */
static size_t simulatedSend(Client *client, size_t maxBytes)
{
	std::string &buf = client->getSendBuffer();
	size_t toSend = std::min(maxBytes, buf.size());

	if (toSend == 0)
		return (0);

	buf.erase(0, toSend);
	return (toSend);
}

/* La regola che Persona A applichera' nel poll(): un fd ha bisogno di
** essere monitorato in scrittura (POLLOUT) se e solo se ha ancora dati
** da spedire. */
static bool needsPollout(Client *client)
{
	return (!client->getSendBuffer().empty());
}

static void testSendCompletoInUnaSolaChiamata()
{
	Server server;
	Client *c = server.addClient(40);

	c->queueMessage("PING :server");
	assert(needsPollout(c));

	size_t inviati = simulatedSend(c, 1024); // buffer piu' piccolo del max
	assert(inviati == std::string("PING :server\r\n").size());
	assert(c->getSendBuffer().empty());
	assert(!needsPollout(c));

	std::cout << "[OK] send completo in una sola chiamata svuota il buffer" << std::endl;
}

static void testSendParzialeSuPiuCicli()
{
	Server server;
	Client *c = server.addClient(41);

	c->queueMessage("012345678901234567890123456789"); // 30 char + \r\n = 32
	std::string messaggioCompleto = c->getSendBuffer();
	size_t totale = messaggioCompleto.size();

	assert(needsPollout(c));

	// simuliamo una rete che accetta solo 10 byte a chiamata
	size_t inviatiTotali = 0;
	int cicli = 0;
	while (needsPollout(c))
	{
		size_t inviati = simulatedSend(c, 10);
		inviatiTotali += inviati;
		++cicli;
		assert(cicli < 100); // safety net anti loop infinito in caso di bug
	}

	assert(inviatiTotali == totale);
	assert(c->getSendBuffer().empty());
	assert(!needsPollout(c));
	assert(cicli == 4); // 32 byte / 10 per ciclo = 4 cicli (10+10+10+2)

	std::cout << "[OK] send parziale su piu' cicli (POLLOUT) svuota correttamente il buffer in " << cicli << " cicli" << std::endl;
}

static void testMessaggioAccodatoDuranteInvioParziale()
{
	Server server;
	Client *c = server.addClient(42);

	c->queueMessage("primo messaggio lungo abbastanza");
	simulatedSend(c, 5); // invio parziale: resta ancora roba nel buffer
	assert(needsPollout(c));

	// nel frattempo, mentre il primo messaggio non e' ancora finito di
	// spedire, ne arriva un altro da accodare (es. un altro client scrive
	// nello stesso canale)
	c->queueMessage("secondo messaggio");

	// il buffer deve contenere ENTRAMBI, in ordine, senza che il secondo
	// sovrascriva il resto del primo non ancora spedito
	assert(c->getSendBuffer().find("messaggio lungo") != std::string::npos);
	assert(c->getSendBuffer().find("secondo messaggio") != std::string::npos);

	// svuotiamo tutto e verifichiamo che nulla sia andato perso
	size_t cicli = 0;
	while (needsPollout(c) && cicli < 100)
	{
		simulatedSend(c, 7);
		++cicli;
	}
	assert(c->getSendBuffer().empty());

	std::cout << "[OK] un messaggio accodato durante un invio parziale non corrompe il buffer" << std::endl;
}

static void testClientSenzaDatiNonRichiedePollout()
{
	Server server;
	Client *c = server.addClient(43);

	assert(!needsPollout(c)); // appena creato, nessun dato da inviare

	c->queueMessage("qualcosa");
	assert(needsPollout(c));

	c->clearSendBuffer(); // simula un flush manuale/reset
	assert(!needsPollout(c));

	std::cout << "[OK] un client senza dati in coda non richiede mai POLLOUT" << std::endl;
}

static void testBroadcastPopolaPollutSoloPerIDestinatari()
{
	Server server;
	Client *alice = server.addClient(44);
	Client *bob = server.addClient(45);
	Client *carol = server.addClient(46);

	Channel *chan = server.createChannel("#poll");
	chan->addClient(alice);
	chan->addClient(bob);
	chan->addClient(carol);

	// alice manda un messaggio: lei stessa e' esclusa dal broadcast
	chan->broadcast("ciao a tutti", alice);

	assert(!needsPollout(alice)); // il mittente non deve auto-ricevere l'eco
	assert(needsPollout(bob));
	assert(needsPollout(carol));

	std::cout << "[OK] dopo un broadcast, solo i destinatari (non il mittente) richiedono POLLOUT" << std::endl;
}

int main()
{
	testSendCompletoInUnaSolaChiamata();
	testSendParzialeSuPiuCicli();
	testMessaggioAccodatoDuranteInvioParziale();
	testClientSenzaDatiNonRichiedePollout();
	testBroadcastPopolaPollutSoloPerIDestinatari();

	std::cout << "\nTutti i test del flusso di invio sono passati." << std::endl;
	return (0);
}
