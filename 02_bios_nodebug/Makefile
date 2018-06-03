CFLAGS = -Wall -Wextra
SRCS = main.c bios_rom.c serial.c ram.c util.c

all: vm

vm: $(SRCS)
	gcc $(CFLAGS) -o $@ $+

run: vm
	./vm

clean:
	rm -f *~ vm

.PHONY: run clean
