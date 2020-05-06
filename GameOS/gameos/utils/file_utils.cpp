#include "file_utils.h"

#include <stdint.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>

namespace filesystem {

#ifdef LINUX_BUILD
const char kPathSeparatorAsChar = '/';
const char *const kPathSeparator = "/";
#else
const char kPathSeparatorAsChar = '\\';
const char *const kPathSeparator = "\\";
#endif

uint64_t get_file_mod_time_ms(const char* fname)
{
#if PLATFORM_WINDOWS
	struct _stat fi = { 0 };
    _stat(fname, &fi);
	return fi.st_mtime * 1000;
#else
    struct stat fi = {0};
    stat(fname, &fi);
	return fi.st_mtim.tv_sec * 1e+3 + fi.st_mtim.tv_nsec / 1e+6;
#endif

}

std::string get_path(const char* fname)
{
    const char* pathend = strrchr(fname, filesystem::kPathSeparatorAsChar);
    if(!pathend)
        return std::string();
    else
        return std::string(fname, pathend - fname);
}


}


