#ifndef BITMAP_H
#define BITMAP_H

#include <stdint.h>
#include <stddef.h>

namespace toys {
    
    inline void set_bit(uint8_t* bitmap, size_t bit) {
        bitmap[bit/8] |= (1 << (7 - bit%8));
    }

    inline void toggle_bit(uint8_t* bitmap, size_t bit) {
        bitmap[bit/8] ^= (1 << (7 - bit%8));
    }

    inline void clear_bit(uint8_t* bitmap, size_t bit) {
        bitmap[bit/8] &= ~(1 << (7 - bit%8));
    }

    inline bool is_bit_set(uint8_t* bitmap, size_t bit) {
        if (bitmap[bit/8] & (1 << (7 - bit%8))) {
            return true;
        }
        
        return false;
    }

    namespace Bitmap {

        inline int64_t allocate(uint8_t* bitmap) {
            for (size_t i = 0; i < sizeof(bitmap) * 8; i++) {
                if (is_bit_set(bitmap, i)) { continue; }

                set_bit(bitmap, i);
                return i;
            }

            return -1;
        }
        
    }

}

#endif