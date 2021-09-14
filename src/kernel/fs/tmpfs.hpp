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
        bool resize(tmpfs_node* node, size_t new_size, bool grow);

    public:
        const char* name;

        tmpfs();

        const char* get_name();

        Vfs::file_description* open(Vfs::fs_node* working_dir, const char* path, uint16_t mode);  
        int read(Vfs::fs_node* path, size_t offset, size_t size, const char* buffer);
        int write(Vfs::fs_node* path, size_t offset, size_t size, char* buffer);
        int mkdir(Vfs::fs_node* working_dir, const char* path);
    };

}

#endif