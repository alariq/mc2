#include <cstdio>
#include <cstdarg>
#include <string.h>
#include <cassert>
#include <ctype.h>

#include "platform_str.h"

// string functions to use in cross-platform environment

int S_strcmp(const char* s1, const char* s2)
{
	assert(s1 && s2);

	int c1, c2, d;
	do {
		c1 = *s1++;
		c2 = *s2++;
		d = c1 - c2;
		if(d) {
			return ((d&0x70000000)>>30) - 1;
		}
	} while(c1);

	return 0;
}

int S_strncmp(const char* s1, const char* s2, size_t max_count)
{
	assert(s1 && s2);

	int c1, c2, d;
	do {

		if(!max_count--)
			return 0;

		c1 = *s1++;
		c2 = *s2++;
		d = c1 - c2;
		if(d) {
			return ((d&0x70000000)>>30) - 1;
		}
	} while(c1);

	return 0;

}

int S_stricmp(const char* s1, const char* s2)
{
	// TODO: implement
#ifdef PLATFORM_WINDOWS
	return _stricmp(s1, s2);
#else
    return strcasecmp(s1, s2);
#endif
}

int S_strnicmp(const char* s1, const char* s2, size_t max_count)
{
	// TODO: implement
#ifdef PLATFORM_WINDOWS
	return _strnicmp(s1, s2, max_count);
#else
    return strncasecmp(s1, s2, max_count);
#endif
}

char* S_strupr(char* s)
{
#ifdef PLATFORM_WINDOWS
	return _strupr(s);
#else
	char* p = s;
    while ((*p = toupper( *p ))) p++;
    return s;
#endif
}

char* S_strlwr(char* s)
{
#ifdef PLATFORM_WINDOWS
	return _strlwr(s);
#else
	char* p = s;
    while ((*p = tolower( *p ))) p++;
    return s;
#endif
}

int S_vsnprintf(char *str, size_t size, const char *format, va_list ap)
{
	assert(str && size>0 && format);

	int count = -1;

#ifdef PLATFORM_WINDOWS
	if(size != 0)
		count = _vsnprintf_s(str, size, _TRUNCATE, format, ap);

	// if formated tring turned out to take more than buf_size
	// return real length of formatted string
	if(count == -1)
		count = _vscprintf(format, ap);
#else
	count = vsnprintf(str, size, format, ap);
#endif
	return count;
}

int S_snprintf(char *str, size_t size, const char *format, ...)
{
	assert(str && size>0 && format);

	va_list ap;

	va_start(ap, format);	
	int count = S_vsnprintf(str, size, format, ap);
	va_end(ap);

	return count;
}

#if 0
int S_sprintf(char *str, const char *format, ...)
{
	assert(str && format);

	va_list ap;
	// this will not work correctly!
	// can use approach from idStr, but for now just disable this function at all
	size_t l = strlen(str);

	va_start(ap, format);	
	int count = S_vsnprintf(str, l - 1, format, ap);
	va_end(ap);
	str[l-1]='\0';

	// warn if string is truncated
	//if(count > l-1) warn

	return count;
}
#endif