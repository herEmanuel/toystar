#include "tty.hpp"
#include <memory/heap.hpp>
#include <video.hpp>
#include <utils.hpp>

#include <stdint.h>

//just to get things going

namespace Tty {
    tty* tty1 = nullptr;

    void init() {
        tty1 = new tty();
    }

    tty::tty() {
        m_cursor_x = LINE_START_X;
        m_cursor_y = 0;
    }

    //TODO: implement it :)
    int tty::read(size_t offset, size_t size, const char* buffer) {
        return -1;
    }

    int tty::write(size_t offset, size_t size, char* buffer) {
        for (size_t i = 0; i < size; i++) {
            print_char(buffer[i], 0xffffff);
        }
        
        return 0;
    }
}