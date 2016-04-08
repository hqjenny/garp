
	.globl	writestats

	.text
	.ent	writestats
writestats:
	.frame	$sp,0,$31		# vars= 0, regs= 0/0, args= 0, extra= 0
	.mask	0x00000000,0
	.fmask	0x00000000,0

	writestats

	j	$31
	.end	writestats

