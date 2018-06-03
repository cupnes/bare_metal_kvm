CFLAGS = -Wall -Wextra

all: vm

vm: asm_code/rom.h vm.c
	gcc $(CFLAGS) -o $@ $+

asm_code/rom.h:
	make -C asm_code

run: vm
	./vm

clean:
	rm -f *~ vm
	make -C asm_code clean

.PHONY: run clean
