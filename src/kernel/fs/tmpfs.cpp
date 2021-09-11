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

    const char* tmpfs::get_name() {
        return name;
    }

    const char* get_name_from_path(const char* path) {
        size_t i = 0;
        size_t lastSlash = 0;

        while (path[i]) {
            if (path[i] == '/') {
                lastSlash = i;
            }

            i++;
        }

        return path+lastSlash+1;
    }

    // /test/test.txt

    // /teste/dev/potato.png

    // ../teste

    //idk bruh
    const char* node_to_path(tmpfs_node* node) {
        
    }

    // ../../../teste

    const char* tmpfs::relative_to_absolute(Vfs::fs_node* node, const char* path) {
        tmpfs_node* tmpNode = reinterpret_cast<tmpfs_node*>(node->device_node);

        while (path[0] == '.') {
            if (path[1] == '.') {
                if (path[2] != '/') { return ""; }

                tmpNode = tmpNode->parent;
                path += 3;
                continue;
            }

            if (path[1] != '/') { return ""; }
            path += 2;
        }

        return node_to_path(tmpNode);
    }

    //TODO: this needs more testing
    //if the node in the path doesn't exist, it will return its parent (if it exists)
    tmpfs_node* path_to_node(const char* path) {
        tmpfs_node* head = root_tmpfs_node->children;
        tmpfs_node* node = root_tmpfs_node;

        if (path == "/") {
            return root_tmpfs_node;
        }

        if (path[0] == '/') {
            path += 1;
        }

        while (head != nullptr && path[0] != '\0') {
            if (strncmp(head->file->name, path, strlen(head->file->name))) {
                if (path[strlen(head->file->name)] != '/' &&
                    path[strlen(head->file->name)] != '\0') {
                    head = head->next;
                    continue;
                }

                if (path[strlen(head->file->name)] == '\0') {
                    return head;
                }
                
                node = head;

                path += strlen(head->file->name)+1;

                head = head->children;
                continue;
            }

            head = head->next;
        }

        //this ensures that the node returned is either the parent or the node itself
        size_t i = 0;
        size_t count = 0;

        while (path[i]) {
            if (path[i] == '/' && path[i+1] != '\0') {
                if (++count > 1) { return nullptr; }
            }

            i++;
        }

        return node;
    }

    Vfs::file_description* tmpfs::open(const char* path, uint16_t mode) {
        tmpfs_node* node = path_to_node(path);

        if (node == nullptr) {
            return nullptr;
        }
        
        if (mode & Vfs::Modes::CREATE) {
            if (strcmp(get_name_from_path(path), node->file->name)
                || node->file->type != Vfs::FileType::Directory) {
                return nullptr; 
            }

            tmpfs_node* new_file = new tmpfs_node;

            new_file->file = new Vfs::fs_node;
            new_file->file->name = get_name_from_path(path); 
            new_file->file->size = 4096;
            new_file->file->permissions = 0;
            new_file->file->type = 0;
            new_file->file->device_node = (void*)new_file;
            new_file->file->fs = tmp_filesystem;

            new_file->data = reinterpret_cast<uint8_t*>(PMM::alloc(1));
            new_file->children = nullptr;

            kprint("file already in memory \n");
            kprint("new file name: %s\n", new_file->file->name);
    
            tmpfs_node* parent_node = node;

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
 
        if (!strcmp(get_name_from_path(path), node->file->name)
            || node->file->type != Vfs::FileType::File) {
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
    int tmpfs::mkdir(const char* path) {
        tmpfs_node* parent = path_to_node(path);

        if (parent == nullptr) {
            return -1;
        }

        if (strcmp(get_name_from_path(path), parent->file->name)) {
            return -1; //TODO: proper error code
        }

        tmpfs_node* dir = new tmpfs_node;

        dir->file = new Vfs::fs_node;
        dir->file->name = get_name_from_path(path);
        dir->file->type = Vfs::FileType::Directory;
        dir->file->device_node = (void*)dir;
        dir->file->fs = tmp_filesystem;

        dir->children = nullptr;

        dir->parent = parent;
        dir->next = parent->children;
        parent->children = dir;

        return 0;
    }

}