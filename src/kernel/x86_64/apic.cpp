#include "apic.hpp"
#include <acpi/madt.hpp>
#include <acpi/acpi.hpp>
#include <memory/vmm.hpp>
#include <vector.hpp>

namespace Apic {

    void init() {
        madt_table* madtTable = reinterpret_cast<madt_table*>(Acpi::findTable("APIC"));
        madt_header* madtHeader = reinterpret_cast<madt_header*>(madtTable->madt_entries);

        size_t entries = madtTable->header.length - sizeof(sdt) - 8;

        toys::vector<madt_lapic*> lapics;
        toys::vector<madt_ioapic*> ioapics;
        toys::vector<madt_iso*> isos;

        for (size_t i = 0; i < entries; i += madtHeader->length) {
            madtHeader = reinterpret_cast<madt_header*>(&madtTable->madt_entries[i]);

            switch (madtHeader->type) {
                case LAPIC: {
                    madt_lapic* lapic = reinterpret_cast<madt_lapic*>(&madtTable->madt_entries[i]);
                    lapics.push_back(lapic);
                    break;
                }

                case IOAPIC: {
                    madt_ioapic* ioapic = reinterpret_cast<madt_ioapic*>(&madtTable->madt_entries[i]);
                    ioapics.push_back(ioapic);
                    break;
                }

                case IOAPIC_ISO: {
                    madt_iso* iso = reinterpret_cast<madt_iso*>(&madtTable->madt_entries[i]);
                    isos.push_back(iso);
                    break;   
                }

                default:
                    break;
            }

        }

        for (size_t i = 0; i < ioapics.size(); i++) {
            ioapic currIoApic(ioapics[i]);

            kprint("I/O Apic max gsi: %d\n", currIoApic.max_gsi());

            for (size_t i = 0; i < currIoApic.max_gsi(); i++) {
                currIoApic.mask_gsi(i);
            }

            //TODO: keep going

        }

    }

    ioapic::ioapic(madt_ioapic* madtIoapic) {
        gsib = madtIoapic->gsib;
        base_address = madtIoapic->ioapic_addr + PHYSICAL_BASE_ADDRESS;
        ioapic_id = madtIoapic->ioapic_id;
    }

    void ioapic::write(const uint32_t reg, const uint32_t value) {
        *reinterpret_cast<uint32_t*>(base_address) = reg;
        *reinterpret_cast<uint32_t*>(base_address + 0x10) = value;
    }

    uint32_t ioapic::read(const uint32_t reg) {
        *reinterpret_cast<uint32_t*>(base_address) = reg;
        return *reinterpret_cast<uint32_t*>(base_address + 0x10);
    }

    void ioapic::setRedirection(uint8_t vector, uint32_t gsi, uint32_t flags) {
        uint32_t reg = ((gsi - gsib) * 2) + 0x10;
        uint64_t redirection = vector;

        //set bit 13 (active low) if the flag specifies that it is active low
        redirection |= (flags & 2) ? (1 << 13) : redirection;

        //set bit 15 (level triggered) if the flag specifies that it is level triggered
        redirection |= (flags & 8) ? (1 << 15) : redirection;

        write(reg, redirection);
        write(reg + 1, (redirection >> 32));
    }

    uint8_t ioapic::max_gsi() {
        uint32_t value = read(1);
        kprint("value: %x\n", value);

        return (value >> 16) & 0xFF;
    }


    void ioapic::mask_gsi(uint32_t gsi) {
        uint32_t reg = ((gsi - gsib) * 2) + 0x10;
        
        uint32_t value = read(reg);
        write(reg, value | (1 << 16));
    }

    void ioapic::unmask_gsi(uint32_t gsi) {
        uint32_t reg = ((gsi - gsib) * 2) + 0x10;
        
        uint32_t value = read(reg);
        write(reg, value & ~(1 << 16));
    }

}