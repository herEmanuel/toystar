#ifndef BITMAP_H
#define BITMAP_H

#include <stdint.h>
#include <stddef.h>

namespace toys {

    void set_bit(uint8_t* bitmap, size_t bit);
    void toggle_bit(uint8_t* bitmap, size_t bit);
    void clear_bit(uint8_t* bitmap, size_t bit);
    bool is_bit_set(uint8_t* bitmap, size_t bit);
    
}

#endif