/****************************************************************************/
//
// BONK: ToolsWindow.c
//
/****************************************************************************/


/*
	Handle all the Stuff to do with the Tools Window.
  A Few Useful Bits Of Terminology
		Tools Window Gadgets = All the gadgets in the Tools Window
		Tools Gadgets = Just the Tools Gadgets themselves, like Line, Curve...
										Not stuff like Brushes, Undo...
 */

#include <stdio.h>
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


struct ToolsWindowGadList
{
	struct List	tgl_List;
	struct Image *tgl_GadImages;
	struct Gadget *tgl_Gadgets;
	UBYTE	*tgl_GadMaps;
	UWORD	tgl_SelectWidth;
	UWORD	tgl_Width;
	UWORD	tgl_Height;
	UWORD	tgl_Depth;
	UWORD	tgl_BrightColour;
	UWORD	tgl_DarkColour;
};

struct ToolsWindowGadget
{
	struct Node tg_Node;
	UWORD	tg_LeftEdge;
	UWORD	tg_TopEdge;
	UWORD	tg_Activation;
	UWORD tg_GadgetID;
	UBYTE	*tg_BrightMap1;
	UBYTE	*tg_DarkMap1;
	UBYTE	*tg_BrightMap2;
	UBYTE	*tg_DarkMap2;
};


struct ToolsWindow
{
	struct SuperWindow tw_sw;
	struct Gadget tw_GadList[TG_NUMOFGADGETS];
	struct Image tw_GadImages[TG_NUMOFGADGETS*2];
	struct ToolsWindowGadList tw_ToolsWindowGadList;
	struct ToolsWindowGadget tw_ToolsWindowGadget[TG_NUMOFGADGETS];
	APTR	tw_GadMaps;
};

UWORD		currenttool = TL_DRAW;
//UWORD		currentbrush = TLB_DOT;
BOOL		gridlockstate = FALSE;



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
		{  NM_ITEM,	"About...",							0,	0,	0,	0,},
		{  NM_ITEM,	"Quit Program...",		 "Q",	0,	0,	0,},

		{ NM_TITLE,	"Edit",									0,	0,	0,	0,},
		{  NM_ITEM,	"Undo",									"Z",	0,	0,	0,},
		{  NM_ITEM,	NM_BARLABEL,						0,	0,	0,	0,},
		{  NM_ITEM,	"Load Blocks...",				0,	0,	0,	0,},
		{  NM_ITEM,	"Load Palette...",			0,	0,	0,	0,},

		{ NM_TITLE,	"Window",								0,	0,	0,	0,},
		{  NM_ITEM,	"Open",									0,	0,	0,	0,},
		{   NM_SUB,	"Map...",								"W",	0,	0,	0,},
		{   NM_SUB,	"Tools...",						 	0,	0,	0,	0,},
		{   NM_SUB,	"Blocks...",						0,	0,	0,	0,},
		{   NM_SUB,	"Brushes...",						0,	0,	0,	0,},
		{   NM_SUB,	"Preview...",						0,	0,	0,	0,},
		{   NM_SUB,	"Reveal...",						0,	0,	0,	0,},
		{   NM_SUB,	"UserConfig...",				0,	0,	0,	0,},
		{   NM_SUB,	"ScreenSettings...",		0,	0,	0,	0,},
		{   NM_SUB,	"About...",							0,	0,	0,	0,},
		{  NM_ITEM,	"Hide",									0,	0,	0,	0,},
		{  NM_ITEM,	"Reveal...",						0,	0,	0,	0,},
		{  NM_ITEM,	"Close",								0,	0,	0,	0,},
//		{  NM_ITEM,	"Make Default",					0,	0,	0,	0,},
		{  NM_ITEM,	NM_BARLABEL,						0,	0,	0,	0,},
		{  NM_ITEM,	"Backdrop?",						0,	CHECKIT|MENUTOGGLE,	0,	0,},
		{  NM_ITEM,	"Borderless?",					0,	CHECKIT|MENUTOGGLE,	0,	0,},
		{  NM_ITEM,	"Title?",								0,	CHECKIT|MENUTOGGLE,	0,	0,},
		{  NM_ITEM,	"BorderGadgets?",				0,	CHECKIT|MENUTOGGLE,	0,	0,},

		{ NM_TITLE,	"Settings",							0,	0,	0,	0,},
		{  NM_ITEM, "Make Backups?",				0,	CHECKIT|MENUTOGGLE,	0,	0,},
//		{  NM_ITEM, "Show Window IDs?",			0,	CHECKIT|MENUTOGGLE,	0,	0,},
		{  NM_ITEM,	NM_BARLABEL,						0,	0,	0,	0,},
		{  NM_ITEM,	"Set Settings...",			0,	0,	0,	0,},
		{  NM_ITEM,	NM_BARLABEL,						0,	0,	0,	0,},
		{  NM_ITEM,	"Load Settings...",			0,	0,	0,	0,},
		{  NM_ITEM,	"Save Settings",				0,	0,	0,	0,},
		{  NM_ITEM,	"Save Settings As...",	0,	0,	0,	0,},

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
#define MN_USER							4

#define MN_EDIT_IDX					13
#define MN_WINDOW_IDX				18
#define MN_SETTINGS_IDX			37
#define MN_USER_IDX					45



#define TG_SIZEOFGADMAP			2*12*8

void __asm MapImages(register __a0 struct MyIMStruct *ms);
void __asm MakeToolsGadgets(register __a0 struct ToolsWindowGadList *tgl);

/* Some nasty external referencing to files incbin'd in Stuff.a */

/* from stuff.a */
extern long __far DrawImage0,DrawImage1,DrawImage2,DrawImage3;
extern long __far LineImage0,LineImage1,LineImage2,LineImage3;
extern long __far BoxImage0,BoxImage1,BoxImage2,BoxImage3;
extern long __far FBoxImage0,FBoxImage1,FBoxImage2,FBoxImage3;
extern long __far CutImage0,CutImage1,CutImage2,CutImage3;
extern long __far FillImage0,FillImage1,FillImage2,FillImage3;
extern long __far UndoImage0,UndoImage1,UndoImage2,UndoImage3;
//extern long __far ClrImage0,ClrImage1,ClrImage2,ClrImage3;
extern long __far PasteImage0,PasteImage1,PasteImage2,PasteImage3;
extern long __far PickImage0,PickImage1,PickImage2,PickImage3;


/* prototypes local to this source file: */
static BOOL ShowToolsWindow(struct SuperWindow *mw);
static void HideToolsWindow(struct SuperWindow *sw);
static void KillOffToolsWindow(struct SuperWindow *sw);
static void ToolsWindowHandler(struct SuperWindow *sw, struct IntuiMessage *imsg);
static BOOL SetupToolWinMenus( struct SuperWindow *sw );
static void DoGadgets(struct ToolsWindow *tw);
static void SortOutToolsWindowMenus(struct SuperWindow *sw,
														 struct IntuiMessage *imsg,
														 UWORD code);
static void SetToolsGadget(struct ToolsWindow *tw,struct Gadget *gad);
static void ToolsWindowGadgetHandler(struct Gadget *gad);
static void HandleHitSelectGadget(struct ToolsWindow *tw,struct Gadget *gad);
static void SetToggleGadget(struct ToolsWindow *tw,struct Gadget *gad, UWORD state);
static void SetGadgetSelection(struct ToolsWindow *tw);


/**************** OpenNewToolsWindow() ****************/
//
// Creates a new Tools Window.
//

BOOL OpenNewToolsWindow(void)
{
	struct ToolsWindow *tw;
	struct	WindowDefault	*wf;

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
	wf = &glob.cg_WinDefs[SWTY_TOOLS];

	tw->tw_sw.sw_Handler = ToolsWindowHandler;
	tw->tw_sw.sw_HideWindow = HideToolsWindow;
	tw->tw_sw.sw_ShowWindow = ShowToolsWindow;
	tw->tw_sw.sw_BuggerOff = KillOffToolsWindow;
	tw->tw_sw.sw_RefreshMenus = SetupToolWinMenus;
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
	tw->tw_sw.sw_MinWidth = 34;
	tw->tw_sw.sw_MinHeight = 34;
	tw->tw_sw.sw_MaxWidth = 0xFFFF;
	tw->tw_sw.sw_MaxHeight = 0xFFFF;
	tw->tw_sw.sw_MenuStrip = NULL;

	tw->tw_sw.sw_FlgBackDrop = wf->wf_FlgBackDrop;
	tw->tw_sw.sw_FlgBorderless = wf->wf_FlgBorderless;
	tw->tw_sw.sw_FlgTitle = wf->wf_FlgTitle;
	tw->tw_sw.sw_FlgBorderGadgets = wf->wf_FlgBorderGadgets; // FALSE

	tw->tw_sw.sw_Title = "Tools";

	/* add ToolsWindow into the superwindows list */
	AddTail(&superwindows, &tw->tw_sw.sw_Node);

	/* open the intuition window (if the window doesn't open, */
	/* it is just left hidden - SEP). */

	ShowToolsWindow(&tw->tw_sw);

	return(TRUE);
}



static BOOL ShowToolsWindow(struct SuperWindow *sw)
{
	struct ToolsWindow *tw;
	ULONG flags, w, h;

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
//		if(sw->sw_FlgBorderGadgets)
//			flags = flags | WFLG_SIZEGADGET | WFLG_SIZEBBOTTOM;

		w=(2*12) + 4;
		h=(5*12) + 4;

		sw->sw_Window = OpenWindowTags(NULL,
											WA_Left,				sw->sw_Left,
											WA_Top,					sw->sw_Top,
//											WA_Width,				sw->sw_Width,
//											WA_Height,			sw->sw_Height,
											WA_InnerWidth,	w,
											WA_InnerHeight,	h,
											WA_MinWidth,		sw->sw_MinWidth,
											WA_MinHeight,		sw->sw_MinHeight,
											WA_MaxWidth,		sw->sw_MaxWidth,
											WA_MaxHeight,		sw->sw_MaxHeight,
											WA_Flags,				flags,
											WA_Backdrop,		sw->sw_FlgBackDrop,
											WA_Borderless,	sw->sw_FlgBorderless,
											WA_CustomScreen, mainscreen,
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

		SetupToolWinMenus( sw );

		/* we want to use our global message port for IDCMP stuff */
		sw->sw_Window->UserPort = mainmsgport;
		ModifyIDCMP(sw->sw_Window,
			IDCMP_CLOSEWINDOW | IDCMP_MENUPICK | IDCMP_CHANGEWINDOW |
			IDCMP_GADGETDOWN | IDCMP_GADGETUP | IDCMP_NEWSIZE | IDCMP_RAWKEY );
	}
	return(TRUE);
}


/************  SetupToolWinMenus()  ************/
//

static BOOL SetupToolWinMenus( struct SuperWindow *sw )
{
	BOOL success = FALSE;

	if (sw->sw_MenuStrip)						/* Clear the menu strip if it exists */
	{
		ClearMenuStrip(sw->sw_Window);
		FreeMenus(sw->sw_MenuStrip);
		sw->sw_MenuStrip = NULL;
	}

	/* Set up some menus */
	SetupWindowMenu( sw, &toolsmenu[MN_WINDOW_IDX] );
	SetupSettingsMenu( &toolsmenu[MN_SETTINGS_IDX] );
	LinkInUserMenu( &toolsmenu[MN_USER_IDX] );

	if( sw->sw_MenuStrip = CreateMenus( toolsmenu, TAG_END ) )
	{
		if ( LayoutMenus( sw->sw_MenuStrip, sw->sw_VisualInfo, TAG_END) )
		{
		if ( SetMenuStrip( sw->sw_Window, sw->sw_MenuStrip) )
			success = TRUE;
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




static void HideToolsWindow(struct SuperWindow *sw)
{
	struct ToolsWindow *tw;

	tw = (struct ToolsWindow *)sw;

	if (sw->sw_Window)												/* make sure it's actually open */
	{
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



static void KillOffToolsWindow(struct SuperWindow *sw)
{
	struct ToolsWindow *tw;

	tw = (struct ToolsWindow *)sw;

	/* close/free the superwindow */
	HideToolsWindow(sw);											/* shut down the Intuition window */
	Remove(&(sw->sw_Node));										/* remove from superwindows list */
	FreeVec(tw->tw_GadMaps);									/* Free the Chip memory for the Gadget Maps */
	FreeVec(tw);															/* free ToolsWindow struct */

}


/************* fitgadget ************/
// Quick'n nasty routine to fit stuff into
// a window. Returns TRUE if we have to go down a line.
// xspace = width of object, x = ptr to xpos counter.

static BOOL fitgadget( struct Window *win, UWORD *x, UWORD xspace )
{
	*x += xspace;
	if( (*x + xspace) > (win->Width - win->BorderRight) )
	{
		/* run out of space - gotta go down a line */
		*x = win->BorderLeft + 2;
		return(TRUE);
	}
	return(FALSE);		// stay on same line
}


/************** DoGadgets **************/
//
// Set up tool window gadgets.
//

static void DoGadgets(struct ToolsWindow *tw)
{
	struct Window *win;
	struct ToolsWindowGadList	*tgl;
	struct ToolsWindowGadget *tg;
	struct DrawInfo *dri;
	UWORD	xpos, ypos, width, height;

	win = tw->tw_sw.sw_Window;

	xpos = win->BorderLeft+2;
	ypos = win->BorderTop+2;
	width = 12;
	height = 12;

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
	tgl->tgl_BrightColour = dri->dri_Pens[SHINEPEN];	//2
	tgl->tgl_DarkColour = dri->dri_Pens[SHADOWPEN];		//1
	FreeScreenDrawInfo( mainscreen, dri );

	/* Gadget setup stuff here!!!! */
	AddTail(&tgl->tgl_List,&tg->tg_Node);
	tg->tg_LeftEdge = xpos;
	tg->tg_TopEdge = ypos;
	tg->tg_Activation = GACT_IMMEDIATE;
	tg->tg_GadgetID = TL_DRAW;
	tg->tg_BrightMap1 = (UBYTE *)&DrawImage0;
	tg->tg_DarkMap1 = (UBYTE *)&DrawImage2;
	tg->tg_BrightMap2 = (UBYTE *)&DrawImage1;
	tg->tg_DarkMap2 = (UBYTE *)&DrawImage3;
	tg++;
	if( fitgadget(win, &xpos, width) ) ypos+=height;

	AddTail(&tgl->tgl_List,&tg->tg_Node);
	tg->tg_LeftEdge = xpos;
	tg->tg_TopEdge = ypos;
	tg->tg_Activation = GACT_IMMEDIATE;
	tg->tg_GadgetID = TL_LINE;
	tg->tg_BrightMap1 = (UBYTE *)&LineImage0;
	tg->tg_DarkMap1 = (UBYTE *)&LineImage2;
	tg->tg_BrightMap2 = (UBYTE *)&LineImage1;
	tg->tg_DarkMap2 = (UBYTE *)&LineImage3;
	tg++;
	if( fitgadget(win, &xpos, width) ) ypos+=height;

	AddTail(&tgl->tgl_List,&tg->tg_Node);
	tg->tg_LeftEdge = xpos;
	tg->tg_TopEdge = ypos;
	tg->tg_Activation = GACT_IMMEDIATE;
	tg->tg_GadgetID = TL_BOX;
	tg->tg_BrightMap1 = (UBYTE *)&BoxImage0;
	tg->tg_DarkMap1 = (UBYTE *)&BoxImage2;
	tg->tg_BrightMap2 = (UBYTE *)&BoxImage1;
	tg->tg_DarkMap2 = (UBYTE *)&BoxImage3;
	tg++;
	if( fitgadget(win, &xpos, width) ) ypos+=height;

	AddTail(&tgl->tgl_List,&tg->tg_Node);
	tg->tg_LeftEdge = xpos;
	tg->tg_TopEdge = ypos;
	tg->tg_Activation = GACT_IMMEDIATE;
	tg->tg_GadgetID = TL_FBOX;
	tg->tg_BrightMap1 = (UBYTE *)&FBoxImage0;
	tg->tg_DarkMap1 = (UBYTE *)&FBoxImage2;
	tg->tg_BrightMap2 = (UBYTE *)&FBoxImage1;
	tg->tg_DarkMap2 = (UBYTE *)&FBoxImage3;
	tg++;
	if( fitgadget(win, &xpos, width) ) ypos+=height;

	AddTail(&tgl->tgl_List,&tg->tg_Node);
	tg->tg_LeftEdge = xpos;
	tg->tg_TopEdge = ypos;
	tg->tg_Activation = GACT_IMMEDIATE;
	tg->tg_GadgetID = TL_FILL;
	tg->tg_BrightMap1 = (UBYTE *)&FillImage0;
	tg->tg_DarkMap1 = (UBYTE *)&FillImage2;
	tg->tg_BrightMap2 = (UBYTE *)&FillImage1;
	tg->tg_DarkMap2 = (UBYTE *)&FillImage3;
	tg++;
	if( fitgadget(win, &xpos, width) ) ypos+=height;

	AddTail(&tgl->tgl_List,&tg->tg_Node);
	tg->tg_LeftEdge = xpos;
	tg->tg_TopEdge = ypos;
	tg->tg_Activation = GACT_IMMEDIATE;
	tg->tg_GadgetID = TL_CUT;
	tg->tg_BrightMap1 = (UBYTE *)&CutImage0;
	tg->tg_DarkMap1 = (UBYTE *)&CutImage2;
	tg->tg_BrightMap2 = (UBYTE *)&CutImage1;
	tg->tg_DarkMap2 = (UBYTE *)&CutImage3;
	tg++;
	if( fitgadget(win, &xpos, width) ) ypos+=height;

	AddTail(&tgl->tgl_List,&tg->tg_Node);
	tg->tg_LeftEdge = xpos;
	tg->tg_TopEdge = ypos;
	tg->tg_Activation = GACT_IMMEDIATE;
	tg->tg_GadgetID = TL_PASTE;
	tg->tg_BrightMap1 = (UBYTE *)&PasteImage0;
	tg->tg_DarkMap1 = (UBYTE *)&PasteImage2;
	tg->tg_BrightMap2 = (UBYTE *)&PasteImage1;
	tg->tg_DarkMap2 = (UBYTE *)&PasteImage3;
	tg++;
	if( fitgadget(win, &xpos, width) ) ypos+=height;

	AddTail(&tgl->tgl_List,&tg->tg_Node);
	tg->tg_LeftEdge = xpos;
	tg->tg_TopEdge = ypos;
	tg->tg_Activation = GACT_IMMEDIATE;
	tg->tg_GadgetID = TL_PICK;
	tg->tg_BrightMap1 = (UBYTE *)&PickImage0;
	tg->tg_DarkMap1 = (UBYTE *)&PickImage2;
	tg->tg_BrightMap2 = (UBYTE *)&PickImage1;
	tg->tg_DarkMap2 = (UBYTE *)&PickImage3;
	tg++;
	if( fitgadget(win, &xpos, width) ) ypos+=height;

	AddTail(&tgl->tgl_List,&tg->tg_Node);
	tg->tg_LeftEdge = xpos;
	tg->tg_TopEdge = ypos;
	tg->tg_Activation = GACT_IMMEDIATE | RELVERIFY;
	tg->tg_GadgetID = TL_UNDO;
	tg->tg_BrightMap1 = (UBYTE *)&UndoImage0;
	tg->tg_DarkMap1 = (UBYTE *)&UndoImage2;
	tg->tg_BrightMap2 = (UBYTE *)&UndoImage1;
	tg->tg_DarkMap2 = (UBYTE *)&UndoImage3;

	/* Call up my little Asm routine to sort out all the stuff, like remapping
			and making up Gadget structures, and crap like that. */
	MakeToolsGadgets(tgl);

	/* Set the selection bit of the appropiate gadgets */
	SetGadgetSelection(tw);

	AddGList(win, tw->tw_GadList, -1, TG_NUMOFGADGETS, NULL);
	RefreshGList(tw->tw_GadList, win, NULL, TG_NUMOFGADGETS);
	RefreshWindowFrame(win);
	tw->tw_sw.sw_GadList = tw->tw_GadList;
}

static void SetGadgetSelection(struct ToolsWindow *tw)
{
	struct Gadget *gad;

	/* Do Tools Gadgets */
	gad = GetGadgetFromID(tw->tw_GadList,currenttool);
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
static void ToolsWindowHandler(struct SuperWindow *sw, struct IntuiMessage *imsg)
{
	ULONG	class;
	UWORD	code, qual;
	struct ToolsWindow *tw;

	tw = (struct ToolsWindow *)sw;

	class = imsg->Class;
	code = imsg->Code;
	qual = imsg->Qualifier;

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
			if(prevprojwin)
				ActivateWindow( prevprojwin->pw_sw.sw_Window );
			break;
		case IDCMP_NEWSIZE:
			DoGadgets(tw);
			break;
		case IDCMP_CHANGEWINDOW:
			ReplyMsg((struct Message *)imsg);
			MakeDefault( sw );
			break;
		case IDCMP_RAWKEY:
			code = HandleGlobalRawKey( sw, code, qual );
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
			case MN_EDIT:
				status |= DoEditMenu(sw,menunumber[i]);
				break;
			case MN_WINDOW:
				status |= DoWindowMenu(sw,menunumber[i]);
				break;
			case MN_SETTINGS:
				DoSettingsMenu(sw,menunumber[i]);
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



void SetTool( UWORD tool )
{
	currenttool = tool;
	if( tool==TL_PASTE && !currentbrush )
		currenttool = TL_DRAW;
	SetToolsWindowGadget( currenttool, TRUE );
	if(prevprojwin)
		ActivateWindow( prevprojwin->pw_sw.sw_Window );
}

UWORD ReadTool( void )
{
	return( currenttool );
}


/******* Tools Window Gadget Handler *********************/
// Well this is the main gadget message handler. Stuff all your
// things like 'currenttool = gad->GadgetID' in here.

static void ToolsWindowGadgetHandler(struct Gadget *gad)
{
	UWORD toolgroup;

	/* Gagdets are gathered together in groups of 20. */
	toolgroup = gad->GadgetID/20;
	switch(toolgroup)
	{
		case 0:		/* Tools */
//			currenttool = gad->GadgetID;
//			SetToolsWindowGadget(gad->GadgetID,TRUE);
				SetTool( gad->GadgetID );
			break;
		case 1:		/* Hit Select (Not needed - handled by hitselect thingy ) */
			break;
		case 2:		/* Toggle Select */
			switch(gad->GadgetID)
			{
				case TL_GRIDLOCK:
					gridlockstate = !(gad->Flags & GFLG_SELECTED);
					break;
			}
			SetToolsWindowGadget( gad->GadgetID, !(gad->Flags & GFLG_SELECTED) );
			break;
		case 3:		/* Brushes */
			break;
	}
}



/****************  SetToolWindowGadget ***********/
//  Go through the full list of tools windows, and sets them up
// to use the correct tools, and brushes and crap like that.
// Inputs:	ToolID = The Tools Window Gadget ID.
//					state = TRUE/FLASE, for whether you want it Set or Not, respectively.

void SetToolsWindowGadget(UWORD toolid,BOOL state)
{
	struct Gadget *gad;
	struct ToolsWindow *tw;
	UWORD	toolgroup;
	UWORD pos;

	if(!IsListEmpty(&superwindows))
	{
		toolgroup = toolid/20;

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
						((gad->GadgetID != toolid) && (gad));
						gad = gad->NextGadget) {}
				if(gad)
				{
					switch(toolgroup)
					{
						case 0:		/* Tools */
							SetToolsGadget(tw,gad);
							break;
						case 1:		/* Hit Select (Shouldn't need this, it has it own handler */
							break;
						case 2:		/* Toggle Select */
							SetToggleGadget(tw,gad,state);
							break;
						case 3:		/*  Brushes   */
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

	/* Deselect all the current Tools Gadgets */
	for(tempgad = tw->tw_GadList;
			((tempgad->GadgetID <= TL_TOOLSEND) && (tempgad));
			tempgad = tempgad->NextGadget)
	{	tempgad->Flags &= ~GFLG_SELECTED; }

	/* Set the appropriate Tool Gadget */
	gad->Flags |= GFLG_SELECTED;

}

/************ Handle the Hit Select Gadgets ***********/
// Handle the sort of things the Hit Select Gadgets need handling of.
//
static void HandleHitSelectGadget(struct ToolsWindow *tw,struct Gadget *gad)
{
	switch( gad->GadgetID )
	{
		case TL_UNDO:
			Undo( &undobuf );
			break;
	}
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


/**************** Get Gadget From ID ******************/
// Return NULL if no Gadget
//
struct Gadget *GetGadgetFromID(struct Gadget *gad,UWORD id)
{
	for( ; ((gad->GadgetID != id) && (gad)); gad = gad->NextGadget) {}
	return(gad);
}
