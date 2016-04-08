
	.globl	loadConfig_shiftUint32Right
	.globl	shiftUint32Right

	.text

	.align	2
	.ent	loadConfig_shiftUint32Right
loadConfig_shiftUint32Right:
	.frame	$sp,0,$31		# vars= 0, regs= 0/0, args= 0, extra= 0
	.mask	0x00000000,0
	.fmask	0x00000000,0

	.set noreorder
	la $2,config_shiftUint32Right
	gaconf $2
	.set reorder

	j	$31
	.end	loadConfig_shiftUint32Right

	.align	2
	.ent	shiftUint32Right
shiftUint32Right:
	.frame	$sp,0,$31		# vars= 0, regs= 0/0, args= 0, extra= 0
	.mask	0x00000000,0
	.fmask	0x00000000,0

	.set noreorder
	mtga $4,$z0
	mtga $5,$d0,3
	mfga $2,$z3
	.set reorder

	j	$31
	.end	shiftUint32Right

