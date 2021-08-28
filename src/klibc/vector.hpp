#ifndef VECTOR_H
#define VECTOR_H

#include "memory/heap.hpp"
#include <video.hpp>
#include <stddef.h>
#include <utils.hpp>

namespace toys {

    template<typename T>
    class vector {
        T* internalData;
        size_t capacity;
        bool isEmpty;

    public:
        vector(size_t size) {
            internalData = new T[size];

            capacity = size;
        }

        vector() {
            isEmpty = true;
            capacity = 0;
            internalData = nullptr;
        }

        ~vector() {
            delete[] internalData;
        }

        T& operator[](size_t i) {
            if (i >= capacity)  {
                // Toystar::utils::panic("vector out of bounds");
                kprint("noooooo\n");
            }

            return internalData[i];
        }

        void resize(size_t elementAmount) {
            if (elementAmount == capacity) 
                return;

            if (!elementAmount) {
                delete[] internalData;
                internalData = nullptr;
                isEmpty = true;
                capacity = 0;
                return;
            } 

            if (isEmpty) {
                internalData = new T[elementAmount];
                isEmpty = false;
                capacity = elementAmount;
                return;
            }

            T* oldData = internalData;
            internalData = new T[elementAmount];

            size_t t = (elementAmount > capacity) ? capacity : elementAmount;

            for (size_t i = 0; i < t; i++) {
                internalData[i] = oldData[i];
            }

            delete[] oldData;

            capacity = elementAmount;
        }

        void push_back(const T value) {
            resize(capacity+1);

            internalData[capacity-1] = value;
        }

        void pop_back() {
            if (!capacity) {
                return;
            }

            resize(capacity-1);
        }

        T* data() {
            return internalData;
        }

        size_t size() {
            return capacity;
        }

        bool empty() {
            return isEmpty;
        }
    };

}

#endif