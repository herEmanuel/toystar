#include "bitmap.hpp"
#include <stdint.h>
#include <stddef.h>

namespace toys {
    
    void setBit(uint8_t* bitmap, size_t bit) {
        bitmap[bit/8] |= (1 << (7 - bit%8));
    }

    void toggleBit(uint8_t* bitmap, size_t bit) {
        bitmap[bit/8] ^= (1 << (7 - bit%8));
    }

    void clearBit(uint8_t* bitmap, size_t bit) {
        bitmap[bit/8] &= ~(1 << (7 - bit%8));
    }

    bool isBitSet(uint8_t* bitmap, size_t bit) {
        if (bitmap[bit/8] & (1 << (7 - bit%8))) {
            return true;
        }
        
        return false;
    }

}