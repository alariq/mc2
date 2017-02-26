#ifndef MBSTRIG_H
#define MBSTRIG_H

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

#endif // MBSTRIG_H
