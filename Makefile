ISO_IMAGE = disk.iso

.PHONY: clean all run

OS = toystar.elf
INIT_MODULE = init.tar

all: $(ISO_IMAGE)

run: $(ISO_IMAGE)
	qemu-system-x86_64.exe -smp 2 -M q35 -m 2G -serial file:CON -cdrom $(ISO_IMAGE)

kvm:
	qemu-system-x86_64.exe -smp 2 -M q35 -m 2G -cdrom $(ISO_IMAGE) -accel whpx

test: 
	qemu-system-x86_64.exe -smp 2 -d int -M smm=off -M q35 -m 2G -serial file:CON -cdrom $(ISO_IMAGE)

limine:
	git clone https://github.com/limine-bootloader/limine.git --branch=v2.0-branch-binary --depth=1
	make -C limine

src/$(OS):
	$(MAKE) -C src

$(ISO_IMAGE): limine src/$(OS)
	rm -rf iso_root
	mkdir -p iso_root
	cp src/$(OS) $(INIT_MODULE) \
		limine.cfg limine/limine.sys limine/limine-cd.bin limine/limine-eltorito-efi.bin iso_root/
	xorriso -as mkisofs -b limine-cd.bin \
		-no-emul-boot -boot-load-size 4 -boot-info-table \
		--efi-boot limine-eltorito-efi.bin \
		-efi-boot-part --efi-boot-image --protective-msdos-label \
		iso_root -o $(ISO_IMAGE)
	limine/limine-install $(ISO_IMAGE)
	rm -rf iso_root

clean:
	rm -f $(ISO_IMAGE)
	$(MAKE) -C src clean
