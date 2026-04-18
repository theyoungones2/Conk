	include "include:exec/types.i"
	include "include:exec/lists.i"
;Lame.i - see lame.c

LAME_FAILAT		equ	20
LAMERR_ALLOK		equ	0
LAMERR_NOTFOUND		equ	LAME_FAILAT
LAMERR_READ		equ	(LAME_FAILAT+1)
LAMERR_MEM		equ	(LAME_FAILAT+2)
LAMERR_CHIPMEM		equ	(LAME_FAILAT+3)
LAMERR_FUCKUP		equ	(LAME_FAILAT+4)

TEMARG_END		equ	0
TEMARG_BYTE		equ	1
TEMARG_WORD		equ	2
TEMARG_LONG		equ	3
TEMARG_STRING		equ	4
TEMARG_IMAGEREF		equ	5
TEMARG_ANIMREF		equ	6
TEMARG_DUDEREF		equ	7
TEMARG_PROGREF		equ	8
TEMARG_SFXREF		equ	9
TEMARG_ARRAY		equ	10
TEMARG_OPERATOR		equ	11
TEMARG_PLYRREF		equ	12
TEMARG_WEAPREF		equ	13
TEMARG_PATHREF		equ	14
TEMARG_RAWFILEREF	equ	15
TEMARG_LEVELREF		equ	16
TEMARG_EOTRREF		equ	17

ID_FORM			equ	'FORM'
ID_CONK			equ	'CONK'
ID_CMAP			equ	'CMAP'
ID_BLKS			equ	'BLKS'
ID_BOBS			equ	'BOBS'
ID_MAP			equ	'MAP '


	STRUCTURE LameReqStruct,0
	;This field should be filled before calling LoadFiles()
	APTR		lr_Operators			;List of template operator routine addresses
	APTR		lr_Templates			;List of templates associated with each operator
	APTR		lr_OperatorNames		;List of names associated with each template
	APTR		lr_ShieldBarImagePtrs		;Ptr to Array[17] of pointers to images
	;All other fields are filled out by LoadFiles()
	UWORD		lr_Error			;error code returned by LoadFiles()
	UWORD		lr_FileNum			;which file error occurred on
	;LoadFiles() puts ptrs to cooked data here
	APTR		lr_FirstSlice			;
	UWORD		lr_FrameRate			;
	UWORD		lr_CopperLists			;
	APTR		lr_CopperList1			;
	APTR		lr_CopperList2			;
	APTR		lr_InterruptCopperWait		;struct CopperWaitNode *
	UWORD		lr_TheFlags			;
	UWORD		lr_ShieldBarHeight		;Height
	APTR		lr_LevelUpdateProg
	LABEL		lr_SizeOf

	STRUCTURE AnimInfoNode,0
	STRUCT		ain_FrameList,MLH_SIZE
	APTR		ain_BMHD
	APTR		ain_BODY
	APTR		ain_CMAP
	; Rest of this data to be filled out when cooking
	APTR		ain_RealBitMap1
	APTR		ain_RealBitMap2
	APTR		ain_BitMap1
	APTR		ain_BitMap2
	UWORD		ain_Width
	UWORD		ain_Height
	UWORD		ain_Depth
	UWORD		ain_BMBytesPerRow
	UWORD		ain_BMByteWidth
	; Playing Data
	APTR		ain_OldIntVector
	APTR		ain_CopperList
	APTR		ain_CopperBitplanePointer
	APTR		ain_CopperColoursHi
	APTR		ain_CopperColoursLo
	UWORD		ain_Cycles
	; Interrupt Data
	UWORD		ain_MainLoopDone
	UWORD		ain_FrameRate
	UWORD		ain_FrameRateCount
	APTR		ain_NextEvent
	UWORD		ain_ScreenToggle
	UWORD		ain_Frame
	LABEL		ain_SizeOf

	STRUCTURE CopperWaitNode,0
	STRUCT		cwn_Node,MLN_SIZE
	UWORD		cwn_Line
	UWORD		cwn_InstructionNum
	UWORD		cwn_Wait1
	UWORD		cwn_Wait1_2
	UWORD		cwn_Command1
	UWORD		cwn_Command1_2
	UWORD		cwn_Wait2
	UWORD		cwn_Wait2_2
	UWORD		cwn_Command2
	UWORD		cwn_Command2_2
	LABEL		cwn_SizeOf
