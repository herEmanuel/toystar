#ifndef ELF_H
#define ELF_H

#include <stdint.h>
#include <memory/vmm.hpp>
#include <fs/vfs.hpp>

//TODO: auxv?

namespace Loader::Elf {

    struct elf_header {
        uint8_t ident[16];
        uint16_t type;
        uint16_t machine;
        uint32_t version;
        uint64_t entry;
        uint64_t phoff;
        uint64_t shoff;
        uint32_t flags;
        uint16_t elf_hdr_size;
        uint16_t ph_entry_size;
        uint16_t ph_count;
        uint16_t shdr_size;
        uint16_t sh_num;
        uint16_t shstrndx;
    };

    struct program_header {
        uint32_t type;
        uint64_t offset;
        uint64_t vaddr;
        uint64_t paddr;
        uint32_t filesz;
        uint32_t memsz;
        uint32_t flags;
        uint32_t align;
    };

    enum PType {
        P_NULL = 0x0,
        P_LOAD = 0x1,
        P_PHDR = 0x6
    };

    bool load(VMM::vmm* pagemap, Vfs::fs_node* file);
}

#endif