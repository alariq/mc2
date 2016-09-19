#include "io.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

long _filelength(int fd) {
    struct stat buf;
    fstat(fd, &buf);
    return buf.st_size;
}

