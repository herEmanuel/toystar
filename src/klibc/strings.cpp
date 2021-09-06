#include "strings.hpp"
#include <stddef.h>
#include <video.hpp>

//TODO: bruh will other cores mess this up?

char charBuffer[20];

const char* itoa(size_t number, size_t base) {
    size_t i = 0;
    size_t c = 0;

    if (number == 0 || (base != 16 && base != 10)) {
        return "0\0";
    }

    if (!number) {
        charBuffer[0] = '-';
        i++;
        c++;
    } else if (base == 16) {
        charBuffer[0] = '0';
        charBuffer[1] = 'x';
        i += 2;
        c += 2;
    }
    
    while (number) {
        size_t digit = number % base;
        charBuffer[i] = (digit <= 9) ? digit + '0' : digit - 10 + 'A';
        number /= base;
        i++;
    }

    char last;
    size_t end = i - 1;

    while (c < end) {
        last = charBuffer[c];

        charBuffer[c] = charBuffer[end];
        charBuffer[end] = last;
        c++;
        end--;
    }

    charBuffer[i] = '\0';

    return charBuffer;
}

size_t atoi(const char* str) {
    size_t num = 0;
    size_t i = 0;
    bool negative = false;

    if (str[0] == '-') {
        negative = true;
        i++;
    }

    while (str[i]) {
        num += str[i] - '0';
        num *= 10;
        i++;
    }

    if (negative) {
        num *= -1;
    }

    return num;
}

bool strcmp(const char* msg1, const char* msg2, size_t len) {
    for (size_t i = 0; i < len; i++) {
        if (msg1[i] != msg2[i]) {
            return false;
        }
    }

    return true;
}

size_t strlen(const char* str) {
    size_t i = 0;

    while (str[i++]);

    return i - 1;
}

const char* substr(const char* src, size_t pos, size_t n) {
    char buffer[n];

    for (size_t i = 0; i < n; i++) {
        buffer[i] = src[pos+i];
    } 

    return buffer;
}

// size_t sprintf(char* buffer, const char* value) {
//     size_t len = strlen(value);
//     kprint("sprintf str: %s\n", value);
//     size_t written = 0;

//     for (size_t i = 0; i < len; i++) {
//         if (value[i] == '\0') 
//             continue;
//         buffer[i] = value[i];
//         written++;
//     } 
//     kprint("written: %d\n", written);
//     return written;
// }

// size_t sprintf(char* buffer, size_t value, size_t base) {
//     const char* num_str = itoa(value, base);
//     kprint("sprintf num: %s\n", num_str);
//     size_t len = strlen(num_str);
//     size_t written = 0;

//     for (size_t i = 0; i < len; i++) {
//         if (num_str[i] == '\0')
//             continue;
//         buffer[i] = num_str[i];
//         written++;
//     }

//     kprint("written: %d\n", written);
//     return written;
// }

// size_t sprintf(char* buffer, size_t value) {
//     return sprintf(buffer, value, 10);   
// }
