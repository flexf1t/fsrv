#include "srv_commands.h"
#include <QFile>
#include <QDir>
#include <QDebug>

ExampleModuleCommand::~ExampleModuleCommand() {}
bool ExampleModuleCommand::execute(STORE_SRV_COMMAND_ARGS)
{
	try 
	{
#ifdef DEBUG
		qDebug() << "Example: execute command" << getName();
#endif
		execute_impl(ex, headers, body, path, resultHeader, resultBody, resultPath);
	}
	catch (QString e)
	{
		error(resultHeader, e);
		return false;
	} 
	done(resultHeader);
	return true;
}
void ExampleModuleCommand::done(QString &h)
{
	//h.append("Result: Done\r\n");
}
void ExampleModuleCommand::error(QString &h, QString &e)
{
	h.append("Result: Error\r\n");
	h.append("Description: " + e + "\r\n");
	qDebug() << "Example: error" << e;
}

#define COMMAND(X, Y) static X Y;\
	QString X::getName() { return #Y; } \
	void X::execute_impl(STORE_SRV_COMMAND_ARGS)

COMMAND(testCommand, test)
{
	resultHeader.append("Result: Done\r\n");
}