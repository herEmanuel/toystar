#ifndef ASSERT_H
#define ASSERT_H

#include <stddef.h>

#ifdef NDEBUG
#define assert(EXPR)
#else
#define assert(EXPR) EXPR || (__assert(#EXPR, __FILE__, __LINE__), false)
#endif

void __assert(const char* msg, const char* file, size_t line);

#endif