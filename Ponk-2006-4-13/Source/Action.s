**********************************************************************************
**	Most of the Stuff to do with the actual running of the game.		**
**********************************************************************************


	incdir	"ck:Ponk/inc/"
	include	"source/libs.gs"
	include	"source/Header.i"
	include	"source/Constants.i"
	include	"source/Structures.i"
	include	"source/RawKeyCodes.i"
	include	"source/Lame.i"

	XDEF	_ActionStart
	XDEF	_stricmp,_strcpy
;	XDEF	_InputChannels
	XDEF	_ScreenSwap,_FirstSlice,_Flags
	XDEF	_UnpackASCII
	XDEF	_CopperIntPtr

	XDEF	_CopperInterruptStruct,CopperIntName,VBlankCounter,CopperIntHandler
	XDEF	VBlankInterruptStruct,VBlankIntName,VBlankCounter,VBlankIntServer
	XDEF	Interrupts,OldCopState,OldIntVector,_DummyCopper,Mode,MainLoopDone
	XDEF	SliceCopperUpdate
	XDEF	_NullSprite
;	XDEF	_ScrollSlice,_SetColumnOffset,_BlockDrawSlice
	XDEF	_JoyDirTable
	XDEF	_GameOn,_Framerate,_StatBarImages
	XDEF	_StatBarImages,_StatBarHeight
	XDEF	_ActionContext
	XDEF	_ReadVirtualJoystick
	XDEF	_VirtualMice
	XDEF	_GameClock,_LevelClock
	XDEF	_VirtualJoysticks

	XREF	_SysBase,_custom,_ALittleCopperList,_LameReq
	XREF	_GfxBase,_DOSBase,_ChipMemBase,_OurTaskBase
	XREF	_ciaa,_LoadWholeFile,_ShellPrintNumber,_ShellPrint
	XREF	_WaitVBlank,_WaitPastVBlank,_JoystickDecode,_WaitBlit,_Random
	XREF	_OpenNewDebugWindow,_FreeDebugWindow,_BlitDebugWindow,_WipeDebugWindow	;,_MoveDebugWindow
	XREF	_StatBoxGroovyConDevice,CON_PutStr,_NullSprites
	XREF	_MainMenu,_NewMenu,_CurrentMenu,_UpdateMenu

	XREF	_EXECUTE,_SETC,_KILL,_HURTDUDE
	XREF	_UpdateYPushSliceTrack,_UpdateXPushSliceTrack
	XREF	_UpdateSprites,_PutSprite,_BlankSprite
	XREF	_RasterTime,_RawMouseY,_RawMouseX
	XREF	_RawKeyArray,_GlobalData
	XREF	_StartPlayerSubTask,_EndPlayerSubTask
	XREF	_CIAABase,_ModulePlayerEnable
	XREF	_InitCIAInterrupt,_StopCIAInterrupt,_SpritePosToggle,_SpriteCopperToggle,_DisplaySprites

	XREF	_InitModulePlayer,_StopModulePlayer,_KPrintF,_FinishAllSamples


	;from Scroll.s
	XREF	_ScrollSlice,_SetColumnOffset,_BlockDrawSlice


;******************************************************************************************************

;FadeData structure
	rsreset
FD_ColourCount	rs.w	1	;number of colours
FD_Timer	rs.w	1	;counts down to zero, then fade stops
FD_SourceList	rs.l	1	;ptr to list of inital colours
FD_SourceMod	rs.l	1	;modulo, can be used to jump MOVE instructions (no postincrement used)
FD_TargetList	rs.l	1	;ptr to list of target RGB colour values
FD_TargetMod	rs.l	1	;modulo, can be used to jump MOVE instructions (no postincrement used)
FD_CopperList	rs.l	1	;ptr to copper-MOVE instructions
FD_Red		rs.w	256	;current red value (8.8 format)
FD_Grn		rs.w	256	;green
FD_Blu		rs.w	256	;blue
FD_RedDeltas	rs.w	256	;8.8 fixed point red-deltas
FD_BluDeltas	rs.w	256	;green
FD_GrnDeltas	rs.w	256	;blue
FD_SIZEOF	rs.w	0


;********************************************************************************************************

	IFNE	CHIPASSEMBLE
	SECTION	WhatLovelyCode,CODE_C
	ELSE
	SECTION	WhatLovelyCode,CODE
	ENDC

;********************************************************************************************************
;Set up stuff

;void __asm ActionStart( register __a0 struct LameReq *lr );

_ActionStart
	;Inputs:	a0.l = LameReq
	movem.l	d0-d7/a1-a6,-(sp)

	MYKPRINTF "NewGameLevel",#0
.SetupNullDisplay
	lea	_custom,a6
	move.l	#_DummyCopper,cop1lc(a6)
	bsr	_WaitVBlank
	bsr	_WaitPastVBlank

	move.w	#DMAF_SETCLR!(DMAF_ALL-DMAF_AUDIO),dmacon(a6)		;dma.on

;******************

.StoreLameStuff
	move.w	lr_FrameRate(a0),_Framerate
	move.w	lr_CopperLists(a0),Mode
	move.l	lr_CopperList1(a0),CopperList1
	move.l	lr_CopperList2(a0),CopperList2
	move.w	lr_ShieldBarHeight(a0),_StatBarHeight

	move.w	lr_TheFlags(a0),_Flags

	move.l	lr_FirstSlice(a0),a1				;Set up slice
	move.l	a1,_FirstSlice

	;move.w	lr_IntroWipe(a0),IntroWipe

	move.l	#GlobalEvents,SS_GlobalEvents(a1)

	bsr	_NullSprites
	eor.w	#1,_SpritePosToggle
	bsr	_NullSprites
	eor.w	#1,_SpritePosToggle

;******************

	bsr	TempSliceStuff

;******************

.SetupInterrupts
	lea	_CopperInterruptStruct,a1	;Set up a copper interrupt for the
	move.b	#NT_INTERRUPT,LN_TYPE(a1)	;bottom of the display window. This
	move.b	#127,LN_PRI(a1)			;triggers the program to start.
	move.l	#CopperIntName,LN_NAME(a1)
	move.l	#VBlankCounter,IS_DATA(a1)		;MainLoop. How handly.
	move.l	#CopperIntHandler,IS_CODE(a1)

	lea	VBlankInterruptStruct,a1
	move.b	#NT_INTERRUPT,LN_TYPE(a1)
	move.b	#7,LN_PRI(a1)	
	move.l	#VBlankIntName,LN_NAME(a1)
	move.l	#VBlankCounter,IS_DATA(a1)
	move.l	#VBlankIntServer,IS_CODE(a1)

	move.l	#INTB_VERTB,d0
	lea	VBlankInterruptStruct,a1
	CALL	_SysBase,AddIntServer
	
	lea	_custom,a5
	move.w	intenar(a5),d0
	and.w	#INTF_COPER,d0
	move.w	d0,OldCopState

	move.w	#INTF_COPER,intena(a5)
	move.l	#INTB_COPER,d0

	lea	_CopperInterruptStruct,a1
	CALL	_SysBase,SetIntVector
	move.l	d0,OldIntVector

	move.w	#INTF_SETCLR!INTF_COPER,intena(a5)
	move.w	#1,Interrupts

;******************

.SetUpBlitter
	lea	_custom,a6
	bsr	_WaitBlit
	move.w	#$FFFF,bltafwm(a6)
	move.w	#$0000,bltalwm(a6)

;******************

.SetUpDebugWindow
	move.l	#0,_CurrentMenu

	;Delete This Some Time
	move.l	_FirstSlice,a0
	move.w	#DEBUG_WINDOW_WIDTH,d0
	move.w	#DEBUG_WINDOW_HEIGHT*8,d1
	move.w	#$FF,d2				;Initial Colour
	bsr	_OpenNewDebugWindow
	lea	_MainMenu,a0
	bsr	_NewMenu

;******************

.ZeroMiscStuff
	move.w	#1,_GameOn
	move.w	#0,FullSpeedAhead
	move.l	#0,_recursiveactionlistaddress
	move.w	#0,_SpritePosToggle
	move.l	#0,_LevelClock

;******************

.SetupSlices
	bsr	SetupSlices
	bsr	SliceCopperUpdate
	;init the mousereading
	bsr	UpdateMice

;******************

.IntroWipe
	bsr	DoIntroWipe

;******************

.SetUpMainLoop
	lea	_custom,a0
	cmp.w	#LACED,Mode
	bra	.MainLoop
	tst.w	vposr(a0)
	bmi	.MainLoop
	move.l	#0,d0
	move.l	#SIGBREAKF_CTRL_C,d1
	CALL	_SysBase,SetSignal

	move.w	#1,MainLoopDone
	move.l	#SIGBREAKF_CTRL_C,d0		;Wait for Ctrl_C sent from the
	CALL	_SysBase,Wait			;Copper interrupt server.


;***********************************************

.MainLoop
	move.w	#1,MainLoopDone
	bsr	WaitForNextCycle
	;CALL	_SysBase,Disable
	;bsr	_WaitVBlank
	;bsr	_WaitPastVBlank

	RCOLOR	#$00F
	move.w	#0,MainLoopDone
	move.l	#0,d0
	move.l	#SIGBREAKF_CTRL_C,d1
	CALL	_SysBase,SetSignal
	;move.w	#noop,_CopperIntPtr

	;move.w	#0,_SpritePosToggle
	;bsr	SliceCopperUpdate
	;CALL	_SysBase,Enable
	;bsr	_WipeDebugWindow		;This MUST always be done soon as possible
	bsr	UpdateInputChannels
	bsr	UpdateSlices
	;bsr	_UpdateMenu
	;bsr	_BlitDebugWindow		;This MUST always be last.

	RCOLOR	#$000

	tst.w	_GameOn
	beq	.ShutDown

	bsr	CheckPauseMode

	lea	_RawKeyArray,a0
	tst.b	Key_Esc(a0)
	bne	.ShutDown

	add.l	#1,_LevelClock
	add.l	#1,_GameClock

	lea	_custom,a6	;?
	lea	VirtualMouse0,a0
	btst.b	#5,VM_ButtonState(a0)		;right button
	beq	.MainLoop

;***********************************************

.ShutDown
	bsr	_WaitVBlank
	bsr	_WaitPastVBlank

	bsr	DoExitWipe

	move.w	#0,_GameOn

	bsr	_FreeDebugWindow

	tst.b	_ModulePlayerEnable		;If playing a module, then we most probably
	bne	.PlayingModule			;want to just leave it going while we goto
	bsr	_FinishAllSamples		;the next level (or whatever).
.PlayingModule

	CALL	_SysBase,Disable

	lea	_custom,a5
	move.w	#INTF_COPER,intena(a5)

	tst.w	Interrupts
	beq	.NoInts

	move.w	#INTB_VERTB,d0
	lea	VBlankInterruptStruct,a1
	CALL	_SysBase,RemIntServer

	move.l	OldIntVector,a1
	move.l	#INTB_COPER,d0
	CALL	_SysBase,SetIntVector
.NoInts
	tst.w	OldCopState
	beq	.Exit
	;move.w	#INTF_SETCLR!INTF_COPER,intena(a5)

.Exit
	CALL	_SysBase,Enable

	;Clear The BSS Data Section
	lea	BSSSectionStart,a0
	move.w	#BSSSectionEnd-BSSSectionStart,d0
	lsr.w	#1,d0
	sub.w	#2,d0
	moveq.l	#0,d1
.ClearBSSLoop
	move.w	d1,(a0)+
	dbra	d0,.ClearBSSLoop
.TheAbsoluteExit
	movem.l	(sp)+,d0-d7/a1-a6
	rts

;***********************************************

CheckPauseMode
	;XYZZY - tidy up exit methods

	;Inputs:	NIL
	;Outputs:	NIL

	lea	_RawKeyArray,a0
	tst.b	Key_Help(a0)
	beq	.Exit
.UpLoop
	tst.b	Key_Help(a0)		;Wait for the Help Key to go back up
	bne	.UpLoop
.StartLoop
	tst.b	Key_Help(a0)
	beq	.TryFireButton
.HelpUpLoop
	tst.b	Key_Help(a0)		;Wait for help key up.
	bne	.HelpUpLoop
	bra	.Exit
.TryFireButton
	move.w	#1,d0
	bsr	_JoystickDecode
	btst	#4,d0
	bne	.FireUpExit
	bra	.StartLoop
.FireUpExit
	move.w	#1,d0
	bsr	_JoystickDecode
	btst	#4,d0
	bne	.FireUpExit
.Exit
	rts

CountActiveStructures
	moveq.l	#0,d0
.Loop1
	tst.w	OB_State(a0)
	beq	.GetNext1
	add.l	#1,d0
.GetNext1
	move.l	OB_Next(a0),a0
	cmp.l	#0,a0
	bne	.Loop1
	rts


WaitForNextCycle
	;Inputs:	NIL
	;Ouputs:	NIL
	; This is just a quick patch to get the game going a little faster.
	;If you hold down the Return Key, then the game will take out the
	;VBlank timing, and just go flat tack. Some alterations had to 
	;be made to the task priority, because if the game takes all
	;the processor power then the input handler can't read the
	;keyboard, and hence it doesn't get the Key_Up message.
	;So if we want to go full speed, I drop the priority down to 
	;something half sane, and put it back up after I'm done.

	lea	_RawKeyArray,a0
	;tst.b	Key_Return(a0)
	;bne	.FullSpeed
.GoSlow
	tst.w	FullSpeedAhead
	beq	.WaitForIt
	;Put The Task Priority Back Up to Normal
	move.l	_OurTaskBase,a1
	move.l	#TASKPRIORITY,d0			;Set Task priority
	CALL	_SysBase,SetTaskPri
	move.w	#0,FullSpeedAhead
.WaitForIt
	move.w	#1,MainLoopDone
	move.w	#intreq,_CopperIntPtr
	move.l	#SIGBREAKF_CTRL_C,d0		;Wait for Ctrl_C sent from the
	CALL	_SysBase,Wait			;Copper interrupt server.
	;bsr	_WaitVBlank
	;bsr	_WaitPastVBlank
	;bsr	_WaitVBlank
	;bsr	_WaitPastVBlank
	bra	.Exit
.FullSpeed
	tst.w	FullSpeedAhead
	bne	.Exit
	;Drop The Task Priority Down
	move.l	_OurTaskBase,a1
	move.l	#1,d0				;Set Task priority
	CALL	_SysBase,SetTaskPri
	move.w	#1,FullSpeedAhead
.Exit
	rts

;VOID __asm UnpackASCII( register __a0 UBYTE *source, register __a1 UBYTE *dest,
;	register __d0 UWORD srcsize );

_UnpackASCII
	;a0 = source - packed array (6 bits/char)
	;a1 = dest buffer
	;d0.w = # of chars

	movem.l	d2-d4,-(sp)

	moveq		#0,d3						;bitbuffer is empty
	sub.w		#1,d0
	bmi			.done

.CharLoop
	move.b	#0,d1						;clear char
	move.w	#6-1,d4					;bit counter

.BitLoop
	sub.w		#1,d3
	bpl			.getbit
	;pull another 16 bits into our buffer
	move.w	(a0)+,d2				;d2 is our buffer
	move.w	#15,d3					;d3 = num of bits in buffer
.getbit
	roxl.w	d2							;grab a bit into x
	roxl.b	d1							;shift into our char
	dbra		d4,.BitLoop

	add.b		#32,d1					;convert to ascii
	cmp.b		#64,d1					;map '@' to NULL
	beq			.done
	move.b	d1,(a1)+				;put char into dest buffer
	dbra	d0,.CharLoop
.done
	move.b	#0,(a1)+				;out of chars - plonk a null at the end...

	movem.l	(sp)+,d2-d4
	rts


TempSliceStuff
	move.l	_FirstSlice,a0

	move.l	SS_Players(a0),a1
	move.l	a1,_ObjectStructures
.Loop1
	cmp.l	#0,OB_Next(a1)
	beq	.PlayerEnd
	move.l	OB_Next(a1),a1
	bra	.Loop1
.PlayerEnd
	move.l	SS_PlayerBullets(a0),OB_Next(a1)
.Loop2
	cmp.l	#0,OB_Next(a1)
	beq	.PlayerBulletsEnd
	move.l	OB_Next(a1),a1
	bra	.Loop2
.PlayerBulletsEnd
	move.l	SS_BadDudes(a0),OB_Next(a1)

	move.l	#0,SS_Players(a0)
	move.l	#0,SS_PlayerBullets(a0)
	move.l	#0,SS_BadDudes(a0)
	rts


;*****************************************************************************************************
;Handlers & Init stuff

CopperIntHandler	;My very own copper interrupt handler. (How very system friendly,...BARF)
	;Inputs:	a0.l = Custom
	;		a1.l = Ptr to VBlank Counter
	;		a5.l = Ptr to Copper Int Server
	;		a6.l = Sysbase
	;Outputs:	Z-Flag Set, so system can sort out all other Interrupts in chain.

	RCOLOR	#$FFF

	cmp.w	#LACED,Mode
	bne	.Continue
	tst.w	vposr(a0)
	bmi	.Number2s

	move.l	CopperList1,cop1lc(a0)
	bra	.Continue
.Number2s
	move.l	CopperList2,cop1lc(a0)
.Continue
	move.w	_Framerate,d0
	cmp.w	(a1),d0
	bcc	.NoSignal
	move.w	#1,(a1)

	tst.w	MainLoopDone
	beq	.NoSignal
.Signal

	tst.w	_GameOn
	beq	.Exit
	move.w	#0,_SpritePosToggle
	bsr	SliceCopperUpdate

	move.l	#SIGBREAKF_CTRL_C,d0	;Signal the Main Loop to go for it.
	move.l	_OurTaskBase,a1
	CALL	_SysBase,Signal

.NoSignal

	movem.l	a0-a6/d0-d7,-(sp)
.Sprites
	move.l	_FirstSlice,a2
	bsr	_UpdateSprites
	bsr	_DisplaySprites
	move.l	SS_Scroll(a2),a3
	move.l	SC_YTrackRoutine(a3),a0
	cmp.l	#0,a0
	beq	.Copper
	jsr	(a0)
.Copper
	movem.l	(sp)+,a0-a6/d0-d7

.Exit
	lea	_custom,a0
	move.w	#INTF_COPER,intreq(a0)
	move.l	#0,d0
	rts

	XDEF	_AnimCopperIntHandler
	XREF	_DoEvents,_eventlist,_SetBitplanePointers
_AnimCopperIntHandler	;My very own copper interrupt handler. (How very system friendly,...BARF)
	;Inputs:	a0.l = Custom
	;		a1.l = Anim Info Node
	;		a5.l = Ptr to Copper Int Server
	;		a6.l = Sysbase
	;Outputs:	Z-Flag Set, so system can sort out all other Interrupts in chain.

	movem.l	a0-a6/d0-d7,-(sp)
	move.l	a1,a3
	add.w	#1,ain_FrameRateCount(a3)
	move.w	ain_FrameRateCount(a3),d0
	cmp.w	ain_FrameRate(a3),d0
	blo	.Exit

	move.w	#0,ain_FrameRateCount(a3)
	tst.w	ain_MainLoopDone(a3)
	beq	.Exit
	move.l	#SIGBREAKF_CTRL_C,d0	;Signal the Main Loop to go for it.
	move.l	_OurTaskBase,a1
	CALL	_SysBase,Signal

	lea	_eventlist,a0
	move.l	a3,a1
	move.l	ain_NextEvent(a3),a2
	move.w	ain_Frame(a3),d0
	bsr	_DoEvents
	move.l	d0,ain_NextEvent(a3)
	move.l	a3,a0
	move.l	ain_BitMap1(a3),a1
	tst.w	ain_ScreenToggle(a3)
	bne	.BitMap1
	move.l	ain_BitMap2(a3),a1
.BitMap1
	bsr	_SetBitplanePointers
.Exit
	movem.l	(sp)+,a0-a6/d0-d7
	lea	_custom,a0
	move.w	#INTF_COPER,intreq(a0)
	move.l	#0,d0
	rts

VBlankIntServer
	;Inputs:	a1.l = Ptr to VBlank counter
	;		a5.l = Ptr to Copper Int Server
	;Outputs:	Z-Flag Set, so system can sort out all other Interrupts in chain.

	add.w	#1,(a1)

ReadMouseRaw
	;hook to call from non-interrupt for init
	lea	_custom,a0

	;Port 0 Mousepos update
	move.w	joy0dat(a0),d0
	move.b	d0,d1
	sub.b	Mouse0RawX,d0
	ext.w	d0
	add.w	d0,Mouse0XPos
	move.b	d1,Mouse0RawX

	move.w	joy0dat(a0),d0
	lsr.w	#8,d0
	move.b	d0,d1
	sub.b	Mouse0RawY,d0
	ext.w	d0
	add.w	d0,Mouse0YPos
	move.b	d1,Mouse0RawY


	;Port 1 Mousepos update
	move.w	joy1dat(a0),d0
	move.b	d0,d1
	sub.b	Mouse1RawX,d0
	ext.w	d0
	add.w	d0,Mouse1XPos
	move.b	d1,Mouse1RawX

	move.w	joy1dat(a0),d0
	lsr.w	#8,d0
	move.b	d0,d1
	sub.b	Mouse1RawY,d0
	ext.w	d0
	add.w	d0,Mouse1YPos
	move.b	d1,Mouse1RawY

	move.l	#0,d0
	rts

SetupCopper
	;Inputs:	NIL
	;Outputs:	NIL
	;It is assumed that the raster dma is off before the slice is setup.
	; 1) Write out the header info (incl. bitplane & sprite ptrs, colours...)
	; 2) Wait for start of next line, and turn the dma on.
	; 3) Wait for the bottom of the slice and turn the dma off.
	; 4) Repeat steps 1-4 for each slice.
	; 5) Write out the CopperInterruptRequest.

	lea	_custom,a6

	move.l	_FirstSlice,a2
	move.l	CopperList1,a3
	move.l	CopperList2,a4
	move.w	#dmacon,(a3)+
	move.w	#DMAF_RASTER,(a3)+			;Turn Off Display DMA
	cmp.l	#0,a4
	beq	.Loop
	move.w	#dmacon,(a4)+
	move.w	#DMAF_RASTER,(a4)+		;Turn Off Display DMA
	move.w	#0,(a4)+
.Loop
	cmp.l	#0,SS_Interlace(a2)
	beq	.JmpToSingleCopper

	move.l	a3,a5
	bsr	.SingleCopper
	move.l	a5,a3
	move.l	a4,a5
	move.l	SS_Interlace(a2),a2
	bsr	.SingleCopper
	move.l	a5,a4

	bra	.GetNext
.JmpToSingleCopper
	move.l	a3,a5
	bsr	.SingleCopper
	move.l	a5,a3
.GetNext
	move.l	SS_Next(a2),a2
	cmp.l	#0,a2
	bne	.Loop

	lea	_LameReq,a2
	tst.l	lr_InterruptCopperWait(a2)
	beq	.IntReqDone
	move.l	lr_InterruptCopperWait(a2),a0
	move.w	cwn_InstructionNum(a0),d0
	lea	cwn_Wait1(a0),a0
	sub.w	#1,d0
.InsertIntReqLoop
	move.l	(a0)+,(a3)+
	dbra	d0,.InsertIntReqLoop
.IntReqDone

	move.l	#$FFFFFFFE,(a3)+			;End copper 1
	cmp.l	#0,a4
	beq	.SetCopper
	move.l	#$FFFFFFFE,(a4)+			;End copper 2
.SetCopper
.Exit
	rts

.SingleCopper
	move.l	a5,SS_CCStart(a2)

	tst.l	SS_CopperHeader(a2)
	beq	.HeaderDone
	move.l	a5,SS_CCHeader(a2)			;Store copper headers place in list
	move.l	SS_CopperHeader(a2),a0
	move.w	SS_CopperHeaderSize(a2),d0
	sub.w	#1,d0
.InsertHeaderLoop
	move.l	(a0)+,(a5)+				;Insert header to list
	dbra	d0,.InsertHeaderLoop
.HeaderDone

	move.l	a5,SS_CCPlanes(a2)
	bsr	BitPlanePointers			;Insert plane pointers to list

	move.l	a5,SS_CCColours(a2)
	bsr	ColourCopy				;Insert colours to list

	move.l	a5,SS_CCSprites(a2)
	bsr	SpritePointers				;Insert sprite pointers to list

	tst.l	SS_CopperDMAOn(a2)
	beq	.DMAOnDone
	move.l	SS_CopperDMAOn(a2),a0
	move.w	cwn_InstructionNum(a0),d0
	lea	cwn_Wait1(a0),a0
	sub.w	#1,d0
.InsertDMAOnLoop
	move.l	(a0)+,(a5)+
	dbra	d0,.InsertDMAOnLoop
.DMAOnDone

	tst.l	SS_CopperDMAOff(a2)
	beq	.DMAOffDone
	move.l	SS_CopperDMAOff(a2),a0
	move.w	cwn_InstructionNum(a0),d0
	lea	cwn_Wait1(a0),a0
	sub.w	#1,d0
.InsertDMAOffLoop
	move.l	(a0)+,(a5)+
	dbra	d0,.InsertDMAOffLoop
.DMAOffDone

.FirstSlice
	move.l	a5,SS_CCEnd(a2)				;Store slice's end location

	rts

ColourCopy
	;Inputs:	a2.l = Slice
	;		a5.l = Running Copper Pointer
	;Outputs:	NIL (sort of, returns updated a5 obviously)

	movem.w	d2-d4,-(sp)

	move.w	#$0C40,d4			;d4 = BPLCON3 value
	tst.l	SS_CopperHeader(a2)		;Get the Header bplcon3 value
	beq	.NoHeader			;if one exists.
	move.l	SS_CopperHeader(a2),a0
	move.w	CH_BplCon3+2(a0),d4
.NoHeader
	and.w	#$1FFF,d4

	move.l	SS_ColourPointers(a2),a1	;a1.l = Ptr To Colour Table
	move.w	SS_Colours(a2),d0		;d0.w = Number Of Colours
	move.w	#color00,d1			;d1.w = Colour Register
	move.w	#32,d3				;d3.w = Bank Switch Counter
	tst.l	_RasterTime
	beq	.NoRasterTime
	 add.l	#3,a1				; Skip Colour Zero
	 sub.w	#1,d0				; Skip Colour Zero
	 add.w	#2,d1				; Skip Colour Zero
	 sub.w	#1,d3				; Skip Colour Zero
.NoRasterTime
	add.w	SS_ColourOffset(a2),d1
.CCLoop2
	move.w	#bplcon3,(a5)+			;Say which colour bank we're dealing with.
	move.w	d4,(a5)+
	add.w	#$2000,d4			;Goto Next Bank
.CCLoop
	move.w	d1,(a5)+			;Put In Colours
	move.b	(a1)+,d2
	lsl.w	#4,d2
	move.b	(a1)+,d2
	lsl.w	#4,d2
	move.b	(a1)+,d2
	lsr.w	#4,d2
	move.w	d2,(a5)+

	add.w	#2,d1				;Goto Next Colour Register
	sub.w	#1,d0				;Take 1 off num of colours
	beq	.LowerNibble			;If zero then we're done
	sub.w	#1,d3				;Take One Off Bank Switch
	bne	.CCLoop
	move.w	#32,d3
	move.w	#color00,d1
	bra	.CCLoop2
.LowerNibble
	move.w	_Flags,d0			;If not AGA palette, then
	btst	#LRB_AGAPALETTE,d0		;just exit.
	beq	.Exit

	and.w	#$1FFF,d4
	or.w	#$0200,d4			;Set LOCT bit (lower nibble set)

	move.l	SS_ColourPointers(a2),a1	;a1.l = Ptr To Colour Table
	move.w	SS_Colours(a2),d0		;d0.w = Number Of Colours
	move.w	#color00,d1			;d1.w = Colour Register
	move.w	#32,d3				;d3.w = Bank Switch Counter
	tst.l	_RasterTime
	beq	.NoRasterTime2
	 add.l	#3,a1				; Skip Colour Zero
	 sub.w	#1,d0				; Skip Colour Zero
	 add.w	#2,d1				; Skip Colour Zero
	 sub.w	#1,d3				; Skip Colour Zero
.NoRasterTime2
	add.w	SS_ColourOffset(a2),d1
.CLLoop2
	move.w	#bplcon3,(a5)+			;Say which colour bank we're dealing with.
	move.w	d4,(a5)+
	add.w	#$2000,d4			;Goto Next Bank
.CLLoop
	move.w	d1,(a5)+			;Put In Colours
	move.b	(a1)+,d2
	lsl.w	#8,d2
	move.b	(a1)+,d2
	lsl.b	#4,d2
	lsl.w	#4,d2
	move.b	(a1)+,d2
	lsl.b	#4,d2
	lsr.w	#4,d2
	move.w	d2,(a5)+

	add.w	#2,d1				;Goto Next Colour Register
	sub.w	#1,d0				;Take 1 off num of colours
	beq	.Exit				;If zero then we're done
	sub.w	#1,d3				;Take One Off Bank Switch
	bne	.CLLoop
	move.w	#32,d3
	move.w	#color00,d1
	bra	.CLLoop2
.Exit
	move.w	#bplcon3,(a5)+			;Back to First Color Bank
	move.w	#$0C40,(a5)+

	movem.w	(sp)+,d2-d4
	rts

BitPlanePointers
	;Inputs:	a2.l = Slice structure
	;		a5.l = Current copper pointer

	move.l	d2,-(sp)

	move.l	SS_PhyBase(a2),d2

	move.w	SS_Planes(a2),d0
	sub.w	#1,d0
	move.w	#bplpt,d1
.Loop
	move.w	d1,(a5)+		;bplXpth
	swap	d2
	move.w	d2,(a5)+

	move.w	d1,(a5)			;bplXptl
	add.w	#2,(a5)+
	swap	d2
	move.w	d2,(a5)+
	add.l	SS_BufferWidth(a2),d2
.GetNext
	add.w	#4,d1
	dbra	d0,.Loop
	move.l	(sp)+,d2
	rts	

SpritePointers
	;Inputs:	a2.l = Slice structure
	;		a5.l = Current copper pointer

	move.l	d2,-(sp)
	move.l	#_NullSprite,d2
	move.w	#8-1,d0
	move.w	#sprpt,d1
.Loop
	move.w	d1,(a5)+		;sprXpth
	swap	d2
	move.w	d2,(a5)+

	move.w	d1,(a5)			;sprXptl
	add.w	#2,(a5)+
	swap	d2
	move.w	d2,(a5)+
.GetNext
	add.w	#4,d1
	dbra	d0,.Loop
	move.l	(sp)+,d2
	rts	

_ScreenSwap
	;Inputs:	a2.l = Slice Structure

	move.l	SS_PhyBase(a2),d0
	move.l	SS_LogBase(a2),SS_PhyBase(a2)
	move.l	d0,SS_LogBase(a2)

	eor.w	#1,SS_ScreenToggle(a2)
	eor.w	#1,_SpriteCopperToggle

	rts

;*******************************************************************************************************
;Slice Entrace routines

NegativeModulo
	;Inputs:	a2.l = Slice to preform entrance


	move.l	SS_CCFooter(a2),a3
	sub.l	#8,a3

	move.l	SS_CopperHeader(a2),a0

	move.w	CH_Bpl1Mod+2(a0),d0
	neg.w	d0
	move.w	#bpl1mod,(a3)+
	move.w	#-46,(a3)+

	move.w	CH_Bpl2Mod+2(a0),d0
	neg.w	d0
	move.w	#bpl2mod,(a3)+
	move.w	#-46,(a3)+

	sub.l	#8,a3

	move.l	#$3001FF00,d2
.Loop
	bsr	_WaitVBlank
	bsr	_WaitPastVBlank
	move.l	d2,(a3)

	add.l	#$01000000,d2
	cmp.l	#$F001FF00,d2
	bls	.Loop
	rts

;*****************************************************************************************************
;Input stuff


_ReadVirtualJoystick
	;inputs:	d0.w = Joystick number 0..3
	;output:	d0.b = switch states

	lsl.w	#2,d0
	lea	VirtualJoystickPtrs,a0
	move.l	(a0,d0.w),a0
	move.w	VJ_PrevState(a0),d0	;(VJ_State in low byte)
	rts


UpdateInputChannels
	movem.l	d2-d3/a2-a3,-(sp)

	bsr	UpdateMice

	;JOYSTICKS

	lea	VirtualJoystickPtrs,a2
	move.w	#4-1,d2			;d2 = loop counter
.Loop
	move.l	(a2)+,a3

	;save old state
	move.b	VJ_State(a3),d0
	move.b	d0,VJ_PrevState(a3)

	move.b	#0,d3			;d3 = switch state

	cmp.w	#VJTYPE_VIRTUAL,VJ_Type(a3)
	beq	.CheckKeys

	move.w	VJ_HWPort(a3),d0
	bsr	_JoystickDecode		;read the hardware joystick
	move.b	d0,d3

	cmp.w	#VJTYPE_BUTTONSONLY,VJ_Type(a3)
	bne	.CheckKeys
	and.w	#$00F0,d3		;mask out direction bits

.CheckKeys
	lea	_RawKeyArray,a0
	lea	VJ_RawKeys(a3),a1

	move.w	#0,d0	;d0 = bitnum
.KeyLoop
	move.b	(a1)+,d1
	cmp.w	#$FF,d1		;no defined key?
	beq	.NextKey
	and.w	#$FF,d1
	tst.b	(a0,d1.w)	;read keystate
	beq	.NextKey
	bset	d0,d3		;yep, keydown
.NextKey
	add.w	#1,d0
	cmp.w	#8,d0
	bne	.KeyLoop

	move.b	d3,d0
	move.b	VJ_PrevState(a3),d1
	bsr	_JoystickMutualExclude
	move.b	d0,VJ_State(a3)	;store new virtual joystick state
	dbra	d2,.Loop

	movem.l	(sp)+,d2-d3/a2-a3
	rts



UpdateMice
	;MICE

	;Update port 0 mouse

	lea	VirtualMouse0,a0
	move.b	VM_ButtonState(a0),d0
	move.b	d0,VM_PrevButtonState(a0)

	move.w	Mouse0XPos,d0
	move.w	d0,d1
	sub.w	VM_XPos(a0),d0
	move.w	d0,VM_DeltaX(a0)
	move.w	d1,VM_XPos(a0)

	move.w	Mouse0YPos,d0
	move.w	d0,d1
	sub.w	VM_YPos(a0),d0
	move.w	d0,VM_DeltaY(a0)
	move.w	d1,VM_YPos(a0)

	;Update port 1 mouse
	lea	VirtualMouse1,a0
	move.b	VM_ButtonState(a0),d0
	move.b	d0,VM_PrevButtonState(a0)

	move.w	Mouse1XPos,d0
	move.w	d0,d1
	sub.w	VM_XPos(a0),d0
	move.w	d0,VM_DeltaX(a0)
	move.w	d1,VM_XPos(a0)

	move.w	Mouse1YPos,d0
	move.w	d0,d1
	sub.w	VM_YPos(a0),d0
	move.w	d0,VM_DeltaY(a0)
	move.w	d1,VM_YPos(a0)

	;read fire buttons

	move.b	#0,d1		;d1 = buttonstate

	lea	_ciaa,a1
	move.b	ciapra(a1),d0
	;check lb0
	btst	#6,d0
	bne	.checklb1
	bset	#4-4,d1
.checklb1
	btst	#7,d0
	bne	.checkmb0
	bset	#4,d1

.checkmb0
	lea	_custom,a1
	move.w	#$FF00,potgo(a1)	;configure for button input
	move.w	potinp(a1),d0
	btst	#8,d0
	bne	.checkmb1
	bset	#6-4,d1
.checkmb1
	btst	#12,d0
	bne	.checkrb0
	bset	#6,d1
.checkrb0
	btst	#10,d0
	bne	.checkrb1
	bset	#5-4,d1
.checkrb1
	btst	#14,d0
	bne	.checkdone
	bset	#5,d1
.checkdone

;	and.b	#%00000100,d1

	lea	VirtualMouse0,a0
	move.b	d1,d0
	lsl.b	#4,d0
	move.b	d0,VM_ButtonState(a0)

	lea	VirtualMouse1,a0
	and.b	#%11110000,d1
	move.b	d1,VM_ButtonState(a0)

	rts



_JoystickMutualExclude

	;in:	d0.b = freshly read new state
	;	d1.b = prev state
	;out:	d0.b = new state with conflicts sussed.

	move.w	d2,-(sp)
	move.b	d0,d2

	;Check Up/Down
	and.b	#%00000011,d0	;extract up and down
	cmp.b	#%00000011,d0	;_both_ pressed?
	bne	.CheckLeftRight
	and.b	#%11111100,d2	;clear up/down
	move.b	d1,d0		;get prevstate
	and.b	#%00000011,d0	;extract prev up/down
	or.b	d0,d2		;insert into new state

.CheckLeftRight
	move.b	d2,d0
	and.b	#%00001100,d0	;extract left and right
	cmp.b	#%00001100,d0	;_both_ pressed?
	bne	.Done
	and.b	#%11110011,d2	;clear left/right
	move.b	d1,d0		;get prevstate
	and.b	#%00001100,d0	;extract prev left/right
	or.b	d0,d2		;insert into new state
.Done
	move.b	d2,d0		;return in d0.b

	move.w	(sp)+,d2
	rts

_stricmp	
	;Inputs:	a0.l = First Sting
	;		a1.l = Second String
	;Returns:	d0.w =	0 - Same
	;			1 - Different
	move.w	d2,-(sp)
	move.w	#0,d2
.Next
	move.b	(a0)+,d0
	move.b	(a1)+,d1
	bne	.NotZero
	tst.b	d0
	beq	.Exit
	move.w	#1,d2
	bra	.Exit
.NotZero
	and.b	#%11011111,d0
	and.b	#%11011111,d1
	cmp.b	d0,d1
	beq	.Next
	move.w	#1,d2
.Exit
	move.w	d2,d0
	move.w	(sp)+,d2
	rts

_strcpy
	;Inputs:	a0.l = Dest String
	;		a1.l = Source String
	;Outputs:	NIL

.Loop
	move.b	(a1)+,d0
	move.b	d0,(a0)+
	bne	.Loop
.	rts

;*****************************************************************************************************
;Slice update

SetupPlayers
	;Inputs:	a2.l = Slice
	MYKPRINTF "DoPlayers",#0

	move.l	a4,-(sp)
	lea	PlayerWeaponBanks,a4

	move.w	SS_NumOfPlayers(a2),d2
	beq	.Exit
	sub.w	#1,d2
.Loop
	bsr	_FindFreePlayerDude
	cmp.l	#0,a0
	beq	.Exit2
	move.l	a0,a3

	jsr	_NotINITBOBAlthoughFairlySimilar

	;XYZZY - tidy up please
	move.w	#1,OB_State(a3)
	move.w	#OB_TYPE_PLAYER,OB_Type(a3)
	move.l	a4,BB_WeaponBank(a3)
	add.l	#WEAPONBAYS*4,a4
	move.l	#$0,OB_Score(a3)
	move.l	#$0,OB_Score+4(a3)

	move.l	BB_Data1(a3),a5
	cmp.l	#0,a5
	beq	.Exit

	move.w	#CONTEXT_BOB,d0
	bsr	_DoActionList

	dbra	d2,.Loop	
.Exit
	move.l	SS_Players(a2),d0
	MYKPRINTF "PlayersDone-First:%lx",d0
	move.l	(sp)+,a4
	rts
.Exit2
	MYKPRINTF "NoPlayers",#0
	move.l	(sp)+,a4
	rts


SetupSlices
	;Inputs:	NIL
	;Outputs:	NIL

	bsr	SetupCopper

	move.l	_FirstSlice,a2
.Loop
	bsr	SetupPlayers

	move.l	SS_Channel0(a2),a5
	move.l	#0,SS_Channel0(a2)

	cmp.l	#0,a5
	beq	.NoAction

	move.w	#CONTEXT_NONE,d0
	bsr	_DoActionList
.NoAction
	move.l	#UpdateObjects,SS_Channel1(a2)
	;turn bullet-dude and player-dude collisions on
	move.w	#3,SS_CollisionTypes(a2)

.GetNext
	;move.l	SS_Next(a2),a2
	;cmp.l	#0,a2
	;bne	.Loop
.Exit

	rts

;XYZZY - Sort through all this crap sometimes... PLEASE!

UpdateSlices
	move.l	_FirstSlice,a2
.Loop
	btst	#1,SS_Flags(a2)
	bne	.TimerStopped
	add.l	#1,SS_Timer(a2)
.TimerStopped

.CheckGlobalEvent
	bsr	UpdateGlobalEvents

	tst.l	SS_Channel1(a2)
	beq	.DoneChannel1
	move.l	SS_Channel1(a2),a0
	jsr	(a0)				;update objects
.DoneChannel1

	RCOLOR	#$00F

	tst.l	SS_Channel2(a2)
	beq	.DoneChannel2
	move.l	SS_Channel2(a2),a0
	jsr	(a0)
.DoneChannel2
.Tracking
	move.l	SS_Scroll(a2),a3
	move.l	SC_XTrackRoutine(a3),a0
	cmp.l	#0,a0
	beq	.CheckYTrack
	jsr	(a0)
.CheckYTrack
	;move.l	SS_Scroll(a2),a3
	;move.l	SC_YTrackRoutine(a3),a0
	;cmp.l	#0,a0
	;beq	.EnergyBar
	;jsr	(a0)

	tst.l	SS_Channel0(a2)
	beq	.DoneChannel0
	RCOLOR	#$F00
	move.l	SS_Channel0(a2),a0
	jsr	(a0)
.DoneChannel0

.ShtatShtuff

	lea	_LameReq,a0
	move.l	lr_LevelUpdateProg(a0),d0
	beq	.Exit
	move.l	d0,a5
	move.w	#CONTEXT_NONE,d0
	bsr	_DoActionList
;.OoohThatWasLuckyTheGroundBrokeMyFall

.Exit
	rts

UpdateGlobalEvents
	tst.w	SS_Event(a2)
	beq	.Exit

	move.w	#0,d4
	move.l	SS_Event(a2),d3
.Loop
	lsr.w	d3
	bcc	.Next

	lea	GE_Table,a0
	lsl.w	#2,d4
	move.l	(a0,d4.w),d0
	lsr.w	#2,d4
	move.l	SS_GlobalEvents(a2),a0
	cmp.l	#0,a0
	beq	.Exit
	add.l	d0,a0

	move.l	a5,-(sp)
	move.l	GE_ActionList(a0),a5
	move.l	GE_Data(a0),a3

	cmp.l	#0,a5
	beq	.NoActionList

	move.w	#CONTEXT_NONE,d0
	bsr	_DoActionList
.NoActionList
	move.l	(sp)+,a5
.Next
	add.w	#1,d4
	cmp.w	#31,d4
	bne	.Loop

.UpdateInternalEvents
	bra	.Exit

	move.l	SS_GlobalEvents(a2),a0
	move.l	SS_GEUpdateFlags(a2),d0
	lsr.l	d0
	bcc	.NoTimerEvent

	move.l	GE_Data(a0),d1
	sub.l	#0,d1
	bne	.NoTimerEvent

	move.l	#GEVENT_TIMER,SS_Event(a2)
.NoTimerEvent
	lsr.l	d0
	bcc	.NoScrollPosEvent

	add.l	#GE_SizeOf,a0
	move.l	GE_Data(a0),d1

	move.l	SS_Scroll(a2),a1
	cmp.l	SC_Pos(a1),d1
	blt	.NoScrollPosEvent	

	move.l	#GEVENT_SCROLLPOS,SS_Event(a2)
.NoScrollPosEvent
.Exit
	rts



SliceCopperUpdate
	movem.l	a2-a4/d2,-(sp)

	move.l	_FirstSlice,a2
.Loop
	move.l	SS_Scroll(a2),a3
	cmp.l	#0,a3
	beq	.GetNext
	bsr	CantThinkOfAName
	
	cmp.l	#0,SS_Interlace(a2)
	beq	.GetNext
	move.l	SS_Interlace(a2),a2
	bsr	CantThinkOfAName
.GetNext

	;move.l	SS_Next(a2),a2
	;cmp.l	#0,a2
	;bne	.Loop
.Exit
	movem.l	(sp)+,a2-a4/d2
	rts
CantThinkOfAName
	bsr	_ScreenSwap			;Swap Screens

	move.l	SC_Pos(a3),d0
	lsr.l	#BITSHIFT-1+4,d0
	and.l	#$FFFFFFFE,d0

	move.w	_Flags,d1
	and.w	#$1,d1
	beq	.NoBurst1

	and.l	#$FFFFFFF8,d0
	
.NoBurst1
	move.l	SC_YPos(a3),d1
	asr.l	#BITSHIFT,d1
	mulu	SS_BufferWidthxPlanes(a2),d1
	add.l	d1,d0

	move.l	SS_PhyBase(a2),a0
	add.l	a0,d0

	move.l	SS_CCPlanes(a2),a1
	move.l	SS_BufferWidth(a2),d1
	move.w	SS_Planes(a2),d2
	sub.w	#1,d2
.FeedCopperLoop					;Feed Phy Plane pointers
	move.w	d0,6(a1)
	swap	d0
	move.w	d0,2(a1)
	swap	d0
	add.l	d1,d0
	add.l	#8,a1
	dbra	d2,.FeedCopperLoop


	move.l	SS_CCHeader(a2),a4		;Hires Scroll
	move.w	#0,CH_BplCon1+2(a4)

	move.l	SC_Pos(a3),d0

	lsr.l	#BITSHIFT,d0
	move.l	SS_DisplayID(a2),d1
	and.l	#$00008000,d1
	beq	.NoHires

	btst.l	#0,d0
	bne	.NoOdd

	move.l	SS_CCHeader(a2),a4		;Hires Scroll
	bset	#1,CH_BplCon1+2(a4)
	bset	#5,CH_BplCon1+2(a4)
.NoOdd
	lsr.l	d0
.NoHires

	move.w	_Flags,d1
	and.w	#$1,d1
	beq	.NoBurst

	move.l	d0,d1
	and.w	#$F,d0
	and.w	#$30,d1
	eor.w	#$30,d1
	lsl.w	#6,d1

	move.l	SS_CCHeader(a2),a4
	or.w	d1,CH_BplCon1+2(a4)
	lsl.w	#4,d1
	or.w	d1,CH_BplCon1+2(a4)

.NoBurst
	and.w	#$F,d0
	eor.w	#$F,d0

	move.w	d0,d1
	lsl.w	#4,d1
	or.w	d1,d0
	and.w	SC_CopperScrollANDBits(a3),d0	;Set smooth scroll pos

	move.l	SS_CCHeader(a2),a4
	move.b	d0,CH_BplCon1+3(a4)

.Exit
	rts


;******************************************************************************************************
;Wipes

	;Each wipe can have an init prog and an update prog.
	;Return non-zero in d0.w if finished otherwise return zero in d0.w

DoIntroWipe
	;Initialises intro wipe & then displays game copper before calling the
	;Wipe update routines

;	bsr	InitIntroWipes

	lea	_custom,a6
	move.l	CopperList1,cop1lc(a6)		;After All setup done we
	bsr	_WaitVBlank			;display game screen
	bsr	_WaitPastVBlank


;	bsr	UpdateWipes
	rts

DoExitWipe
	;Performs the init & update for th exit wipes then displays a blank screen

;	bsr	InitExitWipes

;	bsr	UpdateWipes

	lea	_custom,a6
	move.l	#_ALittleCopperList,cop1lc(a6)
	bsr	_WaitVBlank
	bsr	_WaitPastVBlank

	rts

InitIntroWipes
	move.l	_FirstSlice,a2
	move.w	SS_Colours(a2),d0
	sub.w	#1,d0
	move.l	SS_CCColours(a2),a0
	add.l	#4,a0		;Skip over bplcon3 instruction
	lea	APalette,a1
.Loop
	move.w	2(a0),(a1)+	;Store colour value
	move.w	#0,2(a0)	;Zero copper colour
	add.l	#4,a0		;Point to next colour

	dbra	d0,.Loop

	move.l	_FirstSlice,a2
.IW_Loop
	move.w	SS_IntroWipe_Speed(a2),SS_CurrentWipe_Speed(a2)
	move.l	SS_IntroWipe_Data1(a2),SS_CurrentWipe_Data1(a2)
	move.l	SS_IntroWipe_Data2(a2),SS_CurrentWipe_Data2(a2)

	move.w	#40,SS_CurrentWipe_Speed(a2)
	move.l	#BlackPalette,SS_CurrentWipe_Data1(a2)
	move.l	#APalette,SS_CurrentWipe_Data2(a2)

	move.w	SS_IntroWipe(a2),d0
	move.w	#1,d0
	move.w	d0,SS_CurrentWipe(a2)

	lsl.w	#2,d0
	lea	InitWipeTable,a0
	move.l	(a0,d0.w),a0
	cmp.l	#0,a0
	beq	.IW_GetNext
	jsr	(a0)
.IW_GetNext
	move.l	SS_Next(a2),a2
	cmp.l	#0,a2
	;bne	.IW_Loop
.IW_Exit
	rts

InitExitWipes
	move.l	_FirstSlice,a2
.IW_Loop
	move.w	SS_ExitWipe_Speed(a2),SS_CurrentWipe_Speed(a2)
	move.w	#75,SS_CurrentWipe_Speed(a2)
	move.l	SS_ExitWipe_Data1(a2),SS_CurrentWipe_Data1(a2)
	move.l	#APalette,SS_CurrentWipe_Data1(a2)
	move.l	SS_ExitWipe_Data2(a2),SS_CurrentWipe_Data2(a2)
	move.l	#BlackPalette,SS_CurrentWipe_Data2(a2)

	move.w	SS_ExitWipe(a2),d0
	move.w	#1,d0
	move.w	d0,SS_CurrentWipe(a2)

	lsl.w	#2,d0
	lea	InitWipeTable,a0
	move.l	(a0,d0.w),a0
	cmp.l	#0,a0
	beq	.IW_GetNext
	jsr	(a0)
.IW_GetNext
	move.l	SS_Next(a2),a2
	cmp.l	#0,a2
	;bne	.IW_Loop
.IW_Exit
	rts

UpdateWipes

.UW_MainLoop
	bsr	_WaitVBlank
	bsr	_WaitPastVBlank

	move.l	_FirstSlice,a2
	move.w	#1,d0			;exit code if no routines are called
.UW_Loop
	move.w	SS_CurrentWipe(a2),d1
	lsl.w	#2,d1
	lea	UpdateWipeTable,a0
	move.l	(a0,d1.w),a0
	cmp.l	#0,a0
	beq	.UW_GetNext
	jsr	(a0)
	tst.w	d0
	beq	.UW_GetNext
	move.w	#0,SS_CurrentWipe(a2)
.UW_GetNext
	move.l	SS_Next(a2),a2
	cmp.l	#0,a2
	;bne	.UW_Loop

	tst.w	d0
	beq	.UW_MainLoop
.UW_Exit
	rts

;****************************************

Init_ColourFade
	;Sets up colour fade. SS_CurrentWipe_Data1 & Data2 hold pointers to colour lists
	;Inputs:	NULL
	;Outputs:	NULL

	lea	FadeStruct,a0
	move.w	SS_Colours(a2),FD_ColourCount(a0)
	move.l	SS_CurrentWipe_Data1(a2),FD_SourceList(a0)
	move.l	SS_CurrentWipe_Data2(a2),FD_TargetList(a0)
	move.l	SS_CCColours(a2),FD_CopperList(a0)
	add.l	#4,FD_CopperList(a0)

	move.l	a2,-(sp)
	move.l	a0,a2
	move.w	#20,d0
	bsr	InitFadeStructure
	move.l	(sp)+,a2
.Exit
	rts

Update_ColourFade
	;Inputs:	NULL
	;Outputs:	d0.w - AllOk=0, Finished=-1

	move.l	a2,-(sp)
	lea	FadeStruct,a2
	bsr	UpdateFade
	move.l	(sp)+,a2

.Exit
	rts



InitFadeStructure
	;sets up the FD_RedDeltas,FD_GrnDeltas,FD_BluDeltas,FD_Red,FD_Grn
	;and FD_Blu arrays in a FadeData structure
	;also sets up the FD_Timer field.

	;inputs:
	;a2 = initialised FadeData structure. initial colours are
	;     taken from the values in the SourceList
	;d0 = duration of fade (in cycles. 0=instant transition)
	;outputs:
	;none (except a2, the FadeData struct)

	movem.l	d2-d5/a3,-(sp)

	tst.w	d0
	beq	.Done

	move.w	d0,d4
	move.w	d0,FD_Timer(a2)

	move.l	#2,FD_SourceMod(a2)
	move.l	#2,FD_TargetMod(a2)

	move.l	FD_SourceList(a2),a0
	cmp.l	#0,a0
	beq	.Done
	move.l	FD_TargetList(a2),a1
	cmp.l	#0,a1
	beq	.Done
	move.l	a2,a3			;use a3 as array counter

	move.w	#0,d5			;counter

.Loop
	cmp.w	FD_ColourCount(a2),d5	;done em all?
	bhs	.Done			;yep, exit

	move.w	(a0),d2			;get RGB value from sourcelist
	add.l	FD_SourceMod(a2),a0
	move.w	(a1),d3			;get RGB target value
	add.l	FD_TargetMod(a2),a1

	;work out red delta
	;all working done in 8.8 format
	move.w	d2,d0			;get inital RGB value
	and.w	#$0F00,d0		;extract red component
	move.w	d0,FD_Red(a3)		;store in red table
	move.w	d3,d1			;get target RGB value
	and.w	#$0F00,d1		;extract red component
	sub.w	d0,d1			;calc total change
	ext.l	d1
	divs	d4,d1			;and divide by the duration
	move.w	d1,FD_RedDeltas(a3)	;store it in table

	;work out green delta
	;all working done in 8.8 format
	move.w	d2,d0			;get inital RGB value
	and.w	#$00F0,d0		;extract green component
	lsl.w	#4,d0			;go to 8.8 format
	move.w	d0,FD_Grn(a3)		;store in green table
	move.w	d3,d1			;get target RGB value
	and.w	#$00F0,d1		;extract green component
	lsl.w	#4,d1			;go to 8.8 format
	sub.w	d0,d1			;calc total change
	ext.l	d1
	divs	d4,d1			;and divide by the duration
	move.w	d1,FD_GrnDeltas(a3)	;store it in table

	;work out blue delta
	;all working done in 8.8 format
	move.w	d2,d0			;get inital RGB value
	and.w	#$000F,d0		;extract blue component
	lsl.w	#8,d0			;go to 8.8 format
	move.w	d0,FD_Blu(a3)		;store in blue table
	move.w	d3,d1			;get target RGB value
	and.w	#$000F,d1		;extract blue component
	lsl.w	#8,d1			;go to 8.8 format
	sub.w	d0,d1			;calc total change
	ext.l	d1
	divs	d4,d1			;and divide by the duration
	move.w	d1,FD_BluDeltas(a3)	;store it in table

	add.l	#2,a3			;update array pointer
	add.w	#1,d5			;next colour
	bra	.Loop			;nope, loop

.Done
	movem.l	(sp)+,d2-d5/a3

	rts



UpdateFade
	;uses the data in a FadeData structure to alter the colours in a
	;copperlist

	;inputs:
	;a2 = FadeData structure
	;outputs:
	;d0.w = fade status (-1=finished, 0=still going)

	tst.w	FD_Timer(a2)		;check timer
	beq	.ForceEm		;zero means finish fade
	bmi	.Finished		;-1 means already finished

	sub.w	#1,FD_Timer(a2)

	move.l	a2,a3			;use a3 as a moving pointer
	move.l	FD_CopperList(a2),a0
	move.l	FD_TargetList(a2),a1
	move.w	#0,d2			;counter

.Loop
	cmp.w	FD_ColourCount(a2),d2	;done enuff colours?
	bhs	.Done			;if yep, exit

	move.w	FD_Red(a3),d0		;get 8.8 red component
	add.w	FD_RedDeltas(a3),d0	;add red-delta
	move.w	d0,FD_Red(a3)		;store 8.8 red component

	move.w	d0,d1			;d1 holds running RGB total
	and.w	#$0F00,d1		;mask out crap

	move.w	FD_Grn(a3),d0		;update green
	add.w	FD_GrnDeltas(a3),d0
	move.w	d0,FD_Grn(a3)

	lsr.w	#4,d0			;move to correct position
	and.w	#$00F0,d0		;mask crap
	or.w	d0,d1			;add to running RGB total

	move.w	FD_Blu(a3),d0		;update blue
	add.w	FD_BluDeltas(a3),d0
	move.w	d0,FD_Blu(a3)

	lsr.w	#8,d0
	and.w	#$000F,d0
	or.w	d0,d1			;into running total

	add.l	#2,a3			;update array pointers

	move.w	d1,2(a0)		;put into copperlist
	add.l	#4,a0			;point to next place in copperlist

	add.w	#1,d2			;next colour
	bra	.Loop

.ForceEm
	;fade done - copy the target colours into the copperlist
	;so we're absolutely sure that the fade finishes at exactly the
	;right place. Otherwise we could get nasty tint-effects.

	move.l	FD_TargetList(a2),a0
	move.l	FD_CopperList(a2),a1
	move.w	FD_ColourCount(a2),d0
.ForceLoop
	tst.w	d0			;done em all?
	beq	.Finished		;yep, finished
	move.w	(a0),2(a1)		;copy one
	add.l	FD_TargetMod(a2),a0
	add.l	#4,a1			;skip MOVE instruction
	sub.w	#1,d0			;dec cocunter
	bra	.ForceLoop		;loop

.Finished
	move.w	#-1,d0			;return -1 to indicate fade done
	move.w	d0,FD_Timer(a2)		;set timer, just to be sure
	bra	.Exit
.Done
	move.w	#0,d0			;not finished - return 0
.Exit
	rts










;******************************************************************************************************

	include "Source/BobStuff.s"

;******************************************************************************************************
	SECTION	Vars,BSS

BSSSectionStart		;Zero The Area Between BSSSectionStart & BSSSectionEnd

FullSpeedAhead	ds.w	1	;1 = No VBlank Wait

_BCOldImage	ds.l	1
_BCOldAnimX	ds.l	1
_BCOldAnimY	ds.l	1

_CopperIntPtr	ds.l	1

;CopperSize	ds.l	1
;CopperMem	ds.l	1
CopperList1	ds.l	1
CopperList2	ds.l	1

OldCopState	ds.w	1
OldIntVector	ds.l	1
OldSp		ds.l	1

MainLoopDone	ds.w	1
VBlankCounter	ds.w	1

Interrupts	ds.w	1

_CopperInterruptStruct	ds.b	IS_SIZE
		even
VBlankInterruptStruct	ds.b	IS_SIZE
		even

RapFlag		ds.w	1

KeyList		ds.b	256

PlayerWeaponBanks
		ds.l	MAXPLAYERS*WEAPONBAYS
PlayerMen	ds.w	MAXPLAYERS

GlobalEvents	ds.b	32*GE_SizeOf

UpdatingWaves	ds.b	6*UPDATINGWAVES		;Dudes, Action

_GameOn		ds.w	1


;Set by _DoActionList - the context for the currently executing actionlist
;Read-only from all other routines.
_ActionContext	ds.w	1

_recursiveactionlistaddress
		ds.l	1

Mode		ds.w	1
_Framerate	ds.w	1
_FirstSlice	ds.l	1

OldActionStack	ds.l	20

_ObjectStructures ds.l	1

;updated by vblank server
Mouse0RawX	ds.w	1
Mouse0XPos	ds.w	1
Mouse0RawY	ds.w	1
Mouse0YPos	ds.w	1
Mouse1RawX	ds.w	1
Mouse1XPos	ds.w	1
Mouse1RawY	ds.w	1
Mouse1YPos	ds.w	1

BSSSectionEnd


BlackPalette	ds.w	256
APalette	ds.w	256
_GameClock	ds.l	1
_LevelClock	ds.l	1

FadeStruct	ds.b	FD_SIZEOF

;*****************************************************************************************************
	SECTION Data,data

	;CB_ stuff from CheckBorders routine in BobStuff.s

CB_PastRightRoutines
	dc.l	0,_OBJECTOFF,CB_XStop,CB_XWrap,CB_XBounce1,CB_XBounce2
CB_PastLeftRoutines
	dc.l	0,_OBJECTOFF,CB_XStop,CB_XWrap,CB_XBounce1,CB_XBounce2
CB_PastTopRoutines
	dc.l	0,_OBJECTOFF,CB_YStop,CB_YWrap,CB_YBounce1,CB_YBounce2
CB_PastBottomRoutines
	dc.l	0,_OBJECTOFF,CB_YStop,CB_YWrap,CB_YBounce1,CB_YBounce2




OldAction	dc.l	OldActionStack

ReturnChar	dc.b	10,0

	;XYZZY huh?
GE_Table	dc.l	00*GE_SizeOf,01*GE_SizeOf,02*GE_SizeOf,03*GE_SizeOf
		dc.l	04*GE_SizeOf,05*GE_SizeOf,06*GE_SizeOf,07*GE_SizeOf
		dc.l	08*GE_SizeOf,09*GE_SizeOf,10*GE_SizeOf,11*GE_SizeOf
		dc.l	12*GE_SizeOf,13*GE_SizeOf,14*GE_SizeOf,15*GE_SizeOf
		dc.l	16*GE_SizeOf,17*GE_SizeOf,18*GE_SizeOf,19*GE_SizeOf
		dc.l	20*GE_SizeOf,21*GE_SizeOf,22*GE_SizeOf,23*GE_SizeOf
		dc.l	24*GE_SizeOf,25*GE_SizeOf,26*GE_SizeOf,27*GE_SizeOf
		dc.l	28*GE_SizeOf,29*GE_SizeOf,30*GE_SizeOf,31*GE_SizeOf

	;XYZZY - obsolete? Needs rewriting?
;TimedWaves	dc.w	0		;Type
;		dc.l	0,300<<BITSHIFT,100<<BITSHIFT	;Wave,XPos,YPos
;		dc.w	1,20				;ViewPort,time
;		dc.l	0,0,0,0,0

	;XYZZY
RandomWaves
		dc.w	-1		;HType
		dc.w	60,60		;HData
		dc.w	1		;HAnd

		dc.w	1		;Type
		dc.l	0,170<<BITSHIFT,0<<BITSHIFT	;Wave,XPos,YPos
		dc.w	1				;ViewPort

		dc.w	1		;Type
		dc.l	0,170<<BITSHIFT,90<<BITSHIFT	;Wave,XPos,YPos
		dc.w	1				;ViewPort


CopperIntName	dc.b	"Mr Flibble",0
		even
VBlankIntName	dc.b	"The Boys From the Dwarf",0
		even
_Flags		dc.w	$FFFF

IntroWipe	dc.w	0
UpdateWipeTable	dc.l	0,		Update_ColourFade
InitWipeTable	dc.l	0,		Init_ColourFade
;WipeArgTable	dc.l	NullArgs,	ColourFadeArgs

;NullArgs	dc.w	TEMARG_NULL,	TEMARG_NULL
;ColourFadeArgs	dc.w	TEMARG_PALETTE,	TEMARG_PALETTE

;FadeStruct
;		dc.w	0		;FD_ColourCount
;		dc.w	0		;FD_Timer
;		dc.l	0		;FD_SourceList
;		dc.l	2		;FD_SourceMod
;		dc.l	0		;FD_TargetList
;		dc.l	2		;FD_TargetMod
;		dc.l	0		;FD_CopperList
;		ds.w	256		;FD_Red
;		ds.w	256		;FD_Grn
;		ds.w	256		;FD_Blu
;		ds.w	256		;FD_RedDeltas
;		ds.w	256		;FD_BluDeltas
;		ds.w	256		;FD_GrnDeltas


;BlackPalette	dc.w	$0000,$0000,$0000,$0000,$0000,$0000,$0000,$0000,$0000,$0000,$0000,$0000,$0000,$0000,$0000,$0000,$0000,$0000,$0000,$0000
;		dc.w	$0000,$0000,$0000,$0000,$0000,$0000,$0000,$0000,$0000,$0000,$0000,$0000,$0000,$0000,$0000,$0000,$0000,$0000,$0000,$0000
;		dc.w	$0000,$0000,$0000,$0000,$0000,$0000,$0000,$0000,$0000,$0000,$0000,$0000,$0000,$0000,$0000,$0000,$0000,$0000,$0000,$0000
;		dc.w	$0000,$0000,$0000,$0000,$0000,$0000,$0000,$0000,$0000,$0000,$0000,$0000,$0000,$0000,$0000,$0000,$0000,$0000,$0000,$0000

;		dc.w	$0000,$0000,$0000,$0000,$0000,$0000,$0000,$0000,$0000,$0000,$0000,$0000,$0000,$0000,$0000,$0000,$0000,$0000,$0000,$0000
;		dc.w	$0000,$0000,$0000,$0000,$0000,$0000,$0000,$0000,$0000,$0000,$0000,$0000,$0000,$0000,$0000,$0000,$0000,$0000,$0000,$0000
;		dc.w	$0000,$0000,$0000,$0000,$0000,$0000,$0000,$0000,$0000,$0000,$0000,$0000,$0000,$0000,$0000,$0000,$0000,$0000,$0000,$0000
;		dc.w	$0000,$0000,$0000,$0000,$0000,$0000,$0000,$0000,$0000,$0000,$0000,$0000,$0000,$0000,$0000,$0000,$0000,$0000,$0000,$0000

;		dc.w	$0000,$0000,$0000,$0000,$0000,$0000,$0000,$0000,$0000,$0000,$0000,$0000,$0000,$0000,$0000,$0000,$0000,$0000,$0000,$0000
;		dc.w	$0000,$0000,$0000,$0000,$0000,$0000,$0000,$0000,$0000,$0000,$0000,$0000,$0000,$0000,$0000,$0000,$0000,$0000,$0000,$0000
;		dc.w	$0000,$0000,$0000,$0000,$0000,$0000,$0000,$0000,$0000,$0000,$0000,$0000,$0000,$0000,$0000,$0000,$0000,$0000,$0000,$0000
;		dc.w	$0000,$0000,$0000,$0000,$0000,$0000,$0000,$0000,$0000,$0000,$0000,$0000,$0000,$0000,$0000,$0000,$0000,$0000,$0000,$0000

;		dc.w	$0000,$0000,$0000,$0000,$0000,$0000,$0000,$0000,$0000,$0000,$0000,$0000,$0000,$0000,$0000,$0000,$0000,$0000,$0000,$0000
;		dc.w	$0000,$0000,$0000,$0000,$0000,$0000,$0000,$0000,$0000,$0000,$0000,$0000,$0000,$0000,$0000,$0000,$0000,$0000,$0000,$0000
;		dc.w	$0000,$0000,$0000,$0000,$0000,$0000,$0000,$0000,$0000,$0000,$0000,$0000,$0000,$0000,$0000,$0000,$0000,$0000,$0000,$0000
;		dc.w	$0000,$0000,$0000,$0000,$0000,$0000,$0000,$0000,$0000,$0000,$0000,$0000,$0000,$0000,$0000,$0000,$0000,$0000,$0000,$0000


;**********************************************************************************************
;Object stuctures

VirtualJoystickPtrs
		dc.l	vj0,vj1,vj2,vj3,0

_VirtualJoysticks
vj0		dc.b	0,0
		dc.w	VJTYPE_STDJOYSTICK,1
		dc.b	Key_CrsrUp,Key_CrsrDown,Key_CrsrLeft,Key_CrsrRight,Key_RAlt,255,255,255

vj1		dc.b	0,0
		dc.w	VJTYPE_VIRTUAL,0
		dc.b	Key_A,Key_Z,Key_O,Key_P,Key_Space,255,255,255

vj2		dc.b	0,0
		dc.w	VJTYPE_VIRTUAL,0
		dc.b	255,255,255,255,255,255,255,255

vj3		dc.b	0,0
		dc.w	VJTYPE_VIRTUAL,0
		dc.b	255,255,255,255,255,255,255,255


_VirtualMice
		dc.l	VirtualMouse0,VirtualMouse1,0


VirtualMouse0
		ds.b	VM_SIZEOF
VirtualMouse1
		ds.b	VM_SIZEOF


;XYZZY - obsolete?
;EnergyBar
;		dc.w	0			;SB_Energy
;		dc.w	128			;SB_OldEnergy
;		dc.w	128			;SB_MaxEnergy
;		dc.l	0			;SB_ScaleValue
;		dc.l	_StatBarImages		;SB_Images
;		dc.l	0			;SB_Screen
;		dc.l	0			;SB_ScreenBarHead

;*****************************************************************************************************
;Scroll stuctures

AnimYSpeeds	dc.l	2<<BITSHIFT,0,0,3<<BITSHIFT,0,0,4<<BITSHIFT,0,0,5<<BITSHIFT,0,0,6<<BITSHIFT,0,0,7<<BITSHIFT,0,0,8<<BITSHIFT,0,0,9<<BITSHIFT,0,0,10<<BITSHIFT
		dc.l	0,0,9<<BITSHIFT,0,0,8<<BITSHIFT,0,0,7<<BITSHIFT,0,0,6<<BITSHIFT,0,0,5<<BITSHIFT,0,0,4<<BITSHIFT,0,0,3<<BITSHIFT,0,0,2<<BITSHIFT,0,0,1<<BITSHIFT
		dc.l	0,0,0<<BITSHIFT,0,0,-1<<BITSHIFT,0,0,-2<<BITSHIFT,0,0,-3<<BITSHIFT,0,0,-4<<BITSHIFT,0,0,-5<<BITSHIFT,0,0,-6<<BITSHIFT,0,0,-7<<BITSHIFT,0,0,-8<<BITSHIFT,0,0,-9<<BITSHIFT,0,0,-10<<BITSHIFT
		dc.l	0,0,-9<<BITSHIFT,0,0,-8<<BITSHIFT,0,0,-7<<BITSHIFT,0,0,-6<<BITSHIFT,0,0,-5<<BITSHIFT,0,0,-4<<BITSHIFT,0,0,-3<<BITSHIFT,0,0,-2<<BITSHIFT,0,0,-1<<BITSHIFT,0,0,0<<BITSHIFT,0,0,1<<BITSHIFT

_JoyDirTable
		dc.b	00,00,128,00,192,224,160,00,64,32,96,00,00,00,00,00
		dc.b	00,00,128,00,192,224,160,00,64,32,96,00,00,00,00,00

;**********************************************************************************************

_StatBarImages	ds.l	17
_StatBarHeight	ds.l	17


;**********************************************************************************************
	SECTION	ChipData,DATA_C 

_DummyCopper	dc.w	$FFFF,$FFFE


;****************************
		;Sprites

	CNOP	0,8

_NullSprite	dc.w	0,0,0,0,0,0,0,0
		dc.l	$0000,$0000,0,0
	CNOP	0,8


;**********************************************************************************************

	SECTION	ChipBss,BSS_C



;**********************************************************************************************
