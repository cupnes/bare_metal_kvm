CFLAGS = -Wall -Wextra

all: hello

hello: asm_code/code.h hello.c
	gcc $(CFLAGS) -o $@ $+

asm_code/code.h:
	make -C asm_code

run: hello
	./hello

clean:
	rm -f *~ hello
	make -C asm_code clean

.PHONY: run clean
