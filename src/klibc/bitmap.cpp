#include "bitmap.hpp"
#include <stdint.h>
#include <stddef.h>

namespace toys {
    
    void set_bit(uint8_t* bitmap, size_t bit) {
        bitmap[bit/8] |= (1 << (7 - bit%8));
    }

    void toggle_bit(uint8_t* bitmap, size_t bit) {
        bitmap[bit/8] ^= (1 << (7 - bit%8));
    }

    void clear_bit(uint8_t* bitmap, size_t bit) {
        bitmap[bit/8] &= ~(1 << (7 - bit%8));
    }

    bool is_bit_set(uint8_t* bitmap, size_t bit) {
        if (bitmap[bit/8] & (1 << (7 - bit%8))) {
            return true;
        }
        
        return false;
    }

}