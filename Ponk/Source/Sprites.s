*********************************************************************************************************
;Sprite stuff

	incdir	"PConk:inc/"
	include	"source/libs.gs"
	include	"source/Header.i"
	include	"source/Constants.i"
	include	"source/Structures.i"
	include	"source/Lame.i"


;******************************************************************************************************

	XDEF	_UpdateSprites,_PutSprite,_BlankSprite,_NullSprites
	XDEF	_SpriteXPosList,_SpriteYPosList,_SpritePosToggle,_SpriteCopperToggle,_DisplaySprites

	XREF	_SpriteObjects
	XREF	_SysBase,_custom
	XREF	_GfxBase,_DOSBase,_ChipMemBase,_OurTaskBase
	XREF	_ciaa,_LoadWholeFile,_ShellPrintNumber,_ShellPrint
	XREF	_WaitVBlank,_WaitPastVBlank,_JoystickDecode,_WaitBlit,_Random
	XREF	_OpenNewDebugWindow,_FreeDebugWindow,_BlitDebugWindow,_WipeDebugWindow	;,_MoveDebugWindow
	XREF	_MainMenu,_NewMenu,_CurrentMenu,_UpdateMenu
	XREF	_EXECUTE,_SETC,_KILL,_FIREB,_HURTDUDE
	XREF	_InitLinearAccel,_UpdateYPushSliceTrack,_UpdateXPushSliceTrack
	XREF	_Spam_Die,_Spam_Init,_Spam_PlaySound
	XREF	_DoActionList,_NullSprite
	XREF	_FirstSlice,_KPrintF,_GameOn

;********************************************************************************************************

	IFNE	CHIPASSEMBLE
	SECTION	WhatLovelyCode,CODE_C
	ELSE
	SECTION	WhatLovelyCode,CODE
	ENDC

;********************************************************************************************************

_UpdateSprites
	;Inputs:	a2.l = Slice

	move.w	#0,NumSprites
	eor.w	#1,_SpritePosToggle
	bsr	_NullSprites

	move.l	SS_Players(a2),a3
	cmp.l	#0,a3
	beq	.NoPlayers
	bsr	Update
	tst.w	_GameOn
	beq	.Exit
.NoPlayers
	;move.l	SS_PlayerBullets(a2),a3
	;cmp.l	#0,a3
	;beq	.NoPlayerBullets
	;bsr	Update
	;tst.w	_GameOn
	;beq	.Exit
.NoPlayerBullets
	move.l	SS_BadDudes(a2),a3
	cmp.l	#0,a3
	beq	.NoBadDudes
	bsr	Update
	tst.w	_GameOn
	beq	.Exit
.NoBadDudes
.Exit
	;move.w	NumSprites,d0
	;sub.w	#2,d0
	;bmi	.Exit2

	;move.w	NumSprites,d0
	;and.l	#$FFFF,d0
	;MYKPRINTF "Sprites:%ld",d0
.Exit2
	rts

Update
.Loop
	tst.w	OB_State(a3)
	beq	.GetNext

	move.l	OB_Image(a3),a0
	cmp.l	#0,a0
	beq	.GetNext
	cmp.w	#IM_TYPE_BOB,IM_Type(a0)
	beq	.GetNext
.OnNGoing
	add.w	#1,NumSprites
	cmp.w	#OB_UPDATETYPE_VB,OB_UpdateType(a3)
	bne	.SetSeudo			;If mainloop sprite we always set
	tst.w	_SpritePosToggle
	beq	.AddHalf
.SetSeudo
	move.l	OB_XPos(a3),SPR_SeudoX(a3)
	and.l	#$FFFFFFFE0,SPR_SeudoX(a3)
	move.l	SS_Scroll(a2),a0
	move.l	SC_Pos(a0),d0
	sub.l	OB_AnimXOffset(a3),d0
	sub.l	d0,SPR_SeudoX(a3)
	move.l	OB_YPos(a3),SPR_SeudoY(a3)
	move.l	OB_AnimYOffset(a3),SPR_SeudoAnimY(a3)
	move.l	OB_Image(a3),SPR_SeudoImage(a3)
	bra	.DisplaySprite
.AddHalf
	move.l	BB_XSpeed(a3),d0
	asr.l	d0
	add.l	d0,SPR_SeudoX(a3)
	move.l	BB_YSpeed(a3),d0
	asr.l	d0
	add.l	d0,SPR_SeudoY(a3)

.DisplaySprite
	move.l	SPR_SeudoImage(a3),a4
	bsr	PutVBlankSprite

.GetNext
	tst.w	_GameOn
	beq	.Exit
	move.l	OB_Next(a3),a3
	cmp.l	#0,a3
	bne	.Loop
.Exit
	rts

_DisplaySprites
	move.l	SS_CCSprites(a2),a0
	cmp.l	#0,a0
	beq	.Exit

	lea	_SpriteCopperToggleTable,a1
	;move.w	#1,d1
	move.w	_SpriteCopperToggle,d1
	lsl.w	#2,d1
	move.l	(a1,d1.w),a1

	move.l	(a1)+,(a0)+
	move.l	(a1)+,(a0)+

	move.l	(a1)+,(a0)+
	move.l	(a1)+,(a0)+

	move.l	(a1)+,(a0)+
	move.l	(a1)+,(a0)+

	move.l	(a1)+,(a0)+
	move.l	(a1)+,(a0)+

	move.l	(a1)+,(a0)+
	move.l	(a1)+,(a0)+

	move.l	(a1)+,(a0)+
	move.l	(a1)+,(a0)+

	move.l	(a1)+,(a0)+
	move.l	(a1)+,(a0)+
.Exit
	rts

PutVBlankSprite
	;Inputs:	a2.l = Slice
	;		a3.l = Object

	movem.l	d4-d6/a4-a5,-(sp)

	bclr.b	#2,BB_Flags+3(a3)

	cmp.l	#0,a4
	beq	.Exit

	lea	SpriteCopperTable,a1
	move.w	SP_StartSprite(a4),d0
	lsl.w	d0
	move.w	(a1,d0.w),d0		;Get offset into copper of start sprite
	and.l	#$FFF,d0

	lea	_SpriteCopperToggleTable,a1
	;move.w	#1,d1
	move.w	_SpriteCopperToggle,d1
	lsl.w	#2,d1
	move.l	(a1,d1.w),a1

	add.l	d0,a1

	move.l	SP_Image(a4),d3

	move.w	SP_Number(a4),d4
	cmp.w	#8,d4
	ble	.NumOfSpritesOK
	move.w	#8,d4
.NumOfSpritesOK
	sub.w	#1,d4

	move.l	SPR_SeudoX(a3),d5
	;add.l	OB_AnimXOffset(a3),d5
	move.l	SS_Scroll(a2),a0
	;sub.l	SC_Pos(a0),d5
	lsr.l	#BITSHIFT,d5
	add.l	#$81,d5			;d5 = X Pos

	move.l	SPR_SeudoY(a3),d6
	add.l	SPR_SeudoAnimY(a3),d6
	move.l	SS_Scroll(a2),a0
	sub.l	SC_YPos(a0),d6		;d6 = Y Pos
	lsr.l	#BITSHIFT,d6
	add.l	#$2C,d6
.Loop
	tst.w	SP_Flags(a4)
	beq	.NotAttached
.Attached
	tst.w	d5
	bmi	.Cludge
	move.l	SS_ScreenWidth(a2),d0
	lsr.l	#BITSHIFT,d0
	add.w	#$81,d0
	sub.w	d5,d0
	bpl	.SetAPutSprite
.Cludge
	add.l	SP_Size(a4),d3
	sub.w	#1,d4
	bra	.GetNext1

.SetAPutSprite
	move.w	d5,d0
	move.w	d6,d1
	bmi	.Exit
	move.w	SP_Height(a4),d2	;Set up registers for put sprite
	move.l	d3,a0
	move.l	a1,-(sp)
	move.l	d3,a1
	add.l	SP_Size(a4),a1
	bsr	PutSprite16Colour	;Non AGA
	move.l	(sp)+,a1

	bset.b	#2,BB_Flags+3(a3)

	move.w	d3,CC_spr0ptl(a1)	;Point copper to image
	swap	d3
	move.w	d3,CC_spr0pth(a1)
	swap	d3

	add.l	SP_Size(a4),d3
	sub.w	#1,d4

	move.w	d3,CC_spr1ptl(a1)	;Point copper to image
	swap	d3
	move.w	d3,CC_spr1pth(a1)
	swap	d3
.GetNext1
	add.l	#CC_SizeOf*2,a1
	add.l	SP_Size(a4),d3
	add.w	SP_Width(a4),d5

	dbra	d4,.Attached
	move.l	a4,OB_PrevImage(a3)
	bra	.Exit
.NotAttached
	tst.w	d5
	bmi	.GetNext2
	move.l	SS_ScreenWidth(a2),d0
	lsr.l	#BITSHIFT,d0
	add.w	#$81,d0
	sub.w	d5,d0
	bmi	.GetNext2

	move.w	d5,d0
	move.w	d6,d1
	move.w	SP_Height(a4),d2	;Set up registers for put sprite
	move.l	d3,a0
	bsr	PutSprite4Colour	;Non AGA

	bset.b	#2,BB_Flags+3(a3)

	move.w	d3,CC_spr0ptl(a1)	;Point copper to image
	swap	d3
	move.w	d3,CC_spr0pth(a1)
	swap	d3
.GetNext2
	add.l	#CC_SizeOf,a1
	add.l	SP_Size(a4),d3
	add.w	SP_Width(a4),d5
	dbra	d4,.NotAttached
	move.l	a4,OB_PrevImage(a3)
.Exit
	movem.l	(sp)+,d4-d6/a4-a5
	rts

_PutSprite
	;Inputs:	a2.l = Slice
	;		a3.l = Object

	movem.l	d6/a4-a5,-(sp)

	move.l	OB_Image(a3),a4
	cmp.l	#0,a4
	beq	.Exit

	lea	SpriteCopperTable,a1
	move.w	SP_StartSprite(a4),d0
	lsl.w	d0
	move.w	(a1,d0.w),d0		;Get offset into copper of start sprite
	and.l	#$FFF,d0

	lea	_SpriteCopperToggleTable,a1
	move.w	_SpriteCopperToggle,d1
	lsl.w	#2,d1
	move.l	(a1,d1.w),a1

	add.l	d0,a1

	move.l	SP_Image(a4),d3

	move.w	SP_Number(a4),d4
	cmp.w	#8,d4
	ble	.NumOfSpritesOK
	move.w	#8,d4
.NumOfSpritesOK
	sub.w	#1,d4

	move.l	OB_XPos(a3),d5
	add.l	OB_AnimXOffset(a3),d5
	move.l	SS_Scroll(a2),a0
	sub.l	SC_Pos(a0),d5
	lsr.l	#BITSHIFT,d5
	add.l	#$81,d5			;d5 = X Pos

	move.l	OB_YPos(a3),d6
	add.l	OB_AnimYOffset(a3),d6
	sub.l	SC_YPos(a0),d6		;d6 = Y Pos
	lsr.l	#BITSHIFT,d6
	add.l	#$2C,d6
.Loop
	tst.w	SP_Flags(a4)
	beq	.NotAttached
.Attached
	tst.w	d5
	bmi	.Cludge
	move.l	SS_ScreenWidth(a2),d0
	lsr.l	#BITSHIFT,d0
	add.w	#$81,d0
	sub.w	d5,d0
	bpl	.SetAPutSprite
.Cludge
	add.l	SP_Size(a4),d3
	sub.w	#1,d4
	bra	.GetNext1

.SetAPutSprite
	move.w	d5,d0
	move.w	d6,d1
	move.w	SP_Height(a4),d2	;Set up registers for put sprite
	move.l	d3,a0
	move.l	a1,-(sp)
	move.l	d3,a1
	add.l	SP_Size(a4),a1
	bsr	PutSprite16Colour	;Non AGA
	move.l	(sp)+,a1

	move.w	d3,CC_spr0ptl(a1)	;Point copper to image
	swap	d3
	move.w	d3,CC_spr0pth(a1)
	swap	d3

	add.l	SP_Size(a4),d3
	sub.w	#1,d4

	move.w	d3,CC_spr1ptl(a1)	;Point copper to image
	swap	d3
	move.w	d3,CC_spr1pth(a1)
	swap	d3
.GetNext1
	add.l	#CC_SizeOf*2,a1
	add.l	SP_Size(a4),d3
	add.w	SP_Width(a4),d5

	dbra	d4,.Attached
	move.l	a4,OB_PrevImage(a3)
	bra	.Exit
.NotAttached
	tst.w	d5
	bmi	.GetNext2
	move.l	SS_ScreenWidth(a2),d0
	lsr.l	#BITSHIFT,d0
	add.w	#$81,d0
	sub.w	d5,d0
	bmi	.GetNext2

	move.w	d5,d0
	move.w	d6,d1
	move.w	SP_Height(a4),d2	;Set up registers for put sprite
	move.l	d3,a0
	bsr	PutSprite4Colour	;Non AGA

	move.w	d3,CC_spr0ptl(a1)	;Point copper to image
	swap	d3
	move.w	d3,CC_spr0pth(a1)
	swap	d3
.GetNext2
	add.l	#CC_SizeOf,a1
	add.l	SP_Size(a4),d3
	add.w	SP_Width(a4),d5
	dbra	d4,.NotAttached
	move.l	a4,OB_PrevImage(a3)
.Exit
	movem.l	(sp)+,d6/a4-a5
	rts

_BlankSprite
	;Inputs:	a0.l = Image
	;		a2.l = Slice
	;		a3.l = Object


	move.l	a4,-(sp)

	;MYKPRINTF "BlankSprite",#0

	move.l	a0,a4
	cmp.l	#0,a4
	beq	.Exit

	lea	SpriteCopperTable,a1
	move.w	SP_StartSprite(a4),d0
	lsl.w	d0
	move.w	(a1,d0.w),d0		;Get offset into copper of start sprite
	and.l	#$FFF,d0

	lea	_SpriteCopperToggleTable,a1
	move.l	(a1),a1
	add.l	d0,a1
	bsr	Blank

	lea	_SpriteCopperToggleTable,a1
	move.l	4(a1),a1
	add.l	d0,a1
	bsr	Blank

.Exit
	move.l	#0,OB_PrevImage(a3)
	move.l	(sp)+,a4	
	rts
Blank
	move.l	#_NullSprite,d3

	move.w	SP_Number(a4),d4
	sub.w	#1,d4
.Loop
	move.w	d3,CC_spr0pth(a1)	;Point copper to image
	swap	d3
	move.w	d3,CC_spr0ptl(a1)

	add.l	#CC_SizeOf,a1
	dbra	d4,.Loop
	rts


_NullSprites	
	move.l	_FirstSlice,a0

	lea	_SpriteCopperToggleTable,a1
	;move.w	#1,d1
	move.w	_SpriteCopperToggle,d1
	lsl.w	#2,d1
	move.l	(a1,d1.w),a1
	move.l	#_NullSprite,d1

	move.w	#6,d0
.Loop
	move.w	d1,CC_spr0ptl(a1)	;Point copper to image
	swap	d1
	move.w	d1,CC_spr0pth(a1)
	swap	d1

	add.l	#CC_SizeOf,a1
	dbra	d0,.Loop
.Exit
	rts

PutSprite4Colour	;Place a 4 Colour Sprite
	;Inputs:	d0.w = XPos (Pixels)
	;		d1.w = YPos (Pixels)
	;		d2.w = Height of sprite
	;		a0.l = Sprite Base Address
	;Outputs:	Nil

	move.w	d3,-(sp)
	move.w	d2,-(sp)
	move.w	#0,d3
	move.w	d1,d2		;Copy VSTART
	lsl.w	#8,d2		;Rotate VSTART High Bit
	roxl.w	#1,d3		;Into SPR2CTL
	move.w	d1,d2		;Copy VSTART Again
	add.w	(sp),d2		;Add Height to VSTART to Make VSTOP
	lsl.w	#8,d2		;Rotate VSTOP high bit 
	roxl.w	#1,d3		;Into SPR2CTL
	lsr.w	#1,d0
	roxl.w	#1,d3
	move.w	d1,d2
	lsl.w	#8,d2
	or.w	d0,d2
	add.w	(sp),d1		;Add height
	lsl.w	#8,d1
	or.w	d1,d3
	move.w	d2,(a0)
	move.w	d3,2(a0)
	move.w	(sp)+,d2
	move.w	(sp)+,d3
	rts

PutSprite16Colour	;Place a 16 Colour Sprite
	;Inputs:	d0.w = XPos (Pixels)
	;		d1.w = YPos (Pixels)
	;		d2.w = Height of sprite
	;		a0.l = Even Sprite Base Address
	;		a1.l = Odd Sprite Base Address
	;Outputs:	Nil

	move.w	d3,-(sp)
	move.w	d2,-(sp)
	move.w	#0,d3
	move.w	d1,d2		;Copy VSTART
	lsl.w	#8,d2		;Rotate VSTART High Bit
	roxl.w	#1,d3		;Into SPR2CTL
	move.w	d1,d2		;Copy VSTART Again
	add.w	(sp),d2		;Add Height to VSTART to Make VSTOP
	lsl.w	#8,d2		;Rotate VSTOP high bit 
	roxl.w	#1,d3		;Into SPR2CTL
	lsr.w	#1,d0		;Shift Out HSTART Low Bit
	roxl.w	#1,d3		;Into SPR2CTL
	move.w	d1,d2		;Copy VSTART once again.
	lsl.w	#8,d2		;Shift X Pos into Upper 8 bits (VSTART)
	or.w	d0,d2		;Or in Y Pos (HSTART)
	add.w	(sp),d1		;Add Height to Y Pos To Make VSTOP
	lsl.w	#8,d1		;Move VSTOP into Upper 8 bits
	or.w	d1,d3		;Or VSTOP into SPR2CTL
	or.w	#$80,d3		;Or in Attached Bit
	move.w	d2,(a0)
	move.w	d2,(a1)
	move.w	d3,2(a0)
	move.w	d3,2(a1)
	move.w	(sp)+,d2
	move.w	(sp)+,d3
	rts



;********************************************************************************************************

	SECTION Data,data

NumSprites
	dc.w	0
TempBaseSpeed
	dc.l	0

SpriteCopperTable
	dc.w	0*CC_SizeOf,1*CC_SizeOf,2*CC_SizeOf,3*CC_SizeOf
	dc.w	4*CC_SizeOf,5*CC_SizeOf,6*CC_SizeOf,7*CC_SizeOf

_SpriteXPosList
	dc.l	0,0,0,0,0,0,0,0
_SpriteYPosList
	dc.l	0,0,0,0,0,0,0,0
_SpritePosToggle
	dc.w	0
_SpriteCopperToggle
	dc.w	0
_SpriteCopperToggleTable
	dc.l	_SeudoSpriteCopper1,_SeudoSpriteCopper2


_SeudoSpriteCopper1
	dc.w	spr0pth
	dc.w	0
	dc.w	spr0ptl
	dc.w	0
	dc.w	spr1pth
	dc.w	0
	dc.w	spr1ptl
	dc.w	0
	dc.w	spr2pth
	dc.w	0
	dc.w	spr2ptl
	dc.w	0
	dc.w	spr3pth
	dc.w	0
	dc.w	spr3ptl
	dc.w	0
	dc.w	spr4pth
	dc.w	0
	dc.w	spr4ptl
	dc.w	0
	dc.w	spr5pth
	dc.w	0
	dc.w	spr5ptl
	dc.w	0
	dc.w	spr6pth
	dc.w	0
	dc.w	spr6ptl
	dc.w	0
	dc.w	spr7pth
	dc.w	0
	dc.w	spr7ptl
	dc.w	0

_SeudoSpriteCopper2
	dc.w	spr0pth
	dc.w	0
	dc.w	spr0ptl
	dc.w	0
	dc.w	spr1pth
	dc.w	0
	dc.w	spr1ptl
	dc.w	0
	dc.w	spr2pth
	dc.w	0
	dc.w	spr2ptl
	dc.w	0
	dc.w	spr3pth
	dc.w	0
	dc.w	spr3ptl
	dc.w	0
	dc.w	spr4pth
	dc.w	0
	dc.w	spr4ptl
	dc.w	0
	dc.w	spr5pth
	dc.w	0
	dc.w	spr5ptl
	dc.w	0
	dc.w	spr6pth
	dc.w	0
	dc.w	spr6ptl
	dc.w	0
	dc.w	spr7pth
	dc.w	0
	dc.w	spr7ptl
	dc.w	0

;********************************************************************************************************

	SECTION	ChipData,DATA_C 

