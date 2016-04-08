
	.text
	.align	2
	.globl	loadConfig_mul16To32_4
	.align	2
	.globl	mul16To32_4

	.text
	.text
	.ent	loadConfig_mul16To32_4
loadConfig_mul16To32_4:
	.frame	$sp,0,$31		# vars= 0, regs= 0/0, args= 0, extra= 0
	.mask	0x00000000,0
	.fmask	0x00000000,0

	.set noreorder
	la $2,config_mul16To32_4
	gaconf $2
	.set reorder

	j	$31
	.end	loadConfig_mul16To32_4

	.text
	.ent	mul16To32_4
mul16To32_4:
	.frame	$sp,0,$31		# vars= 0, regs= 0/0, args= 0, extra= 0
	.mask	0x00000000,0
	.fmask	0x00000000,0

	.set noreorder
	li $2,0x80
	li $3,(3*2+0)
	mtgavy $2,$3
	mtga $4,$z0
	mtga $5,$z1
	mtga $0,$z5,7
	mfga $2,$z5
	.set reorder

	j	$31
	.end	mul16To32_4

