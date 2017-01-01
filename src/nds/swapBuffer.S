#include "mips.h"

#define SCREEN_WIDTH	256
#define SCREEN_HEIGHT	192
#define VIDEO_BUFFER_WIDTH 512
#define VIDEO_BUFFER_HEIGHT 512
//#define _PRINTDEBUG


	#ifdef _PRINTDEBUG
	.extern zprintf
	.data
	info1: .asciiz "swp done"
	info0: .asciiz "swp start"
	info2: .asciiz "swp "
	#endif

	.text
	.extern drvWidth
	.extern drvHeight
	.extern iAdd
	.extern iModulo
	.extern xOff
	.extern yOff
	.extern up_screen_addr

	.set mips32

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	.ent swapBufferHQ
	.global	swapBufferHQ
	.type	swapBufferHQ,@function
	.set	noreorder
	.set	noat
swapBufferHQ:

	#ifdef _PRINTDEBUG
	addi	sp, sp, -4
	sw		a0, 0(sp)
	li		a0, 3
	la		a1, info0
	la		t0, zprintf
	jalr	t0
	nop
	lw		a0, 0(sp)
	addi	sp, sp, 4
	#endif

	addi	sp,	sp,	-4
	sw		fp, 0(sp)

	la		AT, up_screen_addr
	lw		a1, 0(AT)

	la		AT, xOff
	pref	0, 0(AT)
	li		v0, SCREEN_WIDTH
	lw		v1, 0(AT)
	la		AT, yOff
	lw		a2, 0(AT)
	mul		v0, v0, a2
	add		v1, v1, v0
	sll		v1, v1, 1	#short(x2)

	add		a1, a1, v1

	la		AT, drvHeight
	lw		v0, 0(AT)

	la		AT, drvWidth
	lw		v1, 0(AT)

	#pitch padding
	li		AT, VIDEO_BUFFER_WIDTH
	sub		AT, AT, v1
	sll		AT, AT, 1	#short(x2)
	move	fp, AT

	la		AT, iModulo
	lw		a3, 0(AT)
	la		AT, iAdd
	lw		a2,	0(AT)

	pref	4, 0(a0)
	add		AT, zero, zero		#mix
	add		t5, zero, zero		#accy
	add		t0, zero, zero

LOOP0:	#for (short h = 0; h < height; h++)
	add		t1, zero, zero

	add		t2, zero, zero		#accx
	add		t3, zero, zero		#pixel
	move	t6, a1

LOOP1:	#for (short w = 0; w < width; ++w)

	lhu		t4, 0(a0)
	addu	a0, a0, 2
	sub		t7, t2, a2
	bltz	t7, MIXEND
	pref	4, 0(a0)		#delay slot
#MIX:
	#mix t3 to t4
	andi	t7, t3, 0x1F
	andi	t8,	t3, 0x03E0
	andi	t9,	t3, 0x7C00

	andi	t3,	t4,	0x1F
	addu	t7,	t7, t3
	srl		t7, t7, 1
	andi	t3,	t4,	0x03E0
	addu	t8, t8, t3
	srl		t8,	t8, 1
	andi	t3, t4, 0x7C00
	addu	t9,	t9, t3
	srl		t9, t9, 1

	andi	t4, t8, 0x03E0
	or		t4, t4, t7
	andi	t9, t9, 0x7C00
	or		t4, t4, t9

MIXEND:
	add		t2, t2, a2
	sub		t7, t2, a3
	bltz	t7, MIX2END
	move	t3, t4			#delay slot
#MIX2:
	beq		AT, zero, NEXT_DEST
	move	t2, t7			#delay slot

	lhu		t4, 0(t6)
	#mix t4 to t3
	andi	t7, t3, 0x1F
	andi	t8,	t3, 0x03E0
	andi	t9,	t3, 0x7C00

	andi	t3,	t4,	0x1F
	addu	t7,	t7, t3
	srl		t7, t7, 1
	andi	t3,	t4,	0x03E0
	addu	t8, t8, t3
	srl		t8,	t8, 1
	andi	t3, t4, 0x7C00
	addu	t9,	t9, t3
	srl		t9, t9, 1

	andi	t3, t8, 0x03E0
	or		t3, t3, t7
	andi	t9, t9, 0x7C00
	or		t3, t3, t9

NEXT_DEST:
	sh		t3, 0(t6)
	addu	t6, t6, 2
MIX2END:
	addi	t1, t1, 1
	bne		t1, v1, LOOP1	#end loop1
	pref	7, 0(t6)		#delay slot

	add		t5, t5, a2
	sub		t4, t5, a3
	bltz	t4, LOOP0TAIL
	li		AT, 1			#delay slot
#NEXT_DEST_ROW:
	add		AT, zero, zero
	move	t5, t4
	addi	a1, a1, SCREEN_WIDTH * 2
	pref	7, 0(a1)

LOOP0TAIL:
	addu	a0,	a0, fp

	addi	t0, t0, 1
	bne		t0, v0, LOOP0	#end loop0
	pref	4, 0(a0)		#delay slot

	#ifdef _PRINTDEBUG
	addi	sp, sp, -4
	li		a0, 3
	la		a1, info1
	la		t0, zprintf
	jalr	t0
	nop
	addi	sp, sp, 4
	#endif

	lw		fp, 0(sp)
	jr		ra
	addi	sp,	sp,	4

	.set	at
	.set	reorder
	.end swapBufferHQ


	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	.ent swapBufferLQ
	.global	swapBufferLQ
	.type	swapBufferLQ,@function
	.set	noreorder
	.set	noat
swapBufferLQ:

	la		AT, up_screen_addr
	lw		a1, 0(AT)

	la		AT, xOff
	pref	0, 0(AT)
	li		v0, SCREEN_WIDTH
	lw		v1, 0(AT)
	la		AT, yOff
	lw		a2, 0(AT)
	mul		v0, v0, a2
	add		v1, v1, v0
	sll		v1, v1, 1	#short(x2)

	add		a1, a1, v1

	la		AT, drvHeight
	lw		v0, 0(AT)

	la		AT, drvWidth
	lw		v1, 0(AT)

	la		AT, iModulo
	lw		a3, 0(AT)
	la		AT, iAdd
	lw		a2,	0(AT)

	pref	4, 0(a0)
	add		t5, zero, zero		#accy

	add		t0, zero, zero
LOOP0LQ:	#for (short h = 0; h < height; h++)

	add		t5, t5, a2
	sub		t4, t5, a3
	bltz	t4, LOOP0LQTAIL
	nop
	move	t5, t4

	add		t2, zero, zero		#accx
	add		t3, zero, zero		#pixel
	move	t6, a1
	move	AT, a0

	add		t1, zero, zero
LOOP1LQ:	#for (short w = 0; w < width; ++w)

	lhu		t4, 0(AT)
	addu	AT, AT, 2
	sub		t7, t2, a2
	bltz	t7, MIXENDLQ
	pref	4, 0(AT)		#delay slot
#MIXLQ:
	#mix t3 to t4
	andi	t7, t3, 0x1F
	andi	t8,	t3, 0x03E0
	andi	t9,	t3, 0x7C00

	andi	t3,	t4,	0x1F
	addu	t7,	t7, t3
	srl		t7, t7, 1
	andi	t3,	t4,	0x03E0
	addu	t8, t8, t3
	srl		t8,	t8, 1
	andi	t3, t4, 0x7C00
	addu	t9,	t9, t3
	srl		t9, t9, 1

	andi	t4, t8, 0x03E0
	or		t4, t4, t7
	andi	t9, t9, 0x7C00
	or		t4, t4, t9

MIXENDLQ:
	move	t3,	t4
	add		t2, t2, a2
	sub		t7, t2, a3
	bltz	t7, LOOP1LQTAIL
	nop						#delay slot
	move	t2, t7
	sh		t3, 0(t6)
	addu	t6, t6, 2
LOOP1LQTAIL:
	addi	t1, t1, 1
	bne		t1, v1, LOOP1LQ	#end loop1
	pref	7, 0(t6)		#delay slot

#NEXT_DEST_ROWLQ:
	addi	a1, a1, SCREEN_WIDTH * 2
	pref	7, 0(a1)

LOOP0LQTAIL:
	addu	a0,	a0, VIDEO_BUFFER_WIDTH * 2
	addi	t0, t0, 1
	bne		t0, v0, LOOP0LQ	#end loop0
	pref	4, 0(a0)		#delay slot

	jr		ra
	nop

	.set	at
	.set	reorder
	.end swapBufferLQ

