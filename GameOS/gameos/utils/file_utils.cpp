#include "file_utils.h"

#include <stdint.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

namespace filesystem {

uint64_t get_file_mod_time_ms(const char* fname)
{
    struct stat fi = {0};
    stat(fname, &fi);
	return fi.st_mtim.tv_sec * 1e+3 + fi.st_mtim.tv_nsec / 1e+6;
}

}


