
	.text
	.align	2
	.globl	loadConfig_add3sub
	.align	2
	.globl	add3sub

	.text
	.text
	.ent	loadConfig_add3sub
loadConfig_add3sub:
	.frame	$sp,0,$31		# vars= 0, regs= 0/0, args= 0, extra= 0
	.mask	0x00000000,0
	.fmask	0x00000000,0

	.set noreorder
	la $2,config_add3sub
	gaconf $2
	.set reorder

	j	$31
	.end	loadConfig_add3sub

	.text
	.ent	add3sub
add3sub:
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
	.end	add3sub

