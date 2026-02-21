*************************************************************************
*			Console Handling Routines			*
*************************************************************************

		incdir	"PConk:inc/"
		include	"source/libs.gs"
		include	"source/Header.i"
		include	"source/Constants.i"
		include	"source/Structures.i"
		include	"source/GroovyCon.i"
		include	"source/RawKeyCodes.i"
		include	"source/Lame.i"


	XDEF	_OpenNewDebugWindow,_FreeDebugWindow,_BlitDebugWindow,_WipeDebugWindow	;,_MoveDebugWindow
	XDEF	CON_PutChar,_CON_PutStr,CON_CursorUp,CON_CursorDown,CON_CursorLeft,CON_CursorRight,CON_CursorSetX,CON_CursorSetY,CON_Clear

	XDEF	_DebugGroovyCon,_DebugGroovyConDevice
	XDEF	_StatBoxGroovyCon,_StatBoxGroovyConDevice
	XDEF	_DebugWindowX,_DebugWindowY
	XDEF	_MainMenu,_NewMenu,_CurrentMenu,_UpdateMenu
	XDEF	_GC_InitGroovyConStruct
	XDEF	_InsertVolumeGroovyCon,_InsertVolumeGroovyConDevice
	XDEF	_PrintInsertVolumeText

	XREF	_SysBase,_custom
	XREF	_GfxBase,_TopazTextFont,_WaitForClearRawKeys
	XREF	_WaitBlit,_WaitVBlank,_WaitPastVBlank

	XREF	_SetupInputHandler,InputHandlerStatus,_IntuitionBase
	XREF	_CopperInterruptStruct,CopperIntName,VBlankCounter,CopperIntHandler
	XREF	VBlankInterruptStruct,VBlankIntName,VBlankCounter,VBlankIntServer
	XREF	OldTaskPri,_OurTaskBase,_RawKeyArray,ErrorCode,Interrupts
	XREF	OldCopState,OldIntVector,_KillSprites
	XREF	_DummyCopper,_KillInputHandler,OldView,OldDMA,OldIntenar,Mode,MainLoopDone
	XREF	SliceCopperUpdate,_LameReq
	XREF	_Spam_Die,_Spam_Init,_Spam_PlaySound

	IFNE	CHIPASSEMBLE
	SECTION	WhatLovelyCode,CODE_C
	ELSE
	SECTION	WhatLovelyCode,CODE
	ENDC

ER_OK			equ	0
ER_MEM			equ	1
ER_INPUTHANDLER		equ	2
ER_LOAD			equ	3

 STRUCTURE DWBLIT,0
	ULONG	dw_BufferOffset
	ULONG	dw_BufferPos
	LABEL	DWBLIT_SIZEOF

*************************************************************************
**				Debug Routines			       **
*************************************************************************

;_MoveDebugWindow
;	;Inputs:	NULL
;	;Returns:	NULL
;
;	move.w	_DebugWindowX,d0
;	add.w	DW_XSpd,d0
;	tst.w	d0
;	bge	.greater0
;	move.l	#0,d0
;	move.w	#2,DW_XSpd
;.greater0
;	move.w	DW_Width,d1
;	lsl.w	#3,d1
;	sub.w	#320,d1
;	neg.w	d1
;	cmp.w	d1,d0
;	blt	.lessthanright
;	move.w	d1,d0
;	sub.w	#1,d0
;	move.w	#-2,DW_XSpd
;.lessthanright
;	move.w	d0,_DebugWindowX
;
;	move.w	_DebugWindowY,d0
;	add.w	DW_YSpd,d0
;	tst.w	d0
;	bge	.greaterY0
;	move.l	#0,d0
;	move.w	#2,DW_YSpd
;.greaterY0
;	move.w	DW_Height,d1
;	sub.w	#256,d1
;	neg.w	d1
;	cmp.w	d1,d0
;	blt	.lessthanbottom
;	move.w	d1,d0
;	sub.w	#1,d0
;	move.w	#-2,DW_YSpd
;.lessthanbottom
;	move.w	d0,_DebugWindowY
;	rts
;
;DW_XSpd		dc.w	2
;DW_YSpd		dc.w	2


_OpenNewDebugWindow
	;Inputs:	a0.l = Associated Slice
	;		d0.w = Width (bytes, but word alligned)
	;		d1.w = Height (pixels)
	;		d2.w = Colour
	;Returns:	d0.w = TRUE  -> It worked,
	;		     = FALSE -> It failed.

	move.l	#0,DW_Base

	move.l	a0,DW_Slice
	move.w	d0,DW_Width
	move.w	d1,DW_Height
	move.w	d2,DW_Colour

	mulu	d1,d0
	move.l	d0,DW_Size
	move.l	#MEMF_CHIP!MEMF_CLEAR,d1
	CALL	_SysBase,AllocMem
	move.l	d0,DW_Base
	beq	.Abort

	lea	_DebugGroovyCon,a0
	bsr	_GC_InitGroovyConStruct

	lea	_DebugGroovyCon,a0		;a0.l = GroovyConStruct
	move.w	DW_Width,d0
	and.l	#$0000FFFF,d0
	move.l	d0,gc_BytesPerPlane(a0)
	move.l	d0,gc_BytesPerRow(a0)
	add.l	DW_Base,d0
	move.l	d0,gc_GfxMem(a0)

	move.w	#1,gc_Depth(a0)
	move.w	DW_Width,gc_ConWidth(a0)
	move.w	DW_Height,d0
	lsr.w	#3,d0
	move.w	d0,gc_ConHeight(a0)
	move.l	_TopazTextFont,gc_TextFont(a0)

	;Set Up Blitting Info
	move.l	DW_Slice,a0
	lea	DW_BltData0,a1
	move.l	#0,dw_BufferOffset(a1)
	move.l	#0,dw_BufferPos(a1)
	lea	DW_BltData1,a1
	move.l	#0,dw_BufferOffset(a1)
	move.l	#0,dw_BufferPos(a1)
	move.l	SS_BufferWidth(a0),d0	;Set up Modulo's
	sub.w	DW_Width,d0
	sub.w	#2,d0
	move.w	d0,DW_WipeBufferMod
	add.l	SS_BufferWidthxPlanes(a0),d0	;\__ Add BufferWidth * (Planes-1) to modulo
	sub.l	SS_BufferWidth(a0),d0		;/
	move.w	d0,DW_DrawBufferMod
	move.w	DW_Height,d0		;Calc BltSize
	lsl.w	#6,d0
	move.w	DW_Width,d1
	lsr.w	#1,d1			;Shift down to words
	add.w	#1,d1			;Add Extra word for Bit Shifting
	or.w	d1,d0
	move.w	d0,DW_DrawBltSize
	move.w	DW_Height,d0
	mulu	SS_Planes(a0),d0
	lsl.w	#6,d0
	or.w	d1,d0
	move.w	d0,DW_WipeBltSize

	move.w	#16,_DebugWindowX
	move.w	#0,_DebugWindowY

	move.w	#TRUE,d0
	rts
.Abort
	move.l	DW_Base,a1
	cmp.l	#0,a1
	beq	.DebugWindowNotAllocated
	move.l	DW_Size,d0
	CALL	_SysBase,FreeMem
	move.l	#0,DW_Base
.DebugWindowNotAllocated
	move.w	#FALSE,d0
	rts

_FreeDebugWindow
	;Inputs:	NULL
	;Return;	NULL

	move.l	DW_Base,a1
	cmp.l	#0,a1
	beq	.Exit
	move.l	DW_Size,d0
	CALL	_SysBase,FreeMem
	move.l	#NULL,DW_Base
	move.l	#0,DW_Size
.Exit
	rts

_BlitDebugWindow
	;Inputs:	NULL
	;Returns:	NULL

	movem.l	d2-d3/a2,-(sp)

	CALL	_GfxBase,OwnBlitter
	lea	_custom,a6
	bsr	_WaitBlit
	BLITNASTY

	btst.b	#6,$bfe001
	bne	.Fred
	add.w	#1,DW_Colour
.Fred
	lea	DW_BltData0,a1
	move.l	DW_Slice,a0			;a0.l = Ptr to Slice
	tst.l	SS_ScreenToggle(a0)
	beq	.DoBuffer0
	lea	DW_BltData1,a1			;a1.l = Pointer to appropriate BltData Area
.DoBuffer0
	move.l	#0,dw_BufferPos(a1)

	lea	_custom,a6
	bsr	_WaitBlit
	move.w	#$FFFF,bltafwm(a6)
	move.w	#$0000,bltalwm(a6)

	tst.l	DW_Base
	beq	.Exit

	move.l	SS_Scroll(a0),a2
	move.w	#-2,bltamod(a6)
	move.w	DW_DrawBufferMod,bltbmod(a6)
	move.w	DW_DrawBufferMod,bltdmod(a6)

	move.w	#0,bltcon1(a6)
	move.l	SC_Pos(a2),d3
	lsr.w	#BITSHIFT,d3
	add.w	_DebugWindowX,d3
	lsl.w	#8,d3
	lsl.w	#4,d3
	and.w	#$F000,d3			;d3.w = bltcon0 shift value

	move.l	SS_BufferWidthxPlanes(a0),d0
	move.l	SC_YPos(a2),d1
	asr.l	#BITSHIFT,d1
	add.w	_DebugWindowY,d1
	mulu	d0,d1				;Cal'd Y Pos
	move.l	SC_Pos(a2),d0
	lsr.l	#BITSHIFT,d0
	add.w	_DebugWindowX,d0
	lsr.w	#3,d0
	and.l	#$FFFFFFFE,d0
	add.l	d1,d0
	move.l	d0,dw_BufferOffset(a1)
	add.l	SS_LogBase(a0),d0
	move.l	d0,dw_BufferPos(a1)		;d0.l = Running Buffer Pointer

	move.w	DW_Colour,d2			;d2.w = Colour
	move.w	SS_Planes(a0),d1
	sub.w	#1,d1
.PlaneLoop
	bsr	_WaitBlit
	lsr.w	#1,d2
	bcc	.ClearPlane
.SetPlane
	move.l	DW_Base,bltapt(a6)
	move.l	d0,bltbpt(a6)
	move.l	d0,bltdpt(a6)
	and.w	#$F000,d3
	or.w	#$0DFC,d3
	move.w	d3,bltcon0(a6)
	move.w	DW_DrawBltSize,bltsize(a6)
	add.l	SS_BufferWidth(a0),d0
	dbra	d1,.PlaneLoop
	bra	.DoneBlitting
.ClearPlane
	move.l	DW_Base,bltapt(a6)
	move.l	d0,bltbpt(a6)
	move.l	d0,bltdpt(a6)
	and.w	#$F000,d3
	or.w	#$0D0C,d3
	move.w	d3,bltcon0(a6)
	move.w	DW_DrawBltSize,bltsize(a6)
	add.l	SS_BufferWidth(a0),d0
	dbra	d1,.PlaneLoop
.DoneBlitting
.Exit
	lea	_custom,a6
	bsr	_WaitBlit
	BLITNICE
	CALL	_GfxBase,DisownBlitter
	movem.l	(sp)+,d2-d3/a2
	rts

_WipeDebugWindow
	;Inputs:	NULL
	;Returns:	NULL

	lea	DW_BltData0,a1
	move.l	DW_Slice,a0			;a0.l = Ptr to Slice
	tst.l	SS_ScreenToggle(a0)
	beq	.DoBuffer0
	lea	DW_BltData1,a1			;a1.l = Pointer to appropriate BltData Area
.DoBuffer0
	cmp.l	#0,dw_BufferPos(a1)
	beq	.QuickExit

	CALL	_GfxBase,OwnBlitter
	lea	_custom,a6
	bsr	_WaitBlit
	BLITNASTY

	move.w	#$FFFF,bltafwm(a6)
	move.w	#$FFFF,bltalwm(a6)

	move.l	DW_Slice,a0
	move.l	SS_TriBase(a0),d0
	add.l	dw_BufferOffset(a1),d0
	move.l	d0,bltapt(a6)
	move.w	DW_WipeBufferMod,bltamod(a6)
	move.l	dw_BufferPos(a1),bltdpt(a6)
	move.w	DW_WipeBufferMod,bltdmod(a6)
	move.w	#0,bltcon1(a6)
	move.w	#$09F0,bltcon0(a6)
	move.w	DW_WipeBltSize,bltsize(a6)
.Exit
	lea	_custom,a6
	bsr	_WaitBlit
	BLITNICE
	CALL	_GfxBase,DisownBlitter
.QuickExit
	rts

ClearDebugWindow
	;Inputs:	NULL
	;Returns:	NULL

	CALL	_GfxBase,OwnBlitter
	lea	_custom,a6

	BLITNASTY
	move.l	DW_Base,bltdpt(a6)
	move.w	#0,bltdmod(a6)
	move.w	#0,bltcon1(a6)
	move.w	#$0100,bltcon0(a6)
	move.w	DW_Height,d0		;Calc BltSize
	lsl.w	#6,d0
	move.w	DW_Width,d1
	lsr.w	#1,d1			;Shift down to words
	or.w	d1,d0
	move.w	d0,bltsize(a6)
	BLITNICE

	CALL	_GfxBase,DisownBlitter
	rts

*************************************************************************
**			GroovyCon Specific Routines		       **
*************************************************************************

;void __asm GC_InitGroovyConStruct(register __a0 struct GroovyCon *gc);

_GC_InitGroovyConStruct
	;Inputs:	a0.l = Ptr to GroovyCon Struct
	;Returns:	NULL

	move.l	#NULL,gc_GfxMem(a0)
	move.l	#NULL,gc_TextFont(a0)
	move.w	#0,gc_CursorX(a0)
	move.w	#0,gc_CursorY(a0)
	rts

GC_PutChar
	;Inputs:	a0.l = Ptr to GroovyCon struct
	;		d0.b = ASCII character to print.
	;Returns:	NULL

	movem.l	d2/a2-a3,-(sp)

	cmp.l	#0,a0
	beq	.Exit

	cmp.l	#0,gc_GfxMem(a0)
	beq	.Exit

	move.l	gc_TextFont(a0),a1
	cmp.l	#0,a1
	beq	.Exit

	cmp.b	#10,d0				;If char is LineFeed then
	bne	.NotReturn			;push the cursor down one
	move.l	a0,a2				;and to the start of that line.
	bsr	GC_CursorDown
	move.l	a2,a0
	move.w	#0,d0
	bsr	GC_CursorSetX
	bra	.Exit
.NotReturn
	cmp.b	tf_LoChar(a1),d0
	bcs	.Exit				;Branch if lower than (unsigned)

	cmp.b	tf_HiChar(a1),d0
	bhi	.Exit

	sub.b	tf_LoChar(a1),d0		;Bring d0 down so that it starts at the
						;font's zeroith character.

	and.w	#$00FF,d0			;Get the characters location in the bitmap
	move.l	tf_CharLoc(a1),a2		;See RKM page 701 for more details.
	lsl.w	#2,d0				;Make long word offset into Charloc
	move.w	(a2,d0.w),d1
	lsr.w	#3,d1				;d1.w = Byte offset in CharData
	move.l	tf_CharData(a1),a2
	lea	(a2,d1.w),a2			;a2.l = Base of character in the bitmap

	;Right we have the characters base address so now work out the Console's cursor
	;address.
	move.l	gc_GfxMem(a0),a3
	move.w	gc_CursorY(a0),d0
	lsl.w	#3,d0				;Multiply it by 8 (the fonts height)
	move.l	gc_BytesPerRow(a0),d1
	mulu	d1,d0
	lea	(a3,d0.l),a3
	move.w	gc_CursorX(a0),d0
	lea	(a3,d0.w),a3			;a3.l = Ptr to correct cursor location

	move.w	tf_Modulo(a1),d2
	move.w	#8-1,d0
.RowLoop
	move.w	gc_Depth(a0),d1
	sub.w	#1,d1
.PlaneLoop
	move.b	(a2),(a3)			;Do some magic in here for bitplane
						;colour hokery pokery.
	add.l	gc_BytesPerPlane(a0),a3
	dbra	d1,.PlaneLoop
	lea	(a2,d2.w),a2			;Add Font Modulo
	dbra	d0,.RowLoop
	bsr	GC_CursorRight
.Exit
	movem.l	(sp)+,d2/a2-a3
	rts

GC_CursorLeft
	;Inputs:	a0.l = Ptr to GroovyCon struct
	;Returns:	NULL

	move.w	gc_CursorX(a0),d0
	sub.w	#1,d0
	bgt	.Exit
	move.w	gc_ConWidth(a0),d0
	sub.w	#1,d0
	move.w	d0,gc_CursorX(a0)
	bsr	GC_CursorUp
	rts
.Exit
	move.w	d0,gc_CursorX(a0)
	rts

GC_CursorRight
	;Inputs:	a0.l = Ptr to GroovyCon struct
	;Returns:	NULL

	move.w	gc_CursorX(a0),d0
	add.w	#1,d0
	cmp.w	gc_ConWidth(a0),d0
	blt	.Exit
	move.w	#0,d0
	move.w	d0,gc_CursorX(a0)
	bsr	GC_CursorDown
	rts
.Exit
	move.w	d0,gc_CursorX(a0)
	rts

GC_CursorUp
	;Inputs:	a0.l = Ptr to GroovyCon struct
	;Returns:	NULL
	
	move.w	gc_CursorY(a0),d0
	sub.w	#1,d0
	bgt	.Exit
	move.w	#0,d0
.Exit
	move.w	d0,gc_CursorY(a0)
	rts

GC_CursorDown
	;Inputs:	a0.l = Ptr to GroovyCon struct
	;Returns:	NULL
	
	move.w	gc_CursorY(a0),d0
	add.w	#1,d0
	cmp.w	gc_ConHeight(a0),d0
	blt	.Exit
	move.w	gc_ConHeight(a0),d0		;Should really scroll here
	sub.w	#1,d0
.Exit
	move.w	d0,gc_CursorY(a0)
	rts

GC_CursorSetX
	;Inputs:	a0.l = Ptr to GroovyCon struct
	;		d0.w = New X
	;Returns:	NULL
	
	move.w	d0,gc_CursorX(a0)
	rts

GC_CursorSetY
	;Inputs:	a0.l = Ptr to GroovyCon struct
	;		d0.w = New Y
	;Returns:	NULL
	
	move.w	d0,gc_CursorY(a0)
	rts

*************************************************************************
**			Generic Console Routines		       **
*************************************************************************

CON_PutChar
	;Inputs:	a3.l = Console Device
	;		d0.b = ASCII char to print
	;Returns:	NULL

	cmp.l	#0,a3
	beq	.Exit
	move.l	cd_ConsoleStruct(a3),a0
	move.l	cd_PutChar(a3),a1
	cmp.l	#0,a1
	beq	.Exit
	jsr	(a1)
.Exit
	rts


_CON_PutStr
	;Inputs:	a0.l = Console Device
	;		a1.l = Formated string to print
	;		Stack holds all string arguments
	;Returns:	NULL

	movem.l	a2/a3,-(sp)

	cmp.l	#0,a0
	beq	.Exit
	move.l	a0,a3
	move.l	a1,a0
	;On the Stack at this Stage is a2 & a3, the return address, then args
	lea	3*4(sp),a1
	lea	CON_PutChar,a2
	CALL	_SysBase,RawDoFmt
.Exit
	movem.l	(sp)+,a2/a3
	rts

CON_CursorUp
	;Inputs:	a0.l = Console Device
	;Returns:	NULL
	cmp.l	#0,a0
	beq	.Exit
	move.l	cd_ConsoleStruct(a0),d0
	move.l	cd_CursorUp(a0),a1
	cmp.l	#0,a1
	beq	.Exit
	move.l	d0,a0
	jsr	(a1)
.Exit
	rts

CON_CursorDown
	;Inputs:	a0.l = Console Device
	;Returns:	NULL
	cmp.l	#0,a0
	beq	.Exit
	move.l	cd_ConsoleStruct(a0),d0
	move.l	cd_CursorDown(a0),a1
	cmp.l	#0,a1
	beq	.Exit
	move.l	d0,a0
	jsr	(a1)
.Exit
	rts
CON_CursorLeft
	;Inputs:	a0.l = Console Device
	;Returns:	NULL
	cmp.l	#0,a0
	beq	.Exit
	move.l	cd_ConsoleStruct(a0),d0
	move.l	cd_CursorLeft(a0),a1
	cmp.l	#0,a1
	beq	.Exit
	move.l	d0,a0
	jsr	(a1)
.Exit
	rts
CON_CursorRight
	;Inputs:	a0.l = Console Device
	;Returns:	NULL
	cmp.l	#0,a0
	beq	.Exit
	move.l	cd_ConsoleStruct(a0),d0
	move.l	cd_CursorRight(a0),a1
	cmp.l	#0,a1
	beq	.Exit
	move.l	d0,a0
	jsr	(a1)
.Exit
	rts
CON_CursorSetX
	;Inputs:	a0.l = Console Device
	;		d0.w = New X
	;Returns:	NULL
	cmp.l	#0,a0
	beq	.Exit
	move.l	cd_ConsoleStruct(a0),d1
	move.l	cd_CursorSetX(a0),a1
	cmp.l	#0,a1
	beq	.Exit
	move.l	d1,a0
	jsr	(a1)
.Exit
	rts
CON_CursorSetY
	;Inputs:	a0.l = Console Device
	;		d0.w = New Y
	;Returns:	NULL
	cmp.l	#0,a0
	beq	.Exit
	move.l	cd_ConsoleStruct(a0),d1
	move.l	cd_CursorSetY(a0),a1
	cmp.l	#0,a1
	beq	.Exit
	move.l	d1,a0
	jsr	(a1)
.Exit
	rts

CON_Clear
	;Inputs:	a0.l = Console Device
	;Returns:	NULL
	move.l	a2,-(sp)

	cmp.l	#0,a0
	beq	.Exit
	move.l	a0,a2
	move.l	cd_ConsoleStruct(a2),a0
	move.l	cd_Clear(a2),a1
	cmp.l	#0,a1
	beq	.Exit
	jsr	(a1)
	move.l	a2,a0
	move.w	#0,d0
	bsr	CON_CursorSetX
	move.l	a2,a0
	move.w	#0,d0
	bsr	CON_CursorSetY
.Exit
	move.l	(sp)+,a2
	rts

; void __asm PrintInsertVolumeText(register __a0 struct GroovyConsoleDevice *,
;				   register __a1 char *fmtstring,
;				   register __a2 char *volumename);
_PrintInsertVolumeText
	;Inputs:	a0.l = ConsoleDevice
	;		a1.l = format string
	;		a2.l = volume name

	move.l	a2,-(sp)
	bsr	_CON_PutStr
	move.l	(sp)+,a2
	rts

*************************************************************************
**				Menu Routines			       **
*************************************************************************

_NewMenu
	;Inputs:	a0.l = New Menu
	;Outputs:	NULL

	move.l	a0,-(sp)
	move.l	_CurrentMenu,a0
	cmp.l	#0,a0
	beq	.NoCurrentMenu
	move.l	mn_Close(a0),a0
	cmp.l	#0,a0
	beq	.NoCurrentMenu
	jsr	(a0)
.NoCurrentMenu
	move.l	(sp)+,a0
	move.l	a0,_CurrentMenu
	move.l	mn_Init(a0),a0
	cmp.l	#0,a0
	beq	.Exit
	jsr	(a0)
.Exit
	rts

_UpdateMenu
	;Inputs:	NULL
	;Outputs:	NULL

	move.l	_CurrentMenu,a0
	cmp.l	#0,a0
	beq	.NoCurrentMenu
	move.l	mn_Update(a0),a0
	cmp.l	#0,a0
	beq	.NoCurrentMenu
	jsr	(a0)
.NoCurrentMenu
	rts

InitMainMenu
	;Inputs:	NULL
	;Returns:	NULL
	move.l	a2,-(sp)
	lea	_DebugGroovyConDevice,a2
	move.l	a2,a0
	bsr	CON_Clear
	bsr	_WaitBlit
	move.l	a2,a0
	lea	MainMenuText,a1
	bsr	_CON_PutStr
	move.l	(sp)+,a2
	rts

UpdateMainMenu
	;Inputs:	NULL
	;Returns:	NULL

	lea	_RawKeyArray,a0
	tst.b	Key_F4(a0)		;Back To WB
	beq	.NotF4
	bsr	_WaitForClearRawKeys
	bsr	BackToWorkbench
.NotF4
	rts

BackToWorkbench
	;Inputs:	NULL
	;Returns:	NULL

	bsr	_Spam_Die

	CALL	_SysBase,Disable

	lea	_custom,a6
	move.l	#_DummyCopper,cop1lc(a6)

	bsr	_WaitVBlank
	bsr	_WaitPastVBlank 
	bsr	_WaitVBlank
	bsr	_WaitPastVBlank
	bsr	_WaitVBlank
	bsr	_WaitPastVBlank

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

* PONK
	FLUSHVIEW

ShutDown
	tst.w	InputHandlerStatus
	beq	.DontCloseHandler
	bsr	_KillInputHandler
.DontCloseHandler
	;tst.w	LameFileStatus
	;beq	.DontFreeLevel

	;bsr	_KillLame
	;lea	_LameReq,a0
	;move.l	lr_Files(a0),a0
	;bsr	_FreeLevelConfig
.DontFreeLevel

	;move.l	_TopazTextFont,a1
	;cmp.l	#0,a1
	;beq	.TopazFontNotOpen
	;CALL	_GfxBase,CloseFont
.TopazFontNotOpen

	move.l	_GfxBase,d0
	;beq	.SkipGfx

	;restore old view
	move.l	OldView,a1
	CALL	_GfxBase,LoadView		;Fix view
	lea	_custom,a5
	move.l	gb_copinit(a6),cop1lc(a5)		;Kick it into life
	CALL	_GfxBase,WaitTOF
	CALL	_GfxBase,WaitTOF

	;move.l	_GfxBase,a1
	;CALL	_SysBase,CloseLibrary
.SkipGfx

	;move.w	ErrorCode,d0
	;tst.w	d0
	;beq	.KillDOS
	;lea	ErrorTable,a0
	;lsl.w	#2,d0
	;move.l	(a0,d0.w),a0
	;bsr	_ShellPrint
.KillDOS
	;move.l	_DOSBase,a1
	;cmp.l	#0,a1
	;beq	.DosNotOpen
	;CALL	_SysBase,CloseLibrary
.DosNotOpen
	;restore normal task priority
	move.l	_OurTaskBase,a1
	move.w	OldTaskPri,d0
	CALL	_SysBase,SetTaskPri

	;But back any dma that went astray
	lea	_custom,a6
	move.w	OldDMA,dmacon(a6)

	move.w	OldIntenar,intena(a6)

	CALL	_IntuitionBase,WBenchToFront
* Back At Workbench

	bsr	OpenAndCloseWorkbenchWindow

* Bring it all back
	;bump the task priority up to max
	move.l	_OurTaskBase,a1
	moveq	#TASKPRIORITY,d0			;Set Task priority
	CALL	_SysBase,SetTaskPri
	move.w	d0,OldTaskPri

	move.w	#ER_OK,ErrorCode

	IFNE	IHANDLER
	bsr	_SetupInputHandler
	tst.w	d0
	;bmi	ShutDown
	move.w	#1,InputHandlerStatus
	ENDC

	move.w	#ER_OK,ErrorCode

	FLUSHVIEW

	bsr	_KillSprites

* ACTION
	lea	_custom,a6
	move.l	#_DummyCopper,cop1lc(a6)

	bsr	_Spam_Init
	tst.w	d0
	;beq	.ShutDown

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

	;bsr	SetupSlices
	;bsr	SliceCopperUpdate

	lea	_custom,a6
	bsr	_WaitBlit
	move.w	#$FFFF,bltafwm(a6)
	move.w	#$0000,bltalwm(a6)

	;lea	FredSFX,a0
	;move.w	#0,d0
	;bsr	_PlaySound

	;Delete This Some Time
	;move.l	_FirstSlice,a0
	;move.w	#DEBUG_WINDOW_WIDTH,d0
	;move.w	#DEBUG_WINDOW_HEIGHT*8,d1
	;move.w	#$FF,d2				;Initial Colour
	;bsr	_OpenNewDebugWindow

	;lea	_MainMenu,a0
	;bsr	_NewMenu

	;lea	_custom,a0
	;cmp.w	#LACED,Mode
	;bra	.Cont
	;tst.w	vposr(a0)
	;bmi	.Cont
	;move.l	#0,d0
	;move.l	#SIGBREAKF_CTRL_C,d1
	;CALL	_SysBase,SetSignal
.Cont
	;move.w	#1,MainLoopDone
	;move.l	#SIGBREAKF_CTRL_C,d0		;Wait for Ctrl_C sent from the
	;CALL	_SysBase,Wait			;Copper interrupt server.

	lea	_LameReq,a0
	move.l	lr_CopperList1(a0),cop1lc(a6)
	rts

OpenAndCloseWorkbenchWindow
	;Inputs:	NULL
	;Returns:	NULL

	lea	WB_NewWindow,a0
	CALL	_IntuitionBase,OpenWindow
	move.l	d0,WB_Window
	beq	WB_Abort

	move.l	WB_Window,a0		;get IDCMP port
	move.l	wd_UserPort(a0),a0
	move.l	a0,WB_MsgPort

	move.b	MP_SIGBIT(a0),d1
	move.l	#1,d0
	lsl.l	d1,d0
	move.l	d0,WB_SigBit


WB_WaitLoop
	move.l	WB_SigBit,d0
	CALL	_SysBase,Wait

WB_GetLoop
	move.l	WB_MsgPort,a0
	CALL	_SysBase,GetMsg
	tst.l	d0
	beq	WB_WaitLoop
	move.l	d0,a0

	move.l	im_Class(a0),WB_Class

	move.l	a0,a1
	CALL	_SysBase,ReplyMsg

	move.l	WB_Class,d0
	and.l	#CLOSEWINDOW,d0
	bne	WB_Exit
	bra	WB_GetLoop

WB_Exit
	move.l	WB_Window,a0
	CALL	_IntuitionBase,CloseWindow

WB_Abort
	rts

*************************************************************************
**				Various Data Bits		       **
*************************************************************************
	SECTION data,DATA

_CurrentMenu		dc.l	0

_MainMenu		dc.l	InitMainMenu
			dc.l	UpdateMainMenu
			dc.l	NULL

MainMenuText		dc.b	"Main Menu",10
			dc.b	"F1. Do Something",10
			dc.b	"F2. Do Something Else",10
			dc.b	"F3. Do Something Other",10
			dc.b	"F4. Back To Workbench",10
			dc.b	0
			even

*************************   Debug Data   ********************************

DW_Base			ds.l	1
DW_Size			ds.l	1
DW_Width		ds.w	1
DW_Height		ds.w	1
DW_Colour		ds.w	1
DW_Slice		ds.l	1
;Blitting Info
DW_DrawBltSize		ds.w	1
DW_DrawBufferMod	ds.w	1
DW_WipeBltSize		ds.w	1
DW_WipeBufferMod	ds.w	1
DW_BltData0		ds.b	DWBLIT_SIZEOF
DW_BltData1		ds.b	DWBLIT_SIZEOF
_DebugWindowX		ds.w	1		;Unshifted Pixels
_DebugWindowY		ds.w	1		;Unshifted Pixels

WB_Window	ds.l	1
WB_MsgPort	ds.l	1
WB_SigBit	ds.l	1
WB_Class	ds.l	1
WB_NewWindow
	dc.w	0				;Left
	dc.w	11				;Top
	dc.w	200				;Width
	dc.w	12				;Height
	dc.b	0				;Detail Pen
	dc.b	1				;BlockPen
	dc.l	CLOSEWINDOW
	dc.l	WINDOWCLOSE+WINDOWDRAG+WINDOWDEPTH
	dc.l	0		;gadgets
	dc.l	0		;checkmark
	dc.l	WB_WindowTitle	;title
	dc.l	WBENCHSCREEN	;screen
	dc.l	0		;bitmap
	dc.w	50
	dc.w	25
	dc.w	200
	dc.w	100
	dc.w	$0001

WB_WindowTitle	dc.b	"Its...",0
		even

*************************   GroovyCon Data   ****************************

_DebugGroovyCon		ds.b	gc_SIZEOF

_DebugGroovyConDevice	dc.l	_DebugGroovyCon		;cd_ConsoleStruct
			dc.l	GC_PutChar		;cd_PutChar
			dc.l	ClearDebugWindow	;cd_Clear
			dc.l	GC_CursorUp		;cd_CursorUp
			dc.l	GC_CursorDown		;cd_CursorDown
			dc.l	GC_CursorLeft		;cd_CursorLeft
			dc.l	GC_CursorRight		;cd_CursorRight
			dc.l	GC_CursorSetX		;cd_CursorSetX
			dc.l	GC_CursorSetY		;cd_CursorSetY

_StatBoxGroovyCon	ds.b	gc_SIZEOF

_StatBoxGroovyConDevice	dc.l	_StatBoxGroovyCon	;cd_ConsoleStruct
			dc.l	GC_PutChar		;cd_PutChar
			dc.l	0			;cd_Clear
			dc.l	GC_CursorUp		;cd_CursorUp
			dc.l	GC_CursorDown		;cd_CursorDown
			dc.l	GC_CursorLeft		;cd_CursorLeft
			dc.l	GC_CursorRight		;cd_CursorRight
			dc.l	GC_CursorSetX		;cd_CursorSetX
			dc.l	GC_CursorSetY		;cd_CursorSetY

_InsertVolumeGroovyCon	ds.b	gc_SIZEOF

_InsertVolumeGroovyConDevice
			dc.l	_InsertVolumeGroovyCon	;cd_ConsoleStruct
			dc.l	GC_PutChar		;cd_PutChar
			dc.l	0			;cd_Clear
			dc.l	GC_CursorUp		;cd_CursorUp
			dc.l	GC_CursorDown		;cd_CursorDown
			dc.l	GC_CursorLeft		;cd_CursorLeft
			dc.l	GC_CursorRight		;cd_CursorRight
			dc.l	GC_CursorSetX		;cd_CursorSetX
			dc.l	GC_CursorSetY		;cd_CursorSetY
