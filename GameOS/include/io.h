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

#define _S_IWRITE S_IWUSR
#define _S_IREAD S_IREAD
#define _O_RDONLY O_RDONLY

long _filelength(int fd);


#endif // IO_H
