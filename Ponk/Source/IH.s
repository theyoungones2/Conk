*************************************************************************
*									*
*			The Input Handler				*
*			-----------------				*
*  Right this is a run down on what the routines contained forthwith	*
* actually do.								*
*  The general idea is that we want to stop the input stream from	*
* trickling down into all the little programs that could just mess	*
* things up for us, and lets face it, these other programs most		*
* probably aren't as important as us, so we can feel quite just in	*
* letting them do nothing for a while, 'cos chances are they weren't	*
* doing very much before anyway.					*
*  So how do we do this I here you say. Well this is the clever, and	*
* might I say very system friendly, way we do it. First we open the	*
* input.device & then put in a new Input Handler at a fairly high	*
* priority, & this then kills the stream dead in its tracks. Very	*
* clever you say, well you'd be right, but there's more. Seeing as	*
* we have a Handler in place, we might as well make better use of it	*
* than just killing the stream, so we also handle the Keyboard from	*
* the handler.								*
*									*
*  The Keyboard:							*
*  ------------								*
*  For this we have an array which contains a byte for every key on	*
* the keyboard and a few that aren't. These bytes are ordered according	*
* to the Raw Key Code, so to see if the A Key is down you just simply	*
* do thus:	Key_A		equ	40				*
*									*
*		lea	_RawKeyArray,a0					*
*		tst.b	Key_A(a0)					*
*		bne	A_Is_Down					*
* And thats all there is to it.						*
*									*
*  We also have a buffer which contains the last KEY_BUFFER_SIZE 	*
* RawKey Events. Only the Key Downs are stored. The routines for this	*
* are:	PutToRawKeyBuffer    - Add a key to the tail of the buffer	*
*	PullFromRawKeyBuffer - Pull a key from the head of the buffer	*
*	ClearRawKeyBuffer    - Clear the Buffer				*
* WARNING: A NULL RawKey = $FF and not $00				*
*									*
* And lastly we have the ASCII Buffer which works just the same as the	*
* RawKey Buffer except you replace the word Raw with ASCII.		*
* NOTE: A Null ASCII = $00						*
*									*
*************************************************************************

 	incdir	"PConk:inc/"
	include	"Source/Libs.gs"
	include	"Source/header.i"
	include	"Source/Constants.i"
	include	"Source/RawKeyCodes.i"

	incdir	"devpac:include/"
	include	"exec/io.i"
	include	"devices/input.i"
	include	"devices/inputevent.i"
	include	"devices/console.i"
	include	"devices/console_lib.i"

	XDEF	_SetupInputHandler,_KillInputHandler,_RawMouseY,_RawMouseX
	XDEF	_PutToRawKeyBuffer,_PullFromRawKeyBuffer,_ClearRawKeyBuffer
	XDEF	_PutToASCIIKeyBuffer,_PullFromASCIIKeyBuffer,_ClearASCIIKeyBuffer
	XDEF	_RawKeyArray,_RawKeyBuffer,_ASCIIKeyBuffer,_WaitForClearRawKeys
	XDEF	_DiskChanged

	XREF	_CreatePort,_DeletePort,_CreateExtIO,_DeleteExtIO,_ShellPrintNumber
	XREF	_SysBase,_custom

KEY_BUFFER_SIZE		equ	10


	IFNE	CHIPASSEMBLE
	SECTION	WhatLovelyCode,code_C
	ELSE
	SECTION	WhatLovelyCode,code
	ENDC

_SetupInputHandler
	;Inputs:	NULL
	;Returns;	NULL

	bsr	_ClearRawKeyBuffer
	bsr	_ClearASCIIKeyBuffer

	move.l	#0,-(sp)	;pri
	move.l	#0,-(sp)	;name
	jsr	_CreatePort	;port returned in d0
	addq.l	#8,sp
	move.l	d0,InputPort
	beq	.Exit

	move.l	#IOSTD_SIZE,-(sp)
	move.l	InputPort,-(sp)
	jsr	_CreateExtIO	;block returned in d0
	addq.l	#8,sp
	move.l	d0,InputIORequest
	beq	.FreeInputPort

	lea	InputDeviceName,a0
	move.l	#0,d0
	move.l	InputIORequest,a1
	move.l	#0,d1
	CALL	_SysBase,OpenDevice
	tst.l	d0
	bne	.FreeInputIOReq

	;Right we have the Input Device all going, so now to get the Console Device
	;going as well. We need this for the RawKeyConvert() function.

	move.l	#0,-(sp)	;pri
	move.l	#0,-(sp)	;name
	jsr	_CreatePort	;port returned in d0
	addq.l	#8,sp
	move.l	d0,ConsolePort
	beq	.FreeInputDevice

	move.l	#IOSTD_SIZE,-(sp)
	move.l	ConsolePort,-(sp)
	jsr	_CreateExtIO	;block returned in d0
	addq.l	#8,sp
	move.l	d0,ConsoleIORequest
	beq	.FreeConsolePort

	lea	ConsoleDeviceName,a0
	move.l	#-1,d0
	move.l	ConsoleIORequest,a1
	move.l	#0,d1
	CALL	_SysBase,OpenDevice
	tst.l	d0
	bne	.FreeConsoleIOReq

	move.l	ConsoleIORequest,a0
	move.l	IO_DEVICE(a0),_ConsoleDevice

	;Set up our Input Handler
	lea	InputInterruptStruct,a0
	move.l	#MyInputHandler,IS_CODE(a0)
	move.l	#0,IS_DATA(a0)
	move.b	#54,LN_PRI(a0)
	move.l	#InputInterruptName,LN_NAME(a0)

	move.l	InputIORequest,a0
	move.l	#InputInterruptStruct,IO_DATA(a0)
	move.w	#IND_ADDHANDLER,IO_COMMAND(a0)
	move.l	a0,a1
	CALL	_SysBase,DoIO

	move.l	#0,d0
	rts

.FreeConsoleIOReq
	move.l	#IOSTD_SIZE,-(sp)
	move.l	ConsoleIORequest,-(sp)
	jsr	_DeleteExtIO
	addq.l	#8,sp
.FreeConsolePort
	move.l	ConsolePort,-(sp)
	jsr	_DeletePort
	addq.l	#4,sp
.FreeInputDevice
	move.l	InputIORequest,a1
	CALL	_SysBase,CloseDevice
.FreeInputIOReq
	move.l	#IOSTD_SIZE,-(sp)
	move.l	InputIORequest,-(sp)
	jsr	_DeleteExtIO
	addq.l	#8,sp
.FreeInputPort
	move.l	InputPort,-(sp)
	jsr	_DeletePort
	addq.l	#4,sp
.Exit
	move.l	#-1,d0			;Give error Return code
	rts


_KillInputHandler
	;Kill The Handler

.Loop2
	;move.w	#KEY_BUFFER_SIZE-1,d1
.Loop
	;lea	_RawKeyBuffer,a0
	;move.b	(a0)+,d0
	;bne	.Loop2
	;dbra	d1,.Loop

	move.l	InputIORequest,a0
	move.l	#InputInterruptStruct,IO_DATA(a0)
	move.w	#IND_REMHANDLER,IO_COMMAND(a0)

	move.l	a0,a1
	CALL	_SysBase,DoIO

	;Free the Console device
	move.l	ConsoleIORequest,a1
	CALL	_SysBase,CloseDevice

	move.l	#IOSTD_SIZE,-(sp)
	move.l	ConsoleIORequest,-(sp)
	jsr	_DeleteExtIO
	addq.l	#8,sp

	move.l	ConsolePort,-(sp)
	jsr	_DeletePort
	addq.l	#4,sp

	;Now Input Device
	move.l	InputIORequest,a1
	CALL	_SysBase,CloseDevice

	move.l	#IOSTD_SIZE,-(sp)
	move.l	InputIORequest,-(sp)
	jsr	_DeleteExtIO
	addq.l	#8,sp

	move.l	InputPort,-(sp)
	jsr	_DeletePort
	addq.l	#4,sp

	rts

MyInputHandler
	;Inputs:	a0.l = Input Events
	;		a1.l = Handler Data
	;Returns:	d0.l = New Input Events

;	bra	.Exit

	cmp.l	#0,a0
	beq	.Exit

	move.l	a0,EventBase
.EventLoop
	cmp.b	#IECLASS_RAWKEY,ie_Class(a0)
	bne	.NotRawKey
	bsr	RawKeyEvent
	bra	.NextEvent
.NotRawKey
	cmp.b	#IECLASS_RAWMOUSE,ie_Class(a0)
	bne	.NotRawMouse
	bsr	RawMouseEvent
	bra	.NextEvent
.NotRawMouse
	cmp.b	#IECLASS_DISKREMOVED,ie_Class(a0)
	bne	.NotDiskRemoved
	move.w	#1,_DiskChanged
	move.l	EventBase,d0
	bra	.RealExit
.NotDiskRemoved
	cmp.b	#IECLASS_DISKINSERTED,ie_Class(a0)
	bne	.NotDiskInserted
	move.w	#1,_DiskChanged
	move.l	EventBase,d0
	bra	.RealExit
.NotDiskInserted

.NextEvent
	move.l	ie_NextEvent(a0),a0
	cmp.l	#0,a0
	bne	.EventLoop
.Exit
	move.l	#0,d0				;Kill the Events
.RealExit
	rts

RawKeyEvent
	move.w	ie_Code(a0),d0
	move.w	d0,d1
	and.w	#$007F,d0			;And Out Up/Down Bit
	tst.w	d0
	blt	.ConvertToASCII
	cmp.w	#IECODE_KEY_CODE_LAST,d0
	bge	.ConvertToASCII
	lsr.w	#7,d1				;Move Up/Down bit down to the lowest bit
	eor.w	#1,d1				;Make 1 when down, and 0 when up
	lea	_RawKeyArray,a1
	move.b	d1,(a1,d0.w)
	beq	.ConvertToASCII
	move.l	a0,-(sp)
	move.b	d1,d0
	bsr	_PutToRawKeyBuffer
	move.l	(sp)+,a0

.ConvertToASCII
	move.l	a6,-(sp)
	movem.l	a0-a2,-(sp)
	lea	_ASCIIKeyBuffer,a1
	move.w	#0,d0
.FindFreeLoop
	cmp.b	#$00,(a1)+
	beq	.FoundOne
	add.w	#1,d0
	cmp.w	#KEY_BUFFER_SIZE,d0
	bne	.FindFreeLoop
	beq	.Exit
.FoundOne
	sub.l	#1,a1
	move.w	#KEY_BUFFER_SIZE,d1
	sub.w	d0,d1
	move.l	#0,a2
	CALL	_ConsoleDevice,RawKeyConvert
.Exit
	movem.l	(sp)+,a0-a2
	move.l	(sp)+,a6
	rts

RawMouseEvent
	move.w	ie_X(a0),d0
	add.w	d0,_RawMouseX
	move.w	ie_Y(a0),d0
	add.w	d0,_RawMouseY
	rts

_PutToRawKeyBuffer
	;Inputs:	d0.b = Raw KeyCode to add
	;Returns:	NULL

	lea	_RawKeyBuffer,a0
	move.w	#0,d1
.Loop
	cmp.b	#$FF,(a0,d1.w)
	beq	.GotFreeOne
	add.w	#1,d1
	cmp.w	#KEY_BUFFER_SIZE,d1
	bne	.Loop
	bra	.Exit
.GotFreeOne
	move.b	d0,(a0,d1.w)
.Exit
	rts

_PullFromRawKeyBuffer
	;Inputs:	NULL
	;Returns:	d0.b = Raw Key Code : $FF = No Key

	lea	_RawKeyBuffer,a0
	move.b	(a0),d0
	move.w	#KEY_BUFFER_SIZE-1,d1
.Loop
	move.b	1(a0),(a0)+
	dbra	d1,.Loop
	move.b	#$FF,-1(a0)
	rts

_ClearRawKeyBuffer
	;Inputs:	NULL
	;Returns:	NULL

	lea	_RawKeyBuffer,a0
	move.w	#KEY_BUFFER_SIZE-1,d0
.InitBufferLoop
	move.b	#$FF,(a0)+
	dbra	d0,.InitBufferLoop
	rts

_PutToASCIIKeyBuffer
	;Inputs:	d0.b = Raw KeyCode to add
	;Returns:	NULL

	lea	_ASCIIKeyBuffer,a0
	move.w	#0,d1
.Loop
	cmp.b	#$00,(a0,d1.w)
	beq	.GotFreeOne
	add.w	#1,d1
	cmp.w	#KEY_BUFFER_SIZE,d1
	bne	.Loop
	bra	.Exit
.GotFreeOne
	move.b	d0,(a0,d1.w)
.Exit
	rts

_PullFromASCIIKeyBuffer
	;Inputs:	NULL
	;Returns:	d0.b = Raw Key Code : $00 = No Key

	lea	_ASCIIKeyBuffer,a0
	move.b	(a0),d0
	move.w	#KEY_BUFFER_SIZE-1,d1
.Loop
	move.b	1(a0),(a0)+
	dbra	d1,.Loop
	move.b	#$00,-1(a0)
	rts

_ClearASCIIKeyBuffer
	;Inputs:	NULL
	;Returns:	NULL
	lea	_ASCIIKeyBuffer,a0
	move.w	#KEY_BUFFER_SIZE-1,d0
.InitBufferLoop
	move.b	#$00,(a0)+
	dbra	d0,.InitBufferLoop
	rts

_WaitForClearRawKeys
	lea	_RawKeyArray,a0
.StartAgain
	move.w	#0,d0
.ClearLoop
	cmp.w	#Key_CapsLock,d0
	beq	.Next
	tst.b	(a0,d0.w)
	bne	.StartAgain
.Next
	add.w	#1,d0
	cmp.w	#IECODE_KEY_CODE_LAST,d0
	bne	.ClearLoop
	rts


*************************************************************************
	SECTION	Vars,BSS

EventBase		ds.l	1
_ConsoleDevice		ds.l	1
ConsolePort		ds.l	1
ConsoleIORequest	ds.l	1
InputPort		ds.l	1
InputIORequest		ds.l	1
InputInterruptStruct	ds.b	IS_SIZE
_RawKeyArray		ds.b	IECODE_KEY_CODE_LAST
		even
_RawKeyBuffer		ds.b	KEY_BUFFER_SIZE
		even
_ASCIIKeyBuffer		ds.b	KEY_BUFFER_SIZE
			ds.b	1		;Kill This (was just for printing the string)
		even
ByteBuffer		ds.b	1
		even
_RawMouseX	ds.w	1
_RawMouseY	ds.w	1
_DiskChanged	ds.w	1	;If TRUE a disk has been added/removed at some stage.
				;Zero this after reading it.
 
*************************************************************************
	SECTION	Data,DATA

InputDeviceName
	dc.b	"input.device",0
	even
ConsoleDeviceName
	dc.b	"console.device",0
	even
InputInterruptName
	dc.b	"Mr Blobby",0
	even
