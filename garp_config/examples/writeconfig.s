
	.globl	writeconfig

	.text
	.ent	writeconfig
writeconfig:
	.frame	$sp,0,$31		# vars= 0, regs= 0/0, args= 0, extra= 0
	.mask	0x00000000,0
	.fmask	0x00000000,0

	writeconfig

	j	$31
	.end	writeconfig

