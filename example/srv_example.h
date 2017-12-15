#pragma once
#include "srv_interface.h"
#include <QMap>
#include <QList>
#include <QPair>
#include <QMutex>

class ExampleModuleCommand;

typedef QList<QPair<QString, QString> > QPairStringList;

class Example
{
public:
	Example();
	bool cq(QString a, QString b);
};

class ExampleModule 
{
	static QMap<QString, ExampleModuleCommand*> *commands;
	Example ex;
	QMutex m;
	ExampleModule();
public:
	static ExampleModule *init();

	bool handleRequest(messageData &req, messageData &res, void **callback);
	bool handleCallback(void *callback);
	static void addCommand(ExampleModuleCommand* c);
};