CFLAGS = -Wall -Wextra
SRCS = main.c bios_rom.c io.c serial.c ram.c util.c

all: vm

vm: $(SRCS)
	gcc $(CFLAGS) -o $@ $+

run: vm
	./vm

clean:
	rm -f *~ vm

02_bios_nodebug:
	rm -rf ../$@
	mkdir -p ../$@
	cp Makefile ../$@/
	for f in $(SRCS); do					\
		sed -E '/DEBUG_PRINT|assert/d' $$f > ../$@/$$f;	\
	done

.PHONY: run clean create_no_debug
