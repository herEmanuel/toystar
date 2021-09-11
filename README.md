# Toystar

![Toystar screenshot](/screenshot.png?raw=true "Toystar screenshot")

Toystar is a 64 bits operating system made with the sole purpose of learning and having fun. It is currently in early development and still lacks a lot of features.

### :sparkles: Features

- [x] GDT/IDT
- [x] PMM/VMM
- [x] kmalloc/kfree (Buddy allocator)
- [x] Apic (xapic)
- [x] SMP support (multicore)
- [x] Keyboard and HPET
- [x] Scheduler
- [x] Syscalls
- [x] Vfs
- [x] Tmpfs
- [ ] RTC driver
- [ ] PCI
- [ ] AHCI driver
- [ ] Ext2 or FAT32 filesystem
- [ ] Elf loader 

### :checkered_flag: Goals

- Have fun 
- Be POSIX compliant
- Port several programs, like bash, gcc and python

### ⚙️ How do i build this?

In order to build Toystar, you will need:

- A x86_64 cross compiler and NASM
- Git
- Xorriso
- WSL or a linux VM (if you are on windows, like me)

Clone this repository and replace the variables in the makefiles containing the name of the tools to be used. After that just type `make`. This will download [Limine](https://github.com/limine-bootloader/limine) and create an iso file, which then can be run using [QEMU](https://www.qemu.org/).
