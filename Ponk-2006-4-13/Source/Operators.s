

	incdir	"ck:Ponk/inc/"
	include	"source/libs.gs"
	include	"source/Header.i"
	include	"source/Constants.i"
	include	"source/Structures.i"
	include	"source/Lame.i"

	XREF	_DoActionList,_BulletImage,_FindFreePlayerDude,_FindFreeBulletDude,_FindFreeBadDude,_InitLinearAccel,_InitCyclicAnimation
	XREF	_ActionContext,_InternalFireWeaponBay
	XREF	_WaitVBlank,_WaitPastVBlank,_UpdateDir16Anim,_ShellPrintNumber,_BackgroundCollision
	XREF	_InertialJoyMovement,_custom
	XREF	_InputChannels
	XREF	_InitXPushSliceTrack,_InitYPushSliceTrack
	XREF	_PlaySample,_OLDBackgroundPointCollision
	XREF	_ShellPrint,_InitInertialJoyMovement,_OldInitActionListPlayer
	XREF	_InitSteroids
	XREF	CON_CursorSetX,CON_CursorSetY,_CON_PutStr
	XREF	_StatBoxGroovyConDevice

	XREF	_InitHomerBadDude1,_InitDirectionalHomer
	XREF	_InitNULL,_InitOperationWolfMovement
	XREF	_InitFollowPath,_InitTimeDelayExecute

	XREF	_SetColumnOffset,_ScrollSlice,_BlockDrawSlice

	XREF	_InitAttatch,_SimplePlatformPlayer,_InitAttachToParent

	XREF	_OldInitDir8Anim,_OldInitDir16Anim,_OldInitDir32Anim,_OldInitDir64Anim
	XREF	_InitDir8Anim,_InitDir16Anim,_InitDir32Anim,_InitDir64Anim
	XREF	_OldInitDir8Image,_OldInitDir16Image,_OldInitDir32Image,_OldInitDir64Image
	XREF	_InitDir8Image,_InitDir16Image,_InitDir32Image,_InitDir64Image
	XREF	_InitDir8Action,_InitDir16Action,_InitDir32Action,_InitDir64Action
	XREF	_NextLevelBuffer,_GameOn,_Random,_GlobalData,_InitJoystickAnim
	XREF	_InitModulePlayer,_StopModulePlayer,_ModulePlayerEnable,_SongDataPtr
	XREF	_InitActionOnFire,_KPrintF,_FirstSlice
	XREF	_ObjectStructures
	XREF	_InitDrawBar,_InitFloorSentry
	XREF	_InitCeilingSentry,_InitLinearPlayer

	XDEF	_GetVariableTable,_SetVariableTable,_AddVariableTable

	XDEF	_EXECUTE,_GetHurt
	XDEF	_OperatorTable,_Templates,_OperatorNames
	XDEF	_InitBackgroundCollision
;	XDEF	_INITBOB
	XDEF	_SpriteObjects
	XDEF	_NotINITBOBAlthoughFairlySimilar
	XDEF	_OBJECTOFF

	;From BenActionRoutines.s:
	XREF	_DirectionMove,_SetSpeedsFromDirection
	XREF	_OLDDirectionMove
	XREF	_FindHeading
	XREF	_InitChannelExecute,_InitDirectionMove
	XREF	_InitBGPointCollision,_InitBGCollision
	XREF	_SineTable,_CosineTable,_ATanTable
	XREF	_InitTrackPlayer
	XREF	_InitJitterSpeed
	XREF	_SetRND,_AddRND
	XREF	_RestrictDirection
	XREF	_InitPlatformPlayer
	XREF	_InitInertialPlayer
	XREF	_InitAsteroidsPlayer
	XREF	_InitMousePlayer
	XREF	_InitActionListPlayer
	XREF	_InitCheckFireButtons
	XREF	_InitXSpeedAnim,_InitYSpeedAnim
	XREF	_InitInertialHomer
	XREF	_RandomEvent,_InitRandomEvent
	XREF	_InitMousePointer

	XREF	_ULongToBCD
	XREF	_GenericCheckFire
	XREF	_FindPlayer,_FindNearestPlayer

	;From Action.s
	XREF	_ScrollModeConstantXSpeed
	XREF	_ScrollXCenterPlayers,_ScrollYCenterPlayers

;********************************************************************************************************

	IFNE	CHIPASSEMBLE
	SECTION	WhatLovelyCode,CODE_C
	ELSE
	SECTION	WhatLovelyCode,CODE
	ENDC

;********************************************************************************************************
;Action commands

	;INPUTS:
	;		a2.l = Slice
	;		a3.l = Object if an object command
	;		a4.l = Other object, eg Collision partner or weapon to use
	;		a5.l = Parameter list

	;		d7.w = Return error code (leave unless error)

;********************************************************************************************************

_EXECUTE
	;Preforms actionlist imbeded in param list
	;SYNTAX:	EXECUTE	ActionList.prog

	move.l	(a5)+,a1
	cmp.l	#0,a1
	beq	.Exit
	move.l	a5,-(sp)
	move.l	a1,a5
	move.w	_ActionContext,d0	;stay in current context
	bsr	_DoActionList
	move.l	(sp)+,a5
.Exit
	rts

;************************************

_SetChannel
	;Sets up for for new channel init routine
	;SYNTAX:	SETC	ChannelNumber.w,ChannelInit.l,Timer.w,Donelist.prog

	move.w	(a5)+,d0
	mulu	#CL_SizeOf,d0
	add.l	#OB_Channel1,d0
	lea	(a3,d0.l),a1

	move.l	a5,CL_SetupProgram(a1)
	move.l	(a5)+,a0
	jsr	(a0)

	move.w	(a5),CL_Timer(a1)
	move.l	2(a5),CL_DoneChannel(a1)
	lea	6(a5),a5
	rts


;************************************


_SpawnPlayerBullet
	;SYNTAX:	Image.im, XOffset.l,YOffset.l,ActionList.prog

	bsr	_FindFreeBulletDude
	cmp.l	#0,a0
	beq	.Exit
.FoundDude

	move.w	#1,OB_State(a0)
	move.w	#OB_TYPE_PLAYERBULLET,OB_Type(a0)

	move.l	a3,-(sp)
	move.l	a0,a3
	bsr	_NotINITBOBAlthoughFairlySimilar
	move.l	a3,a0
	move.l	(sp)+,a3

	move.l	#0,BB_WeaponBank(a0)
	cmp.w	#CONTEXT_WEAPON,_ActionContext
	bne	.NotAWeapon
	add.w	#1,WP_Bullets(a4)
	;XYZZY - change weaponbank to parent field
	move.l	a4,BB_WeaponBank(a0)	;so the object knows its origins
.NotAWeapon
	cmp.w	#CONTEXT_NONE,_ActionContext
	beq	.NoParent
	move.w	#PARENT_OBJECT,OB_ParentType(a0)
	move.l	a3,OB_Parent(a0)
	move.b	BB_Direction(a3),BB_Direction(a0)
	move.l	OB_XPos(a3),OB_XPos(a0)
	move.l	OB_YPos(a3),OB_YPos(a0)
	move.l	BB_XSpeed(a3),BB_XSpeed(a0)
	move.l	BB_YSpeed(a3),BB_YSpeed(a0)
	bra	.SetImage
.NoParent
	move.l	#0,OB_Parent(a0)
	move.w	#PARENT_NONE,OB_ParentType(a0)
	move.b	#0,BB_Direction(a0)

.SetImage
	move.l	(a5),OB_Image(a0)

	move.l	4(a5),d0
	add.l	d0,OB_XPos(a0)
	move.l	8(a5),d0
	add.l	d0,OB_YPos(a0)

	movem.l	a3-a5,-(sp)
	move.l	12(a5),a5
	move.l	a0,a3

	move.w	#CONTEXT_BOB,d0
	bsr	_DoActionList
	movem.l	(sp)+,a3-a5
.Exit
	lea	16(a5),a5
	rts


_SpawnBadDude
	;SYNTAX:	Image.im, XOffset.l,YOffset.l,ActionList.prog

	bsr	_FindFreeBadDude
	cmp.l	#0,a0
	beq	.Exit
.FoundDude
	move.w	#1,OB_State(a0)
	move.w	#OB_TYPE_BADDUDE,OB_Type(a0)

	move.l	a3,-(sp)
	move.l	a0,a3
	bsr	_NotINITBOBAlthoughFairlySimilar
	move.l	a3,a0
	move.l	(sp)+,a3

	cmp.w	#CONTEXT_NONE,_ActionContext
	beq	.NoParent
	move.w	#PARENT_OBJECT,OB_ParentType(a0)
	move.l	a3,OB_Parent(a0)
	move.b	BB_Direction(a3),BB_Direction(a0)
	move.l	OB_XPos(a3),OB_XPos(a0)
	move.l	OB_YPos(a3),OB_YPos(a0)
	move.l	BB_XSpeed(a3),BB_XSpeed(a0)
	move.l	BB_YSpeed(a3),BB_YSpeed(a0)
	bra	.SetImage
.NoParent
	move.l	#0,OB_Parent(a0)
	move.w	#PARENT_NONE,OB_ParentType(a0)
	move.b	#0,BB_Direction(a0)
.SetImage
	move.l	(a5),OB_Image(a0)

	move.l	4(a5),d0
	add.l	d0,OB_XPos(a0)
	move.l	8(a5),d0
	add.l	d0,OB_YPos(a0)

	movem.l	a3-a5,-(sp)
	move.l	12(a5),a5
	move.l	a0,a3
	cmp.l	#0,a5
	beq	.NoAction
	move.w	#CONTEXT_BOB,d0
	bsr	_DoActionList
.NoAction
	movem.l	(sp)+,a3-a5
.Exit
	lea	16(a5),a5
	rts


;XYZZY - Kill this?

_SpawnPlayer
	;SYNTAX:	Image.im, XOffset.l,YOffset.l,ActionList.prog

	move.l	SS_Players(a2),a0
	cmp.l	#0,a0
	beq	.Exit

	move.l	OB_Next(a0),a0		;Skip over first four
	move.l	OB_Next(a0),a0
	move.l	OB_Next(a0),a0
	move.l	OB_Next(a0),a0
	cmp.l	#0,a0
	beq	.Exit

	bsr	_FindFreePlayerDude
	cmp.l	#0,a0
	beq	.Exit
.FoundDude
	move.w	#1,OB_State(a0)
	move.w	#OB_TYPE_PLAYER,OB_Type(a0)

	move.l	a3,-(sp)
	move.l	a0,a3
	bsr	_NotINITBOBAlthoughFairlySimilar
	move.l	a3,a0
	move.l	(sp)+,a3

	cmp.w	#CONTEXT_NONE,_ActionContext
	beq	.NoParent
	move.l	a3,OB_Parent(a0)
	move.w	#PARENT_OBJECT,OB_ParentType(a0)
	move.b	BB_Direction(a3),BB_Direction(a0)
	move.l	OB_XPos(a3),OB_XPos(a0)
	move.l	OB_YPos(a3),OB_YPos(a0)
	move.l	BB_XSpeed(a3),BB_XSpeed(a0)
	move.l	BB_YSpeed(a3),BB_YSpeed(a0)
	bra	.SetImage
.NoParent
	move.l	#0,OB_Parent(a0)
	move.w	#PARENT_NONE,OB_ParentType(a0)
	move.b	#0,BB_Direction(a0)

.SetImage
	move.l	(a5),OB_Image(a0)

	move.l	4(a5),d0
	add.l	d0,OB_XPos(a0)
	move.l	8(a5),d0
	add.l	d0,OB_YPos(a0)

	movem.l	a3-a5,-(sp)
	move.l	12(a5),a5
	move.l	a0,a3
	move.w	#CONTEXT_BOB,d0
	bsr	_DoActionList
	movem.l	(sp)+,a3-a5
.Exit
	lea	16(a5),a5
	rts

;************************************

_SetWeaponBay
	;Syntax:	SetWeaponBay	Bay.w,Weapon.l
	move.l	BB_WeaponBank(a3),a0
	move.w	(a5)+,d0
	lsl.w	#2,d0
	move.l	(a5)+,(a0,d0.w)
	rts

;************************************

_FireWeaponBay
	;in: a2=slice a3=bob a5=Params
	;SYNTAX		FireBay	Bay.w

	move.w	(a5)+,d0
	bsr	_InternalFireWeaponBay
	rts

;************************************

_PowerUpWeaponBay
	;SYNTAX		PowerUpFireBay	Bay.w,Weapon.l

	move.l	BB_WeaponBank(a3),a0
	move.w	(a5),d0
	lsl.w	#2,d0
	move.l	(a0,d0.w),a1
	cmp.l	#0,a1
	beq	.NoWeapon

	move.l	WP_PowerUp(a1),d1
	beq	.Exit
	move.l	d1,(a0,d0.w)
	bra	.Exit
.NoWeapon
	move.l	2(a5),(a0,d0.w)
.Exit
	lea	6(a5),a5
	rts

;************************************

_PowerDownWeaponBay
	;SYNTAX		PowerDownFireBay	Bay.w,Flags.w

	move.l	BB_WeaponBank(a3),a0
	move.w	(a5),d0
	lsl.w	#2,d0
	move.l	(a0,d0.w),a1
	cmp.l	#0,a1
	beq	.Exit

	move.l	WP_PowerDown(a1),d1
	beq	.NoWeapon
	move.l	d1,(a0,d0.w)
	bra	.Exit
.NoWeapon
	tst.w	2(a5)
	bne	.Exit
	move.l	#0,(a0,d0.w)
.Exit
	lea	4(a5),a5
	rts

;************************************

_CollisionPartnerExecute
	;Execute actionlist on collision partner

	cmp.w	#CONTEXT_COLLISION,_ActionContext
	bne	.Exit

	move.l	(a5)+,d0
	beq	.Exit		;GetActionlist
	movem.l	a3-a5,-(sp)	;Save out stuff
	move.l	d0,a5
	move.l	a3,d0
	move.l	a4,a3		;Swap collision partners
	move.l	d0,a4
	move.w	#CONTEXT_COLLISION,d0
	bsr	_DoActionList
	movem.l	(sp)+,a3-a5	;Put registers back
.Exit
	rts

_GetHurt
	;GET HURT
	;Standard hurting of dude for collisions, takes Energy#1 - Damage#2
	;Inputs		a3.l = Dude to hurt
	;		a4.l = Dude which hurt it
	;SYNTAX:	HURTDUDE

	move.w	BB_MaxEnergy(a4),d0
	sub.w	d0,BB_Energy(a3)
	bpl	.Exit

	move.l	a5,-(sp)
	move.l	BB_DeathHu(a3),a5
	cmp.l	#0,a5
	beq	.DeathDone
	move.w	#CONTEXT_COLLISION,d0
	bsr	_DoActionList
.DeathDone
	move.l	(sp)+,a5
.Exit
	rts	

;************************************
;XYZZY - Obsolete?
_InitBackgroundCollision
	;Inputs:	a1.l = Channel
	;		a3.l = Bob
	;		a5.l = Params
	;SYNTAX:	LastSpaceBlock.w, ActionList.l, Shifts.w

	move.w	(a5)+,CL_Data(a1)
	move.l	(a5)+,CL_Data+4(a1)
	move.w	(a5)+,CL_Data+2(a1)
	move.l	#_BackgroundCollision,CL_UpdateCode(a1)
	rts

;************************************

_OBJECTOFF
	;Turns off object
	;Syntax		object off

	move.w	#0,OB_State(a3)

.OtherStuff
	cmp.w	#OB_TYPE_PLAYERBULLET,OB_Type(a3)
	bne	.Exit

	move.l	BB_WeaponBank(a3),a0
	cmp.l	#0,a0
	beq	.Exit
	sub.w	#1,WP_Bullets(a0)
.Exit
	rts

_CollisionPartnerOff
	;Turns off Collision Partner
	;Syntax		object off

	move.w	#0,OB_State(a4)

	cmp.w	#OB_TYPE_PLAYERBULLET,OB_Type(a4)
	bne	.Exit

	move.l	BB_WeaponBank(a4),a0
	cmp.l	#0,a0
	beq	.Exit
	sub.w	#1,WP_Bullets(a0)
.Exit
	rts

_KILLOBJECT
	;Turns off object & executes death routine
	;Syntax		KILLobject

	move.w	#0,OB_State(a3)
.OtherStuff
	cmp.w	#OB_TYPE_PLAYERBULLET,OB_Type(a3)
	bne	.ExecAction

	move.l	BB_WeaponBank(a3),a0
	cmp.l	#0,a0
	beq	.ExecAction
	sub.w	#1,WP_Bullets(a0)
.ExecAction
	move.l	BB_DeathHu(a3),a0
	cmp.l	#0,a0
	beq	.Exit
	move.l	a5,-(sp)
	move.l	a0,a5

	move.w	#CONTEXT_BOB,d0
	bsr	_DoActionList
	move.l	(sp)+,a5
.Exit
	rts

_KillCollisionPartner
	;Turns off object & executes death routine
	;Syntax		KILLobject

	cmp.w	#CONTEXT_COLLISION,_ActionContext
	bne	.Exit

	move.w	#0,OB_State(a4)

	cmp.w	#OB_TYPE_PLAYERBULLET,OB_Type(a4)
	bne	.ExecAction

	move.l	BB_WeaponBank(a4),a0
	cmp.l	#0,a0
	beq	.ExecAction
	sub.w	#1,WP_Bullets(a0)
.ExecAction
	move.l	BB_DeathHu(a4),a0
	cmp.l	#0,a0
	beq	.Exit
	movem.l	a3-a5,-(sp)
	move.l	a0,a5
	move.l	a3,a0
	move.l	a4,a3
	move.l	a0,a4

	move.w	#CONTEXT_COLLISION,d0
	bsr	_DoActionList
	movem.l	(sp)+,a3-a5
.Exit
	rts

;************************************

;_SETXPOS
;	;SYNTAX:	XPos.l
;	move.l	(a5)+,OB_XPos(a3)
;	rts

;************************************

_SETYPOS
	;SYNTAX:	YPos.l
	move.l	(a5)+,OB_YPos(a3)
	rts

;************************************

_SETIMAGE
	;SYNTAX:	Image.im
	move.l	(a5)+,OB_Image(a3)
	move.l	#0,OB_AnimXOffset(a3)
	move.l	#0,OB_AnimYOffset(a3)
	rts

;************************************

_OBSSetBorder
	;SYNTAX:	Left.l,Type.w, Right.l,Type.w, Top.l,Type.w, Bottom.l,Type.w,Map/View.w

	move.w	24(a5),d1

	move.l	(a5)+,BB_BorderLeft(a3)
	move.w	(a5)+,d0
	move.b	d0,BB_BorderLeftType(a3)
	move.l	(a5)+,BB_BorderRight(a3)
	move.w	(a5)+,d0
	move.b	d0,BB_BorderRightType(a3)
	move.l	(a5)+,BB_BorderTop(a3)
	move.w	(a5)+,d0
	move.b	d0,BB_BorderTopType(a3)
	move.l	(a5)+,BB_BorderBottom(a3)
	move.w	(a5)+,d0
	move.b	d0,BB_BorderBottomType(a3)

	move.w	(a5)+,d0
	and.l	#$3,d0
	and.l	#$FFFFFFFC,BB_Flags(a3)
	or.l	d0,BB_Flags(a3)
	bra	.Exit
.Exit
	rts



_SetBorder	;actionroutine
	;Params:
	;0	LeftBorder.l
	;4	LeftBorderType.w
	;6	RightBorder.l
	;10	RightBorderType.w
	;12	TopBorder.l
	;16	TopBorderType.w
	;18	BottomBorder.l
	;22	BottomBorderType.w
	;24	Flags.w	(bits0-3 = ViewRelative flags)
	;26

	move.w	24(a5),d1			;get flags

	;Left border
	move.l	(a5),BB_BorderLeft(a3)
	move.w	4(a5),d0
	btst	#3,d1
	beq	.putlf
	neg.b	d0
.putlf
	move.b	d0,BB_BorderLeftType(a3)

	;Right border
	move.l	6(a5),BB_BorderRight(a3)
	move.w	10(a5),d0
	btst	#2,d1
	beq	.putrt
	neg.b	d0
.putrt
	move.b	d0,BB_BorderRightType(a3)

	;Top border
	move.l	12(a5),BB_BorderTop(a3)
	move.w	16(a5),d0
	btst	#1,d1
	beq	.puttop
	neg.b	d0
.puttop
	move.b	d0,BB_BorderTopType(a3)

	;Bottom border
	move.l	18(a5),BB_BorderBottom(a3)
	move.w	22(a5),d0
	btst	#0,d1
	beq	.putbot
	neg.b	d0
.putbot
	move.b	d0,BB_BorderBottomType(a3)

;	move.w	(a5)+,d0
;	and.l	#$3,d0
;	and.l	#$FFFFFFFC,BB_Flags(a3)
;	or.l	d0,BB_Flags(a3)
;	bra	.Exit
;.Exit
	lea	26(a5),a5

	rts

;************************************
;Internal bob structure setup routine - all new objects should
;come through here at some time.

;XYZZY - check and tidy please

_NotINITBOBAlthoughFairlySimilar
	;In:	a2 = Slice
	;	a3 = bob structure to init

	move.l	#0,d0

	move.l	d0,OB_Parent(a3)
	move.w	#PARENT_NONE,OB_ParentType(a3)

	move.l	d0,OB_Image(a3)
	move.l	d0,BB_HitRoutine1(a3)
	move.l	d0,BB_HitRoutine2(a3)
	move.l	d0,BB_HitRoutine3(a3)
	move.w	d0,BB_MaxEnergy(a3)
	move.w	d0,BB_Energy(a3)
	move.l	d0,BB_DeathHu(a3)
	move.l	#0,BB_Flags(a3)
	;bset.b	#2,BB_Flags+3(a3)
	bset.b	#5,BB_Flags+3(a3)	;don't even want to know.

	move.w	#OB_UPDATETYPE_ML,OB_UpdateType(a3)
	move.l	d0,OB_AnimXOffset(a3)
	move.l	d0,OB_AnimYOffset(a3)

	move.w	d0,OB_Highlight(a3)

	move.w	#-1,OB_Channel1+CL_Timer(a3)
	move.l	d0,OB_Channel1+CL_UpdateCode(a3)
	move.l	d0,OB_Channel1+CL_Parameters(a3)
	move.l	d0,OB_Channel1+CL_Data(a3)
	move.l	d0,OB_Channel1+CL_Data+4(a3)
	move.l	d0,OB_Channel1+CL_Data+8(a3)
	move.l	d0,OB_Channel1+CL_Data+12(a3)

	move.w	#-1,OB_Channel2+CL_Timer(a3)
	move.l	d0,OB_Channel2+CL_UpdateCode(a3)
	move.l	d0,OB_Channel2+CL_Parameters(a3)
	move.l	d0,OB_Channel2+CL_Data(a3)
	move.l	d0,OB_Channel2+CL_Data+4(a3)
	move.l	d0,OB_Channel2+CL_Data+8(a3)
	move.l	d0,OB_Channel2+CL_Data+12(a3)

	move.w	#-1,OB_Channel3+CL_Timer(a3)
	move.l	d0,OB_Channel3+CL_UpdateCode(a3)
	move.l	d0,OB_Channel3+CL_Parameters(a3)
	move.l	d0,OB_Channel3+CL_Data(a3)
	move.l	d0,OB_Channel3+CL_Data+4(a3)
	move.l	d0,OB_Channel3+CL_Data+8(a3)
	move.l	d0,OB_Channel3+CL_Data+12(a3)

	move.w	#-1,OB_Channel4+CL_Timer(a3)
	move.l	d0,OB_Channel4+CL_UpdateCode(a3)
	move.l	d0,OB_Channel4+CL_Parameters(a3)
	move.l	d0,OB_Channel4+CL_Data(a3)
	move.l	d0,OB_Channel4+CL_Data+4(a3)
	move.l	d0,OB_Channel4+CL_Data+8(a3)
	move.l	d0,OB_Channel4+CL_Data+12(a3)

	move.w	#-1,OB_Channel5+CL_Timer(a3)
	move.l	d0,OB_Channel5+CL_UpdateCode(a3)
	move.l	d0,OB_Channel5+CL_Parameters(a3)
	move.l	d0,OB_Channel5+CL_Data(a3)
	move.l	d0,OB_Channel5+CL_Data+4(a3)
	move.l	d0,OB_Channel5+CL_Data+8(a3)
	move.l	d0,OB_Channel5+CL_Data+12(a3)

	;install the defaultborders

	cmp.l	#0,a2		;sanity check.
	beq	.Fuckoff

	;gak.
	move.l	SS_DefaultBorderLeft(a2),BB_BorderLeft(a3)
	move.l	SS_DefaultBorderRight(a2),BB_BorderRight(a3)
	move.l	SS_DefaultBorderTop(a2),BB_BorderTop(a3)
	move.l	SS_DefaultBorderBottom(a2),BB_BorderBottom(a3)

	move.b	SS_DefaultBorderLeftType+1(a2),BB_BorderLeftType(a3)
	move.b	SS_DefaultBorderRightType+1(a2),BB_BorderRightType(a3)
	move.b	SS_DefaultBorderTopType+1(a2),BB_BorderTopType(a3)
	move.b	SS_DefaultBorderBottomType+1(a2),BB_BorderBottomType(a3)

.Fuckoff
	rts

_SetMiscStuff
	;Generalized 'set misc shtuff'
	;SYNTAX:	HitRoutine1.l, HitRoutine2.l, HitRoutine3.l
	;		Damage.w, Energy.w, DeathHu.l, Flags - Border,Speed

	move.l	(a5)+,BB_HitRoutine1(a3)
	move.l	(a5)+,BB_HitRoutine2(a3)
	move.l	(a5)+,BB_HitRoutine3(a3)

	move.w	(a5)+,BB_MaxEnergy(a3)
	move.w	(a5)+,BB_Energy(a3)

	move.l	(a5)+,BB_DeathHu(a3)

	move.w	(a5)+,d0		;XYZZY - field obsolete
;	lsr.w	d0
;	bcc	.NoDefaultBorder

;	lsr.w	d0
;	bcc	.LeaveSpeed

;	move.l	#0,BB_XSpeed(a3)
;	move.l	#0,BB_YSpeed(a3)
;.LeaveSpeed

.Exit
	rts


;************************************

;XYZZY - want to kill this sometime, or change it to a positioning routine

_InitScroll
	;Sets up the scrolling for a level & draws up screen
	;SYNTAX:	XPos.l,YPos.l,Speed.l,BaseSpeed.l

	movem.l	a3-a5,-(sp)

	move.l	SS_Scroll(a2),a3
	cmp.l	#0,a3
	beq	.Exit
.Fine
	move.l	#_ScrollSlice,SS_Channel0(a2)

	move.l	(a5),d0
	bpl	.CheckRight
	move.l	#0,d0
	bra	.SetPos
.CheckRight
	cmp.l	SC_RightBorder(a3),d0
	blt	.SetPos
	move.l	SC_RightBorder(a3),d0
.SetPos
	move.l	d0,SC_Pos(a3)
	move.l	4(a5),SC_YPos(a3)
	move.l	8(a5),SC_Speed(a3)
	move.l	12(a5),SC_BaseSpeed(a3)

	move.l	#0,SC_Speed(a3)
;	move.l	#0,SC_BaseSpeed(a3)

	bsr	_SetColumnOffset

	move.l	SS_PhyBase(a2),a3
	bsr	_BlockDrawSlice
	move.l	SS_LogBase(a2),a3
	bsr	_BlockDrawSlice
	move.l	SS_TriBase(a2),a3
	bsr	_BlockDrawSlice
.Exit
	movem.l	(sp)+,a3-a5
	lea	16(a5),a5
	rts

;************************************

_HighlightBob
	;SYNTAX		HighlightBob	Cycles.w

	move.w	(a5)+,OB_Highlight(a3)
	rts

;************************************

_SetColour
	;Takes RGB triplet & puts into copper list(s)
	;SYNTAX		SetColour	Color.b,R.b,G.b,B.b

	lea	4(a5),a5
	rts

;************************************

_KillChannel
	;Initializes channel struct
	;SYNTAX		KillChannel	Channel.w

	move.w	(a5)+,d0
	mulu	#CL_SizeOf,d0
	add.l	#OB_Channel1,d0
	lea	(a3,d0.l),a1

	move.l	#0,CL_UpdateCode(a1)
	move.l	#0,CL_Parameters(a1)
	move.l	#0,CL_DoneChannel(a1)
	move.w	#0,CL_Timer(a1)
	move.l	#0,CL_Data(a1)
	move.l	#0,CL_Data+4(a1)
	move.l	#0,CL_Data+8(a1)
	move.l	#0,CL_Data+12(a1)
	move.l	#0,CL_Flags(a1)

	rts


;************************************

_PlaySFX
	;SYNTAX:	SFX.l,Volume.b,Period.w,Channel.w

	move.l	(a5)+,a0
	;move.b	(a5)+,SFX_Volume(a0)		;<-Something like that
	;move.w	(a5)+,SFX_Period(a0)

	move.w	(a5)+,d0
	bsr	_PlaySample		;?? What registers corupted
	rts

;************************************

_SimpleSFX
	;SYNTAX:	SFX.l,Channel.w

	move.l	(a5)+,a0		;SpamParam
	move.l	a0,a1
	add.l	#spam_SIZEOF,a1		;Data Start
	move.w	(a5)+,d0		;Channel
	bsr	_PlaySample
	rts

;************************************

_VBlankSprite
	;Turns update of sprite to happen in VBlank
	;SYNTAX		NULL

	lea	SS_SpriteObjects(a2),a0
	move.w	#7,d0
.Loop
	move.w	d0,d1
	lsl.w	#2,d1
	move.l	(a0,d1.w),a1
	cmp.l	#0,a1
	beq	.FoundFree
	dbra	d0,.Loop

	bra	.Exit
.FoundFree
	move.l	a3,(a0,d1.w)
	move.w	#OB_UPDATETYPE_VB,OB_UpdateType(a3)
.Exit
	rts

;************************************

_MainLoopSprite
	
	;SYNTAX		NULL

	lea	SS_SpriteObjects(a2),a0
	move.w	#7,d0
.Loop
	move.l	(a0),a1
	cmp.l	a3,a1
	beq	.FoundUm
	add.l	#4,a0
	dbra	d0,.Loop
.FoundUm
	move.l	#0,(a0)
	move.w	#OB_UPDATETYPE_ML,OB_UpdateType(a3)
	rts



;**********************************
_StatPrintVar
	;in: a2=slice, a3=bob, a5=params
	;0	xpos.l
	;4	ypos.l
	;8	var.w
	;10

	move.w	8(a5),d0
	lsl.w	#2,d0
	lea	_GetVariableTable,a0
	move.l	(a0,d0.w),a0
	jsr	(a0)	;result in d1
	abs.l	d1
	lsr.l	#5,d1

	move.l	d1,d0
	jsr	_ULongToBCD
	move.l	d0,-(sp)

	lea	_StatBoxGroovyConDevice,a0
	move.l	(a5),d0			;get xpos
	lsr.l	#5+3,d0
	bsr	CON_CursorSetX

	lea	_StatBoxGroovyConDevice,a0
	move.l	4(a5),d0		;get ypos
	lsr.l	#5+3,d0
	bsr	CON_CursorSetY

	lea	_StatBoxGroovyConDevice,a0
	lea	PrintVarFormatString,a1
	bsr	_CON_PutStr

	add.l	#4,sp

	lea	10(a5),a5
	rts

;************************************

_StatPrintScore
	;in: a2=slice, a5=params
	;0	xpos.l
	;4	ypos.l
	;8	player.w
	;10

	move.l	a3,-(sp)

	move.w	8(a5),d0		;player
	jsr	_FindPlayer
	cmp.l	#0,a0
	beq	.Exit
	move.l	a0,a3

;	add.l	#1,OB_Score(a3)

	move.l	OB_Score(a3),d0
	jsr	_ULongToBCD

;	move.l	d0,OB_Score+4(a0)
	move.l	d0,-(sp)

	lea	_StatBoxGroovyConDevice,a0
	move.l	(a5),d0
	asr.l	#5+3,d0
	bsr	CON_CursorSetX
	lea	_StatBoxGroovyConDevice,a0
	move.l	4(a5),d0
	asr.l	#5+3,d0
	bsr	CON_CursorSetY

	lea	_StatBoxGroovyConDevice,a0
	lea	PrintVarFormatString,a1
	bsr	_CON_PutStr

	add.l	#4,sp
.Exit
	lea	10(a5),a5
	move.l	(sp)+,a3
	rts


;************************************

_OldStatPrintScore
	;SYTNAX		- X.w, Y.w, Bob.player

	move.l	4(a5),a0

;	add.l	#1,OB_Score(a0)
	move.l	OB_Score(a0),d0
;	cmp.l	OB_Score+4(a0),d0
;	beq	.Exit

	jsr	_ULongToBCD
	move.l	4(a5),a0

	move.l	d0,OB_Score+4(a0)
	move.l	d0,-(sp)

	lea	_StatBoxGroovyConDevice,a0
	move.w	(a5),d0
	bsr	CON_CursorSetX
	lea	_StatBoxGroovyConDevice,a0
	move.w	2(a5),d0
	bsr	CON_CursorSetY

	lea	_StatBoxGroovyConDevice,a0
	lea	PrintVarFormatString,a1
	bsr	_CON_PutStr

	add.l	#4,sp
.Exit
	lea	8(a5),a5
	rts



_StatPrintGlobalScore
	;SYTNAX		- X.w, Y.w, Flags.w

	move.l	4(a5),d1

	move.l	#0,d0
	lea	SS_Players(a2),a0
	
	lsr.w	d1
	bcc	.Nope1
	move.l	OB_Score(a0),d0
.Nope1
	move.l	OB_Next(a0),a0
	lsr.w	d1
	bcc	.Nope2
	add.l	OB_Score(a0),d0
.Nope2
	move.l	OB_Next(a0),a0
	lsr.w	d1
	bcc	.Nope3
	add.l	OB_Score(a0),d0
.Nope3
	move.l	OB_Next(a0),a0
	lsr.w	d1
	bcc	.Nope4
	add.l	OB_Score(a0),d0
.Nope4
	cmp.l	OldGlobalScore,d0
	beq	.Exit

	move.l	d0,OldGlobalScore
	move.l	d0,-(sp)

	lea	_StatBoxGroovyConDevice,a0
	move.l	(a5),d0
	bsr	CON_CursorSetX
	lea	_StatBoxGroovyConDevice,a0
	move.l	2(a5),d0
	bsr	CON_CursorSetY

	lea	_StatBoxGroovyConDevice,a0
	lea	PrintVarFormatString,a1
	bsr	_CON_PutStr

	add.l	#4,sp
.Exit
	lea	6(a5),a5
	rts


;************************************

;XYZZY - obsolete?
_DUMDEEDOOOLDGiveScore
	;Collision routine
	;SYNTAX:	Points.l

	cmp.l	#0,a4				;Is there a collision partner?
	beq	.Exit				;If no, should generate a warning here!

	move.l	a4,a1				;What type is it?
	cmp.w	#OB_TYPE_PLAYERBULLET,OB_Type(a4)
	bne	.DoIt

	move.l	OB_Parent(a4),a1		;If bullet-type, try to find parent player
	cmp.l	#0,a1
	beq	.Exit
.DoIt
	lea	4(a5),a0			;num of points to add
	lea	OB_Score+4(a1),a1

	move.w	#0,CCR
	abcd	-(a0),-(a1)
	abcd	-(a0),-(a1)
	abcd	-(a0),-(a1)
	abcd	-(a0),-(a1)

.Exit
	lea	4(a5),a5
	rts



_GiveScore	;Actionroutine
	;SYNTAX:	Points.l

	cmp.w	#CONTEXT_COLLISION,_ActionContext
	bne	.Exit

	;additional paranoia
	cmp.l	#0,a4				;Is there a collision partner?
	beq	.Exit				;If no, should generate a warning here!

	move.l	a4,a1				;What type is it?
	cmp.w	#OB_TYPE_PLAYERBULLET,OB_Type(a4)
	bne	.DoIt

	cmp.w	#PARENT_OBJECT,OB_ParentType(a4)
	bne	.Exit
	move.l	OB_Parent(a4),a1		;If bullet-type, try to find parent player
	cmp.l	#0,a1
	beq	.Exit
.DoIt
	move.l	(a5),d0				;num of points to add
	add.l	d0,OB_Score(a1)

.Exit
	lea	4(a5),a5
	rts

;************************************

_EndLevel
	;SYNTAX:	NextLevel.txt

	move.w	#0,_GameOn
	lea	_NextLevelBuffer,a0

	move.l	(a5)+,(a0)+	;Copy The 10 Bytes of the Next Level Name
	move.l	(a5)+,(a0)+
	move.w	(a5)+,(a0)+

	;MYKPRINTF	"GameOff-Set",#0

	;move.l	(a5),d0
	;MYKPRINTF "NextCommand:%ld",d0
	move.w	#1,d7

	rts

;************************************

_Dir8Execute
	;SYNTAX:	Action.prog * 8

	move.b	BB_Direction(a3),d0
	add.b	#4,d0

	lsr.b	#5,d0
	and.w	#$FF,d0
	lsl.w	#2,d0

	move.l	a5,-(sp)
	move.l	(a5,d0.w),a5
	cmp.l	#0,a5
	beq	.ActionDone
	move.w	#CONTEXT_BOB,d0
	bsr	_DoActionList
.ActionDone
	move.l	(sp)+,a5
.Exit
	lea	4*8(a5),a5
	rts

_Dir16Execute
	;SYNTAX:	Action.prog * 16

	move.b	BB_Direction(a3),d0
	add.b	#8,d0

	lsr.b	#4,d0
	and.w	#$FF,d0
	lsl.w	#2,d0

	move.l	a5,-(sp)
	move.l	(a5,d0.w),a5
	cmp.l	#0,a5
	beq	.ActionDone
	move.w	#CONTEXT_BOB,d0
	bsr	_DoActionList
.ActionDone
	move.l	(sp)+,a5
.Exit
	lea	4*16(a5),a5
	rts

_Dir32Execute
	;SYNTAX:	Action.prog * 32

	move.b	BB_Direction(a3),d0
	add.b	#16,d0

	lsr.b	#3,d0
	and.w	#$FF,d0
	lsl.w	#2,d0

	move.l	a5,-(sp)
	move.l	(a5,d0.w),a5
	cmp.l	#0,a5
	beq	.ActionDone
	move.w	#CONTEXT_BOB,d0
	bsr	_DoActionList
.ActionDone
	move.l	(sp)+,a5
.Exit
	lea	4*32(a5),a5
	rts

_Dir64Execute
	;SYNTAX:	Action.prog * 64

	move.b	BB_Direction(a3),d0
	add.b	#32,d0

	lsr.b	#2,d0
	and.w	#$FF,d0
	lsl.w	#2,d0

	move.l	a5,-(sp)
	move.l	(a5,d0.w),a5
	cmp.l	#0,a5
	beq	.ActionDone
	move.w	#CONTEXT_BOB,d0
	bsr	_DoActionList
.ActionDone
	move.l	(sp)+,a5
.Exit
	lea	4*64(a5),a5
	rts


;*****************************************

;XYZZY  - remove 8 & 16
_OldDir8Image
	;SYNTAX: Anim.l,offset.w

	move.l	(a5)+,a0

	move.b	#5,d1		;Shifts
	move.w	(a5)+,d0
	add.b	#31,d0		;Halve Accuarcy
	bsr	_DirXImage
	rts

_OldDir16Image
	;SYNTAX: Anim.l,offset.w

	move.l	(a5)+,a0

	move.b	#4,d1		;Shifts
	move.w	(a5)+,d0
	add.b	#15,d0		;Halve Accuarcy
	bsr	_DirXImage
	rts

_DirXImage
	;inputs:	a6.l = Channel Offset
	;		a3.l = Bob structure

	add.b	BB_Direction(a3),d0

	lsr.b	d1,d0

	and.w	#$FF,d0
	move.w	d0,d1

	lsl.w	#3,d0
	lsl.w	#2,d1
	add.w	d1,d0

	move.l	(a0,d0.w),OB_Image(a3)
	move.l	4(a0,d0.w),d1
	move.l	d1,OB_AnimXOffset(a3)
	move.l	8(a0,d0.w),d1
	move.l	d1,OB_AnimYOffset(a3)
.Exit
	rts



_Dir8Image
	;in: a2=Slice, a3=bob, a5=params
	;0 ImageArray.l[8]
	;32 Anim.l
	;36 Flags.w
	;38

	move.b	BB_Direction(a3),d0
	add.b	#16,d0
	and.w	#$FF,d0
	lsr.w	#5,d0	;down to 0..7

	move.l	32(a5),a0	;anim set?
	cmp.l	#0,a0
	bne	.FromAnim

	lsl.w	#2,d0
	move.l	(a5,d0.w),d0
	beq	.Exit		;null image

	move.l	d0,OB_Image(a3)
	move.l	#0,OB_AnimXOffset(a3)
	move.l	#0,OB_AnimYOffset(a3)
	bra	.Exit

.FromAnim
	bsr	ShowFrameFromAnim
.Exit
	lea	38(a5),a5
	rts


_Dir16Image
	;in: a2=Slice, a3=bob, a5=params
	;0 ImageArray.l[16]
	;64 Anim.l
	;68 Flags.w
	;70

	move.b	BB_Direction(a3),d0
	;XYZZY
;	add.b	#8,d0
	and.w	#$FF,d0
	lsr.w	#4,d0	;down to 0..15

	move.l	64(a5),a0	;anim set?
	cmp.l	#0,a0
	bne	.FromAnim

	lsl.w	#2,d0
	move.l	(a5,d0.w),d0
	beq	.Exit		;null image

	move.l	d0,OB_Image(a3)
	move.l	#0,OB_AnimXOffset(a3)
	move.l	#0,OB_AnimYOffset(a3)
	bra	.Exit

.FromAnim
	bsr	ShowFrameFromAnim
.Exit
	lea	70(a5),a5
	rts


_Dir32Image
	;in: a2=Slice, a3=bob, a5=params
	;Anim.l, Flags.w

	move.l	(a5),d0
	beq	.Exit

	move.l	d0,a0
	move.b	BB_Direction(a3),d0
	add.b	#4,d0
	and.w	#$FF,d0
	lsr.w	#3,d0	;down to 0..32
	bsr	ShowFrameFromAnim
.Exit
	lea	6(a5),a5
	rts


_Dir64Image
	;in: a2=Slice, a3=bob, a5=params
	;Anim.l, Flags.w

	move.l	(a5),d0
	beq	.Exit

	move.l	d0,a0
	move.b	BB_Direction(a3),d0
	add.b	#2,d0
	and.w	#$FF,d0
	lsr.w	#2,d0	;down to 0..63
	bsr	ShowFrameFromAnim
.Exit
	lea	6(a5),a5
	rts


ShowFrameFromAnim
	;in: a0=anim, a3=bob, d0.w=framenum

	move.w	d0,d1

	lsl.w	#3,d0
	lsl.w	#2,d1
	add.w	d1,d0

	move.l	(a0,d0.w),OB_Image(a3)
	move.l	4(a0,d0.w),d1
	move.l	d1,OB_AnimXOffset(a3)
	move.l	8(a0,d0.w),d1
	move.l	d1,OB_AnimYOffset(a3)
	rts

;*****************************************


_SetXSpeedsFromList32
	;SYNTAX:	XSpeed.l * 32

	move.b	BB_Direction(a3),d0
	add.b	#4,d0

	lsr.b	#3,d0
	and.w	#$FF,d0
	lsl.w	#2,d0

	move.l	(a5,d0.w),BB_XSpeed(a3)

	lea	4*32(a5),a5
	rts

_SetYSpeedsFromList32
	;SYNTAX:	YSpeed.l * 32

	move.b	BB_Direction(a3),d0
	add.b	#4,d0

	lsr.b	#3,d0
	and.w	#$FF,d0
	lsl.w	#2,d0

	move.l	(a5,d0.w),BB_YSpeed(a3)

	lea	4*32(a5),a5
	rts


;*****************************************

_RNDDirectionMove
	;SYNTAX:	Speed.l

	bsr	_Random

	move.b	d0,BB_Direction(a3)
	move.l	(a5)+,d1

	cmp.l	#0,d1
	bne	.UseMag
	move.l	BB_DirSpeed(a3),d1
.UseMag
	bsr	_SetSpeedsFromDirection
	move.l	d0,BB_XSpeed(a3)
 	move.l	d1,BB_YSpeed(a3)

	rts

_SetVar
	;SYNTAX:	Variable.w, Value.l

	lea	BB_Data1(a3),a0

	tst.w	(a5)+
	beq	.Var1

	add.l	#4,a0
.Var1
	move.l	(a5)+,(a0)
	rts

_AddToVar
	;SYNTAX:	Variable.w, Value.l
	lea	BB_Data1(a3),a0

	tst.w	(a5)+
	beq	.Var1

	add.l	#4,a0
.Var1
	move.l	(a5)+,d0
	add.l	d0,(a0)
	rts

_SubFromVar
	;SYNTAX:	Variable.w, Value.l
	lea	BB_Data1(a3),a0

	tst.w	(a5)+
	beq	.Var1

	add.l	#4,a0
.Var1
	move.l	(a5)+,d0
	sub.l	d0,(a0)
	rts

_ExecuteIfVarCC
	;SYNTAX:	Variable.w, Value.l, CC.w, Action.prog

	lea	BB_Data1(a3),a0

	tst.w	(a5)
	beq	.Var1

	add.l	#4,a0
.Var1
	move.l	2(a5),d1
	sub.l	(a0),d1

	move.w	6(a5),d0
	bne	.Bne
	tst.l	d1
	beq	.Execute
	bra	.Exit
.Bne
	cmp.w	#1,d0
	bne	.Exit
	tst.l	d1
	bne	.Execute
	bra	.Exit

.Execute
	move.l	a5,-(sp)
	move.l	8(a5),a5
	cmp.l	#0,a5
	beq	.ActionDone

	move.w	_ActionContext,d0	;stay in same context
	bsr	_DoActionList
.ActionDone
	move.l	(sp)+,a5

.Exit
	lea	12(a5),a5
	rts	

;*****************************************

_SetDirection
	move.w	(a5)+,d0
	move.b	d0,BB_Direction(a3)
	rts

_RotateClockWise
	move.w	(a5)+,d0
	add.b	d0,BB_Direction(a3)
	rts

_RotateAntiClockWise
	move.w	(a5)+,d0
	sub.b	d0,BB_Direction(a3)
	rts

;*****************************************

_ConstrainToArc
	;SYNTAX: Dir1.w, Dir2.w

	move.b	BB_Direction(a3),d0
	and.w	#$FF,d0
	sub.w	(a5),d0
	move.w	2(a5),d1

	sub.w	(a5),d1

	cmp.b	d0,d1
	bhs	.Exit
.Crop
	lsr.b	d1
	add.b	#128,d1

	cmp.b	d0,d1
	bhs	.Crop2nd
.Crop1st
	move.w	(a5),d0
	move.b	d0,BB_Direction(a3)
	bra	.Exit
.Crop2nd
	move.w	2(a5),d0
	move.b	d0,BB_Direction(a3)
.Exit
	lea	4(a5),a5
	rts

;*****************************************

_Repel
	;Collision Routine
	;a3.l = Object to affect
	;a4.l = Collision partner
	;SYNTAX:	X Shifts.w, Y Shifts.w

	cmp.l	#0,a4
	beq	.Exit

	move.w	(a5),d0
	and.l	#$FFFF,d0
	move.l	BB_XSpeed(a4),d1
	asr.l	d0,d1
	add.l	d1,BB_XSpeed(a3)

	move.w	2(a5),d0
	and.l	#$FFFF,d0
	move.l	BB_YSpeed(a4),d1
	asr.l	d0,d1
	add.l	d1,BB_YSpeed(a3)

.Exit
	lea	4(a5),a5
	rts

;*****************************************

_SetI	;SYNTAX:	SETI Value.l, Var.w


	move.l	(a5),d1

	move.w	4(a5),d0
	lsl.w	#2,d0

	lea	_SetVariableTable,a0
	move.l	(a0,d0.w),a0
	jsr	(a0)

	lea	6(a5),a5
	rts

;*********

_AddI	;SYNTAX:	ADDI Value.l, Var.w

	move.l	(a5),d1

	move.w	4(a5),d0
	lsl.w	#2,d0

	lea	_AddVariableTable,a0
	move.l	(a0,d0.w),a0
	jsr	(a0)

	lea	6(a5),a5
	rts

;*********

_SetV	;SYNTAX:	SETV SourceVar.w, DestVar.w

	move.l	#0,d1

	move.w	(a5),d0
	lsl.w	#2,d0

	lea	_GetVariableTable,a0
	move.l	(a0,d0.w),a0
	jsr	(a0)

	move.w	2(a5),d0
	lsl.w	#2,d0

	lea	_SetVariableTable,a0
	move.l	(a0,d0.w),a0
	jsr	(a0)

	lea	4(a5),a5
	rts

;*********

_AddV	;SYNTAX:	AddV SourceVar.w, DestVar.w

	move.l	#0,d1

	move.w	(a5),d0
	lsl.w	#2,d0

	lea	_GetVariableTable,a0
	move.l	(a0,d0.w),a0
	jsr	(a0)

	move.w	2(a5),d0
	lsl.w	#2,d0

	lea	_AddVariableTable,a0
	move.l	(a0,d0.w),a0
	jsr	(a0)

	lea	4(a5),a5
	rts

;*********

;XYZZY - obsolete?

_OLDSetRND	;SYNTAX:	SetRND	Shifts.w, DestVar.w


	bsr	_Random
	move.w	(a5),d1
	asr.w	d1,d0
	move.w	d0,d1
	ext.l	d1

	move.w	2(a5),d0
	lsl.w	#2,d0

	lea	_SetVariableTable,a0
	move.l	(a0,d0.w),a0
	jsr	(a0)

	lea	4(a5),a5
	rts

;*********

;XYZZY - obsolete?

_OLDAddRND	;SYNTAX:	AddRND	Shifts.w, DestVar.w


	bsr	_Random
	move.w	(a5),d1
	asr.w	d1,d0
	move.w	d0,d1
	ext.l	d1

	move.w	2(a5),d0
	lsl.w	#2,d0

	lea	_AddVariableTable,a0
	move.l	(a0,d0.w),a0
	jsr	(a0)

	lea	4(a5),a5
	rts

;*********

_CompareVariable
	;SYNTAX:	CMPV SourceVar.w, DestVar.w

	move.l	#0,d1

	move.w	(a5),d0
	lsl.w	#2,d0

	lea	_GetVariableTable,a0
	move.l	(a0,d0.w),a0
	jsr	(a0)

	neg.l	d1

	move.w	2(a5),d0
	lsl.w	#2,d0

	lea	_AddVariableTable,a0
	move.l	(a0,d0.w),a0
	jsr	(a0)

	bsr	_SetActionCCR

.Exit
	lea	4(a5),a5
	rts

;*********

_CompareImediate
	;SYNTAX:	CMPV Source.l, DestVar.w


	move.w	4(a5),d0
	lsl.w	#2,d0

	lea	_GetVariableTable,a0
	move.l	(a0,d0.w),a0
	jsr	(a0)

	neg.l	d1
	add.l	(a5),d1

	bsr	_SetActionCCR

.Exit
	lea	6(a5),a5
	rts

;*********
;inputs: a3 = object, d0 = var offset (Var.w * 4)
;outputs: d1.l = value

GetObjectVar
	lsr.w	d0
	lea	VariableTable,a0
	move.w	(a0,d0.w),d0
	move.l	(a3,d0.w),d1
	rts

GetWordObjectVar
	lsr.w	d0
	lea	VariableTable,a0
	move.w	(a0,d0.w),d0
	move.w	(a3,d0.w),d1
	ext.l	d1
	rts

GetByteObjectVar
	lsr.w	d0
	lea	VariableTable,a0
	move.w	(a0,d0.w),d0
	move.b	(a3,d0.w),d1
	ext.w	d1
	ext.l	d1
	rts

;inputs:
;	a3 = object
;	d0 = var offset (Var.w * 4)
;	d1.l = value
;Outputs: none

SetObjectVar
	lsr.w	d0
	lea	VariableTable,a0
	move.w	(a0,d0.w),d0
	move.l	d1,(a3,d0.w)
	rts

SetWordObjectVar
	lsr.w	d0
	lea	VariableTable,a0
	move.w	(a0,d0.w),d0
	move.w	d1,(a3,d0.w)
	rts

SetByteObjectVar
	lsr.w	d0
	lea	VariableTable,a0
	move.w	(a0,d0.w),d0
	move.b	d1,(a3,d0.w)
	rts

AddObjectVar
	lsr.w	d0
	lea	VariableTable,a0
	move.w	(a0,d0.w),d0
	add.l	d1,(a3,d0.w)
	rts

AddWordObjectVar
	lsr.w	d0
	lea	VariableTable,a0
	move.w	(a0,d0.w),d0
	add.w	d1,(a3,d0.w)
	rts

AddByteObjectVar
	lsr.w	d0
	lea	VariableTable,a0
	move.w	(a0,d0.w),d0
	add.b	d1,(a3,d0.w)
	rts

GetGlobalVar
	lsr.w	d0
	lea	VariableTable,a0
	move.w	(a0,d0.w),d0
	lea	_GlobalData,a0
	move.l	(a0,d0.w),d1
	rts
SetGlobalVar
	lsr.w	d0
	lea	VariableTable,a0
	move.w	(a0,d0.w),d0
	lea	_GlobalData,a0
	move.l	d1,(a0,d0.w)
	rts
AddGlobalVar
	lsr.w	d0
	lea	VariableTable,a0
	move.w	(a0,d0.w),d0
	lea	_GlobalData,a0
	add.l	d1,(a0,d0.w)
	rts

GetWordSliceVar
	lsr.w	d0
	lea	VariableTable,a0
	move.w	(a0,d0.w),d0
	move.w	(a2,d0.w),d1
	ext.l	d1
	rts

SetWordSliceVar
	lsr.w	d0
	lea	VariableTable,a0
	move.w	(a0,d0.w),d0
	move.w	d1,(a2,d0.w)
	rts

AddWordSliceVar
	lsr.w	d0
	lea	VariableTable,a0
	move.w	(a0,d0.w),d0
	add.w	d1,(a2,d0.w)
	rts

GetScrollVar
	lsr.w	d0
	lea	VariableTable,a0
	move.w	(a0,d0.w),d0
	move.l	SS_Scroll(a2),a0
	move.l	(a0,d0.w),d1
	rts

SetScrollVar
	lsr.w	d0
	lea	VariableTable,a0
	move.w	(a0,d0.w),d0
	move.l	SS_Scroll(a2),a0
	move.l	d1,(a0,d0.w)
	rts

AddScrollVar
	lsr.w	d0
	lea	VariableTable,a0
	move.w	(a0,d0.w),d0
	move.l	SS_Scroll(a2),a0
	add.l	d1,(a0,d0.w)
	rts

;*****************************************

_IfEqual
	;SYNTAX:	Beq	Action.l

	tst.w	ActionCCR
	bne	.Exit

	move.l	a5,-(sp)
	move.l	(a5),a5
	cmp.l	#0,a5
	beq	.NoAction

	move.w	_ActionContext,d0
	bsr	_DoActionList
.NoAction
	move.l	(sp)+,a5
.Exit
	lea	4(a5),a5
	rts

;*********

_IfNotEqual
	;SYNTAX:	Bne	Action.l

	tst.w	ActionCCR
	beq	.Exit

	move.l	a5,-(sp)
	move.l	(a5),a5
	cmp.l	#0,a5
	beq	.NoAction
	move.w	_ActionContext,d0
	bsr	_DoActionList
.NoAction
	move.l	(sp)+,a5
.Exit
	lea	4(a5),a5
	rts

;*********

_IfGreaterThan
	;SYNTAX:	Bgt	Action.l

	tst.w	ActionCCR
	beq	.Exit
	bmi	.Exit

	move.l	a5,-(sp)
	move.l	(a5),a5
	cmp.l	#0,a5
	beq	.NoAction

	move.w	_ActionContext,d0
	bsr	_DoActionList
.NoAction
	move.l	(sp)+,a5
.Exit
	lea	4(a5),a5
	rts

;*********

_IfLowerThan
	;SYNTAX:	Blt	Action.l

	tst.w	ActionCCR
	beq	.Exit
	bpl	.Exit

	move.l	a5,-(sp)
	move.l	(a5),a5
	cmp.l	#0,a5
	beq	.NoAction

	move.w	_ActionContext,d0
	bsr	_DoActionList
.NoAction
	move.l	(sp)+,a5
.Exit
	lea	4(a5),a5
	rts

;*****************************************

_SetActionCCR
	tst.l	d1
	beq	.Equ
	bmi	.GT
	bpl	.LT
	move.w	#1,ActionCCR
	bra	.Exit
.Equ	move.w	#0,ActionCCR
	bra	.Exit
.GT	move.w	#1,ActionCCR
	bra	.Exit
.LT	move.w	#-1,ActionCCR
.Exit
	rts

;*****************************************

_SetObjectAction
	;	Action.l, Var.w

	move.l	(a5)+,a0
	move.w	(a5)+,d0

	lsl.w	d0
	cmp.w	#6,d0
	bgt	.Exit

	lea	SetObjectActionTable,a1
	move.w	(a1,d0.w),d1
	move.l	a0,(a3,d1.w)
.Exit
	rts


;*****************************************

_SetCollisions
	;	State.w		- 0 = off, 1 = On

	tst.w	OB_State(a3)
	beq	.Exit
	tst.w	(a5)
	beq	.Off
	move.w	#1,OB_State(a3)
	bra	.Exit
.Off
	move.w	#-1,OB_State(a3)
.Exit
	lea	2(a5),a5
	rts

;*****************************************

_PlayModule
	;FileRef

	move.l	(a5)+,d0
	beq	.Exit
	tst.b	_ModulePlayerEnable
	beq	.PlayIt
	cmp.l	_SongDataPtr,d0		;If already playing that module, then just
	beq	.Exit			;ignore this.
.PlayIt
	move.l	d0,a0
	move.l	d0,a1
	add.l	-4(a0),a1		;Add Module Size (From MemNode Struct)
	bsr	_InitModulePlayer
	move.b	#1,_ModulePlayerEnable
.Exit
	rts

_StopModule

	bsr	_StopModulePlayer
	rts

;*****************************************

_OLDInitBackgroundPointCollision
	;Inputs:	a1.l = Channel
	;		a3.l = Bob
	;		a5.l = Params
	;SYNTAX:	LastSpaceBlock.w, ActionList.l, Shifts.w, XOffset.l, YOffset.l

	move.w	(a5)+,CL_Data1(a1)	;Last SpaceBlock
	move.l	(a5)+,CL_Data2(a1)	;Action List
	move.w	(a5)+,CL_Data1+2(a1)	;Shifts
	move.l	(a5)+,CL_Data3(a1)	;X Offsets
	move.l	(a5)+,CL_Data4(a1)	;Y Offsets
	move.l	#_OLDBackgroundPointCollision,CL_UpdateCode(a1)
	rts

;*****************************************

_ExecuteOnPlayer
	;PlayerRef.l,Action.l

	move.l	4(a5),a0
	cmp.l	#0,a0
	beq	.Exit
	move.l	a3,-(sp)
	move.l	a5,-(sp)

	move.l	(a5),a3
	move.l	a0,a5
	move.w	#CONTEXT_BOB,d0
	bsr	_DoActionList

	move.l	(sp)+,a5
	move.l	(sp)+,a3
.Exit
	lea	8(a5),a5
	rts

;*****************************************

_ExecuteIfOnScreen
	;Action.l

	btst.b	#2,BB_Flags+3(a3)
	beq	.Exit

	move.l	(a5),d0
	beq	.Exit
	move.l	a5,-(sp)
	move.l	d0,a5
	move.w	#CONTEXT_BOB,d0
	bsr	_DoActionList
	move.l	(sp)+,a5
.Exit
	lea	4(a5),a5
	rts


;*****************************************
;*****************************************

*****************************************************************

	SECTION Data,data

*****************************************************************
ActionCCR
	ds.w	1

SetObjectActionTable
	dc.w	BB_HitRoutine1, BB_HitRoutine2, BB_HitRoutine3, BB_DeathHu
SetObjectActionTable_END


VariableTable
.ObjectStuff
		dc.w	OB_XPos,	OB_YPos,	BB_XSpeed,	BB_YSpeed
		dc.w	BB_XMaxSpeed,	BB_YMaxSpeed,	BB_XAccel,	BB_YAccel
		dc.w	BB_XDeccel,	BB_YDeccel,	BB_Energy,	BB_MaxEnergy
		dc.w	BB_Data1,	BB_Data2
		dc.w	BB_Direction
.GlobalStuff
		dc.w	0,		4,		8,		12
		dc.w	16
.SliceStuff
		dc.w	SS_CollisionTypes
.ScrollStuff
		dc.w	SC_Pos,		SC_YPos,	SC_Speed,	SC_BaseSpeed
;more object vars
		dc.w	BB_ID

;ptrs to routines for reading variables - index by variable id
_GetVariableTable
.ObjectStuff	dc.l	GetObjectVar,	GetObjectVar,	GetObjectVar,	GetObjectVar
		dc.l	GetObjectVar,	GetObjectVar,	GetObjectVar,	GetObjectVar
		dc.l	GetObjectVar,	GetObjectVar,	GetWordObjectVar,GetWordObjectVar
		dc.l	GetObjectVar,	GetObjectVar,	GetByteObjectVar
.GlobalStuff	dc.l	GetGlobalVar,	GetGlobalVar,	GetGlobalVar,	GetGlobalVar
		dc.l	GetGlobalVar
		dc.l	GetWordSliceVar
		dc.l	GetScrollVar,	GetScrollVar,	GetScrollVar,	GetScrollVar
;more object vars
		dc.l	GetWordObjectVar

;ptrs to routines for seting variables - index by variable id
_SetVariableTable
.ObjectStuff	dc.l	SetObjectVar,	SetObjectVar,	SetObjectVar,	SetObjectVar
		dc.l	SetObjectVar,	SetObjectVar,	SetObjectVar,	SetObjectVar
		dc.l	SetObjectVar,	SetObjectVar,	SetWordObjectVar,SetWordObjectVar
		dc.l	SetObjectVar,	SetObjectVar,	SetByteObjectVar
.GlobalStuff	dc.l	SetGlobalVar,	SetGlobalVar,	SetGlobalVar,	SetGlobalVar
		dc.l	SetGlobalVar
		dc.l	SetWordSliceVar
		dc.l	SetScrollVar,	SetScrollVar,	SetScrollVar,	SetScrollVar
;more object vars
		dc.l	SetWordObjectVar

;ptrs to routines for adding variables - index by variable id
_AddVariableTable
.ObjectStuff	dc.l	AddObjectVar,	AddObjectVar,	AddObjectVar,	AddObjectVar
		dc.l	AddObjectVar,	AddObjectVar,	AddObjectVar,	AddObjectVar
		dc.l	AddObjectVar,	AddObjectVar,	AddWordObjectVar,AddWordObjectVar
		dc.l	AddObjectVar,	AddObjectVar,	AddByteObjectVar
.GlobalStuff	dc.l	AddGlobalVar,	AddGlobalVar,	AddGlobalVar,	AddGlobalVar
		dc.l	AddGlobalVar
		dc.l	AddWordSliceVar
		dc.l	AddScrollVar,	AddScrollVar,	AddScrollVar,	AddScrollVar
;more object vars
		dc.l	AddWordObjectVar

PrintVarFormatString
		dc.b	"%8lx",0
		even

OldGlobalScore	dc.l	0

_SpriteObjects		ds.l	8

;XYZZY - KILL!
_RNDXSpeed
_RNDYSpeed

; Commands & Templates
* 0	_Null
* 1	_InitXSpeedAnim		- Left Threshold,Left Anim, Right Threshold, Right Anim, Stationary Anim
* 2	_InitLinearAccel	- XSpd.l, YSpd.l, XAccell.l, YAccell.l,Flags.w
* 3	_InitAsteroidsPlayer
* 4	_InitInertialPlayer
* 5	_OBSFIREB		- Image.bob, XOffset.l, YOffset.l, Actionlist.prog
* 6	_EXECUTE		- Actionlist.prog
* 7	_InitCyclicAnimation	- Anim.anim,Startframe.w,Cycles.w
* 8	_InitDir16Anim		-
* 9	_InitBackgroundCollision- Last space block.w,Action.l, Shifts.w
* 10	_InitInertialJoyMovement- XMaxSpeed.l,	YMaxSpeed.l, XAccell.l,	YAccell.l, XDeccel.l, YDeccel.l, Channel.w, FireFlags.w, Flags.w, LeftAction.l,	RightAction.l, TopAction.l, BottomAction.l
* 11	_KILLOBJECT		- NULL
* 12	_ScrollModeConstantXSpeed	- Speed.l, AllowedScrollBack.l,backspeed.l,backzonewidth.l, endactionlist.l,flags.w
* 13	_InitDirectionalHomer	- Type.w ,Player.l
* 14	_InitPlatformPlayer	- MaxSpeed.l, XAccell.l, YAccell.l, XDeccel.l, YDeccel.l
* 15	_StatPrintVar		- xpos.l,ypos.l,var.w
* 16	_InitPlatformPlayer
* 17	_SETImage		- Image.bob
* 18	_InitXPushSliceTrack	- Player, Left,Right,Max
* 19	_InitYPushSliceTrack	- Player, Top,Bottom,Max
* 20	_InitScroll		- XPos.l, YPos.l, Speed.l, BaseSpeed.l
* 21	_GetHurt		- NULL
* 22	_FireBay		- Bay.w
* 23	_PowerUp		- Bay.w, NoWeap.Weapon
* 24	_PowerDown		- Bay.w, Flags.w
* 25	_SetWeaponBay		- Bay.w, Action.prog
* 26	_SetChannel		- ChannelNumber.w, ChannelInit.prog, Timer.w, Done.prog
* 27	_HighlightBob		- Cycles.w
* 28	_SetColour		- Color.b, Red.b, Green.b, Blue.b
* 29	_KillChannel		- Channel.w
* 30	_InitMousePlayer
* 31	_InitActionListPlayer
* 32	_InitCheckFireButtons
* 33	_InitYSpeedAnim
* 34	_SimpleSFX		- Sound.sfx, Channel.w
* 35	_PlaySFX		-
* 36	_VBlankSprite		- NULL
* 37	_MainLoopSprite		- NULL
* 38	_InitInertialHomer
* 39	_StatPrintScore
* 40	_InitHomerBadDude1	- Type.w, PlayerRef.l, XMaxSpeed.l, YMaxSpeed.l, XAccel.l, YAccel.l,   XOffset.l, YOffset.l
* 41	_InitTimeDelayExecute	- ActionList.prog, Delay.w, repeats.w
* 42	_InitFollowPath		- Path.path, flags.w
* 43	_OLDDirectionMove		- SpeedMag.l
* 44	_InitChannelExecute	- Action.prog, Delay.w
* 45	_InitDirectionMove	- SpeedMag.l, Delay.w
* 46	_OldInitDir8Anim		- Anim.l, Startframe.w, *8
* 47	_OldInitDir16Anim		- Anim.l, Startframe.w, *16
* 48	_OldInitDir32Anim		- Anim.l, Startframe.w, *32
* 49	_OldInitDir64Anim		- Anim.l, Startframe.w, *64
* 50	_OldInitDir8Image		- Anim.l
* 51	_OldInitDir16Image		- Anim.l
* 52	_OldInitDir32Image		- Anim.l
* 53	_OldInitDir64Image		- Anim.l
* 54	_OldInitActionListPlayer	- InputChannel.w, U.prog, D.prog, L.prog, R.Prog, Flags.l
* 55	_SpawnPlayerBullet	- Image.im, XOffset.l,YOffset.l,ActionList.prog
* 56	_SpawnBadDude		- Image.im, XOffset.l,YOffset.l,ActionList.prog
* 57	_SpawnPlayer		- Image.im, XOffset.l,YOffset.l,ActionList.prog
* 58	_InitDir8Action		- Action * 8
* 59	_InitDir16Action	- Action * 16
* 60	_InitDir32Action	- Action * 32
* 61	_InitDir64Action	- Action * 64
* 62	_OldStatPrintScore		- X.b, Y.b, Player
* 63	_StatPrintGlobalScore	- X.b, Y.b, Flags
* 64	_GiveScore		- Score.l
* 65	_EndLevel		- Next.txt
* 66	_Dir8Execute		- Action*8
* 67	_Dir16Execute		- Action*16
* 68	_Dir32Execute		- Action*32
* 69	_Dir64Execute		- Action*64
* 70	_InitNULL		- NULL
* 71	_RNDXSpeed		- Shifts.w
* 72	_RNDYSpeed		- Shifts.w
* 73	_InitSteroids		- Channel.w, Yaw.w, DirMaxSpeed.l, Acceleration.l, Decelleration.l, DownAction.l
* 74	_SetBorder		- L,t,R,t,T,t,B,t,Map/View
* 75	_RNDDirectionMove	- Speed.l
* 76	_SetVar			- Variable.w, Value.l
* 77	_AddToVar		- Variable.w, Value.l
* 78	_SubFromVar		- Variable.w, Value.l
* 79	_ExecuteIfVarCC		- Variable.w, Value.l, CC.w, Action.prog
* 80	_OldDir8Image		- Anim.l, Offset.w
* 81	_OldDir16Image		- Anim.l, Offset.w
* 82	_Dir32Image		- Anim.l, Flags.w
* 83	_Dir64Image		- Anim.l, Flags.w
* 84	_RotateClockWise	- Brabiens.w
* 85	_RotateAntiClockWise	- Brabiens.w
* 86	_SetXSpeedsFromList32	- XSpd.l *32
* 87	_SetYSpeedsFromList32	- XSpd.l *32
* 88	_SetDirection		- Dir.w
* 89	_InitAttatch		- Player.plyr, XOffset.l, YOffset.l
* 90	_SetMiscStuff 		- HitRoutine1.l, HitRoutine2.l, HitRoutine3.l, Damage.w, Energy.w, DeathHu.l, Flags - Border,Speed
* 91	_InitOperationWolfMovement - Input Channel.w
* 92	_Repel			- XShifts.w, YShifts.w
* 93	_SetI			- Value.l, DestVar.w
* 94	_AddI			- Value.l, DestVar.w
* 95	_SetV			- SourceVar.w, DestVar.w
* 96	_AddV			- SourceVar.w, DestVar.w
* 97	_OLDSetRND			- Shifts.w, DestVar.w
* 98	_OLDAddRND			- Shifts.w, DestVar.w
* 99	_ObjectOff		- NULL
* 100	_CollisionPartnerExecute- Action.l
* 101	_CollisionPartnerOff	- NULL
* 102	_KillCollisionPartner	- NULL
* 103	_JoystickAnimation	- InputChannel.w, Flags.w, Anim*18
* 104	_SetObjectAction	- Action,Number.w
* 105	_SetCollisions		- State
* 106	_PlayModule		- FileRef
* 107	_StopModule		- NULL
* 108	_DoActionOnFire		- Input Channel.w, Delay.w, Action.l
* 109	_CompareVariable	- SourceVar,DestVar
* 110	_CompareImediate	- Imediate,DestVar
* 111	_IfEqual		- Action
* 112	_IfNotEqual		- Action
* 113	_IfGreaterThan		- Action
* 114	_IfLowerThan		- Action
* 115	_OLDInitBackgroundPointCollision - LastSpaceBlock.w, ActionList.l, Shifts.w, XOffset.l, YOffset.l 
* 116	_SimplePlatformPlayer	- InputChannel.w, XAccel.l, Gravity.l, JumpSpeed.l, MaxXSpeed.l, MaxYSpeed.l, LastSpace.w, LastFloor.w, Flags.w
* 117	_ExecuteOnPlayer	- Player.l,Action.l
* 118	_ExecuteIfOnScreen	- Action.l
* 119	_InitAttachToParent	- X Offset.l,Yoffset.l
* 120	_InitDrawBar		- XPos.w, YPos.w, BarWidth.w, MaxVal.l, Variable.w
* 121	_InitFloorSentry	- LSpd.l, RSpd.l, Eotr.eotr, highspace.b, flags.b
* 122	_InitCeilingSentry	- LSpd.l, RSpd.l, Eotr.eotr, highspace.b, flags.b
* 123	_InitLinearPlayer
* 124	_InitBGPointCollision	- XOffset.l, YOffset.l, Eotr.eotr, highspace.b, reserved.b, ActionList.l, DampingShifts.w
* 124	_InitBGCollision
* 126	_InitTrackPlayer	- player.l, speed.l, turnspeed.l, flags.w
* 127	_DirectionMove		- speed.l, flags.w
* 128	_InitJitterSpeed	- xrndmin.l,xrndmax.l,xspdmax.l,yrndmin.l,yrndmax.l,yspdmax.l,which.w,flags.w
* 129	_SetRND			- min.l,max.l,var.w
* 130	_AddRND			- min.l,max.l,var.w
* 131	_ScrollXCenterPlayers	- maxspd.l, trackplayer.w, reserved.w, reserved.l
* 132	_InitDir8Image		- ImageArray[8].l, anim.l, flags.w
* 133	_InitDir16Image		- ImageArray[16].l, anim.l, flags.w
* 134	_InitDir32Image		- anim.l, flags.w
* 135	_InitDir64Image		- anim.l, flags.w
* 136	_Dir8Image		- ImageArray[8].l, anim.l, flags.w
* 137	_Dir16Image		- ImageArray[16].l, anim.l, flags.w
* 138	_InitDir8Anim		- Flags.w, ImageArray.l[8]
* 139	_InitDir16Anim		- Flags.w, ImageArray.l[16]
* 140	_InitDir32Anim		- Flags.w, ImageArray.l[32]
* 141	_InitDir64Anim		- Flags.w, ImageArray.l[64]
* 142	_RestrictDirection	- type.w, min.l,max.l
* 143	_RandomEvent
* 144	_InitRandomEvent
* 145	_InitMousePointer

_OperatorTable		dc.l	0,				_InitXSpeedAnim,		_InitLinearAccel
			dc.l	_InitAsteroidsPlayer,		_InitInertialPlayer,		_SpawnPlayerBullet
			dc.l	_EXECUTE,			_InitCyclicAnimation,		0
			dc.l	_InitBackgroundCollision,	_InitInertialJoyMovement,	_KILLOBJECT
			dc.l	_ScrollModeConstantXSpeed,	_InitDirectionalHomer,		0
			dc.l	_StatPrintVar,			_InitPlatformPlayer,		_SETIMAGE
			dc.l	_InitXPushSliceTrack,		_InitYPushSliceTrack,		_InitScroll
			dc.l	_GetHurt,			_FireWeaponBay,			_PowerUpWeaponBay
			dc.l	_PowerDownWeaponBay,		_SetWeaponBay,			_SetChannel
			dc.l	_HighlightBob,			_SetColour,			_KillChannel
			dc.l	_InitMousePlayer,		_InitActionListPlayer,		_InitCheckFireButtons
			dc.l	_InitYSpeedAnim,		_SimpleSFX,			_PlaySFX
			dc.l	_VBlankSprite,			_MainLoopSprite,		_InitInertialHomer
			dc.l	_StatPrintScore,		_InitHomerBadDude1,		_InitTimeDelayExecute
			dc.l	_InitFollowPath,		_OLDDirectionMove,		_InitChannelExecute
			dc.l	_InitDirectionMove,		_OldInitDir8Anim,		_OldInitDir16Anim
			dc.l	_OldInitDir32Anim,		_OldInitDir64Anim,		_OldInitDir8Image
			dc.l	_OldInitDir16Image,		_OldInitDir32Image,		_OldInitDir64Image
			dc.l	_OldInitActionListPlayer,	_SpawnPlayerBullet,		_SpawnBadDude
			dc.l	_SpawnPlayer,			_InitDir8Action,		_InitDir16Action
			dc.l	_InitDir32Action,		_InitDir64Action,		_OldStatPrintScore
			dc.l	_StatPrintGlobalScore,		_GiveScore,			_EndLevel
			dc.l	_Dir8Execute,			_Dir16Execute,			_Dir32Execute
			dc.l	_Dir64Execute,			_InitNULL,			_RNDXSpeed
			dc.l	_RNDYSpeed,			_InitSteroids,			_SetBorder
			dc.l	_RNDDirectionMove,		_SetVar,			_AddToVar
			dc.l	_SubFromVar,			_ExecuteIfVarCC,		_OldDir8Image
			dc.l	_OldDir16Image,			_Dir32Image,			_Dir64Image
			dc.l	_RotateClockWise,		_RotateAntiClockWise,		_SetXSpeedsFromList32
			dc.l	_SetYSpeedsFromList32,		_SetDirection,			_InitAttatch
			dc.l	_SetMiscStuff,			_InitOperationWolfMovement,	_Repel
			dc.l	_SetI,				_AddI,				_SetV
			dc.l	_AddV,				_OLDSetRND,			_OLDAddRND
			dc.l	_OBJECTOFF,			_CollisionPartnerExecute,	_CollisionPartnerOff
			dc.l	_KillCollisionPartner,		_InitJoystickAnim,		_SetObjectAction
			dc.l	_SetCollisions,			_PlayModule,			_StopModule
			dc.l	_InitActionOnFire,		_CompareVariable,		_CompareImediate
			dc.l	_IfEqual,			_IfNotEqual,			_IfGreaterThan
			dc.l	_IfLowerThan,			_OLDInitBackgroundPointCollision,	_SimplePlatformPlayer
			dc.l	_ExecuteOnPlayer,		_ExecuteIfOnScreen,		_InitAttachToParent
			dc.l	_InitDrawBar,			_InitFloorSentry,		_InitCeilingSentry
			dc.l	_InitLinearPlayer,		_InitBGPointCollision,		_InitBGCollision
			dc.l	_InitTrackPlayer,		_DirectionMove,			_InitJitterSpeed
			dc.l	_SetRND,			_AddRND,			_ScrollXCenterPlayers
			dc.l	_InitDir8Image,			_InitDir16Image,		_InitDir32Image
			dc.l	_InitDir64Image,		_Dir8Image,			_Dir16Image
			dc.l	_InitDir8Anim,			_InitDir16Anim,			_InitDir32Anim
			dc.l	_InitDir64Anim,			_RestrictDirection,		_RandomEvent
			dc.l	_InitRandomEvent,		_InitMousePointer,		_ScrollYCenterPlayers


_Templates		dc.l	.NullTemplate,			.InitXSpeedAnimT,		.InitLinearAccelTemplate
			dc.l	.InitAsteroidsPlayerT,		.InitInertialPlayerT,		.SpawnPlayerBulletT
			dc.l	.ExecuteTemplate,		.InitCycAnimTemplate,		.InitDir16AnimTemplate
			dc.l	.BGCollTemplate,		.InertJoyTemplate,		.KillObjectTemplate
			dc.l	.ScrollModeConstantXSpeedT,	.InitDirectionalHomerT,		.InitPlatformPlayerTemplate
			dc.l	.StatPrintVarT,			.InitPlatformPlayerT,		.SETIMAGET
			dc.l	.InitXPushSliceTrackT,		.InitYPushSliceTrackT,		.InitScrollT
			dc.l	.GetHurtT,			.FireWeaponBayT,		.PowerUpWeaponBayT
			dc.l	.PowerDownWeaponBayT,		.SetWeaponBayT,			.SetChannelT
			dc.l	.HighlightBobT,			.SetColourT,			.KillChannelT
			dc.l	.InitMousePlayerT,		.InitActionListPlayerT,		.InitCheckFireButtonsT
			dc.l	.InitYSpeedAnimT,		.SimpleSFXT,			.PlaySFXT
			dc.l	.NullTemplate,			.NullTemplate,			.InitInertialHomerT
			dc.l	.StatPrintScoreT,		.InitHomerBadDude1T,		.InitTimeDelayExecuteT
			dc.l	.InitFollowPathT,		.OLDDirectionMoveT,		.InitChannelExecuteT
			dc.l	.InitDirectionMoveT,		.OldInitDir8AnimT,		.OldInitDir16AnimT
			dc.l	.OldInitDir32AnimT,		.OldInitDir64AnimT,		.OldInitDir8ImageT
			dc.l	.OldInitDir16ImageT,		.OldInitDir32ImageT,		.OldInitDir64ImageT
			dc.l	.OldInitActionListPlayerT,	.SpawnPlayerBulletT,		.SpawnBadDudeT
			dc.l	.SpawnPlayerT,			.InitDir8ActionT,		.InitDir16ActionT
			dc.l	.InitDir32ActionT,		.InitDir64ActionT,		.OldStatPrintScoreT
			dc.l	.StatPrintGlobalScoreT,		.GiveScoreT,			.EndLevelT
			dc.l	.Dir8ExecuteT,			.Dir16ExecuteT,			.Dir32ExecuteT
			dc.l	.Dir64ExecuteT,			.InitNULLT,			.RNDXSpeedT
			dc.l	.RNDYSpeedT,			.InitSteroidsT,			.SetBorderT
			dc.l	.RNDDirectionMoveT,		.SetVarT,			.AddToVarT
			dc.l	.SubFromVarT,			.ExecuteIfVarCCT,		.OldDir8ImageT
			dc.l	.OldDir16ImageT,		.Dir32ImageT,			.Dir64ImageT
			dc.l	.RotateClockWiseT,		.RotateAntiClockWiseT,		.SetXSpeedsFromList32T
			dc.l	.SetYSpeedsFromList32T,		.SetDirectionT,			.InitAttatchT
			dc.l	.SetMiscStuffT,			.InitOperationWolfMovementT,	.RepelT
			dc.l	.SetIT,				.AddIT,				.SetVT
			dc.l	.AddVT,				.OLDSetRNDT,			.OLDAddRNDT
			dc.l	.NullTemplate,			.CollisionPartnerExecuteT,	.CollisionPartnerOffT
			dc.l	.KillCollisionPartnerT,		.InitJoystickAnimT,		.SetObjectActionT
			dc.l	.SetCollisionsT,		.PlayModuleT,			.StopModuleT
			dc.l	.InitActionOnFireT,		.CompareVariableT,		.CompareImediateT
			dc.l	.IfEqualT,			.IfNotEqualT,			.IfGreaterThanT
			dc.l	.IfLowerThanT,			.OLDInitBackgroundPointCollisionT,	.SimplePlatformPlayerT
			dc.l	.ExecuteOnPlayerT,		.ExecuteIfOnScreenT,		.AttachToParentT
			dc.l	.InitDrawBarT,			.InitFloorSentryT,		.InitCeilingSentryT
			dc.l	.InitLinearPlayerT,		.InitBGPointCollisionT,		.InitBGCollisionT
			dc.l	.InitTrackPlayerT,		.DirectionMoveT,		.InitJitterSpeedT
			dc.l	.SetRNDT,			.AddRNDT,			.ScrollXCenterPlayersT
			dc.l	.InitDir8ImageT,		.InitDir16ImageT,		.InitDir32ImageT
			dc.l	.InitDir64ImageT,		.Dir8ImageT,			.Dir16ImageT
			dc.l	.InitDir8AnimT,			.InitDir16AnimT,		.InitDir32AnimT
			dc.l	.InitDir64AnimT,		.RestrictDirectionT,		.RandomEventT
			dc.l	.InitRandomEventT,		.InitMousePointerT,		.ScrollYCenterPlayersT


.NullTemplate		dc.b	TEMARG_END
.InitXSpeedAnimT	dc.b	TEMARG_LONG,TEMARG_ANIMREF,TEMARG_LONG,TEMARG_ANIMREF,TEMARG_ANIMREF,TEMARG_END
.InitLinearAccelTemplate dc.b	TEMARG_LONG,TEMARG_LONG,TEMARG_LONG,TEMARG_LONG,TEMARG_WORD,TEMARG_END
.InitAsteroidsPlayerT	dc.b	TEMARG_WORD,TEMARG_LONG,TEMARG_LONG,TEMARG_LONG,TEMARG_LONG,TEMARG_WORD,TEMARG_PROGREF,TEMARG_WORD,TEMARG_END
.InitInertialPlayerT	dc.b	TEMARG_WORD,TEMARG_LONG,TEMARG_LONG,TEMARG_LONG,TEMARG_LONG
			dc.b	TEMARG_LONG,TEMARG_LONG,TEMARG_PROGREF,TEMARG_WORD
			dc.b	TEMARG_LONG,TEMARG_LONG,TEMARG_WORD,TEMARG_WORD,TEMARG_END
.ExecuteTemplate	dc.b	TEMARG_PROGREF,TEMARG_END
.InitCycAnimTemplate	dc.b	TEMARG_ANIMREF,TEMARG_BYTE,TEMARG_BYTE,TEMARG_WORD,TEMARG_END
.InitDir16AnimTemplate	dc.b	TEMARG_ANIMREF,TEMARG_WORD,TEMARG_WORD,TEMARG_ANIMREF,TEMARG_WORD,TEMARG_WORD,TEMARG_ANIMREF,TEMARG_WORD,TEMARG_WORD,TEMARG_ANIMREF,TEMARG_WORD,TEMARG_WORD,TEMARG_ANIMREF,TEMARG_WORD,TEMARG_WORD
			dc.b	TEMARG_ANIMREF,TEMARG_WORD,TEMARG_WORD,TEMARG_ANIMREF,TEMARG_WORD,TEMARG_WORD,TEMARG_ANIMREF,TEMARG_WORD,TEMARG_WORD,TEMARG_ANIMREF,TEMARG_WORD,TEMARG_WORD,TEMARG_ANIMREF,TEMARG_WORD,TEMARG_WORD
			dc.b	TEMARG_ANIMREF,TEMARG_WORD,TEMARG_WORD,TEMARG_ANIMREF,TEMARG_WORD,TEMARG_WORD,TEMARG_ANIMREF,TEMARG_WORD,TEMARG_WORD,TEMARG_ANIMREF,TEMARG_WORD,TEMARG_WORD,TEMARG_ANIMREF,TEMARG_WORD,TEMARG_WORD
			dc.b	TEMARG_ANIMREF,TEMARG_WORD,TEMARG_WORD,TEMARG_ANIMREF,TEMARG_WORD,TEMARG_WORD,TEMARG_END
.BGCollTemplate		dc.b	TEMARG_WORD,TEMARG_PROGREF,TEMARG_WORD,TEMARG_END

.InertJoyTemplate	dc.b	TEMARG_LONG,TEMARG_LONG,TEMARG_LONG,TEMARG_LONG,TEMARG_LONG,TEMARG_LONG
			dc.b	TEMARG_WORD,TEMARG_WORD,TEMARG_WORD,TEMARG_PROGREF,TEMARG_PROGREF,TEMARG_PROGREF,TEMARG_PROGREF,TEMARG_END

.KillObjectTemplate	dc.b	TEMARG_END
.ScrollModeConstantXSpeedT	dc.b	TEMARG_LONG,TEMARG_LONG,TEMARG_LONG,TEMARG_LONG,TEMARG_PROGREF,TEMARG_WORD,TEMARG_END
.InitDirectionalHomerT	dc.b	TEMARG_WORD,TEMARG_PLYRREF,TEMARG_WORD,TEMARG_PROGREF,TEMARG_END
;the old one
.InitPlatformPlayerTemplate
			dc.b	TEMARG_WORD,TEMARG_ANIMREF,TEMARG_ANIMREF,TEMARG_ANIMREF
			dc.b	TEMARG_ANIMREF,TEMARG_ANIMREF,TEMARG_ANIMREF,TEMARG_ANIMREF
			dc.b	TEMARG_ANIMREF,TEMARG_LONG,TEMARG_LONG,TEMARG_LONG
			dc.b	TEMARG_LONG,TEMARG_LONG,TEMARG_LONG,TEMARG_WORD
			dc.b	TEMARG_WORD,TEMARG_END
.StatPrintVarT		dc.b	TEMARG_LONG,TEMARG_LONG,TEMARG_WORD,TEMARG_END
.InitPlatformPlayerT	dc.b	TEMARG_WORD,TEMARG_LONG,TEMARG_LONG,TEMARG_LONG,TEMARG_LONG
			dc.b	TEMARG_LONG,TEMARG_LONG,TEMARG_LONG,TEMARG_EOTRREF,TEMARG_WORD
			dc.b	TEMARG_PROGREF,TEMARG_LONG,TEMARG_LONG,TEMARG_LONG,TEMARG_LONG
			dc.b	TEMARG_ANIMREF,TEMARG_ANIMREF,TEMARG_ANIMREF,TEMARG_ANIMREF
			dc.b	TEMARG_ANIMREF,TEMARG_ANIMREF,TEMARG_ANIMREF,TEMARG_ANIMREF,TEMARG_END
.SETIMAGET		dc.b	TEMARG_IMAGEREF,TEMARG_END
.InitXPushSliceTrackT	dc.b	TEMARG_PLYRREF,TEMARG_LONG,TEMARG_LONG,TEMARG_LONG,TEMARG_END
.InitYPushSliceTrackT	dc.b	TEMARG_PLYRREF,TEMARG_LONG,TEMARG_LONG,TEMARG_LONG,TEMARG_END
.InitScrollT		dc.b	TEMARG_LONG,TEMARG_LONG,TEMARG_LONG,TEMARG_LONG,TEMARG_END
.GetHurtT		dc.b	TEMARG_END
.FireWeaponBayT		dc.b	TEMARG_WORD,TEMARG_END
.PowerUpWeaponBayT	dc.b	TEMARG_WORD,TEMARG_WEAPREF,TEMARG_END
.PowerDownWeaponBayT	dc.b	TEMARG_WORD,TEMARG_WORD,TEMARG_END
.SetWeaponBayT		dc.b	TEMARG_WORD,TEMARG_WEAPREF,TEMARG_END
.SetChannelT		dc.b	TEMARG_WORD,TEMARG_OPERATOR,TEMARG_WORD,TEMARG_PROGREF,TEMARG_END
.HighlightBobT		dc.b	TEMARG_WORD,TEMARG_END
.SetColourT		dc.b	TEMARG_BYTE,TEMARG_BYTE,TEMARG_BYTE,TEMARG_BYTE,TEMARG_END
.KillChannelT		dc.b	TEMARG_WORD,TEMARG_END
.InitMousePlayerT	dc.b	TEMARG_WORD,TEMARG_WORD,TEMARG_WORD,TEMARG_LONG,TEMARG_LONG,TEMARG_LONG,TEMARG_LONG,TEMARG_PROGREF
			dc.b	TEMARG_WORD,TEMARG_WORD,TEMARG_LONG,TEMARG_LONG,TEMARG_END
.InitActionListPlayerT	dc.b	TEMARG_WORD,TEMARG_WORD
			dc.b	TEMARG_PROGREF,TEMARG_PROGREF,TEMARG_PROGREF,TEMARG_PROGREF
			dc.b	TEMARG_PROGREF,TEMARG_PROGREF,TEMARG_PROGREF,TEMARG_PROGREF
			dc.b	TEMARG_PROGREF,TEMARG_PROGREF,TEMARG_PROGREF,TEMARG_PROGREF
			dc.b	TEMARG_PROGREF,TEMARG_PROGREF,TEMARG_PROGREF,TEMARG_PROGREF
			dc.b	TEMARG_WORD,TEMARG_END
.InitCheckFireButtonsT	dc.b	TEMARG_WORD,TEMARG_WORD
			dc.b	TEMARG_PROGREF,TEMARG_PROGREF,TEMARG_PROGREF,TEMARG_PROGREF
			dc.b	TEMARG_PROGREF,TEMARG_PROGREF,TEMARG_PROGREF,TEMARG_PROGREF
			dc.b	TEMARG_WORD,TEMARG_END
.InitYSpeedAnimT	dc.b	TEMARG_LONG,TEMARG_ANIMREF,TEMARG_LONG,TEMARG_ANIMREF,TEMARG_ANIMREF,TEMARG_END
.SimpleSFXT		dc.b	TEMARG_SFXREF,TEMARG_WORD,TEMARG_END
.PlaySFXT		dc.b	TEMARG_SFXREF,TEMARG_BYTE,TEMARG_WORD,TEMARG_WORD,TEMARG_END
.InitInertialHomerT	dc.b	TEMARG_WORD,TEMARG_WORD,TEMARG_WORD,TEMARG_LONG,TEMARG_LONG,TEMARG_LONG,TEMARG_LONG,TEMARG_END
.StatPrintScoreT	dc.b	TEMARG_LONG,TEMARG_LONG,TEMARG_WORD,TEMARG_END
.InitHomerBadDude1T	dc.b	TEMARG_WORD,TEMARG_PLYRREF,TEMARG_LONG,TEMARG_LONG,TEMARG_LONG,TEMARG_LONG,TEMARG_END
.InitTimeDelayExecuteT	dc.b	TEMARG_PROGREF,TEMARG_WORD,TEMARG_WORD,TEMARG_END
.InitFollowPathT	dc.b	TEMARG_PATHREF,TEMARG_WORD,TEMARG_END
.OLDDirectionMoveT		dc.b	TEMARG_LONG,TEMARG_END
.InitChannelExecuteT	dc.b	TEMARG_PROGREF,TEMARG_WORD,TEMARG_END
.InitDirectionMoveT	dc.b	TEMARG_LONG,TEMARG_WORD,TEMARG_END

.OldInitDir8AnimT		dc.b	TEMARG_ANIMREF,TEMARG_WORD,TEMARG_ANIMREF,TEMARG_WORD,TEMARG_ANIMREF,TEMARG_WORD,TEMARG_ANIMREF,TEMARG_WORD
			dc.b	TEMARG_ANIMREF,TEMARG_WORD,TEMARG_ANIMREF,TEMARG_WORD,TEMARG_ANIMREF,TEMARG_WORD,TEMARG_ANIMREF,TEMARG_WORD
			dc.b	TEMARG_END
.OldInitDir16AnimT		dc.b	TEMARG_ANIMREF,TEMARG_WORD,TEMARG_ANIMREF,TEMARG_WORD,TEMARG_ANIMREF,TEMARG_WORD,TEMARG_ANIMREF,TEMARG_WORD
			dc.b	TEMARG_ANIMREF,TEMARG_WORD,TEMARG_ANIMREF,TEMARG_WORD,TEMARG_ANIMREF,TEMARG_WORD,TEMARG_ANIMREF,TEMARG_WORD
			dc.b	TEMARG_ANIMREF,TEMARG_WORD,TEMARG_ANIMREF,TEMARG_WORD,TEMARG_ANIMREF,TEMARG_WORD,TEMARG_ANIMREF,TEMARG_WORD
			dc.b	TEMARG_ANIMREF,TEMARG_WORD,TEMARG_ANIMREF,TEMARG_WORD,TEMARG_ANIMREF,TEMARG_WORD,TEMARG_ANIMREF,TEMARG_WORD
			dc.b	TEMARG_END
.OldInitDir32AnimT		dc.b	TEMARG_ANIMREF,TEMARG_WORD,TEMARG_ANIMREF,TEMARG_WORD,TEMARG_ANIMREF,TEMARG_WORD,TEMARG_ANIMREF,TEMARG_WORD
			dc.b	TEMARG_ANIMREF,TEMARG_WORD,TEMARG_ANIMREF,TEMARG_WORD,TEMARG_ANIMREF,TEMARG_WORD,TEMARG_ANIMREF,TEMARG_WORD
			dc.b	TEMARG_ANIMREF,TEMARG_WORD,TEMARG_ANIMREF,TEMARG_WORD,TEMARG_ANIMREF,TEMARG_WORD,TEMARG_ANIMREF,TEMARG_WORD
			dc.b	TEMARG_ANIMREF,TEMARG_WORD,TEMARG_ANIMREF,TEMARG_WORD,TEMARG_ANIMREF,TEMARG_WORD,TEMARG_ANIMREF,TEMARG_WORD
			dc.b	TEMARG_ANIMREF,TEMARG_WORD,TEMARG_ANIMREF,TEMARG_WORD,TEMARG_ANIMREF,TEMARG_WORD,TEMARG_ANIMREF,TEMARG_WORD
			dc.b	TEMARG_ANIMREF,TEMARG_WORD,TEMARG_ANIMREF,TEMARG_WORD,TEMARG_ANIMREF,TEMARG_WORD,TEMARG_ANIMREF,TEMARG_WORD
			dc.b	TEMARG_ANIMREF,TEMARG_WORD,TEMARG_ANIMREF,TEMARG_WORD,TEMARG_ANIMREF,TEMARG_WORD,TEMARG_ANIMREF,TEMARG_WORD
			dc.b	TEMARG_ANIMREF,TEMARG_WORD,TEMARG_ANIMREF,TEMARG_WORD,TEMARG_ANIMREF,TEMARG_WORD,TEMARG_ANIMREF,TEMARG_WORD
			dc.b	TEMARG_END
.OldInitDir64AnimT		dc.b	TEMARG_ANIMREF,TEMARG_WORD,TEMARG_ANIMREF,TEMARG_WORD,TEMARG_ANIMREF,TEMARG_WORD,TEMARG_ANIMREF,TEMARG_WORD
			dc.b	TEMARG_ANIMREF,TEMARG_WORD,TEMARG_ANIMREF,TEMARG_WORD,TEMARG_ANIMREF,TEMARG_WORD,TEMARG_ANIMREF,TEMARG_WORD
			dc.b	TEMARG_ANIMREF,TEMARG_WORD,TEMARG_ANIMREF,TEMARG_WORD,TEMARG_ANIMREF,TEMARG_WORD,TEMARG_ANIMREF,TEMARG_WORD
			dc.b	TEMARG_ANIMREF,TEMARG_WORD,TEMARG_ANIMREF,TEMARG_WORD,TEMARG_ANIMREF,TEMARG_WORD,TEMARG_ANIMREF,TEMARG_WORD
			dc.b	TEMARG_ANIMREF,TEMARG_WORD,TEMARG_ANIMREF,TEMARG_WORD,TEMARG_ANIMREF,TEMARG_WORD,TEMARG_ANIMREF,TEMARG_WORD
			dc.b	TEMARG_ANIMREF,TEMARG_WORD,TEMARG_ANIMREF,TEMARG_WORD,TEMARG_ANIMREF,TEMARG_WORD,TEMARG_ANIMREF,TEMARG_WORD
			dc.b	TEMARG_ANIMREF,TEMARG_WORD,TEMARG_ANIMREF,TEMARG_WORD,TEMARG_ANIMREF,TEMARG_WORD,TEMARG_ANIMREF,TEMARG_WORD
			dc.b	TEMARG_ANIMREF,TEMARG_WORD,TEMARG_ANIMREF,TEMARG_WORD,TEMARG_ANIMREF,TEMARG_WORD,TEMARG_ANIMREF,TEMARG_WORD
			dc.b	TEMARG_ANIMREF,TEMARG_WORD,TEMARG_ANIMREF,TEMARG_WORD,TEMARG_ANIMREF,TEMARG_WORD,TEMARG_ANIMREF,TEMARG_WORD
			dc.b	TEMARG_ANIMREF,TEMARG_WORD,TEMARG_ANIMREF,TEMARG_WORD,TEMARG_ANIMREF,TEMARG_WORD,TEMARG_ANIMREF,TEMARG_WORD
			dc.b	TEMARG_ANIMREF,TEMARG_WORD,TEMARG_ANIMREF,TEMARG_WORD,TEMARG_ANIMREF,TEMARG_WORD,TEMARG_ANIMREF,TEMARG_WORD
			dc.b	TEMARG_ANIMREF,TEMARG_WORD,TEMARG_ANIMREF,TEMARG_WORD,TEMARG_ANIMREF,TEMARG_WORD,TEMARG_ANIMREF,TEMARG_WORD
			dc.b	TEMARG_ANIMREF,TEMARG_WORD,TEMARG_ANIMREF,TEMARG_WORD,TEMARG_ANIMREF,TEMARG_WORD,TEMARG_ANIMREF,TEMARG_WORD
			dc.b	TEMARG_ANIMREF,TEMARG_WORD,TEMARG_ANIMREF,TEMARG_WORD,TEMARG_ANIMREF,TEMARG_WORD,TEMARG_ANIMREF,TEMARG_WORD
			dc.b	TEMARG_ANIMREF,TEMARG_WORD,TEMARG_ANIMREF,TEMARG_WORD,TEMARG_ANIMREF,TEMARG_WORD,TEMARG_ANIMREF,TEMARG_WORD
			dc.b	TEMARG_ANIMREF,TEMARG_WORD,TEMARG_ANIMREF,TEMARG_WORD,TEMARG_ANIMREF,TEMARG_WORD,TEMARG_ANIMREF,TEMARG_WORD
			dc.b	TEMARG_END
.OldInitDir8ImageT		dc.b	TEMARG_ANIMREF,TEMARG_END
.OldInitDir16ImageT	dc.b	TEMARG_ANIMREF,TEMARG_END
.OldInitDir32ImageT	dc.b	TEMARG_ANIMREF,TEMARG_END
.OldInitDir64ImageT	dc.b	TEMARG_ANIMREF,TEMARG_END
.OldInitActionListPlayerT	dc.b	TEMARG_WORD,TEMARG_PROGREF,TEMARG_PROGREF,TEMARG_PROGREF,TEMARG_PROGREF,TEMARG_LONG,TEMARG_END
.SpawnPlayerBulletT	dc.b	TEMARG_IMAGEREF,TEMARG_LONG,TEMARG_LONG,TEMARG_PROGREF,TEMARG_END
.SpawnBadDudeT		dc.b	TEMARG_IMAGEREF,TEMARG_LONG,TEMARG_LONG,TEMARG_PROGREF,TEMARG_END
.SpawnPlayerT		dc.b	TEMARG_IMAGEREF,TEMARG_LONG,TEMARG_LONG,TEMARG_PROGREF,TEMARG_END
.InitDir8ActionT	dc.b	TEMARG_PROGREF,TEMARG_PROGREF,TEMARG_PROGREF,TEMARG_PROGREF,TEMARG_PROGREF,TEMARG_PROGREF,TEMARG_PROGREF,TEMARG_PROGREF,TEMARG_END
.InitDir16ActionT	dc.b	TEMARG_PROGREF,TEMARG_PROGREF,TEMARG_PROGREF,TEMARG_PROGREF,TEMARG_PROGREF,TEMARG_PROGREF,TEMARG_PROGREF,TEMARG_PROGREF
			dc.b	TEMARG_PROGREF,TEMARG_PROGREF,TEMARG_PROGREF,TEMARG_PROGREF,TEMARG_PROGREF,TEMARG_PROGREF,TEMARG_PROGREF,TEMARG_PROGREF,TEMARG_END
.InitDir32ActionT	dc.b	TEMARG_PROGREF,TEMARG_PROGREF,TEMARG_PROGREF,TEMARG_PROGREF,TEMARG_PROGREF,TEMARG_PROGREF,TEMARG_PROGREF,TEMARG_PROGREF
			dc.b	TEMARG_PROGREF,TEMARG_PROGREF,TEMARG_PROGREF,TEMARG_PROGREF,TEMARG_PROGREF,TEMARG_PROGREF,TEMARG_PROGREF,TEMARG_PROGREF
			dc.b	TEMARG_PROGREF,TEMARG_PROGREF,TEMARG_PROGREF,TEMARG_PROGREF,TEMARG_PROGREF,TEMARG_PROGREF,TEMARG_PROGREF,TEMARG_PROGREF
			dc.b	TEMARG_PROGREF,TEMARG_PROGREF,TEMARG_PROGREF,TEMARG_PROGREF,TEMARG_PROGREF,TEMARG_PROGREF,TEMARG_PROGREF,TEMARG_PROGREF,TEMARG_END
.InitDir64ActionT	dc.b	TEMARG_PROGREF,TEMARG_PROGREF,TEMARG_PROGREF,TEMARG_PROGREF,TEMARG_PROGREF,TEMARG_PROGREF,TEMARG_PROGREF,TEMARG_PROGREF
			dc.b	TEMARG_PROGREF,TEMARG_PROGREF,TEMARG_PROGREF,TEMARG_PROGREF,TEMARG_PROGREF,TEMARG_PROGREF,TEMARG_PROGREF,TEMARG_PROGREF
			dc.b	TEMARG_PROGREF,TEMARG_PROGREF,TEMARG_PROGREF,TEMARG_PROGREF,TEMARG_PROGREF,TEMARG_PROGREF,TEMARG_PROGREF,TEMARG_PROGREF
			dc.b	TEMARG_PROGREF,TEMARG_PROGREF,TEMARG_PROGREF,TEMARG_PROGREF,TEMARG_PROGREF,TEMARG_PROGREF,TEMARG_PROGREF,TEMARG_PROGREF
			dc.b	TEMARG_PROGREF,TEMARG_PROGREF,TEMARG_PROGREF,TEMARG_PROGREF,TEMARG_PROGREF,TEMARG_PROGREF,TEMARG_PROGREF,TEMARG_PROGREF
			dc.b	TEMARG_PROGREF,TEMARG_PROGREF,TEMARG_PROGREF,TEMARG_PROGREF,TEMARG_PROGREF,TEMARG_PROGREF,TEMARG_PROGREF,TEMARG_PROGREF
			dc.b	TEMARG_PROGREF,TEMARG_PROGREF,TEMARG_PROGREF,TEMARG_PROGREF,TEMARG_PROGREF,TEMARG_PROGREF,TEMARG_PROGREF,TEMARG_PROGREF
			dc.b	TEMARG_PROGREF,TEMARG_PROGREF,TEMARG_PROGREF,TEMARG_PROGREF,TEMARG_PROGREF,TEMARG_PROGREF,TEMARG_PROGREF,TEMARG_PROGREF,TEMARG_END
.OldStatPrintScoreT	dc.b	TEMARG_WORD,TEMARG_WORD,TEMARG_PLYRREF,TEMARG_END
.StatPrintGlobalScoreT	dc.b	TEMARG_WORD,TEMARG_WORD,TEMARG_WORD,TEMARG_END
.GiveScoreT		dc.b	TEMARG_LONG,TEMARG_END
.EndLevelT		dc.b	TEMARG_LEVELREF,TEMARG_END

.Dir8ExecuteT		dc.b	TEMARG_PROGREF,TEMARG_PROGREF,TEMARG_PROGREF,TEMARG_PROGREF,TEMARG_PROGREF,TEMARG_PROGREF,TEMARG_PROGREF,TEMARG_PROGREF,TEMARG_END
.Dir16ExecuteT		dc.b	TEMARG_PROGREF,TEMARG_PROGREF,TEMARG_PROGREF,TEMARG_PROGREF,TEMARG_PROGREF,TEMARG_PROGREF,TEMARG_PROGREF,TEMARG_PROGREF
			dc.b	TEMARG_PROGREF,TEMARG_PROGREF,TEMARG_PROGREF,TEMARG_PROGREF,TEMARG_PROGREF,TEMARG_PROGREF,TEMARG_PROGREF,TEMARG_PROGREF,TEMARG_END
.Dir32ExecuteT		dc.b	TEMARG_PROGREF,TEMARG_PROGREF,TEMARG_PROGREF,TEMARG_PROGREF,TEMARG_PROGREF,TEMARG_PROGREF,TEMARG_PROGREF,TEMARG_PROGREF
			dc.b	TEMARG_PROGREF,TEMARG_PROGREF,TEMARG_PROGREF,TEMARG_PROGREF,TEMARG_PROGREF,TEMARG_PROGREF,TEMARG_PROGREF,TEMARG_PROGREF
			dc.b	TEMARG_PROGREF,TEMARG_PROGREF,TEMARG_PROGREF,TEMARG_PROGREF,TEMARG_PROGREF,TEMARG_PROGREF,TEMARG_PROGREF,TEMARG_PROGREF
			dc.b	TEMARG_PROGREF,TEMARG_PROGREF,TEMARG_PROGREF,TEMARG_PROGREF,TEMARG_PROGREF,TEMARG_PROGREF,TEMARG_PROGREF,TEMARG_PROGREF,TEMARG_END
.Dir64ExecuteT		dc.b	TEMARG_PROGREF,TEMARG_PROGREF,TEMARG_PROGREF,TEMARG_PROGREF,TEMARG_PROGREF,TEMARG_PROGREF,TEMARG_PROGREF,TEMARG_PROGREF
			dc.b	TEMARG_PROGREF,TEMARG_PROGREF,TEMARG_PROGREF,TEMARG_PROGREF,TEMARG_PROGREF,TEMARG_PROGREF,TEMARG_PROGREF,TEMARG_PROGREF
			dc.b	TEMARG_PROGREF,TEMARG_PROGREF,TEMARG_PROGREF,TEMARG_PROGREF,TEMARG_PROGREF,TEMARG_PROGREF,TEMARG_PROGREF,TEMARG_PROGREF
			dc.b	TEMARG_PROGREF,TEMARG_PROGREF,TEMARG_PROGREF,TEMARG_PROGREF,TEMARG_PROGREF,TEMARG_PROGREF,TEMARG_PROGREF,TEMARG_PROGREF
			dc.b	TEMARG_PROGREF,TEMARG_PROGREF,TEMARG_PROGREF,TEMARG_PROGREF,TEMARG_PROGREF,TEMARG_PROGREF,TEMARG_PROGREF,TEMARG_PROGREF
			dc.b	TEMARG_PROGREF,TEMARG_PROGREF,TEMARG_PROGREF,TEMARG_PROGREF,TEMARG_PROGREF,TEMARG_PROGREF,TEMARG_PROGREF,TEMARG_PROGREF
			dc.b	TEMARG_PROGREF,TEMARG_PROGREF,TEMARG_PROGREF,TEMARG_PROGREF,TEMARG_PROGREF,TEMARG_PROGREF,TEMARG_PROGREF,TEMARG_PROGREF
			dc.b	TEMARG_PROGREF,TEMARG_PROGREF,TEMARG_PROGREF,TEMARG_PROGREF,TEMARG_PROGREF,TEMARG_PROGREF,TEMARG_PROGREF,TEMARG_PROGREF,TEMARG_END
.InitNULLT		dc.b	TEMARG_END
.RNDXSpeedT		dc.b	TEMARG_WORD,TEMARG_END
.RNDYSpeedT		dc.b	TEMARG_WORD,TEMARG_END
.InitSteroidsT		dc.b	TEMARG_WORD,TEMARG_WORD,TEMARG_LONG,TEMARG_LONG,TEMARG_LONG,TEMARG_PROGREF,TEMARG_END
.SetBorderT		dc.b	TEMARG_LONG,TEMARG_WORD,TEMARG_LONG,TEMARG_WORD,TEMARG_LONG,TEMARG_WORD,TEMARG_LONG,TEMARG_WORD,TEMARG_WORD,TEMARG_END
.RNDDirectionMoveT	dc.b	TEMARG_LONG,TEMARG_END
.SetVarT		dc.b	TEMARG_WORD,TEMARG_LONG,TEMARG_END
.AddToVarT		dc.b	TEMARG_WORD,TEMARG_LONG,TEMARG_END
.SubFromVarT		dc.b	TEMARG_WORD,TEMARG_LONG,TEMARG_END
.ExecuteIfVarCCT	dc.b	TEMARG_WORD,TEMARG_LONG,TEMARG_WORD,TEMARG_PROGREF,TEMARG_END
.OldDir8ImageT		dc.b	TEMARG_ANIMREF,TEMARG_WORD,TEMARG_END
.OldDir16ImageT		dc.b	TEMARG_ANIMREF,TEMARG_WORD,TEMARG_END
.Dir32ImageT		dc.b	TEMARG_ANIMREF,TEMARG_WORD,TEMARG_END
.Dir64ImageT		dc.b	TEMARG_ANIMREF,TEMARG_WORD,TEMARG_END
.RotateClockWiseT	dc.b	TEMARG_WORD,TEMARG_END
.RotateAntiClockWiseT	dc.b	TEMARG_WORD,TEMARG_END
.SetXSpeedsFromList32T	dc.b	TEMARG_LONG,TEMARG_LONG,TEMARG_LONG,TEMARG_LONG,TEMARG_LONG,TEMARG_LONG,TEMARG_LONG,TEMARG_LONG,TEMARG_LONG,TEMARG_LONG
			dc.b	TEMARG_LONG,TEMARG_LONG,TEMARG_LONG,TEMARG_LONG,TEMARG_LONG,TEMARG_LONG,TEMARG_LONG,TEMARG_LONG,TEMARG_LONG,TEMARG_LONG
			dc.b	TEMARG_LONG,TEMARG_LONG,TEMARG_LONG,TEMARG_LONG,TEMARG_LONG,TEMARG_LONG,TEMARG_LONG,TEMARG_LONG,TEMARG_LONG,TEMARG_LONG
			dc.b	TEMARG_LONG,TEMARG_LONG,TEMARG_END
.SetYSpeedsFromList32T	dc.b	TEMARG_LONG,TEMARG_LONG,TEMARG_LONG,TEMARG_LONG,TEMARG_LONG,TEMARG_LONG,TEMARG_LONG,TEMARG_LONG,TEMARG_LONG,TEMARG_LONG
			dc.b	TEMARG_LONG,TEMARG_LONG,TEMARG_LONG,TEMARG_LONG,TEMARG_LONG,TEMARG_LONG,TEMARG_LONG,TEMARG_LONG,TEMARG_LONG,TEMARG_LONG
			dc.b	TEMARG_LONG,TEMARG_LONG,TEMARG_LONG,TEMARG_LONG,TEMARG_LONG,TEMARG_LONG,TEMARG_LONG,TEMARG_LONG,TEMARG_LONG,TEMARG_LONG
			dc.b	TEMARG_LONG,TEMARG_LONG,TEMARG_END
.SetDirectionT		dc.b	TEMARG_WORD,TEMARG_END
.InitAttatchT		dc.b	TEMARG_PLYRREF,TEMARG_LONG,TEMARG_LONG,TEMARG_END
.SetMiscStuffT		dc.b	TEMARG_PROGREF,TEMARG_PROGREF,TEMARG_PROGREF
			dc.b	TEMARG_WORD,TEMARG_WORD,TEMARG_PROGREF,TEMARG_WORD,TEMARG_END
.InitOperationWolfMovementT
			dc.b	TEMARG_LONG,TEMARG_LONG,TEMARG_WORD,TEMARG_WORD,TEMARG_WORD,TEMARG_END
.RepelT			dc.b	TEMARG_WORD,TEMARG_WORD,TEMARG_END
.SetIT			dc.b	TEMARG_LONG,TEMARG_WORD,TEMARG_END
.AddIT			dc.b	TEMARG_LONG,TEMARG_WORD,TEMARG_END
.SetVT			dc.b	TEMARG_WORD,TEMARG_WORD,TEMARG_END
.AddVT			dc.b	TEMARG_WORD,TEMARG_WORD,TEMARG_END
.OLDSetRNDT		dc.b	TEMARG_WORD,TEMARG_WORD,TEMARG_END
.OLDAddRNDT		dc.b	TEMARG_WORD,TEMARG_WORD,TEMARG_END
.CollisionPartnerExecuteT
			dc.b	TEMARG_PROGREF,TEMARG_END
.CollisionPartnerOffT	dc.b	TEMARG_END
.KillCollisionPartnerT	dc.b	TEMARG_END
.InitJoystickAnimT	dc.b	TEMARG_WORD,TEMARG_WORD
			dc.b	TEMARG_ANIMREF,TEMARG_ANIMREF,TEMARG_ANIMREF,TEMARG_ANIMREF,TEMARG_ANIMREF,TEMARG_ANIMREF
			dc.b	TEMARG_ANIMREF,TEMARG_ANIMREF,TEMARG_ANIMREF,TEMARG_ANIMREF,TEMARG_ANIMREF,TEMARG_ANIMREF
			dc.b	TEMARG_ANIMREF,TEMARG_ANIMREF,TEMARG_ANIMREF,TEMARG_ANIMREF,TEMARG_ANIMREF,TEMARG_ANIMREF,TEMARG_END
.SetObjectActionT	dc.b	TEMARG_PROGREF,TEMARG_WORD,TEMARG_END
.SetCollisionsT		dc.b	TEMARG_WORD,TEMARG_END
.PlayModuleT		dc.b	TEMARG_RAWFILEREF,TEMARG_END
.StopModuleT		dc.b	TEMARG_END
.InitActionOnFireT	dc.b	TEMARG_WORD,TEMARG_WORD,TEMARG_PROGREF,TEMARG_END
.CompareVariableT	dc.b	TEMARG_WORD,TEMARG_WORD,TEMARG_END
.CompareImediateT	dc.b	TEMARG_LONG,TEMARG_WORD,TEMARG_END
.IfEqualT		dc.b	TEMARG_PROGREF,TEMARG_END
.IfNotEqualT		dc.b	TEMARG_PROGREF,TEMARG_END
.IfGreaterThanT		dc.b	TEMARG_PROGREF,TEMARG_END
.IfLowerThanT		dc.b	TEMARG_PROGREF,TEMARG_END
.OLDInitBackgroundPointCollisionT
			 dc.b	TEMARG_WORD,TEMARG_PROGREF,TEMARG_WORD,TEMARG_LONG,TEMARG_LONG,TEMARG_END
.SimplePlatformPlayerT	dc.b	TEMARG_WORD,TEMARG_LONG,TEMARG_LONG,TEMARG_LONG,TEMARG_LONG,TEMARG_LONG,TEMARG_WORD,TEMARG_WORD,TEMARG_WORD,TEMARG_END
.ExecuteOnPlayerT	dc.b	TEMARG_PLYRREF,TEMARG_PROGREF,TEMARG_END
.ExecuteIfOnScreenT	dc.b	TEMARG_PROGREF,TEMARG_END
.AttachToParentT	dc.b	TEMARG_LONG,TEMARG_LONG,TEMARG_END
.InitDrawBarT		dc.b	TEMARG_WORD,TEMARG_WORD,TEMARG_WORD,TEMARG_LONG,TEMARG_WORD,TEMARG_END
.InitFloorSentryT	dc.b	TEMARG_LONG,TEMARG_LONG,TEMARG_EOTRREF,TEMARG_BYTE,TEMARG_BYTE,TEMARG_END
.InitCeilingSentryT	dc.b	TEMARG_LONG,TEMARG_LONG,TEMARG_EOTRREF,TEMARG_BYTE,TEMARG_BYTE,TEMARG_END
.InitLinearPlayerT	dc.b	TEMARG_WORD,TEMARG_LONG,TEMARG_LONG,TEMARG_PROGREF,TEMARG_WORD,TEMARG_WORD,TEMARG_END
.InitBGPointCollisionT	dc.b	TEMARG_LONG,TEMARG_LONG,TEMARG_EOTRREF,TEMARG_BYTE,TEMARG_BYTE,TEMARG_PROGREF,TEMARG_WORD,TEMARG_END
.InitBGCollisionT	dc.b	TEMARG_EOTRREF,TEMARG_BYTE,TEMARG_BYTE,TEMARG_PROGREF,TEMARG_WORD,TEMARG_END
.InitTrackPlayerT	dc.b	TEMARG_PLYRREF,TEMARG_LONG,TEMARG_LONG,TEMARG_WORD,TEMARG_END
.DirectionMoveT		dc.b	TEMARG_LONG,TEMARG_WORD,TEMARG_END
.InitJitterSpeedT	dc.b	TEMARG_LONG,TEMARG_LONG,TEMARG_LONG,TEMARG_LONG,TEMARG_LONG,TEMARG_LONG,TEMARG_WORD,TEMARG_WORD,TEMARG_END
.SetRNDT
.AddRNDT		dc.b	TEMARG_LONG,TEMARG_LONG,TEMARG_WORD,TEMARG_END
.ScrollXCenterPlayersT	dc.b	TEMARG_LONG,TEMARG_WORD,TEMARG_WORD,TEMARG_LONG,TEMARG_END
.Dir8ImageT
.InitDir8ImageT		dc.b	TEMARG_IMAGEREF,TEMARG_IMAGEREF,TEMARG_IMAGEREF,TEMARG_IMAGEREF
			dc.b	TEMARG_IMAGEREF,TEMARG_IMAGEREF,TEMARG_IMAGEREF,TEMARG_IMAGEREF
			dc.b	TEMARG_ANIMREF,TEMARG_WORD,TEMARG_END
.Dir16ImageT
.InitDir16ImageT	dc.b	TEMARG_IMAGEREF,TEMARG_IMAGEREF,TEMARG_IMAGEREF,TEMARG_IMAGEREF
			dc.b	TEMARG_IMAGEREF,TEMARG_IMAGEREF,TEMARG_IMAGEREF,TEMARG_IMAGEREF
			dc.b	TEMARG_IMAGEREF,TEMARG_IMAGEREF,TEMARG_IMAGEREF,TEMARG_IMAGEREF
			dc.b	TEMARG_IMAGEREF,TEMARG_IMAGEREF,TEMARG_IMAGEREF,TEMARG_IMAGEREF
			dc.b	TEMARG_ANIMREF,TEMARG_WORD,TEMARG_END

.InitDir32ImageT
.InitDir64ImageT	dc.b	TEMARG_ANIMREF,TEMARG_WORD,TEMARG_END
.InitDir8AnimT		dc.b	TEMARG_WORD,TEMARG_ANIMREF,TEMARG_ANIMREF,TEMARG_ANIMREF,TEMARG_ANIMREF
			dc.b	TEMARG_ANIMREF,TEMARG_ANIMREF,TEMARG_ANIMREF,TEMARG_ANIMREF,TEMARG_END
.InitDir16AnimT		dc.b	TEMARG_WORD
			dc.b	TEMARG_ANIMREF,TEMARG_ANIMREF,TEMARG_ANIMREF,TEMARG_ANIMREF
			dc.b	TEMARG_ANIMREF,TEMARG_ANIMREF,TEMARG_ANIMREF,TEMARG_ANIMREF
			dc.b	TEMARG_ANIMREF,TEMARG_ANIMREF,TEMARG_ANIMREF,TEMARG_ANIMREF
			dc.b	TEMARG_ANIMREF,TEMARG_ANIMREF,TEMARG_ANIMREF,TEMARG_ANIMREF
			dc.b	TEMARG_END
.InitDir32AnimT		dc.b	TEMARG_WORD
			dc.b	TEMARG_ANIMREF,TEMARG_ANIMREF,TEMARG_ANIMREF,TEMARG_ANIMREF
			dc.b	TEMARG_ANIMREF,TEMARG_ANIMREF,TEMARG_ANIMREF,TEMARG_ANIMREF
			dc.b	TEMARG_ANIMREF,TEMARG_ANIMREF,TEMARG_ANIMREF,TEMARG_ANIMREF
			dc.b	TEMARG_ANIMREF,TEMARG_ANIMREF,TEMARG_ANIMREF,TEMARG_ANIMREF
			dc.b	TEMARG_ANIMREF,TEMARG_ANIMREF,TEMARG_ANIMREF,TEMARG_ANIMREF
			dc.b	TEMARG_ANIMREF,TEMARG_ANIMREF,TEMARG_ANIMREF,TEMARG_ANIMREF
			dc.b	TEMARG_ANIMREF,TEMARG_ANIMREF,TEMARG_ANIMREF,TEMARG_ANIMREF
			dc.b	TEMARG_ANIMREF,TEMARG_ANIMREF,TEMARG_ANIMREF,TEMARG_ANIMREF
			dc.b	TEMARG_END
.InitDir64AnimT		dc.b	TEMARG_WORD
			dc.b	TEMARG_ANIMREF,TEMARG_ANIMREF,TEMARG_ANIMREF,TEMARG_ANIMREF
			dc.b	TEMARG_ANIMREF,TEMARG_ANIMREF,TEMARG_ANIMREF,TEMARG_ANIMREF
			dc.b	TEMARG_ANIMREF,TEMARG_ANIMREF,TEMARG_ANIMREF,TEMARG_ANIMREF
			dc.b	TEMARG_ANIMREF,TEMARG_ANIMREF,TEMARG_ANIMREF,TEMARG_ANIMREF
			dc.b	TEMARG_ANIMREF,TEMARG_ANIMREF,TEMARG_ANIMREF,TEMARG_ANIMREF
			dc.b	TEMARG_ANIMREF,TEMARG_ANIMREF,TEMARG_ANIMREF,TEMARG_ANIMREF
			dc.b	TEMARG_ANIMREF,TEMARG_ANIMREF,TEMARG_ANIMREF,TEMARG_ANIMREF
			dc.b	TEMARG_ANIMREF,TEMARG_ANIMREF,TEMARG_ANIMREF,TEMARG_ANIMREF
			dc.b	TEMARG_ANIMREF,TEMARG_ANIMREF,TEMARG_ANIMREF,TEMARG_ANIMREF
			dc.b	TEMARG_ANIMREF,TEMARG_ANIMREF,TEMARG_ANIMREF,TEMARG_ANIMREF
			dc.b	TEMARG_ANIMREF,TEMARG_ANIMREF,TEMARG_ANIMREF,TEMARG_ANIMREF
			dc.b	TEMARG_ANIMREF,TEMARG_ANIMREF,TEMARG_ANIMREF,TEMARG_ANIMREF
			dc.b	TEMARG_ANIMREF,TEMARG_ANIMREF,TEMARG_ANIMREF,TEMARG_ANIMREF
			dc.b	TEMARG_ANIMREF,TEMARG_ANIMREF,TEMARG_ANIMREF,TEMARG_ANIMREF
			dc.b	TEMARG_ANIMREF,TEMARG_ANIMREF,TEMARG_ANIMREF,TEMARG_ANIMREF
			dc.b	TEMARG_ANIMREF,TEMARG_ANIMREF,TEMARG_ANIMREF,TEMARG_ANIMREF
			dc.b	TEMARG_END
.RestrictDirectionT	dc.b	TEMARG_WORD,TEMARG_LONG,TEMARG_LONG,TEMARG_END
.RandomEventT		dc.b	TEMARG_WORD,TEMARG_PROGREF,TEMARG_WORD,TEMARG_END
.InitRandomEventT	dc.b	TEMARG_WORD,TEMARG_PROGREF,TEMARG_WORD,TEMARG_END
.InitMousePointerT	dc.b	TEMARG_WORD,TEMARG_WORD,TEMARG_WORD,TEMARG_WORD
			dc.b	TEMARG_LONG,TEMARG_LONG,TEMARG_LONG,TEMARG_LONG,TEMARG_LONG,TEMARG_LONG
			dc.b	TEMARG_WORD,TEMARG_LONG,TEMARG_END
.ScrollYCenterPlayersT	dc.b	TEMARG_LONG,TEMARG_WORD,TEMARG_LONG,TEMARG_WORD,TEMARG_END

_OperatorNames		dc.l	.NoName,			.InitXSpeedAnimName,			.InitLinearAccelName
			dc.l	.InitAsteroidsPlayerName,	.InitInertialPlayerName,				.FireBName
			dc.l	.EXECUTEName,			.CyclicAnimName,			.Dir16AnimName
			dc.l	.InitBGCollName,		.InitJoyMoveName,			.KILLOBJECTName
			dc.l	.ScrollConstantXSpeedName,	.NoName,				.InitPlatPlyrName
			dc.l	.StatPrintVarName,		.InitPlatformPlayerName,				.SETIMAGEName
			dc.l	.InitXTrackName,		.InitYTrackName,			.InitScrollName
			dc.l	.GetHurtName,			.FireBayName,				.PowerUpName
			dc.l	.PowerDownName,			.SetBayName,				.SetChannelName
			dc.l	.HighlightBobName,		.SetColourName,				.KillChannelName
			dc.l	.InitMousePlayerName,		.InitActionListPlayerName,		.InitCheckFireButtonsName
			dc.l	.InitYSpeedAnimName,		.SimpleSFXName,				.PlaySFXName
			dc.l	.VBlankSpriteName,		.MainLoopSpriteName,			.InitInertialHomerName
			dc.l	.StatPrintScoreName,		.InitHomerBadDude1Name,			.TimeDelayExecuteName
			dc.l	.InitFollowPathName,		.OLDDirectionMoveName,			.InitChannelExecuteName
			dc.l	.InitDirectionMoveName,		.OldInitDir8AnimName,			.OldInitDir16AnimName
			dc.l	.OldInitDir32AnimName,		.OldInitDir64AnimName,			.OldInitDir8ImageName
			dc.l	.OldInitDir16ImageName,		.OldInitDir32ImageName,			.OldInitDir64ImageName
			dc.l	.OldInitActionListPlayerName,	.SpawnPlayerBulletName,			.SpawnBadDudeName
			dc.l	.SpawnPlayerName,		.InitDir8ActionName,			.InitDir16ActionName
			dc.l	.InitDir32ActionName,		.InitDir64ActionName,			.OldStatPrintScoreName
			dc.l	.StatPrintGlobalScoreName,	.GiveScoreName,				.EndLevelName
			dc.l	.Dir8ExecuteName,		.Dir16ExecuteName,			.Dir32ExecuteName
			dc.l	.Dir64ExecuteName,		.InitNULLName,				.RNDXSpeedName
			dc.l	.RNDYSpeedName,			.InitSteroidsName,			.SetBordersName
			dc.l	.RNDDirectionMoveName,		.SetVarName,				.AddToVarName
			dc.l	.SubFromVarName,		.ExecuteIfVarCCTName,			.OldDir8ImageName
			dc.l	.OldDir16ImageName,		.Dir32ImageName,			.Dir64ImageName
			dc.l	.RotateClockWiseName,		.RotateAntiClockWiseName,		.SetXSpeedsFromList32Name
			dc.l	.SetYSpeedsFromList32Name,	.SetDirectionName,			.InitAttatchName
			dc.l	.SetMiscStuffName,		.InitOperationWolfMovementName,		.RepelName
			dc.l	.SetIName,			.AddIName,				.SetVName
			dc.l	.AddVName,			.OLDSetRNDName,				.OLDAddRNDName
			dc.l	.OBJECTOFFNAME,			.CollisionPartnerExecuteName,		.CollisionPartnerOffName
			dc.l	.KillCollisionPartnerName,	.InitJoystickAnimName,			.SetObjectActionName
			dc.l	.SetCollisionsName,		.PlayModuleName,			.StopModuleName
			dc.l	.InitActionOnFireName,		.CompareVariableName,			.CompareImediateName
			dc.l	.IfEqualName,			.IfNotEqualName,			.IfGreaterThanName
			dc.l	.IfLowerThanName,		.OLDInitBackgroundPointCollisionName,	.SimplePlatformPlayerName
			dc.l	.ExecuteOnPlayerName,		.ExecuteIfOnScreenName,			.AttachToParentName
			dc.l	.InitDrawBarName,		.InitFloorSentryName,			.InitCeilingSentryName
			dc.l	.InitLinearPlayerName,		.InitBGPointCollisionName,		.InitBGCollisionName
			dc.l	.InitTrackPlayerName,		.DirectionMoveName,			.InitJitterSpeedName
			dc.l	.SetRNDName,			.AddRNDName,				.ScrollXCenterPlayersName
			dc.l	.InitDir8ImageName,		.InitDir16ImageName,			.InitDir32ImageName
			dc.l	.InitDir64ImageName,		.Dir8ImageName,				.Dir16ImageName
			dc.l	.InitDir8AnimName,		.InitDir16AnimName,			.InitDir32AnimName
			dc.l	.InitDir64AnimName,		.RestrictDirectionName,			.RandomEventName
			dc.l	.InitRandomEventName,		.InitMousePointerName,			.ScrollYCenterPlayersName

.NoName			dc.b	"",0
.InitXSpeedAnimName	dc.b	"XSpeedAnim",0
.InitLinearAccelName	dc.b	"LinearAcelleration",0
.InitAsteroidsPlayerName	dc.b	"AsteroidsPlayer",0
.InitInertialPlayerName	dc.b	"InertialPlayer",0
.FireBName		dc.b	"Obs Fire Bullet",0
.EXECUTEName		dc.b	"Execute",0
.CyclicAnimName		dc.b	"CyclicAnim",0
.ScrollConstantXSpeedName	dc.b	"ScrollConstantXSpeed",0
.Dir16AnimName		dc.b	"Set Direction dependant anim",0
.InitBGCollName		dc.b	"Set Background Collision",0
.InitJoyMoveName	dc.b	"Setup Inertial Joystick movement",0
.KILLOBJECTName		dc.b	"Kill Bob",0
.InitPlatPlyrName	dc.b	"Setup Platform player",0
.StatPrintVarName	dc.b	"StatPrintVar",0
.InitPlatformPlayerName	dc.b	"PlatformPlayer",0
.SETIMAGEName		dc.b	"Set Image",0
.InitXTrackName		dc.b	"Set Standard X Tracking",0
.InitYTrackName		dc.b	"Set Standard Y Tracking",0
.InitScrollName		dc.b	"Initialise Scrolling",0
.GetHurtName		dc.b	"Get Hurt",0
.FireBayName		dc.b	"Fire Weapon Bay",0
.PowerUpName		dc.b	"Power up Weapon Bay",0
.PowerDownName		dc.b	"Power Down Weapon Bay",0
.SetBayName		dc.b	"Set Weapon Bay",0
.SetChannelName		dc.b	"Set Channel",0
.HighlightBobName	dc.b	"Highlight Bob",0
.SetColourName		dc.b	"Set Colour",0
.KillChannelName	dc.b	"Kill Channel",0
.InitMousePlayerName	dc.b	"MousePlayer",0
.InitActionListPlayerName	dc.b	"ActionListPlayer",0
.InitCheckFireButtonsName	dc.b	"CheckFireButtons",0
.InitYSpeedAnimName	dc.b	"YSpeedAnim",0
.SimpleSFXName		dc.b	"Simple SFX",0
.PlaySFXName		dc.b	"Play SFX",0
.VBlankSpriteName	dc.b	"VBlank sprite",0
.MainLoopSpriteName	dc.b	"MainLoop sprite",0
.InitInertialHomerName	dc.b	"InertialHomer",0
.StatPrintScoreName	dc.b	"StatPrintScore",0
.InitHomerBadDude1Name	dc.b	"Inertial Homer",0
.TimeDelayExecuteName	dc.b	"Time Delay Execute",0
.InitFollowPathName	dc.b	"Follow path",0
.OLDDirectionMoveName	dc.b	"OLDDirectionMove",0
.InitChannelExecuteName	dc.b	"Channel Execute",0
.InitDirectionMoveName	dc.b	"Channel DIR move",0
.OldInitDir8AnimName	dc.b	"OldDir 8 Anim",0
.OldInitDir16AnimName	dc.b	"OldDir 16 Anim",0
.OldInitDir32AnimName	dc.b	"OldDir 32 Anim",0
.OldInitDir64AnimName	dc.b	"OldDir 64 Anim",0
.OldInitDir8ImageName	dc.b	"OldDir 8 Image",0
.OldInitDir16ImageName	dc.b	"OldDir 16 Image",0
.OldInitDir32ImageName	dc.b	"OldDir 32 Image",0
.OldInitDir64ImageName	dc.b	"OldDir 64 Image",0
.OldInitActionListPlayerName dc.b	"Actionlist player",0
.SpawnPlayerBulletName	dc.b	"Spawn Player Bullet",0
.SpawnBadDudeName	dc.b	"Spawn Bad Dude",0
.SpawnPlayerName	dc.b	"Spawn Player",0
.InitDir8ActionName	dc.b	"Dir 8 Action",0
.InitDir16ActionName	dc.b	"Dir 16 Action",0
.InitDir32ActionName	dc.b	"Dir 32 Action",0
.InitDir64ActionName	dc.b	"Dir 64 Action",0
.OldStatPrintScoreName	dc.b	"Stat print score",0
.StatPrintGlobalScoreName dc.b	"StatPrint global score",0
.GiveScoreName		dc.b	"Give score",0
.EndLevelName		dc.b	"End Level",0
.Dir8ExecuteName	dc.b	"Dir 8 Execute",0
.Dir16ExecuteName	dc.b	"Dir 16 Execute",0
.Dir32ExecuteName	dc.b	"Dir 32 Execute",0
.Dir64ExecuteName	dc.b	"Dir 64 Execute",0
.InitNULLName		dc.b	"Null Channel",0
.RNDXSpeedName		dc.b	"RND X Speed",0
.RNDYSpeedName		dc.b	"RND Y Speed",0
.InitSteroidsName	dc.b	"ObsAsteroids player",0
.SetBordersName		dc.b	"Set Borders",0
.RNDDirectionMoveName	dc.b	"RND Direction Move",0
.SetVarName		dc.b	"Set Var",0
.AddToVarName		dc.b	"Add to Var",0
.SubFromVarName		dc.b	"Sub From Var",0	
.ExecuteIfVarCCTName	dc.b	"Execute If Var CC",0
.OldDir8ImageName	dc.b	"Dir 8 Image",0
.OldDir16ImageName	dc.b	"Dir 16 Image",0
.Dir32ImageName		dc.b	"Dir 32 Image",0
.Dir64ImageName		dc.b	"Dir 64 Image",0
.RotateClockWiseName	dc.b	"Rotate Clockwise",0
.RotateAntiClockWiseName dc.b	"Rotate AntiClockwise",0
.SetXSpeedsFromList32Name dc.b	"SetXSpeeds From List 32",0
.SetYSpeedsFromList32Name dc.b	"SetYSpeeds From List 32",0
.SetDirectionName	dc.b	"Set Direction",0
.InitAttatchName	dc.b	"Attatch",0
.SetMiscStuffName 	dc.b 	"SetMiscStuff",0
.InitOperationWolfMovementName dc.b "Operation Wolf Movement",0
.RepelName		dc.b	"Repel",0
.SetIName		dc.b	"Set Imediate",0
.AddIName		dc.b	"Add Imediate",0
.SetVName		dc.b	"Set Variable",0
.AddVName		dc.b	"Add Variable",0
.OLDSetRNDName		dc.b	"Set RND",0
.OLDAddRNDName		dc.b	"Add RND",0
.OBJECTOFFNAME		dc.b	"Object Off",0
.CollisionPartnerExecuteName dc.b "Collision Partner Execute",0
.CollisionPartnerOffName dc.b	"Collision Partner Off",0
.KillCollisionPartnerName dc.b	"Kill Collision Partner",0
.InitJoystickAnimName	dc.b	"Joystick Animation",0
.SetObjectActionName	dc.b	"Set Object Action",0
.SetCollisionsName	dc.b	"Set Collisions",0
.PlayModuleName		dc.b	"Play Module",0
.StopModuleName		dc.b	"Stop Module",0
.InitActionOnFireName	dc.b	"Action List On Fire",0
.CompareVariableName	dc.b	"Compare Variable",0
.CompareImediateName	dc.b	"Compare Imediate",0
.IfEqualName		dc.b	"If Equal",0
.IfNotEqualName		dc.b	"If Not Equal",0
.IfGreaterThanName	dc.b	"If Greater Than",0
.IfLowerThanName	dc.b	"If Lower Than",0
.OLDInitBackgroundPointCollisionName dc.b	"Background Point Collision",0
.SimplePlatformPlayerName dc.b	"Simple Platform Player",0
.ExecuteOnPlayerName	dc.b	"Execute On Player",0
.ExecuteIfOnScreenName	dc.b	"Execute If On Screen",0
.AttachToParentName	dc.b	"Attach To Parent",0
.InitDrawBarName	dc.b	"Draw Bar",0
.InitFloorSentryName	dc.b	"Floor Sentry",0
.InitCeilingSentryName	dc.b	"Ceiling Sentry",0
.InitLinearPlayerName	dc.b	"LinearPlayer",0
.InitBGPointCollisionName	dc.b	"BGPointCollision",0
.InitBGCollisionName	dc.b	"BGCollision",0
.InitTrackPlayerName	dc.b	"TrackPlayer",0
.DirectionMoveName	dc.b	"DirectionMove",0
.InitJitterSpeedName	dc.b	"JitterSpeed",0
.SetRNDName		dc.b	"SetRND",0
.AddRNDName		dc.b	"AddRND",0
.ScrollXCenterPlayersName	dc.b	"ScrollXCenterPlayers",0
.InitDir8ImageName	dc.b	"Dir8Image",0
.InitDir16ImageName	dc.b	"Dir16Image",0
.InitDir32ImageName	dc.b	"Dir32Image",0
.InitDir64ImageName	dc.b	"Dir64Image",0
.Dir8ImageName		dc.b	"Dir8Image",0
.Dir16ImageName		dc.b	"Dir16Image",0
.InitDir8AnimName	dc.b	"Dir8Anim",0
.InitDir16AnimName	dc.b	"Dir16Anim",0
.InitDir32AnimName	dc.b	"Dir32Anim",0
.InitDir64AnimName	dc.b	"Dir64Anim",0
.RestrictDirectionName	dc.b	"RestrictDirection",0
.RandomEventName
.InitRandomEventName	dc.b	"RandomEvent",0
.InitMousePointerName	dc.b	"MousePointer",0
.ScrollYCenterPlayersName dc.b	"ScrollYCenterPlayers",0

;Thebe, Thebe, Thats all Folks.
