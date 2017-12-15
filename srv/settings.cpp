#include "settings.h"
#include <QCoreApplication>
#include <QDir>
#include <QThreadPool>


QMutex SrvSettings::l;
SrvSettings *SrvSettings::s = 0;
SrvSettings::SrvSettings() 
{
	makeDefault();
	QMutexLocker lock(&l);
	if (s)
		*this = *s; //copy fields
}
SrvSettings::SrvSettings(bool noLock)
{
	makeDefault();
}
void SrvSettings::makeDefault()
{	
#ifdef DEBUG
	debug = true;
#else
	debug = false;
#endif
	tempFileName = "bytes";
	port = 29980;
	threads = QThreadPool::globalInstance()->maxThreadCount();
	maxPendingConnections = 30; //qt default
	maxLineSize = 32768; //32kb
	maxBufferSize = 1048576; //1mb
	maxContentSize = maxBufferSize * 16; //16mb
	maxFileSize = maxContentSize * 32; //512mb
	keepAliveOff = false;
	maxHeaders = 128;
}

void SrvSettings::update(QString filePath)
{
	QMutexLocker lock(&l);
	if (!s)
		s = new SrvSettings(true);

	//QString filePath = QCoreApplication::applicationDirPath() + "/srv_pub.ini";
	if (QFile::exists(filePath))
		read(filePath);
	else
		write(filePath);

	//check tempdir
	if (s->tempDir.isEmpty())
		s->tempDir = QDir::tempPath();
	else
		QDir().mkpath(s->tempDir);
	
	QDir dir(s->tempDir);
	s->tempDir = dir.absolutePath();
}

void SrvSettings::read(const QString& name)
{
	if (!s) return;
	QSettings settings(name, QSettings::IniFormat);
	settings.setIniCodec("UTF-8");
	settings.beginGroup("Main");
	if (settings.contains("Port"))
		s->port = settings.value("Port").toInt();
	if (settings.contains("TempDir"))
		s->tempDir = settings.value("TempDir").toString();
	if (settings.contains("TempFileName"))
		s->tempFileName = settings.value("TempFileName").toString();
	if (settings.contains("Modules"))
		s->modules = settings.value("Modules").toStringList();
	if (settings.contains("Log"))
		s->log = settings.value("Log").toString();
	if (settings.contains("Threads"))
		s->threads = settings.value("Threads").toInt();
	if (settings.contains("Debug"))
		s->debug = settings.value("Debug").toBool();
	settings.endGroup();

	settings.beginGroup("Http");
	if (settings.contains("MaxPendingConnections"))
		s->maxPendingConnections = settings.value("MaxPendingConnections").toInt();
	if (settings.contains("MaxLineSize"))
		s->maxLineSize = settings.value("MaxLineSize").toInt();
	if (settings.contains("MaxBufferSize"))
		s->maxBufferSize = settings.value("MaxBufferSize").toInt();
	if (settings.contains("MaxContentSize"))
		s->maxContentSize = settings.value("MaxContentSize").toLongLong();
	if (settings.contains("MaxFileSize"))
		s->maxFileSize = settings.value("MaxFileSize").toLongLong();
	if (settings.contains("KeepAliveOff"))
		s->keepAliveOff = settings.value("KeepAliveOff").toBool();
	if (settings.contains("MaxHeaders"))
		s->maxHeaders = settings.value("MaxHeaders").toInt();

	settings.endGroup();
}
void SrvSettings::write(const QString& name)
{
	if (!s) return;
	QSettings settings(name, QSettings::IniFormat);
	settings.setIniCodec("UTF-8");
	settings.beginGroup("Main");
	settings.setValue("Port", s->port);
	if (!s->tempDir.isEmpty()) settings.setValue("TempDir", s->tempDir);
	if (!s->tempFileName.isEmpty() && s->tempFileName != "bytes") settings.setValue("TempFileName", s->tempDir);
	if (!s->modules.isEmpty()) settings.setValue("Modules", s->modules);
	if (!s->log.isEmpty()) settings.setValue("Log", s->log);
	if (s->threads != QThreadPool::globalInstance()->maxThreadCount())
		settings.setValue("Threads", s->threads);
	settings.endGroup();

	settings.beginGroup("Http");
	if (s->maxPendingConnections != 30)
		settings.setValue("MaxPendingConnections", s->maxPendingConnections);
	settings.setValue("MaxLineSize", s->maxLineSize);
	settings.setValue("MaxBufferSize", s->maxBufferSize);
	settings.setValue("MaxContentSize", s->maxContentSize);
	settings.setValue("MaxFileSize", s->maxFileSize);
	if (s->keepAliveOff) settings.setValue("KeepAliveOff", s->keepAliveOff);
	settings.setValue("MaxHeaders", s->maxHeaders);
	settings.endGroup();
}