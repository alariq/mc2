
#ifndef PLATFORM_IO_H
#define PLATFORM_IO_H

#include <sys/types.h>
#include <sys/stat.h>

#ifdef PLATFORM_WINDOWS
    #include <io.h> 
#else
    #include <unistd.h>
#endif

#include <stdio.h> // fseek ftell etc...

#ifndef PLATFORM_WINDOWS
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

#else

#include <direct.h>

#define srandom srand
#define random rand

/* Values for the second argument to access.
These may be OR'd together.  */
#define R_OK    4       /* Test for read permission.  */
#define W_OK    2       /* Test for write permission.  */
//#define   X_OK    1       /* execute permission - unsupported in windows*/
#define F_OK    0       /* Test for existence.  */

#define access _access
#define dup2 _dup2
#define execve _execve
#define ftruncate _chsize
#define unlink _unlink
#define fileno _fileno
#define getcwd _getcwd
#define chdir _chdir
#define isatty _isatty
#define lseek _lseek
//read, write, and close are NOT being #defined here, because while there are file handle specific versions for Windows, they probably don't work for sockets. You need to look at your app and consider whether to call e.g. closesocket().

#ifdef _WIN64
#define ssize_t __int64
#else
#define ssize_t long
#endif

#define STDIN_FILENO 0
#define STDOUT_FILENO 1
#define STDERR_FILENO 2

#endif // PLATFORM_WINDOWS

#endif // PLATFORM_IO_H

