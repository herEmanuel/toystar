#ifndef BITMAP_H
#define BITMAP_H

#include <stdint.h>
#include <stddef.h>

namespace toys {

    void setBit(uint8_t* bitmap, size_t bit);
    void toggleBit(uint8_t* bitmap, size_t bit);
    void clearBit(uint8_t* bitmap, size_t bit);
    bool isBitSet(uint8_t* bitmap, size_t bit);
    
}

#endif