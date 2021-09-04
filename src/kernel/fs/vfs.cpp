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

        node* mount_node = new node;
        mount_node->name = target;
        mount_node->fs = fses[fsIndex];
        mount_node->next = vfs_root_node;
        vfs_root_node = mount_node;
   }

}