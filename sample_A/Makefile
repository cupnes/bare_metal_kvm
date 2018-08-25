hello.mbr: hello.o
	ld -m elf_i386 -o $@ $< -T hello.ld -Map hello.map

hello.o: hello.s
	as --32 -o $@ $<

clean:
	rm -f *~ *.o *.mbr *.map

.PHONY: clean
