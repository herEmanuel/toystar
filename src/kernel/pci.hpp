#ifndef PCI_H
#define PCI_H

#include <stdint.h>

#define CONFIG_ADDR 0xCF8
#define CONFIG_DATA 0xCFC

/* 
        PciHeader 
    uint16_t vendor_id;
    uint16_t device_id;
    uint16_t command;
    uint16_t status;
    uint8_t revision;
    uint8_t progIf;
    uint8_t device_subclass;
    uint8_t device_class;

*/

namespace PCI {

    class Device {
    public:
        uint8_t bus;
        uint8_t slot;
        uint8_t function;
        uint8_t dev_class;
        uint8_t dev_subclass;
        uint8_t progIf;
        uint16_t deviceId;
        uint16_t vendorId;
        
        //TODO: continue here

    };

    void init();
    void enumerate_devices();
    void set_address(uint8_t bus, uint8_t device, uint8_t function, uint8_t offset);
    uint32_t read_double_word(uint8_t bus, uint8_t device, uint8_t function, uint8_t offset);
    bool exists(uint8_t bus, uint8_t device, uint8_t function);

}

#endif