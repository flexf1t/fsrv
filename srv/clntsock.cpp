#include "clntsock.h"
#include "modmng.h"
#include "dirmng.h"
#include "srv.h"
#include <QDir>
#include <QDebug>
#include <QFileInfo>
#include <QCoreApplication>

//#include <unistd.h>
//#include <sys/types.h>
//#include <errno.h>
//#include <pwd.h>
//#include <shadow.h>
ClientSocket::ClientSocket(QObject *parent, SrvSettings *settings) : QTcpSocket(parent), s(settings)
{
	clear();
	sharedptr = 0;
	keepAlive = false;
	currentOutput = 0;
	buffer = new char[s->maxLineSize];
}
ClientSocket::~ClientSocket()
{
	delete [] buffer;
	buffer = 0;
	if (currentMessage)
		delete currentMessage;
}
void ClientSocket::clear()
{
	readParams.clear();
	contentReader = 0;
	fr.close();
	mr.close();
	cr.clear();
	count = 0;
	chunked = false;
	contentLength = 0;
	stage = FSrvHandlerInterface::headersStage; 
	currentMessage = 0;
}
void ClientSocket::readRequest()
{
	if (!currentMessage)
		currentMessage = new SocketMessage();

	while (true)
	{
		if (QAbstractSocket::state() != QAbstractSocket::ConnectedState)
			break;
		int bytes = bytesAvailable();
		if (bytes <= 0)
			break;
		switch (stage)
		{
			case FSrvHandlerInterface::headersStage:
			{
				int len = readLineData(buffer, s->maxLineSize);
				if (len <= 0)
					continue;
				FSrvHandlerInterface::removeCrlf(buffer, len);
				if (len == 0)
				{
					if (!chunked && contentLength <= 0)
					{
						stage = FSrvHandlerInterface::endStage;
						break;
					}
					stage = FSrvHandlerInterface::dataStage;
					if (contentReader || contentLength > s->maxContentSize)
					{
						if (!contentReader)
						{
							contentReader = &fr;
							QString disp = "filename=\"" + s->tempFileName + "\"";
							readParams = disp.toUtf8();
						}
						currentMessage->generateUniqueTempDir(s->tempDir);
						contentReader->setTempRoot(currentMessage->body);
						contentReader->setParams(readParams);
						if (contentReader == &fr)
							currentMessage->body = DirMng::Combine(currentMessage->body, fr.getCurrentFileName()).toUtf8();
					}
					//if (IsAuthorized())
					//else
				}
				else if (count < s->maxHeaders)
				{
					count++;
					
					char *ptr = (char*)memchr(buffer, ':', len);
					if (!ptr)
					{
						if (count == 1) //first line
						{
							currentMessage->firstLine = true;
							currentMessage->headers.append(QByteArray(buffer, len));
							break;
						}
						break;
					}
					QByteArray name = QByteArray(buffer, ptr - buffer).trimmed().toLower(); //from start to :
					QByteArray value = QByteArray(ptr + 1, len - (ptr - buffer + 1)).trimmed(); //from : to end
					if (name.isEmpty() || value.isEmpty())
						break;
					currentMessage->headers.append(name);
					currentMessage->headers.append(value);

					if (name == "content-length")
					{
						contentLength = value.toLongLong();
						if (s->maxFileSize != 0 && contentLength > s->maxFileSize)
						{
							stage = FSrvHandlerInterface::errorStage;
							break;
						}
					}
					else if (name == "content-type")
					{
						if (value.contains("multipart/"))
						{
							contentReader = &mr;
							readParams = value;
						}
					}
					else if (name == "transfer-encoding" && value.toLower() == "chunked")
						chunked = true;
					else if (name == "connection" && value.toLower() == "keep-alive")
						keepAlive = !s->keepAliveOff;
					else if (name == "content-disposition")
					{
						contentReader = &fr;
						readParams = value;
					}
				}
				else
					stage = FSrvHandlerInterface::endStage; //stop!
				break;
			}
			case FSrvHandlerInterface::dataStage:
			{
				QByteArray ba;
				if (chunked)
				{
					if (cr.handle(*this, ba) == -1) //some error
						stage = FSrvHandlerInterface::errorStage;
					else if (cr.isFinished())
						stage = FSrvHandlerInterface::endStage;
				}
				else if (contentLength > 0)
				{
					ba = read(s->maxBufferSize);
					contentLength -= ba.size();
					if (contentLength == 0) 
						stage = FSrvHandlerInterface::endStage;
				}
				if (!ba.size())
					break;
				if (contentReader)
					contentReader->handleData(ba);
				else
					currentMessage->body.append(ba);
			}
			default:
			break;
		}
		if (stage == FSrvHandlerInterface::endStage || stage == FSrvHandlerInterface::errorStage)
		{ 
			SocketMessage *readResult = currentMessage;
			if (stage != FSrvHandlerInterface::errorStage)
			{
				if (s->debug)
					qDebug() << "<-" << peerAddress().toString() << (readResult->firstLine ? readResult->headers[0] : "");
				readResult->copyPointer(sharedptr);
				emit requestReady(readResult);
			}
			else
				delete readResult;
			clear();
			break;
		}
		
	}
	return;
}
void ClientSocket::writeAnswer(ModuleMessage *message)
{
	if (!message)
		return;
	if (!isOpen())
	{
		delete message;
		return;
	}
	outputQueue.enqueue(message); //â î÷åðåäü!
	if (currentOutput) //óæå ðàáîòàåì
		return;

	while(!outputQueue.isEmpty())
	{
		currentOutput = outputQueue.dequeue();
		writeModuleMessage(*currentOutput);
		if (currentOutput->callback)
			ModuleManager::processEnd(*currentOutput);
		delete currentOutput;
	}
	currentOutput = 0;
	if (keepAlive)
		keepAlive = false;
	else
		close();
}
void ClientSocket::writeModuleMessage(ModuleMessage &message)
{
	if (s->debug && !message.moduleName.isEmpty())
		qDebug() << QString::number(message.messageCount) + "@" + message.moduleName << "->" << peerAddress().toString();
	if (!message.headers.isEmpty())
	{
		for (int i = 0; i < message.headers.size(); i++)
			write(message.headers[i]);
		write(ModuleFServer::server.toUtf8());
		if (keepAlive)
			write(QString("Connection: keep-alive\r\n").toUtf8());
	}
	if (!isOpen())
		return;
	if (message.body.isEmpty())
	{
		write("\r\n");//end
		return;
	}
	if (message.path)
	{
		QString data = QString::fromUtf8(message.body, message.body.size());//.replace('\\', '/'); // to unix way
		QStringList allPaths = data.split('\n');
		for (int i = 0; i < allPaths.size(); i++)
		{
			QString path = allPaths[i];
			QFileInfo info(path);
		
			if (info.isDir())
			{
				mw.setDir(path);
				mw.setBufferSize(s->maxBufferSize);
				write(QString("Transfer-Encoding: chunked\r\n").toUtf8());
				write(QString("Content-Type: multipart/mixed; boundary=").toUtf8());
				write(mw.getBoundary()); write("\r\n");
				write("\r\n");
				while(isOpen())
				{
					QByteArray data = mw.getPart();
					if (data.isEmpty())
						break;
					ChunkReader::writeChunked(*this, data);
					QCoreApplication::processEvents();
				}
				write("0\r\n\r\n");
			}
			if (info.isFile())
			{
				QFile file(path);
				if (!file.exists() || !file.open(QIODevice::ReadOnly))
					return;
				write(QString("Content-Disposition: attachment; filename=\"%1\"\r\n").arg(info.fileName()).toUtf8());
				//write(QString("File-Name: %1\n").arg(info.fileName()).toUtf8());
				write(QString("Content-Type: application/octet-stream\r\n").toUtf8());
				write(QString("Content-Length: %1\r\n\r\n").arg(file.size()).toUtf8());
				char *data = new char[s->maxBufferSize];
				int bufSize;
				while ((bufSize = file.read(data, s->maxBufferSize)) > 0)
				{
					if (!isOpen())
						break;
					write(data, bufSize);
					while (bytesToWrite() > 0 && flush())
					{
						//nothing
					}

					QCoreApplication::processEvents();
				}
				file.close();
				delete [] data;
			}
		}
	}
	else
	{
		write(QString("Content-Length: %1\r\n\r\n").arg(message.body.size()).toUtf8());
		write(message.body);
	}
}