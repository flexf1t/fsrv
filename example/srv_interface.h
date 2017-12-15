#pragma once

#if defined(_MSC_VER) // Microsoft Visual Studio
#define _FSRV_WIN32
//#define _FSRV_PDA
#elif defined(__BORLANDC__) // Borland C++ Builder
#define _FSRV_WIN32
#elif defined(__MINGW32__)
#define _FSRV_WIN32
#else
#define _FSRV_MCBC
#endif

#ifdef _FSRV_WIN32
#define _FSRV_EXPORT __declspec(dllexport)
#define _FSRV_IMPORT __declspec(dllimport)
#endif //_FSRV_WIN32
#ifdef _FSRV_MCBC
#define _FSRV_DLL_NAME_PREFIX "lib"
#define _FSRV_DLL_NAME_SUFFIX ".so"
#define _FSRV_EXPORT
#define _FSRV_IMPORT
#endif //_FSRV_MCBC

struct messageData
{
	char **headers;
	int headersSize;
	char *body; 
	int size; 
	bool path;
};

extern "C"
{
	_FSRV_EXPORT void *init();

	bool _FSRV_EXPORT request(void *handler, messageData *req, messageData *res, void **callback);
	bool _FSRV_EXPORT end(void *handler, void *callback);
	bool _FSRV_EXPORT release(void *handler);
}