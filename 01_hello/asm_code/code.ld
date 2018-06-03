OUTPUT_FORMAT("binary");

SECTIONS
{
	. = 0x00000000fffffff0;
	.text	: {*(.text)}
	.rodata	: {
		*(.strings)
		*(.rodata)
		*(.rodata.*)
	}
	.data	: {*(.data)}
	.bss	: {*(.bss)}
}
