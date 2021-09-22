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

    public:
        queue() {
        }

        ~queue() {
        }

        optional<T> front() {
            if (!m_data.size()) {
                return nullopt;
            }

            return m_data[0];
        }

        optional<T> back() {
            if (!m_data.size()) {
                return nullopt;
            }

            return m_data[m_data.size()-1];
        }

        T& operator[](size_t index) {
            return m_data[index];
        }

        void push(const T value) {
            m_data.push_back(value);
        }

        optional<T> pop() {
            if (!m_data.size()) {
                return nullopt;
            }

            T value = m_data[0];

            m_data.erase(0);

            return value;
        }

        bool erase(size_t index) {
            return m_data.erase(index);
        }

        size_t size() {
            return m_data.size();
        }
    };

}

#endif