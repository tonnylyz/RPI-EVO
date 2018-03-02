drivers_dir   := drivers
boot_dir      := boot
init_dir      := init
lib_dir       := lib
mm_dir        := mm
vmlinux_elf   := kernel.elf
vmlinux_img   := kernel8.img
link_script   := kernel.lds

modules       := boot drivers init lib user
objects       := $(boot_dir)/*.o \
                 $(init_dir)/*.o \
                 $(drivers_dir)/*.o \
                 $(lib_dir)/*.o

.PHONY: all $(modules) clean

all: $(modules) vmlinux

vmlinux: $(modules)
	$(CROSS)ld -o $(vmlinux_elf) -e _start -T$(link_script) $(objects)
	$(CROSS)objcopy $(vmlinux_elf) -O binary $(vmlinux_img)

lab3: user sdcard.img
	dd if=user/lab3_a.elf of=sdcard.img seek=1000
	dd if=user/lab3_b.elf of=sdcard.img seek=2000

lab4: user sdcard.img
	dd if=user/fktest.elf of=sdcard.img seek=3000
	dd if=user/pingpong.elf of=sdcard.img seek=4000

sdcard.img:
	dd if=/dev/zero of=sdcard.img bs=512 count=10000

$(modules):
	$(MAKE) --directory=$@

clean:
	for d in $(modules); \
		do \
			$(MAKE) --directory=$$d clean; \
		done; \
	rm -rf *.o *~ $(vmlinux_elf)

include include.mk