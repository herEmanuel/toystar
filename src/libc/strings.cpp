#include "strings.h"
#include <stddef.h>

char charBuffer[20];

const char* itoa(size_t number) {
    size_t i = 0;
    size_t c = 0;

    if (number == 0) {
        return "0\0";
    }

    if (!number) {
        charBuffer[0] = '-';
        i++;
        c++;
    }

    while (number) {
        charBuffer[i] = number % 10 + '0';
        number /= 10;
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