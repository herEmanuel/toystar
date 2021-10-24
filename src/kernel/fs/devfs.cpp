#include "devfs.hpp"
#include "vfs.hpp"

#include <memory/heap.hpp>
#include <strings.hpp>

namespace Devfs {
    devfs* devfs_filesystem = nullptr;
    devfs_node* devfs_root_node = nullptr;

    void init() {
        devfs_filesystem = new devfs();
        Vfs::add_fs(devfs_filesystem);
    }

    void add_device(const char* name, device* new_device) {
        devfs_node* device_node = new devfs_node;
        device_node->dev = new_device;
        device_node->file = Vfs::new_fs_node(name, 0, 0, Vfs::FileType::Device, device_node, 
                                                nullptr, devfs_filesystem);

        device_node->children = nullptr;
        device_node->next = devfs_root_node->children;
        devfs_root_node->children = device_node;
    }

    devfs::devfs() {
        name = "devfs";
        devfs_root_node = new devfs_node;

        devfs_root_node->dev = nullptr;
        devfs_root_node->next = nullptr;
        devfs_root_node->children = nullptr;
    }

    const char* devfs::get_name() {
        return name;
    }

    Vfs::file_description* devfs::open(Vfs::fs_node* working_dir, const char* path, uint16_t mode) {
        devfs_node* head = devfs_root_node->children;

        path++;
        while (head != nullptr) {
            if (strcmp(path, head->file->name)) {
                Vfs::file_description* new_fd = new Vfs::file_description;
                new_fd->file = head->file;
                new_fd->mode = mode;
                new_fd->offset = 0;

                return new_fd;
            }

            head = head->next;
        }

        return nullptr;
    }

    int devfs::read(Vfs::fs_node* path, size_t offset, size_t size, const char* buffer) {
        devfs_node* device = reinterpret_cast<devfs_node*>(path->device_node);

        device->dev->read(offset, size, buffer);
    }

    int devfs::write(Vfs::fs_node* path, size_t offset, size_t size, char* buffer) {
        devfs_node* device = reinterpret_cast<devfs_node*>(path->device_node);

        return device->dev->write(offset, size, buffer);
    }
}