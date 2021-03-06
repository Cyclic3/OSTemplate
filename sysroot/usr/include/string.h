#ifndef _STRING_H
#define _STRING_H

#include <sys/cdefs.h>

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

int memcmp(const void*, const void*, size_t);
void* memcpy(void* __restrict, const void* __restrict, size_t);
void* memmove(void*, const void*, size_t);
void* memset(void*, int, size_t);
size_t strlen(const char*);
void reverse(char * s);
char* itoa(int num, char* str, int base);
int strcmp(char *lhs, char *rhs);
int strncmp(char *lhs, char *rhs,size_t n);
#ifdef __cplusplus
}
#endif

#endif
