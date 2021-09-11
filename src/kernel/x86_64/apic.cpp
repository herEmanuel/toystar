#include "apic.hpp"
#include "pic.hpp"
#include <acpi/madt.hpp>
#include <acpi/acpi.hpp>
#include <memory/vmm.hpp>
#include <memory/heap.hpp>
#include <drivers/hpet.hpp>
#include <vector.hpp>

namespace Apic {

    xapic* localApic = nullptr;

    void init() {
        madt_table* madtTable = reinterpret_cast<madt_table*>(Acpi::find_table("APIC"));
        madt_header* madtHeader = reinterpret_cast<madt_header*>(madtTable->madt_entries);

        size_t entries = madtTable->header.length - sizeof(sdt) - 8;

        //TODO: some stuff here doesn't seem to be necessary...
        toys::vector<madt_lapic*> lapics;
        toys::vector<madt_ioapic*> ioapics;
        toys::vector<madt_iso*> isos;
        toys::vector<madt_addrOverride*> addressOverride;

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

                case LAPIC_AO: {
                    madt_addrOverride* ao = reinterpret_cast<madt_addrOverride*>(&madtTable->madt_entries[i]);
                    addressOverride.push_back(ao);
                    break;
                }

                default:
                    break;
            }
        }

        for (size_t i = 0; i < ioapics.size(); i++) {
            ioapic ioApic(ioapics[i]);
            
            size_t max = ioApic.max_gsi();
            size_t baseGsi = ioApic.base();

            if (!baseGsi) {
                for (size_t i = 0; i < 16; i++) {
                    ioApic.set_redirection(i + 0x20, i, 0);
                }
            }

            for (size_t i = 0; i < isos.size(); i++) {

                if (isos[i]->gsi < baseGsi || isos[i]->gsi >= baseGsi + max) 
                    continue;
                
                ioApic.set_redirection(isos[i]->irq + 0x20, isos[i]->gsi, isos[i]->flags);
            }

            for (size_t i = 0; i < max; i++) {
                ioApic.mask_gsi(i);
            }

            if (!baseGsi) {
                ioApic.unmask_gsi(1);
            }
        }

        remap_pic(0x20, 0x28);

        if (addressOverride.size() != 0) {
            localApic = new xapic(addressOverride[0]->addr);
        } else {
            localApic = new xapic(madtTable->lapic_addr);
        }

        asm("sti");
    }

    xapic::xapic(uint64_t baseAddress) {
        base_address = baseAddress + PHYSICAL_BASE_ADDRESS;
        enable();
    }

    uint32_t xapic::read(uint16_t reg) {
        return *reinterpret_cast<uint32_t*>(base_address + reg);
    }

    void xapic::write(uint16_t reg, uint32_t value) {
        *reinterpret_cast<uint32_t*>(base_address + reg) = value;
    }

    void xapic::enable() {
        //spurious interrupt is set to 0xFF, and 8-th bit (enable) is set
        write(LapicRegisters::SIVR, read(LapicRegisters::SIVR) | 0x1FF);
    }

    void xapic::calibrate_timer(uint64_t ms) {
        write(LapicRegisters::DCR, 0x3);
        write(LapicRegisters::InitialCountR, 0xFFFFFFFF);

        Hpet::sleep(ms);

        uint64_t count = 0xFFFFFFFF - read(LapicRegisters::CurrentCountR);

        write(LapicRegisters::LVTTimer, 0x20 | 0x20000);
        write(LapicRegisters::DCR, 0x3);
        write(LapicRegisters::InitialCountR, count);
    }

    void xapic::send_ipi(uint8_t apic, uint64_t ipi) {

    }

    void xapic::eoi() {
        write(LapicRegisters::EOI, 0);
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

    void ioapic::set_redirection(uint8_t vector, uint32_t gsi, uint32_t flags) {
        uint32_t reg = ((gsi - gsib) * 2) + 0x10;
        uint64_t redirection = vector;

        redirection |= (flags & 2) ? (1 << 13) : redirection;

        redirection |= (flags & 8) ? (1 << 15) : redirection;

        write(reg, redirection);
        write(reg + 1, (redirection >> 32));
    }

    uint8_t ioapic::max_gsi() {
        uint32_t value = read(1);

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

    uint32_t ioapic::base() {
        return gsib;
    }

}