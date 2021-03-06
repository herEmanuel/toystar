#ifndef OPTIONAL_H
#define OPTIONAL_H

#include <memory/heap.hpp>

#include <stdint.h>
#include <stddef.h>

namespace toys {

    struct nullopt_t {
        uint8_t n;
    };

    extern nullopt_t nullopt;

    template<typename T>
    class optional {
        bool m_empty;
        T* m_value;

    public:
        optional(T value) {
            m_empty = false;
            m_value = new T(value);
        }

        optional(nullopt_t n) {
            m_value = nullptr;
            m_empty = true;
        }

        ~optional() {
            if (!m_empty) {
                delete m_value;
            }
        }

        T& value_or(T option) {
            if (m_empty) {
                return option;
            }

            return *m_value;
        }

        T* value() {
            return m_value;
        }

    };

}

#endif