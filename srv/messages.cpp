#include "messages.h"
#include "dirmng.h"
#include <QUuid>
#include <QtConcurrentRun>
#include <QFuture>
#include <QMetaType>

//Q_DECLARE_METATYPE(SocketMessage)
//Q_DECLARE_METATYPE(ModuleMessage)

struct MessageHelperForqRegMetaType
{
	MessageHelperForqRegMetaType()
	{
		qRegisterMetaType<SocketMessage*>("SocketMessage*");
		qRegisterMetaType<ModuleMessage*>("ModuleMessage*");
	}
};
static MessageHelperForqRegMetaType mhfqrmt;

IOMessage::IOMessage()
{
	headers.reserve(RESERVED_HEADERS_VECTOR_SIZE);
	path = false;
}
IOMessage::~IOMessage() { }

void IOMessage::copyPointer(IOMessage *m)
{
	if (m) sharedptr = m->sharedptr;
}
void IOMessage::copyPointer(QSharedPointer<ClientSocket> *p)
{
	if (p) sharedptr = *p;
}

SocketMessage::SocketMessage() : IOMessage()
{
	firstLine = false;
}
SocketMessage::~SocketMessage()
{
	if (path)
	{
		QString deletePath = QString(body);
		if (!deletePath.isEmpty())
			QtConcurrent::run(&(DirMng::RemoveDirectory), deletePath);
	}
}
void SocketMessage::getRaw(messageData &raw)
{
	raw.body = body.data();
	raw.size = body.size();
	raw.path = path;
	raw.headersSize = headers.size();
	raw.headers = new char*[raw.headersSize];

	for (int i = 0; i < headers.size(); i++)
		raw.headers[i] = headers[i].data();
}
void SocketMessage::generateUniqueTempDir(QString tempRoot)
{
	QString uuid = QUuid::createUuid();
	body = DirMng::Combine(tempRoot, uuid).toUtf8();
	path = true;
}

ModuleMessage::ModuleMessage() : IOMessage()
{
	callback = 0;
	messageCount = 0;
	headersPtr = 0;
	bodyPtr = 0;
}
ModuleMessage::~ModuleMessage()
{
	if (bodyPtr)
		delete [] bodyPtr;
	if (headersPtr)
	{
		for (int i = 0; i < headersSize; i++)
			delete [] headersPtr[i];
		delete [] headersPtr;
	}
}
void ModuleMessage::setRaw(messageData &raw)
{
	path = raw.path;
	if (raw.body)
	{
		bodyPtr = raw.body;
		body = QByteArray::fromRawData(raw.body, raw.size);
	}
	if (raw.headers)
	{
		headersSize = raw.headersSize;
		headersPtr = raw.headers;
		for (int i = 0; i < raw.headersSize; i++)
		{
			QByteArray ba = QByteArray::fromRawData(raw.headers[i], strlen(raw.headers[i]));
			headers.append(ba);
		}
	}
}