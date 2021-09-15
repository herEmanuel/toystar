#ifndef INITTMPFS_H
#define INITTMPFS_H

#include <stdint.h>
#include <boot/stivale2.hpp>

struct ustar_header {
    char name[100];
    uint64_t mode;
    uint64_t uid;
    uint64_t gid;
    char size[12]; //octal
    char mtime[12];
    uint64_t checksum;
    uint8_t type;
    char link_name[100];
    char signature[6];
    char version[2];
    char owner[32];
    char group[32];
    char device_maj[8];
    char device_min[8];
    char prefix[155];
} __attribute__((packed));

enum UType {
    File = '0',
    Symlink = '2',
    Directory = '5'
};

namespace Tmpfs {

    void load(stivale2_struct_tag_modules* module_info);

}

#endif