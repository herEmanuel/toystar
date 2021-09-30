#include "serial.hpp"
#include <kernel/io.hpp>

namespace Serial {

   void init() {
      outb(COM1 + 1, 0x00);    
      outb(COM1 + 3, 0x80);    
      outb(COM1 + 0, 0x03);   
      outb(COM1 + 1, 0x00);    
      outb(COM1 + 3, 0x03);   
      outb(COM1 + 2, 0xC7);    
      outb(COM1 + 4, 0x0B);    
   }

   int is_transmit_empty() {
      return inb(COM1 + 5) & 0x20;
   }
   
   void send_char(char c) {
      while (is_transmit_empty() == 0);
   
      outb(COM1, c);
   }

}
