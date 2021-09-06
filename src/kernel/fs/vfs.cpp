#include "vfs.hpp"
#include <memory/heap.hpp>
#include <vector.hpp>
#include <strings.hpp>

#include <stddef.h>
#include <stdint.h>

Vfs::node* vfs_root_node = nullptr;
toys::vector<Vfs::filesystem*> fses;

namespace Vfs {

    void add_fs(filesystem* fs) {
        fses.push_back(fs);
    }

    bool mount(const char* source, const char* target) {
        int fsIndex = -1;

        for (size_t i = 0; i < fses.size(); i++) {
            if (strcmp(fses[i]->name, source, strlen(fses[i]->name))) {
                fsIndex = i;
                break;
            }
        }

        if (fsIndex == -1) {
            return false;
        }

        if (vfs_root_node == nullptr) {
            node* mount_node = new node;
            mount_node->name = target;
            mount_node->fs = fses[fsIndex];
            mount_node->parent = nullptr;
            mount_node->children = nullptr;
            mount_node->next = nullptr;
            vfs_root_node = mount_node;

            return true;
        } 

        // / /dev /dev/sd0 /dev/bruh/sd1
        // /dev /test /bruh

        node* head = vfs_root_node->children;
        while (head != nullptr) {
            if (strcmp(head->name, target, strlen(head->name))) {
                //bruh lets just keep going
                // substr(target)
                return false;
            }       

            head = head->next;
        }

        node* mount_node = new node;
        mount_node->name = target;
        mount_node->fs = fses[fsIndex];
        mount_node->parent = vfs_root_node;
        mount_node->children = nullptr;
        mount_node->next = vfs_root_node->children;
        vfs_root_node->children = mount_node;
        
        return true;
    }

    const char* get_fs_path(const char* path) {
        node* parent_node = nullptr;

        // while (vfs_root_node)
    }

}