#ifndef MEMFUNC_H
#define MEMFUNC_H

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

#endif //MEMFUNC_H

