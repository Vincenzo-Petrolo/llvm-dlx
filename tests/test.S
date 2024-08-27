	.text
	.file	"main.c"
	.globl	test                    ; -- Begin function test
	.type	test,@function
test:                                   ; @test
; %bb.0:                                ; %entry
	sw	0(r2), r10
	add	r10, r11, r10
	lw	r10, 0(r2)
	ret
.Lfunc_end0:
	.size	test, .Lfunc_end0-test
                                        ; -- End function
	.ident	"clang version 10.0.1 (git@github.com:Vincenzo-Petrolo/llvm-project.git e5d296ad7a4d087233837d52fe97b96db7192a8b)"
	.section	".note.GNU-stack","",@progbits
