		incdir "Devpac:Include/"
		include	"exec/exec.i"
		include	"dos/dos.i"
		include	"dos/dosextens.i"
		include	"graphics/gfxbase.i"
		include	"devices/audio.i"

		include	"resources/cia_lib.i"
		include	"exec/exec_lib.i"
		include	"dos/dos_lib.i"
		include	"graphics/graphics_lib.i"

CALL	MACRO
	move.l	\1,a6
	jsr	_LVO\2(a6)
	ENDM

_SysBase	equ	4
_custom		equ	$dff000

	XREF	_StartPlayerSubTask,_EndPlayerSubTask
	XREF	_CIAABase,_ModulePlayerEnable
	XREF	_InitCIAInterrupt,_StopCIAInterrupt
	XREF	_InitModulePlayer,_StopModulePlayer

	XDEF	_DOSBase,_GfxBase,_stdout,_OurTaskBase
Start
	move.l	a0,CommandLine
	move.l	d0,CommandLineSize
	add.l	#1,CommandLineSize
	move.l	#0,_DOSBase
	move.l	#0,Handle
	move.l	#0,FileSize
	move.l	#0,FileBuf
	move.l	#0,_CIAABase

	lea	DOSName,a1
	moveq.l	#0,d0
	CALL	_SysBase,OpenLibrary
	move.l	d0,_DOSBase
	beq	ShutDown

	CALL	_DOSBase,Output		;Will return d0.l as handler
	move.l	d0,_stdout

	cmp.l	#2,CommandLineSize
	bne	.GotCommandLine
	lea	UsageError,a0
	bsr	_ShellPrint
	bra	ShutDown
.GotCommandLine

	lea	GfxName,a1
	moveq.l	#0,d0
	CALL	_SysBase,OpenLibrary
	move.l	d0,_GfxBase
	beq	ShutDown

	move.l	CommandLineSize,d0
	move.l	#MEMF_ANY,d1
	CALL	_SysBase,AllocMem
	move.l	d0,FileName
	bne	.GotNameMem
	lea	FileNameMemError,a0
	bsr	_ShellPrint
	bra	ShutDown
.GotNameMem
	move.l	CommandLineSize,d0
	sub.w	#3,d0
	move.l	CommandLine,a0
	move.l	FileName,a1
.NameLoop
	move.b	(a0)+,(a1)+
	dbra	d0,.NameLoop
	move.b	#0,(a1)

	;Open the File
	move.l	FileName,d1
	move.l	#MODE_OLDFILE,d2
	CALL	_DOSBase,Open
	move.l	d0,Handle
	bne	.OpenedFile
	lea	OpenFileError,a0
	bsr	_ShellPrint
	bra	ShutDown
.OpenedFile

	move.l	Handle,d1
	move.l	#0,d2
	move.l	#OFFSET_END,d3
	CALL	_DOSBase,Seek

	move.l	Handle,d1
	move.l	#0,d2
	move.l	#OFFSET_BEGINNING,d3
	CALL	_DOSBase,Seek
	move.l	d0,FileSize

	move.l	FileSize,d0
	move.l	#MEMF_CHIP,d1
	CALL	_SysBase,AllocMem
	move.l	d0,FileBuf
	bne	.GotFileMem
	lea	FileMemError,a0
	bsr	_ShellPrint
	bra	ShutDown
.GotFileMem

	move.l	FileBuf,d0
	add.l	FileSize,d0
	move.l	d0,FileEnd

	move.l	Handle,d1
	move.l	FileBuf,d2
	move.l	FileSize,d3
	CALL	_DOSBase,Read
	cmp.l	FileSize,d0
	beq	.ReadOK
	lea	FileReadError,a0
	bsr	_ShellPrint
	bra	ShutDown
.ReadOK
	move.l	#0,a1
	CALL	_SysBase,FindTask
	move.l	d0,_OurTaskBase

	move.l	FileBuf,a0
	move.l	FileEnd,a1
	bsr	_InitModulePlayer

	bsr	_StartPlayerSubTask
	tst.w	d0
	beq	ShutDown

	bsr	_InitCIAInterrupt
	tst.l	_CIAABase
	bne	.CIAOkay
	lea	CIAErrorTxt,a0
	bsr	_ShellPrint
	bra	ShutDown
.CIAOkay

	move.b	#1,_ModulePlayerEnable
.ClickWait
	CALL	_GfxBase,WaitTOF
	btst.b	#6,$bfe001
	bne	.ClickWait

	bsr	_StopModulePlayer

ShutDown
	tst.l	_CIAABase
	beq	.NoCIAInt
	bsr	_StopCIAInterrupt
.NoCIAInt
	bsr	_EndPlayerSubTask

	move.l	FileBuf,a1
	cmp.l	#0,a1
	beq	.NoFileBuf
	move.l	FileSize,d0
	CALL	_SysBase,FreeMem
.NoFileBuf
	move.l	Handle,d1
	beq	.FileNotOpen
	CALL	_DOSBase,Close
.FileNotOpen
	move.l	FileName,a1
	cmp.l	#0,a1
	beq	.NoFileName
	move.l	CommandLineSize,d0
	CALL	_SysBase,FreeMem
.NoFileName
	move.l	_GfxBase,a1
	cmp.l	#0,a1
	beq	.GfxNotOpen
	CALL	_SysBase,CloseLibrary
.GfxNotOpen
	move.l	_DOSBase,a1
	cmp.l	#0,a1
	beq	.DosNotOpen
	CALL	_SysBase,CloseLibrary
.DosNotOpen
	move.l	#0,d0
	rts

_ShellPrint	;Print 0 terminated String to the Shell
		;Inputs  : a0.l = Text Base
		;Outputs : Nil

	movem.l	d2-d3/a6,-(sp)
	move.l	a0,d2			;Copy Base of text for later use
	bsr	.GetTextLength
	move.l	d0,d3			;Store length for later use
	move.l	_stdout,d1		;d1.l = File (Handler)
					;d2.l = Base of Text
					;d3.l = Length
	CALL	_DOSBase,Write		;Print It.
	movem.l	(sp)+,d2-d3/a6
	rts

.GetTextLength	;Returns the length of any 0 terminated String
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

***************************************************************

	SECTION MyVars,BSS
FileName	ds.l	1
AudioPort	ds.l	1
MainIOReq	ds.l	1
Chan0IOReq	ds.l	1
Chan1IOReq	ds.l	1
Chan2IOReq	ds.l	1
Chan3IOReq	ds.l	1
Handle		ds.l	1
FileSize	ds.l	1
FileBuf		ds.l	1
FileEnd		ds.l	1
CommandLine	ds.l	1
CommandLineSize	ds.l	1

_DOSBase	ds.l	1
_GfxBase	ds.l	1
_stdout		ds.l	1
_OurTaskBase	ds.l	1

	SECTION MyData,DATA

Channels	dc.b	%1111
		even
DOSName		dc.b	"dos.library",0
GfxName		dc.b	"graphics.library",0
AudioDevName	dc.b	"audio.device",0
AudioDeviceError dc.b	"Can't Open 'audio.device'",10,0
CIAErrorTxt	dc.b	"No CIA interrupt available.",10,0
OpenFileError	dc.b	"Can't Open File",10,0
FileMemError	dc.b	"Not Enough Chip Memory",10,0
FileNameMemError dc.b	"Not Enough Memory",10,0
FileReadError	dc.b	"Error Reading File",10,0
UsageError	dc.b	"MyPlay - By Grant R. Young",10
		dc.b	"Usage: MyPlay <module>",10,0
		even
