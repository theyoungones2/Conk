OBSCheckBorders
	;XYZZY Improve sometime please

	move.w	d2,-(sp)

	btst.b	#0,BB_Flags+3(a3)
	beq	.NoBorder
	btst.b	#1,BB_Flags+3(a3)
	bne	.ViewRelative


	move.l	OB_XPos(a3),d0
	tst.l	BB_XSpeed(a3)
	bgt	.LeftOk
	cmp.l	BB_BorderLeft(a3),d0
	bgt	.LeftOk
	move.l	BB_BorderRight(a3),d1
	sub.l	#1,d1
	move.l	BB_BorderLeft(a3),d0
	bsr	.LeftOvers
	bra	.RightOk
.LeftOk
	cmp.l	BB_BorderRight(a3),d0
	blt	.RightOk
	move.l	BB_BorderLeft(a3),d1
	add.l	#1,d1
	move.l	BB_BorderRight(a3),d0
	bsr	.RightOverThere
.RightOk
	move.l	OB_YPos(a3),d0
	tst.l	BB_YSpeed(a3)
	bgt	.TopOk
	cmp.l	BB_BorderTop(a3),d0
	bgt	.TopOk
	move.l	BB_BorderBottom(a3),d1
	sub.l	#1,d1
	move.l	BB_BorderTop(a3),d0
	bsr	.OverTheTop
	bra	.BottomOk
.TopOk
	cmp.l	BB_BorderBottom(a3),d0
	blt	.BottomOk
	move.l	BB_BorderTop(a3),d1
	add.l	#1,d1
	move.l	BB_BorderBottom(a3),d0
	bsr	.RockBottom
.BottomOk
	bra	.NoBorder
.ViewRelative
	move.l	SS_Scroll(a2),a0

	move.l	OB_XPos(a3),d0
	sub.l	SC_Pos(a0),d0

	tst.l	BB_XSpeed(a3)
	bgt	.VRLeftOk

	cmp.l	BB_BorderLeft(a3),d0
	bgt	.VRRightOk
	move.l	BB_BorderRight(a3),d1
	sub.l	#1,d1
	move.l	BB_BorderLeft(a3),d0
	add.l	SC_Pos(a0),d0
	add.l	SC_Pos(a0),d1
	bsr	.LeftOvers
	bra	.VRRightOk
.VRLeftOk
	cmp.l	BB_BorderRight(a3),d0
	blt	.VRRightOk
	move.l	BB_BorderLeft(a3),d1
	add.l	#1,d1
	move.l	BB_BorderRight(a3),d0
	add.l	SC_Pos(a0),d0
	add.l	SC_Pos(a0),d1
	bsr	.RightOverThere
.VRRightOk
	move.l	OB_YPos(a3),d0

	tst.l	BB_YSpeed(a3)
	bgt	.VRTopOk

	cmp.l	BB_BorderTop(a3),d0
	bgt	.VRTopOk
	move.l	BB_BorderBottom(a3),d1
	sub.l	#1,d1
	move.l	BB_BorderTop(a3),d0
	bsr	.OverTheTop
	bra	.VRBottomOk
.VRTopOk
	cmp.l	BB_BorderBottom(a3),d0
	blt	.VRBottomOk
	move.l	BB_BorderTop(a3),d1
	move.l	BB_BorderBottom(a3),d0
	add.l	#1,d1
	bsr	.RockBottom
.VRBottomOk
.NoBorder
	move.w	(sp)+,d2
	rts


.LeftOvers
	move.b	BB_BorderLeftType(a3),d2
	beq	.Off
	cmp.b	#1,d2
	beq	.Kill
	cmp.b	#2,d2
	beq	.StopL
	cmp.b	#3,d2
	beq	.WrapX
	cmp.b	#4,d2
	beq	.Bounce1X
	cmp.b	#5,d2
	beq	.Bounce2X
	rts

.RightOverThere
	move.b	BB_BorderRightType(a3),d2
	beq	.Off
	cmp.b	#1,d2
	beq	.Kill
	cmp.b	#2,d2
	beq	.StopR
	cmp.b	#3,d2
	beq	.WrapX
	cmp.b	#4,d2
	beq	.Bounce1X
	cmp.b	#5,d2
	beq	.Bounce2X
	rts

.OverTheTop
	move.b	BB_BorderTopType(a3),d2
	beq	.Off
	cmp.b	#1,d2
	beq	.Kill
	cmp.b	#2,d2
	beq	.StopY
	cmp.b	#3,d2
	beq	.WrapY
	cmp.b	#4,d2
	beq	.Bounce1Y
	cmp.b	#5,d2
	beq	.Bounce2Y
	rts

.RockBottom
	move.b	BB_BorderBottomType(a3),d2
	beq	.Off
	cmp.b	#1,d2
	beq	.Kill
	cmp.b	#2,d2
	beq	.StopY
	cmp.b	#3,d2
	beq	.WrapY
	cmp.b	#4,d2
	beq	.Bounce1Y
	cmp.b	#5,d2
	beq	.Bounce2Y
	rts

.Off	rts
.Kill
	bsr	_OBJECTOFF
	rts
.StopL	move.l	SS_Scroll(a2),a0
	move.l	d0,OB_XPos(a3)
	move.l	SC_BaseSpeed(a0),BB_XSpeed(a3)
	rts
.StopR	move.l	d0,OB_XPos(a3)
	move.l	#0,BB_XSpeed(a3)
	rts
.StopY	move.l	d0,OB_YPos(a3)
	move.l	#0,BB_YSpeed(a3)
	rts
.WrapX	move.l	d1,OB_XPos(a3)
	rts
.WrapY	move.l	d1,OB_YPos(a3)
	rts
.Bounce1X
	move.l	d0,OB_XPos(a3)
	neg.l	BB_XSpeed(a3)
	neg.b	BB_Direction(a3)
	rts
.Bounce1Y
	move.l	d0,OB_YPos(a3)
	neg.l	BB_YSpeed(a3)
	move.b	#128,d0
	sub.b	BB_Direction(a3),d0
	move.b	d0,BB_Direction(a3)
	rts
.Bounce2X
	move.l	d0,OB_XPos(a3)
	neg.l	BB_XSpeed(a3)
	rts
.Bounce2Y
	move.l	d0,OB_YPos(a3)
	neg.l	BB_YSpeed(a3)
	rts

