CC := x86_64-elf-g++.exe
ASM := x86_64-elf-as.exe

CFLAGS := -I . -ffreestanding -Wall -Wextra -fPIC \
	-fno-exceptions      \
	-fno-rtti			 \
	-fno-stack-protector \
	-fno-pic -fpie       \
	-mno-80387           \
	-mno-mmx             \
	-mno-3dnow           \
	-mno-sse             \
	-mno-sse2            \
	-mno-red-zone

LFLAGS := -ffreestanding -nostdlib -lgcc -pie

OS_NAME := toystar
LINKER := kernel/linker.ld
STIVALE_PATH = boot/stivale2.h

src := $(shell find -name '*.cpp')

asmSrc := $(shell find -name '*.s')

asmObjs := $(subst .s,.o,$(patsubst %,../obj/%,$(asm_src)))

objs := $(subst .cpp,.o,$(patsubst %,../obj/%,$(src))) \
		$(asm_objs)

.PHONY: clean all

all: $(OS_NAME)

$(OS_NAME): $(objs) 
	$(CC) -T $(LINKER) -o $(OS_NAME).elf $(LFLAGS) $(objs)

# obj/%.o: src/%.s | $(dirs)
# 	$(ASM) $< -o $@

# obj/kernel/interrupts/interrupts.o: src/kernel/interrupts/interrupts.cpp
# 	$(CC) -c $< -o $@ $(CFLAGS) -mgeneral-regs-only -mno-80387

../obj/%.o: %.cpp $(STIVALE_PATH) | dirs
	$(CC) -c $< -o $@ $(CFLAGS)

stivale2.h:
	wget https://github.com/stivale/stivale/raw/master/stivale2.h

dirs:
	mkdir -p $(shell dirname $(objs))
# rm -rf ../obj
clean:
	rm -f $(OS_NAME).elf

