#include <stdio.h>
//#include <stdarg.h>
//#include <string.h>
//#include <dos/dos.h>
//#include <dos/dostags.h>
#include <exec/exec.h>
//#include <intuition/intuition.h>
//#include <graphics/gfxbase.h>
#include <graphics/gfxmacros.h>
//#include <rexx/rxslib.h>
//#include <libraries/gadtools.h>
//#include <libraries/diskfont.h>
//#include <libraries/asl.h>

#include <proto/exec.h>
#include <proto/dos.h>
#include <proto/intuition.h>
#include <proto/graphics.h>
//#include <proto/diskfont.h>
#include <proto/asl.h>
//#include <clib/alib_protos.h>
#include <proto/gadtools.h>

#include <graphics/modeid.h>

#include <global.h>
#include <ExternVars.h>

static struct NewMenu	mapmenu[] =
	{
		{ NM_TITLE,	"Project",							0,	0,	0,	0,},
		{  NM_ITEM,	"New",								 "N",	0,	0,	0,},
		{  NM_ITEM,	"Open...",						 "O",	0,	0,	0,},
		{	 NM_ITEM,	NM_BARLABEL,						0,	0,	0,	0,},
		{  NM_ITEM,	"Save",								 "S",	0,	0,	0,},
		{  NM_ITEM,	"Save As...",					 "A",	0,	0,	0,},
		{	 NM_ITEM,	NM_BARLABEL,						0,	0,	0,	0,},
		{  NM_ITEM,	"Hide",									0,	0,	0,	0,},
		{  NM_ITEM,	"Reveal...",						0,	0,	0,	0,},
		{  NM_ITEM,	"Close",								0,	0,	0,	0,},
		{	 NM_ITEM,	NM_BARLABEL,						0,	0,	0,	0,},
		{  NM_ITEM,	"Page Size...",					0,	0,	0,	0,},
		{  NM_ITEM,	"Screen Mode...",				0,	0,	0,	0,},
		{	 NM_ITEM,	NM_BARLABEL,						0,	0,	0,	0,},
		{  NM_ITEM,	"Iconify...",						0,	0,	0,	0,},
		{  NM_ITEM,	"About...",							0,	0,	0,	0,},
		{  NM_ITEM,	"Quit Program...",		 "Q",	0,	0,	0,},

/* 17 */
		{ NM_TITLE,	"Edit",									0,	0,	0,	0,},
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

/* 62 */
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
		{   NM_SUB,	"Sequencer...",					0,	0,	0,	0,},
		{   NM_SUB,	"Range...",							0,	0,	0,	0,},
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

/* 85 */
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

/* 99 */
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

/* 109 */
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

/* 137 */
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

#define PG_SIZEOFGADMAP			2*8*8

extern long __far PJ_MagnifyIn0;
extern long __far PJ_MagnifyIn1;
extern long __far PJ_MagnifyOut0;
extern long __far PJ_MagnifyOut1;

/* prototypes local to this source file: */

BOOL ShowProjWindow(struct SuperWindow *sw);
BOOL SetupProjWindowMenus(struct SuperWindow *sw);
void HideProjWindow(struct SuperWindow *sw);
APTR KillOffProjWindow(struct SuperWindow *sw);
static void DoBorderGadgets(struct ProjectWindow *pw);
void ProjWindowHandler(struct SuperWindow *sw, struct IntuiMessage *imsg);
static void SortOutZoomGadgets(struct ProjectWindow *pw,struct Gadget *gad, WORD mx, WORD my);
//static void SortOutMagnifyGadget(struct ProjectWindow *pw,struct Gadget *gad);
static void SortOutProjWindowMenus(struct SuperWindow *sw,
																	 struct IntuiMessage *imsg,
																	 UWORD code);
static void SussOutCloseGadget(struct SuperWindow *sw);
static void SortOutProjKeys(struct ProjectWindow *pw,
														struct Project *proj,
														UWORD keycode,
														WORD mx, WORD my, UWORD qual);
static void PrintDisabled(struct ProjectWindow *pw);
static void ActivateNextProjWindow(struct Project *currentpj);
static void HandleUp(struct ProjectWindow *pw,WORD mx, WORD my, UWORD qual);
static void HandleDown(struct ProjectWindow *pw,WORD mx, WORD my, UWORD qual);
static void HandleLeft(struct ProjectWindow *pw,WORD mx, WORD my, UWORD qual);
static void HandleRight(struct ProjectWindow *pw,WORD mx, WORD my, UWORD qual);
static void DoProjGadgets(struct ProjectWindow *pw, BOOL clear);
static void HandleBobNameGadget(struct ProjectWindow *pw, struct Gadget *gad);

void DumpPens( void );
void DumpCopperList(void);

UBYTE minterm = 0;

/**************** OpenNewProjWindow() ****************/
//
// Creates a new ProjectWindow, and attaches it to a Project.
//

BOOL OpenNewProjWindow(struct WindowDefault *wf, struct Project *pj)
{
	struct	ProjectWindow	*pw;
	struct File *fl;

	if( !pj || !wf )	return(FALSE);

	/* grab mem for the ProjectWindow struct */
	if ( !( pw = AllocVec(sizeof(struct ProjectWindow), MEMF_ANY|MEMF_CLEAR ) ) )
		return(FALSE);
	if(!(pw->pw_GadMaps = AllocVec((PG_SIZEOFGADMAP*PG_NUMOFGADGETS*2),MEMF_CHIP|MEMF_CLEAR) ) )
	{
		FreeVec(pw);
		return(FALSE);
	}

	/* figure out a unique ID for this window (mainly for AREXX use) */
	InstallUniqueWindowID((struct SuperWindow *)pw);
	pw->pw_sw.sw_Node.ln_Name = (char *)&(pw->pw_sw.sw_idstring);

	/* find the Default settings for this type of SuperWindow */
	//wf = &glob.cg_WindowDefaults[SWTY_PROJECT];

	pw->pw_Project = pj;											/* point to project (if any) */
	pw->pw_sw.sw_Handler = ProjWindowHandler;
	pw->pw_sw.sw_HideWindow = HideProjWindow;
	pw->pw_sw.sw_ShowWindow = ShowProjWindow;
	pw->pw_sw.sw_BuggerOff = KillOffProjWindow;
	pw->pw_sw.sw_RefreshMenus = SetupProjWindowMenus;
	pw->pw_sw.sw_Window = NULL;
	pw->pw_sw.sw_MenuStrip = NULL;
	pw->pw_sw.sw_Type = SWTY_PROJECT;
	pw->pw_sw.sw_VisualInfo = NULL;
	pw->pw_sw.sw_GadList = NULL;
	pw->pw_MagnifyLevel = 4;
	pw->pw_PixelSize = 1;
	pw->pw_Magnified = FALSE;

	/* set up stuff for opening the actual intuition window */
	pw->pw_sw.sw_Left = wf->wf_Left;
	pw->pw_sw.sw_Top = wf->wf_Top;
	pw->pw_sw.sw_Width = wf->wf_Width;
	pw->pw_sw.sw_Height = wf->wf_Height;
	pw->pw_sw.sw_MinWidth = 70;
	pw->pw_sw.sw_MinHeight = 50;
	pw->pw_sw.sw_MaxWidth = 0xFFFF;
	pw->pw_sw.sw_MaxHeight = 1023;	/* This is limited because of various blitter
																	 * opertions that don't use ECS Big_Blit. */
	pw->pw_sw.sw_MenuStrip = NULL;
	memset(pw->pw_sw.sw_Menus,MN_NUMOFMENUS*4,0);

	pw->pw_sw.sw_FlgBackDrop = wf->wf_FlgBackDrop;
	pw->pw_sw.sw_FlgBorderless = wf->wf_FlgBorderless;
	pw->pw_sw.sw_FlgTitle = wf->wf_FlgTitle;
	pw->pw_sw.sw_FlgBorderGadgets = wf->wf_FlgBorderGadgets;

	fl = pw->pw_Project->pj_File;
	pw->pw_sw.sw_Title = fl ? fl->fl_Name : "Untitled";

	/* add the ProjectWindow into the superwindows list */
	AddTail(&superwindows, &pw->pw_sw.sw_Node);

	/* link to a project */
	pw->pw_NextProjWindow = pj->pj_ProjectWindows;
	pj->pj_ProjectWindows = pw;

	/* open the intuition window (if the window doesn't open, */
	/* it is just left hidden - SEP). */

	ShowProjWindow(&pw->pw_sw);

	return(TRUE);
}



BOOL ShowProjWindow(struct SuperWindow *sw)
{
	struct ProjectWindow *pw;
	ULONG flags;

	pw = (struct ProjectWindow *)sw;

	if( (sw) && (!sw->sw_Window) )				/* make sure it isn't already open */
	{
		/* Set up the Visual Info for GadTools */
		if (!(sw->sw_VisualInfo = GetVisualInfo(MAINSCREEN,TAG_END)))
			return(FALSE);

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
											//WA_Gadgets,			sw->sw_FlgBorderGadgets ?
											//	sw->sw_GadList : NULL,
											WA_AutoAdjust,	TRUE,
											WA_Title,				sw->sw_FlgTitle ?
												BuildWindowTitle(sw) : NULL,
											TAG_DONE,				NULL);

		if( !sw->sw_Window ) return(FALSE);

		if(sw->sw_FlgBorderGadgets)
			DoBorderGadgets(pw);
		else
			sw->sw_GadList = NULL;

		DoProjGadgets(pw,TRUE);

		/* Tell the window which superwindow it is attached to */
		/* (so we'll know which handler routines to call) */
		sw->sw_Window->UserData = (BYTE *)pw;

		SetupProjWindowMenus(sw);

		/* we want to use our global message port for IDCMP stuff */
		sw->sw_Window->UserPort = mainmsgport;
		ModifyIDCMP(sw->sw_Window, IDCMP_CLOSEWINDOW | IDCMP_RAWKEY | IDCMP_MENUPICK |
			IDCMP_CHANGEWINDOW | IDCMP_GADGETUP | IDCMP_MOUSEBUTTONS |
			IDCMP_MENUVERIFY | IDCMP_ACTIVEWINDOW | IDCMP_INACTIVEWINDOW |
			IDCMP_REFRESHWINDOW | IDCMP_MOUSEMOVE);
		GetTrueWindowSize(pw);

		if(pw->pw_Project->pj_Disabled)
			PrintDisabled(pw);
		else
			RedisplayProjWindow(pw,TRUE);

		SetCorrectPointer(pw);

		lastprojectwindow = pw;

		SetSuperWindowScreenTitle(sw);
	}
}

BOOL SetupProjWindowMenus(struct SuperWindow *sw)
{
	BOOL success = FALSE;
	struct ProjectWindow *pw;
	struct Menu *mn;
	UWORD menucount;

	pw = (struct ProjectWindow *)sw;

	if(!sw) return(TRUE);

	if(sw->sw_MenuStrip)						/* Clear the menu strip if it exists */
	{
		ClearMenuStrip(sw->sw_Window);
		FreeMenus(sw->sw_MenuStrip);
		sw->sw_MenuStrip = NULL;
	}

	/* Set up some menus */
	SetupWindowMenu( sw, &mapmenu[MN_WINDOW_IDX] );
	SetupEditMenu( pw->pw_Project, &mapmenu[MN_EDIT_IDX] );
	SetupSettingsMenu( &mapmenu[MN_SETTINGS_IDX] );
	SetupModeMenu( sw, &mapmenu[MN_MODE_IDX] );
	LinkInUserMenu( &mapmenu[MN_USER_IDX] );

	if( sw->sw_MenuStrip = CreateMenus( mapmenu, TAG_END ) )
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

void HideProjWindow(struct SuperWindow *sw)
{
	struct ProjectWindow *pw;
	UWORD menu;

	WaitForSubTaskDone(mainmsgport);

	pw = (struct ProjectWindow *)sw;

	if(sw->sw_Window)												/* make sure it's actually open */
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
			for(menu = 0; menu < MN_NUMOFMENUS; menu++)
				sw->sw_Menus[menu] = NULL;
		}
		RememberWindow(sw);											/* save old window data */

		/* free up string used for window title */
		if(sw->sw_Window->Title)
			FreeVec(sw->sw_Window->Title);

		if(sw->sw_GadList)
		{
			RemoveGList(sw->sw_Window, sw->sw_GadList, PG_NUMOFGADGETS);
			sw->sw_GadList=NULL;
		}
		if(pw->pw_GTGadList)
		{
			RemoveGList(sw->sw_Window, pw->pw_GTGadList, -1);
			FreeGadgets(pw->pw_GTGadList);
			pw->pw_GTGadList = NULL;
		}

		MyClearPointer(sw->sw_Window);
		CloseWindowSafely(sw->sw_Window);				/* close it */
		sw->sw_Window = NULL;										/* indicate that it's gone */
		if(sw->sw_VisualInfo) FreeVisualInfo(sw->sw_VisualInfo);
		sw->sw_VisualInfo = NULL;
		SetProcessWindowPtr();
		if(lastprojectwindow == pw)
			lastprojectwindow = NULL;
	}
}


/****************  KillOffProjWindow ****************/
//
//  Returns: (struct Palette *), for the new palette
// to use after this project is closed. If NULL, then
// no new palette needs to be used.

APTR KillOffProjWindow(struct SuperWindow *sw)
{
	struct ProjectWindow *pw;
	struct Project *pj;
	APTR result = NULL;

	WaitForSubTaskDone(mainmsgport);

	pw = (struct ProjectWindow *)sw;

	/* see which project we're attached to */
	pj = pw->pw_Project;

	/* close/free the superwindow */
	HideProjWindow(sw);												/* shut down the Intuition window */
	UnlinkProjectWindow(pw);
	Remove(&sw->sw_Node);											/* remove from superwindows list */
	FreeVec(pw->pw_GadMaps);									/* Free the Chip memory for the Gadget Maps */
	FreeVec(pw);															/* free ProjectWindow struct */

	if(!pj->pj_ProjectWindows)
	{
		/* we've just shut down the last window attached to the */
		/* project, so we also want to kill off the project */

		result = FreeProject(pj);
	}
	return(result);
}


static void DoBorderGadgets(struct ProjectWindow *pw)
{
	struct Window *win;
	struct ToolsWindowGadList	*tgl;
	struct ToolsWindowGadget *tg;
	struct DrawInfo *dri;
	struct Gadget *gad;
	WORD	gadgettop,gadgetleft;

	if( (!pw) || !(win = pw->pw_sw.sw_Window) )
		return;

	if(pw->pw_sw.sw_GadList)
	{
		RemoveGList(win, pw->pw_GadList, -1);
		pw->pw_sw.sw_GadList = NULL;
	}

	//ClearWindow( win );

	tgl = &pw->pw_ToolsWindowGadList;
	tg = pw->pw_ToolsWindowGadget;
	gadgettop = mainscreen->WBorTop + mainscreen->Font->ta_YSize + 1;

	if(mainscreen->Width < 640)
		gadgetleft = -12;
	else
	{
		if(pw->pw_sw.sw_FlgBackDrop)
			gadgetleft = -12;
		else
			gadgetleft = -14;
	}

	NewList(&tgl->tgl_List);
	tgl->tgl_GadImages = pw->pw_GadImages;
	tgl->tgl_Gadgets = pw->pw_GadList;
	tgl->tgl_GadMaps = pw->pw_GadMaps;
	tgl->tgl_SelectWidth = 13;
	tgl->tgl_Width = 1;
	tgl->tgl_Height = 8;
	tgl->tgl_Depth = 8;

	dri = GetScreenDrawInfo( mainscreen );
	tgl->tgl_BrightColour = dri->dri_Pens[SHINEPEN];
	tgl->tgl_DarkColour = dri->dri_Pens[SHADOWPEN];
	FreeScreenDrawInfo( mainscreen, dri );

	AddTail(&tgl->tgl_List,&tg->tg_Node);
	tg->tg_LeftEdge = gadgetleft;
	tg->tg_TopEdge = gadgettop;
	tg->tg_Activation = GACT_RELVERIFY | GACT_RIGHTBORDER;
	tg->tg_GadgetID = PJG_IN;
	tg->tg_BrightMap1 = (UBYTE *)&PJ_MagnifyIn0;
	tg->tg_DarkMap1 = (UBYTE *)&PJ_MagnifyIn1;
	tg->tg_BrightMap2 = (UBYTE *)&PJ_MagnifyIn1;
	tg->tg_DarkMap2 = (UBYTE *)&PJ_MagnifyIn0;
	tg++;
	AddTail(&tgl->tgl_List,&tg->tg_Node);
	tg->tg_LeftEdge = gadgetleft;
	tg->tg_TopEdge = gadgettop+8;
	tg->tg_Activation = GACT_RELVERIFY | GACT_RIGHTBORDER;
	tg->tg_GadgetID = PJG_OUT;
	tg->tg_BrightMap1 = (UBYTE *)&PJ_MagnifyOut0;
	tg->tg_DarkMap1 = (UBYTE *)&PJ_MagnifyOut1;
	tg->tg_BrightMap2 = (UBYTE *)&PJ_MagnifyOut1;
	tg->tg_DarkMap2 = (UBYTE *)&PJ_MagnifyOut0;

	/* Call up my little Asm routine to sort out all the stuff, like remapping
			and making up Gadget structures, and crap like that. */
	MakeToolsGadgets(tgl);

	gad = pw->pw_GadList;
	gad->Flags |= GFLG_RELRIGHT;
	gad++;
	gad->Flags |= GFLG_RELRIGHT;

	pw->pw_sw.sw_GadList = pw->pw_GadList;

	/* attach our groovy new gadgets to the window */
	AddGList(win, pw->pw_sw.sw_GadList, -1, -1, NULL);

		/* need to redraw the frame coz we're not using GimmeZeroZero */
	RefreshWindowFrame(win);
}

/************** DoProjGadgets() **************/
//
// Sets up and formats all the gadgets within the SeqWindow.
// Adapts to font and window sizes.
// Call at window opening, and again at NEWSIZE events.

static void DoProjGadgets(struct ProjectWindow *pw, BOOL clear)
{
	struct Gadget *gad;
	struct NewGadget ng;
	struct Window *win;
	UWORD buttonh,buttonw;
	UWORD	winbottom;
	struct MyBob *bob;

	if( (!pw) || !(win = pw->pw_sw.sw_Window) )
		return;

	/* first, kill any old gadgets that may be active */
	if(pw->pw_GTGadList)
	{
		RemoveGList(win, pw->pw_GTGadList, -1);
		FreeGadgets(pw->pw_GTGadList);
		pw->pw_GTGadList = NULL;
	}

	if(clear)
		ClearWindow( win );

	pw->pw_TextGadgetHeight = 0;

	if( (pw->pw_Project->pj_EditType == EDTY_BOBS) || (pw->pw_Project->pj_EditType == EDTY_SPRITES) )
	{
		/* use window font */
		SetFont(win->RPort,windowtextfont);
		pw->pw_sw.sw_TextAttr = &windowtextattr;
		pw->pw_sw.sw_TextFont = windowtextfont;
	
		buttonh = win->RPort->Font->tf_YSize + 4;
		buttonw = TextLength(win->RPort,"WWWWWWWWWWWWW",13)+4;

		/* fall back to Topaz 8 if the window is too small */
		//AdjustWindowFont(&pw->pw_sw,buttonw,buttonh);

		/* init gadget stuff */
		gad = CreateContext(&pw->pw_GTGadList);
	
		buttonh = win->RPort->Font->tf_YSize + 4;
		buttonw = TextLength(win->RPort,"WWWWWWWWWWWWW",13)+4;
		winbottom = win->Height - win->BorderBottom-1;	
		pw->pw_TextGadgetHeight = buttonh;

		bob = pw->pw_Project->pj_CurrentBob;

		ng.ng_TextAttr = pw->pw_sw.sw_TextAttr;
		ng.ng_VisualInfo = pw->pw_sw.sw_VisualInfo;
		ng.ng_TopEdge = winbottom-buttonh+1;
		ng.ng_LeftEdge = win->BorderLeft;
		ng.ng_Width = win->Width - win->BorderLeft - win->BorderRight;
		ng.ng_Height = buttonh;
		ng.ng_GadgetText = NULL;
		ng.ng_GadgetID = PJG_BOBNAME;
		ng.ng_Flags = 0;
		gad = CreateGadget( STRING_KIND, gad, &ng,
						GTST_String,		bob ? bob->bb_Node.ln_Name : "\0",
						GTST_MaxChars,	BOBNAMESIZE-1,
						TAG_END );
		pw->pw_GTGadgets[0] = gad;
	
		/* check for error */
		if (!gad)
		{
			FreeGadgets(pw->pw_sw.sw_GadList);
			return;
		}
	
		/* attach our groovy new gadgets to the window */
		AddGList(win, pw->pw_GTGadList, -1, -1, NULL);
		GT_RefreshWindow(win, NULL);

		/* need to redraw the frame coz we're not using GimmeZeroZero */
		RefreshWindowFrame(win);
	}
}


void ProjWindowHandler(struct SuperWindow *sw, struct IntuiMessage *imsg)
{
	ULONG	class;
	UWORD	code,qual;
	WORD mx,my;
	struct IntuiMessage *gtimsg;
	//struct Gadget *gad;
	struct ProjectWindow *pw;
	struct Project *proj;
	struct Window *win;
	BOOL processed = FALSE;

	pw = (struct ProjectWindow *)sw;
	proj = pw->pw_Project;

	if( gtimsg = GT_FilterIMsg(imsg) )
	{
		/* read out all we need */
		class = gtimsg->Class;
		code = gtimsg->Code;
		qual = gtimsg->Qualifier;
		mx = gtimsg->MouseX;
		my = gtimsg->MouseY;
		//gad = (struct Gadget *)gtimsg->IAddress;
		win = sw->sw_Window;

		GT_PostFilterIMsg(gtimsg);

		if(!proj->pj_Disabled)
		{
			/* If and only if the Project Is Enabled, can we
			 * do the stuff below */
			switch(class)
			{
				case IDCMP_RAWKEY:
					ReplyMsg((struct Message *)imsg);
					processed = TRUE;
					SortOutProjKeys(pw,proj,code,mx,my,qual);
					break;
				case IDCMP_GADGETUP:
					if( ((struct Gadget *)imsg->IAddress)->GadgetID == PJG_BOBNAME)
						HandleBobNameGadget(pw,imsg->IAddress);
					else
						SortOutZoomGadgets(pw,imsg->IAddress,mx,my);
					processed = TRUE;
					ReplyMsg((struct Message *)imsg);
					break;
				case IDCMP_ACTIVEWINDOW:
					ReplyMsg((struct Message *)imsg);
					lastprojectwindow = pw;
					if(currenttool == TL_MAGNIFY)
						SetMagnifyBoxSize(pw);
					SetCorrectPointer(pw);
					processed = TRUE;
					break;
			}
		}

		if(!processed)
		{
			switch(class)
			{
				case IDCMP_CLOSEWINDOW:
					ReplyMsg((struct Message *)imsg);
					SussOutCloseGadget(sw);
					break;
				case IDCMP_MENUPICK:
					SortOutProjWindowMenus( sw,imsg,code );
					break;
				case IDCMP_REFRESHWINDOW:
					ReplyMsg((struct Message *)imsg);
					WaitForSubTaskDone(mainmsgport);
					GT_BeginRefresh(sw->sw_Window);
					GT_EndRefresh(sw->sw_Window,TRUE);
					if(proj->pj_Disabled)
						PrintDisabled(pw);
					else
						RedisplayProjWindow(pw,TRUE);
					break;
				default:
					ReplyMsg((struct Message *)imsg);
					break;
			}
		}
	}
}


/************* Sort Out Zoom Gadgets ***************/
static void SortOutZoomGadgets(struct ProjectWindow *pw,struct Gadget *gad, WORD mx, WORD my)
{
	//UWORD level;
	//struct PropInfo *pi;
	WORD	oldwidth,oldheight,xdelta,ydelta;
	WORD bmx,bmy;

	WaitForSubTaskDone(mainmsgport);

	if( (pw->pw_MagnifyLevel == MAGNIFYLEVELS) && (gad->GadgetID == PJG_IN) )
		return;
	if( (pw->pw_MagnifyLevel == 2) && (gad->GadgetID == PJG_OUT) )
		return;

	EraseSelector(pw);

	if(gad->GadgetID == PJG_IN)
		pw->pw_MagnifyLevel++;

	if(gad->GadgetID == PJG_OUT)
		pw->pw_MagnifyLevel--;

	if(pw->pw_MagnifyLevel > MAGNIFYLEVELS)
		pw->pw_MagnifyLevel = MAGNIFYLEVELS;

	if(pw->pw_MagnifyLevel < 2)
		pw->pw_MagnifyLevel = 2;
		
	if(pw->pw_Magnified)
	{
		pw->pw_PixelSize = pw->pw_MagnifyLevel;
		oldwidth = pw->pw_MagWidth;
		oldheight = pw->pw_MagHeight;
		GetTrueWindowSize(pw);
		xdelta = (oldwidth - pw->pw_MagWidth) / 2;
		ydelta = (oldheight - pw->pw_MagHeight) / 2;
		pw->pw_XOffset += xdelta;
		pw->pw_YOffset += ydelta;
		if(pw->pw_XOffset < 0)
			pw->pw_XOffset = 0;
		if(pw->pw_YOffset < 0)
			pw->pw_YOffset = 0;
		RedisplayProjWindow(pw,TRUE);
		GetBitMapOffsets(mx,my,&bmx,&bmy,pw);
		DrawSelector(pw,bmx, bmy);
	}
	if(currenttool == TL_MAGNIFY)
	{
		EraseSelector(pw);
		SetMagnifyBoxSize(pw);
		GetBitMapOffsets(mx,my,&bmx,&bmy,pw);
		DrawSelector(pw,bmx, bmy);
	}
}

/********** SussOutCloseGadget **************/
static void SussOutCloseGadget(struct SuperWindow *sw)
{
	struct Project *pj,*pj2;
	struct ProjectWindow *pw;
	int numofwindows;
	struct Palette *pal = NULL;
	struct File *fl;
	UWORD numofprojects;

	pw = (struct ProjectWindow *)sw;
	pj = pw->pw_Project;
	numofwindows = CountProjectWindows(pj);

	fl = pj->pj_File;
	numofprojects = 0;
	if(fl && (numofwindows == 1))
	{
		for(pj2 = fl->fl_FirstProject; pj2; pj2 = pj2->pj_NextProjectInFile)
			numofprojects++;
	}
	if( (numofprojects <= 1) ||
		 ((numofprojects >= 2) && GroovyReq("Close Project","This Will Erase The Project\nFrom The File\nAre You Really Sure?","Kill It|Cancel")) )
	{
		if( (pj->pj_ModifiedFlag) && (numofwindows == 1) )
		{
			if( ModifiedProjectRequester(pj) )
				pal = (struct Palette *)KillOffProjWindow(sw);
		}
		else
			pal = (struct Palette *)KillOffProjWindow(sw);
	}

	if(pal)
		InstallNewGlobPalette(pal);

	SussOutQuiting(FALSE);
}

/********** SORT OUT PROJECT MENUS **************/

static void SortOutProjWindowMenus(struct SuperWindow *sw,
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
		( i < 10 ) && ( menunumber[i] != MENUNULL ) && !(status & 1);
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
			case MN_EDIT:
				DoEditMenu((struct ProjectWindow *)sw,menunumber[i]);
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

/**************** SortOutProjKeys *******************/

static void SortOutProjKeys(struct ProjectWindow *pw,
														struct Project *pj,
														UWORD keycode,
														WORD mx, WORD my,
														UWORD qual)
{
	struct MyBob *bob = NULL;

	WaitForSubTaskDone(mainmsgport);

	if( (qual & IEQUALIFIER_LALT) || (qual & IEQUALIFIER_RALT) )
		altdown = TRUE;
	else
		altdown = FALSE;

	switch (keycode)
	{
		case 0x26:		/* J Key */
			ActivateNextProjWindow(pj);
			break;

		case 0x19:		/* P key */
			UseProjSettings(pj);
			break;

		case 0x16:		/* U key */
			DoUndo(pj);
			break;

		case 0x40:			/* Space */
			break;

		case 0x17:		/* I key */
			break;

		case 0x18:		/* O key */
			OutlineCurrentBrush(lmbcol);
			break;

		case 0x37:		/* M Key */
			if(currenttool != TL_MAGNIFY)
			{
				prevtool = currenttool;
				currenttool = TL_MAGNIFY;
				SetMagnifyBoxSize(pw);
				SetToolsWindowGadget(TL_MAGNIFY,!pw->pw_Magnified);
			}
			break;
		case 0x4C:			/* UP */
			HandleUp(pw,mx,my,qual);
			break;

		case 0x4D:			/* DOWN */
			HandleDown(pw,mx,my,qual);
			break;

		case 0x4F:			/* LEFT */
			HandleLeft(pw,mx,my,qual);
			break;

		case 0x4E:			/* RIGHT */
			HandleRight(pw,mx,my,qual);
			break;

		case 0x0B:		/* - Key */
			SortOutZoomGadgets(pw,&pw->pw_GadList[1],mx,my);
			break;

		case 0x0C:		/* + Key */
			SortOutZoomGadgets(pw,&pw->pw_GadList[0],mx,my);
			break;

		case 0x3E:		/* Numeric 8 (UP) */
			WaitForSubTaskDone(mainmsgport);
			if(pj->pj_CutBobBoxHeight > 2)
			{
				EraseSelector(pw);
				pj->pj_CutBobBoxHeight--;
				DrawSelector(pw,pw->pw_SelX0,pw->pw_SelY0);
			}
			break;
		case 0x2D:		/* Numeric 4 (LEFT) */
			WaitForSubTaskDone(mainmsgport);
			if(pj->pj_CutBobBoxWidth > 16)
			{
				EraseSelector(pw);
				pj->pj_CutBobBoxWidth -= 16;
				DrawSelector(pw,pw->pw_SelX0,pw->pw_SelY0);
			}
			break;
		case 0x2F:		/* Numeric 6 (RIGHT) */
			WaitForSubTaskDone(mainmsgport);
			if(pj->pj_CutBobBoxWidth < pj->pj_Width)
			{
				EraseSelector(pw);
				pj->pj_CutBobBoxWidth += 16;
				DrawSelector(pw,pw->pw_SelX0,pw->pw_SelY0);
			}
			break;
		case 0x1E:		/* Numeric 2 (DOWN) */
			WaitForSubTaskDone(mainmsgport);
			if(pj->pj_CutBobBoxHeight < pj->pj_Height)
			{
				EraseSelector(pw);
				pj->pj_CutBobBoxHeight++;
				DrawSelector(pw,pw->pw_SelX0,pw->pw_SelY0);
			}
			break;
		case 0x44:		/* Return */
			break;

		case 0x35:			/* B */
			if( (qual & IEQUALIFIER_LSHIFT) || (qual & IEQUALIFIER_RSHIFT) )
				RestoreBrush(currentbrush);
			else
				SetNewTool(TL_GETBRUSH,TRUE);
			break;

		case 0x39:			/* . */
			SetNewTool(TLB_DOT,TRUE);
			break;

		case 0x38:			/* , */
			SetNewTool(TL_PICKCOLOUR,TRUE);
			break;

		case 0x24:			/* G */
			SetNewTool(TL_GRIDLOCK,TRUE);
			break;

		case 0x25:			/* H */
			if(currentbrush == &brushes[BR_CUSTOM])
			{
				HalveBrush(&currentbrush->br_DrawBrush, globpalette);
			}
			else
				GroovyReq("Size Brush","Can Only Transform\nCustom Brush","Okay");
			break;

		case 0x01:			/* 1 */
			WaitForSubTaskDone(mainmsgport);
			if( (qual & IEQUALIFIER_LSHIFT) || (qual & IEQUALIFIER_RSHIFT) )
			{
				switch(pj->pj_EditType)
				{
					case EDTY_BOBS:
						GotoNewBob(pj, (struct MyBob *)pj->pj_BobList.lh_Head);
						RefreshProjectName(pj);
						break;
					case EDTY_SPRITES:
						GotoNewSprite(pj, (struct MyBob *)pj->pj_BobList.lh_Head);
						RefreshProjectName(pj);
						break;
				}
			}
			else
			{
				switch(pj->pj_EditType)
				{
					case EDTY_BOBS:
						GotoPreviousBob(pj);
						break;
					case EDTY_SPRITES:
						GotoPreviousSprite(pj);
						break;
				}
			}
			break;

		case 0x02:			/* 2 */
			WaitForSubTaskDone(mainmsgport);
			if( (qual & IEQUALIFIER_LSHIFT) || (qual & IEQUALIFIER_RSHIFT) )
			{
				switch(pj->pj_EditType)
				{
					case EDTY_BOBS:
						GotoNewBob(pj, (struct MyBob *)pj->pj_BobList.lh_TailPred);
						RefreshProjectName(pj);
						break;
					case EDTY_SPRITES:
						GotoNewSprite(pj, (struct MyBob *)pj->pj_BobList.lh_TailPred);
						RefreshProjectName(pj);
						break;
				}
			}
			else
			{
				switch(pj->pj_EditType)
				{
					case EDTY_BOBS:
						GotoNextBob(pj);
						break;
					case EDTY_SPRITES:
						GotoNextSprite(pj);
						break;
				}
			}
			break;

		case 0x03:			/* 3 */
			WaitForSubTaskDone(mainmsgport);
			if(BlockAllSuperWindows())
			{
				bob = GetBobRequester(pw, "Goto Bob");
				UnblockAllSuperWindows();
			}
			if(bob)
			{
				GotoNewBob(pj, bob);
				RefreshProjectName(pj);
			}
			break;

		case 0x00:			/* ` */
			WaitForSubTaskDone(mainmsgport);
			if(pj->pj_EditType == EDTY_BOBS)
			{
				if(pw->pw_Project->pj_EditBobMask)
					TurnOnBobImageEditing(pw->pw_Project);
				else
					TurnOnBobMaskEditing(pw->pw_Project);
				RefreshEditMenu(pw->pw_Project);
			}
			break;

		case 0x50:			/* F1 */
			if(drawmode != DRMD_MATTE) NewDrawMode(DRMD_MATTE);
			break;

		case 0x51:			/* F2 */
			if(drawmode != DRMD_COLOUR) NewDrawMode(DRMD_COLOUR);
			break;

		case 0x52:			/* F3 */
			if(drawmode != DRMD_REPLACE) NewDrawMode(DRMD_REPLACE);
			break;

		case 0x54:			/* F5 */
			if(drawmode != DRMD_SHADE) NewDrawMode(DRMD_SHADE);
			break;

		case 0x56:			/* F7 */
			if(drawmode != DRMD_CYCLE) NewDrawMode(DRMD_CYCLE);
			break;

		case 0x57:			/* F8 */
			if(drawmode != DRMD_SMOOTH) NewDrawMode(DRMD_SMOOTH);
			break;

		case 0x32:			/* X */
			if( (qual & IEQUALIFIER_LALT) || (qual & IEQUALIFIER_RALT) )
			{
				EraseSelector(pw);
				RotateBrushHandle(currentbrush);
				DrawSelector(pw, pw->pw_SelX0, pw->pw_SelY0);
			}
			else
				FlipBrushHorizontally(&currentbrush->br_DrawBrush);
			break;

		case 0x15:			/* Y */
			FlipBrushVertically(&currentbrush->br_DrawBrush);
			break;

		case 0x31:			/* Z */
			RotateBrush90(&currentbrush->br_DrawBrush);
			break;

		case 0x22:			/* D */
			//DumpCopperList();
			DumpPens();
			break;
	}
}

void DumpPens( void )
{
	UWORD count = 0;

	while(globpens[count] != 0xFFFF)
	{
		printf("$%x",globpens[count++]);
	}
}

void DumpCopperList(void)
{
	struct View *view;
	struct	cprlist *cprlist;
	UWORD *copinst,count;

	printf("CopperInit:\n");
	copinst = (UWORD *)GfxBase->copinit;
	for(count = 0; count < (sizeof(struct copinit)/2); count++)
		printf("%04x %04x\n",copinst[count*2],copinst[(count*2)+1]);

	view = GfxBase->ActiView;
	cprlist = view->LOFCprList;
	copinst = cprlist->start;
	printf("\nActiCopper\n");
	for(count = 0; count < cprlist->MaxCount; count++)
		printf("%04x %04x\n",copinst[count*2],copinst[(count*2)+1]);

	printf("\nBytesPerRow: %d\n",mainscreen->RastPort.BitMap->BytesPerRow);
}


static void HandleUp(struct ProjectWindow *pw,WORD mx, WORD my, UWORD qual)
{
	WaitForSubTaskDone(mainmsgport);

	if(pw->pw_YOffset != 0)
	{
		if( (qual & IEQUALIFIER_LALT) || (qual & IEQUALIFIER_RALT) )
		{
			ScrollUp(pw,16,mx,my);
			return;
		}
		if( (qual & IEQUALIFIER_LSHIFT) || (qual & IEQUALIFIER_RSHIFT) )
		{
			pw->pw_YOffset -= (pw->pw_MagHeight-4);
			if(pw->pw_YOffset < 0)
				pw->pw_YOffset = 0;
			RedisplayProjWindow(pw,TRUE);
			return;
		}
		if(qual & IEQUALIFIER_CONTROL)
		{
			pw->pw_YOffset = 0;
			RedisplayProjWindow(pw,TRUE);
			return;
		}
		ScrollUp(pw,4,mx,my);
	}
}

static void HandleDown(struct ProjectWindow *pw,WORD mx, WORD my, UWORD qual)
{
	struct Project *pj;
	WORD viewbottom, bmbottom;

	WaitForSubTaskDone(mainmsgport);

	if( (qual & IEQUALIFIER_LALT) || (qual & IEQUALIFIER_RALT) )
	{
		ScrollDown(pw,16,mx,my);
		return;
	}

	if( !(qual & (IEQUALIFIER_LSHIFT | IEQUALIFIER_RSHIFT | IEQUALIFIER_CONTROL)) )
	{
		ScrollDown(pw,4,mx,my);
		return;
	}

	pj = pw->pw_Project;

	viewbottom = pw->pw_YOffset + pw->pw_MagHeight;
	bmbottom = pj->pj_Height;

	if(bmbottom <= viewbottom)
		return;

	/* We can move, so thats cool */

	if(qual & IEQUALIFIER_CONTROL)
	{
		pw->pw_YOffset = pj->pj_Height-pw->pw_MagHeight;
		if(pw->pw_YOffset < 0)
			pw->pw_YOffset = 0;		
		RedisplayProjWindow(pw,TRUE);
	}
	else			/* Shift Is Selected */
	{
		pw->pw_YOffset += (pw->pw_MagHeight-4);

		viewbottom = pw->pw_YOffset + pw->pw_MagHeight;
		bmbottom = pj->pj_Height;
		if(bmbottom < viewbottom)
		{
			pw->pw_YOffset = pj->pj_Height - pw->pw_MagHeight;
			if(pw->pw_YOffset < 0)
				pw->pw_YOffset = 0;
		}
		RedisplayProjWindow(pw,TRUE);
	}
}

static void HandleLeft(struct ProjectWindow *pw,WORD mx, WORD my, UWORD qual)
{
	WaitForSubTaskDone(mainmsgport);

	if(pw->pw_XOffset != 0)
	{
		if( (qual & IEQUALIFIER_LALT) || (qual & IEQUALIFIER_RALT) )
		{
			ScrollLeft(pw,16,mx,my);
			return;
		}
		if( (qual & IEQUALIFIER_LSHIFT) || (qual & IEQUALIFIER_RSHIFT) )
		{
			pw->pw_XOffset -= (pw->pw_MagWidth-4);
			if(pw->pw_XOffset < 0)
				pw->pw_XOffset = 0;
			RedisplayProjWindow(pw,TRUE);
			return;
		}
		if(qual & IEQUALIFIER_CONTROL)
		{
			pw->pw_XOffset = 0;
			RedisplayProjWindow(pw,TRUE);
			return;
		}
		ScrollLeft(pw,4,mx,my);
	}
}

static void HandleRight(struct ProjectWindow *pw,WORD mx, WORD my, UWORD qual)
{
	struct Project *pj;
	WORD viewright, bmright;

	WaitForSubTaskDone(mainmsgport);

	if( (qual & IEQUALIFIER_LALT) || (qual & IEQUALIFIER_RALT) )
	{
		ScrollRight(pw,16,mx,my);
		return;
	}

	if( !(qual & (IEQUALIFIER_LSHIFT | IEQUALIFIER_RSHIFT | IEQUALIFIER_CONTROL)) )
	{
		ScrollRight(pw,4,mx,my);
		return;
	}

	pj = pw->pw_Project;

	viewright = pw->pw_XOffset + pw->pw_MagWidth;
	bmright = pj->pj_Width;

	if(bmright <= viewright)
		return;

	/* We can move, so thats cool */

	if(qual & IEQUALIFIER_CONTROL)
	{
		pw->pw_XOffset = pj->pj_Width-pw->pw_MagWidth;
		if(pw->pw_XOffset < 0)
			pw->pw_XOffset = 0;		
		RedisplayProjWindow(pw,TRUE);
	}
	else			/* Shift Is Selected */
	{
		pw->pw_XOffset += (pw->pw_MagWidth-4);

		viewright = pw->pw_XOffset + pw->pw_MagWidth;
		bmright = pj->pj_Width;
		if(bmright < viewright)
		{
			pw->pw_XOffset = pj->pj_Width - pw->pw_MagWidth;
			if(pw->pw_XOffset < 0)
				pw->pw_XOffset = 0;
		}
		RedisplayProjWindow(pw,TRUE);
	}
}


void ScrollUp(struct ProjectWindow *pw,UWORD ydelta, WORD mx, WORD my)
{
	WORD yoffset,magdelta;
	struct Window *win;
	struct Project *proj;
	struct Magnify mg;
	UWORD	bmwidth,winwidth;
	WORD	bmx,bmy;

	yoffset = pw->pw_YOffset;

	WaitForSubTaskDone(mainmsgport);

	if(yoffset > 0)
	{
		EraseSelector(pw);

		win = pw->pw_sw.sw_Window;
		proj = pw->pw_Project;

		yoffset -= ydelta;
		if(yoffset < 0)
			yoffset = 0;

		ydelta = yoffset - (pw->pw_YOffset);

		pw->pw_YOffset = yoffset;

		if(ydelta)
		{
			magdelta = ydelta * pw->pw_PixelSize;

			ScrollRaster(win->RPort, 0, magdelta,
									win->BorderLeft,
									win->BorderTop,
									(pw->pw_MagWidth * pw->pw_PixelSize) + win->BorderLeft - 1,
									(pw->pw_MagHeight * pw->pw_PixelSize) + win->BorderTop  - 1);


			winwidth = (win->Width) - (win->BorderRight) - (win->BorderLeft);
			bmwidth = ( ((proj->pj_Width) - (pw->pw_XOffset)) * pw->pw_PixelSize );
			if( bmwidth < winwidth )
				winwidth = bmwidth;

			mg.mg_SrcBitMap = pw->pw_Project->pj_MainBitMap;
			mg.mg_SrcXOffset = pw->pw_XOffset;
			mg.mg_SrcYOffset = pw->pw_YOffset;
			mg.mg_DestRastPort = win->RPort;
			mg.mg_DestXOffset = win->BorderLeft;
			mg.mg_DestYOffset = win->BorderTop;
			mg.mg_DestWidth = winwidth;
			mg.mg_DestHeight = -magdelta;
			mg.mg_DestPixelSize = pw->pw_PixelSize;
			mg.mg_ClearRight = (win->Width) - (win->BorderRight)-1;
			mg.mg_ClearBottom = (-magdelta)+(win->BorderTop)-1;
			mg.mg_Scale = TRUE;
			mg.mg_ScaleBitMap = workbm;
			MagnifyBitMap(&mg);
			GetBitMapOffsets(mx,my,&bmx,&bmy,pw);
			DrawSelector(pw, bmx, bmy);
		}
		OutlineBitMapRegions(pw);
	}
}


void ScrollDown(struct ProjectWindow *pw,UWORD ydelta, WORD mx, WORD my)
{
	struct Project *proj;
	struct Window *win;
	UWORD bmbottom,magtop;
	WORD yoffset,magdelta;
	struct Magnify mg;
	UWORD	bmwidth,winwidth;
	WORD	bmx,bmy;

	win = pw->pw_sw.sw_Window;

	yoffset = pw->pw_YOffset;

	proj = pw->pw_Project;

	bmbottom = (proj->pj_Height - yoffset);

	WaitForSubTaskDone(mainmsgport);

	if( bmbottom > pw->pw_MagHeight )
	{
		EraseSelector(pw);

		yoffset += ydelta;
		bmbottom = (proj->pj_Height - yoffset);

		if(bmbottom < pw->pw_MagHeight)
			yoffset = proj->pj_Height - pw->pw_MagHeight;
		if(yoffset < 0) yoffset=0;

		ydelta = yoffset - (pw->pw_YOffset) ;

		pw->pw_YOffset = yoffset;

		if(ydelta)
		{
			magdelta = ydelta * pw->pw_PixelSize;
			magtop = (win->BorderTop) + ( (pw->pw_MagHeight - ydelta) * pw->pw_PixelSize);

			ScrollRaster(win->RPort, 0, magdelta,
									 win->BorderLeft,
									 win->BorderTop,
									(pw->pw_MagWidth * pw->pw_PixelSize) + win->BorderLeft - 1,
									(pw->pw_MagHeight * pw->pw_PixelSize) + win->BorderTop - 1 );

			winwidth = (win->Width) - (win->BorderRight) - (win->BorderLeft);
			bmwidth = ( ((proj->pj_Width) - (pw->pw_XOffset)) * pw->pw_PixelSize );
			if( bmwidth < winwidth )
				winwidth = bmwidth;

			mg.mg_SrcBitMap = pw->pw_Project->pj_MainBitMap;
			mg.mg_SrcXOffset = pw->pw_XOffset;
			mg.mg_SrcYOffset = pw->pw_YOffset + (pw->pw_MagHeight - ydelta);
			mg.mg_DestRastPort = win->RPort;
			mg.mg_DestXOffset = win->BorderLeft;
			mg.mg_DestYOffset = magtop;
			mg.mg_DestWidth = winwidth;
			mg.mg_DestHeight = magdelta;
			mg.mg_DestPixelSize = pw->pw_PixelSize;
			mg.mg_ClearRight = (win->Width) - (win->BorderRight)-1;
			mg.mg_ClearBottom = (win->Height-pw->pw_TextGadgetHeight) - (win->BorderBottom)-1;
			mg.mg_Scale = TRUE;
			mg.mg_ScaleBitMap = workbm;
			MagnifyBitMap(&mg);
			GetBitMapOffsets(mx,my,&bmx,&bmy,pw);
			DrawSelector(pw, bmx, bmy);
		}
		OutlineBitMapRegions(pw);
	}
}

void ScrollLeft(struct ProjectWindow *pw,UWORD xdelta, WORD mx, WORD my)
{
	WORD xoffset,magdelta;
	struct Window *win;
	struct Project *proj;
	struct Magnify mg;
	UWORD	bmheight,winheight;
	WORD	bmx,bmy;

	xoffset = pw->pw_XOffset;

	WaitForSubTaskDone(mainmsgport);
	if(xoffset > 0)
	{
		EraseSelector(pw);

		win = pw->pw_sw.sw_Window;
		proj = pw->pw_Project;

		xoffset -= xdelta;
		if(xoffset < 0)
			xoffset = 0;

		xdelta = xoffset - (pw->pw_XOffset);

		pw->pw_XOffset = xoffset;

		if(xdelta)
		{
			magdelta = xdelta * pw->pw_PixelSize;

			ScrollRaster(win->RPort, magdelta, 0,
									win->BorderLeft,
									win->BorderTop,
									(pw->pw_MagWidth * pw->pw_PixelSize) + win->BorderLeft - 1,
									(pw->pw_MagHeight * pw->pw_PixelSize) + win->BorderTop - 1);


			winheight = (win->Height-pw->pw_TextGadgetHeight) - (win->BorderBottom) - (win->BorderTop);
			bmheight = ( ((proj->pj_Height) - (pw->pw_YOffset)) * pw->pw_PixelSize );
			if( bmheight < winheight )
				winheight = bmheight;

			mg.mg_SrcBitMap = pw->pw_Project->pj_MainBitMap;
			mg.mg_SrcXOffset = pw->pw_XOffset;
			mg.mg_SrcYOffset = pw->pw_YOffset;
			mg.mg_DestRastPort = win->RPort;
			mg.mg_DestXOffset = win->BorderLeft;
			mg.mg_DestYOffset = win->BorderTop;
			mg.mg_DestWidth = -magdelta;
			mg.mg_DestHeight = winheight;
			mg.mg_DestPixelSize = pw->pw_PixelSize;
			mg.mg_ClearRight = (-magdelta)+(win->BorderLeft)-1;
			mg.mg_ClearBottom = (win->Height-pw->pw_TextGadgetHeight) - (win->BorderBottom)-1;
			mg.mg_Scale = TRUE;
			mg.mg_ScaleBitMap = workbm;
			MagnifyBitMap(&mg);
			GetBitMapOffsets(mx,my,&bmx,&bmy,pw);
			DrawSelector(pw, bmx, bmy);
		}
		OutlineBitMapRegions(pw);
	}
}


void ScrollRight(struct ProjectWindow *pw,UWORD xdelta, WORD mx, WORD my)
{
	struct Project *proj;
	struct Window *win;
	UWORD bmright,magleft;
	WORD xoffset,magdelta;
	struct Magnify mg;
	UWORD	bmheight,winheight;
	WORD	bmx,bmy;

	win = pw->pw_sw.sw_Window;

	xoffset = pw->pw_XOffset;

	proj = pw->pw_Project;

	WaitForSubTaskDone(mainmsgport);
	bmright = (proj->pj_Width - xoffset);
	if( bmright > pw->pw_MagWidth )
	{
		EraseSelector(pw);

		xoffset += xdelta;
		bmright = (proj->pj_Width - xoffset);

		if(bmright < pw->pw_MagWidth)
			xoffset = proj->pj_Width - pw->pw_MagWidth;
			if(xoffset < 0) xoffset=0;

		xdelta = xoffset - (pw->pw_XOffset) ;

		pw->pw_XOffset = xoffset;

		if(xdelta)
		{
			magdelta = xdelta * pw->pw_PixelSize;
			magleft = (win->BorderLeft) + ( (pw->pw_MagWidth - xdelta) * pw->pw_PixelSize);

			ScrollRaster(win->RPort, magdelta, 0,
									 win->BorderLeft,
									 win->BorderTop,
									(pw->pw_MagWidth * pw->pw_PixelSize) + win->BorderLeft - 1,
									(pw->pw_MagHeight * pw->pw_PixelSize) + win->BorderTop - 1 );

			winheight = (win->Height-pw->pw_TextGadgetHeight) - (win->BorderBottom) - (win->BorderTop);
			bmheight = ( ((proj->pj_Height) - (pw->pw_YOffset)) * pw->pw_PixelSize );
			if( bmheight < winheight )
				winheight = bmheight;

			mg.mg_SrcBitMap = pw->pw_Project->pj_MainBitMap;
			mg.mg_SrcXOffset = pw->pw_XOffset + (pw->pw_MagWidth - xdelta);
			mg.mg_SrcYOffset = pw->pw_YOffset;
			mg.mg_DestRastPort = win->RPort;
			mg.mg_DestXOffset = magleft;
			mg.mg_DestYOffset = win->BorderTop;
			mg.mg_DestWidth = magdelta;
			mg.mg_DestHeight = winheight;
			mg.mg_DestPixelSize = pw->pw_PixelSize;
			mg.mg_ClearRight = (win->Width) - (win->BorderRight)-1;
			mg.mg_ClearBottom = (win->Height-pw->pw_TextGadgetHeight) - (win->BorderBottom)-1;
			mg.mg_Scale = TRUE;
			mg.mg_ScaleBitMap = workbm;
			MagnifyBitMap(&mg);
			GetBitMapOffsets(mx,my,&bmx,&bmy,pw);
			DrawSelector(pw,bmx, bmy);
		}
		OutlineBitMapRegions(pw);
	}
}


void GetTrueWindowSize(struct ProjectWindow *pw)
{
	struct Window *win;
	UWORD winwidth,winheight;

	if( (pw) && (win=pw->pw_sw.sw_Window) )
	{
		pw->pw_sw.sw_Left = win->LeftEdge;
		pw->pw_sw.sw_Top = win->TopEdge;
		pw->pw_sw.sw_Width = win->Width;
		pw->pw_sw.sw_Height = win->Height;

		winwidth = (win->Width) - (win->BorderRight) - (win->BorderLeft);
		winheight = (win->Height - pw->pw_TextGadgetHeight) - (win->BorderBottom) - (win->BorderTop);

		winwidth = winwidth / pw->pw_PixelSize;
		winheight = winheight / pw->pw_PixelSize;

		pw->pw_MagWidth = winwidth;
		pw->pw_MagHeight = winheight;
	}
}

/******************   RedisplayProjWindow   **************************/
// Fill in the whole window with the magnified picture.

void RedisplayProjWindow(struct ProjectWindow *pw, BOOL fullyredisplay)
{
	struct Window *win;
	struct Project *pj;
	struct Magnify mg;
	struct QuickMagnify qm;
	UWORD winwidth,winheight;
	UWORD bmwidth,bmheight;

	if( (!pw) || !(win = pw->pw_sw.sw_Window) || !(pj = pw->pw_Project) )
	{
		printf("RedisplayProjWindow: Bad Project\n");
		printf("pw: %d  pw->Window: %d  pj: %d\n", pw, pw->pw_sw.sw_Window, pj );
		return;
	}

	if( (!pj->pj_Disabled) && (pj->pj_MainBitMap) )
	{
		SetAPen(win->RPort,0);
		SetDrMd(win->RPort,JAM1);

		EraseSelector(pw);

		winwidth = (win->Width) - (win->BorderRight) - (win->BorderLeft);

		winheight = (win->Height-pw->pw_TextGadgetHeight)-(win->BorderBottom)-(win->BorderTop);

		if(fullyredisplay)
		{
			bmwidth = ( ((pj->pj_Width) - (pw->pw_XOffset)) * pw->pw_PixelSize );
			if( bmwidth < winwidth )
				winwidth = bmwidth;

			bmheight = ( ((pj->pj_Height) - (pw->pw_YOffset)) * pw->pw_PixelSize );
			if( bmheight < winheight )
				winheight = bmheight;

			if(pw->pw_PixelSize == 1)
			{
				RectFill(win->RPort, win->BorderLeft, win->BorderTop,
								 (win->Width)-(win->BorderRight)-1, (win->Height-pw->pw_TextGadgetHeight)-(win->BorderBottom)-1 );
				BltBitMapRastPort(pj->pj_MainBitMap, pw->pw_XOffset, pw->pw_YOffset,
													win->RPort, win->BorderLeft, win->BorderTop,
													winwidth, winheight, 0xC0);
			}
			else
			{
				mg.mg_SrcBitMap = pj->pj_MainBitMap;
				mg.mg_SrcXOffset = pw->pw_XOffset;
				mg.mg_SrcYOffset = pw->pw_YOffset;
				mg.mg_DestRastPort = win->RPort;
				mg.mg_DestXOffset = win->BorderLeft;
				mg.mg_DestYOffset = win->BorderTop;
				mg.mg_DestWidth = winwidth;
				mg.mg_DestHeight = winheight;
				mg.mg_DestPixelSize = pw->pw_PixelSize;
				mg.mg_ClearRight = (win->Width) - (win->BorderRight)-1;
				mg.mg_ClearBottom = (win->Height)-(win->BorderBottom)-1;
				if(workbm)
				{
					mg.mg_Scale = TRUE;
					mg.mg_ScaleBitMap = workbm;
				}
				else
				{
					mg.mg_Scale = FALSE;
					mg.mg_ScaleBitMap = NULL;
				}
				MagnifyBitMap(&mg);
			}
			DoProjGadgets(pw,FALSE);
		}
		else
		{
			bmwidth = ( ((pj->pj_Width) - (pw->pw_XOffset)) * pw->pw_PixelSize );
			if( bmwidth < winwidth )
				winwidth = bmwidth;

			bmheight = ( ((pj->pj_Height) - (pw->pw_YOffset)) * pw->pw_PixelSize );
			if( bmheight < winheight )
				winheight = bmheight;

			if(pw->pw_PixelSize == 1)
			{
				BltBitMapRastPort(pj->pj_MainBitMap, pw->pw_XOffset, pw->pw_YOffset,
													win->RPort, win->BorderLeft, win->BorderTop,
													winwidth, winheight, 0xC0);
			}
			else
			{
				qm.qm_SrcBitMap = pj->pj_MainBitMap;
				qm.qm_SrcXOffset = pw->pw_XOffset;
				qm.qm_SrcYOffset = pw->pw_YOffset;
				qm.qm_SrcWidth = winwidth / pw->pw_PixelSize;
				qm.qm_SrcHeight = winheight / pw->pw_PixelSize;
				qm.qm_DestBitMap = workbm;
				qm.qm_DestXOffset = 0;
				qm.qm_DestYOffset = 0;
				qm.qm_DestPixelSize = pw->pw_PixelSize;
				QuickMagnifyBitMap(&qm);
				BltBitMapRastPort(workbm,0,0,
													win->RPort,win->BorderLeft,win->BorderTop,
													winwidth,winheight,0xC0);
			}
			if( ( (pj->pj_EditType == EDTY_BOBS) || (pj->pj_EditType == EDTY_SPRITES) ) && (pw->pw_GTGadgets[0]) )
			{
				GT_SetGadgetAttrs(pw->pw_GTGadgets[0], win, NULL,
						GTST_String,		pj->pj_CurrentBob ? pj->pj_CurrentBob->bb_Node.ln_Name : "\0",
						TAG_END );
			}
		}
		SetAPen(win->RPort,globpens[SHINEPEN]);
		SetDrPt(win->RPort,0xAAAA);
		if( winwidth < ((win->Width) - (win->BorderRight) - (win->BorderLeft)) )
		{
			Move(win->RPort,winwidth+win->BorderLeft, win->BorderTop);
			Draw(win->RPort,winwidth+win->BorderLeft, winheight+win->BorderTop);
		}
		if( winheight < ((win->Height-pw->pw_TextGadgetHeight)-(win->BorderBottom)-(win->BorderTop)) )
		{
			Move(win->RPort,win->BorderLeft,					winheight+win->BorderTop);
			Draw(win->RPort,winwidth+win->BorderLeft,	winheight+win->BorderTop);
		}
		SetDrPt(win->RPort,0xFFFF);
	}
}


static void PrintDisabled(struct ProjectWindow *pw)
{
	struct Window *win;
	UWORD	midx,midy,winwidth,txtwidth,txtheight;

	if( (!pw) || !(win = pw->pw_sw.sw_Window) )
		return;

	if(win)
	{
		winwidth = win->Width - win->BorderRight - win->BorderLeft;
		midx = winwidth / 2;
		midy = ((win->Height-pw->pw_TextGadgetHeight) - win->BorderBottom - win->BorderTop) / 2;

		ClearWindow( win );

		/* use window font */
		SetFont(win->RPort, windowtextfont);
		pw->pw_sw.sw_TextAttr = &windowtextattr;
		pw->pw_sw.sw_TextFont = windowtextfont;

		/* fall back to Topaz 8 if the window is too small */
		AdjustWindowFont(&pw->pw_sw,(TextLength(win->RPort, "Disabled", 8) +
			+ win->BorderLeft + win->BorderRight ),
			(windowtextfont->tf_YSize * 2) );

		txtwidth = TextLength(win->RPort, "Disabled", 8);
		txtheight = pw->pw_sw.sw_TextFont->tf_YSize;

		SetAPen(win->RPort,globpens[TEXTPEN]);
		Move( win->RPort,midx-(txtwidth/2)+win->BorderLeft,midy-(txtheight/2)+win->BorderTop );
		Text(win->RPort,"Disabled",8);
	}
}


void SetMagnifyBoxSize(struct ProjectWindow *pw)
{
	struct Window *win;
	UWORD winwidth,winheight;

	if( (pw) && (win = pw->pw_sw.sw_Window) )
	{
		winwidth = win->Width - win->BorderLeft - win->BorderRight;
		winheight = (win->Height - pw->pw_TextGadgetHeight) - win->BorderTop - win->BorderBottom;

		pw->pw_MagnifyBoxWidth = (winwidth / pw->pw_MagnifyLevel);
		pw->pw_MagnifyBoxHeight = (winheight / pw->pw_MagnifyLevel);
	}
}

void GetBitMapOffsets(WORD mx, WORD my, WORD *bmx, WORD *bmy, struct ProjectWindow *pw)
{
	struct Project *pj;
	struct Window *win;

	pj = pw->pw_Project;
	win = pw->pw_sw.sw_Window;

	*bmx = pw->pw_XOffset + ( (mx - win->BorderLeft) / (pw->pw_PixelSize) );
	*bmy = pw->pw_YOffset + ( (my - win->BorderTop) / (pw->pw_PixelSize) );

	if(*bmx < 0)
		*bmx = 0;
	if(*bmy < 0)
		*bmy = 0;
	if(*bmx >= pj->pj_Width)
		*bmx = pj->pj_Width-1;
	if(*bmy >= pj->pj_Height)
		*bmy = pj->pj_Height-1;
}

/***************   GridLockOffsets()   ************************/
//
//  Snap the givin x & y registers to 8x8 gridlock. if the
// optional BrushInfo is givin it will lock the top left of the
// brush to the grid. The locks will truncate at the right & bottom.

void GridLockOffsets(WORD *bmx, WORD *bmy, struct BrushInfo *bi,
										 WORD width, WORD height)
{
	if(gridlockstate)
	{
		if(bi)
		{
			*bmx -= bi->bi_XHandle;
			*bmy -= bi->bi_YHandle;
		}
		if(*bmx < (width-1))
		{
			*bmx += 4;
			*bmx &= 0xFFF8;
		}
		if(*bmy < (height-1))
		{
			*bmy += 4;
			*bmy &= 0xFFF8;
		}
		if(bi)
		{
			*bmx += bi->bi_XHandle;
			*bmy += bi->bi_YHandle;
		}
	}
}

/***************   GetBrushGridLockOffsets()   ************************/
//
//  Snap the givin x & y registers to 8x8 gridlock.

void GetBrushGridLockOffsets(WORD *minx, WORD *miny, WORD *maxx, WORD *maxy)
{
	if(gridlockstate)
	{
		*minx += 4;
		*minx &= 0xFFF8;
		*miny += 4;
		*miny &= 0xFFF8;
		*maxx += 4;
		*maxx &= 0xFFF8;
		*maxy += 4;
		*maxy &= 0xFFF8;
	}
}


static void ActivateNextProjWindow(struct Project *currentpj)
{
	struct Project *pj;
	struct ProjectWindow *pw;
	struct Window *win;

	pj = (struct Project *)currentpj->pj_Node.ln_Succ;
	if( !pj->pj_Node.ln_Succ )
		pj = (struct Project *)projects.lh_Head;

	for(pw = pj->pj_ProjectWindows; pw; pw = pw->pw_NextProjWindow)
	{
		if(win = pw->pw_sw.sw_Window)
		{
			WindowToFront(win);
			ActivateWindow(win);
		}
	}
}

static void HandleBobNameGadget(struct ProjectWindow *pw, struct Gadget *gad)
{
	struct MyBob *bob;
	struct Project *pj;

	pj = pw->pw_Project;

	bob = pj->pj_CurrentBob;

	SetBobName(pj, bob, ((struct StringInfo *)gad->SpecialInfo)->Buffer);
	UpdateBobNameGadgets(pj, bob);
}

void OutlineBitMapRegions(struct ProjectWindow *pw)
{
	struct Window *win;
	UWORD	 winpixwidth, winwidth, minwidth, erasewidth;
	UWORD	 winpixheight, winheight, minheight, eraseheight;

	if(win = pw->pw_sw.sw_Window)
	{
		SetDrMd(win->RPort,JAM2);
		SetBPen(win->RPort,0);

		winwidth = win->Width - win->BorderLeft - win->BorderRight;
		winpixwidth = (pw->pw_Project->pj_Width - pw->pw_XOffset) * pw->pw_PixelSize;
		minwidth = (winpixwidth < winwidth) ? winpixwidth : winwidth;

		winheight = (win->Height - pw->pw_TextGadgetHeight) - win->BorderTop - win->BorderBottom;
		winpixheight = (pw->pw_Project->pj_Height - pw->pw_YOffset) * pw->pw_PixelSize;
		minheight = (winpixheight < winheight) ? winpixheight : winheight;

		if(winpixwidth >= winwidth)
		{
			SetAPen(win->RPort,0);
			erasewidth = pw->pw_MagWidth * pw->pw_PixelSize;
			if(erasewidth < winwidth)
			{
				Move(win->RPort,erasewidth+win->BorderLeft, win->BorderTop);
				Draw(win->RPort,erasewidth+win->BorderLeft, winheight-1+win->BorderTop);
			}
		}
		if(winpixheight >= winheight)
		{
			SetAPen(win->RPort,0);
			eraseheight = pw->pw_MagHeight * pw->pw_PixelSize;
			if(eraseheight < winheight)
			{
				Move(win->RPort,win->BorderLeft,						eraseheight+win->BorderTop);
				Draw(win->RPort,winwidth-1+win->BorderLeft,	eraseheight+win->BorderTop);
			}
		}
		if(winpixwidth < winwidth)
		{
			/* The right of the bm is displayed in the window */
			
			SetAPen(win->RPort,globpens[SHINEPEN]);
			SetDrPt(win->RPort,0xAAAA);
			Move(win->RPort,winpixwidth+win->BorderLeft, win->BorderTop);
			Draw(win->RPort,winpixwidth+win->BorderLeft, minheight-1+win->BorderTop);
			SetDrPt(win->RPort,0xFFFF);
		}
		if(winpixheight < winheight)
		{
			/* The bottom of the bm is displayed in the window */
			SetAPen(win->RPort,globpens[SHINEPEN]);
			SetDrPt(win->RPort,0xAAAA);
			Move(win->RPort,win->BorderLeft,					winpixheight+win->BorderTop);
			Draw(win->RPort,minwidth-1+win->BorderLeft,	winpixheight+win->BorderTop);
			SetDrPt(win->RPort,0xFFFF);
		}
	}
}
