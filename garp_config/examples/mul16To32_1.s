
	.text
	.align	2
	.globl	loadConfig_mul16To32_1
	.align	2
	.globl	mul16To32_1

	.text
	.text
	.ent	loadConfig_mul16To32_1
loadConfig_mul16To32_1:
	.frame	$sp,0,$31		# vars= 0, regs= 0/0, args= 0, extra= 0
	.mask	0x00000000,0
	.fmask	0x00000000,0

	.set noreorder
	la $2,config_mul16To32_1
	gaconf $2
	.set reorder

	j	$31
	.end	loadConfig_mul16To32_1

	.text
	.ent	mul16To32_1
mul16To32_1:
	.frame	$sp,0,$31		# vars= 0, regs= 0/0, args= 0, extra= 0
	.mask	0x00000000,0
	.fmask	0x00000000,0

	.set noreorder
	mtga $4,$z0
	mtga $5,$z1,5
	mfga $2,$z8
	.set reorder

	j	$31
	.end	mul16To32_1

