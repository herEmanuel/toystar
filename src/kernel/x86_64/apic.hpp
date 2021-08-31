#ifndef APIC_H
#define APIC_H

#include <acpi/madt.hpp>
#include <stdint.h>
#include <stddef.h>

namespace Apic {

    void init();

    enum LapicRegisters {
        EOI = 0xB0,
        ICR = 0x300,
        SIVR = 0xF0,
        TPR = 0x80,
        DCR = 0x3E0,
        LVTTimer = 0x320,
        InitialCountR = 0x380,
        CurrentCountR = 0X390
    };

    class xapic {
        uint64_t base_address;
        uint8_t lapic_id;

    public:

        xapic(uint64_t baseAddress);

        void write(uint16_t reg, uint32_t value);
        uint32_t read(uint16_t reg);

        void calibrate_timer(uint64_t ms);
        void enable();
        void send_ipi(uint8_t apic, uint64_t ipi);
        void eoi();
    };

    class ioapic {
        uint64_t base_address;
        uint8_t ioapic_id;
        uint32_t gsib;

    public:

        ioapic(madt_ioapic* madtIoapic);

        void write(const uint32_t reg, const uint32_t value);
        uint32_t read(const uint32_t reg);

        void setRedirection(uint8_t vector, uint32_t gsi, uint32_t flags);

        uint8_t max_gsi();
        void mask_gsi(uint32_t gsi);
        void unmask_gsi(uint32_t gsi);

        uint32_t base();

    };

    extern xapic* localApic;
}

#endif