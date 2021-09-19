#include "tmpfs.hpp"
#include "vfs.hpp"
#include <memory/heap.hpp>
#include <memory/pmm.hpp>
#include <memory/vmm.hpp>
#include <video.hpp>

#include <stdint.h>
#include <stddef.h>
#include <strings.hpp>
#include <memory.hpp>
#include <math.hpp>

Tmpfs::tmpfs* tmp_filesystem = nullptr;

namespace Tmpfs {

    tmpfs_node* root_tmpfs_node = nullptr;

    void init() {
        tmp_filesystem = new tmpfs();
        Vfs::add_fs(tmp_filesystem);
    }

    tmpfs::tmpfs() {
        name = "tmpfs";
        root_tmpfs_node = new tmpfs_node;

        root_tmpfs_node->file = Vfs::new_fs_node("/", 0, 0, Vfs::FileType::Directory, 
                                                (void*)root_tmpfs_node, tmp_filesystem);
        
        root_tmpfs_node->children = nullptr;
        root_tmpfs_node->next = nullptr;
        root_tmpfs_node->parent = nullptr;
    }   

    const char* tmpfs::get_name() {
        return name;
    }

    //TODO: PLEASE NO / AT THE END
    const char* get_name_from_path(const char* path) {
        size_t i = 0;
        int lastSlash = -1;

        while (path[i]) {
            if (path[i] == '/' && path[i+1] != '\0') {
                lastSlash = i;
            }

            i++;
        }

        return path+lastSlash+1;
    }

    //TODO: this needs more testing
    //if the node in the path doesn't exist, it will return its parent (if it exists)
    tmpfs_node* path_to_node(const char* path, tmpfs_node* curr_dir) {
        tmpfs_node* head = root_tmpfs_node->children;
        tmpfs_node* node = (path[0] != '/') ? curr_dir : root_tmpfs_node;

        if (path == "/") {
            return root_tmpfs_node;
        }

        //parse relative paths
        if (path[0] != '/') {
            if (curr_dir == nullptr) {
                return nullptr;
            }
            
            while (path[0] == '.' && node != nullptr) {
                if (path[1] == '.') {
                    if (path[2] != '/') {
                        return nullptr;
                    }

                    node = node->parent;
                    path += 3;
                    continue;
                }

                if (path[1] != '/') {
                    return nullptr;
                }
                
                path += 2;
            }

            if (node == nullptr) {
                return nullptr;
            }

            head = node->children;
        } else {
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

    Vfs::file_description* tmpfs::open(Vfs::fs_node* working_dir, const char* path, uint16_t mode) {        
        tmpfs_node* wdir = (working_dir != nullptr) ? (tmpfs_node*) working_dir->device_node : nullptr;
        tmpfs_node* node = path_to_node(path, wdir);
        
        if (node == nullptr) {
            return nullptr;
        }
  
        if (mode & Vfs::Modes::CREATE) {
            if (strcmp(get_name_from_path(path), node->file->name)
                || node->file->type != Vfs::FileType::Directory) {
                return nullptr; 
            }

            tmpfs_node* new_file = new tmpfs_node;
            
            new_file->file = Vfs::new_fs_node(get_name_from_path(path), 4096, 0, Vfs::FileType::File, 
                                                (void*)new_file, tmp_filesystem);

            new_file->data = reinterpret_cast<uint8_t*>(PMM::alloc(1));
            new_file->children = nullptr;

            tmpfs_node* parent_node = node;

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

    bool tmpfs::resize(tmpfs_node* node, size_t new_size, bool grow) {
        size_t pages = DIV_CEIL(new_size, PAGE_SIZE);

        //TODO: use non-sequential pages
        void* new_data = PMM::alloc(pages);

        if (new_data == nullptr) {
            return false;
        }

        size_t size = (grow) ? node->file->size : pages * PAGE_SIZE;

        memcpy(new_data, (void*)node->data, size);

        PMM::free((void*)node->data, DIV_CEIL(node->file->size, PAGE_SIZE));

        node->data = (uint8_t*) new_data;

        return true;
    }

    int tmpfs::read(Vfs::fs_node* path, size_t offset, size_t size, const char* buffer) {
        tmpfs_node* node = reinterpret_cast<tmpfs_node*>(path->device_node);

        if (offset + size > node->file->size) {
            return -1;
        }

        memcpy((void*)buffer, (void*)(node->data+offset), size);

        return 0;
    }

    //TODO: deleting contents of the file?
    int tmpfs::write(Vfs::fs_node* path, size_t offset, size_t size, char* buffer) {
        tmpfs_node* node = reinterpret_cast<tmpfs_node*>(path->device_node);

        if (offset + size > node->file->size) {
            if (!resize(node, offset + size, true)) {
                return -1;
            }
        }

        memcpy((void*)(node->data+offset), (void*)buffer, size);
        
        return 0;
    }

    int tmpfs::mkdir(Vfs::fs_node* working_dir, const char* path) {
        tmpfs_node* wdir = (working_dir != nullptr) ? (tmpfs_node*) working_dir->device_node : nullptr;
        tmpfs_node* parent = path_to_node(path, wdir);

        if (parent == nullptr) {
            return -1;
        }

        if (strcmp(get_name_from_path(path), parent->file->name)) {
            return -1; //TODO: proper error code
        }
        
        tmpfs_node* dir = new tmpfs_node;

        dir->file = Vfs::new_fs_node(get_name_from_path(path), 0, 0, 
                                        Vfs::FileType::Directory, (void*)dir, tmp_filesystem);

        dir->children = nullptr;
        dir->parent = parent;
        dir->next = parent->children;
        parent->children = dir;
        
        return 0;
    }
}