#include "inittmpfs.hpp"
#include "tmpfs.hpp"
#include "vfs.hpp"

#include <boot/stivale2.hpp>
#include <stdint.h>
#include <stddef.h>
#include <video.hpp>
#include <utils.hpp>
#include <strings.hpp>
#include <memory.hpp>
#include <math.hpp>

namespace Tmpfs {

    void load(stivale2_struct_tag_modules* module_info) {
        if (module_info->module_count == 0) {
            Toystar::utils::panic("no modules were loaded");
        }

        size_t module_size = module_info->modules[0].end - module_info->modules[0].begin;
        ustar_header* file;

        //initial value is 512 so that we skip the first entry (useless directory)
        for (size_t i = 512; i < module_size; i += 512) {
            file = reinterpret_cast<ustar_header*>(module_info->modules[0].begin + i);

            char* name = file->name + 10;

            //end of the tar file
            if (file->type == 0) {
                break;
            }

            switch (file->type) {

                case UType::File: {
                    auto fd = tmp_filesystem->open(nullptr, name, Vfs::Modes::CREATE);

                    if (fd == nullptr) {
                        kprint("Could not create file %s\n", name);
                        continue;
                    }

                    size_t file_size = octal_to_int(file->size);
                
                    tmp_filesystem->write(fd->file, 0, file_size, ((char*)file)+512);

                    i += DIV_CEIL(file_size, 512) * 512;
                    break;
                }

                case UType::Symlink: {
                    //TODO
                    break;
                }

                case UType::Directory: {
                    //remove the / at the end 
                    size_t i = 0;
                    while (name[i]) {i++;}
                    name[i-1] = '\0';

                    int res = tmp_filesystem->mkdir(nullptr, name);

                    if (res == -1) {
                        kprint("Could not create directory %s\n", name);
                    }

                    break;
                }

                default:
                    Toystar::utils::panic("unsupported file type in init module");
                    break;
            }
        }
    }
}