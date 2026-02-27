#include <stdio.h>
//#include <stdarg.h>
#include <string.h>
//#include <dos/dos.h>
//#include <dos/dostags.h>
#include <exec/exec.h>
//#include <intuition/intuition.h>
//#include <graphics/gfxbase.h>
//#include <rexx/rxslib.h>
//#include <libraries/gadtools.h>
//#include <libraries/diskfont.h>
//#include <libraries/asl.h>

#include <proto/exec.h>
//#include <proto/dos.h>
#include <proto/intuition.h>
#include <proto/graphics.h>
//#include <proto/diskfont.h>
//#include <proto/asl.h>
//#include <clib/alib_protos.h>
#include <proto/gadtools.h>

#include <global.h>
#include <ExternVars.h>

/* menus */

static struct NewMenu	editpalettemenu[] =
	{
		{ NM_TITLE,	"Project",							0,	0,	0,	0,},
		{  NM_ITEM,	"New",								 "N",	0,	0,	0,},
		{  NM_ITEM,	"Open...",						 "O",	0,	0,	0,},
		{	 NM_ITEM,	NM_BARLABEL,						0,	0,	0,	0,},
		{  NM_ITEM,	"Save",								 "S",	NM_ITEMDISABLED,	0,	0,},
		{  NM_ITEM,	"Save As...",					 "A",	NM_ITEMDISABLED,	0,	0,},
		{	 NM_ITEM,	NM_BARLABEL,						0,	0,	0,	0,},
		{  NM_ITEM,	"Hide",									0,	NM_ITEMDISABLED,	0,	0,},
		{  NM_ITEM,	"Reveal...",						0,	0,	0,	0,},
		{  NM_ITEM,	"Close",								0,	NM_ITEMDISABLED,	0,	0,},
		{	 NM_ITEM,	NM_BARLABEL,						0,	0,	0,	0,},
		{  NM_ITEM,	"Page Size...",					0,	NM_ITEMDISABLED,	0,	0,},
		{  NM_ITEM,	"Screen Mode...",				0,	0,	0,	0,},
		{	 NM_ITEM,	NM_BARLABEL,						0,	0,	0,	0,},
		{  NM_ITEM,	"Iconify...",						0,	0,	0,	0,},
		{  NM_ITEM,	"About...",							0,	0,	0,	0,},
		{  NM_ITEM,	"Quit Program...",		 "Q",	0,	0,	0,},

/* 22 */
		{ NM_TITLE,	"Edit",									0,	NM_MENUDISABLED,	0,	0,},
		{  NM_ITEM,	"Colour",								0,	0,	0,	0,},
		{  	NM_SUB,	"Use Palette",				 "P",	NM_MENUDISABLED,	0,	0,},
		{  	NM_SUB,	"Remap",								0,	NM_MENUDISABLED,	0,	0,},
		{  	NM_SUB,	"Load Palette...",			0,	0,	0,	0,},
		{  	NM_SUB,	"Save Palette...",			0,	0,	0,	0,},
		{  NM_ITEM,	"Undo",								 "Z",	0,	0,	0,},
		{  NM_ITEM,	"Name Chunk...",				0,	0,	0,	0,},
		{	 NM_ITEM,	NM_BARLABEL,						0,	0,	0,	0,},
		{  NM_ITEM,	"Blocks",								0,	0,	0,	0,},
		{  	NM_SUB,	"Dummy",								0,	0,	0,	0,},
		{  	NM_SUB,	"Dummy",								0,	0,	0,	0,},
		{  NM_ITEM,	"Bobs",									0,	0,	0,	0,},
		{  	NM_SUB,	"Previous",						 "1",	0,	0,	0,},
		{  	NM_SUB,	"Next",								 "2",	0,	0,	0,},
		{  	NM_SUB,	"Goto...",						 "3",	0,	0,	0,},
		{  	NM_SUB,	"Play...",						 "4",	0,	0,	0,},
		{  	NM_SUB,	"Ping Pong...",				 "6",	0,	0,	0,},
		{	  NM_SUB,	NM_BARLABEL,						0,	0,	0,	0,},
		{  	NM_SUB,	"Insert...",						0,	0,	0,	0,},
		{  	NM_SUB,	"Delete...",						0,	0,	0,	0,},
		{  	NM_SUB,	"Copy...",							0,	0,	0,	0,},
		{	  NM_SUB,	NM_BARLABEL,						0,	0,	0,	0,},
		{  	NM_SUB,	"Edit Mask?",					 "`",	CHECKIT|MENUTOGGLE,	0,	0,},
		{  	NM_SUB,	"Delete Mask",					0,	0,	0,	0,},
		{  	NM_SUB,	"Remake Mask",					0,	0,	0,	0,},
		{	  NM_SUB,	NM_BARLABEL,						0,	0,	0,	0,},
		{  	NM_SUB,	"Edit Zone",						0,	0,	0,	0,},
		{  	NM_SUB,	"Delete Zone",					0,	0,	0,	0,},
		{  	NM_SUB,	"Remake Zone",					0,	0,	0,	0,},
		{  NM_ITEM,	"Sprites",							0,	0,	0,	0,},
		{  	NM_SUB,	"Previous",						 "1",	0,	0,	0,},
		{  	NM_SUB,	"Next",								 "2",	0,	0,	0,},
		{  	NM_SUB,	"Goto...",						 "3",	0,	0,	0,},
		{  	NM_SUB,	"Play...",						 "4",	0,	0,	0,},
		{  	NM_SUB,	"Ping Pong...",				 "6",	0,	0,	0,},
		{	  NM_SUB,	NM_BARLABEL,						0,	0,	0,	0,},
		{  	NM_SUB,	"Insert...",						0,	0,	0,	0,},
		{  	NM_SUB,	"Delete...",						0,	0,	0,	0,},
		{  	NM_SUB,	"Copy...",							0,	0,	0,	0,},
		{	  NM_SUB,	NM_BARLABEL,						0,	0,	0,	0,},
		{  	NM_SUB,	"Set Number...",				0,	0,	0,	0,},
		{	  NM_SUB,	NM_BARLABEL,						0,	0,	0,	0,},
		{  	NM_SUB,	"Edit Zone",						0,	0,	0,	0,},
		{  	NM_SUB,	"Delete Zone",					0,	0,	0,	0,},
		{  	NM_SUB,	"Remake Zone",					0,	0,	0,	0,},
		{  NM_ITEM,	"ILBM",									0,	0,	0,	0,},
		{  	NM_SUB,	"Dummy",								0,	0,	0,	0,},
		{  	NM_SUB,	"Dummy",								0,	0,	0,	0,},
		{  NM_ITEM,	NM_BARLABEL,						0,	0,	0,	0,},
		{  NM_ITEM,	"Blocks",								0,	CHECKIT|MENUTOGGLE,	0,	0,},
		{  NM_ITEM,	"Bobs",									0,	CHECKIT|MENUTOGGLE,	0,	0,},
		{  NM_ITEM,	"Sprites",							0,	CHECKIT|MENUTOGGLE,	0,	0,},
		{  NM_ITEM,	"ILBM",									0,	CHECKIT|MENUTOGGLE,	0,	0,},

/* 67 */
		{ NM_TITLE,	"Window",								0,	0,	0,	0,},
		{  NM_ITEM,	"Open",									0,	0,	0,	0,},
		{   NM_SUB,	"Project...",					 "W",	0,	0,	0,},
		{   NM_SUB,	"About...",						 	0,	0,	0,	0,},
		{   NM_SUB,	"Reveal...",						0,	0,	0,	0,},
		{   NM_SUB,	"ScreenConfig...",			0,	0,	0,	0,},
		{   NM_SUB,	"Tools...",							0,	0,	0,	0,},
		{   NM_SUB,	"UserConfig...",				0,	0,	0,	0,},
		{   NM_SUB,	"Palette...",						0,	0,	0,	0,},
		{   NM_SUB,	"EditPalette...",				0,	0,	0,	0,},
		{   NM_SUB,	"Sequencer...",					0,	NM_ITEMDISABLED,	0,	0,},
		{   NM_SUB,	"Range...",							0,	NM_ITEMDISABLED,	0,	0,},
		{  NM_ITEM,	"Hide",									0,	0,	0,	0,},
		{  NM_ITEM,	"Reveal...",						0,	0,	0,	0,},
		{  NM_ITEM,	"Close",								0,	0,	0,	0,},
		{  NM_ITEM,	"Make Default",					0,	0,	0,	0,},
		{  NM_ITEM,	NM_BARLABEL,						0,	0,	0,	0,},
		{  NM_ITEM,	"Backdrop?",						0,	CHECKIT|MENUTOGGLE,	0,	0,},
		{  NM_ITEM,	"Borderless?",					0,	CHECKIT|MENUTOGGLE,	0,	0,},
		{  NM_ITEM,	"Title?",								0,	CHECKIT|MENUTOGGLE,	0,	0,},
		{  NM_ITEM,	"BorderGadgets?",				0,	CHECKIT|MENUTOGGLE,	0,	0,},
		{  NM_ITEM,	NM_BARLABEL,						0,	0,	0,	0,},
		{  NM_ITEM,	"Vertical?",						0,	CHECKIT|MENUTOGGLE|NM_ITEMDISABLED,	0,	0,},

/* 90 */
		{ NM_TITLE,	"Prefs",								0,	0,	0,	0,},
		{  NM_ITEM, "Save Chunky?",					0,	CHECKIT|MENUTOGGLE,	0,	0,},
		{  NM_ITEM, "Make Backups?",				0,	CHECKIT|MENUTOGGLE,	0,	0,},
		{  NM_ITEM, "Auto Activate?",				0,	CHECKIT|MENUTOGGLE,	0,	0,},
		{  NM_ITEM, "Keep Screen Mode?",		0,	CHECKIT|MENUTOGGLE,	0,	0,},
		{  NM_ITEM, "Exclude Brush?",				0,	CHECKIT|MENUTOGGLE,	0,	0,},
		{  NM_ITEM, "Show Window IDs?",			0,	CHECKIT|MENUTOGGLE,	0,	0,},
		{  NM_ITEM, "Create Icons?",				0,	CHECKIT|MENUTOGGLE,	0,	0,},
		{  NM_ITEM,	NM_BARLABEL,						0,	0,	0,	0,},
		{  NM_ITEM,	"Screen Settings...",		0,	0,	0,	0,},
		{  NM_ITEM,	NM_BARLABEL,						0,	0,	0,	0,},
		{  NM_ITEM,	"Load Settings...",			0,	0,	0,	0,},
		{  NM_ITEM,	"Save Settings",				0,	0,	0,	0,},
		{  NM_ITEM,	"Save Settings As...",	0,	0,	0,	0,},

/* 104 */
		{ NM_TITLE,	"Mode",									0,	0,	0,	0,},
		{  NM_ITEM, "Matte",								0,	CHECKIT|MENUTOGGLE,	0,	0,},
		{  NM_ITEM, "Colour",								0,	CHECKIT|MENUTOGGLE,	0,	0,},
		{  NM_ITEM, "Replace",							0,	CHECKIT|MENUTOGGLE,	0,	0,},
		{  NM_ITEM, "Shade",								0,	CHECKIT|MENUTOGGLE,	0,	0,},
		{  NM_ITEM, "Smooth",								0,	CHECKIT|MENUTOGGLE,	0,	0,},
		{  NM_ITEM, "Cycle",								0,	CHECKIT|MENUTOGGLE,	0,	0,},
		{  NM_ITEM, "Dummy",								0,	CHECKIT|MENUTOGGLE,	0,	0,},
		{  NM_ITEM, "Dummy",								0,	CHECKIT|MENUTOGGLE,	0,	0,},
		{  NM_ITEM, "Dummy",								0,	CHECKIT|MENUTOGGLE,	0,	0,},

/* 114 */
		{ NM_TITLE,	"Brush",								0,	0,	0,	0,},
		{  NM_ITEM, "AnimBrush",						0,	0,	0,	0,},
		{   NM_SUB,	"Load...",							0,	0,	0,	0,},
		{   NM_SUB,	"Save As...",						0,	0,	0,	0,},
		{   NM_SUB,	"Pick Up...",						0,	0,	0,	0,},
		{   NM_SUB,	"Settings...",					0,	0,	0,	0,},
		{   NM_SUB,	"Use",									0,	0,	0,	0,},
		{   NM_SUB,	"Free",									0,	0,	0,	0,},
		{  NM_ITEM, "Load...",							0,	0,	0,	0,},
		{  NM_ITEM, "Save...",							0,	0,	0,	0,},
		{  NM_ITEM, "Copy",								 "C",	0,	0,	0,},
		{  NM_ITEM, "Paste",							 "V",	0,	0,	0,},
		{  NM_ITEM, "Restore",						 "B",	0,	0,	0,},
		{  NM_ITEM, "Size",									0,	0,	0,	0,},
		{   NM_SUB,	"Stretch",							0,	0,	0,	0,},
		{   NM_SUB,	"Halve",								0,	0,	0,	0,},
		{   NM_SUB,	"Double",								0,	0,	0,	0,},
		{   NM_SUB,	"Double Horiz",					0,	0,	0,	0,},
		{   NM_SUB,	"Double Vert",					0,	0,	0,	0,},
		{  NM_ITEM, "Flip",									0,	0,	0,	0,},
		{   NM_SUB,	"Horizontal",						0,	0,	0,	0,},
		{   NM_SUB,	"Vertical",							0,	0,	0,	0,},
		{   NM_SUB,	"Rotate 90°",						0,	0,	0,	0,},
		{  NM_ITEM, "Outline",						 "o",	0,	0,	0,},
		{  NM_ITEM, "Handle",								0,	0,	0,	0,},
		{   NM_SUB,	"Rotate",								0,	0,	0,	0,},
		{   NM_SUB,	"Place",								0,	0,	0,	0,},
		{  NM_ITEM, "Free",									0,	0,	0,	0,},

/* 142 */
		{ NM_END,		NULL,										0,	0,	0,	0,},
		{ NM_END,		NULL,										0,	0,	0,	0,},

		{ NM_END,		NULL,										0,	0,	0,	0,},
		{ NM_END,		NULL,										0,	0,	0,	0,},
		{ NM_END,		NULL,										0,	0,	0,	0,},
		{ NM_END,		NULL,										0,	0,	0,	0,},
		{ NM_END,		NULL,										0,	0,	0,	0,},
		{ NM_END,		NULL,										0,	0,	0,	0,},
		{ NM_END,		NULL,										0,	0,	0,	0,},
		{ NM_END,		NULL,										0,	0,	0,	0,},
		{ NM_END,		NULL,										0,	0,	0,	0,},
		{ NM_END,		NULL,										0,	0,	0,	0,},
		{ NM_END,		NULL,										0,	0,	0,	0,},
		{ NM_END,		NULL,										0,	0,	0,	0,},
		{ NM_END,		NULL,										0,	0,	0,	0,},
		{ NM_END,		NULL,										0,	0,	0,	0,},
		{ NM_END,		NULL,										0,	0,	0,	0,},
		{ NM_END,		NULL,										0,	0,	0,	0,},
		{ NM_END,		NULL,										0,	0,	0,	0,},
		{ NM_END,		NULL,										0,	0,	0,	0,},
		{ NM_END,		NULL,										0,	0,	0,	0,},
		{ NM_END,		NULL,										0,	0,	0,	0,},
	};

#define MN_PROJECT					0
#define MN_EDIT							1
#define MN_WINDOW						2
#define MN_SETTINGS					3
#define MN_MODE							4
#define MN_BRUSH						5
#define MN_USER							6

#define MN_EDIT_IDX					17
#define MN_WINDOW_IDX				71
#define MN_SETTINGS_IDX			94
#define MN_MODE_IDX					108
#define MN_BRUSH_IDX				116
#define MN_USER_IDX					146

#define MYGT_REDSLIDER	0
#define MYGT_GREENSLIDER	1
#define MYGT_BLUESLIDER	2
#define MYGT_AGACYCLE	3
#define MYGT_COPYBUTTON 4
#define MYGT_EXCHANGEBUTTON 5
#define MYGT_SPREADBUTTON 6
#define MYGT_OKBUTTON 7
#define MYGT_CANCELBUTTON 8
#define MYGT_COLOURNUMBER 9
#define MYGT_HEXCHECKBOX 10
/* REMEMBER:  Change EPW_NUMOFGADGETS */
/* Dummy GadgetID for RefreshGadget() */
#define MYGT_COLOURSLIDERS 11

/* labels for the Windows/Projects selector (a cycle gadget) */

static UBYTE *cyclelabels[] = {
	(UBYTE *)"256",
	(UBYTE *)"16",
	NULL };


/************ Prototypes for functions local to this file ************/

BOOL ShowEditPaletteWindow(struct SuperWindow *sw);
BOOL SetupEditPaletteWindowMenus(struct SuperWindow *sw);
void HideEditPaletteWindow(struct SuperWindow *sw);
APTR KillOffEditPaletteWindow(struct SuperWindow *sw);
void EditPaletteWindowHandler(struct SuperWindow *sw,	struct IntuiMessage *imsg);

static void HandleGadgetUp(struct SuperWindow *sw,
													 struct Gadget *gad,
													 UWORD code);
static void SortOutEditPaletteWindowMenus(struct SuperWindow *sw,
																		 struct IntuiMessage *imsg,
																		 UWORD code);
static void SetupEditPaletteGadgets(struct EditPaletteWindow *rw, BOOL clear);
static void DrawColour(struct EditPaletteWindow *epw,UWORD col);
void SussColourPick(struct EditPaletteWindow *epw, WORD mx, WORD my, UWORD button);
static void NewColour(struct EditPaletteWindow *epw, UWORD col);
static void HandleMouseMove(struct EditPaletteWindow *epw, struct Gadget *gad, UWORD code);
static void SetColour(struct Palette *pal, UWORD col);
static void HandleOKGadget(struct EditPaletteWindow *epw);
static void HandleCancelGadget(struct EditPaletteWindow *epw);
void HandleExchangePick(struct EditPaletteWindow *epw, UWORD destcol);
void HandleCopyPick(struct EditPaletteWindow *epw, UWORD destcol);
void HandleSpreadPick(struct EditPaletteWindow *epw, UWORD col);
static void RefreshGadget(struct Project *pj, struct EditPaletteWindow *missepw, UWORD gadgetid);
static void UnlinkEditPaletteWindow(struct EditPaletteWindow *epw);
static struct EditPaletteWindow *GetPrevEditPaletteWindow(struct EditPaletteWindow *epw);


/************ OpenNewEditPaletteWindow ************/
//
// Opens a EditPaletteWindow. The window displays a full palette
// with all the palette editing tools required.
//
// Inputs: col = start col

BOOL OpenNewEditPaletteWindow( struct WindowDefault *wf, struct Project *pj, UWORD col)
{
	struct	EditPaletteWindow	*epw;

	/* grab mem for the EditPaletteWindow struct */
	if ( !( epw= AllocVec(sizeof(struct EditPaletteWindow), MEMF_CLEAR ) ) )
		return(FALSE);

	/* figure out a unique ID for this window (mainly for AREXX use) */
	InstallUniqueWindowID((struct SuperWindow *)epw);
	epw->epw_sw.sw_Node.ln_Name = (char *)&(epw->epw_sw.sw_idstring);

	/* find the Default settings for this type of SuperWindow */
	//wf = &glob.cg_WindowDefaults[SWTY_EDITPALETTE];

	/* set up handler routines and assorted SuperWindow data */
	epw->epw_sw.sw_Handler = EditPaletteWindowHandler;
	epw->epw_sw.sw_HideWindow = HideEditPaletteWindow;
	epw->epw_sw.sw_ShowWindow = ShowEditPaletteWindow;
	epw->epw_sw.sw_BuggerOff = KillOffEditPaletteWindow;
	epw->epw_sw.sw_RefreshMenus = SetupEditPaletteWindowMenus;
	epw->epw_sw.sw_Window = NULL;
	epw->epw_sw.sw_MenuStrip = NULL;
	epw->epw_sw.sw_GadList = NULL;
	epw->epw_sw.sw_Type = SWTY_EDITPALETTE;
	epw->epw_sw.sw_VisualInfo = NULL;

	/* set up stuff for opening the actual intuition window */
	epw->epw_sw.sw_Left = wf->wf_Left;
	epw->epw_sw.sw_Top = wf->wf_Top;
	epw->epw_sw.sw_Width = wf->wf_Width;
	epw->epw_sw.sw_Height = wf->wf_Height;
	epw->epw_sw.sw_MinWidth = 310;					/* minwidth set for topaz8 */
	epw->epw_sw.sw_MaxWidth = 0xFFFF;
	epw->epw_sw.sw_MaxHeight = 0XFFFF;
	epw->epw_sw.sw_MenuStrip = NULL;

	epw->epw_sw.sw_FlgBackDrop = wf->wf_FlgBackDrop;
	epw->epw_sw.sw_FlgBorderless = wf->wf_FlgBorderless;
	epw->epw_sw.sw_FlgTitle = wf->wf_FlgTitle;
	epw->epw_sw.sw_FlgBorderGadgets = wf->wf_FlgBorderGadgets;

	/* setup SuperWindow basename */
	epw->epw_sw.sw_Title = "Edit Palette";

	/* link to a project */
	epw->epw_Project = pj;
	epw->epw_NextEditPaletteWindow = pj->pj_EditPaletteWindows;
	pj->pj_EditPaletteWindows = epw;
	epw->epw_Palette = &pj->pj_Palette;
	memcpy(&epw->epw_RevertPalette, epw->epw_Palette, sizeof(struct Palette));

	epw->epw_CurrentColour = col;
	if( GfxBase->LibNode.lib_Version >= 39 )
		epw->epw_256Scale = TRUE;
	else
		epw->epw_256Scale = FALSE;

	epw->epw_HexReadOut = FALSE;

	epw->epw_PaletteBox.pb_Palette = epw->epw_Palette;

	/* add window into the superwindows list */
	AddTail(&superwindows, &epw->epw_sw.sw_Node);

	/* open the intuition window (if the window doesn't open, */
	/* it is just left hidden - SEP). */
	ShowEditPaletteWindow(&epw->epw_sw);

	return(TRUE);
}



/************ ShowEditPaletteWindow ************/
//
// Displays a RevealWindow, and sets up all its gadgets, lists
// etc...
//

BOOL ShowEditPaletteWindow(struct SuperWindow *sw)
{
	struct EditPaletteWindow *epw;
	UWORD minh;
	ULONG flags;

	epw = (struct EditPaletteWindow *)sw;

	if(!sw->sw_Window)				/* make sure it isn't already open */
	{
		/* Set up the Visual Info for GadTools */
		if (!(epw->epw_sw.sw_VisualInfo = GetVisualInfo(MAINSCREEN,TAG_END)))
			return(FALSE);

		minh = MAINSCREEN->Font->ta_YSize+1 +	MAINSCREEN->WBorTop + ((8+5)*6);

		flags = WFLG_ACTIVATE | WFLG_NEWLOOKMENUS;
		if(sw->sw_FlgTitle)
			flags = flags | WFLG_DRAGBAR | WFLG_DEPTHGADGET | WFLG_CLOSEGADGET;
		if(sw->sw_FlgBorderGadgets)
			flags = flags | WFLG_SIZEGADGET;

		sw->sw_Window = OpenWindowTags(NULL,
											WA_Left,				sw->sw_Left,
											WA_Top,					sw->sw_Top,
											WA_Width,				sw->sw_Width,
											WA_Height,			sw->sw_Height,
											WA_MinWidth,		sw->sw_MinWidth,
											WA_MinHeight,		minh,
											WA_MaxWidth,		sw->sw_MaxWidth,
											WA_MaxHeight,		sw->sw_MaxHeight,
											WA_Flags,				flags,
											WA_ReportMouse,	TRUE,
											WA_Backdrop,		sw->sw_FlgBackDrop,
											WA_Borderless,	sw->sw_FlgBorderless,
											WA_CustomScreen,	MAINSCREEN,
											WA_AutoAdjust,	TRUE,
											WA_Title,				sw->sw_FlgTitle ?
												BuildWindowTitle(sw) : NULL,
											TAG_DONE,				NULL);

		if( !(sw->sw_Window)) return(FALSE);
		((struct EditPaletteWindow *)sw)->epw_PaletteBox.pb_Window = sw->sw_Window;

		SetupEditPaletteGadgets(epw,TRUE);

		/* Tell the window which superwindow it is attached to */
		/* (so we'll know which handler routine to call) */
		sw->sw_Window->UserData = (BYTE *)epw;

		SetupEditPaletteWindowMenus(sw);

		/* we want to use our global message port for IDCMP stuff */
		sw->sw_Window->UserPort = mainmsgport;
		ModifyIDCMP(sw->sw_Window, IDCMP_CLOSEWINDOW | IDCMP_MENUPICK | IDCMP_NEWSIZE |
			IDCMP_REFRESHWINDOW | SLIDERIDCMP | BUTTONIDCMP | CYCLEIDCMP | CHECKBOXIDCMP |
			IDCMP_MOUSEBUTTONS | IDCMP_CHANGEWINDOW | IDCMP_INACTIVEWINDOW | IDCMP_MOUSEMOVE);

		SetSuperWindowScreenTitle(sw);
	}
	return(TRUE);
}


BOOL SetupEditPaletteWindowMenus(struct SuperWindow *sw)
{
	BOOL success = FALSE;
	struct Menu *mn;
	UWORD menucount;

	if (sw->sw_MenuStrip)						/* Clear the menu strip if it exists */
	{
		ClearMenuStrip(sw->sw_Window);
		FreeMenus(sw->sw_MenuStrip);
		sw->sw_MenuStrip = NULL;
	}

	/* Set up some menus */
	SetupWindowMenu( sw, &editpalettemenu[MN_WINDOW_IDX] );
	SetupEditMenu( NULL, &editpalettemenu[MN_EDIT_IDX] );
	SetupSettingsMenu( &editpalettemenu[MN_SETTINGS_IDX] );
	SetupModeMenu( sw, &editpalettemenu[MN_MODE_IDX] );
	LinkInUserMenu( &editpalettemenu[MN_USER_IDX] );

	if( sw->sw_MenuStrip = CreateMenus( editpalettemenu, TAG_END ) )
	{
		if ( LayoutMenus( sw->sw_MenuStrip, sw->sw_VisualInfo, TAG_END) )
		{
			if ( SetMenuStrip( sw->sw_Window, sw->sw_MenuStrip) )
			{
				mn = sw->sw_MenuStrip;
				for(menucount = 0; menucount < MN_NUMOFMENUS; menucount++)
				{
					sw->sw_Menus[menucount] = mn;
					if(mn) mn = mn->NextMenu;
				}
				success = TRUE;
			}
		}
	}

	/* free stuff if any errors occured */
	if( !success && sw->sw_MenuStrip )
	{
		FreeMenus(sw->sw_MenuStrip);
		sw->sw_MenuStrip = NULL;
	}

	return( success );
}


/******************  HideEditPaletteWindow()  ******************/
//
// Hides the given EditPaletteWindow. Frees gadgets and stuff.
//

void HideEditPaletteWindow(struct SuperWindow *sw)
{
	struct EditPaletteWindow *epw;

	epw = (struct EditPaletteWindow *)sw;

	if (sw->sw_Window)												/* make sure it's actually open */
	{
		if( sw->sw_BlockingHandle )
		{
			UnblockWindow( sw->sw_BlockingHandle );
			sw->sw_BlockingHandle = NULL;
		}
		if (sw->sw_MenuStrip)										/* Clear the menu strip if it exists */
		{
			ClearMenuStrip(sw->sw_Window);
			FreeMenus(sw->sw_MenuStrip);
			sw->sw_MenuStrip = NULL;
		}
		RememberWindow(sw);											/* save old window data */

		/* free up string used for window title */
		if(sw->sw_Window->Title)
			FreeVec(sw->sw_Window->Title);

		/* kill the window */
		CloseWindowSafely(sw->sw_Window);				/* close it */
		sw->sw_Window = NULL;										/* indicate that it's gone */
		epw->epw_PaletteBox.pb_Window = NULL;
		FreeGadgets(epw->epw_sw.sw_GadList);
		epw->epw_sw.sw_GadList = NULL;
		if (sw->sw_VisualInfo) FreeVisualInfo(sw->sw_VisualInfo);
		sw->sw_VisualInfo = NULL;
		SetProcessWindowPtr();
	}
}


/******************  KillOffEditPaletteWindow()  ******************/
//
// Shuts down a RevealWindow and removes it from the SuperWindows
// list.
//

APTR KillOffEditPaletteWindow(struct SuperWindow *sw)
{
	struct EditPaletteWindow *epw;

	epw = (struct EditPaletteWindow *)sw;

	/* close/free the superwindow */
	HideEditPaletteWindow(sw);									/* shut down the Intuition window */
	UnlinkEditPaletteWindow(epw);
	Remove(&(sw->sw_Node));											/* remove from superwindows list */
	FreeVec(epw);																/* free RevealWindow struct */
	return(NULL);
}

/******************  EditPaletteWindowHandler  ******************/
//
// Processes IDCMP messages coming into any EditPaletteWindows,
// dispatching appropriate bits of code.
//

void EditPaletteWindowHandler(struct SuperWindow *sw,
	struct IntuiMessage *imsg)
{
	WORD mx,my;
	ULONG	class;
	UWORD	code;
	struct IntuiMessage *gtimsg;
	struct Gadget *gad;

	if( gtimsg = GT_FilterIMsg(imsg) )
	{

		/* read out all we need */
		class = gtimsg->Class;
		mx = gtimsg->MouseX;
		my = gtimsg->MouseY;
		code = gtimsg->Code;
		gad = (struct Gadget *)gtimsg->IAddress;

		GT_PostFilterIMsg(gtimsg);

		switch(class)
		{
			case IDCMP_CLOSEWINDOW:
				/* tell this window to bugger off */
				ReplyMsg((struct Message *)imsg);
				KillOffEditPaletteWindow(sw);
				if( !RemakeScreenIfNeeded(globpalette) )
					InstallPalette(mainscreen,globpalette);
				SussOutQuiting(FALSE);
				break;
			case IDCMP_REFRESHWINDOW:
				/* repair GadTool renderings */
				ReplyMsg((struct Message *)imsg);
				GT_BeginRefresh(sw->sw_Window);
				GT_EndRefresh(sw->sw_Window,TRUE);
				break;
			case IDCMP_MOUSEMOVE:
				ReplyMsg((struct Message *)imsg);
				HandleMouseMove((struct EditPaletteWindow *)sw,gad,code);
				break;
			case IDCMP_GADGETUP:
				ReplyMsg((struct Message *)imsg);
				HandleGadgetUp(sw,gad,code);
				break;
			case IDCMP_MENUPICK:
				/* menuroutine does the reply() */
				SortOutEditPaletteWindowMenus( sw,imsg,code );
				break;
			case IDCMP_NEWSIZE:
				/* resuss all the gadget placements/font stuff */
				/* according to the new windowsize. */
				//printf("Width: %d  Height %d\n",sw->sw_Window->Width,sw->sw_Window->Height);
				SetupEditPaletteGadgets((struct EditPaletteWindow *)sw,TRUE);
				break;
			case IDCMP_MOUSEBUTTONS:
				ReplyMsg((struct Message *)imsg);
				if(!(code & IECODE_UP_PREFIX))
					SussColourPick((struct EditPaletteWindow *)sw, mx,my,code);
				break;
			case IDCMP_INACTIVEWINDOW:
				RemakeScreenIfNeeded( globpalette );
				//RemakeScreenIfNeeded( ((struct EditPaletteWindow *)sw)->epw_Palette );
				break;
			default:
				/* catch any superfluous crap going through... */
				ReplyMsg((struct Message *)imsg);
				break;
		}
	}
	else
		ReplyMsg((struct Message *)imsg);
}


/*******************   HandleMouseMove()   ****************************/
//
//  The only time we get a IDCMP_MOUSEMOVE event is when gadtools sends
// it to us. This is for the rgb sliders. So all we have to do is update
// the appropriate colour.

static void HandleMouseMove(struct EditPaletteWindow *epw, struct Gadget *gad, UWORD code)
{
	if(!epw->epw_256Scale)
		code |= code<<4;

	if(gad)
	{
		if(gad == epw->epw_Gadgets[MYGT_REDSLIDER])
		{
			epw->epw_Palette->pl_Colours[epw->epw_CurrentColour].r = code;
			RefreshGadget(epw->epw_Project, epw, MYGT_REDSLIDER);
		}
		if(gad == epw->epw_Gadgets[MYGT_GREENSLIDER])
		{
			epw->epw_Palette->pl_Colours[epw->epw_CurrentColour].g = code;
			RefreshGadget(epw->epw_Project, epw, MYGT_GREENSLIDER);
		}
		if(gad == epw->epw_Gadgets[MYGT_BLUESLIDER])
		{
			epw->epw_Palette->pl_Colours[epw->epw_CurrentColour].b = code;
			RefreshGadget(epw->epw_Project, epw, MYGT_BLUESLIDER);
		}
	}
	SetColour(epw->epw_Palette, epw->epw_CurrentColour);
}

/*******************   SetColour()   ********************************/
//
//  Set the given colour from the given palette. This just updates the
// mainscreen viewport to represent the correct colour.
//  Takes AGA and all that into account.

static void SetColour(struct Palette *pal, UWORD col)
{
	ULONG red,green,blue;

	if( (ownscreen) && (pal == globpalette) )
	{
		if( GfxBase->LibNode.lib_Version >= 39 )
		{
			red = pal->pl_Colours[col].r;
			red |= ((red << 24) | (red << 16) | (red << 8));
			green = pal->pl_Colours[col].g;
			green |= ((green << 24) | (green << 16) | (green << 8));
			blue = pal->pl_Colours[col].b;
			blue |= ((blue << 24) | (blue << 16) | (blue << 8));
			SetRGB32(&mainscreen->ViewPort,col,red,green,blue);
		}
		else
			SetRGB4(&mainscreen->ViewPort,col,pal->pl_Colours[col].r>>4,pal->pl_Colours[col].g>>4,pal->pl_Colours[col].b>>4);
	}
}

/****************  HandleGadgetUp()  ****************/
//
// Process GADGETUP IDCMP events for the RevealWindow.
//

static void HandleGadgetUp(struct SuperWindow *sw, struct Gadget *gad, UWORD code)
{
	struct EditPaletteWindow *epw;

	epw = (struct EditPaletteWindow *)sw;

	epw->epw_WaitingForNextColour = FALSE;
	epw->epw_ColourPick = NULL;

	switch(gad->GadgetID)
	{
		case MYGT_OKBUTTON:
			HandleOKGadget(epw);
			break;
		case MYGT_CANCELBUTTON:
			HandleCancelGadget(epw);
			break;
		case MYGT_SPREADBUTTON:
			epw->epw_WaitingForNextColour = TRUE;
			epw->epw_ColourPick = HandleSpreadPick;
			break;
		case MYGT_COPYBUTTON:
			epw->epw_WaitingForNextColour = TRUE;
			epw->epw_ColourPick = HandleCopyPick;
			break;
		case MYGT_EXCHANGEBUTTON:
			epw->epw_WaitingForNextColour = TRUE;
			epw->epw_ColourPick = HandleExchangePick;
			break;
		case MYGT_AGACYCLE:
			if(code)
			{
				epw->epw_256Scale = FALSE;
				GT_SetGadgetAttrs(epw->epw_Gadgets[MYGT_REDSLIDER], epw->epw_sw.sw_Window, NULL,
					GTSL_Max,			15,
					TAG_END);
				GT_SetGadgetAttrs(epw->epw_Gadgets[MYGT_GREENSLIDER], epw->epw_sw.sw_Window, NULL,
					GTSL_Max,			15,
					TAG_END);
				GT_SetGadgetAttrs(epw->epw_Gadgets[MYGT_BLUESLIDER], epw->epw_sw.sw_Window, NULL,
					GTSL_Max,			15,
					TAG_END);
			}
			else
			{
				epw->epw_256Scale = TRUE;
				GT_SetGadgetAttrs(epw->epw_Gadgets[MYGT_REDSLIDER], epw->epw_sw.sw_Window, NULL,
					GTSL_Max,			255,
					TAG_END);
				GT_SetGadgetAttrs(epw->epw_Gadgets[MYGT_GREENSLIDER], epw->epw_sw.sw_Window, NULL,
					GTSL_Max,			255,
					TAG_END);
				GT_SetGadgetAttrs(epw->epw_Gadgets[MYGT_BLUESLIDER], epw->epw_sw.sw_Window, NULL,
					GTSL_Max,			255,
					TAG_END);
			}
			NewColour(epw,epw->epw_CurrentColour);
			break;
		case MYGT_HEXCHECKBOX:
			if(gad->Flags & GFLG_SELECTED)
				epw->epw_HexReadOut = TRUE;
			else
				epw->epw_HexReadOut = FALSE;
			SetupEditPaletteGadgets(epw,FALSE);
			break;
	}
}

/******************   HandleOKGadget()   ******************/
//
//  Sorts out all the stuff to do with handling the ok gadget.
// It copies the EditPaletteWindow palette to the current globpalette.
// This could cause a few problems if people do dum things, but that's
// not my problem (user be wear).

static void HandleOKGadget(struct EditPaletteWindow *epw)
{
	KillOffEditPaletteWindow(&epw->epw_sw);
	RemakeScreenIfNeeded(globpalette);
	SussOutQuiting(FALSE);
}

/******************   HandleCancelGadget()   ********************/
//
//  Reset the palette back to the way it was before we opened the
// EditPaletteWindow. We let another routine ( RemakeScreenIfNeeded() )
// handle most of the crap.

static void HandleCancelGadget(struct EditPaletteWindow *epw)
{
	memcpy(epw->epw_Palette, &epw->epw_RevertPalette, sizeof(struct Palette));
	KillOffEditPaletteWindow(&epw->epw_sw);

	if( !RemakeScreenIfNeeded(globpalette) )
		InstallPalette(mainscreen,globpalette);
	SussOutQuiting(FALSE);
}

/******************   RemakeScreenIfNeeded()   ********************/
//
//  This gets the ideal pens for the given palette. If these match
// up with the current globpens, then we don't need to remake the
// screen, but if the pens are different, then we need to remake
// the screen with the correct pens.

BOOL RemakeScreenIfNeeded(struct Palette *pal)
{
	UWORD	 newpens[NUMDRIPENS+1];
	struct Palette *oldglobpal;
	UWORD  pennum;
	BOOL done = FALSE, penssame = TRUE;

	if(	mainscreen->RastPort.BitMap->Depth != glob.cg_ScrConfig.sg_DisplayDepth)
	{
		penssame = FALSE;
		goto RemakeScreen;
	}

	SussDrawInfoPens( newpens, pal, pal->pl_Count );

	for(pennum = 0; (pennum<NUMDRIPENS+1) && (!done); pennum++)
	{
		if( (newpens[pennum] == 0xFFFF) || (globpens[pennum] == 0xFFFF) )
			done = TRUE;

		if(newpens[pennum] != globpens[pennum])
		{
			penssame = FALSE;
			done = TRUE;
		}
	}
RemakeScreen:
	if( !penssame )
	{
		oldglobpal = globpalette;
		globpalette = pal;
		RemakeScreen();
		globpalette = oldglobpal;
		RefreshAllProjectTitles();
		return(TRUE);
	}
	return(FALSE);
}

/************************   HandleExchangePick()   **********************/
//
//  This is a handler routine for after the user has selected a destination
// colour.
//

void HandleExchangePick(struct EditPaletteWindow *epw, UWORD destcol)
{
	struct RGB8 tmprgb;
	struct RGB8 *sourcergb, *destrgb;

	epw->epw_WaitingForNextColour = FALSE;
	epw->epw_ColourPick = NULL;

	sourcergb = &epw->epw_Palette->pl_Colours[epw->epw_CurrentColour];
	destrgb = &epw->epw_Palette->pl_Colours[destcol];

	tmprgb.r = sourcergb->r;
	tmprgb.g = sourcergb->g;
	tmprgb.b = sourcergb->b;

	sourcergb->r = destrgb->r;
	sourcergb->g = destrgb->g;
	sourcergb->b = destrgb->b;

	destrgb->r = tmprgb.r;
	destrgb->g = tmprgb.g;
	destrgb->b = tmprgb.b;

	SetColour(epw->epw_Palette, epw->epw_CurrentColour);
	SetColour(epw->epw_Palette, destcol);

	NewColour(epw,destcol);
	RefreshGadget(epw->epw_Project, epw, MYGT_COLOURSLIDERS);
}

/************************   HandleCopyPick()   **********************/
//
//  This is a handler routine for after the user has selected a destination
// colour.
//

void HandleCopyPick(struct EditPaletteWindow *epw, UWORD destcol)
{
	epw->epw_WaitingForNextColour = FALSE;
	epw->epw_ColourPick = NULL;

	epw->epw_Palette->pl_Colours[destcol].r = epw->epw_Palette->pl_Colours[epw->epw_CurrentColour].r;
	epw->epw_Palette->pl_Colours[destcol].g = epw->epw_Palette->pl_Colours[epw->epw_CurrentColour].g;
	epw->epw_Palette->pl_Colours[destcol].b = epw->epw_Palette->pl_Colours[epw->epw_CurrentColour].b;

	SetColour(epw->epw_Palette, destcol);

	NewColour(epw,destcol);
	RefreshGadget(epw->epw_Project, epw, MYGT_COLOURSLIDERS);
}

/************************   HandleSpreadPick()   **********************/
//
//  This is a handler routine for after the user has selected a destination
// colour.
//

void HandleSpreadPick(struct EditPaletteWindow *epw, UWORD destcol)
{
	UWORD sourcecol,tmpcol,steps,col;
	LONG	rgbdelta[3], runningrgb[3],sourcergb[3],destrgb[3];
	struct Palette *pal;

	epw->epw_WaitingForNextColour = FALSE;
	epw->epw_ColourPick = NULL;

	sourcecol = epw->epw_CurrentColour;

	if(sourcecol > destcol)
	{
		tmpcol = sourcecol;
		sourcecol = destcol;
		destcol = tmpcol;
	}

	steps = destcol - sourcecol;
	if( (steps == 0) || (steps == 1) )
		return;

	pal = epw->epw_Palette;

	if(epw->epw_256Scale)
	{
		sourcergb[0]  = pal->pl_Colours[sourcecol].r;
		sourcergb[0] |= (sourcergb[0]*(1<<16)) | (sourcergb[0]*(1<< 8));
		sourcergb[1]  = pal->pl_Colours[sourcecol].g;
		sourcergb[1] |= (sourcergb[1]*(1<<16)) | (sourcergb[1]*(1<< 8));
		sourcergb[2]  = pal->pl_Colours[sourcecol].b;
		sourcergb[2] |= (sourcergb[2]*(1<<16)) | (sourcergb[2]*(1<< 8));
	
		destrgb[0]  = pal->pl_Colours[destcol].r;
		destrgb[0] |= (destrgb[0]*(1<<16)) | (destrgb[0]*(1<<8));
		destrgb[1]  = pal->pl_Colours[destcol].g;
		destrgb[1] |= (destrgb[1]*(1<<16)) | (destrgb[1]*(1<<8));
		destrgb[2]  = pal->pl_Colours[destcol].b;
		destrgb[2] |= (destrgb[2]*(1<<16)) | (destrgb[2]*(1<<8));
	
		rgbdelta[0]  = destrgb[0] - sourcergb[0];
		rgbdelta[1]  = destrgb[1] - sourcergb[1];
		rgbdelta[2]  = destrgb[2] - sourcergb[2];
	
		rgbdelta[0] /= steps;
		rgbdelta[1] /= steps;
		rgbdelta[2] /= steps;
	
		runningrgb[0] = sourcergb[0];
		runningrgb[1] = sourcergb[1];
		runningrgb[2] = sourcergb[2];
	
		for(col = sourcecol+1; col < destcol; col++)
		{
			runningrgb[0] += rgbdelta[0];
			runningrgb[1] += rgbdelta[1];
			runningrgb[2] += rgbdelta[2];
	
			pal->pl_Colours[col].r = runningrgb[0]>>16;
			pal->pl_Colours[col].g = runningrgb[1]>>16;
			pal->pl_Colours[col].b = runningrgb[2]>>16;
	
			SetColour(pal,col);
		}
	}
	else
	{
		pal->pl_Colours[sourcecol].r = (pal->pl_Colours[sourcecol].r & 0xF0) | (pal->pl_Colours[sourcecol].r>>4);
		pal->pl_Colours[sourcecol].g = (pal->pl_Colours[sourcecol].g & 0xF0) | (pal->pl_Colours[sourcecol].g>>4);
		pal->pl_Colours[sourcecol].b = (pal->pl_Colours[sourcecol].b & 0xF0) | (pal->pl_Colours[sourcecol].b>>4);
		SetColour(pal,sourcecol);
		sourcergb[0]  = pal->pl_Colours[sourcecol].r;
		sourcergb[0] |= (sourcergb[0]*(1<<16)) | (sourcergb[0]*(1<< 8));
		sourcergb[1]  = pal->pl_Colours[sourcecol].g;
		sourcergb[1] |= (sourcergb[1]*(1<<16)) | (sourcergb[1]*(1<< 8));
		sourcergb[2]  = pal->pl_Colours[sourcecol].b;
		sourcergb[2] |= (sourcergb[2]*(1<<16)) | (sourcergb[2]*(1<< 8));
	
		pal->pl_Colours[destcol].r = (pal->pl_Colours[destcol].r & 0xF0) | (pal->pl_Colours[destcol].r>>4);
		pal->pl_Colours[destcol].g = (pal->pl_Colours[destcol].g & 0xF0) | (pal->pl_Colours[destcol].g>>4);
		pal->pl_Colours[destcol].b = (pal->pl_Colours[destcol].b & 0xF0) | (pal->pl_Colours[destcol].b>>4);
		SetColour(pal,destcol);
		destrgb[0]  = pal->pl_Colours[destcol].r;
		destrgb[0] |= (destrgb[0]*(1<<16)) | (destrgb[0]*(1<<8));
		destrgb[1]  = pal->pl_Colours[destcol].g;
		destrgb[1] |= (destrgb[1]*(1<<16)) | (destrgb[1]*(1<<8));
		destrgb[2]  = pal->pl_Colours[destcol].b;
		destrgb[2] |= (destrgb[2]*(1<<16)) | (destrgb[2]*(1<<8));
	
		rgbdelta[0]  = destrgb[0] - sourcergb[0];
		rgbdelta[1]  = destrgb[1] - sourcergb[1];
		rgbdelta[2]  = destrgb[2] - sourcergb[2];
	
		rgbdelta[0] /= steps;
		rgbdelta[1] /= steps;
		rgbdelta[2] /= steps;
	
		runningrgb[0] = sourcergb[0];
		runningrgb[1] = sourcergb[1];
		runningrgb[2] = sourcergb[2];
	
		for(col = sourcecol+1; col < destcol; col++)
		{
			runningrgb[0] += rgbdelta[0];
			runningrgb[1] += rgbdelta[1];
			runningrgb[2] += rgbdelta[2];
	
			pal->pl_Colours[col].r = runningrgb[0]>>20;
			pal->pl_Colours[col].g = runningrgb[1]>>20;
			pal->pl_Colours[col].b = runningrgb[2]>>20;
			pal->pl_Colours[col].r |= pal->pl_Colours[col].r<<4;
			pal->pl_Colours[col].g |= pal->pl_Colours[col].g<<4;
			pal->pl_Colours[col].b |= pal->pl_Colours[col].b<<4;
	
			SetColour(pal,col);
		}

	}
	NewColour(epw,destcol);
	RefreshGadget(epw->epw_Project, epw, MYGT_COLOURSLIDERS);
}

/************* SortOutEditPaletteWindowMenus() **************/
//
// Handle MENUPICK IDCMP messages - note the reply() is given
// here rather than in the main loop.
// Why is this so? Well... who can tell?
//

static void SortOutEditPaletteWindowMenus(struct SuperWindow *sw,
													 struct IntuiMessage *imsg,
													 UWORD code)
{
	int i = 0;
	UWORD menunumber[10];
	UWORD menunum,itemnum,subnum;
	UWORD status = 0;

	WaitForSubTaskDone(mainmsgport);

	/* NastyBits alert: */
	/* Read out up to 10 NextSelect thingys. Hey, it works! */
	menunumber[i] = code;
	while ( (i < 10) && ( menunumber[i] != MENUNULL) )
	{
		i++;
		menunumber[i] =
			( ItemAddress( sw->sw_MenuStrip, menunumber[i-1] ) )->NextSelect;
	}
	/* reply pronto */
	ReplyMsg((struct Message *)imsg);


	/* Go through out little array of menunumbers and process them */
	/* 'status' is used so certain events (eg Quit) can terminate processing. */

	for(i=0;
		( i < 10 ) &&
			( menunumber[i] != MENUNULL ) &&
			!(status & 1);
		i++ )
	{
		menunum = MENUNUM(menunumber[i]);
		itemnum = ITEMNUM(menunumber[i]);			// These two lines should be
		subnum = SUBNUM(menunumber[i]);				// killed some time.

		switch(menunum)
		{
			case MN_PROJECT:
				status |= DoProjectMenu(sw,menunumber[i]);
				break;
			case MN_WINDOW:
				status |= DoWindowMenu(sw,menunumber[i]);
				break;
			case MN_SETTINGS:
				DoSettingsMenu(sw,menunumber[i]);
				break;
			case MN_MODE:
				DoModeMenu(sw,menunumber[i]);
				break;
			case MN_BRUSH:
				DoBrushMenu(sw,menunumber[i]);
				break;
			case MN_USER:
				DoUserMenu(sw,menunumber[i]);
				break;
		}
	}
	if (status & 2)
	{
		(*sw->sw_HideWindow)(sw);
		(*sw->sw_ShowWindow)(sw);
	}
}


/************** SetupEditPaletteGadgets **************/
//
// Sets up and formats all the gadgets within the EditPaletteWindow.
// Adapts to font and window sizes.
// Call at window opening, and again at NEWSIZE events.

static void SetupEditPaletteGadgets(struct EditPaletteWindow *epw, BOOL clear)
{
	struct Gadget *gad;
	struct NewGadget ng;
	struct Window *win;
	UWORD buttonh,buttonw;
	UWORD	wintop,winbottom,winleft,winright,letterwidth,wingadgetright;
	UWORD minwidth,minheight;
	UWORD texty;

	win = epw->epw_sw.sw_Window;

	/* first, kill any old gadgets that may be active */
	if(epw->epw_sw.sw_GadList)
	{
		RemoveGList(win, epw->epw_sw.sw_GadList, -1);
		FreeGadgets(epw->epw_sw.sw_GadList);
		epw->epw_sw.sw_GadList = NULL;
	}

	if(clear)	ClearWindow( win );


	/* use window font */
	SetFont(win->RPort,windowtextfont);
	epw->epw_sw.sw_TextAttr = &windowtextattr;
	epw->epw_sw.sw_TextFont = windowtextfont;

	buttonh = win->RPort->Font->tf_YSize + 5;
	buttonw = TextLength(win->RPort,"EXCHANGE",8)+4;
	minheight = buttonh*6;
	minwidth  = (buttonw*4)+32;

	/* fall back to Topaz 8 if the window is too small */
	AdjustWindowFont(&epw->epw_sw,minwidth,minheight);  //(buttonw*4)+20,buttonh*6);

	/* init gadget stuff */
	gad = CreateContext(&epw->epw_sw.sw_GadList);

	buttonh = win->RPort->Font->tf_YSize + 3;
	buttonw = TextLength(win->RPort,"EXCHANGE",8)+4;
	wintop = win->BorderTop;
	winbottom = win->Height - win->BorderBottom-1;
	winleft = win->BorderLeft;
	winright = win->Width - win->BorderRight-1;
	letterwidth = TextLength(win->RPort,"W",1)+3;
	wingadgetright = (buttonw+2)*4; //winleft+letterwidth+128+buttonw;

	/* Print 'R','G','B' to the left of where the sliders are going to be */

	texty = wintop+1+win->RPort->Font->tf_Baseline+1;
	SetAPen(win->RPort,globpens[SHINEPEN]);
	Move(win->RPort,winleft+1,texty);
	Text(win->RPort,"R",1);
	texty += buttonh+1;
	Move(win->RPort,winleft+1,texty);
	Text(win->RPort,"G",1);
	texty += buttonh+1;
	Move(win->RPort,winleft+1,texty);
	Text(win->RPort,"B",1);

	/* Create RGB Slider Gadgets */

	ng.ng_TextAttr = epw->epw_sw.sw_TextAttr;					/* Red Slider */
	ng.ng_VisualInfo = epw->epw_sw.sw_VisualInfo;
	ng.ng_LeftEdge = winleft+letterwidth;
	ng.ng_TopEdge = wintop+1;
	ng.ng_Width = 128;
	ng.ng_Height = buttonh;
	ng.ng_GadgetText = NULL;
	ng.ng_GadgetID = MYGT_REDSLIDER;
	ng.ng_Flags = 0;
	gad = CreateGadget( SLIDER_KIND, gad, &ng,
		GTSL_Min,					0,
		GTSL_Max,					epw->epw_256Scale ? 255 : 15,
		GTSL_Level,				0,
		GTSL_MaxLevelLen,	3,
		GTSL_LevelFormat,	epw->epw_HexReadOut ? " %02lx" : "%3ld",
		GTSL_LevelPlace,	PLACETEXT_RIGHT,
		TAG_END );
	epw->epw_Gadgets[MYGT_REDSLIDER] = gad;

	ng.ng_TopEdge += buttonh+1;												/* Green Slider */
	ng.ng_GadgetID = MYGT_GREENSLIDER;
	gad = CreateGadget( SLIDER_KIND, gad, &ng,
		GTSL_Min,					0,
		GTSL_Max,					epw->epw_256Scale ? 255 : 15,
		GTSL_Level,				0,
		GTSL_MaxLevelLen,	3,
		GTSL_LevelFormat,	epw->epw_HexReadOut ? " %02lx" : "%3ld",
		GTSL_LevelPlace,	PLACETEXT_RIGHT,
		TAG_END );
	epw->epw_Gadgets[MYGT_GREENSLIDER] = gad;

	ng.ng_TopEdge += buttonh+1;												/* Blue Slider */
	ng.ng_GadgetID = MYGT_BLUESLIDER;
	gad = CreateGadget( SLIDER_KIND, gad, &ng,
		GTSL_Min,					0,
		GTSL_Max,					epw->epw_256Scale ? 255 : 15,
		GTSL_Level,				0,
		GTSL_MaxLevelLen,	3,
		GTSL_LevelFormat,	epw->epw_HexReadOut ? " %02lx" : "%3ld",
		GTSL_LevelPlace,	PLACETEXT_RIGHT,
		TAG_END );
	epw->epw_Gadgets[MYGT_BLUESLIDER] = gad;

	ng.ng_LeftEdge = winleft+1;																/* OK Gadget */
	ng.ng_TopEdge = winbottom-buttonh-1;
	ng.ng_Width = buttonw;
	ng.ng_Height = buttonh;
	ng.ng_GadgetText = "OK";
	ng.ng_GadgetID = MYGT_OKBUTTON;
	ng.ng_Flags = 0;
	gad = CreateGadget( BUTTON_KIND, gad, &ng, TAG_END );
	epw->epw_Gadgets[MYGT_OKBUTTON] = gad;

	ng.ng_LeftEdge += buttonw-26;														/* Hex CheckBox */
	ng.ng_TopEdge -= (buttonh+1);
	ng.ng_GadgetText = "Hex";
	ng.ng_GadgetID = MYGT_HEXCHECKBOX;
	ng.ng_Flags = PLACETEXT_LEFT;
	gad = CreateGadget( CHECKBOX_KIND, gad, &ng,
		GTCB_Checked,		epw->epw_HexReadOut,
		TAG_END );
	epw->epw_Gadgets[MYGT_HEXCHECKBOX] = gad;

	ng.ng_LeftEdge = wingadgetright-buttonw-1;							/* Cancel Gadget */
	ng.ng_TopEdge = winbottom-buttonh-1;
	ng.ng_GadgetText = "Cancel";
	ng.ng_GadgetID = MYGT_CANCELBUTTON;
	ng.ng_Flags = 0;
	gad = CreateGadget( BUTTON_KIND, gad, &ng, TAG_END );
	epw->epw_Gadgets[MYGT_CANCELBUTTON] = gad;

	ng.ng_LeftEdge -= (buttonw+1);													/* Spread Gadget */
	ng.ng_GadgetText = "Spread";
	ng.ng_GadgetID = MYGT_SPREADBUTTON;
	gad = CreateGadget( BUTTON_KIND, gad, &ng, TAG_END );
	epw->epw_Gadgets[MYGT_SPREADBUTTON] = gad;

	ng.ng_TopEdge -= (buttonh+1);														/* Exchange Gadget */
	ng.ng_GadgetText = "Exchange";
	ng.ng_GadgetID = MYGT_EXCHANGEBUTTON;
	gad = CreateGadget( BUTTON_KIND, gad, &ng, TAG_END );
	epw->epw_Gadgets[MYGT_EXCHANGEBUTTON] = gad;

	ng.ng_LeftEdge += buttonw+1;														/* Number Gadget */
	ng.ng_GadgetText = NULL;
	ng.ng_GadgetID = MYGT_COLOURNUMBER;
	gad = CreateGadget( NUMBER_KIND, gad, &ng,
		GTNM_Number,			6,
		GTNM_Border,			TRUE,
		TAG_END );
	epw->epw_Gadgets[MYGT_COLOURNUMBER] = gad;

	ng.ng_TopEdge -= (buttonh+1);														/* Bevel Box */
	DrawBevelBox(win->RPort,ng.ng_LeftEdge,ng.ng_TopEdge,buttonw,buttonh,
		GT_VisualInfo,		ng.ng_VisualInfo,
		GTBB_Recessed,		TRUE);
	epw->epw_CBoxLeft = ng.ng_LeftEdge+3;
	epw->epw_CBoxTop = ng.ng_TopEdge+2;
	epw->epw_CBoxWidth = buttonw-6;
	epw->epw_CBoxHeight = buttonh-4;

	ng.ng_LeftEdge -= (buttonw+1);													/* Copy Gadget */
	ng.ng_GadgetText = "Copy";
	ng.ng_GadgetID = MYGT_COPYBUTTON;
	gad = CreateGadget( BUTTON_KIND, gad, &ng, TAG_END );
	epw->epw_Gadgets[MYGT_COPYBUTTON] = gad;

	ng.ng_LeftEdge = winleft+1;															/* AGA Cycle Gadget */
	ng.ng_GadgetText = NULL;
	ng.ng_GadgetID = MYGT_AGACYCLE;
	gad = CreateGadget( CYCLE_KIND, gad, &ng,
		GTCY_Labels,	cyclelabels,
		GTCY_Active,	epw->epw_256Scale ? 0 : 1,
		TAG_END );
	epw->epw_Gadgets[MYGT_AGACYCLE] = gad;

	/* check for error */
	if (!gad)
	{
		FreeGadgets(epw->epw_sw.sw_GadList);
		return;
	}

	/* attach our groovy new gadgets to the window */
	AddGList(win, epw->epw_sw.sw_GadList, -1, -1, NULL);
//	RefreshGList(epw->epw_sw.sw_GadList, win, NULL, -1);
	GT_RefreshWindow(win, NULL);

	if(clear)
	{
			/* Palette Stuff */
		RethinkPaletteBox(&epw->epw_PaletteBox, wingadgetright+1, wintop,
											winright-(wingadgetright+1)+1, winbottom-wintop+1);
			/* need to redraw the frame coz we're not using GimmeZeroZero */
		RefreshWindowFrame(win);
	}
	NewColour(epw,epw->epw_CurrentColour);
}

static void DrawColour(struct EditPaletteWindow *epw,UWORD col)
{
	struct Window *win;
	if(win = epw->epw_sw.sw_Window)
	{
		SetAPen(win->RPort,col);
		RectFill(win->RPort,epw->epw_CBoxLeft,epw->epw_CBoxTop,
						 epw->epw_CBoxLeft+epw->epw_CBoxWidth-1,epw->epw_CBoxTop+epw->epw_CBoxHeight-1);
	}
}

/***********************   SussColourPick()   ********************/
//
//  This sorts out the user mouse button events, and sets the current
// colour if the user selected one.

void SussColourPick(struct EditPaletteWindow *epw, WORD mx, WORD my, UWORD button)
{
	UWORD col;
	struct PaletteBox *pb;

	pb = &epw->epw_PaletteBox;

	if( (mx < pb->pb_BoxLeft) ||
			(my < pb->pb_BoxTop) ||
			(mx > (pb->pb_BoxLeft + pb->pb_BoxWidth - 1)) ||
			(my > (pb->pb_BoxTop + pb->pb_BoxHeight - 1)) )
		return;

	mx -= pb->pb_BoxLeft;
	my -= pb->pb_BoxTop;

	col = ((mx / pb->pb_ColourWidth) * pb->pb_ColoursHigh) +
				(my / pb->pb_ColourHeight);

	if(epw->epw_WaitingForNextColour)
		(*epw->epw_ColourPick)(epw,col);
	else
		NewColour(epw,col);
}

/************************   NewColour()   *********************/
//
//  Set the Gadgets up for a new colour.
//

static void NewColour(struct EditPaletteWindow *epw, UWORD col)
{
	DrawColour(epw,col);
	GT_SetGadgetAttrs(epw->epw_Gadgets[MYGT_COLOURNUMBER], epw->epw_sw.sw_Window, NULL,
		GTNM_Number, col,
		TAG_END);
	GT_SetGadgetAttrs(epw->epw_Gadgets[MYGT_REDSLIDER], epw->epw_sw.sw_Window, NULL,
		GTSL_Level, epw->epw_256Scale ?
			epw->epw_Palette->pl_Colours[col].r : epw->epw_Palette->pl_Colours[col].r>>4,
		TAG_END);
	GT_SetGadgetAttrs(epw->epw_Gadgets[MYGT_GREENSLIDER], epw->epw_sw.sw_Window, NULL,
		GTSL_Level,  epw->epw_256Scale ?
			epw->epw_Palette->pl_Colours[col].g : epw->epw_Palette->pl_Colours[col].g>>4,
		TAG_END);
	GT_SetGadgetAttrs(epw->epw_Gadgets[MYGT_BLUESLIDER], epw->epw_sw.sw_Window, NULL,
		GTSL_Level,  epw->epw_256Scale ?
			epw->epw_Palette->pl_Colours[col].b : epw->epw_Palette->pl_Colours[col].b>>4,
		TAG_END);
	RestoreColour(&epw->epw_PaletteBox,epw->epw_CurrentColour);
	epw->epw_CurrentColour = col;
	BevelColour(&epw->epw_PaletteBox,epw->epw_CurrentColour);
}


/************************   RefreshGadget()   *********************/
//
//  Go through all the EditPaletteWindows that are attached to this
// project (execpt for missepw, which can be NULL), and update the
// appropirate Gadget.
//

static void RefreshGadget(struct Project *pj, struct EditPaletteWindow *missepw, UWORD gadgetid)
{
	struct EditPaletteWindow *epw;
	struct Window *win;

	if( !pj )
		return;
	if(!pj->pj_EditPaletteWindows)
		return;

	epw = pj->pj_EditPaletteWindows;
	while(epw)
	{
		if( (epw != missepw) && (win = epw->epw_sw.sw_Window) )
		{
			switch(gadgetid)
			{
				case MYGT_REDSLIDER:
					GT_SetGadgetAttrs(epw->epw_Gadgets[MYGT_REDSLIDER], epw->epw_sw.sw_Window, NULL,
								GTSL_Level, epw->epw_256Scale ?
								epw->epw_Palette->pl_Colours[epw->epw_CurrentColour].r : epw->epw_Palette->pl_Colours[epw->epw_CurrentColour].r>>4,
								TAG_END);
					break;
				case MYGT_GREENSLIDER:
					GT_SetGadgetAttrs(epw->epw_Gadgets[MYGT_GREENSLIDER], epw->epw_sw.sw_Window, NULL,
								GTSL_Level, epw->epw_256Scale ?
								epw->epw_Palette->pl_Colours[epw->epw_CurrentColour].g : epw->epw_Palette->pl_Colours[epw->epw_CurrentColour].g>>4,
								TAG_END);
					break;
				case MYGT_BLUESLIDER:
					GT_SetGadgetAttrs(epw->epw_Gadgets[MYGT_BLUESLIDER], epw->epw_sw.sw_Window, NULL,
								GTSL_Level, epw->epw_256Scale ?
								epw->epw_Palette->pl_Colours[epw->epw_CurrentColour].b : epw->epw_Palette->pl_Colours[epw->epw_CurrentColour].b>>4,
								TAG_END);
					break;
				case MYGT_COLOURSLIDERS:
					GT_SetGadgetAttrs(epw->epw_Gadgets[MYGT_REDSLIDER], epw->epw_sw.sw_Window, NULL,
								GTSL_Level, epw->epw_256Scale ?
								epw->epw_Palette->pl_Colours[epw->epw_CurrentColour].r : epw->epw_Palette->pl_Colours[epw->epw_CurrentColour].r>>4,
								TAG_END);
					GT_SetGadgetAttrs(epw->epw_Gadgets[MYGT_GREENSLIDER], epw->epw_sw.sw_Window, NULL,
								GTSL_Level, epw->epw_256Scale ?
								epw->epw_Palette->pl_Colours[epw->epw_CurrentColour].g : epw->epw_Palette->pl_Colours[epw->epw_CurrentColour].g>>4,
								TAG_END);
					GT_SetGadgetAttrs(epw->epw_Gadgets[MYGT_BLUESLIDER], epw->epw_sw.sw_Window, NULL,
								GTSL_Level, epw->epw_256Scale ?
								epw->epw_Palette->pl_Colours[epw->epw_CurrentColour].b : epw->epw_Palette->pl_Colours[epw->epw_CurrentColour].b>>4,
								TAG_END);
					break;
			}
		}
		epw = epw->epw_NextEditPaletteWindow;
	}
}

/**************** UnlinkEditPaletteWindow() ****************/
//
// Detaches a superwindow from its parent project.
// (note that we assume that the window _is_ actually
// attached to a project in the first place).

static void UnlinkEditPaletteWindow(struct EditPaletteWindow *epw)
{
	struct EditPaletteWindow *prevwindow,*nextwindow;
	struct Project *pj;

	/*	unlink the superwindow from the projects window list */

	pj = epw->epw_Project;

	nextwindow = epw->epw_NextEditPaletteWindow;
	prevwindow = GetPrevEditPaletteWindow(epw);

	if (prevwindow == NULL)
		pj->pj_EditPaletteWindows = nextwindow;
	else
		prevwindow->epw_NextEditPaletteWindow = nextwindow;

	epw->epw_Project = NULL;
	epw->epw_NextEditPaletteWindow = NULL;
}

/***************** GetPrevEditPaletteWindow() **************/
//
//  Find the prevous window connected to this project.
// If there is no previous window, then NULL is returned.

static struct EditPaletteWindow *GetPrevEditPaletteWindow(struct EditPaletteWindow *epw)
{
	struct EditPaletteWindow *prevwindow;
	struct Project *pj;

	pj = epw->epw_Project;
	prevwindow = pj->pj_EditPaletteWindows;

	if (prevwindow == epw)
		prevwindow = NULL;
	else
	{
		while (prevwindow->epw_NextEditPaletteWindow != epw)
			prevwindow = prevwindow->epw_NextEditPaletteWindow;
	}
	return(prevwindow);
}
