INCLUDES := -I../include
%.o: %.c
	$(CC) $(CFLAGS) $(INCLUDES) -c $<

.PHONY: clean
all: uart.o sd.o timer.o
clean:
	rm -rf *~ *.o

include ../include.mk
