#include "gameos.hpp"

#ifdef LINUX_BUILD
#include <cstdarg>
#else
#include <windows.h>
#endif

#include <cstdio>
#include <cassert>
#include <cstring>
#include "utils/string_utils.h"

#ifdef _DEBUG
static int enter_debugger = 1;
#else
static int enter_debugger = 0;
#endif

static const int MAX_LOG_LINE = 1024;

static void output_debug_text(const char* text)
{
#ifdef LINUX_BUILD
    fprintf(stderr, "%s", text);
#else
	OutputDebugString(text);
#endif

}

static void logmsg(const char* header, const char* fmt, va_list ap) 
{
	assert(fmt);
	if (!strlen(fmt)) return;

	char				text[MAX_LOG_LINE] = {0};

    size_t header_len = strlen(header);

    sprintf(text, "%s: ", header);
	vsnprintf(text + header_len, MAX_LOG_LINE - header_len - 1, fmt, ap);

	size_t len = strlen(text);
	text[len] = '\0';
    output_debug_text(text);
}

// those are game specific callbacks (should move them outside to game)
int __cdecl InternalFunctionStop( const char* fmt, ... )
{
	assert(fmt);
	va_list	ap;

    va_start(ap, fmt);
    logmsg("STOP", fmt, ap);
    va_end(ap);

    //exit(1);
    return enter_debugger;
}

int __cdecl InternalFunctionStop( const char* Message, const char* value)
{
    fprintf(stderr, Message, value);
    return enter_debugger;
}

int __cdecl InternalFunctionPause( const char* fmt, ... )
{
	assert(fmt);
	va_list	ap;

    va_start(ap, fmt);
    logmsg("PAUSE", fmt, ap);
    va_end(ap);

    return enter_debugger;

}
void __cdecl InternalFunctionSpew( const char* Group, const char* fmt, ... )
{
	assert(fmt);
	va_list	ap;

    char buf[256];
    StringFormat(buf, sizeof(buf)-1, "SPEW: <%s>", Group ? Group : "");

    va_start(ap, fmt);
    logmsg(buf, fmt, ap);
    va_end(ap);
}

int __stdcall ErrorHandler( int Flags, const char* Text )
{
    // TODO: additionally check flags to decide if enter debugger or not
    fputs(Text, stderr);
    return enter_debugger;
}
