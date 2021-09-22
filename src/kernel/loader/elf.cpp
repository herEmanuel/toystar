#include "elf.hpp"
#include <memory/vmm.hpp>
#include <memory/pmm.hpp>
#include <memory/heap.hpp>
#include <fs/vfs.hpp>
#include <video.hpp>

#include <stdint.h>
#include <stddef.h>
#include <strings.hpp>
#include <math.hpp>

namespace Loader::Elf {

    int64_t load(VMM::vmm* pagemap, Vfs::fs_node* file) {
        elf_header start;

        int res = Vfs::read(file, 0, sizeof(elf_header), (const char*)&start);
        if (res == -1) {
            return -1;
        }

        if (start.ident[0] != 0x7F || strncmp("ELF", (const char*)start.ident[1], 3)) {
            return -1;
        }

        if (start.ident[4] != ELF_64_BITS || start.ident[5] != ELF_LITTLE_INDIAN 
            || start.ident[7] != ELF_SYSV) {
            return -1;
        }
        
        program_header* phdr = new program_header[start.ph_count];

        res = Vfs::read(file, start.phoff, sizeof(program_header) * start.ph_count, 
                        (const char*)phdr);
        if (res == -1) {
            delete[] phdr;
            return -1;
        }
        
        for (size_t i = 0; i < start.ph_count; i++) {
            if (phdr[i].type != PType::P_LOAD) {
                continue;
            }

            size_t alignment = phdr[i].vaddr & 4095;
            size_t page_number = DIV_CEIL(phdr[i].memsz + alignment, PAGE_SIZE);

            void* mem_area = PMM::alloc(page_number);
            if (mem_area == nullptr) {
                delete[] phdr;
                return -1;
            }

            res = Vfs::read(file, phdr[i].offset, phdr[i].filesz, 
                            (const char*) mem_area + alignment + PHYSICAL_BASE_ADDRESS);

            if (res == -1) {
                delete[] phdr;
                PMM::free(mem_area, page_number);
                return -1;
            }

            // execute flag is still being ignored
            size_t prot = (phdr[i].flags & PT_WRITE) ? 0b111 : 0b101;

            pagemap->map_range(phdr[i].vaddr, (uint64_t)mem_area, page_number*PAGE_SIZE, 
                                prot, 0);
        }

        delete[] phdr;
        
        return start.entry;
    }
}