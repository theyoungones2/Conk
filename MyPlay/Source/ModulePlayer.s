	OPT NOCHKIMM

;********************************************
;* ----- Protracker V1.1B Playroutine ----- *
;* Lars "Zap" Hamre/Amiga Freelancers 1991  *
;* Bekkeliveien 10, N-2010 STRØMMEN, Norway *
;********************************************
; Several Bug fixes, amd now uses the audio device.

		incdir	"Devpac:Include/"
		include "exec/types.i"
		include "exec/ables.i"
		include "exec/execbase.i"
		include	"graphics/gfxbase.i"
		include "hardware/custom.i"
		include	"dos/dos.i"
		include	"dos/dosextens.i"

		include	"resources/cia_lib.i"
		include	"exec/exec_lib.i"
		include	"dos/dos_lib.i"
		include	"graphics/graphics_lib.i"


  STRUCTURE SpamParam,0
	STRUCT	spam_PackedName,10
	BYTE	spam_Pri
	UBYTE	spam_Volume
	UWORD	spam_SampleRate
	ULONG	spam_Length
	LONG	spam_RepeatStart
	ULONG	spam_RepeatLength
	UWORD	spam_Flags
	WORD	spam_Repeats
	STRUCT	spam_Reserved,4*2
	LABEL	spam_SIZEOF

CALL	MACRO
	move.l	\1,a6
	jsr	_LVO\2(a6)
	ENDM

_SysBase	equ	4
_custom		equ	$dff000

NoteInfo	equ     $00
NoteInit	equ     $04
NoteFinish      equ     $08
NoteStart       equ     $0c
NoteStop	equ     $10
NoteFreqVol     equ     $14
NoteFreq	equ     $18
NoteVol		equ     $1c

;---- cia interrupt ----

ciatalo = $400
ciatahi = $500
ciatblo = $600
ciatbhi = $700
ciacra  = $e00
ciacrb  = $f00

	XREF	_GfxBase
	XREF	_Spam_Init,_Spam_Die,_Spam_PlaySound
	XREF	_playertask

	XDEF	_InitCIAInterrupt,_StopCIAInterrupt
	XDEF	_InitModulePlayer,_UpdateModulePlayer,_StopModulePlayer
	XDEF	_CIAABase,_ModulePlayerEnable,_SongDataPtr

_InitCIAInterrupt
	;Inputs:	NIL
	;Outputs:	_CIAABase will be not zero if everything goes ok.
	movem.l	d2-d7/a2-a6,-(sp)
	lea	$bfe001,a5
	move.l	#0,_CIAABase
	moveq	#0,d0
	lea	CIAAName,a1
	CALL	_SysBase,OpenResource
	move.l	d0,_CIAABase
	beq	.Exit

	move.l	_GfxBase,a1
	move.w	gb_DisplayFlags(a1),d0	; displayflags
	move.l	#1773447,d7		; PAL Clock Speed
	btst	#PALn,d0		; pal?
	beq	.IsPAL
	move.l	#1789773,d7 ; ntsc
.IsPAL
	move.l	d7,TimerValue
	divu	#125,d7 ; default to normal 50 hz timer

.TryTimerA
	lea	MusicIntServer,a1
	moveq	#0,d0	; bit 0: timer a
	CALL	_CIAABase,AddICRVector
	clr.l	TimerFlag
	tst.l	d0
	bne	.TryTimerB
	move.l	a5,CIAAAddr
	move.b	d7,ciatalo(a5)
	lsr.w	#8,d7
	move.b	d7,ciatahi(a5)
	bset	#0,ciacra(a5)
	bra	.Exit
.TryTimerB
	lea	MusicIntServer,a1
	moveq	#1,d0	; bit 1: timer b
	CALL	_CIAABase,AddICRVector
	move.l	#1,TimerFlag
	tst.l	d0
	bne	.Exit
	move.l	a5,CIAAAddr
	move.b	d7,ciatblo(a5)
	lsr.w	#8,d7
	move.b	d7,ciatbhi(a5)
	bset	#0,ciacrb(a5)
.Exit
	movem.l	(sp)+,d2-d7/a2-a6
	rts

_StopCIAInterrupt
	movem.l	d2-d7/a2-a6,-(sp)
	tst.l	_CIAABase
	beq	.Exit
	move.l	CIAAAddr,a5
	tst.l	TimerFlag
	beq	.StopTimerA
.StopTimerB
	bclr	#0,ciacrb(a5)
	moveq	#1,d0
	bra	.RemoveInterrupt
.StopTimerA
	bclr	#0,ciacra(a5)
	moveq	#0,d0
.RemoveInterrupt
	lea	MusicIntServer,a1
	moveq	#0,d0
	CALL	_CIAABase,RemICRVector
	clr.l	_CIAABase
.Exit
	movem.l	(sp)+,d2-d7/a2-a6
	rts

_CIAIntHandler
	;a1 = Data Area
	;a5 = Code Ptr
	movem.l	d0-d7/a0-a6,-(sp)
	move.l	#SIGBREAKF_CTRL_C,d0	;Signal the Main Loop to go for it.
	move.l	_playertask,a1
	cmp.l	#0,a1
	beq	.PlayerTaskNotRunning
	CALL	_SysBase,Signal
.PlayerTaskNotRunning
	movem.l	(sp)+,d0-d7/a0-a6
	rts

;---- tempo ----

SetTempo
	movem.l	d0-d2/a0-a4,-(sp)
	move.l	_CIAABase,d2
	beq	.Exit
	cmp.w	#32,d0
	bhs.s	.setemsk
	moveq	#32,d0
.setemsk
	move.w	d0,RealTempo
	move.l	TimerValue,d2
	divu	d0,d2
	move.l	CIAAAddr,a4
	move.l	TimerFlag,d0
	beq.s	.settema
	move.b	d2,ciatblo(a4)
	lsr.w	#8,d2
	move.b	d2,ciatbhi(a4)
	bra	.Exit
.settema
	move.b	d2,ciatalo(a4)
	lsr.w	#8,d2
	move.b	d2,ciatahi(a4)
.Exit
	movem.l	(sp)+,d0-d2/a0-a4
	rts

n_note		EQU	0  ; W
n_cmd		EQU	2  ; W
n_cmdlo		EQU	3  ; B
n_start		EQU	4  ; L
n_length	EQU	8  ; W
n_loopstart	EQU	10 ; L
n_replen	EQU	14 ; W
n_period	EQU	16 ; W
n_finetune	EQU	18 ; B
n_volume	EQU	19 ; B
n_dmabit	EQU	20 ; W
n_toneportdirec	EQU	22 ; B
n_toneportspeed	EQU	23 ; B
n_wantedperiod	EQU	24 ; W
n_vibratocmd	EQU	26 ; B
n_vibratopos	EQU	27 ; B
n_tremolocmd	EQU	28 ; B
n_tremolopos	EQU	29 ; B
n_wavecontrol	EQU	30 ; B
n_glissfunk	EQU	31 ; B
n_sampleoffset	EQU	32 ; B
n_pattpos	EQU	33 ; B
n_loopcount	EQU	34 ; B
n_funkoffset	EQU	35 ; B
n_wavestart	EQU	36 ; L
n_reallength	EQU	40 ; W
n_channelno	EQU	42 ; W
n_realper	EQU	44 ; W
n_realvol	EQU	46 ; W

_InitModulePlayer
	;Inputs:	a0.l = Module Start Address
	;		a1.l = Module End Address
	;Outputs:	NIL

	movem.l d2-d3/a2-a3,-(sp)

	MOVE.L  A0,_SongDataPtr
	move.l  a1,a3
	subq.l  #4,a3

	lea	_ModuleSpamParam,a2
	move.w	#1,spam_Flags(a2)	;Set 'IsPeriod' Flag

	moveq   #0,d0		   ; Check for old modules
	moveq   #0,d1
	move.w  #31-1,-(sp)		;Store Num of Instruments for mtloop3 below
	tst.b   470(a0)
	beq.s   1$
	cmp.b   #1,499(a0)
	beq.s   1$
	cmp.l   #'M.K.',1080(a0)
	beq.s   1$
	move.w  #480,d0
	move.w  #484,d1
	move.w  #15-1,(sp)		;Store Num of Instruments for mtloop3 below
1$      move.w  d0,seqadj
	move.w  d1,blkadj

	MOVE.L  A0,A1
	LEA     952(A1),A1
	suba.w  d0,a1			;Skip to Song Positions

	MOVEQ   #127,D0			;Go through all 127 Song Positions
	MOVEQ   #0,D1			;and work out the maximum pattern num.
mtloop  MOVE.L  D1,D2			;d2.l = Max Pattern
	SUBQ.W  #1,D0
mtloop2 MOVE.B  (A1)+,D1
	CMP.B   D2,D1
	BGT.S   mtloop
	DBRA    D0,mtloop2
	ADDQ.B  #1,D2			;Add 1 to d2 to make it the num of Patterns

	LEA     mt_SampleStarts,A1
	ASL.L   #8,D2			;Multiply by 1024 (the size of a pattern)
	ASL.L   #2,D2
	ADD.L   #1084,D2		;Add Offset to Start of pattern data
	ADD.L   A0,D2			;Add Module Base
	MOVE.L  D2,A2			;a2.l = Sample Data Base
	suba.w  blkadj,a2
	move.w  (sp)+,d0		;Get Number of samples
mtloop3
	cmp.l   a2,a3			;Compare sample base to end of file
	blt.s   .inrange
	clr.l   (a2)			;If not in file then clear the first 4 bytes
.inrange
	MOVE.L  A2,(A1)+		;Store Base address into Sample Start table
	MOVEQ   #0,D1
	MOVE.W  42(A0),D1		;d1.l = Sample Length
	beq.b    .nosample
	move.w  42+4(a0),d3		;Get sample repeat point
	add.w   42+6(a0),d3		;add repeat length
	cmp.w   d1,d3			;compare to sample length
	bls.b   .repeat_ok		;skip if ok
	lsr.w   42+4(a0)		;half the repeat point
.repeat_ok
	ASL.L   #1,D1			;double sample length (into bytes)
	move.l  d1,d2			;add to sample base ptr
	ADD.l   d1,a2			;add to sample base ptr
.nosample
	ADD.L   #30,A0			;Skip to next sample info block
	DBRA    D0,mtloop3

	or.b	#2,$bfe001		;Turn Filter Off/On ?
	move.b	#6,mt_speed
	clr.b	mt_counter
	clr.b	mt_SongPos
	clr.w	mt_PatternPos
	bsr	_StopModulePlayer

	moveq   #0,d0
	movem.l (sp)+,d2-d3/a2-a3
	rts

_StopModulePlayer
	sf	_ModulePlayerEnable
	lea	_custom,a0
	clr.w	aud0+ac_vol(a0)
	clr.w	aud1+ac_vol(a0)
	clr.w	aud2+ac_vol(a0)
	clr.w	aud3+ac_vol(a0)
	;move.w	#$f,dmacon(a0)
	rts

* Register conventions:
* d5-d7 = Always available
* a5 = NotePlayer
* a6 = chan#temp

_UpdateModulePlayer
	movem.l d0-d7/a0-a6,-(SP)
	tst.b	_ModulePlayerEnable			;Are we all set to go ?
	beq	mt_exit
	ADDQ.B  #1,mt_counter			;Add one to the mt_counter
	MOVE.B  mt_counter,D0			;This counts up to mt_speed, and
	CMP.B   mt_speed,D0			;then we move onto a new note.
	BLO.S   mt_NoNewNote
	CLR.B   mt_counter
	TST.B   mt_PattDelTime2
	BEQ.S   mt_GetNewNote
	BSR.S   mt_NoNewAllChannels
	BRA     mt_dskip

mt_NoNewNote
	BSR.S   mt_NoNewAllChannels
	BRA     mt_NoNewPosYet

mt_NoNewAllChannels
	lea	_custom+aud0,a5
	lea	mt_chan1temp,a6
	bsr	mt_CheckEfx
	lea	_custom+aud1,a5
	lea	mt_chan2temp,a6
	bsr	mt_CheckEfx
	lea	_custom+aud2,a5
	lea	mt_chan3temp,a6
	bsr	mt_CheckEfx
	lea	_custom+aud3,a5
	lea	mt_chan4temp,a6
	bra	mt_CheckEfx

mt_GetNewNote
	MOVE.L	_SongDataPtr,A0
	LEA	12(A0),A3
	LEA	952(A0),A2      ;Pattern Pos
	suba.w	seqadj,a2
	LEA	1084(A0),A0     ;Pattern Data
	suba.w	blkadj,a0
	MOVEQ	#0,D0
	MOVEQ	#0,D1
	MOVE.B	mt_SongPos,D0
	MOVE.B	0(A2,D0.W),D1
	ASL.L	#8,D1
	ASL.L	#2,D1
	ADD.W	mt_PatternPos,D1
	;clr.w	mt_dmacontemp

	lea	_custom+aud0,a5
	lea	mt_chan1temp,a6
	bsr.s	mt_PlayVoice
	lea	_custom+aud1,a5
	lea	mt_chan2temp,a6
	bsr.s	mt_PlayVoice
	lea	_custom+aud2,a5
	lea	mt_chan3temp,a6
	bsr.s	mt_PlayVoice
	lea	_custom+aud3,a5
	lea	mt_chan4temp,a6
	bsr.s	mt_PlayVoice
	bra	mt_SetDMA

mt_PlayVoice
	;Inputs:	d1.l = Pattern Data
	;		a0.l = Module Base
	;		a5.l = Audio Channels Hardware address
	;		a6.l = Channel Temp Storage Area
	TST.L   (A6)
	BNE.S   mt_plvskip
	BSR     mt_PerNop
mt_plvskip
	MOVE.L  0(A0,D1.L),(A6)	;Copy Pattern Note to Temp storage
	ADDQ.L  #4,D1
	MOVEQ   #0,D2
	MOVE.B  n_cmd(A6),D2	;Glue the sample number together
	AND.B   #$F0,D2
	LSR.B   #4,D2
	MOVE.B  (A6),D0
	AND.B   #$F0,D0
	OR.B    D0,D2		;d2.l = sample number
	BEQ     mt_SetRegs	;Skip the rest of this crap if no sample
	MOVEQ   #0,D3
	LEA     mt_SampleStarts,A1
	MOVE    D2,D4
	SUBQ.L  #1,D2
	ASL.L   #2,D2
	MULU    #30,D4
	MOVE.L  0(A1,D2.L),n_start(A6)
	MOVE.W  0(A3,D4.L),n_length(A6)
	MOVE.W  0(A3,D4.L),n_reallength(A6)
	MOVE.B  2(A3,D4.L),n_finetune(A6)
	moveq	#0,d0
	MOVE.B  3(A3,D4.L),d0
	move.b	d0,n_volume(A6)
	MOVE.w  d0,n_realvol(A6)
	MOVE.W  4(A3,D4.L),D3		 ;Get Repeat Point

	TST.W	D3
	BEQ.S	mt_NoLoop
	MOVE.L	n_start(A6),D2		;Get Note Start
	ASL.W	#1,D3			;Make Repeat Point Byte Offset
	ADD.L	D3,D2			;Add Repeat Point Offset
	MOVE.L	D3,n_loopstart(A6)	;Store the Stuff Out
	MOVE.L	D2,n_wavestart(A6)
	MOVE.W	4(A3,D4.L),D0		;Get Repeat Point
	ADD.W	6(A3,D4.L),D0		;Add Repeat Length
	MOVE.W	D0,n_length(A6)		;Store Repeat End (word) Into Length field
	MOVE.W	6(A3,D4.L),n_replen(A6) ;Save replen
	moveq	#0,d0
	move.b	n_volume(a6),d0
	move.w	d0,n_realvol(a6)
	move.w	d0,8(a5)		;Set Volume
	bra.s	mt_SetRegs

mt_NoLoop
	MOVE.L  n_start(A6),D2
	MOVE.L  #0,n_loopstart(A6)
	MOVE.L  D2,n_wavestart(A6)
	MOVE.W  6(A3,D4.L),n_replen(A6) ; Save replen
	moveq	#0,d0
	move.b	n_volume(a6),d0
	move.w	d0,n_realvol(a6)
	move.w	d0,8(a5)	; set volume
mt_SetRegs
	MOVE.W  (A6),D0
	AND.W   #$0FFF,D0
	BEQ     mt_CheckMoreEfx ; If no note
	MOVE.W  2(A6),D0
	AND.W   #$0FF0,D0
	CMP.W   #$0E50,D0
	BEQ.S   mt_DoSetFineTune
	MOVE.B  2(A6),D0
	AND.B   #$0F,D0
	CMP.B   #3,D0   ; TonePortamento
	BEQ.S   mt_ChkTonePorta
	CMP.B   #5,D0
	BEQ.S   mt_ChkTonePorta
	CMP.B   #9,D0   ; Sample Offset
	BNE.S   mt_SetPeriod
	BSR     mt_CheckMoreEfx
	BRA.S   mt_SetPeriod

mt_DoSetFineTune
	BSR     mt_SetFineTune
	BRA.S   mt_SetPeriod

mt_ChkTonePorta
	BSR     mt_SetTonePorta
	BRA     mt_CheckMoreEfx

mt_SetPeriod
	MOVEM.L D0-D1/A0-A1,-(SP)
	MOVE.W  (A6),D1
	AND.W   #$0FFF,D1
	tst.b   n_finetune(a6)
	beq      mt_noft
	LEA     mt_PeriodTable,A1
	MOVEQ   #0,D0
	MOVEQ   #36,D2
mt_ftuloop
	CMP.W   0(A1,D0.W),D1
	BHS.S   mt_ftufound
	ADDQ.L  #2,D0
	DBRA    D2,mt_ftuloop
mt_ftufound
	MOVEQ   #0,D1
	MOVE.B  n_finetune(A6),D1
	MULU    #36*2,D1
	ADD.L   D1,A1
	MOVE.W  0(A1,D0.W),d1
mt_noft
	move.w  d1,n_period(A6)
	MOVEM.L (SP)+,D0-D1/A0-A1

	MOVE.W  2(A6),D0
	AND.W   #$0FF0,D0
	CMP.W   #$0ED0,D0 ; Notedelay
	BEQ     mt_CheckMoreEfx

	;move.w	n_dmabit(a6),_custom+dmacon
	btst	#2,n_wavecontrol(a6)
	bne.s	mt_vibnoc
	clr.b	n_vibratopos(a6)
mt_vibnoc
	BTST    #6,n_wavecontrol(A6)
	BNE.S   mt_trenoc
	CLR.B   n_tremolopos(A6)
mt_trenoc
	;move.l	n_start(a6),(a5)	; set start
	;move.w	n_length(a6),4(a5)	; set length
	move.w	n_period(a6),d0
	move.w	d0,6(a5)		; set period
	;move.w	n_dmabit(a6),d0
	;or.w	d0,mt_dmacontemp

	movem.l	d0-d7/a0-a6,-(sp)
	lea	_ModuleSpamParam,a0
        move.l	n_start(a6),a1
        moveq	#0,d0
        move.w	n_length(a6),d0
        add.l	d0,d0
	move.l	d0,spam_Length(a0)
	move.l	n_loopstart(a6),spam_RepeatStart(a0)
	moveq	#0,d0
	move.w	n_replen(a6),d0
	cmp.w	#1,d0
	bne.b	.notone
	moveq	#0,d0
.notone
	add.l	d0,d0
	move.l	d0,spam_RepeatLength(a0)
	move.w	#-1,spam_Repeats(a0)

	move.w	n_channelno(a6),d0
	moveq	#0,d1
	move.w	n_period(a6),d1
	move.w	d1,n_realper(a6)
	;swap	d1
	move.w	d1,spam_SampleRate(a0)	;Really Period
	moveq	#0,d1
	;move.b	n_volume(a6),d1
	;move.w	d1,n_realvol(a6)
	move.w	n_realvol(a6),d1
	;lsl.w	#2,d1
	move.b	d1,spam_Volume(a0)
	;jsr	NoteStart(a5)
	bsr	_Spam_PlaySound

	movem.l (sp)+,d0-d7/a0-a6
	BRA     mt_CheckMoreEfx

mt_SetDMA
	;move.w	mt_dmadelay,d0
mt_waitdma
	;dbra	d0,mt_waitdma
	;move.w	mt_dmacontemp,d0
	;or.w	#$8000,d0
	;move.w	d0,$dff096
	;move.w	mt_dmadelay,d0
mt_waitdma2
	;dbra	d0,mt_waitdma2

	lea	$dff000,a5
	;lea	mt_chan4temp,a6
	;move.l	n_loopstart(a6),$d0(a5)
	;move.w	n_replen(a6),$d4(a5)
	;lea	mt_chan3temp,a6
	;move.l	n_loopstart(a6),$c0(a5)
	;move.w	n_replen(a6),$c4(a5)
	;lea	mt_chan2temp,a6
	;move.l	n_loopstart(a6),$b0(a5)
	;move.w	n_replen(a6),$b4(a5)
	;lea	mt_chan1temp,a6
	;move.l	n_loopstart(a6),$a0(a5)
	;move.w	n_replen(a6),$a4(a5)

mt_dskip
	ADD.W   #16,mt_PatternPos
	MOVE.B  mt_PattDelTime,D0
	BEQ.S   mt_dskc
	MOVE.B  D0,mt_PattDelTime2
	CLR.B   mt_PattDelTime
mt_dskc TST.B   mt_PattDelTime2
	BEQ.S   mt_dska
	SUBQ.B  #1,mt_PattDelTime2
	BEQ.S   mt_dska
	SUB.W   #16,mt_PatternPos
mt_dska TST.B   mt_PBreakFlag
	BEQ.S   mt_nnpysk
	SF      mt_PBreakFlag
	MOVEQ   #0,D0
	MOVE.B  mt_PBreakPos,D0
	CLR.B   mt_PBreakPos
	LSL.W   #4,D0
	MOVE.W  D0,mt_PatternPos
mt_nnpysk
	CMP.W   #1024,mt_PatternPos
	BLO.S   mt_NoNewPosYet
mt_NextPosition
	MOVEQ	#0,D0
	MOVE.B	mt_PBreakPos,D0
	LSL.W	#4,D0
	MOVE.W	D0,mt_PatternPos
	CLR.B	mt_PBreakPos
	CLR.B	mt_PosJumpFlag

	move.b	mt_SongPos,d1       ; Fixes and opts
	addq.B	#1,d1
	and.B	#$7F,d1
	move.L	_SongDataPtr,A0
	suba.w	seqadj,a0
	move.b	$3b6(a0),d2
	cmp.b	d2,d1
	blo.s	1$

	move.b	$3b7(a0),d1
	cmp.b	d2,d1		   ; < Fix - range check for song repeat
	blo.s	1$
	moveq	#0,d1
1$      move.b	d1,mt_SongPos

mt_NoNewPosYet
	TST.B   mt_PosJumpFlag
	BNE     mt_NextPosition

mt_exit movem.l (sp)+,d0-d7/a0-a6
	RTS


mt_CheckEfx
	BSR     mt_UpdateFunk
	MOVE.W  n_cmd(A6),D0
	AND.W   #$0FFF,D0
	BEQ.S   mt_PerNop
	MOVE.B  n_cmd(A6),D0
	AND.B   #$0F,D0
	BEQ     mt_Arpeggio
	CMP.B   #1,D0
	BEQ     mt_PortaUp
	CMP.B   #2,D0
	BEQ     mt_PortaDown
	CMP.B   #3,D0
	BEQ     mt_TonePortamento
	CMP.B   #4,D0
	BEQ     mt_Vibrato
	CMP.B   #5,D0
	BEQ     mt_TonePlusVolSlide
	CMP.B   #6,D0
	BEQ     mt_VibratoPlusVolSlide
	CMP.B   #$E,D0
	BEQ     mt_E_Commands
SetBack move.w  n_period(a6),6(a5)
	CMP.B   #7,D0
	BEQ     mt_Tremolo
	CMP.B   #$A,D0
	BEQ     mt_VolumeSlide
mt_Return2
	RTS

mt_PerNop
	move.w  n_period(a6),6(a5)
	RTS


*** mt_Arpeggio - Process arpeggio commands (off-cycles)
mt_Arpeggio
	moveq	#0,d0
	move.b	mt_counter,d0
	divs	#3,d0
	swap	d0
	cmp.w	#0,d0
	beq.s	mt_arpeggio2
	cmp.w	#2,d0
	beq.s	mt_arpeggio1
	moveq	#0,d0
	move.b	n_cmdlo(a6),d0
	lsr.b	#4,d0
	bra.s	mt_arpeggio3

mt_arpeggio1
	moveq	#0,d0
	move.b	n_cmdlo(a6),d0
	and.b	#15,d0
	bra.s	mt_arpeggio3

mt_arpeggio2
	move.w	n_period(a6),d2
	bra.s	mt_arpeggio4

mt_arpeggio3
	asl.w	#1,d0
	moveq	#0,d1
	move.b	n_finetune(a6),d1
	mulu	#36*2,d1
	lea	mt_PeriodTable,a0
	add.l	d1,a0
	moveq	#0,d1
	move.w	n_period(a6),d1
	moveq	#36,d3
mt_arploop
	move.w	(a0,d0.w),d2
	cmp.w	(a0),d1
	bhs.s	mt_arpeggio4
	addq.l	#2,a0
	dbra	d3,mt_arploop
	rts

mt_arpeggio4
	move.w	d2,6(a5)
	rts

mt_FinePortaUp
	TST.B   mt_counter
	BNE     mt_Return2
	MOVE.B  #$0F,mt_LowMask
mt_PortaUp
	MOVEQ   #0,D0
	MOVE.B  n_cmdlo(A6),D0
	AND.B   mt_LowMask,D0
	MOVE.B  #$FF,mt_LowMask
	SUB.W   D0,n_period(A6)
	MOVE.W  n_period(A6),D0
	AND.W   #$0FFF,D0
	CMP.W   #113,D0
	BPL.S   mt_PortaUskip
	AND.W   #$F000,n_period(A6)
	OR.W    #113,n_period(A6)
mt_PortaUskip
	MOVE.W  n_period(A6),D0
	AND.W   #$0FFF,D0
	move.w	d0,6(a5)
	RTS



mt_FinePortaDown
	TST.B   mt_counter
	BNE     mt_Return2
	MOVE.B  #$0F,mt_LowMask
mt_PortaDown
	CLR.W   D0
	MOVE.B  n_cmdlo(A6),D0
	AND.B   mt_LowMask,D0
	MOVE.B  #$FF,mt_LowMask
	ADD.W   D0,n_period(A6)
	MOVE.W  n_period(A6),D0
	AND.W   #$0FFF,D0
	CMP.W   #856,D0
	BMI.S   mt_PortaDskip
	AND.W   #$F000,n_period(A6)
	OR.W    #856,n_period(A6)
mt_PortaDskip
	MOVE.W  n_period(A6),D0
	AND.W   #$0FFF,D0
	move.w	d0,6(a5)
	RTS

mt_SetTonePorta
	MOVE.L  A0,-(SP)
	MOVE.W  (A6),D2
	AND.W   #$0FFF,D2
	MOVEQ   #0,D0
	MOVE.B  n_finetune(A6),D0
	MULU    #36*2,D0
	LEA     mt_PeriodTable,A0
	ADD.L   D0,A0
	MOVEQ   #0,D0
mt_StpLoop
	CMP.W   0(A0,D0.W),D2
	BHS.S   mt_StpFound
	ADDQ.W  #2,D0
	CMP.W   #36*2,D0
	BLO.S   mt_StpLoop
	MOVEQ   #35*2,D0
mt_StpFound
	MOVE.B  n_finetune(A6),D2
	AND.B   #8,D2
	BEQ.S   mt_StpGoss
	TST.W   D0
	BEQ.S   mt_StpGoss
	SUBQ.W  #2,D0
mt_StpGoss
	MOVE.W  0(A0,D0.W),D2
	MOVE.L  (SP)+,A0
	MOVE.W  D2,n_wantedperiod(A6)
	MOVE.W  n_period(A6),D0
	CLR.B   n_toneportdirec(A6)
	CMP.W   D0,D2
	BEQ.S   mt_ClearTonePorta
	BGE     mt_Return2
	MOVE.B  #1,n_toneportdirec(A6)
	RTS

mt_ClearTonePorta
	CLR.W   n_wantedperiod(A6)
	RTS

mt_TonePortamento
	MOVE.B  n_cmdlo(A6),D0
	BEQ.S   mt_TonePortNoChange
	MOVE.B  D0,n_toneportspeed(A6)
	CLR.B   n_cmdlo(A6)
mt_TonePortNoChange
	TST.W   n_wantedperiod(A6)
	BEQ     mt_Return2
	MOVEQ   #0,D0
	MOVE.B  n_toneportspeed(A6),D0
	TST.B   n_toneportdirec(A6)
	BNE.S   mt_TonePortaUp
mt_TonePortaDown
	ADD.W   D0,n_period(A6)
	MOVE.W  n_wantedperiod(A6),D0
	CMP.W   n_period(A6),D0
	BGT.S   mt_TonePortaSetPer
	MOVE.W  n_wantedperiod(A6),n_period(A6)
	CLR.W   n_wantedperiod(A6)
	BRA.S   mt_TonePortaSetPer

mt_TonePortaUp
	SUB.W   D0,n_period(A6)
	MOVE.W  n_wantedperiod(A6),D0
	CMP.W   n_period(A6),D0
	BLT.S   mt_TonePortaSetPer
	MOVE.W  n_wantedperiod(A6),n_period(A6)
	CLR.W   n_wantedperiod(A6)

mt_TonePortaSetPer
	MOVE.W  n_period(A6),D2
	MOVE.B  n_glissfunk(A6),D0
	AND.B   #$0F,D0
	BEQ.S   mt_GlissSkip
	MOVEQ   #0,D0
	MOVE.B  n_finetune(A6),D0
	MULU    #36*2,D0
	LEA     mt_PeriodTable,A0
	ADD.L   D0,A0
	MOVEQ   #0,D0
mt_GlissLoop
	CMP.W   0(A0,D0.W),D2
	BHS.S   mt_GlissFound
	ADDQ.W  #2,D0
	CMP.W   #36*2,D0
	BLO.S   mt_GlissLoop
	MOVEQ   #35*2,D0
mt_GlissFound
	MOVE.W  0(A0,D0.W),D2
mt_GlissSkip
	move.w	d2,6(a5) ; set period
	RTS

mt_Vibrato
	MOVE.B  n_cmdlo(A6),D0
	BEQ.S   mt_Vibrato2
	MOVE.B  n_vibratocmd(A6),D2
	AND.B   #$0F,D0
	BEQ.S   mt_vibskip
	AND.B   #$F0,D2
	OR.B    D0,D2
mt_vibskip
	MOVE.B  n_cmdlo(A6),D0
	AND.B   #$F0,D0
	BEQ.S   mt_vibskip2
	AND.B   #$0F,D2
	OR.B    D0,D2
mt_vibskip2
	MOVE.B  D2,n_vibratocmd(A6)
mt_Vibrato2
	MOVE.B  n_vibratopos(A6),D0
	LEA     mt_VibratoTable,A4
	LSR.W   #2,D0
	AND.W   #$001F,D0
	MOVEQ   #0,D2
	MOVE.B  n_wavecontrol(A6),D2
	AND.B   #$03,D2
	BEQ.S   mt_vib_sine
	LSL.B   #3,D0
	CMP.B   #1,D2
	BEQ.S   mt_vib_rampdown
	MOVE.B  #255,D2
	BRA.S   mt_vib_set
mt_vib_rampdown
	TST.B   n_vibratopos(A6)
	BPL.S   mt_vib_rampdown2
	MOVE.B  #255,D2
	SUB.B   D0,D2
	BRA.S   mt_vib_set
mt_vib_rampdown2
	MOVE.B  D0,D2
	BRA.S   mt_vib_set
mt_vib_sine
	MOVE.B  0(A4,D0.W),D2
mt_vib_set
	MOVE.B  n_vibratocmd(A6),D0
	AND.W   #15,D0
	MULU    D0,D2
	LSR.W   #7,D2
	MOVE.W  n_period(A6),D0
	TST.B   n_vibratopos(A6)
	BMI.S   mt_VibratoNeg
	ADD.W   D2,D0
	BRA.S   mt_Vibrato3
mt_VibratoNeg
	SUB.W   D2,D0
mt_Vibrato3
	move.w	d0,6(a5)
	MOVE.B  n_vibratocmd(A6),D0
	LSR.W   #2,D0
	AND.W   #$003C,D0
	ADD.B   D0,n_vibratopos(A6)
	RTS

mt_TonePlusVolSlide
	BSR     mt_TonePortNoChange
	BRA     mt_VolumeSlide

mt_VibratoPlusVolSlide
	BSR     mt_Vibrato2
	BRA     mt_VolumeSlide

mt_Tremolo
	MOVE.B  n_cmdlo(A6),D0
	BEQ.S   mt_Tremolo2
	MOVE.B  n_tremolocmd(A6),D2
	AND.B   #$0F,D0
	BEQ.S   mt_treskip
	AND.B   #$F0,D2
	OR.B    D0,D2
mt_treskip
	MOVE.B  n_cmdlo(A6),D0
	AND.B   #$F0,D0
	BEQ.S   mt_treskip2
	AND.B   #$0F,D2
	OR.B    D0,D2
mt_treskip2
	MOVE.B  D2,n_tremolocmd(A6)
mt_Tremolo2
	MOVE.B  n_tremolopos(A6),D0
	LEA     mt_VibratoTable,A4
	LSR.W   #2,D0
	AND.W   #$001F,D0
	MOVEQ   #0,D2
	MOVE.B  n_wavecontrol(A6),D2
	LSR.B   #4,D2
	AND.B   #$03,D2
	BEQ.S   mt_tre_sine
	LSL.B   #3,D0
	CMP.B   #1,D2
	BEQ.S   mt_tre_rampdown
	MOVE.B  #255,D2
	BRA.S   mt_tre_set
mt_tre_rampdown
	TST.B   n_vibratopos(A6)
	BPL.S   mt_tre_rampdown2
	MOVE.B  #255,D2
	SUB.B   D0,D2
	BRA.S   mt_tre_set
mt_tre_rampdown2
	MOVE.B  D0,D2
	BRA.S   mt_tre_set
mt_tre_sine
	MOVE.B  0(A4,D0.W),D2
mt_tre_set
	MOVE.B  n_tremolocmd(A6),D0
	AND.W   #15,D0
	MULU    D0,D2
	LSR.W   #6,D2
	MOVEQ   #0,D0
	MOVE.B  n_volume(A6),D0
	TST.B   n_tremolopos(A6)
	BMI.S   mt_TremoloNeg
	ADD.W   D2,D0
	BRA.S   mt_Tremolo3
mt_TremoloNeg
	SUB.W   D2,D0
mt_Tremolo3
	BPL.S   mt_TremoloSkip
	CLR.W   D0
mt_TremoloSkip
	CMP.W   #$40,D0
	BLS.S   mt_TremoloOk
	MOVE.W  #$40,D0
mt_TremoloOk
	move.w	d0,8(a5)
	move.w	d0,n_realvol(a6)
	MOVE.B  n_tremolocmd(A6),D0
	LSR.W   #2,D0
	AND.W   #$003C,D0
	ADD.B   D0,n_tremolopos(A6)
	RTS

mt_SampleOffset
	MOVEQ   #0,D0
	MOVE.B  n_cmdlo(A6),D0
	BEQ.S   mt_sononew
	MOVE.B  D0,n_sampleoffset(A6)
mt_sononew
	MOVE.B  n_sampleoffset(A6),D0
	LSL.W   #7,D0
	CMP.W   n_length(A6),D0
	BGE.S   mt_sofskip
	SUB.W   D0,n_length(A6)
	LSL.W   #1,D0
	ADD.L   D0,n_start(A6)
	RTS
mt_sofskip
	MOVE.W  #$0001,n_length(A6)
	RTS



*** mt_VolumeSlide ($A), mt_VolumeFineUp ($EA), mt_VolumeFineDown ($EB)
mt_VolumeSlide
	MOVEQ   #0,D0
	MOVE.B  n_cmdlo(A6),D0
	LSR.B   #4,D0
	BEQ.S   vol_VolSlideDown

vol_VolSlideUp
	add.b	d0,n_volume(a6)
	cmp.b	#$40,n_volume(a6)
	bmi.s	mt_vsuskip
	move.b	#$40,n_volume(a6)
mt_vsuskip
	move.b	n_volume(a6),d0
	move.w	d0,n_realvol(a6)
	move.w	d0,8(a5)
	rts

mt_VolumeFineUp
	TST.B   mt_counter
	BNE     mt_Return2
	MOVE.B  n_cmdlo(A6),D0
	and.w   #$F,D0
	BRA     vol_VolSlideUp

vol_VolSlideDown
	MOVE.B  n_cmdlo(A6),D0
	AND.w   #$0F,D0
vol_VolSlideDown2
	SUB.B   D0,n_volume(A6)
	bhs.S   .mt_vsdskip
	CLR.B   n_volume(A6)
.mt_vsdskip

	MOVE.B  n_volume(A6),d0
vol_set
	move.w	d0,n_realvol(a6)
	move.w	d0,8(a5)
	RTS

mt_VolumeFineDown
	TST.B   mt_counter
	BNE     mt_Return2
	MOVE.B  n_cmdlo(A6),D0
	AND.w   #$0F,D0
	BRA     vol_VolSlideDown2


mt_PositionJump
	MOVE.B  n_cmdlo(A6),D0
	SUBQ.B  #1,D0
	MOVE.B  D0,mt_SongPos
mt_pj2  CLR.B   mt_PBreakPos
	ST      mt_PosJumpFlag
	RTS


*** mt_VolumeChange - Effect $C, on-cycles
mt_VolumeChange
	MOVEQ   #0,D0
	MOVE.B  n_cmdlo(A6),D0
	CMP.B   #$40,D0
	BLS.S   .mt_VolumeOk
	MOVEQ   #$40,D0
.mt_VolumeOk
	MOVE.B  D0,n_volume(A6)
	move.w	d0,n_realvol(a6)
	move.w	d0,8(a5)
	RTS


mt_PatternBreak
	MOVEQ   #0,D0
	MOVE.B  n_cmdlo(A6),D0
	MOVE.L  D0,D2
	LSR.B   #4,D0
	MULU    #10,D0
	AND.B   #$0F,D2
	ADD.B   D2,D0
	CMP.B   #63,D0
	BHI.S   mt_pj2
	MOVE.B  D0,mt_PBreakPos
	ST      mt_PosJumpFlag
	RTS

mt_SetSpeed
	MOVEQ   #0,D0
	MOVE.B  3(A6),D0
	BEQ     .out
	CMP.B   #$20,D0
	bhs	SetTempo
	CLR.B   mt_counter
	MOVE.B  D0,mt_speed
.out
	RTS


mt_CheckMoreEfx
	BSR     mt_UpdateFunk
	MOVE.B  2(A6),D0
	AND.B   #$0F,D0
	CMP.B   #$9,D0
	BEQ     mt_SampleOffset
	CMP.B   #$B,D0
	BEQ     mt_PositionJump
	CMP.B   #$D,D0
	BEQ     mt_PatternBreak
	CMP.B   #$E,D0
	BEQ.S   mt_E_Commands
	CMP.B   #$F,D0
	BEQ.S   mt_SetSpeed
	CMP.B   #$C,D0
	BEQ     mt_VolumeChange
	BRA     mt_PerNop

mt_E_Commands
	MOVE.B  n_cmdlo(A6),D0
	AND.B   #$F0,D0
	LSR.B   #4,D0
	BEQ.S   mt_FilterOnOff
	CMP.B   #1,D0
	BEQ     mt_FinePortaUp
	CMP.B   #2,D0
	BEQ     mt_FinePortaDown
	CMP.B   #3,D0
	BEQ     mt_SetGlissControl
	CMP.B   #4,D0
	BEQ     mt_SetVibratoControl
	CMP.B   #5,D0
	BEQ     mt_SetFineTune
	CMP.B   #6,D0
	BEQ     mt_JumpLoop
	CMP.B   #7,D0
	BEQ     mt_SetTremoloControl
	CMP.B   #9,D0
	BEQ     mt_RetrigNote
	CMP.B   #$A,D0
	BEQ     mt_VolumeFineUp
	CMP.B   #$B,D0
	BEQ     mt_VolumeFineDown
	CMP.B   #$C,D0
	BEQ     mt_NoteCut
	CMP.B   #$D,D0
	BEQ     mt_NoteDelay
	CMP.B   #$E,D0
	BEQ     mt_PatternDelay
	CMP.B   #$F,D0
	BEQ     mt_FunkIt
	RTS

mt_FilterOnOff
	move.b	n_cmdlo(a6),d0
	and.b	#1,d0
	asl.b	#1,d0
	and.b	#$fd,$bfe001
	or.b	d0,$bfe001
	RTS

mt_SetGlissControl
	MOVE.B  n_cmdlo(A6),D0
	AND.B   #$0F,D0
	AND.B   #$F0,n_glissfunk(A6)
	OR.B    D0,n_glissfunk(A6)
	RTS


mt_SetVibratoControl
	MOVE.B  n_cmdlo(A6),D0
	AND.B   #$0F,D0
	AND.B   #$F0,n_wavecontrol(A6)
	OR.B    D0,n_wavecontrol(A6)
	RTS


mt_SetFineTune
	MOVE.B  n_cmdlo(A6),D0
	AND.B   #$0F,D0
	MOVE.B  D0,n_finetune(A6)
	RTS


mt_JumpLoop
	TST.B   mt_counter
	BNE     mt_Return2
	MOVE.B  n_cmdlo(A6),D0
	AND.B   #$0F,D0
	BEQ.S   mt_SetLoop
	TST.B   n_loopcount(A6)
	BEQ.S   mt_jumpcnt
	SUBQ.B  #1,n_loopcount(A6)
	BEQ     mt_Return2
mt_jmploop
	MOVE.B  n_pattpos(A6),mt_PBreakPos
	ST      mt_PBreakFlag
	RTS

mt_jumpcnt
	MOVE.B  D0,n_loopcount(A6)
	BRA.S   mt_jmploop

mt_SetLoop
	MOVE.W  mt_PatternPos,D0
	LSR.W   #4,D0
	MOVE.B  D0,n_pattpos(A6)
	RTS

mt_SetTremoloControl
	MOVE.B  n_cmdlo(A6),D0
	AND.B   #$0F,D0
	LSL.B   #4,D0
	AND.B   #$0F,n_wavecontrol(A6)
	OR.B    D0,n_wavecontrol(A6)
	RTS

mt_RetrigNote
	MOVE.L  D1,-(SP)
	MOVEQ   #0,D0
	MOVE.B  n_cmdlo(A6),D0
	AND.B   #$0F,D0
	BEQ	mt_rtnend
	MOVEQ   #0,D1
	MOVE.B  mt_counter,D1
	BNE.S   mt_rtnskp
	MOVE.W  (A6),D1
	AND.W   #$0FFF,D1
	BNE.S   mt_rtnend
	MOVEQ   #0,D1
	MOVE.B  mt_counter,D1
mt_rtnskp
	DIVU    D0,D1
	SWAP    D1
	TST.W   D1
	BNE.S   mt_rtnend
mt_DoRetrig
	;move.w	n_dmabit(a6),$dff096	; channel dma off
	;move.l	n_start(a6),(a5)	; set sampledata pointer
	;move.w	n_length(a6),4(a5)	; set length

        movem.l d1-d4/a0-a1,-(sp)
	lea	_ModuleSpamParam,a0
        move.l  n_start(a6),a1
        moveq   #0,d0
        move.w  n_length(a6),d0
        add.l   d0,d0
	move.l	d0,spam_Length(a0)
        move.l  n_loopstart(a6),spam_RepeatStart(a0)
        moveq   #0,d0
        move.w  n_replen(a6),d0
        cmp.w   #1,d0
        bne.b   .notone
        moveq   #0,d0
.notone
        add.l   d0,d0
	move.l	d0,spam_RepeatLength(a0)
	move.w	#-1,spam_Repeats(a0)

        move.w  n_channelno(a6),d0
        moveq   #0,d1
        move.w  n_period(a6),d1
        move.w  d1,n_realper(a6)
        ;swap    d1
	move.w	d1,spam_SampleRate(a0)	;Really Period
        moveq   #0,d1
        move.b  n_volume(a6),d1
        move.w  d1,n_realvol(a6)
	move.b	d1,spam_Volume(a0)
        ;lsl.w   #2,d1
        ;jsr     NoteStart(a5)
	bsr	_Spam_PlaySound
        movem.l (sp)+,d1-d4/a0-a1

	;move.w	mt_dmadelay,d0
mt_rtnloop1
	;dbra	d0,mt_rtnloop1
	;move.w	n_dmabit(a6),d0
	;bset	#15,d0
	;move.w	d0,$dff096
	;move.w	mt_dmadelay,d0
mt_rtnloop2
	;dbra	d0,mt_rtnloop2
	;move.l	n_loopstart(a6),(a5)
	;move.l	n_replen(a6),4(a5)

mt_rtnend
	MOVE.L  (SP)+,D1
	RTS


mt_NoteCut
	MOVEQ   #0,D0
	MOVE.B  n_cmdlo(A6),D0
	AND.B   #$0F,D0
	CMP.B   mt_counter,D0
	BNE     mt_Return2
	CLR.B   n_volume(A6)
	move.w	d0,n_realvol(a6)
	move.w	#0,8(a5)
	RTS

mt_NoteDelay
	MOVEQ   #0,D0
	MOVE.B  n_cmdlo(A6),D0
	AND.B   #$0F,D0
	CMP.B   mt_counter,D0
	BNE     mt_Return2
	MOVE.W  (A6),D0
	BEQ     mt_Return2
	MOVE.L  D1,-(SP)
	BRA     mt_DoRetrig

mt_PatternDelay
	TST.B   mt_counter
	BNE     mt_Return2
	MOVEQ   #0,D0
	MOVE.B  n_cmdlo(A6),D0
	AND.B   #$0F,D0
	TST.B   mt_PattDelTime2
	BNE     mt_Return2
	ADDQ.B  #1,D0
	MOVE.B  D0,mt_PattDelTime
	RTS

mt_FunkIt
	TST.B   mt_counter
	BNE     mt_Return2
	MOVE.B  n_cmdlo(A6),D0
	AND.B   #$0F,D0
	LSL.B   #4,D0
	AND.B   #$0F,n_glissfunk(A6)
	OR.B    D0,n_glissfunk(A6)
	TST.B   D0
	BEQ     mt_Return2
mt_UpdateFunk
	MOVEM.L D1/A0,-(SP)
	MOVEQ   #0,D0
	MOVE.B  n_glissfunk(A6),D0
	LSR.B   #4,D0
	BEQ.S   mt_funkend
	LEA     mt_FunkTable,A0
	MOVE.B  0(A0,D0.W),D0
	ADD.B   D0,n_funkoffset(A6)
	BTST    #7,n_funkoffset(A6)
	BEQ.S   mt_funkend
	CLR.B   n_funkoffset(A6)

	MOVE.L  n_loopstart(A6),D0
	add.l	n_start(a6),d0
	MOVEQ   #0,D1
	MOVE.W  n_replen(A6),D1
	ADD.L   D1,D0
	ADD.L   D1,D0
	MOVE.L  n_wavestart(A6),A0
	ADDQ.L  #1,A0
	CMP.L   D0,A0
	BLO.S   mt_funkok
	MOVE.L  n_loopstart(A6),A0
	add.l	n_start(a6),a0
mt_funkok
	MOVE.L  A0,n_wavestart(A6)
	MOVEQ   #-1,D0
	SUB.B   (A0),D0
	MOVE.B  D0,(A0)
mt_funkend
	MOVEM.L (SP)+,D1/A0
	RTS

	SECTION MyData,DATA

******************************************************************
**		PROTRACKER STUFF				**
******************************************************************

* CIA Stuff
RealTempo	dc.w 125
CIAAAddr	dc.l 0
CIAAName	dc.b "ciaa.resource",0
		even
_CIAABase	dc.l 0
TimerFlag	dc.l 0
TimerValue	dc.l 0

MusicIntServer
	dc.l 0,0
	dc.b 2,5 ; type, priority
	dc.l MusIntName
	dc.l 0,_CIAIntHandler

MusIntName	dc.b "MyPlay Interrupt",0
	even

* Protracker Stuff

mt_FunkTable dc.b 0,5,6,7,8,10,11,13,16,19,22,26,32,43,64,128

mt_VibratoTable
        dc.b   0,24,49,74,97,120,141,161
        dc.b 180,197,212,224,235,244,250,253
        dc.b 255,253,250,244,235,224,212,197
        dc.b 180,161,141,120,97,74,49,24

mt_PeriodTable
; Tuning 0, Normal
        dc.w    856,808,762,720,678,640,604,570,538,508,480,453
        dc.w    428,404,381,360,339,320,302,285,269,254,240,226
        dc.w    214,202,190,180,170,160,151,143,135,127,120,113
; Tuning 1
        dc.w    850,802,757,715,674,637,601,567,535,505,477,450
        dc.w    425,401,379,357,337,318,300,284,268,253,239,225
        dc.w    213,201,189,179,169,159,150,142,134,126,119,113
; Tuning 2
        dc.w    844,796,752,709,670,632,597,563,532,502,474,447
        dc.w    422,398,376,355,335,316,298,282,266,251,237,224
        dc.w    211,199,188,177,167,158,149,141,133,125,118,112
; Tuning 3
        dc.w    838,791,746,704,665,628,592,559,528,498,470,444
        dc.w    419,395,373,352,332,314,296,280,264,249,235,222
        dc.w    209,198,187,176,166,157,148,140,132,125,118,111
; Tuning 4
        dc.w    832,785,741,699,660,623,588,555,524,495,467,441
        dc.w    416,392,370,350,330,312,294,278,262,247,233,220
        dc.w    208,196,185,175,165,156,147,139,131,124,117,110
; Tuning 5
        dc.w    826,779,736,694,655,619,584,551,520,491,463,437
        dc.w    413,390,368,347,328,309,292,276,260,245,232,219
        dc.w    206,195,184,174,164,155,146,138,130,123,116,109
; Tuning 6
        dc.w    820,774,730,689,651,614,580,547,516,487,460,434
        dc.w    410,387,365,345,325,307,290,274,258,244,230,217
        dc.w    205,193,183,172,163,154,145,137,129,122,115,109
; Tuning 7
        dc.w    814,768,725,684,646,610,575,543,513,484,457,431
        dc.w    407,384,363,342,323,305,288,272,256,242,228,216
        dc.w    204,192,181,171,161,152,144,136,128,121,114,108
; Tuning -8
        dc.w    907,856,808,762,720,678,640,604,570,538,508,480
        dc.w    453,428,404,381,360,339,320,302,285,269,254,240
        dc.w    226,214,202,190,180,170,160,151,143,135,127,120
; Tuning -7
        dc.w    900,850,802,757,715,675,636,601,567,535,505,477
        dc.w    450,425,401,379,357,337,318,300,284,268,253,238
        dc.w    225,212,200,189,179,169,159,150,142,134,126,119
; Tuning -6
        dc.w    894,844,796,752,709,670,632,597,563,532,502,474
        dc.w    447,422,398,376,355,335,316,298,282,266,251,237
        dc.w    223,211,199,188,177,167,158,149,141,133,125,118
; Tuning -5
        dc.w    887,838,791,746,704,665,628,592,559,528,498,470
        dc.w    444,419,395,373,352,332,314,296,280,264,249,235
        dc.w    222,209,198,187,176,166,157,148,140,132,125,118
; Tuning -4
        dc.w    881,832,785,741,699,660,623,588,555,524,494,467
        dc.w    441,416,392,370,350,330,312,294,278,262,247,233
        dc.w    220,208,196,185,175,165,156,147,139,131,123,117
; Tuning -3
        dc.w    875,826,779,736,694,655,619,584,551,520,491,463
        dc.w    437,413,390,368,347,328,309,292,276,260,245,232
        dc.w    219,206,195,184,174,164,155,146,138,130,123,116
; Tuning -2
        dc.w    868,820,774,730,689,651,614,580,547,516,487,460
        dc.w    434,410,387,365,345,325,307,290,274,258,244,230
        dc.w    217,205,193,183,172,163,154,145,137,129,122,115
; Tuning -1
        dc.w    862,814,768,725,684,646,610,575,543,513,484,457
        dc.w    431,407,384,363,342,323,305,288,272,256,242,228
        dc.w    216,203,192,181,171,161,152,144,136,128,121,114

* Channel Storage Area - Longs are:
* 1) Note Data (n_note, n_cmd, n_cmdlo)
* 2 - Up) See n_xxxxx defines in MultiPlayer.s
mt_chan1temp	dc.l	0,0,0,0,0,$00010000,0,0,0,0,0,0
mt_chan2temp	dc.l	0,0,0,0,0,$00020000,0,0,0,0,1,0
mt_chan3temp	dc.l	0,0,0,0,0,$00040000,0,0,0,0,2,0
mt_chan4temp	dc.l	0,0,0,0,0,$00080000,0,0,0,0,3,0
*				   ^^^^ - Word For n_dmabit
mt_SampleStarts dc.l    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
                dc.l    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0

_SongDataPtr	dc.l 0
mt_speed	dc.b 0
mt_counter	dc.b 0
mt_SongPos	dc.b 0
mt_PBreakPos	dc.b 0
mt_PosJumpFlag	dc.b 0
mt_PBreakFlag	dc.b 0
mt_LowMask	dc.b 0
mt_PattDelTime	dc.b 0
mt_PattDelTime2	dc.b 0,0
_ModulePlayerEnable	dc.b	0
	even
mt_PatternPos	dc.w 0

seqadj		dc.w    0               ; < Fix - for old modules
blkadj		dc.w    0

_ModuleSpamParam	ds.b	spam_SIZEOF
