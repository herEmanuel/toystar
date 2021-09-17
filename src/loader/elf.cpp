#include "elf.hpp"
#include <memory/vmm.hpp>
#include <memory/heap.hpp>
#include <fs/vfs.hpp>

#include <stdint.h>
#include <stddef.h>
#include <strings.hpp>

namespace Loader::Elf {

    bool load(VMM::vmm* pagemap, Vfs::fs_node* file) {
        elf_header start;

        int res = Vfs::read(file, 0, sizeof(elf_header), (const char*)&start);
        if (res == -1) {
            return false;
        }

        if (start.ident[0] != 0x7F || strncmp("ELF", (const char*)start.ident[1], 3)) {
            return false;
        }

        /*
            return if the executable is not 64 bits or not little endian or 
            if it doesn't use the SysV abi
        */
        if (start.ident[4] != 0x2 || start.ident[5] != 0x1 
            || start.ident[7] != 0x0) {
            return false;
        }
        
        program_header* phdr = new program_header[start.ph_count];

        res = Vfs::read(file, start.phoff, sizeof(program_header) * start.ph_count, 
                        (const char*)phdr);
        if (res == -1) {
            delete[] phdr;
            return false;
        }

        for (size_t i = 0; i < start.ph_count; i++) {
            if (phdr[i].type == PType::P_LOAD) {
                return false;
            }
        }


        delete[] phdr;

    }

}