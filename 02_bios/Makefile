CFLAGS = -Wall -Wextra

all: vm

vm: asm_code/code.h main.c
	gcc $(CFLAGS) -o $@ $+

asm_code/code.h:
	make -C asm_code

run: vm
	./vm

clean:
	rm -f *~ vm
	make -C asm_code clean

.PHONY: run clean
