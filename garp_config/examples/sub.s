
	.text
	.align	2
	.globl	loadSubConfig
	.align	2
	.globl	sub

	.text
	.text
	.ent	loadSubConfig
loadSubConfig:
	.frame	$sp,0,$31		# vars= 0, regs= 0/0, args= 0, extra= 0
	.mask	0x00000000,0
	.fmask	0x00000000,0

	.set noreorder
	la $2,subConfig
	gaconf $2
	.set reorder

	j	$31
	.end	loadSubConfig

	.text
	.ent	sub
sub:
	.frame	$sp,0,$31		# vars= 0, regs= 0/0, args= 0, extra= 0
	.mask	0x00000000,0
	.fmask	0x00000000,0

	.set noreorder
	mtga $4,$z0
	mtga $5,$d0,1
	mfga $2,$d1
	.set reorder

	j	$31
	.end	sub

