/****************************************************************************/
//
// BONK: PreviewWindow.c
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

static struct NewMenu	previewmenu[] =
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

static BOOL ShowPreviewWindow(struct SuperWindow *sw);
static void HidePreviewWindow(struct SuperWindow *sw);
static void KillOffPreviewWindow(struct SuperWindow *sw);
static void PreviewWindowHandler(struct SuperWindow *sw,	struct IntuiMessage *imsg);
static BOOL SetupPreviewWinMenus( struct SuperWindow *sw );

static void SortOutPreviewWindowMenus(struct SuperWindow *sw,
	struct IntuiMessage *imsg, UWORD code);

static void UnlinkPreviewWindow(struct PreviewWindow *vw);
static struct PreviewWindow *GetPrevPreviewWindow(struct PreviewWindow *vw);



BOOL OldOpenNewPreviewWindow( struct Project *proj )
{
	return( OpenNewBlkAnimWindow(proj) );
}

/************ OpenNewPreviewWindow ************/
//

BOOL OpenNewPreviewWindow( struct Project *proj )
{

	struct	PreviewWindow	*vw;
	struct	WindowDefault	*wf;

	if( !proj ) return(FALSE);

	/* grab mem for the PreviewWindow struct */
	if ( !( vw= AllocVec(sizeof(struct PreviewWindow), MEMF_CLEAR ) ) )
		return(FALSE);

	/* figure out a unique ID for this window (mainly for AREXX use) */
	InstallUniqueWindowID((struct SuperWindow *)vw);
	vw->vw_sw.sw_Node.ln_Name = (char *)&(vw->vw_sw.sw_idstring);

	/* find the Default settings for this type of SuperWindow */
	wf = &glob.cg_WinDefs[SWTY_PREVIEW];

	/* attach it to the Project */
	vw->vw_Project = proj;

	vw->vw_NextPreviewWindow = proj->pj_PreviewWindows;
	proj->pj_PreviewWindows = vw;

	/* set up handler routines and assorted SuperWindow data */
	vw->vw_sw.sw_Handler = PreviewWindowHandler;
	vw->vw_sw.sw_HideWindow = HidePreviewWindow;
	vw->vw_sw.sw_ShowWindow = ShowPreviewWindow;
	vw->vw_sw.sw_BuggerOff = KillOffPreviewWindow;
	vw->vw_sw.sw_RefreshMenus = SetupPreviewWinMenus;
	vw->vw_sw.sw_Window = NULL;
	vw->vw_sw.sw_MenuStrip = NULL;
	vw->vw_sw.sw_GadList = NULL;
	vw->vw_sw.sw_Type = SWTY_PREVIEW;
	vw->vw_sw.sw_VisualInfo = NULL;

	/* set up stuff for opening the actual intuition window */
	vw->vw_sw.sw_Left = wf->wf_Left;
	vw->vw_sw.sw_Top = wf->wf_Top;
	vw->vw_sw.sw_Width = wf->wf_Width;
	vw->vw_sw.sw_Height = wf->wf_Height;
	vw->vw_sw.sw_MinWidth = 48;
	vw->vw_sw.sw_MaxWidth = 0xFFFF;
	vw->vw_sw.sw_MaxHeight = 0XFFFF;
	vw->vw_sw.sw_MenuStrip = NULL;

	vw->vw_sw.sw_FlgBackDrop = wf->wf_FlgBackDrop;
	vw->vw_sw.sw_FlgBorderless = wf->wf_FlgBorderless;
	vw->vw_sw.sw_FlgTitle = wf->wf_FlgTitle;
	vw->vw_sw.sw_FlgBorderGadgets = wf->wf_FlgBorderGadgets;

	/* setup SuperWindow basename */
	vw->vw_sw.sw_Title = "Preview";

	/* add window into the SuperWindows list */
	AddTail(&superwindows, &vw->vw_sw.sw_Node);

	/* open the intuition window (if the window doesn't open, */
	/* it is just left hidden - SEP). */
	ShowPreviewWindow(&vw->vw_sw);

	return(TRUE);
}



/************ ShowPreviewWindow ************/
//

static BOOL ShowPreviewWindow(struct SuperWindow *sw)
{
	struct PreviewWindow *vw;
	UWORD minh;
	ULONG flags;

	vw = (struct PreviewWindow *)sw;

	if(!sw->sw_Window)				/* make sure it isn't already open */
	{
		/* Set up the Visual Info for GadTools */
		if (!(vw->vw_sw.sw_VisualInfo = GetVisualInfo( mainscreen,TAG_END )))
			return(FALSE);

		minh = mainscreen->Font->ta_YSize + 10 +
			mainscreen->WBorTop;

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
		sw->sw_Window->UserData = (BYTE *)vw;

		SetupPreviewWinMenus( sw );

		/* we want to use our global message port for IDCMP stuff */
		sw->sw_Window->UserPort = mainmsgport;
		ModifyIDCMP(sw->sw_Window, IDCMP_CLOSEWINDOW | IDCMP_MENUPICK |
			IDCMP_NEWSIZE | IDCMP_REFRESHWINDOW | IDCMP_CHANGEWINDOW |
			IDCMP_RAWKEY );

		if( !globlocks->bs_PreviewColours)
			CalcBlockPreviewColours(globlocks);
		DrawMapPreview( vw );

	}

	return(TRUE);
}


/************  SetupPreviewWinMenus()  ************/
//

static BOOL SetupPreviewWinMenus( struct SuperWindow *sw )
{
	BOOL success = FALSE;

	if (sw->sw_MenuStrip)						/* Clear the menu strip if it exists */
	{
		ClearMenuStrip(sw->sw_Window);
		FreeMenus(sw->sw_MenuStrip);
		sw->sw_MenuStrip = NULL;
	}

	/* Set up some menus */
	SetupWindowMenu( sw, &previewmenu[MN_WINDOW_IDX] );
	SetupSettingsMenu( &previewmenu[MN_SETTINGS_IDX] );
	LinkInUserMenu( &previewmenu[MN_USER_IDX] );

	if( sw->sw_MenuStrip = CreateMenus( previewmenu, TAG_END ) )
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





/******************  HidePreviewWindow()  ******************/
//
// Hides the given PreviewWindow.
//

static void HidePreviewWindow(struct SuperWindow *sw)
{
	struct PreviewWindow *vw;

	vw = (struct PreviewWindow *)sw;

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

		if (sw->sw_VisualInfo) FreeVisualInfo(sw->sw_VisualInfo);
		sw->sw_VisualInfo = NULL;
		SetProcessWindowPtr();
	}
}


/******************  KillOffPreviewWindow()  ******************/
//
// Shuts down a PreviewWindow and removes it from the SuperWindows
// list.
//

static void KillOffPreviewWindow(struct SuperWindow *sw)
{
	struct PreviewWindow *vw;
	struct Project *proj;

	vw = (struct PreviewWindow *)sw;

	/* remove it from the project */

	if( proj = vw->vw_Project )
		UnlinkPreviewWindow(vw);

	/* close/free the SuperWindow */
	HidePreviewWindow(sw);											/* shut down the Intuition window */
	Remove(&(sw->sw_Node));											/* remove from SuperWindows list */
	FreeVec(vw);																/* free BlockWindow struct */
}

/******************  PreviewWindowHandler  ******************/
//
// Processes IDCMP messages coming into any BlockWindows,
// dispatching appropriate bits of code.
//

static void PreviewWindowHandler(struct SuperWindow *sw,
	struct IntuiMessage *imsg)
{
	ULONG	class;
	UWORD	code, qual;
	struct IntuiMessage *gtimsg;
	struct Gadget *gad;

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
				KillOffPreviewWindow(sw);
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
				break;
			case IDCMP_MENUPICK:
				/* menuroutine does the reply() */
				SortOutPreviewWindowMenus( sw,imsg,code );
				break;
			case IDCMP_NEWSIZE:
				/* resuss all the gadget placements/font stuff */
				/* according to the new windowsize. */
//				SetupRevealGadgets((struct BlockWindow *)sw);
				DrawMapPreview( (struct PreviewWindow *)sw );
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


/************* SortOutPreviewWindowMenus() **************/
//
// Handle MENUPICK IDCMP messages - note the reply() is given
// here rather than in the main loop.
// Why is this so? Well... who can tell?
//

static void SortOutPreviewWindowMenus(struct SuperWindow *sw,
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


void DrawMapPreview( struct PreviewWindow *vw )
{
	UWORD x, y, *p;
	struct Project *proj;
	struct Window *win;

	if( win = vw->vw_sw.sw_Window )
	{
		if( proj = vw->vw_Project )
		{
			for( y=0; y < win->Height-win->BorderTop-win->BorderBottom; y++ )
			{
				p = proj->pj_Map + ((y+vw->vw_WinY) * proj->pj_MapW) + vw->vw_WinX;
				for( x=0; x < win->Width-win->BorderLeft-win->BorderRight; x++ )
				{

					if(x<proj->pj_MapW && y<proj->pj_MapH)
					{
						SetAPen( win->RPort, FindBlockColour( globlocks, *p++ ) );
						WritePixel( win->RPort, win->BorderLeft+x, win->BorderTop+y );
					}
				}
			}
		}
	}
}



/**************** UnlinkPreviewWindow() ****************/
//
// Detaches a previewwindow from its parent project.
// (note that we assume that the window _is_ actually
// attached to a project in the first place).

static void UnlinkPreviewWindow(struct PreviewWindow *vw)
{
	struct PreviewWindow *prevwindow,*nextwindow;
	struct Project *proj;

	proj = vw->vw_Project;

	nextwindow = vw->vw_NextPreviewWindow;
	prevwindow = GetPrevPreviewWindow(vw);

	if (prevwindow == NULL)
		proj->pj_PreviewWindows = nextwindow;
	else
		prevwindow->vw_NextPreviewWindow = nextwindow;
}


/***************** GetPrevPreviewWindow() **************/
//
//  Find the prevous previewwindow connected to this project.
// If there is no previous window, then NULL is returned.

static struct PreviewWindow *GetPrevPreviewWindow(struct PreviewWindow *vw)
{
	struct PreviewWindow *prevwindow;
	struct Project *proj;

	proj = vw->vw_Project;
	prevwindow = proj->pj_PreviewWindows;

	if (prevwindow == vw)
		prevwindow = NULL;
	else
	{
		while (prevwindow->vw_NextPreviewWindow != vw)
			prevwindow = prevwindow->vw_NextPreviewWindow;
	}
	return(prevwindow);
}



void NewDrawMapPreview( struct PreviewWindow *vw )
{
	UWORD x, y, *p;
	struct Project *proj;
	struct Window *win;
	struct BitMap *bm;

	if( win = vw->vw_sw.sw_Window )
	{
		if( proj = vw->vw_Project )
		{
			if( bm = CreateBitMap( proj->pj_MapW, proj->pj_MapH,
				globlocks->bs_Depth, TRUE ) )
			{
				for( y=0; y < win->Height-win->BorderTop-win->BorderBottom; y++ )
				{
					p = proj->pj_Map + ((y+vw->vw_WinY) * proj->pj_MapW) + vw->vw_WinX;
					for( x=0; x < win->Width-win->BorderLeft-win->BorderRight; x++ )
					{

						if(x<proj->pj_MapW && y<proj->pj_MapH)
						{
//							SetAPen( win->RPort, FindBlockColour( globlocks, *p++ ) );
//							WritePixel( win->RPort, win->BorderLeft+x, win->BorderTop+y );
							BMPlot( bm, x, y, FindBlockColour( globlocks, *p++ ) );
						}
					}
				}
				BltBitMapRastPort( bm, 0, 0, win->RPort,
					win->BorderLeft,win->BorderTop, x, y, 0xC0 );

				DestroyBitMap( bm, proj->pj_MapW, proj->pj_MapH, globlocks->bs_Depth );
			}
		}
	}
}

