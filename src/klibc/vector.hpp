#ifndef VECTOR_H
#define VECTOR_H

#include "memory/heap.hpp"
#include <video.hpp>
#include <stddef.h>

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
            if (i < 0 || i >= capacity)  {
                kprint("shit, array out of bounds\n");
                // return nullptr;
            }

            return internalData[i];
        }

        void resize(size_t elementAmount) {
            if (elementAmount == capacity) {
                return;
            }

            // internalData =  reinterpret_cast<T*>(krealloc(internalData, sizeof(T)*elementAmount));

            T* oldData = internalData;
            internalData = new T[elementAmount];

            size_t t = (elementAmount > capacity) ? capacity : elementAmount;

            for (size_t i = 0; i < t; i++) {
                internalData[i] = oldData[i];
            }

            delete[] oldData;

            capacity = elementAmount;

            if (!capacity) {
                isEmpty = true;
            }
        }

        void push_back(const T value) {
            if (isEmpty) {
                internalData = new T;

                internalData[capacity++] = value;

                isEmpty = false;
                return;
            }

            // internalData = reinterpret_cast<T*>(krealloc(internalData, sizeof(T)*capacity+1));
        
            T* oldData = internalData;
            internalData = new T[capacity+1];

            for (size_t i = 0; i < capacity; i++) {
                internalData[i] = oldData[i];
            }

            delete[] oldData;

            internalData[capacity++] = value;
        }

        void pop_back() {
            if (!capacity) {
                return;
            }

            // internalData = reinterpret_cast<T*>(krealloc(internalData, sizeof(T)*capacity-1));

            T* oldData = internalData;
            internalData = new T[capacity-1];

            for (size_t i = 0; i < capacity-1; i++) {
                internalData[i] = oldData[i];
            }

            delete[] oldData;

            if (!(--capacity)) {
                isEmpty = true;
            }
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