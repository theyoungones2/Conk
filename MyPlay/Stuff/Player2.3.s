;**************************************************
;*    ----- Protracker V2.3A Playroutine -----    *
;**************************************************

;DMAWait = 200 ; Set this as low as possible without losing low notes.

; CIA Version 1:
; Call SetCIAInt to install the interrupt server. Then call mt_init
; to initialize the song. Playback starts when the mt_enable flag
; is set to a nonzero value. To end the song and turn off all voices,
; call mt_end. At last, call ResetCIAInt to remove the interrupt.

; This playroutine is not very fast, optimized or well commented,
; but all the new commands in PT2.3 should work.
; If it's not good enough, you'll have to change it yourself.
; We'll try to write a faster routine soon...

; Changes from V1.0C playroutine:
; - Vibrato depth changed to be compatible with Noisetracker 2.0.
;   You'll have to double all vib. depths on old PT modules.
; - Funk Repeat changed to Invert Loop.
; - Period set back earlier when stopping an effect.

; You can use this routine to play a module. Just remove the semicolons.
; Exit by pressing both mousebuttons.
;
;main	BSR.S	SetCIAInt
;	BSR	mt_init
;	ST	mt_Enable
;	MOVE.L	4.W,A6
;	LEA	DOSname,A1
;	MOVEQ	#0,D0
;	JSR	LVOOpenLibrary(A6)
;	TST.L	D0
;	BEQ.S	theend
;	MOVE.L	D0,A6
;wloop	MOVEQ	#10,D1
;	JSR	LVODelay(A6)
;	BTST	#6,$BFE001
;	BNE.S	wloop
;	BTST	#2,$DFF016
;	BNE.S	wloop
;	MOVE.L	A6,A1
;	MOVE.L	4.W,A6
;	JSR	LVOCloseLibrary(A6)
;theend	BSR	mt_end
;	BSR	ResetCIAInt
;	RTS
;
;DOSname	dc.b "dos.library",0

;---- CIA Interrupt ----

AddICRVector	=   -6
RemICRVector	=  -12
LVOOpenResource	= -498
LVOOpenLibrary 	= -552
LVOCloseLibrary	= -414
LVODelay	= -198

ciatalo = $400
ciatahi = $500
ciatblo = $600
ciatbhi = $700
ciacra  = $E00
ciacrb  = $F00

SetCIAInt
	MOVEQ	#2,D6
	LEA	$BFD000,A5
	MOVE.B	#'b',CIAAname+3
SetCIALoop
	MOVEQ	#0,D0
	LEA	CIAAname,A1
	MOVE.L	4.W,A6
	JSR	LVOOpenResource(A6)
	MOVE.L	D0,CIAAbase
	BEQ	mt_Return

	LEA	GfxName,A1
	MOVEQ	#0,D0
	JSR	LVOOpenLibrary(A6)
	TST.L	D0
	BEQ	ResetCIAInt
	MOVE.L	D0,A1
	MOVE.W	206(A1),D0	; DisplayFlags
	BTST	#2,D0		; PAL?
	BEQ.S	WasNTSC
	MOVE.L	#1773447,D7 ; PAL
	BRA.S	sciask
WasNTSC	MOVE.L	#1789773,D7 ; NTSC
sciask	MOVE.L	D7,TimerValue
	DIVU	#125,D7 ; Default to normal 50 Hz timer
	JSR	LVOCloseLibrary(A6)

	MOVE.L	CIAAbase,A6
	CMP.W	#2,D6
	BEQ.S	TryTimerA
TryTimerB
	LEA	MusicIntServer,A1
	MOVEQ	#1,D0	; Bit 1: Timer B
	JSR	AddICRVector(A6)
	MOVE.L	#1,TimerFlag
	TST.L	D0
	BNE.S	CIAError
	MOVE.L	A5,CIAAaddr
	MOVE.B	D7,ciatblo(A5)
	LSR.W	#8,D7
	MOVE.B	D7,ciatbhi(A5)
	BSET	#0,ciacrb(A5)
	RTS

TryTimerA
	LEA	MusicIntServer,A1
	MOVEQ	#0,D0	; Bit 0: Timer A
	JSR	AddICRVector(A6)
	CLR.L	TimerFlag
	TST.L	D0
	BNE.S	CIAError
	MOVE.L	A5,CIAAaddr
	MOVE.B	D7,ciatalo(A5)
	LSR.W	#8,D7
	MOVE.B	D7,ciatahi(A5)
	BSET	#0,ciacra(A5)
	RTS

CIAError
	MOVE.B	#'a',CIAAname+3
	LEA	$BFE001,A5
	SUBQ.W	#1,D6
	BNE	SetCIALoop
	CLR.L	CIAAbase
	RTS

ResetCIAInt
	MOVE.L	CIAAbase,D0
	BEQ	mt_Return
	CLR.L	CIAAbase
	MOVE.L	D0,A6
	MOVE.L	CIAAaddr,A5
	TST.L	TimerFlag
	BEQ.S	ResTimerA

	BCLR	#0,ciacrb(A5)
	MOVEQ	#1,D0
	BRA.S	RemInt

ResTimerA
	BCLR	#0,ciacra(A5)
	MOVEQ	#0,D0
RemInt	LEA	MusicIntServer,A1
	MOVEQ	#0,d0
	JSR	RemICRVector(A6)
	RTS

;---- Tempo ----

SetTempo
	MOVE.L	CIAAbase,D2
	BEQ	mt_Return
	CMP.W	#32,D0
	BHS.S	setemsk
	MOVEQ	#32,D0
setemsk	MOVE.W	D0,RealTempo
	MOVE.L	TimerValue,D2
	DIVU	D0,D2
	MOVE.L	CIAAaddr,A4
	MOVE.L	TimerFlag,D0
	BEQ.S	SetTemA
	MOVE.B	D2,ciatblo(A4)
	LSR.W	#8,D2
	MOVE.B	D2,ciatbhi(A4)
	RTS

SetTemA	MOVE.B	D2,ciatalo(A4)
	LSR.W	#8,D2
	MOVE.B	D2,ciatahi(A4)
	RTS

MusicIntServer
	dc.l 0,0
	dc.b 2,5 ; type, priority
	dc.l musintname
	dc.l 0,mt_music

musintname	dc.b "Protracker MusicInt",0

;---- Playroutine ----

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

mt_init	move.l	FileBuf,A0
	MOVE.L	A0,mt_SongDataPtr
	MOVE.L	A0,A1
	LEA	952(A1),A1
	MOVEQ	#127,D0
	MOVEQ	#0,D1
mtloop	MOVE.L	D1,D2
	SUBQ.W	#1,D0
mtloop2	MOVE.B	(A1)+,D1
	CMP.B	D2,D1
	BGT.S	mtloop
	DBRA	D0,mtloop2
	ADDQ.B	#1,D2
			
	LEA	mt_SampleStarts,A1
	ASL.L	#8,D2
	ASL.L	#2,D2
	ADD.L	#1084,D2
	ADD.L	A0,D2
	MOVE.L	D2,A2
	MOVEQ	#30,D0
mtloop3	CLR.L	(A2)
	MOVE.L	A2,(A1)+
	MOVEQ	#0,D1
	MOVE.W	42(A0),D1
	ASL.L	#1,D1
	ADD.L	D1,A2
	ADD.L	#30,A0
	DBRA	D0,mtloop3

	OR.B	#2,$BFE001
	MOVE.B	#6,mt_speed
	CLR.B	mt_counter
	CLR.B	mt_SongPos
	CLR.W	mt_PatternPos
mt_end	SF	mt_Enable
	LEA	$DFF000,A0
	CLR.W	$A8(A0)
	CLR.W	$B8(A0)
	CLR.W	$C8(A0)
	CLR.W	$D8(A0)
	MOVE.W	#$F,$DFF096
	RTS

mt_music
	MOVEM.L	D0-D4/A0-A6,-(SP)
	TST.B	mt_Enable
	BEQ	mt_exit
	ADDQ.B	#1,mt_counter
	MOVE.B	mt_counter,D0
	CMP.B	mt_speed,D0
	BLO.S	mt_NoNewNote
	CLR.B	mt_counter
	TST.B	mt_PattDelTime2
	BEQ.S	mt_GetNewNote
	BSR.S	mt_NoNewAllChannels
	BRA	mt_dskip

mt_NoNewNote
	BSR.S	mt_NoNewAllChannels
	BRA	mt_NoNewPosYet

mt_NoNewAllChannels
	LEA	$DFF0A0,A5
	LEA	mt_chan1temp,A6
	BSR	mt_CheckEfx
	LEA	$DFF0B0,A5
	LEA	mt_chan2temp,A6
	BSR	mt_CheckEfx
	LEA	$DFF0C0,A5
	LEA	mt_chan3temp,A6
	BSR	mt_CheckEfx
	LEA	$DFF0D0,A5
	LEA	mt_chan4temp,A6
	BRA	mt_CheckEfx

mt_GetNewNote
	MOVE.L	mt_SongDataPtr,A0
	LEA	12(A0),A3
	LEA	952(A0),A2	;pattpo
	LEA	1084(A0),A0	;patterndata
	MOVEQ	#0,D0
	MOVEQ	#0,D1
	MOVE.B	mt_SongPos,D0
	MOVE.B	(A2,D0.W),D1
	ASL.L	#8,D1
	ASL.L	#2,D1
	ADD.W	mt_PatternPos,D1
	CLR.W	mt_DMACONtemp

	LEA	$DFF0A0,A5
	LEA	mt_chan1temp,A6
	BSR.S	mt_PlayVoice
	LEA	$DFF0B0,A5
	LEA	mt_chan2temp,A6
	BSR.S	mt_PlayVoice
	LEA	$DFF0C0,A5
	LEA	mt_chan3temp,A6
	BSR.S	mt_PlayVoice
	LEA	$DFF0D0,A5
	LEA	mt_chan4temp,A6
	BSR.S	mt_PlayVoice
	BRA	mt_SetDMA

mt_PlayVoice
	TST.L	(A6)
	BNE.S	mt_plvskip
	BSR	mt_PerNop
mt_plvskip
	MOVE.L	(A0,D1.L),(A6)
	ADDQ.L	#4,D1
	MOVEQ	#0,D2
	MOVE.B	n_cmd(A6),D2
	AND.B	#$F0,D2
	LSR.B	#4,D2
	MOVE.B	(A6),D0
	AND.B	#$F0,D0
	OR.B	D0,D2
	TST.B	D2
	BEQ	mt_SetRegs
	MOVEQ	#0,D3
	LEA	mt_SampleStarts,A1
	MOVE	D2,D4
	SUBQ.L	#1,D2
	ASL.L	#2,D2
	MULU	#30,D4
	MOVE.L	(A1,D2.L),n_start(A6)
	MOVE.W	(A3,D4.L),n_length(A6)
	MOVE.W	(A3,D4.L),n_reallength(A6)
	MOVE.B	2(A3,D4.L),n_finetune(A6)
	MOVE.B	3(A3,D4.L),n_volume(A6)
	MOVE.W	4(A3,D4.L),D3 ; Get repeat
	TST.W	D3
	BEQ.S	mt_NoLoop
	MOVE.L	n_start(A6),D2	; Get start
	ASL.W	#1,D3
	ADD.L	D3,D2		; Add repeat
	MOVE.L	D2,n_loopstart(A6)
	MOVE.L	D2,n_wavestart(A6)
	MOVE.W	4(A3,D4.L),D0	; Get repeat
	ADD.W	6(A3,D4.L),D0	; Add replen
	MOVE.W	D0,n_length(A6)
	MOVE.W	6(A3,D4.L),n_replen(A6)	; Save replen
	MOVEQ	#0,D0
	MOVE.B	n_volume(A6),D0
	MOVE.W	D0,8(A5)	; Set volume
	BRA.S	mt_SetRegs

mt_NoLoop
	MOVE.L	n_start(A6),D2
	ADD.L	D3,D2
	MOVE.L	D2,n_loopstart(A6)
	MOVE.L	D2,n_wavestart(A6)
	MOVE.W	6(A3,D4.L),n_replen(A6)	; Save replen
	MOVEQ	#0,D0
	MOVE.B	n_volume(A6),D0
	MOVE.W	D0,8(A5)	; Set volume
mt_SetRegs
	MOVE.W	(A6),D0
	AND.W	#$0FFF,D0
	BEQ	mt_CheckMoreEfx	; If no note
	MOVE.W	2(A6),D0
	AND.W	#$0FF0,D0
	CMP.W	#$0E50,D0
	BEQ.S	mt_DoSetFineTune
	MOVE.B	2(A6),D0
	AND.B	#$0F,D0
	CMP.B	#3,D0	; TonePortamento
	BEQ.S	mt_ChkTonePorta
	CMP.B	#5,D0
	BEQ.S	mt_ChkTonePorta
	CMP.B	#9,D0	; Sample Offset
	BNE.S	mt_SetPeriod
	BSR	mt_CheckMoreEfx
	BRA.S	mt_SetPeriod

mt_DoSetFineTune
	BSR	mt_SetFineTune
	BRA.S	mt_SetPeriod

mt_ChkTonePorta
	BSR	mt_SetTonePorta
	BRA	mt_CheckMoreEfx

mt_SetPeriod
	MOVEM.L	D0-D1/A0-A1,-(SP)
	MOVE.W	(A6),D1
	AND.W	#$0FFF,D1
	LEA	mt_PeriodTable,A1
	MOVEQ	#0,D0
	MOVEQ	#36,D2
mt_ftuloop
	CMP.W	(A1,D0.W),D1
	BHS.S	mt_ftufound
	ADDQ.L	#2,D0
	DBRA	D2,mt_ftuloop
mt_ftufound
	MOVEQ	#0,D1
	MOVE.B	n_finetune(A6),D1
	MULU	#36*2,D1
	ADD.L	D1,A1
	MOVE.W	(A1,D0.W),n_period(A6)
	MOVEM.L	(SP)+,D0-D1/A0-A1

	MOVE.W	2(A6),D0
	AND.W	#$0FF0,D0
	CMP.W	#$0ED0,D0 ; Notedelay
	BEQ	mt_CheckMoreEfx

	MOVE.W	n_dmabit(A6),$DFF096
	BTST	#2,n_wavecontrol(A6)
	BNE.S	mt_vibnoc
	CLR.B	n_vibratopos(A6)
mt_vibnoc
	BTST	#6,n_wavecontrol(A6)
	BNE.S	mt_trenoc
	CLR.B	n_tremolopos(A6)
mt_trenoc
	MOVE.L	n_start(A6),(A5)	; Set start
	MOVE.W	n_length(A6),4(A5)	; Set length
	MOVE.W	n_period(A6),D0
	MOVE.W	D0,6(A5)		; Set period
	MOVE.W	n_dmabit(A6),D0
	OR.W	D0,mt_DMACONtemp
	BRA	mt_CheckMoreEfx
 
mt_SetDMA
	MOVE.W	#300,D0
mt_WaitDMA
	DBRA	D0,mt_WaitDMA
	MOVE.W	mt_DMACONtemp,D0
	OR.W	#$8000,D0
	MOVE.W	D0,$DFF096
	MOVE.W	#300,D0
mt_WaitDMA2
	DBRA	D0,mt_WaitDMA2

	LEA	$DFF000,A5
	LEA	mt_chan4temp,A6
	MOVE.L	n_loopstart(A6),$D0(A5)
	MOVE.W	n_replen(A6),$D4(A5)
	LEA	mt_chan3temp,A6
	MOVE.L	n_loopstart(A6),$C0(A5)
	MOVE.W	n_replen(A6),$C4(A5)
	LEA	mt_chan2temp,A6
	MOVE.L	n_loopstart(A6),$B0(A5)
	MOVE.W	n_replen(A6),$B4(A5)
	LEA	mt_chan1temp,A6
	MOVE.L	n_loopstart(A6),$A0(A5)
	MOVE.W	n_replen(A6),$A4(A5)

mt_dskip
	ADD.W	#16,mt_PatternPos
	MOVE.B	mt_PattDelTime,D0
	BEQ.S	mt_dskc
	MOVE.B	D0,mt_PattDelTime2
	CLR.B	mt_PattDelTime
mt_dskc	TST.B	mt_PattDelTime2
	BEQ.S	mt_dska
	SUBQ.B	#1,mt_PattDelTime2
	BEQ.S	mt_dska
	SUB.W	#16,mt_PatternPos
mt_dska	TST.B	mt_PBreakFlag
	BEQ.S	mt_nnpysk
	SF	mt_PBreakFlag
	MOVEQ	#0,D0
	MOVE.B	mt_PBreakPos,D0
	CLR.B	mt_PBreakPos
	LSL.W	#4,D0
	MOVE.W	D0,mt_PatternPos
mt_nnpysk
	CMP.W	#1024,mt_PatternPos
	BLO.S	mt_NoNewPosYet
mt_NextPosition	
	MOVEQ	#0,D0
	MOVE.B	mt_PBreakPos,D0
	LSL.W	#4,D0
	MOVE.W	D0,mt_PatternPos
	CLR.B	mt_PBreakPos
	CLR.B	mt_PosJumpFlag
	ADDQ.B	#1,mt_SongPos
	AND.B	#$7F,mt_SongPos
	MOVE.B	mt_SongPos,D1
	MOVE.L	mt_SongDataPtr,A0
	CMP.B	950(A0),D1
	BLO.S	mt_NoNewPosYet
	CLR.B	mt_SongPos
mt_NoNewPosYet	
	TST.B	mt_PosJumpFlag
	BNE.S	mt_NextPosition
mt_exit	MOVEM.L	(SP)+,D0-D4/A0-A6
	RTS

mt_CheckEfx
	BSR	mt_UpdateFunk
	MOVE.W	n_cmd(A6),D0
	AND.W	#$0FFF,D0
	BEQ.S	mt_PerNop
	MOVE.B	n_cmd(A6),D0
	AND.B	#$0F,D0
	BEQ.S	mt_Arpeggio
	CMP.B	#1,D0
	BEQ	mt_PortaUp
	CMP.B	#2,D0
	BEQ	mt_PortaDown
	CMP.B	#3,D0
	BEQ	mt_TonePortamento
	CMP.B	#4,D0
	BEQ	mt_Vibrato
	CMP.B	#5,D0
	BEQ	mt_TonePlusVolSlide
	CMP.B	#6,D0
	BEQ	mt_VibratoPlusVolSlide
	CMP.B	#$E,D0
	BEQ	mt_E_Commands
SetBack	MOVE.W	n_period(A6),6(A5)
	CMP.B	#7,D0
	BEQ	mt_Tremolo
	CMP.B	#$A,D0
	BEQ	mt_VolumeSlide
mt_Return
	RTS

mt_PerNop
	MOVE.W	n_period(A6),6(A5)
	RTS

mt_Arpeggio
	MOVEQ	#0,D0
	MOVE.B	mt_counter,D0
	DIVS	#3,D0
	SWAP	D0
	CMP.W	#0,D0
	BEQ.S	mt_Arpeggio2
	CMP.W	#2,D0
	BEQ.S	mt_Arpeggio1
	MOVEQ	#0,D0
	MOVE.B	n_cmdlo(A6),D0
	LSR.B	#4,D0
	BRA.S	mt_Arpeggio3

mt_Arpeggio1
	MOVEQ	#0,D0
	MOVE.B	n_cmdlo(A6),D0
	AND.B	#15,D0
	BRA.S	mt_Arpeggio3

mt_Arpeggio2
	MOVE.W	n_period(A6),D2
	BRA.S	mt_Arpeggio4

mt_Arpeggio3
	ASL.W	#1,D0
	MOVEQ	#0,D1
	MOVE.B	n_finetune(A6),D1
	MULU	#36*2,D1
	LEA	mt_PeriodTable,A0
	ADD.L	D1,A0
	MOVEQ	#0,D1
	MOVE.W	n_period(A6),D1
	MOVEQ	#36,D3
mt_arploop
	MOVE.W	(A0,D0.W),D2
	CMP.W	(A0),D1
	BHS.S	mt_Arpeggio4
	ADDQ.L	#2,A0
	DBRA	D3,mt_arploop
	RTS

mt_Arpeggio4
	MOVE.W	D2,6(A5)
	RTS

mt_FinePortaUp
	TST.B	mt_counter
	BNE.S	mt_Return
	MOVE.B	#$0F,mt_LowMask
mt_PortaUp
	MOVEQ	#0,D0
	MOVE.B	n_cmdlo(A6),D0
	AND.B	mt_LowMask,D0
	MOVE.B	#$FF,mt_LowMask
	SUB.W	D0,n_period(A6)
	MOVE.W	n_period(A6),D0
	AND.W	#$0FFF,D0
	CMP.W	#113,D0
	BPL.S	mt_PortaUskip
	AND.W	#$F000,n_period(A6)
	OR.W	#113,n_period(A6)
mt_PortaUskip
	MOVE.W	n_period(A6),D0
	AND.W	#$0FFF,D0
	MOVE.W	D0,6(A5)
	RTS	
 
mt_FinePortaDown
	TST.B	mt_counter
	BNE	mt_Return
	MOVE.B	#$0F,mt_LowMask
mt_PortaDown
	CLR.W	D0
	MOVE.B	n_cmdlo(A6),D0
	AND.B	mt_LowMask,D0
	MOVE.B	#$FF,mt_LowMask
	ADD.W	D0,n_period(A6)
	MOVE.W	n_period(A6),D0
	AND.W	#$0FFF,D0
	CMP.W	#856,D0
	BMI.S	mt_PortaDskip
	AND.W	#$F000,n_period(A6)
	OR.W	#856,n_period(A6)
mt_PortaDskip
	MOVE.W	n_period(A6),D0
	AND.W	#$0FFF,D0
	MOVE.W	D0,6(A5)
	RTS

mt_SetTonePorta
	MOVE.L	A0,-(SP)
	MOVE.W	(A6),D2
	AND.W	#$0FFF,D2
	MOVEQ	#0,D0
	MOVE.B	n_finetune(A6),D0
	MULU	#36*2,D0 ;37?
	LEA	mt_PeriodTable,A0
	ADD.L	D0,A0
	MOVEQ	#0,D0
mt_StpLoop
	CMP.W	(A0,D0.W),D2
	BHS.S	mt_StpFound
	ADDQ.W	#2,D0
	CMP.W	#36*2,D0 ;37?
	BLO.S	mt_StpLoop
	MOVEQ	#35*2,D0
mt_StpFound
	MOVE.B	n_finetune(A6),D2
	AND.B	#8,D2
	BEQ.S	mt_StpGoss
	TST.W	D0
	BEQ.S	mt_StpGoss
	SUBQ.W	#2,D0
mt_StpGoss
	MOVE.W	(A0,D0.W),D2
	MOVE.L	(SP)+,A0
	MOVE.W	D2,n_wantedperiod(A6)
	MOVE.W	n_period(A6),D0
	CLR.B	n_toneportdirec(A6)
	CMP.W	D0,D2
	BEQ.S	mt_ClearTonePorta
	BGE	mt_Return
	MOVE.B	#1,n_toneportdirec(A6)
	RTS

mt_ClearTonePorta
	CLR.W	n_wantedperiod(A6)
	RTS

mt_TonePortamento
	MOVE.B	n_cmdlo(A6),D0
	BEQ.S	mt_TonePortNoChange
	MOVE.B	D0,n_toneportspeed(A6)
	CLR.B	n_cmdlo(A6)
mt_TonePortNoChange
	TST.W	n_wantedperiod(A6)
	BEQ	mt_Return
	MOVEQ	#0,D0
	MOVE.B	n_toneportspeed(A6),D0
	TST.B	n_toneportdirec(A6)
	BNE.S	mt_TonePortaUp
mt_TonePortaDown
	ADD.W	D0,n_period(A6)
	MOVE.W	n_wantedperiod(A6),D0
	CMP.W	n_period(A6),D0
	BGT.S	mt_TonePortaSetPer
	MOVE.W	n_wantedperiod(A6),n_period(A6)
	CLR.W	n_wantedperiod(A6)
	BRA.S	mt_TonePortaSetPer

mt_TonePortaUp
	SUB.W	D0,n_period(A6)
	MOVE.W	n_wantedperiod(A6),D0
	CMP.W	n_period(A6),D0
	BLT.S	mt_TonePortaSetPer
	MOVE.W	n_wantedperiod(A6),n_period(A6)
	CLR.W	n_wantedperiod(A6)

mt_TonePortaSetPer
	MOVE.W	n_period(A6),D2
	MOVE.B	n_glissfunk(A6),D0
	AND.B	#$0F,D0
	BEQ.S	mt_GlissSkip
	MOVEQ	#0,D0
	MOVE.B	n_finetune(A6),D0
	MULU	#36*2,D0
	LEA	mt_PeriodTable,A0
	ADD.L	D0,A0
	MOVEQ	#0,D0
mt_GlissLoop
	CMP.W	(A0,D0.W),D2
	BHS.S	mt_GlissFound
	ADDQ.W	#2,D0
	CMP.W	#36*2,D0
	BLO.S	mt_GlissLoop
	MOVEQ	#35*2,D0
mt_GlissFound
	MOVE.W	(A0,D0.W),D2
mt_GlissSkip
	MOVE.W	D2,6(A5) ; Set period
	RTS

mt_Vibrato
	MOVE.B	n_cmdlo(A6),D0
	BEQ.S	mt_Vibrato2
	MOVE.B	n_vibratocmd(A6),D2
	AND.B	#$0F,D0
	BEQ.S	mt_vibskip
	AND.B	#$F0,D2
	OR.B	D0,D2
mt_vibskip
	MOVE.B	n_cmdlo(A6),D0
	AND.B	#$F0,D0
	BEQ.S	mt_vibskip2
	AND.B	#$0F,D2
	OR.B	D0,D2
mt_vibskip2
	MOVE.B	D2,n_vibratocmd(A6)
mt_Vibrato2
	MOVE.B	n_vibratopos(A6),D0
	LEA	mt_VibratoTable,A4
	LSR.W	#2,D0
	AND.W	#$001F,D0
	MOVEQ	#0,D2
	MOVE.B	n_wavecontrol(A6),D2
	AND.B	#$03,D2
	BEQ.S	mt_vib_sine
	LSL.B	#3,D0
	CMP.B	#1,D2
	BEQ.S	mt_vib_rampdown
	MOVE.B	#255,D2
	BRA.S	mt_vib_set
mt_vib_rampdown
	TST.B	n_vibratopos(A6)
	BPL.S	mt_vib_rampdown2
	MOVE.B	#255,D2
	SUB.B	D0,D2
	BRA.S	mt_vib_set
mt_vib_rampdown2
	MOVE.B	D0,D2
	BRA.S	mt_vib_set
mt_vib_sine
	MOVE.B	(A4,D0.W),D2
mt_vib_set
	MOVE.B	n_vibratocmd(A6),D0
	AND.W	#15,D0
	MULU	D0,D2
	LSR.W	#7,D2
	MOVE.W	n_period(A6),D0
	TST.B	n_vibratopos(A6)
	BMI.S	mt_VibratoNeg
	ADD.W	D2,D0
	BRA.S	mt_Vibrato3
mt_VibratoNeg
	SUB.W	D2,D0
mt_Vibrato3
	MOVE.W	D0,6(A5)
	MOVE.B	n_vibratocmd(A6),D0
	LSR.W	#2,D0
	AND.W	#$003C,D0
	ADD.B	D0,n_vibratopos(A6)
	RTS

mt_TonePlusVolSlide
	BSR	mt_TonePortNoChange
	BRA	mt_VolumeSlide

mt_VibratoPlusVolSlide
	BSR.S	mt_Vibrato2
	BRA	mt_VolumeSlide

mt_Tremolo
	MOVE.B	n_cmdlo(A6),D0
	BEQ.S	mt_Tremolo2
	MOVE.B	n_tremolocmd(A6),D2
	AND.B	#$0F,D0
	BEQ.S	mt_treskip
	AND.B	#$F0,D2
	OR.B	D0,D2
mt_treskip
	MOVE.B	n_cmdlo(A6),D0
	AND.B	#$F0,D0
	BEQ.S	mt_treskip2
	AND.B	#$0F,D2
	OR.B	D0,D2
mt_treskip2
	MOVE.B	D2,n_tremolocmd(A6)
mt_Tremolo2
	MOVE.B	n_tremolopos(A6),D0
	LEA	mt_VibratoTable,A4
	LSR.W	#2,D0
	AND.W	#$001F,D0
	MOVEQ	#0,D2
	MOVE.B	n_wavecontrol(A6),D2
	LSR.B	#4,D2
	AND.B	#$03,D2
	BEQ.S	mt_tre_sine
	LSL.B	#3,D0
	CMP.B	#1,D2
	BEQ.S	mt_tre_rampdown
	MOVE.B	#255,D2
	BRA.S	mt_tre_set
mt_tre_rampdown
	TST.B	n_vibratopos(A6)
	BPL.S	mt_tre_rampdown2
	MOVE.B	#255,D2
	SUB.B	D0,D2
	BRA.S	mt_tre_set
mt_tre_rampdown2
	MOVE.B	D0,D2
	BRA.S	mt_tre_set
mt_tre_sine
	MOVE.B	(A4,D0.W),D2
mt_tre_set
	MOVE.B	n_tremolocmd(A6),D0
	AND.W	#15,D0
	MULU	D0,D2
	LSR.W	#6,D2
	MOVEQ	#0,D0
	MOVE.B	n_volume(A6),D0
	TST.B	n_tremolopos(A6)
	BMI.S	mt_TremoloNeg
	ADD.W	D2,D0
	BRA.S	mt_Tremolo3
mt_TremoloNeg
	SUB.W	D2,D0
mt_Tremolo3
	BPL.S	mt_TremoloSkip
	CLR.W	D0
mt_TremoloSkip
	CMP.W	#$40,D0
	BLS.S	mt_TremoloOk
	MOVE.W	#$40,D0
mt_TremoloOk
	MOVE.W	D0,8(A5)
	MOVE.B	n_tremolocmd(A6),D0
	LSR.W	#2,D0
	AND.W	#$003C,D0
	ADD.B	D0,n_tremolopos(A6)
	RTS

mt_SampleOffset
	MOVEQ	#0,D0
	MOVE.B	n_cmdlo(A6),D0
	BEQ.S	mt_sononew
	MOVE.B	D0,n_sampleoffset(A6)
mt_sononew
	MOVE.B	n_sampleoffset(A6),D0
	LSL.W	#7,D0
	CMP.W	n_length(A6),D0
	BGE.S	mt_sofskip
	SUB.W	D0,n_length(A6)
	LSL.W	#1,D0
	ADD.L	D0,n_start(A6)
	RTS
mt_sofskip
	MOVE.W	#$0001,n_length(A6)
	RTS

mt_VolumeSlide
	MOVEQ	#0,D0
	MOVE.B	n_cmdlo(A6),D0
	LSR.B	#4,D0
	TST.B	D0
	BEQ.S	mt_VolSlideDown
mt_VolSlideUp
	ADD.B	D0,n_volume(A6)
	CMP.B	#$40,n_volume(A6)
	BMI.S	mt_vsuskip
	MOVE.B	#$40,n_volume(A6)
mt_vsuskip
	MOVE.B	n_volume(A6),D0
	MOVE.W	D0,8(A5)
	RTS

mt_VolSlideDown
	MOVEQ	#0,D0
	MOVE.B	n_cmdlo(A6),D0
	AND.B	#$0F,D0
mt_VolSlideDown2
	SUB.B	D0,n_volume(A6)
	BPL.S	mt_vsdskip
	CLR.B	n_volume(A6)
mt_vsdskip
	MOVE.B	n_volume(A6),D0
	MOVE.W	D0,8(A5)
	RTS

mt_PositionJump
	MOVE.B	n_cmdlo(A6),D0
	SUBQ.B	#1,D0
	MOVE.B	D0,mt_SongPos
mt_pj2	CLR.B	mt_PBreakPos
	ST 	mt_PosJumpFlag
	RTS

mt_VolumeChange
	MOVEQ	#0,D0
	MOVE.B	n_cmdlo(A6),D0
	CMP.B	#$40,D0
	BLS.S	mt_VolumeOk
	MOVEQ	#$40,D0
mt_VolumeOk
	MOVE.B	D0,n_volume(A6)
	MOVE.W	D0,8(A5)
	RTS

mt_PatternBreak
	MOVEQ	#0,D0
	MOVE.B	n_cmdlo(A6),D0
	MOVE.L	D0,D2
	LSR.B	#4,D0
	MULU	#10,D0
	AND.B	#$0F,D2
	ADD.B	D2,D0
	CMP.B	#63,D0
	BHI.S	mt_pj2
	MOVE.B	D0,mt_PBreakPos
	ST	mt_PosJumpFlag
	RTS

mt_SetSpeed
	MOVEQ	#0,D0
	MOVE.B	3(A6),D0
	BEQ	mt_end
	CMP.B	#32,D0
	BHS	SetTempo
	CLR.B	mt_counter
	MOVE.B	D0,mt_speed
	RTS

mt_CheckMoreEfx
	BSR	mt_UpdateFunk
	MOVE.B	2(A6),D0
	AND.B	#$0F,D0
	CMP.B	#$9,D0
	BEQ	mt_SampleOffset
	CMP.B	#$B,D0
	BEQ	mt_PositionJump
	CMP.B	#$D,D0
	BEQ.S	mt_PatternBreak
	CMP.B	#$E,D0
	BEQ.S	mt_E_Commands
	CMP.B	#$F,D0
	BEQ.S	mt_SetSpeed
	CMP.B	#$C,D0
	BEQ	mt_VolumeChange
	BRA	mt_PerNop

mt_E_Commands
	MOVE.B	n_cmdlo(A6),D0
	AND.B	#$F0,D0
	LSR.B	#4,D0
	BEQ.S	mt_FilterOnOff
	CMP.B	#1,D0
	BEQ	mt_FinePortaUp
	CMP.B	#2,D0
	BEQ	mt_FinePortaDown
	CMP.B	#3,D0
	BEQ.S	mt_SetGlissControl
	CMP.B	#4,D0
	BEQ	mt_SetVibratoControl
	CMP.B	#5,D0
	BEQ	mt_SetFineTune
	CMP.B	#6,D0
	BEQ	mt_JumpLoop
	CMP.B	#7,D0
	BEQ	mt_SetTremoloControl
	CMP.B	#9,D0
	BEQ	mt_RetrigNote
	CMP.B	#$A,D0
	BEQ	mt_VolumeFineUp
	CMP.B	#$B,D0
	BEQ	mt_VolumeFineDown
	CMP.B	#$C,D0
	BEQ	mt_NoteCut
	CMP.B	#$D,D0
	BEQ	mt_NoteDelay
	CMP.B	#$E,D0
	BEQ	mt_PatternDelay
	CMP.B	#$F,D0
	BEQ	mt_FunkIt
	RTS

mt_FilterOnOff
	MOVE.B	n_cmdlo(A6),D0
	AND.B	#1,D0
	ASL.B	#1,D0
	AND.B	#$FD,$BFE001
	OR.B	D0,$BFE001
	RTS	

mt_SetGlissControl
	MOVE.B	n_cmdlo(A6),D0
	AND.B	#$0F,D0
	AND.B	#$F0,n_glissfunk(A6)
	OR.B	D0,n_glissfunk(A6)
	RTS

mt_SetVibratoControl
	MOVE.B	n_cmdlo(A6),D0
	AND.B	#$0F,D0
	AND.B	#$F0,n_wavecontrol(A6)
	OR.B	D0,n_wavecontrol(A6)
	RTS

mt_SetFineTune
	MOVE.B	n_cmdlo(A6),D0
	AND.B	#$0F,D0
	MOVE.B	D0,n_finetune(A6)
	RTS

mt_JumpLoop
	TST.B	mt_counter
	BNE	mt_Return
	MOVE.B	n_cmdlo(A6),D0
	AND.B	#$0F,D0
	BEQ.S	mt_SetLoop
	TST.B	n_loopcount(A6)
	BEQ.S	mt_jumpcnt
	SUBQ.B	#1,n_loopcount(A6)
	BEQ	mt_Return
mt_jmploop	MOVE.B	n_pattpos(A6),mt_PBreakPos
	ST	mt_PBreakFlag
	RTS

mt_jumpcnt
	MOVE.B	D0,n_loopcount(A6)
	BRA.S	mt_jmploop

mt_SetLoop
	MOVE.W	mt_PatternPos,D0
	LSR.W	#4,D0
	MOVE.B	D0,n_pattpos(A6)
	RTS

mt_SetTremoloControl
	MOVE.B	n_cmdlo(A6),D0
	AND.B	#$0F,D0
	LSL.B	#4,D0
	AND.B	#$0F,n_wavecontrol(A6)
	OR.B	D0,n_wavecontrol(A6)
	RTS

mt_RetrigNote
	MOVE.L	D1,-(SP)
	MOVEQ	#0,D0
	MOVE.B	n_cmdlo(A6),D0
	AND.B	#$0F,D0
	BEQ.S	mt_rtnend
	MOVEQ	#0,D1
	MOVE.B	mt_counter,D1
	BNE.S	mt_rtnskp
	MOVE.W	(A6),D1
	AND.W	#$0FFF,D1
	BNE.S	mt_rtnend
	MOVEQ	#0,D1
	MOVE.B	mt_counter,D1
mt_rtnskp
	DIVU	D0,D1
	SWAP	D1
	TST.W	D1
	BNE.S	mt_rtnend
mt_DoRetrig
	MOVE.W	n_dmabit(A6),$DFF096	; Channel DMA off
	MOVE.L	n_start(A6),(A5)	; Set sampledata pointer
	MOVE.W	n_length(A6),4(A5)	; Set length
	MOVE.W	#300,D0
mt_rtnloop1
	DBRA	D0,mt_rtnloop1
	MOVE.W	n_dmabit(A6),D0
	BSET	#15,D0
	MOVE.W	D0,$DFF096
	MOVE.W	#300,D0
mt_rtnloop2
	DBRA	D0,mt_rtnloop2
	MOVE.L	n_loopstart(A6),(A5)
	MOVE.L	n_replen(A6),4(A5)
mt_rtnend
	MOVE.L	(SP)+,D1
	RTS

mt_VolumeFineUp
	TST.B	mt_counter
	BNE	mt_Return
	MOVEQ	#0,D0
	MOVE.B	n_cmdlo(A6),D0
	AND.B	#$F,D0
	BRA	mt_VolSlideUp

mt_VolumeFineDown
	TST.B	mt_counter
	BNE	mt_Return
	MOVEQ	#0,D0
	MOVE.B	n_cmdlo(A6),D0
	AND.B	#$0F,D0
	BRA	mt_VolSlideDown2

mt_NoteCut
	MOVEQ	#0,D0
	MOVE.B	n_cmdlo(A6),D0
	AND.B	#$0F,D0
	CMP.B	mt_counter,D0
	BNE	mt_Return
	CLR.B	n_volume(A6)
	MOVE.W	#0,8(A5)
	RTS

mt_NoteDelay
	MOVEQ	#0,D0
	MOVE.B	n_cmdlo(A6),D0
	AND.B	#$0F,D0
	CMP.B	mt_counter,D0
	BNE	mt_Return
	MOVE.W	(A6),D0
	BEQ	mt_Return
	MOVE.L	D1,-(SP)
	BRA	mt_DoRetrig

mt_PatternDelay
	TST.B	mt_counter
	BNE	mt_Return
	MOVEQ	#0,D0
	MOVE.B	n_cmdlo(A6),D0
	AND.B	#$0F,D0
	TST.B	mt_PattDelTime2
	BNE	mt_Return
	ADDQ.B	#1,D0
	MOVE.B	D0,mt_PattDelTime
	RTS

mt_FunkIt
	TST.B	mt_counter
	BNE	mt_Return
	MOVE.B	n_cmdlo(A6),D0
	AND.B	#$0F,D0
	LSL.B	#4,D0
	AND.B	#$0F,n_glissfunk(A6)
	OR.B	D0,n_glissfunk(A6)
	TST.B	D0
	BEQ	mt_Return
mt_UpdateFunk
	MOVEM.L	A0/D1,-(SP)
	MOVEQ	#0,D0
	MOVE.B	n_glissfunk(A6),D0
	LSR.B	#4,D0
	BEQ.S	mt_funkend
	LEA	mt_FunkTable,A0
	MOVE.B	(A0,D0.W),D0
	ADD.B	D0,n_funkoffset(A6)
	BTST	#7,n_funkoffset(A6)
	BEQ.S	mt_funkend
	CLR.B	n_funkoffset(A6)

	MOVE.L	n_loopstart(A6),D0
	MOVEQ	#0,D1
	MOVE.W	n_replen(A6),D1
	ADD.L	D1,D0
	ADD.L	D1,D0
	MOVE.L	n_wavestart(A6),A0
	ADDQ.L	#1,A0
	CMP.L	D0,A0
	BLO.S	mt_funkok
	MOVE.L	n_loopstart(A6),A0
mt_funkok
	MOVE.L	A0,n_wavestart(A6)
	MOVEQ	#-1,D0
	SUB.B	(A0),D0
	MOVE.B	D0,(A0)
mt_funkend
	MOVEM.L	(SP)+,A0/D1
	RTS

;/* End of File */
