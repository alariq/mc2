#ifndef __FILE_UTILS__
#define __FILE_UTILS__

#include <stdint.h>

namespace filesystem {

uint64_t get_file_mod_time_ms(const char* filename);

}

#endif //__FILE_UTILS__

