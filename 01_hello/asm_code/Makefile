CFLAGS  = -Wall -Wextra
CFLAGS += -nostdinc -nostdlib -fno-builtin -fno-common -c
LDFLAGS = -Map code.map -s -x -T code.ld

all: code.h

%.h: %.bin
	xxd -i $^ > $@

%.bin: %.o
	ld $(LDFLAGS) -o $@ $+

%.o: %.s
	gcc $(CFLAGS) -o $@ $+

clean:
	rm -f *.o *.bin *.h *.map *~

.PHONY: clean
