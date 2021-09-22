#ifndef ELF_H
#define ELF_H

#include <stdint.h>
#include <memory/vmm.hpp>
#include <fs/vfs.hpp>

//TODO: auxv?

namespace Loader::Elf {

    constexpr size_t PT_WRITE = 0x2;

    constexpr size_t ELF_64_BITS = 0x2;
    constexpr size_t ELF_LITTLE_INDIAN = 0x1;
    constexpr size_t ELF_SYSV = 0x0;

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
        uint32_t flags;
        uint64_t offset;
        uint64_t vaddr;
        uint64_t paddr;
        uint64_t filesz;
        uint64_t memsz;
        uint64_t align;
    };

    enum PType {
        P_NULL = 0x0,
        P_LOAD = 0x1,
        P_PHDR = 0x6
    };

    int64_t load(VMM::vmm* pagemap, Vfs::fs_node* file);
}

#endif