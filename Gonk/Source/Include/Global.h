#include <intuition/intuition.h>
#include <intuition/screens.h>
#include <exec/nodes.h>

#include <libraries/iffparse.h>

/* MACROS */

#define MAINSCREEN mainscreen

/* CONSTANTS */

#define	BASENAME	"GONK"
#define SCREENNAME "GONK"
/* Number of Gadgets on the Tools Window. */
//#define TG_NUMOFGADGETS	30
#define TG_NUMOFGADGETS	41
#define PG_NUMOFGADGETS	2
#define MAGNIFYLEVELS 24
#define BOB_MAXWIDTH 1023
#define BOB_MAXHEIGHT 1023

/* Name length stuff */
#define BOBNAMESIZE 14
#define ANIMNAMESIZE 14
#define BANKNAMESIZE 14
#define BOBFILENAMESIZE 10
#define ANIMFILENAMESIZE 10
#define BANKFILENAMESIZE 10

#define SIGB_BREAK SIGBREAKB_CTRL_C
#define SIGB_GOOD	SIGBREAKB_CTRL_E
#define SIGB_BAD	SIGBREAKB_CTRL_F

#define SIGF_BREAK 1L<<SIGB_BREAK
#define SIGF_GOOD	1L<<SIGB_GOOD
#define SIGF_BAD	1L<<SIGB_BAD

/*  List of Error Codes */

/*enum	errors	{ERROR_ALLOK=0,ERROR_SCREEN, ERROR_WINDOW,
							ERROR_INTUITION,ERROR_GFX,ERROR_DOS,ERROR_GADTOOLS,
							ERROR_MEMORY,ERROR_REXX,ERROR_BLANK,ERROR_FONT,
							ERROR_ASLLIB, ERROR_ASL,ERROR_IFFPARSELIB,
							ERROR_LAYERSLIB,ERROR_ };
 */

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

#define TL_POINT		0
#define TL_PLOT			1
#define TL_LINE			2
#define TL_CURVE		3
#define TL_OBOX			4
#define TL_SBOX			5
#define TL_OCIRCLE	6
#define TL_SCIRCLE	7
#define TL_OELLIPSE	8
#define TL_SELLIPSE	9
#define TL_GETBRUSH 10
#define TL_GETANIMBRUSH  11
#define TL_FILL			12
#define TL_BRUSHHANDLE	13
#define TL_MAGNIFY	14
#define TL_TYPE			15
#define TL_PICKCOLOUR		16
#define TL_HITZONE		17
//#define TL_SPRAY		17

#define TL_UNDO			20
#define	TL_CLR			21

#define TL_GRIDLOCK	40

#define TLB_DOT			60
#define TLB_BRUSH1	61
#define TLB_BRUSH2	62
#define TLB_BRUSH3	63
#define TLB_BRUSH4	64
#define TLB_BRUSH5	65
#define TLB_BRUSH6	66
#define TLB_BRUSH7	67
#define TLB_BRUSH8	68
#define TLB_CUSTOM	69
#define TLB_ANIMBRUSH	70


#define	PJG_IN			0
#define PJG_OUT			1
#define PJG_MAGNIFY 2
#define PJG_BOBNAME 3

/*  IFF Error codes */
#define MYIFFERR_ALLOK 0
#define MYIFFERR_CANTOPENFILE 1
#define MYIFFERR_NOBMHDCHUNK 2
#define MYIFFERR_NOCMAPCHUNK 3
#define MYIFFERR_NOCAMGCHUNK 4
#define MYIFFERR_NOBODYCHUNK 5
#define MYIFFERR_UNKNOWNCOMPRESSION 6
#define MYIFFERR_MEMORY 7
#define MYIFFERR_NOTILBMFILE 8
#define MYIFFERR_MAXERROR 8

/* Brush ID Codes */
#define BR_NUMOFBRUSHES 11
#define BR_DOT (TLB_DOT-TL_BRUSHSTART)
#define BR_BRUSH1 (TLB_BRUSH1-TL_BRUSHSTART)
#define BR_BRUSH2 (TLB_BRUSH2-TL_BRUSHSTART)
#define BR_BRUSH3 (TLB_BRUSH3-TL_BRUSHSTART)
#define BR_BRUSH4 (TLB_BRUSH4-TL_BRUSHSTART)
#define BR_BRUSH5 (TLB_BRUSH5-TL_BRUSHSTART)
#define BR_BRUSH6 (TLB_BRUSH6-TL_BRUSHSTART)
#define BR_BRUSH7 (TLB_BRUSH7-TL_BRUSHSTART)
#define BR_BRUSH8 (TLB_BRUSH8-TL_BRUSHSTART)
#define BR_CUSTOM (TLB_CUSTOM-TL_BRUSHSTART)
#define BR_ANIMBRUSH (TLB_ANIMBRUSH-TL_BRUSHSTART)

/* Draw Modes */
#define DRMD_MATTE		0
#define DRMD_COLOUR		1
#define DRMD_REPLACE	2
#define DRMD_SHADE		3
#define DRMD_SMOOTH		4
#define DRMD_CYCLE		5
#define DRMD_SOLID		6

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

#define MN_NUMOFMENUS 7

/* PROJECT MENU */

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
#define MNPRJ_PAGESIZE 10
#define MNPRJ_SCREENMODE 11
/* bar */
#define MNPRJ_ICONIFY 13
#define MNPRJ_ABOUT 14
#define MNPRJ_QUIT 15

#define MNPRJ_NEW_IDX 1
#define MNPRJ_OPEN_IDX 2
/* bar */
#define MNPRJ_SAVE_IDX 4
#define MNPRJ_SAVEAS_IDX 5
/* bar */
#define MNPRJ_HIDE_IDX 7
#define MNPRJ_REVEAL_IDX 8
#define MNPRJ_CLOSE_IDX 9
/* bar */
#define MNPRJ_PAGESIZE_IDX 11
#define MNPRJ_SCREENMODE_IDX 12
/* bar */
#define MNPRJ_ICONIFY_IDX 14
#define MNPRJ_ABOUT_IDX 15
#define MNPRJ_QUIT_IDX 16

/* EDIT MENU */
#define MNEDT_COLOUR				0
	#define MNEDTCOL_USEPALETTE		0
	#define MNEDTCOL_REMAP				1
	#define MNEDTCOL_LOADPALETTE	2
	#define MNEDTCOL_SAVEPALETTE	3
#define MNEDT_UNDO					1
#define MNEDT_NAMECHUNK			2
/* bar */
#define MNEDT_BLOCKS2				4
	#define MNEDTBLOCKS_DUMMY1	0
	#define MNEDTBLOCKS_DUMMY2	1
#define MNEDT_BOBS2					5
	#define MNEDTBOBS_PREVIOUS	0
	#define MNEDTBOBS_NEXT			1
	#define MNEDTBOBS_GOTO			2
	#define MNEDTBOBS_PLAY			3
	#define MNEDTBOBS_PINGPONG	4
	/* bar */
	#define MNEDTBOBS_INSERT		6
	#define MNEDTBOBS_DELETE		7
	#define MNEDTBOBS_COPY			8
	/* bar */
	#define MNEDTBOBS_EDITMASK	10
	#define MNEDTBOBS_DELETEMASK	11
	#define MNEDTBOBS_REMAKEMASK	12
	/* bar */
	#define MNEDTBOBS_EDITZONE	14
	#define MNEDTBOBS_DELETEZONE	15
	#define MNEDTBOBS_REMAKEZONE	16
#define MNEDT_SPRITES2			6
	#define MNEDTSPRITES_PREVIOUS	0
	#define MNEDTSPRITES_NEXT			1
	#define MNEDTSPRITES_GOTO			2
	#define MNEDTSPRITES_PLAY			3
	#define MNEDTSPRITES_PINGPONG	4
	/* bar */
	#define MNEDTSPRITES_INSERT		6
	#define MNEDTSPRITES_DELETE		7
	#define MNEDTSPRITES_COPY			8
	/* bar */
	#define MNEDTSPRITES_SETNUMBER	10
	/* bar */
	#define MNEDTSPRITES_EDITZONE	12
	#define MNEDTSPRITES_DELETEZONE	13
	#define MNEDTSPRITES_REMAKEZONE	14
#define MNEDT_FRAME2				7
	#define MNEDTFRAME_DUMMY1		0
	#define MNEDTFRAME_DUMMY2		1
/* bar */
#define MNEDT_BLOCKS				9
#define MNEDT_BOBS					10
#define MNEDT_SPRITES				11
#define MNEDT_FRAME					12

#define MNEDT_NUMOFITEMS	13

#define MNEDT_COLOUR_IDX				1
	#define MNEDTCOL_USEPALETTE_IDX		2
	#define MNEDTCOL_REMAP_IDX				3
	#define MNEDTCOL_LOADPALETTE_IDX	4
	#define MNEDTCOL_SAVEPALETTE_IDX	5
#define MNEDT_UNDO_IDX					6
#define MNEDT_NAMECHUNK_IDX			7
/* bar */
#define MNEDT_BLOCKS2_IDX				9
	#define MNEDTBLOCKS_DUMMY1_IDX	10
	#define MNEDTBLOCKS_DUMMY2_IDX	11
#define MNEDT_BOBS2_IDX					12
	#define MNEDTBOBS_PREVIOUS_IDX	13
	#define MNEDTBOBS_NEXT_IDX			14
	#define MNEDTBOBS_GOTO_IDX			15
	#define MNEDTBOBS_PLAY_IDX			16
	#define MNEDTBOBS_PINGPONG_IDX	17
	/* bar */
	#define MNEDTBOBS_INSERT_IDX		19
	#define MNEDTBOBS_DELETE_IDX		20
	#define MNEDTBOBS_COPY_IDX			21
	/* bar */
	#define MNEDTBOBS_EDITMASK_IDX		23
	#define MNEDTBOBS_DELETEMASK_IDX	24
	#define MNEDTBOBS_REMAKEMASK_IDX	25
	/* bar */
	#define MNEDTBOBS_EDITZONE_IDX	27
	#define MNEDTBOBS_DELETEZONE_IDX	28
	#define MNEDTBOBS_REMAKEZONE_IDX	29
#define MNEDT_SPRITES2_IDX			30
	#define MNEDTSPRITES_PREVIOUS_IDX	31
	#define MNEDTSPRITES_NEXT_IDX			32
	#define MNEDTSPRITES_GOTO_IDX			33
	#define MNEDTSPRITES_PLAY_IDX			34
	#define MNEDTSPRITES_PINGPONG_IDX	35
	/* bar */
	#define MNEDTSPRITES_INSERT_IDX		37
	#define MNEDTSPRITES_DELETE_IDX		38
	#define MNEDTSPRITES_COPY_IDX			39
	/* bar */
	#define MNEDTSPRITES_SETNUMBER_IDX	41
	/* bar */
	#define MNEDTSPRITES_EDITZONE_IDX	43
	#define MNEDTSPRITES_DELETEZONE_IDX	44
	#define MNEDTSPRITES_REMAKEZONE_IDX	45
#define MNEDT_FRAME2_IDX				46
	#define MNEDTFRAME_DUMMY1_IDX		47
	#define MNEDTFRAME_DUMMY2_IDX		48
/* bar */
#define MNEDT_BLOCKS_IDX				50
#define MNEDT_BOBS_IDX					51
#define MNEDT_SPRITES_IDX				52
#define MNEDT_FRAME_IDX					53

/* WINDOW MENU */

#define MNWIN_OPEN					0
	/* subitems */
	#define MNWINOPEN_PROJ			0
	#define MNWINOPEN_ABOUT			1
	#define MNWINOPEN_REVEAL		2
	#define MNWINOPEN_SCRCON		3
	#define MNWINOPEN_TOOLS			4
	#define MNWINOPEN_USERCON		5
	#define MNWINOPEN_PALETTE		6
	#define MNWINOPEN_EDPALETTE	7
	#define MNWINOPEN_SEQUENCER	8
	#define MNWINOPEN_RANGE			9
#define MNWIN_HIDE					1
#define	MNWIN_REVEAL				2
#define	MNWIN_CLOSE					3
#define	MNWIN_MAKEDEFAULT		4
/* bar */
#define MNWIN_BACKDROP			6
#define MNWIN_BORDERLESS		7
#define MNWIN_TITLE					8
#define MNWIN_BORDERGADGETS	9
/* bar */
#define MNWIN_VERTICAL			11

#define MNWIN_OPEN_IDX					1
	/* subitems */
	#define MNWINOPEN_PROJ_IDX			2
	#define MNWINOPEN_ABOUT_IDX			3
	#define MNWINOPEN_REVEAL_IDX		4
	#define MNWINOPEN_SCRCON_IDX		5
	#define MNWINOPEN_TOOLS_IDX			6
	#define MNWINOPEN_USERCON_IDX		7
	#define MNWINOPEN_PALETTE_IDX		8
	#define MNWINOPEN_EDPALETTE_IDX		9
	#define MNWINOPEN_SEQUENCER_IDX		10
	#define MNWINOPEN_RANGE_IDX				11
#define MNWIN_HIDE_IDX						12
#define	MNWIN_REVEAL_IDX					13
#define	MNWIN_CLOSE_IDX						14
#define	MNWIN_MAKEDEFAULT_IDX			15
/* bar */
#define MNWIN_BACKDROP_IDX				17
#define MNWIN_BORDERLESS_IDX			18
#define MNWIN_TITLE_IDX						19
#define MNWIN_BORDERGADGETS_IDX		20
/* bar */
#define MNWIN_VERTICAL_IDX				22

/* SETTINGS MENU */

#define MNSET_SAVECHUNKY			0
#define MNSET_MAKEBACKUPS			1
#define MNSET_AUTOACTIVATE		2
#define MNSET_KEEPSCREENMODE	3
#define MNSET_EXCLUDEBRUSH		4
#define MNSET_WINDOWIDS				5
#define MNSET_CREATEICONS			6
/* bar */
#define MNSET_SCREENSETTINGS	8
/* bar */
#define MNSET_LOADSETTINGS		10
#define MNSET_SAVESETTINGS		11
#define MNSET_SAVESETTINGSAS	12

#define MNSET_SAVECHUNKY_IDX			1
#define MNSET_MAKEBACKUPS_IDX			2
#define MNSET_AUTOACTIVATE_IDX		3
#define MNSET_KEEPSCREENMODE_IDX	4
#define MNSET_EXCLUDEBRUSH_IDX		5
#define MNSET_WINDOWIDS_IDX				6
#define MNSET_CREATEICONS_IDX			7
/* bar */
#define MNSET_SCREENSETTINGS_IDX	9
/* bar */
#define MNSET_LOADSETTINGS_IDX		11
#define MNSET_SAVESETTINGS_IDX		12
#define MNSET_SAVESETTINGSAS_IDX	13

/* Mode Menu */
#define MNMDE_MATTE						0
#define MNMDE_COLOUR					1
#define MNMDE_REPLACE					2
#define MNMDE_SHADE						3
#define MNMDE_SMOOTH					4
#define MNMDE_CYCLE						5
#define MNMDE_DUMMY1					6
#define MNMDE_DUMMY2					7
#define MNMDE_DUMMY3					8

#define MNMDE_MATTE_IDX				1
#define MNMDE_COLOUR_IDX			2
#define MNMDE_REPLACE_IDX			3
#define MNMDE_SHADE_IDX				4
#define MNMDE_SMOOTH_IDX			5
#define MNMDE_CYCLE_IDX				6
#define MNMDE_DUMMY1_IDX			7
#define MNMDE_DUMMY2_IDX			8
#define MNMDE_DUMMY3_IDX			9

/* Brush Menu */
#define MNBRH_ANIMBRUSH				0
	#define MNBRHANIM_LOAD				0
	#define MNBRHANIM_SAVEAS			1
	#define MNBRHANIM_PICKUP			2
	#define MNBRHANIM_SETTINGS		3
	#define MNBRHANIM_USE					4
	#define MNBRHANIM_FREE				5
#define MNBRH_LOAD						1
#define MNBRH_SAVE						2
#define MNBRH_COPY						3
#define MNBRH_PASTE						4
#define MNBRH_RESTORE					5
#define MNBRH_SIZE						6
	#define MNBRHSIZE_STRETCH			0
	#define MNBRHSIZE_HALVE				1
	#define MNBRHSIZE_DOUBLE			2
	#define MNBRHSIZE_DOUBLEHORIZ	3
	#define MNBRHSIZE_DOUBLEVERT	4
#define MNBRH_FLIP						7
	#define MNBRHFLIP_HORIZONTAL	0
	#define MNBRHFLIP_VERICAL			1
	#define MNBRHFLIP_ROTATE90		2
#define MNBRH_OUTLINE					8
#define MNBRH_HANDLE					9
	#define MNBRHHANDLE_ROTATE		0
	#define MNBRHHANDLE_PLACE			1
#define MNBRH_FREE						10

#define MNBRH_ANIMBRUSH_IDX				1
	#define MNBRHANIM_LOAD_IDX				2
	#define MNBRHANIM_SAVEAS_IDX			3
	#define MNBRHANIM_PICKUP_IDX			4
	#define MNBRHANIM_SETTINGS_IDX		5
	#define MNBRHANIM_USE_IDX					6
	#define MNBRHANIM_FREE_IDX			7
#define MNBRH_LOAD_IDX						8
#define MNBRH_SAVE_IDX						9
#define MNBRH_COPY_IDX						10
#define MNBRH_PASTE_IDX						11
#define MNBRH_RESTORE_IDX					12
#define MNBRH_SIZE_IDX						13
	#define MNBRHSIZE_STRETCH_IDX			14
	#define MNBRHSIZE_HALVE_IDX				15
	#define MNBRHSIZE_DOUBLE_IDX			16
	#define MNBRHSIZE_DOUBLEHORIZ_IDX	17
	#define MNBRHSIZE_DOUBLEVERT_IDX	18
#define MNBRH_FLIP_IDX						19
	#define MNBRHFLIP_HORIZONTAL_IDX	20
	#define MNBRHFLIP_VERICAL_IDX			21
	#define MNBRHFLIP_ROTATE90_IDX		22
#define MNBRH_OUTLINE_IDX					23
#define MNBRH_HANDLE_IDX					24
	#define MNBRHHANDLE_ROTATE_IDX		25
	#define MNBRHHANDLE_PLACE_IDX			26
#define MNBRH_FREE_IDX						27

/*****************************************************************/
//
// DATA STRUCTURES
//


/************** Palette structures **************/
//
//
//

struct RGB8
{
	UBYTE	r,g,b;
};

struct Palette
{
	/* number of colours in this palette */
	UWORD								pl_Count;
	/* ptr to RGB data - 8 bits per component, in rgb ubyte triplets */
	/* ie same format as iff CMAP chunk */
	struct RGB8					pl_Colours[256];
};

struct Range
{
	UWORD	rn_Cell[256];	/* 0xFFFF == empty cell */
};

struct PaletteBox
{
	struct Window		*pb_Window;
	struct Palette	*pb_Palette;
	UWORD						 pb_BoxLeft;
	UWORD						 pb_BoxTop;
	UWORD						 pb_BoxWidth;
	UWORD						 pb_BoxHeight;
	UWORD						 pb_ColourWidth;
	UWORD						 pb_ColourHeight;
	UWORD						 pb_ColoursWide;
	UWORD						 pb_ColoursHigh;
};

struct ToolsWindowGadList
{
	struct List	tgl_List;
	struct Image *tgl_GadImages;
	struct Gadget *tgl_Gadgets;
	UBYTE	*tgl_GadMaps;
	WORD	tgl_SelectWidth;
	WORD	tgl_Width;
	WORD	tgl_Height;
	WORD	tgl_Depth;
	UWORD	tgl_BrightColour;
	UWORD	tgl_DarkColour;
};

struct ToolsWindowGadget
{
	struct Node tg_Node;
	WORD	tg_LeftEdge;
	WORD	tg_TopEdge;
	UWORD	tg_Activation;
	UWORD tg_GadgetID;
	UBYTE	*tg_BrightMap1;
	UBYTE	*tg_DarkMap1;
	UBYTE	*tg_BrightMap2;
	UBYTE	*tg_DarkMap2;
};

/********** SuperWindow Base Structure *******/

struct SuperWindow
{
	struct	Node	sw_Node;

	/* handler functions */
	void		(*sw_Handler)(struct SuperWindow *, struct IntuiMessage *);
	void		(*sw_HideWindow)(struct SuperWindow *);
	BOOL		(*sw_ShowWindow)(struct SuperWindow *);
	APTR		(*sw_BuggerOff)(struct SuperWindow *);
	BOOL		(*sw_RefreshMenus)(struct SuperWindow *);

	/* assorted stuff */
	UWORD		sw_Type;
	struct	TextAttr *sw_TextAttr;
	struct	TextFont *sw_TextFont;
	struct	Window	*sw_Window;
	struct	Menu		*sw_MenuStrip;
	struct	Gadget	*sw_GadList;
	APTR						 sw_BlockingHandle;	/* for blocking the window */
	APTR		*sw_VisualInfo;
	UWORD		sw_id;
	char		sw_idstring[6];
	struct	Menu		*sw_Menus[MN_NUMOFMENUS];

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
	BOOL		sw_FlgBackDrop;
	BOOL		sw_FlgBorderless;
	BOOL		sw_FlgTitle;
	BOOL		sw_FlgBorderGadgets;
	UWORD		sw_Flags;					/* See SW?_ defines below */
};
#define SWB_HIDING 0
#define SWF_HIDING 1<<SWB_HIDING
/* _HIDING - Used when the window is just temporarily hidden, ie when we are
             changing the screen. It is not used when the window is just
             normally hidden through the menus */
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
#define SWTY_PALETTE				6
#define SWTY_EDITPALETTE		7
#define SWTY_SEQUENCE				8
#define SWTY_RANGE					9
#define	SWTY_NUM						10	/* num of SuperWindow types */

/* ProjectWindow */

 /* different editing modes */
#define	EDTY_BLOCKS 				0
#define EDTY_BOBS						1
#define EDTY_SPRITES				2
#define EDTY_FRAME					3

struct ProjectWindow
{
	struct SuperWindow		pw_sw;
	struct	Project				*pw_Project;
	struct	ProjectWindow	*pw_NextProjWindow;
	//struct	Gadget				pw_PropGadget;
	//struct	PropInfo			pw_PropInfo;
	//struct	Image					pw_PropImage;
	WORD		pw_MagnifyLevel;		/* 1(small pixels) to 24(large pixels) */
	WORD		pw_PixelSize;
	BOOL		pw_Magnified;
		/* X & Y Offsets to the main bitmap. */
	WORD		pw_XOffset;
	WORD		pw_YOffset;
		/* How many Magnified pixels fit into window */
	WORD		pw_MagWidth;
	WORD		pw_MagHeight;
	UWORD		pw_SelType;				/* The Tool used to draw the selector */
	BOOL		pw_SelState;			/* TRUE/FALSE, as to wheather the selector is shown or not */
	UWORD		pw_SelToolState;	/* Mouse buttons used for tool */
	WORD		pw_SelX0;					/* X Offset of cursor */
	WORD		pw_SelY0;					/* Y Offset of cursor */
	WORD		pw_SelX1;					/* Used as Anchor points of lines, and boxes... */
	WORD		pw_SelY1;
	WORD		pw_DamageX0;			/* These are the selector damage areas. */
	WORD		pw_DamageY0;
	WORD		pw_DamageX1;
	WORD		pw_DamageY1;
	WORD		pw_MagnifyBoxWidth;
	WORD		pw_MagnifyBoxHeight;
	WORD		pw_PrevX;					/* Position from previous move event, stops the cursor */
	WORD		pw_PrevY;					/* being drawn multiple times at the same place. */
	struct Gadget							pw_GadList[PG_NUMOFGADGETS];
	struct Image							pw_GadImages[PG_NUMOFGADGETS*2];
	struct ToolsWindowGadList	pw_ToolsWindowGadList;
	struct ToolsWindowGadget	pw_ToolsWindowGadget[PG_NUMOFGADGETS];
	APTR		pw_GadMaps;				/* Pointer to MEMF_CHIP chunk. For the Images to be remaped into */
	//UWORD		pw_SelStage;
	WORD		pw_MouseX;
	WORD		pw_MouseY;
	WORD		pw_BMMouseX;
	WORD		pw_BMMouseY;
	struct	Gadget		*pw_GTGadList;
	struct	Gadget		*pw_GTGadgets[1];
	UWORD		pw_TextGadgetHeight;
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
	UWORD								rw_NowShowing;
	char								rw_TitleString[14];
	struct List					rw_Labels;
	struct Gadget				*rw_LVGadget;
	UWORD								rw_CurrentSelection;
};

/* values for rw_NowShowing */
#define RWNS_WINDOWS	0
#define RWNS_PROJECTS	1

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


struct ToolsWindow
{
	struct SuperWindow				 tw_sw;
	struct Gadget							 tw_GadList[TG_NUMOFGADGETS];
	struct Image							 tw_GadImages[TG_NUMOFGADGETS*2];
	struct ToolsWindowGadList	 tw_ToolsWindowGadList;
	struct ToolsWindowGadget	 tw_ToolsWindowGadget[TG_NUMOFGADGETS];
	void											*tw_GadMaps;
	BOOL											 tw_Vertical;
};

struct PaletteWindow
{
	struct SuperWindow	pw_sw;
	UWORD								pw_MinX1;
	UWORD								pw_MinY1;
	UWORD								pw_MaxX1;
	UWORD								pw_MaxY1;
	UWORD								pw_MinX2;
	UWORD								pw_MinY2;
	UWORD								pw_MaxX2;
	UWORD								pw_MaxY2;
	UWORD								pw_MinX3;
	UWORD								pw_MinY3;
	UWORD								pw_MaxX3;
	UWORD								pw_MaxY3;
	struct PaletteBox		pw_PaletteBox;
};


#define EPW_NUMOFGADGETS 11
struct EditPaletteWindow
{
	struct SuperWindow				 epw_sw;
	struct EditPaletteWindow	*epw_NextEditPaletteWindow;
	struct Project						*epw_Project;
	struct Gadget							*epw_Gadgets[EPW_NUMOFGADGETS];
	struct Palette						*epw_Palette;
	struct Palette						 epw_RevertPalette;
	UWORD											 epw_CBoxLeft;						/* Colour Box Stuff (Displays the current colour) */
	UWORD											 epw_CBoxTop;
	UWORD											 epw_CBoxWidth;
	UWORD											 epw_CBoxHeight;
	struct PaletteBox					 epw_PaletteBox;
	UWORD										 	 epw_CurrentColour;
	BOOL											 epw_WaitingForNextColour;
	void	(*epw_ColourPick)(struct EditPaletteWindow *epw, UWORD col);
	BOOL											 epw_256Scale;
	BOOL											 epw_HexReadOut;
};

#define RW_NUMOFGADGETS 8
struct RangeWindow
{
	struct SuperWindow	 rw_sw;
	struct Project 			*rw_Project;
	struct RangeWindow	*rw_NextRangeWindow;
	struct Range				 rw_RevertRange[8];
	struct Range				 rw_UndoRange;
	struct Range				*rw_CurrentRange;
	UWORD								 rw_CurrentRangeNumber;
	UWORD								 rw_FirstRangeCell;						/* First Displayed Cell in range box */
	struct Gadget				*rw_Gadgets[RW_NUMOFGADGETS];
	UWORD								 rw_CurrentColour;						/* Currently Display Colour 0xFFFF = No Colour */
	UWORD								 rw_SelectedColours[256];			/* List of Selected Colours. 0xFFFF = End Of List */
	UWORD								 rw_CBoxLeft;						/* Colour Box Stuff (Displays the current colour) */
	UWORD								 rw_CBoxTop;
	UWORD								 rw_CBoxWidth;
	UWORD								 rw_CBoxHeight;
	struct PaletteBox		 rw_PaletteBox;
	UWORD								 rw_RangeBoxLeft;				/* Range Box Stuff */
	UWORD								 rw_RangeBoxTop;
	UWORD								 rw_RangeBoxWidth;
	UWORD								 rw_RangeBoxHeight;
	UWORD								 rw_RangeColourWidth;
	UWORD								 rw_RangeColourHeight;
	UWORD							 	 rw_RangeColoursWide;
	BOOL							   rw_ButtonDown;					/* TRUE = LMB Down */
	UWORD							 	 rw_DragAnchorColour;
	UWORD								 rw_DragLastColour;
};

struct Frame
{
	struct	Node				 fr_Node;
	struct	MyBob				*fr_Bob;
	WORD								 fr_X;
	WORD								 fr_Y;
	UWORD								 fr_Number;
};

struct Anim
{
	struct	Node	an_Node;
	struct	List	an_Frames;
	UBYTE					an_Name[ANIMNAMESIZE];
	UWORD					an_Number;
	UWORD					an_Speed;
};

struct Sequence
{
	struct Project			*sq_Project;
	struct SeqWindow		*sq_SeqWindows;
	struct List					 sq_Anims;
	BOOL								 sq_ModifiedFlag;
};

#define SEQ_NUMOFGADGETS 26
struct SeqWindow
{
	struct SuperWindow	 asw_sw;
	struct Sequence			*asw_Sequence;
	struct SeqWindow		*asw_NextSeqWindow;
	struct Gadget				*asw_Gadgets[SEQ_NUMOFGADGETS];
	struct MyBob				*asw_ViewBob;
	struct Anim					*asw_CurrentAnim;
	struct Frame				*asw_CurrentFrame;
	UWORD								 asw_BobBoxLeft;
	UWORD								 asw_BobBoxTop;
	UWORD								 asw_BobBoxWidth;
	UWORD								 asw_BobBoxHeight;
	UWORD								 asw_AnimBoxLeft;
	UWORD								 asw_AnimBoxTop;
	UWORD								 asw_AnimBoxWidth;
	UWORD								 asw_AnimBoxHeight;
	WORD								 asw_BobLeft;
	WORD								 asw_BobTop;
	WORD								 asw_BobWidth;
	WORD								 asw_BobHeight;
	WORD								 asw_FrameLeft;
	WORD								 asw_FrameTop;
	WORD								 asw_FrameWidth;
	WORD								 asw_FrameHeight;
	BOOL								 asw_ButtonDown;
	WORD								 asw_HandleLeft;
	WORD								 asw_HandleTop;
	BOOL								 asw_RelativePositions;
};

struct CrapChunk
{
	struct Node		 cc_Node;
	ULONG					 cc_ChunkID;
	ULONG					 cc_ChunkSize;
	void					*cc_ChunkData;
};
/************** File Structure *****************/
//
struct File
{
	char							*fl_Name;
	char							*fl_Drawer;
	struct List				 fl_CrapChunks;
	struct Project		*fl_FirstProject;
};

/************** Project Structure **************/
//
//
struct Project
{
	struct	Node							 pj_Node;						/* File Holds Name */
	struct	ProjectWindow			*pj_ProjectWindows;	/* SuperWindows viewing this Project */
	BOOL											 pj_ModifiedFlag;		/* has this project been changed? */
	struct File								*pj_File;
	struct Project						*pj_NextProjectInFile;	/* Next Project attached to This File */

	/* ADD YOUR OWN (APP-SPECIFIC) FIELDS IN HERE */
	struct	RastPort					*pj_MainRPort;
	struct  BitMap						*pj_MainBitMap;
	struct	BitMap						*pj_UndoBitMap;
	struct	BitMap						*pj_MaskBitMap;
	struct	Palette						 pj_Palette;
	UWORD											 pj_Width;
	UWORD											 pj_Height;
	UWORD											 pj_Depth;
	BOOL											 pj_IsFile;
	BOOL											 pj_Disabled;
	BOOL											 pj_Edable;
	ULONG											 pj_DisplayID;
	WORD											 pj_UndoTop;
	WORD											 pj_UndoLeft;
	WORD											 pj_UndoBottom;
	WORD											 pj_UndoRight;
	UWORD											 pj_EditType;
	UWORD											 pj_CutBobBoxWidth;
	UWORD											 pj_CutBobBoxHeight;
	struct	List							 pj_BobList;
	struct	MyBob 						*pj_CurrentBob;
	BOOL											 pj_CurrentBobModified;
	BOOL											 pj_EditBobMask;
	BOOL											 pj_BobHasMask;
	struct	Sequence					*pj_Sequence;
	struct	Range							*pj_CurrentRange;
	UWORD											 pj_CurrentRangeNumber;
	struct	Range							 pj_Ranges[8];
	struct	RangeWindow				*pj_RangeWindows;
	UWORD											 pj_CycleCellNumber;
	struct EditPaletteWindow	*pj_EditPaletteWindows;
	char											 pj_BankName[16];
};

/*					Project explanation:

IsFile:				If False then our project isn't an actual file, but just
							something that we've made up from scratch, ie Untitled.
Disabled:			A project can be disabled if you can't edit it properly. At this stage
							it only gets disabled if the depth is wrong for the screen
Edable:				If we are unable to edit the file, for one reason or another, then this
							flag should be FALSE. The only reason I can think of right now for not
							being able to edit a file is if it's HAM.
CurrentBobModified: TRUE if the current bob has been modified since last storing
							it out to a bob structure.
EditBobMask:	TRUE if we are in bob mask editing mode.
BobHasMask:		TRUE if the current bob has a mask. (This will be in pj_MaskBitMap)
*/

/* DataNode used to form lists of objects */
/* great for temporary stuff */

struct DataNode
{
	struct Node					dn_Node;
	APTR								dn_Ptr;
};

struct ChunkNode
{
	struct Node					cn_Node;
	LONG								cn_ID;
	LONG								cn_Type;
	ULONG								cn_Size;
};



/************** WindowDefault structure **************/

struct WindowDefault
{
	WORD		wf_Left;
	WORD		wf_Top;
	WORD		wf_Width;
	WORD		wf_Height;
	UWORD		wf_SWType;
	BOOL		wf_FlgBackDrop;
	BOOL		wf_FlgBorderless;
	BOOL		wf_FlgTitle;
	BOOL		wf_FlgBorderGadgets;
	UWORD		wf_Flags;								/* See Defines Below */
};
/*  Attached Bit, used for projects that have mutiple windows on it. If bit is set
 * this window is attached to the previous project */
#define WF_PROJECTTYPEMASK 0x07
#define WFB_ATTACHED	3
#define WFF_ATTACHED	1<<WFB_ATTACHED
#define WFB_TOOLSWINDOWVERTICAL 4
#define WFF_TOOLSWINDOWVERTICAL 1<<WFB_TOOLSWINDOWVERTICAL

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
	/* Settings Menu Stuff */
	BOOL									cg_Dummy1;
	BOOL									cg_SaveChunky;
	BOOL									cg_MakeBackups;
	BOOL									cg_AutoActivate;
	BOOL									cg_ExcludeBrush;
	BOOL									cg_ShowWindowID;
	BOOL									cg_CreateIcons;
	BOOL									cg_KeepScreenMode;

	struct ScreenConfig		cg_ScrConfig;

	struct UserMenuEntry	cg_UserMenu[20];

	struct WindowDefault	cg_WindowDefaults[SWTY_NUM];
	UWORD									cg_NumOfOpenWindows;
	struct WindowDefault	cg_OpenWindowDefinitions[0];		/* times cg_NumOfOpenWindows */

/*UWORD									cg_NumOfColours;
	struct RGB8						cg_Colours[cg_NumOfColours]; */
};

struct MyIMStruct				/* Image Mapping Structure */
{
	APTR		mim_Dest;
	APTR		mim_Source1;
	APTR		mim_Source2;
	UWORD		mim_Colour1;
	UWORD		mim_Colour2;
	UWORD		mim_Width;
	UWORD		mim_Height;
	UWORD		mim_Depth;
};

struct MyILBMStruct
{
	struct IFFHandle			*mis_Handle;
	UWORD									 mis_Error;
	UWORD									 mis_BitMapWidth;
	UWORD									 mis_BitMapHeight;
	/* Useful properties here */
	struct StoredProperty	*mis_BMHD;
	struct StoredProperty	*mis_CMAP;
	struct StoredProperty	*mis_CAMG;
	struct StoredProperty	*mis_GRAB;
	struct CollectionItem	*mis_CRNG;
	struct CollectionItem	*mis_DRNG;
	struct ContextNode		*mis_BODY;
};

struct Magnify
{
	APTR		mg_SrcBitMap;
	UWORD		mg_SrcXOffset;
	UWORD		mg_SrcYOffset;
	APTR		mg_DestRastPort;
	UWORD		mg_DestXOffset;
	UWORD		mg_DestYOffset;
	UWORD		mg_DestWidth;
	UWORD		mg_DestHeight;
	UWORD		mg_DestPixelSize;
	UWORD		mg_ClearRight;
	UWORD		mg_ClearBottom;
	UWORD		mg_Scale;
	APTR		mg_ScaleBitMap;
};

struct QuickMagnify
{
	APTR		qm_SrcBitMap;
	UWORD		qm_SrcXOffset;
	UWORD		qm_SrcYOffset;
	UWORD		qm_SrcWidth;
	UWORD		qm_SrcHeight;
	APTR		qm_DestBitMap;
	UWORD		qm_DestXOffset;
	UWORD		qm_DestYOffset;
	UWORD		qm_DestPixelSize;
};

struct BrushInfo
{
	WORD	bi_Width;
	WORD	bi_Height;
	WORD	bi_Depth;
	WORD	bi_XHandle;
	WORD	bi_YHandle;
	struct BitMap	*bi_Image;
	struct BitMap *bi_Mask;
};

struct Brush
{
	UWORD	br_ID;
	struct BrushInfo br_InitialBrush;
	struct BrushInfo br_DrawBrush;
};

struct ABrush
{
	struct Node			 ab_Node;
	struct BitMap		*ab_Image;
	struct BitMap		*ab_Mask;
};

struct AnimBrush
{
	struct List				 ab_ABrushList;	/* List of ABrush's */
	struct ABrush			*ab_CurrentABrush;
	UWORD							 ab_Width;
	UWORD							 ab_Height;
	UWORD							 ab_Depth;
	UWORD							 ab_Direction; /* See defines Below */
	UWORD							 ab_PingPongDirection; /* Same As Above */
};
#define ABDIR_FORWARD 0
#define ABDIR_REVERSE 1
#define ABDIR_PINGPONG 2

/* Paul's Bob Image Structure */
struct PaulsBobImage
{
	ULONG		im_Mask;
	ULONG		im_Image;
	UWORD		im_DMod;
	UWORD		im_Size;
	LONG		im_ColLeftPos;
	LONG		im_ColRightPos;
	LONG		im_ColUpPos;
	LONG		im_ColDownPos;
	ULONG		im_ColWidth;
	ULONG		im_ColHeight;
};	/* 36 bytes, NOTE if this grows so must the BobHeader structure. */

/* My Bob Struct
	 -------------
	This structure is used for Bobs and Sprites so there is a little bit of sprite/bob
	specific stuff, but not much.
	 Note: The Collision Zones are all offset from the top left (0,0) of the main bitmap
	so to get the correct zone for the bob, do this:
	realleft =  bb_ColLeft - bb_Left
*/
struct MyBob
{
	struct	Node		bb_Node;
	UWORD						bb_Number;
	WORD						bb_Top, bb_Left;
	WORD						bb_Width, bb_Height, bb_Depth;
	WORD						bb_ColLeft, bb_ColRight;
	WORD						bb_ColTop, bb_ColBottom;
	UWORD						bb_Flags;														/* See defines below */
	UWORD						bb_SpriteNumber;										/* Only used for sprites */
	struct	BitMap *bb_Image;
	struct	BitMap *bb_Mask;														/* Only used for Bobs */
	UBYTE		bb_Name[BOBNAMESIZE];
};
#define MBB_COLLISIONZONESET	0
#define MBF_COLLISIONZONESET	1<<MBB_COLLISIONZONESET

struct BobBankFileHeader
{
	UBYTE	bf_BankName[16];
	UWORD	bf_NumberOfBobs;
	UBYTE bf_Depth;
	UBYTE bf_Pad;
	UWORD bf_Flags;						/* See BBFH?_* Defines Below */
	UWORD	bf_Reserved[4];
};
#define BBFHB_CHUNKY	0
#define BBFHF_CHUNKY	1<<BBFHB_CHUNKY

/* These collision zones are relative to the bob, unlike the MyBob Collision Zones */
struct BobFileHeader
{
	WORD bf_X;
	WORD bf_Y;
	WORD bf_Width;
	WORD bf_Height;
	UWORD bf_Flags;				/* See BFH?_* Defines Below */
	WORD bf_ColLeft;
	WORD bf_ColTop;
	WORD bf_ColWidth;
	WORD bf_ColHeight;
	UBYTE	bf_Name[BOBFILENAMESIZE];
	UWORD bf_Reserved[1];
	/* Image Data (((bf_Width+15)/8) & 0xFFFE)*bf_Height*bf_Depth */
	/* Mask Data same size as image data */
};
#define BFHB_HASMASK 0
#define BFHF_HASMASK 1<<BFHB_HASMASK
#define BFHB_NOREALIMAGE 1
#define BFHF_NOREALIMAGE 1<<BFHB_NOREALIMAGE
#define BFHB_COLLISIONZONESET 2
#define BFHF_COLLISIONZONESET 1<<BFHB_COLLISIONZONESET

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
	UWORD sfh_ColLeft;
	UWORD sfh_ColTop;
	UWORD sfh_ColWidth;
	UWORD sfh_ColHeight;
	UBYTE	sfh_Name[BOBFILENAMESIZE];
	UWORD sfh_Reserved[1];
	/* Image Data (((sf_Width+15)/8) & 0xFFFE)*sf_Height*sf_Depth */
};

#include <protos.h>
