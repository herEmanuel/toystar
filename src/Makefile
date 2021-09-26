CC := x86_64-elf-g++
ASM := nasm

CFLAGS := -I . -I ./kernel -I ./klibc -ffreestanding -Wall -Wextra \
	-std=c++17			 \
	-fno-pic 		     \
	-fno-exceptions      \
	-fno-rtti			 \
	-fno-stack-protector \
	-fno-use-cxa-atexit  \
	-mno-80387           \
	-mno-mmx             \
	-mno-3dnow           \
	-mno-sse             \
	-mno-sse2            \
	-mno-red-zone		 \
	-mcmodel=kernel		


LFLAGS := -nostdlib -no-pie -lgcc

ASM_FLAGS := -felf64

OS_NAME := toystar
LINKER := kernel/linker.ld
STIVALE_PATH = boot/stivale2.hpp

CRTBEGIN_OBJ := $(shell $(CC) $(CFLAGS) -print-file-name=crtbegin.o)
CRTEND_OBJ := $(shell $(CC) $(CFLAGS) -print-file-name=crtend.o)

SRC := $(shell find -name '*.cpp')

ASM_SRC := $(shell find -name '*.asm' ! -name 'crt*.asm')

ASM_OBJS := $(subst .asm,.o,$(patsubst %,../obj/%,$(ASM_SRC)))

OBJS := $(subst .cpp,.o,$(patsubst %,../obj/%,$(SRC))) \
		$(ASM_OBJS) 

.PHONY: clean all

all: $(OS_NAME)

$(OS_NAME): $(OBJS) ../obj/crti.o ../obj/crtn.o
	$(CC) -T $(LINKER) -o $(OS_NAME).elf $(LFLAGS) ../obj/crti.o $(CRTBEGIN_OBJ) $(OBJS) $(CRTEND_OBJ) ../obj/crtn.o 

../obj/%.o: %.asm | $(dirs)
	$(ASM) $(ASM_FLAGS) $< -o $@

../obj/%.o: %.cpp $(STIVALE_PATH) | dirs
	$(CC) -c $< -o $@ $(CFLAGS)

stivale2.h:
	wget https://github.com/stivale/stivale/raw/master/stivale2.h

dirs:
	mkdir -p $(shell dirname $(OBJS))

clean:
	rm -f $(OS_NAME).elf

	