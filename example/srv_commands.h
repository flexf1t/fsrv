#pragma once
#include "srv_example.h"
#include <QString>

#define STORE_SRV_COMMAND_ARGS Example &ex, const QPairStringList &headers, const QByteArray &body, const bool path, QString &resultHeader, QByteArray &resultBody, bool &resultPath

class ExampleModuleCommand
{
protected:
	virtual void done(QString &h);
	virtual void error(QString &h, QString &e);
	virtual void execute_impl(STORE_SRV_COMMAND_ARGS) = 0;
public:
	virtual ~ExampleModuleCommand();
	virtual QString getName() = 0;
	bool execute(STORE_SRV_COMMAND_ARGS);
};

#define DECLARE_COMMAND(X) class X : public ExampleModuleCommand {\
	public:	X() { ExampleModule::addCommand(this); }\
		QString getName();\
		void execute_impl(STORE_SRV_COMMAND_ARGS);\
	}\

DECLARE_COMMAND(testCommand);