#include "sockethandlers.h"
#include <QDir>
#include <QDateTime>
#include <QCoreApplication>
#include <time.h>
#include "dirmng.h"

#if defined(_MSC_VER) // Microsoft Visual Studio
#define _FSRV_WIN32
//#define _FSRV_PDA
#elif defined(__BORLANDC__) // Borland C++ Builder
#define _FSRV_WIN32
#elif defined(__MINGW32__)
#define _FSRV_WIN32
#else
#define _FSRV_MCBC
#endif

#ifdef _FSRV_WIN32
#include <sys/utime.h>
#else
#include <utime.h>
#endif

void FSrvHandlerInterface::removeCrlf(char *buffer, int &len)
{
	if (len > 1 && buffer[len - 2] == '\r')
	{
		buffer[len - 2] = 0;
		len -= 2;
	}
	else
	{
		buffer[len - 1] = 0;
		len -= 1;
	}
}

bool FileReader::setFile(QString path, qint64 lastMod)
{
	close();
	path.replace('\\', '/');//to unix separators anyway
	if (path.contains(DirMng::relativeSym))
		return false;

	QDir dir = QFileInfo(path).absoluteDir();
	if (!dir.exists())
		if (!dir.mkpath("."))
			return false;
			
	file.setFileName(path);
	if (!file.open(QIODevice::WriteOnly))
	{
		qWarning() << "FileHandler: cannot create file" << path;
		return false;
	}
	opened = true;
	if (lastMod <= 0)
		return opened;

	utimbuf t;
	t.modtime = lastMod;
	t.actime = lastMod;

	utime(path.toLocal8Bit().data(), &t);

	return opened;
}
void FileReader::close()
{
	file.close();
	opened = false;
}
int FileReader::handleData(const QByteArray &data)
{
	return opened ? file.write(data) : 0;
}
void FileReader::setParams(QByteArray disposition)
{
	close();
	qint64 lastMod = 0;
	QList<QByteArray> list = disposition.split(';');
	for (int i = 0; i < list.size(); i++)
	{
		int pos = list[i].indexOf('=');
		if (pos <= 0)
			continue;
		QByteArray key = list[i].left(pos).toLower().trimmed();
		QByteArray value = list[i].mid(pos + 1, list[i].size() - pos);
		if (value.size() < 0)
			continue;
		else if (value[0] == '"')
		{
			value.remove(0, 1);
			value.remove(value.size() - 1, 1);
		}
		if (key == "filename")
			relativePath = QString::fromUtf8(value).replace('\\', '/');//to unix separators anyway
		else if (key == "lastmod")
		{
			bool ok = false;
			lastMod = value.simplified().toLongLong(&ok) / 1000; //to seconds
			if (!ok) lastMod = 0;
		}
	}
	if (!relativePath.isEmpty())
		setFile(DirMng::Combine(tempRoot, relativePath), lastMod);
}

void MultipartReader::setParams(QByteArray contentType)
{
	stage = newContentStage;
	deferredLineFeedSize = 0;
	file.close();
	boundary.clear();
	temp.clear();
	QStringList l = QString(contentType).split("boundary=");
	if (l.size() == 2)
		boundary = l[1].toUtf8();
}
void MultipartReader::setTempRoot(QByteArray path)
{
	tempRoot = path;
	file.setTempRoot(path);
}
bool MultipartReader::checkBoundary(QByteArray line)
{
	if (line.size() <= boundary.size())
		return false;
	int sizediff = (line.size() - boundary.size());
	if (sizediff < 2 || sizediff > 4)
		return false;
	if (line[0] != '-' || line[1] != '-')
		return false;
	bool endPossible = false;
	int originalSize = line.size();
	if (line[line.size() - 2] == '-' && line[line.size() - 1] == '-')
	{
		endPossible = true;
		line.remove(line.size() - 2, 2);
	}
	line.remove(0, 2);
	if (line == boundary)
		stage = endPossible ? endStage : headersStage;
	return true;
}
int MultipartReader::handleData(const QByteArray &data)
{
	QByteArray ba = data;
	if (!temp.isEmpty())
	{
		ba.prepend(temp);
		temp.clear();
	}	
	QByteArray ref;
	char *raw = ba.data();
	int size = ba.size();
	int readed = 0;
	do
	{
		ref.setRawData(raw, size);
		readed = handleDataImpl(ref);
		raw += readed;
		size -= readed;
	}
	while(readed > 0 && size > 0);
	if (size > 0)
		temp = QByteArray(raw, size);
	return ba.size() - size;
}
int MultipartReader::handleDataImpl(QByteArray &data)
{
	if (boundary.isEmpty() || stage == endStage)
		return data.size();
	int newLineIndex = data.indexOf('\n');
	if (newLineIndex < 0 && stage != dataStage)
		return 0;

	int newLineSize = 1;
	if (newLineIndex > 0 && data[newLineIndex - 1] == '\r')
	{
		newLineIndex--;
		newLineSize++;
	}
	QByteArray lineData = data.left(newLineIndex);//hot
	bool boundFound = false;
	if (newLineIndex < 0)
	{
		if (data.size() < (boundary.size() + 4)) // data may be part of boundary -> need to wait full boundary
			return 0;
		boundFound = checkBoundary(data);
	}
	else
		boundFound = checkBoundary(lineData);

	if (boundFound)
	{
		deferredLineFeedSize = 0;
		file.close();
		return lineData.size() + newLineSize;
	}

	switch (stage)
	{
		case headersStage:
		{
			if (lineData.size() == 0)
			{
				stage = dataStage;
				return newLineSize;
			}

			int pos = lineData.indexOf(':');
			QByteArray header = lineData.left(pos).trimmed().toLower();
			QByteArray value = lineData.mid(pos + 1, lineData.size() - pos).trimmed();

			if (header == "content-disposition")
				file.setParams(value);
		}
		//!!no break - its ok
		case newContentStage:
			return lineData.size() + newLineSize;
		case dataStage:
		{
			if (deferredLineFeedSize != 0)
			{
				file.handleData(deferredLineFeedSize == 1 ? DirMng::lf : DirMng::crlf);
				deferredLineFeedSize = 0;
			}
			if (lineData.size())
			{
				deferredLineFeedSize = newLineSize;
				return file.handleData(lineData) + newLineSize;
			}
			else if (newLineIndex != -1)//if (newLineIndex == 0 || newLineIndex == 1) - if line data is empty newLineIndex always will be 0 or 1
			{
				deferredLineFeedSize = newLineSize;
				//file.handleData(newLineIndex == 0 ? DirMng::lf : DirMng::crlf);
				return newLineSize;
			}
			return file.handleData(data);
		}
		case endStage:
			return data.size();
		default: break;
	}
	return 0;
}
/////////////////////////////////////////////
const QByteArray MultipartWriter::lines = QByteArray("--");
const QByteArray MultipartWriter::octetStream = QString("Content-Type: application/octet-stream\r\n").toUtf8();
void MultipartWriter::setDir(QString path) 
{ 
	setFiles(path, DirMng::GetSubRelativeToSrc(path, "")); 
}
void MultipartWriter::generateBoundary()
{
	boundary = QByteArray("fsrvbound_.oOo._")
		+ QByteArray::number(qrand()).toBase64() 
		+ QByteArray::number(qrand()).toBase64()
        + QByteArray::number(qrand()).toBase64();

    // boundary must not be longer than 70 characters, see RFC 2046, section 5.1.1
    if (boundary.count() > 70)
		boundary = boundary.left(70);
}
void MultipartWriter::clear()
{
	stage = FSrvHandlerInterface::newContentStage;
	basePath.clear();
	paths.clear();
	boundary.clear();
	currentFilePath.clear();
	currentFileName.clear();
}
void MultipartWriter::setFiles(QString path, QStringList files)
{
	clear();
	basePath = path;
	//TODO: may be insert?
	paths = files + paths;
	generateBoundary();
}
QByteArray MultipartWriter::getPart(int *fileReadedBytes)
{
	QByteArray res;
	if (boundary.isEmpty() || basePath.isEmpty())
		return res;
	switch (stage)
	{
		case FSrvHandlerInterface::newContentStage:
			if (paths.isEmpty())
				return res;
			res.append(DirMng::crlf).append(lines).append(boundary).append(DirMng::crlf);
			stage = FSrvHandlerInterface::headersStage;
			break;
		case FSrvHandlerInterface::headersStage:
		{
			if (paths.isEmpty())
				return res;
			currentFile.close();
			long long fileSize = 0;
			long long lastMod = 0;
			while(!paths.isEmpty())
			{
				currentFileName = paths.takeFirst();
				currentFilePath = DirMng::Combine(basePath, currentFileName);

				info.setFile(currentFilePath);
				if (info.isDir())
					paths = DirMng::GetSubRelativeToSrc(basePath, currentFileName) + paths;
					//paths.prepend(DirMng::GetSubRelativeToSrc(basePath, currentFileName));
				else
				{
					fileSize = info.size();
					lastMod = info.lastModified().toMSecsSinceEpoch();
					currentFile.setFileName(currentFilePath);
					if (fileSize == 0 || !currentFile.exists() || !currentFile.open(QIODevice::ReadOnly))
						continue;
					break;
				}
			}
			res.append(octetStream);

			QString dispostion = "Content-Disposition: attachment; filename=\"%1\"; lastmod=%2\r\n\r\n";
			dispostion = dispostion.arg(currentFileName).arg(lastMod);
			res.append(dispostion.toUtf8());

			stage = FSrvHandlerInterface::dataStage;
			break;
		}
		case FSrvHandlerInterface::dataStage:
		{
			res = currentFile.read(MAX_PART_BUFFER);
			if (fileReadedBytes)
				*fileReadedBytes = res.size();
			if (currentFile.atEnd())
			{
				currentFile.close();
				if (paths.isEmpty())
				{
					res.append(DirMng::crlf).append(lines).append(boundary).append(lines).append(DirMng::crlf);
					stage = FSrvHandlerInterface::endStage;
				}
				else
					stage = FSrvHandlerInterface::newContentStage;
			}
			break;
		}
		case FSrvHandlerInterface::endStage:
		default: break;
	}
	return res;
}
/////////////////////////////////////////////
int ChunkReader::handle(QTcpSocket &s, QByteArray &data)
{
	bool ok = false;
	if (!s.isOpen())// || getStop())
		return 0;
	switch (stage)
	{
		case FSrvHandlerInterface::sizeStage: //chunk size
		{ 
			int size = s.readLine((char*)&temp, CHUNK_SIZE);
			QByteArray ba = QByteArray::fromRawData((char*)&temp, size);
			if (ba.isEmpty())
				return 0;
			if (ba[ba.size() - 1] != '\n') //not full line
			{
				sizeBuffer.append(ba);
				return 0;
			}
			else if (!sizeBuffer.isEmpty())
			{
				ba.prepend(sizeBuffer);
				sizeBuffer.clear();
			}

			chunkSize = ba.simplified().toInt(&ok, 16);
			if (!ok)
				return -1; 
			if (chunkSize == 0) // end of chunks
				stage = FSrvHandlerInterface::endStage;
			else
				stage = FSrvHandlerInterface::dataStage;
			return size;
		}
		case FSrvHandlerInterface::dataStage: //chunk data
		{ 
			data = s.read(chunkSize);
			if (data.isEmpty())
				return 0;
			chunkSize -= data.size();
			if (chunkSize == 0)
				stage = FSrvHandlerInterface::crlfStage;
			else if (chunkSize < 0)
				stage = FSrvHandlerInterface::crlfStage;
			return data.size();
		}
		case FSrvHandlerInterface::crlfStage: //wait for \n
		{ 
			int len = s.readLine(crlfBuffer, 2); //just \r\n
			if ((len == 2 && crlfBuffer[0] == '\r') || crlfBuffer[0] == '\n')
				stage = FSrvHandlerInterface::sizeStage;
			return len;
		}
		default:
		case FSrvHandlerInterface::endStage: //wait for \n
			return 0;
	}
	return 0;
}
void ChunkReader::writeChunked(QTcpSocket &s, QByteArray data)
{
	if (!s.isOpen())
		return;

	int done = 0;
	char *ptr = data.data();
	int totalSize = data.size();
	while (done < totalSize)
	{
		int size = CHUNK_SIZE;
		if (done + size > totalSize)
			size = totalSize - done;
		s.write(QByteArray().setNum(size, 16));
		s.write("\r\n");
		s.write(ptr, size);
		s.write("\r\n");
		ptr += size;
		done += size;
	}
	while (s.bytesToWrite() > 0 && s.flush())
	{
		//nothing
	}
}