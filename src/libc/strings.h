#ifndef STRINGS_H
#define STRINGS_H

#include <stddef.h>

extern char charBuffer[20];

const char* itoa(size_t number);
size_t atoi(const char* str);

#endif