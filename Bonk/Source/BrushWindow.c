/****************************************************************************/
//
// BONK: BrushWindow.c
//
/****************************************************************************/


#include <stdio.h>
#include <string.h>
//#include <dos/dos.h>
//#include <dos/dostags.h>
#include <exec/exec.h>
#include <intuition/intuition.h>
//#include <graphics/gfxbase.h>
//#include <rexx/rxslib.h>
#include <libraries/gadtools.h>
//#include <libraries/diskfont.h>
//#include <libraries/asl.h>
//#include <libraries/iffparse.h>

#include <proto/exec.h>
//#include <proto/dos.h>
#include <proto/intuition.h>
#include <proto/graphics.h>
//#include <proto/diskfont.h>
//#include <proto/asl.h>
#include <clib/alib_protos.h>
#include <proto/gadtools.h>
//#include <proto/iffparse.h>



#include <global.h>
#include <ExternVars.h>


/* menus */

static struct NewMenu	brushmenu[] =
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

/************ Prototypes for functions local to this file ************/

static BOOL ShowBrushWindow(struct SuperWindow *sw);
static void HideBrushWindow(struct SuperWindow *sw);
static void KillOffBrushWindow(struct SuperWindow *sw);
static void BrushWindowHandler(struct SuperWindow *sw,	struct IntuiMessage *imsg);
static BOOL SetupBrushWinMenus( struct SuperWindow *sw );

static void SortOutBrushWindowMenus(struct SuperWindow *sw,
	struct IntuiMessage *imsg, UWORD code);
static void SetupBrushGadgets(struct BrushWindow *uw);
static WORD CalcUWInnerWidth( struct TextFont *tf );
static WORD CalcUWInnerHeight( struct TextFont *tf );
static void RefreshBrushWindow( struct BrushWindow *uw );

/************ OpenNewBrushWindow ************/
//
//

BOOL OpenNewBrushWindow(void)
{

	struct	BrushWindow	*uw;
	struct	WindowDefault	*wf;

	/* grab mem for the BrushWindow struct */
	if ( !( uw= AllocVec(sizeof(struct BrushWindow), MEMF_CLEAR ) ) )
		return(FALSE);

	/* figure out a unique ID for this window (mainly for AREXX use) */
	InstallUniqueWindowID((struct SuperWindow *)uw);
	uw->uw_sw.sw_Node.ln_Name = (char *)&(uw->uw_sw.sw_idstring);

	/* find the Default settings for this type of SuperWindow */
	wf = &glob.cg_WinDefs[SWTY_BRUSH];

	/* set up handler routines and assorted SuperWindow data */
	uw->uw_sw.sw_Handler = BrushWindowHandler;
	uw->uw_sw.sw_HideWindow = HideBrushWindow;
	uw->uw_sw.sw_ShowWindow = ShowBrushWindow;
	uw->uw_sw.sw_BuggerOff = KillOffBrushWindow;
	uw->uw_sw.sw_RefreshMenus = SetupBrushWinMenus;
	uw->uw_sw.sw_Window = NULL;
	uw->uw_sw.sw_MenuStrip = NULL;
	uw->uw_sw.sw_GadList = NULL;
	uw->uw_sw.sw_Type = SWTY_BRUSH;
	uw->uw_sw.sw_VisualInfo = NULL;

	/* set up stuff for opening the actual intuition window */
	uw->uw_sw.sw_Left = wf->wf_Left;
	uw->uw_sw.sw_Top = wf->wf_Top;
	uw->uw_sw.sw_Width = wf->wf_Width;
	uw->uw_sw.sw_Height = wf->wf_Height;
	uw->uw_sw.sw_MinWidth = 110;					/* minwidth set for topaz8 */
	uw->uw_sw.sw_MaxWidth = 0xFFFF;
	uw->uw_sw.sw_MaxHeight = 0XFFFF;
	uw->uw_sw.sw_MenuStrip = NULL;

	uw->uw_sw.sw_FlgBackDrop = wf->wf_FlgBackDrop;
	uw->uw_sw.sw_FlgBorderless = wf->wf_FlgBorderless;
	uw->uw_sw.sw_FlgTitle = wf->wf_FlgTitle;
	uw->uw_sw.sw_FlgBorderGadgets = wf->wf_FlgBorderGadgets;

	/* setup SuperWindow basename */
	uw->uw_sw.sw_Title = "Brushes";

	/* add window into the SuperWindows list */
	AddTail(&superwindows, &uw->uw_sw.sw_Node);

	/* open the intuition window (if the window doesn't open, */
	/* it is just left hidden - SEP). */
	ShowBrushWindow(&uw->uw_sw);

	return(TRUE);
}



/************ ShowBrushWindow ************/
//

static BOOL ShowBrushWindow(struct SuperWindow *sw)
{
	struct BrushWindow *uw;
	ULONG flags;
	struct Window *win;

	uw = (struct BrushWindow *)sw;

	if(!sw->sw_Window)				/* make sure it isn't already open */
	{
		/* Set up the Visual Info for GadTools */
		if (!(uw->uw_sw.sw_VisualInfo = GetVisualInfo(MAINSCREEN,TAG_END)))
			return(FALSE);

//		minh = mainscreen->Font->ta_YSize + 70 +
//			mainscreen->WBorTop;

//		minh = CalcUWInnerHeight( topaztextfont ) + mainscreen->Font->ta_YSize + 70 +
//			mainscreen->WBorTop;

		flags = WFLG_ACTIVATE | WFLG_NEWLOOKMENUS;
		if(sw->sw_FlgTitle)
			flags = flags | WFLG_DRAGBAR | WFLG_DEPTHGADGET | WFLG_CLOSEGADGET;
		if(sw->sw_FlgBorderGadgets)
			flags = flags | WFLG_SIZEGADGET | WFLG_SIZEBBOTTOM;

		win = sw->sw_Window = OpenWindowTags(NULL,
											WA_Left,				sw->sw_Left,
											WA_Top,					sw->sw_Top,
											WA_Width,				sw->sw_Width,
											WA_Height,			sw->sw_Height,
//											WA_MinWidth,		sw->sw_MinWidth,
//											WA_MinHeight,		minh,
											WA_MaxWidth,		sw->sw_MaxWidth,
											WA_MaxHeight,		sw->sw_MaxHeight,
											WA_Flags,				flags,
											WA_Backdrop,		sw->sw_FlgBackDrop,
											WA_Borderless,	sw->sw_FlgBorderless,
											WA_CustomScreen,	MAINSCREEN,
											WA_AutoAdjust,	TRUE,
											WA_Title,				sw->sw_FlgTitle ?
												BuildWindowTitle(sw) : NULL,
											TAG_DONE,				NULL);

		if( !(sw->sw_Window)) return(FALSE);

		WindowLimits( win, win->BorderLeft + win->BorderRight +
			CalcUWInnerWidth( topaztextfont ),
			win->BorderTop + win->BorderBottom +
			CalcUWInnerHeight( topaztextfont ),
			~0, ~0 );

		/* Tell the window which SuperWindow it is attached to */
		/* (so we'll know which handler routine to call) */
		sw->sw_Window->UserData = (BYTE *)uw;

		SetupBrushWinMenus( sw );

		SetupBrushGadgets( uw );
		RefreshBrushWindow( uw );

		/* we want to use our global message port for IDCMP stuff */
		sw->sw_Window->UserPort = mainmsgport;
		ModifyIDCMP(sw->sw_Window, IDCMP_CLOSEWINDOW | IDCMP_MENUPICK |
			IDCMP_NEWSIZE | IDCMP_REFRESHWINDOW | IDCMP_CHANGEWINDOW |
			IDCMP_RAWKEY | BUTTONIDCMP );
	}

	return(TRUE);
}



/************  SetupBrushWinMenus()  ************/
//

static BOOL SetupBrushWinMenus( struct SuperWindow *sw )
{
	BOOL success = FALSE;

	if (sw->sw_MenuStrip)						/* Clear the menu strip if it exists */
	{
		ClearMenuStrip(sw->sw_Window);
		FreeMenus(sw->sw_MenuStrip);
		sw->sw_MenuStrip = NULL;
	}

	/* Set up some menus */
	SetupWindowMenu( sw, &brushmenu[MN_WINDOW_IDX] );
	SetupSettingsMenu( &brushmenu[MN_SETTINGS_IDX] );
	LinkInUserMenu( &brushmenu[MN_USER_IDX] );

	if( sw->sw_MenuStrip = CreateMenus( brushmenu, TAG_END ) )
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



/******************  HideBrushWindow()  ******************/
//
// Hides the given BrushWindow.
//

static void HideBrushWindow(struct SuperWindow *sw)
{
	struct BrushWindow *uw;

	uw = (struct BrushWindow *)sw;

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

		/* kill the window */
		CloseWindowSafely(sw->sw_Window);				/* close it */
		sw->sw_Window = NULL;										/* indicate that it's gone */

		FreeGadgets(uw->uw_sw.sw_GadList);
		uw->uw_sw.sw_GadList = NULL;

		if (sw->sw_VisualInfo) FreeVisualInfo(sw->sw_VisualInfo);
		sw->sw_VisualInfo = NULL;
		SetProcessWindowPtr();
	}
}


/******************  KillOffBrushWindow()  ******************/
//
// Shuts down a BrushWindow and removes it from the SuperWindows
// list.
//

static void KillOffBrushWindow(struct SuperWindow *sw)
{
	struct BrushWindow *uw;

	uw = (struct BrushWindow *)sw;

	/* close/free the SuperWindow */
	HideBrushWindow(sw);												/* shut down the Intuition window */
	Remove(&(sw->sw_Node));											/* remove from SuperWindows list */
	FreeVec(uw);																/* free BrushWindow struct */
}

/******************  BrushWindowHandler  ******************/
//
// Processes IDCMP messages coming into any BrushWindows,
// dispatching appropriate bits of code.
//

static void BrushWindowHandler(struct SuperWindow *sw,
	struct IntuiMessage *imsg)
{
	ULONG	class;
	UWORD	code, qual;
	struct IntuiMessage *gtimsg;
	struct Gadget *gad;
	struct Brush *tempbrush;

	if( gtimsg = GT_FilterIMsg(imsg) )
	{

		/* read out all we need */
		class = gtimsg->Class;
		code = gtimsg->Code;
		qual = gtimsg->Qualifier;
		gad = (struct Gadget *)gtimsg->IAddress;

		GT_PostFilterIMsg(gtimsg);

		switch(class)
		{
			case IDCMP_CLOSEWINDOW:
				/* tell this window to bugger off */
				ReplyMsg((struct Message *)imsg);
				KillOffBrushWindow(sw);
				SussOutQuiting(FALSE);
				break;
			case IDCMP_REFRESHWINDOW:
				/* repair GadTool renderings */
				ReplyMsg((struct Message *)imsg);
				GT_BeginRefresh(sw->sw_Window);
				GT_EndRefresh(sw->sw_Window,TRUE);
				break;
			case IDCMP_GADGETUP:
//				ReplyMsg((struct Message *)imsg);
//				HandleGadgetUp(sw,gad,code);
				switch( gad->GadgetID )
				{
					case 0:
						/* Goto previous brush */
						currentbrush = (struct Brush *)currentbrush->br_Node.ln_Pred;
						RefreshAllBrushWindows();
						break;
					case 1:
						/* Delete brush */
						if( tempbrush = currentbrush )
						{
							if( currentbrush->br_Node.ln_Succ->ln_Succ )
							{
								currentbrush = (struct Brush *)currentbrush->br_Node.ln_Succ;
							}
							else
							{
								if( currentbrush->br_Node.ln_Pred->ln_Pred )
									currentbrush = (struct Brush *)currentbrush->br_Node.ln_Pred;
								else
									currentbrush = NULL;
									if( ReadTool() == TL_PASTE )
										SetTool( TL_DRAW );
							}
							Remove( &tempbrush->br_Node );
							FreeBrush( tempbrush );
							RefreshAllBrushWindows();
						}
						break;
					case 2:
						/* Goto next brush */
						currentbrush = (struct Brush *)currentbrush->br_Node.ln_Succ;
						RefreshAllBrushWindows();
						break;
				}
				break;
			case IDCMP_MENUPICK:
				/* menuroutine does the reply() */
				SortOutBrushWindowMenus( sw,imsg,code );
				break;
			case IDCMP_NEWSIZE:
				SetupBrushGadgets( (struct BrushWindow *)sw );
				RefreshBrushWindow( (struct BrushWindow *)sw );
				break;
			case IDCMP_CHANGEWINDOW:
				ReplyMsg((struct Message *)imsg);
				MakeDefault( sw );
				break;
			case IDCMP_RAWKEY:
				code = HandleGlobalRawKey( sw, code, qual );
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


/************* SortOutBrushWindowMenus() **************/
//
// Handle MENUPICK IDCMP messages - note the reply() is given
// here rather than in the main loop.
// Why is this so? Well... who can tell?
//

static void SortOutBrushWindowMenus(struct SuperWindow *sw,
													 struct IntuiMessage *imsg,
													 UWORD code)
{
	int i = 0;
	UWORD menunumber[10];
	UWORD menunum,itemnum,subnum;
	UWORD status = 0;

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



/************** SetupBrushGadgets **************/
//
//

static void SetupBrushGadgets(struct BrushWindow *uw)
{
	struct Gadget *gad;
	struct NewGadget ng;
	struct Window *win;
	UWORD buttonh;

	win = uw->uw_sw.sw_Window;

	/* first, kill any old gadgets that may be active */
	if(uw->uw_sw.sw_GadList)
	{
		RemoveGList(win, uw->uw_sw.sw_GadList, -1);
		FreeGadgets(uw->uw_sw.sw_GadList);
		uw->uw_sw.sw_GadList = NULL;
	}

	ClearWindow( win );

	/* use window font */
	SetFont(win->RPort,windowtextfont);
	uw->uw_sw.sw_TextAttr = &windowtextattr;
	uw->uw_sw.sw_TextFont = windowtextfont;

	/* fall back to Topaz 8 if the window is too small */
//	AdjustWindowFont(&uw->uw_sw,(TextLength(win->RPort,"Del",3) * 3 + 4*4),
//									(windowtextfont->tf_YSize) + 3 + 4 + BLKH );
	AdjustWindowFont(&uw->uw_sw, CalcUWInnerWidth( windowtextfont ),
		CalcUWInnerHeight( windowtextfont ) );

	/* init gadget stuff */
	gad = CreateContext(&uw->uw_sw.sw_GadList);

	buttonh = win->RPort->Font->tf_YSize + 3;
	//buttonw = ( win->Width - win->BorderLeft - win->BorderRight - 4*4) / 3;
	/* gadget */
	ng.ng_TextAttr = uw->uw_sw.sw_TextAttr;
	ng.ng_VisualInfo = uw->uw_sw.sw_VisualInfo;
	ng.ng_LeftEdge = win->BorderLeft + 4;
	ng.ng_TopEdge = win->BorderTop + 2;
	ng.ng_Width = MyTextLength( win->RPort->Font, "<" ) + 8;
	ng.ng_Height = buttonh;
	ng.ng_GadgetText = "<";
	ng.ng_GadgetID = 0;
	ng.ng_Flags = 0;

	uw->uw_Gadgets[0] = gad = CreateGadget( BUTTON_KIND, gad, &ng, TAG_END );

	/* gadget */
	ng.ng_LeftEdge += ng.ng_Width + 4;
	ng.ng_Width = MyTextLength( win->RPort->Font, "Del" ) + 8;
	ng.ng_GadgetText = "Del";
	ng.ng_GadgetID = 1;

	uw->uw_Gadgets[1] = gad = CreateGadget( BUTTON_KIND, gad, &ng, TAG_END );

	/* gadget */
	ng.ng_LeftEdge += ng.ng_Width + 4;
	ng.ng_Width = MyTextLength( win->RPort->Font, ">" ) + 8;
	ng.ng_GadgetText = ">";
	ng.ng_GadgetID = 2;

	uw->uw_Gadgets[2] = gad = CreateGadget( BUTTON_KIND, gad, &ng, TAG_END );

	/* check for error */
	if (!gad)
	{
		FreeGadgets(uw->uw_sw.sw_GadList);
		uw->uw_sw.sw_GadList = NULL;
		return;
	}

	/* attach our groovy new gadgets to the window */
	AddGList(win, uw->uw_sw.sw_GadList, -1, -1, NULL);
//	RefreshGList(rw->rw_sw.sw_GadList, win, NULL, -1);
	GT_RefreshWindow(win, NULL);

	/* need to redraw the frame coz we're not using GimmeZeroZero */
	RefreshWindowFrame(win);
}



static WORD CalcUWInnerWidth( struct TextFont *tf )
{
	WORD wibble;
	wibble = MyTextLength( tf, "<" ) +
		MyTextLength( tf, "Del" ) +
		MyTextLength( tf, ">" ) + 4*4 + 3*8;

	return( wibble );
}


static WORD CalcUWInnerHeight( struct TextFont *tf )
{
	WORD wibble;
	wibble = tf->tf_YSize + 3 + 8;
	return( wibble );
}




static void RefreshBrushWindow( struct BrushWindow *uw )
{
	struct Window *win;
	UWORD x, y, *p;
	UWORD xindent, yindent;

	if( (win = uw->uw_sw.sw_Window) && uw->uw_sw.sw_GadList)
	{
		xindent = win->BorderLeft;
		yindent = win->BorderTop + win->RPort->Font->tf_YSize + 3 + 8;
		SetAPen( win->RPort, 0 );
		SetDrMd( win->RPort, JAM1 );
		RectFill( win->RPort, xindent, yindent,
			win->Width - win->BorderRight - 1, win->Height - win->BorderBottom - 1 );

		if( currentbrush )
		{

			if( currentbrush->br_Node.ln_Pred->ln_Pred )
			{
				GT_SetGadgetAttrs( uw->uw_Gadgets[0], win, NULL,
					GA_Disabled, FALSE, TAG_END );
			}
			else
			{
				GT_SetGadgetAttrs( uw->uw_Gadgets[0], win, NULL,
					GA_Disabled, TRUE, TAG_END );
			}
			GT_SetGadgetAttrs( uw->uw_Gadgets[1], win, NULL,
				GA_Disabled, FALSE, TAG_END );
			if( currentbrush->br_Node.ln_Succ->ln_Succ )
			{
				GT_SetGadgetAttrs( uw->uw_Gadgets[2], win, NULL,
					GA_Disabled, FALSE, TAG_END );
			}
			else
			{
				GT_SetGadgetAttrs( uw->uw_Gadgets[2], win, NULL,
					GA_Disabled, TRUE, TAG_END );
			}

			p = currentbrush->br_Data;
			for( y = 0; y < currentbrush->br_Height; y++ )
			{
				for( x = 0; x < currentbrush->br_Width; x++ )
				{
					BltBlockWindow( win, xindent + x*BLKW, yindent + y*BLKH, *p );
					p++;
				}
			}
		}
		else
		{
			GT_SetGadgetAttrs( uw->uw_Gadgets[0], win, NULL,
				GA_Disabled, TRUE, TAG_END );
			GT_SetGadgetAttrs( uw->uw_Gadgets[1], win, NULL,
				GA_Disabled, TRUE, TAG_END );
			GT_SetGadgetAttrs( uw->uw_Gadgets[2], win, NULL,
				GA_Disabled, TRUE, TAG_END );
		}
	}
}




void RefreshAllBrushWindows( void )
{
	struct SuperWindow *sw;

	for (sw = (struct SuperWindow *)superwindows.lh_Head;
		sw->sw_Node.ln_Succ;
		sw = (struct SuperWindow *)sw->sw_Node.ln_Succ )
	{
		if( sw->sw_Window && (sw->sw_Type == SWTY_BRUSH))
			RefreshBrushWindow( (struct BrushWindow *)sw );
	}
}


