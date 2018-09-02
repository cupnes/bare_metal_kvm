CC := gcc
CFLAGS := -Wall -g -Iinclude/

CFLAGS = -Wall -Wextra
# SRCS = main.c bios_rom.c io.c serial.c ram.c util.c
OBJS = main.o bios_rom.o io.o serial.o ram.o util.o

all: vm

vm: $(OBJS)
	$(CC) $(CFLAGS) -lSDL -pthread -o $@ $^

%.o: %.c
	@echo "CC       $<"
	@$(CC) -MM -MF $(subst .o,.d,$@) -MP -MT "$@ $(subst .o,.d,$@)" $(CFLAGS) $<
	@$(CC) -c -o $@ $(CFLAGS) $<

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
