#ifndef MEMFUNC_H
#define MEMFUNC_H

#ifdef PLATFORM_WINDOWS
#include<memory.h>
#else
#include<string.h>
#endif

#include <cassert>

template <typename T, int N>
void ArrayCopy(T (&dst)[N], const T (&src)[N]) {
    for(int i=0;i<N;++i) {
        dst[i] = src[i];
    }
}

template <typename T, int N>
void ArrayCopy(T (&dst)[N], const T (&src)[N], size_t n) {
    assert(n <= N);
    for(size_t i=0;i<n;++i) {
        dst[i] = src[i];
    }
}

template <typename T, int N>
void ArrayCopy(T* dst, const T (&src)[N], size_t n) {
    assert(n <= N);
    for(size_t i=0;i<n;++i) {
        dst[i] = src[i];
    }
}

template <typename T, int N>
void ArrayCopy(T (&dst)[N], const T* src, size_t n) {
    assert(n <= N);
    for(size_t i=0;i<n;++i) {
        dst[i] = src[i];
    }
}

template <typename T, int N, int M>
void ArrayCopy(T (&dst)[N][M], const T (&src)[N][M]) {
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
void* MemCpy(T (& dst)[N], const T* src, size_t n) {
	assert(sizeof(T)*N >= n);
    return memcpy(dst, src, n);
}

template <typename T, int N>
void* MemCpy(T (& dst)[N], const void* src) {
    return memcpy(dst, src, sizeof(T) * N);
}

template <typename T, int N>
void* MemCpy(void* dst, const T (& src)[N]) {
    return memcpy(dst, src, sizeof(T) * N);
}

template <typename T, int N>
void* MemCpy(T* dst, const T (& src)[N], size_t n) {
	assert(sizeof(T)*N >= n);
    return memcpy(dst, src, n);
}

template <typename T, int N>
void* MemCpy(T (& dst)[N], const T (& src)[N]) {
    return memcpy(dst, src, sizeof(T) * N);
}

template <typename T, int N>
void* MemCpy(T (& dst)[N], const T (& src)[N], size_t n) {
	assert(sizeof(T) * N >= n);
    return memcpy(dst, src, n);
}

template <typename T, int N, int M>
void* MemCpy(T (& dst)[N][M], const T (& src)[N][M]) {
    return memcpy(dst, src, sizeof(T) * N * M);
}

template <typename T, int N, int M>
void* MemCpy(T (& dst)[N][M], const T (& src)[N][M], size_t n) {
	assert(sizeof(T) * N * M >= n);
    return memcpy(dst, src, n);
}


#endif //MEMFUNC_H

