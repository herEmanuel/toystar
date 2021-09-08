#include "tmpfs.hpp"
#include "vfs.hpp"
#include <memory/heap.hpp>
#include <memory/pmm.hpp>
#include <video.hpp>

#include <stdint.h>
#include <stddef.h>
#include <strings.hpp>
#include <memory.hpp>

Tmpfs::tmpfs* tmp_filesystem = nullptr;

namespace Tmpfs {

    tmpfs_node* root_tmpfs_node = nullptr;

    void init() {
        tmp_filesystem = new tmpfs();
        tmp_filesystem->name = "tmpfs";
        Vfs::add_fs(tmp_filesystem);
    }

    tmpfs::tmpfs() {
        root_tmpfs_node = new tmpfs_node;

        root_tmpfs_node->file = new Vfs::fs_node; 
        root_tmpfs_node->file->name = "/";
        root_tmpfs_node->file->size = 0;
        root_tmpfs_node->file->permissions = 0;
        root_tmpfs_node->file->type = Vfs::FileType::Directory;
        root_tmpfs_node->file->device_node = (void*)root_tmpfs_node;
        root_tmpfs_node->file->fs = tmp_filesystem;
        
        root_tmpfs_node->children = nullptr;
        root_tmpfs_node->next = nullptr;
        root_tmpfs_node->parent = nullptr;
    }   

    const char* tmpfs::getName() {
        return name;
    }

    //TODO: this needs more testing
    tmpfs_node* tmpfs::path_to_node(const char* path) {
        tmpfs_node* head = root_tmpfs_node->children;
        tmpfs_node* parent = nullptr;

        if (path == "/") {
            return root_tmpfs_node;
        }

        while (head != nullptr) {
            kprint("loop\n");
            if (strcmp(head->file->name, path, strlen(head->file->name))) {
                if (path[strlen(head->file->name)] != '/' 
                    || head->file->type != Vfs::FileType::Directory) {
                    head = head->next;
                    continue;
                }

                parent = head;

                path += strlen(head->file->name);

                head = head->children;
                continue;
            }

            head = head->next;
        }

        kprint("loop ended\n");

        size_t i = 1;

        while (path[i]) {
            if (path[i] == '/') {
                return nullptr;
            }

            i++;
        }

        return parent;
    }

    Vfs::file_description* tmpfs::open(const char* path, uint16_t mode) {
        tmpfs_node* node = path_to_node(path);

        if (mode & Vfs::Modes::CREATE) {
            if (node != nullptr) {
                return nullptr;
            }

            size_t lastSlash = 0;
            size_t i = 0;

            while (path[i]) {
                if (path[i] == '/') {
                    lastSlash = i;
                }

                i++;
            }

            tmpfs_node* new_file = new tmpfs_node;

            new_file->file = new Vfs::fs_node;
            new_file->file->name = path+lastSlash+1;
            new_file->file->size = 4096;
            new_file->file->permissions = 0;
            new_file->file->type = 0;
            new_file->file->device_node = (void*)new_file;
            new_file->file->fs = tmp_filesystem;

            new_file->data = reinterpret_cast<uint8_t*>(PMM::alloc(1));
            new_file->children = nullptr;

            kprint("file already in memory \n");
            kprint("new file name: %s\n", new_file->file->name);
            kprint("lastslash: %d\n", lastSlash);
    
            const char* parent_path;

            if (!lastSlash) {
                parent_path = "/";
            } else {
                char buffer[lastSlash];
                substr(buffer, path, 0, lastSlash-1);
                buffer[lastSlash] = '\0';
                parent_path = buffer;
            }

            kprint("parent path: %s\n", parent_path);

            tmpfs_node* parent_node = path_to_node(parent_path);

            kprint("New file parent: %s\n", parent_node->file->name);

            new_file->parent = parent_node;
            new_file->next = parent_node->children;
            parent_node->children = new_file;

            Vfs::file_description* fd = new Vfs::file_description;
            fd->file = new_file->file;
            fd->mode = mode;
            fd->offset = 0;

            return fd;
        }
 
        if (node == nullptr) {
            return nullptr;
        }

        Vfs::file_description* fd = new Vfs::file_description;
        fd->file = node->file;
        fd->mode = mode;
        fd->offset = 0;

        return fd;
    } 

    int tmpfs::read(Vfs::fs_node* path, size_t offset, size_t size, const char* buffer) {
        tmpfs_node* node = reinterpret_cast<tmpfs_node*>(path->device_node);

        if (offset + size > node->file->size) {
            return -1;
        }

        memcpy((void*)buffer, (void*)(node->data+offset), size);

        return 0;
    }

    int tmpfs::write(Vfs::fs_node* path, size_t offset, size_t size, const char* buffer) {
        tmpfs_node* node = reinterpret_cast<tmpfs_node*>(path->device_node);

        if (offset + size > node->file->size) {
            return -1;
        }

        memcpy((void*)(node->data+offset), (void*)buffer, size);
        
        return 0;
    }

    //TODO: implement
    int tmpfs::mkdir(Vfs::fs_node* parent) {

    }

}