#ifndef STRINGS_H
#define STRINGS_H

#include <stddef.h>

extern char charBuffer[20];

const char* itoa(size_t number, size_t base);
size_t atoi(const char* str);

bool strcmp(const char* msg1, const char* msg2, size_t len);
size_t strlen(const char* str);

#endif