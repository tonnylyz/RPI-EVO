INCLUDES = -I../include/ -I./

CFLAGS += -nostdlib -static

all: lab3

lab3: lab3_a.o lab3_b.o
	$(LD) -o lab3_a.elf -e main lab3_a.o
	$(LD) -o lab3_b.elf -e main lab3_b.o

%.o: %.c
	$(CC) $(CFLAGS) $(INCLUDES) -c -o $@ $<

%.o: %.S
	$(CC) $(CFLAGS) $(INCLUDES) -c -o $@ $<


.PHONY: clean

clean:
	rm -rf *~ *.o *.elf

include ../include.mk