/*
	Handle all the Stuff to do with the Tools Window.
  A Few Useful Bits Of Terminology
		Tools Window Gadgets = All the gadgets in the Tools Window
		Tools Gadgets = Just the Tools Gadgets themselves, like Line, Curve...
										Not stuff like Brushes, Undo...
 */

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


#include <exec/exec.h>
//#include <intuition/intuition.h>
//#include <libraries/gadtools.h>
//#include <graphics/gfx.h>
//#include <dos/dostags.h>

#include <stdio.h>

#include <proto/exec.h>
#include <proto/intuition.h>
#include <proto/gadtools.h>
//#include <proto/graphics.h>
//#include <proto/dos.h>

#include <global.h>
#include <ExternVars.h>

static struct NewMenu	toolsmenu[] =
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
		{  NM_ITEM,	"Vertical?",						0,	CHECKIT|MENUTOGGLE,	0,	0,},

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

#define TG_SIZEOFGADMAP			2*12*8

/* Some nasty external referencing to files incbin'd in MiscAsm.a */
extern long __far BR_CustomIn0, BR_CustomIn1, BR_CustomOut0, BR_CustomOut1;
extern long __far BR_AnimBrushIn0, BR_AnimBrushIn1, BR_AnimBrushOut0, BR_AnimBrushOut1;
extern long __far BR_Brush0In0, BR_Brush0In1, BR_Brush0Out0, BR_Brush0Out1;
extern long __far BR_Brush1In0, BR_Brush1In1, BR_Brush1Out0, BR_Brush1Out1;
extern long __far BR_Brush2In0, BR_Brush2In1, BR_Brush2Out0, BR_Brush2Out1;
extern long __far BR_Brush3In0, BR_Brush3In1, BR_Brush3Out0, BR_Brush3Out1;
extern long __far BR_Brush4In0, BR_Brush4In1, BR_Brush4Out0, BR_Brush4Out1;
extern long __far BR_Brush5In0, BR_Brush5In1, BR_Brush5Out0, BR_Brush5Out1;
extern long __far BR_Brush6In0, BR_Brush6In1, BR_Brush6Out0, BR_Brush6Out1;
extern long __far BR_Brush7In0, BR_Brush7In1, BR_Brush7Out0, BR_Brush7Out1;
extern long __far BR_Brush8In0, BR_Brush8In1, BR_Brush8Out0, BR_Brush8Out1;
extern long __far BrushHandleIn0, BrushHandleIn1, BrushHandleOut0, BrushHandleOut1;
extern long __far ClrIn0, ClrIn1, ClrOut0, ClrOut1;
extern long __far CurveIn0, CurveIn1, CurveOut0, CurveOut1;
extern long __far CutBrushIn0, CutBrushIn1, CutBrushOut0, CutBrushOut1;
extern long __far PickUpAnimBrushIn0, PickUpAnimBrushIn1, PickUpAnimBrushOut0, PickUpAnimBrushOut1;
extern long __far FillIn0, FillIn1, FillOut0, FillOut1;
extern long __far GridLockIn0, GridLockIn1, GridLockOut0, GridLockOut1;
extern long __far LineIn0, LineIn1, LineOut0, LineOut1;
extern long __far MagnifyIn0, MagnifyIn1, MagnifyOut0, MagnifyOut1;
extern long __far OBoxIn0, OBoxIn1, OBoxOut0, OBoxOut1;
extern long __far OCircleIn0, OCircleIn1, OCircleOut0, OCircleOut1;
extern long __far OEllipseIn0, OEllipseIn1, OEllipseOut0, OEllipseOut1;
extern long __far PlotIn0, PlotIn1, PlotOut0, PlotOut1;
extern long __far PointIn0, PointIn1, PointOut0, PointOut1;
extern long __far SBoxIn0, SBoxIn1, SBoxOut0, SBoxOut1;
extern long __far SCircleIn0, SCircleIn1, SCircleOut0, SCircleOut1;
extern long __far SEllipseIn0, SEllipseIn1, SEllipseOut0, SEllipseOut1;
extern long __far SprayIn0, SprayIn1, SprayOut0, SprayOut1;
extern long __far TextIn0, TextIn1, TextOut0, TextOut1;
extern long __far UndoIn0, UndoIn1, UndoOut0, UndoOut1;
extern long __far PickColourIn0, PickColourIn1, PickColourOut0, PickColourOut1;
extern long __far HitZoneIn0, HitZoneIn1, HitZoneOut0, HitZoneOut1;


/* prototypes local to this source file: */
BOOL ShowToolsWindow(struct SuperWindow *mw);
BOOL SetupToolsWindowMenus(struct SuperWindow *sw);
void HideToolsWindow(struct SuperWindow *sw);
APTR KillOffToolsWindow(struct SuperWindow *sw);
void ToolsWindowHandler(struct SuperWindow *sw, struct IntuiMessage *imsg);
static void DoGadgets(struct ToolsWindow *tw);
static void SortOutToolsWindowMenus(struct SuperWindow *sw,
														 struct IntuiMessage *imsg,
														 UWORD code);
static void SetToolsGadget(struct ToolsWindow *tw,struct Gadget *gad);
static void SetBrushGadget(struct ToolsWindow *tw,struct Gadget *gad);
static void ToolsWindowGadgetHandler(struct Gadget *gad);
static void HandleHitSelectGadget(struct ToolsWindow *tw,struct Gadget *gad);
static void SetToggleGadget(struct ToolsWindow *tw,struct Gadget *gad, UWORD state);
static void SetGadgetSelection(struct ToolsWindow *tw);


/**************** OpenNewToolsWindow() ****************/
//
// Creates a new Tools Window.
//

BOOL OpenNewToolsWindow(struct WindowDefault *wf)
{
	struct ToolsWindow *tw;

	if(!wf)	wf = &glob.cg_WindowDefaults[SWTY_TOOLS];

	/* grab mem for the ToolsWindow struct */
	if (!(tw = AllocVec(sizeof(struct ToolsWindow), MEMF_CLEAR ) ) )
		return(FALSE);
	if(!(tw->tw_GadMaps = AllocVec((TG_SIZEOFGADMAP*TG_NUMOFGADGETS*2),MEMF_CHIP|MEMF_CLEAR) ) )
	{
		FreeVec(tw);
		return(FALSE);
	}

	/* figure out a unique ID for this window (mainly for AREXX use) */
	InstallUniqueWindowID((struct SuperWindow *)tw);
	tw->tw_sw.sw_Node.ln_Name = (char *)&(tw->tw_sw.sw_idstring);

	/* find the Default settings for this type of SuperWindow */
	//wf = &glob.cg_WindowDefaults[SWTY_TOOLS];

	tw->tw_sw.sw_Handler = ToolsWindowHandler;
	tw->tw_sw.sw_HideWindow = HideToolsWindow;
	tw->tw_sw.sw_ShowWindow = ShowToolsWindow;
	tw->tw_sw.sw_BuggerOff = KillOffToolsWindow;
	tw->tw_sw.sw_RefreshMenus = SetupToolsWindowMenus;
	tw->tw_sw.sw_Window = NULL;
	tw->tw_sw.sw_MenuStrip = NULL;
	tw->tw_sw.sw_Type = SWTY_TOOLS;
	tw->tw_sw.sw_VisualInfo = NULL;
	tw->tw_sw.sw_GadList = NULL;

	/* set up stuff for opening the actual intuition window */
	tw->tw_sw.sw_Left = wf->wf_Left;
	tw->tw_sw.sw_Top = wf->wf_Top;
	tw->tw_sw.sw_Width = wf->wf_Width;
	tw->tw_sw.sw_Height = wf->wf_Height;
	tw->tw_sw.sw_MinWidth = 50;
	tw->tw_sw.sw_MinHeight = 50;
	tw->tw_sw.sw_MaxWidth = 0xFFFF;
	tw->tw_sw.sw_MaxHeight = 0xFFFF;
	tw->tw_sw.sw_MenuStrip = NULL;

	tw->tw_sw.sw_FlgBackDrop = wf->wf_FlgBackDrop;
	tw->tw_sw.sw_FlgBorderless = wf->wf_FlgBorderless;
	tw->tw_sw.sw_FlgTitle = wf->wf_FlgTitle;
	tw->tw_sw.sw_FlgBorderGadgets = wf->wf_FlgBorderGadgets;

	tw->tw_sw.sw_Title = "Tools";
	tw->tw_Vertical = (wf->wf_Flags & WFF_TOOLSWINDOWVERTICAL) ? TRUE : FALSE;

	/* add ToolsWindow into the superwindows list */
	AddTail(&superwindows, &tw->tw_sw.sw_Node);

	/* open the intuition window (if the window doesn't open, */
	/* it is just left hidden - SEP). */

	ShowToolsWindow(&tw->tw_sw);

	return(TRUE);
}



BOOL ShowToolsWindow(struct SuperWindow *sw)
{
	struct ToolsWindow *tw;
	ULONG flags;
	UWORD w,h;

	tw = (struct ToolsWindow *)sw;

	if(!sw->sw_Window)				/* make sure it isn't already open */
	{
		/* Set up the Visual Info for GadTools */
		if (!(sw->sw_VisualInfo = GetVisualInfo(mainscreen,TAG_END)))
			return(FALSE);

		flags = WFLG_ACTIVATE | WFLG_NEWLOOKMENUS;
		if(sw->sw_FlgTitle)
			flags = flags | WFLG_DRAGBAR | WFLG_DEPTHGADGET; // | WFLG_CLOSEGADGET;
			/* No close gadget 'cos the window gets too small for it. */
		//if(sw->sw_FlgBorderGadgets)
		//	flags = flags | WFLG_SIZEGADGET | WFLG_SIZEBBOTTOM;

		if(tw->tw_Vertical)
		{
			w=(3*12) + 4;
			h=(11*12) + 4;
		}
		else
		{
			w=(11*12) + 4;
			h=(3*12) + 4;
		}

		sw->sw_Window = OpenWindowTags(NULL,
											WA_Left,				sw->sw_Left,
											WA_Top,					sw->sw_Top,
											//WA_Width,				sw->sw_Width,
											//WA_Height,			sw->sw_Height,
											WA_InnerWidth,	w,
											WA_InnerHeight,	h,
											WA_MinWidth,		sw->sw_MinWidth,
											WA_MinHeight,		sw->sw_MinHeight,
											WA_MaxWidth,		sw->sw_MaxWidth,
											WA_MaxHeight,		sw->sw_MaxHeight,
											WA_Flags,				flags,
											WA_ReportMouse,	TRUE,
											WA_Backdrop,		sw->sw_FlgBackDrop,
											WA_Borderless,	sw->sw_FlgBorderless,
											WA_CustomScreen, MAINSCREEN,
											WA_AutoAdjust,	TRUE,
											WA_Title,				sw->sw_FlgTitle ?
												BuildWindowTitle(sw) : NULL,
											TAG_DONE,				NULL);

		if( !(sw->sw_Window)) return(FALSE);

		/* Gadget sussing routine */
		DoGadgets(tw);

		/* Tell the window which superwindow it is attached to */
		/* (so we'll know which handler routine to call) */
		sw->sw_Window->UserData = (BYTE *)tw;

		SetupToolsWindowMenus(sw);

		/* we want to use our global message port for IDCMP stuff */
		sw->sw_Window->UserPort = mainmsgport;
		ModifyIDCMP(sw->sw_Window,	IDCMP_CLOSEWINDOW | IDCMP_MENUPICK | IDCMP_CHANGEWINDOW |
																IDCMP_GADGETDOWN | IDCMP_GADGETUP | IDCMP_MOUSEMOVE);

		SetSuperWindowScreenTitle(sw);
	}
	return(TRUE);
}


BOOL SetupToolsWindowMenus(struct SuperWindow *sw)
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
	SetupWindowMenu( sw, &toolsmenu[MN_WINDOW_IDX] );
	SetupEditMenu( NULL, &toolsmenu[MN_EDIT_IDX] );
	SetupSettingsMenu( &toolsmenu[MN_SETTINGS_IDX] );
	SetupModeMenu( sw, &toolsmenu[MN_MODE_IDX] );
	LinkInUserMenu( &toolsmenu[MN_USER_IDX] );

	if( sw->sw_MenuStrip = CreateMenus( toolsmenu, TAG_END ) )
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


void HideToolsWindow(struct SuperWindow *sw)
{
	struct ToolsWindow *tw;

	tw = (struct ToolsWindow *)sw;

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

		if(tw->tw_sw.sw_GadList)
		{
			RemoveGList(sw->sw_Window, tw->tw_GadList, -1);
			tw->tw_sw.sw_GadList = NULL;
		}

		CloseWindowSafely(sw->sw_Window);				/* close it */
		sw->sw_Window = NULL;										/* indicate that it's gone */
		if (sw->sw_VisualInfo) FreeVisualInfo(sw->sw_VisualInfo);
		sw->sw_VisualInfo = NULL;
		SetProcessWindowPtr();
	}
}



APTR KillOffToolsWindow(struct SuperWindow *sw)
{
	struct ToolsWindow *tw;

	tw = (struct ToolsWindow *)sw;

	/* close/free the superwindow */
	HideToolsWindow(sw);											/* shut down the Intuition window */
	Remove(&(sw->sw_Node));										/* remove from superwindows list */
	FreeVec(tw->tw_GadMaps);									/* Free the Chip memory for the Gadget Maps */
	FreeVec(tw);															/* free ToolsWindow struct */
	return(NULL);
}


static void DoGadgets(struct ToolsWindow *tw)
{
	struct Window *win;
	struct ToolsWindowGadList	*tgl;
	struct ToolsWindowGadget *tg;
	struct DrawInfo *dri;
	//UWORD	coords[
	UWORD x,y;

	win = tw->tw_sw.sw_Window;

	if(tw->tw_sw.sw_GadList)
	{
		RemoveGList(win, tw->tw_GadList, -1);
		tw->tw_sw.sw_GadList = NULL;
	}

	ClearWindow( win );

	tgl = &tw->tw_ToolsWindowGadList;
	tg = tw->tw_ToolsWindowGadget;

	NewList(&tgl->tgl_List);
	tgl->tgl_GadImages = tw->tw_GadImages;
	tgl->tgl_Gadgets = tw->tw_GadList;
	tgl->tgl_GadMaps = tw->tw_GadMaps;
	tgl->tgl_SelectWidth = 12;
	tgl->tgl_Width = 1;
	tgl->tgl_Height = 12;
	tgl->tgl_Depth = 8;

	dri = GetScreenDrawInfo( mainscreen );
	tgl->tgl_BrightColour = dri->dri_Pens[SHINEPEN];
	tgl->tgl_DarkColour = dri->dri_Pens[SHADOWPEN];
	FreeScreenDrawInfo( mainscreen, dri );

	if(tw->tw_Vertical)
	{
		x = win->BorderLeft+2+24;
		y = win->BorderTop+2;
	}
	else
	{
		x = win->BorderLeft+2;
		y = win->Height-win->BorderBottom-1-36-2;
	}

	/* Brush Gadgets */

	AddTail(&tgl->tgl_List,&tg->tg_Node);					/* TLB_DOT */
	tg->tg_LeftEdge = x;
	tg->tg_TopEdge = y;
	tg->tg_Activation = GACT_IMMEDIATE;
	tg->tg_GadgetID = TLB_DOT;
	tg->tg_BrightMap1 = (UBYTE *)&BR_Brush0Out1;
	tg->tg_DarkMap1 = (UBYTE *)&BR_Brush0Out0;
	tg->tg_BrightMap2 = (UBYTE *)&BR_Brush0In1;
	tg->tg_DarkMap2 = (UBYTE *)&BR_Brush0In0;
	tg++;
	if(tw->tw_Vertical) y += 12;
	else x += 12;

	AddTail(&tgl->tgl_List,&tg->tg_Node);					/* TLB_BRUSH1 */
	tg->tg_LeftEdge = x;
	tg->tg_TopEdge = y;
	tg->tg_Activation = GACT_IMMEDIATE;
	tg->tg_GadgetID = TLB_BRUSH1;
	tg->tg_BrightMap1 = (UBYTE *)&BR_Brush1Out1;
	tg->tg_DarkMap1 = (UBYTE *)&BR_Brush1Out0;
	tg->tg_BrightMap2 = (UBYTE *)&BR_Brush1In1;
	tg->tg_DarkMap2 = (UBYTE *)&BR_Brush1In0;
	tg++;
	if(tw->tw_Vertical) y += 12;
	else x += 12;

	AddTail(&tgl->tgl_List,&tg->tg_Node);					/* TLB_BRUSH2 */
	tg->tg_LeftEdge = x;
	tg->tg_TopEdge = y;
	tg->tg_Activation = GACT_IMMEDIATE;
	tg->tg_GadgetID = TLB_BRUSH2;
	tg->tg_BrightMap1 = (UBYTE *)&BR_Brush2Out1;
	tg->tg_DarkMap1 = (UBYTE *)&BR_Brush2Out0;
	tg->tg_BrightMap2 = (UBYTE *)&BR_Brush2In1;
	tg->tg_DarkMap2 = (UBYTE *)&BR_Brush2In0;
	tg++;
	if(tw->tw_Vertical) y += 12;
	else x += 12;

	AddTail(&tgl->tgl_List,&tg->tg_Node);					/* TLB_BRUSH3 */
	tg->tg_LeftEdge = x;
	tg->tg_TopEdge = y;
	tg->tg_Activation = GACT_IMMEDIATE;
	tg->tg_GadgetID = TLB_BRUSH3;
	tg->tg_BrightMap1 = (UBYTE *)&BR_Brush3Out1;
	tg->tg_DarkMap1 = (UBYTE *)&BR_Brush3Out0;
	tg->tg_BrightMap2 = (UBYTE *)&BR_Brush3In1;
	tg->tg_DarkMap2 = (UBYTE *)&BR_Brush3In0;
	tg++;
	if(tw->tw_Vertical) y += 12;
	else x += 12;

	AddTail(&tgl->tgl_List,&tg->tg_Node);					/* TLB_BRUSH4 */
	tg->tg_LeftEdge = x;
	tg->tg_TopEdge = y;
	tg->tg_Activation = GACT_IMMEDIATE;
	tg->tg_GadgetID = TLB_BRUSH4;
	tg->tg_BrightMap1 = (UBYTE *)&BR_Brush4Out1;
	tg->tg_DarkMap1 = (UBYTE *)&BR_Brush4Out0;
	tg->tg_BrightMap2 = (UBYTE *)&BR_Brush4In1;
	tg->tg_DarkMap2 = (UBYTE *)&BR_Brush4In0;
	tg++;
	if(tw->tw_Vertical) y += 12;
	else x += 12;

	AddTail(&tgl->tgl_List,&tg->tg_Node);					/* TLB_BRUSH5 */
	tg->tg_LeftEdge = x;
	tg->tg_TopEdge = y;
	tg->tg_Activation = GACT_IMMEDIATE;
	tg->tg_GadgetID = TLB_BRUSH5;
	tg->tg_BrightMap1 = (UBYTE *)&BR_Brush5Out1;
	tg->tg_DarkMap1 = (UBYTE *)&BR_Brush5Out0;
	tg->tg_BrightMap2 = (UBYTE *)&BR_Brush5In1;
	tg->tg_DarkMap2 = (UBYTE *)&BR_Brush5In0;
	tg++;
	if(tw->tw_Vertical) y += 12;
	else x += 12;

	AddTail(&tgl->tgl_List,&tg->tg_Node);					/* TLB_BRUSH6 */
	tg->tg_LeftEdge = x;
	tg->tg_TopEdge = y;
	tg->tg_Activation = GACT_IMMEDIATE;
	tg->tg_GadgetID = TLB_BRUSH6;
	tg->tg_BrightMap1 = (UBYTE *)&BR_Brush6Out1;
	tg->tg_DarkMap1 = (UBYTE *)&BR_Brush6Out0;
	tg->tg_BrightMap2 = (UBYTE *)&BR_Brush6In1;
	tg->tg_DarkMap2 = (UBYTE *)&BR_Brush6In0;
	tg++;
	if(tw->tw_Vertical) y += 12;
	else x += 12;

	AddTail(&tgl->tgl_List,&tg->tg_Node);					/* TLB_BRUSH7 */
	tg->tg_LeftEdge = x;
	tg->tg_TopEdge = y;
	tg->tg_Activation = GACT_IMMEDIATE;
	tg->tg_GadgetID = TLB_BRUSH7;
	tg->tg_BrightMap1 = (UBYTE *)&BR_Brush7Out1;
	tg->tg_DarkMap1 = (UBYTE *)&BR_Brush7Out0;
	tg->tg_BrightMap2 = (UBYTE *)&BR_Brush7In1;
	tg->tg_DarkMap2 = (UBYTE *)&BR_Brush7In0;
	tg++;
	if(tw->tw_Vertical) y += 12;
	else x += 12;

	AddTail(&tgl->tgl_List,&tg->tg_Node);					/* TLB_BRUSH8 */
	tg->tg_LeftEdge = x;
	tg->tg_TopEdge = y;
	tg->tg_Activation = GACT_IMMEDIATE;
	tg->tg_GadgetID = TLB_BRUSH8;
	tg->tg_BrightMap1 = (UBYTE *)&BR_Brush8Out1;
	tg->tg_DarkMap1 = (UBYTE *)&BR_Brush8Out0;
	tg->tg_BrightMap2 = (UBYTE *)&BR_Brush8In1;
	tg->tg_DarkMap2 = (UBYTE *)&BR_Brush8In0;
	tg++;
	if(tw->tw_Vertical) y += 12;
	else x += 12;

	/* Tools Gadgets */
	if(tw->tw_Vertical)
	{
		x = win->BorderLeft+2;
		y = win->BorderTop+2;
	}
	else
	{
		x = win->BorderLeft+2;
		y = win->Height-win->BorderBottom-1-12-2;
	}

	AddTail(&tgl->tgl_List,&tg->tg_Node);					/* TLB_CUSTOM */
	tg->tg_LeftEdge = x;
	tg->tg_TopEdge = y;
	tg->tg_Activation = GACT_IMMEDIATE;
	tg->tg_GadgetID = TLB_CUSTOM;
	tg->tg_BrightMap1 = (UBYTE *)&BR_CustomOut0;
	tg->tg_DarkMap1 = (UBYTE *)&BR_CustomOut1;
	tg->tg_BrightMap2 = (UBYTE *)&BR_CustomIn0;
	tg->tg_DarkMap2 = (UBYTE *)&BR_CustomIn1;
	tg++;
	if(tw->tw_Vertical) { x += 12; }
	else { y -= 12; }
	AddTail(&tgl->tgl_List,&tg->tg_Node);					/* TLB_ANIMBRUSH */
	tg->tg_LeftEdge = x;
	tg->tg_TopEdge = y;
	tg->tg_Activation = GACT_IMMEDIATE;
	tg->tg_GadgetID = TLB_ANIMBRUSH;
	tg->tg_BrightMap1 = (UBYTE *)&BR_AnimBrushOut1;
	tg->tg_DarkMap1 = (UBYTE *)&BR_AnimBrushOut0;
	tg->tg_BrightMap2 = (UBYTE *)&BR_AnimBrushIn1;
	tg->tg_DarkMap2 = (UBYTE *)&BR_AnimBrushIn0;
	tg++;

	/* Tools Gadgets */
	if(tw->tw_Vertical)
	{
		x = win->BorderLeft+2;
		y = win->BorderTop+2+12;
	}
	else
	{
		x = win->BorderLeft+2+12;
		y = win->Height-win->BorderBottom-1-12-2;
	}

	AddTail(&tgl->tgl_List,&tg->tg_Node);					/* TL_POINT */
	tg->tg_LeftEdge = x;
	tg->tg_TopEdge = y;
	tg->tg_Activation = GACT_IMMEDIATE;
	tg->tg_GadgetID = TL_POINT;
	tg->tg_BrightMap1 = (UBYTE *)&PointOut0;
	tg->tg_DarkMap1 = (UBYTE *)&PointOut1;
	tg->tg_BrightMap2 = (UBYTE *)&PointIn0;
	tg->tg_DarkMap2 = (UBYTE *)&PointIn1;
	tg++;
	if(tw->tw_Vertical) { x += 12; }
	else { y -= 12; }
	AddTail(&tgl->tgl_List,&tg->tg_Node);					/* TL_PLOT */
	tg->tg_LeftEdge = x;
	tg->tg_TopEdge = y;
	tg->tg_Activation = GACT_IMMEDIATE;
	tg->tg_GadgetID = TL_PLOT;
	tg->tg_BrightMap1 = (UBYTE *)&PlotOut0;
	tg->tg_DarkMap1 = (UBYTE *)&PlotOut1;
	tg->tg_BrightMap2 = (UBYTE *)&PlotIn0;
	tg->tg_DarkMap2 = (UBYTE *)&PlotIn1;
	tg++;
	if(tw->tw_Vertical) { y += 12; x -= 12; }
	else { x += 12; y += 12; }
	AddTail(&tgl->tgl_List,&tg->tg_Node);					/* TL_LINE */
	tg->tg_LeftEdge = x;
	tg->tg_TopEdge = y;
	tg->tg_Activation = GACT_IMMEDIATE;
	tg->tg_GadgetID = TL_LINE;
	tg->tg_BrightMap1 = (UBYTE *)&LineOut0;
	tg->tg_DarkMap1 = (UBYTE *)&LineOut1;
	tg->tg_BrightMap2 = (UBYTE *)&LineIn0;
	tg->tg_DarkMap2 = (UBYTE *)&LineIn1;
	tg++;
	if(tw->tw_Vertical) { x += 12; }
	else { y -= 12; }
	AddTail(&tgl->tgl_List,&tg->tg_Node);					/* TL_CURVE */
	tg->tg_LeftEdge = x;
	tg->tg_TopEdge = y;
	tg->tg_Activation = GACT_IMMEDIATE;
	tg->tg_GadgetID = TL_CURVE;
	tg->tg_BrightMap1 = (UBYTE *)&CurveOut0;
	tg->tg_DarkMap1 = (UBYTE *)&CurveOut1;
	tg->tg_BrightMap2 = (UBYTE *)&CurveIn0;
	tg->tg_DarkMap2 = (UBYTE *)&CurveIn1;
	tg++;
	if(tw->tw_Vertical) { y += 12; x -= 12; }
	else { x += 12; y += 12; }
	AddTail(&tgl->tgl_List,&tg->tg_Node);					/* TL_OBOX */
	tg->tg_LeftEdge = x;
	tg->tg_TopEdge = y;
	tg->tg_Activation = GACT_IMMEDIATE;
	tg->tg_GadgetID = TL_OBOX;
	tg->tg_BrightMap1 = (UBYTE *)&OBoxOut0;
	tg->tg_DarkMap1 = (UBYTE *)&OBoxOut1;
	tg->tg_BrightMap2 = (UBYTE *)&OBoxIn0;
	tg->tg_DarkMap2 = (UBYTE *)&OBoxIn1;
	tg++;
	if(tw->tw_Vertical) { x += 12; }
	else { y -= 12; }
	AddTail(&tgl->tgl_List,&tg->tg_Node);					/* TL_SBOX */
	tg->tg_LeftEdge = x;
	tg->tg_TopEdge = y;
	tg->tg_Activation = GACT_IMMEDIATE;
	tg->tg_GadgetID = TL_SBOX;
	tg->tg_BrightMap1 = (UBYTE *)&SBoxOut0;
	tg->tg_DarkMap1 = (UBYTE *)&SBoxOut1;
	tg->tg_BrightMap2 = (UBYTE *)&SBoxIn0;
	tg->tg_DarkMap2 = (UBYTE *)&SBoxIn1;
	tg++;
	if(tw->tw_Vertical) { y += 12; x -= 12; }
	else { x += 12; y += 12; }
	AddTail(&tgl->tgl_List,&tg->tg_Node);					/* TL_OCIRCLE */
	tg->tg_LeftEdge = x;
	tg->tg_TopEdge = y;
	tg->tg_Activation = GACT_IMMEDIATE;
	tg->tg_GadgetID = TL_OCIRCLE;
	tg->tg_BrightMap1 = (UBYTE *)&OCircleOut0;
	tg->tg_DarkMap1 = (UBYTE *)&OCircleOut1;
	tg->tg_BrightMap2 = (UBYTE *)&OCircleIn0;
	tg->tg_DarkMap2 = (UBYTE *)&OCircleIn1;
	tg++;
	if(tw->tw_Vertical) { x += 12; }
	else { y -= 12; }
	AddTail(&tgl->tgl_List,&tg->tg_Node);					/* TL_SCIRCLE */
	tg->tg_LeftEdge = x;
	tg->tg_TopEdge = y;
	tg->tg_Activation = GACT_IMMEDIATE;
	tg->tg_GadgetID = TL_SCIRCLE;
	tg->tg_BrightMap1 = (UBYTE *)&SCircleOut0;
	tg->tg_DarkMap1 = (UBYTE *)&SCircleOut1;
	tg->tg_BrightMap2 = (UBYTE *)&SCircleIn0;
	tg->tg_DarkMap2 = (UBYTE *)&SCircleIn1;
	tg++;
	if(tw->tw_Vertical) { y += 12; x -= 12; }
	else { x += 12; y += 12; }
	AddTail(&tgl->tgl_List,&tg->tg_Node);					/* TL_OELLIPSE */
	tg->tg_LeftEdge = x;
	tg->tg_TopEdge = y;
	tg->tg_Activation = GACT_IMMEDIATE;
	tg->tg_GadgetID = TL_OELLIPSE;
	tg->tg_BrightMap1 = (UBYTE *)&OEllipseOut1;
	tg->tg_DarkMap1 = (UBYTE *)&OEllipseOut0;
	tg->tg_BrightMap2 = (UBYTE *)&OEllipseIn1;
	tg->tg_DarkMap2 = (UBYTE *)&OEllipseIn0;
	tg++;
	if(tw->tw_Vertical) { x += 12; }
	else { y -= 12; }
	AddTail(&tgl->tgl_List,&tg->tg_Node);					/* TL_SELLIPSE */
	tg->tg_LeftEdge = x;
	tg->tg_TopEdge = y;
	tg->tg_Activation = GACT_IMMEDIATE;
	tg->tg_GadgetID = TL_SELLIPSE;
	tg->tg_BrightMap1 = (UBYTE *)&SEllipseOut1;
	tg->tg_DarkMap1 = (UBYTE *)&SEllipseOut0;
	tg->tg_BrightMap2 = (UBYTE *)&SEllipseIn1;
	tg->tg_DarkMap2 = (UBYTE *)&SEllipseIn0;
	tg++;
	if(tw->tw_Vertical) { y += 12; x -= 12; }
	else { x += 12; y += 12; }
	AddTail(&tgl->tgl_List,&tg->tg_Node);					/* TL_GETBRUSH */
	tg->tg_LeftEdge = x;
	tg->tg_TopEdge = y;
	tg->tg_Activation = GACT_IMMEDIATE;
	tg->tg_GadgetID = TL_GETBRUSH;
	tg->tg_BrightMap1 = (UBYTE *)&CutBrushOut0;
	tg->tg_DarkMap1 = (UBYTE *)&CutBrushOut1;
	tg->tg_BrightMap2 = (UBYTE *)&CutBrushIn0;
	tg->tg_DarkMap2 = (UBYTE *)&CutBrushIn1;
	tg++;
	if(tw->tw_Vertical) { x += 12; }
	else { y -= 12; }
	AddTail(&tgl->tgl_List,&tg->tg_Node);					/* TL_GETANIMBRUSH */
	tg->tg_LeftEdge = x;
	tg->tg_TopEdge = y;
	tg->tg_Activation = GACT_IMMEDIATE;
	tg->tg_GadgetID = TL_GETANIMBRUSH;
	tg->tg_BrightMap1 = (UBYTE *)&PickUpAnimBrushOut1;
	tg->tg_DarkMap1 = (UBYTE *)&PickUpAnimBrushOut0;
	tg->tg_BrightMap2 = (UBYTE *)&PickUpAnimBrushIn1;
	tg->tg_DarkMap2 = (UBYTE *)&PickUpAnimBrushIn0;
	tg++;
	if(tw->tw_Vertical) { y += 12; x -= 12; }
	else { x += 12; y += 12; }
	AddTail(&tgl->tgl_List,&tg->tg_Node);					/* TL_FILL */
	tg->tg_LeftEdge = x;
	tg->tg_TopEdge = y;
	tg->tg_Activation = GACT_IMMEDIATE;
	tg->tg_GadgetID = TL_FILL;
	tg->tg_BrightMap1 = (UBYTE *)&FillOut0;
	tg->tg_DarkMap1 = (UBYTE *)&FillOut1;
	tg->tg_BrightMap2 = (UBYTE *)&FillIn0;
	tg->tg_DarkMap2 = (UBYTE *)&FillIn1;
	tg++;
	if(tw->tw_Vertical) { x += 12; }
	else { y -= 12; }
	AddTail(&tgl->tgl_List,&tg->tg_Node);					/* TL_BRUSHHANDLE */
	tg->tg_LeftEdge = x;
	tg->tg_TopEdge = y;
	tg->tg_Activation = GACT_IMMEDIATE;
	tg->tg_GadgetID = TL_BRUSHHANDLE;
	tg->tg_BrightMap1 = (UBYTE *)&BrushHandleOut0;
	tg->tg_DarkMap1 = (UBYTE *)&BrushHandleOut1;
	tg->tg_BrightMap2 = (UBYTE *)&BrushHandleIn0;
	tg->tg_DarkMap2 = (UBYTE *)&BrushHandleIn1;
	tg++;
	if(tw->tw_Vertical) { y+= 12; x -= 12; }
	else { x+= 12; y += 12; }
	AddTail(&tgl->tgl_List,&tg->tg_Node);					/* TL_MAGNIFY */
	tg->tg_LeftEdge = x;
	tg->tg_TopEdge = y;
	tg->tg_Activation = GACT_IMMEDIATE;
	tg->tg_GadgetID = TL_MAGNIFY;
	tg->tg_BrightMap1 = (UBYTE *)&MagnifyOut0;
	tg->tg_DarkMap1 = (UBYTE *)&MagnifyOut1;
	tg->tg_BrightMap2 = (UBYTE *)&MagnifyIn0;
	tg->tg_DarkMap2 = (UBYTE *)&MagnifyIn1;
	tg++;
	if(tw->tw_Vertical) { x += 12; }
	else { y -= 12; }
	AddTail(&tgl->tgl_List,&tg->tg_Node);					/* TL_TYPE */
	tg->tg_LeftEdge = x;
	tg->tg_TopEdge = y;
	tg->tg_Activation = GACT_IMMEDIATE;
	tg->tg_GadgetID = TL_TYPE;
	tg->tg_BrightMap1 = (UBYTE *)&TextOut0;
	tg->tg_DarkMap1 = (UBYTE *)&TextOut1;
	tg->tg_BrightMap2 = (UBYTE *)&TextIn0;
	tg->tg_DarkMap2 = (UBYTE *)&TextIn1;
	tg++;
	if(tw->tw_Vertical) { y += 12; x += 12;}
	else { x += 12; y -= 12; }
	AddTail(&tgl->tgl_List,&tg->tg_Node);					/* TL_HitZone */
	tg->tg_LeftEdge = x;
	tg->tg_TopEdge = y;
	tg->tg_Activation = GACT_IMMEDIATE;
	tg->tg_GadgetID = TL_HITZONE;
	tg->tg_BrightMap1 = (UBYTE *)&HitZoneOut1;
	tg->tg_DarkMap1 = (UBYTE *)&HitZoneOut0;
	tg->tg_BrightMap2 = (UBYTE *)&HitZoneIn1;
	tg->tg_DarkMap2 = (UBYTE *)&HitZoneIn0;
	tg++;
	if(tw->tw_Vertical) { x -= 12; }
	else { y += 12; }
	AddTail(&tgl->tgl_List,&tg->tg_Node);					/* TL_PickColour */
	tg->tg_LeftEdge = x;
	tg->tg_TopEdge = y;
	tg->tg_Activation = GACT_IMMEDIATE;
	tg->tg_GadgetID = TL_PICKCOLOUR;
	tg->tg_BrightMap1 = (UBYTE *)&PickColourOut1;
	tg->tg_DarkMap1 = (UBYTE *)&PickColourOut0;
	tg->tg_BrightMap2 = (UBYTE *)&PickColourIn1;
	tg->tg_DarkMap2 = (UBYTE *)&PickColourIn0;
	tg++;

	if(tw->tw_Vertical) { x -= 12; }
	else { y += 12; }
	AddTail(&tgl->tgl_List,&tg->tg_Node);					/* TL_GRIDLOCK */
	tg->tg_LeftEdge = x;
	tg->tg_TopEdge = y;
	tg->tg_Activation = GACT_IMMEDIATE;
	tg->tg_GadgetID = TL_GRIDLOCK;
	tg->tg_BrightMap1 = (UBYTE *)&GridLockOut0;
	tg->tg_DarkMap1 = (UBYTE *)&GridLockOut1;
	tg->tg_BrightMap2 = (UBYTE *)&GridLockIn0;
	tg->tg_DarkMap2 = (UBYTE *)&GridLockIn1;
	tg++;

	/* Hit Select Gadgets */

	if(tw->tw_Vertical) { y += 12; }
	else { x += 12; }
	AddTail(&tgl->tgl_List,&tg->tg_Node);					/* TL_UNDO */
	tg->tg_LeftEdge = x;
	tg->tg_TopEdge = y;
	tg->tg_Activation = GACT_RELVERIFY;
	tg->tg_GadgetID = TL_UNDO;
	tg->tg_BrightMap1 = (UBYTE *)&UndoOut0;
	tg->tg_DarkMap1 = (UBYTE *)&UndoOut1;
	tg->tg_BrightMap2 = (UBYTE *)&UndoIn0;
	tg->tg_DarkMap2 = (UBYTE *)&UndoIn1;
	tg++;
	if(tw->tw_Vertical) { x += 12; }
	else { y -= 12; }
	AddTail(&tgl->tgl_List,&tg->tg_Node);					/* TL_CLR */
	tg->tg_LeftEdge = x;
	tg->tg_TopEdge = y;
	tg->tg_Activation = GACT_RELVERIFY;
	tg->tg_GadgetID = TL_CLR;
	tg->tg_BrightMap1 = (UBYTE *)&ClrOut0;
	tg->tg_DarkMap1 = (UBYTE *)&ClrOut1;
	tg->tg_BrightMap2 = (UBYTE *)&ClrIn0;
	tg->tg_DarkMap2 = (UBYTE *)&ClrIn1;
	//tg++;

	/* Call up my little Asm routine to sort out all the stuff, like remapping
			and making up Gadget structures, and crap like that. */
	MakeToolsGadgets(tgl);

	/* Set the selection bit of the appropiate gadgets */
	SetGadgetSelection(tw);

	AddGList(win, tw->tw_GadList, -1, TG_NUMOFGADGETS, NULL);
	RefreshGList(tw->tw_GadList, win, NULL, TG_NUMOFGADGETS);
	tw->tw_sw.sw_GadList = tw->tw_GadList;
}

static void SetGadgetSelection(struct ToolsWindow *tw)
{
	struct Gadget *gad;

	/* Do Tools Gadgets */
	gad = GetGadgetFromID(tw->tw_GadList,currenttool);
	if(gad)
		gad->Flags |= GFLG_SELECTED;

	/* Brush Gadget */
	gad = GetGadgetFromID(tw->tw_GadList,(currentbrush->br_ID + TL_BRUSHSTART) );
	if(gad)
		gad->Flags |= GFLG_SELECTED;

	/* Do GridLock Gadget */
	if(gridlockstate)
	{
		gad = GetGadgetFromID(tw->tw_GadList,TL_GRIDLOCK);
		if(gad)
			gad->Flags |= GFLG_SELECTED;
	}
}


/* Main message handler for the tools window */
void ToolsWindowHandler(struct SuperWindow *sw, struct IntuiMessage *imsg)
{
	ULONG	class;
	UWORD	code;
	struct ToolsWindow *tw;

	tw = (struct ToolsWindow *)sw;

	class = imsg->Class;
	code = imsg->Code;

	switch(class)
	{
		case IDCMP_CLOSEWINDOW:
			ReplyMsg((struct Message *)imsg);
			KillOffToolsWindow(sw);
			SussOutQuiting(FALSE);
			break;
		case IDCMP_MENUPICK:
			SortOutToolsWindowMenus( sw,imsg,code );
			break;
		case IDCMP_GADGETDOWN:
			/* Call up the appropriate routine for the gadget groups */
			ToolsWindowGadgetHandler(imsg->IAddress);
			ReplyMsg((struct Message *)imsg);
			break;
		case IDCMP_GADGETUP:
			/* For hit Select Gadgets we can just call up the right routine */
			HandleHitSelectGadget(tw,imsg->IAddress);
			ReplyMsg((struct Message *)imsg);
			break;
		default:
			ReplyMsg((struct Message *)imsg);
			break;
	}
}



/********** SORT OUT TOOLS MENUS **************/

static void SortOutToolsWindowMenus(struct SuperWindow *sw,
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


/******* Tools Window Gadget Handler *********************/
// Well this is the main gadget message handler. Stuff all your
// things like 'currenttool = gad->GadgetID' in here.

static void ToolsWindowGadgetHandler(struct Gadget *gad)
{
	UWORD toolgroup;
	struct Brush *tmpbrush;

	/* Gagdets are gathered together in groups of 20. */
	toolgroup = gad->GadgetID/20;
	switch(toolgroup)
	{
		case 0:		/* Tools */
			if(currenttool != gad->GadgetID)
			{
				prevtool = currenttool;
				currenttool = gad->GadgetID;
				WaitForSubTaskDone(mainmsgport);
				SetToolsWindowGadget(gad->GadgetID,TRUE);
			}
			break;
		case 1:		/* Hit Select (Not needed) */
			break;
		case 2:		/* Toggle Select */
			switch(gad->GadgetID)
			{
				case TL_GRIDLOCK:
					gridlockstate = !(gad->Flags & GFLG_SELECTED);
					break;
			}
			WaitForSubTaskDone(mainmsgport);
			SetToolsWindowGadget( gad->GadgetID, !(gad->Flags & GFLG_SELECTED) );
			break;
		case 3:		/* Brushes */
			tmpbrush = &brushes[gad->GadgetID - TL_BRUSHSTART];
			if( ((tmpbrush == &brushes[BR_CUSTOM]) && (brushes[BR_CUSTOM].br_DrawBrush.bi_Image)) ||
					((tmpbrush == &brushes[BR_ANIMBRUSH]) && (brushes[BR_ANIMBRUSH].br_DrawBrush.bi_Image)) ||
					((tmpbrush != &brushes[BR_CUSTOM]) && (tmpbrush != &brushes[BR_ANIMBRUSH])) )
			{
				WaitForSubTaskDone(mainmsgport);
				currentbrush = tmpbrush;
				SetToolsWindowGadget(gad->GadgetID, TRUE);
			}
			break;
	}
	if(lastprojectwindow && !glob.cg_AutoActivate)
		ActivateWindow(lastprojectwindow->pw_sw.sw_Window);
}



/****************  SetToolsWindowGadget ***********/
//  Go through the full list of tools windows, and set them up
// to use the correct tools, and brushes and crap like that.
// Inputs:	ToolID = The Tools Window Gadget ID.
//					state = TRUE/FALSE, for wheather you want it Set or Not, respectively.

void SetToolsWindowGadget(UWORD toolid,BOOL state)
{
	struct Gadget *gad;
	struct ToolsWindow *tw;
	UWORD	toolgroup;
	UWORD pos;
	struct ProjectWindow *pw;
	WORD	oldwidth,oldheight,xdelta,ydelta;
	WORD	winright,winbottom;

	if(!IsListEmpty(&superwindows))
	{
		toolgroup = toolid/20;

		pw = lastprojectwindow;

		switch(toolid)
		{
			case TL_MAGNIFY:
				if( (pw) && (pw->pw_Magnified) )
				{
					EraseSelector(pw);
					pw->pw_Magnified = FALSE;
					NewTool(pw,prevtool);
					//currenttool = prevtool;
					pw->pw_PixelSize = 1;
					oldwidth = pw->pw_MagWidth;
					oldheight = pw->pw_MagHeight;
					GetTrueWindowSize(pw);
					xdelta = (oldwidth - pw->pw_MagWidth) / 2;
					ydelta = (oldheight - pw->pw_MagHeight) / 2;
					pw->pw_XOffset += xdelta;
					pw->pw_YOffset += ydelta;
					winright = pw->pw_XOffset + pw->pw_MagWidth;
					winbottom = pw->pw_YOffset + pw->pw_MagHeight;
					if(winright > pw->pw_Project->pj_Width)
						pw->pw_XOffset = pw->pw_Project->pj_Width - pw->pw_MagWidth;
					if(winbottom > pw->pw_Project->pj_Height)
						pw->pw_YOffset = pw->pw_Project->pj_Height - pw->pw_MagHeight;
					if(pw->pw_XOffset < 0)
						pw->pw_XOffset = 0;
					if(pw->pw_YOffset < 0)
						pw->pw_YOffset = 0;
					RedisplayProjWindow(pw,TRUE);
					DrawSelector(pw,pw->pw_SelX0,pw->pw_SelY0);
					return;
				}
				break;
		}

		for(tw=(struct ToolsWindow *)superwindows.lh_Head;
				tw->tw_sw.sw_Node.ln_Succ;
				tw = (struct ToolsWindow *)tw->tw_sw.sw_Node.ln_Succ)
		{
			if((tw->tw_sw.sw_Type == SWTY_TOOLS) && (tw->tw_sw.sw_Window))
			{
				/* Remove the GadList so we can twiddle it a bit */
				pos = RemoveGList(tw->tw_sw.sw_Window, tw->tw_GadList, TG_NUMOFGADGETS);

				/* Get a handle on the Gadget from the ToolID */
				for(gad=tw->tw_GadList;
						((gad) && (gad->GadgetID != toolid));
						gad = gad->NextGadget) {}
				if(gad)
				{
					switch(toolgroup)
					{
						case 0:		/* Tools */
							SetToolsGadget(tw,gad);
							break;
						case 1:		/* Hit Select (Shouldn't need this, it has its own handler */
							break;
						case 2:		/* Toggle Select */
							SetToggleGadget(tw,gad,state);
							break;
						case 3:		/*  Brushes   */
							SetBrushGadget(tw,gad);
							break;
					}
				}
				AddGList(tw->tw_sw.sw_Window,tw->tw_GadList,pos,TG_NUMOFGADGETS,NULL);
				RefreshGList(tw->tw_GadList,tw->tw_sw.sw_Window,NULL,TG_NUMOFGADGETS);
			}
		}
	}
}

/********  Set Tools Gadget ****************/
// Set a Tools Gadget for this window.
//
static void SetToolsGadget(struct ToolsWindow *tw,struct Gadget *gad)
{
	struct Gadget *tempgad;

	/* Find the first Brush Gadget */
	for(tempgad = tw->tw_GadList;
			((tempgad) && (tempgad->GadgetID != TL_TOOLSSTART));
			tempgad = tempgad->NextGadget) {}

	/* Deselect all the current Tools Gadgets */
	for(;	((tempgad) && (tempgad->GadgetID <= TL_TOOLSEND) && (tempgad->GadgetID >= TL_TOOLSSTART));
			tempgad = tempgad->NextGadget)
	{	tempgad->Flags &= ~GFLG_SELECTED; }

	/* Set the appropriate Tool Gadget */
	gad->Flags |= GFLG_SELECTED;

}

/********  Set Brush Gadget ****************/
// Set a Brush Gadget for this window.
//
static void SetBrushGadget(struct ToolsWindow *tw,struct Gadget *gad)
{
	struct Gadget *tempgad;

	/*  If we have no custom brush defined, and the user selected to use
	 * the custom brush, then just return. */
	if( (gad->GadgetID == TLB_CUSTOM) && (!brushes[BR_CUSTOM].br_DrawBrush.bi_Image) )
		return;

	/* Find the first Brush Gadget */
	for(tempgad = tw->tw_GadList;
			((tempgad) && (tempgad->GadgetID != TL_BRUSHSTART));
			tempgad = tempgad->NextGadget) {}

	/* Deselect all the current Brush Gadgets */
	for( ;	((tempgad) && (tempgad->GadgetID <= TL_BRUSHEND) && (tempgad->GadgetID >= TL_BRUSHSTART));
			tempgad = tempgad->NextGadget)
		tempgad->Flags &= ~GFLG_SELECTED;

	/* Set the appropriate Tool Gadget */
	gad->Flags |= GFLG_SELECTED;

}

/************ Handle the Hit Select Gadgets ***********/
// Handle the sort of things the Hit Select Gadgets need handling of.
//
static void HandleHitSelectGadget(struct ToolsWindow *tw,struct Gadget *gad)
{
	struct ProjectWindow *pw;

	WaitForSubTaskDone(mainmsgport);

	switch(gad->GadgetID)
	{
		case TL_UNDO:
			if(lastprojectwindow)
				DoUndo(lastprojectwindow->pw_Project);
			break;
		case TL_CLR:
			if( (lastprojectwindow) && (GroovyReq("Clear Project","Are You Sure?","Yes|No")) )
			{
				ResetUndoBuffer(lastprojectwindow->pw_Project);
				SetRast(lastprojectwindow->pw_Project->pj_MainRPort,rmbcol);
				for(pw = lastprojectwindow->pw_Project->pj_ProjectWindows;
						pw;
						pw = pw->pw_NextProjWindow)
					RedisplayProjWindow(pw,TRUE);
				UpdateUndo(lastprojectwindow->pw_Project,0,0);
				UpdateUndo(lastprojectwindow->pw_Project,lastprojectwindow->pw_Project->pj_Width-1,lastprojectwindow->pw_Project->pj_Height-1);
				SussOutModificationStuff(lastprojectwindow->pw_Project);
			}
			break;
	}
	if(lastprojectwindow && !glob.cg_AutoActivate)
		ActivateWindow(lastprojectwindow->pw_sw.sw_Window);
}

/****************** Set toggle Gadget *********************/
// Handle the toggling of the toggle gadgets. Their not true
// toggle select gadgets, but this way I get to have more control over
// them.
//
static void SetToggleGadget(struct ToolsWindow *tw,struct Gadget *gad, UWORD state)
{
	if(state)
		gad->Flags |= GFLG_SELECTED;
	else
		gad->Flags &= ~GFLG_SELECTED;
}

void SetNewTool(UWORD toolid,BOOL state)
{
	UWORD toolgroup;
	struct Brush *tmpbrush;

	/* Gagdets are gathered together in groups of 20. */
	toolgroup = toolid/20;
	switch(toolgroup)
	{
		case 0:		/* Tools */
			if(currenttool != toolid)
			{
				prevtool = currenttool;
				currenttool = toolid;
				WaitForSubTaskDone(mainmsgport);
				SetToolsWindowGadget(toolid,TRUE);
			}
			break;
		case 2:		/* Toggle Select */
			WaitForSubTaskDone(mainmsgport);
			switch(toolid)
			{
				case TL_GRIDLOCK:
					gridlockstate = !gridlockstate;
					SetToolsWindowGadget( toolid, gridlockstate );
					break;
			}
			break;
		case 3:		/* Brushes */
			tmpbrush = &brushes[toolid - TL_BRUSHSTART];
			if( ((tmpbrush == &brushes[BR_CUSTOM]) && (brushes[BR_CUSTOM].br_DrawBrush.bi_Image)) ||
					((tmpbrush == &brushes[BR_ANIMBRUSH]) && (brushes[BR_ANIMBRUSH].br_DrawBrush.bi_Image)) ||
					((tmpbrush != &brushes[BR_CUSTOM]) && (tmpbrush != &brushes[BR_ANIMBRUSH])) )
			{
				WaitForSubTaskDone(mainmsgport);
				currentbrush = tmpbrush;
				SetToolsWindowGadget(toolid, TRUE);
			}
			break;
	}
}
