
	.text
	.align	2
	.globl	loadConfig_reverse
	.align	2
	.globl	reverse

	.text
	.text
	.ent	loadConfig_reverse
loadConfig_reverse:
	.frame	$sp,0,$31		# vars= 0, regs= 0/0, args= 0, extra= 0
	.mask	0x00000000,0
	.fmask	0x00000000,0

	.set noreorder
	la $2,config_reverse
	gaconf $2
	.set reorder

	j	$31
	.end	loadConfig_reverse

	.text
	.ent	reverse
reverse:
	.frame	$sp,0,$31		# vars= 0, regs= 0/0, args= 0, extra= 0
	.mask	0x00000000,0
	.fmask	0x00000000,0

	.set noreorder
	mtga $4,$z0,2
	mfga $2,$z1
	.set reorder

	j	$31
	.end	reverse

