#include <QCoreApplication>
#include <QTextCodec>
#include <QTime>
#include <QLocale>
#include <iostream>
#include "srv.h"
#include <signal.h>

struct Exiter
{
	static ModuleFServer *server;
    Exiter(ModuleFServer *_server) {
		server = _server;
        signal(SIGINT, &Exiter::exit);
        signal(SIGTERM, &Exiter::exit);
        //signal(SIGBREAK, &Exiter::exit);
    }

    static void exit(int sig) {
        QCoreApplication::exit(sig);
    }
};

ModuleFServer *Exiter::server = 0;

void consoleMsgOutput(QtMsgType type, const char *msg)
{	
	std::string time = QTime::currentTime().toString("HH:mm:ss.zzz").toStdString();
	switch (type) {
		case QtDebugMsg:
			fprintf(stderr, "[%s] %s\n", time.c_str(), msg);
			break;
		case QtWarningMsg:
			fprintf(stderr, "[%s][Warning] %s\n", time.c_str(), msg);
			break;
		case QtCriticalMsg:
			fprintf(stderr, "[%s][Critical] %s\n", time.c_str(), msg);
			break;
		case QtFatalMsg:
			fprintf(stderr, "[%s][FATAL] %s\n", time.c_str(), msg);
			abort();
    }
}

int main(int argc, char *argv[])
{
	qInstallMsgHandler(consoleMsgOutput);
	QCoreApplication app(argc, argv);
	QLocale curLocale(QLocale::Russian, QLocale::RussianFederation);
	QLocale::setDefault(curLocale);	
	QTextCodec::setCodecForCStrings(QTextCodec::codecForName("UTF-8"));
	
	//setlocale(LC_ALL, "");
	setlocale(LC_NUMERIC, "C");

	QString ini = QCoreApplication::applicationDirPath() + "/srv.ini";
	char *arg = (argc > 1) ? argv[1] : 0;
	if (arg != 0)
		ini = QString(arg);

	ModuleFServer *server = new ModuleFServer(ini);
	if (!server->start())
	{
		std::cerr << "Failed to start server";
		return 1;
	}
	//QObject::connect(&app, SIGNAL(aboutToQuit()), server, SLOT(deleteLater()));
	//Exiter e(server);
	return app.exec();
}
