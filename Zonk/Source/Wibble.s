

	XDEF	_PackASCII
	XDEF	_UnpackASCII
	XDEF	_IsPackable
	XDEF	_MakePackable


************************************************************************

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

	move.w	#7,d2					;reset bit pointer
.CharLoop
	tst.w		d0						;no chars left?
	beq			.Done
	move.b	(a0)+,d1			;get a char
	bne			.convert			;hit a null?
	move.b	#64,d1				;map null to '@' (our custom null)
	sub.l		#1,a0					;make sure we stay on the null
.convert
	sub.b		#32,d1
	;pack char into dest
	lsl.b		#2,d1					;ignore top 2 bits of char
	move.w	#6-1,d3				;bit counter
.BitLoop
	bclr.b	d2,(a1)				;preclear dest bit
	lsl.b		d1						;grab char bit
	bcc			.noset				;should it be set?
	bset.b	d2,(a1)				;set it.
.noset
	sub.w		#1,d2					;point to next dest bit.
	bpl			.nextbit			;end of byte?
	add.l		#1,a1					;yep, point to next byte
	move.w	#7,d2					;reset bit pointer
.nextbit
	dbra		d3,.BitLoop

	sub.w		#1,d0
	bra			.CharLoop

.Done
	cmp.w		#7,d2					;partially filled byte?
	beq			.bytepad			;nope, bugger off
.bitpadloop							;pad out rest of byte with zeros
	bclr.b	d2,(a1)
	sub.w		#1,d2
	bpl			.bitpadloop

.bytepad
	add.l	#1,a1
	sub.w	#1,d0
	bmi	.Fuckoff
.bytepadloop
	move.b	#0,(a1)+
	dbra		d0,.bytepadloop

.Fuckoff
	movem.w	(sp)+,d2-d3
	rts

************************************************************************

;VOID __asm UnpackASCII( register __a0 UBYTE *source, register __a1 UBYTE *dest,
;	register __d0 UWORD chrcount );

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


************************************************************************

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


************************************************************************

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
	sub.b		#'a'-'A',d2				;to uppercase

.checkvalidity
	move.b	d2,d0
	bsr			_IsPackable
	tst.l		d0
	bne			.put
	;not packable
	move.l	#1,d3						;changed one...
	move.b	#' ',d2					;...to a space.
.put
	move.b	d2,(a0)+
	bra		.snarfchar

.done
	move.l	d3,d0
	movem.l	(sp)+,d2-d3
	rts


