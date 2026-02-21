		include "include:exec/types.i"
;Structures

;VirtualJoystick structure - may or may not represent an actual joystick
	rsreset
VJ_PrevState	rs.b	1	;state of switches in prev frame
VJ_State	rs.b	1	;current state of switches
VJ_Type		rs.w	1	;type of device (see below)
VJ_HWPort	rs.w	1	;use depends on type
VJ_RawKeys	rs.b	8	;keyboard equivalents
VJ_SIZEOF	rs.w	0

VJTYPE_VIRTUAL		equ	0	;no actual joystick, check only keys
VJTYPE_STDJOYSTICK	equ	1	;standard joystick in port 0 or 1
VJTYPE_CD32PAD		equ	2	;CD^32 joypad
VJTYPE_BUTTONSONLY	equ	3	;read the buttons only (for mice)
;VJTYPE_PARJOYSTICK	;parallelport joystick?
;VJTYPE_MOUSEEMU	;mouse emulation?


;VirtualMouse
		rsreset
VM_Reserved	rs.l	1
VM_XPos		rs.w	1
VM_YPos		rs.w	1
VM_DeltaX	rs.w	1
VM_DeltaY	rs.w	1
VM_PrevButtonState	rs.b	1
VM_ButtonState	rs.b	1
VM_SIZEOF	rs.w	0

;Level Structure
	rsreset
LL_Slice			rs.l	1
LL_Objects			rs.l	1
LL_AttackWaves			rs.l	1
LL_CopperMem			rs.l	1
LL_CopperSize			rs.l	1
LL_SizeOf			rs.b	0


	rsreset
SB_Energy		rs.w	1
SB_OldEnergy		rs.w	1
SB_MaxEnergy		rs.w	1
SB_ScaleValue		rs.l	1
SB_Images		rs.l	1
SB_Screen		rs.l	1
SB_ScreenBarHead	rs.l	1

;Updating Formation struct
		rsreset
UFM_Next		rs.l	1
UFM_Previous		rs.l	1
UFM_Counter		rs.w	1
UFM_Dudes		rs.w	1
UFM_CurrentDudes	rs.w	1
UFM_NextDudeParams	rs.l	1
UFM_XOrigin		rs.l	1
UFM_YOrigin		rs.l	1
UFM_SizeOf		rs.b	0

;Formation struct
		rsreset
FM_Next			rs.l	1
FM_Previous		rs.l	1
FM_Dudes		rs.w	1
FM_Flags		rs.w	1
FM_Params		rs.l	1
FM_SizeOf		rs.b	0

;Formation dude Params = Trigger.w,X.l,Y.l,ActionList.l
		rsreset
FMP_Trigger		rs.w	1
FMP_XPos		rs.l	1
FMP_YPos		rs.l	1
FMP_Action		rs.l	1
FMP_SizeOf		rs.b	0

;Slice Structure
	rsreset
SS_Next				rs.l	1	;General screen stuff
SS_DisplayID			rs.l	1
SS_LogBase			rs.l	1
SS_PhyBase			rs.l	1
SS_TriBase			rs.l	1

SS_CopperList			rs.l	1	;Copper data
SS_CopperHeader			rs.l	1
SS_CopperHeaderSize		rs.w	1
SS_CopperDMAOn			rs.l	1	;struct CopperWaitNode *
SS_CopperDMAOff			rs.l	1	;struct CopperWaitNode *

SS_CCStart			rs.l	1	;Copper addresses
SS_CCHeader			rs.l	1
SS_CCColours			rs.l	1
SS_CCPlanes			rs.l	1
SS_CCSprites			rs.l	1
SS_CCFooter			rs.l	1
SS_CCEnd			rs.l	1

SS_Planes			rs.w	1

SS_ColourPointers		rs.l	1	;Colour stuff
SS_Colours			rs.w	1
SS_ColourOffset			rs.w	1

SS_IntroWipe			rs.w	1	;Wipe stuff
SS_IntroWipe_Speed		rs.w	1
SS_IntroWipe_Data1		rs.l	1
SS_IntroWipe_Data2		rs.l	1
SS_ExitWipe			rs.w	1
SS_ExitWipe_Speed		rs.w	1
SS_ExitWipe_Data1		rs.l	1
SS_ExitWipe_Data2		rs.l	1
SS_CurrentWipe			rs.w	1
SS_CurrentWipe_Speed		rs.w	1
SS_CurrentWipe_Data1		rs.l	1
SS_CurrentWipe_Data2		rs.l	1

SS_Flags			rs.l	1
	;0= Collide with Explosions on/off
	;1= Slice Timer off
	;2= Add bobs to head/tail
SS_ScreenToggle			rs.w	1
						;Interlace stuff ends here
SS_ScreenWidth			rs.l	1
SS_BufferWidth			rs.l	1	;General buffer stuff
SS_BufferWidthx16xPlanes	rs.l	1
SS_BufferWidthxPlanes		rs.w	1
SS_BufferWidthxIPlanes		rs.w	1
SS_BufferHeight			rs.l	1

SS_Interlace			rs.l	1

SS_EnergyBar			rs.l	1
SS_Channel0			rs.l	1	;Update Stuff
SS_Channel1			rs.l	1
SS_Channel2			rs.l	1
SS_Scroll			rs.l	1
SS_Players			rs.l	1
SS_PlayersTail			rs.l	1
SS_NumOfPlayers			rs.w	1
SS_PlayerBullets		rs.l	1
SS_PlayerBulletsTail		rs.l	1
SS_BadDudes			rs.l	1
SS_BadDudesTail			rs.l	1
SS_Timer			rs.l	1
SS_CollisionTypes		rs.w	1
	;bit0 = baddude-player (on by default)
	;bit1 = baddude-playerbullet (on by default)
	;bit2 = baddude-baddude
	;bit3 = player-player
	;bit4 = player-playerbullet
	;bit5 = playerbullet-playerbullet

SS_NextRightScrolledAttackWave	rs.l	1
SS_NextLeftScrolledAttackWave	rs.l	1

SS_NextTimedAttackWave		rs.l	1
SS_TimedWavesToGo		rs.w	1

SS_NextRandomAttackWave		rs.l	1
SS_RandomWavesToGo		rs.w	1

SS_ActiveAttackWaves		rs.l	1
SS_ActiveAttackWaveHead		rs.l	1

SS_DefaultBorderLeft		rs.l	1
SS_DefaultBorderLeftType	rs.w	1
SS_DefaultBorderRight		rs.l	1
SS_DefaultBorderRightType	rs.w	1
SS_DefaultBorderTop		rs.l	1
SS_DefaultBorderTopType		rs.w	1
SS_DefaultBorderBottom		rs.l	1
SS_DefaultBorderBottomType	rs.w	1
SS_DefaultBorderFlags		rs.w	1

SS_PushScrollLeft		rs.l	1
SS_PushScrollRight		rs.l	1
SS_PushScrollTop		rs.l	1
SS_PushScrollBottom		rs.l	1

SS_CenterLine			rs.l	1
SS_BottomScreenPos		rs.l	1

SS_ClipLeft			rs.l	1
SS_ClipRight			rs.l	1
SS_ClipTop			rs.l	1
SS_ClipBottom			rs.l	1

SS_GlobalEvents			rs.l	1
SS_GEUpdateFlags		rs.l	1	;TIMER,SCROLLPOS
SS_Event			rs.w	1

SS_SpriteObjects		rs.l	8

SS_SizeOf			rs.b	0

;Slice Flags
;0 = Timer On/Off		1 = Collide with explosions
;2 = 


;Scroll Structure
	rsreset
SC_Pos				rs.l	1	;Scroll x pos (shifted)
SC_YPos				rs.l	1	;unshifted (!)
SC_Speed			rs.l	1
SC_BaseSpeed			rs.l	1
SC_OldSpeed			rs.l	1
SC_CopperScrollANDBits		rs.w	1

SC_MapBank			rs.l	1	;Scroll Data stuff
SC_MapBlocksHigh		rs.w	1
SC_MapBlocksWide		rs.w	1
SC_BufferBlocksHigh		rs.w	1
SC_BufferBlocksWide		rs.w	1
SC_BlockBank			rs.l	1
SC_BlockSize			rs.w	1
SC_RightBorder			rs.l	1

SC_LogPosToDraw			rs.l	1	;Draw info
SC_LogBlocksToDraw		rs.l	1
SC_LogColumnOffset		rs.l	1
SC_MapColumnOffset		rs.l	1
SC_BlocksDrawn			rs.w	1
SC_PosBlocksDrawn		rs.w	1
SC_NegBlocksDrawn		rs.w	1
SC_PosColumnOffset		rs.l	1
SC_NegColumnOffset		rs.l	1
SC_PosBufferOffset		rs.l	1
SC_NegBufferOffset		rs.l	1
SC_CurrentMapOffset		rs.l	1
SC_CurrentBufferOffset		rs.l	1
SC_BlockSizeXBufferWidth	rs.l	1

SC_NeededDrawnTable		rs.w	16

SC_XTrackRoutine		rs.l	1
SC_YTrackRoutine		rs.l	1
SC_XTrack			rs.l	1	;object to track
SC_YTrack			rs.l	1	;object to track
SC_LeftTrackPos			rs.l	1	;pushwindow dimensions
SC_RightTrackPos		rs.l	1
SC_TopTrackPos			rs.l	1
SC_BottomTrackPos		rs.l	1
SC_XTrackMaxSpeed		rs.l	1
SC_YTrackMaxSpeed		rs.l	1
SC_Data				rs.l	0
SC_XData			rs.l	4
SC_YData			rs.l	4
SC_SizeOf			rs.b	0

;Copper header structure
	rsreset
CH_Bpl1Mod			rs.w	2
CH_Bpl2Mod			rs.w	2
CH_BplCon0			rs.w	2
CH_BplCon1			rs.w	2
CH_BplCon2			rs.w	2
CH_BplCon3			rs.w	2
CH_BplCon4			rs.w	2
CH_DdfStrt			rs.w	2
CH_DdfStop			rs.w	2
CH_DiwStrt			rs.w	2
CH_DiwStop			rs.w	2
CH_FMode			rs.w	2
CH_SizeOf			rs.b	0

;Input channel structure
	rsreset
IC_Next				rs.l	1
IC_Routine			rs.l	1
IC_Data				rs.w	3
IC_JoyResult			rs.w	1
IC_MouseXResult			rs.w	1
IC_MouseYResult			rs.w	1
IC_Keys				rs.b	8
IC_Size				rs.b	0


;Image struct
		rsreset
IM_Type			rs.w	1
IM_Mask			rs.l	1	;pointer to mask
IM_Image		rs.l	1	;pointer to image
IM_DMod			rs.w	1	;c & d mod
IM_Size			rs.w	1	;bltsize
IM_Height		rs.w	1
IM_Width		rs.w	1
IM_ColLeft		rs.l	1
IM_ColRight		rs.l	1
IM_ColTop		rs.l	1
IM_ColBottom		rs.l	1
IM_ColWidth		rs.l	1
IM_ColHeight		rs.l	1

;Sprite Image struct
		rsreset
SP_Type			rs.w	1
SP_Size			rs.l	1	;Size of each image
SP_Image		rs.l	1	;pointer to image(s)
SP_Number		rs.w	1	;Number of sprites used
SP_Flags		rs.w	1	;1=Attached
SP_Height		rs.w	1
SP_Width		rs.w	1
SP_ColLeft		rs.l	1
SP_ColRight		rs.l	1
SP_ColTop		rs.l	1
SP_ColBottom		rs.l	1
SP_ColWidth		rs.l	1
SP_ColHeight		rs.l	1
SP_StartSprite		rs.w	1
 
;Channel Struct
		rsreset
CL_UpdateCode		rs.l	1
CL_Parameters		rs.l	1
CL_DoneChannel		rs.l	1
CL_Timer		rs.w	1
CL_Data			rs.l	0
CL_Data1		rs.l	1
CL_Data2		rs.l	1
CL_Data3		rs.l	1
CL_Data4		rs.l	1
CL_Flags		rs.l	1
CL_SetupProgram		rs.l	1
CL_SizeOf		rs.b	0

;Object Struct
		rsreset
OB_Next			rs.l	1	;Display information
OB_Previous		rs.l	1
OB_State		rs.w	1
OB_Type			rs.w	1
OB_UpdateType		rs.w	1
OB_XPos			rs.l	1
OB_YPos			rs.l	1
OB_AnimXOffset		rs.l	1
OB_AnimYOffset		rs.l	1
OB_Image		rs.l	1
OB_OldAnimXOffset	rs.l	1
OB_OldAnimYOffset	rs.l	1
OB_OldImage		rs.l	1
OB_PrevImage		rs.l	1
OB_ScreenOffset1	rs.l	1	;\
OB_ScreenOffset2	rs.l	1	; }see Sprite struct
OB_WipeSize		rs.w	2	;/
OB_ModSize		rs.w	2	;1 word in sprite struct
OB_Highlight		rs.w	1
OB_InputChannel		rs.l	1	;XYZZY obsolete?
OB_Parent		rs.l	1
OB_Channel1		rs.b	CL_SizeOf
OB_Channel2		rs.b	CL_SizeOf
OB_Channel3		rs.b	CL_SizeOf
OB_Channel4		rs.b	CL_SizeOf
OB_Channel5		rs.b	CL_SizeOf
OB_AttachSprite		rs.l	1
OB_AttachSpriteX	rs.l	1
OB_AttachSpriteY	rs.l	1
OB_ParentType		rs.w	1	;see below
OB_Pad			rs.w	1
OB_Score		rs.l	2
OB_SizeOf		rs.b	0

;SpriteObject
	rsset		OB_AttachSprite
SPR_SeudoImage		rs.l	1
SPR_SeudoX		rs.l	1
SPR_SeudoY		rs.l	1
SPR_SeudoAnimY		rs.l	1

;More object Struct
		rsset	OB_SizeOf
BB_Type			rs.l	1	;Update information
BB_XMaxSpeed		rs.l	1
BB_YMaxSpeed		rs.l	1
BB_XSpeed		rs.l	1
BB_YSpeed		rs.l	1
BB_XAccel		rs.l	1
BB_YAccel		rs.l	1
BB_XDeccel		rs.l	1
BB_YDeccel		rs.l	1
BB_MaxEnergy		rs.w	1	;really the Damage field
BB_Energy		rs.w	1	;Shields
BB_ID			rs.w	1	;Object ID
BB_MaxBullets		rs.w	1
BB_Bullets		rs.w	1
BB_MaxBulletDelay	rs.w	1
BB_BulletDelay		rs.w	1
BB_FireAction		rs.l	1	;obsolete?
BB_WeaponBank		rs.l	1
BB_HitRoutine1		rs.l	1
BB_HitRoutine2		rs.l	1
BB_HitRoutine3		rs.l	1
BB_DeathHu		rs.l	1

BB_UpdatingWaves	rs.w	1	;nowhere to be found

BB_BorderLeftType	rs.b	1
BB_BorderRightType	rs.b	1
BB_BorderTopType	rs.b	1
BB_BorderBottomType	rs.b	1
BB_BorderLeft		rs.l	1
BB_BorderRight		rs.l	1
BB_BorderTop		rs.l	1
BB_BorderBottom		rs.l	1

BB_Shadow		rs.w	1
BB_ShadowXOffset	rs.l	1
BB_ShadowYOffset	rs.l	1

BB_Direction		rs.b	1
BB_Pad			rs.b	1
BB_DirSpeed		rs.l	1
BB_MaxDirSpeed		rs.l	1
BB_Flags		rs.l	1	;unused,unused,OnScreen,ChildAttached
BB_Data1		rs.l	1
BB_Data2		rs.l	1

BB_SizeOf		rs.b	0


;OB_ParentType values:
PARENT_NONE		equ	0
PARENT_OBJECT		equ	1
PARENT_SCROLLWAVE	equ	2
PARENT_WEAPON		equ	3
PARENT_TIMEDWAVE	equ	4




;AttackWave params:	AttackWave.l,XOrigin.l,YOrigin.l,Counter.w,Dudes.w
		rsreset
AW_Type			rs.w	1
AW_Data			rs.l	1	;DudeAction,Formation,Action
AW_XOrg			rs.l	1
AW_YOrg			rs.l	1
AW_Flags		rs.w	1
AW_SizeOf		rs.b	0


;XYZZY - obsolete
;Timed wave extension
;		rsset	AW_SizeOf
;TW_Time			rs.w	1
;TW_SizeOf		rs.b	0

;Random wave header
		rsreset
RW_HType		rs.w	1
RW_HData		rs.l	1
RW_HAnd			rs.w	1
RW_HeadSizeOf		rs.b	0

;Random wave extension
		rsset	AW_SizeOf
RW_SizeOf		rs.b	0

;Scrolled wave extension
		rsset	AW_SizeOf
SW_NextLeft		rs.l	1
SW_PreviousLeft		rs.l	1
SW_NextRight		rs.l	1
SW_PreviousRight	rs.l	1
SW_AppearCount		rs.w	1	; 0 = Forever
SW_LeftTrigger		rs.l	1	;-1 = NoTrigger
SW_RightTrigger		rs.l	1
SW_SizeOf		rs.b	0

AW_Type_Timed		equ	0
AW_Type_Random		equ	1
AW_Type_Scroll		equ	2

;Weapon Struct

		rsreset
WP_PowerUp		rs.l	1
WP_PowerDown		rs.l	1
WP_FireActionList	rs.l	1
WP_Bullets		rs.w	1
WP_MaxBullets		rs.w	1
WP_BulletDelay		rs.w	1
WP_MaxBulletDelay	rs.w	1
WP_Flags		rs.w	1	;bit 0: 0=exec on player, 1=create bullet
WP_Reserved		rs.w	1
WP_SizeOf		rs.l	1

;EOTR Header - Corresponds with the DiskEOTRHeader in Game.h
			rsreset
EOTR_Noddy		rs.b	LN_SIZE
EOTR_Name		rs.b	10
EOTR_BlocksetName	rs.b	16
EOTR_NumOfBlocks	rs.w	1
EOTR_Reserved		rs.l	1
EOTR_Data		rs.w	0	;data starts here


;GlobalEvent Struct
	rsreset
GE_ActionList		rs.l	1
GE_Data			rs.l	1
GE_SizeOf		rs.b	0


PATHNAMEPACKSIZE	equ	10

	rsreset
PH_Name		rs.b	PATHNAMEPACKSIZE
PH_XPos		rs.l	1
PH_YPos		rs.l	1
PH_Data		rs.b	0

	STRUCTURE Blocks,0
	STRUCT	blks_Name,16
	UWORD	blks_NumOfBlocks
	UWORD	blks_Depth
	UWORD	blks_LayoutWidth
	UWORD	blks_Flags
	UWORD	blks_PixW
	UWORD	blks_PixH
	STRUCT	blks_Reserved,4*2
	LABEL	blks_SIZEOF


SFXNAMEPACKSIZE	equ	10

  STRUCTURE SpamParam,0
	STRUCT	spam_PackedName,SFXNAMEPACKSIZE
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



;TimedWave struct
	rsreset
TW_Time		rs.w	1
TW_Type		rs.b	1
TW_Flags	rs.b	1
TW_Event	rs.l	1	;ActionList or Formation (depends on Type)
TW_XPos		rs.l	1
TW_YPos		rs.l	1
TW_SIZEOF	rs.w	0

TWTYPE_ACTIONLIST	equ	0
TWTYPE_DUDEINIT		equ	1
TWTYPE_FORMATION	equ	2

TWFLAG_VIEWRELATIVE	equ	$01
