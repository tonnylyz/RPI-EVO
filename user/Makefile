INCLUDES = -I../include/ -I./

CFLAGS += -nostdlib -static

USERLIB := printf.o \
		print.o \
		libos.o \
		fork.o \
		syscall_lib.o \
		ipc.o \
		pgfault.o \
		string.o

all: lab4

lab3: lab3_a.o lab3_b.o
	$(LD) -o lab3_a.elf -e main lab3_a.o
	$(LD) -o lab3_b.elf -e main lab3_b.o

lab4: fktest.elf pingpong.elf $(USERLIB) entry.o

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