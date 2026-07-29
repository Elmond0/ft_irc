#include "Server.hpp"
#include "Client.hpp"
#include "Channel.hpp"
#include <iostream>
#include <cassert>

/*
** Test di edge case in isolamento (nessuna rete, nessun parser).
** Ogni funzione copre uno scenario limite discusso con il team.
** In caso di fallimento, assert() interrompe con il messaggio a schermo.
*/

static void testClientPrefixVuoto()
{
	Server server;
	Client *c = server.addClient(10);

	// nickname/username non ancora impostati: non deve crashare
	std::string prefix = c->getPrefix();
	assert(prefix == "!@localhost");

	std::cout << "[OK] Client::getPrefix() con campi vuoti" << std::endl;
}

static void testClientQueueMessageAccumula()
{
	Server server;
	Client *c = server.addClient(11);

	c->queueMessage("primo");
	c->queueMessage("secondo");
	assert(c->getSendBuffer() == "primo\r\nsecondo\r\n");

	c->clearSendBuffer();
	assert(c->getSendBuffer().empty());

	// clear su buffer già vuoto: non deve dare errori
	c->clearSendBuffer();
	assert(c->getSendBuffer().empty());

	std::cout << "[OK] Client::queueMessage accumula, clearSendBuffer idempotente" << std::endl;
}

static void testChannelRemoveClientMaiAggiunto()
{
	Server server;
	Client *estraneo = server.addClient(20);
	Channel *chan = server.createChannel("#test1");

	size_t sizePrima = chan->getClients().size();
	chan->removeClient(estraneo); // non e' mai stato aggiunto
	assert(chan->getClients().size() == sizePrima);

	std::cout << "[OK] Channel::removeClient su client mai aggiunto (no-op)" << std::endl;
}

static void testChannelAddClientDuplicato()
{
	Server server;
	Client *alice = server.addClient(21);
	Channel *chan = server.createChannel("#test2");

	chan->addClient(alice);
	chan->addClient(alice); // stesso client due volte
	assert(chan->getClients().size() == 1);

	std::cout << "[OK] Channel::addClient non duplica lo stesso client" << std::endl;
}

static void testChannelAddOperatorSenzaEssereMembro()
{
	Server server;
	Client *bob = server.addClient(22);
	Channel *chan = server.createChannel("#test3");

	chan->addOperator(bob); // bob non e' membro del canale
	assert(!chan->isOperator(bob));

	std::cout << "[OK] Channel::addOperator rifiuta chi non e' membro" << std::endl;
}

static void testChannelBroadcastCanaleVuoto()
{
	Channel chan("#vuoto");
	chan.broadcast("messaggio a nessuno"); // non deve crashare
	assert(chan.getClients().empty());

	std::cout << "[OK] Channel::broadcast su canale vuoto non crasha" << std::endl;
}

static void testChannelBroadcastExcludeNonMembro()
{
	Server server;
	Client *alice = server.addClient(23);
	Client *bob = server.addClient(24);
	Client *estraneo = server.addClient(25); // non entra nel canale

	Channel *chan = server.createChannel("#test4");
	chan->addClient(alice);
	chan->addClient(bob);

	chan->broadcast("ciao", estraneo); // exclude di qualcuno che non e' dentro
	assert(alice->getSendBuffer().find("ciao") != std::string::npos);
	assert(bob->getSendBuffer().find("ciao") != std::string::npos);

	std::cout << "[OK] Channel::broadcast con exclude non membro funziona comunque" << std::endl;
}

static void testChannelUltimoOperatoreRimosso()
{
	Server server;
	Client *alice = server.addClient(26);
	Channel *chan = server.createChannel("#test5");

	chan->addClient(alice);
	chan->addOperator(alice);
	assert(chan->isOperator(alice));

	chan->removeClient(alice); // se ne va l'unico operatore
	assert(!chan->isOperator(alice));
	assert(chan->isEmpty());

	std::cout << "[OK] Channel: rimozione dell'unico operatore non crasha" << std::endl;
}

static void testChannelModeStringVuota()
{
	Channel chan("#test6");
	assert(chan.getModeString() == ""); // nessuna modalita' attiva

	chan.setInviteOnly(true);
	assert(chan.getModeString() == "+i");

	std::cout << "[OK] Channel::getModeString vuota quando nessun modo e' attivo" << std::endl;
}

static void testServerGetClientByFdInesistente()
{
	Server server;
	assert(server.getClientByFd(999) == 0);

	std::cout << "[OK] Server::getClientByFd su fd inesistente ritorna NULL" << std::endl;
}

static void testServerRemoveClientDoppio()
{
	Server server;
	Client *c = server.addClient(30);
	(void)c;

	server.removeClient(30);
	assert(server.getClientByFd(30) == 0);

	server.removeClient(30); // seconda rimozione: non deve fare doppio delete
	assert(server.getClientByFd(30) == 0);

	std::cout << "[OK] Server::removeClient e' sicuro se chiamato due volte" << std::endl;
}

static void testServerCreateChannelDuplicato()
{
	Server server;
	Channel *chan1 = server.createChannel("#dup");
	Channel *chan2 = server.createChannel("#dup"); // stesso nome

	assert(chan1 == chan2); // deve essere lo STESSO puntatore, non uno nuovo

	std::cout << "[OK] Server::createChannel non duplica canali con stesso nome" << std::endl;
}

static void testServerRemoveChannelIfEmptyConMembri()
{
	Server server;
	Client *alice = server.addClient(31);
	Channel *chan = server.createChannel("#nonvuoto");

	chan->addClient(alice);
	server.removeChannelIfEmpty("#nonvuoto");

	assert(server.getChannel("#nonvuoto") != 0); // non va distrutto

	std::cout << "[OK] Server::removeChannelIfEmpty non tocca un canale con membri" << std::endl;
}

static void testServerRemoveClientPuliscePiuCanali()
{
	Server server;
	Client *alice = server.addClient(32);

	Channel *chanA = server.createChannel("#a");
	Channel *chanB = server.createChannel("#b");
	chanA->addClient(alice);
	chanB->addClient(alice);

	server.removeClient(32); // alice esce dalla rete: deve sparire da entrambi

	assert(chanA->getClients().empty());
	assert(chanB->getClients().empty());

	std::cout << "[OK] Server::removeClient rimuove il client da TUTTI i canali" << std::endl;
}

int main()
{
	testClientPrefixVuoto();
	testClientQueueMessageAccumula();

	testChannelRemoveClientMaiAggiunto();
	testChannelAddClientDuplicato();
	testChannelAddOperatorSenzaEssereMembro();
	testChannelBroadcastCanaleVuoto();
	testChannelBroadcastExcludeNonMembro();
	testChannelUltimoOperatoreRimosso();
	testChannelModeStringVuota();

	testServerGetClientByFdInesistente();
	testServerRemoveClientDoppio();
	testServerCreateChannelDuplicato();
	testServerRemoveChannelIfEmptyConMembri();
	testServerRemoveClientPuliscePiuCanali();

	std::cout << "\nTutti i test edge-case sono passati." << std::endl;
	return (0);
}
