#ifndef MAP_H
#define MAP_H

#include <memory/heap.hpp>
#include <utils.hpp>
#include <memory.hpp>
#include "optional.hpp"

#include <stdint.h>
#include <stddef.h>

namespace toys {

    template<typename Key, typename T>
    struct map_entry {
        Key key;
        T value;

        map_entry* next;
    };

    template<typename Key, typename T>
    class map {
    public:
        size_t m_capacity;
        size_t m_size;
        map_entry<Key, T>** m_data;

        //djb2 algorithm
        size_t hash(size_t key) {
            uint64_t hash = 5381;

            for (size_t i = 0; i < sizeof(key); i++) {
                hash = ((hash << 5) + hash) + ((key >> (i*8)) & 0xFF);
            }

            return hash & m_capacity;
        }

        size_t hash(const char* key) {
            uint64_t hash = 5381;
            size_t i = 0;

            while(key[i]) {
                hash = ((hash << 5) + hash) + key[i];
                i++;
            }

            return hash % m_capacity;
        }

    public:
        map() {
            m_size = 0;
            m_capacity = 10;
            m_data = new map_entry<Key, T>*[m_capacity];

            for (size_t i = 0; i < m_capacity; i++) {
                m_data[i] = nullptr;
            }
        }

        ~map() {
            for (size_t i = 0; i < m_capacity; i++) {
                if (m_data[i] == nullptr) 
                    continue;

                map_entry<Key, T>* head = m_data[i];
                map_entry<Key, T>* to_free = nullptr;

                while (head != nullptr) {
                    to_free = head;
                    head = head->next;
                    delete to_free;
                }
            }

            delete[] m_data;
        }

        void insert(Key key, T value) {
            size_t index = hash(key);

            map_entry<Key, T>* newEntry = new map_entry<Key, T>;
            newEntry->key = key;
            newEntry->value = value;
            newEntry->next = nullptr;

            if (m_data[index] != nullptr) {
                newEntry->next = m_data[index];
            }

            m_data[index] = newEntry;
            m_size++;

            if (m_size * (m_size/2) > m_capacity) {
                m_capacity += m_capacity / 2;
                map_entry<Key, T>** oldData = m_data;

                m_data = new map_entry<Key, T>*[m_capacity];
                memcpy(m_data, oldData, sizeof(uintptr_t)*m_capacity);
                delete[] oldData;
            }
        }

        bool erase(Key key) {
            size_t index = hash(key);

            if (m_data[index] == nullptr) {
                return false;
            }

            map_entry<Key, T>** head = &m_data[index];
            while (*head != nullptr) {
                if ((*head)->key == key) {
                    
                    map_entry<Key, T>* oldData = *head;
                    *head = (*head)->next;
                    delete oldData;

                    m_size--;
                    return true;
                }

                head = &(*head)->next;
            }

            return false;
        }

        optional<T> find(Key key) {
            size_t index = hash(key);
            
            if (m_data[index] == nullptr) {
                return nullopt;
            }
            
            map_entry<Key, T>* head = m_data[index];
            while (head != nullptr) {
                if (head->key == key) {
                    return head->value;
                }
                head = head->next;
            }
          
            return nullopt;
        }

        optional<T> operator[](Key key) {
            return find(key);
        }

        size_t size() {
            return m_size;
        }

    };

}

#endif