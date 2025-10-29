#include "pch.h"

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

#define WIN32_LEAN_AND_MEAN
#include <crtdbg.h>
#include "debug_utils.h"

#include "app.h"


void DebugOut(char *_fmt, ...)
{
    char buf[2048];
    va_list ap;
    va_start (ap, _fmt);
    vsprintf(buf, _fmt, ap);
    OutputDebugString(buf);
}


void DarwiniaReleaseAssert(bool _condition, char const *_fmt, ...)
{
	if (!_condition)
	{
		char buf[512];
		va_list ap;
		va_start (ap, _fmt);
		vsprintf(buf, _fmt, ap);

		DWORD rc = GetLastError();
		if (rc != ERROR_SUCCESS) {
			LPVOID lpMsgBuf;

			FormatMessage(
				FORMAT_MESSAGE_ALLOCATE_BUFFER |
				FORMAT_MESSAGE_FROM_SYSTEM,
				NULL,
				rc,
				MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
				(LPTSTR) &lpMsgBuf,
				0, NULL );

			sprintf( buf + strlen(buf), "\nLast error: %s (%d)", lpMsgBuf, rc );
		}
		ShowCursor(true);
		MessageBox(NULL, buf, "Fatal Error", MB_OK);
        GenerateBlackBox( buf );
#ifndef _DEBUG
		exit(-1);
#else
		_ASSERT(_condition);
#endif
	}
}


int g_newReportingThreshold = -1; // All memory allocations larger than this will be reported


void SetNewReportingThreshold(int _size)
{
	g_newReportingThreshold = _size;
}


#ifdef OVERLOADED_NEW
#undef new
void *operator new (unsigned _size, char const *_filename, int _line)
{
	void *p = malloc(_size);

	if ((signed)_size > g_newReportingThreshold)
	{
		DebugOut("%s line %d: %d bytes at 0x%0x\n", _filename, _line, _size, p);
	}
	DarwiniaDebugAssert(p);

	return p;
}


void *operator new[] (unsigned _size, char const *_filename, int _line)
{
	void *p = malloc(_size);

	if ((signed)_size > g_newReportingThreshold)
	{
		DebugOut("%s line %d: %d bytes at 0x%0x\n", _filename, _line, _size, p);
	}
	DarwiniaDebugAssert(p);

	return p;
}


void operator delete (void *p, char const *_filename, int _line)
{
	free(p);
}


void operator delete[] (void *p, char const *_filename, int _line)
{
	free(p);
}


#endif // #ifdef OVERLOADED_NEW

