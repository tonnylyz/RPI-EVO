INCLUDES := -I../include -I./include

%.o: %.c
	$(CC) $(CFLAGS) $(INCLUDES) -c $<

.PHONY: clean

all: uart.o mbox.o

clean:
	rm -rf *~ *.o

include ../include.mk
