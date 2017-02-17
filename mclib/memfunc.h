#ifndef MEMFUNC_H
#define MEMFUNC_H

#ifdef PLATFORM_WINDOWS
#include<memory.h>
#else
#include<string.h>
#endif

#include <cassert>

template <typename T, int N>
void ArrayCopy(T (&dst)[N], T (&src)[N]) {
    for(int i=0;i<N;++i) {
        dst[i] = src[i];
    }
}

template <typename T, int N, int M>
void ArrayCopy(T (&dst)[N][M], T (&src)[N][M]) {
    for(int i=0;i<N;++i) {
        for(int j=0;j<M;++j) {
            dst[i][j] = src[i][j];
        }
    }
}

template <typename T, int N>
void* ArraySet(T (&dst)[N], unsigned char val) {
    for(int i=0;i<N;++i) {
        dst[i] = val;
    }
    return dst;
}

template <typename T, int N>
void* MemSet(T (& dst)[N], unsigned char val) {
    return memset(dst, val, sizeof(T) * N);
}

template <typename T, int N>
void* MemCpy(T (& dst)[N], const void* src, size_t n) {
	assert(sizeof(T)*N >= n);
    return memcpy(dst, src, n);
}

template <typename T, int N>
void* MemCpy(void* dst, const T (& src)[N], size_t n) {
	assert(sizeof(T)*N >= n);
    return memcpy(dst, src, n);
}

#endif //MEMFUNC_H

