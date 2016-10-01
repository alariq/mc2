#include "stdlib_win.h"

#include <libgen.h>
#include <string.h>
#include <stdlib.h> // free
#include <assert.h> 
#include <stdint.h>

// or 
//#define _GNU_SOURCE
//#include <string.h>

void _splitpath (const char* path, char* drive, char* dir, char* fname, char* ext)
{
    char* copy_base = strdup(path);
    char* copy_dir = strdup(path);
    char* d = dirname(copy_dir);    
    char* b = basename(copy_base);

    if(dir) {
        strcpy(dir, d);
    }
    if(fname) {
        strcpy(fname, b);
        char* dot = strrchr(fname, '.');
        *dot='\0';
    }

    free(copy_dir);
    free(copy_base);

    if(drive) {
        strcpy(drive, "");
    }

    if(ext) {
        char* dot = strrchr(fname, '.');
        if(dot) {
            strcpy(ext, dot+1);
        } else {
            strcpy(ext, "");
        }
    }

}

static void u32toa_naive(uint32_t value, char* buffer) {
    char temp[10];
    char *p = temp;
    do {
        *p++ = char(value % 10) + '0';
        value /= 10;
    } while (value > 0);

    do {
        *buffer++ = *--p;
    } while (p != temp);

    *buffer = '\0';
}

static void i32toa_naive(int32_t value, char* buffer) {
    uint32_t u = static_cast<uint32_t>(value);
    if (value < 0) {
        *buffer++ = '-';
        u = ~u + 1;
    }
    u32toa_naive(u, buffer);
}

void _itoa(int value, char* str, int radix)
{
   i32toa_naive(value, str); 
}
