
	.text
	.align	2
	.globl	loadAdd3Config
	.align	2
	.globl	add3

	.text
	.text
	.ent	loadAdd3Config
loadAdd3Config:
	.frame	$sp,0,$31		# vars= 0, regs= 0/0, args= 0, extra= 0
	.mask	0x00000000,0
	.fmask	0x00000000,0

	.set noreorder
	la $2,add3Config
	gaconf $2
	.set reorder

	j	$31
	.end	loadAdd3Config

	.text
	.ent	add3
add3:
	.frame	$sp,0,$31		# vars= 0, regs= 0/0, args= 0, extra= 0
	.mask	0x00000000,0
	.fmask	0x00000000,0

	.set noreorder
	mtga $4,$z0
	mtga $5,$z1
	mtga $6,$d1,1
	mfga $2,$d2
	.set reorder

	j	$31
	.end	add3

