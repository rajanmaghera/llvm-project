	.section	__TEXT,__text,regular,pure_instructions
	.build_version macos, 11, 3
	.globl	_second_main                    ; -- Begin function second_main
	.p2align	2
_second_main:                           ; @second_main
	.cfi_startproc
; %bb.0:
	lsl	x8, x0, #6
	add	x0, x8, x0, lsl #5
	ret
	.cfi_endproc
                                        ; -- End function
	.p2align	2                               ; -- Begin function main
_main:                                  ; @main
	.cfi_startproc
; %bb.0:
	sub	sp, sp, #32
	stp	x29, x30, [sp, #16]             ; 16-byte Folded Spill
	.cfi_def_cfa_offset 32
	.cfi_offset w30, -8
	.cfi_offset w29, -16
	ldr	x0, [x1, #8]
	bl	_atoi
	bl	_second_main
	mov	x8, sp
	str	x0, [x8]
	adrp	x0, _fmt_str@PAGE
	add	x0, x0, _fmt_str@PAGEOFF
	bl	_printf
	mov	x0, #0                          ; =0x0
	ldp	x29, x30, [sp, #16]             ; 16-byte Folded Reload
	add	sp, sp, #32
	ret
	.cfi_endproc
                                        ; -- End function
	.section	__TEXT,__const
_fmt_str:                               ; @fmt_str
	.ascii	"%ld\n"

.subsections_via_symbols
