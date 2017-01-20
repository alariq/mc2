#ifndef PLATFORM_MBSTRIG_H
#define PLATFORM_MBSTRIG_H

#ifndef PLATFORM_WINDOWS

wchar_t *_wcslwr(
           wchar_t * str
        );
unsigned char *_mbslwr(
           unsigned char * str
        );

int isleadbyte(
        int c 
        );

unsigned char *_mbsinc(
           const unsigned char *current 
        );

unsigned char *_mbsdec(const unsigned char* start, const unsigned char* current);

#else
#include <mbstring.h>
#endif

#endif // PLATFORM_MBSTRIG_H
