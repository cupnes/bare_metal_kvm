output_A.mbr: output_A.o
	ld -m elf_i386 -o $@ $< -T mbr.ld -Map output_A.map

output_A.o: output_A.s
	as --32 -o $@ $<

clean:
	rm -f *~ *.o *.mbr *.map

.PHONY: clean
