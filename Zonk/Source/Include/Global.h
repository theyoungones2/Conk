#include "protos.h"
#include "Spam.h"
#include "Game.h"

/******************************************************/
//
// GLOBAL DEFINITIONS
//
/******************************************************/

/* Disable assertions? */
//#define NDEBUG
#include <assert.h>

/* mydebug.h 1=on/0=off */
#define MYDEBUG 0
#include "mydebug.h"

#define PROGNAME "ZONK"

/* IFF types that may be in pictures */
#define	ID_ILBM		MAKE_ID('I','L','B','M')
#define	ID_BMHD		MAKE_ID('B','M','H','D')
#define	ID_BODY		MAKE_ID('B','O','D','Y')
#define	ID_CMAP		MAKE_ID('C','M','A','P')
#define	ID_CRNG		MAKE_ID('C','R','N','G')
#define	ID_GRAB		MAKE_ID('G','R','A','B')
#define	ID_SPRT		MAKE_ID('S','P','R','T')
#define	ID_DEST		MAKE_ID('D','E','S','T')
#define	ID_CAMG		MAKE_ID('C','A','M','G')

#define	ID_8SVX		MAKE_ID('8','S','V','X')
#define	ID_VHDR		MAKE_ID('V','H','D','R')

/* custom IFF chunknames */
#define	ID_CONK		MAKE_ID('C','O','N','K')	/* our FORM type */
#define	ID_MAP		MAKE_ID('M','A','P',' ')	/* backgroundmap */
#define	ID_BLKS		MAKE_ID('B','L','K','S')	/* blockset */
#define	ID_BOBS		MAKE_ID('B','O','B','S')	/* bob images */
#define	ID_ANIM		MAKE_ID('A','N','I','M')	/* bob/sprite anims */
#define	ID_PROG		MAKE_ID('P','R','O','G')	/* programs */
#define	ID_DISP		MAKE_ID('D','I','S','P')	/* display */
#define	ID_PLYR		MAKE_ID('P','L','Y','R')	/* player */
#define	ID_DFRM		MAKE_ID('D','F','R','M')	/* dude formations */
#define	ID_TWAV		MAKE_ID('T','W','A','V')	/* timed attackwaves */
#define	ID_SWAV		MAKE_ID('S','W','A','V')	/* scroll triggered attackwaves */
#define	ID_WEAP		MAKE_ID('W','E','A','P')	/* weapon definitions */
#define	ID_SFX		MAKE_ID('S','F','X',' ')	/* sound effects */
#define	ID_THAP		MAKE_ID('T','H','A','P')	/* named paths */
#define	ID_ZCFG		MAKE_ID('Z','C','F','G')	/* Zonk config */
#define	ID_SPRT		MAKE_ID('S','P','R','T')	/* sprite images */
#define	ID_EOTR		MAKE_ID('E','O','T','R')	/* edge of the road */
/* Game config chunk - can't put other chunks in same file. */
#define	ID_GAME		MAKE_ID('G','A','M','E')	/* Game config */

/* max name stringsizes (including null) */
#define GENERICNAMESIZE 14

#define PROGNAMESIZE GENERICNAMESIZE
#define BOBNAMESIZE GENERICNAMESIZE
#define ANIMNAMESIZE GENERICNAMESIZE
#define FORMATIONNAMESIZE GENERICNAMESIZE
#define SFXNAMESIZE	GENERICNAMESIZE
#define PATHNAMESIZE	GENERICNAMESIZE
#define WEAPONNAMESIZE GENERICNAMESIZE
#define LEVELNAMESIZE GENERICNAMESIZE
#define EOTRNAMESIZE GENERICNAMESIZE
#define MAPNAMESIZE 16				// These two are not packed - maybe change
#define BLOCKSETNAMESIZE 16		// sometime to be consistant...

/* bytes needed to pack names ( at 6 bits/char) */
/* null terminator may be dropped to squeeze in extra char - see 'Wibble.s' */
#define GENERICNAMEPACKSIZE 10
#define PROGNAMEPACKSIZE GENERICNAMEPACKSIZE
#define BOBNAMEPACKSIZE GENERICNAMEPACKSIZE
#define ANIMNAMEPACKSIZE GENERICNAMEPACKSIZE
#define FORMATIONNAMEPACKSIZE GENERICNAMEPACKSIZE
#ifndef SFXNAMEPACKSIZE
	#define SFXNAMEPACKSIZE	GENERICNAMEPACKSIZE
#endif
#define PATHNAMEPACKSIZE GENERICNAMEPACKSIZE
#define WEAPONNAMEPACKSIZE GENERICNAMEPACKSIZE
#define LEVELNAMEPACKSIZE GENERICNAMEPACKSIZE
#define EOTRNAMEPACKSIZE GENERICNAMEPACKSIZE

/* misc namesizes ( including null )*/
#define CHUNKINFOSTRINGSIZE 64
#define MYMAXFONTNAMESIZE 128
#define MAXFILENAMESIZE 256
#define MAXWINNAMESIZE 64

/* Block defines */
#define		BLKW	16						/* blockwidth in pixels */
#define		BLKBYTEW 2					/* blockwidth in bytes */
#define		BLKH	16						/* blockheight in pixels */
#define		MAXNUMOFBLOCKS 1024

/* Some RAWKEY defines... */
#define RAWKEY_UP			0x4C
#define RAWKEY_DOWN 	0x4D
#define RAWKEY_LEFT		0x4F
#define RAWKEY_RIGHT	0x4E
#define RAWKEY_SPACE	0x40

/* The this-chunk-has-been-modified character */
#define MODCHAR '+'

/******************************************************/
//
// GLOBAL VARIABLES
//
/******************************************************/

#ifndef ZONKMAIN_C

extern struct IntuitionBase		*IntuitionBase;
extern struct GfxBase					*GfxBase;
extern struct DosLibrary			*DOSBase;
extern struct Library					*GadToolsBase;
extern struct RxsLib					*RexxSysBase;
extern struct Library					*DiskfontBase;
extern struct Library					*AslBase;
extern struct Library					*IFFParseBase;
extern struct Library					*LayersBase;
extern struct ScreenModeRequester		*smreq;
extern struct FileRequester					*filereq;
extern struct FontRequester					*fontreq;
//extern struct ScreenConfig					globalscreenconfig;
extern struct ZonkConfig						zonkfig;

extern struct MsgPort					*mainmsgport;
extern struct List						superwindows;
extern UBYTE									*defname;
/* fallback font */
extern struct TextFont				*topaztextfont;

#endif	/* ZONKMAIN_C */

#ifndef SCREENS_C
extern struct TextFont *gfxscrfont;
extern struct TextFont *gfxwinfont;
extern struct TextFont *mainwinfont;
#endif	/* SCREENS_C */


#ifndef CHUNKSTUFF_C
extern struct List chunklist;
extern struct List filelist;
#endif	/* CHUNKSTUFF_C */

#ifndef OPERATORSTUFF_C
extern struct OperatorDef operators[];
extern UWORD numofoperators;
#endif	/* OPERATORSTUFF_C */

#ifndef MENUSTUFF_C
extern struct NewMenu projectmenu[];
extern struct NewMenu windowmenu[];
extern struct NewMenu settingsmenu[];
#endif	/* MENUSTUFF_C */

#ifndef VARSTUFF_C
extern struct Node varnodes[];
#endif	/* VARSTUFF_C */

/******************************************************/
//
// STRUCTURE DEFINITIONS
//
/******************************************************/

struct Chunk {
	struct Node					ch_Node;
	ULONG								ch_TypeID;				// eg 'BLKS', 'CMAP', 'BOBS' etc...

	BOOL								(*ch_Edit)(struct Chunk *);
	VOID								(*ch_Free)(struct Chunk *);
	BOOL								(*ch_MakeInfoString)( struct Chunk *, UBYTE * );
	BOOL								(*ch_Saver)(struct Chunk *, struct IFFHandle *);

	struct Filetracker	*ch_ParentFile;		// The file this chunk is from (or NULL)
	BOOL								ch_Modified;
	UWORD								ch_LockCount;			// How many things are using this chunk

	/* chunk-specific data is stored in ch_Data and/or ch_DataList */
	/* maybe add more general storage fields in here sometime if needed... */
	ULONG								ch_Size;					// any units
	APTR								ch_Data;
	struct List					ch_DataList;
};

/* generic chunk uses ch_Data to point to data, bytesize given in ch_Size */
/* PROG chunk uses ch_DataList for list of Programs */

struct Filetracker {
	struct Node	ft_Node;
	char				ft_Drawer[256];						// Path
	char				ft_File[256];							// Filename
	ULONG				ft_OriginalChunkCount;		// How many chunks when first loaded
	ULONG				ft_ChunkCount;						// How many chunks now
};


struct RGB8
{
	UBYTE	r,g,b;
};

struct Palette
{
	/* number of colours in this palette */
	UWORD								pl_Count;
	/* RGB data - 8 bits per component, in rgb ubyte triplets */
	/* ie same format as iff CMAP chunk */
	struct RGB8					pl_Colours[256];
};



struct ScreenConfig
{
	/* gfx screen type: */
	/* 0 = no gfx screen */
	/* 1 = create new pubscreen */
	/* 2 = use existing pubscreen */ 
	UWORD	scfg_GFXScrType;

	/* gfx screen parameters (only apply with type 1 above ) */
	UWORD	scfg_Width;
	UWORD	scfg_Height;
	UWORD	scfg_Depth;
	ULONG	scfg_DisplayID;
	UWORD	scfg_OverScanType;
	BOOL	scfg_AutoScroll;

	/* fonts for the gfx screen */
	UBYTE	scfg_GFXScrFontName[ MYMAXFONTNAMESIZE ];
	struct TextAttr	scfg_GFXScrFont;
	UBYTE	scfg_GFXWinFontName[ MYMAXFONTNAMESIZE ];
	struct TextAttr	scfg_GFXWinFont;

	UBYTE	scfg_GFXPubScrName[ MAXPUBSCREENNAME+1 ];

	/* main screen type: */
	/* 0 = use default public screen */
	/* 1 = use gfx screen */
	/* 2 = use existing named pubscreen */
	UWORD	scfg_MainScrType;

	/* if mainscrtype=2 then use this named screen */
	UBYTE	scfg_MainPubScrName[ MAXPUBSCREENNAME+1 ];

	/* if DefaultFont=true then use system font instead of mainwinfont */
	BOOL	scfg_DefaultFont;
	UBYTE	scfg_MainWinFontName[ MYMAXFONTNAMESIZE ];
	struct TextAttr	scfg_MainWinFont;

	/* Wow man, groovy! Look at the colours on the wall... */
	struct Palette	scfg_Palette;

	/* not used (yet) */
	UWORD	scfg_Flags;
	ULONG	scfg_Reserved[5];
};


struct WinDim
{
	UWORD	Left;
	UWORD	Top;
	UWORD	Width;
	UWORD	Height;
	ULONG	Reserved[2];
};


struct ZonkConfig		/* as saved on disk */
{
	/* screen/palette settings */
	struct ScreenConfig	scfg;
	BOOL								MakeBackups;
	BOOL								SaveIcons;
	struct WinDim				SuperWindowDefs[16];	/* room for a few more */
	struct WinDim				FileReqDef;
	struct WinDim				FontReqDef;
	struct WinDim				SMReqDef;
	UWORD								OpReqW;
	UWORD								OpReqH;
	UBYTE crappad[ 512 - 4 - 16*16 - 16*3 - 4 ];
};


/**** SuperWindow ****/

struct SuperWindow
{
	struct Node		sw_Node;

	/* handler functions */
	VOID		(*sw_Handler)(struct SuperWindow *, struct IntuiMessage *);
	VOID		(*sw_HideWindow)(struct SuperWindow *);
	BOOL		(*sw_ShowWindow)(struct SuperWindow *);
	VOID		(*sw_BuggerOff)(struct SuperWindow *);
	BOOL		(*sw_RefreshMenus)(struct SuperWindow *);
	UBYTE		*(*sw_MakeTitle)(struct SuperWindow *);

	/* assorted stuff */
	UWORD						sw_Type;
	struct	Window	*sw_Window;
	struct	Gadget	*sw_Gadgets;
	APTR						sw_BlockingHandle;	/* for blocking the window */
	APTR						sw_VisualInfo;
	struct HOStuff	*sw_hos;
	UBYTE						sw_Name[ MAXWINNAMESIZE ];

	/* Menu crap for keeping track of gadtools menu creation */
	/* - global menus: */
	struct Menu			*sw_ProjectMenu;		/* always the first menu */
	struct Menu			*sw_EditMenu;
	struct Menu			*sw_WindowMenu;
	struct Menu			*sw_SettingsMenu;
	struct Menu			*sw_UserMenu;
	/* - Custom Menu(s) for this SuperWindow */
	struct Menu			*sw_CustomMenu;

	/* space to store intuition window values when we hide the window */
/*	WORD		sw_Left;
	WORD		sw_Top;
	WORD		sw_Width;
	WORD		sw_Height;
*/
	struct WinDim sw_WinDim;

};

// Values for sw_Type:
#define SWTY_ABOUT	 		1
#define	SWTY_SCRCONFIG	2
#define SWTY_CHUNK			3
#define SWTY_PROG				4
#define SWTY_FORMATION	5
#define SWTY_TIMEDWAVE	6
#define SWTY_WEAPON			7
#define SWTY_SFX				8
#define SWTY_THAP				9
#define SWTY_SWAVE			10
#define SWTY_GAME				11
#define SWTY_DISPLAY		12
#define SWTY_EOTR				13


struct DataNode
{
	struct Node	dn_Node;
	APTR				dn_Data;
};

struct BLKSFileHeader
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
};



struct Program
{
	struct Node			pg_Node;
	UBYTE						pg_Name[ PROGNAMESIZE ];
	BOOL						pg_Locked;
	struct MinList	pg_OpList;	// a list of OpNodes
};


struct OpNode
{
	struct MinNode		on_Node;
	UBYTE							*on_OpData;	// ptr to operator word and parameters
};


struct OpPage
{
	WORD				op_Start;				// which param is at beginning of this page
	WORD				op_Count;				// how many objects on this page
};


struct OperatorDef
{
	UBYTE					*od_Name;
	UWORD					od_Class;
	UWORD					od_Type;						// see below
	UWORD					od_ParamSize;				// size of parameters (in internal format )
	STRPTR 				od_Template;
	STRPTR 				*od_TemplateLabels;
	UWORD					od_PageCount;
	struct OpPage *od_PageLayout;
	APTR					*od_Aux;						//Auxilary field info
};


// od_ParamSize is a fixed number indicating the byte length of the
// expected parameters. Variable sized parameters (other operators, strings)
// are replaced by pointers to AllocVec()ed memory.

// od_Type values:
#define OPTYPE_ACTION 0
#define OPTYPE_CHANNL 1



struct Formation
{
	struct Node			fm_Node;												// for linking to the chunk
	UBYTE						fm_Name[ FORMATIONNAMESIZE ];		// name
	struct MinList	fm_DudeList;										// list of dudes

};


struct Dude
{
	struct MinNode	du_Node;												//link into formation
	BOOL						du_Tagged;
	UBYTE						du_Program[ PROGNAMESIZE ];
	LONG						du_XPos;
	LONG						du_YPos;
	UWORD						du_Delay;
};



struct WeaponDef
{
	struct Node			wd_Node;
	UBYTE						wd_Name[ WEAPONNAMESIZE ];
	UBYTE						wd_FireProgram[ PROGNAMESIZE ];
	UWORD						wd_FireDelay;
	UWORD						wd_MaxBullets;
	UBYTE						wd_PowerUp[ WEAPONNAMESIZE ];
	UBYTE						wd_PowerDown[ WEAPONNAMESIZE ];
	UWORD						wd_Flags;	/* bit 0: 0=exec on player, 1=create bullet */
};



struct TimedWave
{
	struct Node Noddy;
	UBYTE				Event[ GENERICNAMESIZE ];
	UWORD				Time;
	LONG				X;
	LONG				Y;
	UBYTE				Type;			// see below
	UBYTE				Flags;		// see below
};

#define TWTYPE_ACTIONLIST	0
#define TWTYPE_DUDEINIT		1
#define TWTYPE_FORMATION	2

#define TWFLAG_VIEWRELATIVE 0x01



struct Map
{
	UBYTE			map_Name[ MAPNAMESIZE ];
	UWORD			map_Width;
	UWORD			map_Height;
	UWORD			*map_Data;
};



struct Blockset
{
	UBYTE			bs_Name[ BLOCKSETNAMESIZE ];
	UWORD			bs_NumOfBlocks;
	UWORD			bs_Depth;
	UWORD			bs_LayoutWidth;	// how many blocks across in the Blocks window
														// (20 for lores)
	UWORD			bs_Flags;				//future
	UWORD			bs_BitMapHeight;
	UWORD			*bs_Images;
};

/* Params for the DrawMapBM() function */

struct DrawMapBMArgs
{
	struct BitMap 	*bm;				/* destination bitmap */
	UWORD						BMWidth;
	UWORD						BMHeight;
	struct Blockset *Blockset;
	struct Map			*Map;
	LONG						MapX;				/* block position to be drawn at topleft of bitmap */
	LONG						MapY;
	WORD						ViewWidth;	/* view dimensions in pixels */
	WORD						ViewHeight;
	UWORD						Flags;
	UWORD						ZoomFactor;	/* scale ratio for BitmapScale */
};



struct SFXHeader
{
	struct Node				sfx_Node;
	UBYTE							sfx_Name[ SFXNAMESIZE ];
	struct SpamParam 	sfx_Spam;
	/* sample data starts here... */
};


/***** BOB STUFF *****/

/* Bob Bank header */

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

/* File format bob header */

struct BobHeader
{
	UWORD bf_X;
	UWORD bf_Y;
	UWORD bf_Width;
	UWORD bf_Height;
	UWORD bf_Flags;
	UWORD bf_ColLeft;
	UWORD bf_ColTop;
	UWORD bf_ColWidth;
	UWORD bf_ColHeight;
	UBYTE	bf_Name[10];
	UWORD bf_Reserved[1];
	/* Image Data (((bf_Width+15)/8) & 0xFFFE)*bf_Height*bf_Depth */
	/* Mask Data same size as image data */
};

/* Struct for storing auxilary info about bob (such as uncompressed name...) */
/* This used internally by Zonk only */

struct BobInfoNode
{
	struct Node 			bin_Node;
	UBYTE 						bin_Name[ BOBNAMESIZE ];
	struct BobHeader	*bin_BobHeader;
	struct Chunk			*bin_Chunk;
};

/***** bob anim stuff *****/

/* ANIM chunk header */

struct SequenceFileHeader
{
	UBYTE	sfh_BobBankName[16];
	UWORD	sfh_NumOfAnims;
	/* array of AnimFileHeader Structs */
};


struct AnimFileHeader
{
	UBYTE afh_AnimName[10];
	UWORD	afh_NumOfFrames;
	UWORD	afh_AnimSpeed;
	/* Array of FrameFileHeader Structs */
};

struct FrameFileHeader
{
	ULONG	ffh_BobNumber;
	LONG	ffh_XOffset;
	LONG	ffh_YOffset;
};

/* Auxilary info node for bob animations - to store name in unpacked form */
/* and other Zonk-specific stuff. */

struct AnimInfoNode
{
	struct Node 					ain_Node;
	UBYTE									ain_Name[ ANIMNAMESIZE ];
	struct AnimFileHeader *ain_Anim;
	struct Chunk					*ain_Chunk;
};

/***** SCROLLWAVE STUFF *****/

/* Header for SWAV chunk on disk */
struct DFSWAVHeader
{
	UWORD	NumOfWaves;
	UWORD LfHead;
	UWORD RtHead;
	ULONG	Reserved;
};


/* Disk Format Scroll Triggered AttackWave Structure */
struct DFScrollWave
{
	/* struct linked into both left-sorted list and right-sorted list.						*/
	/* Left list is used when scrolling left. Same for right. Just the other			*/
	/* way round. Got it?																													*/
	UWORD	dsf_LfSucc;		/* index num of next DFScrollWave, (0xFFFF = end of list) */
	UWORD	dsf_RtSucc;		/* links for rightlist */

	/* dsf_Name is a Formation or an Actionlist depending on dsg_Flag values */
	UBYTE	dsf_Name[ GENERICNAMEPACKSIZE ];
	UBYTE	dsf_Flags;			/* see values below */
	UBYTE	dsf_AppearCount;	/* Number of times attackwave appears. 0=always. 		*/
	UWORD	dsf_LfTrigger;	/* triggerposition when scrolling left (unshifted)... */
	UWORD	dsf_RtTrigger;	/* ...and for scrolling right. 0xFFFF = no trigger		*/
	/* The position at which wave is to appear (unshifted). */
	WORD	dsf_XPos;
	WORD	dsf_YPos;
};

/* dsf_Flag values:																									*/
/* bit 0:			Wave type. 0= ActionList, 1=Formation.								*/
/* bits 1-7:	Undefined - leave zero. Maybe extend to more types in */
/*						here later.	*/


/* Zonk-Internal ScrollWave structure */
struct ZScrollWave
{
	struct MinNode zsw_Node;
	UWORD	zsw_Num;		/* index num of this wave (for sorting) 0xFFFF = sorted */
	UWORD	zsw_TrigFlags;	/* bit 0: rttrigger on/off, bit 1:lftrigger */
	struct BobInfoNode	*zsw_Image;
	WORD								zsw_ImageXOffset;
	WORD								zsw_ImageYOffset;
	UWORD								zsw_ImageW;
	UWORD								zsw_ImageH;
	/* these fields correspond to DFScrollWave struct */
	UWORD zsw_LfSucc;
	UWORD zsw_RtSucc;
	UBYTE	zsw_Name[ GENERICNAMESIZE ];
	UBYTE	zsw_Flags;
	UBYTE	zsw_AppearCount;
	UWORD	zsw_LfTrigger;
	UWORD	zsw_RtTrigger;
	WORD	zsw_XPos;
	WORD	zsw_YPos;
};


/***** PATH STUFF *****/

struct Path
{
	struct Node			pth_Node;
	UBYTE						pth_Name[ PATHNAMESIZE ];
	struct MinList	pth_Segments;
};


struct PathSeg
{
	struct MinNode psg_Node;
	UBYTE	psg_Type;						/* see defines below */
	UBYTE	psg_Tagged;					/* 1=tagged, 0=not */
	WORD	psg_XPos;						/* no bitshifts */
	WORD	psg_YPos;
	WORD	psg_CtrlX[2];
	WORD	psg_CtrlY[2];
	/* to be refined */
	WORD	psg_Speed;					/* pixels per frame, bitshifted */
	WORD	psg_SpreadType;			/* see defines below */
	WORD	psg_SpreadValue;		/* use depends on spead type */
};

#define PSGTYPE_HEAD 0
#define PSGTYPE_LINE 1
#define PSGTYPE_BEZCURVE 2

#define PSGSPREAD_NTOTAL		0		/* n points total, spread over whole segment */
#define PSGSPREAD_CONSTSPD	1		/* points evenly spaced n apart */

/***** GAME CONFIG STUFF *****/

struct GameConfig
{
	struct GameFileHeader gc_GameFileHeader;
	struct List						gc_Levels;		/* list of struct LevelConfigs */
};

struct LevelConfig
{
	struct Node							lc_Node;
	struct LevelFileHeader	lc_LevelFileHeader;
	UBYTE										lc_Name[ LEVELFULLNAMESIZE ]; /* unpacked */
	struct List							lc_Files;		/* list of struct LFiles */
};

struct LFile
{
	struct Node	lf_Node;
	UWORD				lf_Type;
	UBYTE				lf_Name[ MAXFILENAMESIZE ];
};


/*****************************************************************/

struct ZonkDisp
{
	UWORD	zdp_Reserved;
	UWORD zdp_Flags;								/* See defines below */
	UWORD	zdp_FrameRate;
	UBYTE	zdp_Map[16];
	UBYTE	zdp_BlockSet[16];
	UBYTE zdp_BackDrop[GENERICNAMESIZE];
	UBYTE zdp_InitProgram[PROGNAMESIZE];

	/* obsolete ? */
	UWORD	zdp_LevelWidth;
	UWORD zdp_LevelHeight;
	UWORD zdp_LevelDepth;
	ULONG	zdp_DisplayID;

	UWORD	zdp_NumOfPlayerStructs;	/* Num Of Player Structures to Allocate */
	UWORD	zdp_NumOfBulletStructs;	/* Num Of Bullet Structures to Allocate */
	UWORD	zdp_NumOfBDStructs;			/* Num Of Bad Dude Structures To Allocate */

		/* Sprite Stuff */
	UWORD	zdp_SpriteColourBank;		/* 0=0, 1=16, 2=32, 3=48, ... */
	UWORD	zdp_SpriteBurstMode;			/* 0 = No Burst(16), 1 = Half burst(32), 2 = Full Burst(64) */

	UWORD	zdp_GameHeaderHeight;								/* Height of the blanked area above the main game */
	UWORD	zdp_StatHeaderHeight;								/* Height of the blanked area above the statbox */
	UBYTE	zdp_StatBackDropPicture[GENERICNAMESIZE];
	ULONG	zdp_StatDisplayID;
	UBYTE	zdp_StatUpdateProg[PROGNAMESIZE];	/* Update code run every cycle */
	UBYTE	zdp_StatShieldBarImage[GENERICNAMESIZE];	/* Picture of all the shield bar images */

		/* Default Object Border Info */
	LONG	zdp_DefaultBorderLeft;
	UWORD	zdp_DefaultBorderLeftType;		/* different from disk format - viewrelative flags below */
	LONG	zdp_DefaultBorderRight;
	UWORD	zdp_DefaultBorderRightType;
	LONG	zdp_DefaultBorderTop;
	UWORD	zdp_DefaultBorderTopType;
	LONG	zdp_DefaultBorderBottom;
	UWORD	zdp_DefaultBorderBottomType;
	UWORD	zdp_DefaultBorderFlags;
	/* bit0 = LeftViewRelative? bit1 = RightViewRelative? */
	/* bit2 = TopViewRelative? bit3 = BottomViewRelative? */
	/* different from disk format! */

		/* Player Info */
	UWORD	zdp_NumOfPlayers;
	UBYTE	zdp_PlayerInitProgram1[PROGNAMESIZE];
	UBYTE	zdp_PlayerInitProgram2[PROGNAMESIZE];
	UBYTE	zdp_PlayerInitProgram3[PROGNAMESIZE];
	UBYTE	zdp_PlayerInitProgram4[PROGNAMESIZE];

		/* Palette */
	struct Palette	zdp_Palette;
};

/* Display Structure on disk */

struct Display
{
	UWORD dp_Reserved;
	UWORD dp_Flags;								/* See defines below */
	UWORD	dp_FrameRate;
	UBYTE	dp_Map[16];
	UBYTE	dp_BlockSet[16];
	UBYTE dp_BackDrop[GENERICNAMEPACKSIZE];
	UBYTE dp_InitProgram[PROGNAMEPACKSIZE];
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
	UBYTE	dp_StatBackDropPicture[GENERICNAMEPACKSIZE];
	ULONG	dp_StatDisplayID;
	UBYTE	dp_StatUpdateProg[PROGNAMEPACKSIZE];	/* Update code run every cycle */
	UBYTE	dp_StatShieldBarImage[GENERICNAMEPACKSIZE];	/* Picture of all the shield bar images */

		/* Default Object Border Info */
	LONG	dp_DefaultBorderLeft;
	WORD	dp_DefaultBorderLeftType;		/* -ve implies view relative */
	LONG	dp_DefaultBorderRight;
	WORD	dp_DefaultBorderRightType;		/* -ve implies view relative */
	LONG	dp_DefaultBorderTop;
	WORD	dp_DefaultBorderTopType;		/* -ve implies view relative */
	LONG	dp_DefaultBorderBottom;
	WORD	dp_DefaultBorderBottomType;		/* -ve implies view relative */
	UWORD	dp_DefaultBorderFlags;

		/* Player Info */
	UWORD	dp_NumOfPlayers;
	UBYTE	dp_PlayerInitProgram1[PROGNAMEPACKSIZE];
	UBYTE	dp_PlayerInitProgram2[PROGNAMEPACKSIZE];
	UBYTE	dp_PlayerInitProgram3[PROGNAMEPACKSIZE];
	UBYTE	dp_PlayerInitProgram4[PROGNAMEPACKSIZE];

		/* Palette */
	UWORD	dp_NumOfColours;
	UBYTE	dp_Palette[0];
};

#define DPB_AGAONLY 0
#define DPF_AGAONLY 1<<DPB_AGAONLY
#define DPB_SMOOTHBACKGROUND		1
#define DPF_SMOOTHBACKGROUND		1<<DPB_SMOOTHBACKGROUND

struct EdgeOfTheRoad
{
	UBYTE Name[ EOTRNAMESIZE ];
	UBYTE BlocksetName[ BLOCKSETNAMESIZE ];

	UWORD NumOfBlocks;
	UBYTE Data[MAXNUMOFBLOCKS];
};
