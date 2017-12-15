#pragma once

//#include <string>
#include <QMutex>
#include <QSettings>
#include <QStringList>

/*! \defgroup SrvSettings Настройки сервера
	\sa SrvMainSettings, SrvHttpSettings
@{*/
//! Класс настроек сервера. \sa SrvMainSettings, SrvHttpSettings
/*! Класс настроек сервера читает настройки из ini-файла при запуске сервера. 
При отсутствии файла создает новый и заполняет значениями по умолчанию.
При отсутствии настройки в файле использует настройку по умолчанию. 
Потокобезопасный. Кодировка ini-файла UTF-8.
После чтения ini-файла, все объекты класса содержат одни и теже прочитанные настройки. */
class SrvSettings 
{
	SrvSettings(bool noLock);
	static SrvSettings *s;
	static QMutex l;
	static void read(const QString& name);
	static void write(const QString& name);
	void makeDefault();
	//SrvSettings(SrvSettings const&);
	//SrvSettings& operator= (SrvSettings const&);
public:
	/// <summary>
	/// Создать объект с текущим (последним прочитанным) состояним настроек.
	/// Изменение объекта не влияет на настройки\другие объекты.
	/// </summary>
	/// <returns></returns>
	SrvSettings();
	/// <summary>
	/// Обновить настройки, прочитав ini-файл
	/// </summary>
	/// <param name="filePath">Путь к ini-файлу</param>
	static void update(QString filePath);
	/*!@}*/
	/*! \defgroup SrvMainSettings Основные настройки сервера
		\ingroup SrvSettings
	@{*/
	//[Main]
	int port; //!< Порт. По умолчанию 29980
	QString tempDir; //!< Временная директория сервера. По умолчанию QDir::tempPath
	QString tempFileName; //!< Имя временного файла. По умолчанию "bytes"
	QStringList modules; //!< Имя или путь к модулям сервера. Перечислены в ini-файле через запятую
	QString log; //!< Путь к лог-файлу
	int threads; //!< Количество потоков для обработки запросов. По умолчанию QThreadPool::globalInstance()->maxThreadCount(). По факту, на каждый поток создается еще один дополнительный, так что их будет threads*2
	bool debug; //!< Флаг дебажных сообщений. True для включения
	/*!@}*/
	//[Http]
	/*! \defgroup SrvHttpSettings Http настройки сервера
		\ingroup SrvSettings
	@{*/
	int maxPendingConnections; //!< Максимум ожидающих соединений. По умолчанию 30
	int maxLineSize; //!< Максимальный размер заголовка. По умолчанию 32кб
	int maxBufferSize; //!< Максимальный размер буфера клиента для чтения\записи контента. По умолчанию 1мб
	qint64 maxContentSize; //!< Максимальный размер контента сообщения для хранения в памяти. По умолчанию 16мб
	qint64 maxFileSize; //!< Максимальный размер контента сообщения для хранения на диске. По умолчанию 512мб
	int maxHeaders; //!< Максимальный количество заголовков. По умолчанию 128
	bool keepAliveOff; //!< Отключение функции keep-alive. По умолчанию false (функция включена)
};
/*!@}*/