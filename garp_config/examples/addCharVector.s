
	.globl	addCharVector

	.text
	.ent	addCharVector
addCharVector:
	.frame	$sp,0,$31		# vars= 0, regs= 0/0, args= 0, extra= 0
	.mask	0x00000000,0
	.fmask	0x00000000,0

	.set noreorder
 writestats
	la $8,config_addCharVector
	gaconf $8
 writestats
	la $8,queue0_addCharVector
	galqc $8,$0
	li $8,0x3000
	li $9,1
	mtgavz $8,$9
 writestate
	addi $4,$4,2
	gabump $4
 writestate
 writestate
 writestate
	mfga $2,$z0
 writestats
	.set reorder

	j	$31
	.end	addCharVector

