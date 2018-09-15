#ifndef __FILE_UTILS__
#define __FILE_UTILS__

#include <stdint.h>
#include <string>

namespace filesystem {

extern const char kPathSeparatorAsChar;
extern const char *const kPathSeparator;

uint64_t get_file_mod_time_ms(const char* filename);


std::string get_path(const char* fname);

}

#endif //__FILE_UTILS__

