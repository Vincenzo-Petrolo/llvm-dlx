	.text
	.file	"main.c"
	.globl	test                    ; -- Begin function test
	.type	test,@function
test:                                   ; @test
; %bb.0:                                ; %entry
	add	x0, x1, x0
	ret
.Lfunc_end0:
	.size	test, .Lfunc_end0-test
                                        ; -- End function
	.ident	"clang version 10.0.1 (git@github.com:Vincenzo-Petrolo/llvm-project.git 18f1ef7c0882ba547cb0093925208678cb173314)"
	.section	".note.GNU-stack","",@progbits
