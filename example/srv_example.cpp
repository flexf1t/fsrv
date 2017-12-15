#include "srv_example.h"
#include "srv_commands.h"
#include <QCoreApplication>
#include <QDir>
#include <QSettings>
#include <QUrl>
#include <QDebug>

Example::Example()
{
	qDebug() << "Example: hello world";
}
bool Example::cq(QString a, QString b)
{
	return (a.compare(b, Qt::CaseInsensitive) == 0);
}

////////////////////////////////////////

ExampleModule *ExampleModule::init()
{
	return new ExampleModule();
}
ExampleModule::ExampleModule() : ex()
{
}
QMap<QString, ExampleModuleCommand*> *ExampleModule::commands = 0;

void ExampleModule::addCommand(ExampleModuleCommand* c)
{
	if (!commands)
		commands = new QMap<QString, ExampleModuleCommand*>();
	commands->insert(c->getName().toLower(), c);
}

bool ExampleModule::handleRequest(messageData &req, messageData &res, void **callback)
{
	QPairStringList headers;
	QString resultHeader, commandName, httpVersion;
	QByteArray resultBody;

	bool commandDone = false;
	int i = 0;
	if (req.headersSize % 2 == 1)
	{
		i++;
		httpVersion = "HTTP/1.1";
		QString reqType;
		QString firstLine(req.headers[0]);
		QStringList list = firstLine.split(' ', QString::SkipEmptyParts);
		if (list.size() == 3)
		{
			reqType = list[0];
			int index = list[1].indexOf('?');
			QString url = list[1].mid(0, index);
			QString q;
			if (index >= 0)
				q = list[1].mid(index, -1);
			if (!url.isEmpty())
			{
				index = url.lastIndexOf(".example");
				if (index != -1)
					commandName = url.mid(0, index);
				if (!commandName.isEmpty() && commandName[0] == '/')
					commandName.remove(0, 1);
			}
			if (!q.isEmpty())
			{
				QUrl url = QUrl::fromEncoded(q.toUtf8());
				headers.append(url.queryItems());
			}
		}
	}

	for (; i < req.headersSize; i+=2)
	{
		QPair<QString, QString> p(
			QString::fromUtf8(req.headers[i], strlen(req.headers[i])),
			QString::fromUtf8(req.headers[i + 1], strlen(req.headers[i + 1]))
		);
		headers.append(p);
		if (!strcmp(req.headers[i], "command"))
			commandName = QString::fromUtf8(req.headers[i + 1]);
	}
	bool commandFound = false;
	if (!commandName.isEmpty())
	{
		QByteArray data = QByteArray::fromRawData(req.body, req.size);
		ExampleModuleCommand *cmd = commands->value(commandName.toLower(), 0);
		if (cmd)
		{
			resultHeader += "Command: " + commandName + "\r\n";
			m.lock();
			commandDone = cmd->execute(ex, headers, data, req.path, resultHeader, resultBody, res.path);
			m.unlock();
			commandFound = true;
			if (!httpVersion.isEmpty())
			{
				if (resultBody.isEmpty())
					resultHeader.prepend(httpVersion + " 204 No Content\r\n");
				else
					resultHeader.prepend(httpVersion + " 200 OK\r\n");
			}
		}
	}
	if (!commandFound)
		return false;

	//sending:
	if (!resultBody.isEmpty())
	{
		if (res.path)
		{
			m.lock();
			*callback = &m;
		}
		res.size = resultBody.size();
		res.body = new char[res.size];
		memcpy(res.body, resultBody.data(), res.size);
	}
	if (!resultHeader.isEmpty())
	{
		QByteArray h = resultHeader.toUtf8();
		res.headersSize = 1;//h.size();
		res.headers = new char*[1];
		res.headers[0] = new char[h.size() + 1];
		memcpy(res.headers[0], h.data(), h.size());
		memset(res.headers[0] + h.size(), 0, sizeof(char));
	}
	return true;
}

bool ExampleModule::handleCallback(void *callback)
{
	if (!callback)
		return true;
	QMutex *s = (QMutex *)callback;
	s->unlock();
	return true;
}