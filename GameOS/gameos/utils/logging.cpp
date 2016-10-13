#include "utils/logging.h"

#ifdef PLATFORM_WINDOWS
#include <windows.h>
#else
#include <cstdarg>
#endif

#include <cstdio>
#include <cassert>
#include <cstring>

namespace logging {

static const int MAX_LOG_LINE = 1024;

void logmsg(eLogCategory lc, const char* file, int line, const char* fmt, ...)
{
	assert(lc < NUM_LC_LOGCATEGORY && lc>=0 && fmt);

	static const char* 	msgs[NUM_LC_LOGCATEGORY] = { "DEBUG", "INFO", "WARNING", "ERROR" };
	char				text[MAX_LOG_LINE] = {0};
	va_list				ap;				

	if (!strlen(fmt)) return;

// to support jump to error
#ifdef PLATFORM_WINDOWS
	sprintf(text, "%s(%d): %s: ", file, line, msgs[lc]);
#else
	sprintf(text, "%s:%d: %s: ", file, line, msgs[lc]);
#endif
	size_t hdr_len = strlen(text);

	va_start(ap, fmt);	
	vsnprintf(text+strlen(text), MAX_LOG_LINE - hdr_len-1, fmt, ap); // 10 for "decorations"
	va_end(ap);

	size_t len = strlen(text);
	text[len] = '\0';


#ifdef PLATFORM_WINDOWS
	OutputDebugString(text);
#else
	puts(text);
#endif
}


}

