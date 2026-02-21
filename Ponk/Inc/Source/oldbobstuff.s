;*****************************************************************************
;Bob stuff

	XDEF 	_DoActionList,_FindFreePlayerDude,_FindFreeBadDude,_FindFreeBulletDude
	XDEF	_BackgroundCollision,_OLDBackgroundPointCollision
	XDEF	_recursiveactionlistaddress,_PointToBackground
	XDEF	_ObjectStructures

	XREF	_NotINITBOBAlthoughFairlySimilar
	XREF	_AllocPlayer,_AllocPlayerBullet,_AllocBadDude
	XREF	_OBJECTOFF

UpdateObjects
	;Inputs:	a2.l = slice structure

	CALL	_GfxBase,OwnBlitter
	lea	_custom,a6
	move.w	#$FFFF,bltafwm(a6)
	move.w	#$0000,bltalwm(a6)

	;BLITNASTY

	RCOLOR	#$FFF
	bsr	WipeBobs

	move.l	SS_BadDudes(a2),a3
	cmp.l	#0,a3
	beq	.NoBadDudes
	RCOLOR	#$0F0
	bsr	UpdateBobList
	tst.w	_GameOn
	beq	.Exit
.NoBadDudes

	move.l	SS_Players(a2),a3
	cmp.l	#0,a3
	beq	.NoPlayers
	RCOLOR	#$030
	bsr	UpdateBobList
	bsr	UpdatePlayerWeaponBays
	;tst.w	_GameOn
	;beq	.Exit
.NoPlayers
	move.l	SS_PlayerBullets(a2),a3
	cmp.l	#0,a3
	beq	.NoPlayerBullets
	RCOLOR	#$080
	bsr	UpdateBobList
.NoPlayerBullets
	;tst.w	_GameOn
	;beq	.Exit


	RCOLOR	#$0FF

	bsr	UpdateTimedAttackWaves
	bsr	UpdateScrollAttackWaves

	bsr	UpdateCurrentWaves

	RCOLOR	#$FF0

	;move.w	SS_CollisionTypes(a2),d2
	;lsr.w	d2
	bsr	PlayerToBD
	;lsr.w	d2
	bsr	PlayerBulletsToBD
;	bsr	PlayersToPlayers
.Exit
	CALL	_GfxBase,DisownBlitter

	;MYKPRINTF "GameOff-ObjectsDone",#0

	rts

UpdateBobList
.Loop1
	move.l	OB_Image(a3),OB_OldImage(a3)
	move.l	OB_AnimXOffset(a3),OB_OldAnimXOffset(a3)
	move.l	OB_AnimYOffset(a3),OB_OldAnimYOffset(a3)

	tst.w	OB_State(a3)
	beq	.GetNextDude

	btst.b	#4,BB_Flags+3(a3)	;Do we update if off screen
	beq	.Update
.CheckIfOnScreen
	btst.b	#2,BB_Flags+3(a3)	;If so are we off screen
	beq	.PutDude
.Update
;	bsr	CheckBorders		;old position...

	move.w	#0,d7

	lea	OB_Channel1(a3),a6
	bsr	UpdateChannel
	lea	OB_Channel2(a3),a6
	bsr	UpdateChannel
	lea	OB_Channel3(a3),a6
	bsr	UpdateChannel
	lea	OB_Channel4(a3),a6
	bsr	UpdateChannel
	lea	OB_Channel5(a3),a6
	bsr	UpdateChannel

.DrawDude

	move.l	BB_XSpeed(a3),d0
	add.l	d0,OB_XPos(a3)
	move.l	BB_YSpeed(a3),d0
	add.l	d0,OB_YPos(a3)

	bsr	CheckBorders		;...new position.

	;cmp.w	#OB_UPDATETYPE_VB,OB_UpdateType(a3)
	;beq	.GetNextDude
.PutDude
	bsr	PutDude
.GetNextDude
	;tst.w	_GameOn
	;beq	.Exit

	move.l	OB_Next(a3),a3
	cmp.l	#0,a3
	bne	.Loop1
	rts
.Exit
	;MYKPRINTF "GameOff-ObjectListDone",#0
	rts


UpdateChannel
	move.l	CL_UpdateCode(a6),a0
	cmp.l	#0,a0
	beq	.CheckTimer
	jsr	(a0)
	tst.w	d7
	beq	.CheckTimer

	move.w	#0,d7
	move.l	#0,CL_UpdateCode(a6)
	move.l	CL_DoneChannel(a6),a5
	cmp.l	#0,a5
	beq	.Exit
	move.w	#CONTEXT_BOB,d0
	bsr	_DoActionList
	bra	.Exit
.CheckTimer
	tst.w	CL_Timer(a6)
	beq	.Exit
	sub.w	#1,CL_Timer(a6)
	bne	.Exit
	move.l	#0,CL_UpdateCode(a6)
	move.l	CL_DoneChannel(a6),a5
	cmp.l	#0,a5
	beq	.Exit

	move.w	#CONTEXT_BOB,d0
	bsr	_DoActionList

	;MYKPRINTF "Execute-TimerDoneAction",#0

.Exit
	move.w	#0,d0
	rts






***********************************************

CheckBorders
	;in: a2 = slice, a3 = bob

;	rts
	move.l	d2,-(sp)

;	btst.b	#0,BB_Flags+3(a3)
;	beq	.NoBorders
;	btst.b	#1,BB_Flags+3(a3)
;	bne	.ViewRelative

	;Check Left Border
	move.l	BB_BorderLeft(a3),d0
	move.l	BB_BorderRight(a3),d1	;opposite border (for wrap)
	tst.b	BB_BorderLeftType(a3)
	beq	.CheckRightBorder
	bpl	.LfCheck
	;relative
	move.l	SS_Scroll(a2),a0
	cmp.l	#0,a0
	beq	.LfCheck
	add.l	SC_Pos(a0),d0
	add.l	SC_Pos(a0),d1
.LfCheck
	cmp.l	OB_XPos(a3),d0
	ble	.CheckRightBorder

	move.b	BB_BorderLeftType(a3),d2
	abs.b	d2
	and.w	#$FF,d2
	lsl.w	#2,d2
	lea	CB_PastLeftRoutines,a1
	move.l	(a1,d2.w),a1
	jsr	(a1)
	bra	.CheckTopBorder

.CheckRightBorder
	move.l	BB_BorderRight(a3),d0
	move.l	BB_BorderLeft(a3),d1	;opposite border (for wrap)
	tst.b	BB_BorderRightType(a3)
	beq	.CheckTopBorder
	bpl	.RtCheck
	;relative
	move.l	SS_Scroll(a2),a0
	cmp.l	#0,a0
	beq	.RtCheck
	add.l	SC_Pos(a0),d0
	add.l	SC_Pos(a0),d1
.RtCheck
	cmp.l	OB_XPos(a3),d0
	bge	.CheckTopBorder

	move.b	BB_BorderRightType(a3),d2
	abs.b	d2
	and.w	#$FF,d2
	lsl.w	#2,d2
	lea	CB_PastRightRoutines,a1
	move.l	(a1,d2.w),a1
	jsr	(a1)

.CheckTopBorder
	move.l	BB_BorderTop(a3),d0
	move.l	BB_BorderBottom(a3),d1	;opposite border (for wrap)
	tst.b	BB_BorderTopType(a3)
	beq	.CheckBottomBorder
	bpl	.TopCheck
	;relative
	move.l	SS_Scroll(a2),a0
	cmp.l	#0,a0
	beq	.TopCheck
	add.l	SC_YPos(a0),d0
	add.l	SC_YPos(a0),d1
.TopCheck
	cmp.l	OB_YPos(a3),d0
	ble	.CheckBottomBorder

	move.b	BB_BorderTopType(a3),d2
	abs.b	d2
	and.w	#$FF,d2
	lsl.w	#2,d2
	lea	CB_PastTopRoutines,a1
	move.l	(a1,d2.w),a1
	jsr	(a1)
	bra	.Done

.CheckBottomBorder
	move.l	BB_BorderBottom(a3),d0
	move.l	BB_BorderTop(a3),d1	;opposite border (for wrap)
	tst.b	BB_BorderBottomType(a3)
	beq	.Done
	bpl	.BotCheck
	;relative
	move.l	SS_Scroll(a2),a0
	cmp.l	#0,a0
	beq	.BotCheck
	add.l	SC_YPos(a0),d0
	add.l	SC_YPos(a0),d1
.BotCheck
	cmp.l	OB_YPos(a3),d0
	bge	.Done

	move.b	BB_BorderBottomType(a3),d2
	abs.b	d2
	and.w	#$FF,d2
	lsl.w	#2,d2
	lea	CB_PastBottomRoutines,a1
	move.l	(a1,d2.w),a1
	jsr	(a1)

.Done
.NoBorders
	move.l	(sp)+,d2
	rts

	;Handler routines for CheckBorders
	;in: d0 = border, d1 = opposite border, a2=slice, a3=bob
CB_XStop
	move.l	d0,OB_XPos(a3)
	move.l	#0,BB_XSpeed(a3)
	rts
CB_YStop
	move.l	d0,OB_YPos(a3)
	move.l	#0,BB_YSpeed(a3)
	rts
CB_XWrap
	move.l	d1,OB_XPos(a3)
	rts
CB_YWrap
	move.l	d1,OB_YPos(a3)
	rts
CB_XBounce1
	move.l	d0,OB_XPos(a3)
	neg.l	BB_XSpeed(a3)
	neg.b	BB_Direction(a3)
	rts
CB_YBounce1
	move.l	d0,OB_YPos(a3)
	neg.l	BB_YSpeed(a3)
	move.b	#128,d0
	sub.b	BB_Direction(a3),d0
	move.b	d0,BB_Direction(a3)
	rts
CB_XBounce2
	move.l	d0,OB_XPos(a3)
	neg.l	BB_XSpeed(a3)
	rts
CB_YBounce2
	move.l	d0,OB_YPos(a3)
	neg.l	BB_YSpeed(a3)
	rts



***********************************************

PutDude
	move.l	OB_Image(a3),a0
	cmp.l	#0,a0
	beq	.CheckBlankSprite
	cmp.w	#IM_TYPE_BOB,IM_Type(a0)
	beq	.BlitBob
	;bsr	_PutSprite
	bra	.Exit
.BlitBob
	lea	_custom,a6
	bsr	_WaitBlit
	move.w	#$FFFF,bltafwm(a6)
	move.w	#$0000,bltalwm(a6)
	bsr	BlitBob
.CheckBlankSprite
	cmp.l	#0,OB_PrevImage(a3)
	beq	.NoBlankSprite

	move.l	OB_PrevImage(a3),a0
	;bsr	_BlankSprite
.NoBlankSprite
.Exit
	rts

***********************************************


UpdatePlayerWeaponBays
	lea	PlayerWeaponBanks,a0
	move.w	#(MAXPLAYERS*WEAPONBAYS)-1,d0
.Loop
	move.l	(a0)+,a1
	cmp.l	#0,a1
	beq	.NextWeapon
	cmp.w	#0,WP_BulletDelay(a1)
	beq	.NextWeapon
	sub.w	#1,WP_BulletDelay(a1)
.NextWeapon
	dbra	d0,.Loop
.Exit
	rts

***********************************************

WipeBobs
	;Inputs:	a2.l = Slice

	lea	_custom,a6
	bsr	_WaitBlit
	move.w	#$FFFF,bltafwm(a6)
	move.w	#$FFFF,bltalwm(a6)

	movem.l	a3-a5,-(sp)
	
	move.l	SS_Players(a2),a3
	cmp.l	#0,a3
	beq	.NoPlayers
	lea	SS_Players(a2),a4
	lea	SS_PlayersTail(a2),a5
	bsr	WipeList
.NoPlayers
	move.l	SS_PlayerBullets(a2),a3
	cmp.l	#0,a3
	beq	.NoPlayerBullets
	lea	SS_PlayerBullets(a2),a4
	lea	SS_PlayerBulletsTail(a2),a5
	bsr	WipeList
.NoPlayerBullets
	move.l	SS_BadDudes(a2),a3
	cmp.l	#0,a3
	beq	.NoBadDudes
	lea	SS_BadDudes(a2),a4
	lea	SS_BadDudesTail(a2),a5
	bsr	WipeList
.NoBadDudes
	bra	.Exit
.Exit
	movem.l	(sp)+,a3-a5
	rts

WipeList
.Loop	
	move.w	SS_ScreenToggle(a2),d0
	lsl.w	#2,d0			;make long word

	lea	OB_ScreenOffset1(a3),a1
	move.l	(a1,d0.w),d1
	bpl	.Wipe
	tst.w	OB_State(a3)
	bne	.Wipe
.DeLink
	bra	.GetNext	;KILL ME
	move.l	OB_Next(a3),a1
	cmp.l	#0,a1
	bne	.NextExists
.NoNext
	move.l	OB_Previous(a3),a0
	cmp.l	#0,a0
	bne	.NoNextPrevExists
.NoNextNoPrev
	move.l	#0,(a4)
	move.l	#0,(a5)
	bra	.LinkToMain
.NoNextPrevExists
	move.l	a0,(a5)
	move.l	#0,OB_Next(a0)
	bra	.LinkToMain
.NextExists
	move.l	OB_Previous(a3),a0
	cmp.l	#0,a0
	bne	.NextExistsPrevExists
.NextExistsNoPrev
	move.l	a1,(a4)
	move.l	#0,OB_Previous(a1)
	bra	.LinkToMain
.NextExistsPrevExists
	move.l	a0,OB_Previous(a1)
	move.l	a4,OB_Next(a0)
.LinkToMain
	move.l	_ObjectStructures,a0
	cmp.l	#0,a0
	beq	.OnlyOneInList

	move.l	a3,OB_Previous(a0)
	move.l	a0,OB_Next(a3)
	move.l	#0,OB_Previous(a3)
	move.l	a3,_ObjectStructures
	bra	.GetNext
.OnlyOneInList
	move.l	#0,OB_Next(a3)
	move.l	#0,OB_Previous(a3)
	move.l	a3,_ObjectStructures
.Wipe
	bsr	WipeBob
.GetNext
	move.l	OB_Next(a3),a3
	cmp.l	#0,a3
	bne	.Loop
.Exit
	rts

Loop1
	move.w	SS_ScreenToggle(a2),d0
	lsl.w	#2,d0			;make long word

	lea	OB_ScreenOffset1(a3),a1
	move.l	(a1,d0.w),d1
	bpl	.Wipe
	tst.w	OB_State(a3)
	bne	.Wipe
.DeLink
	move.l	OB_Next(a3),a0
	move.l	a0,-(sp)
	cmp.l	#0,a0
	beq	.NoNext
	move.l	OB_Previous(a3),OB_Previous(a0)
.NoNext
	move.l	OB_Previous(a3),a0
	cmp.l	#0,a0
	bne	.Previous
.NoPrevious
	cmp.l	SS_Players(a2),a3
	bne	.NotPlayers
	move.l	OB_Next(a3),SS_Players(a2)
	bra	.LinkToMain
.NotPlayers	
	cmp.l	SS_PlayerBullets(a2),a3
	bne	.NotPlayerBullets
	move.l	OB_Next(a3),SS_PlayerBullets(a2)
	bra	.LinkToMain
.NotPlayerBullets
	move.l	OB_Next(a3),SS_BadDudes(a2)
	bra	.LinkToMain
.Previous
	move.l	OB_Next(a3),OB_Next(a0)
.LinkToMain

	move.l	_ObjectStructures,a0
	cmp.l	#0,a0
	beq	.OnlyOneInList
	move.l	a3,OB_Previous(a0)
	move.l	a0,OB_Next(a3)
	move.l	#0,OB_Previous(a3)
	move.l	a3,_ObjectStructures
	bra	.SetNext
.OnlyOneInList
	move.l	#0,OB_Next(a3)
	move.l	#0,OB_Previous(a3)
	move.l	a3,_ObjectStructures
.SetNext
	move.l	(sp)+,a3
	cmp.l	#0,a3
	bne	Loop1
	bra	.Exit
.Wipe
	bsr	WipeBob
.GetNext1
	move.l	OB_Next(a3),a3
	cmp.l	#0,a3
	bne	Loop1
.Exit
	rts

***********************************************

_DoActionList
	;Inputs		a2.l = Slice
	;		a3.l = Bob1
	;		a4.l = Bob2
	;		a5.l = ActionList
	;		d0.w = Context (see Constants.i)

	cmp.l	#0,a5
	beq	.NullActionList
	
	move.w	_ActionContext,-(sp)
	move.w	d0,_ActionContext

	move.w	#0,d7

	move.l	OldAction,a0
	cmp.l	(a0),a5
	bne	.ActionOk
	move.l	a5,_recursiveactionlistaddress
	bra	.Exit
.ActionOk
	add.l	#4,OldAction
	move.l	a5,4(a0)
.Loop
	move.l	(a5)+,a1
	cmp.l	#0,a1
	beq	.ActionDone
	jsr	(a1)

	;tst.w	_GameOn
	;beq	.ActionDone1

	tst.w	d7
	beq	.Loop
.Error
	move.w	#0,d7
	;bra	.ActionDone
.ActionDone1
	;MYKPRINTF "ActionDone",#0
.ActionDone
	move.l	OldAction,a0
	move.l	#0,(a0)
	sub.l	#4,OldAction
.Exit
	move.w	(sp)+,_ActionContext
.NullActionList
	rts


***********************************************

UpdateTimedAttackWaves
	;Time dependant attack waves
	;Inputs:	a2.l = Slice

.ReDo
	move.w	SS_TimedWavesToGo(a2),d0
	beq	.BuggerOff

	move.l	SS_NextTimedAttackWave(a2),a0

	move.w	TW_Time(a0),d0
	move.l	_LevelClock,d1
	cmp.w	d1,d0
	bhi	.BuggerOff

	;DING!
	move.b	TW_Type(a0),d0
	cmp.b	#TWTYPE_ACTIONLIST,d0
	beq	.Type_ActionList
	cmp.b	#TWTYPE_DUDEINIT,d0
	beq	.Type_DudeInit
	cmp.b	#TWTYPE_FORMATION,d0
	beq	.Type_Formation

.NextWave
	add.l	#TW_SIZEOF,SS_NextTimedAttackWave(a2)
	sub.w	#1,SS_TimedWavesToGo(a2)
	bra	.ReDo

.BuggerOff
	rts

.Type_ActionList
	move.l	TW_Event(a0),d0
	beq	.NextWave
	move.l	a5,-(sp)

	move.l	d0,a5
	move.w	#CONTEXT_NONE,d0
	;slice in a2
	bsr	_DoActionList

	move.l	(sp)+,a5

	bra	.NextWave




.Type_DudeInit
	movem.l	a3-a5,-(sp)
	move.l	a0,a4

	bsr	_FindFreeBadDude
	cmp.l	#0,a0
	beq	.NoDude

	move.l	a0,a3
	move.w	#OB_STATE_ON,OB_State(a3)
	move.w	#OB_TYPE_BADDUDE,OB_Type(a3)

	bsr	_NotINITBOBAlthoughFairlySimilar

	;XYZZY - check viewrelative here...
	move.l	TW_XPos(a4),OB_XPos(a3)
	move.l	TW_YPos(a4),OB_YPos(a3)

	move.l	#0,BB_XSpeed(a3)
	move.l	#0,BB_YSpeed(a3)

	move.l	TW_Event(a4),a5
	cmp.l	#0,a5
	beq	.NoDude

	move.w	#CONTEXT_BOB,d0
	bsr	_DoActionList

.NoDude
	movem.l	(sp)+,a3-a5
	bra	.NextWave


.Type_Formation
	move.w	#50,d0
	move.l	SS_ActiveAttackWaves(a2),a1
.Loop
	tst.l	UFM_Previous(a1)
	beq	.FoundFree
	add.l	#UFM_SizeOf,a1
	dbra	d0,.Loop
	bra	.NextWave			;Error

.FoundFree
	move.l	a4,-(sp)

	move.l	SS_ActiveAttackWaveHead(a2),a4
	cmp.l	#0,a4
	bne	.SetHead

	move.l	#-1,UFM_Previous(a1)
	move.l	a1,SS_ActiveAttackWaveHead(a2)
	bra	.FillOutForm
.SetHead
	move.l	a1,UFM_Next(a4)
	move.l	a4,UFM_Previous(a1)
	move.l	a1,SS_ActiveAttackWaveHead(a2)
.FillOutForm
	move.l	TW_Event(a0),a4

	move.w	#-1,UFM_Counter(a1)
	move.w	FM_Dudes(a4),UFM_Dudes(a1)
	move.w	#0,UFM_CurrentDudes(a1)
	move.l	FM_Params(a4),UFM_NextDudeParams(a1)

	move.l	TW_XPos(a0),UFM_XOrigin(a1)
	move.l	TW_YPos(a0),UFM_YOrigin(a1)

	move.l	(sp)+,a4
	bra	.NextWave

***********************************************


UpdateScrollAttackWaves
	;Scroll dependant attack waves
	;Inputs:	a2.l = Slice


	move.l	SS_Scroll(a2),a3

	tst.l	SC_Speed(a3)
	beq	.Exit
	bmi	.GoingLeft
.GoingRight
	move.l	SS_NextLeftScrolledAttackWave(a2),a0
	cmp.l	#0,a0
	beq	.CheckLeftTrigger

	move.l	SW_NextRight(a0),a1
	cmp.l	#0,a1
	beq	.CheckLeftTrigger
	move.l	SW_RightTrigger(a1),d0		;Check where we are in the right
	cmp.l	SC_Pos(a3),d0			;trigger list
	bgt	.CheckLeftTrigger

	move.l	a1,SS_NextLeftScrolledAttackWave(a2)
	bra	.GoingRight
.CheckLeftTrigger
	move.l	SS_NextRightScrolledAttackWave(a2),a0
	cmp.l	#0,a0
	beq	.Exit

	move.l	SW_LeftTrigger(a0),d0		;Check if we need to start a new
	sub.l	SS_ScreenWidth(a2),d0		;left trigger dude
	cmp.l	SC_Pos(a3),d0
	bgt	.Exit
.HitRightTrigger
	move.l	SW_NextLeft(a0),SS_NextRightScrolledAttackWave(a2)
	bsr	StartWave
	bra	.GoingRight

.GoingLeft
	move.l	SS_NextRightScrolledAttackWave(a2),a0
	cmp.l	#0,a0
	beq	.CheckRightTrigger

	move.l	SW_PreviousLeft(a0),a1
	cmp.l	#0,a1
	beq	.CheckRightTrigger
	move.l	SW_LeftTrigger(a1),d0
	sub.l	SS_ScreenWidth(a2),d0
	cmp.l	SC_Pos(a3),d0
	blt	.CheckRightTrigger

	move.l	a1,SS_NextRightScrolledAttackWave(a2)
	bra	.GoingLeft
.CheckRightTrigger
	move.l	SS_NextLeftScrolledAttackWave(a2),a0
	cmp.l	#0,a0
	beq	.Exit

	move.l	SW_RightTrigger(a0),d0
	cmp.l	SC_Pos(a3),d0	
	blt	.Exit
.HitLeftTrigger
	move.l	SW_PreviousRight(a0),SS_NextLeftScrolledAttackWave(a2)
	bsr	StartWave
	bra	.GoingLeft
.Exit
	rts

StartWave
	tst.w	SW_AppearCount(a0)
	beq	.CountDone
	bmi	.Exit

	sub.w	#1,SW_AppearCount(a0)
	bne	.CountDone
	move.w	#-1,SW_AppearCount(a0)
.CountDone
	move.w	AW_Flags(a0),d0
	and.w	#$7,d0			;Lower 3 bits = type

	beq	.StartDude
	cmp.w	#1,d0
	beq	.StartFormation
	cmp.w	#2,d0
	beq	.StartAction
	bra	.Exit

.StartDude
	movem.l	a3-a5,-(sp)
	move.l	a0,a4

	bsr	_FindFreeBadDude
	cmp.l	#0,a0
	beq	.NoDude

	move.l	a0,a3

	bsr	_NotINITBOBAlthoughFairlySimilar

	move.w	#OB_STATE_ON,OB_State(a3)
	move.w	#OB_TYPE_BADDUDE,OB_Type(a3)

	move.l	a4,OB_Parent(a3)
	move.l	#PARENT_SCROLLWAVE,OB_ParentType(a3)

	move.l	AW_XOrg(a4),OB_XPos(a3)
	move.l	AW_YOrg(a4),OB_YPos(a3)

	move.l	#0,BB_XSpeed(a3)
	move.l	#0,BB_YSpeed(a3)

	move.l	SS_BadDudes(a2),a0

	move.l	AW_Data(a4),a5
	cmp.l	#0,a5
	beq	.NoDude

	move.w	#CONTEXT_BOB,d0
	bsr	_DoActionList
.NoDude
	movem.l	(sp)+,a3-a5
	bra	.Exit

.StartFormation
	move.w	#50,d0
	move.l	SS_ActiveAttackWaves(a2),a1
.Loop
	tst.l	UFM_Previous(a1)
	beq	.FoundFree
	add.l	#UFM_SizeOf,a1
	dbra	d0,.Loop
	bra	.Exit			;Error
.FoundFree
	move.l	a4,-(sp)

	move.l	SS_ActiveAttackWaveHead(a2),a4
	cmp.l	#0,a4
	bne	.SetHead

	move.l	#-1,UFM_Previous(a1)
	move.l	a1,SS_ActiveAttackWaveHead(a2)
	bra	.FillOutForm
.SetHead
	move.l	a1,UFM_Next(a4)
	move.l	a4,UFM_Previous(a1)
	move.l	a1,SS_ActiveAttackWaveHead(a2)
.FillOutForm
	move.l	AW_Data(a0),a4

	move.w	#-1,UFM_Counter(a1)
	move.w	FM_Dudes(a4),UFM_Dudes(a1)
	move.w	#0,UFM_CurrentDudes(a1)
	move.l	FM_Params(a4),UFM_NextDudeParams(a1)

	move.l	AW_XOrg(a0),UFM_XOrigin(a1)
	move.l	AW_YOrg(a0),UFM_YOrigin(a1)

	move.l	(sp)+,a4
	bra	.Exit

.StartAction
	move.l	a5,-(sp)
	move.l	AW_Data(a0),a5
	cmp.l	#0,a5
	beq	.NoAction
	move.w	#CONTEXT_NONE,d0
	bsr	_DoActionList
.NoAction
	move.l	(sp)+,a5

.Exit
	rts

***********************************************

UpdateRandomAttackWaves
	;Random dependant attack waves
	;Inputs:	a2.l = Slice
	;Seq:		Type.w,Data.l,AND.w,AttackWave.l,XOrigin.l,YOrigin.l,Map/View.w
	;Types		-1 = Timer, 0 = RND, 1 = AttackWaveDead

	rts

***********************************************
	
UpdateCurrentWaves
	;Starts the new dudes of formations

	movem.l	a3-a5,-(sp)

	move.l	SS_ActiveAttackWaves(a2),a4
	tst.l	UFM_Previous(a4)		;Previous used as 'state'
	beq	.Exit
.Loop
	add.w	#1,UFM_Counter(a4)
	move.w	UFM_Counter(a4),d0

	tst.w	UFM_Dudes(a4)			;Is Formation finished initalizing
	beq	.GetNext

	move.l	UFM_NextDudeParams(a4),a0
	cmp.w	FMP_Trigger(a0),d0
	blo	.GetNext

	bsr	_FindFreeBadDude
	cmp.l	#0,a0
	beq	.GetNext
	
	move.l	a0,a3
	move.l	UFM_NextDudeParams(a4),a0

	bsr	_NotINITBOBAlthoughFairlySimilar

	move.w	#OB_STATE_ON,OB_State(a3)
	move.w	#OB_TYPE_BADDUDE,OB_Type(a3)

	move.l	UFM_XOrigin(a4),d0
	add.l	FMP_XPos(a0),d0
	move.l	d0,OB_XPos(a3)

	move.l	UFM_YOrigin(a4),d0
	add.l	FMP_YPos(a0),d0
	move.l	d0,OB_YPos(a3)

	move.l	#0,BB_XSpeed(a3)
	move.l	#0,BB_YSpeed(a3)

	move.l	FMP_Action(a0),a5
	cmp.l	#0,a5
	beq	.NoAction

	move.w	#CONTEXT_BOB,d0
	bsr	_DoActionList
.NoAction
	add.l	#FMP_SizeOf,UFM_NextDudeParams(a4)
	add.w	#1,UFM_CurrentDudes(a4)
	sub.w	#1,UFM_Dudes(a4)
.GetNext
	move.l	UFM_Next(a4),a4
	cmp.l	#0,a4
	bne	.Loop
.Exit
	movem.l	(sp)+,a3-a5
	rts


***********************************************

_FindFreePlayerDude
	;a2.l = Slice Structure
	;Outputs:	a0.l = Free bob structure (0 = Fail)

	move.l	a3,-(sp)
	move.l	_ObjectStructures,a0
	cmp.l	#0,a0
	beq	.Exit
.LinkIntoList
	move.l	SS_Players(a2),a1
	cmp.l	#0,a1
	beq	.FirstPlayer

	move.l	OB_Next(a0),a3
	move.l	a3,_ObjectStructures		;Next In Global List Now Becomes head
	cmp.l	#0,a3
	beq	.Skip1
	move.l	#0,OB_Previous(a3)		;First in global list has previous of 0
.Skip1
	btst.b	#2,SS_Flags+3(a2)		;Check weather we add to head or tail
	;beq	.Head
.Tail
	move.l	SS_PlayersTail(a2),a1
	move.l	#0,OB_Next(a0)			;New struct becomes last
	move.l	a0,OB_Next(a1)			;old struct has next of new one
	move.l	a1,OB_Previous(a0)		;new struct has previous of old one
	move.l	a0,SS_PlayersTail(a2)		;New struct now becomes tail
	bra	.Exit
.Head
	move.l	a1,OB_Next(a0)			;First In Player List Becomes  second
	move.l	#0,OB_Previous(a0)		;current has previous of 0
	move.l	a0,OB_Previous(a1)		;Current struct becomes previous to next in player list
	move.l	a0,SS_Players(a2)		;current struct becomes first player
	bra	.Exit

.FirstPlayer
	move.l	OB_Next(a0),a3
	move.l	a3,_ObjectStructures		;Next In Global List Now Becomes head
	cmp.l	#0,a3
	beq	.Skip2
	move.l	#0,OB_Previous(a3)		;First In Global List = Zero
.Skip2	move.l	#0,OB_Next(a0)			;zero current struct's next
	move.l	#0,OB_Previous(a0)		;zero current struct's previous
	move.l	a0,SS_Players(a2)		;current struct becomes last & only player
	move.l	a0,SS_PlayersTail(a2)		;Tail is also = 1st in list
	bra	.Exit
.Fail
	move.l	#0,a0
.Exit
	move.l	(sp)+,a3
	rts

***********************************************

_FindFreeBulletDude
	;a2.l = Slice Structure
	;Outputs:	a0.l = Free bob structure (0 = Fail)

	move.l	a3,-(sp)
	move.l	_ObjectStructures,a0
	cmp.l	#0,a0
	beq	.Exit
.LinkIntoList
	move.l	SS_PlayerBullets(a2),a1
	cmp.l	#0,a1
	beq	.FirstPlayerBullet

	move.l	OB_Next(a0),a3
	move.l	a3,_ObjectStructures		;Next In Global List Now Becomes head
	cmp.l	#0,a3
	beq	.Skip1
	move.l	#0,OB_Previous(a3)		;First in global list has previous of 0
.Skip1
	btst.b	#2,SS_Flags+3(a2)		;Check weather we add to head or tail
	;beq	.Head
.Tail
	move.l	SS_PlayerBulletsTail(a2),a1
	move.l	#0,OB_Next(a0)			;New struct becomes last
	move.l	a0,OB_Next(a1)			;old struct has next of new one
	move.l	a1,OB_Previous(a0)		;new struct has previous of old one
	move.l	a0,SS_PlayerBulletsTail(a2)	;New struct now becomes tail
	bra	.Exit
.Head
	move.l	a3,_ObjectStructures		;Next In Global List Now Becomes head
	move.l	#0,OB_Previous(a3)		;First in global list has previous of 0
	move.l	a1,OB_Next(a0)			;First In Player List Becomes  second
	move.l	a0,OB_Previous(a1)		;Current struct becomes previous to next in player list
	move.l	a0,SS_PlayerBullets(a2)		;current struct becomes first player
	bra	.Exit
.FirstPlayerBullet
	move.l	OB_Next(a0),a3
	cmp.l	#0,a3
	beq	.Fail
	move.l	a3,_ObjectStructures		;Next In Global List Now Becomes head
	move.l	#0,OB_Previous(a3)		;First In Global List = Zero
	move.l	#0,OB_Next(a0)			;zero current struct's next
	move.l	#0,OB_Previous(a0)		;zero current struct's previous
	move.l	a0,SS_PlayerBullets(a2)		;current struct becomes last & only player
	move.l	a0,SS_PlayerBulletsTail(a2)		;current struct becomes last & only player
	bra	.Exit
.Fail
	move.l	#0,a0
.Exit
	move.l	(sp)+,a3
	rts


***********************************************

_FindFreeBadDude
	;a2.l = Slice Structure
	;Outputs:	a0.l = Free bob structure (0 = Fail)

	move.l	a3,-(sp)
	move.l	_ObjectStructures,a0
	cmp.l	#0,a0
	beq	.Exit
.LinkIntoList
	move.l	SS_BadDudes(a2),a1
	cmp.l	#0,a1
	beq	.FirstBadDude

	move.l	OB_Next(a0),a3
	move.l	a3,_ObjectStructures		;Next In Global List Now Becomes head
	cmp.l	#0,a3
	beq	.Skip1
	move.l	#0,OB_Previous(a3)		;First in global list has previous of 0
.Skip1
	btst.b	#2,SS_Flags+3(a2)		;Check weather we add to head or tail
	;beq	.Head
.Tail
	move.l	SS_BadDudesTail(a2),a1
	move.l	#0,OB_Next(a0)			;New struct becomes last
	move.l	a0,OB_Next(a1)			;old struct has next of new one
	move.l	a1,OB_Previous(a0)		;new struct has previous of old one
	move.l	a0,SS_BadDudesTail(a2)		;New struct now becomes tail
	bra	.Exit
.Head
	move.l	a3,_ObjectStructures		;Next In Global List Now Becomes head
	move.l	#0,OB_Previous(a3)		;First in global list has previous of 0
	move.l	a1,OB_Next(a0)			;First In Player List Becomes  second
	move.l	a0,OB_Previous(a1)		;Current struct becomes previous to next in player list
	move.l	a0,SS_BadDudes(a2)		;current struct becomes first player
	bra	.Exit
.FirstBadDude
	move.l	OB_Next(a0),a3
	cmp.l	#0,a3
	beq	.Fail
	move.l	a3,_ObjectStructures		;Next In Global List Now Becomes head
	move.l	#0,OB_Previous(a3)		;First In Global List = Zero
	move.l	#0,OB_Next(a0)			;zero current struct's next
	move.l	#0,OB_Previous(a0)		;zero current struct's previous
	move.l	a0,SS_BadDudes(a2)		;current struct becomes last & only player
	move.l	a0,SS_BadDudesTail(a2)		;current struct becomes last & only player
	bra	.Exit
.Fail
	move.l	#0,a0
.Exit
	move.l	(sp)+,a3
	rts

;******************************************************


PlayerToBD
	;Inputs:	a2.l = Slice
	move.l	SS_Players(a2),a3
	move.l	SS_BadDudes(a2),a4
	bsr	CheckCollision
	rts

PlayersToPlayers
	;Inputs:	a2.l = Slice
	move.l	SS_Players(a2),a3
	move.l	SS_Players(a2),a4
	bsr	CheckCollision
	rts

PlayersToPlayerBullets
	;Inputs:	a2.l = Slice
	move.l	SS_Players(a2),a3
	move.l	SS_Players(a2),a4
	bsr	CheckCollision
	rts

PlayerBulletsToBD
	;Inputs:	a2.l = Slice
	move.l	SS_PlayerBullets(a2),a3
	move.l	SS_BadDudes(a2),a4
	bsr	CheckCollision
	rts

PlayerBulletsToPlayerBullets
	;Inputs:	a2.l = Slice
	move.l	SS_PlayerBullets(a2),a3
	move.l	SS_PlayerBullets(a2),a4
	bsr	CheckCollision
	rts

BDToBD
	;Inputs:	a2.l = Slice
	move.l	SS_BadDudes(a2),a3
	move.l	SS_BadDudes(a2),a4
	bsr	CheckCollision
	rts

CheckCollision
	;Inputs:	a3 = List 1
	;		a4 = List 2

	movem.l	d1-d6/a1-a5,-(sp)

	cmp.l	#NULL,a3			;Do all the standard NULL checks.
	beq	.Exit
	cmp.l	#NULL,a4
	beq	.Exit
	move.l	a4,a5				; Copy out the Second List Base address
.BigLoop					;for quick retreval.
	tst.w	OB_State(a3)
	ble	.GetNextBigBob

	btst.b	#2,BB_Flags+3(a3)		;On screen ?
	bne	.BigCont			;sounds a bit rude

	btst.b	#5,BB_Flags+3(a3)		;Do we not do collision check if offscreen
	bne	.GetNextBigBob
.BigCont

	move.l	OB_Image(a3),a0			; Pre calculate everything we can.
	cmp.l	#0,a0
	beq	.GetNextBigBob

	move.l	OB_XPos(a3),d0
	add.l	OB_AnimXOffset(a3),d0
	move.l	d0,d1
	add.l	IM_ColLeft(a0),d0		;d0.l = Left of Big Bob
	add.l	IM_ColRight(a0),d1		;d1.l = Right of Big Bob

	move.l	OB_YPos(a3),d2
	add.l	OB_AnimYOffset(a3),d2
	move.l	d2,d3
	add.l	IM_ColTop(a0),d2		;d2.l = Top of Big Bob
	add.l	IM_ColBottom(a0),d3		;d3.l = Bottom of Big Bob

	move.l	a5,a4				; Grab our Base address again.
.SmallLoop
	cmp.l	a3,a4
	beq	.NoCollision

	tst.w	OB_State(a4)
	ble	.NoCollision

	btst.b	#2,BB_Flags+3(a3)		;On screen ?
	bne	.SmallCont			;sounds a bit rude

	btst.b	#5,BB_Flags+3(a3)		;Do we not do collision check if offscreen
	bne	.GetNextBigBob
.SmallCont

	move.l	OB_Image(a4),a0
	cmp.l	#0,a0
	beq	.NoCollision

	move.l	OB_YPos(a4),d4
	add.l	OB_AnimYOffset(a4),d4
	add.l	IM_ColTop(a0),d4
	cmp.l	d4,d3				;Is Big Bob Bottom < Small Bob Top ?
	blt	.NoCollision			;Yep, No Collision.
	add.l	IM_ColHeight(a0),d4
	cmp.l	d4,d2				;Is Big Bob Top > Small Bob Bottom ?
	bgt	.NoCollision			;Yep, No Collision.

	move.l	OB_XPos(a4),d4
	add.l	OB_AnimXOffset(a4),d4
	add.l	IM_ColLeft(a0),d4
	cmp.l	d4,d1				;Is Big Bob Right < Small Bob Left ?
	blt	.NoCollision			;Yep, No Collision.
	add.l	IM_ColWidth(a0),d4
	cmp.l	d4,d0				;Is Big Bob Left > Small Bob Right ?
	bgt	.NoCollision			;Yep, No Collision.

	bsr	_CollActionList
	move.l	a4,a0
	move.l	a3,a4
	move.l	a0,a3
	bsr	_CollActionList
	move.l	a4,a0
	move.l	a3,a4
	move.l	a0,a3

	tst.w	OB_State(a4)
	beq	.GetNextBigBob
.NoCollision
	move.l	OB_Next(a4),a4			;Get Next Small Bob
	cmp.l	#0,a4
	bne	.SmallLoop
.GetNextBigBob
	move.l	OB_Next(a3),a3			;Get Next Big Bob
	cmp.l	#0,a3
	bne	.BigLoop
.Exit
	movem.l	(sp)+,d1-d6/a1-a5
	rts

_CollActionList
	;Inputs:	a3.l = Object to affect
	;		a4.l = Other object 
	movem.l	a0-a6/d0-d7,-(sp)

	move.w	OB_Type(a4),d0
	lea	BB_HitRoutine1(a3),a0
	move.l	(a0,d0.w),a5
	cmp.l	#0,a5
	beq	.NoCollRoutineHere

	move.w	#CONTEXT_COLLISION,d0
	bsr	_DoActionList

.NoCollRoutineHere
	movem.l	(sp)+,a0-a6/d0-d7
	rts

;XYZZY - still used?

_BackgroundCollision
	;Inputs:	a2.l = Slice
	;		a3.l = Object

	movem.l	d2-d7/a4-a5,-(sp)

	move.w	CL_Data(a6),d4		;d4.w = Last Space Block
	move.w	CL_Data+2(a6),d5	;d5.l = Bit Shifts (-1 = No Bit Shifts)

	move.l	SS_Scroll(a2),a4	;a4.l = Scroll Struct
	move.l	OB_Image(a3),a5		;a5.l = Object Image
	cmp.l	#0,a5
	beq	.Exit

	tst.l	BB_XSpeed(a3)
	beq	.CheckLeftAndRight
	bpl	.GoingRight
.GoingLeft
	bsr	.Left
	bra	.CheckVertical
.GoingRight
	bsr	.Right
	bra	.CheckVertical
.CheckLeftAndRight
	bsr	.Left
	bsr	.Right
.CheckVertical
	tst.l	BB_YSpeed(a3)
	beq	.CheckUpAndDown
	bpl	.GoingDown
.GoingUp
	bsr	.Up
	bra	.Exit
.GoingDown
	bsr	.Down
	bra	.Exit
.CheckUpAndDown
	bsr	.Up
	bsr	.Down
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
	bmi	.LNoAsr
	neg.l	d0
	asr.l	d5,d0
	move.l	d0,BB_XSpeed(a3)
.LNoAsr
	move.l	CL_Data+4(a6),d0	;d0.l = Action List
	beq	.LeftExit
	move.l	a5,-(sp)
	move.l	d0,a5

	move.w	#CONTEXT_BOB,d0
	bsr	_DoActionList
	move.l	(sp)+,a5

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
	bmi	.RNoAsr
	neg.l	d0
	asr.l	d5,d0
	move.l	d0,BB_XSpeed(a3)
.RNoAsr
	move.l	CL_Data+4(a6),d0	;d0.l = Action List
	beq	.RightExit
	move.l	a5,-(sp)
	move.l	d0,a5

	move.w	#CONTEXT_BOB,d0
	bsr	_DoActionList
	move.l	(sp)+,a5

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

.Up
	move.l	OB_XPos(a3),d2
	add.l	OB_AnimXOffset(a3),d2
	add.l	IM_ColLeft(a5),d2
.ULoop
	move.l	OB_YPos(a3),d1
	add.l	OB_AnimYOffset(a3),d1
	add.l	IM_ColTop(a5),d1
	add.l	BB_YSpeed(a3),d1
	move.l	d2,d0

	bsr	_PointToBackground

	cmp.w	d4,d0
	blt	.UNope

	move.l	BB_YSpeed(a3),d0
	move.l	#0,BB_YSpeed(a3)
	tst.w	d5
	bmi	.UNoAsr
	neg.l	d0
	asr.l	d5,d0
	move.l	d0,BB_YSpeed(a3)
.UNoAsr
	move.l	CL_Data+4(a6),d0	;d0.l = Action List
	beq	.UpExit
	move.l	a5,-(sp)
	move.l	d0,a5

	move.w	#CONTEXT_BOB,d0
	bsr	_DoActionList
	move.l	(sp)+,a5

	bra	.UpExit
.UNope
	move.l	OB_XPos(a3),d0
	add.l	OB_AnimXOffset(a3),d0
	add.l	IM_ColRight(a5),d0
	cmp.l	d0,d2
	beq	.UpExit
	add.l	#16<<BITSHIFT,d2
	cmp.l	d0,d2
	ble	.ULoop
	move.l	d0,d2
	bra	.ULoop
.UpExit
	rts

.Down
	move.l	OB_XPos(a3),d2
	add.l	OB_AnimXOffset(a3),d2
	add.l	IM_ColLeft(a5),d2
.DLoop
	move.l	OB_YPos(a3),d1
	add.l	OB_AnimYOffset(a3),d1
	add.l	IM_ColBottom(a5),d1
	add.l	BB_YSpeed(a3),d1
	move.l	d2,d0

	bsr	_PointToBackground

	cmp.w	d4,d0
	blt	.DNope

	move.l	BB_YSpeed(a3),d0
	move.l	#0,BB_YSpeed(a3)
	tst.w	d5
	bmi	.DNoAsr
	lsr.l	d5,d0
	neg.l	d0
	move.l	d0,BB_YSpeed(a3)
.DNoAsr
	move.l	CL_Data+4(a6),d0	;d0.l = Action List
	beq	.DownExit
	move.l	a5,-(sp)
	move.l	d0,a5

	move.w	#CONTEXT_BOB,d0
	bsr	_DoActionList
	move.l	(sp)+,a5

	bra	.DownExit
.DNope
	move.l	OB_XPos(a3),d0
	add.l	OB_AnimXOffset(a3),d0
	add.l	IM_ColRight(a5),d0
	cmp.l	d0,d2
	beq	.DownExit
	add.l	#16<<BITSHIFT,d2
	cmp.l	d0,d2
	ble	.DLoop
	move.l	d0,d2
	bra	.DLoop
.DownExit
	rts

_PointToBackground
	;Assumes SC_MapBank is valid!
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
	bge	.Zero

	mulu	SC_MapBlocksWide(a0),d1
	add.l	d1,d0
	lsl.l	#1,d0

	move.l	SC_MapBank(a0),a0
	add.l	d0,a0

	move.w	(a0),d0
	bra	.Exit
.Zero
	move.w	#0,d0
.Exit
	rts

_OLDBackgroundPointCollision
	;Inputs:	a2.l = Slice
	;		a3.l = Object
	;		CL_Data1.w = Last Space Block
	;		CL_Data1+2.w = Shifts Right On Collision
	;		CL_Data2 = Action List On Collision
	;		CL_Data3 = X Offset (shifted Pixels)
	;		CL_Data4 = Y Offset (shifted Pixels)

	movem.l	d2-d7/a4-a5,-(sp)

	move.w	CL_Data(a6),d4		;d4.w = Last Space Block
	move.w	CL_Data+2(a6),d5	;d5.l = Bit Shifts (-1 = No Bit Shifts)

	move.l	CL_Data3(a6),d6		;d6.l = X Offset
	move.l	CL_Data4(a6),d7		;d7.l = Y Offset

.CheckHorizontal
	move.l	OB_XPos(a3),d0
	add.l	d6,d0
	add.l	BB_XSpeed(a3),d0
	move.l	OB_YPos(a3),d1
	add.l	d7,d1
	bsr	_PointToBackground
	cmp.w	d4,d0
	blt	.CheckVertical
	move.l	BB_XSpeed(a3),d0
	move.l	#0,BB_XSpeed(a3)
	tst.w	d5
	bmi	.NoHorizontalBitShifts
	neg.l	d0
	asr.l	d5,d0
	move.l	d0,BB_XSpeed(a3)
.NoHorizontalBitShifts
	move.l	CL_Data2(a6),d0	;d0.l = Action List
	beq	.CheckVertical
	move.l	a5,-(sp)
	move.l	d0,a5
	move.w	#CONTEXT_BOB,d0
	bsr	_DoActionList
	move.l	(sp)+,a5

.CheckVertical
	move.l	OB_XPos(a3),d0
	add.l	d6,d0
	move.l	OB_YPos(a3),d1
	add.l	d7,d1
	add.l	BB_YSpeed(a3),d1
	bsr	_PointToBackground
	cmp.w	d4,d0
	blt	.Exit
	move.l	BB_YSpeed(a3),d0
	move.l	#0,BB_YSpeed(a3)
	tst.w	d5
	bmi	.NoVerticalBitShifts
	neg.l	d0
	asr.l	d5,d0
	move.l	d0,BB_YSpeed(a3)
.NoVerticalBitShifts
	move.l	CL_Data2(a6),d0		;d0.l = Action List
	beq	.Exit
	move.l	a5,-(sp)
	move.l	d0,a5
	move.w	#CONTEXT_BOB,d0
	bsr	_DoActionList
	move.l	(sp)+,a5
.Exit
	movem.l	(sp)+,d2-d7/a4-a5
	move.w	#0,d0
	rts


BlitBob
	;assumes bltafwm, lwm, custom set
	;inputs		a2.l = Slice struct
	;		a3.l = Bob struct

	movem.l	a4-a5/d2-d6,-(sp)

	bclr.b	#2,BB_Flags+3(a3)
	move.l	OB_Image(a3),a0
	cmp.l	#0,a0
	beq	.Exit

	move.w	#-2,d6
	move.w	IM_DMod(a0),d2			;set up pre worked out modulo's

	move.l	IM_Mask(a0),a4
	move.l	IM_Image(a0),a5
	tst.w	OB_Highlight(a3)
	beq	.NoHighlight
	sub.w	#1,OB_Highlight(a3)
	move.l	IM_Mask(a0),a5
.NoHighlight
	move.w	IM_Size(a0),d3

	move.l	OB_XPos(a3),d0			;get x pos
	add.l	OB_AnimXOffset(a3),d0
	asr.l	#BITSHIFT,d0

	move.l	SS_Scroll(a2),a1

	move.l	SC_Pos(a1),d4			;XClip Left
	add.l	SS_ClipLeft(a2),d4
	lsr.l	#BITSHIFT,d4
	sub.l	d0,d4
	bgt	.ClipLeft
.LeftOk
	move.l	SC_Pos(a1),d4
	add.l	SS_ClipRight(a2),d4
	lsr.l	#BITSHIFT,d4
	move.w	IM_Width(a0),d1
	and.l	#$FFFF,d1
	sub.l	d1,d4
	sub.l	d0,d4				;XClip Right
	bmi	.ClipRight
.RightOk
	move.l	d0,d1
	lsr.l	#3,d0

	bsr	_WaitBlit

	move.l	#12,d4
	lsl.l	d4,d1
	and.l	#$F000,d1
	move.w	d1,bltcon1(a6)
	or.w	#$0FCA,d1			;set bltcon0
	move.w	d1,bltcon0(a6)

	move.l	OB_YPos(a3),d1
	add.l	OB_AnimYOffset(a3),d1
	asr.l	#BITSHIFT,d1

	move.l	SS_ClipTop(a2),d4
	asr.l	#BITSHIFT,d4
	sub.l	d1,d4
	ble	.TopOk
	move.w	IM_Height(a0),d5
	and.l	#$FFFF,d5
	sub.l	d4,d5
	bpl	.ClipTop
	bra	.Exit
.TopOk
	move.l	SS_ClipBottom(a2),d4
	lsr.l	#BITSHIFT,d4

	move.w	IM_Height(a0),d5
	and.l	#$FFFF,d5
	sub.l	d5,d4
	sub.l	d1,d4
	bpl	.BottomOk
	add.l	d4,d5
	bgt	.ClipBottom
	bra	.Exit
.BottomOk

	mulu	SS_BufferWidthxPlanes(a2),d1		; } work out offset into playfield
	add.l	d0,d1					;/
	move.l	SS_LogBase(a2),a1
	add.l	d1,a1					;add offset to base of log screen

	move.w	d6,bltamod(a6)
	move.w	d6,bltbmod(a6)

	move.w	d2,bltcmod(a6)
	move.w	d2,bltdmod(a6)
	move.l	a4,bltapt(a6)
	move.l	a5,bltbpt(a6)

	move.l	a1,bltdpt(a6)			;set c & d pth to playfield
	move.l	a1,bltcpt(a6)

	move.w	SS_ScreenToggle(a2),d0
	lsl.w	d0
	lea	OB_WipeSize(a3),a1erIntName       VBlankIntName       _StatBarHeight    ö  _StatBarImages    ²  _VirtualJoysticks     Z      ò  è   ChipData@ ê   
ÿÿÿþ                                      ï  _NullSprite      _DummyCopper          ò  B¦  B¥  B¤  B£  B¢  B¡  B   BŸ  Bž  B  Bœ  B›  Bš  B™  B˜  B—  B–  B•             jUSER:PAUL_ATKIN/1.fd                                                                         w     LIBS.GS                                                             ü  BÝÿÿÿý  ¢    r  \  6     Ô   ¤   ž   „   f   V   4   &      
          ˆ        Jx   	     	Ì  Ž  Ž  Â  ”  V   ì   ~      	     ˆ  
2  
8  
D  
V  
&  
P  
>  
J      ò                                                                                                                      Çš  Ç™              h                                                                                              €  ë  ø	libinit.o                                                        J  &    ÿÿÿýHERE ANYTHING TO Wipe
	bpl	.Wipe
	tst.w	OB_State(a3)		;is the state off
	bne	.Wipe
.DeLink
	;bra	.GetNext	;KILL ME
	move.l	OB_Next(a3),a1
	cmp.l	#0,a1
	bne	.NextExists
.NoNext
	move.l	OB_Previous(a3),a0
	cmp.l	#0,a0
	bne	.NoNextPrevExists
.NoNextNoPrev
	move.l	#0,(a4)			;no next or previous so we zero pntr in slice struct
	move.l	#0,(a5)
	bra	.LinkToMain
.NoNextPrevExists
	move.l	a0,(a5)			;at tail so we make previous the head
	move.l	#0,OB_Next(a0)		;previous now becomes head
	bra	.LinkToMain
.NextExists
	move.l	OB_Previous(a3),a0
	cmp.l	#0,a0
	bne	.NextExistsPrevExists
.NextExistsNoPrev
	move.l	a0,(a4)			;at head so next becomes head
	move.l	#0,OB_Previous(a1)	;next has previous of zero
	bra	.LinkToMain
.NextExistsPrevExists
	move.l	a0,OB_Previous(a1)	;Somewhere in the middle so- next's previous = current previous
	move.l	a1,OB_Next(a0)		;prev's next = current next
.LinkToMain
	move.l	_ObjectStructures,a0
	cmp.l	#0,a0
	beq	.OnlyOneInList

	move.l	a3,OB_Previous(a0)
	move.l	a0,OB_Next(a3)
	move.l	#0,OB_Previous(a3)
	move.l	a3,_ObjectStructures
	bra	.GetNext
.OnlyOneInList
	move.l	#0,OB_Next(a3)
	move.l	#0,OB_Previous(a3)
	move.l	a3,_ObjectStructures
.Wipe
	bsr	WipeBob
.GetNext
	move.l	OB_Next(a3),a3
	cmp.l	#0,a3
	bne	.Loop
.Exit
	rts

Loop1
	move.w	SS_ScreenToggle(a2),d0
	lsl.w	#2,d0			;mak