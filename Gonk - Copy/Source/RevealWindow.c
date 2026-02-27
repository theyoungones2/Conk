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

static struct NewMenu	revealmenu[] =
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

/* labels for the Windows/Projects selector (a cycle gadget) */

static UBYTE *cyclelabels[] = {
	(UBYTE *)"Windows",
	(UBYTE *)"Projects",
	NULL };


/************ Prototypes for functions local to this file ************/

BOOL ShowRevealWindow(struct SuperWindow *sw);
BOOL SetupRevealWindowMenus(struct SuperWindow *sw);
void HideRevealWindow(struct SuperWindow *sw);
APTR KillOffRevealWindow(struct SuperWindow *sw);
void RevealWindowHandler(struct SuperWindow *sw,	struct IntuiMessage *imsg);

static void HandleGadgetUp(struct SuperWindow *sw,
													 struct Gadget *gad,
													 UWORD code);
static void SortOutRevealWindowMenus(struct SuperWindow *sw,
																		 struct IntuiMessage *imsg,
																		 UWORD code);
static void RemakeRevealList(struct RevealWindow *rw);
static void FreeRevealList(struct RevealWindow *rw);
static void SetupRevealGadgets(struct RevealWindow *rw);


/************ OpenNewRevealWindow ************/
//
// Opens a RevealWindow. The window displays a list of all the
// currently hidden SuperWindows or Projects, allowing the user
// to make them reappear.
//

BOOL OpenNewRevealWindow(struct WindowDefault *wf, UWORD type)
{
	struct	RevealWindow	*rw;
	BOOL ret = FALSE;

	/* grab mem for the RevealWindow struct */
	if ( !( rw= AllocVec(sizeof(struct RevealWindow), MEMF_CLEAR ) ) )
		return(ret);

	/* figure out a unique ID for this window (mainly for AREXX use) */
	InstallUniqueWindowID((struct SuperWindow *)rw);
	rw->rw_sw.sw_Node.ln_Name = (char *)&(rw->rw_sw.sw_idstring);

	/* find the Default settings for this type of SuperWindow */
	//wf = &glob.cg_WindowDefaults[SWTY_REVEAL];

	/* set up handler routines and assorted SuperWindow data */
	rw->rw_sw.sw_Handler = RevealWindowHandler;
	rw->rw_sw.sw_HideWindow = HideRevealWindow;
	rw->rw_sw.sw_ShowWindow = ShowRevealWindow;
	rw->rw_sw.sw_BuggerOff = KillOffRevealWindow;
	rw->rw_sw.sw_RefreshMenus = SetupRevealWindowMenus;
	rw->rw_sw.sw_Window = NULL;
	rw->rw_sw.sw_MenuStrip = NULL;
	rw->rw_sw.sw_GadList = NULL;
	rw->rw_sw.sw_Type = SWTY_REVEAL;
	rw->rw_sw.sw_VisualInfo = NULL;

	/* set up stuff for opening the actual intuition window */
	rw->rw_sw.sw_Left = wf->wf_Left;
	rw->rw_sw.sw_Top = wf->wf_Top;
	rw->rw_sw.sw_Width = wf->wf_Width;
	rw->rw_sw.sw_Height = wf->wf_Height;
	rw->rw_sw.sw_MinWidth = 110;					/* minwidth set for topaz8 */
	rw->rw_sw.sw_MaxWidth = 0xFFFF;
	rw->rw_sw.sw_MaxHeight = 0XFFFF;
	rw->rw_sw.sw_MenuStrip = NULL;

	rw->rw_sw.sw_FlgBackDrop = wf->wf_FlgBackDrop;
	rw->rw_sw.sw_FlgBorderless = wf->wf_FlgBorderless;
	rw->rw_sw.sw_FlgTitle = wf->wf_FlgTitle;
	rw->rw_sw.sw_FlgBorderGadgets = wf->wf_FlgBorderGadgets;

	/* setup SuperWindow basename */
	rw->rw_sw.sw_Title = "Reveal";

	rw->rw_NowShowing = type;		//RWNS_WINDOWS;

	/* add window into the superwindows list */
	AddTail(&superwindows, &rw->rw_sw.sw_Node);

	/* init stuff for listview gadget */
	NewList(&rw->rw_Labels);
	rw->rw_LVGadget = NULL;
	rw->rw_CurrentSelection = 0xFFFF;

	/* open the intuition window (if the window doesn't open, */
	/* it is just left hidden - SEP). */
	ret = ShowRevealWindow(&rw->rw_sw);

	return(ret);
}



/************ ShowRevealWindow ************/
//
// Displays a RevealWindow, and sets up all its gadgets, lists
// etc...
//

BOOL ShowRevealWindow(struct SuperWindow *sw)
{
	struct RevealWindow *rw;
	UWORD minh;
	ULONG flags;

	rw = (struct RevealWindow *)sw;

	if(!sw->sw_Window)				/* make sure it isn't already open */
	{
		/* Set up the Visual Info for GadTools */
		if (!(rw->rw_sw.sw_VisualInfo = GetVisualInfo(MAINSCREEN,TAG_END)))
			return(FALSE);

		minh = MAINSCREEN->Font->ta_YSize + 70 +
			MAINSCREEN->WBorTop;

		flags = WFLG_ACTIVATE | WFLG_NEWLOOKMENUS;
		if(sw->sw_FlgTitle)
			flags = flags | WFLG_DRAGBAR | WFLG_DEPTHGADGET | WFLG_CLOSEGADGET;
		if(sw->sw_FlgBorderGadgets)
			flags = flags | WFLG_SIZEGADGET | WFLG_SIZEBBOTTOM;

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

		SetupRevealGadgets(rw);

		/* Tell the window which superwindow it is attached to */
		/* (so we'll know which handler routine to call) */
		sw->sw_Window->UserData = (BYTE *)rw;

		SetupRevealWindowMenus(sw);

		/* we want to use our global message port for IDCMP stuff */
		sw->sw_Window->UserPort = mainmsgport;
		ModifyIDCMP(sw->sw_Window, IDCMP_CLOSEWINDOW | IDCMP_MENUPICK | IDCMP_NEWSIZE |
			IDCMP_REFRESHWINDOW | BUTTONIDCMP | CYCLEIDCMP | LISTVIEWIDCMP | IDCMP_CHANGEWINDOW |
			IDCMP_MOUSEMOVE);

		SetSuperWindowScreenTitle(sw);
	}

	RemakeRevealList(rw);

	return(TRUE);
}


BOOL SetupRevealWindowMenus(struct SuperWindow *sw)
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
	SetupWindowMenu( sw, &revealmenu[MN_WINDOW_IDX] );
	SetupEditMenu( NULL, &revealmenu[MN_EDIT_IDX] );
	SetupSettingsMenu( &revealmenu[MN_SETTINGS_IDX] );
	SetupModeMenu( sw, &revealmenu[MN_MODE_IDX] );
	LinkInUserMenu( &revealmenu[MN_USER_IDX] );

	if( sw->sw_MenuStrip = CreateMenus( revealmenu, TAG_END ) )
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


/******************  HideRevealWindow()  ******************/
//
// Hides the given RevealWindow. Frees gadgets and stuff.
//

void HideRevealWindow(struct SuperWindow *sw)
{
	struct RevealWindow *rw;

	rw = (struct RevealWindow *)sw;

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

		FreeGadgets(rw->rw_sw.sw_GadList);
		rw->rw_sw.sw_GadList = NULL;
		rw->rw_LVGadget = NULL;
		FreeRevealList( rw );
		if (sw->sw_VisualInfo) FreeVisualInfo(sw->sw_VisualInfo);
		sw->sw_VisualInfo = NULL;
		SetProcessWindowPtr();
	}
}


/******************  KillOffRevealWindow()  ******************/
//
// Shuts down a RevealWindow and removes it from the SuperWindows
// list.
//

APTR KillOffRevealWindow(struct SuperWindow *sw)
{
	struct RevealWindow *rw;

	rw = (struct RevealWindow *)sw;

	/* close/free the superwindow */
//	if (sw->sw_VisualInfo) FreeVisualInfo(sw->sw_VisualInfo);
	HideRevealWindow(sw);												/* shut down the Intuition window */
	Remove(&(sw->sw_Node));											/* remove from superwindows list */
	FreeVec(rw);																/* free RevealWindow struct */
	return(NULL);
}

/******************  RevealWindowHandler  ******************/
//
// Processes IDCMP messages coming into any RevealWindows,
// dispatching appropriate bits of code.
//

void RevealWindowHandler(struct SuperWindow *sw,
	struct IntuiMessage *imsg)
{
	ULONG	class;
	UWORD	code;
	struct IntuiMessage *gtimsg;
	struct Gadget *gad;

	if( gtimsg = GT_FilterIMsg(imsg) )
	{

		/* read out all we need */
		class = gtimsg->Class;
		code = gtimsg->Code;
		gad = (struct Gadget *)gtimsg->IAddress;

		GT_PostFilterIMsg(gtimsg);

		switch(class)
		{
			case IDCMP_CLOSEWINDOW:
				/* tell this window to bugger off */
				ReplyMsg((struct Message *)imsg);
				KillOffRevealWindow(sw);
				SussOutQuiting(FALSE);
				break;
			case IDCMP_REFRESHWINDOW:
				/* repair GadTool renderings */
				ReplyMsg((struct Message *)imsg);
				GT_BeginRefresh(sw->sw_Window);
				GT_EndRefresh(sw->sw_Window,TRUE);
				break;
			case IDCMP_GADGETUP:
				ReplyMsg((struct Message *)imsg);
				HandleGadgetUp(sw,gad,code);
				break;
			case IDCMP_MENUPICK:
				/* menuroutine does the reply() */
				SortOutRevealWindowMenus( sw,imsg,code );
				break;
			case IDCMP_NEWSIZE:
				/* resuss all the gadget placements/font stuff */
				/* according to the new windowsize. */
				SetupRevealGadgets((struct RevealWindow *)sw);
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


/****************  HandleGadgetUp()  ****************/
//
// Process GADGETUP IDCMP events for the RevealWindow.
//

static void HandleGadgetUp(struct SuperWindow *sw,
	struct Gadget *gad, UWORD code)
{
	struct RevealWindow *rw;
	struct ProjectWindow *pw;
	struct Project *proj;
	struct DataNode *dn;

	rw = (struct RevealWindow *)sw;

	switch(gad->GadgetID)
	{
		case 0:
			/* the listview gadget */
			rw->rw_CurrentSelection = code;
			break;
		case 1:
			/* the cycle gadet */
			rw->rw_NowShowing = code;
			/* just update this windows list */
			RemakeRevealList(rw);
			break;
		case 2:
			/* the 'show' gadget */
			if( rw->rw_CurrentSelection != 0xFFFF )
			{
				dn = (struct DataNode *)FindNthNode(&rw->rw_Labels, rw->rw_CurrentSelection);
				switch(rw->rw_NowShowing)
				{
					case RWNS_WINDOWS:
						sw = (struct SuperWindow *)dn->dn_Ptr;
						if( sw && !sw->sw_Window )
						{
							(*sw->sw_ShowWindow)(sw);
						}
						break;
					case RWNS_PROJECTS:
						proj = (struct Project *)dn->dn_Ptr;
						/* go through and reshow all windows on this project */
						for(pw = proj->pj_ProjectWindows;
							pw;
							pw = pw->pw_NextProjWindow )
						{
							if(!(pw->pw_sw.sw_Window))
								(*pw->pw_sw.sw_ShowWindow)(&pw->pw_sw);
						}
				}
				/* Refresh all the revealwindow lists */
				RV_RemakeAll();
			}
			break;
	}
}


/************* SortOutRevealWindowMenus() **************/
//
// Handle MENUPICK IDCMP messages - note the reply() is given
// here rather than in the main loop.
// Why is this so? Well... who can tell?
//

static void SortOutRevealWindowMenus(struct SuperWindow *sw,
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


/********** RV_RemakeAll() **********/
//
// Goes through all the currently-visible RevealWindows
// and remakes their lists. This is a global function,
// and should be called whenever superwindows are hidden
// or revealed.
//

void RV_RemakeAll(void)
{
	struct SuperWindow *sw;

	for (sw = (struct SuperWindow *)superwindows.lh_Head;
		sw->sw_Node.ln_Succ;
		sw = (struct SuperWindow *)sw->sw_Node.ln_Succ )
	{
		if( sw->sw_Window && (sw->sw_Type == SWTY_REVEAL))
			RemakeRevealList( (struct RevealWindow *)sw );
	}
}


/************** SetupRevealGadgets **************/
//
// Sets up and formats all the gadgets within the RevealWindow.
// Adapts to font and window sizes.
// Call at window opening, and again at NEWSIZE events.

static void SetupRevealGadgets(struct RevealWindow *rw)
{
	struct Gadget *gad;
	struct NewGadget ng;
	struct Window *win;
	UWORD buttonh;

	win = rw->rw_sw.sw_Window;


	/* first, kill any old gadgets that may be active */
	if(rw->rw_sw.sw_GadList)
	{
		RemoveGList(win, rw->rw_sw.sw_GadList, -1);
		FreeGadgets(rw->rw_sw.sw_GadList);
		rw->rw_sw.sw_GadList = NULL;
	}

	ClearWindow( win );


	/* use window font */
	SetFont(win->RPort,windowtextfont);
	rw->rw_sw.sw_TextAttr = &windowtextattr;
	rw->rw_sw.sw_TextFont = windowtextfont;

	/* fall back to Topaz 8 if the window is too small */
	AdjustWindowFont(&rw->rw_sw,(TextLength(win->RPort,"Projects",8) + 38),
									(windowtextfont->tf_YSize * 5) + 18);

	/* init gadget stuff */
	gad = CreateContext(&rw->rw_sw.sw_GadList);

	buttonh = win->RPort->Font->tf_YSize + 3;

	/* create a ListView gadget */
	ng.ng_TextAttr = rw->rw_sw.sw_TextAttr;
	ng.ng_VisualInfo = rw->rw_sw.sw_VisualInfo;
	ng.ng_LeftEdge = win->BorderLeft + 4;
	ng.ng_TopEdge = win->BorderTop + 2;
	ng.ng_Width = win->Width - win->BorderLeft - win->BorderRight - 8;
	ng.ng_Height = win->Height - win->BorderTop - win->BorderBottom - (buttonh * 2) - 8;
	ng.ng_GadgetText = NULL;
	ng.ng_GadgetID = 0;
	ng.ng_Flags = 0;

	if( !IsListEmpty(&rw->rw_Labels) )
	{
		/* reattach the existing label list */
		gad = CreateGadget( LISTVIEW_KIND, gad, &ng,
			GTLV_Labels,&rw->rw_Labels,
			GTLV_ShowSelected, NULL,
			TAG_END );
	}
	else
	{
		/* haven't yet got a list to attach to it */
		gad = CreateGadget( LISTVIEW_KIND, gad, &ng,
			GTLV_ShowSelected, NULL,
			TAG_END );
	}

	/* want to remember listview gadget... */
	rw->rw_LVGadget = gad;

	/* create Cycle gadget ("Windows/Projects") */
	ng.ng_LeftEdge = win->BorderLeft + 4;
	ng.ng_TopEdge = win->Height - win->BorderBottom - (buttonh * 2) - 4;
	ng.ng_Width = TextLength(win->RPort,"Projects",8) + 30;
	ng.ng_Height = buttonh;
	ng.ng_GadgetID = 1;
	gad = CreateGadget( CYCLE_KIND, gad, &ng,
		GTCY_Labels, cyclelabels,
		GTCY_Active, rw->rw_NowShowing,
		TAG_END );

	/* Create Button Gadget ("Show") */
	ng.ng_LeftEdge = win->BorderLeft + 4;
	ng.ng_TopEdge = win->Height - win->BorderBottom - buttonh - 2;
	ng.ng_Width = TextLength(win->RPort,"Show",4) + 8;
	ng.ng_GadgetText = "Show";
	ng.ng_GadgetID = 2;
	ng.ng_Flags = 0;
	gad = CreateGadget( BUTTON_KIND, gad, &ng, TAG_END );

	/* check for error */
	if (!gad)
	{
		FreeGadgets(rw->rw_sw.sw_GadList);
		return;
	}

	/* attach our groovy new gadgets to the window */
	AddGList(win, rw->rw_sw.sw_GadList, -1, -1, NULL);
//	RefreshGList(rw->rw_sw.sw_GadList, win, NULL, -1);
	GT_RefreshWindow(win, NULL);

	/* need to redraw the frame coz we're not using GimmeZeroZero */
	RefreshWindowFrame(win);
}



/**********  RemakeRevealList  **********/
//
// Creates a list of currently hidden windows or projects.
// If a list is already attached to the RevealWindow, then it
// will be freed automagically.
//


static void RemakeRevealList(struct RevealWindow *rw)
{
	struct DataNode *dn;
	struct SuperWindow *sw;
	struct Project *proj;
	struct ProjectWindow *pw;

	/* only do this if revealwindow itself is visible... */
	if(rw->rw_sw.sw_Window)
	{
		/* detach old labels from the gadget */
		GT_SetGadgetAttrs(rw->rw_LVGadget, rw->rw_sw.sw_Window, NULL,
			GTLV_Labels,~0,TAG_END);

		/* free old label list if it exists */
		FreeRevealList(rw);

		/* create new labels */

		/* check if we want Projects or windows... */
		switch( rw->rw_NowShowing )
		{
			case RWNS_WINDOWS:
				/* Create list of hidden SuperWindows */
				/* can safely assume that superwindows list isn't empty */

				for(sw = (struct SuperWindow *)superwindows.lh_Head;
					sw->sw_Node.ln_Succ;
					sw = (struct SuperWindow *)sw->sw_Node.ln_Succ )
				{
					if(!sw->sw_Window)
					{
						dn = AllocVec( sizeof(struct DataNode), MEMF_CLEAR|MEMF_ANY );
						if(dn)
						{
							dn->dn_Node.ln_Name = sw->sw_Title;
							dn->dn_Ptr = sw;
							AddTail( &rw->rw_Labels, (struct Node *)dn );
						}
					}
				}
				break;

			case RWNS_PROJECTS:
				/* Create list of Projects which have Hidden windows */

				for(proj = (struct Project *)projects.lh_Head;
					proj->pj_Node.ln_Succ;
					proj = (struct Project *)proj->pj_Node.ln_Succ )
				{
					/* find first ProjectWindow */
					pw = proj->pj_ProjectWindows;
					/* look for hidden ones */
					while(pw)
					{
						if( !(pw->pw_sw.sw_Window) )
						{
							/* add project name to label list */
							dn = AllocVec( sizeof(struct DataNode), MEMF_CLEAR|MEMF_ANY );
							if(dn)
							{
								dn->dn_Node.ln_Name = proj->pj_File ? proj->pj_File->fl_Name : "Untitled";
								dn->dn_Ptr = proj;
								AddTail( &rw->rw_Labels, (struct Node *)dn );
							}
							/* don't need to process any more windows on this project */
							pw = NULL;
						}
						else
						{
							/* find next ProjectWindow */
							pw = pw->pw_NextProjWindow;
						}
					}
				}
				break;
		}
		/* 0xFFFF indicates no selection */
		rw->rw_CurrentSelection = 0xFFFF;

		/* attach new labels */
		GT_SetGadgetAttrs(rw->rw_LVGadget, rw->rw_sw.sw_Window, NULL,
			GTLV_Labels,&rw->rw_Labels,
			GTLV_Selected,~0,TAG_END);
	}
}


/**********  FreeRevealList()  **********/
//
// Frees the list of labels (can be either projects
// or windows) attached to a RevealWindow.
// Note: the list should be detached from the listview gadget
// before calling this routine.

static void FreeRevealList(struct RevealWindow *rw)
{
		struct DataNode *dn,*nextdn;

		dn = (struct DataNode *)rw->rw_Labels.lh_Head;
		while( !IsListEmpty(&rw->rw_Labels) )
		{
			nextdn = (struct DataNode *)dn->dn_Node.ln_Succ;
			Remove( (struct Node *)dn );
			FreeVec(dn);
			dn = nextdn;
		}
}
