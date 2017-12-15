#include "srv_interface.h"
#include "srv_example.h"

void *init()
{
	return ExampleModule::init();
}

bool request(void *handler, messageData *req, messageData *res, void **callback)
{
	if (handler != 0)
		return ((ExampleModule *)handler)->handleRequest(*req, *res, callback);
	return false;
}

bool end(void *handler, void *callback)
{
	if (handler != 0)
		return ((ExampleModule *)handler)->handleCallback(callback);
	return false;
}

bool release(void *handler)
{
	if (handler != 0)
		delete ((ExampleModule *)handler);
	return true;
}