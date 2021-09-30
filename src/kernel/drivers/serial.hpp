#ifndef SERIAL_H
#define SERIAL_H

#define COM1 0x3f8 

namespace Serial {

    void init();
    void send_char(char c);

}

#endif