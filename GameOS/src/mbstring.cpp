#include<mbstring.h>

unsigned char *_mbslwr(
           unsigned char * str
        )
{

}

// see this for a lot of useful information http://www.gnu.org/software/libc/manual/html_node/Converting-a-Character.html#Converting-a-Character

int isleadbyte(
        int c 
        )
{
    // only ok for C locale and single byte character set
    return 0; // sebi !NB
}

const unsigned char *_mbsinc(
           const unsigned char *current 
        )
{
    // only ok for C locale and single byte character set
    return current++; // sebi !NB
}

