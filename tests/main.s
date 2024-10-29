	.text
	.file	"main.c"
	.globl	acc
	.type	acc,@function
acc:
	sw	8(r2), r10
	lhi	r5, 0
	addi	r5, r5, 0
	sw	4(r2), r5
	sw	0(r2), r5
	j	.LBB0_1
.LBB0_1:
	lw	r5, 0(r2)
	lw	r6, 8(r2)
	sge	r5, r5, r6
	bnez	r5, .LBB0_4
	j	.LBB0_2
.LBB0_2:
	lw	r5, 0(r2)
	lw	r6, 4(r2)
	add	r5, r6, r5
	sw	4(r2), r5
	j	.LBB0_3
.LBB0_3:
	lw	r5, 0(r2)
	addi	r5, r5, 1
	sw	0(r2), r5
	j	.LBB0_1
.LBB0_4:
	lw	r10, 4(r2)
	ret
.Lfunc_end0:
	.size	acc, .Lfunc_end0-acc

	.globl	main
	.type	main,@function
main:
	lhi	r5, 0
	addi	r5, r5, 0
	sw	16(r2), r5
	lhi	r5, 2
	addi	r5, r5, 2
	sw	12(r2), r5
	lhi	r5, 1
	addi	r5, r5, 1
	sw	8(r2), r5
	lw	r10, 12(r2)
	lhi	r5, %hi(acc)
	ori	r5, r5, %lo(acc)
	sw	4(r2), r5
	jalr	r5
	lw	r5, 8(r2)
	sw	0(r2), r10
	addi	r10, r5, 0
	lw	r5, 4(r2)
	jalr	r5
	lw	r5, 0(r2)
	add	r10, r5, r10
	ret
.Lfunc_end1:
	.size	main, .Lfunc_end1-main

	.globl	_start
	.type	_start,@function
_start:
	lhi	r5, %hi(main)
	ori	r5, r5, %lo(main)
	jalr	r5
	ret
.Lfunc_end2:
	.size	_start, .Lfunc_end2-_start

	.ident	"clang version 10.0.1 (https://github.com/Vincenzo-Petrolo/llvm-project.git e86f0cc6b73906c51584bdf9f4af231a0c82f0d9)"
	.section	".note.GNU-stack","",@progbits
	.addrsig
	.addrsig_sym acc
	.addrsig_sym main
