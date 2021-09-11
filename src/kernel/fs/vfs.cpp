#include "vfs.hpp"
#include <memory/heap.hpp>
#include <vector.hpp>
#include <strings.hpp>
#include <video.hpp>
#include <x86_64/cpu.hpp>

#include <stddef.h>
#include <stdint.h>

Vfs::node* vfs_root_node = nullptr;
toys::vector<Vfs::filesystem*> fses;

//TODO: relative paths and make it safe for multiple cores

namespace Vfs {

    void add_fs(filesystem* fs) {
        fses.push_back(fs);
    }

    bool mount(const char* source, const char* target) {
        filesystem* fs = nullptr;

        for (size_t i = 0; i < fses.size(); i++) {
            if (strncmp(fses[i]->get_name(), source, strlen(fses[i]->get_name()))) {
                fs = fses[i];
                break;
            }
        }

        if (fs == nullptr) {
            return false;
        }

        if (vfs_root_node == nullptr) {
            node* mount_node = new node;
            mount_node->name = target;
            mount_node->fs = fs;
            mount_node->parent = nullptr;
            mount_node->children = nullptr;
            mount_node->next = nullptr;
            vfs_root_node = mount_node;

            return true;
        } 

        node* head = vfs_root_node->children;
        node* parent = vfs_root_node;
        const char* target_str = target;

        while (head != nullptr) {
            if (strncmp(head->name, target_str, strlen(head->name))) {
                if (target_str[strlen(head->name)] != '/') {
                    head = head->next;
                    continue;
                }

                parent = head;

                target_str += strlen(head->name);

                head = head->children;
                continue;
            }       

            head = head->next;
        }

        node* mount_node = new node;
        mount_node->name = target_str;
        mount_node->fs = fs;
        mount_node->parent = parent;
        mount_node->children = nullptr;
        mount_node->next = parent->children;
        parent->children = mount_node;
        
        return true;
    }

    void print_nodes(node* root) {
        if (root == nullptr) {
            return;
        }

        node* head = root->children;

        while (head != nullptr) {
            const char* parent_name = (head->parent != nullptr) ? head->parent->name : "null";
            kprint("name: %s | parent: %s\n", head->name, parent_name);
            print_nodes(head);
            head = head->next;
        }

    }

    path* get_absolute_path(const char* vfs_path) {
        node* head = vfs_root_node->children;

        const char* fs_path = vfs_path;
        node* parent = vfs_root_node;

        while (head != nullptr) {
            if (strncmp(head->name, fs_path, strlen(head->name))) {
                if (fs_path[strlen(head->name)] != '/') {
                    head = head->next;
                    continue;
                }

                parent = head;

                fs_path += strlen(head->name);

                head = head->children;
                continue;
            } 

            head = head->next;
        }

        if (parent == nullptr) {
            return nullptr;
        }

        path* _path = new path;
        _path->fs_path = fs_path;
        _path->fs = parent->fs;

        return _path;
    }

    file_description* open(const char* path, uint16_t mode) {
        if (path[0] != '/') {
            fs_node* working_dir =  Cpu::local_core()->working_dir;

            path = working_dir->fs->relative_to_absolute(working_dir, path);
            return working_dir->fs->open(path, mode);
        }

        Vfs::path* vfs_path = get_absolute_path(path);
    
        return vfs_path->fs->open(vfs_path->fs_path, mode);
    }

    int read(fs_node* path, size_t offset, size_t size, const char* buffer) {
        return path->fs->read(path, offset, size, buffer);
    }

    int write(fs_node* path, size_t offset, size_t size, const char* buffer) {
        return path->fs->write(path, offset, size, buffer);
    }

    int mkdir(const char* path) {
        Vfs::path* vfs_path = get_absolute_path(path);

        return vfs_path->fs->mkdir(vfs_path->fs_path);
    }

}