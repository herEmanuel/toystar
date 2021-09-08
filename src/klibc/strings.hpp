#ifndef STRINGS_H
#define STRINGS_H

#include <stddef.h>

extern char charBuffer[20];

const char* itoa(size_t number, size_t base);
size_t atoi(const char* str);

bool strcmp(const char* msg1, const char* msg2, size_t len);
size_t strlen(const char* str);
void substr(char* buffer, const char* src, size_t pos, size_t n);

// size_t sprintf(char* buffer, const char* value);
// size_t sprintf(char* buffer, size_t value);
// size_t sprintf(char* buffer, size_t value, size_t base);

// template<typename T, typename... Args>
// size_t sprintf(char* buffer, const char* format, T value, Args... args) {
//     size_t i = 0;

//     while (format[i]) {
    
//         if (format[i] == '%') {
//             size_t written = 0;

//             switch (format[i+1]) {
//                 case 's':
//                     written = sprintf(buffer+i, value);
//                     break;

//                 case 'd':
//                     written = sprintf(buffer+i, value, 10);
//                     break;

//                 case 'x':
//                     written = sprintf(buffer+i, value, 16);
//                     break;

//                 case '%':
//                     buffer[i] = '%';
//                     break;
//             }

//             if (format[i+2] != '\0' && sizeof...(args) != 0) {
//                 return sprintf(buffer+i+written, format+i+2, args...);
//             }

//             i += written;
//             break;
//         }

//         buffer[i] = format[i];
//         i++;
//     }

//     buffer[i] = '\0';

//     return 0;
// }

#endif