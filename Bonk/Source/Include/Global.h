/****************************************************************************/
//
// BONK: Global.h
//
/****************************************************************************/



//#include <intuition/intuition.h>
//#include <intuition/screens.h>
//#include <exec/nodes.h>


#define VERS "$VER: Bonk 0.01 ("__AMIGADATE__
#define VSTRING "0.01"

#define PROGNAME "BONK"

/* name sizes (including NULL) */
#define MAPNAMESIZE				16
#define BLOCKSETNAMESIZE	16

#define BRUSHHISTORY 10

/* MACROS */

#define MAINSCREEN mainscreen				/* kludge from waaaay back */

/* CONSTANTS */

#define	BASENAME	"MAPEDIT"
#define SCREENNAME "BONK"

/* Block dimensions */
#define BLKW	16			/* pixelwidth */
#define BLKWORDW 1		/* wordwidth */
#define BLKH	16			/* height */

/*  List of Error Codes */
enum	errors	{ERROR_ALLOK=0,ERROR_SCREEN, ERROR_WINDOW,
							ERROR_INTUITION,ERROR_GFX,ERROR_DOS,ERROR_GADTOOLS,
							ERROR_MEMORY,ERROR_REXX,ERROR_MENUS,ERROR_FONT,
							ERROR_ASLLIB, ERROR_ASL, ERROR_IFFPARSE };


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

/* custom IFF chunknames */
#define	ID_CONK		MAKE_ID('C','O','N','K')
#define	ID_MAP		MAKE_ID('M','A','P',' ')
#define	ID_BLKS		MAKE_ID('B','L','K','S')

/*****************************************************************/
//
//	MENUITEM CODES
//
// These defines can be used to index NewMenu arrays to suss out checkmarks
// etc... However, the actual menu positions vary from window to window and
// should be defined locally.
// The Project, Window, and Settings menus will generally be constant over
// all windows, but their positions may move about dependant on any other
// menus we feel we need to shove in. Anyway, we're the programmers so we
// can do whatever we bloody well feel like. So there. Nyah nyah nyaaaaaaaaaah!
//

/* PROJECT MENU */

/*
enum	projmenu	{PM_NEW=0,PM_OPEN,PM_BAR1,
									PM_SAVE,PM_SAVEAS,PM_BAR2,
									PM_HIDE,PM_REVEAL,PM_CLOSE,PM_BAR3,
									PM_ABOUT,PM_QUIT };
*/

#define MNPRJ_NEW 0
#define MNPRJ_OPEN 1
/* bar */
#define MNPRJ_SAVE 3
#define MNPRJ_SAVEAS 4
/* bar */
#define MNPRJ_HIDE 6
#define MNPRJ_REVEAL 7
#define MNPRJ_CLOSE 8
/* bar */
#define MNPRJ_ABOUT 10
#define MNPRJ_QUIT 11

/* EDIT MENU */
#define MNEDIT_UNDO					0
#define MNEDIT_LOADBLOCKS		2
#define MNEDIT_LOADPALETTE	3
/* more... later. */

/* WINDOW MENU */

#define MNWIN_OPEN					0
	/* subitems */
	#define MNWINOPEN_PROJ			0
	#define MNWINOPEN_TOOLS			1
	#define MNWINOPEN_BLOCKS		2
	#define MNWINOPEN_BRUSHES		3
	#define MNWINOPEN_PREVIEW		4
	#define MNWINOPEN_REVEAL		5
	#define MNWINOPEN_USERCON		6
	#define MNWINOPEN_SCRCON		7
	#define MNWINOPEN_ABOUT			8

#define MNWIN_HIDE					1
#define	MNWIN_REVEAL				2
#define	MNWIN_CLOSE					3
//#define	MNWIN_MAKEDEFAULT		4
/* bar */
#define MNWIN_BACKDROP			5
#define MNWIN_BORDERLESS		6
#define MNWIN_TITLE					7
#define MNWIN_BORDERGADGETS	8

#define MNWIN_OPEN_IDX					1
	/* subitems */

	#define MNWINOPEN_PROJ_IDX			2
	#define MNWINOPEN_TOOLS_IDX			3
	#define MNWINOPEN_BLOCKS_IDX		4
	#define MNWINOPEN_BRUSHES_IDX		5
	#define MNWINOPEN_PREVIEW_IDX		6
	#define MNWINOPEN_REVEAL_IDX		7
	#define MNWINOPEN_USERCON_IDX		8
	#define MNWINOPEN_SCRCON_IDX		9
	/* dummy here */
	#define MNWINOPEN_ABOUT_IDX			10

#define MNWIN_HIDE_IDX						11
#define	MNWIN_REVEAL_IDX					12
#define	MNWIN_CLOSE_IDX						13
//#define	MNWIN_MAKEDEFAULT_IDX			14
/* bar = 14 */
#define MNWIN_BACKDROP_IDX				15
#define MNWIN_BORDERLESS_IDX			16
#define MNWIN_TITLE_IDX						17
#define MNWIN_BORDERGADGETS_IDX		18


/* SETTINGS MENU */

#define MNSET_MAKEBACKUPS			0
//#define MNSET_WINDOWIDS				1
/* bar */
#define MNSET_SETSETTINGS			2
/* bar */
#define MNSET_LOADSETTINGS		4
#define MNSET_SAVESETTINGS		5
#define MNSET_SAVESETTINGSAS	6

#define MNSET_MAKEBACKUPS_IDX			1
//#define MNSET_WINDOWIDS_IDX				2
/* bar */
#define MNSET_SETSETTINGS_IDX			3
/* bar */
#define MNSET_LOADSETTINGS_IDX		5
#define MNSET_SAVESETTINGS_IDX		6
#define MNSET_SAVESETTINGSAS_IDX	7


/* MAP MENU */

#define MNMAP_FIXBG					0
#define MNMAP_RESIZE				1
#define MNMAP_RENAME				2
#define MNMAP_GOTOBM				3
	/* subitems */
	#define MNMAPGOTOBM_1				0
	#define MNMAPGOTOBM_2				1
	#define MNMAPGOTOBM_3				2
	#define MNMAPGOTOBM_4				3
	#define MNMAPGOTOBM_5				4
	#define MNMAPGOTOBM_6				5
	#define MNMAPGOTOBM_7				6
	#define MNMAPGOTOBM_8				7
	#define MNMAPGOTOBM_9				8
	#define MNMAPGOTOBM_10			9
#define MNMAP_SETBM					4
	/* subitems */
	#define MNMAPSETBM_1				0
	#define MNMAPSETBM_2				1
	#define MNMAPSETBM_3				2
	#define MNMAPSETBM_4				3
	#define MNMAPSETBM_5				4
	#define MNMAPSETBM_6				5
	#define MNMAPSETBM_7				6
	#define MNMAPSETBM_8				7
	#define MNMAPSETBM_9				8
	#define MNMAPSETBM_10			9

#define MNMAP_FIXBG_IDX					1
#define MNMAP_RESIZE_IDX				2
#define MNMAP_RENAME_IDX					3
#define MNMAP_GOTOBM_IDX				4
	/* subitems */
	#define MNMAPGOTOBM_1_IDX				5
	#define MNMAPGOTOBM_2_IDX				6
	#define MNMAPGOTOBM_3_IDX				7
	#define MNMAPGOTOBM_4_IDX				8
	#define MNMAPGOTOBM_5_IDX				9
	#define MNMAPGOTOBM_6_IDX				10
	#define MNMAPGOTOBM_7_IDX				11
	#define MNMAPGOTOBM_8_IDX				12
	#define MNMAPGOTOBM_9_IDX				13
	#define MNMAPGOTOBM_10_IDX			14
#define MNMAP_SETBM_IDX						15
	/* subitems */
	#define MNMAPSETBM_1_IDX				16
	#define MNMAPSETBM_2_IDX				17
	#define MNMAPSETBM_3_IDX				18
	#define MNMAPSETBM_4_IDX				19
	#define MNMAPSETBM_5_IDX				20
	#define MNMAPSETBM_6_IDX				21
	#define MNMAPSETBM_7_IDX				22
	#define MNMAPSETBM_8_IDX				23
	#define MNMAPSETBM_9_IDX				24
	#define MNMAPSETBM_10_IDX				25

/* The Tool gadget ID's, these are split up into group as follows:
			00-19		Tools
			20-39		Hit Select (Undo, Clr, ...)
			40-59		Toggle Select (Magnify, Grid Lock, ...)
			60-79		Brushes

*/
#define TL_TOOLSSTART		0
#define TL_TOOLSEND			19
#define TL_HITSTART			20
#define TL_HITEND				39
#define	TL_TOGGLESTART	40
#define TL_TOGGLEEND		59
#define TL_BRUSHSTART		60
#define TL_BRUSHEND			79


#define	TL_DRAW			0
#define	TL_LINE			1
#define	TL_BOX			2
#define	TL_FBOX			3
#define	TL_CUT			4
#define TL_PASTE		5
#define	TL_FILL			6
#define TL_PICK			7
#define TL_UNDO			20
//#define TL_CLR			21
#define TL_GRIDLOCK	40

/* number of gadgets in tools window */
#define TG_NUMOFGADGETS	9


/*****************************************************************/
//
// DATA STRUCTURES
//


struct ReqCrap
{
	char	*rc_Label;							/* gadget label */
	UWORD	rc_Type;								/* see HOOPYREQTYPE_... defines */
	APTR	rc_Data;								/* depends on type */
	LONG	rc_Value;
	struct Gadget *rc_Gadget;			/* gadget ptr, filled in by DoHoopyReq */
};

#define HOOPYREQTYPE_TEXT		0
#define HOOPYREQTYPE_INT		1
#define HOOPYREQTYPE_STRING	2
#define HOOPYREQTYPE_BUTTON	3
#define HOOPYREQTYPE_CHECK	4
#define HOOPYREQTYPE_CYCLE	5
#define HOOPYREQTYPE_NULL		6

/*
TEXT  - Draws the text in the rc_Label field. Nothing clickable, no return value.
INT   - Integer gadget, initial number given in rc_Value. Final number
				returned in rc_Value.
CHECK - CheckBox gadget. Initial/return states in rc_Value.
CYCLE - Cycle gadget. Array of strings to cycle through given in rc_Data.
				Inital/return selections in rc_Value.
STRING - rc_Data = buffer, rc_Value = maxchars
*/


/* see Brushes.c */

struct Brush
{
	struct Node br_Node;
	UWORD	br_Width;
	UWORD br_Height;
	UWORD *br_Data;
};




struct UndoItem
{
	UWORD	ui_Block;
	UWORD ui_XPos;
	UWORD ui_YPos;
};

struct UndoBuffer
{
	struct ProjectWindow *ub_ProjectWindow;
	ULONG ub_Index;
	ULONG	ub_ArraySize;
	struct UndoItem *ub_Array;
};

#define INITUNDOBUFSIZE 1000
#define UNDOBUFSTEP	500

/********** SuperWindow Base Structure *******/

struct SuperWindow
{
	struct	Node	sw_Node;

	/* handler functions */
	void		(*sw_Handler)(struct SuperWindow *, struct IntuiMessage *);
	void		(*sw_HideWindow)(struct SuperWindow *);
	BOOL		(*sw_ShowWindow)(struct SuperWindow *);
	void		(*sw_BuggerOff)(struct SuperWindow *);
	BOOL		(*sw_RefreshMenus)(struct SuperWindow *);

	/* assorted stuff */
	UWORD		sw_Type;
	struct	TextAttr *sw_TextAttr;
	struct	TextFont *sw_TextFont;
	struct	Window	*sw_Window;
	struct	Menu		*sw_MenuStrip;
	struct	Gadget	*sw_GadList;
	APTR		*sw_VisualInfo;
	UWORD		sw_id;
	char		sw_idstring[6];

	/* stuff needed to open the intuition window */
	WORD		sw_Left;
	WORD		sw_Top;
	WORD		sw_Width;
	WORD		sw_Height;
	WORD		sw_MinWidth;
	WORD		sw_MinHeight;
	UWORD		sw_MaxWidth;
	UWORD		sw_MaxHeight;
	char		*sw_Title;				/* SuperWindow base title (no idstring etc...) */
//	ULONG		sw_Flags;
	BOOL		sw_FlgBackDrop;
	BOOL		sw_FlgBorderless;
	BOOL		sw_FlgTitle;
	BOOL		sw_FlgBorderGadgets;
};

/*					SuperWindow explanation:

Node:				Linkdata, name and stuff. The name field is pointed to sw_idstring
						for easy AREXX reference.
NextProjectWindow: Pointer to the next window	attached to this process.
Handler:		Handles IDCMP messages going into this window. The handler
						routine is responsible for replying to the message.
HideWindow:	Routine to hide the window (used, for example, when we're moving
						the superwindow to a new screen).
ShowWindow:	Redisplays the window if it's hidden.
BuggerOff:	Frees all data associated with this superwindow and shuts it
						down and unlinks it and frees the superwindow struct and stuff.
Type:				Which type of superwindow this is (eg image, map, text etc...)
Screen:			Pointer the Intuition Screen that the window is on.
Window:			Pointer to an associated Intuition window. This field will be NULL
						if there is no intuition window at the moment (it could be hidden).
						When the window is open, the wd_UserData field should point back to
						the SuperWindow structure it is attached to, so we know where to
						send all the IDCMP messages.
MenuStrip:	Pointer to the menu strip for that superwindow. Used so that we can
						remember what to kill when we close the window.
VisualInfo:	Pointer to the GadTools visual info struct. Each window has its
						own VisualInfo, so that any window can goto any screen.
idstring:		The windows unique reference id. Starting from W1 to W999. This is
						really only for AREXX reference, but can be used by anything..
id:					The number part of the idstring, stored as a uword.
WinSettings:This stuff is concerned with the preservation of the window when
						it is hidden, and then re-opened.
*/



/************** SuperWindow extensions: **************/

/* types of superwindows */
#define	SWTY_PROJECT				0
#define	SWTY_ABOUT					1
#define	SWTY_REVEAL					2
#define SWTY_SCREENCONFIG		3
#define SWTY_TOOLS					4
#define SWTY_USERCONFIG			5
#define SWTY_BLOCK					6
#define SWTY_PREVIEW				7
#define SWTY_BRUSH					8
#define SWTY_BLKANIM				9

#define	SWTY_NUM						10	/* num of SuperWindow types */

/* ProjectWindow */

struct ProjectWindow
{
	struct SuperWindow pw_sw;
	struct	Project	*pw_Project;
	struct	ProjectWindow *pw_NextProjWindow;

	/* current viewing position, in map coords */
	UWORD	pw_MapX;
	UWORD	pw_MapY;
	/* selector stuff */
	BOOL	pw_SelState;	/* TRUE = onscreen, FALSE = not drawn (so don't erase!) */
	UWORD	pw_SelType;		/* the tool type used to draw the selector (curenttool) */
	UWORD	pw_SelToolState;	/* toolstate when the selector was drawn */
	WORD	pw_SelX0;			/* last drawn selector positions (blk coords) */
	WORD	pw_SelY0;
	WORD	pw_SelX1;			/* (other corner for rectangles etc) */
	WORD	pw_SelY1;

	WORD	pw_AnchorX;				/* anchor point for rubberbanding (map coords) */
	WORD	pw_AnchorY;
	WORD	pw_PrevX;					/* position from previous move event */
	WORD	pw_PrevY;

	UWORD	pw_ScrollyState;	/* which scrollbar is currently down (or GAD_NONE) */
	struct Gadget *pw_VScroller;
	struct Gadget *pw_UpArrow;
	struct Gadget *pw_DownArrow;
	struct Gadget *pw_HScroller;
	struct Gadget *pw_LeftArrow;
	struct Gadget *pw_RightArrow;
	struct Image *pw_UpImage;
	struct Image *pw_DownImage;
	struct Image *pw_LeftImage;
	struct Image *pw_RightImage;
};


/* BlockWindow */

struct BlockWindow
{
	struct SuperWindow bw_sw;
	WORD	bw_WinX;					/* position of viewing window */
	WORD	bw_WinY;
	BOOL	bw_DragFlag;
	WORD	bw_PrevX;					/* position from previous move event */
	WORD	bw_PrevY;
	WORD	bw_AnchorX;				/* anchor point for rubberbanding (map coords) */
	WORD	bw_AnchorY;

	BOOL	bw_SelState;
	WORD	bw_SelX0;					/* coords of last drawn selector (if SelState TRUE) */
	WORD	bw_SelY0;
	WORD	bw_SelX1;
	WORD	bw_SelY1;

};


/* BrushWindow */

struct BrushWindow
{
	struct SuperWindow uw_sw;
	struct Gadget *uw_Gadgets[3];
	UWORD		uw_Dummy[4];
};


/* PreviewWindow */

struct PreviewWindow
{
	struct SuperWindow		vw_sw;
	struct Project				*vw_Project;
	struct PreviewWindow	*vw_NextPreviewWindow;
	WORD	vw_WinX;				/* position of viewing window */
	WORD	vw_WinY;
};


/* AboutWindow */

struct AboutWindow
{
	struct SuperWindow aw_sw;
	char aw_crap[14];
};

/* RevealWindow */

struct RevealWindow
{
	struct SuperWindow	rw_sw;
	UWORD								rw_NowShowing;				/* see below for values */
	char								rw_TitleString[14];
	struct List					rw_Labels;
	struct Gadget				*rw_LVGadget;
	UWORD								rw_CurrentSelection;
};

/* values for rw_NowShowing */
#define RWNS_WINDOWS	0			/* show hidden windows */
#define RWNS_PROJECTS	1			/* show hidden projects only */

/*
rw_NowShowing:
	What this RevealWindow is currently showing - hidden Projects
	or hidden SuperWindows.
rw_TitleString:
	Space to store window title.
rw_Labels:
	List of names (of hidden windows/projects) to
	be displayed in the listview gadget (A List of DataNodes).
rw_LVGadget:
	ptr to the actual listview gadget (or NULL).
rw_CurrentSelection:
	The most recently selected item in the listview.
	0xFFFF = none selected
*/


/* ScreenConfigWindow */

/* structure definition moved to ScreenConfigWindow.c */



/* BlkAnimWindow */
struct BlkAnimWindow
{
	struct SuperWindow baw_sw;
	struct Project				*baw_Project;
	struct BlkAnimWindow	*baw_NextBlkAnimWindow;
	WORD									baw_CurrentAnimNum;	/* ~0 = none selected */
	struct BlkAnim				*baw_CurrentAnim;
	WORD									baw_BlkViewTopEdge;
	WORD									baw_BlkViewWidth;
	WORD									baw_BlkViewLeft;
	WORD									baw_SelectedBlk;
	ULONG									baw_LastSeconds;
	ULONG									baw_LastMicros;
	struct Gadget					*baw_Gadgets[10];
	UWORD	baw_undobuf[256];
};



/* BlkAnim */

struct BlkAnim
{
	struct Node	ba_Node;
	UWORD	ba_Length;
	UWORD	ba_Data[256];
};




/************** Project Structure **************/


struct Project
{
	struct	Node	pj_Node;								/* ln_Name is name of project */
	char		*pj_Drawer;										/* path for loading/saving */
	struct	ProjectWindow *pj_ProjectWindows;	/* ProjectWindows viewing this Project */
	struct	PreviewWindow *pj_PreviewWindows;
	struct	BlkAnimWindow	*pj_BlkAnimWindows;
	struct	List					pj_BlkAnims;		/* BlkAnim list for this project */
	UWORD									pj_NumOfBlkAnims;	/* num of items in above list */
	/* blk stuff here please */

	BOOL		pj_ModifiedFlag;							/* has this project been changed? */
	BOOL		pj_UntitledFlag;							/* unset path/filename? */
	/* the next two flags are for the 'Save-BlockSet-and/or-Palette-along-with- */
	/* the-mapdata?' requester that comes up when we save a project.            */
	BOOL		pj_SavePaletteFlag;
	BOOL		pj_SaveBlockSetFlag;

	/* map attributes */
	UWORD		pj_MapW;									/* width in blocks */
	UWORD		pj_MapH;									/* height in blocks */
	UWORD		*pj_Map;									/* ptr to mapdata */
	BOOL		pj_BackLockFlag;					/* Locking on/off */
	UWORD		*pj_BackMap;							/* copy of map for locked background */
	UBYTE		pj_MapName[ MAPNAMESIZE ];

	/* 10 bookmark positions */
	UWORD		pj_BookMarkX[10];
	UWORD		pj_BookMarkY[10];

	/* small bitmap (1 pixel = 1 block) to speed up preview window rendering */
//	struct BitMap	*pj_PreviewBM;
//	UWORD		pj_PreviewBMDepth;
};


/************** DataNode structure **************/

/* DataNode, used to form lists of objects */
/* (great for temporary stuff) */

struct DataNode
{
	struct Node					dn_Node;
	APTR								dn_Ptr;
};

/************** Palette structure **************/
//
//
//

struct Palette
{
	/* number of colours in this palette */
	UWORD								pl_Count;
	/* ptr to RGB data - 8 bits per component, in rgb ubyte triplets */
	/* ie same format as iff CMAP chunk */
	UBYTE								*pl_Colours;
};

/************** WindowDefault structure **************/
// Used to store window dimensions and attributes.

struct WindowDefault
{
	WORD		wf_Left;
	WORD		wf_Top;
	WORD		wf_Width;
	WORD		wf_Height;
	BOOL		wf_FlgBackDrop;
	BOOL		wf_FlgBorderless;
	BOOL		wf_FlgTitle;
	BOOL		wf_FlgBorderGadgets;
};


#define		MYMAXCMDLEN	80
struct UserMenuEntry
{
	char		um_Name[20];
	char		um_Command[MYMAXCMDLEN];
};

#define MYMAXFONTNAMESIZE 256

struct ScreenConfig
{
	char								sg_BorrowedName[MAXPUBSCREENNAME+2];
	ULONG								sg_DisplayID;
	ULONG								sg_DisplayWidth;
	ULONG								sg_DisplayHeight;
	UWORD								sg_DisplayDepth;
	UWORD								sg_OverscanType;
	BOOL								sg_OwnScreen;
	BOOL								sg_AutoScroll;
	BOOL								sg_Shanghai;
	/* screenfont */
	char								sg_SFNameBuf[MYMAXFONTNAMESIZE];
	UWORD								sg_SFYSize;
	UBYTE								sg_SFStyle;
	UBYTE								sg_SFFlags;
	/* windowfont */
	char								sg_WFNameBuf[MYMAXFONTNAMESIZE];
	UWORD								sg_WFYSize;
	UBYTE								sg_WFStyle;
	UBYTE								sg_WFFlags;

};

struct Config
{
	BOOL									cg_CreateIcons;
	BOOL									cg_MakeBackups;
	struct ScreenConfig		cg_ScrConfig;
	struct WindowDefault	cg_WinDefs[SWTY_NUM];
	struct UserMenuEntry	cg_UserMenu[20];
};


/********************** BlockSet Structure **********************/
//
// At the moment at map editor is prettymuch hardcoded for 16*16
// interleaved blocks.
//
// Note that this is the _internal_ blockset struct - the
// file format that blocksets are saved out under is given
// in the BLKSHeader struct in 'Files.c'.
//

struct BlockSet
{
	char					bs_Name[ BLOCKSETNAMESIZE ];	// nullterminated name.
	UWORD					bs_NumOfBlocks;
	UWORD					bs_LayoutWidth;		// how many blocks across in the Blocks window
																	// (20 for lores)
	UBYTE					bs_Depth;					// bitplanes
	UBYTE					bs_Pad;						// 0
	UWORD					bs_BytesPerRow;		// bitmap width when blitting
	ULONG					bs_BlkSize;				// size of each block, in bytes
	UWORD					bs_BitMapHeight;	// info for loading into the graphics editor
	UBYTE					*bs_Data;					// interleaved image data
	UBYTE					*bs_PreviewColours;	// array - colour for each block
};

#include <protos.h>
