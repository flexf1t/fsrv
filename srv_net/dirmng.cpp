#include "dirmng.h"
#include <QDateTime>

const QString DirMng::relativeSym = QString("../");
const QByteArray DirMng::crlf = QByteArray("\r\n");
const QByteArray DirMng::lf = QByteArray("\n");

bool DirMng::RemoveDirectory(const QString &path)
{
	bool result = true;
	QDir dir(path);

	if (dir.exists())
	{
		foreach (QFileInfo info, dir.entryInfoList(QDir::NoDotAndDotDot | QDir::System | QDir::Hidden | QDir::AllDirs | QDir::Files, QDir::DirsFirst))
		{
			if (info.isDir())
				result = RemoveDirectory(info.absoluteFilePath());
			else
				result = QFile::remove(info.absoluteFilePath());
			if (!result)
				return result;
		}
		result = dir.rmdir(path);
	}
	return result;
}

QStringList DirMng::GetSub(const QString &srcPath, const QString &nameFilters, QDir::Filters filters)
{
	QDir sourceDir(srcPath);
	QStringList fileNames = sourceDir.entryList(QStringList(nameFilters), filters);
	return fileNames;
}
QStringList DirMng::GetSubRelativeToSrc(const QString &srcPath, QString subPath)
{
	QString fullPath = DirMng::Combine(srcPath, subPath);
	QDir sourceDir(fullPath);
	QStringList list = sourceDir.entryList(QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot);
	for (int i = 0; i < list.size(); i++)
		list[i] = DirMng::Combine(subPath, list[i]);
	return list;
}
QStringList DirMng::GetAllSubFiles(const QString& srcPath, const QString& path)
{
	QDir sourceDir(Combine(srcPath, path));
	QStringList result;
	QStringList fileNames = sourceDir.entryList(QDir::Files | QDir::NoDotAndDotDot);
	for (int i = 0; i < fileNames.size(); i++)
		result << Combine(path, fileNames[i]);

	QStringList dirNames = sourceDir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);
	for (int i = 0; i < dirNames.size(); i++)
		result << GetAllSubFiles(srcPath, Combine(path, dirNames[i]));
	return result;
}

QString DirMng::Combine(const QString& s1, const QString& s2)
{
	if (s2.isEmpty())
		return s1;
	if (s1.isEmpty())
		return s2;
	if (s1.lastIndexOf("/") == s1.length() || s2.lastIndexOf("/") == 0)
		return s1 + s2;
	else
		return s1 + "/" + s2;
}
QString DirMng::Combine(const QString& s1, const QString& s2, const QString& s3)
{
	return Combine(Combine(s1, s2), s3);
}

bool DirMng::CreateDirIfNotExists(const QString& path)
{
	QDir dir(path);
	if (!dir.exists())
		return dir.mkpath(".");
	return true;
}

int DirMng::GetFreeDirNumber(const QString& path)
{
	int i = 0;
	QString tmpPath = Combine(path, QString::number(i));	
	while (i >= 0)
	{
		QFileInfo fi(tmpPath);
		if (!fi.exists() || !fi.isDir())
			return i;
		else
		{
			i++;
			tmpPath = Combine(path, QString::number(i));
		}
	}
	return -1;
}

//QString DirMng::GetFileLastModified(const QString& path)
//{
//	QFileInfo fi(path);
//	QDateTime dt = fi.lastModified();
//	return dt.toString("yyyy-MM-dd");
//}
//
//int DirMng::GetMandate()
//{
//	/*FILE *fp = popen("macid -l", "r");
//	if (fp == NULL)
//		return -1;
//
//	char m[8];
//	fgets(m, sizeof(m) - 1, fp);
//	QString res = QString(m);
//	pclose(fp);
//	return res.toInt();*/
//	return 0;
//}