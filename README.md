# Toystar

![Toystar screenshot](/os.png?raw=true "Toystar screenshot")

Toystar is a 64 bits operating system made with the sole purpose of learning and having fun. It is currently in early development and still lacks a lot of features.

### Features being worked on/planned

- PCI
- Ext2 or FAT32 filesystem
- AHCI driver
- Preemptive multitasking scheduler

... and many more!

### How do i build this?

In order to build Toystar, you will need:

- A x86_64 cross compiler and GAS (Gnu assembler)
- Git
- Xorriso
- WSL or a linux VM (if you are on windows, like me)

Clone this repository and replace the variables in the makefiles containing the name of the tools to be used. After that just type `make`. This will download [Limine](https://github.com/limine-bootloader/limine) and create an iso file, which then can be run using [QEMU](https://www.qemu.org/).
