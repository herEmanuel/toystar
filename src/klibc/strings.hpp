#ifndef STRINGS_H
#define STRINGS_H

#include <stddef.h>

extern char charBuffer[20];

const char* itoa(size_t number, size_t base);
size_t atoi(const char* str);

size_t octal_to_int(const char* str);

bool strncmp(const char* msg1, const char* msg2, size_t len);
bool strcmp(const char* msg1, const char* msg2);
size_t strlen(const char* str);
void substr(char* buffer, const char* src, size_t pos, size_t n);

#endif