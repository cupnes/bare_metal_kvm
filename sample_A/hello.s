	.code16

	.text

	movb	$0x41, %al
	movb	$0x0e, %ah
	int	$0x10

halt_loop:
	hlt
	jmp	halt_loop
