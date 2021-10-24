#ifndef TTY_H
#define TTY_H

#include <fs/devfs.hpp>
#include <video.hpp>

#include <stddef.h>
#include <stdint.h>

namespace Tty {

    constexpr size_t LINE_START_X = 10;

    void init();

    class tty : public Devfs::device {
        uint16_t m_cursor_x;
        uint16_t m_cursor_y;

    public:
        tty();

        int read(size_t offset, size_t size, const char* buffer);
        int write(size_t offset, size_t size, char* buffer);
    };

    extern tty* tty1;
}

#endif