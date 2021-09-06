#ifndef VFS_H
#define VFS_H

#include <stdint.h>
#include <stddef.h>

namespace Vfs {

    class filesystem {
    public:
        const char* name;

        virtual int open(const char* path, uint16_t mode) { return -1; };  
        virtual int read(const char* path, size_t cnt, const char* buffer) { return -1; };
        virtual int write(const char* path, size_t cnt, const char* buffer) { return -1; };
        virtual int mkdir(const char* path) { return -1; };
    };

    struct node {
        const char* name;
        filesystem* fs;

        node* parent;
        node* children;
        node* next;
    };

    void add_fs(filesystem* fs);

    bool mount(const char* source, const char* target);
    bool unmount(node* vfs_node);

    const char* get_fs_path(const char* path);

    int open(const char* path, uint16_t mode);  
    int read(const char* path, size_t cnt, const char* buffer);
    int write(const char* path, size_t cnt, const char* buffer);
    int mkdir(const char* path);
}


#endif