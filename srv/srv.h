#include <QTcpServer>
#include <QVector>

class Worker;
class Settings;

/*! \defgroup Srv Сервер 
*@{*/
//! Основной класс сервера. \sa SrvSettings
class ModuleFServer: public QTcpServer
{
	Q_OBJECT
	static QByteArray logBA;
	static char *log;
	
	void incomingConnection(int socketID);
	QVector<Worker*> workers;
public:
	/// <summary>
	/// Конструктор объекта сервера
	/// </summary>
	/// <param name="ini">Путь к ini-файлу</param>
	ModuleFServer(QString ini);
	~ModuleFServer();
	/// <summary>
	/// Старт сервера
	/// </summary>
	/// <returns>True при успешном запуске</returns>
	bool start();

	static const QString name; //!< Имя сервера
	static const QString version; //!< Версия сервера
	static const QString server; //!< Заголовок сервера

	/// <summary>
	/// Функция записи сообщения в журнал (в лог или в консоль)
	/// </summary>
	/// <param name="type">Тип сообщения</param>
	/// <param name="msg">Указатель на строку сообщения</param>
	static void output(QtMsgType type, const char *msg);
};
/*!@}*/