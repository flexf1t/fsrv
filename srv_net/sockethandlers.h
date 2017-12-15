#pragma once
#include <QTcpSocket>
#include <QMutex>
#include <QStringList>
#include <QFile>
#include <QFileInfo>

class FSrvHandlerInterface
{
protected:
	QString tempRoot;
public:
	virtual ~FSrvHandlerInterface() {}
	virtual void setParams(QByteArray params) = 0;
	virtual void setTempRoot(QByteArray path) { tempRoot = path; }
	virtual int handleData(const QByteArray &data) = 0;
	virtual QString getCurrentFileName() = 0;
	virtual void close() {}

	static void removeCrlf(char *buffer, int &len);
	enum stages
	{
		newContentStage = 0,
		headersStage,
		sizeStage,
		crlfStage,
		dataStage,
		endStage,
		errorStage
	};
};
class FileReader : public FSrvHandlerInterface
{
	QString relativePath;
	QFile file;
	bool opened;
	bool setFile(QString path, qint64 lastMod = 0);
public:
	~FileReader() { close(); }
	void close() override;
	void setParams(QByteArray disposition) override;
	int handleData(const QByteArray &data)  override;
	QString getCurrentFileName() override { return relativePath; }
};

class MultipartReader : public FSrvHandlerInterface
{
	FileReader file;
	QByteArray temp;
	QByteArray boundary;
	int stage;

	int deferredLineFeedSize;
	bool checkBoundary(QByteArray line);
	int handleDataImpl(QByteArray &data);
public:
	void close() override { file.close(); }
	int handleData(const QByteArray &data) override;
	void setParams(QByteArray contentType) override;
	void setTempRoot(QByteArray path) override;
	QString getCurrentFileName() override { return file.getCurrentFileName(); }
};

class MultipartWriter
{
	static const QByteArray lines;
	static const QByteArray octetStream;
	int MAX_PART_BUFFER;

	QByteArray boundary;
	int stage;
	QString basePath;
	QStringList paths;

	QFileInfo info;
	QFile currentFile;
	QString currentFilePath;
	QString currentFileName;

	void generateBoundary();
	void clear();
public:
	MultipartWriter() { MAX_PART_BUFFER = 1048576; clear(); } //1mb
	void setDir(QString path);
	void setFiles(QString path, QStringList files);
	void setBufferSize(int size) { MAX_PART_BUFFER = size; }

	QByteArray getBoundary() { return boundary; }
	QByteArray getPart(int *fileReadedBytes = 0);
	QString getCurrentFileName() { return currentFileName; }
};
class ChunkReader
{
	static const int CHUNK_SIZE = 4096; //to settings mb?
	int stage;
	int chunkSize;
	QByteArray sizeBuffer;
	char *crlfBuffer;
	char temp[CHUNK_SIZE];
public:
	ChunkReader() { crlfBuffer = new char[3]; clear(); }
	~ChunkReader() { delete [] crlfBuffer; }
	void clear() { sizeBuffer.clear(); chunkSize = 0; stage = FSrvHandlerInterface::sizeStage; }
	int handle(QTcpSocket &s, QByteArray &data);
	bool isFinished() { return stage == FSrvHandlerInterface::endStage; }
	static void writeChunked(QTcpSocket &s, QByteArray data);
};