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

            internalData =  reinterpret_cast<T*>(krealloc(internalData, sizeof(T)*elementAmount));

            capacity = elementAmount;

            if (!capacity) {
                isEmpty = true;
            }
        }

        void push_back(const T value) {
            internalData = reinterpret_cast<T*>(krealloc(internalData, sizeof(T)*capacity+1));
        
            internalData[capacity++] = value;

            if (isEmpty) {
                isEmpty = false;
            }
        }

        void pop_back() {
            if (!capacity) {
                return;
            }

            internalData = reinterpret_cast<T*>(krealloc(internalData, sizeof(T)*capacity-1));

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