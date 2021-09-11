#include "pci.hpp"
#include "io.hpp"
#include "video.hpp"

#include <stdint.h>
#include <stddef.h>

//TODO: finish pci support when i actually need it 

namespace PCI {

    void set_address(uint8_t bus, uint8_t device, uint8_t function, uint8_t offset) {
        uint32_t addr = 0;

        addr |= offset & 0xFC; // offsets are always aligned to 32 bits
        addr |= function << 8;
        addr |= device << 11;
        addr |= bus << 16;
        addr |= 1 << 31; //Enable bit

        outl(CONFIG_ADDR, addr);
    }

    uint32_t read_double_word(uint8_t bus, uint8_t device, uint8_t function, uint8_t offset) {
        uint32_t value;

        set_address(bus, device, function, offset);

        value = inl(CONFIG_DATA);
        return value;
    }

    bool exists(uint8_t bus, uint8_t device, uint8_t function) {
        if (read_double_word(bus, device, function, 0) == 0xFFFFFFFF) {
            return false;
        }

        return true;
    }

    void enumerate_devices() {   
        for (size_t bus = 0; bus < 256; bus++) {
            for (size_t slot = 0; slot < 32; slot++) {
                for (size_t function = 0; function < 8; function++) {

                    if (!exists(bus, slot, function)) 
                        continue;

                    // kprint("Detected a device on bus %d, slot %d and function %d\n", bus, slot, function);

                }
            }
        }
        
    }
}