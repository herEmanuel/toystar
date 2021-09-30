#ifndef UTILS_H
#define UTILS_H

#include <drivers/serial.hpp>

#include <stddef.h>

void panic(const char* msg);

void log(const char* msg);
void log(size_t num);
void log(size_t num, size_t base);

template<typename T, typename... Args>
void log(const char* msg, T value, Args... args) {
    size_t i = 0;
    
    while (msg[i]) {
        if (msg[i] == '%') {
            switch (msg[++i]) {
                case 'd':
                case 'i':
                    log(value, 10);
                    break;

                case 's':
                    log(value);
                    break;

                case 'x':
                    log(value, 16);
                    break;

                case '%':
                    Serial::send_char('%');
                    break;

                default:
                    break;
            }
            i++;
            
            log(msg + i, args...);
            return;
        }

        Serial::send_char(msg[i]);
        i++;
    }
}

#endif