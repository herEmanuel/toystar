#ifndef DEVFS_H
#define DEVFS_H

#include "vfs.hpp"

namespace Devfs {

    void init();

    class device {
    public:
        virtual int read(size_t offset, size_t size, const char* buffer) { return -1; };
        virtual int write(size_t offset, size_t size, char* buffer) { return -1; };
    };

    struct devfs_node {
        Vfs::fs_node* file;

        device* dev;

        devfs_node* next;
        devfs_node* children;
    };

    class devfs : public Vfs::filesystem {
     public:
        const char* name;

        devfs();

        const char* get_name();

        Vfs::file_description* open(Vfs::fs_node* working_dir, const char* path, uint16_t mode);  
        int read(Vfs::fs_node* path, size_t offset, size_t size, const char* buffer);
        int write(Vfs::fs_node* path, size_t offset, size_t size, char* buffer);
    };

    void add_device(const char* name, device* new_device);
}

#endif