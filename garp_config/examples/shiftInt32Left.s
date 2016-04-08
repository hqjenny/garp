
	.globl	loadConfig_shiftInt32Left
	.globl	shiftInt32Left

	.text

	.align	2
	.ent	loadConfig_shiftInt32Left
loadConfig_shiftInt32Left:
	.frame	$sp,0,$31		# vars= 0, regs= 0/0, args= 0, extra= 0
	.mask	0x00000000,0
	.fmask	0x00000000,0

	.set noreorder
	la $2,config_shiftInt32Left
	gaconf $2
	.set reorder

	j	$31
	.end	loadConfig_shiftInt32Left

	.align	2
	.ent	shiftInt32Left
shiftInt32Left:
	.frame	$sp,0,$31		# vars= 0, regs= 0/0, args= 0, extra= 0
	.mask	0x00000000,0
	.fmask	0x00000000,0

	.set noreorder
	mtga $4,$z0
	mtga $5,$d0,3
	mfga $2,$z3
	.set reorder

	j	$31
	.end	shiftInt32Left

