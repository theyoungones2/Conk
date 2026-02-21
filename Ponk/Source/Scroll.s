	incdir	"PConk:inc/"
	include	"source/libs.gs"
	include	"source/Header.i"
	include	"source/Constants.i"
	include	"source/Structures.i"
	include	"source/Lame.i"


	XDEF	_ScrollSlice,_SetColumnOffset,_BlockDrawSlice

;*****************************************************************************
;Scroll update

_ScrollSlice
	;Inputs:	a2.l = Slice struct

	;RCOLOR	#$F00
	;rts
	move.l	SS_Scroll(a2),a3

	tst.l	SC_Speed(a3)
	beq	.SkipChangeDirCheck
	bmi	.SpeedNeg
.SpeedPos
	tst.l	SC_OldSpeed(a3)
	bne	.NotAtStart1
	bsr	_SetColumnOffset
	bra	.SkipChangeDirCheck
.NotAtStart1
	bpl	.SkipChangeDirCheck
	bsr	ChangeDirNegToPos
	bra	.SkipChangeDirCheck
.SpeedNeg
	tst.l	SC_OldSpeed(a3)
	bne	.NotAtStart2
	bsr	_SetColumnOffset
	bra	.SkipChangeDirCheck
.NotAtStart2
	bmi	.SkipChangeDirCheck
	bsr	ChangeDirPosToNeg
.SkipChangeDirCheck
	move.w	#0,RapFlag
	bsr	DrawColumn

	move.l	#0,d1
	move.l	SC_Pos(a3),d0
	add.l	SC_Speed(a3),d0
	bmi	.KillSpeed

	move.l	SC_RightBorder(a3),d1

	cmp.l	d1,d0
	blt	.Fine
.KillSpeed
	move.l	#0,SC_Speed(a3)
	move.l	#0,SC_BaseSpeed(a3)
	move.l	d1,SC_Pos(a3)
	move.l	d1,d0
.Fine
	move.l	d0,SC_Pos(a3)

	tst.l	SC_Speed(a3)
	beq	.NoOld
	move.l	SC_Speed(a3),SC_OldSpeed(a3)
.NoOld

	tst.w	RapFlag
	beq	.NoReset

	tst.l	SC_Speed(a3)
	bmi	.NegSpeedRap
	move.w	#$0000,SC_PosBlocksDrawn(a3)
	move.w	#$0FFF,SC_NegBlocksDrawn(a3)
	move.l	#0,SC_PosColumnOffset(a3)
	move.l	#0,SC_PosBufferOffset(a3)
	bra	.SkipNegRap
.NegSpeedRap
	move.w	#$0FFF,SC_PosBlocksDrawn(a3)
	move.w	#$0000,SC_NegBlocksDrawn(a3)
	move.l	#0,SC_NegColumnOffset(a3)
	move.l	#0,SC_NegBufferOffset(a3)
.SkipNegRap
	move.w	#0,SC_BlocksDrawn(a3)

	move.l	#0,SC_CurrentMapOffset(a3)
	move.l	#0,SC_CurrentBufferOffset(a3)

	bsr	_SetColumnOffset
.NoReset

	bsr	_UpdateAnimaniacs

.Exit
	;RCOLOR	#$00F
	rts

ChangeDirPosToNeg
	;Inputs:	a2.l = Slice
	;		a3.l = Scroll
	move.w	SC_BlocksDrawn(a3),SC_PosBlocksDrawn(a3)
	move.l	SC_CurrentMapOffset(a3),SC_PosColumnOffset(a3)
	move.l	SC_CurrentBufferOffset(a3),SC_PosBufferOffset(a3)

	move.w	SC_NegBlocksDrawn(a3),SC_BlocksDrawn(a3)
	move.l	SC_NegColumnOffset(a3),SC_CurrentMapOffset(a3)
	move.l	SC_NegBufferOffset(a3),SC_CurrentBufferOffset(a3)

	;RCOLOR	#$FFF
	;bsr	_WaitVBlank
	;bsr	_WaitPastVBlank
.Exit
	rts

ChangeDirNegToPos
	;Inputs:	a2.l = Slice


	move.w	SC_BlocksDrawn(a3),SC_NegBlocksDrawn(a3)
	move.l	SC_CurrentMapOffset(a3),SC_NegColumnOffset(a3)
	move.l	SC_CurrentBufferOffset(a3),SC_NegBufferOffset(a3)

	move.w	SC_PosBlocksDrawn(a3),SC_BlocksDrawn(a3)
	move.l	SC_PosColumnOffset(a3),SC_CurrentMapOffset(a3)
	move.l	SC_PosBufferOffset(a3),SC_CurrentBufferOffset(a3)

	;RCOLOR	#$FFF
	;bsr	_WaitVBlank
	;bsr	_WaitPastVBlank

.Exit
	rts


_SetColumnOffset
	;Inputs:	a2.l = Slice
	move.l	SS_Scroll(a2),a3

	move.l	SC_Pos(a3),d0
	lsr.l	#BITSHIFT+3,d0
	and.l	#$FFFFFFFE,d0

	move.l	SS_BufferWidth(a2),d1
	sub.l	#6,d1
	tst.l	SC_Speed(a3)
	beq	.Exit
	bpl	.Positive
	move.l	#-4,d1
.Positive
	add.l	d1,d0
	bpl	.Set
	move.l	#0,d0
.Set
	move.l	d0,SC_MapColumnOffset(a3)
	move.l	d0,SC_LogColumnOffset(a3)

.Exit
	rts


_UpdateAnimaniacs
	rts

DrawColumn
	;Updates drawing a column of blocks
	;Inputs:	a2.l = Slice

	move.l	SS_Scroll(a2),a3		;Get Scroll structure
	cmp.l	#0,a3
	beq	.Exit

	move.l	SC_LogPosToDraw(a3),a1
	move.l	SC_LogBlocksToDraw(a3),a0
	move.l	SS_BufferWidth(a2),d2
.Loop1
	move.l	(a0),a4				;Get next block address and column offset pos
	move.l	(a1),d0
	beq	.UpToDate

	move.l	#0,(a1)+			;Clear out the table as we draw
	move.l	#0,(a0)+
	move.l	SS_LogBase(a2),a5
	add.l	d0,a5				;Add screen offset to screen base

	move.w	SS_Planes(a2),d1
	sub.w	#1,d1
.Loop2
	move.w	(a4)+,(a5)			;Draw line. Blocks 16 lines high
	add.l	d2,a5
	move.w	(a4)+,(a5)
	add.l	d2,a5
	move.w	(a4)+,(a5)
	add.l	d2,a5
	move.w	(a4)+,(a5)
	add.l	d2,a5
	move.w	(a4)+,(a5)
	add.l	d2,a5
	move.w	(a4)+,(a5)
	add.l	d2,a5
	move.w	(a4)+,(a5)
	add.l	d2,a5
	move.w	(a4)+,(a5)
	add.l	d2,a5
	move.w	(a4)+,(a5)
	add.l	d2,a5
	move.w	(a4)+,(a5)
	add.l	d2,a5
	move.w	(a4)+,(a5)
	add.l	d2,a5
	move.w	(a4)+,(a5)
	add.l	d2,a5
	move.w	(a4)+,(a5)
	add.l	d2,a5
	move.w	(a4)+,(a5)
	add.l	d2,a5
	move.w	(a4)+,(a5)
	add.l	d2,a5
	move.w	(a4)+,(a5)
	add.l	d2,a5

	dbra	d1,.Loop2
	bra	.Loop1
.UpToDate

	move.l	SC_Pos(a3),d0
	move.l	SC_Speed(a3),d2
	add.l	d0,d2

	cmp.l	d2,d0
	beq	.Exit
	blt	.Pos
.Neg		
	lsr.l	#BITSHIFT-1,d2
	lsr.l	#BITSHIFT-1,d0			;Shift and and out, leaving pos between 0-15 (x2)
	and.l	#$1E,d2
	and.l	#$1E,d0

	eor.l	#$1E,d2
	eor.l	#$1E,d0
	bra	.Comp
.Pos
	lsr.l	#BITSHIFT-1,d2
	lsr.l	#BITSHIFT-1,d0			;Shift and and out, leaving pos between 0-15 (x2)
	and.w	#$1E,d2
	and.w	#$1E,d0
.Comp
	cmp.w	d2,d0
	blt	.NoRap				;If register without Speed added is less, we have not rapped
.Rap
	move.w	#$1E,d2
	move.w	#1,RapFlag

.NoRap
	lea	SC_NeededDrawnTable(a3),a0
	move.w	(a0,d2.w),d2			;Check how many blocks have to be drawn by next cycle
	sub.w	SC_BlocksDrawn(a3),d2
	sub.w	#1,d2
	bmi	.Exit				;Exit if we are up to date

	move.l	SC_CurrentMapOffset(a3),d1
	move.l	SC_CurrentBufferOffset(a3),d0
	move.l	SC_LogPosToDraw(a3),a4		;Pre-load registers 
	move.l	SC_LogBlocksToDraw(a3),a5
.Loop3
	move.l	SS_LogBase(a2),a0
	move.l	SC_LogColumnOffset(a3),d4
	add.l	d4,a0
	add.l	d0,a0				;Work out place to draw block
	add.l	d0,d4
	move.l	d4,(a4)

	move.l	SC_MapBank(a3),a1
	add.l	SC_MapColumnOffset(a3),a1
	add.l	d1,a1				;Work out map offset
	move.w	(a1),d3
	mulu	SC_BlockSize(a3),d3
	move.l	SC_BlockBank(a3),a1
	add.l	d3,a1
	move.l	a1,(a5)

	move.w	SC_MapBlocksWide(a3),d4
	and.l	#$FFFF,d4
	lsl.l	d4
	add.l	d4,d1
	move.l	d1,SC_CurrentMapOffset(a3)	;Set current offsets for next block
	add.l	SS_BufferWidthx16xPlanes(a2),d0
	move.l	d0,SC_CurrentBufferOffset(a3)

	move.w	SS_Planes(a2),d4
	sub.w	#1,d4
	move.l	SS_BufferWidth(a2),d5
.Loop4
	move.w	(a1)+,(a0)
	add.l	d5,a0
	move.w	(a1)+,(a0)
	add.l	d5,a0
	move.w	(a1)+,(a0)
	add.l	d5,a0
	move.w	(a1)+,(a0)
	add.l	d5,a0
	move.w	(a1)+,(a0)
	add.l	d5,a0
	move.w	(a1)+,(a0)
	add.l	d5,a0
	move.w	(a1)+,(a0)
	add.l	d5,a0
	move.w	(a1)+,(a0)
	add.l	d5,a0
	move.w	(a1)+,(a0)
	add.l	d5,a0
	move.w	(a1)+,(a0)
	add.l	d5,a0
	move.w	(a1)+,(a0)
	add.l	d5,a0
	move.w	(a1)+,(a0)
	add.l	d5,a0
	move.w	(a1)+,(a0)
	add.l	d5,a0
	move.w	(a1)+,(a0)
	add.l	d5,a0
	move.w	(a1)+,(a0)
	add.l	d5,a0
	move.w	(a1)+,(a0)
	add.l	d5,a0

	dbra	d4,.Loop4

	move.l	SS_TriBase(a2),a0
	add.l	(a4)+,a0
	move.l	(a5)+,a1
	cmp.l	#0,SS_TriBase(a2)
	beq	.NoTripple

	move.w	SS_Planes(a2),d4
	sub.w	#1,d4
.Loop5
	move.w	(a1)+,(a0)
	add.l	d5,a0
	move.w	(a1)+,(a0)
	add.l	d5,a0
	move.w	(a1)+,(a0)
	add.l	d5,a0
	move.w	(a1)+,(a0)
	add.l	d5,a0
	move.w	(a1)+,(a0)
	add.l	d5,a0
	move.w	(a1)+,(a0)
	add.l	d5,a0
	move.w	(a1)+,(a0)
	add.l	d5,a0
	move.w	(a1)+,(a0)
	add.l	d5,a0
	move.w	(a1)+,(a0)
	add.l	d5,a0
	move.w	(a1)+,(a0)
	add.l	d5,a0
	move.w	(a1)+,(a0)
	add.l	d5,a0
	move.w	(a1)+,(a0)
	add.l	d5,a0
	move.w	(a1)+,(a0)
	add.l	d5,a0
	move.w	(a1)+,(a0)
	add.l	d5,a0
	move.w	(a1)+,(a0)
	add.l	d5,a0
	move.w	(a1)+,(a0)
	add.l	d5,a0

	dbra	d4,.Loop5
.NoTripple
	add.w	#1,SC_BlocksDrawn(a3)
	dbra	d2,.Loop3
.Exit
	rts

_BlockDrawSlice
	;Draws up an entire slice's blocks.
	;Inputs:	a2.l = Slice struct
	;		a3.l = Destination Buffer

	move.l	SS_Scroll(a2),a4

	move.l	SC_Pos(a4),d2
	lsr.l	#BITSHIFT+3,d2
	and.l	#$FFFFFFFE,d2
	beq	.AtStartOfMap
	sub.l	#2,a3
.AtStartOfMap
	move.l	SC_MapBank(a4),a5	;Get correct place in map data
	add.l	d2,a5
	add.l	d2,a3			;Get correct offset into buffer
	move.l	SC_BlockBank(a4),a6

	move.l	#0,d4
.Loop2
	move.l	#0,d5
.Loop1
	move.l	d5,d0
	lsl.w	d0
	move.l	d4,d1			;\
	lsl.l	d1
	mulu	SC_MapBlocksWide(a4),d1	; >Find Block number
	add.l	d1,d0			;/

	move.w	(a5,d0.l),d0		;Find block's offset into block data
	mulu	SC_BlockSize(a4),d0
	move.l	a6,a1
	add.l	d0,a1

	move.l	a3,a0

	move.w	SS_Planes(a2),d1
	sub.w	#1,d1
	move.l	SS_BufferWidth(a2),d0
.DrawLoop
	move.w	(a1)+,(a0)
	add.l	d0,a0
	move.w	(a1)+,(a0)
	add.l	d0,a0
	move.w	(a1)+,(a0)
	add.l	d0,a0
	move.w	(a1)+,(a0)
	add.l	d0,a0
	move.w	(a1)+,(a0)
	add.l	d0,a0
	move.w	(a1)+,(a0)
	add.l	d0,a0
	move.w	(a1)+,(a0)
	add.l	d0,a0
	move.w	(a1)+,(a0)
	add.l	d0,a0
	move.w	(a1)+,(a0)
	add.l	d0,a0
	move.w	(a1)+,(a0)
	add.l	d0,a0
	move.w	(a1)+,(a0)
	add.l	d0,a0
	move.w	(a1)+,(a0)
	add.l	d0,a0
	move.w	(a1)+,(a0)
	add.l	d0,a0
	move.w	(a1)+,(a0)
	add.l	d0,a0
	move.w	(a1)+,(a0)
	add.l	d0,a0
	move.w	(a1)+,(a0)
	add.l	d0,a0

	dbra	d1,.DrawLoop

	add.w	#1,d5
	add.l	#2,a3			;Add to registers to point to next block across
	cmp.w	SC_BufferBlocksWide(a4),d5
	bne	.Loop1
	add.l	SC_BlockSizeXBufferWidth(a4),a3
	sub.l	SS_BufferWidth(a2),a3
	add.w	#1,d4			;Add to registers to point to next line down
	cmp.w	SC_BufferBlocksHigh(a4),d4
	bne	.Loop2

	rts	

;******************************************************************************************************
	SECTION	Vars,BSS

RapFlag		ds.w	1
