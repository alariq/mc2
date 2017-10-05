#pragma once

#ifndef PLATFORM_STR_H
#define PLATFORM_STR_H

#include<stdio.h> // va_list


// string functions to use in cross-platform environment
int	S_strcmp(const char* s1, const char* s2);
int S_strncmp(const char* s1, const char* s2, size_t max_count);
int S_stricmp(const char* s1, const char* s2);
int S_strnicmp(const char* s1, const char* s2, size_t max_count);
char* S_strupr(char* s);
char* S_strlwr(char* s);
int S_snprintf(char *str, size_t size, const char *format, ...);
int S_vsnprintf(char *str, size_t size, const char *format, va_list ap);
//int S_sprintf(char *str, const char *format, ...);

#define _strdup strdup


#endif // PLATFORM_STR_H
