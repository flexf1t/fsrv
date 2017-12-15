#pragma once

#include <QString>
#include <QDir>

class DirMng
{
private:
	DirMng();
	~DirMng();

public:
	static const QString relativeSym;
	static const QByteArray crlf;
	static const QByteArray lf;

	static bool RemoveDirectory(const QString& path);
	static QStringList GetSub(const QString& srcPath, const QString& nameFilters, QDir::Filters filters);
	static QStringList GetSubRelativeToSrc(const QString &srcPath, QString subPath);
	static QStringList GetAllSubFiles(const QString& srcPath, const QString& path);
	static QString Combine(const QString& s1, const QString& s2);
	static QString Combine(const QString& s1, const QString& s2, const QString& s3);
	static bool CreateDirIfNotExists(const QString& path);
	static int GetFreeDirNumber(const QString& path);
};