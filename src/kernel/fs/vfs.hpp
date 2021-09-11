#ifndef VFS_H
#define VFS_H

#include <stdint.h>
#include <stddef.h>

namespace Vfs {

    enum FileType {
        File,
        Directory,
        Symlink
    };

    enum Modes {
        CREATE = 0x1,
        WRITE = 0x2,
        READ = 0x4
    };

    struct file_description;
    struct fs_node;

    class filesystem {
    public:

        virtual const char* get_name() { return " "; };

        virtual const char* relative_to_absolute(fs_node* node, const char* path) { return " "; };
 
        virtual file_description* open(const char* path, uint16_t mode) { return nullptr; };  
        virtual int read(fs_node* path, size_t offset, size_t size, const char* buffer) { return -1; };
        virtual int write(fs_node* path, size_t offset, size_t size, const char* buffer) { return -1; };
        virtual int mkdir(const char* path) { return -1; };
    };

    struct fs_node {
        const char* name; //file name

        size_t size; //file size (in bytes)

        uint8_t permissions; //permissions (read only, etc)

        uint8_t type; //directory, symlink, etc

        void* device_node; //pointer to a wrapper for this node
        filesystem* fs;
    };

    struct file_description {
        fs_node* file;
        uint16_t mode;
        size_t offset;
    };

    struct node {
        const char* name;
        filesystem* fs;

        node* parent;
        node* children;
        node* next;
    };

    struct path {
        const char* fs_path;
        filesystem* fs;
    };

    void add_fs(filesystem* fs);

    bool mount(const char* source, const char* target);
    void print_nodes(node* root);

    path* get_absolute_path(const char* path);

    file_description* open(const char* path, uint16_t mode);  
    int read(fs_node* path, size_t offset, size_t size, const char* buffer);
    int write(fs_node* path, size_t offset, size_t size, const char* buffer);
    int mkdir(const char* path);
}

extern Vfs::node* vfs_root_node;

#endif