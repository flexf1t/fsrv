#pragma once
#include <QString>
#include <QReadWriteLock>
#include "clntsock.h"
#include "messages.h"
#include <stdlib.h>
/*! \defgroup ModulesInterface Взаимодействие с модулями сервера
@{*/
/// <summary>
/// Указатель на функцию инициализации модуля
/// </summary>
/// <returns>Указатель на хендлер</returns>
typedef void *(*initFunc)();
/// <summary>
/// Указатель на фукцию обработки запроса
/// </summary>
/// <param name="handler">Хендлер модуля</param>
/// <param name="req">Указатель на структуру с данными запроса</param>
/// <param name="res">Указатель на структуру с данными ответа</param>
/// <param name="callback">Указатель на данные необходимые модулю для функции end</param>
/// <returns>True при успешной обработке запроса</returns>
/// <seealso cref="messageData, endFunc()"/>
typedef bool (*requestFunc)(void *handler, messageData *req, messageData *res, void **callback);
/// <summary>
/// Указатель на фукцию завершения отправки ответа
/// </summary>
/// <param name="handler">Хендлер модуля</param>
/// <param name="callback">Указатель на данные необходимые модулю</param>
/// <returns></returns>
typedef bool (*endFunc)(void *handler, void *callback);
/// <summary>
/// Указатель на функцию освобождения модуля
/// </summary>
/// <param name="handler">Хендлер модуля</param>
/// <returns>True при успешном освобождении</returns>
typedef bool (*releaseFunc)(void *handler);
/*!@}*/
class Settings;
class ServerModule;
class ModuleManager
{
	static QReadWriteLock lock;
	static QVector<ServerModule*> modules;
	static const bool UNLOADABLE = false;
	static bool unload();
public:
	static bool load(QStringList modulesList);
	static ModuleMessage *processRequest(SocketMessage &data);
	static void processEnd(ModuleMessage &data);
};