**************************************************
*    ----- Protracker V2.1A Playroutine -----    *
* Peter "CRAYON" Hanning / Mushroom Studios 1992 *
*    Vinterstigen 12, 14440 Ronninge, Sweden     *
**************************************************

;---- cia interrupt ----

ciatalo = $400
ciatahi = $500
ciatblo = $600
ciatbhi = $700
ciacra  = $e00
ciacrb  = $f00

setciaint
	moveq	#2,d6
	lea	$bfd000,a5
	move.b	#'b',ciaaname+3
setcialoop
	moveq	#0,d0
	lea	ciaaname,a1
	CALL	_SysBase,OpenResource
	move.l	d0,ciaabase
	beq	mt_return

	move.l	_GfxBase,a1
	move.w	gb_DisplayFlags(a1),d0	; displayflags
	btst	#PALn,d0		; pal?
	beq	WasNTSC
	move.l	#1773447,d7 ; pal
	bra.s	sciask
WasNTSC	move.l	#1789773,d7 ; ntsc
sciask	move.l	d7,timervalue
	divu	#125,d7 ; default to normal 50 hz timer

	move.l	ciaabase,a6
	cmp.w	#2,d6
	beq.s	trytimera
trytimerb
	lea	musicintserver,a1
	moveq	#1,d0	; bit 1: timer b
	CALL	ciaabase,AddICRVector
	move.l	#1,timerflag
	tst.l	d0
	bne.s	ciaerror
	move.l	a5,ciaaaddr
	move.b	d7,ciatblo(a5)
	lsr.w	#8,d7
	move.b	d7,ciatbhi(a5)
	bset	#0,ciacrb(a5)
	rts

trytimera
	lea	musicintserver,a1
	moveq	#0,d0	; bit 0: timer a
	CALL	ciaabase,AddICRVector
	clr.l	timerflag
	tst.l	d0
	bne.s	ciaerror
	move.l	a5,ciaaaddr
	move.b	d7,ciatalo(a5)
	lsr.w	#8,d7
	move.b	d7,ciatahi(a5)
	bset	#0,ciacra(a5)
	rts

ciaerror
	move.b	#'a',ciaaname+3
	lea	$bfe001,a5
	subq.w	#1,d6
	bne	setcialoop
	clr.l	ciaabase
	rts

resetciaint
	move.l	ciaabase,d0
	beq	mt_return
	clr.l	ciaabase
	move.l	d0,a6
	move.l	ciaaaddr,a5
	tst.l	timerflag
	beq.s	restimera

	bclr	#0,ciacrb(a5)
	moveq	#1,d0
	bra.s	remint

restimera
	bclr	#0,ciacra(a5)
	moveq	#0,d0
remint	lea	musicintserver,a1
	moveq	#0,d0
	jsr	_LVORemICRVector(a6)	;Don't Use CALL (base is zero'd above)
	rts

;---- tempo ----

settempo
	move.l	ciaabase,d2
	beq	mt_return
	cmp.w	#32,d0
	bhs.s	setemsk
	moveq	#32,d0
setemsk	move.w	d0,realtempo
	move.l	timervalue,d2
	divu	d0,d2
	move.l	ciaaaddr,a4
	move.l	timerflag,d0
	beq.s	settema
	move.b	d2,ciatblo(a4)
	lsr.w	#8,d2
	move.b	d2,ciatbhi(a4)
	rts

settema	move.b	d2,ciatalo(a4)
	lsr.w	#8,d2
	move.b	d2,ciatahi(a4)
	rts

;---- playroutine ----

n_note		equ	0  ; w
n_cmd		equ	2  ; w
n_cmdlo		equ	3  ; b
n_start		equ	4  ; l
n_length	equ	8  ; w
n_loopstart	equ	10 ; l
n_replen	equ	14 ; w
n_period	equ	16 ; w
n_finetune	equ	18 ; b
n_volume	equ	19 ; b
n_dmabit	equ	20 ; w
n_toneportdirec	equ	22 ; b
n_toneportspeed	equ	23 ; b
n_wantedperiod	equ	24 ; w
n_vibratocmd	equ	26 ; b
n_vibratopos	equ	27 ; b
n_tremolocmd	equ	28 ; b
n_tremolopos	equ	29 ; b
n_wavecontrol	equ	30 ; b
n_glissfunk	equ	31 ; b
n_sampleoffset	equ	32 ; b
n_pattpos	equ	33 ; b
n_loopcount	equ	34 ; b
n_funkoffset	equ	35 ; b
n_wavestart	equ	36 ; l
n_reallength	equ	40 ; w

mt_init	move.l	FileBuf,a0
	move.l	a0,mt_songdataptr
	move.l	a0,a1
	lea	952(a1),a1		;Skip Header Crap
	moveq	#127,d0			;d0 = Counter
	moveq	#0,d1			;d1 = 0
mtloop	move.l	d1,d2
	subq.w	#1,d0
mtloop2	move.b	(a1)+,d1
	cmp.b	d2,d1
	bgt.s	mtloop
	dbra	d0,mtloop2
	addq.b	#1,d2

	lea	mt_samplestarts,a1
	asl.l	#8,d2
	asl.l	#2,d2
	add.l	#1084,d2
	add.l	a0,d2
	move.l	d2,a2
	moveq	#30,d0
mtloop3	clr.l	(a2)
	move.l	a2,(a1)+
	moveq	#0,d1
	move.w	42(a0),d1
	asl.l	#1,d1
	add.l	d1,a2
	add.l	#30,a0
	dbra	d0,mtloop3

	or.b	#2,$bfe001
	move.b	#6,mt_speed
	clr.b	mt_counter
	clr.b	mt_songpos
	clr.w	mt_patternpos
mt_end	sf	mt_enable
	lea	$dff000,a0
	clr.w	$a8(a0)
	clr.w	$b8(a0)
	clr.w	$c8(a0)
	clr.w	$d8(a0)
	move.w	#$f,$dff096
	rts

mt_music
	movem.l	d0-d4/a0-a6,-(sp)
	tst.b	mt_enable
	beq	mt_exit
	addq.b	#1,mt_counter
	move.b	mt_counter,d0
	cmp.b	mt_speed,d0
	blo.s	mt_nonewnote
	clr.b	mt_counter
	tst.b	mt_pattdeltime2
	beq.s	mt_getnewnote
	bsr.s	mt_nonewallchannels
	bra	mt_dskip

mt_nonewnote
	bsr.s	mt_nonewallchannels
	bra	mt_nonewposyet

mt_nonewallchannels
	lea	$dff0a0,a5
	lea	mt_chan1temp,a6
	bsr	mt_checkefx
	lea	$dff0b0,a5
	lea	mt_chan2temp,a6
	bsr	mt_checkefx
	lea	$dff0c0,a5
	lea	mt_chan3temp,a6
	bsr	mt_checkefx
	lea	$dff0d0,a5
	lea	mt_chan4temp,a6
	bra	mt_checkefx

mt_getnewnote
	move.l	mt_songdataptr,a0
	lea	12(a0),a3
	lea	952(a0),a2	;pattpo
	lea	1084(a0),a0	;patterndata
	moveq	#0,d0
	moveq	#0,d1
	move.b	mt_songpos,d0
	move.b	(a2,d0.w),d1
	asl.l	#8,d1
	asl.l	#2,d1
	add.w	mt_patternpos,d1
	clr.w	mt_dmacontemp

	lea	$dff0a0,a5
	lea	mt_chan1temp,a6
	bsr.s	mt_playvoice
	lea	$dff0b0s,a5
	lea	mt_chan2temp,a6
	bsr.s	mt_playvoice
	lea	$dff0c0,a5
	lea	mt_chan3temp,a6
	bsr.s	mt_playvoice
	lea	$dff0d0,a5
	lea	mt_chan4temp,a6
	bsr.s	mt_playvoice
	bra	mt_setdma

mt_playvoice
	tst.l	(a6)
	bne.s	mt_plvskip
	bsr	mt_pernop
mt_plvskip
	move.l	(a0,d1.l),(a6)
	addq.l	#4,d1
	moveq	#0,d2
	move.b	n_cmd(a6),d2
	and.b	#$f0,d2
	lsr.b	#4,d2
	move.b	(a6),d0
	and.b	#$f0,d0
	or.b	d0,d2
	tst.b	d2
	beq	mt_setregs
	moveq	#0,d3
	lea	mt_samplestarts,a1
	move	d2,d4
	subq.l	#1,d2
	asl.l	#2,d2
	mulu	#30,d4
	move.l	(a1,d2.l),n_start(a6)
	move.w	(a3,d4.l),n_length(a6)
	move.w	(a3,d4.l),n_reallength(a6)
	move.b	2(a3,d4.l),n_finetune(a6)
	move.b	3(a3,d4.l),n_volume(a6)
	move.w	4(a3,d4.l),d3 ; get repeat
	tst.w	d3
	beq.s	mt_noloop
	move.l	n_start(a6),d2	; get start
	asl.w	#1,d3
	add.l	d3,d2		; add repeat
	move.l	d2,n_loopstart(a6)
	move.l	d2,n_wavestart(a6)
	move.w	4(a3,d4.l),d0	; get repeat
	add.w	6(a3,d4.l),d0	; add replen
	move.w	d0,n_length(a6)
	move.w	6(a3,d4.l),n_replen(a6)	; save replen
	moveq	#0,d0
	move.b	n_volume(a6),d0
	move.w	d0,8(a5)	; set volume
	bra.s	mt_setregs

mt_noloop
	move.l	n_start(a6),d2
	add.l	d3,d2
	move.l	d2,n_loopstart(a6)
	move.l	d2,n_wavestart(a6)
	move.w	6(a3,d4.l),n_replen(a6)	; save replen
	moveq	#0,d0
	move.b	n_volume(a6),d0
	move.w	d0,8(a5)	; set volume
mt_setregs
	move.w	(a6),d0
	and.w	#$0fff,d0
	beq	mt_checkmoreefx	; if no note
	move.w	2(a6),d0
	and.w	#$0ff0,d0
	cmp.w	#$0e50,d0
	beq.s	mt_dosetfinetune
	move.b	2(a6),d0
	and.b	#$0f,d0
	cmp.b	#3,d0	; toneportamento
	beq.s	mt_chktoneporta
	cmp.b	#5,d0
	beq.s	mt_chktoneporta
	cmp.b	#9,d0	; sample offset
	bne.s	mt_setperiod
	bsr	mt_checkmoreefx
	bra.s	mt_setperiod

mt_dosetfinetune
	bsr	mt_setfinetune
	bra.s	mt_setperiod

mt_chktoneporta
	bsr	mt_settoneporta
	bra	mt_checkmoreefx

mt_setperiod
	movem.l	d0-d1/a0-a1,-(sp)
	move.w	(a6),d1
	and.w	#$0fff,d1
	lea	mt_periodtable,a1
	moveq	#0,d0
	moveq	#36,d2
mt_ftuloop
	cmp.w	(a1,d0.w),d1
	bhs.s	mt_ftufound
	addq.l	#2,d0
	dbra	d2,mt_ftuloop
mt_ftufound
	moveq	#0,d1
	move.b	n_finetune(a6),d1
	mulu	#36*2,d1
	add.l	d1,a1
	move.w	(a1,d0.w),n_period(a6)
	movem.l	(sp)+,d0-d1/a0-a1

	move.w	2(a6),d0
	and.w	#$0ff0,d0
	cmp.w	#$0ed0,d0 ; notedelay
	beq	mt_checkmoreefx

	move.w	n_dmabit(a6),$dff096
	btst	#2,n_wavecontrol(a6)
	bne.s	mt_vibnoc
	clr.b	n_vibratopos(a6)
mt_vibnoc
	btst	#6,n_wavecontrol(a6)
	bne.s	mt_trenoc
	clr.b	n_tremolopos(a6)
mt_trenoc
	move.l	n_start(a6),(a5)	; set start
	move.w	n_length(a6),4(a5)	; set length
	move.w	n_period(a6),d0
	move.w	d0,6(a5)		; set period
	move.w	n_dmabit(a6),d0
	or.w	d0,mt_dmacontemp
	bra	mt_checkmoreefx
 
mt_setdma
	move	mt_dmadelay,d0
mt_waitdma
	dbra	d0,mt_waitdma
	move.w	mt_dmacontemp,d0
	or.w	#$8000,d0
	move.w	d0,$dff096
	move.w	#300,d0
mt_waitdma2
	dbra	d0,mt_waitdma2

	lea	$dff000,a5
	lea	mt_chan4temp,a6
	move.l	n_loopstart(a6),$d0(a5)
	move.w	n_replen(a6),$d4(a5)
	lea	mt_chan3temp,a6
	move.l	n_loopstart(a6),$c0(a5)
	move.w	n_replen(a6),$c4(a5)
	lea	mt_chan2temp,a6
	move.l	n_loopstart(a6),$b0(a5)
	move.w	n_replen(a6),$b4(a5)
	lea	mt_chan1temp,a6
	move.l	n_loopstart(a6),$a0(a5)
	move.w	n_replen(a6),$a4(a5)

mt_dskip
	add.w	#16,mt_patternpos
	move.b	mt_pattdeltime,d0
	beq.s	mt_dskc
	move.b	d0,mt_pattdeltime2
	clr.b	mt_pattdeltime
mt_dskc	tst.b	mt_pattdeltime2
	beq.s	mt_dska
	subq.b	#1,mt_pattdeltime2
	beq.s	mt_dska
	sub.w	#16,mt_patternpos
mt_dska	tst.b	mt_pbreakflag
	beq.s	mt_nnpysk
	sf	mt_pbreakflag
	moveq	#0,d0
	move.b	mt_pbreakpos,d0
	clr.b	mt_pbreakpos
	lsl.w	#4,d0
	move.w	d0,mt_patternpos
mt_nnpysk
	cmp.w	#1024,mt_patternpos
	blo.s	mt_nonewposyet
mt_nextposition	
	moveq	#0,d0
	move.b	mt_pbreakpos,d0
	lsl.w	#4,d0
	move.w	d0,mt_patternpos
	clr.b	mt_pbreakpos
	clr.b	mt_posjumpflag

	addq.b	#1,mt_songpos
	and.b	#$7f,mt_songpos
	move.b	mt_songpos,d1
	move.l	mt_songdataptr,a0
	cmp.b	950(a0),d1
	blo.s	mt_nonewposyet
	clr.b	mt_songpos
mt_nonewposyet	
	tst.b	mt_posjumpflag
	bne.s	mt_nextposition
mt_exit	movem.l	(sp)+,d0-d4/a0-a6
	rts

mt_checkefx
	bsr	mt_updatefunk
	move.w	n_cmd(a6),d0
	and.w	#$0fff,d0
	beq.s	mt_pernop
	move.b	n_cmd(a6),d0
	and.b	#$0f,d0
	beq.s	mt_arpeggio
	cmp.b	#1,d0
	beq	mt_portaup
	cmp.b	#2,d0
	beq	mt_portadown
	cmp.b	#3,d0
	beq	mt_toneportamento
	cmp.b	#4,d0
	beq	mt_vibrato
	cmp.b	#5,d0
	beq	mt_toneplusvolslide
	cmp.b	#6,d0
	beq	mt_vibratoplusvolslide
	cmp.b	#$e,d0
	beq	mt_e_commands
setback	move.w	n_period(a6),6(a5)
	cmp.b	#7,d0
	beq	mt_tremolo
	cmp.b	#$a,d0
	beq	mt_volumeslide
mt_return
	rts

mt_pernop
	move.w	n_period(a6),6(a5)
	rts

mt_arpeggio
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
	lea	mt_periodtable,a0
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

mt_fineportaup
	tst.b	mt_counter
	bne.s	mt_return
	move.b	#$0f,mt_lowmask
mt_portaup
	moveq	#0,d0
	move.b	n_cmdlo(a6),d0
	and.b	mt_lowmask,d0
	move.b	#$ff,mt_lowmask
	sub.w	d0,n_period(a6)
	move.w	n_period(a6),d0
	and.w	#$0fff,d0
	cmp.w	#113,d0
	bpl.s	mt_portauskip
	and.w	#$f000,n_period(a6)
	or.w	#113,n_period(a6)
mt_portauskip
	move.w	n_period(a6),d0
	and.w	#$0fff,d0
	move.w	d0,6(a5)
	rts	
 
mt_fineportadown
	tst.b	mt_counter
	bne	mt_return
	move.b	#$0f,mt_lowmask
mt_portadown
	clr.w	d0
	move.b	n_cmdlo(a6),d0
	and.b	mt_lowmask,d0
	move.b	#$ff,mt_lowmask
	add.w	d0,n_period(a6)
	move.w	n_period(a6),d0
	and.w	#$0fff,d0
	cmp.w	#856,d0
	bmi.s	mt_portadskip
	and.w	#$f000,n_period(a6)
	or.w	#856,n_period(a6)
mt_portadskip
	move.w	n_period(a6),d0
	and.w	#$0fff,d0
	move.w	d0,6(a5)
	rts

mt_settoneporta
	move.l	a0,-(sp)
	move.w	(a6),d2
	and.w	#$0fff,d2
	moveq	#0,d0
	move.b	n_finetune(a6),d0
	mulu	#37*2,d0
	lea	mt_periodtable,a0
	add.l	d0,a0
	moveq	#0,d0
mt_stploop
	cmp.w	(a0,d0.w),d2
	bhs.s	mt_stpfound
	addq.w	#2,d0
	cmp.w	#37*2,d0
	blo.s	mt_stploop
	moveq	#35*2,d0
mt_stpfound
	move.b	n_finetune(a6),d2
	and.b	#8,d2
	beq.s	mt_stpgoss
	tst.w	d0
	beq.s	mt_stpgoss
	subq.w	#2,d0
mt_stpgoss
	move.w	(a0,d0.w),d2
	move.l	(sp)+,a0
	move.w	d2,n_wantedperiod(a6)
	move.w	n_period(a6),d0
	clr.b	n_toneportdirec(a6)
	cmp.w	d0,d2
	beq.s	mt_cleartoneporta
	bge	mt_return
	move.b	#1,n_toneportdirec(a6)
	rts

mt_cleartoneporta
	clr.w	n_wantedperiod(a6)
	rts

mt_toneportamento
	move.b	n_cmdlo(a6),d0
	beq.s	mt_toneportnochange
	move.b	d0,n_toneportspeed(a6)
	clr.b	n_cmdlo(a6)
mt_toneportnochange
	tst.w	n_wantedperiod(a6)
	beq	mt_return
	moveq	#0,d0
	move.b	n_toneportspeed(a6),d0
	tst.b	n_toneportdirec(a6)
	bne.s	mt_toneportaup
mt_toneportadown
	add.w	d0,n_period(a6)
	move.w	n_wantedperiod(a6),d0
	cmp.w	n_period(a6),d0
	bgt.s	mt_toneportasetper
	move.w	n_wantedperiod(a6),n_period(a6)
	clr.w	n_wantedperiod(a6)
	bra.s	mt_toneportasetper

mt_toneportaup
	sub.w	d0,n_period(a6)
	move.w	n_wantedperiod(a6),d0
	cmp.w	n_period(a6),d0
	blt.s	mt_toneportasetper
	move.w	n_wantedperiod(a6),n_period(a6)
	clr.w	n_wantedperiod(a6)

mt_toneportasetper
	move.w	n_period(a6),d2
	move.b	n_glissfunk(a6),d0
	and.b	#$0f,d0
	beq.s	mt_glissskip
	moveq	#0,d0
	move.b	n_finetune(a6),d0
	mulu	#36*2,d0
	lea	mt_periodtable,a0
	add.l	d0,a0
	moveq	#0,d0
mt_glissloop
	cmp.w	(a0,d0.w),d2
	bhs.s	mt_glissfound
	addq.w	#2,d0
	cmp.w	#36*2,d0
	blo.s	mt_glissloop
	moveq	#35*2,d0
mt_glissfound
	move.w	(a0,d0.w),d2
mt_glissskip
	move.w	d2,6(a5) ; set period
	rts

mt_vibrato
	move.b	n_cmdlo(a6),d0
	beq.s	mt_vibrato2
	move.b	n_vibratocmd(a6),d2
	and.b	#$0f,d0
	beq.s	mt_vibskip
	and.b	#$f0,d2
	or.b	d0,d2
mt_vibskip
	move.b	n_cmdlo(a6),d0
	and.b	#$f0,d0
	beq.s	mt_vibskip2
	and.b	#$0f,d2
	or.b	d0,d2
mt_vibskip2
	move.b	d2,n_vibratocmd(a6)
mt_vibrato2
	move.b	n_vibratopos(a6),d0
	lea	mt_vibratotable,a4
	lsr.w	#2,d0
	and.w	#$001f,d0
	moveq	#0,d2
	move.b	n_wavecontrol(a6),d2
	and.b	#$03,d2
	beq.s	mt_vib_sine
	lsl.b	#3,d0
	cmp.b	#1,d2
	beq.s	mt_vib_rampdown
	move.b	#255,d2
	bra.s	mt_vib_set
mt_vib_rampdown
	tst.b	n_vibratopos(a6)
	bpl.s	mt_vib_rampdown2
	move.b	#255,d2
	sub.b	d0,d2
	bra.s	mt_vib_set
mt_vib_rampdown2
	move.b	d0,d2
	bra.s	mt_vib_set
mt_vib_sine
	move.b	(a4,d0.w),d2
mt_vib_set
	move.b	n_vibratocmd(a6),d0
	and.w	#15,d0
	mulu	d0,d2
	lsr.w	#7,d2
	move.w	n_period(a6),d0
	tst.b	n_vibratopos(a6)
	bmi.s	mt_vibratoneg
	add.w	d2,d0
	bra.s	mt_vibrato3
mt_vibratoneg
	sub.w	d2,d0
mt_vibrato3
	move.w	d0,6(a5)
	move.b	n_vibratocmd(a6),d0
	lsr.w	#2,d0
	and.w	#$003c,d0
	add.b	d0,n_vibratopos(a6)
	rts

mt_toneplusvolslide
	bsr	mt_toneportnochange
	bra	mt_volumeslide

mt_vibratoplusvolslide
	bsr.s	mt_vibrato2
	bra	mt_volumeslide

mt_tremolo
	move.b	n_cmdlo(a6),d0
	beq.s	mt_tremolo2
	move.b	n_tremolocmd(a6),d2
	and.b	#$0f,d0
	beq.s	mt_treskip
	and.b	#$f0,d2
	or.b	d0,d2
mt_treskip
	move.b	n_cmdlo(a6),d0
	and.b	#$f0,d0
	beq.s	mt_treskip2
	and.b	#$0f,d2
	or.b	d0,d2
mt_treskip2
	move.b	d2,n_tremolocmd(a6)
mt_tremolo2
	move.b	n_tremolopos(a6),d0
	lea	mt_vibratotable,a4
	lsr.w	#2,d0
	and.w	#$001f,d0
	moveq	#0,d2
	move.b	n_wavecontrol(a6),d2
	lsr.b	#4,d2
	and.b	#$03,d2
	beq.s	mt_tre_sine
	lsl.b	#3,d0
	cmp.b	#1,d2
	beq.s	mt_tre_rampdown
	move.b	#255,d2
	bra.s	mt_tre_set
mt_tre_rampdown
	tst.b	n_vibratopos(a6)
	bpl.s	mt_tre_rampdown2
	move.b	#255,d2
	sub.b	d0,d2
	bra.s	mt_tre_set
mt_tre_rampdown2
	move.b	d0,d2
	bra.s	mt_tre_set
mt_tre_sine
	move.b	(a4,d0.w),d2
mt_tre_set
	move.b	n_tremolocmd(a6),d0
	and.w	#15,d0
	mulu	d0,d2
	lsr.w	#6,d2
	moveq	#0,d0
	move.b	n_volume(a6),d0
	tst.b	n_tremolopos(a6)
	bmi.s	mt_tremoloneg
	add.w	d2,d0
	bra.s	mt_tremolo3
mt_tremoloneg
	sub.w	d2,d0
mt_tremolo3
	bpl.s	mt_tremoloskip
	clr.w	d0
mt_tremoloskip
	cmp.w	#$40,d0
	bls.s	mt_tremolook
	move.w	#$40,d0
mt_tremolook
	move.w	d0,8(a5)
	move.b	n_tremolocmd(a6),d0
	lsr.w	#2,d0
	and.w	#$003c,d0
	add.b	d0,n_tremolopos(a6)
	rts

mt_sampleoffset
	moveq	#0,d0
	move.b	n_cmdlo(a6),d0
	beq.s	mt_sononew
	move.b	d0,n_sampleoffset(a6)
mt_sononew
	move.b	n_sampleoffset(a6),d0
	lsl.w	#7,d0
	cmp.w	n_length(a6),d0
	bge.s	mt_sofskip
	sub.w	d0,n_length(a6)
	lsl.w	#1,d0
	add.l	d0,n_start(a6)
	rts
mt_sofskip
	move.w	#$0001,n_length(a6)
	rts

mt_volumeslide
	moveq	#0,d0
	move.b	n_cmdlo(a6),d0
	lsr.b	#4,d0
	tst.b	d0
	beq.s	mt_volslidedown
mt_volslideup
	add.b	d0,n_volume(a6)
	cmp.b	#$40,n_volume(a6)
	bmi.s	mt_vsuskip
	move.b	#$40,n_volume(a6)
mt_vsuskip
	move.b	n_volume(a6),d0
	move.w	d0,8(a5)
	rts

mt_volslidedown
	moveq	#0,d0
	move.b	n_cmdlo(a6),d0
	and.b	#$0f,d0
mt_volslidedown2
	sub.b	d0,n_volume(a6)
	bpl.s	mt_vsdskip
	clr.b	n_volume(a6)
mt_vsdskip
	move.b	n_volume(a6),d0
	move.w	d0,8(a5)
	rts

mt_positionjump
	move.b	n_cmdlo(a6),d0
	subq.b	#1,d0
	move.b	d0,mt_songpos
mt_pj2	clr.b	mt_pbreakpos
	st 	mt_posjumpflag
	rts

mt_volumechange
	moveq	#0,d0
	move.b	n_cmdlo(a6),d0
	cmp.b	#$40,d0
	bls.s	mt_volumeok
	moveq	#$40,d0
mt_volumeok
	move.b	d0,n_volume(a6)
	move.w	d0,8(a5)
	rts

mt_patternbreak
	moveq	#0,d0
	move.b	n_cmdlo(a6),d0
	move.l	d0,d2
	lsr.b	#4,d0
	mulu	#10,d0
	and.b	#$0f,d2
	add.b	d2,d0
	cmp.b	#63,d0
	bhi.s	mt_pj2
	move.b	d0,mt_pbreakpos
	st	mt_posjumpflag
	rts

mt_setspeed
	moveq	#0,d0
	move.b	3(a6),d0
	beq	mt_end
	cmp.b	#32,d0
	bhs	settempo
	clr.b	mt_counter
	move.b	d0,mt_speed
	rts

mt_checkmoreefx
	bsr	mt_updatefunk
	move.b	2(a6),d0
	and.b	#$0f,d0
	cmp.b	#$9,d0
	beq	mt_sampleoffset
	cmp.b	#$b,d0
	beq	mt_positionjump
	cmp.b	#$d,d0
	beq.s	mt_patternbreak
	cmp.b	#$e,d0
	beq.s	mt_e_commands
	cmp.b	#$f,d0
	beq.s	mt_setspeed
	cmp.b	#$c,d0
	beq	mt_volumechange
	bra	mt_pernop

mt_e_commands
	move.b	n_cmdlo(a6),d0
	and.b	#$f0,d0
	lsr.b	#4,d0
	beq.s	mt_filteronoff
	cmp.b	#1,d0
	beq	mt_fineportaup
	cmp.b	#2,d0
	beq	mt_fineportadown
	cmp.b	#3,d0
	beq.s	mt_setglisscontrol
	cmp.b	#4,d0
	beq	mt_setvibratocontrol
	cmp.b	#5,d0
	beq	mt_setfinetune
	cmp.b	#6,d0
	beq	mt_jumploop
	cmp.b	#7,d0
	beq	mt_settremolocontrol
	cmp.b	#9,d0
	beq	mt_retrignote
	cmp.b	#$a,d0
	beq	mt_volumefineup
	cmp.b	#$b,d0
	beq	mt_volumefinedown
	cmp.b	#$c,d0
	beq	mt_notecut
	cmp.b	#$d,d0
	beq	mt_notedelay
	cmp.b	#$e,d0
	beq	mt_patterndelay
	cmp.b	#$f,d0
	beq	mt_funkit
	rts

mt_filteronoff
	move.b	n_cmdlo(a6),d0
	and.b	#1,d0
	asl.b	#1,d0
	and.b	#$fd,$bfe001
	or.b	d0,$bfe001
	rts	

mt_setglisscontrol
	move.b	n_cmdlo(a6),d0
	and.b	#$0f,d0
	and.b	#$f0,n_glissfunk(a6)
	or.b	d0,n_glissfunk(a6)
	rts

mt_setvibratocontrol
	move.b	n_cmdlo(a6),d0
	and.b	#$0f,d0
	and.b	#$f0,n_wavecontrol(a6)
	or.b	d0,n_wavecontrol(a6)
	rts

mt_setfinetune
	move.b	n_cmdlo(a6),d0
	and.b	#$0f,d0
	move.b	d0,n_finetune(a6)
	rts

mt_jumploop
	tst.b	mt_counter
	bne	mt_return
	move.b	n_cmdlo(a6),d0
	and.b	#$0f,d0
	beq.s	mt_setloop
	tst.b	n_loopcount(a6)
	beq.s	mt_jumpcnt
	subq.b	#1,n_loopcount(a6)
	beq	mt_return
mt_jmploop	move.b	n_pattpos(a6),mt_pbreakpos
	st	mt_pbreakflag
	rts

mt_jumpcnt
	move.b	d0,n_loopcount(a6)
	bra.s	mt_jmploop

mt_setloop
	move.w	mt_patternpos,d0
	lsr.w	#4,d0
	move.b	d0,n_pattpos(a6)
	rts

mt_settremolocontrol
	move.b	n_cmdlo(a6),d0
	and.b	#$0f,d0
	lsl.b	#4,d0
	and.b	#$0f,n_wavecontrol(a6)
	or.b	d0,n_wavecontrol(a6)
	rts

mt_retrignote
	move.l	d1,-(sp)
	moveq	#0,d0
	move.b	n_cmdlo(a6),d0
	and.b	#$0f,d0
	beq.s	mt_rtnend
	moveq	#0,d1
	move.b	mt_counter,d1
	bne.s	mt_rtnskp
	move.w	(a6),d1
	and.w	#$0fff,d1
	bne.s	mt_rtnend
	moveq	#0,d1
	move.b	mt_counter,d1
mt_rtnskp
	divu	d0,d1
	swap	d1
	tst.w	d1
	bne.s	mt_rtnend
mt_doretrig
	move.w	n_dmabit(a6),$dff096	; channel dma off
	move.l	n_start(a6),(a5)	; set sampledata pointer
	move.w	n_length(a6),4(a5)	; set length
	move.w	#300,d0
mt_rtnloop1
	dbra	d0,mt_rtnloop1
	move.w	n_dmabit(a6),d0
	bset	#15,d0
	move.w	d0,$dff096
	move.w	#300,d0
mt_rtnloop2
	dbra	d0,mt_rtnloop2
	move.l	n_loopstart(a6),(a5)
	move.l	n_replen(a6),4(a5)
mt_rtnend
	move.l	(sp)+,d1
	rts

mt_volumefineup
	tst.b	mt_counter
	bne	mt_return
	moveq	#0,d0
	move.b	n_cmdlo(a6),d0
	and.b	#$f,d0
	bra	mt_volslideup

mt_volumefinedown
	tst.b	mt_counter
	bne	mt_return
	moveq	#0,d0
	move.b	n_cmdlo(a6),d0
	and.b	#$0f,d0
	bra	mt_volslidedown2

mt_notecut
	moveq	#0,d0
	move.b	n_cmdlo(a6),d0
	and.b	#$0f,d0
	cmp.b	mt_counter,d0
	bne	mt_return
	clr.b	n_volume(a6)
	move.w	#0,8(a5)
	rts

mt_notedelay
	moveq	#0,d0
	move.b	n_cmdlo(a6),d0
	and.b	#$0f,d0
	cmp.b	mt_counter,d0
	bne	mt_return
	move.w	(a6),d0
	beq	mt_return
	move.l	d1,-(sp)
	bra	mt_doretrig

mt_patterndelay
	tst.b	mt_counter
	bne	mt_return
	moveq	#0,d0
	move.b	n_cmdlo(a6),d0
	and.b	#$0f,d0
	tst.b	mt_pattdeltime2
	bne	mt_return
	addq.b	#1,d0
	move.b	d0,mt_pattdeltime
	rts

mt_funkit
	tst.b	mt_counter
	bne	mt_return
	move.b	n_cmdlo(a6),d0
	and.b	#$0f,d0
	lsl.b	#4,d0
	and.b	#$0f,n_glissfunk(a6)
	or.b	d0,n_glissfunk(a6)
	tst.b	d0
	beq	mt_return
mt_updatefunk
	movem.l	a0/d1,-(sp)
	moveq	#0,d0
	move.b	n_glissfunk(a6),d0
	lsr.b	#4,d0
	beq.s	mt_funkend
	lea	mt_funktable,a0
	move.b	(a0,d0.w),d0
	add.b	d0,n_funkoffset(a6)
	btst	#7,n_funkoffset(a6)
	beq.s	mt_funkend
	clr.b	n_funkoffset(a6)

	move.l	n_loopstart(a6),d0
	moveq	#0,d1
	move.w	n_replen(a6),d1
	add.l	d1,d0
	add.l	d1,d0
	move.l	n_wavestart(a6),a0
	addq.l	#1,a0
	cmp.l	d0,a0
	blo.s	mt_funkok
	move.l	n_loopstart(a6),a0
mt_funkok
	move.l	a0,n_wavestart(a6)
	moveq	#-1,d0
	sub.b	(a0),d0
	move.b	d0,(a0)
mt_funkend
	movem.l	(sp)+,a0/d1
	rts

