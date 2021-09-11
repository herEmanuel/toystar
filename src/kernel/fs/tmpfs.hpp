#ifndef TMPFS_H
#define TMPFS_H

#include "vfs.hpp"

#include <stdint.h>
#include <stddef.h>

namespace Tmpfs {

    void init();

    struct tmpfs_node {
        Vfs::fs_node* file;

        uint8_t* data;

        tmpfs_node* next;
        tmpfs_node* parent;
        tmpfs_node* children;
    };

    class tmpfs : public Vfs::filesystem {
    public:
        const char* name;

        tmpfs();

        const char* getName();

        const char* relative_to_absolute(Vfs::fs_node* node, const char* path);

        Vfs::file_description* open(const char* path, uint16_t mode);  
        int read(Vfs::fs_node* path, size_t offset, size_t size, const char* buffer);
        int write(Vfs::fs_node* path, size_t offset, size_t size, const char* buffer);
        int mkdir(const char* path);
    };

}

#endif