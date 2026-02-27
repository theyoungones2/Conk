//#include <stdio.h>
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
//#include <proto/graphics.h>
//#include <proto/diskfont.h>
//#include <proto/asl.h>
//#include <clib/alib_protos.h>
#include <proto/gadtools.h>

#include <global.h>
#include <ExternVars.h>

static struct NewMenu	aboutmenu[] =
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

/* 75 */
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

/* 98 */
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

/* 112 */
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

/* 122 */
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

/* 150 */
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

BOOL ShowAboutWindow(struct SuperWindow *sw);
BOOL SetupAboutWindowMenus(struct SuperWindow *sw);
void HideAboutWindow(struct SuperWindow *sw);
APTR KillOffAboutWindow(struct SuperWindow *sw);
void AboutWindowHandler(struct SuperWindow *sw,	struct IntuiMessage *imsg);
void SortOutAboutWindowMenus(struct SuperWindow *sw,
													 struct IntuiMessage *imsg,
													 UWORD code);


/*************  OpenNewAboutWindow()  *************/
//
//  This routine provides a transitional path along which
//  program flow occurs in a local-linear fashion. The initial
//  state is one where there are no AboutWindows, or to cover
//  all situations, the total sum of AboutWindows which have
//  passed through this transition stage from the initial state
//  of not being to a later stage of being should be one less than
//  the number existing after the transitional line has been passed,
//  although other factors may complicate matters. Generally
//  speaking, the state of postexistance existing after the
//  preexistant state will be, assuming favourable conditions
//  exist, a different state of existance. Usually, if the
//  prestates and poststates match with the statechanges required
//  at a given flow control state then this function is probably
//  the one you want to use. If the two state conditions differ
//  significantly for the given state, then you will have to use
//  another function to provide the necessary state transitional
//  operations. This is really just restatement of the obvious.

BOOL OpenNewAboutWindow(struct WindowDefault *wf)
{
	struct	AboutWindow	*aw;

	/* grab mem for the AboutWindow struct */
	if ( !( aw= AllocVec(sizeof(struct AboutWindow), MEMF_CLEAR ) ) )
		return(FALSE);

	/* figure out a unique ID for this window (mainly for AREXX use) */
	InstallUniqueWindowID((struct SuperWindow *)aw);
	aw->aw_sw.sw_Node.ln_Name = (char *)&(aw->aw_sw.sw_idstring);

	/* find the Default settings for this type of SuperWindow */
	//wf = &glob.cg_WindowDefaults[SWTY_ABOUT];

	aw->aw_sw.sw_Handler = AboutWindowHandler;
	aw->aw_sw.sw_HideWindow = HideAboutWindow;
	aw->aw_sw.sw_ShowWindow = ShowAboutWindow;
	aw->aw_sw.sw_BuggerOff = KillOffAboutWindow;
	aw->aw_sw.sw_RefreshMenus = SetupAboutWindowMenus;
	aw->aw_sw.sw_Window = NULL;
	aw->aw_sw.sw_MenuStrip = NULL;
	aw->aw_sw.sw_Type = SWTY_ABOUT;
	aw->aw_sw.sw_VisualInfo = NULL;
	aw->aw_sw.sw_GadList = NULL;

	/* set up stuff for opening the actual intuition window */
	aw->aw_sw.sw_Left = wf->wf_Left;
	aw->aw_sw.sw_Top = wf->wf_Top;
	aw->aw_sw.sw_Width = wf->wf_Width;
	aw->aw_sw.sw_Height = wf->wf_Height;
	aw->aw_sw.sw_MinWidth = 50;
	aw->aw_sw.sw_MinHeight = 50;
	aw->aw_sw.sw_MaxWidth = 0xFFFF;
	aw->aw_sw.sw_MaxHeight = 0XFFFF;
	aw->aw_sw.sw_MenuStrip = NULL;

	aw->aw_sw.sw_FlgBackDrop = wf->wf_FlgBackDrop;
	aw->aw_sw.sw_FlgBorderless = wf->wf_FlgBorderless;
	aw->aw_sw.sw_FlgTitle = wf->wf_FlgTitle;
	aw->aw_sw.sw_FlgBorderGadgets = wf->wf_FlgBorderGadgets;

	/* setup SuperWindow base title */
	aw->aw_sw.sw_Title = Txt_About;

	/* add window into the superwindows list */
	AddTail(&superwindows, &aw->aw_sw.sw_Node);

	/* open the intuition window (if the window doesn't open, */
	/* it is just left hidden - SEP). */

	ShowAboutWindow(&aw->aw_sw);

	return(TRUE);
}



BOOL ShowAboutWindow(struct SuperWindow *sw)
{
	struct AboutWindow *aw;
	struct Gadget *gad;
	struct NewGadget ng;
	ULONG flags;

	aw = (struct AboutWindow *) sw;

	if(!sw->sw_Window)				/* make sure it isn't already open */
	{
		/* Set up the Visual Info for GadTools */
		if (!(aw->aw_sw.sw_VisualInfo = GetVisualInfo(MAINSCREEN,TAG_END)))
			return(FALSE);

		sw->sw_GadList = NULL;
		gad = CreateContext(&sw->sw_GadList);
		ng.ng_TextAttr = NULL;
		ng.ng_VisualInfo = sw->sw_VisualInfo;
		ng.ng_LeftEdge = 20;
		ng.ng_TopEdge = 50;
		ng.ng_Width = 50;
		ng.ng_Height = 12;
		ng.ng_GadgetText = Txt_TestGad;
		ng.ng_GadgetID = 0;
		ng.ng_Flags = 0;

		gad = CreateGadget( BUTTON_KIND, gad, &ng, TAG_END );

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
											WA_MinHeight,		sw->sw_MinHeight,
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
											WA_Gadgets,			sw->sw_GadList,
											TAG_DONE,				NULL);

		if( !(sw->sw_Window)) return(FALSE);

		GT_RefreshWindow( sw->sw_Window, NULL );

		/* Tell the window which superwindow it is attached to */
		/* (so we'll know which handler routine to call) */
		sw->sw_Window->UserData = (BYTE *)aw;

		SetupAboutWindowMenus(sw);

		/* we want to use our global message port for IDCMP stuff */
		sw->sw_Window->UserPort = mainmsgport;
		ModifyIDCMP(sw->sw_Window, IDCMP_CLOSEWINDOW | IDCMP_MENUPICK |
			IDCMP_REFRESHWINDOW | BUTTONIDCMP | IDCMP_CHANGEWINDOW | IDCMP_MOUSEMOVE);

		SetSuperWindowScreenTitle(sw);
	}

	return(TRUE);
}


BOOL SetupAboutWindowMenus(struct SuperWindow *sw)
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
	SetupWindowMenu( sw, &aboutmenu[MN_WINDOW_IDX] );
	SetupEditMenu( NULL, &aboutmenu[MN_EDIT_IDX] );
	SetupSettingsMenu( &aboutmenu[MN_SETTINGS_IDX] );
	SetupModeMenu( sw, &aboutmenu[MN_MODE_IDX] );
	LinkInUserMenu( &aboutmenu[MN_USER_IDX] );

	if( sw->sw_MenuStrip = CreateMenus( aboutmenu, TAG_END ) )
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


void HideAboutWindow(struct SuperWindow *sw)
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

		CloseWindowSafely(sw->sw_Window);				/* close it */
		sw->sw_Window = NULL;										/* indicate that it's gone */

		FreeGadgets(sw->sw_GadList);
		sw->sw_GadList = NULL;
		if (sw->sw_VisualInfo) FreeVisualInfo(sw->sw_VisualInfo);
		sw->sw_VisualInfo = NULL;
		SetProcessWindowPtr();
	}
}



APTR KillOffAboutWindow(struct SuperWindow *sw)
{
	struct AboutWindow *aw;

	aw = (struct AboutWindow *)sw;

	/* close/free the superwindow */
	HideAboutWindow(sw);												/* shut down the Intuition window */
	Remove(&(sw->sw_Node));											/* remove it from superwindows list */
	FreeVec(aw);																/* free AboutWindow struct */
	return(NULL);
}



void AboutWindowHandler(struct SuperWindow *sw,
	struct IntuiMessage *imsg)
{
	ULONG	class;
	UWORD	code;
	struct IntuiMessage *gtimsg;

	if( gtimsg = GT_FilterIMsg(imsg) )
	{

		class = imsg->Class;
		code = imsg->Code;

		GT_PostFilterIMsg(gtimsg);

		switch(class)
		{
			case IDCMP_CLOSEWINDOW:
				ReplyMsg((struct Message *)imsg);
				KillOffAboutWindow(sw);
				SussOutQuiting(FALSE);
				break;
			case IDCMP_REFRESHWINDOW:
				ReplyMsg((struct Message *)imsg);
				GT_BeginRefresh(sw->sw_Window);
				GT_EndRefresh(sw->sw_Window,TRUE);
				break;
			case IDCMP_GADGETUP:
				ReplyMsg((struct Message *)imsg);
				HideAboutWindow(sw);
				RV_RemakeAll();
				break;
			case IDCMP_MENUPICK:
				SortOutAboutWindowMenus( sw,imsg,code );
				break;
			default:
				ReplyMsg((struct Message *)imsg);
				break;
		}
	}
	else
		ReplyMsg((struct Message *)imsg);
}

/************* SortOutAboutWindowMenus() **************/
//
// Handle MENUPICK IDCMP messages - note the reply() is given
// here rather than in the main loop.
// Why is this so? Well... who can tell?
//

void SortOutAboutWindowMenus(struct SuperWindow *sw,
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
