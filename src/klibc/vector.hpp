#ifndef VECTOR_H
#define VECTOR_H

#include "memory/heap.hpp"
#include <utils.hpp>

#include <stddef.h>

namespace toys {

    template<typename T>
    class vector {
        T* m_data;
        size_t m_capacity;
        bool m_isEmpty;

    public:
        vector(size_t size) {
            m_data = new T[size];

            m_capacity = size;
        }

        vector() {
            m_isEmpty = true;
            m_capacity = 0;
            m_data = nullptr;
        }

        ~vector() {
            delete[] m_data;
        }

        T& operator[](size_t i) {
            if (i >= m_capacity)  {
                panic("vector out of bounds");
            }

            return m_data[i];
        }

        void resize(size_t elementAmount) {
            if (elementAmount == m_capacity) 
                return;

            if (!elementAmount) {
                delete[] m_data;
                m_data = nullptr;
                m_isEmpty = true;
                m_capacity = 0;
                return;
            } 

            if (m_isEmpty) {
                m_data = new T[elementAmount];
                m_isEmpty = false;
                m_capacity = elementAmount;
                return;
            }

            T* oldData = m_data;
            m_data = new T[elementAmount];

            size_t t = (elementAmount > m_capacity) ? m_capacity : elementAmount;

            for (size_t i = 0; i < t; i++) {
                m_data[i] = oldData[i];
            }

            delete[] oldData;
            
            m_capacity = elementAmount;
        }

        void push_back(const T value) {
            resize(m_capacity+1);

            m_data[m_capacity-1] = value;
        }

        void pop_back() {
            if (!m_capacity) {
                return;
            }

            resize(m_capacity-1);
        }

        bool erase(size_t index) {
            if (index >= m_capacity) {
                return false;
            }

            for (size_t i = index; i < m_capacity; i++) {
                m_data[i] = m_data[i+1];
            }

            resize(m_capacity-1);
        }
        
        T* data() {
            return m_data;
        }

        size_t size() {
            return m_capacity;
        }

        bool empty() {
            return m_isEmpty;
        }
    };

}

#endif