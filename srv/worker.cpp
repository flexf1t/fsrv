#include "worker.h"
#include "clntsock.h"
#include "modmng.h"

Worker::Worker() : QObject(0), busyness(0)
{
	moveToThread(&thread);
	connect(&thread, SIGNAL(finished()), this, SLOT(deleteLater()));
	thread.start();
}
Worker::~Worker()
{
	thread.quit();
    thread.wait();
}
void Worker::newClient(int socketID)
{
	ClientSocket *client = new ClientSocket(this, &s);
	client->setSocketDescriptor(socketID);
	if (!client->isValid())
	{
		delete client;
		return;
	}
	busyness.fetchAndAddOrdered(1);
	client->sharedptr = new QSharedPointer<ClientSocket>(client, &QObject::deleteLater);
	//clients.append(client);
	connect(client, SIGNAL(readyRead()), client, SLOT(readRequest()));
	connect(client, SIGNAL(disconnected()), this, SLOT(removeClient()));
	connect(client, SIGNAL(requestReady(SocketMessage *)), &rp, SLOT(processRequest(SocketMessage *)));
}
void Worker::removeClient()
{
	ClientSocket *client = qobject_cast<ClientSocket*>(sender());
	if (!client)
		 return;
	disconnect(client, SIGNAL(readyRead()), client, SLOT(readRequest()));
	disconnect(client, SIGNAL(disconnected()), this, SLOT(removeClient()));
	disconnect(client, SIGNAL(requestReady(SocketMessage *)), &rp, SLOT(processRequest(SocketMessage *)));
	//int pos = clients.indexOf(client);
	//if (pos != -1)
	//	clients.remove(pos);
	delete client->sharedptr;
	busyness.fetchAndAddOrdered(-1);
}

RequestProcesser::RequestProcesser() : QObject(0)
{
	moveToThread(&thread);
	connect(&thread, SIGNAL(finished()), this, SLOT(deleteLater()));
	thread.start();
}
RequestProcesser::~RequestProcesser()
{
	thread.quit();
    thread.wait();
}
void RequestProcesser::processRequest(SocketMessage *readResult)
{
	if (!readResult)
		return;
	ClientSocket *client = qobject_cast<ClientSocket*>(sender());
	if (client == 0)
	{
		delete readResult;
		return;
	}
	ModuleMessage *m = ModuleManager::processRequest(*readResult);
	if (!m)
	{
		m = new ModuleMessage();
		m->body = "404 Not Found";
		m->headers.append("HTTP/1.1 404 Not Found\r\nContent-Type: text/html\r\n");
	}
	m->copyPointer(readResult);
	delete readResult;
	QMetaObject::invokeMethod(client,
		"writeAnswer",
		Qt::QueuedConnection,
		Q_ARG(ModuleMessage*, m)
	);
}