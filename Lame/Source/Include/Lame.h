#define LACE_KEY			0x00000004
#define NullName(a)		((*(a) & 0xFC) == 0x80)
/* #define STOP_PAUL 1 */

/* Kill These */
#define GFXF_AA_ALICE	4
#define GFXF_AA_LISA	8

#define LAME_FAILAT													20
#define LAMERR_ALLOK												0
#define LAMERR_ENDOFPROG										1
#define LAMERR_NAME_NOT_UNIQUE							2
#define LAMERR_EMPTY_NAME										3
#define LAMERR_MULTIPLE_PLYR_CHUNKS					4
#define LAMERR_EMPTY												5
#define LAMERR_MULTIPLE_DISP_CHUNKS					6
#define LAMERR_RECURSIVE_ACTIONLIST					7
#define LAMERR_FILE_NOT_FOUND								LAME_FAILAT
#define LAMERR_READ													(LAME_FAILAT+1)
#define LAMERR_MEM													(LAME_FAILAT+2)
#define LAMERR_CHIPMEM											(LAME_FAILAT+3)
#define LAMERR_FUCKUP												(LAME_FAILAT+4)
#define LAMERR_NODISPLAY										(LAME_FAILAT+5)
#define LAMERR_NOBOB												(LAME_FAILAT+6)
#define LAMERR_ANIMNOTFOUND									(LAME_FAILAT+7)
#define LAMERR_IMAGENOTFOUND								(LAME_FAILAT+8)
#define LAMERR_PROGNOTFOUND									(LAME_FAILAT+9)
#define LAMERR_BADTEMPLATE									(LAME_FAILAT+10)
#define LAMERR_MAPNOTFOUND									(LAME_FAILAT+11)
#define LAMERR_BLOCKSNOTFOUND								(LAME_FAILAT+12)
#define LAMERR_UNDEFINEDBLOCKS							(LAME_FAILAT+13)
#define LAMERR_BUGGER_OFF_BACK_TO_WORKBENCH	(LAME_FAILAT+14)
#define LAMERR_REQUIRES_AGA									(LAME_FAILAT+15)
#define LAMERR_TOO_MANY_PLAYERS							(LAME_FAILAT+16)
#define LAMERR_PLYR_NUMBER_TOO_HIGH					(LAME_FAILAT+17)
#define LAMERR_FORMATION_NOT_FOUND					(LAME_FAILAT+18)
#define LAMERR_WEAPON_NOT_FOUND							(LAME_FAILAT+19)
#define LAMERR_NO_WEAPONS										(LAME_FAILAT+20)
#define LAMERR_SPRITE_BURST_WRONG						(LAME_FAILAT+21)
#define LAMERR_SPRITE_DEPTH_WRONG						(LAME_FAILAT+22)
#define LAMERR_ANIM_IMAGE_CHUNK_NOT_FOUND		(LAME_FAILAT+23)
#define LAMERR_ANIM_IMAGE_NOT_FOUND					(LAME_FAILAT+23)
#define LAMERR_PATH_NOT_FOUND								(LAME_FAILAT+24)
#define LAMERR_ILBM_MISSING_CHUNK						(LAME_FAILAT+25)
#define LAMERR_BITMAP_BAD_SIZE							(LAME_FAILAT+26)
#define LAMERR_MALFORMED_ANIMATION_FILE			(LAME_FAILAT+27)
#define LAMERR_TRUNCATED_CONFIG_FILE				(LAME_FAILAT+28)
#define LAMERR_LEVEL_NOT_FOUND							(LAME_FAILAT+29)
#define LAMERR_NO_LEVELS_LOADED							(LAME_FAILAT+30)
#define LAMERR_ANIMATION_NOT_FOUND					(LAME_FAILAT+31)
#define LAMERR_NOT_GAMECONFIG_FILE					(LAME_FAILAT+32)
#define LAMERR_SFX_NOT_FOUND								(LAME_FAILAT+33)
#define LAMERR_CANT_ALLOC_CIA_INTERRUPT			(LAME_FAILAT+34)
#define LAMERR_DOS_VOLUME_NOT_FOUND					(LAME_FAILAT+35)
#define LAMERR_SHIELD_BAR_WRONG_DEPTH				(LAME_FAILAT+36)
#define LAMERR_SHIELD_IMAGE_TOO_SHORT				(LAME_FAILAT+37)
#define LAMERR_DISPLAY_PALETTE_TOO_SMALL		(LAME_FAILAT+38)
#define LAMERR_EOTRNOTFOUND									(LAME_FAILAT+39)

#define ID_FORM 0x464F524D
#define ID_CONK	0x434F4E4B
#define ID_CMAP 0x434D4150
#define ID_BLKS 0x424C4B53
#define ID_BOBS 0x424F4253
#define ID_MAP  0x4D415020
#define ID_DEMO	0x44454D4F
#define ID_ANIM	0x414E494D
#define	ID_PROG	0x50524F47
#define ID_DFRM 0x4446524D
#define ID_THAP 0x54484150
#define ID_DISP 0x44495350
#define ID_WEAP 0x57454150
#define ID_TWAV 0x54574156
#define ID_SWAV 0x53574156
#define ID_SFX  0x53465820
#define ID_SPRT 0x53505254
#define ID_GCFG 0x47434647
#define ID_GAME 0x47414D45
#define ID_EVNT 0x45564E54
#define ID_EOTR	0x454F5452

#define ID_ILBM 0x494C424D
#define ID_BMHD 0x424D4844
#define ID_BODY 0x424F4459
#define ID_ANHD 0x414E4844
#define ID_DLTA 0x444C5441

/* Unused IDs
#define ID_PLYR 0x504C5952
#define ID_WAVE 0x57415645
#define	ID_SCRN	0x5343524E
#define	ID_MODE	0x4D4F4445
#define	ID_BANM	0x42414E4D
#define	ID_ATTK	0x4154544B	
#define	ID_ATCK	0x4154434B
#define	ID_PLEB	0x504C4542
#define	ID_PINK	0x50494E4B
#define	ID_FLYD	0x464C5944
#define ID_STAT 0x53544154
 */

#define TEMARG_END						0
#define TEMARG_BYTE						1
#define TEMARG_WORD						2
#define TEMARG_LONG						3
#define TEMARG_STRING					4
#define TEMARG_IMAGEREF				5
#define TEMARG_ANIMREF				6
#define TEMARG_DUDEREF				7
#define TEMARG_PROGREF				8
#define TEMARG_SFXREF					9
#define TEMARG_ARRAY					10
#define TEMARG_OPERATOR				11
#define TEMARG_PLYRREF				12
#define TEMARG_WEAPREF				13
#define TEMARG_PATHREF				14
#define TEMARG_RAWFILEREF			15
#define TEMARG_LEVELREF				16
#define TEMARG_EOTRREF				17

#define ALOP_END						0
#define ALOP_SETCHANNEL			1
#define ALOP_SETIMAGE				2
#define ALOP_SETANIM				3
#define ALOP_SETDUDE				4
#define ALOP_GIVEBONUS			5
#define ALOP_GIVEWEAPON			6
#define ALOP_DIE						7

#define SCOP_CYCLICANIM			8


#define GENERICFILENAMESIZE				10

#define BOBFILENAMESIZE				10
#define ANIMFILENAMESIZE			10
#define PROGFILENAMESIZE			10
#define FORMATIONFILENAMESIZE	10
#define SFXFILENAMESIZE				10
#define PATHFILENAMESIZE			10
#define WEAPFILENAMESIZE			10
#define LEVELFILENAMESIZE			10

#define GENERICFULLNAMESIZE				14

#define BOBFULLNAMESIZE				14
#define ANIMFULLNAMESIZE			14
#define PROGFULLNAMESIZE			14
#define FORMATIONFULLNAMESIZE	14
#define SFXFULLNAMESIZE				14
#define PATHFULLNAMESIZE			14
#define WEAPFULLNAMESIZE			14
#define LEVELFULLNAMESIZE			14

/************************************************************************/
// LAME STRUCTURES
/************************************************************************/

struct MemNode
{
	struct MinNode	 mn_Node;
	char						*mn_Name;
	ULONG						 mn_Type;
	ULONG						 mn_MemSize;		/* Keep this the last thing in this structure, */
																	/* so that run-time stuff can work of the size */
																	/* of its own data area easily. (PlayModule for example) */
};


/******************   FileStuff   ********************/
struct FileNode
{
	struct Node			 fn_Node;
	struct List			 fn_ChunkList;
	UWORD						 fn_FileType;			/* CONK, ILBM, ANIM, MODULE */
	ULONG						 fn_Form;					/* ID_CONK, ID_ILBM, ID_ANIM, 0=RawFile */
	BOOL						 fn_UsedInLevel;
	BOOL						 fn_Complete;
	void						*fn_Data;				/* Ptr to Form Specific data storage, used for modules/animations */
	char						 fn_CompressedName[BOBFILENAMESIZE];
};
/*
	fn_UsedInLevel:	If True the file is in use for this level.
									If False the file is not in use so it can be dealloc'd if we get strapped for mem.
	fn_Complete:		If True the file contains all the chunks that it should have.
									If False the file is missing some chunks. This happens when we dealloc a 
									chunk from a file, but the file still contains other data.
									Look at the 'FlushFiles()' routine.
 */

struct ChunkNode
{
	struct Node			cn_Node;
	ULONG						cn_TypeID;
	APTR						cn_Data;
	APTR						cn_FirstItem;
	BOOL						cn_Cooked;
	BOOL						cn_Freeable;
};

/*
	cn_Cooked:		This Chunk has been processed.
	cn_Freeable:  If False then the chunk is in use at run time (ie, bob images)
							 If True it can be freed. Note that cn_Cooked must be true for this to
							 to be valid.
	NOTE: That if fn_UsedInLevel == FALSE, then all the corresponding chunks can be dealloc'd
				without concern.
 */

struct FileNameStruct
{
	UWORD  fns_FileType;
	char	*fns_Name;
};

struct LevelNode
{
	struct Node							 ln_Node;
	struct LevelFileHeader	*ln_LevelFileHeader;
	UWORD										 ln_NumOfFiles;
	struct FileNameStruct		*ln_FileNames;		/* Ptr to array of FileNameStruct */
};

/******************   Anim File Stuff   ********************/

/* Don't Forget To Change The 'lame.i' equivalent */
struct AnimInfoNode
{
	struct MinList			 ain_FrameList;
	struct BitMapHeader	*ain_BMHD;
	UBYTE								*ain_BODY;
	UBYTE								*ain_CMAP;
	/* Rest of this data to be filled out when cooking */
	UBYTE								*ain_RealBitMap1;
	UBYTE								*ain_RealBitMap2;
	UBYTE								*ain_BitMap1;
	UBYTE								*ain_BitMap2;
	UWORD								 ain_Width;
	UWORD								 ain_Height;
	UWORD								 ain_Depth;
	UWORD								 ain_BMBytesPerRow;
	UWORD								 ain_BMByteWidth;
	/* Playing Data */
	void								*ain_OldIntVector;
	UWORD								*ain_CopperList;
	struct CopperPointer *ain_CopperBitplanePointer;
	UWORD								*ain_CopperColoursHi;
	UWORD								*ain_CopperColoursLo;
	UWORD								 ain_Cycles;
	/* Interrupt Data */
	UWORD								 ain_MainLoopDone;
	UWORD								 ain_FrameRate;		/* 0 & 1 = Fastest */
	UWORD								 ain_FrameRateCount;
	struct EventNode		*ain_NextEvent;
	UWORD								 ain_ScreenToggle;
	UWORD								 ain_Frame;
};

struct AnimHeader
{
	UBYTE ah_Operation;
	UBYTE ah_Mask;
	UWORD ah_Width;
	UWORD ah_Height;
	WORD  ah_X;
	WORD	ah_Y;
	ULONG ah_AbsTime;
	ULONG ah_RelTime;
	UBYTE ah_Interleave;
	UBYTE ah_Pad0;
	ULONG ah_Bits;
	UBYTE ah_Pad[16];
};

struct FrameNode
{
	struct MinNode		 frn_Node;
	struct AnimHeader *frn_ANHD;
	UBYTE							*frn_DLTA;
};

/* XYZZY - complete crap? */

/******************   Event Stuff  *****************
 Events used for timing of special effect stuff.
 File Format:
  UWORD NumOfEvents;
		UWORD frame, event, data[...]
*/


struct EventNode
{
	struct Node		 ev_Node;
	UWORD					 ev_Frame;
	UWORD					 ev_Type;			/* See Defines Below */
	UWORD				 (*ev_InitRoutine)(struct AnimInfoNode *,struct EventNode *);
	void					*ev_EventData;
};

#define EVT_SFX	0
 /* voice, sample-name */
#define EVT_COLOUR	1
 /* UBYTE colour, UBYTE red, UBYTE green, UBYTE blue */
#define EVT_CYCLES	2
 /* UWORD cycles, ~0=endless */
#define EVT_FRAMERATE	3
 /* UWORD framerate */

struct SFXEvent
{
	UWORD							 se_Voice;
	struct SpamParam	*se_SpamParam;
};

struct ColourEvent
{
	UBYTE	ce_ColourReg;
	UBYTE	ce_Red;
	UBYTE	ce_Green;
	UBYTE	ce_Blue;
};
/******************   BobStuff   ********************/
// BobNode - used for storing processed bob images and for linking them
// into the bob list.

/* Paul's Bob Image Structure */
struct PaulsBobImage
{
	UWORD		im_Type;					/* See defines below */
	ULONG		im_Mask;
	ULONG		im_Image;					/* buf = 48, 88 */
	UWORD		im_DMod;					/* bufw - (bobwidth + 2) */
	UWORD		im_Size;					/* bltsize */
	UWORD		im_Height;
	UWORD		im_Width;
	LONG		im_ColLeft;				/* 5 bitshifts */
	LONG		im_ColRight;
	LONG		im_ColTop;
	LONG		im_ColBottom;
	ULONG		im_ColWidth;
	ULONG		im_ColHeight;
	UWORD		im_Kludge;			/* This is here so that the Number field is at the same place */
													/* with the sprite, this must be so for the Anim routines */
	UWORD		im_Number;
};
#define IMTY_BOB 0
#define IMTY_SPRITE 1
#define IMTY_PICTURE 2

struct BobNode
{
	struct Node						bn_Node;
	struct PaulsBobImage	bn_ImageHeader;
};


struct BobBankFileHeader
{
	UBYTE	bf_BankName[16];
	UWORD	bf_NumberOfBobs;
	UBYTE bf_Depth;
	UBYTE bf_Pad;
	UWORD bf_Flags;
	UWORD	bf_Reserved[4];
	/* Array of BobHeader */
};

struct BobHeader
{
	UWORD bf_X;
	UWORD bf_Y;
	UWORD bf_Width;
	UWORD bf_Height;
	UWORD bf_Flags;
	WORD bf_ColLeft;
	WORD bf_ColTop;
	WORD bf_ColWidth;
	WORD bf_ColHeight;
	UBYTE	bf_Name[10];
	UWORD bf_Reserved[1];
	/* Image Data (((bf_Width+15)/8) & 0xFFFE)*bf_Height*bf_Depth */
	/* Mask Data same size as image data */
};

/******************   BobStuff   ********************/
// BobNode - used for storing processed bob images and for linking them
// into the bob list.

/* Paul's Sprite Image Structure */
struct PaulsSpriteImage
{
	UWORD				sp_Type;				/* See IMTY_* defines above */
	ULONG				sp_Size;				/* Size of each image */
	APTR				sp_Image;				/* pointer to image(s) */
	UWORD				sp_NumberUsed;	/* Number of sprites used */
	UWORD				sp_Flags;				/* See Defines Below */
	UWORD				sp_Height;			/* Height of Image (unshifted pixels) */
	UWORD				sp_Width;				/* Unshifted pixel Width of one sprite (16,32, or 64) */
	LONG				sp_ColLeft;
	LONG				sp_ColRight;
	LONG				sp_ColTop;
	LONG				sp_ColBottom;
	ULONG				sp_ColWidth;
	ULONG				sp_ColHeight;
	UWORD				sp_StartSprite;
	UWORD				sp_Number;			/* Make sure this is same offset as im_Number (Bobs) */
};
#define SPB_ATTACHED 0
#define SPF_ATTACHED 1<<SPB_ATTACHED

struct SpriteNode
{
	struct Node							sn_Node;
	struct PaulsSpriteImage	sn_ImageHeader;
};

struct SpriteBankFileHeader
{
	UBYTE	sbfh_BankName[16];
	UWORD	sbfh_NumberOfSprites;
	UBYTE sbfh_Depth;
	UBYTE sbfh_Pad;
	UWORD sbfh_Flags;
	UWORD	sbfh_Reserved[4];
};

struct SpriteFileHeader
{
	UWORD sfh_X;
	UWORD sfh_Y;
	UWORD sfh_Width;
	UWORD sfh_Height;
	UWORD sfh_Flags;
	UWORD	sfh_SpriteNumber;			/* 0-7 */
	WORD sfh_ColLeft;
	WORD sfh_ColTop;
	WORD sfh_ColWidth;
	WORD sfh_ColHeight;
	UBYTE	sfh_Name[BOBFILENAMESIZE];
	UWORD sfh_Reserved[1];
	/* Image Data (((sf_Width+15)/8) & 0xFFFE)*sf_Height*sf_Depth */
};


/******************   AnimStuff   ********************/
// BobImage AnimSequence storage and linking
struct AnimNode
{
	struct Node						 an_Node;
	struct AnimFileHeader	*an_AnimHeader;
};

struct FrameFileHeader
{
	ULONG	ffh_BobNumber;
	LONG	ffh_XOffset;
	LONG	ffh_YOffset;
};

struct AnimFileHeader
{
	UBYTE afh_AnimName[10];
	UWORD	afh_NumOfFrames;
	UWORD	afh_AnimSpeed;
	/* Array of FrameFileHeader Structs */
};

struct SequenceFileHeader
{
	UBYTE	sfh_BobBankName[16];
	UWORD	sfh_NumOfAnims;
	/* array of AnimFileHeader Structs */
};

/******************   ProgStuff   ********************/
struct ProgNode
{
	struct Node						 pn_Node;
	UBYTE									*pn_SrcData;
	UBYTE									*pn_Prog;
	ULONG									 pn_ProgSize;
};

struct ProgFileHeader
{
	UBYTE		pfh_Name[PROGFILENAMESIZE];
	ULONG		pfh_Memory;
	ULONG		pfh_ProgFileSize;
};

/******************   WaveStuff   ********************/
struct PaulsFormationParam
{
	UWORD												 pp_Delay;
	LONG												 pp_XPos;							/* Shifted Pixels */
	LONG												 pp_YPos;
	ULONG												*pp_Prog;
};

struct PaulsFormation
{
	struct PaulsFormation				*pf_Next;
	struct PaulsFormation				*pf_Previous;
	UWORD												 pf_Dudes;
	UWORD												 pf_Flags;
	struct PaulsFormationParam	*pf_Params;
};

struct PaulsActiveWaveData
{
	struct PaulsActiveWave			*sw_Next;
	struct PaulsActiveWave			*sw_Previous;
	UWORD												 sw_Counter;
	UWORD												 sw_Dudes;
	UWORD												 sw_CurrentDudes;
	struct PaulsFormationParams	*sw_NextDudeParams;
	LONG												 sw_XOrigin;
	LONG												 sw_YOrigin;
};

struct FormationNode
{
	struct Node									 fn_Node;
	struct PaulsFormation				 fn_PaulsFormation;
	struct PaulsFormationParam	*fn_Params;
};

struct DiskFormationDude
{
	UBYTE dfd_Program[ PROGFILENAMESIZE ];
	LONG	dfd_XPos;				/* Unshifted Pixels */
	LONG	dfd_YPos;
	UWORD	dfd_Delay;
};

/*					---   Formation Disk Format   ----
 *
 *	ULONG		Number Of Formations;
 *		UBYTE		FormationName[FORMATIONFILENAMESIZE];		\
 *		ULONG		Number Of Dudes in Formation ;					|---> * # Formations;
 *		 struct	DiskFormationDude (* # Dudes);					/
 *
 */


/* TWAV chunk structure: */
/* First longword is number of attackwaves in chunk, */
/* then comes array of DiskTimedWave structs. */

struct DiskTimedWave
{
	UBYTE				dtw_PackedEvent[ GENERICFILENAMESIZE ];
	UWORD				dtw_Time;
	LONG				dtw_X;
	LONG				dtw_Y;
	UBYTE				dtw_Type;
	UBYTE				dtw_Flags;
};

#define TWTYPE_ACTIONLIST	0
#define TWTYPE_DUDEINIT		1
#define TWTYPE_FORMATION	2

#define TWFLAG_VIEWRELATIVE 0x01


struct PaulsTimedWave
{
	UWORD		ptw_Time;
	UBYTE		ptw_Type;
	UBYTE		ptw_Flags;
	APTR		ptw_Event;
	LONG		ptw_XPos;
	LONG		ptw_YPos;
};


/* obsolete, I think */
#define PTWB_TIMED		0
#define PTWF_TIMED		1<<PTWB_TIMED
#define PTWB_RANDOM		1
#define PTWF_RANDOM		1<<PTWB_RANDOM
#define PTWB_SCROLL		2
#define PTWF_SCROLL		1<<PTWB_SCROLL

#define PTW_RELATIVE_MAP					 0
#define PTW_RELATIVE_VIEWLEFT			 1
#define PTW_RELATIVE_VIEWRIGHT		 2

struct TimedWaveNode
{
	struct	Node 						 twn_Node;
	UWORD										 twn_NumberOfTimedWaves;
	struct	PaulsTimedWave	*twn_PaulsTimedWave;
};


/* Header for SWAV chunk on disk */
struct DFSWAVHeader
{
	UWORD	dswh_NumOfWaves;
	UWORD dswh_LfHead;			/* ~0 = No Head */
	UWORD dswh_RtHead;
	ULONG	dswh_Reserved;
};

/* Disk Format Scroll Triggered AttackWave Structure */
struct DFScrollWave
{
	/* struct linked into both left-sorted list and right-sorted list.						*/
	/* Left list is used when scrolling left. Same for right. Just the other			*/
	/* way round. Got it?																													*/
	UWORD	dsw_LfSucc;		/* index num of next DFScrollWave, (0xFFFF = end of list) */
	UWORD	dsw_RtSucc;		/* links for rightlist */

	/* dsf_Name is a Formation or an Actionlist depending on dsg_Flag values */
	UBYTE	dsw_Name[ PROGFILENAMESIZE ];
	UBYTE	dsw_Flags;			/* see values below */
	UBYTE	dsw_AppearCount;		/* Number of times attackwave appears. 0=always. 			*/
	UWORD	dsw_LfTrigger;	/* triggerposition when scrolling left (unshifted)... */
	UWORD	dsw_RtTrigger;	/* ...and for scrolling right. 0xFFFF = no trigger		*/
	/* The position at which wave is to appear (unshifted). */
	WORD	dsw_XPos;
	WORD	dsw_YPos;
};

/* dsw_Flag values:																									*/
/* bits 0-2:	Wave type. 0=NewDude, 1=Formation, 2=ActionList				*/
/* bits 3-7:	Undefined - leave zero. Maybe extend to more types in */
/*						here later.	*/

struct ScrollWaveNode
{
	struct	Node 						 swn_Node;
	UWORD										 swn_NumberOfScrollWaves;
	struct	PaulsScrollWave	*swn_PaulsScrollWave;		/* actually ptr to an array of PaulsScrollWave */
};

struct PaulsScrollWave
{
	UWORD											 psw_Type;
	void											*psw_ActionFormation;	/* Ptr to either an actionlist or a formation depending upon psw_Flags */
	LONG											 psw_XPos;						/* wave x origin (shifted pixels) */
	LONG											 psw_YPos;						/* wave y origin (shifted pixels) */
	UWORD											 psw_Flags;						/* See dsw_Flag above */

	struct PaulsScrollWave		*psw_NextLeft;
	struct PaulsScrollWave		*psw_PrevLeft;
	struct PaulsScrollWave		*psw_NextRight;
	struct PaulsScrollWave		*psw_PrevRight;
	UWORD											 psw_AppearCount;			/* 0 = always */
	ULONG											 psw_LeftTrigger;			/* ~0 = no trigger (shifted pixels) */
	ULONG											 psw_RightTrigger;		/* ~0 = no trigger (shifted pixels) */
};

/******************   PlayerStuff  ********************/

struct PlayerFile
{
	UWORD pf_NumOfPlayers;
	UWORD	pf_NumOfStructures;
	/* InitActionList[PROGFILENAMESIZE] * NumOfPlayers */
};

struct PaulsChannel
{
	UBYTE										*cl_UpdateCode;
	UBYTE										*cl_Parameters;
	ULONG										 cl_DoneChannel;
	UWORD										 cl_Timer;
	ULONG										 cl_Data[5];
	UBYTE										*cl_SetUpProgram;
};

struct PaulsObject
{
	struct PaulsObject			*ob_Next;
	struct PaulsObject			*ob_Previous;
	UWORD										 ob_State;
	UWORD										 ob_Type;
	UWORD										 ob_IMType;
	LONG										 ob_XPos;
	LONG										 ob_YPos;
	LONG										 ob_AnimXOffset;
	LONG										 ob_AnimYOffset;
	struct PaulsBobImage		*ob_Image;
	LONG										 ob_OldAnimXOffset;
	LONG										 ob_OldAnimYOffset;
	struct PaulsBobImage		*ob_OldImage;
	struct PaulsBobImage		*ob_PrevImage;
	ULONG										 ob_ScreenOffset1;
	ULONG										 ob_ScreenOffset2;
	UWORD										 ob_WipeSize[2];
	UWORD										 ob_ModSize[2];
	UWORD										 ob_Highlight;
	ULONG										 ob_InputChannel;
	struct PaulsObject			*ob_Parent;
	struct PaulsChannel			 ob_Channel1;
	struct PaulsChannel			 ob_Channel2;
	struct PaulsChannel			 ob_Channel3;
	struct PaulsChannel			 ob_Channel4;
	struct PaulsChannel			 ob_Channel5;
	ULONG										 ob_AttachSprite;
	ULONG										 ob_AttachSpriteX;
	ULONG										 ob_AttachSpriteY;
	UWORD										 ob_ParentType;
	UWORD										ob_Pad;
	ULONG										 ob_Score[2];
};

struct PaulsBob
{
	struct PaulsObject			 bb_PaulsObject;
	ULONG										 bb_Type;
	ULONG										 bb_XMaxSpeed;
	ULONG										 bb_YMaxSpeed;
	LONG										 bb_XSpeed;
	LONG										 bb_YSpeed;
	LONG										 bb_XAccel;
	LONG										 bb_YAccel;
	LONG										 bb_XDeccel;
	LONG										 bb_YDeccel;
	UWORD										 bb_MaxEnergy;
	UWORD										 bb_Energy;
	UWORD										 bb_Fuel;
	UWORD										 bb_MaxBullets;
	UWORD										 bb_Bullets;
	UWORD										 bb_MaxBulletDelay;
	UWORD										 bb_BulletDelay;
	void										*bb_FireAction;
	void										*bb_WeaponBank;
	void										*bb_HitRoutine1;
	void										*bb_HitRoutine2;
	void										*bb_HitRoutine3;
	ULONG										 bb_DeathHu;
	UWORD										 bb_UpdatingWaves;

	ULONG										 bb_DefaultBorderTypes;
	LONG										 bb_DefaultBorderLeft;
	LONG										 bb_DefaultBorderRight;
	LONG										 bb_DefaultBorderTop;
	LONG										 bb_DefaultBorderBottom;

	UWORD										 bb_Shadow;
	ULONG										 bb_ShadowXOffset;
	ULONG										 bb_ShadowYOffset;

	UBYTE										 bb_Direction;
	UBYTE										 bb_Pad;
	ULONG										 bb_DirSpeed;
	ULONG										 bb_MaxDirSpeed;
	ULONG										 bb_Flags;
	ULONG										 bb_Data1;
	ULONG										 bb_Data2;
};

struct PlayerNode
{
	struct Node						 pn_Node;
	struct PaulsBob				 pn_Bob;
};

struct BulletNode
{
	struct Node						 bn_Node;
	struct PaulsBob				 bn_Bob;
};

struct BadDudeNode
{
	struct Node						 bdn_Node;
	struct PaulsBob				 bdn_Bob;
};

/*******************  Weapon Stuff   **********************/

/* First longword of WEAP chunk gives number of weapons saved. */
/* WeaponDiskDef structures follow. */

struct WeaponDiskDef
{
	UBYTE wdd_Name[ WEAPFILENAMESIZE ];
	UBYTE wdd_FireProgram[ PROGFILENAMESIZE ];
	UWORD wdd_FireDelay;
	UWORD wdd_MaxBullets;
	UBYTE wdd_PowerUp[ WEAPFILENAMESIZE ];
	UBYTE wdd_PowerDown[ WEAPFILENAMESIZE ];
	UWORD wdd_Flags;
	UWORD wdd_Reserved;
};

struct PaulsWeapon
{
	void									*wp_PowerUp;
	void									*wp_PowerDown;
	void									*wp_FireActionList;
	UWORD									 wp_Bullets;
	UWORD									 wp_MaxBullets;
	UWORD									 wp_BulletDelay;
	UWORD									 wp_MaxBulletDelay;
	UWORD									 wp_Flags;
	UWORD									 wp_Reserved;
};

struct WeaponNode
{
	struct Node						 wn_Node;
	struct PaulsWeapon		 wn_PaulsWeapon;
};

/******************   SfxStuff   **********************/

/* Longword at start to say how many sound effects there are */

struct SpamParam
{
	UBYTE	spam_PackedName[SFXFILENAMESIZE];
	BYTE	spam_Pri;
	UBYTE	spam_Volume;
	UWORD	spam_SampleRate;		/* in samples/sec */
	ULONG	spam_Length;				/* length of sampledata in bytes */
	LONG	spam_RepeatStart;		/* Start offset of the repeat data */
	ULONG	spam_RepeatLength;	/* Repeat Data Length ( 0 = No Repeat ) */
	UWORD	spam_Flags;					/* See Defines Below */
	WORD spam_Repeats;				/* How many times to play the repeatsection */
														/* -1=forever */
	UWORD	spam_Reserved[4];
};
#define SPAMB_ISPERIOD 0
#define SPAMF_ISPERIOD 1<<SPAMB_ISPERIOD

#define SPAMERR_ALLOK					0
#define SPAMERR_MEM						1
#define SPAMERR_AUDIODEVICE		2

struct SFXNode
{
	struct	Node						 sfx_Node;
	struct	SpamParam				*sfx_SpamParam;
};

/******************   PATH Stuff   ********************/
// Path Disk Format:
// LONG	NumOfPaths;
//	struct PathHeader Path1;
//	struct PathHeader Path2;
//	...
//	struct PathHeader Path(NumOfPaths);

struct PathHeader
{
	UBYTE		 ph_PackedName[10];
	LONG		 ph_XPos;
	LONG		 ph_YPos;
	UWORD		 ph_Data[0];
	/*
	ULONG	ph_SourceSize;			0 = No Source Path Data
	...
	ULONG	ph_CompiledSize;
	...
	*/
};

struct PathNode
{
	struct Node					 pn_Node;
	struct PathNode			*pn_NextPathNodeInChunk;
	struct PathHeader		*pn_PathHeader;
};

/******************   SliceStuff   ********************/

struct Display									/* Display Structure on disk */
{
	UWORD dp_Reserved;
	UWORD dp_Flags;								/* See defines below */
	UWORD	dp_FrameRate;
	UBYTE	dp_Map[16];
	UBYTE	dp_BlockSet[16];
	UBYTE dp_BackDrop[BOBFILENAMESIZE];
	UBYTE dp_InitProgram[PROGFILENAMESIZE];
	UWORD	dp_LevelWidth;
	UWORD dp_LevelHeight;
	UWORD dp_LevelDepth;
	ULONG	dp_DisplayID;
	UWORD	dp_NumOfPlayerStructs;	/* Num Of Player Structures to Allocate */
	UWORD	dp_NumOfBulletStructs;	/* Num Of Bullet Structures to Allocate */
	UWORD	dp_NumOfBDStructs;			/* Num Of Bad Dude Structures To Allocate */

		/* Sprite Stuff */
	UWORD	dp_SpriteColourBank;		/* 0=0, 1=16, 2=32, 3=48, ... */
	UWORD	dp_SpriteBurstMode;			/* 0 = No Burst(16), 1 = Half burst(32), 2 = Full Burst(64) */

	UWORD	dp_GameHeaderHeight;								/* Height of the blanked area above the main game */
	UWORD	dp_StatHeaderHeight;								/* Height of the blanked area above the statbox */
	UBYTE	dp_StatBackDropPicture[BOBFILENAMESIZE];
	ULONG	dp_StatDisplayID;
	UBYTE	dp_StatUpdateProg[PROGFILENAMESIZE];	/* Update code run every cycle */
	UBYTE	dp_StatShieldBarImage[BOBFILENAMESIZE];	/* Picture of all the shield bar images */

		/* Default Object Border Info */
	LONG	dp_DefaultBorderLeft;
	UWORD	dp_DefaultBorderLeftType;
	LONG	dp_DefaultBorderRight;
	UWORD	dp_DefaultBorderRightType;
	LONG	dp_DefaultBorderTop;
	UWORD	dp_DefaultBorderTopType;
	LONG	dp_DefaultBorderBottom;
	UWORD	dp_DefaultBorderBottomType;
	UWORD	dp_DefaultBorderFlags;

		/* Player Info */
	UWORD	dp_NumOfPlayers;
	UBYTE	dp_PlayerInitProgram1[PROGFILENAMESIZE];
	UBYTE	dp_PlayerInitProgram2[PROGFILENAMESIZE];
	UBYTE	dp_PlayerInitProgram3[PROGFILENAMESIZE];
	UBYTE	dp_PlayerInitProgram4[PROGFILENAMESIZE];

		/* Palette */
	UWORD	dp_NumOfColours;
	UBYTE	dp_Palette[0];
};

#define DPB_AGAONLY 0
#define DPF_AGAONLY 1<<DPB_AGAONLY
#define DPB_SMOOTHBACKGROUND		1
#define DPF_SMOOTHBACKGROUND		1<<DPB_SMOOTHBACKGROUND
#define DPB_STATBOXATTOP 2					/* Stat Box is at the top of the screen */
#define DPF_STATBOXATTOP 1<<DPB_STATBOXATTOP

struct DisplayNode
{
	UWORD										*dn_CopperList1;
	UWORD										*dn_CopperList2;
};

struct SliceHeader
{
	struct SliceHeader			*sh_Next;
	ULONG										 sh_DisplayID;
	UBYTE										*sh_LogBase;
	UBYTE										*sh_PhyBase;
	UBYTE										*sh_TriBase;

	UWORD										*sh_CopperList;
	UWORD										*sh_CopperHeader;
	UWORD										 sh_CopperHeaderSize;
	struct CopperWaitNode		*sh_CopperDMAOn;
	struct CopperWaitNode		*sh_CopperDMAOff;

	UWORD										*sh_CCStart;
	UWORD										*sh_CCHeader;
	UWORD										*sh_CCColours;
	UWORD										*sh_CCPlanes;
	UWORD										*sh_CCSprites;
	UWORD										*sh_CCFooter;
	UWORD										*sh_CCEnd;

	UWORD										 sh_Planes;

	UBYTE										*sh_ColourPointers;
	UWORD										 sh_Colours;
	UWORD										 sh_ColourOffset;

	UWORD										 sh_IntroWipe;
	UWORD										 sh_IntroWipe_Speed;
	APTR										 sh_IntroWipe_Data1;
	APTR										 sh_IntroWipe_Data2;
	UWORD										 sh_ExitWipe;
	UWORD										 sh_ExitWipe_Speed;
	APTR										 sh_ExitWipe_Data1;
	APTR										 sh_ExitWipe_Data2;
	UWORD										 sh_CurrentWipe;
	UWORD										 sh_CurrentWipe_Speed;
	APTR										 sh_CurrentWipe_Data1;
	APTR										 sh_CurrentWipe_Data2;

	ULONG										 sh_Flags;
	UWORD										 sh_ScreenToggle;

	ULONG										 sh_ScreenWidth;			/* Shifted Pixels */
	ULONG										 sh_BufferWidth;			/* Bytes */
	ULONG										 sh_BufferWidthx16xPlanes;
	UWORD										 sh_BufferWidthxPlanes;
	UWORD										 sh_BufferWidthxIPlanes;
	ULONG										 sh_BufferHeight;
};

struct GameSlice
{
	struct SliceHeader					 ss_SliceHeader;
	struct SliceHeader					*ss_InterlaceData;

	APTR											   ss_EnergyBar;
	APTR												 ss_Channel0;
	APTR												 ss_Channel1;
	APTR												 ss_Channel2;
	struct PaulsScroll					*ss_Scroll;
	struct PaulsObject					*ss_Players;
	struct PaulsObject					*ss_PlayersTail;
	UWORD												 ss_NumOfPlayers;
	struct PaulsObject					*ss_PlayerBullets;
	struct PaulsObject					*ss_PlayersBulletsTail;
	struct PaulsObject					*ss_BadDudes;
	struct PaulsObject					*ss_BadDudesTail;
	struct PaulsWave						*ss_Timer;								/* Timer Attack Waves */
	UWORD												 ss_CollisionTypes;

	struct PaulsScrollWave			*ss_NextRightScrollAttackWave;
	struct PaulsScrollWave			*ss_NextLeftScrollAttackWave;
	struct PaulsTimedWave				*ss_NextTimeAttackWave;
	UWORD												 ss_TimedAttackWavesToGo;
	struct PaulsWave						*ss_NextRandomAttackWave;
	UWORD												 ss_RandomAttackWavesToGo;
	struct PaulsActiveWaveData	*ss_ActiveAttackWaves;
	ULONG												 ss_ActiveAttackWaveHead;

	LONG												 ss_DefaultBorderLeft;
	UWORD												 ss_DefaultBorderLeftType;
	LONG												 ss_DefaultBorderRight;
	UWORD												 ss_DefaultBorderRightType;
	LONG												 ss_DefaultBorderTop;
	UWORD												 ss_DefaultBorderTopType;
	LONG												 ss_DefaultBorderBottom;
	UWORD												 ss_DefaultBorderBottomType;
	UWORD												 ss_DefaultBorderFlags;

	LONG												 ss_PushScrollLeft;
	LONG												 ss_PushScrollRight;
	LONG												 ss_PushScrollTop;
	LONG												 ss_PushScrollBottom;

	LONG												 ss_CenterLine;
	LONG												 ss_BottomScreenPos;

	ULONG												 ss_ClipLeft;										/* Unshifted Pixels */
	ULONG												 ss_ClipRight;
	ULONG												 ss_ClipTop;
	ULONG												 ss_ClipBottom;

	LONG												 ss_GlobalEvents;
	LONG												 ss_EventUpdateFlags;
	UWORD												 ss_Event;

	APTR												 ss_SpriteObjects[8];
};

struct PaulsScroll
{
	ULONG										 sc_Pos;
	ULONG										 sc_YPos;
	ULONG										 sc_Speed;
	ULONG										 sc_BaseSpeed;
	ULONG										 sc_OldSpeed;
	UWORD										 sc_CopperScrollANDBits;

	UWORD										*sc_MapBank;
	UWORD										 sc_MapBlocksHigh;
	UWORD										 sc_MapBlocksWide;
	UWORD										 sc_BufferBlocksHigh;
	UWORD										 sc_BufferBlocksWide;
	UWORD										*sc_BlockBank;
	UWORD										 sc_BlockSize;
	LONG										 sc_RightBorder;				/* Right of Map in Shifted pixels (should be signed)*/

	UBYTE									 **sc_LogPosToDraw;				/* Ptr To (BlocksHigh) Table of Buffer Offsets to draw */
	UWORD									 **sc_LogBlocksToDraw;		/* Ptr To (BlocksHigh) Table of Blocks To Draw (corrisponding to 'LogPosToDraw' */
	ULONG										 sc_LogColumnOffset;
	ULONG										 sc_MapColumnOffset;
	UWORD										 sc_BlocksDrawn;
	UWORD										 sc_PosBlocksDrawn;
	UWORD										 sc_NegBlocksDrawn;
	ULONG										 sc_PosColumnOffset;
	ULONG										 sc_NegColumnOffset;
	ULONG										 sc_PosBufferOffset;
	ULONG										 sc_NegBufferOffset;
	ULONG										 sc_CurrentMapOffset;
	ULONG										 sc_CurrentBufferOffset;
	ULONG										 sc_BlockSizexBufferWidth;

	UWORD										 sc_NeededDrawnTable[16];

	ULONG										 sc_XTrackRoutine;
	ULONG										 sc_YTrackRoutine;
	ULONG										 sc_XTrack;
	ULONG										 sc_YTrack;
	LONG										 sc_LeftTrackPos;
	LONG										 sc_RightTrackPos;
	LONG										 sc_TopTrackPos;
	LONG										 sc_BottomTrackPos;
	LONG										 sc_XTrackMaxSpeed;
	LONG										 sc_YTrackMaxSpeed;

	ULONG	sc_Data[8];
};

struct SliceNode
{
	struct	Node						 sn_Node;
	UWORD										 sn_Type;			/* See defines below */
	UBYTE										*sn_BitMap1;
	UBYTE										*sn_BitMap2;
	UBYTE										*sn_BitMap3;	/* For StatSlice this is used for flags: 1 = bitmap allocated */
	void										*sn_SliceData;
};
#define SNTYPE_GAMESLICE	1
#define SNTYPE_2NDPLAYFIELD	2
#define SNTYPE_STATBOX	3

struct CopperPointer
{
	UWORD	cp_HiRegister;
	UWORD	cp_HiPointer;
	UWORD	cp_LoRegister;
	UWORD	cp_LoPointer;
};

struct PaulsCopperHeader
{
	UWORD										 ch_Bpl1Mod[2];
	UWORD										 ch_Bpl2Mod[2];
	UWORD										 ch_BplCon0[2];
	UWORD										 ch_BplCon1[2];
	UWORD										 ch_BplCon2[2];
	UWORD										 ch_BplCon3[2];
	UWORD										 ch_BplCon4[2];
	UWORD										 ch_DdfStrt[2];
	UWORD										 ch_DdfStop[2];
	UWORD										 ch_DiwStrt[2];
	UWORD										 ch_DiwStop[2];
	UWORD										 ch_FMode[2];
};

struct CopperWaitNode
{
	struct MinNode					 cwn_Node;
	UWORD										 cwn_Line;						/* Hardware line to wait for (up to 0x01FF) */
	UWORD										 cwn_InstructionNum;	/* Probably either 2 or 4 */
	UWORD										 cwn_Wait1[2];				/* Wait Instruction (could be PAL wait) */
	UWORD										 cwn_Command1[2];			/* Either Command(intreq|dmacon) or noop */
	UWORD										 cwn_Wait2[2];				/* The real wait if there was a PAL wait */
	UWORD										 cwn_Command2[2];			/* Could Be IntReq or anything */	
};

/*
struct StatFile
{
	UBYTE	sf_BackDropPicture[BOBFILENAMESIZE];
	ULONG	sf_DisplayID;
	UWORD	sf_Flags;									/* See 'SB?_*' defines below									*/
	UBYTE	sf_UpdateProg[PROGFILENAMESIZE];	/* Update code run every cycle				*/

	UWORD	sf_StatBoxHeight;					/* \ 																					*/
	UWORD	sf_StatBoxDepth;					/*  \___Provide these if no Picture given.		*/
	UWORD sf_NumOfColours;					/*  /																					*/
	UBYTE	sf_Palette[0];						/* /																					*/
};
#define SBB_BOXATTOP 0					/* Stat Box is at the top of the screen */
#define SBF_BOXATTOP 1<<SBB_BOXATTOP
*/
struct StatSlice
{
	struct SliceHeader				 st_SliceHeader;
	struct SliceHeader				*st_InterlaceData;
	APTR											 st_EnergyBar;
	APTR											 st_Channel0;
};

struct GroovyCon
{
	APTR		gc_GfxMem;
	ULONG		gc_BytesPerPlane;
	ULONG		gc_BytesPerRow;
	UWORD		gc_Depth;
	UWORD		gc_ConWidth;			/* Bytes */
	UWORD		gc_ConHeight;			/* Characters */
	APTR		gc_TextFont;

	// These fields are private.
	UWORD		gc_CursorX;
	UWORD		gc_CursorY;
};

struct GroovyConsoleDevice
{
	APTR		cd_ConsoleStruct;		/* Ptr to the Console's Structure (ie, 'GroovyCon' Struct) */
	APTR		cd_PutChar;					/* a0.l = Console Struct, d0.b = ASCII char value */
	APTR		cd_Clear;						/* a0.l = Console Struct */
	APTR		cd_CursorUp;				/* a0.l = Console Struct */
	APTR		cd_CursorDown;			/* a0.l = Console Struct */
	APTR		cd_CursorLeft;			/* a0.l = Console Struct */
	APTR		cd_CursorRight;			/* a0.l = Console Struct */
	APTR		cd_CursorSetX;			/* a0.l = Console Struct, d0.w = New X Pos */
	APTR		cd_CursorSetY;			/* a0.l = Console Struct, d0.w = New Y Pos */
};

/******************   ILBM Stuff  ********************/
struct BitMapHeader
{
	UWORD	 bmh_Width;			/* Width in pixels, be careful of this being bigger than PageWidth */
	UWORD	 bmh_Height;		/* Height in pixels, be careful of this being bigger than PageHeight */
	WORD	 bmh_Left;			/* Left position */
	WORD	 bmh_Top;				/* Top position */
	UBYTE	 bmh_Depth;			/* Number of planes */
	UBYTE	 bmh_Masking;		/* Masking type */
	UBYTE	 bmh_Compression;	/* Compression type */
	UBYTE	 bmh_Flags;				/* See below for definitions */
	UWORD	 bmh_Transparent;	/* Transparent color */
	UBYTE	 bmh_XAspect;		/* Pixel aspect from the displayinfo->resoloution.x, Used */
	UBYTE	 bmh_YAspect;		/* for printing ILBM's correctly. */
	WORD	 bmh_PageWidth;
	WORD	 bmh_PageHeight;
};
/* Flags */
#define BMHDB_CMAPOK	7
#define BMHDF_CMAPOK	(1 << BMHDB_CMAPOK)
/*  Compression techniques  */
#define	cmpNone			0
#define	cmpByteRun1		1

struct PaulsPictureImage
{
	UWORD		 ppi_Type;
	UWORD		 ppi_Width;				/* True width in pixels, (not 8 byte alligned), use ((ppi->ppi_Width>>3)+7)&0xFFF0; to get aligned byte width */
	UWORD		 ppi_Height;
	UWORD		 ppi_Depth;
	ULONG		 ppi_DisplayID;
	UBYTE		*ppi_Palette;
	UBYTE		*ppi_Image;				/* 8 byte alligned */
};

struct ILBMNode
{
	struct Node								 ilbm_Node;
	UBYTE											 ilbm_NameBuffer[BOBFILENAMESIZE];
	UBYTE											*ilbm_Image;		/* TRUE Image start address, not 8 byte alligned */
	struct PaulsPictureImage	 ilbm_PaulsPictureImage;
};


/******************   MiscStuff   ********************/

struct BLKSHeader
{
	UBYTE	blks_Name[16];
	UWORD	blks_NumOfBlocks;
	UWORD blks_Depth;
	UWORD	blks_LayoutWidth;	// how many blocks across in the Blocks window
													// (20 for lores)
	/* future expansion stuff */
	UWORD	blks_Flags;				// set to 0!
	UWORD	blks_PixW;				// width in pixels (currently 16)
	UWORD	blks_PixH;				// height (currently 16)
	UWORD blks_BitMapHeight;// How high the bitmap is to be made.
	UWORD	blks_Reserved[3];	// set to 0!
	UWORD	blks_Data[0];
};

struct MAPHeader
{
	UBYTE	mh_Name[16];
	UWORD mh_BlocksWide;
	UWORD mh_BlocksHigh;
	UWORD mh_Data[0];
	/* Block Data in Horizontal form. ( (x0,y0),(x1,y0),(x2,y0) ), ( (x0,y1),(x1,y1),(x2,y1) ) */
};

struct LameReq
{
	//This field should be filled before calling LoadFiles()
	ULONG										*lr_Operators;				//List of template operator routine addresses
	UBYTE									 **lr_Templates;				//List of templates associated with each operator
	char									 **lr_OperatorNames;
	UWORD									 **lr_ShieldBarImagePtrs;	//Ptr to Array[17] of pointers to images

	//All other fields are filled out by LoadFiles()
	UWORD										 lr_Error;						//error code returned by LoadFiles()
	UWORD										 lr_FileNum;					//which file error occurred on

	//LoadFiles() puts ptrs to cooked data here
	APTR										 lr_FirstSlice;					//Ptr to First Slice
	UWORD										 lr_FrameRate;					//1 = Full
	UWORD										 lr_CopperLists;				//Num Of CopperLists (0=1, 1=2)
	UWORD						 				*lr_CopperList1;				//CopperList Memory
	UWORD										*lr_CopperList2;				//CopperList Memory
	struct CopperWaitNode		*lr_InterruptCopperWait;
	UWORD										 lr_TheFlags;						//Various Flags (see defines below)
	UWORD										 lr_ShieldBarHeight;		//Depth*Height
	APTR										 lr_LevelUpdateProg;

};

#define LRB_AGA 0
#define LRF_AGA 1<<LRB_AGA
#define LRB_AGAPALETTE 1
#define LRF_AGAPALETTE 1<<LRB_AGAPALETTE


struct VirtualJoystick		/* from structures.i */
{
	UBYTE		VJ_PrevState;		/* state of switches in prev frame */
	UBYTE		VJ_State;				/* current state of switches  */
	UWORD		VJ_Type;				/* type of device (see below) */
	UWORD		VJ_HWPort;			/* use depends on type */
	UBYTE		VJ_RawKeys[8];	/* keyboard equivalents */
};

#define VJTYPE_VIRTUAL			0	/* no actual joystick, check only keys */
#define VJTYPE_STDJOYSTICK	1	/* standard joystick in port 0 or 1 */
#define VJTYPE_CD32PAD			2	/* CD^32 joypad (not implemented yet) */

struct DiskFormatVJ
{
	UWORD Type;
	UWORD HWPort;
	UBYTE	RawKeys[8];
	UBYTE	Pad[4];
};
