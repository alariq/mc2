#ifndef LIBMAIN_H
#define LIBMAIN_H

#ifdef __cplusplus
extern "C" {
#endif

extern const char* getStringById(unsigned int id);
extern void initStringResources();
extern void freeStringResources();

#ifdef __cplusplus
}
#endif

const char* getStringById__(unsigned int id);
void initStringResources__();
void freeStringResources__();

#endif // LIBMAIN_H
