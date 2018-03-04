INCLUDES = -I../include/ -I./

CFLAGS += -nostdlib -static

USERLIB := fd.o file.o fork.o fs.o fsipc.o ipc.o libos.o pgfault.o print.o printf.o string.o syscall_lib.o pageref.o

all: lab5

lab3: lab3_a.o lab3_b.o
	$(LD) -o lab3_a.elf -e main lab3_a.o
	$(LD) -o lab3_b.elf -e main lab3_b.o

lab4: fktest.elf pingpong.elf $(USERLIB) entry.o

lab5: fstest.elf serv.elf

%.elf: entry.o %.o $(USERLIB)
	$(LD) -T user.lds -o $@ -e _start -G 0 -static -n -nostdlib $^

%.o: %.c
	$(CC) $(CFLAGS) $(INCLUDES) -c -o $@ $<

%.o: %.S
	$(CC) $(CFLAGS) $(INCLUDES) -c -o $@ $<


.PHONY: clean

clean:
	rm -rf *~ *.o *.elf

include ../include.mk