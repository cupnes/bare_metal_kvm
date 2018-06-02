CFLAGS = -Wall -Wextra
SRCS = main.c con.c common.c debug.c

all: vm

vm: $(SRCS)
	gcc $(CFLAGS) -o $@ $+

run: vm
	./vm

clean:
	rm -f *~ vm

.PHONY: run clean
