* NOTE: Text Macros Can't Have Spaces in the String *

;MYKPRINTF("<Text>",num.l)
MYKPRINTF	MACRO
;	  movem.l	a0-a1/d0-d1,-(sp)
;	  move.l	\2,-(sp)
;	  pea		.Text\@
;	  bsr		_KPrintF
;	  lea		8(sp),sp
;	  movem.l	(sp)+,a0-a1/d0-d1
;	  bra	.Skip\@

;.Text\@	  dc.b	\1,10,0
;	  even
;.Skip\@
	ENDM

;DBUGPRINT1(num.l)
DBUGPRINT1	MACRO
	 IFEQ	DBUGLEVEL-1
	  movem.l	a0-a1/d0-d1,-(sp)
	  move.l	\1,d0
	  bsr		_ShellPrintNumber
	  movem.l	(sp)+,a0-a1/d0-d1
	 ENDC
	ENDM

;DBUGPRINT2(num.l)
DBUGPRINT2	MACRO
	 IFEQ	DBUGLEVEL-2
	  movem.l	a0-a1/d0-d1,-(sp)
	  move.l	\1,d0
	  bsr		_ShellPrintNumber
	  movem.l	(sp)+,a0-a1/d0-d1
	 ENDC
	DBUGPRINT1 \1
	ENDM

;DBUGNTEXTPRINT1("<Text>",num.l)
DBUGNTEXTPRINT1	MACRO
	 IFEQ	DBUGLEVEL-1
	  movem.l	a0-a1/d0-d1,-(sp)

	  movem.l	a0-a1/d0-d1,-(sp)
	  move.l	#.Text\@,a0
	  bsr		_ShellPrint
	  movem.l	(sp)+,a0-a1/d0-d1

	  move.l	\2,d0
	  bsr		_ShellPrintNumber
	  movem.l	(sp)+,a0-a1/d0-d1

	  bra	.Skip\@

.Text\@	  dc.b	\1,0
	  even
.Skip\@
	 ENDC
	ENDM

;DBUGNTEXTPRINT2("<Text>",num.l)
DBUGNTEXTPRINT2	MACRO
	 IFEQ	DBUGLEVEL-2

	  movem.l	a0-a1/d0-d1,-(sp)

	  movem.l	a0-a1/d0-d1,-(sp)
	  move.l	#.Text\@,a0
	  bsr		_ShellPrint
	  movem.l	(sp)+,a0-a1/d0-d1

	  move.l	\2,d0
	  bsr	_ShellPrintNumber
	  movem.l	(sp)+,a0-a1/d0-d1

	  bra	.Skip\@

.Text\@	  dc.b	\1,0
	  even
.Skip\@
	 ENDC
	 DBUGNTEXTPRINT1 \1,\2
	ENDM


;CALL(LibraryBase,Routine)
CALL	MACRO
	move.l	\1,a6
	jsr	_LVO\2(a6)
	ENDM

;RCOLOR	MACRO
;	IFNE	RASTERTIME
;	lea	_custom,a6
;	move.w	#$0C40,bplcon3(a6)
;	move.w	\1,color00(a6)
;	ENDC
;	ENDM

;RCOLOR(colour)
RCOLOR	MACRO
	tst.l	_RasterTime
	beq	.\@
	lea	_custom,a6
	move.w	#$0C40,bplcon3(a6)
	move.w	\1,color00(a6)
.\@
	ENDM

		;must have customchip base in a6
WAITBLIT	MACRO
	btst	#6,dmaconr(a6)
.\@
	btst	#6,dmaconr(a6)
	bne	.\@
	ENDM

BLITNASTY	MACRO
		lea	_custom,a6
		move.w	#$8400,dmacon(a6)
		ENDM

BLITNICE	MACRO
		lea	_custom,a6
		move.w	#$0400,dmacon(a6)
		ENDM

FLUSHVIEW	MACRO
.\@
	move.l	#0,a1
	CALL	_GfxBase,LoadView
	CALL	_GfxBase,WaitTOF
	CALL	_GfxBase,WaitTOF

	cmp.l	#0,gb_ActiView(a6)		;Any other view appeared?
	bne.s	.\@
	ENDM

bnesr		MACRO
		beq	.\@
		bsr	\1
.\@
		ENDM

beqsr		MACRO
		bne	.\@
		bsr	\1
.\@
		ENDM
bccsr		MACRO
		bcs	.\@
		bsr	\1
.\@
		ENDM
bcssr		MACRO
		bcc	.\@
		bsr	\1
.\@
		ENDM
bgtsr		MACRO
		ble	.\@
		bsr	\1
.\@
		ENDM
bmisr		MACRO
		bpl	.\@
		bsr	\1
.\@
		ENDM
bplsr		MACRO
		bmi	.\@
		bsr	\1
.\@
		ENDM
abs		MACRO
		tst.\0	\1
		bpl	.\@
		neg.\0	\1
.\@
		ENDM

absq		MACRO
		bpl	.\@
		neg.\0	\1
.\@
		ENDM

color00		equ	color
color01		equ	color00+2
color02		equ	color01+2
color03		equ	color02+2
color04		equ	color03+2
color05		equ	color04+2
color06		equ	color05+2
color07		equ	color06+2
color08		equ	color07+2
color09		equ	color08+2
color10		equ	color09+2
color11		equ	color10+2
color12		equ	color11+2
color13		equ	color12+2
color14		equ	color13+2
color15		equ	color14+2
color16		equ	color15+2
color17		equ	color16+2
color18		equ	color17+2
color19		equ	color18+2
color20		equ	color19+2
color21		equ	color20+2
color22		equ	color21+2
color23		equ	color22+2
color24		equ	color23+2
color25		equ	color24+2
color26		equ	color25+2
color27		equ	color26+2
color28		equ	color27+2
color29		equ	color28+2
color30		equ	color29+2
color31		equ	color30+2

bpl1pth		equ	bplpt
bpl1ptl		equ	bplpt+2
bpl2pth		equ	bplpt+4
bpl2ptl		equ	bplpt+6
bpl3pth		equ	bplpt+8
bpl3ptl		equ	bplpt+10
bpl4pth		equ	bplpt+12
bpl4ptl		equ	bplpt+14
bpl5pth		equ	bplpt+16
bpl5ptl		equ	bplpt+18
bpl6pth		equ	bplpt+20
bpl6ptl		equ	bplpt+22
bpl7pth		equ	bplpt+24
bpl7ptl		equ	bplpt+26
bpl8pth		equ	bplpt+28
bpl8ptl		equ	bplpt+30

bplcon4		equ	$10C

spr0pth		equ	sprpt
spr0ptl		equ	sprpt+02
spr1pth		equ	sprpt+04
spr1ptl		equ	sprpt+06
spr2pth		equ	sprpt+08
spr2ptl		equ	sprpt+10
spr3pth		equ	sprpt+12
spr3ptl		equ	sprpt+14
spr4pth		equ	sprpt+16
spr4ptl		equ	sprpt+18
spr5pth		equ	sprpt+20
spr5ptl		equ	sprpt+22
spr6pth		equ	sprpt+24
spr6ptl		equ	sprpt+26
spr7pth		equ	sprpt+28
spr7ptl		equ	sprpt+30

spr0data	equ	spr+sd_dataa+$00
spr1data	equ	spr+sd_dataa+$08
spr2data	equ	spr+sd_dataa+$10
spr3data	equ	spr+sd_dataa+$18
spr4data	equ	spr+sd_dataa+$20
spr5data	equ	spr+sd_dataa+$28
spr6data	equ	spr+sd_dataa+$30
spr7data	equ	spr+sd_dataa+$38

fmode		equ	$1FC

noop		equ	$1FE
