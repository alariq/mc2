#ifndef TIMING_H
#define TIMING_H

#include <stdint.h>

namespace timing {

void sleep(unsigned int nanosec);
void init();
uint64_t gettickcount();
uint64_t ticks2ms(uint64_t ticks);

};

#endif // TIMING_H
