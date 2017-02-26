#ifndef IO_H
#define IO_H

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h> // fseek ftell etc...

#define _stat ::stat
#define _fstat ::fstat
#define _creat ::creat
#define _open ::open
#define _close ::close
#define _read ::read
#define _write ::write
#define _lseek ::lseek 
#define _chmod ::chmod 

// on Windows write means read also, there are no write-only wiles
#define _S_IWRITE (S_IWUSR|S_IRUSR|S_IRGRP|S_IROTH)
#define _S_IREAD S_IREAD
#define _O_RDONLY O_RDONLY
#define _O_CREAT O_CREAT
#define _O_TRUNC O_TRUNC
#define _O_BINARY  0 // all files are binary on linux
#define _O_RDWR O_RDWR

long _filelength(int fd);


#endif // IO_H
