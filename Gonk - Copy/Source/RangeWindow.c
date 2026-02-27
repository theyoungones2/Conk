#include <stdio.h>
//#include <stdarg.h>
#include <string.h>
//#include <dos/dos.h>
//#include <dos/dostags.h>
#include <exec/exec.h>
//#include <intuition/intuition.h>
#include <intuition/gadgetclass.h>
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

static struct NewMenu	rangemenu[] =
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

/* Don't forget to change RW_NUMOFGADGETS */
#define MYGT_RANGESCROLLER	0
#define MYGT_COLOURNUMBER	1
#define MYGT_RANGENUMBERSLIDER	2
#define MYGT_REVERTBUTTON	3
#define MYGT_UNDOBUTTON 4
#define MYGT_CLEARBUTTON 5
#define MYGT_REVERSEBUTTON 6
#define MYGT_OKBUTTON 7

/* labels for the Windows/Projects selector (a cycle gadget) */

//static UBYTE *cyclelabels[] = {
//	(UBYTE *)"256",
//	(UBYTE *)"16",
//	NULL };


/************ Prototypes for functions local to this file ************/

BOOL ShowRangeWindow(struct SuperWindow *sw);
BOOL SetupRangeWindowMenus(struct SuperWindow *sw);
void HideRangeWindow(struct SuperWindow *sw);
APTR KillOffRangeWindow(struct SuperWindow *sw);
void RangeWindowHandler(struct SuperWindow *sw,	struct IntuiMessage *imsg);
static void HandleGadgetUp(struct SuperWindow *sw, struct Gadget *gad, UWORD code);
static void SortOutRangeWindowMenus(struct SuperWindow *sw, struct IntuiMessage *imsg, UWORD code);
static void DoGadgets(struct RangeWindow *rw, BOOL clear);
static void RethinkRangeBox(struct RangeWindow *rw,
												UWORD winleft, UWORD wintop, UWORD winwidth, UWORD winheight);
static void DrawCBoxColour(struct RangeWindow *rw,UWORD col);
static void SetCurrentColour(struct RangeWindow *rw, UWORD col);
static void HandleMouseButtons(struct RangeWindow *rw, WORD mx, WORD my, UWORD button, BOOL shiftdown);
static void HandleMouseMove(struct RangeWindow *rw, struct Gadget *gad, UWORD code, WORD mx, WORD my);
static void InitSelectedColours(struct RangeWindow *rw);
static void RangeRestoreColour(struct RangeWindow *rw, UWORD col);
static void SelectColour(struct RangeWindow *rw, UWORD col);
static void DeselectColours(struct RangeWindow *rw);
static BOOL IsColourSelected(struct RangeWindow *rw, UWORD col);
static void AddSelectedColours(struct RangeWindow *rw, UWORD startcell);
static void SortOutMenuVerify(struct RangeWindow *rw,WORD mx,WORD my,
											 UWORD code, struct IntuiMessage *imsg);
static void CopyRange(struct Range *srcrange, struct Range *destrange);
static void DoRangeUndo(struct RangeWindow *rw);


/************ OpenNewRangeWindow() ************/
//
// Opens a RangeWindow.
//
// Inputs: Project = struct Project *, Range = Start Range

BOOL OpenNewRangeWindow(struct WindowDefault *wf, struct Project *pj)
{
	struct	RangeWindow		*rw;

	/* grab mem for the RangeWindow struct */
	if ( !( rw= AllocVec(sizeof(struct RangeWindow), MEMF_CLEAR ) ) )
		return(FALSE);

	/* figure out a unique ID for this window (mainly for AREXX use) */
	InstallUniqueWindowID((struct SuperWindow *)rw);
	rw->rw_sw.sw_Node.ln_Name = (char *)&(rw->rw_sw.sw_idstring);

	/* find the Default settings for this type of SuperWindow */
	//wf = &glob.cg_WindowDefaults[SWTY_RANGE];

	/* set up handler routines and assorted SuperWindow data */
	rw->rw_sw.sw_Handler = RangeWindowHandler;
	rw->rw_sw.sw_HideWindow = HideRangeWindow;
	rw->rw_sw.sw_ShowWindow = ShowRangeWindow;
	rw->rw_sw.sw_BuggerOff = KillOffRangeWindow;
	rw->rw_sw.sw_RefreshMenus = SetupRangeWindowMenus;
	rw->rw_sw.sw_Window = NULL;
	rw->rw_sw.sw_MenuStrip = NULL;
	rw->rw_sw.sw_GadList = NULL;
	rw->rw_sw.sw_Type = SWTY_RANGE;
	rw->rw_sw.sw_VisualInfo = NULL;

	/* set up stuff for opening the actual intuition window */
	rw->rw_sw.sw_Left = wf->wf_Left;
	rw->rw_sw.sw_Top = wf->wf_Top;
	rw->rw_sw.sw_Width = wf->wf_Width;
	rw->rw_sw.sw_Height = wf->wf_Height;
	rw->rw_sw.sw_MinWidth = 250;					/* minwidth set for topaz8 */
	rw->rw_sw.sw_MaxWidth = 0xFFFF;
	rw->rw_sw.sw_MaxHeight = 0XFFFF;
	rw->rw_sw.sw_MenuStrip = NULL;

	rw->rw_sw.sw_FlgBackDrop = wf->wf_FlgBackDrop;
	rw->rw_sw.sw_FlgBorderless = wf->wf_FlgBorderless;
	rw->rw_sw.sw_FlgTitle = wf->wf_FlgTitle;
	rw->rw_sw.sw_FlgBorderGadgets = wf->wf_FlgBorderGadgets;

	rw->rw_Project = pj;
	rw->rw_NextRangeWindow = pj->pj_RangeWindows;
	pj->pj_RangeWindows = rw;
	rw->rw_CurrentRangeNumber = pj->pj_CurrentRangeNumber;
	rw->rw_CurrentRange = &pj->pj_Ranges[rw->rw_CurrentRangeNumber];
	InitSelectedColours(rw);
	rw->rw_ButtonDown = FALSE;
	rw->rw_DragAnchorColour = 0xFFFF;
	rw->rw_DragLastColour = 0xFFFF;

	CopyRange(&pj->pj_Ranges[0], &rw->rw_RevertRange[0]);
	CopyRange(&pj->pj_Ranges[1], &rw->rw_RevertRange[1]);
	CopyRange(&pj->pj_Ranges[2], &rw->rw_RevertRange[2]);
	CopyRange(&pj->pj_Ranges[3], &rw->rw_RevertRange[3]);
	CopyRange(&pj->pj_Ranges[4], &rw->rw_RevertRange[4]);
	CopyRange(&pj->pj_Ranges[5], &rw->rw_RevertRange[5]);
	CopyRange(&pj->pj_Ranges[6], &rw->rw_RevertRange[6]);
	CopyRange(&pj->pj_Ranges[7], &rw->rw_RevertRange[7]);
	CopyRange(rw->rw_CurrentRange, &rw->rw_UndoRange);

	/* setup SuperWindow basename */
	rw->rw_sw.sw_Title = "Range";
	rw->rw_PaletteBox.pb_Palette = &pj->pj_Palette;

	/* add window into the superwindows list */
	AddTail(&superwindows, &rw->rw_sw.sw_Node);

	/* open the intuition window (if the window doesn't open, */
	/* it is just left hidden - SEP). */
	ShowRangeWindow(&rw->rw_sw);

	return(TRUE);
}



/************ ShowRangeWindow() ************/
//
// Displays a RangeWindow, and sets up all its gadgets, lists
// etc...
//

BOOL ShowRangeWindow(struct SuperWindow *sw)
{
	struct RangeWindow *rw;
	UWORD minh;
	ULONG flags;

	rw = (struct RangeWindow *)sw;

	if(!sw->sw_Window)				/* make sure it isn't already open */
	{
		/* Set up the Visual Info for GadTools */
		if (!(rw->rw_sw.sw_VisualInfo = GetVisualInfo(MAINSCREEN,TAG_END)))
			return(FALSE);

		minh = MAINSCREEN->Font->ta_YSize+1 +	MAINSCREEN->WBorTop + ((8+5)*7);

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
		((struct RangeWindow *)sw)->rw_PaletteBox.pb_Window = sw->sw_Window;

		DoGadgets(rw,TRUE);

		/* Tell the window which superwindow it is attached to */
		/* (so we'll know which handler routine to call) */
		sw->sw_Window->UserData = (BYTE *)rw;

		SetupRangeWindowMenus(sw);

		/* we want to use our global message port for IDCMP stuff */
		sw->sw_Window->UserPort = mainmsgport;
		ModifyIDCMP(sw->sw_Window, IDCMP_CLOSEWINDOW | IDCMP_MENUPICK |
			IDCMP_REFRESHWINDOW | SLIDERIDCMP | SCROLLERIDCMP | BUTTONIDCMP | ARROWIDCMP |
			IDCMP_MOUSEMOVE | IDCMP_MOUSEBUTTONS | IDCMP_MENUVERIFY |
			IDCMP_NEWSIZE | IDCMP_CHANGEWINDOW);

		SetSuperWindowScreenTitle(sw);
	}
	return(TRUE);
}


BOOL SetupRangeWindowMenus(struct SuperWindow *sw)
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
	SetupWindowMenu( sw, &rangemenu[MN_WINDOW_IDX] );
	SetupEditMenu( NULL, &rangemenu[MN_EDIT_IDX] );
	SetupSettingsMenu( &rangemenu[MN_SETTINGS_IDX] );
	SetupModeMenu( sw, &rangemenu[MN_MODE_IDX] );
	LinkInUserMenu( &rangemenu[MN_USER_IDX] );

	if( sw->sw_MenuStrip = CreateMenus( rangemenu, TAG_END ) )
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


/******************  HideRangeWindow()  ******************/
//
// Hides the given RangeWindow. Frees gadgets and stuff.
//

void HideRangeWindow(struct SuperWindow *sw)
{
	struct RangeWindow *rw;

	rw = (struct RangeWindow *)sw;

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
		((struct RangeWindow *)sw)->rw_PaletteBox.pb_Window = NULL;
		if(rw->rw_sw.sw_GadList) FreeGadgets(rw->rw_sw.sw_GadList);
		rw->rw_sw.sw_GadList = NULL;
		if (sw->sw_VisualInfo) FreeVisualInfo(sw->sw_VisualInfo);
		sw->sw_VisualInfo = NULL;
		SetProcessWindowPtr();
	}
}


/******************  KillOffRangeWindow()  ******************/
//
// Shuts down a RangeWindow and removes it from the SuperWindows
// list.
//

APTR KillOffRangeWindow(struct SuperWindow *sw)
{
	struct RangeWindow *rw;

	rw = (struct RangeWindow *)sw;

	/* close/free the superwindow */
	HideRangeWindow(sw);												/* shut down the Intuition window */
	Remove(&(sw->sw_Node));											/* remove from superwindows list */
	FreeVec(rw);																/* free RevealWindow struct */
	return(NULL);
}

/******************  RangeWindowHandler()  ******************/
//
// Processes IDCMP messages coming into any RangeWindows,
// dispatching appropriate bits of code.
//

void RangeWindowHandler(struct SuperWindow *sw, struct IntuiMessage *imsg)
{
	WORD mx,my;
	ULONG	class;
	UWORD	code;
	struct IntuiMessage *gtimsg;
	struct Gadget *gad;
	UWORD qual;

	if( gtimsg = GT_FilterIMsg(imsg) )
	{
		/* read out all we need */
		class = gtimsg->Class;
		mx = gtimsg->MouseX;
		my = gtimsg->MouseY;
		code = gtimsg->Code;
		gad = (struct Gadget *)gtimsg->IAddress;
		qual = gtimsg->Qualifier;

		GT_PostFilterIMsg(gtimsg);

		switch(class)
		{
			case IDCMP_CLOSEWINDOW:
				/* tell this window to bugger off */
				ReplyMsg((struct Message *)imsg);
				KillOffRangeWindow(sw);
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
				HandleMouseMove((struct RangeWindow *)sw,gad,code,mx,my);
				break;
			case IDCMP_GADGETUP:
				ReplyMsg((struct Message *)imsg);
				HandleGadgetUp(sw,gad,code);
				break;
			case IDCMP_MENUPICK:
				/* menuroutine does the reply() */
				SortOutRangeWindowMenus( sw,imsg,code );
				break;
			case IDCMP_NEWSIZE:
				/* resuss all the gadget placements/font stuff */
				/* according to the new windowsize. */
				DoGadgets((struct RangeWindow *)sw,TRUE);
				break;
			case IDCMP_MOUSEBUTTONS:
				ReplyMsg((struct Message *)imsg);
				HandleMouseButtons((struct RangeWindow *)sw, mx,my,code,((qual &IEQUALIFIER_LSHIFT) || (qual & IEQUALIFIER_RSHIFT)) ? TRUE : FALSE);
				break;
			case IDCMP_MENUVERIFY:
				SortOutMenuVerify((struct RangeWindow *)sw,mx,my,code,imsg);
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

static void HandleMouseMove(struct RangeWindow *rw, struct Gadget *gad, UWORD code, WORD mx, WORD my)
{
	WORD oldmincol,oldmaxcol,newmincol,newmaxcol,col,newlastcol;
	BOOL forgadget = FALSE;
	struct Project *pj;
	struct PaletteBox *pb;

	pj = rw->rw_Project;
	pb = &rw->rw_PaletteBox;
	if(gad)
	{
		if(gad == rw->rw_Gadgets[MYGT_RANGESCROLLER])
		{
			rw->rw_FirstRangeCell = code;
			RethinkRangeBox(rw,rw->rw_RangeBoxLeft,rw->rw_RangeBoxTop,rw->rw_RangeBoxWidth,rw->rw_RangeBoxHeight);
			forgadget = TRUE;
		}
		if(gad == rw->rw_Gadgets[MYGT_RANGENUMBERSLIDER])
		{
			rw->rw_CurrentRangeNumber = code-1;
			pj->pj_CurrentRangeNumber = rw->rw_CurrentRangeNumber;
			rw->rw_CurrentRange = &pj->pj_Ranges[rw->rw_CurrentRangeNumber];
			pj->pj_CurrentRange = &pj->pj_Ranges[rw->rw_CurrentRangeNumber];
			CopyRange(rw->rw_CurrentRange, &rw->rw_UndoRange);
			RethinkRangeBox(rw,rw->rw_RangeBoxLeft,rw->rw_RangeBoxTop,rw->rw_RangeBoxWidth,rw->rw_RangeBoxHeight);
			forgadget = TRUE;
		}
	}
	if(!forgadget)
	{
		if(rw->rw_ButtonDown)
		{
			if(mx < pb->pb_BoxLeft) mx = pb->pb_BoxLeft;
			if(my < pb->pb_BoxTop) my = pb->pb_BoxTop;
			if(mx > (pb->pb_BoxLeft + pb->pb_BoxWidth - 1)) mx = pb->pb_BoxLeft + pb->pb_BoxWidth - 1;
			if(my > (pb->pb_BoxTop + pb->pb_BoxHeight - 1)) my = pb->pb_BoxTop + pb->pb_BoxHeight - 1;

			mx -= pb->pb_BoxLeft;
			my -= pb->pb_BoxTop;
			
			if(rw->rw_DragAnchorColour < rw->rw_DragLastColour)
			{
				oldmincol = rw->rw_DragAnchorColour;
				oldmaxcol = rw->rw_DragLastColour;
			}
			else
			{
				oldmincol = rw->rw_DragLastColour;
				oldmaxcol = rw->rw_DragAnchorColour;
			}
			newlastcol = ((mx / pb->pb_ColourWidth) * pb->pb_ColoursHigh) +
						(my / pb->pb_ColourHeight);
			if(rw->rw_DragAnchorColour < newlastcol)
			{
				newmincol = rw->rw_DragAnchorColour;
				newmaxcol = newlastcol;
			}
			else
			{
				newmincol = newlastcol;
				newmaxcol = rw->rw_DragAnchorColour;
			}

			if(oldmincol < newmincol)
			{
				for(col = oldmincol; col <= (newmincol-1); col++) RangeRestoreColour(rw,col);
			}
			if(newmaxcol < oldmaxcol)
			{
				for(col = newmaxcol+1; col <= oldmaxcol; col++) RangeRestoreColour(rw,col);
			}
			if(newmincol < oldmincol)
			{
				for(col = newmincol; col <= (oldmincol-1); col++) BevelColour(&rw->rw_PaletteBox,col);
			}
			if(oldmaxcol < newmaxcol)
			{
				for(col = oldmaxcol+1; col <= newmaxcol; col++) BevelColour(&rw->rw_PaletteBox,col);
			}
			rw->rw_DragLastColour = newlastcol;
		}
	}
}

/****************  HandleGadgetUp()  ****************/
//
// Process GADGETUP IDCMP events for the RevealWindow.
//

static void HandleGadgetUp(struct SuperWindow *sw, struct Gadget *gad, UWORD code)
{
	struct RangeWindow *rw;
	WORD cell,stopcell,leftcell;
	struct Range *rn;
	UWORD tmpcell;

	rw = (struct RangeWindow *)sw;

	switch(gad->GadgetID)
	{
		case MYGT_OKBUTTON:
			KillOffRangeWindow(sw);
			SussOutQuiting(FALSE);
			break;
		case MYGT_REVERSEBUTTON:
			rn = rw->rw_CurrentRange;
			for(cell = 255; (cell > 0) && (rn->rn_Cell[cell] == 0xFFFF); cell--);
			/* No use reversing it if either the range is empty or there is only one cell */
			if(cell > 0)
			{
				CopyRange(rw->rw_CurrentRange, &rw->rw_UndoRange);
				leftcell = 0;
				for(stopcell = cell/2; (cell > stopcell); cell--)
				{
					tmpcell = rn->rn_Cell[leftcell];
					rn->rn_Cell[leftcell] = rn->rn_Cell[cell];
					rn->rn_Cell[cell] = tmpcell;
					leftcell++;
				}
				RethinkRangeBox(rw,rw->rw_RangeBoxLeft,rw->rw_RangeBoxTop,rw->rw_RangeBoxWidth,rw->rw_RangeBoxHeight);
			}
			break;
		case MYGT_UNDOBUTTON:
			DoRangeUndo(rw);
			break;
		case MYGT_REVERTBUTTON:
			CopyRange(rw->rw_CurrentRange, &rw->rw_UndoRange);
			CopyRange(&rw->rw_RevertRange[rw->rw_CurrentRangeNumber], rw->rw_CurrentRange);
			RethinkRangeBox(rw,rw->rw_RangeBoxLeft,rw->rw_RangeBoxTop,rw->rw_RangeBoxWidth,rw->rw_RangeBoxHeight);
			break;
		case MYGT_CLEARBUTTON:
			CopyRange(rw->rw_CurrentRange, &rw->rw_UndoRange);
			for(cell = 0; cell < 256; cell++)
				rw->rw_CurrentRange->rn_Cell[cell] = 0xFFFF;			
			RethinkRangeBox(rw,rw->rw_RangeBoxLeft,rw->rw_RangeBoxTop,rw->rw_RangeBoxWidth,rw->rw_RangeBoxHeight);
			break;
	}
}

/*
/******************   HandleCancelGadget()   ********************/
//
//  Reset the palette back to the way it was before we opened the
// EditPaletteWindow. We let another routine ( RemakeScreenIfNeeded() )
// handle most of the crap.

static void HandleCancelGadget(struct EditPaletteWindow *epw)
{
	KillOffEditPaletteWindow(&epw->epw_sw);

	if( !RemakeScreenIfNeeded(globpalette) )
		InstallPalette(mainscreen,globpalette);
	SussOutQuiting(FALSE);
}
*/

/************* SortOutRangeWindowMenus() **************/
//
// Handle MENUPICK IDCMP messages - note the reply() is given
// here rather than in the main loop.
// Why is this so? Well... who can tell?
//

static void SortOutRangeWindowMenus(struct SuperWindow *sw,
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


/************** DoGadgets() **************/
//
// Sets up and formats all the gadgets within the RangeWindow.
// Adapts to font and window sizes.
// Call at window opening, and again at NEWSIZE events.

static void DoGadgets(struct RangeWindow *rw, BOOL clear)
{
	struct Gadget *gad;
	struct NewGadget ng;
	struct Window *win;
	UWORD buttonh,buttonw;
	UWORD	wintop,winbottom,winleft,winright,wingadgetright;
	UWORD minwidth,minheight;

	win = rw->rw_sw.sw_Window;

	/* first, kill any old gadgets that may be active */
	if(rw->rw_sw.sw_GadList)
	{
		RemoveGList(win, rw->rw_sw.sw_GadList, -1);
		FreeGadgets(rw->rw_sw.sw_GadList);
		rw->rw_sw.sw_GadList = NULL;
	}

	if(clear)	ClearWindow( win );

	/* use window font */
	SetFont(win->RPort,windowtextfont);
	rw->rw_sw.sw_TextAttr = &windowtextattr;
	rw->rw_sw.sw_TextFont = windowtextfont;

	buttonh = win->RPort->Font->tf_YSize + 5;
	buttonw = TextLength(win->RPort,"REVERSE",7)+4;
	minheight = buttonh*7;
	minwidth  = (buttonw*2)+32;

	/* fall back to Topaz 8 if the window is too small */
	AdjustWindowFont(&rw->rw_sw, minwidth, minheight);

	/* init gadget stuff */
	gad = CreateContext(&rw->rw_sw.sw_GadList);

	buttonh = win->RPort->Font->tf_YSize + 3;
	buttonw = TextLength(win->RPort,"REVERSE",8)+4;
	wintop = win->BorderTop;
	winbottom = win->Height - win->BorderBottom-1;
	winleft = win->BorderLeft;
	winright = win->Width - win->BorderRight-1;
	wingadgetright = ((winright-winleft)/4)+winleft;

	if(clear)
		RethinkPaletteBox(&rw->rw_PaletteBox, winleft, wintop, (wingadgetright-winleft)+1, (winbottom-wintop)+1);

	winleft += rw->rw_PaletteBox.pb_BoxWidth+2;

	if(clear)
		RethinkRangeBox(rw, winleft, wintop, (winright-winleft)+1, buttonh);

	ng.ng_TextAttr = rw->rw_sw.sw_TextAttr;					/* Range Scroller */
	ng.ng_VisualInfo = rw->rw_sw.sw_VisualInfo;
	ng.ng_LeftEdge = winleft;
	ng.ng_TopEdge = wintop+buttonh+1;
	ng.ng_Width = winright-winleft;
	ng.ng_Height = buttonh;
	ng.ng_GadgetText = NULL;
	ng.ng_GadgetID = MYGT_RANGESCROLLER;
	ng.ng_Flags = 0;
	gad = CreateGadget( SCROLLER_KIND, gad, &ng,
		GTSC_Top,					0,
		GTSC_Total,				256,
		GTSC_Visible,			rw->rw_RangeColoursWide,
		GTSC_Arrows,			12,
		PGA_Freedom,			LORIENT_HORIZ,
		TAG_END );
	rw->rw_Gadgets[MYGT_RANGESCROLLER] = gad;

	ng.ng_TopEdge += buttonh+1;													/* Colour Number Gadget */
	ng.ng_Width = buttonw;
	ng.ng_GadgetText = NULL;
	ng.ng_GadgetID = MYGT_COLOURNUMBER;
	gad = CreateGadget( NUMBER_KIND, gad, &ng,
		GTNM_Number,			6,
		GTNM_Border,			TRUE,
		TAG_END );
	rw->rw_Gadgets[MYGT_COLOURNUMBER] = gad;

	ng.ng_LeftEdge += buttonw+1;												/* Bevel Box */
	DrawBevelBox(win->RPort,ng.ng_LeftEdge,ng.ng_TopEdge,ng.ng_Width,ng.ng_Height,
		GT_VisualInfo,		ng.ng_VisualInfo,
		GTBB_Recessed,		TRUE);
	rw->rw_CBoxLeft = ng.ng_LeftEdge+3;
	rw->rw_CBoxTop = ng.ng_TopEdge+2;
	rw->rw_CBoxWidth = buttonw-6;
	rw->rw_CBoxHeight = buttonh-4;

	ng.ng_TopEdge += buttonh+1;													/* Revert Button */
	ng.ng_LeftEdge = winleft;
	ng.ng_GadgetText = "Revert";
	ng.ng_GadgetID = MYGT_REVERTBUTTON;
	ng.ng_Flags = 0;
	gad = CreateGadget( BUTTON_KIND, gad, &ng, TAG_END );
	rw->rw_Gadgets[MYGT_REVERTBUTTON] = gad;

	ng.ng_LeftEdge += buttonw+1;												/* Undo Button */
	ng.ng_GadgetText = "Undo";
	ng.ng_GadgetID = MYGT_UNDOBUTTON;
	ng.ng_Flags = 0;
	gad = CreateGadget( BUTTON_KIND, gad, &ng, TAG_END );
	rw->rw_Gadgets[MYGT_UNDOBUTTON] = gad;

	ng.ng_TopEdge += buttonh+1;													/* Clear Button */
	ng.ng_LeftEdge = winleft;
	ng.ng_GadgetText = "Clear";
	ng.ng_GadgetID = MYGT_CLEARBUTTON;
	ng.ng_Flags = 0;
	gad = CreateGadget( BUTTON_KIND, gad, &ng, TAG_END );
	rw->rw_Gadgets[MYGT_CLEARBUTTON] = gad;

	ng.ng_LeftEdge += buttonw+1;												/* Reverse Button */
	ng.ng_GadgetText = "Reverse";
	ng.ng_GadgetID = MYGT_REVERSEBUTTON;
	ng.ng_Flags = 0;
	gad = CreateGadget( BUTTON_KIND, gad, &ng, TAG_END );
	rw->rw_Gadgets[MYGT_REVERSEBUTTON] = gad;

	ng.ng_LeftEdge = winleft;
	ng.ng_TopEdge += buttonh+1;
	ng.ng_GadgetText = NULL;
	ng.ng_GadgetID = MYGT_RANGENUMBERSLIDER;
	ng.ng_Flags = 0;
	gad = CreateGadget( SLIDER_KIND, gad, &ng,
		GTSL_Min,					1,
		GTSL_Max,					8,
		GTSL_Level,				rw->rw_CurrentRangeNumber+1,
		GTSL_MaxLevelLen,	1,
		GTSL_LevelFormat,	"%1ld",
		GTSL_LevelPlace,	PLACETEXT_BELOW,
		TAG_END );
	rw->rw_Gadgets[MYGT_RANGENUMBERSLIDER] = gad;

	ng.ng_LeftEdge += buttonw+1;												/* Ok Button */
	ng.ng_GadgetText = "Ok";
	ng.ng_GadgetID = MYGT_OKBUTTON;
	ng.ng_Flags = 0;
	gad = CreateGadget( BUTTON_KIND, gad, &ng, TAG_END );
	rw->rw_Gadgets[MYGT_OKBUTTON] = gad;

	if (!gad)
	{
		FreeGadgets(rw->rw_sw.sw_GadList);
		return;
	}

	/* attach our groovy new gadgets to the window */
	AddGList(win, rw->rw_sw.sw_GadList, -1, -1, NULL);
//	RefreshGList(epw->epw_sw.sw_GadList, win, NULL, -1);
	GT_RefreshWindow(win, NULL);

	if(clear)
		RefreshWindowFrame(win);

	SetCurrentColour(rw,rw->rw_CurrentColour);
}


/*********************   RethinkRangeBox()   ********************/
//
//  This routine will fit the Range into the given rectangle.
//

static void RethinkRangeBox(struct RangeWindow *rw,
												UWORD winleft, UWORD wintop, UWORD winwidth, UWORD winheight)
{
	struct Window *win;
	struct Range *rn;
	UWORD colourswide;
	UWORD colourwidth;
	UWORD x,y;
	UWORD countx,col;

	win = rw->rw_sw.sw_Window;
	if(win)
	{
		colourswide = winwidth/8;

		colourwidth = 8;

		rw->rw_RangeBoxLeft = winleft;
		rw->rw_RangeBoxTop = wintop;
		rw->rw_RangeBoxWidth = colourwidth * colourswide;
		rw->rw_RangeBoxHeight = winheight;
		rw->rw_RangeColourWidth = colourwidth;
		rw->rw_RangeColourHeight = winheight;
		rw->rw_RangeColoursWide = colourswide;

		x = rw->rw_RangeBoxLeft;
		y = rw->rw_RangeBoxTop;
		rn = rw->rw_CurrentRange;
		for(countx = rw->rw_FirstRangeCell;
				countx < rw->rw_FirstRangeCell+colourswide;
				countx++)
		{
			col = rn->rn_Cell[countx];
			if(col != 0xFFFF)
			{
				SetAPen(win->RPort,col);
				RectFill(win->RPort,x,y,x+colourwidth-1,y+winheight-1);
			}
			else
			{
				SetAPen(win->RPort,0);
				RectFill(win->RPort,x,y,x+colourwidth-1,y+winheight-1);
				SetAPen(win->RPort,globpens[SHADOWPEN]);
				Move(win->RPort,x+1,y+1);
				Draw(win->RPort,x+colourwidth-2,y+winheight-2);
				Move(win->RPort,x+1,y+winheight-2);
				Draw(win->RPort,x+colourwidth-2,y+1);
			}
			x += colourwidth;
		}
	}
}

static void DrawCBoxColour(struct RangeWindow *rw,UWORD col)
{
	struct Window *win;
	UWORD minx,miny,maxx,maxy;

	if(win = rw->rw_sw.sw_Window)
	{
		minx = rw->rw_CBoxLeft;
		miny = rw->rw_CBoxTop;
		maxx = rw->rw_CBoxLeft+rw->rw_CBoxWidth-1;
		maxy = rw->rw_CBoxTop+rw->rw_CBoxHeight-1;
		if(col != 0xFFFF)
		{
			SetAPen(win->RPort,col);
			RectFill(win->RPort,minx,miny,maxx,maxy);
			GT_SetGadgetAttrs(rw->rw_Gadgets[MYGT_COLOURNUMBER], rw->rw_sw.sw_Window, NULL,
												GTNM_Number, col,
												TAG_END);
		}
		else
		{
			SetAPen(win->RPort,0);
			RectFill(win->RPort,minx,miny,maxx,maxy);
			SetAPen(win->RPort,globpens[SHADOWPEN]);
			Move(win->RPort,minx,miny);
			Draw(win->RPort,maxx,maxy);
			Move(win->RPort,minx,maxy);
			Draw(win->RPort,maxx,miny);
		}
	}
}

static void SetCurrentColour(struct RangeWindow *rw, UWORD col)
{
	rw->rw_CurrentColour = col;
	DrawCBoxColour(rw,col);
}

/*********************   SortOutMenuVerify   ********************/

static void SortOutMenuVerify(struct RangeWindow *rw,WORD mx,WORD my,
											 UWORD code, struct IntuiMessage *imsg)
{
	UWORD qual;

	qual = imsg->Qualifier;

	if( code == MENUHOT )
	{
		/* the menu event came from this window, so we can cancel it */
		/* if we feel the inclination. */

		/* check mouse position against window boundaries */
		if( (imsg->MouseX >= rw->rw_RangeBoxLeft) && (imsg->MouseX < (rw->rw_RangeBoxLeft+rw->rw_RangeBoxWidth)) &&
			( (imsg->MouseY >= rw->rw_RangeBoxTop)  && (imsg->MouseY < (rw->rw_RangeBoxTop +rw->rw_RangeBoxHeight)) &&
			(qual & IEQUALIFIER_RBUTTON)) )
		{
			/* RMB down within window - cancel the menu, */
			/* and send the event on as a SELECTDOWN */
			imsg->Code = MENUCANCEL;
			ReplyMsg((struct Message *)imsg);
			HandleMouseButtons(rw, mx, my, IECODE_RBUTTON, ((qual &IEQUALIFIER_LSHIFT) || (qual & IEQUALIFIER_RSHIFT)) ? TRUE : FALSE);
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

/***********************   HandleMouseButtons()   ********************/
//
//  This sorts out the user mouse button events, and sets the current
// colour if the user selected one.

static void HandleMouseButtons(struct RangeWindow *rw, WORD mx, WORD my, UWORD button, BOOL shiftdown)
{
	WORD col,cell;
	struct PaletteBox *pb;

	pb = &rw->rw_PaletteBox;

	if( button & IECODE_UP_PREFIX )
	{
		if( (button&IECODE_LBUTTON) && (rw->rw_ButtonDown) && (rw->rw_DragAnchorColour != 0xFFFF) )
		{
			if(rw->rw_DragAnchorColour < rw->rw_DragLastColour)
			{
				for(col = rw->rw_DragAnchorColour; col <= rw->rw_DragLastColour; col++)
					SelectColour(rw,col);
			}
			else
			{
				for(col = rw->rw_DragAnchorColour; col >= rw->rw_DragLastColour; col--)
					SelectColour(rw,col);
			}
			rw->rw_ButtonDown = FALSE;
			rw->rw_DragAnchorColour = 0xFFFF;
			rw->rw_DragLastColour = 0xFFFF;		
		}
	}
	else
	{
		if( (mx >= pb->pb_BoxLeft) &&
				(my >= pb->pb_BoxTop) &&
				(mx <= (pb->pb_BoxLeft + pb->pb_BoxWidth - 1)) &&
				(my <= (pb->pb_BoxTop + pb->pb_BoxHeight - 1)) )
		{
			mx -= pb->pb_BoxLeft;
			my -= pb->pb_BoxTop;

			col = ((mx / pb->pb_ColourWidth) * pb->pb_ColoursHigh) +
						(my / pb->pb_ColourHeight);
	
			if(!shiftdown)
				DeselectColours(rw);
			SetCurrentColour(rw,col);
			BevelColour(&rw->rw_PaletteBox,col);
			rw->rw_ButtonDown = TRUE;
			rw->rw_DragAnchorColour = col;
			rw->rw_DragLastColour = col;
		}
		else
		{
			if(	(!rw->rw_ButtonDown) &&
					(mx >= rw->rw_RangeBoxLeft) &&
					(my >= rw->rw_RangeBoxTop) &&
					(mx <= (rw->rw_RangeBoxLeft + rw->rw_RangeBoxWidth - 1)) &&
					(my <= (rw->rw_RangeBoxTop + rw->rw_RangeBoxHeight - 1)) )
			{
				mx -= rw->rw_RangeBoxLeft;
				my -= rw->rw_RangeBoxTop;

				cell = (mx / rw->rw_RangeColourWidth)+rw->rw_FirstRangeCell;

				CopyRange(rw->rw_CurrentRange, &rw->rw_UndoRange);
				if(button == IECODE_RBUTTON)
				{
					rw->rw_CurrentRange->rn_Cell[cell] = 0xFFFF;
					RethinkRangeBox(rw,rw->rw_RangeBoxLeft,rw->rw_RangeBoxTop,rw->rw_RangeBoxWidth,rw->rw_RangeBoxHeight);
				}
				else
					AddSelectedColours(rw,cell);
				return;
			}
			else
			{
				if(rw->rw_CurrentColour != 0xFFFF)
				{
					DeselectColours(rw);
					SetCurrentColour(rw,0xFFFF);
				}
			}
		}
	}
}

/**********************   InitSelectedColours()   ***************/

static void InitSelectedColours(struct RangeWindow *rw)
{
	memset(rw->rw_SelectedColours,0xFF,512);
	rw->rw_CurrentColour = 0xFFFF;
	//if(rw->rw_PaletteBoxWidth)
	//	RethinkPaletteBox(rw,rw->rw_PaletteBoxLeft,rw->rw_PaletteBoxTop,rw->rw_PaletteBoxWidth,rw->rw_PaletteBoxHeight);
	//if(rw->rw_CBoxWidth)
	//	DrawCBoxColour(rw, rw->rw_CurrentColour);
}

/**********************   RangeRestoreColour()   ********************/

static void RangeRestoreColour(struct RangeWindow *rw, UWORD col)
{
	struct Window *win;

	if( (win = rw->rw_sw.sw_Window) && (col != 0xFFFF) )
	{
		RestoreColour(&rw->rw_PaletteBox,col);
		if( IsColourSelected(rw,col) )
			BevelColour(&rw->rw_PaletteBox,col);
	}
}

/**********************   SelectColour()   ***********************/

static void SelectColour(struct RangeWindow *rw, UWORD col)
{
	UWORD count;

	if(col == 0xFFFF)
		return;

	for(count = 0;
		 ((count < 256) && (rw->rw_SelectedColours[count] != 0xFFFF));
		 count++)
	{
		if(rw->rw_SelectedColours[count] == col)
			return;
	}
	rw->rw_SelectedColours[count] = col;
	BevelColour(&rw->rw_PaletteBox,col);
}

/**********************   DeselectColours()   *********************/

static void DeselectColours(struct RangeWindow *rw)
{
	UWORD count;
	UWORD col;

	for(count = 0;
		 ((count < 256) && (rw->rw_SelectedColours[count] != 0xFFFF));
		 count++)
	{
		col = rw->rw_SelectedColours[count];
		rw->rw_SelectedColours[count] = 0xFFFF;
		RestoreColour(&rw->rw_PaletteBox,col);
	}
}

/*******************   GetColourCellInRange()   ********************/

UWORD GetColourCellInRange(struct Range *rn, UWORD col)
{
	UWORD cell;

	for(cell = 0; cell < 256; cell++)
	{
		if(rn->rn_Cell[cell] == col)
			return(cell);
	}
	return(0xFFFF);
}

/********************   IsColourSelected()   ***********************/

static BOOL IsColourSelected(struct RangeWindow *rw, UWORD col)
{
	UWORD cell;

	for(cell = 0;
		 (cell < 256) && (rw->rw_SelectedColours[cell] != 0xFFFF);
			cell++)
	{
		if(rw->rw_SelectedColours[cell] == col)
			return(TRUE);
	}
	return(FALSE);	
}

/********************   AddSelectedColours()   **********************/

static void AddSelectedColours(struct RangeWindow *rw, UWORD startcell)
{
	struct Range *rn;
	UWORD selectcell=0;
	UWORD oldcol;

	rn = rw->rw_CurrentRange;

	oldcol = rn->rn_Cell[startcell];

	if(rw->rw_CurrentColour == 0xFFFF)
	{
		rn->rn_Cell[startcell] = 0xFFFF;
		DeselectColours(rw);
		SelectColour(rw,oldcol);
		SetCurrentColour(rw,oldcol);
		RethinkRangeBox(rw,rw->rw_RangeBoxLeft,rw->rw_RangeBoxTop,rw->rw_RangeBoxWidth,rw->rw_RangeBoxHeight);
		return;
	}

	while( (selectcell < 256) &&
				 (rw->rw_SelectedColours[selectcell] != 0xFFFF) &&
				 (startcell < 256) )
	{
		rn->rn_Cell[startcell] = rw->rw_SelectedColours[selectcell];
		selectcell++;
		startcell++;
	}
	DeselectColours(rw);
	SelectColour(rw,oldcol);
	SetCurrentColour(rw,oldcol);
	RethinkRangeBox(rw,rw->rw_RangeBoxLeft,rw->rw_RangeBoxTop,rw->rw_RangeBoxWidth,rw->rw_RangeBoxHeight);
}

/***********************   CopyRange()   *****************************/

static void CopyRange(struct Range *srcrange, struct Range *destrange)
{
	UWORD cell;

	for(cell = 0; cell <= 255; cell++)
		destrange->rn_Cell[cell] = srcrange->rn_Cell[cell];
}

/***********************   UpdateUndo()   ****************************/

static void DoRangeUndo(struct RangeWindow *rw)
{
	static struct Range tmprange;

	CopyRange(rw->rw_CurrentRange, &tmprange);
	CopyRange(&rw->rw_UndoRange, rw->rw_CurrentRange);
	CopyRange(&tmprange, &rw->rw_UndoRange);
	RethinkRangeBox(rw,rw->rw_RangeBoxLeft,rw->rw_RangeBoxTop,rw->rw_RangeBoxWidth,rw->rw_RangeBoxHeight);
}
