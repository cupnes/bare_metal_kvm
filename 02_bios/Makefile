CFLAGS = -Wall -Wextra
SRCS = main.c bios_rom.c io.c crtc.c ram.c common.c debug.c

all: vm

vm: $(SRCS)
	gcc $(CFLAGS) -o $@ $+

run: vm
	./vm

clean:
	rm -f *~ vm

.PHONY: run clean
