#pragma once
#include <QString>
#include <QMap>
#include <QSharedPointer>
#include "clntsock.h"
//! \ingroup ModulesInterface
//! Структура данных запроса\ответа.
/*! Данные запроса являются указателем на данные из SocketMessage.
Данные ответа переносятся в ModuleMessage, который используется как интерфейс к этим данным.
Удаление происходит посредством SocketMessage (кроме удаления headers) и ModuleMessage.*/
struct messageData
{
	/// <summary>Указатель на массив строк заголовков/ответов. Нечетные строки - ключ.
	/// Четные строки - значение. Если строк нечетное число, то 1 строка - стартовая строка запроса.
	/// Строки ответа должны содержать все необходимые символы для формирования заголовка, так как происходит их простое соединение
	/// </summary>
	char **headers; //!< 
	int headersSize; //!< Размер массива строк. Если строк нечетное число, то 1 строка - стартовая строка запроса
	char *body; //!< Указатель на содержимое запроса/ответа. Если path == true, то указатель на путь к файлу\каталогу
	int size; //!< Размер содержимого запроса/ответа
	bool path; //!< Флаг того, что body - путь. Если path == true, то указатель на путь к файлу\каталогу
};
//! \ingroup ModulesInterface
//! Базовый класс данных запроса\ответа.
class IOMessage 
{
	static const int RESERVED_HEADERS_VECTOR_SIZE = 16; //!< Зарезервированный размер вектора заголовков
	QSharedPointer<ClientSocket> sharedptr;
public:
	IOMessage();
	virtual ~IOMessage() = 0;
	/// <summary>
	/// Привязка сообщения к конкретному сокету, чтобы сокет не удалился пока есть сообщение
	/// </summary>
	/// <param name="p">Указатель на умный указатель на сокет</param>
	void copyPointer(QSharedPointer<ClientSocket> *p);
	/// <summary>
	/// Привязка сообщения к конкретному сокету, чтобы сокет не удалился пока есть сообщение
	/// </summary>
	/// <param name="m">Указатель на другое сообщение, откуда возьмется сокет</param>
	void copyPointer(IOMessage *m);

	QVector<QByteArray> headers; //!< Заголовки
	QByteArray body; //!< Содержимое или путь к файлу\каталогу
	bool path; //!< Флаг того, что body - путь. Если path == true, то указатель на путь к файлу\каталогу
};
//! \ingroup ModulesInterface
//! Класс данных запроса. \sa ModuleMessage
class SocketMessage : public IOMessage
{
public:
	SocketMessage();
	~SocketMessage();
	bool firstLine; //!< Флаг наличия стартовой строки http запроса

	/// <summary>
	/// Получить в формате сишной структуры для передачи в модуль
	/// </summary>
	/// <param name="raw">Ссылка на переменную структуры для записи</param>
	void getRaw(messageData &raw);
	/// <summary>
	/// Генерирование временной папки для записи данных запроса. 
	/// </summary>
	/// <param name="tempRoot">Путь к временной папке сервера</param>
	void generateUniqueTempDir(QString tempRoot);
};
//! \ingroup ModulesInterface
//! Класс данных ответа. \sa SocketMessage
class ModuleMessage : public IOMessage
{
	char **headersPtr; 
	int headersSize;
	char *bodyPtr; 
public:
	ModuleMessage();
	/// <summary>
	/// Деструктор. Удаляет все данные, по указателям из messageDate.
	/// </summary>
	/// <returns></returns>
	~ModuleMessage();
	int messageCount;
	QString moduleName; //!< Имя ответившего модуля

	void *callback; //!< Указатель на данные для колбека \sa endFunc()

	/// <summary>
	/// Заполнить данными из messageDate
	/// </summary>
	/// <param name="raw">Ссылка на данные ответа</param>
	void setRaw(messageData &raw);
};