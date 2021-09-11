#include "hpet.hpp"
#include <acpi/acpi.hpp>
#include <memory/vmm.hpp>

namespace Hpet {

    const uint64_t MILISECOND_IN_FEMTOSECONDS = 1000000000000;

    uint32_t clock;
    hpetMem* hpet;

    void init() {
        hpet_table* table = reinterpret_cast<hpet_table*>(Acpi::find_table("HPET"));
        hpet = reinterpret_cast<hpetMem*>(table->address + PHYSICAL_BASE_ADDRESS);
        clock = hpet->general_capabilities >> 32;

        hpet->general_configuration = 1; //main counter starts counting
    }

    void sleep(uint64_t ms) {
        uint64_t target = hpet->main_counter_val + (ms * MILISECOND_IN_FEMTOSECONDS) / clock;
        while (hpet->main_counter_val < target); 
    }

}