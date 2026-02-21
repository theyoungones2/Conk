#include <stdio.h>
//#include <stdarg.h>
//#include <string.h>
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

#include <graphics/modeid.h>

#include <global.h>
#include <ExternVars.h>

static struct NewMenu	palmenu[] =
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

/* prototypes local to this source file: */

BOOL ShowPalWindow(struct SuperWindow *mw);
BOOL SetupPalWindowMenus(struct SuperWindow *sw);
void HidePalWindow(struct SuperWindow *sw);
APTR KillOffPalWindow(struct SuperWindow *sw);
void PalWindowHandler(struct SuperWindow *sw, struct IntuiMessage *imsg);
static void SortOutPalWindowMenus(struct SuperWindow *sw,
																	struct IntuiMessage *imsg,
																	UWORD code);
static void SussOutCloseGadget(struct SuperWindow *sw);
static void RethinkPaletteWindow(struct PaletteWindow *pw);
static void DrawPaletteWindowPens(struct PaletteWindow *pw);
static void SussColourPick(struct PaletteWindow *pw, WORD mx, WORD my, UWORD button);
static void SortOutMenuVerify(struct PaletteWindow *pw,WORD mx,WORD my,
											 UWORD code, struct IntuiMessage *imsg);
static void SortOutPalKeys(struct PaletteWindow *pw,UWORD keycode);
//static void DoPalGadgets(struct PaletteWindow *pw);
//static void SortOutPalGadgets(struct PaletteWindow *pw,struct Gadget *gad);
//static void SortOutPalKeys(struct PaletteWindow *pw,
//														UWORD keycode);

/**************** OpenNewPalWindow() ****************/
//
// Creates a new PaletteWindow.
//

BOOL OpenNewPaletteWindow(struct WindowDefault *wf)
{
	struct	PaletteWindow	*pw;

	/* grab mem for the PaletteWindow struct */
	if ( !( pw = AllocVec(sizeof(struct PaletteWindow), MEMF_ANY|MEMF_CLEAR ) ) )
		return(FALSE);

	/* figure out a unique ID for this window (mainly for AREXX use) */
	InstallUniqueWindowID((struct SuperWindow *)pw);
	pw->pw_sw.sw_Node.ln_Name = (char *)&(pw->pw_sw.sw_idstring);

	/* find the Default settings for this type of SuperWindow */
	//wf = &glob.cg_WindowDefaults[SWTY_PALETTE];

	pw->pw_sw.sw_Handler = PalWindowHandler;
	pw->pw_sw.sw_HideWindow = HidePalWindow;
	pw->pw_sw.sw_ShowWindow = ShowPalWindow;
	pw->pw_sw.sw_BuggerOff = KillOffPalWindow;
	pw->pw_sw.sw_RefreshMenus = SetupPalWindowMenus;
	pw->pw_sw.sw_Window = NULL;
	pw->pw_sw.sw_MenuStrip = NULL;
	pw->pw_sw.sw_Type = SWTY_PALETTE;
	pw->pw_sw.sw_VisualInfo = NULL;
	pw->pw_sw.sw_GadList = NULL;

	/* set up stuff for opening the actual intuition window */
	pw->pw_sw.sw_Left = wf->wf_Left;
	pw->pw_sw.sw_Top = wf->wf_Top;
	pw->pw_sw.sw_Width = wf->wf_Width;
	pw->pw_sw.sw_Height = wf->wf_Height;
	pw->pw_sw.sw_MinWidth = 50;
	pw->pw_sw.sw_MinHeight = 50;
	pw->pw_sw.sw_MaxWidth = 0xFFFF;
	pw->pw_sw.sw_MaxHeight = 0xFFFF;
	pw->pw_sw.sw_MenuStrip = NULL;

	pw->pw_sw.sw_FlgBackDrop = wf->wf_FlgBackDrop;
	pw->pw_sw.sw_FlgBorderless = wf->wf_FlgBorderless;
	pw->pw_sw.sw_FlgTitle = wf->wf_FlgTitle;
	pw->pw_sw.sw_FlgBorderGadgets = wf->wf_FlgBorderGadgets;

	pw->pw_sw.sw_Title = "Palette";

	pw->pw_PaletteBox.pb_Palette = globpalette;

	/* add PaletteWindow into the superwindows list */
	AddTail(&superwindows, &pw->pw_sw.sw_Node);

	/* open the intuition window (if the window doesn't open, */
	/* it is just left hidden - SEP). */

	ShowPalWindow(&pw->pw_sw);

	return(TRUE);
}



BOOL ShowPalWindow(struct SuperWindow *sw)
{
	struct PaletteWindow *pw;
	ULONG flags;

	pw = (struct PaletteWindow *)sw;

	if(!sw->sw_Window)				/* make sure it isn't already open */
	{
		/* Set up the Visual Info for GadTools */
		if (!(sw->sw_VisualInfo = GetVisualInfo(MAINSCREEN,TAG_END)))
			return(FALSE);

		//DoPalGadgets(pw);

		flags = WFLG_ACTIVATE | WFLG_NEWLOOKMENUS;
		if(sw->sw_FlgTitle)
			flags = flags | WFLG_DRAGBAR | WFLG_DEPTHGADGET | WFLG_CLOSEGADGET;
		if(sw->sw_FlgBorderGadgets)
			flags = flags | WFLG_SIZEGADGET | WFLG_SIZEBRIGHT;

		sw->sw_Window = OpenWindowTags(NULL,
											WA_Left,				sw->sw_Left,
											WA_Top,					sw->sw_Top,
											WA_Width,				sw->sw_Width,
											WA_Height,			sw->sw_Height,
											WA_MinWidth,		sw->sw_MinWidth,
											WA_MinHeight,		sw->sw_MinHeight,
											WA_MaxWidth,		sw->sw_MaxWidth,
											WA_MaxHeight,		sw->sw_MaxHeight,
											WA_SmartRefresh,TRUE,
											WA_Flags,				flags,
											WA_ReportMouse,	TRUE,
											WA_Backdrop,		sw->sw_FlgBackDrop,
											WA_Borderless,	sw->sw_FlgBorderless,
											WA_CustomScreen, MAINSCREEN,
											WA_Gadgets,			sw->sw_GadList,
											WA_AutoAdjust,	TRUE,
											WA_Title,				sw->sw_FlgTitle ?
												BuildWindowTitle(sw) : NULL,
											TAG_DONE,				NULL);

		if( !(sw->sw_Window)) return(FALSE);
		((struct PaletteWindow *)sw)->pw_PaletteBox.pb_Window = sw->sw_Window;

		/* Tell the window which superwindow it is attached to */
		/* (so we'll know which handler routine to call) */
		sw->sw_Window->UserData = (BYTE *)pw;

		SetupPalWindowMenus(sw);

		/* we want to use our global message port for IDCMP stuff */
		sw->sw_Window->UserPort = mainmsgport;
		ModifyIDCMP(sw->sw_Window, IDCMP_CLOSEWINDOW | IDCMP_MENUPICK |
			IDCMP_CHANGEWINDOW | IDCMP_MOUSEBUTTONS | IDCMP_NEWSIZE | IDCMP_MENUVERIFY |
			IDCMP_ACTIVEWINDOW | IDCMP_RAWKEY | IDCMP_MOUSEMOVE);
		RethinkPaletteWindow(pw);
		lastpalettewindow = pw;

		SetSuperWindowScreenTitle(sw);
	}
}



BOOL SetupPalWindowMenus(struct SuperWindow *sw)
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
	SetupWindowMenu( sw, &palmenu[MN_WINDOW_IDX] );
	SetupEditMenu( NULL, &palmenu[MN_EDIT_IDX] );
	SetupSettingsMenu( &palmenu[MN_SETTINGS_IDX] );
	SetupModeMenu( sw, &palmenu[MN_MODE_IDX] );
	LinkInUserMenu( &palmenu[MN_USER_IDX] );

	if( sw->sw_MenuStrip = CreateMenus( palmenu, TAG_END ) )
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

void HidePalWindow(struct SuperWindow *sw)
{
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

		//if(sw->sw_GadList)
		//{
		//	RemoveGList(sw->sw_Window,&((struct PaletteWindow *)sw)->pw_PropGadget,1);
		//	sw->sw_GadList=NULL;
		//}

		CloseWindowSafely(sw->sw_Window);				/* close it */
		sw->sw_Window = NULL;										/* indicate that it's gone */
		((struct PaletteWindow *)sw)->pw_PaletteBox.pb_Window = NULL;
		if (sw->sw_VisualInfo) FreeVisualInfo(sw->sw_VisualInfo);
		sw->sw_VisualInfo = NULL;
		SetProcessWindowPtr();
		if(lastpalettewindow == (struct PaletteWindow *)sw)
			lastpalettewindow = NULL;
	}
}



APTR KillOffPalWindow(struct SuperWindow *sw)
{
	struct PaletteWindow *pw;

	pw = (struct PaletteWindow *)sw;

	/* close/free the superwindow */
	HidePalWindow(sw);												/* shut down the Intuition window */
	Remove(&(sw->sw_Node));										/* remove from superwindows list */
	FreeVec(pw);															/* free PaletteWindow struct */
	return(NULL);
}


void PalWindowHandler(struct SuperWindow *sw, struct IntuiMessage *imsg)
{
	ULONG	class;
	UWORD	code,qual;
	WORD mx,my;
	struct IntuiMessage *gtimsg;
	struct Gadget *gad;
	struct PaletteWindow *pw;
	struct Window *win;

	pw = (struct PaletteWindow *)sw;

	if( gtimsg = GT_FilterIMsg(imsg) )
	{
		/* read out all we need */
		class = gtimsg->Class;
		code = gtimsg->Code;
		qual = gtimsg->Qualifier;
		mx = gtimsg->MouseX;
		my = gtimsg->MouseY;
		gad = (struct Gadget *)gtimsg->IAddress;
		win = sw->sw_Window;

		GT_PostFilterIMsg(gtimsg);

		switch(class)
		{
			case IDCMP_MOUSEBUTTONS:
				ReplyMsg((struct Message *)imsg);
				if(!(code & IECODE_UP_PREFIX))
					SussColourPick(pw, mx,my,code);
				break;
			case IDCMP_RAWKEY:
				ReplyMsg((struct Message *)imsg);
				SortOutPalKeys(pw,code);
				break;
			case IDCMP_CLOSEWINDOW:
				ReplyMsg((struct Message *)imsg);
				SussOutCloseGadget(sw);
				break;
			case IDCMP_MENUPICK:
				SortOutPalWindowMenus( sw,imsg,code );
				break;
			case IDCMP_CHANGEWINDOW:
				ReplyMsg((struct Message *)imsg);
				break;
			case IDCMP_NEWSIZE:
				ReplyMsg((struct Message *)imsg);
				RethinkPaletteWindow(pw);
				break;
			case IDCMP_MENUVERIFY:
				SortOutMenuVerify(pw,mx,my,code,imsg);
				break;
			case IDCMP_ACTIVEWINDOW:
				ReplyMsg((struct Message *)imsg);
				lastpalettewindow = pw;
				break;
			default:
				ReplyMsg((struct Message *)imsg);
				break;
		}
	}
}


/********** SORT OUT PALETTE MENUS **************/

static void SortOutPalWindowMenus(struct SuperWindow *sw,
													 struct IntuiMessage *imsg,
													 UWORD code)
{
	int i = 0;
	UWORD menunumber[10];
	UWORD menunum,itemnum,subnum;
	UWORD status = 0;

	WaitForSubTaskDone(mainmsgport);

	menunumber[i] = code;
	while ( (i < 10) && ( menunumber[i] != MENUNULL) )
	{
		i++;
		menunumber[i] =
			( ItemAddress( sw->sw_MenuStrip, menunumber[i-1] ) )->NextSelect;
	}
	ReplyMsg((struct Message *)imsg);

	for(i=0;
		( i < 10 ) &&
			( menunumber[i] != MENUNULL ) &&
			!(status & 1);
		i++ )
	{
		menunum = MENUNUM(menunumber[i]);
		itemnum = ITEMNUM(menunumber[i]);
		subnum = SUBNUM(menunumber[i]);

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

/********** SussOutCloseGadget **************/
static void SussOutCloseGadget(struct SuperWindow *sw)
{
	KillOffPalWindow(sw);

	SussOutQuiting(FALSE);
}


/*********************   SortOutMenuVerify   ********************/

static void SortOutMenuVerify(struct PaletteWindow *pw,WORD mx,WORD my,
											 UWORD code, struct IntuiMessage *imsg)
{
	UWORD qual;
	struct Window *win;
	struct Project *pj;
	BOOL foundit = FALSE;

	win = pw->pw_sw.sw_Window;
	qual = imsg->Qualifier;

	if( code == MENUHOT )
	{
		/* the menu event came from this window, so we can cancel it */
		/* if we feel the inclination. */

		/* check mouse position against window boundaries */
		if( (imsg->MouseX >= win->BorderLeft) && (imsg->MouseX < (win->Width-win->BorderRight)) &&
			(imsg->MouseY >= win->BorderTop) && (imsg->MouseY < (win->Height-win->BorderBottom)) &&
			(qual & IEQUALIFIER_RBUTTON) )
		{
			/* RMB down within window - cancel the menu, */
			/* and send the event on as a SELECTDOWN */
			imsg->Code = MENUCANCEL;
			ReplyMsg((struct Message *)imsg);
			if( (mx >= pw->pw_MinX1-2) && (my >= pw->pw_MinY1-2) &&
					(mx <= pw->pw_MaxX3+2) && (my <= pw->pw_MaxY3+2) )
			{
				for(pj = (struct Project *)projects.lh_Head;
						(pj->pj_Node.ln_Succ) && (!foundit);
						pj = (struct Project *)pj->pj_Node.ln_Succ)
				{
					if(globpalette == &pj->pj_Palette)
					{
						OpenNewEditPaletteWindow( &glob.cg_WindowDefaults[SWTY_EDITPALETTE], pj, lmbcol);
						foundit = TRUE;
					}
				}
				if(!foundit)
					GroovyReq("Edit Palette Error","Palette Not Associated With\nAny Currently Open Projects","Okay");
			}
			else
				SussColourPick(pw, mx, my, IECODE_RBUTTON);
		}
		else
		{
			/* mouse outside window - let the menu come up as normal */
			ReplyMsg((struct Message *)imsg);
		}
	}
	else
	{
		/* menu event came from some other window - just let it go */
		ReplyMsg((struct Message *)imsg);
	}
}



static void SortOutPalKeys(struct PaletteWindow *pw,UWORD keycode)
{
	switch (keycode)
	{
		case 0x40:
			break;
/*
		case 0x4C:			/* UP */
			ScrollUp(pw,4,mx,my);
			break;

		case 0x4D:			/* DOWN */
			ScrollDown(pw,4,mx,my);
			break;

		case 0x4F:			/* LEFT */
			ScrollLeft(pw,4,mx,my);
			break;

		case 0x4E:			/* RIGHT */
			ScrollRight(pw,4,mx,my);
			break;
 */
	}

}


static void RethinkPaletteWindow(struct PaletteWindow *pw)
{
	struct Window *win;
	UWORD winwidth,winheight;
	int numofcolours;
	UWORD boxwidth,boxheight;

	win = pw->pw_sw.sw_Window;
	if(win)
	{
		ClearWindow(win);
		pw->pw_PaletteBox.pb_Palette = globpalette;
		winwidth = win->Width - win->BorderLeft - win->BorderRight;
		winheight = win->Height - win->BorderTop - win->BorderBottom;
		numofcolours = globpalette->pl_Count;

		boxwidth = (winwidth / 3) - 2;
		boxheight = 10;

		DrawIndentBox(win,win->BorderLeft+1,win->BorderTop+1,
									win->BorderLeft+boxwidth+1,win->BorderTop+boxheight);

		DrawIndentBox(win,win->BorderLeft+boxwidth + 3,win->BorderTop+1,
									(win->Width) - win->BorderRight - boxwidth - 4,win->BorderTop+boxheight);

		DrawIndentBox(win,(win->Width) - win->BorderRight - boxwidth-2,win->BorderTop+1,
									(win->Width) - win->BorderRight - 2, win->BorderTop+boxheight);

		pw->pw_MinX1 = win->BorderLeft+3;
		pw->pw_MinY1 = win->BorderTop+3;
		pw->pw_MaxX1 = win->BorderLeft+boxwidth - 1;
		pw->pw_MaxY1 = win->BorderTop+boxheight - 2;

		pw->pw_MinX2 = win->BorderLeft+boxwidth + 5;
		pw->pw_MinY2 = win->BorderTop+3;
		pw->pw_MaxX2 = (win->Width) - win->BorderRight - boxwidth - 6;
		pw->pw_MaxY2 = win->BorderTop+boxheight - 2;

		pw->pw_MinX3 = (win->Width) - win->BorderRight - boxwidth;
		pw->pw_MinY3 = win->BorderTop+3;
		pw->pw_MaxX3 = (win->Width) - win->BorderRight - 4;
		pw->pw_MaxY3 = win->BorderTop+boxheight - 2;

		RethinkPaletteBox(&pw->pw_PaletteBox, win->BorderLeft, win->BorderTop+boxheight+2,
											winwidth, winheight-(boxheight+2));

		DrawPaletteWindowPens(pw);
	}
}

static void DrawPaletteWindowPens(struct PaletteWindow *pw)
{
	struct Window *win;
	if(win = pw->pw_sw.sw_Window)
	{
		SetAPen(win->RPort,lmbcol);
		RectFill(win->RPort,pw->pw_MinX1,pw->pw_MinY1,pw->pw_MaxX1,pw->pw_MaxY1);
		BevelColour(&pw->pw_PaletteBox,lmbcol);
		SetAPen(win->RPort,mmbcol);
		RectFill(win->RPort,pw->pw_MinX2,pw->pw_MinY2,pw->pw_MaxX2,pw->pw_MaxY2);
		SetAPen(win->RPort,rmbcol);
		RectFill(win->RPort,pw->pw_MinX3,pw->pw_MinY3,pw->pw_MaxX3,pw->pw_MaxY3);
	}
}

static void SussColourPick(struct PaletteWindow *pw, WORD mx, WORD my, UWORD button)
{
	UWORD col;
	struct PaletteBox *pb;

	pb = &pw->pw_PaletteBox;

	if( (mx < pb->pb_BoxLeft) ||
			(my < pb->pb_BoxTop) ||
			(mx > (pb->pb_BoxLeft + pb->pb_BoxWidth - 1)) ||
			(my > (pb->pb_BoxTop + pb->pb_BoxHeight - 1)) )
		return;

	mx -= pb->pb_BoxLeft;
	my -= pb->pb_BoxTop;

	col = ((mx / pb->pb_ColourWidth) * pb->pb_ColoursHigh) +
				(my / pb->pb_ColourHeight);

	switch( button )
	{
		case IECODE_LBUTTON:
			SetPalettePen(col,1);
			break;
		case IECODE_MBUTTON:
			SetPalettePen(col,2);
			break;
		case IECODE_RBUTTON:
			SetPalettePen(col,3);
			break;
	}
	if(lastprojectwindow && !glob.cg_AutoActivate)
		ActivateWindow(lastprojectwindow->pw_sw.sw_Window);
}

void SetPalettePen(UWORD col, UWORD pen)
{
	UWORD oldlmbcol;

	oldlmbcol = lmbcol;
	switch(pen)
	{
		case 1:
			lmbcol = col;
			break;
		case 2:
			mmbcol = col;
			break;
		case 3:
			rmbcol = col;
			break;
	}
	RethinkPaletteWindows(oldlmbcol);
}

/****************************   RethinkPaletteWindows()   *******************/
//
//  Redisplays the pens of all the palettewindows.

void RethinkPaletteWindows(UWORD oldlmbcol)
{
	struct PaletteWindow *pw;

	for(pw=(struct PaletteWindow *)superwindows.lh_Head;
			pw->pw_sw.sw_Node.ln_Succ;
			pw = (struct PaletteWindow *)pw->pw_sw.sw_Node.ln_Succ)
	{
		if((pw->pw_sw.sw_Type == SWTY_PALETTE) && (pw->pw_sw.sw_Window))
		{
			RestoreColour(&pw->pw_PaletteBox,oldlmbcol);
			DrawPaletteWindowPens(pw);
		}
	}
}

/****************************   RedisplayAllPaletteWindows()   *******************/
//
//  Redisplays all the palette windows. This is to say, that it completely redraws
// every palette window from scratch.

void RedisplayAllPaletteWindows( void )
{
	struct PaletteWindow *pw;

	for(pw=(struct PaletteWindow *)superwindows.lh_Head;
			pw->pw_sw.sw_Node.ln_Succ;
			pw = (struct PaletteWindow *)pw->pw_sw.sw_Node.ln_Succ)
	{
		if( (pw->pw_sw.sw_Type == SWTY_PALETTE) && (pw->pw_sw.sw_Window) )
		{
			RethinkPaletteWindow(pw);
		}
	}	
}
