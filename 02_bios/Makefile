CFLAGS = -Wall -Wextra
SRCS = main.c rtc.c a20.c con.c common.c

all: vm

vm: asm_code/code.h $(SRCS)
	gcc $(CFLAGS) -o $@ $+

asm_code/code.h:
	make -C asm_code

run: vm
	./vm

clean:
	rm -f *~ vm
	make -C asm_code clean

.PHONY: run clean
