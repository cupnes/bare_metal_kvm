CFLAGS = -Wall -Wextra

all: vm

vm: rom/rom.h main.c
	gcc $(CFLAGS) -o $@ $+

rom/rom.h:
	make -C rom

run: vm
	./vm

clean:
	rm -f *~ vm
	make -C rom clean

.PHONY: run clean
