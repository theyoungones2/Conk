
	incdir	"PConk:inc/"
	include	"source/libs.gs"
	include	"source/Header.i"
	include	"source/Constants.i"
	include	"source/Structures.i"
	include	"source/Lame.i"


	XREF	_DoActionList,_BulletImage,_FindFreeDude
	XREF	_WaitVBlank,_WaitPastVBlank,_ShellPrintNumber
;	XREF	_InputChannels
	XREF	_FirstSlice,_ScreenSwap,_Flags
	XREF	_JoyDirTable,_SetSpeedsFromDirection,_custom
	XREF	_PointToBackground
	XREF	_SpriteXPosList,_SpriteYPosList
	XREF	_KPrintF,_SpritePosToggle,_Framerate
	XREF	_GetVariableTable,_SetVariableTable,_AddVariableTable
	XREF	_StatBarImages,_StatBarHeight
	XREF	_FindFreeBulletDude,_NotINITBOBAlthoughFairlySimilar
	;from Action.s
	XREF	_ReadVirtualJoystick
	XREF	_RawMouseX,_RawMouseY
	XREF	_VirtualMice
	;from BensActionRoutines.s
	XREF	_FindPlayer,_FindNearestPlayer

	XDEF	_InternalFireWeaponBay,_InternalFireMultiWeaponBays
	XDEF	_SpeedDependantAnimUpdate,_SpeedDependantAnimInit
	XDEF	_InitXPushSliceTrack,_InitYPushSliceTrack,_UpdateXPushSliceTrack,_UpdateYPushSliceTrack
	XDEF	_InitOperationWolfMovement,_OldInitActionListPlayer,_InitSteroids
	XDEF	_InitHomerBadDude1,_InitDirectionalHomer,_InitJoystickAnim
	XDEF	_InitFollowPath,_InitTimeDelayExecute,_InitAttachToParent
	XDEF	_InitLinearAccel,_UpdateLinearAccel
	XDEF	_InitCyclicAnimation,_InitNULL,_SimplePlatformPlayer
	XDEF	_InertialJoyMovement,_InitInertialJoyMovement
	XDEF	_OldInitDir8Anim,_OldInitDir16Anim,_OldInitDir32Anim,_OldInitDir64Anim
	XDEF	_OldInitDir8Image,_OldInitDir16Image,_OldInitDir32Image,_OldInitDir64Image
	XDEF	_InitDir8Image,_InitDir16Image,_InitDir32Image,_InitDir64Image
	XDEF	_InitDir8Action,_InitDir16Action,_InitDir32Action,_InitDir64Action
	XDEF	_InitAttatch,_InitActionOnFire
	XDEF	_InitDrawBar,_InitFloorSentry,_InitCeilingSentry
	XDEF	_InitLinearPlayer
	XDEF	_ScrollModeConstantXSpeed
	XDEF	_ScrollXCenterPlayers,_ScrollYCenterPlayers
	XDEF	_InitDir8Anim,_InitDir16Anim,_InitDir32Anim,_InitDir64Anim
	XDEF	_FindNearestObject


;************************************************************
;Action routines
;	Inputs:	a2.l = Slice
;		a5.l = Action list

;************************************************************

;********************************************************************************************************

	IFNE	CHIPASSEMBLE
	SECTION	WhatLovelyCode,CODE_C
	ELSE
	SECTION	WhatLovelyCode,CODE
	ENDC

;********************************************************************************************************


_InitAttatch
	;SYNTAX:	Player.l, XOffset.l, YOffset.l

	move.l	(a5)+,a0
	bset.b	#3,BB_Flags+3(a0)

	move.l	a0,CL_Data(a1)
	move.l	(a5)+,CL_Data+4(a1)
	move.l	(a5)+,CL_Data+8(a1)
	move.l	#UpdateAttatch,CL_UpdateCode(a1)
	rts

UpdateAttatch
	move.l	CL_Data(a6),a0
	cmp.l	#0,a0
	beq	.Die

	cmp.w	#OB_STATE_ON,OB_State(a0)
	bne	.Die

	btst.b	#3,BB_Flags+3(a0)
	bne	.Fine
.Die
	move.w	#1,d7
	bra	.Exit
.Fine
	move.l	OB_XPos(a0),d0
	add.l	CL_Data+4(a6),d0
	move.l	d0,OB_XPos(a3)

	move.l	OB_YPos(a0),d0
	add.l	CL_Data+8(a6),d0
	move.l	d0,OB_YPos(a3)
.Exit
	rts

;************************************************************

_InitAttachToParent
	;SYNTAX:	 XOffset.l, YOffset.l
	cmp.w	#PARENT_OBJECT,OB_ParentType(a3)
	bne	.Exit
	move.l	OB_Parent(a3),a0
	cmp.l	#0,a0
	beq	.Exit

	bset.b	#3,BB_Flags+3(a0)
	move.l	a0,CL_Data(a1)
	move.l	(a5),CL_Data+4(a1)
	move.l	4(a5),CL_Data+8(a1)
	move.l	#UpdateAttatch,CL_UpdateCode(a1)
.Exit
	lea	8(a5),a5
	rts


;************************************************************

_InitTimeDelayExecute
	;SYNTAX:	Actionlist.prog, Delay.w, Repeats.w(0=Forever)

	move.l	a5,CL_Parameters(a1)
	move.w	#0,CL_Data(a1)
	move.w	6(a5),CL_Data+2(a1)

	move.l	#_UpdateTimeDelayExecute,CL_UpdateCode(a1)
.Exit
	lea	8(a5),a5
	rts

_UpdateTimeDelayExecute
	tst.w	CL_Data(a6)
	beq	.Execute
	sub.w	#1,CL_Data(a6)
	bra	.Exit
.Execute
	move.l	CL_Parameters(a6),a0
	move.w	4(a0),CL_Data(a6)

	move.l	a5,-(sp)

	move.l	(a0),a5
	cmp.l	#0,a5
	beq	.ExecuteDone
	move.w	#CONTEXT_BOB,d0
	bsr	_DoActionList
.ExecuteDone
	move.l	(sp)+,a5

	tst.w	CL_Data+2(a6)
	beq	.Exit

	sub.w	#1,CL_Data+2(a6)
	tst.w	CL_Data+2(a6)
	bne	.Exit

	move.l	#1,d7
.Exit
	rts

;************************************************************

_InitDrawBar
	;Inputs:	a1.l = Channel
	;		a2.l = Slice
	;		a3.l = Bob
	;		a5.l = Paramlist
	;Params:
	;	XPos.w		in pixels (rounded to nearest word)
	;	YPos.w		in lines
	;	Width.w		Max bar width, in pixels (rounded to nearest word)
	;	MaxVal.l	Value of variable at max width
	;	Var.w		Variable id *4

	;Channel var usage:
	;CL_Data1.l:	Base address of destination
	;CL_Data2.w:	Scale value (8 bitshifts)
	;CL_Data3.w:	Variable to display
	;CL_Data4.w:	Prevous bar size

	;Find the statbar slice. NASTY!  XYZZY
	move.l	_FirstSlice,a0
	move.l	SS_Next(a0),a0
	cmp.l	#0,a0			;no statbar?
	beq	.FuckOffAndDie

	;save params for later
	move.l	a5,CL_Parameters(a1)

	;Suss base dest address of bar

	move.w	2(a5),d0		;get YPos
	move.l	SS_BufferWidthxPlanes(a0),d1
	mulu	d0,d1
	move.w	(a5),d0			;get XPos
	lsr.w	#3,d0			;num of bytes
	and.l	#$0000FFFE,d0		;round to nearest word

	add.l	d0,d1
	add.l	SS_PhyBase(a0),d1
	move.l	d1,CL_Data1(a1)

	;Suss the scale factor

	move.w	4(a5),d0		;get Width (in pixels)
	and.l	#$0000FFF0,d0		;round to nearest word
	move.w	#8,d1
	lsl.l	d1,d0			;8 bitshifts
	move.l	6(a5),d1		;get maxval
	tst.w	d1
	beq	.FuckOffAndDie
	divu	d1,d0		;divide by MaxVal
	move.w	d0,CL_Data2(a1)		;store

	;Store variable id
	move.w	10(a5),d0		;get id
	lsl.w	#2,d0			;longword index
	move.w	d0,CL_Data3(a1)

	move.w	#0,CL_Data4(a1)

	;Skip PC over params
	add.l	#2+2+2+4+2,a5

	;Install it.
	move.l	#.UpdateRoutine,CL_UpdateCode(a1)

.FuckOffAndDie
	rts


.UpdateRoutine
	;Inputs:	a6.l = Channel
	;		a2.l = Slice
	;		a3.l = Bob

	movem.l	d2-d4/a2-a4,-(sp)

	move.l	_FirstSlice,a0
	move.l	SS_Next(a0),a2
	cmp.l	#0,a2			;no statbar?
	beq	.FuckOffAndDie

	move.w	CL_Data3(a6),d0		;get var id (already *4)
	lea	_GetVariableTable,a0
	move.l	(a0,d0.w),a0
	jsr	(a0)			;call read routine, result in d1.l

	;if the value is less than zero then we're just going to show zero.
	tst.l	d1
	bpl	.NoClipToZilch
	move.l	#0,d1
.NoClipToZilch

	mulu	CL_Data2(a6),d1		;* scale factor
	lsr.w	#8,d1			;shift down to pixels

	cmp.w	CL_Data4(a6),d1		;no change?
	beq	.Exit
	move.w	d1,CL_Data4(a6)

	move.l	CL_Parameters(a6),a0
	move.w	4(a0),d0		;get width
	lsr.w	#4,d0			;/16 to get words

	move.l	CL_Data1(a6),a4		;Pntr to Screen
	move.l	SS_BufferWidth(a2),d3
	move.w	SS_Planes(a2),d4	;d4 = num of words to draw (vertically)
	mulu	_StatBarHeight,d4
	sub.w	#1,d4			;for dbra

	;Step through all the words in the bar
.WordLoop
	sub.w	#1,d0
	bmi	.Exit			;done em all?

	cmp.w	#0,d1			;run out of value...
	ble	.DrawZilch

	cmp.w	#16,d1			;less than 16 value left...
	blt	.DrawPartial
	;else draw full - 16 or more value left

	move.l	_StatBarImages,a1	;first image
	bra	.DoDraw
.DrawZilch
	lea	_StatBarImages,a1
	move.l	16*4(a1),a1		;last image
	bra	.DoDraw
.DrawPartial
	;partially filled word
	move.w	#16,d2
	sub.w	d1,d2
	lsl.w	#2,d2
	lea	_StatBarImages,a1
	move.l	(a1,d2.w),a1

.DoDraw
	move.w	d4,d2			;num of words to copy
	move.l	a4,a0			;copy dest ptr

.CopyLoop
	move.w	(a1)+,(a0)		;copy a word
	add.l	d3,a0			;next line
	dbra	d2,.CopyLoop

	sub.w	#16,d1			;subtract another 16 from value
	add.l	#2,a4			;increment dest ptr
	bra	.WordLoop

.Exit
	movem.l	(sp)+,d2-d4/a2-a4
	rts



;************************************************************

_InitNULL
	move.l	#_UpdateNULL,CL_UpdateCode(a1)
	rts
_UpdateNULL
	rts

;************************************************************



;************************************************************

_InitFollowPath
	;in: a1=channel,a2=slice, a3=bob, a5=params
	;0	Path.l
	;4	Flags.w	(0=Ignore path StartPos, 1=repeat? )

	move.l	a5,CL_Parameters(a1)
	move.l	#.Update,CL_UpdateCode(a1)

	move.l	(a5),a0			;get path
	lea	PH_Data(a0),a0		;skip pathheader
	move.l	a0,CL_Data1(a1)		;pathdata in CL_Data1.l
	move.w	#0,CL_Data2(a1)		;step counter in CL_Data2.w

	move.l	#0,CL_Data3(a1)		;previous step xspd
	move.l	#0,CL_Data4(a1)		;previous step yspd

	move.w	4(a5),d0
	btst	#0,d0			;Ignore path startpos?
	bne	.IgnoreStartPos

	move.l	(a5),a0			;get path
	move.l	PH_XPos(a0),d0
	asl.l	#BITSHIFT,d0
	move.l	d0,OB_XPos(a3)

	move.l	PH_YPos(a0),d0
	asl.l	#BITSHIFT,d0
	move.l	d0,OB_YPos(a3)
.IgnoreStartPos
	lea	6(a5),a5
	rts

.Update
	;in: a2=slice, a3=bob,a6=channel

	sub.w	#1,CL_Data2(a6)
	bpl	.Exit

	;step count run down

	move.l	CL_Data1(a6),a0
	move.w	(a0)+,d0
	bmi	.StartLineSeg
	;end path
	move.l	CL_Parameters(a6),a0
	move.w	4(a0),d0		;flags
	;stop
	move.l	CL_Data3(a6),d0
	sub.l	d0,BB_XSpeed(a3)
	move.l	CL_Data4(a6),d0
	sub.l	d0,BB_YSpeed(a3)
	move.w	#1,d7		;stop ze channel
	bra	.Exit
.StartLineSeg
	neg.w	d0
	sub.w	#1,d0
	move.w	d0,CL_Data2(a6)

	move.w	(a0)+,d0
	ext.l	d0
	move.l	d0,d1
	sub.l	CL_Data3(a6),d0
	add.l	d0,BB_XSpeed(a3)
	move.l	d1,CL_Data3(a6)

	move.w	(a0)+,d0
	ext.l	d0
	move.l	d0,d1
	sub.l	CL_Data4(a6),d0
	add.l	d0,BB_YSpeed(a3)
	move.l	d1,CL_Data4(a6)

	move.l	a0,CL_Data1(a6)

.Exit
	move.l	(sp)+,a5
	rts


;************************************************************

_SpeedDependantAnimInit
	;Inputs:	a1.l = Channel
	;		a2.l = Slice
	;		a3.l = Bob
	;		a5.l = Paramlist

	move.l	(a5)+,CL_Data(a1)	;Anim
	move.l	#_SpeedDependantAnimUpdate,CL_UpdateCode(a1)
	rts

_SpeedDependantAnimUpdate
	;Inputs:	a1.l = Channel
	;		a2.l = Slice
	;		a3.l = Bob

	add.l	a3,a1

	move.l	BB_XSpeed(a3),d0
	asr.l	#BITSHIFT-1,d0
	and.l	#$1E,d0

	lsl.w	#2,d0
	move.w	d0,d1
	lsl.w	d1
	add.w	d1,d0

	move.l	CL_Data(a1),a0			;Anim pointer

	move.l	(a0,d0.w),OB_Image(a3)
	move.l	4(a0,d0.w),d1
	move.l	d1,OB_AnimXOffset(a3)
	move.l	8(a0,d0.w),d1
	move.l	d1,OB_AnimYOffset(a3)
.Exit
	rts


;************************************************************

_InitOperationWolfMovement
	;SYNTAX		XMaxSpeed.l,	YMaxSpeed.l,
	;		Channel.w,	FireFlags.w,
	;		Flags.w

	move.l	(a5)+,BB_XMaxSpeed(a3)
	move.l	(a5)+,BB_YMaxSpeed(a3)

	move.w	(a5)+,d0
;	lsl.w	#2,d0
;	lea	_InputChannels,a0
;	move.l	(a0,d0.w),OB_InputChannel(a3)

	move.w	(a5)+,CL_Flags(a1)
	move.w	(a5)+,CL_Flags+2(a1)
	move.l	#_UpdateOperationWolfMovement,CL_UpdateCode(a1)
	rts

_UpdateOperationWolfMovement

	move.w	d2,-(sp)

	move.w	#4,d2
	btst.b	#5,CL_Flags+3(a6)
	beq	.SlowMouse
	move.w	#5,d2
.SlowMouse

;	move.l	OB_InputChannel(a3),a0
	move.l	SS_Scroll(a2),a1
	
	move.w	_RawMouseX,d0
	move.w	#0,_RawMouseX
;	move.w	IC_MouseXResult(a0),d0
	lsl.w	d2,d0
	ext.l	d0

	move.l	d0,d1
	abs.l	d1
	cmp.l	BB_XMaxSpeed(a3),d1
	blt	.XOk

	move.l	d0,d1
	move.l	BB_XMaxSpeed(a3),d0
	tst.l	d1
	bpl	.XOk
	neg.l	d0
.XOk
	add.l	SC_BaseSpeed(a1),d0
	move.l	d0,BB_XSpeed(a3)

.Y
	move.w	_RawMouseY,d0
	move.w	#0,_RawMouseY
;	move.w	IC_MouseYResult(a0),d0
	lsl.w	d2,d0
	ext.l	d0

	move.l	d0,d1
	abs.l	d1
	cmp.l	BB_YMaxSpeed(a3),d1
	blt	.YOk

	move.l	d0,d1
	move.l	BB_YMaxSpeed(a3),d0
	tst.l	d1
	bpl	.YOk
	neg.l	d0
.YOk
	move.l	d0,BB_YSpeed(a3)

.CheckFire
;	move.l	a0,-(sp)
;	btst.b	#4,IC_JoyResult(a0)
;	bnesr	.Fire
;	move.l	(sp)+,a0

;	move.w	IC_JoyResult(a0),d0
;	and.w	#%10000,d0
;	move.w	d0,CL_Data(a6)

.Exit
	move.w	(sp)+,d2
	rts
.Fire
	move.w	CL_Flags+2(a6),d0
	and.l	#$FFFF,d0
	btst.l	#4,d0
	beq	.DoFire

	tst.w	CL_Data(a6)
	beq	.DoFire
	bra	.NoFire
.DoFire
	movem.w	d2-d3,-(sp)
	move.w	CL_Flags(a6),d2
	move.w	#0,d3
.Loop
	btst	d3,d2
	beq	.GetNext

	move.w	d3,d0
	bsr	_InternalFireWeaponBay
.GetNext
	add.w	#1,d3
	cmp.w	#1,d3	;XYZZY - WEAPONBAYS?
	bne	.Loop

	movem.w	(sp)+,d2-d3
.NoFire
	rts

_InitInertialJoyMovement
	;SYNTAX:	XMaxSpeed.l,	YMaxSpeed.l
	;		XAccell.l,	YAccell.l
	;		XDeccel.l,	YDeccel.l
	;		Channel.w,	FireFlags.w
	;		Flags.w,	LeftAction.l
	;		RightAction.l,	TopAction.l,
	;		BottomAction.l

	;Fire Flags - Bits per bay to fire
	;Flags	- L,R,U,D off, Auto/Manual fire, Calc Direction,XDirConstrain,YDirConstrain

	move.l	(a5),BB_XMaxSpeed(a3)
	move.l	4(a5),BB_YMaxSpeed(a3)
	move.l	8(a5),BB_XAccel(a3)
	move.l	12(a5),BB_YAccel(a3)

	move.l	16(a5),BB_XDeccel(a3)
 	move.l	20(a5),BB_YDeccel(a3)

;	move.w	24(a5),d0		;inputchannel
;	lsl.w	#2,d0
;	lea	_InputChannels,a0
;	move.l	(a0,d0.w),OB_InputChannel(a3)
;	move.w	d0,OB_InputChannel(a3)

	move.w	26(a5),CL_Flags(a1)
	move.w	28(a5),CL_Flags+2(a1)

	move.l	a5,CL_Parameters(a1)

	move.l	#_InertialJoyMovement,CL_UpdateCode(a1)

	lea	46(a5),a5
	rts

_InertialJoyMovement
	;Inputs:	a2.l = Slice
	;		a3.l = Player

	movem.l	d2-d3/a5,-(sp)

;	move.l	OB_InputChannel(a3),a0
;	move.w	IC_JoyResult(a0),d1
;	move.w	OB_InputChannel(a3),d0
;	move.w	#0,d0

	move.l	CL_Parameters(a6),a5

	move.w	24(a5),d0		;inputchannel
	jsr	_ReadVirtualJoystick
	move.b	d0,d1

	move.w	#0,d2
	move.w	#0,d3

	tst.w	d1
	beq	.NoDir

	move.w	CL_Flags+2(a6),d0
	and.l	#$FFFF,d0
	btst.l	#5,d0
	bne	.NoDirChange

	move.w	d1,d0
	and.w	#$F,d0
	tst.w	d0
	beq	.NoDirChange

	btst.b	#6,CL_Flags+3(a6)
	beq	.NoXDirConstrain
	and.w	#12,d0
	beq	.NoDirChange
	bra	.NoYDirConstrain
.NoXDirConstrain
	btst.b	#7,CL_Flags+3(a6)
	beq	.NoYDirConstrain
	move.w	d1,d0
	and.w	#3,d0
	beq	.NoDirChange
.NoYDirConstrain
	move.w	d1,d0
	and.w	#$F,d0
	lea	_JoyDirTable,a0
	move.b	(a0,d0.w),BB_Direction(a3)

.NoDirChange
	lsr.w	d1
	bcssr	.Up
	lsr.w	d1
	bcssr	.Down
	lsr.w	d1
	bcssr	.Left
	lsr.w	d1
	bcssr	.Right
	lsr.w	d1
	bcssr	.Fire
.NoDir
	tst.w	d2
	bne	.MovedX
	move.l	SS_Scroll(a2),a0
	move.l	BB_XSpeed(a3),d0
	cmp.l	SC_BaseSpeed(a0),d0
	beq	.MovedX
	bpl	.XMinus
	bmi	.XPlus 
.MovedX	tst.w	d3
	bne	.Exit
	cmp.l	#0,BB_YSpeed(a3)
	beq	.Exit
	bpl	.YMinus
	bmi	.YPlus
	bra	.Exit

.XMinus	move.l	BB_XDeccel(a3),d0
	sub.l	d0,BB_XSpeed(a3)
	move.l	BB_XSpeed(a3),d0
	cmp.l	SC_BaseSpeed(a0),d0
	bgt	.MovedX
	move.l	SC_BaseSpeed(a0),BB_XSpeed(a3)
	bra	.MovedX

.XPlus	move.l	BB_XDeccel(a3),d0
	add.l	d0,BB_XSpeed(a3)
	move.l	BB_XSpeed(a3),d0
	cmp.l	SC_BaseSpeed(a0),d0
	blt	.MovedX
	move.l	SC_BaseSpeed(a0),BB_XSpeed(a3)
	bra	.MovedX

.YMinus	move.l	BB_YDeccel(a3),d0
	sub.l	d0,BB_YSpeed(a3)
	bpl	.Exit
	move.l	#0,BB_YSpeed(a3)
	bra	.Exit
.YPlus	move.l	BB_YDeccel(a3),d0
	add.l	d0,BB_YSpeed(a3)
	bmi	.Exit
	move.l	#0,BB_YSpeed(a3)
.Exit
	move.w	24(a5),d0
	jsr	_ReadVirtualJoystick
	and.w	#%10000,d0
	move.w	d0,CL_Data(a6)

	move.l	#0,d0

	movem.l	(sp)+,d2-d3/a5
	rts

.Left
	move.l	CL_Parameters(a6),a0
	move.l	30(a0),d0
	beq	.NoLeftAction
	move.l	a5,-(sp)
	move.l	d0,a5
	move.w	#CONTEXT_BOB,d0
	jsr	_DoActionList
	move.l	(sp)+,a5
.NoLeftAction
	move.w	CL_Flags+2(a6),d0
	and.l	#$FFFF,d0
	btst.l	#0,d0
	bne	.LeftExit

	move.w	#1,d2
	move.l	BB_XSpeed(a3),d0
	add.l	BB_XMaxSpeed(a3),d0
	sub.l	SC_BaseSpeed(a0),d0
	bgt	.LeftOk
	move.l	BB_XMaxSpeed(a3),d0
	neg.l	d0
	move.l	d0,BB_XSpeed(a3)
	rts
.LeftOk	move.l	BB_XAccel(a3),d0
	sub.l	d0,BB_XSpeed(a3)
.LeftExit
	rts

.Right
	move.l	CL_Parameters(a6),a0
	move.l	34(a0),d0
	beq	.NoRightAction
	move.l	a5,-(sp)
	move.l	d0,a5
	move.w	#CONTEXT_BOB,d0
	jsr	_DoActionList
	move.l	(sp)+,a5
.NoRightAction
	move.w	CL_Flags+2(a6),d0
	and.l	#$FFFF,d0
	btst.l	#1,d0
	bne	.RightExit

	move.w	#1,d2
	move.l	BB_XSpeed(a3),d0
	sub.l	BB_XMaxSpeed(a3),d0
	sub.l	SC_BaseSpeed(a0),d0
	bmi	.RightOk
	move.l	BB_XMaxSpeed(a3),BB_XSpeed(a3)
	rts
.RightOk
	move.l	BB_XAccel(a3),d0
	add.l	d0,BB_XSpeed(a3)
.RightExit
	rts

.Up
	move.l	CL_Parameters(a6),a0
	move.l	38(a0),d0
	beq	.NoUpAction
	move.l	a5,-(sp)
	move.l	d0,a5
	move.w	#CONTEXT_BOB,d0
	jsr	_DoActionList
	move.l	(sp)+,a5
.NoUpAction
	move.w	CL_Flags+2(a6),d0
	and.l	#$FFFF,d0
	btst.l	#2,d0
	bne	.UpExit

	move.w	#-2,d3
	move.l	BB_YSpeed(a3),d0
	add.l	BB_YMaxSpeed(a3),d0
	bgt	.UpOk
	move.l	BB_YMaxSpeed(a3),d0
	neg.l	d0
	move.l	d0,BB_YSpeed(a3)
	rts
.UpOk	move.l	BB_YAccel(a3),d0
	sub.l	d0,BB_YSpeed(a3)
.UpExit
	rts

.Down
	move.l	CL_Parameters(a6),a0
	move.l	42(a0),d0
	beq	.NoDownAction
	move.l	a5,-(sp)
	move.l	d0,a5
	move.w	#CONTEXT_BOB,d0
	jsr	_DoActionList
	move.l	(sp)+,a5
.NoDownAction
	move.w	CL_Flags+2(a6),d0
	and.l	#$FFFF,d0
	btst.l	#3,d0
	bne	.DownExit

	move.w	#2,d3

	move.l	BB_YSpeed(a3),d0
	sub.l	BB_YMaxSpeed(a3),d0
	bmi	.DownOk
	move.l	BB_YMaxSpeed(a3),BB_YSpeed(a3)
	rts
.DownOk	move.l	BB_YAccel(a3),d0
	add.l	d0,BB_YSpeed(a3)
.DownExit
	rts

.Fire
	move.w	CL_Flags+2(a6),d0
	and.l	#$FFFF,d0
	btst.l	#4,d0
	beq	.DoFire

	tst.w	CL_Data(a6)
	beq	.DoFire
	bra	.NoFire
.DoFire
	movem.w	d2-d3,-(sp)
	move.w	CL_Flags(a6),d2
	move.w	#0,d3
.Loop
	btst	d3,d2
	beq	.GetNext

	move.w	d3,d0
	bsr	_InternalFireWeaponBay

.GetNext
	add.w	#1,d3
	cmp.w	#WEAPONBAYS,d3
	bne	.Loop

	movem.w	(sp)+,d2-d3
.NoFire
	rts



;************************************************************
;Internal mechanism for firing weapon bays

_InternalFireWeaponBay

	;inputs:
	;	d0.w = weapon bay number
	;	a2   = slice
	;	a3   = bob

	movem.l	a4-a5,-(sp)

	;get weapon structure
	lsl.w	#2,d0
	move.l	BB_WeaponBank(a3),a4
	move.l	(a4,d0.w),a4		;a4 = weapon struct
	cmp.l	#0,a4			;no weapon?
	beq	.Exit

	move.w	WP_MaxBullets(a4),d0
	cmp.w	WP_Bullets(a4),d0
	beq	.Exit
	cmp.w	#0,WP_BulletDelay(a4)	;XYZZY - Move delay mechanism into obj
	bne	.Exit

	move.l	WP_FireActionList(a4),a5
	cmp.l	#0,a5
	beq	.Exit

	;create a bullet, or just execute on player?
	move.w	WP_Flags(a4),d0
	btst	#0,d0
	beq	.ExecOnPlayer

	;Create new bullet and execute on that...

	bsr	_FindFreeBulletDude
	cmp.l	#0,a0
	beq	.Exit
	move.w	#1,OB_State(a0)
	move.w	#OB_TYPE_PLAYERBULLET,OB_Type(a0)

	move.l	a3,-(sp)
	move.l	a0,a3
	bsr	_NotINITBOBAlthoughFairlySimilar
	move.l	a3,a0
	move.l	(sp)+,a3

	move.l	OB_XPos(a3),OB_XPos(a0)
	move.l	OB_YPos(a3),OB_YPos(a0)
	move.l	BB_XSpeed(a3),BB_XSpeed(a0)
	move.l	BB_YSpeed(a3),BB_YSpeed(a0)
	move.l	#0,OB_Image(a0)
	move.l	a3,OB_Parent(a0)
	move.w	#PARENT_OBJECT,OB_ParentType(a0)
	move.b	BB_Direction(a3),BB_Direction(a0)

	add.w	#1,WP_Bullets(a4)
	move.w	WP_MaxBulletDelay(a4),WP_BulletDelay(a4)
	move.l	a4,BB_WeaponBank(a0)

	movem.l	a3-a4,-(sp)
	move.l	a0,a3
	move.w	#CONTEXT_BOB,d0		;could be CONTEXT_WEAPON
	bsr	_DoActionList
	movem.l	(sp)+,a3-a4
	bra	.Exit

.ExecOnPlayer
	;XYZZY - move delay mecahinism into obj
	move.w	WP_MaxBulletDelay(a4),WP_BulletDelay(a4)
	move.w	#CONTEXT_WEAPON,d0
	bsr	_DoActionList

.Exit
	movem.l	(sp)+,a4-a5
	rts


;************************************************************
_InternalFireMultiWeaponBays
	;in:	d0.w = bays to fire (1 bit per bay)
	;	a2=slice,a3=bob

	movem.w	d2-d3,-(sp)
	move.w	d0,d2

	move.w	#0,d3
.Loop
	btst	d3,d2
	beq	.Next
	move.w	d3,d0
	bsr	_InternalFireWeaponBay
.Next
	add.w	#1,d3
	cmp.w	#WEAPONBAYS,d3
	bne	.Loop

	movem.w	(sp)+,d2-d3
	rts

;************************************************************

_InitJoystickAnim
	;Sets up anims depending on joystick result
	;SYNTAX:	InputChannel.w, Flags.w, Anim*18
	;Flags: Bit 0 - Set Direction
	;	Bit 1 - If set only play anim once
	;	Bit 2 - Stop on Null anim

	move.w	(a5)+,d0
	move.w	(a5)+,CL_Flags(a1)

	move.l	a5,CL_Parameters(a1)
	move.l	#.UpdateJoystickAnim,CL_UpdateCode(a1)

	move.l	#0,CL_Data(a1)		;Anim Base
	move.w	#0,CL_Data+4(a1)	;Frame Offset
	move.w	#3,CL_Data+6(a1)	;Old Direction (3 Can't Happen)
	move.w	#0,CL_Data+8(a1)	;Anim Done Flag (Only used if Play Anim Once is Set)

	lea	4*18(a5),a5
	rts

.UpdateJoystickAnim
	;XYZZY - messy, but more or less ok...
	move.l	CL_Parameters(a6),a0		;Anim Array Base
	move.w	-4(a0),d0
	jsr	_ReadVirtualJoystick
	and.w	#$FF,d0
	move.w	d0,d1

	btst.b	#0,CL_Flags+1(a6)	;Calc Direction Flag ?
	beq	.NoDirChange

	move.w	d1,d0			;Calc Direction
	and.w	#$F,d0
	tst.w	d0
	beq	.NoDirChange
	lea	_JoyDirTable,a0
	move.b	(a0,d0.w),BB_Direction(a3)

.NoDirChange
	cmp.w	CL_Data+6(a6),d1		;Check weather direction has changed
	beq	.UpdateAnim
.SetNewAnim
	move.w	d1,CL_Data+6(a6)
	lea	_JoyAnimTable,a0
	lsl.w	d1
	move.w	(a0,d1.w),d0

	move.l	CL_Parameters(a6),a0		;Anim Array Base
	move.l	(a0,d0.w),d0			;Keep Old Anim If New Not Set
	bne	.PutNewAnim
	btst.b	#2,CL_Flags+1(a6)		;stop on null anim?
	beq	.UpdateAnim
	move.w	#1,CL_Data+8(a6)
	bra	.UpdateAnim

.PutNewAnim
	move.l	d0,CL_Data(a6)			;Get New anim
	move.w	#0,CL_Data+4(a6)
	move.w	#0,CL_Data+8(a6)
.UpdateAnim
	tst.w	CL_Data+8(a6)
	bne	.Exit

	move.l	CL_Data(a6),a0			;Anim Base
	cmp.l	#0,a0
	beq	.Exit
	move.w	CL_Data+4(a6),d0		;Frame Offset

	move.l	(a0,d0.w),OB_Image(a3)
	move.l	4(a0,d0.w),d1
	move.l	d1,OB_AnimXOffset(a3)
	move.l	8(a0,d0.w),d1
	move.l	d1,OB_AnimYOffset(a3)

	add.w	#12,d0
	add.w	#12,CL_Data+4(a6)	;frame offset
	tst.l	(a0,d0.w)
	bne	.Exit
	move.w	#0,CL_Data+4(a6)
	btst.b	#1,CL_Flags+1(a6)	;Play Anim Once ?
	beq	.Exit
	move.w	#1,CL_Data+8(a6)
.Exit
	rts



;************************************************************


_InitSteroids
	;SYNTAX:	Channel.w, Yaw.w, DirMaxSpeed.l, Acceleration.l, Decelleration.l, DownAction.l

	move.l	a5,CL_Parameters(a1)

	move.l	4(a5),BB_MaxDirSpeed(a3)
	move.l	#0,BB_DirSpeed(a3)
	move.b	BB_Direction(a3),CL_Data(a1)

	move.w	#%1111,CL_Flags(a1)

	move.l	a5,CL_Parameters(a1)
	move.l	#.UpdateSteroids,CL_UpdateCode(a1)

	lea	20(a5),a5
	rts

.UpdateSteroids
	;Inputs:	a2.l = Slice
	;		a3.l = Player
 
	movem.w	d2-d3,-(sp)
	movem.l	a4-a5,-(sp)

	move.l	CL_Parameters(a6),a5
	move.w	(a5),d0
	jsr	_ReadVirtualJoystick
	and.w	#$FF,d0
	move.w	d0,d3

;	move.l	OB_InputChannel(a3),a0
;	move.w	IC_JoyResult(a0),d3

	move.w	#0,d2

	move.l	CL_Parameters(a6),a4

	lsr.w	d3
	bcssr	.Up
	lsr.w	d3
	bcssr	.Down
	lsr.w	d3
	bcssr	.Left
	lsr.w	d3
	bcssr	.Right
	lsr.w	d3
	bcssr	.Fire
.NoDir
	tst.w	d2
	bne	.Exit

	move.w	#0,d1

	move.l	BB_XSpeed(a3),d0
	asr.l	#4,d0
	beq	.KillX
	sub.l	d0,BB_XSpeed(a3)
	bra	.CheckY
.KillX
	move.w	#1,d1
.CheckY
	move.l	BB_YSpeed(a3),d0
	asr.l	#4,d0
	beq	.KillY
	sub.l	d0,BB_YSpeed(a3)
	bra	.Exit
.KillY
	tst.w	d1
	beq	.Exit

	move.l	#0,BB_YSpeed(a3)
	move.l	#0,BB_XSpeed(a3)
.Exit
	movem.l	(sp)+,a4-a5
	movem.w	(sp)+,d2-d3
	rts

.Left
	move.w	2(a4),d0
	sub.b	d0,BB_Direction(a3)
.LeftOk
	rts

.Right
	move.w	2(a4),d0
	add.b	d0,BB_Direction(a3)
.RightOk
	rts

.Up
	move.l	BB_XSpeed(a3),d0
	muls	BB_XSpeed+2(a3),d0

	move.l	BB_YSpeed(a3),d1
	muls	BB_YSpeed+2(a3),d1

	add.l	d1,d0

	move.l	BB_MaxDirSpeed(a3),d1
	beq	.NoMaxSpeed
	mulu	BB_MaxDirSpeed+2(a3),d1

	cmp.l	d1,d0
	bgt	.UpOk
.NoMaxSpeed
	move.w	#1,d2
	move.b	BB_Direction(a3),CL_Data(a6)

	move.l	8(a4),d1		;Get polar Accel
	move.b	BB_Direction(a3),d0
	bsr	_SetSpeedsFromDirection	;convert to cartesian

	add.l	d0,BB_XSpeed(a3)	;Add cartesian accel to speeds
	add.l	d1,BB_YSpeed(a3)
.UpOk
	rts

.Down
	move.l	a5,-(sp)
	move.l	16(a4),a5
	cmp.l	#0,a5
	beq	.NoDown
	move.w	#CONTEXT_BOB,d0
	bsr	_DoActionList
.NoDown
	move.l	(sp)+,a5
.DownOk
	rts

.Fire
	movem.w	d2-d3,-(sp)
	move.w	CL_Flags(a6),d2
	move.w	#0,d3
.Loop
	btst	d3,d2
	beq	.GetNext

	move.w	d3,d0
	bsr	_InternalFireWeaponBay
.GetNext
	add.w	#1,d3
	cmp.w	#1,d3	;XYZZY - WEAPONBAYS?
	bne	.Loop

	movem.w	(sp)+,d2-d3
.NoFire	rts

;************************************************************

_OldInitActionListPlayer
	;Player has action lists for all possible inputs
	;SYNTAX:	InputChannel.w, U.prog, D.prog, L.prog ,R.prog , Flags.l
	;Inputs:	a1.l = Channel
	;		a2.l = Slice
	;		a3.l = Bob
	;		a5.l = Paramlist

	move.w	(a5)+,d0
;	lsl.w	#2,d0
;	lea	_InputChannels,a0
;	move.l	(a0,d0.w),OB_InputChannel(a3)

	move.l	a5,CL_Parameters(a1)		;U,D,L,R,Tags
	lea	20(a5),a5
	move.l	#_OldUpdateActionListPlayer,CL_UpdateCode(a1)
	rts

_OldUpdateActionListPlayer

	;Inputs:	a6.l = Channel
	;		a2.l = Slice
	;		a3.l = Bob

	move.l	a5,-(sp)
	move.l	CL_Parameters(a6),a5

	;XYZZY - is this needed? or used?
	tst.w	BB_BulletDelay(a3)
	beq	.NoBulletDelay
	sub.w	#1,BB_BulletDelay(a3)
.NoBulletDelay
	move.w	#0,d0
	btst.b	d0,16(a5)
	;bnesr	_StandardBorderCheck

;	move.l	OB_InputChannel(a3),a0
;	move.w	IC_JoyResult(a0),d1
	move.w	-2(a5),d0		;ick.
	jsr	_ReadVirtualJoystick
	move.w	d0,d1

	move.w	#0,d2
	move.w	#0,d3

	tst.w	d1
	beq	.NoDir
	lsr.w	d1
	bcssr	.Up
	lsr.w	d1
	bcssr	.Down
	lsr.w	d1
	bcssr	.Left
	lsr.w	d1
	bcssr	.Right
	lsr.w	d1
;	bcssr	.Fire
.NoDir
	move.w	#1,d0
	btst.b	d0,16(a5)
	bnesr	_StandardDecelleration
.Exit
	move.l	(sp)+,a5
	rts
.Left
	move.l	a5,-(sp)
	move.l	8(a5),a5
	cmp.l	#0,a5
	beq	.LeftDone
	move.w	#CONTEXT_BOB,d0
	bsr	_DoActionList
.LeftDone
	move.l	(sp)+,a5
	move.w	#-2,d2
	rts
.Right
	move.l	a5,-(sp)
	move.l	12(a5),a5
	cmp.l	#0,a5
	beq	.RightDone
	move.w	#CONTEXT_BOB,d0
	bsr	_DoActionList
.RightDone
	move.l	(sp)+,a5
	move.w	#2,d2
	rts
.Up
	move.l	a5,-(sp)
	move.l	(a5),a5
	cmp.l	#0,a5
	beq	.UpDone
	move.w	#CONTEXT_BOB,d0
	bsr	_DoActionList
.UpDone
	move.l	(sp)+,a5
	move.w	#1,d3
	rts
.Down
	move.l	a5,-(sp)
	move.l	4(a5),a5
	cmp.l	#0,a5
	beq	.DownDone
	move.w	#CONTEXT_BOB,d0
	bsr	_DoActionList
.DownDone
	move.l	(sp)+,a5
	move.w	#1,d3
	rts
;.Fire
;	tst.w	BB_BulletDelay(a3)
;	bne	.NoFire
;	move.w	BB_Bullets(a3),d0
;	cmp.w	BB_MaxBullets(a3),d0
;	bge	.NoFire
;	move.w	BB_MaxBulletDelay(a3),BB_BulletDelay(a3)
;	move.l	a5,-(sp)
;	move.l	BB_FireAction(a3),a5
;	bsr	_DoActionList
;	move.l	(sp)+,a5
;.NoFire	rts



;*************************************************************

_InitActionOnFire
	;Executes an action whenever firebutton is depressed
	;Input channel.w, Delay.w,Action.l

	move.w	2(a5),CL_Data(a1)	;Delay
	move.w	#0,CL_Data+6(a1)
	move.l	4(a5),CL_Data+2(a1)	;Action

	move.l	a5,CL_Parameters(a1)
	move.l	#_UpdateActionOnFire,CL_UpdateCode(a1)
	lea	8(a5),a5
	rts

_UpdateActionOnFire
	move.l	a5,-(sp)
	move.l	CL_Parameters(a6),a5

	sub.w	#1,CL_Data+6(a6)
	tst.w	CL_Data+6(a6)
	bgt	.Exit

	move.w	CL_Data(a6),CL_Data+6(a6)

	move.w	(a5),d0		;get input channel
	jsr	_ReadVirtualJoystick
	btst	#4,d0
	beq	.Exit

	move.l	CL_Data+2(a6),d0
	beq	.Exit
	move.l	a5,-(sp)
	move.l	d0,a5

	move.w	#CONTEXT_BOB,d0
	bsr	_DoActionList
	move.l	(sp)+,a5
.Exit
	move.l	(sp)+,a5
	rts

;*************************************************************

_SimplePlatformPlayer
	;InputChannel,XAccel,Gravity,JumpSpeed,MaxXSpeed,MaxYSpeed,LastSpace.w,LastFloor.w,Flags.w

	move.w	(a5)+,d0
;	lsl.w	#2,d0
;	lea	_InputChannels,a0
;	move.l	(a0,d0.w),OB_InputChannel(a3)

	move.l	(a5),BB_XDeccel(a3)
	move.l	(a5)+,BB_XAccel(a3)
	move.l	(a5)+,BB_YAccel(a3)
	move.l	(a5)+,CL_Data+2(a1)
	neg.l	CL_Data+2(a1)


	move.l	(a5)+,BB_XMaxSpeed(a3)
	move.l	(a5)+,BB_YMaxSpeed(a3)

	move.l	a5,CL_Parameters(a1)
	lea	4(a5),a5

	move.w	(a5)+,d0

	move.b	d0,CL_Data(a1)
	move.b	#0,CL_Data+1(a1)
	move.l	#.Update,CL_UpdateCode(a1)
	rts

.Update
	move.l	d2,-(sp)
	move.w	#0,d2

.CheckStatus
	btst.b	#0,CL_Data+1(a6)	;State - 0 = On Ground, 1 = In Air
	beq	.OnGround
.InAir

.Grav
	move.l	BB_YSpeed(a3),d0
	add.l	BB_YAccel(a3),d0		;Gravity
	sub.l	BB_YMaxSpeed(a3),d0
	bmi	.GravOk
	move.l	BB_YMaxSpeed(a3),d0
	move.l	d0,BB_YSpeed(a3)
	bra	.CheckAirJordan
.GravOk
	move.l	BB_YAccel(a3),d0
	add.l	d0,BB_YSpeed(a3)

.CheckAirJordan
	btst.b	#0,CL_Data(a6)		;Allowed to move in air 0=off
	beq	.NoAirJordan
.AirJordan
;	move.l	OB_InputChannel(a3),a0
;	move.w	IC_JoyResult(a0),d2
	move.l	CL_Parameters(a6),a0
	move.w	-6(a0),d0
	jsr	_ReadVirtualJoystick
	and.w	#$FF,d0
	move.w	d0,d2

	move.l	SS_Scroll(a2),a0

.CheckLeftAir
	btst	#2,d2
	beq	.CheckRightAir

	bsr	.Left
	bra	.NoRightAir

.CheckRightAir
	btst	#3,d2
	beq	.NoRightAir

	bsr	.Right
	bra	.NoRightAir

.NoRightAir
.NoAirJordan
.CheckInAirCollision
	move.l	CL_Parameters(a6),a0
	move.w	2(a0),d0
	move.w	#-1,d1
	bsr	SimplePlatformPlayerBackgroundHorizontalCollision

	tst.l	BB_YSpeed(a3)
	beq	.Exit
	bmi	.DoTopCheck

	tst.b	BB_Direction(a3)
	bne	.AddOrSub
	move.b	#127,BB_Direction(a3)
	bra	.CheckLand
.AddOrSub
	bmi	.Sub
	add.b	#32,BB_Direction(a3)
	bra	.CheckLand
.Sub
	sub.b	#32,BB_Direction(a3)

.CheckLand
	move.l	CL_Parameters(a6),a0
	move.w	(a0),d0		;Last Space block
	move.w	#-1,d1
	bsr	SimplePlatformPlayerBackgroundLandCollision
	tst.w	d0
	beq	.Exit
	and.b	#$FE,CL_Data+1(a6)
	move.l	#0,BB_YSpeed(a3)
	bra	.Exit
.DoTopCheck

	tst.b	BB_Direction(a3)
	bne	.AddOrSub2
	move.b	#0,BB_Direction(a3)
	bra	.CheckTop
.AddOrSub2
	bpl	.Sub2
	add.b	#32,BB_Direction(a3)
	bra	.CheckTop
.Sub2
	sub.b	#32,BB_Direction(a3)

.CheckTop
	move.l	CL_Parameters(a6),a0
	move.w	2(a0),d0	;Last Floor Block
	move.w	#-1,d1
	bsr	SimplePlatformPlayerBackgroundTopCollision
	tst.w	d0
	beq	.Exit
	move.l	#0,BB_YSpeed(a3)
	bra	.Exit

.OnGround
	move.l	#0,BB_YSpeed(a3)

;	move.l	OB_InputChannel(a3),a0
;	move.w	IC_JoyResult(a0),d2
	move.l	CL_Parameters(a6),a0
	move.w	-6(a0),d0
	jsr	_ReadVirtualJoystick
	and.w	#$FF,d0
	move.w	d0,d2

	move.l	SS_Scroll(a2),a0

.CheckUp
	btst	#0,d2
	beq	.CheckDown
	bsr	.Up
	bra	.CheckLeft
.CheckDown
	btst	#1,d2
	beq	.CheckLeft
	bsr	.Down
.CheckLeft
	btst	#2,d2
	beq	.CheckRight
	bsr	.Left
	bra	.CheckFire
.CheckRight
	btst	#3,d2
	beq	.CheckFire
	bsr	.Right
.CheckFire

.CheckDeccel
	tst.w	d2
	bne	.CheckonGroundCollisions
	move.l	SS_Scroll(a2),a0
	move.l	BB_XSpeed(a3),d0
	cmp.l	SC_BaseSpeed(a0),d0
	beq	.CheckonGroundCollisions
	bpl	.XMinus
	bmi	.XPlus

.XMinus	move.l	BB_XDeccel(a3),d0
	sub.l	d0,BB_XSpeed(a3)
	move.l	BB_XSpeed(a3),d0
	cmp.l	SC_BaseSpeed(a0),d0
	bgt	.CheckonGroundCollisions
	move.l	SC_BaseSpeed(a0),BB_XSpeed(a3)
	bra	.CheckonGroundCollisions

.XPlus	move.l	BB_XDeccel(a3),d0
	add.l	d0,BB_XSpeed(a3)
	move.l	BB_XSpeed(a3),d0
	cmp.l	SC_BaseSpeed(a0),d0
	ble	.CheckonGroundCollisions
	move.l	SC_BaseSpeed(a0),BB_XSpeed(a3)

.CheckonGroundCollisions
	move.l	CL_Parameters(a6),a0
	move.w	2(a0),d0
	move.w	#-1,d1
	bsr	SimplePlatformPlayerBackgroundHorizontalCollision

	move.l	CL_Parameters(a6),a0
	move.w	(a0),d0
	move.w	#-1,d1
	bsr	SimplePlatformPlayerBackgroundLedgeCollision
	tst.w	d0
	bne	.Exit			;if no collision we are falling
	or.b	#1,CL_Data+1(a6)	;set state to in air

.Exit
	move.l	(sp)+,d2
	move.w	#0,d0
	rts

.Up
	or.b	#1,CL_Data+1(a6)
	move.l	CL_Data+2(a6),BB_YSpeed(a3)	;Jump speed

	move.l	CL_Parameters(a6),a0
	move.w	2(a0),d0	;Last Floor Block
	move.w	#-1,d1
	bsr	SimplePlatformPlayerBackgroundTopCollision
	tst.w	d0
	beq	.UpExit

	and.b	#$FE,CL_Data+1(a6)
	move.l	#0,BB_YSpeed(a3)

.UpExit
	rts
.Down
	rts
.Left
	move.b	#192,BB_Direction(a3)
	move.w	#1,d2
	move.l	BB_XSpeed(a3),d0
	add.l	BB_XMaxSpeed(a3),d0
	sub.l	SC_BaseSpeed(a0),d0
	bgt	.LeftOk
	move.l	BB_XMaxSpeed(a3),d0
	neg.l	d0
	move.l	d0,BB_XSpeed(a3)
	bra	.LeftExit
.LeftOk
	move.l	BB_XAccel(a3),d0
	sub.l	d0,BB_XSpeed(a3)
.LeftExit
	rts
.Right
	move.b	#64,BB_Direction(a3)
	move.w	#1,d2
	move.l	BB_XSpeed(a3),d0
	sub.l	BB_XMaxSpeed(a3),d0
	sub.l	SC_BaseSpeed(a0),d0
	bmi	.RightOk
	move.l	BB_XMaxSpeed(a3),BB_XSpeed(a3)
	bra	.RightExit
.RightOk
	move.l	BB_XAccel(a3),d0
	add.l	d0,BB_XSpeed(a3)
.RightExit
	rts




SimplePlatformPlayerBackgroundHorizontalCollision
	;Inputs:	a2.l = Slice
	;		a3.l = Object
	;		d0.w = LastSpace Block
	;		d1.w = Bitshifts -1 = stop

	movem.l	d2-d7/a4-a5,-(sp)

	move.w	d0,d4		;d4.w = Last Space Block
	move.w	d1,d5		;d5.l = Bit Shifts (-1 = No Bit Shifts)

	move.l	SS_Scroll(a2),a4	;a4.l = Scroll Struct
	move.l	OB_Image(a3),a5		;a5.l = Object Image
	cmp.l	#0,a5
	beq	.Exit

	tst.l	BB_XSpeed(a3)
	beq	.CheckLeftAndRight
	bpl	.GoingRight
.GoingLeft
	bsr	.Left
	bra	.Exit
.GoingRight
	bsr	.Right
	bra	.Exit
.CheckLeftAndRight
	bsr	.Left
	bsr	.Right
.Exit
	movem.l	(sp)+,d2-d7/a4-a5
	move.w	#0,d0
	rts


.Left
	move.l	OB_YPos(a3),d3
	add.l	OB_AnimYOffset(a3),d3
	add.l	IM_ColTop(a5),d3
.LLoop
	move.l	OB_XPos(a3),d0
	add.l	OB_AnimXOffset(a3),d0
	add.l	IM_ColLeft(a5),d0
	add.l	BB_XSpeed(a3),d0
	move.l	d3,d1

	bsr	_PointToBackground

	cmp.w	d4,d0
	blt	.LNope

	move.l	BB_XSpeed(a3),d0
	move.l	#0,BB_XSpeed(a3)
	tst.w	d5
	bmi	.LeftExit
	neg.l	d0
	asr.l	d5,d0
	move.l	d0,BB_XSpeed(a3)
	bra	.LeftExit
.LNope
	move.l	OB_YPos(a3),d1
	add.l	OB_AnimYOffset(a3),d1
	add.l	IM_ColBottom(a5),d1
	cmp.l	d1,d3
	beq	.LeftExit
	add.l	#16<<BITSHIFT,d3
	cmp.l	d1,d3
	ble	.LLoop
	move.l	d1,d3
	bra	.LLoop
.LeftExit
	rts

.Right
	move.l	OB_YPos(a3),d3
	add.l	OB_AnimYOffset(a3),d3
	add.l	IM_ColTop(a5),d3
.RLoop
	move.l	OB_XPos(a3),d0
	add.l	OB_AnimXOffset(a3),d0
	add.l	IM_ColRight(a5),d0
	add.l	BB_XSpeed(a3),d0
	move.l	d3,d1

	bsr	_PointToBackground

	cmp.w	d4,d0
	blt	.RNope

	move.l	BB_XSpeed(a3),d0
	move.l	#0,BB_XSpeed(a3)
	tst.w	d5
	bmi	.RightExit
	neg.l	d0
	asr.l	d5,d0
	move.l	d0,BB_XSpeed(a3)
	bra	.RightExit
.RNope
	move.l	OB_YPos(a3),d1
	add.l	OB_AnimYOffset(a3),d1
	add.l	IM_ColBottom(a5),d1
	cmp.l	d1,d3
	beq	.RightExit
	add.l	#16<<BITSHIFT,d3
	cmp.l	d1,d3
	ble	.RLoop
	move.l	d1,d3
	bra	.RLoop
.RightExit
	rts



SimplePlatformPlayerBackgroundLedgeCollision
	;Inputs:	a2.l = Slice
	;		a3.l = Object
	;		d0.w = LastSpace Block
	;		d1.w = Bitshifts -1 = stop
	;Outputs	d0.w - 1 = Collision

	movem.l	d2-d7/a4-a5,-(sp)

	move.w	d0,d4			;d4.w = Last Space Block
	move.w	d1,d5			;d5.l = Bit Shifts (-1 = No Bit Shifts)

	move.l	SS_Scroll(a2),a4	;a4.l = Scroll Struct
	move.l	OB_Image(a3),a5		;a5.l = Object Image
	cmp.l	#0,a5
	beq	.Exit

.CheckLedge

	move.l	OB_XPos(a3),d2
	add.l	OB_AnimXOffset(a3),d2
	add.l	IM_ColLeft(a5),d2
.Loop
	move.l	OB_YPos(a3),d1
	add.l	OB_AnimYOffset(a3),d1
	add.l	IM_ColBottom(a5),d1
	add.l	#32,d1
	move.l	d2,d0

	bsr	_PointToBackground

	cmp.w	d4,d0
	blt	.UNope

	move.w	#1,d0
	bra	.Exit
.UNope
	move.l	OB_XPos(a3),d1
	add.l	OB_AnimXOffset(a3),d1
	add.l	IM_ColRight(a5),d1

	cmp.l	d1,d2
	beq	.SetNExit

	add.l	#16<<BITSHIFT,d2
	cmp.l	d1,d2
	ble	.Loop
	move.l	d1,d2
	bra	.Loop
.SetNExit
	move.w	#0,d0
.Exit
	movem.l	(sp)+,d2-d7/a4-a5
	rts

SimplePlatformPlayerBackgroundLandCollision
	;Inputs:	a2.l = Slice
	;		a3.l = Object
	;		d0.w = LastSpace Block
	;		d1.w = Bitshifts -1 = stop
	;Outputs	d0.w - 1 = Collision

	movem.l	d2-d7/a4-a5,-(sp)

	move.w	d0,d4			;d4.w = Last Space Block
	move.w	d1,d5			;d5.l = Bit Shifts (-1 = No Bit Shifts)

	move.l	SS_Scroll(a2),a4	;a4.l = Scroll Struct
	move.l	OB_Image(a3),a5		;a5.l = Object Image
	cmp.l	#0,a5
	beq	.Exit

.CheckLand

	move.l	OB_XPos(a3),d2
	add.l	OB_AnimXOffset(a3),d2
	add.l	IM_ColLeft(a5),d2
.Loop
	move.l	OB_YPos(a3),d1
	add.l	OB_AnimYOffset(a3),d1
	add.l	BB_YSpeed(a3),d1
	add.l	IM_ColBottom(a5),d1
	move.l	d2,d0

	bsr	LandingPointToBackground

	cmp.w	d4,d0
	blt	.UNope

	move.l	OB_YPos(a3),d1			;If Collision When Obj+YSpeed &
	add.l	OB_AnimYOffset(a3),d1		;No Collision When Obj~+YSpeed
	add.l	IM_ColBottom(a5),d1		;Then We have landed
	move.l	d2,d0

	bsr	LandingPointToBackground
	cmp.w	d4,d0
	bgt	.UNope

	move.w	#1,d0
	bra	.Exit
.UNope
	move.l	OB_XPos(a3),d1
	add.l	OB_AnimXOffset(a3),d1
	add.l	IM_ColRight(a5),d1
	cmp.l	d1,d2
	beq	.SetNExit
	add.l	#16<<BITSHIFT,d2
	cmp.l	d1,d2
	ble	.Loop
	move.l	d1,d2
	bra	.Loop
.SetNExit
	move.w	#0,d0
.Exit
	movem.l	(sp)+,d2-d7/a4-a5
	rts

LandingPointToBackground
	;Just like normal but off bottom returns 255
	;Inputs:	a2.l = Slice
	;		d0.l = XPos
	;		d1.l = YPos
	;Outputs:	d0.w = BlockNo

	move.l	SS_Scroll(a2),a0

	tst.l	d0
	bmi	.Zero
	tst.l	d1
	bmi	.Zero

	lsr.l	#BITSHIFT,d0
	lsr.l	#4,d0
	lsr.l	#BITSHIFT,d1
	lsr.l	#4,d1

	cmp.w	SC_MapBlocksWide(a0),d0
	bge	.Zero
	cmp.w	SC_MapBlocksHigh(a0),d1
	bge	.TwoFiveFive

	mulu	SC_MapBlocksWide(a0),d1
	add.l	d1,d0
	lsl.l	#1,d0

	move.l	SC_MapBank(a0),a0
	add.l	d0,a0

	move.w	(a0),d0
	bra	.Exit
.Zero
	move.w	#0,d0
	bra	.Exit
.TwoFiveFive
	move.w	#255,d0
.Exit
	rts


SimplePlatformPlayerBackgroundTopCollision
	;Inputs:	a2.l = Slice
	;		a3.l = Object
	;		d0.w = LastSpace Block
	;		d1.w = Bitshifts -1 = stop
	;Outputs	d0.w - 1 = Collision

	movem.l	d2-d7/a4-a5,-(sp)

	move.w	d0,d4			;d4.w = Last Space Block
	move.w	d1,d5			;d5.l = Bit Shifts (-1 = No Bit Shifts)

	move.l	SS_Scroll(a2),a4	;a4.l = Scroll Struct
	move.l	OB_Image(a3),a5		;a5.l = Object Image
	cmp.l	#0,a5
	beq	.Exit

.Top
	move.l	OB_XPos(a3),d2
	add.l	OB_AnimXOffset(a3),d2
	add.l	IM_ColLeft(a5),d2
.Loop
	move.l	OB_YPos(a3),d1
	add.l	OB_AnimYOffset(a3),d1
	add.l	IM_ColTop(a5),d1
	add.l	BB_YSpeed(a3),d1
	move.l	d2,d0

	bsr	_PointToBackground

	cmp.w	d4,d0
	blt	.UNope

	move.w	#1,d0
	bra	.Exit
.UNope
	move.l	OB_XPos(a3),d0
	add.l	OB_AnimXOffset(a3),d0
	add.l	IM_ColRight(a5),d0
	cmp.l	d0,d2
	beq	.SetNExit
	add.l	#16<<BITSHIFT,d2
	cmp.l	d0,d2
	ble	.Loop
	move.l	d0,d2
	bra	.Loop
.UpExit
	rts
.SetNExit
	move.w	#0,d0
.Exit
	movem.l	(sp)+,d2-d7/a4-a5
	rts



;*************************************************************
_ScrollXCenterPlayers ;(action)
	;in: a2=slice, a3=bob,a5=params
	;params:
	;0	MaxScrollSpeed
	;4	TrackPlayer.w (0..3 = players, 4=all players)
	;6	reserved 6bytes
	;12


	;XYZZY - check for player existance and player borders

	move.l	SS_Scroll(a2),a0
	cmp.l	#0,a0
	beq	.InitExit

	move.l	(a5),SC_XData(a0)	;max spd
	move.l	(a5),SC_XData+4(a0)
	neg.l	SC_XData+4(a0)		;min speed

	move.w	4(a5),SC_XTrack(a0)	;player to track

	move.l	#.Update,SC_XTrackRoutine(a0)

.InitExit
	lea	12(a5),a5
	rts

.Update	;******** NOT A CHANNELROUTINE! ********
	;In: a2 = Slice, a3 = Scroll struct

	move.w	SC_XTrack(a3),d0
	cmp.w	#4,d0
	beq	.trackall
	jsr	_FindPlayer
	cmp.l	#0,a0
	beq	.Exit
	move.l	OB_XPos(a0),d0
	bra	.foundpos
.trackall
	bsr	FindAveragePlayerXPos

.foundpos
	sub.l	#160<<5,d0
	bpl	.checkxmax
	move.l	#0,d0
.checkxmax
	cmp.l	SC_RightBorder(a3),d0
	blt	.sussdir
	move.l	SC_RightBorder(a3),d0
.sussdir
	sub.l	SC_Pos(a3),d0
	cmp.l	SC_XData+4(a3),d0
	bge	.checkxspdmax
	move.l	SC_XData+4(a3),d0
.checkxspdmax
	cmp.l	SC_XData(a3),d0
	ble	.put
	move.l	SC_XData(a3),d0
.put
	move.l	d0,SC_Speed(a3)

.Exit
	rts


FindAveragePlayerXPos
	;in: a2 = slice struct
	;out: d0.l = average xpos of players
	move.l	d2,-(sp)

	move.l	#0,d0
	move.w	#0,d1
	move.l	SS_Players(a2),a0

	cmp.l	#0,a0
	beq	.Exit
.Loop
	add.w	#1,d1
	add.l	OB_XPos(a0),d0
	move.l	OB_Next(a0),a0
	cmp.l	#0,a0
	bne	.Loop

	cmp.w	#1,d1
	beq	.Exit
	cmp.w	#2,d1
	beq	.TwoPlayer
	cmp.w	#3,d1
	beq	.ThreePlayer
	cmp.w	#4,d1
	beq	.FourPlayer
.LotsaPlayers
.ThreePlayer	;only 1,2 and 4 players supported now...
	;XYZZY - long division here please (utilitiy.library?)
	move.l	#0,d0
	bra	.Exit
.FourPlayer
	asr.l	#2,d0
	bra	.Exit
.TwoPlayer
	asr.l	#1,d0
.Exit
	move.l	(sp)+,d2
	rts


;*************************************************************
_ScrollYCenterPlayers ;(action)
	;in: a2=slice, a3=bob,a5=params
	;params:
	;0	MaxScrollSpeed
	;4	TrackPlayer.w (0..3 = players, 4=all players)
	;6	reserved 6bytes
	;12


	;XYZZY - check for player existance and player borders

	move.l	SS_Scroll(a2),a0
	cmp.l	#0,a0
	beq	.InitExit

	move.l	(a5),SC_YData(a0)	;max spd
	move.l	(a5),SC_YData+4(a0)
	neg.l	SC_YData+4(a0)		;min speed

	move.w	4(a5),SC_YTrack(a0)	;player to track

	move.l	#.Update,SC_YTrackRoutine(a0)

.InitExit
	lea	12(a5),a5
	rts

.Update	;******** NOT A CHANNELROUTINE! ********
	;In: a2 = Slice, a3 = Scroll struct

	move.w	SC_YTrack(a3),d0
	cmp.w	#4,d0
	beq	.trackall
	jsr	_FindPlayer
	cmp.l	#0,a0
	beq	.Exit
	move.l	OB_YPos(a0),d0
	bra	.foundpos
.trackall
	bsr	FindAveragePlayerYPos
;	bra	.Exit

.foundpos
	move.l	SS_CenterLine(a2),d1
	asl.l	#BITSHIFT,d1
	sub.l	d1,d0
	bpl	.checkymax
	move.l	#0,d0
.checkymax
	move.l	SS_BottomScreenPos(a2),d1
	asl.l	#BITSHIFT,d1
	cmp.l	d1,d0
	blt	.put
	move.l	d1,d0
.put
	move.l	d0,SC_YPos(a3)

.Exit
	rts


FindAveragePlayerYPos
	;in: a2 = slice struct
	;out: d0.l = average ypos of players
	move.l	d2,-(sp)

	move.l	#0,d0
	move.w	#0,d1
	move.l	SS_Players(a2),a0

	cmp.l	#0,a0
	beq	.Exit
.Loop
	add.w	#1,d1
	add.l	OB_YPos(a0),d0
	move.l	OB_Next(a0),a0
	cmp.l	#0,a0
	bne	.Loop

	cmp.w	#1,d1
	beq	.Exit
	cmp.w	#2,d1
	beq	.TwoPlayer
	cmp.w	#3,d1
	beq	.ThreePlayer
	cmp.w	#4,d1
	beq	.FourPlayer
.LotsaPlayers
.ThreePlayer	;only 1,2 and 4 players supported now...
	;XYZZY - long division here please (utilitiy.library?)
	move.l	#0,d0
	bra	.Exit
.FourPlayer
	asr.l	#2,d0
	bra	.Exit
.TwoPlayer
	asr.l	#1,d0
.Exit
	move.l	(sp)+,d2
	rts


;*************************************************************
_ScrollModeConstantXSpeed ;(action)
	;in: a2=slice, a3=bob,a5=params

	;params:
	;0	ScrollSpeed.l
	;4	AllowedScrollBack.l
	;8	ScrollBackSpeed.l
	;12	ScrollBackZoneWidth.l
	;16	EndOfMapActionList
	;20	Flags.w
	;22

	move.l	SS_Scroll(a2),a0
	cmp.l	#0,a0
	beq	.noscroll

	move.l	a5,SC_XData(a0)
;	move.l	#0,SC_XData+4(a0)
	move.l	#.Update,SC_XTrackRoutine(a0)
	move.l	(a5),SC_Speed(a0)

.noscroll
	lea	22(a5),a5
	rts

	;SC_XData.l: params
	;SC_XData+4.l: scrollpos (excluding scrollback)

.Update
	;In: a2 = Slice, a3 = Scroll struct

	move.l	a5,-(sp)

	move.l	SC_XData(a3),a5	;get params
	tst.l	(a5)
	beq	.Exit
	bmi	.LeftScroll
.RightScroll
	move.l	SC_Pos(a3),d0
	cmp.l	SC_RightBorder(a3),d0
	bge	.EndOfMap

	;XYZZY - check multiple players here please
	move.l	SS_Players(a2),a0
	move.l	OB_XPos(a0),d0
	sub.l	#16<<5,d0
	sub.l	SC_Pos(a3),d0
	beq	.rt_stop
	bpl	.rt_forward

	;doing scrollback
	move.l	SC_XData+4(a3),d0
	sub.l	SC_Pos(a3),d0
	cmp.l	4(a5),d0		;check scrollback limit
	bge	.rt_stop
	move.l	8(a5),SC_Speed(a3)	;set speed to scrollback speed
	neg.l	SC_Speed(a3)
	bra	.Exit
.rt_stop
	move.l	#0,SC_Speed(a3)
	bra	.Exit

.rt_forward
	move.l	(a5),SC_Speed(a3)
	move.l	SC_Pos(a3),d0
	cmp.l	SC_XData+4(a3),d0
	ble	.Exit
	move.l	d0,SC_XData+4(a3)
	bra	.Exit

.LeftScroll
	;XYZZY - leftscrolling here please!
	bra	.Exit


.EndOfMap
	;finish scrolling
	move.l	#0,SC_XTrackRoutine(a3)
	move.l	#0,SC_Speed(a3)

	;execute endofmap actionlist if any
	move.l	16(a5),a5
	cmp.l	#0,a5
	beq	.Exit
	move.w	#CONTEXT_NONE,d0
	bsr	_DoActionList
.Exit
	move.l	(sp)+,a5
	rts

;*************************************************************

_InitXPushSliceTrack
;	bra	_ScrollModeCenterPlayers


	;Inputs:	a2.l = Slice
	;		a3.l = Bob
	;		a5.l = Paramlist
	;Player, Left,Right,Max

	move.l	SS_Scroll(a2),a0
	move.l	(a5)+,SC_XTrack(a0)
	move.l	SS_Players(a2),SC_XTrack(a0)	;hack to 1st player?
	move.l	(a5)+,SC_LeftTrackPos(a0)
	move.l	(a5)+,SC_RightTrackPos(a0)
	move.l	(a5)+,d0
	bne	.SetLimit
	move.l	#9999999,d0
.SetLimit
	move.l	d0,SC_XTrackMaxSpeed(a0)
	move.l	#_UpdateXPushSliceTrack,SC_XTrackRoutine(a0)
;	move.l	#1<<5,SC_Speed(a0)
	rts

_InitYPushSliceTrack
	;Inputs:	a2.l = Slice
	;		a3.l = Bob
	;		a5.l = Paramlist
	;Player, top, bottom, max

	move.l	SS_Scroll(a2),a0
	move.l	(a5)+,SC_YTrack(a0)
	move.l	(a5)+,SC_TopTrackPos(a0)
	move.l	(a5)+,SC_BottomTrackPos(a0)
	move.l	(a5)+,d0
	bne	.SetLimit
	move.l	#9999999,d0
.SetLimit
	move.l	d0,SC_YTrackMaxSpeed(a0)
	move.l	#_UpdateYPushSliceTrack,SC_YTrackRoutine(a0)
	rts

_UpdateXPushSliceTrack
	;Inputs:
	;		a2.l = Slice
	;		a3.l = Scroll

	move.l	d2,-(sp)

	move.l	SC_XTrack(a3),a0
	cmp.l	#0,a0
	beq	.Exit

	move.l	OB_XPos(a0),d0

	move.l	BB_XSpeed(a0),d2
	sub.l	SC_BaseSpeed(a3),d2
	beq	.KillSpeed
	bpl	.CheckRightPush
.CheckLeftPush
	move.l	SC_Pos(a3),d1
	add.l	SC_LeftTrackPos(a3),d1
	sub.l	d1,d0
	bpl	.KillSpeed

	move.l	BB_XSpeed(a0),d1
	;move.l	d0,d1
	abs.l	d1
	;and.l	#$FFFFFFE0,d1
	cmp.l	SC_XTrackMaxSpeed(a3),d1
	ble	.SetSpeed1
	move.l	SC_XTrackMaxSpeed(a3),d1
.SetSpeed1
	neg.l	d1
	move.l	d1,BB_XSpeed(a0)
	move.l	d1,SC_Speed(a3)
	bra	.Exit
.CheckRightPush
	move.l	SC_Pos(a3),d1
	add.l	SC_RightTrackPos(a3),d1
	sub.l	d0,d1
	bpl	.KillSpeed

	move.l	BB_XSpeed(a0),d1
	abs.l	d1
	;and.l	#$FFFFFFFE0,d1
	cmp.l	SC_XTrackMaxSpeed(a3),d1
	blt	.SetSpeed2
	move.l	SC_XTrackMaxSpeed(a3),d1
.SetSpeed2
	move.l	d1,BB_XSpeed(a0)
	move.l	d1,SC_Speed(a3)
	bra	.Exit
.KillSpeed
	move.l	SC_BaseSpeed(a3),SC_Speed(a3)
.Exit	
	move.l	(sp)+,d2
	rts

_UpdateYPushSliceTrack
	;Inputs:
	;		a2.l = Slice
	;		a3.l = Scroll
	;YPos is NOT shifted

	movem.l	d2,-(sp)

	move.l	SC_YTrack(a3),a0
	cmp.w	#OB_UPDATETYPE_VB,OB_UpdateType(a0)
	bne	.Set2ObjectY			;Is trackee being updated in VBlank

.Set2SeudoY
	move.l	SPR_SeudoY(a0),d0
	move.l	d0,d2
	asr.l	#BITSHIFT,d0
	bra	.TstSpeed

.Set2ObjectY
	cmp.w	#1,_Framerate
	beq	.Set
	tst.w	_SpritePosToggle
	bne	.Exit
.Set
	move.l	OB_YPos(a0),d0
	move.l	d0,d2
	asr.l	#BITSHIFT,d0

.TstSpeed
	tst.l	BB_YSpeed(a0)
	beq	.Exit
	bpl	.CheckBottomPush
.CheckTopPush
	move.l	SC_TopTrackPos(a3),d1
	add.l	SC_YPos(a3),d1
	asr.l	#BITSHIFT,d1
	cmp.l	d1,d0
	bgt	.Exit

	move.l	d2,d1		;OB_YPos(a0),d1
	sub.l	SC_TopTrackPos(a3),d1
	move.l	d1,SC_YPos(a3)
	asr.l	#BITSHIFT,d1
	bra	.CropYToScreen

.CheckBottomPush
	move.l	SC_BottomTrackPos(a3),d1
	add.l	SC_YPos(a3),d1
	asr.l	#BITSHIFT,d1
	cmp.l	d1,d0
	blt	.Exit

	move.l	d2,d1		;OB_YPos(a0),d1
	sub.l	SC_BottomTrackPos(a3),d1
	move.l	d1,SC_YPos(a3)
;	lsr.l	#BITSHIFT,d1

.CropYToScreen
	move.l	SC_YPos(a3),d0
	bpl	.NotOffTop
	move.l	#0,SC_YPos(a3)
	bra	.Exit
.NotOffTop
	move.l	SS_BottomScreenPos(a2),d1
	asl.l	#BITSHIFT,d1
	cmp.l	d1,d0
	blt	.Exit
	move.l	d1,SC_YPos(a3)
.Exit
	movem.l	(sp)+,d2
	rts

;*************************************************************


;*************************************************************


_InitLinearAccel
	;Inputs:	a1.l = Channel
	;		a3.l = Bob
	;		a5.l = Params

	move.w	16(a5),d0
	bne	.NoSetSpeeds
	move.l	(a5),BB_XSpeed(a3)
	move.l	4(a5),BB_YSpeed(a3)
.NoSetSpeeds
	move.l	8(a5),CL_Data(a1)		;BB_XAccel(a3)
	move.l	12(a5),CL_Data+4(a1)		;BB_YAccel(a3)

	lea	18(a5),a5
	move.l	#_UpdateLinearAccel,CL_UpdateCode(a1)
.Exit
	rts

;_InitLinearAccel2
	;Inputs:	a1.l = Channel
	;		a3.l = Bob
	;		a5.l = Params - XAccel, YAccel

;	move.l	(a5)+,CL_Data(a1)		;BB_XAccel(a3)
;	move.l	(a5)+,CL_Data+4(a1)		;BB_YAccel(a3)

;	move.l	#_UpdateLinearAccel,CL_UpdateCode(a1)
;.Exit
;	rts

_UpdateLinearAccel
	;Inputs:	a2.l = Slice
	;		a3.l = Bob


	move.l	CL_Data(a6),d0
	add.l	d0,BB_XSpeed(a3)
	move.l	CL_Data+4(a6),d0
	add.l	d0,BB_YSpeed(a3)

	move.l	#0,d7
.Exit	rts

;*************************************************************

_InitCyclicAnimation
	;Inputs:	a1.l = Channel
	;		a3.l = Bob
	;		a5.l = Params

	;anim.l,flags.b,startframe.b,repeats.w

	move.l	a5,CL_Parameters(a1)
	move.l	(a5)+,CL_Data(a1)
	move.w	(a5)+,d0
	and.w	#$FF,d0
	mulu	#12,d0
	move.w	d0,CL_Data+4(a1)
	move.w	(a5)+,CL_Data+6(a1)
	move.l	#.Update,CL_UpdateCode(a1)
	rts

.Update
	;inputs:	a6.l = Channel
	;		a3.l = Bob structure


	move.l	CL_Data(a6),a0		;anim pointer
	move.w	CL_Data+4(a6),d0

	move.l	(a0,d0.w),OB_Image(a3)
	move.l	4(a0,d0.w),d1
	move.l	d1,OB_AnimXOffset(a3)
	move.l	8(a0,d0.w),d1
	move.l	d1,OB_AnimYOffset(a3)

	add.w	#12,d0
	add.w	#12,CL_Data+4(a6)	;frame offset
	tst.l	(a0,d0.w)
	bne	.Exit
	move.w	#0,CL_Data+4(a6)
	tst.w	CL_Data+6(a6)		;Repeats
	beq	.Exit
	sub.w	#1,CL_Data+6(a6)
	bne	.Exit
	;kill channel next frame
	move.l	#.KillChannel,CL_UpdateCode(a6)
	move.l	CL_Parameters(a6),a0
	move.b	4(a0),d0		;flags
	btst	#0,d0			;null image?
	beq	.Exit
	move.l	#0,OB_Image(a3)
.Exit
	rts

.KillChannel
	move.w	#1,d7
	rts

;*************************************************************

_InitDir8Anim	;channelroutine
	;in: a1=channel, a2=slice, a3=bob, a5=params
	;0	Flags.w
	;	(bit0 = oneshotanims?
	;	bit1 = NullAnims pause?
	;	bit2 = pause at zero speed?
	;2	AnimArray.l[8]
	;34

	move.l	a5,CL_Parameters(a1)

	move.b	#5,CL_Data1+2(a1)	;shift down to 0..7
	move.b	#16,CL_Data1+3(a1)	;rotate 16 brabians
	move.w	#-1,CL_Data1(a1)	;invalid direction
	move.l	#0,CL_Data2(a1)		;no anim

	move.l	#UpdateDirAnim,CL_UpdateCode(a1)
	lea	34(a5),a5
	rts


_InitDir16Anim	;channelroutine
	;in: a1=channel, a2=slice, a3=bob, a5=params
	;0	Flags.w
	;	(bit0 = oneshotanims?
	;	bit1 = NullAnims pause?
	;	bit2 = pause at zero speed?
	;2	AnimArray.l[16]
	;66

	move.l	a5,CL_Parameters(a1)

	move.b	#4,CL_Data1+2(a1)	;shift down to 0..15
	move.b	#8,CL_Data1+3(a1)	;rotate 8 brabians
	move.w	#-1,CL_Data1(a1)	;invalid direction
	move.l	#0,CL_Data2(a1)		;no anim

	move.l	#UpdateDirAnim,CL_UpdateCode(a1)
	lea	66(a5),a5
	rts

_InitDir32Anim	;channelroutine
	;in: a1=channel, a2=slice, a3=bob, a5=params
	;0	Flags.w
	;	(bit0 = oneshotanims?
	;	bit1 = NullAnims pause?
	;	bit2 = pause at zero speed?
	;2	AnimArray.l[32]
	;130

	move.l	a5,CL_Parameters(a1)

	move.b	#3,CL_Data1+2(a1)	;shift down to 0..31
	move.b	#4,CL_Data1+3(a1)	;rotate 4 brabians
	move.w	#-1,CL_Data1(a1)	;invalid direction
	move.l	#0,CL_Data2(a1)		;no anim

	move.l	#UpdateDirAnim,CL_UpdateCode(a1)
	lea	130(a5),a5
	rts

_InitDir64Anim	;channelroutine
	;in: a1=channel, a2=slice, a3=bob, a5=params
	;0	Flags.w
	;	(bit0 = oneshotanims?
	;	bit1 = NullAnims pause?
	;	bit2 = pause at zero speed?
	;2	AnimArray.l[64]
	;258

	move.l	a5,CL_Parameters(a1)

	move.b	#2,CL_Data1+2(a1)	;shift down to 0..63
	move.b	#2,CL_Data1+3(a1)	;rotate 2 brabians
	move.w	#-1,CL_Data1(a1)	;invalid direction
	move.l	#0,CL_Data2(a1)		;no anim

	move.l	#UpdateDirAnim,CL_UpdateCode(a1)
	lea	258(a5),a5
	rts

UpdateDirAnim
	;in: a2=slice, a3=bob, a6=channel

;	CL_Data1+0.w: previous index
;	CL_Data1+2.b: shifts
;	CL_Data1+3.b: rotations
;	CL_Data2.l: base of current anim (null=no current anim)
;	CL_Data3.l: frameptr
	move.l	a5,-(sp)
	move.l	CL_Parameters(a6),a5

	move.w	(a5),d0
	btst	#2,d0
	beq	.SussDir
	;check for zero speed

	tst.l	BB_XSpeed(a3)
	bne	.SussDir
	tst.l	BB_YSpeed(a3)
	beq	.Exit

.SussDir
	;suss out current index
	move.b	BB_Direction(a3),d0
	add.b	CL_Data1+3(a6),d0
	and.w	#$FF,d0
	move.b	CL_Data1+2(a6),d1
	and.w	#$FF,d1
	lsr.w	d1,d0

	;change of direction?
	cmp.w	CL_Data1(a6),d0
	beq	.ContAnim	;no, just continue anim

	;new direction! oooooooooo...
	move.w	d0,CL_Data1(a6)

	lsl.w	#2,d0
	move.l	2(a5,d0.w),a0	;get anim for new dir
	cmp.l	#0,a0
	beq	.NullAnim	;ignore if null

	tst.l	(a0)		;empty anim?
	beq	.NullAnim	;treat as null

	move.l	a0,CL_Data2(a6)	;new anim base
	move.l	a0,CL_Data3(a6)	;new anim ptr
	bra	.ContAnim

.NullAnim
	move.w	(a5),d0
	btst	#1,d0
	beq	.ContAnim
	move.l	#0,CL_Data2(a6)	;stop anim
	bra	.Exit

.ContAnim
	tst.l	CL_Data2(a6)	;anim playing?
	beq	.Exit		;if nope, exit

	;get a new frame
	move.l	CL_Data3(a6),a0
	move.l	(a0)+,d0
	bne	.PutFrame
	;anim end

	move.w	(a5),d0
	btst	#0,d0
	beq	.Wrap

	;oneshot anims
	move.l	#0,CL_Data2(a6)	;stop on last frame
	bra	.Exit
.Wrap
	move.l	CL_Data2(a6),a0
	move.l	(a0)+,d0
.PutFrame
	move.l	d0,OB_Image(a3)
	move.l	(a0)+,OB_AnimXOffset(a3)
	move.l	(a0)+,OB_AnimYOffset(a3)
	move.l	a0,CL_Data3(a6)

.Exit
	move.l	(sp)+,a5
	rts


;*************************************************************

_OldInitDir8Anim
	;SYNTAX: Anim,startframe.w *8

	move.l	a5,CL_Parameters(a1)
	add.l	#6*8,a5
	move.b	#5,CL_Data(a1)
	move.b	#16,CL_Data+1(a1)
	move.l	#_OldUpdateDirAnim,CL_UpdateCode(a1)

	rts

_OldInitDir16Anim
	;SYNTAX: Anim,startframe.w *16

	move.l	a5,CL_Parameters(a1)
	add.l	#6*16,a5
	move.b	#4,CL_Data(a1)
	move.b	#8,CL_Data+1(a1)
	move.w	#0,CL_Data+2(a1)
	move.l	#0,CL_Data+4(a1)
	move.l	#_OldUpdateDirAnim,CL_UpdateCode(a1)

	rts

_OldInitDir32Anim
	;SYNTAX: Anim,startframe.w *32

	move.l	a5,CL_Parameters(a1)
	add.l	#6*32,a5
	move.b	#3,CL_Data(a1)
	move.b	#4,CL_Data+1(a1)
	move.w	#0,CL_Data+2(a1)
	move.l	#0,CL_Data+4(a1)
	move.l	#_OldUpdateDirAnim,CL_UpdateCode(a1)

	rts

_OldInitDir64Anim
	;SYNTAX: Anim,startframe.w, *64

	move.l	a5,CL_Parameters(a1)
	add.l	#6*64,a5
	move.b	#2,CL_Data(a1)
	move.b	#2,CL_Data+1(a1)
	move.w	#0,CL_Data+2(a1)
	move.l	#0,CL_Data+4(a1)
	move.l	#_OldUpdateDirAnim,CL_UpdateCode(a1)

	rts

_OldUpdateDirAnim
	;inputs:	a6.l = Channel Offset
	;		a3.l = Bob structure


	move.b	BB_Direction(a3),d0
	add.b	CL_Data+1(a6),d0

	move.b	CL_Data(a6),d1
	lsr.b	d1,d0

	and.w	#$FF,d0
	move.w	d0,d1
	lsl.w	#2,d0
	lsl.w	d1
	add.w	d1,d0
	and.l	#$FFFF,d0

	move.l	CL_Parameters(a6),a1
	add.l	d0,a1
	cmp.l	#0,(a1)
	beq	.Exit

	move.l	(a1)+,a0

	cmp.l	CL_Data+4(a6),a0
	beq	.SameAnim
	move.l	a0,CL_Data+4(a6)
	move.w	(a1)+,CL_Data+2(a6)
.SameAnim
	move.w	CL_Data+2(a6),d0
	
	move.l	(a0,d0.w),OB_Image(a3)
	move.l	4(a0,d0.w),d1
	move.l	d1,OB_AnimXOffset(a3)
	move.l	8(a0,d0.w),d1
	move.l	d1,OB_AnimYOffset(a3)

	add.w	#12,d0
	add.w	#12,CL_Data+2(a6)
	tst.l	(a0,d0.w)
	bne	.Exit
	move.w	#0,CL_Data+2(a6)
.Exit
	rts

;*************************************************************

_OldInitDir8Image
	;SYNTAX: Anim.l

	move.l	(a5)+,CL_Parameters(a1)
	move.b	#5,CL_Data(a1)
	move.b	#16,CL_Data+1(a1)
	move.l	#_UpdateDirImage,CL_UpdateCode(a1)

	rts


_OldInitDir16Image
	;SYNTAX: Anim.l

	move.l	(a5)+,CL_Parameters(a1)
	move.b	#4,CL_Data(a1)
	move.b	#8,CL_Data+1(a1)
	move.l	#_UpdateDirImage,CL_UpdateCode(a1)

	rts

_OldInitDir32Image
	;SYNTAX: Anim.l

	move.l	(a5)+,CL_Parameters(a1)
	move.b	#3,CL_Data(a1)
	move.b	#4,CL_Data+1(a1)
	move.l	#_UpdateDirImage,CL_UpdateCode(a1)

	rts

_OldInitDir64Image
	;SYNTAX: Anim.l

	move.l	(a5)+,CL_Parameters(a1)
	move.b	#2,CL_Data(a1)
	move.b	#2,CL_Data+1(a1)
	move.l	#_UpdateDirImage,CL_UpdateCode(a1)

	rts

_UpdateDirImage
	;inputs:	a6.l = Channel Offset
	;		a3.l = Bob structure

	move.b	BB_Direction(a3),d0
	add.b	CL_Data+1(a6),d0

	move.b	CL_Data(a6),d1
	lsr.b	d1,d0

	and.w	#$FF,d0
	move.w	d0,d1

	lsl.w	#3,d0
	lsl.w	#2,d1
	add.w	d1,d0

	move.l	CL_Parameters(a6),a0
	move.l	(a0,d0.w),OB_Image(a3)
	move.l	4(a0,d0.w),d1
	move.l	d1,OB_AnimXOffset(a3)
	move.l	8(a0,d0.w),d1
	move.l	d1,OB_AnimYOffset(a3)
.Exit
	rts



;*************************************************************



_InitDir8Image	;channelroutine
	;in: a1=channel, a2=slice, a3=bob, a5=params
	;0	array of 8 images
	;32	anim.l
	;36	flags.w
	;38
	move.l	a5,CL_Parameters(a1)
	move.b	#5,CL_Data(a1)
	move.b	#16,CL_Data+1(a1)
	move.l	32(a5),CL_Data2(a1)	;anim
	move.w	36(a5),CL_Data3(a1)	;flags
	move.l	#UpdateDirImage,CL_UpdateCode(a1)
	lea	38(a5),a5
	rts



_InitDir16Image
	;in: a1=channel, a2=slice, a3=bob, a5=params
	;0	array of 16 images
	;64	anim.l
	;68	flags.w
	;70
	move.l	a5,CL_Parameters(a1)
	move.b	#4,CL_Data(a1)
	move.b	#8,CL_Data+1(a1)
	move.l	64(a5),CL_Data2(a1)	;anim
	move.w	68(a5),CL_Data3(a1)	;flags
	move.l	#UpdateDirImage,CL_UpdateCode(a1)
	lea	70(a5),a5
	rts

_InitDir32Image
	;in: a1=channel, a2=slice, a3=bob, a5=params
	;0	anim.l
	;4	flags.w
	;6

	move.l	a5,CL_Parameters(a1)
	move.l	(a5),d0
	beq	.Exit

	move.l	d0,CL_Data2(a1)		;anim
	move.b	#3,CL_Data(a1)
	move.b	#4,CL_Data+1(a1)
	move.w	2(a5),CL_Data3(a1)	;flags

	move.l	#UpdateDirImage,CL_UpdateCode(a1)
.Exit
	lea	6(a5),a5
	rts

_InitDir64Image
	;in: a1=channel, a2=slice, a3=bob, a5=params
	;0	anim.l
	;4	flags.w
	;6

	move.l	a5,CL_Parameters(a1)
	move.l	(a5),d0
	beq	.Exit

	move.l	d0,CL_Data2(a1)		;anim
	move.b	#2,CL_Data(a1)
	move.b	#2,CL_Data+1(a1)
	move.w	2(a5),CL_Data3(a1)	;flags

	move.l	#UpdateDirImage,CL_UpdateCode(a1)
.Exit
	lea	6(a5),a5
	rts



UpdateDirImage
	;in: a2=slice, a3=bob, a6=channel
	move.l	a5,-(sp)
	move.l	CL_Parameters(a6),a5

	move.b	BB_Direction(a3),d0
	add.b	CL_Data+1(a6),d0

	move.b	CL_Data(a6),d1
	lsr.b	d1,d0
	and.w	#$FF,d0			;d0 = scaled direction index

	move.l	CL_Data2(a6),d1
	bne	.UseImageFromAnim

	lsl.w	#2,d0
	move.l	(a5,d0.w),d1		;get image
	beq	.FuckOffAndDie		;buggeroff if null
	move.l	d1,OB_Image(a3)
	move.l	#0,OB_AnimXOffset(a3)
	move.l	#0,OB_AnimYOffset(a3)
	bra	.FuckOffAndDie

.UseImageFromAnim
	move.l	d1,a0
	move.w	d0,d1

	lsl.w	#3,d0
	lsl.w	#2,d1
	add.w	d1,d0

	move.l	(a0,d0.w),OB_Image(a3)
	move.l	4(a0,d0.w),d1
	move.l	d1,OB_AnimXOffset(a3)
	move.l	8(a0,d0.w),d1
	move.l	d1,OB_AnimYOffset(a3)

.FuckOffAndDie
	move.l	(sp)+,a5
	rts


;*************************************************************

_InitDir8Action
	;SYNTAX: Anim.l

	move.l	a5,CL_Parameters(a1)
	move.b	#5,CL_Data(a1)
	move.b	#4,CL_Data+1(a1)
	move.l	#_UpdateDirAction,CL_UpdateCode(a1)
	lea	4(a5),a5

	rts

_InitDir16Action
	;SYNTAX: Anim.l

	move.l	a5,CL_Parameters(a1)
	move.b	#4,CL_Data(a1)
	move.b	#8,CL_Data+1(a1)
	move.l	#_UpdateDirAction,CL_UpdateCode(a1)
	lea	4(a5),a5

	rts

_InitDir32Action
	;SYNTAX: Anim.l

	move.l	a5,CL_Parameters(a1)
	move.b	#3,CL_Data(a1)
	move.b	#16,CL_Data+1(a1)
	move.l	#_UpdateDirAction,CL_UpdateCode(a1)
	lea	4(a5),a5

	rts

_InitDir64Action
	;SYNTAX: Anim.l

	move.l	a5,CL_Parameters(a1)
	move.b	#2,CL_Data(a1)
	move.b	#32,CL_Data+1(a1)
	move.l	#_UpdateDirAction,CL_UpdateCode(a1)
	lea	4(a5),a5

	rts

_UpdateDirAction
	;inputs:	a6.l = Channel Offset
	;		a3.l = Bob structure

	move.b	BB_Direction(a3),d0
	add.b	CL_Data+1(a6),d0

	move.b	CL_Data(a6),d1
	lsr.b	d1,d0
	and.w	#$FF,d0
	lsl.w	#2,d0

	move.l	a5,-(sp)
	move.l	CL_Parameters(a6),a0
	move.l	(a0,d0.w),a5
	cmp.l	#0,a5
	beq	.ActionDone

	move.w	#CONTEXT_BOB,d0
	bsr	_DoActionList
.ActionDone
	move.l	(sp)+,a5
.Exit
	rts


;*************************************************************

;XYZZY
_OLDInitXSpeedAnim
	;Plays anim depending on X speed
	;Params:   	LeftThreshold.l, LeftAnim.anim,
	;		RightThreshold.l, RightAnim.anim,
	;		NeutralAnim.anim

	move.l	(a5),CL_Data(a1)
	move.l	8(a5),CL_Data+4(a1)

	move.l	#0,CL_Data+8(a1)
	move.w	#0,CL_Data+12(a1)
	
	move.l	a5,CL_Parameters(a1)
	move.l	#.UpdateXSpeedAnim,CL_UpdateCode(a1)
	lea	20(a5),a5
	rts

.UpdateXSpeedAnim
	move.l	BB_XSpeed(a3),d0
	bra	UpdateSpeedAnim

;XYZZY
_OLDInitYSpeedAnim
	;Plays anim depending on X speed
	;Params:	UpThreshold.l, UpAnim.anim,
	;		DownThreshold.l, DownAnim.anim,
	;		NeutralAnim.anim

	move.l	(a5),CL_Data(a1)
	move.l	8(a5),CL_Data+4(a1)

	move.l	#0,CL_Data+8(a1)
	move.w	#0,CL_Data+12(a1)

	move.l	a5,CL_Parameters(a1)
	move.l	#.UpdateYSpeedAnim,CL_UpdateCode(a1)
	lea	20(a5),a5
	rts

.UpdateYSpeedAnim
	move.l	BB_YSpeed(a3),d0
	;FALL THROUGH

;XYZZY - kill	
UpdateSpeedAnim
	move.l	CL_Parameters(a6),a0
	move.l	CL_Data+8(a6),d1	;Current anim
	tst.w	d0
	bpl	.Right
.Left
	neg.l	d0
	sub.l	CL_Data(a6),d0
	bmi	.CheckStationaryAnim
.CheckLeftAnim
	cmp.l	4(a0),d1
	beq	.UpdateAnim
	move.l	4(a0),CL_Data+8(a6)	;Install left anim
	move.w	#0,CL_Data+12(a6)	;Zero anim frame
	bra	.UpdateAnim
.Right
	sub.l	CL_Data(a6),d0
	bmi	.CheckStationaryAnim
.CheckRightAnim
	cmp.l	12(a0),d1
	beq	.UpdateAnim
	move.l	12(a0),CL_Data+8(a6)	;Install right anim
	move.w	#0,CL_Data+12(a6)		;Zero anim frame
	bra	.UpdateAnim
.CheckStationaryAnim
	cmp.l	16(a0),d1
	beq	.UpdateAnim
	move.l	16(a0),d0
	beq	.UpdateAnim
	move.l	d0,CL_Data+8(a6)	;Install Stationary anim
	move.w	#0,CL_Data+12(a6)	;Zero anim frame

.UpdateAnim
	move.l	CL_Data+8(a6),a0
	cmp.l	#0,a0
	beq	.Exit
	move.w	CL_Data+12(a6),d0
	
	move.l	(a0,d0.w),OB_Image(a3)
	move.l	4(a0,d0.w),d1
	move.l	d1,OB_AnimXOffset(a3)
	move.l	8(a0,d0.w),d1
	move.l	d1,OB_AnimYOffset(a3)

	add.w	#12,d0
	add.w	#12,CL_Data+12(a6)
	tst.l	(a0,d0.w)
	bne	.Exit
	move.w	#0,CL_Data+12(a6)
.Exit
	rts



;*************************************************************

_InitHomerBadDude1
	;Inertial homer
	;Inputs:	a1.l = Channel
	;		a2.l = Slice
	;		a3.l = Bob
	;		a5.l = Paramlist

	;SYNTAX:	Type.w, PlayerRef.l, XMaxSpeed.l, YMaxSpeed.l, XAccel.l, YAccel.l

	;TYPES:			0 = Player
	;			1 = Nearest Player
	;			2 = Nearest BD

	move.l	6(a5),BB_XMaxSpeed(a3)
	move.l	10(a5),BB_YMaxSpeed(a3)
	move.l	14(a5),BB_XAccel(a3)
	move.l	18(a5),BB_YAccel(a3)

	move.w	(a5),d0
	beq	.PlayerType
	cmp.w	#1,d0
	beq	.NearestPlayerType
.NearestBDType
	move.l	SS_BadDudes(a2),a0
	beq	.Exit
	move.l	a1,-(sp)
	bsr	_FindNearestObject
	move.l	(sp)+,a1
	cmp.l	#0,a0
	beq	.Exit
	bset.b	#3,BB_Flags+3(a0)
	move.l	a0,CL_Data(a1)
	move.l	#UpdateHomerBadDude1,CL_UpdateCode(a1)

	bra	.Exit
.NearestPlayerType
	move.l	SS_Players(a2),a0
	beq	.Exit
	move.l	a1,-(sp)
	bsr	_FindNearestObject
	move.l	(sp)+,a1
	cmp.l	#0,a0
	beq	.Exit
	bset.b	#3,BB_Flags+3(a0)
	move.l	a0,CL_Data(a1)
	move.l	#UpdateHomerBadDude1,CL_UpdateCode(a1)

	bra	.Exit
.PlayerType
	move.l	2(a5),a0
;	bset.b	#3,BB_Flags+3(a0)
	move.l	a0,CL_Data(a1)
	move.l	#UpdateHomerBadDude1,CL_UpdateCode(a1)
.Exit
	lea	22(a5),a5
	rts


UpdateHomerBadDude1
	;D-Oh
	;Inertial homing
	;CL_Data = Object to home to

	move.l	CL_Data(a6),a1
	cmp.l	#0,a1
	beq	.OhFuckThisCouldGetMessy

	;Simple minded check to see if target is still there.
	cmp.w	#0,OB_State(a1)
	beq	.OhFuckThisCouldGetMessy

;	btst.b	#3,BB_Flags+3(a1)
;	beq	.OhFuckThisCouldGetMessy

	move.l	OB_XPos(a3),d0
	sub.l	OB_XPos(a1),d0

	bmi	.Right
	bpl	.Left
.CheckVertical
	move.l	CL_Data(a6),a1
	move.l	OB_YPos(a3),d0
	sub.l	OB_YPos(a1),d0

	bmi	.Down
	bpl	.Up
.Exit
	rts


.OhFuckThisCouldGetMessy
	move.w	#1,d7		;remove channelroutine
	bra	.Exit

.Right
	move.l	BB_XSpeed(a3),d0
	add.l	BB_XAccel(a3),d0

	abs.l	d0
	cmp.l	BB_XMaxSpeed(a3),d0
	bls	.RightOk
.RightMax
	move.l	BB_XMaxSpeed(a3),BB_XSpeed(a3)
	bra	.CheckVertical
.RightOk
	move.l	BB_XAccel(a3),d0
	add.l	d0,BB_XSpeed(a3)
	bra	.CheckVertical
.Left
	move.l	BB_XSpeed(a3),d0
	sub.l	BB_XAccel(a3),d0

	abs.l	d0
	cmp.l	BB_XMaxSpeed(a3),d0
	bls	.LeftOk
.LeftMax
	move.l	BB_XMaxSpeed(a3),BB_XSpeed(a3)
	neg.l	BB_XSpeed(a3)
	bra	.CheckVertical
.LeftOk
	move.l	BB_XAccel(a3),d0
	sub.l	d0,BB_XSpeed(a3)
	bra	.CheckVertical
.Up
	move.l	BB_YSpeed(a3),d0
	sub.l	BB_YAccel(a3),d0

	abs.l	d0
	cmp.l	BB_YMaxSpeed(a3),d0
	bls	.UpOk
.UpMax
	move.l	BB_YMaxSpeed(a3),BB_YSpeed(a3)
	neg.l	BB_YSpeed(a3)
	bra	.Exit
.UpOk
	move.l	BB_YAccel(a3),d0
	sub.l	d0,BB_YSpeed(a3)
	bra	.Exit
.Down
	move.l	BB_YSpeed(a3),d0
	add.l	BB_YAccel(a3),d0

	abs.l	d0
	cmp.l	BB_YMaxSpeed(a3),d0
	bls	.DownOk
.DownMax
	move.l	BB_YMaxSpeed(a3),BB_YSpeed(a3)
	bra	.Exit
.DownOk
	move.l	BB_YAccel(a3),d0
	add.l	d0,BB_YSpeed(a3)
	bra	.Exit





_FindNearestObject
	;In:	a0 = list of dest objects to search
	;	a3 = src object
	;Out:	a0 = nearest object

	move.l	#0,d1
	move.l	d1,a1
.Loop
	tst.w	OB_State(a0)
	beq	.GetNext

	move.l	OB_XPos(a0),d0
	sub.l	OB_XPos(a3),d0

	add.l	OB_YPos(a0),d0
	sub.l	OB_YPos(a3),d0
	abs.l	d0
	cmp.l	d0,d1
	bgt	.GetNext

	move.l	d0,d1
	move.l	a0,a1
.GetNext
	move.l	OB_Next(a0),a0
	cmp.l	#0,a0
	bne	.Loop

	move.l	a1,a0
	rts






;*************************************************************

_InitDirectionalHomer
	;Sets the direction field to track an object. Use directional
	;animation and/or movement to get homing
	;SYNTAX:	Type.w, PlayerRef.l, Quadrants.w, HomelessAction

	;TYPES:			0 = Player
	;			1 = Nearest Player
	;			2 = Nearest BD
	;			3 = First Active BD

	move.w	6(a5),CL_Data+4(a1)
	move.w	(a5),d0
	beq	.PlayerType
	cmp.w	#1,d0
	beq	.NearestPlayerType
	cmp.w	#2,d0
	beq	.NearestBDType

.NearestBDType
	move.l	SS_BadDudes(a2),a0
	beq	.Exit
	bsr	.GetClosest

	cmp.l	#0,a0
	beq	.Exit
	move.l	a0,CL_Data(a1)
	move.l	#_UpdateDirectionalHomer,CL_UpdateCode(a1)

	bra	.Exit
.NearestPlayerType
	move.l	SS_Players(a2),a0
	beq	.Exit
	bsr	.GetClosest

	cmp.l	#0,a0
	beq	.Exit
	move.l	a0,CL_Data(a1)
	move.l	#_UpdateDirectionalHomer,CL_UpdateCode(a1)

	bra	.Exit
.PlayerType
	move.l	2(a5),CL_Data(a1)
	move.l	#_UpdateDirectionalHomer,CL_UpdateCode(a1)
.Exit
	lea	12(a5),a5
	rts

.GetClosest
	;a0.l = List to search

	move.l	#0,d1
	move.l	a4,-(sp)
.Loop
	tst.w	OB_State(a0)
	beq	.GetNext

	move.l	OB_XPos(a0),d0
	sub.l	OB_XPos(a3),d0

	add.l	OB_YPos(a0),d0
	sub.l	OB_YPos(a3),d0
	abs.l	d0
	cmp.l	d0,d1
	bgt	.GetNext

	move.l	d0,d1
	move.l	a0,a4
.GetNext
	move.l	OB_Next(a0),a0
	cmp.l	#0,a0
	bne	.Loop

	move.l	a4,a0
	move.l	(sp)+,a4
	rts

_UpdateDirectionalHomer
	movem.l	a4/d2,-(sp)
	move.l	CL_Data(a6),a4
	move.w	CL_Data+4(a6),d2
	bsr	AHoopyHomingRoutine
	movem.l	(sp)+,a4/d2
.Exit
	rts

;*************************************************************

AHoopyHomingRoutine
	;a directional (16 diferent angles) homing routine
	;calculates a new bearing on object

	;inputs:
	;a3   = Homer
	;a4   = Homee
	;d2.w = Quadrants


	move.b	BB_Direction(a3),d0
	lsr.b	#4,d0
	and.w	#$FF,d0
	move.w	d0,-(sp)

	move.l	OB_XPos(a3),d0
	move.l	OB_YPos(a3),d1
	bsr	FindHeading
	move.w	d0,d1
	move.w	(sp),d0
	and.w	#$FF,d0
	bsr	FindRotateDir

	add.w	(sp)+,d0
	and.w	#$F,d0

	lea	QuadConstrainTable,a0
	move.b	(a0,d0.w),d1
	and.w	#$FF,d1
	and.w	d1,d2
	beq	.Exit

.SetDir
	lsl.b	#4,d0
	move.b	d0,BB_Direction(a3)
.Exit
	rts

FindHeading
	;determine the direction of a line from a given point toward
	;anouther object

	;d0,d1 = bob coords to take heading from

	;returns direction (0-15) in d0

	move.w	d2,-(sp)		;save d2

	move.w	#0,d2			;zero the running index thingy

	sub.l	OB_XPos(a4),d0		;calc deltax
	bpl	FH_DXPos		;positive?
	neg.w	d0			;nope, make it positive...
	bset	#3,d2			;...and set dx bit in index
FH_DXPos

	sub.l	OB_YPos(a4),d1		;calc deltay
	bpl	FH_DYPos		;positive?
	neg.w	d1			;nope, make it positive...
	bset	#2,d2			;...and set dy bit in index
FH_DYPos
	move.w	d1,a1			;save dy

	;0
	lsr.w	#1,d1			;get DY/2
	cmp.w	d1,d0			;is DX < DY/2 ?
	blo	FH_0			;yep, we are in area 0

	move.w	a1,d1			;get DY
	cmp.w	d1,d0			;is DX < DY ?
	blo	FH_1			;yep, we are in area 1

	lsl.w	#1,d1			;get DY*2
	cmp.w	d1,d0			;is DX < DY*2 ?
	blo	FH_2			;yep, we are in area 2

	;else, we are in area 3.
	or.w	#%0011,d2
	bra	FH_GetDir
FH_2
	or.w	#%0010,d2
	bra	FH_GetDir

FH_1
	or.w	#%0001,d2
	bra	FH_GetDir
FH_0
	;bits 0,1 already set to zero, so no action needed

FH_GetDir
	move.w	a1,d1
	add.w	d0,d1

	lsl.w	#1,d2			;use as word index
	lea	FH_Directions,a0	;get addr of table
	move.w	(a0,d2.w),d0		;get direction

	move.w	(sp)+,d2		;restore d2
	rts


FindRotateDir
	;determines the quickest way to rotate to get to a given heading
	;(ie is clockwise or anticlockwise the shortest route?)

	;inputs:
	;d0 = Current heading (0-15)
	;d1 = desired heading (0-15)

	;outputs:
	;d0 = -1 if anticlockwise
	;      0 if no movement needed (ie already facing right way)
	;     +1 if clockwise

	cmp.w	d0,d1
	beq	.NoRotate

	sub.w	d0,d1
	and.w	#$000F,d1
	cmp.w	#8,d1
	ble	.ClockWise

	;anticlockwise
	move.w	#-1,d0
	rts
.ClockWise
	move.w	#1,d0
	rts
.NoRotate
	move.w	#0,d0
	rts


;*************************************************************



_StandardDecelleration

	tst.w	d2
	bne	.MovedX
	move.l	SS_Scroll(a2),a0
	move.l	BB_XSpeed(a3),d0
	cmp.l	SC_BaseSpeed(a0),d0
	beq	.MovedX
	bpl	.XMinus
	bmi	.XPlus 
.MovedX	tst.w	d3
	bne	.Exit
	cmp.l	#0,BB_YSpeed(a3)
	beq	.Exit
	bpl	.YMinus
	bmi	.YPlus
	bra	.Exit

.XMinus	move.l	BB_XAccel(a3),d0
	sub.l	d0,BB_XSpeed(a3)
	move.l	BB_XSpeed(a3),d0
	cmp.l	SC_BaseSpeed(a0),d0
	bpl	.MovedX
	move.l	SC_BaseSpeed(a0),BB_XSpeed(a3)
	bra	.MovedX

.XPlus	move.l	BB_XAccel(a3),d0
	add.l	d0,BB_XSpeed(a3)
	cmp.l	SC_BaseSpeed(a0),d0
	bmi	.MovedX
	move.l	SC_BaseSpeed(a0),BB_XSpeed(a3)
	bra	.MovedX

.YMinus	move.l	BB_YAccel(a3),d0
	sub.l	d0,BB_YSpeed(a3)
	bpl	.Exit
	move.l	#0,BB_YSpeed(a3)
	bra	.Exit
.YPlus	move.l	BB_YAccel(a3),d0
	add.l	d0,BB_YSpeed(a3)
	bmi	.Exit
	move.l	#0,BB_YSpeed(a3)
.Exit	move.l	#0,d0
	rts

;*************************************************************

_ColourCycle
	;Colour cycles range of colours one place down
	;SYNTAX Start Colour.w, No of Colours.w, Flags.w

	move.w	(a5),d0
	move.w	2(a5),d2

	lsl.w	#3,d0			;Get offset into copperlist
	and.l	#$FFFF,d0

	move.l	SS_CCColours(a2),a0
	add.l	d0,a0
	move.l	a0,a1			;Save for later

	move.w	2(a0),d1
.Loop
	move.w	6(a0),d0		;Save out 2nd colour
	move.w	d1,6(a0)		;Stuff 1st into 2nd
	add.l	#4,a0
	sub.w	#1,d2
	beq	.LastOne
	move.w	6(a0),d1		;Save out 3rd
	move.w	d0,2(a0)		;Stuff 2nd into 3rd
	add.l	#4,a0
	sub.w	#1,d2
	beq	.LastOne_d1
	bra	.Loop
.LastOne_d1
	move.w	d1,d0
.LastOne
	move.w	d0,2(a1)
.Exit
	rts


;*************************************************************

_InitFloorSentry
	;In:	a1.l = Channel
	;	a2.l = Slice
	;	a3.l = Bob
	;	a5.l = Paramlist
	;Params:
	;0	LSpd.fixed
	;4	RSpd.fixed
	;8	EdgeOfTheRoad.eotr
	;12	HighestSpace.b
	;13	Flags.b	bit0 = Update Direction?
	;		bit1 = Start Left?
	;		bit2 = Check Floor?
	;14


	move.l	#16<<BITSHIFT,CL_Data1(a1)
	bra	InitXSentry
	rts

_InitCeilingSentry
	;In:	a1.l = Channel
	;	a2.l = Slice
	;	a3.l = Bob
	;	a5.l = Paramlist
	;Params:
	;0	LSpd.fixed
	;4	RSpd.fixed
	;8	EdgeOfTheRoad.eotr
	;12	HighestSpace.b
	;13	Flags.b	bit0 = Update Direction?
	;		bit1 = Start Left?
	;		bit2 = Check Ceiling?
	;14

	move.l	#-16<<BITSHIFT,CL_Data1(a1)
	;FALL THROUGH

InitXSentry
	move.l	#XSentryUpdateRoutine,CL_UpdateCode(a1)

	;save params for later
	move.l	a5,CL_Parameters(a1)

	btst.b	#1,13(a5)
	beq	.StartRight
	;StartLeft
	move.l	0(a5),BB_XSpeed(a3)
	neg.l	BB_XSpeed(a3)
	btst.b	#0,13(a5)
	beq	.Done
	move.b	#192,BB_Direction(a3)
	bra	.Done
.StartRight
	move.l	4(a5),BB_XSpeed(a3)
	btst.b	#0,13(a5)
	beq	.Done
	move.b	#64,BB_Direction(a3)
.Done
	lea	14(a5),a5
	rts

XSentryUpdateRoutine
	;In:	a6.l = Channel
	;	a2.l = Slice
	;	a3.l = Bob
	;	CL_Data1 = +16<<BITSHIFT for floorsentry
	;		 = -16<<BITSHIFT for ceilingsentry

	movem.l	d2-d3/a4-a5,-(sp)

	move.l	CL_Parameters(a6),a5

	;get image
	move.l	OB_Image(a3),a4
	cmp.l	#0,a4
	beq	.Done

	tst.l	CL_Data1(a6)
	bmi	.CeilingHugger

	;else, floorcrawler
	;find bottom ypos of image
	move.l	OB_YPos(a3),d3
	add.l	OB_AnimYOffset(a3),d3
	add.l	IM_ColBottom(a4),d3
	bra	.foo
.CeilingHugger
	;find top ypos of image
	move.l	OB_YPos(a3),d3
	add.l	OB_AnimYOffset(a3),d3
	add.l	IM_ColTop(a4),d3


.foo
	tst.l	BB_XSpeed(a3)
	beq	.Done
	bpl	.GoingRight

	;Going Left

	move.l	OB_XPos(a3),d2
	add.l	BB_XSpeed(a3),d2
	add.l	OB_AnimXOffset(a3),d2
	add.l	IM_ColLeft(a4),d2
	bra	.CheckForWall

.GoingRight
	move.l	OB_XPos(a3),d2
	add.l	BB_XSpeed(a3),d2
	add.l	OB_AnimXOffset(a3),d2
	add.l	IM_ColRight(a4),d2

.CheckForWall
	move.l	d2,d0
	move.l	d3,d1
	bsr	_PointToBackground

	move.l	8(a5),a0		;get EOTR
	cmp.w	EOTR_NumOfBlocks(a0),d0
	bhs	.Done

	move.b	EOTR_Data(a0,d0.w),d0
	cmp.b	12(a5),d0
	bhi	.Boing

.MaybeCheckFloor
	btst.b	#2,13(a5)
	beq	.Done

	move.l	d2,d0
	move.l	d3,d1
;	add.l	#16<<BITSHIFT,d1	;Down a block
	add.l	CL_Data1(a6),d1		;Up or Down a block
	bsr	_PointToBackground

	move.l	8(a5),a0		;get EOTR
	move.b	EOTR_Data(a0,d0.w),d0
	cmp.b	12(a5),d0
	bhi	.Done

.Boing
	tst.l	BB_XSpeed(a3)
	bmi	.GoRight

	move.l	0(a5),BB_XSpeed(a3)
	neg.l	BB_XSpeed(a3)
	btst.b	#0,13(a5)
	beq	.Done
	move.b	#192,BB_Direction(a3)
	bra	.Done
.GoRight
	move.l	4(a5),BB_XSpeed(a3)
	btst.b	#0,13(a5)
	beq	.Done
	move.b	#64,BB_Direction(a3)
	bra	.Done

.Done
	movem.l	(sp)+,d2-d3/a4-a5
	rts



;************************************************************


_InitLinearPlayer
	;In:	a1.l = Channel
	;	a2.l = Slice
	;	a3.l = Bob
	;	a5.l = Paramlist
	;Params:
	;0	Joystick.w
	;2	XSpd.fixed
	;6	YSpd.fixed
	;10	FireActionList.l
	;14	Flags.w (0=autofirerepeat, 1=fireallweaponbays, 2=lockdirection )
	;16	SetDirection.w (0=8way,1=left/right,2=up/down,3=no)
	;18

	move.l	a5,CL_Parameters(a1)
	move.l	#.UpdateRoutine,CL_UpdateCode(a1)
	move.w	#0,CL_Data1(a1)		;prev joystick value.
	lea	18(a5),a5
	rts

.UpdateRoutine
	;In:	a6.l = Channel
	;	a2.l = Slice
	;	a3.l = Bob

	movem.l	d2/a5,-(sp)
	move.l	CL_Parameters(a6),a5

	;Read joystick
	move.w	(a5),d0
	jsr	_ReadVirtualJoystick
	move.b	d0,d2

	move.l	#0,BB_YSpeed(a3)
	move.l	#0,BB_XSpeed(a3)

;chkup
	btst	#0,d2
	beq	.chkdn
	move.l	6(a5),BB_YSpeed(a3)
	neg.l	BB_YSpeed(a3)
	bra	.chklf

.chkdn
	btst	#1,d2
	beq	.chklf
	move.l	6(a5),BB_YSpeed(a3)

.chklf
	btst	#2,d2
	beq	.chkrt
	move.l	2(a5),BB_XSpeed(a3)
	neg.l	BB_XSpeed(a3)
	bra	.updatedir

.chkrt
	btst	#3,d2
	beq	.updatedir
	move.l	2(a5),BB_XSpeed(a3)

.updatedir

;**** SET DIRECTION
	move.w	d2,d1

	move.w	16(a5),d0
	beq	.SetDir_DoIt
	cmp.w	#3,d0		;no?
	beq	.SetDir_Done
	cmp.w	#1,d0		;Left/Right only?
	beq	.SetDir_LeftRight
	cmp.w	#2,d0		;Up/Down only?
	bne	.SetDir_Done
;Up/Down
	and.b	#%00000011,d1
	bra	.SetDir_DoIt
.SetDir_LeftRight
	and.b	#%00001100,d1
.SetDir_DoIt
	move.w	14(a5),d0	;check for robotron flag
	btst	#2,d0
	beq	.SetDir_ReallyDoIt

	btst	#4,d2
	bne	.SetDir_Done	;don't change dir if button if down

.SetDir_ReallyDoIt
	and.w	#$F,d1
	beq	.SetDir_Done	;no dirs pressed?
	lea	_JoyDirTable,a0
	move.b	(a0,d1.w),BB_Direction(a3)
.SetDir_Done


;**** Check fire button

	move.b	d2,d0
	move.w	14(a5),d1
	btst	#0,d1		;autofirerepeat?
	bne	.FCheck_AutoRepeat

	move.w	CL_Data1(a6),d1
	btst	#4,d1		;fire down in previous frame?
	bne	.FCheck_Done

.FCheck_AutoRepeat
	btst	#4,d0
	beq	.FCheck_Done

	;BANG.

	move.w	14(a5),d0
	btst	#1,d0		;fire all weapon bays?
	beq	.FCheck_DoActionList

	move.w	#$FFFF,d0	;_ALL_ of them.
	jsr	_InternalFireMultiWeaponBays

.FCheck_DoActionList
	move.l	10(a5),d0	;Fire ActionList
	beq	.FCheck_Done
	move.l	a5,-(sp)
	move.l	d0,a5
	move.w	#CONTEXT_BOB,d0
	bsr	_DoActionList
	move.l	(sp)+,a5

.FCheck_Done

.done
	move.w	d2,CL_Data1(a6)
	movem.l	(sp)+,d2/a5
	rts


;******************************************************************

		SECTION	Constants,DATA

;******************************************************************

_JoyAnimTable
	dc.w	00,04,08,0,12,28,32,0,16,20,24
	dc.w	0,0,0,0,0
	dc.w	36,40,44,0,48,64,68,0,52,56,60
	dc.w	0,0,0,0,0

FH_Directions
	dc.w	15,14,13,12,8,9,10,11,0,1,2,3,7,6,5,4

QuadConstrainTable
	dc.b	9,1,1,1,3,2,2,2,6,4,4,4,12,8,8,8
	even

_ResultToDirTable
;	dc.w	16,00,04,17,06,17,02,01,03,17,17,17,17
;	dc.w	17,16,08,12,17,14,15,13,17,10,09,11

	dc.b	00,01,05,00,07,08,06,00,03,02,04,00,00,00,00,00
	dc.b	09,10,14,00,16,17,15,00,12,11,13,00,00,00,00,00
