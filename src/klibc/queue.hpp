#ifndef QUEUE_H
#define QUEUE_H

#include "vector.hpp"
#include "optional.hpp"
#include <memory/heap.hpp>

#include <stdint.h>
#include <stddef.h>

//TODO: test this

namespace toys {

    template<typename T>
    class queue {
        vector<T> m_data;
        size_t m_size;

    public:
        queue() {
            m_size = 0;
        }

        ~queue() {
        }

        optional<T> front() {
            if (!m_size) {
                return nullopt;
            }

            return m_data[0];
        }

        optional<T> back() {
            if (!m_size) {
                return nullopt;
            }

            return m_data[m_size-1];
        }

        void push(const T value) {
            m_data.push_back(value);
            m_size++;
        }

        optional<T> pop() {
            if (!m_size) {
                return nullopt;
            }

            T value = m_data[0];

            vector<T> new_data(m_size-1);

            for (size_t i = 0; i < m_size-1; i++) {
                new_data[i] = m_data[i+1];
            }

            m_data = new_data;
            m_size--;

            return value;
        }

        size_t size() {
            return m_size;
        }
    };

}

#endif