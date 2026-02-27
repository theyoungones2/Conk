#include <stdio.h>
//#include <stdarg.h>
#include <string.h>
//#include <dos/dos.h>
//#include <dos/dostags.h>
#include <exec/exec.h>
#include <intuition/intuition.h>
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
#include <proto/asl.h>
//#include <clib/alib_protos.h>
#include <proto/gadtools.h>

#include <intuition/gadgetclass.h>
#include <global.h>
#include <ExternVars.h>

static struct NewMenu	configmenu[] =
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

BOOL ShowScreenConfigWindow(struct SuperWindow *sw);
BOOL SetupScreenConfigWindowMenus(struct SuperWindow *sw);
void HideScreenConfigWindow(struct SuperWindow *sw);
APTR KillOffScreenConfigWindow(struct SuperWindow *sw);
void ScreenConfigWindowHandler(struct SuperWindow *sw,
																			struct IntuiMessage *imsg);

static void DoGadgets(struct ScreenConfigWindow *cw);
static void HandleGadgetUp(struct ScreenConfigWindow *cw,
													 struct Gadget *gad,
													 UWORD code);
static void SortOutConfigWindowMenus(struct SuperWindow *sw,
																		 struct IntuiMessage *imsg,
																		 UWORD code);
static void SelectScreenMode(struct ScreenConfigWindow *cw);
static void SelectScreenName(struct ScreenConfigWindow *cw);
static void SelectScreenFont(struct ScreenConfigWindow *cw);
static void SelectWindowFont(struct ScreenConfigWindow *cw);
static void CopyConfig(struct ScreenConfigWindow *cw);
static void FreeConfigCopy(struct ScreenConfigWindow *cw);
static void HandleOKGadget(struct ScreenConfigWindow *cw);
static void InstallNameInfo(struct ScreenConfigWindow *cw);

/* my gadget IDs (also used as array indexes) */

#define MYCY_CREATESCR	0
#define MYBT_SCRNAME		1
#define MYTX_SCRNAME		2
#define MYBT_SCRMODE		3
#define MYTX_SCRMODE		4
#define MYCB_SHANGHAI		5
#define MYBT_SCRFONT		6
#define MYTX_SCRFONT		7
#define MYNM_SCRFONT		8
#define MYBT_WINFONT		9
#define MYTX_WINFONT		10
#define MYNM_WINFONT		11
#define MYBT_OK					12
#define MYBT_CANCEL			13
#define NUM_GADGETS			14

struct ScreenConfigWindow
{
	struct SuperWindow	cw_sw;
	char								cw_TitleString[22];
	struct NameInfo			cw_DisplayIDNameInfo;
	struct Gadget				*cw_Gadgets[NUM_GADGETS];
	struct ScreenConfig	cw_ScrConfig;
};


static UBYTE *cyclelabels[] = {
	(UBYTE *)"Use existing PubScreen",
	(UBYTE *)"Create new PubScreen",
	NULL };



/***************  OpenNewScreenConfigWindow()  ***************/
//
// Open a new ScreenConfigWindow for twiddling various screenmode
// and font settings.
//

BOOL OpenNewScreenConfigWindow(struct WindowDefault *wf)
{
	struct	ScreenConfigWindow	*cw;

	/* grab mem for the ScreenConfigWindow struct */
	if ( !( cw= AllocVec(sizeof(struct ScreenConfigWindow), MEMF_CLEAR ) ) )
		return(FALSE);

	/* figure out a unique ID for this window (mainly for AREXX use) */
	InstallUniqueWindowID((struct SuperWindow *)cw);
	cw->cw_sw.sw_Node.ln_Name = (char *)&(cw->cw_sw.sw_idstring);

	/* find the Default settings for this type of SuperWindow */
	//wf = &glob.cg_WindowDefaults[SWTY_SCREENCONFIG];

	/* set up handler routines and assorted SuperWindow data */
	cw->cw_sw.sw_Handler = ScreenConfigWindowHandler;
	cw->cw_sw.sw_HideWindow = HideScreenConfigWindow;
	cw->cw_sw.sw_ShowWindow = ShowScreenConfigWindow;
	cw->cw_sw.sw_BuggerOff = KillOffScreenConfigWindow;
	cw->cw_sw.sw_RefreshMenus = SetupScreenConfigWindowMenus;
	cw->cw_sw.sw_Window = NULL;
	cw->cw_sw.sw_MenuStrip = NULL;
	cw->cw_sw.sw_GadList = NULL;
	cw->cw_sw.sw_Type = SWTY_SCREENCONFIG;
	cw->cw_sw.sw_VisualInfo = NULL;

	/* set up stuff for opening the actual intuition window */
	cw->cw_sw.sw_Left = wf->wf_Left;
	cw->cw_sw.sw_Top = wf->wf_Top;
	cw->cw_sw.sw_Width = wf->wf_Width;
	cw->cw_sw.sw_Height = wf->wf_Height;
	cw->cw_sw.sw_MinWidth = (22 * 8) + 32 + 16;					/* minwidth set for topaz8 */
	cw->cw_sw.sw_MaxWidth = 0xFFFF;
	cw->cw_sw.sw_MaxHeight = 0XFFFF;
	cw->cw_sw.sw_MenuStrip = NULL;

	cw->cw_sw.sw_FlgBackDrop = wf->wf_FlgBackDrop;
	cw->cw_sw.sw_FlgBorderless = wf->wf_FlgBorderless;
	cw->cw_sw.sw_FlgTitle = wf->wf_FlgTitle;
	cw->cw_sw.sw_FlgBorderGadgets = wf->wf_FlgBorderGadgets;


	cw->cw_sw.sw_Title = "Screen Settings";


	/* read settings from global screenconfig */
	CopyConfig(cw);

	/* add window into the superwindows list */
	AddTail(&superwindows, &cw->cw_sw.sw_Node);

	/* open the intuition window (if the window doesn't open, */
	/* it is just left hidden - SEP). */
	ShowScreenConfigWindow(&cw->cw_sw);

	return(TRUE);
}



/************ ShowScreenConfigWindow ************/
//
// Display a ScreenConfigWindow.
//

BOOL ShowScreenConfigWindow(struct SuperWindow *sw)
{
	struct ScreenConfigWindow *cw;
	UWORD  minh;
	ULONG flags;
	cw = (struct ScreenConfigWindow *)sw;

	if(!sw->sw_Window)				/* make sure it isn't already open */
	{
		/* Set up the Visual Info for GadTools */
		if (!(cw->cw_sw.sw_VisualInfo = GetVisualInfo(MAINSCREEN,TAG_END)))
			return(FALSE);

		minh = MAINSCREEN->Font->ta_YSize + 1 + MAINSCREEN->WBorTop + 11 +
			((8 * 7) + 16 + 21 + 16);

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

		/* Gadgetsussing routine */
		DoGadgets(cw);

		/* Tell the window which superwindow it is attached to */
		/* (so we'll know which handler routine to call) */
		sw->sw_Window->UserData = (BYTE *)cw;

		SetupScreenConfigWindowMenus(sw);

		/* we want to use our global message port for IDCMP stuff */
		sw->sw_Window->UserPort = mainmsgport;
		ModifyIDCMP(sw->sw_Window, IDCMP_CLOSEWINDOW | IDCMP_MENUPICK | IDCMP_NEWSIZE |
			IDCMP_REFRESHWINDOW | BUTTONIDCMP | TEXTIDCMP | IDCMP_CHANGEWINDOW |
			IDCMP_MOUSEMOVE);

		SetSuperWindowScreenTitle(sw);
	}
	return(TRUE);
}


BOOL SetupScreenConfigWindowMenus(struct SuperWindow *sw)
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
	SetupWindowMenu( sw, &configmenu[MN_WINDOW_IDX] );
	SetupEditMenu( NULL, &configmenu[MN_EDIT_IDX] );
	SetupSettingsMenu( &configmenu[MN_SETTINGS_IDX] );
	SetupModeMenu( sw, &configmenu[MN_MODE_IDX] );
	LinkInUserMenu( &configmenu[MN_USER_IDX] );

	if( sw->sw_MenuStrip = CreateMenus( configmenu, TAG_END ) )
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


/******************  HideScreenConfigWindow()  ******************/
//
// Hide a ScreenConfigWindow.
//

void HideScreenConfigWindow(struct SuperWindow *sw)
{
	struct ScreenConfigWindow *cw;

	cw = (struct ScreenConfigWindow *)sw;

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

		FreeGadgets(cw->cw_sw.sw_GadList);
		cw->cw_sw.sw_GadList = NULL;
		if (sw->sw_VisualInfo) FreeVisualInfo(sw->sw_VisualInfo);
		sw->sw_VisualInfo = NULL;
		SetProcessWindowPtr();
	}
}


/******************  KillOffScreenConfigWindow()  ******************/
//
// #include <stdcommentcrap.h>
//

APTR KillOffScreenConfigWindow(struct SuperWindow *sw)
{
	struct ScreenConfigWindow *cw;

	cw = (struct ScreenConfigWindow *)sw;

	FreeConfigCopy( cw );

	/* close/free the superwindow */
//	if (sw->sw_VisualInfo) FreeVisualInfo(sw->sw_VisualInfo);
	HideScreenConfigWindow(sw);									/* shut down the Intuition window */
	Remove(&(sw->sw_Node));											/* remove from superwindows list */
	FreeVec(cw);
	return(NULL);
}

/******************  ScreenConfigWindowHandler  ******************/
//
// Event handler for ScreenConfigWindow.
// Uses gadtools filtering.
//

void ScreenConfigWindowHandler(struct SuperWindow *sw,
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
				KillOffScreenConfigWindow(sw);
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
				HandleGadgetUp( (struct ScreenConfigWindow *)sw, gad, code);
				break;
			case IDCMP_MENUPICK:
				/* menuroutine does the reply() */
				SortOutConfigWindowMenus( sw,imsg,code );
				break;
			case IDCMP_NEWSIZE:
				/* resuss all the gadget placements/font stuff */
				/* according to the new windowsize. */
				DoGadgets((struct ScreenConfigWindow *)sw);
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



/***************  HandleGadgetUp()  ***************/
//
// Handle GADGETUP messages - see which clickything has been
// hit, and take action.

static void HandleGadgetUp(struct ScreenConfigWindow *cw,
	struct Gadget *gad, UWORD code)
{
	struct Window *win;

	win = cw->cw_sw.sw_Window;

	switch(gad->GadgetID)
	{
		case MYCY_CREATESCR:
			/* "create new pubscreen/use existing pubscreen" cyclegadget */

			GT_SetGadgetAttrs(cw->cw_Gadgets[MYBT_SCRNAME], win, NULL,
				GA_Disabled, code, TAG_END);
			GT_SetGadgetAttrs(cw->cw_Gadgets[MYBT_SCRFONT], win, NULL,
				GA_Disabled, !code, TAG_END);
			GT_SetGadgetAttrs(cw->cw_Gadgets[MYBT_SCRMODE], win, NULL,
				GA_Disabled, !code, TAG_END);
			GT_SetGadgetAttrs(cw->cw_Gadgets[MYCB_SHANGHAI], win, NULL,
				GA_Disabled, !code, TAG_END);

			cw->cw_ScrConfig.sg_OwnScreen = code;
			break;
		case MYBT_SCRNAME:
			SelectScreenName(cw);
			break;
		case MYCB_SHANGHAI:
			cw->cw_ScrConfig.sg_Shanghai = code;
			break;
		case MYBT_SCRMODE:
			SelectScreenMode(cw);
			break;
		case MYBT_SCRFONT:
			SelectScreenFont(cw);
			break;
		case MYBT_WINFONT:
			SelectWindowFont(cw);
			break;
		case MYBT_CANCEL:
			KillOffScreenConfigWindow(&cw->cw_sw);
			break;
		case MYBT_OK:
			HandleOKGadget(cw);
			break;
	}
}


/************* SortOutConfigWindowMenus() **************/
//
// Handle MENUPICK IDCMP messages - note the reply() is given
// here rather than in the main loop.
// Why is this so? Well... who can tell?
//

static void SortOutConfigWindowMenus(struct SuperWindow *sw,
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
	/* Has a window refresh been requested? */
	if (status & 2)
	{
		(*sw->sw_HideWindow)(sw);
		(*sw->sw_ShowWindow)(sw);
	}
}


/***************  DoGadgets()  ***************/
//
//  Format and set up all the gadtools gadgets in
//  a ScreenConfigWindow. All are stretchable.
//

static void DoGadgets(struct ScreenConfigWindow *cw)
{
	struct Gadget *gad;
	struct NewGadget ng;
	struct Window *win;
	UWORD buttonw, buttonh, textw, numw;

	win = cw->cw_sw.sw_Window;

	/* first, kill any old gadgets that may be active */
	if(cw->cw_sw.sw_GadList)
	{
		RemoveGList(win, cw->cw_sw.sw_GadList, -1);
		FreeGadgets(cw->cw_sw.sw_GadList);
		cw->cw_sw.sw_GadList = NULL;
	}

	ClearWindow( win );

	/* use window font */
	SetFont(win->RPort, windowtextfont);
	cw->cw_sw.sw_TextAttr = &windowtextattr;
	cw->cw_sw.sw_TextFont = windowtextfont;

	/* fall back to Topaz 8 if the window is too small */
	AdjustWindowFont(&cw->cw_sw,(TextLength(win->RPort, "Use existing PubScreen", 22) +
		32 + win->BorderLeft + win->BorderRight ),
		(windowtextfont->tf_YSize * 7) + 16 + 21 + 16);

	/* init gadget stuff */
	gad = CreateContext(&cw->cw_sw.sw_GadList);

	buttonw = TextLength(win->RPort,"Use screen...",13) + 8;
	buttonh = win->RPort->Font->tf_YSize + 3;
	textw = win->Width - win->BorderLeft - 4 -
		( win->BorderLeft + 8 + buttonw );

	/* create CycleGadget */
	ng.ng_TextAttr = cw->cw_sw.sw_TextAttr;
	ng.ng_VisualInfo = cw->cw_sw.sw_VisualInfo;
	ng.ng_LeftEdge = win->BorderLeft + 4;
	ng.ng_TopEdge = win->BorderTop + 2;
	ng.ng_Width = TextLength(win->RPort, "Use existing PubScreen", 22) + 32;
	ng.ng_Height = buttonh;
	ng.ng_GadgetText = NULL;
	ng.ng_GadgetID = MYCY_CREATESCR;
	ng.ng_Flags = 0;
	gad = CreateGadget( CYCLE_KIND, gad, &ng,
		GTCY_Labels, cyclelabels,
		GTCY_Active, cw->cw_ScrConfig.sg_OwnScreen,
		TAG_END );

	cw->cw_Gadgets[MYCY_CREATESCR] = gad;

	/* create ButtonGadget "Use screen... " */
	ng.ng_LeftEdge = win->BorderLeft + 4;
	ng.ng_TopEdge = ng.ng_TopEdge + buttonh + 2;
	ng.ng_Width = buttonw;
	ng.ng_Height = buttonh;
	ng.ng_GadgetText = "Use screen...";
	ng.ng_GadgetID = MYBT_SCRNAME;
	ng.ng_Flags = 0;
	gad = CreateGadget( BUTTON_KIND, gad, &ng,
		GA_Disabled, cw->cw_ScrConfig.sg_OwnScreen,
		TAG_END );
	cw->cw_Gadgets[MYBT_SCRNAME] = gad;

	/* text gadget to display screen name */
	ng.ng_LeftEdge = win->BorderLeft + 8 + buttonw;
	ng.ng_Width = textw;
	ng.ng_GadgetText = NULL;
	ng.ng_GadgetID = MYTX_SCRNAME;
	ng.ng_Flags = 0;

	gad = CreateGadget( TEXT_KIND, gad, &ng,
		GTTX_Border,	TRUE,
		GTTX_Text,		cw->cw_ScrConfig.sg_BorrowedName,
		GA_Disabled,	cw->cw_ScrConfig.sg_OwnScreen,
		TAG_END );
	cw->cw_Gadgets[MYTX_SCRNAME] = gad;

	/* create ButtonGadget "ScreenMode... " */
	ng.ng_LeftEdge = win->BorderLeft + 4;
	ng.ng_TopEdge = ng.ng_TopEdge + buttonh + 2;
	ng.ng_Width = buttonw;
	ng.ng_Height = buttonh;
	ng.ng_GadgetText = "ScreenMode...";
	ng.ng_GadgetID = MYBT_SCRMODE;
	ng.ng_Flags = 0;
	gad = CreateGadget( BUTTON_KIND, gad, &ng,
		GA_Disabled, !cw->cw_ScrConfig.sg_OwnScreen,
		TAG_END );
	cw->cw_Gadgets[MYBT_SCRMODE] = gad;

	/* text gadget to display screen mode */
	ng.ng_LeftEdge = win->BorderLeft + 8 + buttonw;
	ng.ng_Width = textw;
	ng.ng_GadgetText = NULL;
	ng.ng_GadgetID = MYTX_SCRMODE;
	ng.ng_Flags = 0;

	InstallNameInfo(cw);

	gad = CreateGadget( TEXT_KIND, gad, &ng,
		GTTX_Border, TRUE,
		GTTX_Text, cw->cw_DisplayIDNameInfo.Name,
		GA_Disabled, !cw->cw_ScrConfig.sg_OwnScreen,
		TAG_END );
	cw->cw_Gadgets[MYTX_SCRMODE] = gad;

	/* shanghai checkbox */
	ng.ng_LeftEdge = win->Width - win->BorderRight - 4 - 26;
	ng.ng_TopEdge = ng.ng_TopEdge + buttonh + 2;
	ng.ng_Width = buttonw;
	ng.ng_Height = buttonh;
	ng.ng_GadgetText = "'Shanghai' windows";
	ng.ng_GadgetID = MYCB_SHANGHAI;
	ng.ng_Flags = PLACETEXT_LEFT;
	gad = CreateGadget( CHECKBOX_KIND, gad, &ng,
		GTCB_Checked, cw->cw_ScrConfig.sg_Shanghai,
		TAG_END );
	cw->cw_Gadgets[MYCB_SHANGHAI] = gad;

	/* create ButtonGadget "ScreenFont... " */
	ng.ng_LeftEdge = win->BorderLeft + 4;
	ng.ng_TopEdge = ng.ng_TopEdge + buttonh + 2 + 8;
	ng.ng_Width = buttonw;
	ng.ng_Height = buttonh;
	ng.ng_GadgetText = "ScreenFont...";
	ng.ng_GadgetID = MYBT_SCRFONT;
	ng.ng_Flags = 0;
	gad = CreateGadget( BUTTON_KIND, gad, &ng,
		GA_Disabled, !cw->cw_ScrConfig.sg_OwnScreen,
		TAG_END );
	cw->cw_Gadgets[MYBT_SCRFONT] = gad;

	numw = TextLength(win->RPort,"WW",2) +8;
	textw = textw - numw - 2;

	/* text gadget to display screen font */
	ng.ng_LeftEdge = win->BorderLeft + 8 + buttonw;
	ng.ng_Width = textw;
	ng.ng_GadgetText = NULL;
	ng.ng_GadgetID = MYTX_SCRFONT;
	ng.ng_Flags = 0;

	gad = CreateGadget( TEXT_KIND, gad, &ng,
		GTTX_Border, TRUE,
		GTTX_Text, cw->cw_ScrConfig.sg_SFNameBuf,
		TAG_END );
	cw->cw_Gadgets[MYTX_SCRFONT] = gad;

	/* number gadget to display screen font size */
	ng.ng_LeftEdge = win->BorderLeft + 8 + buttonw + textw + 2;
	ng.ng_Width = numw;
	ng.ng_GadgetText = NULL;
	ng.ng_GadgetID = MYNM_SCRFONT;
	ng.ng_Flags = 0;

	gad = CreateGadget( NUMBER_KIND, gad, &ng,
		GTNM_Border, TRUE,
		GTNM_Number, cw->cw_ScrConfig.sg_SFYSize,
		TAG_END );
	cw->cw_Gadgets[MYNM_SCRFONT] = gad;


	/* create ButtonGadget "WindowFont... " */
	ng.ng_LeftEdge = win->BorderLeft + 4;
	ng.ng_TopEdge = ng.ng_TopEdge + buttonh + 2;
	ng.ng_Width = buttonw;
	ng.ng_Height = buttonh;
	ng.ng_GadgetText = "WindowFont...";
	ng.ng_GadgetID = MYBT_WINFONT;
	ng.ng_Flags = 0;
	gad = CreateGadget( BUTTON_KIND, gad, &ng, TAG_END );
	cw->cw_Gadgets[MYBT_WINFONT] = gad;

	/* text gadget to display window font name  */
	ng.ng_LeftEdge = win->BorderLeft + 8 + buttonw;
	ng.ng_Width = textw;
	ng.ng_GadgetText = NULL;
	ng.ng_GadgetID = MYTX_WINFONT;
	ng.ng_Flags = 0;

	gad = CreateGadget( TEXT_KIND, gad, &ng,
		GTTX_Border, TRUE,
		GTTX_Text, cw->cw_ScrConfig.sg_WFNameBuf,
		TAG_END );
	cw->cw_Gadgets[MYTX_WINFONT] = gad;

	/* number gadget to display window font size */
	ng.ng_LeftEdge = win->BorderLeft + 8 + buttonw + textw + 2;
	ng.ng_Width = numw;
	ng.ng_GadgetText = NULL;
	ng.ng_GadgetID = MYNM_WINFONT;
	ng.ng_Flags = 0;

	gad = CreateGadget( NUMBER_KIND, gad, &ng,
		GTNM_Border, TRUE,
		GTNM_Number, cw->cw_ScrConfig.sg_WFYSize,
		TAG_END );
	cw->cw_Gadgets[MYNM_WINFONT] = gad;

	/* create ButtonGadget "OK" */
	ng.ng_LeftEdge = win->BorderLeft + 4;
	ng.ng_TopEdge = win->Height - win->BorderBottom - 2 - buttonh;
	ng.ng_Width = TextLength(win->RPort, "Cancel", 6) + 8;
	ng.ng_Height = buttonh;
	ng.ng_GadgetText = "OK";
	ng.ng_GadgetID = MYBT_OK;
	ng.ng_Flags = 0;
	gad = CreateGadget( BUTTON_KIND, gad, &ng, TAG_END );
	cw->cw_Gadgets[MYBT_OK] = gad;

	/* create ButtonGadget "Cancel" */
	ng.ng_LeftEdge = win->Width - win->BorderRight -
		TextLength(win->RPort, "Cancel", 6) - 8 - 4;
//	ng.ng_Width = TextLength(win->RPort, "Cancel", 6) + 8;
	ng.ng_GadgetText = "Cancel";
	ng.ng_GadgetID = MYBT_CANCEL;
	ng.ng_Flags = 0;
	gad = CreateGadget( BUTTON_KIND, gad, &ng, TAG_END );
	cw->cw_Gadgets[MYBT_CANCEL] = gad;


	/* check for error */
	if (!gad)
	{
		FreeGadgets(cw->cw_sw.sw_GadList);
		return;
	}

	/* attach our groovy new gadgets to the window */
	AddGList(win, cw->cw_sw.sw_GadList, -1, -1, NULL);
	GT_RefreshWindow(win, NULL);

	/* need to redraw the frame coz we're not using GimmeZeroZero */
	RefreshWindowFrame(win);
}


/***************  SelectScreenMode()  ***************/
//
//  Bring up a screenmode requester and copy new
//  settings out into the ScreenConfigWindow struct.
//

static void SelectScreenMode(struct ScreenConfigWindow *cw)
{
	BOOL	pleb;

	if( BlockAllSuperWindows() )
	{
		/* Call up the ASL screenmode requester (wb2.1+ only...) */
		/* gotta suss out an alternative too... */

		pleb = AslRequestTags(screenmodereq,
			ASLSM_Window,								cw->cw_sw.sw_Window,
			ASLSM_InitialDisplayID,			cw->cw_ScrConfig.sg_DisplayID,
			ASLSM_InitialDisplayWidth,	cw->cw_ScrConfig.sg_DisplayWidth,
			ASLSM_InitialDisplayHeight,	cw->cw_ScrConfig.sg_DisplayHeight,
			ASLSM_InitialDisplayDepth,	cw->cw_ScrConfig.sg_DisplayDepth,
			ASLSM_InitialOverscanType,	cw->cw_ScrConfig.sg_OverscanType,
			ASLSM_InitialAutoScroll,		cw->cw_ScrConfig.sg_AutoScroll,
			TAG_END );
		UnblockAllSuperWindows();
		if (pleb)
		{

			/* Copy out all the groovy new userselected screenmode data */
			/* into our local ScreenConfig struct */

			cw->cw_ScrConfig.sg_DisplayID = screenmodereq->sm_DisplayID;
			cw->cw_ScrConfig.sg_DisplayWidth = screenmodereq->sm_DisplayWidth;
			cw->cw_ScrConfig.sg_DisplayHeight = screenmodereq->sm_DisplayHeight;
			cw->cw_ScrConfig.sg_DisplayDepth = screenmodereq->sm_DisplayDepth;
			cw->cw_ScrConfig.sg_OverscanType = screenmodereq->sm_OverscanType;
			cw->cw_ScrConfig.sg_AutoScroll = screenmodereq->sm_AutoScroll;

			/* show the new screenmode name */
			InstallNameInfo(cw);
			GT_SetGadgetAttrs(cw->cw_Gadgets[MYTX_SCRMODE],cw->cw_sw.sw_Window, NULL,
				GTTX_Text,		cw->cw_DisplayIDNameInfo.Name,
				TAG_END);
		}
	}
}


/***************  SelectScreenName  ***************/
//
// Bring up a PubScreen requester (home brew) to select
// an Existing PubScreen.
//

static void SelectScreenName(struct ScreenConfigWindow *cw)
{
	if( BlockAllSuperWindows() )
	{
		PubScreenRequester(cw->cw_sw.sw_Window, cw->cw_ScrConfig.sg_BorrowedName);
		UnblockAllSuperWindows();
	}
	GT_SetGadgetAttrs(cw->cw_Gadgets[MYTX_SCRNAME],cw->cw_sw.sw_Window, NULL,
		GTTX_Text,		cw->cw_ScrConfig.sg_BorrowedName,
		TAG_END);
}


/***************  SelectScreenFont()  ***************/
//
// Bring up a font requester to install a new screenfont.
//

static void SelectScreenFont(struct ScreenConfigWindow *cw)
{
	BOOL pleb;

	if( BlockAllSuperWindows())
	{
		pleb = AslRequestTags(fontreq,
			ASLFO_TextAttr,			&windowtextattr,
			ASLFO_Window,				cw->cw_sw.sw_Window,
	//		ASLFO_SleepWindow,	TRUE,
			ASLFO_DoStyle,			TRUE,
			ASLFO_InitialName,	cw->cw_ScrConfig.sg_SFNameBuf,
			ASLFO_InitialSize,	cw->cw_ScrConfig.sg_SFYSize,
			TAG_END );
		UnblockAllSuperWindows();
		if(pleb)
		{
			/* free the old fontname */
	//		if(cw->cw_ScrConfig.sg_ScreenTextAttr.ta_Name)
	//			FreeVec(cw->cw_ScrConfig.sg_ScreenTextAttr.ta_Name);

			/* install the new fontattr in the ScreenConfigWindow struct */

			Mystrncpy( cw->cw_ScrConfig.sg_SFNameBuf, fontreq->fo_Attr.ta_Name,
				MYMAXFONTNAMESIZE );
			cw->cw_ScrConfig.sg_SFYSize = fontreq->fo_Attr.ta_YSize;
			cw->cw_ScrConfig.sg_SFStyle = fontreq->fo_Attr.ta_Style;
			cw->cw_ScrConfig.sg_SFFlags = fontreq->fo_Attr.ta_Flags;

			/* display the new font name and size */
			GT_SetGadgetAttrs(cw->cw_Gadgets[MYTX_SCRFONT],cw->cw_sw.sw_Window, NULL,
				GTTX_Text,		cw->cw_ScrConfig.sg_SFNameBuf,
				TAG_END);
			GT_SetGadgetAttrs(cw->cw_Gadgets[MYNM_SCRFONT],cw->cw_sw.sw_Window, NULL,
				GTNM_Number,		cw->cw_ScrConfig.sg_SFYSize,
				TAG_END);
		}
	}
}



/***************  SelectWindowFont()  ***************/
//
// Bring up a font requester to install a new windowfont.
//


static void SelectWindowFont(struct ScreenConfigWindow *cw)
{
	BOOL pleb;

	if( BlockAllSuperWindows() )
	{
		pleb = AslRequestTags(fontreq,
			ASLFO_TextAttr,			&windowtextattr,
			ASLFO_Window,				cw->cw_sw.sw_Window,
			ASLFO_DoStyle,			TRUE,
			ASLFO_InitialName,	cw->cw_ScrConfig.sg_WFNameBuf,
			ASLFO_InitialSize,	cw->cw_ScrConfig.sg_WFYSize,
			TAG_END );
		UnblockAllSuperWindows();
		if(pleb)
		{

			/* install it */
	//		if(cw->cw_ScrConfig.sg_WindowTextAttr.ta_Name)
	//			FreeVec(cw->cw_ScrConfig.sg_WindowTextAttr.ta_Name);

			Mystrncpy( cw->cw_ScrConfig.sg_WFNameBuf, fontreq->fo_Attr.ta_Name,
				MYMAXFONTNAMESIZE );
			cw->cw_ScrConfig.sg_WFYSize = fontreq->fo_Attr.ta_YSize;
			cw->cw_ScrConfig.sg_WFStyle = fontreq->fo_Attr.ta_Style;
			cw->cw_ScrConfig.sg_WFFlags = fontreq->fo_Attr.ta_Flags;

			GT_SetGadgetAttrs(cw->cw_Gadgets[MYTX_WINFONT],cw->cw_sw.sw_Window, NULL,
				GTTX_Text,		cw->cw_ScrConfig.sg_WFNameBuf,
				TAG_END);
			GT_SetGadgetAttrs(cw->cw_Gadgets[MYNM_WINFONT],cw->cw_sw.sw_Window, NULL,
				GTNM_Number,		cw->cw_ScrConfig.sg_WFYSize,
				TAG_END);
		}
	}
}


/**************  CopyConfig()  **************/
//
// Copy the global screen config parameters out into local storage
// in the ScreenConfigWindow struct. Then we can play about with
// the params as much as we want before copying them back out into
// the global store (or not... if we hit cancel).
//

static void CopyConfig(struct ScreenConfigWindow *cw)
{
	/* groovy to the max! */
	cw->cw_ScrConfig = glob.cg_ScrConfig;						/* ZAP! */

	/* gotta make up new namespaces */
//	cw->cw_ScrConfig.sg_ScreenTextAttr.ta_Name =
//		MyStrDup( glob.cg_ScrConfig.sg_ScreenTextAttr.ta_Name );
//	cw->cw_ScrConfig.sg_WindowTextAttr.ta_Name =
//		MyStrDup( glob.cg_ScrConfig.sg_WindowTextAttr.ta_Name );

}


/**************  FreeConfigCopy()  **************/
//
// Frees anything that needs freeing of the data set up by CopyConfig().
//

static void FreeConfigCopy(struct ScreenConfigWindow *cw)
{
	/* free mem used to store screenfont name */
//	if (cw->cw_ScrConfig.sg_ScreenTextAttr.ta_Name)
//	{
//		FreeVec(cw->cw_ScrConfig.sg_ScreenTextAttr.ta_Name);
//		cw->cw_ScrConfig.sg_ScreenTextAttr.ta_Name = NULL;
//	}

//	/* free mem used to store windowfont name */
//	if (cw->cw_ScrConfig.sg_WindowTextAttr.ta_Name)
//	{
//		FreeVec(cw->cw_ScrConfig.sg_WindowTextAttr.ta_Name);
//		cw->cw_ScrConfig.sg_WindowTextAttr.ta_Name = NULL;
//	}
}


	
/**************  HandleOKGadget()  **************/
//
// The (l)user has clicked on the OK gadget, so we've got to
// set up the chosen screen configuration.
//

static void HandleOKGadget(struct ScreenConfigWindow *cw)
{
	/* Hide the config window */
//	HideScreenConfigWindow(&cw->cw_sw);

	/* Use the new settings */
	if( UseNewScreenSettings(&cw->cw_ScrConfig) )
	{
		/* Kill the window. Got to do this last 'cos all our groovy */
		/* new configdata is attached to the SuperWindow. */
		//KillOffScreenConfigWindow(&cw->cw_sw);
		//RV_RemakeAll();
	}
	else
		CloseAllSuperWindows();
}


/*****************  InstallNameInfo  *****************/
//
// The routine probably does something important but I can't
// be bothered trying to figure out what that is exactly.
// Most likely it installs a Name. Or an Info. Or both.
//
// Or something.
//

static void InstallNameInfo(struct ScreenConfigWindow *cw)
{
	GetDisplayInfoData(NULL,(UBYTE *)&cw->cw_DisplayIDNameInfo,sizeof(struct NameInfo),
                DTAG_NAME,cw->cw_ScrConfig.sg_DisplayID);
}
