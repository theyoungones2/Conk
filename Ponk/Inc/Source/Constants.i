
* Global Game Constants that should be included in each module *

PALMODE		equ	1		;1 = Pal, 0 = NTSC
;RASTERTIME	equ	1		;1 = On,  0 = Off (obsolete, use -r command code)
CHIPASSEMBLE	equ	0
IHANDLER	equ	1
TASKPRIORITY	equ	9

LRB_AGA			equ	0
LRF_AGA			equ	1<<LRB_AGA
LRB_AGAPALETTE		equ	1
LRF_AGAPALETTE		equ	1<<LRB_AGAPALETTE

DBUGLEVEL		equ	0

LACED			equ	1
NONLACED		equ	0

BITSHIFT		equ	5
BARSHIFT		equ	5

MAXLONG 		equ $80000000-1

OB_UPDATETYPE_ML	equ	0
OB_UPDATETYPE_VB	equ	1

OB_TYPE_PLAYER		equ	0
OB_TYPE_PLAYERBULLET	equ	4
OB_TYPE_BADDUDE		equ	8

OB_STATE_DYING		equ	-1
OB_STATE_OFF		equ	0
OB_STATE_ON		equ	1

IM_TYPE_BOB		equ	0
IM_TYPE_SPRITE		equ	1

WAVE_MAP		equ	0
WAVE_VIEWRIGHT		equ	2
WAVE_VIEWLEFT		equ	1

INITALISINGWAVES	equ	10
UPDATINGWAVES		equ	30

PATH_END		equ	0
PATH_RECORDED		equ	1
PATH_LINE		equ	2

NULL			equ	0
TRUE			equ	1
FALSE			equ	0

MAXPLAYERS		equ	4

WEAPONBAYS		equ	16

;Actionlist contexts - defines valid inputs for Actions (operators)
CONTEXT_NONE		equ	0	;a2=slice a5=params
CONTEXT_BOB		equ	1	;a2=slice a5=params a3=bob (any type)
CONTEXT_COLLISION	equ	2	;a2=slice a5=params a3=bob a4=otherbob
CONTEXT_WEAPON		equ	3	;a2=slice a5=params a3=bob a4=weapon
;more soon... maybe

GEVENT_TIMER		equ	1<<0
GEVENT_SCROLLPOS	equ	1<<1

GEVENT_PLAYERDEAD	equ	1<<2
GEVENT_PLAYERMENGONE	equ	1<<3
GEVENT_LEVELEND		equ	1<<4
GEVENT_GAMEOVER		equ	1<<5
GEVENT_SWAVESFINISHED	equ	1<<6
GEVENT_TWAVESFINISHED	equ	1<<7
GEVENT_RWAVESFINISHED	equ	1<<8

GEVENT_USER1		equ	1<<29
GEVENT_USER2		equ	1<<30
GEVENT_USER3		equ	1<<31

;Debug Window Width & Height in characters
;Remember Width _MUST_ be even.
DEBUG_WINDOW_WIDTH	equ	34
DEBUG_WINDOW_HEIGHT	equ	10

;Sprite pointer offsets into copperlist

CC_spr0pth	equ	2
CC_spr0ptl	equ	6
CC_spr1pth	equ	10
CC_spr1ptl	equ	14
CC_spr2pth	equ	18
CC_spr2ptl	equ	22
CC_spr3pth	equ	26
CC_spr3ptl	equ	30
CC_spr4pth	equ	34
CC_spr4ptl	equ	38
CC_spr5pth	equ	42
CC_spr5ptl	equ	46
CC_spr6pth	equ	50
CC_spr6ptl	equ	54
CC_spr7pth	equ	58
CC_spr7ptl	equ	62
CC_SizeOf	equ	8

