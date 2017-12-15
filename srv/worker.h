#pragma once
#include <QThread>
#include <QAtomicInt>
#include <QVector>
#include "settings.h"

class ClientSocket;
class Settings;
class SocketMessage;

class RequestProcesser : public QObject
{
	Q_OBJECT
	QThread thread;
public:
	RequestProcesser();
	~RequestProcesser();
public slots:
	void processRequest(SocketMessage *readResult);
};
class Worker : public QObject
{
	Q_OBJECT
	SrvSettings s;
	RequestProcesser rp; //second thread for processing request via modules
	QAtomicInt busyness;
	QThread thread;
	//QVector<ClientSocket*> clients;
public:
	Worker();
	~Worker();
	int getBusyness() { return busyness; }
public slots:
	void newClient(int socketID);
	void removeClient();
};