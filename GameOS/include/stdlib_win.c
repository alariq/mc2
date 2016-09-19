#include "stdlib_win.h"

#include <libgen.h>
#include <string.h>
#include <stdlib.h> // free

// or 
//#define _GNU_SOURCE
//#include <string.h>

void _splitpath (const char* path, char* drive, char* dir, char* fname, char* ext)
{
    char* copy_base = strdup(path);
    char* copy_dir = strdup(path);
    char* d = dirname(copy_dir);    
    char* b = basename(copy_base);

    strcpy(dir, d);    
    strcpy(fname, b);

    free(copy_dir);
    free(copy_base);

    strcpy(drive, "");
    char* dot = strrchr(fname, '.');
    if(dot) {
        strcpy(ext, dot+1);
    } else {
        strcpy(ext, "");
    }    
}

