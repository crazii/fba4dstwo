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


#define LOOPUNROLL_HQ_PREFIX(PREFIX, N) \
	sub		t7, t2, a2; \
	bltz	t7, PREFIX##MIXEND##N; \
	lhu		t4, N(a0); \
	\
	andi	t3, t3, 0x7BDE; \
	andi	t4,	t4, 0x7BDE; \
	addu	t4, t4, t3; \
	srl		t4, t4, 1; \
	\
PREFIX##MIXEND##N: \
	add		t2, t2, a2; \
	sub		t7, t2, a3; \
	bltz	t7, PREFIX##MIX2END##N; \
	move	t3, t4; \
	\
	beq		AT, zero, PREFIX##NEXT_DEST##N; \
	move	t2, t7; \
	\
	lhu		t4, 0(t6); \
	\
	andi	t3, t3, 0x7BDE; \
	andi	t4,	t4, 0x7BDE; \
	addu	t3, t4, t3; \
	srl		t3, t3, 1; \
	\
PREFIX##NEXT_DEST##N: \
	sh		t3, 0(t6); \
	addu	t6, t6, 2; \
PREFIX##MIX2END##N:


#define LOOPUNROLL_HQ(N) LOOPUNROLL_HQ_PREFIX(CORE, N)
#define LOOPUNROLL_HQFINAL(N) LOOPUNROLL_HQ_PREFIX(FINAL, N)

	.text
	.extern drvWidth
	.extern drvHeight
	.extern iAdd
	.extern iModulo
	.extern xOff
	.extern yOff
	.extern up_screen_addr

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	.ent swapBufferHQ
	.global	swapBufferHQ
	.type	swapBufferHQ,@function
	.set mips32
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

	LOOPUNROLL_HQ(0)
	LOOPUNROLL_HQ(2)
	LOOPUNROLL_HQ(4)
	LOOPUNROLL_HQ(6)
	LOOPUNROLL_HQ(8)
	LOOPUNROLL_HQ(10)
	LOOPUNROLL_HQ(12)
	LOOPUNROLL_HQ(14)
	LOOPUNROLL_HQ(16)
	LOOPUNROLL_HQ(18)
	LOOPUNROLL_HQ(20)
	LOOPUNROLL_HQ(22)
	LOOPUNROLL_HQ(24)
	LOOPUNROLL_HQ(26)
	LOOPUNROLL_HQ(28)
	LOOPUNROLL_HQ(30)

#MIX2END:
	addi	t1, t1, 16
	bne		t1, v1, LOOP1	#end loop1
	addi	a0, a0, 32		#delay slot

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







#define LOOPUNROLL_LQ_PREFIX(PREFIX, N) \
	sub		t7, t2, a2; \
	bltz	t7, PREFIX##MIXENDLQ##N; \
	lhu		t4, N(AT); \
	\
	andi	t3, t3, 0x7BDE; \
	andi	t4,	t4, 0x7BDE; \
	addu	t4, t4, t3; \
	srl		t4, t4, 1; \
	\
PREFIX##MIXENDLQ##N: \
	add		t2, t2, a2; \
	sub		t7, t2, a3; \
	bltz	t7, PREFIX##LOOPUNROLLNEXT##N; \
	move	t3,	t4; \
	move	t2, t7; \
	sh		t3, 0(t6); \
	addu	t6, t6, 2; \
PREFIX##LOOPUNROLLNEXT##N:

#define LOOPUNROLL_LQ(N) LOOPUNROLL_LQ_PREFIX(CORE, N)
#define LOOPUNROLL_LQFINAL(N) LOOPUNROLL_LQ_PREFIX(FINAL, N)

#define LQ_UNROLL 16

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	.ent swapBufferLQ
	.global	swapBufferLQ
	.type	swapBufferLQ,@function
	.set	noreorder
	.set	noat
swapBufferLQ:
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
	move	fp, v1
	li		AT, (LQ_UNROLL-1)
	not		AT, AT
	and		v1, v1, AT

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
LOOP1LQ:	#for (short w = 0; w < width/32; ++w)

	#pref	5, 0(t6)
	#pref	4, 0(AT)

	LOOPUNROLL_LQ(0)
	LOOPUNROLL_LQ(2)
	LOOPUNROLL_LQ(4)
	LOOPUNROLL_LQ(6)
	LOOPUNROLL_LQ(8)
	LOOPUNROLL_LQ(10)
	LOOPUNROLL_LQ(12)
	LOOPUNROLL_LQ(14)
	LOOPUNROLL_LQ(16)
	LOOPUNROLL_LQ(18)
	LOOPUNROLL_LQ(20)
	LOOPUNROLL_LQ(22)
	LOOPUNROLL_LQ(24)
	LOOPUNROLL_LQ(26)
	LOOPUNROLL_LQ(28)
	LOOPUNROLL_LQ(30)
#if LQ_UNROLL != 16
	LOOPUNROLL_LQ(32)
	LOOPUNROLL_LQ(34)
	LOOPUNROLL_LQ(36)
	LOOPUNROLL_LQ(38)
	LOOPUNROLL_LQ(40)
	LOOPUNROLL_LQ(42)
	LOOPUNROLL_LQ(44)
	LOOPUNROLL_LQ(46)
	LOOPUNROLL_LQ(48)
	LOOPUNROLL_LQ(50)
	LOOPUNROLL_LQ(52)
	LOOPUNROLL_LQ(54)
	LOOPUNROLL_LQ(56)
	LOOPUNROLL_LQ(58)
	LOOPUNROLL_LQ(60)
	LOOPUNROLL_LQ(62)
#endif

#LOOP1LQTAIL:
	addi	t1, t1, LQ_UNROLL
	bne		t1, v1, LOOP1LQ	#end loop1
	addi	AT, AT, LQ_UNROLL*2		#delay slot

	beq		fp, v1, NEXT_DEST_ROWLQ
LOOP1LQFINAL:	#for (short w = 0; w < width%32; ++w)
	LOOPUNROLL_LQFINAL(0)
	LOOPUNROLL_LQFINAL(2)
	LOOPUNROLL_LQFINAL(4)
	LOOPUNROLL_LQFINAL(6)
	addi	t1, t1, 4
	bne		t1, fp, LOOP1LQFINAL	#end loop1final
	addi	AT, AT, 8				#delay slot

NEXT_DEST_ROWLQ:
	addi	a1, a1, SCREEN_WIDTH * 2

LOOP0LQTAIL:
	addi	t0, t0, 1
	bne		t0, v0, LOOP0LQ	#end loop0
	addu	a0,	a0, VIDEO_BUFFER_WIDTH * 2	#delay slot

	lw		fp, 0(sp)
	jr		ra
	addi	sp,	sp,	4

	.set	at
	.set	reorder
	.end swapBufferLQ

