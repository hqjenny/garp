
	.globl	addInt32Vector

	.text
	.ent	addInt32Vector
addInt32Vector:
	.frame	$sp,0,$31		# vars= 0, regs= 0/0, args= 0, extra= 0
	.mask	0x00000000,0
	.fmask	0x00000000,0

	.set noreorder
	la $8,config_addInt32Vector
	gaconf $8
        li $8,0x0C00
        li $9,1
	mtgavz $8,$9
	mtga $4,$z0,4
	gabump $5
	mfga $2,$z1
	.set reorder

	j	$31
	.end	addInt32Vector

