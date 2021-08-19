#ifndef APIC_H
#define APIC_H

#include <acpi/madt.hpp>
#include <stdint.h>
#include <stddef.h>

namespace Apic {

    void init();

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

    };

}

#endif