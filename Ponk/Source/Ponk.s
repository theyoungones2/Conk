	incdir	"ck:Ponk/inc/"
	include	"source/libs.gs"
	include	"source/header.i"
	include	"source/Constants.i"
	include	"source/Structures.i"
	include	"source/RawKeyCodes.i"
	include	"source/Lame.i"

	incdir	"Include:"
	include "workbench/startup.i"

;_custom		equ	$DFF000
AbsExecBase	equ	4

	XDEF	_SysBase,_GfxBase,_DOSBase,_IntuitionBase,_ChipMemBase,_OurTaskBase
	XDEF	_KillSprites,_ShellPrint,_WaitVBlank,_WaitPastVBlank
	XDEF	_JoystickDecode,_WaitBlit,_LoadWholeFile,_Random
	XDEF	_ConstrainedRandom
	XDEF	_ShellPrintNumber,_GetTextLength,_stdout,_TopazTextFont,_RasterTime

	XDEF	OldTaskPri,_OurTaskBase,ErrorCode,OldView,OldDMA,OldIntenar
	XDEF	InputHandlerStatus,_LameReq
	XDEF	_PackASCII,_NextLevelBuffer,_GlobalData,_ALittleCopperList
	XDEF	_OldDirLock,_OurDirLock

	XREF	_ActionStart,_SetupInputHandler,_KillInputHandler,_ciaa
	XREF	_InitLame,_KillLame,_FreeFiles,_LoadFiles,_FreeFiles,_FlushFiles,_ExamineFiles
	XREF	_LoadGameConfig,_FreeGameConfig
	XREF	_OperatorTable,_Templates,_OperatorNames,_SETC1
	XREF	_custom
	XREF	_RawKeyArray,_ASCIIKeyBuffer
	XREF	_KPrintF,_RunGame
	XREF	_StartPlayerSubTask,_EndPlayerSubTask
	XREF	_CIAABase,_ModulePlayerEnable
	XREF	_InitCIAInterrupt,_StopCIAInterrupt,_SpritePosToggle,_SpriteCopperToggle,_DisplaySprites
	XREF	_InitModulePlayer,_StopModulePlayer,_StatBarImages
	XREF	_StatBarImages

	;From Action.s
	XREF	_GameClock,_LevelClock,_DummyCopper


ER_OK			equ	0
ER_MEM			equ	1
ER_INPUTHANDLER		equ	2
ER_LOAD			equ	3

CHIP_MEM_SIZE	equ	10		;BUFFERWIDTH*BUFFERHEIGHT*PLANES*2

**************************************************************

	IFNE	CHIPASSEMBLE
	SECTION	WhatLovelyCode,code_C
	ELSE
	SECTION	WhatLovelyCode,code
	ENDC

Start

	move.l	sp,OldSP
	movem.l	d0/a0,-(sp)		;save initial values

	lea	_custom,a6
	move.w	vhposr(a6),RN_seed

	move.l	#0,_ChipMemBase
	move.l	#0,_DOSBase
	move.l	#0,_GfxBase
	move.l	#0,_IntuitionBase
	move.l	AbsExecBase,_SysBase

	lea	_custom,a6
	move.w	dmaconr(a6),OldDMA
	or.w	#DMAF_SETCLR,OldDMA

	move.w	intenar(a6),OldIntenar
	or.w	#INTF_SETCLR,OldIntenar

	;open dos.library
	lea	DOSName,a1
	move.l	#0,d0
	CALL	_SysBase,OpenLibrary
	move.l	d0,_DOSBase
	beq	ShutDown

	CALL	_DOSBase,Output		;Will return d0.l as handler
	move.l	d0,_stdout		;d0.l = File (Handler)

	;open graphics.library
	lea	GfxName,a1
	move.l	#0,d0
	CALL	_SysBase,OpenLibrary
	move.l	d0,_GfxBase
	beq	ShutDown

	;open intuition.library library
	lea	IntuitionName,a1
	move.l	#0,d0
	CALL	_SysBase,OpenLibrary
	move.l	d0,_IntuitionBase
	beq	ShutDown

	;save old view
	move.l	_GfxBase,a6
	move.l	gb_ActiView(a6),OldView

	move.l	d0,a0
	move.b	gb_ChipRevBits0(a0),d0
	and.b	#SETCHIPREV_AA,d0
	cmp.b	#SETCHIPREV_AA,d0
	beq	.Aga
	move.l	#0,d0
	bra	.PrintRev
.Aga	move.l	#$FF,d0
.PrintRev
	;bsr	_ShellPrintNumber

	move.l	#MEMF_CHIP,d1
	CALL	_SysBase,AvailMem
	;bsr	_ShellPrintNumber

	move.l	#MEMF_FAST,d1
	CALL	_SysBase,AvailMem
	;bsr	_ShellPrintNumber

	bsr	_InitLame
	move.b	#130,_NextLevelBuffer

	;Sort out all the wb message crap.

	clr.l	ReturnMsg
	sub.l	a1,a1
	CALL	_SysBase,FindTask	;find us
	move.l	d0,_OurTaskBase
	move.l	d0,a4

	tst.l	pr_CLI(a4)
	beq	.FromWorkbench

	movem.l	(sp)+,d0/a0

;	move.l	#Template,d1
;	move.l	#ReadArgsArray,d2
;	move.l	#0,d3
;	CALL	_DOSBase,ReadArgs
;	move.l	d0,ReadArgsPntr
;	bne	.LoadLevel
;	bra	ShutDown
;.LoadLevel
;	move.l	ReadArgsArray,a0

	;copy the cli args (game name) out into a buffer
	lea	ArgBuf,a1
	move.w	d0,d1
	cmp.w	#64-1,d1
	bhs	ShutDown
	sub.w	#1,d1
	bmi	ShutDown
.aloop
	cmp.b	#10,(a0)
	beq	.adone
	cmp.b	#0,(a0)
	beq	.adone
	move.b	(a0)+,(a1)+
	dbra	d1,.aloop
.adone
	move.b	#0,(a1)

	lea	ArgBuf,a0
	bsr	_LoadGameConfig
	cmp.w	#LAME_FAILAT,d0
	bge	ShutDown

;	move.l	ReadArgsPntr,d1
;	CALL	_DOSBase,FreeArgs

.NoReadArgs
		; we were called from the CLI

	move.w	#1,LameFileStatus
	bra	.EndStartup		;and run the user prog

		; we were called from the Workbench
.FromWorkbench

	lea	pr_MsgPort(a4),a0
	CALL	_SysBase,WaitPort	;wait for a message
	lea	pr_MsgPort(a4),a0
	CALL	_SysBase,GetMsg		;then get it
	move.l	d0,ReturnMsg		;save it for later reply
	beq	ShutDown
	move.l	d0,a0

	cmp.l	#2,sm_NumArgs(a0)
	bne	ShutDown

	move.l	sm_ArgList(a0),a0
	lea	wa_SIZEOF(a0),a0

	move.l	a0,-(sp)
	move.l	wa_Lock(a0),d1
	CALL	_DOSBase,CurrentDir
	move.l	(sp)+,a0

	move.l	wa_Name(a0),a0
	cmp.l	#0,a0
	beq	ShutDown

	bsr	_LoadGameConfig
	cmp.w	#LAME_FAILAT,d0
	bge	ShutDown
	move.w	#1,LameFileStatus

	movem.l	(sp)+,d0/a0		;restore
.EndStartup
	;kill view
	FLUSHVIEW

	CALL	_IntuitionBase,CloseWorkBench
	move.l	d0,IShotJR

	;kill view
;	FLUSHVIEW
	lea	_custom,a6
	move.l	#_DummyCopper,cop1lc(a6)

	bsr	_main			;call our program
					; returns to here with exit code in d0
	move.l	d0,-(sp)
	move.l	ReturnMsg,d0
	beq	.ExitToDos

	CALL	_SysBase,Forbid
	move.l	ReturnMsg,a1
	CALL	_SysBase,ReplyMsg
.ExitToDos
	move.l	(sp)+,d0

	rts

		; the program starts here
_main	
	lea	TopazTextAttr,a0			;open the Topaz Font
	CALL	_GfxBase,OpenFont
	move.l	d0,_TopazTextFont
	beq	ShutDown

	;bump the task priority up
	move.l	_OurTaskBase,a1
	move.l	#TASKPRIORITY,d0			;Set Task priority
	CALL	_SysBase,SetTaskPri
	move.w	d0,OldTaskPri

	move.w	#ER_OK,ErrorCode

	IFNE	IHANDLER
	bsr	_SetupInputHandler
	tst.w	d0
	bmi	ShutDown
	move.w	#1,InputHandlerStatus
	ENDC

;******************

.SetupModPlayerSubtTask
	bsr	_StartPlayerSubTask
	tst.w	d0
	beq	ShutDown
	move.b	#0,_ModulePlayerEnable
	bsr	_InitCIAInterrupt
	tst.l	_CIAABase
	bne	.CIAOkay
	lea	CIAErrorTxt,a0
	bsr	_ShellPrint
	bra	ShutDown
.CIAOkay

	move.l	#0,_GameClock

	FLUSHVIEW
.LoadLevel
	move.w	#ER_LOAD,ErrorCode
	lea	_LameReq,a0
	move.l	#_OperatorTable,lr_Operators(a0)
	move.l	#_Templates,lr_Templates(a0)
	move.l	#_OperatorNames,lr_OperatorNames(a0)
	move.l	#_StatBarImages,lr_ShieldBarImagePtrs(a0)
	bsr	_RunGame
	move.w	#ER_OK,ErrorCode

**************************************************************

ShutDown
	FLUSHVIEW

	;Make sure we stop the module player before killing lame, else
	;if it's playing a module lame won't free the module data. (makes sense)
	bsr	_StopModulePlayer

	tst.l	_CIAABase
	beq	.NoCIAInt
	bsr	_StopCIAInterrupt
.NoCIAInt
	bsr	_EndPlayerSubTask

	tst.w	InputHandlerStatus
	beq	.DontCloseHandler
	bsr	_KillInputHandler
.DontCloseHandler

	tst.w	LameFileStatus
	beq	.DontFreeLevel

	bsr	_FreeGameConfig
	bsr	_KillLame
.DontFreeLevel

	move.l	_TopazTextFont,a1
	cmp.l	#0,a1
	beq	.TopazFontNotOpen
	CALL	_GfxBase,CloseFont
.TopazFontNotOpen

	move.l	_GfxBase,d0
	beq	.SkipGfx

	;restore old view
	move.l	OldView,a1
	CALL	_GfxBase,LoadView		;Fix view
	lea	_custom,a5
	move.l	gb_copinit(a6),cop1lc(a5)		;Kick it into life
	CALL	_GfxBase,WaitTOF
	CALL	_GfxBase,WaitTOF

	tst.l	IShotJR
	beq	.NoWB
	CALL	_IntuitionBase,OpenWorkBench
.NoWB


	move.l	_GfxBase,a1
	CALL	_SysBase,CloseLibrary
.SkipGfx
	move.l	_IntuitionBase,a1
	cmp.l	#0,a1
	beq	.SkipIntuition
	CALL	_SysBase,CloseLibrary
.SkipIntuition

	move.w	ErrorCode,d0
	tst.w	d0
	beq	.KillDOS
	lea	ErrorTable,a0
	lsl.w	#2,d0
	move.l	(a0,d0.w),a0
	bsr	_ShellPrint
.KillDOS
	move.l	_DOSBase,d0
	beq	.DosNotOpen

	;CD back to original dir if needed
	move.l	_OldDirLock,d1
	beq	.FreeOurLock
	CALL	_DOSBase,CurrentDir

.FreeOurLock
	move.l	_OurDirLock,d1
	beq	.ReallyKillDOS
	CALL	_DOSBase,UnLock

.ReallyKillDOS
	move.l	_DOSBase,a1
	cmp.l	#0,a1
	beq	.DosNotOpen
	CALL	_SysBase,CloseLibrary

.DosNotOpen

	;restore normal task priority
	move.l	_OurTaskBase,a1
	move.w	OldTaskPri,d0
	CALL	_SysBase,SetTaskPri

	;Put back any dma that went astray
	lea	_custom,a6
	move.w	OldDMA,dmacon(a6)

	move.w	OldIntenar,intena(a6)

	move.l	OldSP,sp

	;exit back to DOS
	move.l	#0,d0
	rts


_KillSprites
	CALL	_GfxBase,WaitTOF
	lea	_custom,a6
	move.w	#DMAF_SPRITE,dmacon(a6)		;Kill sprite dma.
	move.l	#0,spr0data(a6)
	move.l	#0,spr1data(a6)
	move.l	#0,spr2data(a6)
	move.l	#0,spr3data(a6)
	move.l	#0,spr4data(a6)
	move.l	#0,spr5data(a6)
	move.l	#0,spr6data(a6)
	move.l	#0,spr7data(a6)
	rts


_ShellPrintNumber
	;Inputs:	d0.l = Num


	movem.l	d2-d3,-(sp)
	move.l	d0,d2
	move.l	#28,d1		;d1.w = Bit shifts
	move.l	#$F0000000,d3	;d3.l = Mask
	lea	HexNumbers,a0
	lea	NumberBuffer,a1
.Loop
	move.l	d2,d0
	and.l	d3,d0
	lsr.l	d1,d0
	move.b	(a0,d0.w),(a1)+
	lsr.l	#4,d3			;Shift the Mask Accross
	sub.l	#4,d1
	bpl	.Loop
	move.b	#10,(a1)+
	move.b	#0,(a1)
	lea	NumberBuffer,a0
	bsr	_ShellPrint

	movem.l	(sp)+,d2-d3
	rts

_ShellPrint	;Print 0 terminated String to the Shell
		;Inputs  : a0.l = Text Base
		;Outputs : Nil

	movem.l	d2-d3/a6,-(sp)
	move.l	a0,d2			;Copy Base of text for later use
	bsr	_GetTextLength
	move.l	d0,d3			;Store length for later use
	CALL	_DOSBase,Output		;Will return d0.l as handler
	move.l	d0,d1			;d1.l = File (Handler)
					;d2.l = Base of Text
					;d3.l = Length
	CALL	_DOSBase,Write		;Print It.
	movem.l	(sp)+,d2-d3/a6
	rts

_GetTextLength	;Returns the length of any 0 terminated String
		;(not including termination character)
		;Inputs  : a0.l = Base of text
		;Outputs : d0.l = Length of String
	move.l	#0,d0
.Loop
	tst.b	(a0)+
	beq	.Exit
	add.l	#1,d0
	bra	.Loop
.Exit
	rts

	IFNE	PALMODE

_WaitVBlank
	lea	_custom,a6
.Loop1	btst	#0,vposr+1(a6)
	beq	.Loop1
.Loop2	cmp.b	#45,vhposr(a6)
	bne	.Loop2
	rts
_WaitPastVBlank
	lea	_custom,a6
.Loop1	btst	#0,vposr+1(a6)
	beq	.Loop1
.Loop2	cmp.b	#46,vhposr(a6)
	bne	.Loop2
	rts

	ELSE

_WaitVBlank
	lea	_custom,a6
.Loop	cmp.b	#$FE,vhposr(a6)
	bne	.Loop
	rts
_WaitPastVBlank
	lea	_custom,a6
.Loop	cmp.b	#$FF,vhposr(a6)
	bne	.Loop
	rts

	ENDC

_WaitBlit
	lea	_custom,a6
	btst	#6,dmaconr(a6)
.Loop
	btst	#6,dmaconr(a6)
	bne	.Loop
	rts

_JoystickDecode
	;converts a JOYxDAT value into a byte value with bits
	;for each direction (as on C64).
	;d0 = 0 for port0, 1 for port1
	;decoded value returned in d0

	movem.l	d1-d3/a0-a1/a6,-(sp)
	moveq.w	#0,d3
	lea	_custom,a6
	tst.w	d0
	bne	.Port1

	;Read Port 0
	move.w	joy0dat(a6),d0
	lea	_ciaa,a0
	;check left button
	btst.b	#6,ciapra(a0)
	bne	.CheckRB0
	bset	#4,d3
.CheckRB0
	move.w	#$FF00,potgo(a6)	;configure for button input
	move.w	potinp(a6),d1		;read pot
	btst	#10,d1			;check right button
	bne	.CheckMB0
	bset	#6,d3
.CheckMB0
	btst	#8,d1			;check middle button
	bne	.GoForIt
	bset	#5,d3
	bra	.GoForIt

	;Read Port 1
.Port1
	move.w	joy1dat(a6),d0
	lea	_ciaa,a0
	btst.b	#7,ciapra(a0)
	bne	.CheckRB1
	bset	#4,d3
.CheckRB1
	move.w	#$FF00,potgo(a6)	;configure for button input
	move.w	potinp(a6),d1		;read pot
	btst	#14,d1			;check right button
	bne	.CheckMB1
	bset	#6,d3
.CheckMB1
	btst	#12,d1			;check middle button
	bne	.GoForIt
	bset	#5,d3
	;fall thru

.GoForIt
	btst	#9,d0
	beq	.JoyRight
.BobLeft
	or.b	#4,d3
	bra	.JoyUp
.JoyRight
	btst	#1,d0
	beq	.JoyUp
.BobRight
	or.b	#8,d3
.JoyUp
	move.w	d0,d1
	move.w	d0,d2
	and.w	#$0200,d1
	and.w	#$0100,d2
	lsr.w	#1,d1
	eor.w	d1,d2
	beq	.JoyDown
.BobUp
	or.b	#1,d3
	bra	.JoyFire
.JoyDown
	move.w	d0,d1
	move.w	d0,d2
	and.w	#$0002,d1
	and.w	#$0001,d2
	lsr.w	#1,d1
	eor.w	d1,d2
	beq	.JoyFire
.BobDown
	or.b	#2,d3
.JoyFire
	move.w	d3,d0
	bra	.NotFire

	lea	_RawKeyArray,a0
	tst.b	Key_A(a0)
	beq	.NotUp
	bset	#0,d0
.NotUp
	tst.b	Key_Z(a0)
	beq	.NotDown
	bset	#1,d0
.NotDown
	tst.b	Key_O(a0)
	beq	.NotLeft
	bset	#2,d0
.NotLeft
	tst.b	Key_P(a0)
	beq	.NotRight
	bset	#3,d0
.NotRight
	tst.b	Key_Space(a0)
	beq	.NotFire
	bset	#4,d0
.NotFire
	movem.l	(sp)+,d1-d3/a0-a1/a6
	rts




_OpenFile	;Open a DOS File
	;Inputs:	a0.l = File Name
	;Outputs:	a2.l = File Handle, 0 = Error

	move.l	a0,d1			;Copy Name
	move.l	#MODE_OLDFILE,d2	;Type
	CALL	_DOSBase,Open
	move.l	d0,a2			;Store the File Handle for later.
	rts

_CloseFile	;Close an Open File
	;Inputs:	a2.l = FileHandle
	;Outputs:	NIL
	cmp.l	#0,a2
	beq	.Exit
	move.l	a2,d1
	CALL	_DOSBase,Close
.Exit
	rts

	
_GetFileLength	;Return the length of a file.
	;Inputs:	a2.l = FileHandle
	;Ouputs:	d0.l = Length

	movem.l	d2-d4,-(sp)

	cmp.l	#0,a2
	beq	.Exit

	move.l	a2,d1			;Copy File Handle
	move.l	#0,d2			;Set Position
	move.l	#OFFSET_END,d3		;Seek to End
	CALL	_DOSBase,Seek
	move.l	a2,d1			;And Seek back to the
	move.l	#0,d2			;Start again, so we 
	move.l	#OFFSET_BEGINNING,d3	;can read where we were.
	CALL	_DOSBase,Seek
.Exit
	movem.l	(sp)+,d2-d4
	rts


_ReadFile	;Read in a selected number of bytes of a file.
	;Inputs:	a0.l = Storage Ptr
	;		a2.l = FileHandle
	;		d0.l = Number of Bytes
	;Outputs:	d0.l = Number of bytes actually read.

	movem.l	d2-d3,-(sp)
	move.l	a2,d1			;Copy File Handle
	move.l	a0,d2			;Copy Storage ptr
	move.l	d0,d3			;Read in the number of bytes that we want.
	CALL	_DOSBase,Read
	movem.l	(sp)+,d2-d3
	rts


_SeekFile	;Seek to a certain position in the file
	;Inputs:	a2.l = File Handle
	;		d0.l = File Position
	;Outputs:	d0.l = Old Position

	movem.l	d2-d3,-(sp)
	move.l	a2,d1			;Copy File Handle
	move.l	d0,d2			;Set Position
	move.l	#OFFSET_BEGINNING,d3	;Seek to Wanted position
	CALL	_DOSBase,Seek
	movem.l	(sp)+,d2-d3
	rts


_LoadWholeFile
	;Inputs:	a0.l = Ptr to Name
	;		a1.l = Ptr to Storage
	;Outputs:	d0.l : 1 = Fail, 0 = All OK.

	movem.l	a2-a3,-(sp)

	move.l	a1,a3			;Save Storage for later.

	bsr	_OpenFile
	cmp.l	#0,a2
	beq	.Error

	bsr	_GetFileLength
	move.l	a3,a0			;Copy Storage Ptr back.
	bsr	_ReadFile

	bsr	_CloseFile

	move.l	#0,d0
	bra	.Exit
.Error
	move.l	#1,d0
.Exit
	movem.l	(sp)+,a2-a3
	rts

;WORD __asm ConstrainedRandom( register __d0 WORD min, register __d1 WORD max );

_ConstrainedRandom
	;produce a signed random number between min and max
	;in: d0.w=min, d1.w=max (both signed)
	;out: d0.w random number

	move.l	d2,-(sp)
	add.w	#1,d1
	move.w	d1,d2
	sub.w	d0,d2		;d2 = max-min
	move.w	d0,-(sp)
	jsr	_Random
	mulu	d2,d0		;rand()*(max-min)
	swap	d0		;/RAND_MAX
	add.w	(sp)+,d0	;+min
	move.l	(sp)+,d2
	rts


_Random	;Get a random number
	;Inputs:	none
	;Outputs:	d0.w = new random number (0 - 65535)
	;Registers used: Nil

	movem.l	d1-d2,-(sp)
	moveq	#0,d0
	move.l	d0,d1
	move.l	d0,d2
	move.w	RN_seed,d1
	move.w	d1,d0
	lsl.l	#8,d0
	move.b	#$FD,d0
	sub.l	d1,d0
	subx.l	d1,d0
	swap	d0
	move.w	d0,d1
	clr.w	d0
	swap	d0
	subx.w	d1,d0
	addx.w	d2,d0
	move.w	d0,RN_seed
	movem.l	(sp)+,d1-d2
	rts


;VOID __asm PackASCII( register __a0 UBYTE *source, register __a1 UBYTE *dest,
;	register __d0 UWORD numchars );

_PackASCII
	;a0 = src ASCII string
	;a1 = dest buffer
	;d0.w = number of chars to pack

	;filter out crap
	movem.l	a0-a2/d0,-(sp)
	bsr	_MakePackable
	movem.l	(sp)+,a0-a2/d0

	movem.w	d2-d3,-(sp)

	move.w	#7,d2				;reset bit pointer
.CharLoop
	tst.w	d0				;no chars left?
	beq	.Done
	move.b	(a0)+,d1			;get a char
	bne	.convert			;hit a null?
	move.b	#64,d1				;map null to '@' (our custom null)
	sub.l	#1,a0				;make sure we stay on the null
.convert
	sub.b	#32,d1
	;pack char into dest
	lsl.b	#2,d1				;ignore top 2 bits of char
	move.w	#6-1,d3				;bit counter
.BitLoop
	bclr.b	d2,(a1)				;preclear dest bit
	lsl.b	d1				;grab char bit
	bcc	.noset				;should it be set?
	bset.b	d2,(a1)				;set it.
.noset
	sub.w	#1,d2				;point to next dest bit.
	bpl	.nextbit			;end of byte?
	add.l	#1,a1				;yep, point to next byte
	move.w	#7,d2				;reset bit pointer
.nextbit
	dbra	d3,.BitLoop

	sub.w	#1,d0
	bra	.CharLoop

.Done
	cmp.w	#7,d2				;partially filled byte?
	beq	.bytepad			;nope, bugger off
.bitpadloop					;pad out rest of byte with zeros
	bclr.b	d2,(a1)
	sub.w	#1,d2
	bpl	.bitpadloop

.bytepad
	add.l	#1,a1
	sub.w	#1,d0
	bmi	.Fuckoff
.bytepadloop
	move.b	#0,(a1)+
	dbra	d0,.bytepadloop

.Fuckoff
	movem.w	(sp)+,d2-d3
	rts


;BOOL __asm IsPackable( register __d0 UBYTE c );

_IsPackable
	cmp.b	#32,d0
	blo		.nope
	cmp.b	#96,d0
	bhs		.nope
	cmp.b	#'@',d0
	beq		.nope
	move.l	#1,d0
	rts
.nope
	move.l	#0,d0
	rts


;BOOL __asm MakePackable( register __a0 UBYTE *str );

_MakePackable
	movem.l	d2-d3,-(sp)
	move.l	#0,d3

.snarfchar
	move.b	(a0),d2
	beq			.done

	cmp.b		#'a',d2
	blo			.checkvalidity
	cmp.b		#'z',d2
	bhi			.checkvalidity
	sub.b		#'a'-'A',d2	;to uppercase

.checkvalidity
	move.b	d2,d0
	bsr	_IsPackable
	tst.l	d0
	bne	.put			;not packable
	move.l	#1,d3			;changed one...
	move.b	#' ',d2			;...to a space.
.put
	move.b	d2,(a0)+
	bra	.snarfchar

.done
	move.l	d3,d0
	movem.l	(sp)+,d2-d3
	rts


**************************************************************


	SECTION Vars,BSS

_NextLevelBuffer ds.b	10

RN_seed		ds.w	1
_ChipMemBase	ds.l	1
_OurTaskBase	ds.l	1

ErrorCode	ds.w	1
ReturnMsg	ds.l	1

OldTaskPri	ds.w	1
OldSP		ds.l	1
OldView		ds.l	1
OldDMA		ds.w	1
OldIntenar	ds.w	1
_SysBase	ds.l	1
_GfxBase	ds.l	1
_IntuitionBase	ds.l	1
_DOSBase	ds.l	1
_stdout		ds.l	1
_TopazTextFont	ds.l	1

InputHandlerStatus	ds.w	1
LameFileStatus	ds.w	1
NumberBuffer	ds.b	8+1+1
		even

_GlobalData	ds.l	10

IShotJR		ds.l	1	;did we kill WB?
_OldDirLock	ds.l	1
_OurDirLock	ds.l	1

**************************************************************

	SECTION Data,DATA


_LameReq	ds.b	lr_SizeOf
		even


Template	dc.b	"LEVEL/A,R=RASTERTIME/S",0
		even

ReadArgsArray	dc.l	0	;\__ Both Get Filled By ReadArgs
_RasterTime	dc.l	0	;/

ReadArgsPntr	dc.l	0

ArgBuf		ds.b	64
		even

DOSName		DOSNAME
		even
GfxName		GRAPHICSNAME
		even
IntuitionName	dc.b	"intuition.library",0
		even
HexNumbers	dc.b	"0123456789ABCDEF"
		even
TopazTextAttr	dc.l	.TopazFontName		;ta_Name
		dc.w	8			;ta_YSize
		dc.b	0			;ta_Style
		dc.b	0			;ta_Flags
.TopazFontName	dc.b	"topaz.font",0
		even

ErrorTable	dc.l	0,.MemoryTxt,.InputHandlerTxt,.LoaderTxt
.MemoryTxt	dc.b	"Not Enuf Memory",0
		even
.InputHandlerTxt dc.b	"Can't Install Input Handler",10,0
		even
.LoaderTxt	dc.b	"",10,0
		even
CIAErrorTxt	dc.b	"No CIA interrupt available.",10,0
		even

**************************************************************

	SECTION Chip,DATA_C

_ALittleCopperList
	dc.w	bplcon0,$0200
	dc.w	$FFFF,$FFFE
	dc.w	$FFFF,$FFFE

