CFLAGS  = -Wall -Wextra
CFLAGS += -nostdinc -nostdlib -fno-builtin -fno-common -c
LDFLAGS = -Map rom.map -s -x -T rom.ld

all: rom.h

%.h: %.bin
	xxd -i $^ > $@

%.bin: %.o
	ld $(LDFLAGS) -o $@ $+

%.o: %.s
	gcc $(CFLAGS) -o $@ $+

clean:
	rm -f *.o *.bin *.h *.map *~

.PHONY: clean
