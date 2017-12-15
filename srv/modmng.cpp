#include "modmng.h"
#include "settings.h"
#include <QStringList>
#include <QVector>
#include <QDebug>
#include <QMutex>
#include <QLibrary>

class ServerModule 
{
	QMutex m;
	int reqCount;

	QString name;
	QLibrary dll;

	void *module;
	initFunc init;
	requestFunc request;
	releaseFunc release;
	endFunc	end;

	void close();
public:
	ServerModule();
	~ServerModule();

	bool loadLib(QString _name);
	bool processRequest(messageData *req, messageData *res, void **callback);
	bool processEnd(void *callback);
	QString getName() { return name; }
	int getReqCount();
};

ServerModule::ServerModule()
{
	reqCount = 0;
	init = 0;
	request = 0;
	release = 0;
	end = 0;
	module = 0;
}
ServerModule::~ServerModule()
{
	close();
}
bool ServerModule::loadLib(QString _name)
{
	name = _name;
	dll.setFileName(name);
	try
	{
		if (!dll.load())
			throw 0;
		init = (initFunc)dll.resolve("init");
		request = (requestFunc)dll.resolve("request");
		release = (releaseFunc)dll.resolve("release");
		end = (endFunc)dll.resolve("end");
		if (!init || !request || !release)
			throw 0;
		module = init();
		if (!module)
			throw 1;
	}
	catch(int e)
	{
		
		qDebug() << "Module -" << name;
		if (e == 0)
			qDebug() << dll.errorString();
		close();
		return false;
	}
	qDebug() << "Module +" << name;
	return true;
}
void ServerModule::close()
{
	try
	{
		if (module)
			release(module);
		if (dll.isLoaded())
			dll.unload();
	}
	catch (...) 
	{
	}	
	init = 0;
	request = 0;
	release = 0;
	module = 0;
}
bool ServerModule::processRequest(messageData *req, messageData *res, void **callback)
{
	if (!module)
		return false;
	return request(module, req, res, callback);
}
bool ServerModule::processEnd(void *callback)
{
	if (!module || !end)
		return false;
	return end(module, callback);
}
int ServerModule::getReqCount()
{ 
	int value;
	m.lock();
	value = ++reqCount;
	m.unlock();
	return value;
}


QVector<ServerModule*> ModuleManager::modules;
QReadWriteLock ModuleManager::lock;

bool ModuleManager::load(QStringList modulesList)
{
	lock.lockForWrite();
	for (int i = 0; i < modules.size(); i++)
		delete modules[i];
	modules.clear();
	for (int i = 0; i < modulesList.size(); i++)
	{
		if (modulesList[i].isEmpty())
			continue;
		ServerModule *m = new ServerModule();
		if (m->loadLib(modulesList[i]))
			modules.append(m);
		else
			delete m;
	}
	lock.unlock();
	return true;
}
bool ModuleManager::unload()
{
	if (UNLOADABLE)
		lock.lockForWrite();
	for (int i = 0; i < modules.size(); i++)
		delete modules[i];
	modules.clear();
	if (UNLOADABLE)
		lock.unlock();
	return true;
}
ModuleMessage *ModuleManager::processRequest(SocketMessage &data)
{
	if (modules.size() == 0 || data.headers.size() == 0)
		return 0;

	bool requestProcessed = false;

	messageData req = {0};
	messageData res = {0};
	data.getRaw(req);

	void *callback = 0;
	ModuleMessage *result = 0;

	if (UNLOADABLE)
		lock.lockForRead();
	{
		for (int i = 0; i < modules.size(); i++)
		{
			if (modules[i]->processRequest(&req, &res, &callback))
			{
				result = new ModuleMessage();
				//result->module = modules[i];
				result->messageCount = modules[i]->getReqCount();
				result->moduleName = modules[i]->getName();
				break;
			}
		}
	}
	if (UNLOADABLE)
		lock.unlock();
	
	if (result)
	{
		result->callback = callback;
		result->setRaw(res);
	}
	//cleanup
	if (req.headers)
		delete [] req.headers;

	/*if (res.body)
		delete [] res.body;
	if (res.headers)
	{
		for (int i = 0; i < res.headersSize; i++)
			delete [] res.headers[i];
		delete [] res.headers;
	}*/
	return result;
}
void ModuleManager::processEnd(ModuleMessage &data)
{
	if (UNLOADABLE)
		lock.lockForRead();
	for (int i = 0; i < modules.size(); i++)
	{
		if (modules[i]->getName() == data.moduleName) // TODO: is it safe?
		{
			modules[i]->processEnd(data.callback);
			break;
		}
	}
	if (UNLOADABLE)
		lock.unlock();
}