#include "srv.h"
#include "worker.h"
#include "modmng.h"
//#include <iostream>
#include <QThreadPool>
#include <QDir>
#include <QTime>
#include <QDebug>

const QString ModuleFServer::version = "0.6";
const QString ModuleFServer::name = "fsrv";
const QString ModuleFServer::server = "Server: " + ModuleFServer::name + " " + ModuleFServer::version + "\r\n";
char *ModuleFServer::log = 0;
QByteArray ModuleFServer::logBA;

void ModuleFServer::output(QtMsgType type, const char *msg)
{
	std::string time = QTime::currentTime().toString("HH:mm:ss.zzz").toStdString();
	FILE *f = fopen(ModuleFServer::log, "a+");
	if (!f)
		return;
	switch (type) {
		case QtDebugMsg:
			fprintf(f, "[%s] %s\n", time.c_str(), msg);
			break;
		case QtWarningMsg:
			fprintf(f, "[%s][Warning] %s\n", time.c_str(), msg);
			break;
		case QtCriticalMsg:
			fprintf(f, "[%s][Critical] %s\n", time.c_str(), msg);
			break;
		case QtFatalMsg:
			fprintf(f, "[%s][FATAL] %s\n", time.c_str(), msg);
			fclose(f);
			abort();
	}
	fclose(f);
}
ModuleFServer::ModuleFServer(QString ini) : QTcpServer()
{
	SrvSettings::update(ini);
	SrvSettings s;
	if (!s.log.isEmpty())
	{
		ModuleFServer::logBA = s.log.toLocal8Bit();
		ModuleFServer::log = ModuleFServer::logBA.data();
		qInstallMsgHandler(output);
	}
	qDebug() << "Server:" << name << version;
	qDebug() << "Server build date:" << __DATE__ << __TIME__;
	int workersCount = s.threads;
	for (int i = 0; i < workersCount; i++)
	{
		Worker *worker = new Worker();
		workers.append(worker);
	}
	qDebug() << "Server working threads:" << workersCount;
	setMaxPendingConnections(s.maxPendingConnections);
	qDebug() << "Server max pending connections:" << maxPendingConnections();
	qDebug() << "Server temp:" << s.tempDir;
	ModuleManager::load(s.modules);
}
ModuleFServer::~ModuleFServer()
{
	for (int i = 0; i < workers.size(); i++)
		delete workers[i];
	//ModuleManager::unload();
}
bool ModuleFServer::start()
{
	SrvSettings s;
	int port = s.port;
	bool working = listen(QHostAddress::Any, port);
	if (working)
		qDebug() << "Started on port" << port;
	return working;
}
void ModuleFServer::incomingConnection(int socketID)
{
	int size = workers.size();
	if (size <= 0)
		return;
	int min = workers[0]->getBusyness();
	int pos = 0;
	for (int i = 1; i < size; i++)
	{
		int b = workers[i]->getBusyness();
		if (b < min)
		{
			min = b;
			pos = i;
		}
	}
	QMetaObject::invokeMethod(workers[pos],
				"newClient",
				Qt::QueuedConnection,
				Q_ARG(int, socketID));
}