/****************************************************************************/
//
// BONK: AboutWindow.c
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
		{  NM_ITEM,	"About...",							0,	0,	0,	0,},
		{  NM_ITEM,	"Quit Program...",		 "Q",	0,	0,	0,},

		{ NM_TITLE,	"Edit",									0,	0,	0,	0,},
		{  NM_ITEM,	"Undo",									"Z",	0,	0,	0,},
		{  NM_ITEM,	NM_BARLABEL,						0,	0,	0,	0,},
		{  NM_ITEM,	"Load Blocks...",				0,	0,	0,	0,},
		{  NM_ITEM,	"Load Palette...",			0,	0,	0,	0,},

		{ NM_TITLE,	"Window",								0,	0,	0,	0,},
		{  NM_ITEM,	"Open",									0,	0,	0,	0,},
		{   NM_SUB,	"Map...",								0,	0,	0,	0,},
		{   NM_SUB,	"Tools...",						 	0,	0,	0,	0,},
		{   NM_SUB,	"Blocks...",						0,	0,	0,	0,},
		{   NM_SUB,	"Brushes...",						0,	0,	0,	0,},
		{   NM_SUB,	"Preview...",						0,	0,	0,	0,},
		{   NM_SUB,	"Reveal...",						0,	0,	0,	0,},
		{   NM_SUB,	"UserConfig...",				0,	0,	0,	0,},
		{   NM_SUB,	"ScreenConfig...",			0,	0,	0,	0,},
		{   NM_SUB,	"About...",							0,	0,	0,	0,},
		{  NM_ITEM,	"Hide",									0,	0,	0,	0,},
		{  NM_ITEM,	"Reveal...",						0,	0,	0,	0,},
		{  NM_ITEM,	"Close",								0,	0,	0,	0,},
		{  NM_ITEM,	"Make Default",					0,	0,	0,	0,},
		{  NM_ITEM,	NM_BARLABEL,						0,	0,	0,	0,},
		{  NM_ITEM,	"Backdrop?",						0,	CHECKIT|MENUTOGGLE,	0,	0,},
		{  NM_ITEM,	"Borderless?",					0,	CHECKIT|MENUTOGGLE,	0,	0,},
		{  NM_ITEM,	"Title?",								0,	CHECKIT|MENUTOGGLE,	0,	0,},
		{  NM_ITEM,	"BorderGadgets?",				0,	CHECKIT|MENUTOGGLE,	0,	0,},

		{ NM_TITLE,	"Settings",							0,	0,	0,	0,},
		{  NM_ITEM, "Create Icons?",				0,	CHECKIT|MENUTOGGLE,	0,	0,},
		{  NM_ITEM, "Show Window IDs?",			0,	CHECKIT|MENUTOGGLE,	0,	0,},
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
#define MN_SETTINGS_IDX			38
#define MN_USER_IDX					47



/************ Prototypes for functions local to this file ************/

static BOOL ShowAboutWindow(struct SuperWindow *sw);
static void HideAboutWindow(struct SuperWindow *sw);
static void KillOffAboutWindow(struct SuperWindow *sw);
static void AboutWindowHandler(struct SuperWindow *sw,	struct IntuiMessage *imsg);
static BOOL SetupAboutWinMenus( struct SuperWindow *sw );

static void SortOutAboutWindowMenus(struct SuperWindow *sw,
	struct IntuiMessage *imsg, UWORD code);
static void LayOutGadgets( struct AboutWindow *aw );
static void FreeAboutGadgets( struct AboutWindow *aw );

/************ OpenNewAboutWindow ************/
//

BOOL OpenNewAboutWindow( void )
{
	struct List *l;

	if( l = BlockAllWindows() )
	{
		GroovyReq( PROGNAME, "Version 0.90 (Beta)\nBy Ben Campbell\n©1996 42° South", "Cool!" );
	
		FreeBlockedWindows( l );
	}
	return TRUE;
}



BOOL OldOpenNewAboutWindow( void )
{

	struct	AboutWindow	*aw;
	struct	WindowDefault	*wf;

	/* grab mem for the AboutWindow struct */
	if ( !( aw= AllocVec(sizeof(struct AboutWindow), MEMF_CLEAR ) ) )
		return(FALSE);

	/* figure out a unique ID for this window (mainly for AREXX use) */
	InstallUniqueWindowID((struct SuperWindow *)aw);
	aw->aw_sw.sw_Node.ln_Name = (char *)&(aw->aw_sw.sw_idstring);

	/* find the Default settings for this type of SuperWindow */
	wf = &glob.cg_WinDefs[SWTY_ABOUT];

	/* set up handler routines and assorted SuperWindow data */
	aw->aw_sw.sw_Handler = AboutWindowHandler;
	aw->aw_sw.sw_HideWindow = HideAboutWindow;
	aw->aw_sw.sw_ShowWindow = ShowAboutWindow;
	aw->aw_sw.sw_BuggerOff = KillOffAboutWindow;
	aw->aw_sw.sw_RefreshMenus = SetupAboutWinMenus;
	aw->aw_sw.sw_Window = NULL;
	aw->aw_sw.sw_MenuStrip = NULL;
	aw->aw_sw.sw_GadList = NULL;
	aw->aw_sw.sw_Type = SWTY_ABOUT;
	aw->aw_sw.sw_VisualInfo = NULL;

	/* set up stuff for opening the actual intuition window */
	aw->aw_sw.sw_Left = wf->wf_Left;
	aw->aw_sw.sw_Top = wf->wf_Top;
	aw->aw_sw.sw_Width = wf->wf_Width;
	aw->aw_sw.sw_Height = wf->wf_Height;
	aw->aw_sw.sw_MinWidth = 48;
	aw->aw_sw.sw_MaxWidth = 0xFFFF;
	aw->aw_sw.sw_MaxHeight = 0XFFFF;
	aw->aw_sw.sw_MenuStrip = NULL;

	aw->aw_sw.sw_FlgBackDrop = wf->wf_FlgBackDrop;
	aw->aw_sw.sw_FlgBorderless = wf->wf_FlgBorderless;
	aw->aw_sw.sw_FlgTitle = wf->wf_FlgTitle;
	aw->aw_sw.sw_FlgBorderGadgets = wf->wf_FlgBorderGadgets;

	/* setup SuperWindow basename */
	aw->aw_sw.sw_Title = "About";

	/* add window into the SuperWindows list */
	AddTail(&superwindows, &aw->aw_sw.sw_Node);

	/* open the intuition window (if the window doesn't open, */
	/* it is just left hidden - SEP). */
	ShowAboutWindow(&aw->aw_sw);

	return(TRUE);
}



/************ ShowAboutWindow ************/
//

static BOOL ShowAboutWindow(struct SuperWindow *sw)
{
	struct AboutWindow *aw;
	UWORD minh;
	ULONG flags;

	aw = (struct AboutWindow *)sw;

	if(!sw->sw_Window)				/* make sure it isn't already open */
	{
		/* Set up the Visual Info for GadTools */
		if (!(aw->aw_sw.sw_VisualInfo = GetVisualInfo( mainscreen,TAG_END )))
			return(FALSE);

		minh = mainscreen->Font->ta_YSize + 10 +
			mainscreen->WBorTop;

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
											WA_Backdrop,		sw->sw_FlgBackDrop,
											WA_Borderless,	sw->sw_FlgBorderless,
											WA_CustomScreen,	MAINSCREEN,
											WA_AutoAdjust,	TRUE,
											WA_Title,				sw->sw_FlgTitle ?
												BuildWindowTitle(sw) : NULL,
											TAG_DONE,				NULL);

		if( !(sw->sw_Window)) return(FALSE);

		/* Tell the window which SuperWindow it is attached to */
		/* (so we'll know which handler routine to call) */
		sw->sw_Window->UserData = (BYTE *)aw;

		SetupAboutWinMenus( sw );

		/* we want to use our global message port for IDCMP stuff */
		sw->sw_Window->UserPort = mainmsgport;
		ModifyIDCMP(sw->sw_Window, IDCMP_CLOSEWINDOW | IDCMP_MENUPICK |
			IDCMP_NEWSIZE | IDCMP_REFRESHWINDOW | IDCMP_CHANGEWINDOW |
			IDCMP_MOUSEBUTTONS | IDCMP_RAWKEY |
			BUTTONIDCMP | TEXTIDCMP );

		/* setup other stuff */
		LayOutGadgets( (struct AboutWindow *)sw );
	}
	return(TRUE);
}


/************  SetupAboutWinMenus()  ************/
//

static BOOL SetupAboutWinMenus( struct SuperWindow *sw )
{
	BOOL success = FALSE;

	if (sw->sw_MenuStrip)						/* Clear the menu strip if it exists */
	{
		ClearMenuStrip(sw->sw_Window);
		FreeMenus(sw->sw_MenuStrip);
		sw->sw_MenuStrip = NULL;
	}

	/* Set up some menus */
	SetupWindowMenu( sw, &aboutmenu[MN_WINDOW_IDX] );
	SetupSettingsMenu( &aboutmenu[MN_SETTINGS_IDX] );
	LinkInUserMenu( &aboutmenu[MN_USER_IDX] );

	if( sw->sw_MenuStrip = CreateMenus( aboutmenu, TAG_END ) )
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





/******************  HideAboutWindow()  ******************/
//
// Hides the given AboutWindow.
//

static void HideAboutWindow(struct SuperWindow *sw)
{
	struct AboutWindow *aw;

	aw = (struct AboutWindow *)sw;

	if (sw->sw_Window)												/* make sure it's actually open */
	{
		if (sw->sw_MenuStrip)										/* Clear the menu strip if it exists */
		{
			ClearMenuStrip(sw->sw_Window);
			FreeMenus(sw->sw_MenuStrip);
			sw->sw_MenuStrip = NULL;
		}
		RememberWindow(sw);											/* save old window data */

		FreeAboutGadgets( aw );

		/* free up string used for window title */
		if(sw->sw_Window->Title)
			FreeVec(sw->sw_Window->Title);

		/* kill the window */
		CloseWindowSafely(sw->sw_Window);				/* close it */
		sw->sw_Window = NULL;										/* indicate that it's gone */

		if (sw->sw_VisualInfo) FreeVisualInfo(sw->sw_VisualInfo);
		sw->sw_VisualInfo = NULL;
		SetProcessWindowPtr();
	}
}


/******************  KillOffAboutWindow()  ******************/
//
// Shuts down a AboutWindow and removes it from the SuperWindows
// list.
//

static void KillOffAboutWindow(struct SuperWindow *sw)
{
	struct AboutWindow *aw;

	aw = (struct AboutWindow *)sw;

	/* close/free the SuperWindow */
	HideAboutWindow(sw);											/* shut down the Intuition window */
	Remove(&(sw->sw_Node));											/* remove from SuperWindows list */
	FreeVec(aw);																/* free BlockWindow struct */
}

/******************  AboutWindowHandler  ******************/
//
// Processes IDCMP messages coming into any BlockWindows,
// dispatching appropriate bits of code.
//

static void AboutWindowHandler(struct SuperWindow *sw,
	struct IntuiMessage *imsg)
{
	struct AboutWindow *aw;
	ULONG	class, seconds, micros;
	UWORD	code, qual;
	UWORD	mx,my;
	struct IntuiMessage *gtimsg;
	struct Gadget *gad;

	aw = (struct AboutWindow *)sw;

	if( gtimsg = GT_FilterIMsg(imsg) )
	{

		/* read out all we need */
		class = gtimsg->Class;
		code = gtimsg->Code;
		qual = gtimsg->Qualifier;
		mx = gtimsg->MouseX;
		my = gtimsg->MouseY;
		seconds = gtimsg->Seconds;
		micros = gtimsg->Micros;

		gad = (struct Gadget *)gtimsg->IAddress;

		GT_PostFilterIMsg(gtimsg);

		switch(class)
		{
			case IDCMP_CLOSEWINDOW:
				/* tell this window to bugger off */
				ReplyMsg((struct Message *)imsg);
				KillOffAboutWindow(sw);
//				SussOutQuiting(FALSE);
				break;
			case IDCMP_REFRESHWINDOW:
				/* repair GadTool renderings */
				ReplyMsg((struct Message *)imsg);
				GT_BeginRefresh(sw->sw_Window);
				GT_EndRefresh(sw->sw_Window,TRUE);
				break;
			case IDCMP_GADGETUP:
				ReplyMsg((struct Message *)imsg);
				KillOffAboutWindow(sw);
//				HandleGadgetUp( (struct AboutWindow *)sw, gad, code);
				break;
			case IDCMP_MENUPICK:
				/* menuroutine does the reply() */
				SortOutAboutWindowMenus( sw,imsg,code );
				break;
			case IDCMP_NEWSIZE:
				/* resuss all the gadget placements/font stuff */
				/* according to the new windowsize. */
				ReplyMsg((struct Message *)imsg);
				LayOutGadgets( (struct AboutWindow *)sw );
				break;
			case IDCMP_RAWKEY:
				ReplyMsg((struct Message *)imsg);
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


/************* SortOutAboutWindowMenus() **************/
//
// Handle MENUPICK IDCMP messages - note the reply() is given
// here rather than in the main loop.
// Why is this so? Well... who can tell?
//

static void SortOutAboutWindowMenus(struct SuperWindow *sw,
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




/***************** LayOutGadgets() **************/

static void LayOutGadgets( struct AboutWindow *aw )
{
	struct Window *win;
	struct Gadget *gad;
	struct NewGadget ng;
	struct TextFont *tf = windowtextfont;

	static char *textlines[] =
		{
			{ "Bonk - The MapEditor" },
			{ "Copyright © 1994 42° South" },
			{	"Version "VSTRING },
			{	"" },
			{ "By Ben Campbell" },
			{ "Compiled "__DATE__ },
		};

#define NUMOFLINES 6

	struct	DrawInfo *dri;
	UWORD i, left, width, y;

	if( !(dri = GetScreenDrawInfo( mainscreen ) ) )
		return;

	if(	aw->aw_sw.sw_GadList )
		FreeAboutGadgets( aw );

	win = aw->aw_sw.sw_Window;
	ClearWindow( win );

	gad = CreateContext( &aw->aw_sw.sw_GadList );

	ng.ng_TextAttr = &windowtextattr;
	ng.ng_VisualInfo = aw->aw_sw.sw_VisualInfo;
	ng.ng_LeftEdge = win->BorderLeft + 4;
	ng.ng_Height = windowtextfont->tf_YSize + 4;
	ng.ng_TopEdge = win->Height - win->BorderBottom - ng.ng_Height - 4;
	ng.ng_Width = win->Width - win->BorderLeft - win->BorderRight - 8;
	ng.ng_GadgetText = NULL;
	ng.ng_Flags = 0;

	gad = CreateGadget( BUTTON_KIND, gad, &ng, TAG_END );

	if( !gad )
	{
		FreeAboutGadgets( aw );
		return;
	}



	AddGList(win, aw->aw_sw.sw_GadList, -1, -1, NULL);
	GT_RefreshWindow( win, NULL );
	/* need to redraw the frame coz we're not using GimmeZeroZero */
	RefreshWindowFrame( win );


	/* draw text */

	y = win->BorderTop + 4 + tf->tf_Baseline;
	width = win->Width - win->BorderLeft - win->BorderRight - 8;
//	SetAPen( win->RPort, 1 );
	SetDrMd( win->RPort, JAM1 );

	for( i=0; i < NUMOFLINES; i++ )
	{
		left = ( width - MyTextLength( tf, textlines[i] ) ) / 2;
//		SetAPen( win->RPort, 1 );
//		Move( win->RPort, left + win->BorderLeft + 5, y+1 );
//		Text( win->RPort, textlines[i], strlen( textlines[i] ) );
		SetAPen( win->RPort, dri->dri_Pens[ TEXTPEN ] );
		Move( win->RPort, left + win->BorderLeft + 4, y );
		Text( win->RPort, textlines[i], strlen( textlines[i] ) );
		y += tf->tf_YSize;
	}
	FreeScreenDrawInfo( mainscreen, dri );
}


/***************** FreeAboutGadgets() **************/

static void FreeAboutGadgets( struct AboutWindow *aw )
{
	/* kill any old gadgets that may be active */
	if(aw->aw_sw.sw_GadList)
	{
		RemoveGList(aw->aw_sw.sw_Window, aw->aw_sw.sw_GadList, -1);
		FreeGadgets(aw->aw_sw.sw_GadList);
		aw->aw_sw.sw_GadList = NULL;
	}
}

