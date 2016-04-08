
	.text
	.align	2
	.globl	loadCountlzConfig
	.align	2
	.globl	countlz

	.text
	.text
	.ent	loadCountlzConfig
loadCountlzConfig:
	.frame	$sp,0,$31		# vars= 0, regs= 0/0, args= 0, extra= 0
	.mask	0x00000000,0
	.fmask	0x00000000,0

	.set noreorder
	la $2,countlzConfig
	gaconf $2
	.set reorder

	j	$31
	.end	loadCountlzConfig

	.text
	.ent	countlz
countlz:
	.frame	$sp,0,$31		# vars= 0, regs= 0/0, args= 0, extra= 0
	.mask	0x00000000,0
	.fmask	0x00000000,0

	.set noreorder
	mtga $4,$d0,4
	mfga $2,$z2
	.set reorder
	andi $2,63

	j	$31
	.end	countlz

