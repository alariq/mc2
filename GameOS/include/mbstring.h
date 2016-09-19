#ifndef MBSTRIG_H
#define MBSTRIG_H

char *_strlwr(
           char * str
        );
wchar_t *_wcslwr(
           wchar_t * str
        );
unsigned char *_mbslwr(
           unsigned char * str
        );

int isleadbyte(
        int c 
        );

const unsigned char *_mbsinc(
           const unsigned char *current 
        );


#endif // MBSTRIG_H
