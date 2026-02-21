
	incdir	"PConk:inc/"
	include	"source/libs.gs"
	include	"source/Header.i"
	include	"source/Constants.i"
	include	"source/Structures.i"
	include	"source/Lame.i"

	XDEF	_DirectionMove
	XDEF	_OLDDirectionMove
	XDEF	_InitChannelExecute
	XDEF	_InitDirectionMove
	XDEF	_InitBGPointCollision
	XDEF	_InitBGCollision
	XDEF	_InitTrackPlayer
	XDEF	_FindHeading
	XDEF	_InitJitterSpeed
	XDEF	_SetRND
	XDEF	_AddRND
	XDEF	_RestrictDirection
	XDEF	_InitPlatformPlayer
	XDEF	_InitInertialPlayer
	XDEF	_InitAsteroidsPlayer
	XDEF	_InitMousePlayer
	XDEF	_InitActionListPlayer
	XDEF	_InitCheckFireButtons
	XDEF	_InitXSpeedAnim,_InitYSpeedAnim
	XDEF	_InitInertialHomer
	XDEF	_RandomEvent,_InitRandomEvent
	XDEF	_InitMousePointer

	XDEF	_ULongToBCD
	XDEF	_GenericCheckFire
	XDEF	_FindPlayer,_FindNearestPlayer

	XDEF	_SineTable,_CosineTable,_SetSpeedsFromDirection
	XDEF	_ATanTable

	XREF	_ActionContext
	;from Ponk.s
	XREF	_Random,_ConstrainedRandom
	;From BobStuff.s
	XREF	_DoActionList,_PointToBackground

	;From Operators.s
	XREF	_GetVariableTable,_SetVariableTable,_AddVariableTable

	;From Action.s
	XREF	_ReadVirtualJoystick
	XREF	_VirtualMice
	XREF	_JoyDirTable
	XREF	_FirstSlice

	;FromActionRoutines.s
	XREF	_InternalFireWeaponBay,_InternalFireMultiWeaponBays



;action routines
;in:	a2 = slice
;	a3 = bob
;	a4 = other bob (for collisions)
;	a5 = parameters
;out:	d7 = error, else leave unchanged

;channelinit routines
;in:	a1 = channel struct
;	a2 = slice
;	a3 = bob
;	a5 = parameters
;out:	d7 = error, else leave unchanged
;	a5 = updated parameter ptr

;channelupdate routines
;in:	a2 = slice
;	a3 = bob
;	a6 = channel struct
;out:	d7 = error, else leave unchanged


************************************************************************
* ACTIONROUTINES/CHANNELROUTINES
************************************************************************

**********************************
*InitChannelExecute
*Executes the given actionlist every nth cycle (0 delay = every cycle).
*

	IFNE	CHIPASSEMBLE
	SECTION	WhatLovelyCode,CODE_C
	ELSE
	SECTION	WhatLovelyCode,CODE
	ENDC

_InitChannelExecute
;in:	a1 = channel struct
;	a2 = slice
;	a3 = bob
;	a5 = parameters:
;		LONG ActionList to execute
;		WORD delay (number of cycles between updates, 0 = none)
;out:	a5 = updated parameter ptr

	lea	.Update,a0
	move.l	a0,CL_UpdateCode(a1)
	move.l	a5,CL_Parameters(a1)
	move.w	#0,CL_Data(a1)		;CL_Data used for delay counter
	lea	4+2(a5),a5		;skip params
	rts

.Update
;in:	a2 = slice,a3 = bob,a6 = channel struct
;out:	none

	sub.w	#1,CL_Data(a6)		;timer done?
	bpl	.Exit
	move.l	CL_Parameters(a6),a0
	move.w	4(a0),CL_Data(a6)	;reset timer

	move.l	a5,-(sp)
	move.l	(a0),a5			;get actionlist

	move.w	#CONTEXT_BOB,d0
	jsr	_DoActionList		;execute (on current bob/slice)
	move.l	(sp)+,a5
.Exit
	rts


**********************************
* DirectionMove (actionroutine)
* Sets a bob moving in the direction it is facing (BB_Direction).
* This routine takes the speed magnitude (a parameter) and direction,
* and works out what the x & y speeds should be.

;XYZZY - kill sometime

_OLDDirectionMove
;type:	Actionroutine
;in:	a2 = slice
;	a3 = bob
;	a5 = parameters:
;		LONG Speed Magnitude
;out:	d7 = error, else leave unchanged

	move.l	(a5)+,d1
;	cmp.l	#0,d1
;	bne	.UseMag
;	move.l	BB_DirSpeed(a3),d1
;.UseMag
	move.b	BB_Direction(a3),d0
	bsr	_SetSpeedsFromDirection
	move.l	d0,BB_XSpeed(a3)
 	move.l	d1,BB_YSpeed(a3)
	rts


_DirectionMove
;type:	Actionroutine
;in:	a2 = slice
;	a3 = bob
;	a5 = parameters:
;	0 Speed Magnitude
;	4 Flags (bit 0 = add to speeds?)
;	6

;out:	d7 = error, else leave unchanged

	move.l	(a5)+,d1
	move.w	(a5)+,d0
	btst	#0,d0
	beq	.Absolute

	move.b	BB_Direction(a3),d0
	bsr	_SetSpeedsFromDirection
	add.l	d0,BB_XSpeed(a3)
 	add.l	d1,BB_YSpeed(a3)
	rts

.Absolute
	move.b	BB_Direction(a3),d0
	bsr	_SetSpeedsFromDirection
	move.l	d0,BB_XSpeed(a3)
 	move.l	d1,BB_YSpeed(a3)
	rts


**********************************
_SetRND
;type:	Action
;in:	a1 = channel struct, a2 = slice, a3 = bob, a5 = params
;parameters:
;	0	MinValue
;	4	MaxValue
;	8	Dest Var
;	10
;out:	a5 = updated parameter ptr

	move.l	(a5)+,d0
	move.l	(a5)+,d1

	bsr	_ConstrainedRandom
	move.w	d0,d1
	ext.l	d1

	move.w	(a5)+,d0
	lsl.w	#2,d0
	lea	_SetVariableTable,a0
	move.l	(a0,d0.w),a0
	jsr	(a0)

	rts



**********************************
_AddRND
;type:	Action
;in:	a1 = channel struct, a2 = slice, a3 = bob, a5 = params
;parameters:
;	0	MinValue
;	4	MaxValue
;	8	Dest Var
;	10
;out:	a5 = updated parameter ptr

	move.l	(a5)+,d0
	move.l	(a5)+,d1

	bsr	_ConstrainedRandom
	move.w	d0,d1
	ext.l	d1

	move.w	(a5)+,d0
	lsl.w	#2,d0

	lea	_AddVariableTable,a0
	move.l	(a0,d0.w),a0
	jsr	(a0)

	rts


**********************************
_RestrictDirection
;type:	Action
;in:	a2 = slice, a3 = bob, a5 = params
;parameters:
;	0	Type.w: leftright,updown,4way,8way
;	2	MinAngle.l
;	6	MaxAngle.l
;	10
;out:	a5 = updated parameter ptr

	move.b	BB_Direction(a3),d1
	move.w	(a5),d0		;get type
	bra	.MinMax
	cmp.w	#1,d0
	beq	.LeftRight
	cmp.w	#2,d0
	beq	.UpDown
	cmp.w	#3,d0
	beq	.FourWay
	cmp.w	#4,d0
	beq	.EightWay

	bra	.Exit

.LeftRight
	and.b	#%10000000,d1
	add.b	#64,d1
	bra	.Exit
.UpDown
	add.b	#64,d1
	and.b	#%10000000,d1
	bra	.Exit
.FourWay
	and.b	#%11000000,d1
	bra	.Exit
.EightWay
	and.b	#%11100000,d1
	bra	.Exit


.MinMax
	movem.l	d2-d3,-(sp)
	move.b	BB_Direction(a3),d0	;get dir
	move.b	2+3(a5),d1	;get min
	move.b	6+3(a5),d2	;get max

	sub.b	d1,d0
	sub.b	d1,d2

	cmp.b	d0,d2
	bcc 	.inside

	move	#255,d3
	sub.b	d2,d3
	asr	#1,d3
	add.b	d2,d3

	cmp.b	d0,d3
	bcc	.closertomax
	bra	.MinMaxDone

.closertomax
	add.b	d2,d1
	bra	.MinMaxDone

.inside
	move.b	BB_Direction(a3),d1

.MinMaxDone	;result in d1
	movem.l	(sp)+,d2-d3

.Exit
	move.b	d1,BB_Direction(a3)
	lea	10(a5),a5
	rts







**********************************

_InitXSpeedAnim
	;in: a2=slice,a3=bob,a5=params
	;Plays anim depending on X speed
	;Params:
	;0  	LeftThreshold.l
	;4	LeftAnim.anim
	;8	RightThreshold.l
	;12	RightAnim.anim
	;16	NeutralAnim.anim
	;20

	move.l	a5,CL_Parameters(a1)
	move.l	#0,CL_Data1(a1)		;animbase (or null)
	move.l	#0,CL_Data2(a1)		;animptr
	lea	.Update,a0
	move.l	a0,CL_UpdateCode(a1)
	lea	20(a5),a5
	rts

.Update
	move.l	BB_XSpeed(a3),d0
	bra	UpdateVarAnim


_InitYSpeedAnim
	;in: a2=slice,a3=bob,a5=params
	;Plays anim depending on Y speed
	;Params:
	;0  	UpThreshold.l
	;4	UpAnim.anim
	;8	DownThreshold.l
	;12	DownAnim.anim
	;16	NeutralAnim.anim
	;20

	move.l	a5,CL_Parameters(a1)
	move.l	#0,CL_Data1(a1)		;animbase (or null)
	move.l	#0,CL_Data2(a1)		;animptr
	lea	.Update,a0
	move.l	a0,CL_UpdateCode(a1)
	lea	20(a5),a5
	rts

.Update
	move.l	BB_YSpeed(a3),d0

	;fall through

UpdateVarAnim
	;for _InitXSpeedAnim, _InitYSpeedAnim
	;in:	a2=slice,a3=bob
	;	a6=channel, d0.l = value to use to select anim

	move.l	a5,-(sp)
	move.l	CL_Parameters(a6),a5

	move.l	4(a5),a1	;low anim
	cmp.l	(a5),d0		;below low threshold?
	blt	.selected

	move.l	12(a5),a1	;high anim
	cmp.l	8(a5),d0	;above upper threshold?
	bgt	.selected

	move.l	16(a5),a1	;neutral anim
.selected
	cmp.l	CL_Data1(a6),a1	;already playing?
	beq	.Exit
	cmp.l	#0,a1		;null?
	beq	.Exit

	lea	CL_Data1(a6),a0
	bsr	_InternalInstallAnim
.Exit
	;update current anim
	lea	CL_Data1(a6),a0
	bsr	_InternalDoCyclicAnim
	move.l	(sp)+,a5
	rts


**********************************

_InitInertialHomer
;in:	a1 = channel struct, a2 = slice, a3 = bob, a5 = parameters
;params:
;0	Target.w	(0..3 = player)
;2	Reserved.w
;4	Flags.w		(bit0 = tracknearestplayer?)
;6	XAccel.l
;10	YAccel.l
;14	MaxXSpd.l
;18	MaxYSpd.l
;22

	move.l	a5,CL_Parameters(a1)
	lea	.Update,a0
	move.l	a0,CL_UpdateCode(a1)
	lea	22(a5),a5
	rts

.Update
	;in: a2=slice,a3=bob,a6=channel
	move.l	a5,-(sp)
	move.l	CL_Parameters(a6),a5

	move.w	4(a5),d0
	btst	#0,d0		;tracknearestplayer?
	bne	.FindNearest

	move.w	0(a5),d0
	bsr	_FindPlayer
	cmp.l	#0,a0
	beq	.Bugger
	bra	.DoXTrack

.FindNearest
	bsr	_FindNearestPlayer
	cmp.l	#0,a0
	beq	.Bugger

.DoXTrack
	move.l	OB_XPos(a3),d0
	cmp.l	OB_XPos(a0),d0
	beq	.XTrackDone
	bgt	.LeftAccel

	;rightaccel
	move.l	BB_XSpeed(a3),d0
	add.l	6(a5),d0	;xaccel
	cmp.l	14(a5),d0	;maxxspd
	ble	.XPut
	move.l	14(a5),d0
	bra	.XPut

.LeftAccel
	move.l	14(a5),d1
	neg.l	d1		;minxspd
	move.l	BB_XSpeed(a3),d0
	sub.l	6(a5),d0	;xaccel
	cmp.l	d1,d0
	bge	.XPut
	move.l	d1,d0
.XPut
	move.l	d0,BB_XSpeed(a3)
.XTrackDone

	move.l	OB_YPos(a3),d0
	cmp.l	OB_YPos(a0),d0
	beq	.YTrackDone
	bgt	.UpAccel

	;downaccel
	move.l	BB_YSpeed(a3),d0
	add.l	10(a5),d0	;yaccel
	cmp.l	18(a5),d0	;maxyspd
	ble	.YPut
	move.l	18(a5),d0
	bra	.YPut

.UpAccel
	move.l	18(a5),d1
	neg.l	d1		;minyspd
	move.l	BB_YSpeed(a3),d0
	sub.l	10(a5),d0	;yaccel
	cmp.l	d1,d0
	bge	.YPut
	move.l	d1,d0
.YPut
	move.l	d0,BB_YSpeed(a3)
.YTrackDone

.Bugger
	move.l	(sp)+,a5
	rts


_FindPlayer
	;in: d0.w = playernum (0..3)
	;out: a0=player (or NULL)

	move.l	_FirstSlice,a0
	move.l	SS_Players(a0),a0
	cmp.l	#0,a0
	beq	.Exit
.LoopFindLast
	cmp.l	#0,OB_Next(a0)
	beq	.Loop
	move.l	OB_Next(a0),a0
	bra	.LoopFindLast

.Loop
	sub.w	#1,d0
	bmi	.Exit
	cmp.l	#0,a0
	beq	.Exit
	move.l	OB_Previous(a0),a0
	bra	.Loop
.Exit
	rts


_FindNearestPlayer
	;in: a3=object
	;out: a0=player (or NULL)

	movem.l	d2-d4,-(sp)

	move.l	OB_XPos(a3),d2
	move.l	OB_YPos(a3),d3
	move.l	#0,a0
	move.l	#$7FFFFFFF,d4

	move.l	_FirstSlice,a1
	move.l	SS_Players(a1),a1
.Loop
	cmp.l	#0,a1
	beq	.Exit

	move.l	OB_XPos(a1),d0
	sub.l	d2,d0
	abs.l	d0

	move.l	OB_YPos(a1),d1
	sub.l	d3,d1
	abs.l	d1

	add.l	d1,d0
	cmp.l	d4,d0
	bge	.Next
	move.l	d0,d4
	move.l	a1,a0

.Next
	move.l	OB_Next(a1),a1
	bra	.Loop

.Exit
	movem.l	(sp)+,d2-d4
	rts


**********************************
* InitDirectionMove (ChannelRoutine)
* Channelroutine version of DirectionMove.
* Delay parameter specifies the number of frames to wait between updates.


_InitDirectionMove
;in:	a1 = channel struct
;	a2 = slice
;	a3 = bob
;	a5 = parameters:
;		LONG Speed Magnitude
;		WORD delay (number of cycles between updates, 0 = none)
;out:	a5 = updated parameter ptr

	lea	.Update,a0
	move.l	a0,CL_UpdateCode(a1)
	move.l	a5,CL_Parameters(a1)
	move.l	(a5)+,d1
	beq	.LeaveDirSpeed
	move.l	d1,BB_DirSpeed(a3)
.LeaveDirSpeed
	move.w	(a5)+,CL_Data(a1)
	rts

.Update
;in:	a2 = slice,a3 = bob,a6 = channel struct
;out:	none

	sub.w	#1,CL_Data(a6)			;dec delay counter
	bpl	.Exit				;delay done?
	move.l	CL_Parameters(a6),a0
	move.w	4(a0),CL_Data(a6)		;reset delay counter
	move.l	BB_DirSpeed(a3),d1		;get speed magnitude
	move.b	BB_Direction(a3),d0
	bsr	_SetSpeedsFromDirection		;dooooooo it!
	move.l	d0,BB_XSpeed(a3)		;put speeds into bob
	move.l	d1,BB_YSpeed(a3)
.Exit
	rts




;**********************************
_InitBGPointCollision
	;In:	a1 = Channel
	;	a2 = Slice
	;	a3 = Bob
	;	a5 = Params
	;Params:
	;0	XOffset.f
	;4	YOffset.f
	;8	EdgeOfTheRoad.eotr
	;12	HighestSpaceValue.b
	;13	reserved.b
	;14	ActionList.l
	;18	DampingShifts.w	(0=stop, 1=none, 2=half, 3=quarter... )
	;20

	;XYZZY - Should generate warning if no map
	move.l	SS_Scroll(a2),a0
	cmp.l	#0,a0
	beq	.Abort
	move.l	SC_MapBank(a0),a0
	cmp.l	#0,a0
	beq	.Abort

	move.l	a5,CL_Parameters(a1)
	move.l	#BGPointCollision,CL_UpdateCode(a1)
.Abort
	lea	20(a5),a5
	rts


BGPointCollision
	;Inputs:	a2 = Slice
	;		a3 = Object
	;		a6 = Channel

	movem.l	d2-d4/a5,-(sp)

	move.l	CL_Parameters(a6),a5

	move.w	#0,d4			;clear BoingFlag
	move.l	OB_XPos(a3),d2
	add.l	(a5),d2			;Add XOffset
	move.l	OB_YPos(a3),d3
	add.l	4(a5),d3		;Add YOffset

;CheckHoriz
	move.l	d2,d0
	move.l	d3,d1
	add.l	BB_XSpeed(a3),d0
	jsr	_PointToBackground

	;check with eotr
	move.l	8(a5),a0		;get eotr
	cmp.l	#0,a0
	beq	.h_noeotr
	move.b	EOTR_Data(a0,d0.w),d0
	cmp.b	12(a5),d0		;compare to HighestSpaceVal
	bls	.CheckVert
	bra	.xboing
.h_noeotr
	tst.w	d0
	beq	.CheckVert
.xboing
	move.w	#1,d4			;set boingflag
	move.w	18(a5),d1		;dampingshifts
	beq	.xstop
	move.l	BB_XSpeed(a3),d0
	neg.l	d0			;boing!
	sub.w	#1,d1
	beq	.xput
	asr.l	d1,d0
	bra	.xput
.xstop
	move.l	#0,d0
.xput
	move.l	d0,BB_XSpeed(a3)

.CheckVert
	move.l	d2,d0
	move.l	d3,d1
	add.l	BB_YSpeed(a3),d1
	jsr	_PointToBackground

	;check with eotr
	move.l	8(a5),a0		;get eotr
	cmp.l	#0,a0
	beq	.v_noeotr
	move.b	EOTR_Data(a0,d0.w),d0
	cmp.b	12(a5),d0		;compare to HighestSpaceVal
	bls	.CheckBoth
	bra	.yboing
.v_noeotr
	tst.w	d0
	beq	.CheckBoth
.yboing
	move.w	#1,d4			;set boingflag
	move.w	18(a5),d1		;dampingshifts
	beq	.ystop
	move.l	BB_YSpeed(a3),d0
	neg.l	d0			;boing!
	sub.w	#1,d1
	beq	.yput
	asr.l	d1,d0
	bra	.yput
.ystop
	move.l	#0,d0
.yput
	move.l	d0,BB_YSpeed(a3)

	;Need to check both as well, to stop us getting caught
	;on the rare perfect corner collision. Bugger.
.CheckBoth
	move.l	d2,d0
	move.l	d3,d1
	add.l	BB_XSpeed(a3),d0
	add.l	BB_YSpeed(a3),d1
	jsr	_PointToBackground

	;check with eotr
	move.l	8(a5),a0		;get eotr
	cmp.l	#0,a0
	beq	.both_noeotr
	move.b	EOTR_Data(a0,d0.w),d0
	cmp.b	12(a5),d0		;compare to HighestSpaceVal
	bls	.CheckBoingFlag
	bra	.bothboing
.both_noeotr
	tst.w	d0
	beq	.CheckBoingFlag
.bothboing
	move.w	#1,d4			;set boingflag
	move.w	18(a5),d2		;dampingshifts
	beq	.bothstop
	move.l	BB_XSpeed(a3),d0
	neg.l	d0			;boing!
	move.l	BB_YSpeed(a3),d1
	neg.l	d1			;boing!
	sub.w	#1,d2
	beq	.yput
	asr.l	d2,d0
	asr.l	d2,d1
	bra	.yput
.bothstop
	move.l	#0,d0
	move.l	#0,d1
.bothput
	move.l	d0,BB_XSpeed(a3)
	move.l	d1,BB_YSpeed(a3)


.CheckBoingFlag
	tst.w	d4
	beq	.Exit

	move.l	14(a5),d0	;ActionList
	beq	.Exit
;	move.l	a5,-(sp)
	move.l	d0,a5
	move.w	#CONTEXT_BOB,d0
	bsr	_DoActionList
;	move.l	(sp)+,a5
.Exit
	movem.l	(sp)+,d2-d4/a5
	rts


;**********************************
_InitBGCollision
	;In:	a1 = Channel
	;	a2 = Slice
	;	a3 = Bob
	;	a5 = Params
	;Params:
	;0	EdgeOfTheRoad.eotr
	;4	HighestSpaceValue.b
	;5	reserved.b
	;6	ActionList.l
	;10	DampingShifts.w	(0=stop, 1=none, 2=half, 3=quarter... )
	;12

	;XYZZY - Should generate warning if no map
	move.l	SS_Scroll(a2),a0
	cmp.l	#0,a0
	beq	.Abort
	move.l	SC_MapBank(a0),a0
	cmp.l	#0,a0
	beq	.Abort

	move.l	a5,CL_Parameters(a1)
	move.l	#BGCollision,CL_UpdateCode(a1)
.Abort
	lea	12(a5),a5
	rts




BGCollision
	;Inputs:	a2 = Slice
	;		a3 = Object
	;		a6 = Channel

	movem.l	d2-d5/a4-a5,-(sp)
	move.l	CL_Parameters(a6),a5

	move.w	#0,d4			;clear boingflag

	move.l	OB_Image(a3),a4
	move.l	OB_XPos(a3),d5		;save xpos for ycheck
	move.l	d5,d0
	add.l	OB_AnimXOffset(a3),d0
	add.l	BB_XSpeed(a3),d0
	move.l	OB_YPos(a3),d1
	add.l	OB_AnimYOffset(a3),d1

	tst.l	BB_XSpeed(a3)
	beq	.YCheck
	bpl	.GoingRight

;GoingLeft
	add.l	IM_ColLeft(a4),d0
	add.l	IM_ColTop(a4),d1
	move.l	IM_ColHeight(a4),d3
	move.b	4(a5),d2		;HighSpaceVal
	move.l	0(a5),a0		;Eotr
	bsr 	_CheckVlineBGIntersection
	tst.w	d0
	beq	.YCheck

	add.l	#16<<BITSHIFT,d1
	sub.l	IM_ColLeft(a4),d1
	sub.l	OB_AnimXOffset(a3),d1
	bra	.XBoing
.GoingRight
	add.l	IM_ColRight(a4),d0
	add.l	IM_ColTop(a4),d1
	move.l	IM_ColHeight(a4),d3
	move.b	4(a5),d2		;HighSpaceVal
	move.l	0(a5),a0		;Eotr
	bsr 	_CheckVlineBGIntersection
	tst.w	d0
	beq	.YCheck

	sub.l	#1<<BITSHIFT,d1
	sub.l	IM_ColRight(a4),d1
	sub.l	OB_AnimXOffset(a3),d1

.XBoing
	move.w	#1,d4		;set boingflag
	move.w	10(a5),d2	;get dampingshifts
	beq	.XStop
	move.l	BB_XSpeed(a3),d1
	neg.l	d1
	sub.w	#1,d2
	beq	.XPut
	asr.l	d2,d1
.XPut
	move.l	d1,BB_XSpeed(a3)
	bra	.YCheck
.XStop
	;align to block edge
	move.l	d1,OB_XPos(a3)
	move.l	#0,BB_XSpeed(a3)

.YCheck
;	move.l	OB_XPos(a3),d0
	move.l	d5,d0			;get old xpos
	add.l	OB_AnimXOffset(a3),d0
	move.l	OB_YPos(a3),d1
	add.l	OB_AnimYOffset(a3),d1
	add.l	BB_YSpeed(a3),d1

	tst.l	BB_YSpeed(a3)
	beq	.CheckBoingFlag
	bpl	.GoingDown

;GoingUp
	add.l	IM_ColLeft(a4),d0
	add.l	IM_ColTop(a4),d1
	move.l	IM_ColWidth(a4),d3
	move.b	4(a5),d2		;HighSpaceVal
	move.l	0(a5),a0		;Eotr
	bsr 	_CheckHlineBGIntersection
	tst.w	d0
	beq	.CheckBoingFlag

	add.l	#16<<5,d1
	sub.l	IM_ColTop(a4),d1
	sub.l	OB_AnimYOffset(a3),d1
	bra	.YBoing

.GoingDown
	add.l	IM_ColLeft(a4),d0
	add.l	IM_ColBottom(a4),d1
	move.l	IM_ColWidth(a4),d3
	move.b	4(a5),d2		;HighSpaceVal
	move.l	0(a5),a0		;Eotr
	bsr 	_CheckHlineBGIntersection
	tst.w	d0
	beq	.CheckBoingFlag

	sub.l	#1<<5,d1
	sub.l	IM_ColBottom(a4),d1
	sub.l	OB_AnimYOffset(a3),d1

.YBoing
	move.w	#1,d4		;set boingflag
	move.w	10(a5),d2	;get dampingshifts
	beq	.YStop
	move.l	BB_YSpeed(a3),d1
	neg.l	d1
	sub.w	#1,d2
	beq	.YPut
	asr.l	d2,d1
;	cmp.l	#-16,d1
;	blt	.YPut
;	cmp.l	#16,d1
;	bgt	.YPut
;	move.l	#0,d1
.YPut
	move.l	d1,BB_YSpeed(a3)
	bra	.CheckBoingFlag
.YStop
	;align to block edge
	move.l	d1,OB_YPos(a3)
	move.l	#0,BB_YSpeed(a3)

.CheckBoingFlag
	tst.w	d4
	beq	.Exit

	move.l	6(a5),d0	;ActionList
	beq	.Exit
	move.l	d0,a5
	move.w	#CONTEXT_BOB,d0
	bsr	_DoActionList

.Exit
	movem.l	(sp)+,d2-d5/a4-a5
	rts


;**********************************
_InitTrackPlayer
	;In:	a1 = Channel
	;	a2 = Slice
	;	a3 = Bob
	;	a5 = Params
	;Params:
	;0	Player to track
	;4	Speed
	;8	TurnSpeed
	;12	Flags: bit0=direct tracking, bit1=nearestplayer
	;14

	;XYZZY - Should put in tracking for _nearest_ player

	move.l	a5,CL_Parameters(a1)
	move.l	#.Update,CL_UpdateCode(a1)
.Abort
	lea	14(a5),a5
	rts

.Update
	movem.l	d2/a5,-(sp)
	move.l	CL_Parameters(a6),a5

	move.l	(a5),a0		;get player
	bsr	_FindHeading

	btst.b	#0,13(a5)		;directtracking?
	bne	.DirectTrack
	move.b	d0,d1
	sub.b	BB_Direction(a3),d1	;target dir - current dir 
	beq	.Move			;already lined up?
	bmi	.AntiClockWise

	;ClockWise
	move.l	8(a5),d2		;get turnspeed
	cmp.b	d1,d2
	ble	.Rotate
	move.b	d1,d2
	bra	.Rotate
.AntiClockWise
	move.l	8(a5),d2
	neg.b	d2
	cmp.b	d1,d2
	bge	.Rotate
	move.b	d1,d2
.Rotate
	add.b	d2,BB_Direction(a3)
	bra	.Move

.DirectTrack
	move.b	d0,BB_Direction(a3)
.Move
	move.l	4(a5),d1	;get speed
	beq	.FuckOff
	move.b	BB_Direction(a3),d0
	bsr	_SetSpeedsFromDirection
	move.l	d0,BB_XSpeed(a3)
	move.l	d1,BB_YSpeed(a3)

.FuckOff
	movem.l	(sp)+,d2/a5
	rts

;**********************************
_InitPlatformPlayer
	;In:	a1 = Channel
	;	a2 = Slice
	;	a3 = Bob
	;	a5 = Params
	;Params:
	;0	InputChannel.w
	;2	XAccel.l
	;6	MaxXSpd.l
	;10	XDrag.l
	;14	ClimbSpeed.l
	;18	JumpSpeed.l
	;22	Gravity.l
	;26	MaxFallSpeed.l
	;30	EdgeOfTheRoad.l
	;34	Flags.w
	;36	FireActionList.l
	;40	CollisionLeft.l
	;44	CollisionTop.l
	;48	CollisionWidth.l
	;52	CollisionHeight.l
	;56
	;WalkLeftAnim,WalkRightAnim
	;FaceLeftAnim,FaceRightAnim
	;JumpLeftAnim,JumpRightAnim
	;ClimbUpAnim,ClimbDownAnim

	;88

	;XYZZY - Should generate warning if no map
	move.l	SS_Scroll(a2),a0
	cmp.l	#0,a0
	beq	.Abort
	move.l	SC_MapBank(a0),a0
	cmp.l	#0,a0
	beq	.Abort

	move.l	#0,BB_XSpeed(a3)
	move.l	#0,BB_YSpeed(a3)

	move.w	#0,CL_Data1(a1)		;set state to falling
	move.w	#2,CL_Data2(a1)		;FaceLeftAnim
	move.w	#-1,CL_Data2+2(a1)	;kick start anim system

	move.l	a5,CL_Parameters(a1)
	move.l	#UpdatePlatformPlayer,CL_UpdateCode(a1)
.Abort
	lea	88(a5),a5
	rts

	;CL_Data1.w holds state: 0=falling,1=walking,2=climbing
	;CL_Data2.w holds anim number
	;CL_Data2+2.w holds previous animnumber

UpdatePlatformPlayer
	;in: a2=slice, a3=object, a6=channel

	movem.l	d2-d3/a5,-(sp)
	move.l	CL_Parameters(a6),a5
	;all states

	;check state
	move.w	CL_Data1(a6),d0
	beq	.Falling
	cmp.w	#2,d0
	beq	.Climbing
	;else =1, walking

;Walking
	bsr	PlatformPlayerHorizWallCheck

	;Check floor
	move.l	#1<<5,BB_YSpeed(a3)
	bsr	PlatformPlayerFloorCheck

	;don't need to move to block boundary - already there.
	move.l	#0,BB_YSpeed(a3)
	tst.w	d0
	bne	.Walk_CheckJoyStick

	;Start Falling...
	move.w	#0,CL_Data1(a6)
	bra	.UpdateAnim


.Walk_CheckJoyStick

	bsr	PlatformPlayerXDrag

	move.w	(a5),d0
	jsr	_ReadVirtualJoystick

	;check left
	btst	#2,d0
	beq	.Walk_CheckLeftDone
	move.w	#0,CL_Data2(a6)	;walkleftanim
	move.l	BB_XSpeed(a3),d1
	sub.l	2(a5),d1
	move.l	6(a5),d2	;max xspeed
	neg.l	d2
	cmp.l	d2,d1
	bge	.Walk_CheckLeftPut
	move.l	d2,d1
.Walk_CheckLeftPut
	move.l	d1,BB_XSpeed(a3)
.Walk_CheckLeftDone

	;check right
	btst	#3,d0
	beq	.Walk_CheckRightDone
	move.w	#1,CL_Data2(a6)	;walkrightanim
	move.l	BB_XSpeed(a3),d1
	add.l	2(a5),d1
	move.l	6(a5),d2	;max xspeed
	cmp.l	d2,d1
	ble	.Walk_CheckRightPut
	move.l	d2,d1
.Walk_CheckRightPut
	move.l	d1,BB_XSpeed(a3)
.Walk_CheckRightDone


	;check for up
;	btst	#0,d0
;	beq	.Walk_CheckUpDone
;	move.w	#2,CL_Data1(a6)		;into climb state
;	move.l	14(a5),d1		;climbspeed
;	neg.l	d1
;	move.l	d1,BB_YSpeed(a3)
;.Walk_CheckUpDone

	;check for jump
	btst	#0,d0			;up?
	beq	.Walk_CheckJumpDone
	move.w	#0,CL_Data1(a6)		;into fall state
	move.l	18(a5),d1
	neg.l	d1
	move.l	d1,BB_YSpeed(a3)
	bsr	PlatformPlayerCeilingCheck
.Walk_CheckJumpDone

	;check for fire
	move.l	36(a5),a0		;fire actionlist
	move.w	34(a5),d1		;flags
	and.w	#3,d1
	bsr	_GenericCheckFire	;joystick state already in d0.w
	;fire check done.

	tst.l	BB_XSpeed(a3)
	bne	.Walk_NotStopped
	and.w	#1,CL_Data2(a6)
	add.w	#2,CL_Data2(a6)
.Walk_NotStopped

	bra	.UpdateAnim

.Falling
	move.l	BB_YSpeed(a3),d0
;	cmp.l	18(a5),d0	;jumpspeed
;	bge	.Fall_AnimGoingDown

	;Fall_AnimGoingUp
	tst.l	BB_XSpeed(a3)
	beq	.Fall_AnimSussed
	bpl	.Fall_AnimUpRight
	move.w	#4,CL_Data2(a6)
	bra	.Fall_AnimSussed
.Fall_AnimUpRight
	move.w	#5,CL_Data2(a6)
	bra	.Fall_AnimSussed

.Fall_AnimGoingDown
	tst.l	BB_XSpeed(a3)
	beq	.Fall_AnimSussed
	bpl	.Fall_AnimDownRight
	move.w	#6,CL_Data2(a6)
	bra	.Fall_AnimSussed
.Fall_AnimDownRight
	move.w	#7,CL_Data2(a6)
.Fall_AnimSussed

	bsr	PlatformPlayerHorizWallCheck
	bsr	PlatformPlayerMoveLeftRight

	move.w	34(a5),d0		;flags
	btst	#2,d0			;no midair firing?
	bne	.Fall_FireDone

	;check for fire
	move.w	(a5),d0
	jsr	_ReadVirtualJoystick
	move.l	36(a5),a0		;fire actionlist
	move.w	34(a5),d1		;flags
	and.w	#3,d1
	bsr	_GenericCheckFire
.Fall_FireDone


	tst.l	BB_YSpeed(a3)
	beq	.Fall_DoGravity
	bpl	.Fall_CheckFloor

.Fall_CheckCeiling
	bsr	PlatformPlayerCeilingCheck
	bra	.Fall_DoGravity

.Fall_CheckFloor
	bsr	PlatformPlayerFloorCheck
	tst.w	d0
	beq	.Fall_DoGravity

	cmp.w	#3,d0
	beq	.Fall_HitFloor
;	cmp.w	#2,d0
;	bne	.Fall_HitFloor

	move.l	OB_Image(a3),a0
	move.l	OB_YPos(a3),d2
;	add.l	IM_ColBottom(a0),d2
	add.l	44(a5),d2		;top
	add.l	52(a5),d2		;height
;	add.l	OB_AnimYOffset(a3),d2
	move.l	d2,d3
	add.l	BB_YSpeed(a3),d3

	sub.l	#1<<5,d2

	asr.l	#5,d2
	asr.l	#4,d2

	asr.l	#5,d3
	asr.l	#4,d3

	cmp.l	d2,d3
	beq	.Fall_DoGravity

.Fall_HitFloor
	;hit floor - stop falling, start walking
	move.l	d1,OB_YPos(a3)
	;XYZZY - add actionroutine option here
	move.l	#0,BB_YSpeed(a3)
	move.w	#1,CL_Data1(a6)		;walk mode
;	move.w	#10,OB_Highlight(a3)
	bra	.UpdateAnim

.Fall_DoGravity
	move.l	BB_YSpeed(a3),d0
	add.l	22(a5),d0	;gravity
	cmp.l	26(a5),d0	;maxfallspeed
	ble	.Fall_DoGravityPut
	move.l	26(a5),d0
.Fall_DoGravityPut
	move.l	d0,BB_YSpeed(a3)
	bra	.UpdateAnim

.Climbing
	move.w	(a5),d0
	jsr	_ReadVirtualJoystick

	btst	#0,d0
	beq	.Climb_CheckUpDone
	move.l	14(a5),d1
	neg.l	d1
	move.l	d1,BB_YSpeed(a3)
.Climb_CheckUpDone

	btst	#1,d0
	beq	.Climb_CheckDownDone
	move.l	14(a5),d1
	move.l	d1,BB_YSpeed(a3)
.Climb_CheckDownDone

	btst	#2,d0
	beq	.Climb_CheckLeftDone
	move.w	#1,CL_Data1(a6)		;walk mode
.Climb_CheckLeftDone

	btst	#3,d0
	beq	.Climb_CheckRightDone
	move.w	#1,CL_Data1(a6)		;walk mode
.Climb_CheckRightDone

.UpdateAnim
	lea	CL_Data3(a6),a0
	move.w	CL_Data2(a6),d0
	cmp.w	CL_Data2+2(a6),d0
	beq	.NoNewAnim
	move.w	d0,CL_Data2+2(a6)
	lsl.w	#2,d0
	move.l	56(a5,d0.w),a1
	bsr	_InternalInstallAnim
	bra	.UpdateAnimDone
.NoNewAnim
	bsr	_InternalDoCyclicAnim
.UpdateAnimDone
	movem.l	(sp)+,d2-d3/a5
	rts


PlatformPlayerMoveLeftRight
	;out: d0.b = joystick status

	bsr	PlatformPlayerXDrag

.CheckJoyStick
	move.w	(a5),d0
	jsr	_ReadVirtualJoystick

	;check left
	btst	#2,d0
	beq	.CheckLeftDone
	move.l	BB_XSpeed(a3),d1
	sub.l	2(a5),d1
	move.l	6(a5),d2	;max xspeed
	neg.l	d2
	cmp.l	d2,d1
	bge	.CheckLeftPut
	move.l	d2,d1
.CheckLeftPut
	move.l	d1,BB_XSpeed(a3)
.CheckLeftDone

	;check right
	btst	#3,d0
	beq	.CheckRightDone
	move.l	BB_XSpeed(a3),d1
	add.l	2(a5),d1
	move.l	6(a5),d2	;max xspeed
	cmp.l	d2,d1
	ble	.CheckRightPut
	move.l	d2,d1
.CheckRightPut
	move.l	d1,BB_XSpeed(a3)
.CheckRightDone
	rts


PlatformPlayerXDrag
	;in: a2=slice,a3=bob,a5=platformplayer params

	move.w	(a5),d0
	jsr	_ReadVirtualJoystick
	and.b	#%00001100,d0
	bne	.Exit

	move.l	BB_XSpeed(a3),d0
	beq	.Exit
	bpl	.GoingRight

	add.l	10(a5),d0
	bmi	.Put
	move.l	#0,d0
	bra	.Put

.GoingRight
	sub.l	10(a5),d0
	bpl	.Put
	move.l	#0,d0
	;fall thru
.Put
	move.l	d0,BB_XSpeed(a3)
.Exit
	rts




PlatformPlayerHorizWallCheck
	;Stop player at wall
	;in: a2=slice,a3=bob,a5=platformplayer params

	movem.l	d2-d3/a4,-(sp)

	move.l	OB_Image(a3),a4
	move.l	OB_XPos(a3),d0
;	add.l	OB_AnimXOffset(a3),d0
	add.l	BB_XSpeed(a3),d0

	move.l	OB_YPos(a3),d1
;	add.l	OB_AnimYOffset(a3),d1

	tst.l	BB_XSpeed(a3)
	beq	.Exit
	bmi	.GoingLeft

;GoingRight

;	add.l	IM_ColRight(a4),d0
	add.l	40(a5),d0	;left
	add.l	48(a5),d0	;width
;	add.l	IM_ColTop(a4),d1
	add.l	44(a5),d1	;top

;	move.l	IM_ColHeight(a4),d3
	move.l	52(a5),d3	;height

	move.w	#2,d2
	move.l	30(a5),a0		;get EdgeOfTheRoad
	bsr	_CheckVlineBGIntersection
	tst.w	d0
	beq	.Exit

	;hit a wall to the right
	sub.l	#1<<5,d1
;	sub.l	OB_AnimXOffset(a3),d1
;	sub.l	IM_ColRight(a4),d1
	sub.l	40(a5),d1	;left
	sub.l	48(a5),d1	;width
	move.l	d1,OB_XPos(a3)
	move.l	#0,BB_XSpeed(a3)
	bra	.Exit

.GoingLeft
;	add.l	IM_ColLeft(a4),d0
	add.l	40(a5),d0	;left
;	add.l	IM_ColTop(a4),d1
	add.l	44(a5),d1	;top
;	move.l	IM_ColHeight(a4),d3
	move.l	52(a5),d3	;height

	move.w	#2,d2
	move.l	30(a5),a0		;get EdgeOfTheRoad
	bsr	_CheckVlineBGIntersection
	tst.w	d0
	beq	.Exit

	;hit a wall to the left
	add.l	#16<<5,d1
;	sub.l	OB_AnimXOffset(a3),d1
;	sub.l	IM_ColLeft(a4),d1
	sub.l	40(a5),d1	;left
	move.l	d1,OB_XPos(a3)
	move.l	#0,BB_XSpeed(a3)

.Exit
	movem.l	(sp)+,d2-d3/a4
	rts


PlatformPlayerFloorCheck
	;in: a2=slice,a3=bob,a5=platformplayer params
	;out: d0.w = 0 for no floor, 1 for floor

	movem.l	d2-d3/a4,-(sp)

	tst.l	BB_YSpeed(a3)
	bmi	.Exit

	move.l	OB_Image(a3),a4
	move.l	OB_XPos(a3),d0
;	add.l	IM_ColLeft(a4),d0
	add.l	40(a5),d0	;left
;	add.l	OB_AnimXOffset(a3),d0

	move.l	OB_YPos(a3),d1
;	add.l	OB_AnimYOffset(a3),d1
	add.l	BB_YSpeed(a3),d1
;	add.l	IM_ColBottom(a4),d1
	add.l	44(a5),d1	;top
	add.l	52(a5),d1	;height

;	move.l	IM_ColWidth(a4),d3
	move.l	48(a5),d3	;width

	move.w	#0,d2			;can hit wall, floor or ladders
	move.l	30(a5),a0		;get EdgeOfTheRoad
	bsr	_CheckHlineBGIntersection
	tst.w	d0
	beq	.Exit

	sub.l	#1<<5,d1
;	sub.l	OB_AnimYOffset(a3),d1
;	sub.l	IM_ColBottom(a4),d1
	sub.l	44(a5),d1	;top
	sub.l	52(a5),d1	;height


.Exit
	movem.l	(sp)+,d2-d3/a4
	;result in d0.w
	rts



PlatformPlayerCeilingCheck
	;in: a2=slice,a3=bob,a5=platformplayer params
	;out: d0.w = 0 for no floor, 1 for floor

	movem.l	d2-d3/a4,-(sp)

	tst.l	BB_YSpeed(a3)
	bpl	.Exit

	move.l	OB_Image(a3),a4
	move.l	OB_XPos(a3),d0
;	add.l	IM_ColLeft(a4),d0
	add.l	40(a5),d0		;left
;	add.l	OB_AnimXOffset(a3),d0

	move.l	OB_YPos(a3),d1
;	add.l	OB_AnimYOffset(a3),d1
	add.l	BB_YSpeed(a3),d1
;	add.l	IM_ColTop(a4),d1
	add.l	44(a5),d1		;top

;	move.l	IM_ColWidth(a4),d3
	move.l	48(a5),d3		;width

	move.w	#2,d2			;can hit wall only
	move.l	30(a5),a0		;get EdgeOfTheRoad
	bsr	_CheckHlineBGIntersection
	tst.w	d0
	beq	.Exit

	add.l	#16<<5,d1
;	sub.l	OB_AnimYOffset(a3),d1
;	sub.l	IM_ColTop(a4),d1
	sub.l	44(a5),d1	;top
	move.l	d1,OB_YPos(a3)
	move.l	#0,BB_YSpeed(a3)
.Exit
	movem.l	(sp)+,d2-d3/a4
	;result in d0.w
	rts


;**********************************
_InitAsteroidsPlayer
	;In:	a1 = Channel
	;	a2 = Slice
	;	a3 = Bob
	;	a5 = Params
	;Params:
	;	0 = Joystick.w
	;	2 = RotSpeed.l
	;	6 = Fwd Accel.l
	;	10 = Rev. Accel.l
	;	14 = MaxSpeed.l
	;	18 = Drag.w (0=none... n=lots)
	;	20 = FireActionList.l
	;	24 = Flags.w (bit0=autofire?, bit1=fireweaponbays?)
	;	26

	move.l	a5,CL_Parameters(a1)

	move.l	14(a5),d0
	muls	d0,d0
	move.l	d0,CL_Data1(a1)		;maxspeed^2

	move.l	#.Update,CL_UpdateCode(a1)
	lea	26(a5),a5
	rts

.Update
	;In:	a2=slice,a3=bob,a6=channel

	movem.l	d2-d3/a5,-(sp)
	move.l	CL_Parameters(a6),a5

	move.w	#0,d3		;clear up/down flag

	move.w	(a5),d0		;joystick
	jsr	_ReadVirtualJoystick
	move.w	d0,d2
	move.w	24(a5),d1	;flags
	and.w	#3,d1
	move.l	20(a5),a0	;fire actionlist
	jsr	_GenericCheckFire

	btst	#2,d2		;left?
	beq	.LeftDone
	move.l	2(a5),d0
	sub.b	d0,BB_Direction(a3)
.LeftDone

	btst	#3,d2		;right?
	beq	.RightDone
	move.l	2(a5),d0
	add.b	d0,BB_Direction(a3)
.RightDone

	move.l	BB_XSpeed(a3),d0
;	muls	BB_XSpeed+2(a3),d0
	muls	d0,d0
	move.l	BB_YSpeed(a3),d1
;	muls	BB_YSpeed+2(a3),d1
	muls	d1,d1
	add.l	d1,d0
	cmp.l	CL_Data1(a6),d0
;	bhi	.CheckDrag
	bhi	.MinorDrag

;	add.l	#4,BB_YSpeed(a3)	;gravity
;	move.w	#1,d3

	btst	#0,d2		;up?
	beq	.UpDone
	move.w	#1,d3		;set up/down flag
	move.b	BB_Direction(a3),d0
	move.l	6(a5),d1	;fwd accel
	jsr	_SetSpeedsFromDirection
	add.l	d0,BB_XSpeed(a3)
	add.l	d1,BB_YSpeed(a3)
.UpDone

	btst	#1,d2		;down?
	beq	.DownDone
	move.w	#1,d3		;set up/down flag
	move.b	BB_Direction(a3),d0
	add.b	#128,d0
	move.l	10(a5),d1	;rev accel
	jsr	_SetSpeedsFromDirection
	add.l	d0,BB_XSpeed(a3)
	add.l	d1,BB_YSpeed(a3)
.DownDone
	bra	.CheckDrag

.MinorDrag
	move.w	#5,d1
	bra	.DoDrag

.CheckDrag
	tst.w	d3
	bne	.Exit

	move.w	#0,d1
	add.w	18(a5),d1	;drag value

.DoDrag
	move.l	BB_XSpeed(a3),d0
	asr.l	d1,d0
	beq	.PutX
	neg.l	d0
	add.l	BB_XSpeed(a3),d0
.PutX
	move.l	d0,BB_XSpeed(a3)

	move.l	BB_YSpeed(a3),d0
	asr.l	d1,d0
	beq	.PutY
	neg.l	d0
	add.l	BB_YSpeed(a3),d0
.PutY
	move.l	d0,BB_YSpeed(a3)

.Exit
	movem.l	(sp)+,d2-d3/a5
	rts

;**********************************
_InitMousePlayer
	;In:	a1 = Channel
	;	a2 = Slice
	;	a3 = Bob
	;	a5 = Params
	;Params:
	;0	mouse.w
	;2	XAccel.w	0..3
	;4	YAccel.w	0..3
	;6	MaxXSpd.l
	;10	MaxYSpd.l
	;14	XDrag.l
	;18	YDrag.l
	;22	FireActionList.l
	;26	Flags.w
	;		bit0 = autofirerepeat?
	;		bit1 = fireallweaponbays?
	;		bit2 = direct movement?
	;28	AxisFreedom.w	(both,Xonly,yonly)
	;30	BaseXSpd.l
	;34	Gravity.l
	;38

	move.l	a5,CL_Parameters(a1)
	lea	.Update,a0
	move.l	a0,CL_UpdateCode(a1)

	lea	38(a5),a5
	rts

.Update
	;in: a2=slice,a3=bob,a6=channel
	movem.l	a4-a5,-(sp)
	move.l	CL_Parameters(a6),a5

	move.l	30(a5),d0		;Base x speed
	sub.l	d0,BB_XSpeed(a3)

	move.w	(a5),d0	;mouse
	lsl.w	#2,d0
	lea	_VirtualMice,a0
	move.l	(a0,d0.w),a4

	move.w	26(a5),d0	;flags
	btst	#2,d0		;direct?
	beq	.Move_Inertial

	;Move_Direct

	move.w	28(a5),d0	;axisfreedom
	cmp.w	#2,d0		;yonly?
	beq	.Move_DirectY

	move.w	#3,d1
	add.w	2(a5),d1
	move.w	VM_DeltaX(a4),d0
	ext.l	d0
	asl.l	d1,d0
	move.l	d0,BB_XSpeed(a3)

.Move_DirectY
	move.w	28(a5),d0	;axisfreedom
	cmp.w	#1,d0		;xonly?
	beq	.Move_Done

	move.w	#3,d1
	add.w	4(a5),d1
	move.w	VM_DeltaY(a4),d0
	ext.l	d0
	asl.l	d1,d0
	move.l	d0,BB_YSpeed(a3)
	bra	.Move_Done

.Move_Inertial
	move.w	28(a5),d0	;axisfreedom
	cmp.w	#2,d0		;yonly?
	beq	.Move_InertialDoY

	move.w	#0,d1
	add.w	2(a5),d1
	move.w	VM_DeltaX(a4),d0
	ext.l	d0
	asl.l	d1,d0
	add.l	BB_XSpeed(a3),d0

	;do drag
;	move.w	VM_DeltaX(a4),d1
;	abs.w	d1
;	cmp.w	#8,d0
;	blo	.Move_InertialXPut
	
	tst.w	VM_DeltaX(a4)
	bne	.Move_InertialXPut
	tst.w	VM_DeltaY(a4)
	bne	.Move_InertialXPut

	tst.l	d0
	beq	.Move_InertialXPut
	bmi	.Move_InertialGoingLeft
	;going right
	sub.l	14(a5),d0
	bpl	.Move_InertialXPut
	move.l	#0,d0
	bra	.Move_InertialXPut
.Move_InertialGoingLeft
	add.l	14(a5),d0
	bmi	.Move_InertialXPut
	move.l	#0,d0
.Move_InertialXPut
	move.l	d0,BB_XSpeed(a3)

.Move_InertialDoY
	move.w	28(a5),d0	;axisfreedom
	cmp.w	#1,d0		;xonly?
	beq	.Move_Done

	move.w	#0,d1
	add.w	4(a5),d1
	move.w	VM_DeltaY(a4),d0
	ext.l	d0
	asl.l	d1,d0
	add.l	BB_YSpeed(a3),d0

	;do drag
;	move.w	VM_DeltaY(a4),d1
;	abs.w	d1
;	cmp.w	#8,d1
;	blo	.Move_InertialYPut
	
	tst.w	VM_DeltaY(a4)
	bne	.Move_InertialYPut
	tst.w	VM_DeltaX(a4)
	bne	.Move_InertialYPut

	tst.l	d0
	beq	.Move_InertialYPut
	bmi	.Move_InertialGoingUp
	;going down
	sub.l	18(a5),d0
	bpl	.Move_InertialYPut
	move.l	#0,d0
	bra	.Move_InertialYPut
.Move_InertialGoingUp
	add.l	14(a5),d0
	bmi	.Move_InertialYPut
	move.l	#0,d0
.Move_InertialYPut
	move.l	d0,BB_YSpeed(a3)
.Move_Done


	;X Speed Limit
	move.w	28(a5),d0	;axisfreedom
	cmp.w	#2,d0		;yonly?
	beq	.XLimit_Done

	move.l	BB_XSpeed(a3),d0
	beq	.XLimit_Done
	bmi	.XLimit_GoingLeft
	;going right
	cmp.l	6(a5),d0
	ble	.XLimit_Done
	move.l	6(a5),d0
	bra	.XLimit_Put
.XLimit_GoingLeft
	move.l	6(a5),d1
	neg.l	d1
	cmp.l	d1,d0
	bge	.XLimit_Done
	move.l	d1,d0
.XLimit_Put
	move.l	d0,BB_XSpeed(a3)
.XLimit_Done

	;Y Speed Limit
	move.w	28(a5),d0	;axisfreedom
	cmp.w	#1,d0		;xonly?
	beq	.YLimit_Done

	move.l	BB_YSpeed(a3),d0
	beq	.YLimit_Done
	bmi	.YLimit_GoingUp
	;going down
	cmp.l	10(a5),d0
	ble	.YLimit_Done
	move.l	10(a5),d0
	bra	.YLimit_Put
.YLimit_GoingUp
	move.l	10(a5),d1
	neg.l	d1
	cmp.l	d1,d0
	bge	.YLimit_Done
	move.l	d1,d0
.YLimit_Put
	move.l	d0,BB_YSpeed(a3)
.YLimit_Done

	;FIRING
	move.l	22(a5),a0			;fireactionlist
	move.w	VM_PrevButtonState(a4),d0	;prev and current bytes
	move.w	26(a5),d1			;flags
	and.w	#3,d1
	bsr	_GenericCheckFire

	move.l	30(a5),d0		;Base x speed
	add.l	d0,BB_XSpeed(a3)

	movem.l	(sp)+,a4-a5
	rts

;**********************************
_InitMousePointer
	;In:	a1 = Channel
	;	a2 = Slice
	;	a3 = Bob
	;	a5 = Params
	;Params:
	;0	mouseport.w
	;2	pad.w
	;4	scale.w (0..4 (2=normal hardware speed) )
	;6	AxisFreedom.w	(both,Xonly,yonly)
	;8	LBDown.l
	;12	LBUp.l
	;16	RBDown.l
	;20	RBUp.l
	;24	MBDown.l
	;28	MBUp.l
	;32	Flags.w
	;		bit0 = autorepeatMBDown
	;		bit1 = autorepeatRBDown
	;		bit2 = autorepeatLBDown
	;34	reserved.l
	;38

	move.l	a5,CL_Parameters(a1)
	lea	.Update,a0
	move.l	a0,CL_UpdateCode(a1)

	lea	38(a5),a5
	rts

.Update
	;in: a2=slice,a3=bob,a6=channel
	movem.l	a4-a5,-(sp)
	move.l	CL_Parameters(a6),a5

	;read mouse
	move.w	(a5),d0		;get mouseport
	lsl.w	#2,d0
	lea	_VirtualMice,a0
	move.l	(a0,d0.w),a4

	move.w	4(a5),d1	;get scalevalue
	add.w	#BITSHIFT-2,d1

	cmp.w	#2,6(a5)	;y freedom only?
	beq	.XDone
	move.w	VM_DeltaX(a4),d0
	ext.l	d0
	asl.l	d1,d0
	move.l	d0,BB_XSpeed(a3)
.XDone

	cmp.w	#1,6(a5)	;x freedom only?
	beq	.YDone
	move.w	VM_DeltaY(a4),d0
	ext.l	d0
	asl.l	d1,d0
	move.l	d0,BB_YSpeed(a3)
.YDone

	movem.l	(sp)+,a4-a5
	rts


;**********************************
_InitActionListPlayer
	;In:	a1 = Channel
	;	a2 = Slice
	;	a3 = Bob
	;	a5 = Params
	;Params:
	;0	Joystick.w
	;2	Reserved.w
	;4	UpPressed.l
	;8	UpRelease.l
	;12	DnPressed.l
	;16	DnRelease.l
	;20	LfPressed.l
	;24	LfRelease.l
	;28	RtPressed.l
	;32	RtRelease.l
	;36	Fire0Pressed.l
	;40	Fire0Release.l
	;44	Fire1Pressed.l
	;48	Fire1Release.l
	;52	Fire2Pressed.l
	;56	Fire2Release.l
	;60	Fire3Pressed.l
	;64	Fire3Release.l
	;68	Flags.w	(bits 0..7 autorepeat 'pressed' actionlist)
	;70

	move.l	a5,CL_Parameters(a1)
	move.w	#0,CL_Data1(a1)		;check all 8 bits
	move.w	68(a5),d0
	move.b	d0,CL_Data2(a1)		;autorepeat flags
	move.l	#UpdateActionListPlayer,CL_UpdateCode(a1)
	lea	70(a5),a5
	rts


_InitCheckFireButtons
	;In:	a1 = Channel
	;	a2 = Slice
	;	a3 = Bob
	;	a5 = Params
	;Params:
	;0	Joystick.w
	;2	Reserved.w
	;4	Fire0Pressed.l
	;8	Fire0Release.l
	;12	Fire1Pressed.l
	;16	Fire1Release.l
	;20	Fire2Pressed.l
	;24	Fire2Release.l
	;28	Fire3Pressed.l
	;32	Fire3Release.l
	;36	Flags.w	(bits 0..3 autorepeat 'pressed' actionlist)
	;38

	move.l	a5,CL_Parameters(a1)
	move.w	#4,CL_Data1(a1)		;only check firebuttons
	move.w	36(a5),d0
	lsl.w	#4,d0			;move autorepeat flags to correct bitpos
	move.b	d0,CL_Data2(a1)
	move.l	#UpdateActionListPlayer,CL_UpdateCode(a1)
	lea	38(a5),a5
	rts

UpdateActionListPlayer
	;Update routine for _InitActionListPlayer & _InitCheckFireButtons
	;in:	a2=slice, a3=bob, a6=channel
	;	CL_Data1 = first joystick bit to check (0 for all,
	;		4=for fire buttons only)
	;	CL_Data2.b = autorepeat flags
	;	CL_Parameters = see _InitActionListPlayer/_InitCheckFireButtons

	movem.l	d2-d4/a4-a5,-(sp)
	move.l	CL_Parameters(a6),a5

	move.w	0(a5),d0	;joystick
	jsr	_ReadVirtualJoystick
	move.w	d0,d2		;d2.w = joystick state (current & prev)

	;Check for releases

	move.b	d2,d1		;d1.b = current state
	move.w	d2,d4
	lsr.w	#8,d4		;d4.b = prevstate

	not.b	d1
	and.b	d1,d4		;result in d4 (1=release, 0=no release)

	move.w	CL_Data1(a6),d3		;d3=bitnum
	lea	8(a5),a4	;a4=first release actionlist
	move.l	a5,-(sp)
.ReleaseLoop
	btst	d3,d4
	beq	.ReleaseLoopNext
	move.l	(a4),d0		;get actionlist
	beq	.ReleaseLoopNext
	move.l	d0,a5
	move.w	#CONTEXT_BOB,d0
	bsr	_DoActionList
.ReleaseLoopNext
	lea	8(a4),a4	;skip to next release actionlist
	add.w	#1,d3
	cmp.w	#8,d3
	bne	.ReleaseLoop
	move.l	(sp)+,a5

	;check for presses
	move.w	d2,d4
	lsr.w	#8,d4		;d4.b = prevstate
	move.b	d4,d1		;d1.b = prevstate
	not.b	d4		;d4.b = ~prev
	and.b	d2,d4		;d4.b = ~prev & current

	and.b	d2,d1		;d1.b = prev & current
	and.b	CL_Data2(a6),d1	;d1.b = prev & current & autorepeat

	;d4.b = (~prev & current) | (prev & current & autorepeat)
	or.b	d1,d4

	move.w	CL_Data1(a6),d3	;d3=bitnum
	lea	4(a5),a4	;a4=first press actionlist
	move.l	a5,-(sp)
.PressLoop
	btst	d3,d4
	beq	.PressLoopNext
	move.l	(a4),d0		;get actionlist
	beq	.PressLoopNext
	move.l	d0,a5
	move.w	#CONTEXT_BOB,d0
	bsr	_DoActionList
.PressLoopNext
	lea	8(a4),a4	;skip to next press actionlist
	add.w	#1,d3
	cmp.w	#8,d3
	bne	.PressLoop
	move.l	(sp)+,a5

	movem.l	(sp)+,d2-d4/a4-a5
	rts


;**********************************
_InitInertialPlayer
	;In:	a1 = Channel
	;	a2 = Slice
	;	a3 = Bob
	;	a5 = Params
	;Params:
	;0	Joystick.w
	;2	XAccel.l
	;6	YAccel.l
	;10	MaxXSpd.l
	;14	MaxYSpd.l
	;18	XDrag.l
	;22	YDrag.l
	;26	FireActionList.l
	;30	Flags.w		(bit0=autofirerepeat?, bit1=fireallweaponbays?
	;			bit2 = robotron dir locking on fire)
	;32	Gravity.l
	;36	BaseXSpd.l
	;40	Setdirecton.w	(8way,left/right,up/down,no)
	;42	AxisFreedom.w	(both,Xonly,yonly)
	;44

	move.l	a5,CL_Parameters(a1)
	move.l	#.Update,CL_UpdateCode(a1)
	lea	44(a5),a5
	rts

.Update
	;a2=slice,a3=bob,a6=channel

	movem.l	d2-d3/a5,-(sp)
	move.l	CL_Parameters(a6),a5

	move.l	36(a5),d0		;basexspd
	sub.l	d0,BB_XSpeed(a3)

	move.w	(a5),d0
	jsr	_ReadVirtualJoystick
	move.w	d0,d2

;**** XDRAG

	cmp.w	#2,42(a5)	;yaxis only?
	beq	.XDrag_Done

	;pressing left/right?
	and.b	#%00001100,d0
	bne	.XDrag_Done

	move.l	BB_XSpeed(a3),d0
	beq	.XDrag_Done
	bmi	.XDrag_SpdNegative
	;speed positive
	sub.l	18(a5),d0
	bpl	.XDrag_Put
	move.l	#0,d0
	bra	.XDrag_Put
.XDrag_SpdNegative
	add.l	18(a5),d0
	bmi	.XDrag_Put
	move.l	#0,d0
.XDrag_Put
	move.l	d0,BB_XSpeed(a3)
.XDrag_Done


;**** YDRAG
	cmp.w	#1,42(a5)	;xaxis only?
	beq	.YDrag_Done
	;pressing up/down?
	move.b	d2,d0
	and.b	#%00000011,d0
	bne	.YDrag_Done

.YDrag_DoGravity
	tst.l	32(a5)
 	beq	.YDrag_DoIt
	move.l	BB_YSpeed(a3),d0
	add.l	32(a5),d0
	cmp.l	14(a5),d0
	ble	.YDrag_PutGrav
	move.l	14(a5),d0
.YDrag_PutGrav
	move.l	d0,BB_YSpeed(a3)
	bra	.YDrag_Done

.YDrag_DoIt
	;do drag
	move.l	BB_YSpeed(a3),d0
	beq	.YDrag_Done
	bmi	.YDrag_SpdNegative
	;speed positive
	sub.l	22(a5),d0
	bpl	.YDrag_Put
	move.l	#0,d0
	bra	.YDrag_Put
.YDrag_SpdNegative
	add.l	22(a5),d0
	bmi	.YDrag_Put
	move.l	#0,d0
.YDrag_Put
	move.l	d0,BB_YSpeed(a3)

.YDrag_Done



;**** XACCEL

	cmp.w	#2,42(a5)	;yaxis only?
	beq	.XAccel_Done

	;check left
	btst	#2,d2
	beq	.XAccel_LeftDone
	move.l	BB_XSpeed(a3),d0
	sub.l	2(a5),d0			;sub xaccel
	move.l	10(a5),d1
	neg.l	d1
	cmp.l	d1,d0				;cmp to -maxxspd
	bge	.XAccel_LeftPut
	move.l	d1,d0
.XAccel_LeftPut
	move.l	d0,BB_XSpeed(a3)
.XAccel_LeftDone

	;check right
	btst	#3,d2
	beq	.XAccel_RightDone
	move.l	BB_XSpeed(a3),d0
	add.l	2(a5),d0			;add xaccel
	cmp.l	10(a5),d0			;cmp to maxxspd
	ble	.XAccel_RightPut
	move.l	10(a5),d0
.XAccel_RightPut
	move.l	d0,BB_XSpeed(a3)
.XAccel_RightDone
.XAccel_Done


;**** YACCEL
	cmp.w	#1,42(a5)	;xaxis only?
	beq	.YAccel_Done

	;check up
	btst	#0,d2
	beq	.YAccel_UpDone
	move.l	BB_YSpeed(a3),d0
	sub.l	6(a5),d0			;sub xaccel
	move.l	14(a5),d1
	neg.l	d1
	cmp.l	d1,d0				;cmp to -maxxspd
	bge	.YAccel_UpPut
	move.l	d1,d0
.YAccel_UpPut
	move.l	d0,BB_YSpeed(a3)
.YAccel_UpDone

	;check down
	btst	#1,d2
	beq	.YAccel_DownDone
	move.l	BB_YSpeed(a3),d0
	add.l	6(a5),d0			;add xaccel
	cmp.l	14(a5),d0			;cmp to maxxspd
	ble	.YAccel_DownPut
	move.l	14(a5),d0
.YAccel_DownPut
	move.l	d0,BB_YSpeed(a3)
.YAccel_DownDone
.YAccel_Done


;**** Check fire button

	move.b	d2,d0
	move.w	30(a5),d1
	btst	#0,d1		;autofirerepeat?
	bne	.FCheck_AutoRepeat

	move.w	CL_Data1(a6),d1
	btst	#4,d1		;fire down in previous frame?
	bne	.FCheck_Done

.FCheck_AutoRepeat
	btst	#4,d0
	beq	.FCheck_Done

	;BANG.

	move.w	30(a5),d0
	btst	#1,d0		;fire all weapon bays?
	beq	.FCheck_DoActionList

	move.w	#$FFFF,d0	;_ALL_ of them.
	jsr	_InternalFireMultiWeaponBays

.FCheck_DoActionList
	move.l	26(a5),d0	;Fire ActionList
	beq	.FCheck_Done
	move.l	a5,-(sp)
	move.l	d0,a5
	move.w	#CONTEXT_BOB,d0
	bsr	_DoActionList
	move.l	(sp)+,a5

.FCheck_Done
	move.w	d2,CL_Data1(a6)


;**** SET DIRECTION
	move.w	d2,d1

	move.w	40(a5),d0
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
	move.w	30(a5),d0
	btst	#2,d0			;robotron flag set?
	beq	.SetDir_ReallyDoIt

	;don't change dir if fire down
	btst	#4,d2
	bne	.SetDir_Done
.SetDir_ReallyDoIt

	and.w	#$F,d1
	beq	.SetDir_Done	;no dirs pressed?
	lea	_JoyDirTable,a0
	move.b	(a0,d1.w),BB_Direction(a3)

.SetDir_Done

	move.l	36(a5),d0		;basexspd
	add.l	d0,BB_XSpeed(a3)

.Exit

	movem.l	(sp)+,d2-d3/a5
	rts





;**********************************
_InitJitterSpeed	;channelroutine
	;In:	a1 = Channel
	;	a2 = Slice
	;	a3 = Bob
	;	a5 = Params
	;Params:
	;0	xrndmin
	;4	xrndmax
	;8	maxxpsd
	;12	yrndmin
	;16	yrndmax
	;20	maxyspd
	;24	Which axis? 0=jitterboth,1=only x, 2=only y
	;26	Rate.w (0=every cycle)
	;28

	;XYZZY - finish off: flags etc...

	move.l	a5,CL_Parameters(a1)
	move.w	26(a5),CL_Data1(a1)	;CL_Data1.w = delay counter
	move.l	#.Update,CL_UpdateCode(a1)
	lea	28(a5),a5
	rts

.Update
	;In:	a2=slice, a3=bob, a6=channel

	move.l	a5,-(sp)
	move.l	CL_Parameters(a6),a5

	sub.w	#1,CL_Data1(a6)
	bpl	.Exit
	move.w	26(a5),CL_Data1(a6)	;reset counter

	cmp.w	#2,24(a5)
	beq	.xspdlimit
	move.l	0(a5),d0	;xrndmin
	move.l	4(a5),d1	;xrndmax
	jsr	_ConstrainedRandom
	ext.l	d0
	add.l	d0,BB_XSpeed(a3)
.xspdlimit

.yjitter
	cmp.w	#1,24(a5)
	beq	.yspdlimit
	move.l	12(a5),d0	;yrndmin
	move.l	16(a5),d1	;yrndmax
	jsr	_ConstrainedRandom
	ext.l	d0
	add.l	d0,BB_YSpeed(a3)

.yspdlimit

.Exit
	move.l	(sp)+,a5
	rts

;**********************************
_RandomEvent	;action
	;In:	a2 = Slice
	;	a3 = Bob
	;	a5 = Params
	;Params:
	;0	Probability.w
	;2	ActionList.l
	;6	flags.w
	;8

	jsr	_Random
	lsr.w	#8,d0
	cmp.b	1(a5),d0
	bhi	.Exit

	move.l	a5,-(sp)
	move.l	2(a5),d0
	beq	.AList_Null
	move.l	d0,a5
	move.w	_ActionContext,d0
	bsr	_DoActionList
.AList_Null
	move.l	(sp)+,a5

.Exit
	lea	8(a5),a5
	rts



;**********************************
_InitRandomEvent	;channelroutine
	;In:	a1 = Channel
	;	a2 = Slice
	;	a3 = Bob
	;	a5 = Params
	;Params:
	;0	Probability.w
	;2	ActionList.l
	;6	flags.w
	;8

	move.l	a5,CL_Parameters(a1)
	move.l	#.Update,CL_UpdateCode(a1)
	lea	8(a5),a5
	rts

.Update
	;In:	a2=slice, a3=bob, a6=channel

	move.l	a5,-(sp)
	move.l	CL_Parameters(a6),a5

	jsr	_Random
	lsr.w	#8,d0
	cmp.b	1(a5),d0
	bhi	.Done

	move.l	2(a5),d0
	beq	.Done
	move.l	d0,a5
	move.w	_ActionContext,d0
	bsr	_DoActionList

.Done
	move.l	(sp)+,a5
	rts


************************************************************************
* GENERAL PURPOSE ROUTINES
************************************************************************



_InternalInstallAnim
	;in:	a0 = ptr to workspace (2 longwords, animbase,animptr)
	;	a1 = anim
	;	a3 = bob

	move.l	a1,(a0)		;set anim base
	move.l	(a1)+,d0
	beq	.NullAnim

	move.l	d0,OB_Image(a3)
	move.l	(a1)+,OB_AnimXOffset(a3)
	move.l	(a1)+,OB_AnimYOffset(a3)
	move.l	a1,4(a0)	;store anim ptr
	rts
.NullAnim
	move.l	d0,(a0)		;null anim base
	rts


_InternalDoCyclicAnim
	;in:	a0 = 8 byte work area.
	;	a3 = bob

	move.l	4(a0),a1	;get anim ptr
	move.l	(a1)+,d0
	bne	.putimage
	;wrap
	move.l	(a0),a1		;start at base
	move.l	(a1)+,d0
.putimage
	move.l	d0,OB_Image(a3)
	move.l	(a1)+,OB_AnimXOffset(a3)
	move.l	(a1)+,OB_AnimYOffset(a3)
	move.l	a1,4(a0)	;store anim ptr
	rts



_SetSpeedsFromDirection
;in:	d0.b = direction
;	d1.l = magnitude of speed (5 bitshifts)
;out:	d0.l = xspeed
;	d1.l = yspeed

	move.l	d2,-(sp)
	moveq.l	#0,d2
	move.b	d0,d2
	lea	_SineTable,a0

	move.b	64(a0,d2.w),d0	;read from cosinetable
	ext.w	d0
	muls	d1,d0		;calc yspd
	asr.l	#7,d0		;allow for bitshifts in cosine value
	move.l	d0,a1		;save for later

	move.b	(a0,d2.w),d0	;read from sinetable
	ext.w	d0
	muls	d1,d0		;calc xspd
	asr.l	#7,d0		;allow for bitshifts in sine value

	move.l	a1,d1		;retrieve yspd
	neg.l	d1

.fuckoff
	move.l	(sp)+,d2
	rts



_CheckHlineBGIntersection
	;in:	a2 = Slice
	;	a0 = EdgeOfTheRoad
	;	d0.l, d1.l = left of hline (5 bitshifts)
	;	d2.b = HighestSpaceValue
	;	d3.l = Width of line (5 bitshifts)
	;out:	d0.w = 0 for no collision, or eotrvalue
	;	d1.l = ypos, rounded down to nearest block

	movem.l	d3-d4,-(sp)

	sub.l	#1<<5,d3

	move.l	d1,-(sp)

	move.l	SS_Scroll(a2),a1

	;Convert YPos to map coords and clip
	asr.l	#BITSHIFT,d1
	asr.l	#4,d1
	bmi	.OffMap
	cmp.w	SC_MapBlocksHigh(a1),d1
	bhs	.OffMap

	;Clip XPos/Width and convert to map coords
	tst.l	d0
	bpl	.leftok
	add.l	d0,d3		;chop the left
	bmi	.OffMap		;completely off the left?
	move.l	#0,d0
.leftok
	move.l	d0,d4
	and.l	#%111111111,d4
	add.l	d4,d3
	lsr.l	#BITSHIFT,d0
	lsr.l	#4,d0
	lsr.l	#BITSHIFT,d3
	lsr.l	#4,d3
	;check right
	move.w	d0,d4
	add.w	d3,d4
	cmp.w	SC_MapBlocksWide(a1),d4
	blo	.rightok
	;clip right
	move.w	SC_MapBlocksWide(a1),d3
	sub.w	d0,d3
	sub.w	#1,d3
	bmi	.OffMap		;completely off right?
.rightok

	move.w	SC_MapBlocksWide(a1),d4
	and.l	#$0000FFFF,d4
	lsl.w	#1,d4			;cos blocks are words, not bytes
	lsl.w	#1,d0

	mulu	d4,d1
	add.l	d0,d1
	move.l	SC_MapBank(a1),a1
	add.l	d1,a1			;ptr to first block

	lea	EOTR_Data(a0),a0	;find eotr array
.BlkLoop
	move.w	(a1)+,d0		;get block
	cmp.b	(a0,d0.w),d2
	blo	.Hit
	sub.w	#1,d3
	bpl	.BlkLoop
	;no hit
.OffMap
	move.w	#0,d0
	bra	.Exit
.Hit
	move.b	(a0,d0.w),d0
	and.w	#$FF,d0
;	move.w	#1,d0
.Exit
	move.l	(sp)+,d1
	and.l	#%11111111111111111111111000000000,d1
	movem.l	(sp)+,d3-d4
	rts



;XYZZY - d1 returns nearest block boundary - need to clip this value to map boundary
_CheckVlineBGIntersection
	;in:	a2 = Slice
	;	a0 = EdgeOfTheRoad
	;	d0.l, d1.l = top of vline (5 bitshifts)
	;	d2.b = HighestSpaceValue
	;	d3.l = height of line (5 bitshifts)
	;out:	d0.w = 0 for no collision, 1 for collision
	;	d1.l = xpos, rounded down to nearest block

	movem.l	d3-d4,-(sp)

	sub.l	#1<<5,d3

	move.l	d0,-(sp)

	move.l	SS_Scroll(a2),a1

	;Convert XPos to map coords and clip
	asr.l	#BITSHIFT,d0
	asr.l	#4,d0
	bmi	.OffMap
	cmp.w	SC_MapBlocksWide(a1),d0
	bhs	.OffMap

	;Clip YPos/Height and convert to map coords
	tst.l	d1
	bpl	.topok
	add.l	d1,d3		;chop the top
	bmi	.OffMap		;completely off the top?
	move.l	#0,d1
.topok
	move.l	d1,d4
	and.l	#%111111111,d4
	add.l	d4,d3
	lsr.l	#BITSHIFT,d1
	lsr.l	#4,d1
	lsr.l	#BITSHIFT,d3
	lsr.l	#4,d3
	;check bottom
	move.w	d1,d4
	add.w	d3,d4
	cmp.w	SC_MapBlocksHigh(a1),d4
	blo	.bottomok
	;clip bottom
	move.w	SC_MapBlocksHigh(a1),d3
	sub.w	d1,d3
	sub.w	#1,d3
	bmi	.OffMap		;completely off bottom?
.bottomok

	move.w	SC_MapBlocksWide(a1),d4
	and.l	#$0000FFFF,d4
	lsl.w	#1,d4			;cos blocks are words, not bytes
	lsl.w	#1,d0

	mulu	d4,d1
	add.l	d0,d1
	move.l	SC_MapBank(a1),a1
	add.l	d1,a1			;ptr to first block

	lea	EOTR_Data(a0),a0	;find eotr array
.BlkLoop
	move.w	(a1),d0			;get block
	cmp.b	(a0,d0.w),d2
	blo	.Hit
	add.l	d4,a1			;down a row
	sub.w	#1,d3
	bpl	.BlkLoop
	;no hit
.OffMap
	move.w	#0,d0
	bra	.Exit
.Hit
;	move.w	#1,d0
	move.b	(a0,d0.w),d0
	and.w	#$FF,d0

.Exit
	move.l	(sp)+,d1
	and.l	#%11111111111111111111111000000000,d1
	movem.l	(sp)+,d3-d4
	rts





_FindHeading
	;in:	a0 - target object
	;	a3 - source object
	;out:	d0.b - direction from source to target (in brabians)

	movem.l	d2-d3,-(sp)

	lea	_ATanTable,a1

	move.l	OB_XPos(a0),d2
	sub.l	OB_XPos(a3),d2	;d2 = targetx-sourcex (dx)

	move.l	OB_YPos(a3),d3
	sub.l	OB_YPos(a0),d3	;d3 = sourcey-targety (-dy)

	;check that dx!=0 and dy!=0
	tst.l	d2
	bne	.GeraldTheMouse
	tst.l	d3
	beq	.BuggerOff

.GeraldTheMouse

	tst.l	d2
	bmi	.dxminus

	tst.l	d3
	bmi	.dyminus1

	cmp.l	d2,d3
	blt	.oct1
.oct0
	asl.l	#8,d2
	divs	d3,d2		;dx/dy
	move.b	(a1,d2.w),d0
	bra	.BuggerOff
.oct1
	asl.l	#8,d3
	divs	d2,d3		;dy/dx
	move.b	#64,d0
	sub.b	(a1,d3.w),d0
	bra	.BuggerOff

.dyminus1
	neg.l	d3
	cmp.l	d2,d3
	blt	.oct3
.oct2
	asl.l	#8,d2
	divs	d3,d2		;dx/dy
	move.b	#128,d0
	sub.b	(a1,d2.w),d0
	bra	.BuggerOff
.oct3
	asl.l	#8,d3
	divs	d2,d3		;dy/dx
	move.b	#64,d0
	add.b	(a1,d3.w),d0
	bra	.BuggerOff

.dxminus
	neg.l	d2
	tst.l	d3
	bmi	.dyminus2

	cmp.l	d2,d3
	blt	.oct5
.oct4
	asl.l	#8,d2
	divs	d3,d2		;dx/dy
	move.b	(a1,d2.w),d0
	neg.b	d0
	bra	.BuggerOff
.oct5
	asl.l	#8,d3
	divs	d2,d3		;dy/dx
	move.b	#192,d0
	add.b	(a1,d3.w),d0
	bra	.BuggerOff

.dyminus2
	neg.l	d3
	cmp.l	d2,d3
	blt	.oct7
.oct6
	asl.l	#8,d2
	divs	d3,d2		;dx/dy
	move.b	#128,d0
	add.b	(a1,d2.w),d0
	bra	.BuggerOff
.oct7
	asl.l	#8,d3
	divs	d2,d3		;dy/dx
	move.b	#192,d0
	sub.b	(a1,d3.w),d0
	;fall thru

.BuggerOff
	movem.l	(sp)+,d2-d3
	rts




_ULongToBCD
	;in: d0.l binary number
	;out: d0.l bcd number

	movem.l	d2-d3,-(sp)

	move.l	#0,BCDTemp
	move.w	#30-1,d2	;more than thirty bits would cause overflow
	lea	BCDPowersOfTwo,a0
.Loop
	lsr.l	d0
	bcc	.Zilch
.One
	lea	BCDTemp+4,a1
	lea	4(a0),a0
	move.w	#0,CCR
	abcd	-(a0),-(a1)
	abcd	-(a0),-(a1)
	abcd	-(a0),-(a1)
	abcd	-(a0),-(a1)
	lea	4(a0),a0
	bra	.Next
.Zilch
	lea	4(a0),a0
.Next
	dbra	d2,.Loop

	movem.l	(sp)+,d2-d3
	move.l	BCDTemp,d0
	rts


_GenericCheckFire
	;in:	a0 = Fire ActionList (or NULL)
	;	a2 = Slice
	;	a3 = Bob
	;	d0.w =	joystick state:
	;	(low byte = current state, highbyte = prev state)
	;	d1.w = flags:
	;	(bit0 = autofirerepeat?, bit1 = fire weaponbays?)

	btst	#0,d1			;autofirerepeat?
	bne	.FCheck_AutoRepeat

	btst	#8+4,d0			;fire down in previous frame?
	bne	.FCheck_Done

.FCheck_AutoRepeat
	btst	#4,d0			;bang?
	beq	.FCheck_Done

	;bang.

	btst	#1,d1			;fire all weapon bays?
	beq	.FCheck_DoActionList

	move.l	a0,-(sp)
	move.w	#$FFFF,d0		;_ALL_ of them.
	jsr	_InternalFireMultiWeaponBays
	move.l	(sp)+,a0

.FCheck_DoActionList
	cmp.l	#0,a0			;Fire ActionList?
	beq	.FCheck_Done
	move.l	a5,-(sp)
	move.l	a0,a5
	move.w	#CONTEXT_BOB,d0
	bsr	_DoActionList
	move.l	(sp)+,a5

.FCheck_Done
	rts


************************************************************************
* DATA
************************************************************************

	SECTION Data,data

BCDTemp	dc.l	0
BCDPowersOfTwo	;last two entrys would cause an overflow
	dc.l	$1,$2,$4,$8,$16,$32,$64,$128,$256
	dc.l	$512,$1024,$2048,$4096,$8192,$16384,$32768,$65536
	dc.l	$131072,$262144,$524288,$1048576,$2097152,$4194304,$8388608,$16777216
	dc.l	$33554432,$67108864,$134217728,$268435456,$536870912,$1073741824,0,0



	;Trig tables - values have 7 bitshifts
	;indexed by Brabians: 256 Brabians to the circle.
_SineTable
	;theta = 0 Brabians
	dc.b	0,3,6,9,12,15,18,21,24,27,30,34,37,39,42,45
	dc.b	48,51,54,57,60,62,65,68,70,73,75,78,80,83,85,87
	;32 Brabians
	dc.b	90,92,94,96,98,100,102,104,106,107,109,110,112,113,115,116
	dc.b	117,118,120,121,122,122,123,124,125,125,126,126,126,127,127,127
_CosineTable
	;64 Brabians
	dc.b	127,127,127,127,126,126,126,125,125,124,123,122,122,121,120,118
	dc.b	117,116,115,113,112,110,109,107,106,104,102,100,98,96,94,92
	;96 Brabians
	dc.b	90,87,85,83,80,78,75,73,70,68,65,62,60,57,54,51
	dc.b	48,45,42,39,37,34,30,27,24,21,18,15,12,9,6,3
	;128 Brabians
	dc.b	0,-3,-6,-9,-12,-15,-18,-21,-24,-27,-30,-34,-37,-39,-42,-45
	dc.b	-48,-51,-54,-57,-60,-62,-65,-68,-70,-73,-75,-78,-80,-83,-85,-87
	;160 Brabians
	dc.b	-90,-92,-94,-96,-98,-100,-102,-104,-106,-107,-109,-110,-112,-113,-115,-116
	dc.b	-117,-118,-120,-121,-122,-122,-123,-124,-125,-125,-126,-126,-126,-127,-127,-127
	;192 Brabians
	dc.b	-127,-127,-127,-127,-126,-126,-126,-125,-125,-124,-123,-122,-122,-121,-120,-118
	dc.b	-117,-116,-115,-113,-112,-110,-109,-107,-106,-104,-102,-100,-98,-96,-94,-92
	;224 Brabians
	dc.b	-90,-87,-85,-83,-80,-78,-75,-73,-70,-68,-65,-62,-60,-57,-54,-51
	dc.b	-48,-45,-42,-39,-37,-34,-30,-27,-24,-21,-18,-15,-12,-9,-6,-3
	;end of sinetable - repeat first 64 values for cosine table
	;0 (256) Brabians
	dc.b	0,3,6,9,12,15,18,21,24,27,30,34,37,39,42,45
	dc.b	48,51,54,57,60,62,65,68,70,73,75,78,80,83,85,87
	;32 (288) Brabians
	dc.b	90,92,94,96,98,100,102,104,106,107,109,110,112,113,115,116
	dc.b	117,118,120,121,122,122,123,124,125,125,126,126,126,127,127,127

_ATanTable
	;generated by Utils/ATanGen.c
	;index by gradient to find angle (0..32 brabians)
	;256 entries (gradient 0.0 => 0, 1.0 => 256 )
	dc.b	0,0,0,0,0,0,0,1,1,1,1,1,1,2,2,2
	dc.b	2,2,2,3,3,3,3,3,3,3,4,4,4,4,4,4
	dc.b	5,5,5,5,5,5,6,6,6,6,6,6,6,7,7,7
	dc.b	7,7,7,8,8,8,8,8,8,8,9,9,9,9,9,9
	dc.b	9,10,10,10,10,10,10,11,11,11,11,11,11,11,12,12
	dc.b	12,12,12,12,12,13,13,13,13,13,13,13,14,14,14,14
	dc.b	14,14,14,15,15,15,15,15,15,15,15,16,16,16,16,16
	dc.b	16,16,17,17,17,17,17,17,17,17,18,18,18,18,18,18
	dc.b	18,19,19,19,19,19,19,19,19,20,20,20,20,20,20,20
	dc.b	20,20,21,21,21,21,21,21,21,21,22,22,22,22,22,22
	dc.b	22,22,22,23,23,23,23,23,23,23,23,23,24,24,24,24
	dc.b	24,24,24,24,24,25,25,25,25,25,25,25,25,25,26,26
	dc.b	26,26,26,26,26,26,26,26,27,27,27,27,27,27,27,27
	dc.b	27,27,27,28,28,28,28,28,28,28,28,28,28,29,29,29
	dc.b	29,29,29,29,29,29,29,29,29,30,30,30,30,30,30,30
	dc.b	30,30,30,30,31,31,31,31,31,31,31,31,31,31,31,31
