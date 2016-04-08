
	.globl	loadConfig_demandRead
	.globl	demandRead

	.text

	.align	2
	.ent	loadConfig_demandRead
loadConfig_demandRead:
	.frame	$sp,0,$31		# vars= 0, regs= 0/0, args= 0, extra= 0
	.mask	0x00000000,0
	.fmask	0x00000000,0

	.set noreorder
	la $2,config_demandRead
	gaconf $2
	.set reorder

	j	$31
	.end	loadConfig_demandRead

	.align	2
	.ent	demandRead
demandRead:
	.frame	$sp,0,$31		# vars= 0, regs= 0/0, args= 0, extra= 0
	.mask	0x00000000,0
	.fmask	0x00000000,0

	.set noreorder
	mtga $4,$z0,15
	mfga $4,$z0
	.set reorder

	j	$31
	.end	demandRead

