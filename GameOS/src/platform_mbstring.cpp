#ifndef PLATFORM_WINDOWS

#include "platform_mbstring.h"
#include "platform_str.h"

unsigned char *_mbslwr(
           unsigned char * str
        )
{
    // only ok for C locale and single byte character set
	return (unsigned char*)S_strlwr((char*)str);
}

// what a stupid signature
unsigned char *_mbsdec(const unsigned char* start, const unsigned char* current)
{
    // only ok for C locale and single byte character set
    if(start >= current)
        return 0;
    return const_cast<unsigned char*>(current - 1);
}

// see this for a lot of useful information http://www.gnu.org/software/libc/manual/html_node/Converting-a-Character.html#Converting-a-Character

int isleadbyte(
        int c 
        )
{
    // only ok for C locale and single byte character set
    return 0; // sebi !NB
}

unsigned char *_mbsinc(
           const unsigned char *current 
        )
{
    // only ok for C locale and single byte character set
    return const_cast<unsigned char*>(current + 1); // sebi !NB
}

#endif // PLATFORM_WINDOWS
