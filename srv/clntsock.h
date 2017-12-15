#pragma once
#include <QTcpSocket>
#include <QQueue>
#include <QSharedPointer>
#include <QByteArray>
#include "settings.h"
#include "sockethandlers.h"

class SocketMessage;
class ModuleMessage;
class ClientSocket : public QTcpSocket
{
	Q_OBJECT
	SrvSettings *s;
	ChunkReader cr;
	MultipartWriter mw;

	FSrvHandlerInterface *contentReader;
	MultipartReader mr;
	FileReader fr;

	char *buffer;

	SocketMessage *currentMessage;
	ModuleMessage *currentOutput;
	QQueue<ModuleMessage*> outputQueue;

	QByteArray readParams;
	qint64 contentLength;
	int count;
	bool keepAlive;
	bool chunked;
	int stage;
	//bool authorized;

	void clear();
	void writeModuleMessage(ModuleMessage &message);
signals:
	void requestReady(SocketMessage *readResult);
public slots:
	//void processRequestProxy(SocketMessage *readResult);
	void writeAnswer(ModuleMessage *message);
	void readRequest();
public:
	QSharedPointer<ClientSocket> *sharedptr;
	ClientSocket(QObject *parent, SrvSettings *settings);
	~ClientSocket();
};
