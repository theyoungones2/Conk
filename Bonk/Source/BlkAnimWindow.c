/****************************************************************************/
//
// BONK: BlkAnimWindow.c
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


UWORD powers_of_two[] = { 1,2,4,8,16,32,64,128,256 };

/* menus */

static struct NewMenu	blkanimmenu[] =
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
		{   NM_SUB,	"Project...",						0,	0,	0,	0,},
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



/****** Gadget indexs/IDs ******/

#define GAD_MINUS			0
#define GAD_ANIMNUM		1
#define GAD_PLUS			2
#define GAD_NEWANIM		3
#define GAD_DELANIM		4
#define GAD_LENGTH		5
#define GAD_POS				6
#define GAD_BLOCK			7
#define GAD_SCROLLER	8
#define GAD_PUTBLK		9
#define GAD_INSBLK		10
#define GAD_DELBLK		11
#define GAD_UNDO			12





/************ Prototypes for functions local to this file ************/

static BOOL ShowBlkAnimWindow(struct SuperWindow *sw);
static void HideBlkAnimWindow(struct SuperWindow *sw);
static void KillOffBlkAnimWindow(struct SuperWindow *sw);
static void BlkAnimWindowHandler(struct SuperWindow *sw,	struct IntuiMessage *imsg);
static BOOL SetupBlkAnimWinMenus( struct SuperWindow *sw );
static void HandleGadgetUp( struct BlkAnimWindow *baw, struct Gadget *gad, UWORD code );

static void SortOutBlkAnimWindowMenus(struct SuperWindow *sw,
	struct IntuiMessage *imsg, UWORD code);

static void UnlinkBlkAnimWindow(struct BlkAnimWindow *baw);
static struct BlkAnimWindow *GetPrevBlkAnimWindow(struct BlkAnimWindow *baw);


static void SussBawGadgets( struct BlkAnimWindow *baw );
static void FreeBawGadgets( struct BlkAnimWindow *baw );
static void DrawBlkView( struct BlkAnimWindow *baw );
void HandleSingleClick( struct BlkAnimWindow *baw, WORD x, WORD y );
void HandleDoubleClick( struct BlkAnimWindow *baw, WORD x, WORD y );

/************ OpenNewBlkAnimWindow ************/
//

BOOL OpenNewBlkAnimWindow( struct Project *proj )
{

	struct	BlkAnimWindow	*baw;
	struct	WindowDefault	*wf;

	if( !proj ) return(FALSE);

	/* grab mem for the BlkAnimWindow struct */
	if ( !( baw= AllocVec(sizeof(struct BlkAnimWindow), MEMF_CLEAR ) ) )
		return(FALSE);

	/* figure out a unique ID for this window (mainly for AREXX use) */
	InstallUniqueWindowID((struct SuperWindow *)baw);
	baw->baw_sw.sw_Node.ln_Name = (char *)&(baw->baw_sw.sw_idstring);

	/* find the Default settings for this type of SuperWindow */
	wf = &glob.cg_WinDefs[SWTY_BLKANIM];

	/* attach it to the Project */
	baw->baw_Project = proj;

	baw->baw_NextBlkAnimWindow = proj->pj_BlkAnimWindows;
	proj->pj_BlkAnimWindows = baw;

	/* set up handler routines and assorted SuperWindow data */
	baw->baw_sw.sw_Handler = BlkAnimWindowHandler;
	baw->baw_sw.sw_HideWindow = HideBlkAnimWindow;
	baw->baw_sw.sw_ShowWindow = ShowBlkAnimWindow;
	baw->baw_sw.sw_BuggerOff = KillOffBlkAnimWindow;
	baw->baw_sw.sw_RefreshMenus = SetupBlkAnimWinMenus;
	baw->baw_sw.sw_Window = NULL;
	baw->baw_sw.sw_MenuStrip = NULL;
	baw->baw_sw.sw_GadList = NULL;
	baw->baw_sw.sw_Type = SWTY_BLKANIM;
	baw->baw_sw.sw_VisualInfo = NULL;

	/* set up stuff for opening the actual intuition window */
	baw->baw_sw.sw_Left = wf->wf_Left;
	baw->baw_sw.sw_Top = wf->wf_Top;
	baw->baw_sw.sw_Width = wf->wf_Width;
	baw->baw_sw.sw_Height = wf->wf_Height;
	baw->baw_sw.sw_MinWidth = 48;
	baw->baw_sw.sw_MaxWidth = 0xFFFF;
	baw->baw_sw.sw_MaxHeight = 0XFFFF;
	baw->baw_sw.sw_MenuStrip = NULL;

	baw->baw_sw.sw_FlgBackDrop = wf->wf_FlgBackDrop;
	baw->baw_sw.sw_FlgBorderless = wf->wf_FlgBorderless;
	baw->baw_sw.sw_FlgTitle = wf->wf_FlgTitle;
	baw->baw_sw.sw_FlgBorderGadgets = wf->wf_FlgBorderGadgets;

	/* setup SuperWindow basename */
	baw->baw_sw.sw_Title = "Block Anim";

	/* add window into the SuperWindows list */
	AddTail(&superwindows, &baw->baw_sw.sw_Node);

	/* open the intuition window (if the window doesn't open, */
	/* it is just left hidden - SEP). */
	ShowBlkAnimWindow(&baw->baw_sw);

	return(TRUE);
}



/************ ShowBlkAnimWindow ************/
//

static BOOL ShowBlkAnimWindow(struct SuperWindow *sw)
{
	struct BlkAnimWindow *baw;
	struct BlkAnim *ba;
	UWORD minh;
	ULONG flags;

	baw = (struct BlkAnimWindow *)sw;

	if(!sw->sw_Window)				/* make sure it isn't already open */
	{
		/* Set up the Visual Info for GadTools */
		if (!(baw->baw_sw.sw_VisualInfo = GetVisualInfo( mainscreen,TAG_END )))
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
		sw->sw_Window->UserData = (BYTE *)baw;

		SetupBlkAnimWinMenus( sw );

		/* we want to use our global message port for IDCMP stuff */
		sw->sw_Window->UserPort = mainmsgport;
		ModifyIDCMP(sw->sw_Window, IDCMP_CLOSEWINDOW | IDCMP_MENUPICK |
			IDCMP_NEWSIZE | IDCMP_REFRESHWINDOW | IDCMP_CHANGEWINDOW |
			IDCMP_MOUSEBUTTONS | IDCMP_RAWKEY |
			BUTTONIDCMP | TEXTIDCMP | NUMBERIDCMP | CYCLEIDCMP | SCROLLERIDCMP );

		/* setup other stuff */
		if( ba = (struct BlkAnim *)FindNthNode( &baw->baw_Project->pj_BlkAnims, 0 ) )
			baw->baw_CurrentAnimNum = 0;
		else
			baw->baw_CurrentAnimNum = -1;



		SussBawGadgets( baw );
	}
	return(TRUE);
}


/************  SetupBlkAnimWinMenus()  ************/
//

static BOOL SetupBlkAnimWinMenus( struct SuperWindow *sw )
{
	BOOL success = FALSE;

	if (sw->sw_MenuStrip)						/* Clear the menu strip if it exists */
	{
		ClearMenuStrip(sw->sw_Window);
		FreeMenus(sw->sw_MenuStrip);
		sw->sw_MenuStrip = NULL;
	}

	/* Set up some menus */
	SetupWindowMenu( sw, &blkanimmenu[MN_WINDOW_IDX] );
	SetupSettingsMenu( &blkanimmenu[MN_SETTINGS_IDX] );
	LinkInUserMenu( &blkanimmenu[MN_USER_IDX] );

	if( sw->sw_MenuStrip = CreateMenus( blkanimmenu, TAG_END ) )
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





/******************  HideBlkAnimWindow()  ******************/
//
// Hides the given BlkAnimWindow.
//

static void HideBlkAnimWindow(struct SuperWindow *sw)
{
	struct BlkAnimWindow *baw;

	baw = (struct BlkAnimWindow *)sw;

	if (sw->sw_Window)												/* make sure it's actually open */
	{
		if (sw->sw_MenuStrip)										/* Clear the menu strip if it exists */
		{
			ClearMenuStrip(sw->sw_Window);
			FreeMenus(sw->sw_MenuStrip);
			sw->sw_MenuStrip = NULL;
		}
		RememberWindow(sw);											/* save old window data */

		FreeBawGadgets( baw );

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


/******************  KillOffBlkAnimWindow()  ******************/
//
// Shuts down a BlkAnimWindow and removes it from the SuperWindows
// list.
//

static void KillOffBlkAnimWindow(struct SuperWindow *sw)
{
	struct BlkAnimWindow *baw;
	struct Project *proj;

	baw = (struct BlkAnimWindow *)sw;

	/* remove it from the project */

	if( proj = baw->baw_Project )
		UnlinkBlkAnimWindow(baw);

	/* close/free the SuperWindow */
	HideBlkAnimWindow(sw);											/* shut down the Intuition window */
	Remove(&(sw->sw_Node));											/* remove from SuperWindows list */
	FreeVec(baw);																/* free BlockWindow struct */
}

/******************  BlkAnimWindowHandler  ******************/
//
// Processes IDCMP messages coming into any BlockWindows,
// dispatching appropriate bits of code.
//

static void BlkAnimWindowHandler(struct SuperWindow *sw,
	struct IntuiMessage *imsg)
{
	struct BlkAnimWindow *baw;
	ULONG	class, seconds, micros;
	UWORD	code, qual;
	UWORD	mx,my;
	struct IntuiMessage *gtimsg;
	struct Gadget *gad;

	baw = (struct BlkAnimWindow *)sw;

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
				KillOffBlkAnimWindow(sw);
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
				HandleGadgetUp( (struct BlkAnimWindow *)sw, gad, code);
				break;
			case IDCMP_MENUPICK:
				/* menuroutine does the reply() */
				SortOutBlkAnimWindowMenus( sw,imsg,code );
				break;
			case IDCMP_NEWSIZE:
				/* resuss all the gadget placements/font stuff */
				/* according to the new windowsize. */
				ReplyMsg((struct Message *)imsg);
				SussBawGadgets( (struct BlkAnimWindow *)sw );
				break;
			case IDCMP_RAWKEY:
				ReplyMsg((struct Message *)imsg);
				code = HandleGlobalRawKey( sw, code, qual );
				break;
			case IDCMP_MOUSEBUTTONS:
				ReplyMsg((struct Message *)imsg);
				if( code == ( IECODE_LBUTTON|IECODE_UP_PREFIX) )
					if( DoubleClick( baw->baw_LastSeconds, baw->baw_LastMicros,
						seconds, micros ) )
					{
						HandleDoubleClick( (struct BlkAnimWindow *)sw, mx, my );
						baw->baw_LastSeconds = 0;
						baw->baw_LastMicros = 0;
					}
					else
					{
						HandleSingleClick( (struct BlkAnimWindow *)sw, mx, my );
						baw->baw_LastSeconds = seconds;
						baw->baw_LastMicros = micros;
					}
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


/************* SortOutBlkAnimWindowMenus() **************/
//
// Handle MENUPICK IDCMP messages - note the reply() is given
// here rather than in the main loop.
// Why is this so? Well... who can tell?
//

static void SortOutBlkAnimWindowMenus(struct SuperWindow *sw,
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



/**************** UnlinkBlkAnimWindow() ****************/
//
// Detaches a blkanimwindow from its parent project.
// (note that we assume that the window _is_ actually
// attached to a project in the first place).

static void UnlinkBlkAnimWindow(struct BlkAnimWindow *baw)
{
	struct BlkAnimWindow *prevwindow,*nextwindow;
	struct Project *proj;

	proj = baw->baw_Project;

	nextwindow = baw->baw_NextBlkAnimWindow;
	prevwindow = GetPrevBlkAnimWindow(baw);

	if (prevwindow == NULL)
		proj->pj_BlkAnimWindows = nextwindow;
	else
		prevwindow->baw_NextBlkAnimWindow = nextwindow;
}


/***************** GetPrevBlkAnimWindow() **************/
//
//  Find the prevous blkanimwindow connected to this project.
// If there is no previous window, then NULL is returned.

static struct BlkAnimWindow *GetPrevBlkAnimWindow(struct BlkAnimWindow *baw)
{
	struct BlkAnimWindow *prevwindow;
	struct Project *proj;

	proj = baw->baw_Project;
	prevwindow = proj->pj_BlkAnimWindows;

	if (prevwindow == baw)
		prevwindow = NULL;
	else
	{
		while (prevwindow->baw_NextBlkAnimWindow != baw)
			prevwindow = prevwindow->baw_NextBlkAnimWindow;
	}
	return(prevwindow);
}



/***************** SussBawGadgets() **************/

static void SussBawGadgets( struct BlkAnimWindow *baw )
{
	struct Window *win;
	struct Gadget *gad;
	struct NewGadget ng;
	struct BlkAnim	*ba;
	struct TextFont *tf = windowtextfont;
	UWORD	th = windowtextfont->tf_YSize;
	UWORD hspace = 4;
	BOOL	noanim, disable;

	static STRPTR lengthlabels[] =
		{ "2", "4", "8", "16", "32", "64", "128", "256", NULL };

	printf( "\nSussBawGadgets():\n  baw->baw_CurrentAnimNum = %d\n",
		baw->baw_CurrentAnimNum );

	if( baw->baw_CurrentAnimNum == -1)
		noanim = TRUE;
	else
		noanim = FALSE;

	if(	baw->baw_sw.sw_GadList )
		FreeBawGadgets( baw );

	win = baw->baw_sw.sw_Window;
	ClearWindow( win );

	if( baw->baw_CurrentAnimNum != -1 )
	{
		ba = (struct BlkAnim *)FindNthNode( &baw->baw_Project->pj_BlkAnims,
		baw->baw_CurrentAnimNum );
		baw->baw_CurrentAnim = ba;

		printf(" ba: %ld\n",ba);
		printf(" ba_Length: %d\n", ba->ba_Length );
		printf(" baw_CurrentAnimNum: %d\n", baw->baw_CurrentAnimNum );
	}
	else
		baw->baw_CurrentAnim = ba = NULL;

	gad = CreateContext( &baw->baw_sw.sw_GadList );

	/* const values for all the gadgets */
	ng.ng_TextAttr = &windowtextattr;
	ng.ng_VisualInfo = baw->baw_sw.sw_VisualInfo;

	ng.ng_LeftEdge = win->BorderLeft + hspace;
	ng.ng_TopEdge = win->BorderTop + 4;
	ng.ng_Height = th + 4;
	ng.ng_GadgetText = "-";
	ng.ng_Width = MyTextLength( tf, ng.ng_GadgetText ) + 8;
	ng.ng_GadgetID = GAD_MINUS;
	ng.ng_Flags = 0;
	if( baw->baw_CurrentAnimNum > 0 )
		gad = CreateGadget( BUTTON_KIND, gad, &ng,
			GA_Disabled, noanim, TAG_END );
	else
		gad = CreateGadget( BUTTON_KIND, gad, &ng,
			GA_Disabled, TRUE, TAG_END );

	ng.ng_LeftEdge += ( ng.ng_Width );
	ng.ng_Height = th + 4;
	ng.ng_GadgetText = NULL;
	ng.ng_Width = MyTextLength( tf, "0000" ) + 8;
	ng.ng_GadgetID = GAD_ANIMNUM;
	ng.ng_Flags = PLACETEXT_ABOVE;

	if( baw->baw_CurrentAnimNum != -1 )
		gad = CreateGadget( NUMBER_KIND, gad, &ng,
			GTNM_Number, baw->baw_CurrentAnimNum,
			GTNM_Justification, GTJ_CENTER,
			GTNM_Border, TRUE, TAG_END );
	else
		gad = CreateGadget( NUMBER_KIND, gad, &ng,
			GTNM_Border, TRUE, TAG_END );

	/* see if there's a next blkanim */
	if( baw->baw_CurrentAnimNum != -1 &&
		baw->baw_CurrentAnimNum + 1 < baw->baw_Project->pj_NumOfBlkAnims )
		disable = FALSE;
	else
		disable = TRUE;

	ng.ng_LeftEdge += ( ng.ng_Width );
	ng.ng_Height = th + 4;
	ng.ng_GadgetText = "+";
	ng.ng_Width = MyTextLength( tf, ng.ng_GadgetText ) + 8;
	ng.ng_GadgetID = GAD_PLUS;
	ng.ng_Flags = 0;
	gad = CreateGadget( BUTTON_KIND, gad, &ng,
		GA_Disabled, noanim|disable, TAG_END );

	ng.ng_LeftEdge += ( ng.ng_Width + hspace );
	ng.ng_Height = th + 4;
	ng.ng_GadgetText = "New";
	ng.ng_Width = MyTextLength( tf, ng.ng_GadgetText ) + 8;
	ng.ng_GadgetID = GAD_NEWANIM;
	ng.ng_Flags = 0;
	gad = CreateGadget( BUTTON_KIND, gad, &ng, TAG_END );

	ng.ng_LeftEdge += ( ng.ng_Width + hspace );
	ng.ng_Height = th + 4;
	ng.ng_GadgetText = "Delete";
	ng.ng_Width = MyTextLength( tf, ng.ng_GadgetText ) + 8;
	ng.ng_GadgetID = GAD_DELANIM;
	ng.ng_Flags = 0;
	gad = CreateGadget( BUTTON_KIND, gad, &ng,
			GA_Disabled, noanim, TAG_END );

	/* 2nd row */

	ng.ng_TopEdge += ( th + 4*3 );
	ng.ng_Height = th + 4;
	ng.ng_GadgetText = "Length:";
	ng.ng_LeftEdge = win->BorderLeft + hspace +
		MyTextLength( tf, ng.ng_GadgetText ) + 8;
	ng.ng_Width = MyTextLength( tf, "0000" ) + 32;
	ng.ng_GadgetID = GAD_LENGTH;
	ng.ng_Flags = 0;
	gad = CreateGadget( CYCLE_KIND, gad, &ng,
		GTCY_Labels, lengthlabels,
		GTCY_Active, ba ? ba->ba_Length : 0,
		GA_Disabled, noanim,
		TAG_END );

	/* 3rd row */

	ng.ng_TopEdge += ( th + 4*3 );
	ng.ng_Height = th + 4;
	ng.ng_GadgetText = "Pos:";
	ng.ng_LeftEdge = win->BorderLeft + hspace +
		MyTextLength( tf, ng.ng_GadgetText ) + 8;
	ng.ng_Width = MyTextLength( tf, "000" ) + 8;
	ng.ng_GadgetID = GAD_POS;
	ng.ng_Flags = PLACETEXT_LEFT;
	gad = CreateGadget( NUMBER_KIND, gad, &ng,
		GTNM_Number, 42,
		GTNM_Justification, GTJ_CENTER,
		GTNM_Border, TRUE, TAG_END );

	ng.ng_GadgetText = "Block:";
	ng.ng_LeftEdge += ng.ng_Width +
		MyTextLength( tf, ng.ng_GadgetText ) + 8 + hspace;
	ng.ng_Width = MyTextLength( tf, "0000" ) + 8;
	ng.ng_GadgetID = GAD_BLOCK;
	ng.ng_Flags = PLACETEXT_LEFT;
	gad = CreateGadget( NUMBER_KIND, gad, &ng,
		GTNM_Number, 42,
		GTNM_Justification, GTJ_CENTER,
		GTNM_Border, TRUE, TAG_END );

	/* setup for 4th row: the BlkView thingy */

	baw->baw_BlkViewTopEdge = ng.ng_TopEdge + th + 8 ;
	baw->baw_BlkViewWidth =
		( win->Width - win->BorderLeft - win->BorderRight - 4 ) / ( BLKW + 2 );

	/* 5th row */
	ng.ng_LeftEdge = win->BorderLeft;
	ng.ng_TopEdge = baw->baw_BlkViewTopEdge + BLKH + 4;
	ng.ng_Width = ( baw->baw_BlkViewWidth * ( BLKW + 2 ) ) + 4;
	ng.ng_Height = th + 4;
	ng.ng_GadgetText = NULL;
	ng.ng_GadgetID = GAD_SCROLLER;
	ng.ng_Flags = 0;

	if(ba)
	{
		gad = CreateGadget( SCROLLER_KIND, gad, &ng,
			GA_Disabled, noanim,
			GA_RelVerify, TRUE,
			GTSC_Top, baw->baw_BlkViewLeft,
			GTSC_Total, powers_of_two[ ba->ba_Length + 1 ],
			GTSC_Arrows, 16,
			GTSC_Visible, baw->baw_BlkViewWidth, TAG_END );
	}
	else
	{
		gad = CreateGadget( SCROLLER_KIND, gad, &ng,
			GA_Disabled, noanim,
//			GTSC_Top, baw->baw_BlkViewLeft,
//			GTSC_Total, powers_of_two[ ba->ba_Length + 1 ],
			GTSC_Arrows, 16, TAG_END );
//			GTSC_Visible, baw->baw_BlkViewWidth, TAG_END );
	}

	/* 6th row */

	ng.ng_LeftEdge = win->BorderLeft + hspace;
	ng.ng_TopEdge += ng.ng_Height + 4;
	ng.ng_Height = th + 4;
	ng.ng_GadgetText = "Put";
	ng.ng_Width = MyTextLength( tf, ng.ng_GadgetText ) + 8;
	ng.ng_GadgetID = GAD_PUTBLK;
	ng.ng_Flags = 0;
	gad = CreateGadget( BUTTON_KIND, gad, &ng,
		GA_Disabled, noanim, TAG_END );


	/* 7th row */

	ng.ng_LeftEdge = win->BorderLeft + hspace;
	ng.ng_TopEdge += ng.ng_Height + 4;
	ng.ng_Height = th + 4;
	ng.ng_GadgetText = "Insert";
	ng.ng_Width = MyTextLength( tf, ng.ng_GadgetText ) + 8;
	ng.ng_GadgetID = GAD_INSBLK;
	ng.ng_Flags = 0;
	gad = CreateGadget( BUTTON_KIND, gad, &ng,
		GA_Disabled, noanim, TAG_END );

	/* 8th row */

	ng.ng_LeftEdge = win->BorderLeft + hspace;
	ng.ng_TopEdge += ng.ng_Height + 4;
	ng.ng_Height = th + 4;
	ng.ng_GadgetText = "Delete";
	ng.ng_Width = MyTextLength( tf, ng.ng_GadgetText ) + 8;
	ng.ng_GadgetID = GAD_DELBLK;
	ng.ng_Flags = 0;
	gad = CreateGadget( BUTTON_KIND, gad, &ng,
		GA_Disabled, noanim, TAG_END );


	/* 9th row */

	ng.ng_LeftEdge = win->BorderLeft + hspace;
	ng.ng_TopEdge += ng.ng_Height + 4;
	ng.ng_Height = th + 4;
	ng.ng_GadgetText = "Undo";
	ng.ng_Width = MyTextLength( tf, ng.ng_GadgetText ) + 8;
	ng.ng_GadgetID = GAD_UNDO;
	ng.ng_Flags = 0;
	gad = CreateGadget( BUTTON_KIND, gad, &ng,
		GA_Disabled, noanim, TAG_END );




	if( !gad )
	{
		FreeBawGadgets( baw );
		return;
	}

	DrawBevelBox( win->RPort,
		win->BorderLeft,
		baw->baw_BlkViewTopEdge,
		( baw->baw_BlkViewWidth * ( BLKW + 2 ) ) + 4 + 2,
		BLKH + 4,
		GT_VisualInfo, baw->baw_sw.sw_VisualInfo,
		TAG_END );

	DrawBlkView( baw );

	AddGList(win, baw->baw_sw.sw_GadList, -1, -1, NULL);
	GT_RefreshWindow( win, NULL );
	/* need to redraw the frame coz we're not using GimmeZeroZero */
	RefreshWindowFrame( win );

}

/***************** FreeBawGadgets() **************/

static void FreeBawGadgets( struct BlkAnimWindow *baw )
{
	/* kill any old gadgets that may be active */
	if(baw->baw_sw.sw_GadList)
	{
		RemoveGList(baw->baw_sw.sw_Window, baw->baw_sw.sw_GadList, -1);
		FreeGadgets(baw->baw_sw.sw_GadList);
		baw->baw_sw.sw_GadList = NULL;
	}
}


/***************** DrawBlkView() *****************/

static void DrawBlkView( struct BlkAnimWindow *baw )
{
	LONG x,y;
	UWORD i,b;
	struct Window *win;
	struct BlkAnim *ba;

	if( baw->baw_CurrentAnimNum != -1 && baw->baw_CurrentAnim)
	{
		win = baw->baw_sw.sw_Window;
		x = 4 + win->BorderLeft;
		y = baw->baw_BlkViewTopEdge + 2;
		ba = baw->baw_CurrentAnim;
		b = baw->baw_BlkViewLeft;

		SetAPen( win->RPort, 0 );
		RectFill( win->RPort,
			win->BorderLeft + 2,
			baw->baw_BlkViewTopEdge + 1,
			win->BorderLeft + ( ( BLKW + 2 ) * baw->baw_BlkViewWidth ) + 2 + 1,
			baw->baw_BlkViewTopEdge + 1 + BLKH + 1 );

		for( i=0; i < baw->baw_BlkViewWidth; i++ )
		{
			if( b < powers_of_two[ ba->ba_Length + 1] )
			{
				if( b == baw->baw_SelectedBlk )
					DrawBevelBox( win->RPort, x-2, y-1, BLKW + 4, BLKH + 2,
						GTBB_Recessed, TRUE,
						GT_VisualInfo, baw->baw_sw.sw_VisualInfo,
						TAG_END );
				BltBlockWindow( win, x, y, ba->ba_Data[ b++ ] );
				x = x + BLKW + 2;
			}
		}
	}
}


/*****************  HandleGadgetUp() *****************/

static void HandleGadgetUp( struct BlkAnimWindow *baw, struct Gadget *gad, UWORD code )
{
	struct BlkAnim *ba;
	struct Project *proj;

	proj = baw->baw_Project;

	switch( gad->GadgetID )
	{
		case GAD_NEWANIM:
			if( ba = CreateNewBlkAnim( 5 ) )
			{
				AddTail( &proj->pj_BlkAnims, (struct Node *)ba );
				proj->pj_NumOfBlkAnims++;
				baw->baw_CurrentAnimNum = proj->pj_NumOfBlkAnims - 1;

				SussBawGadgets( baw );
			}
			break;
		case GAD_MINUS:
			if( baw->baw_CurrentAnimNum > 0 )
				baw->baw_CurrentAnimNum--;
				SussBawGadgets( baw );
			break;
		case GAD_PLUS:
			if( baw->baw_CurrentAnimNum+1 < proj->pj_NumOfBlkAnims )
				baw->baw_CurrentAnimNum++;
				SussBawGadgets( baw );
			break;
		case GAD_LENGTH:
			if( baw->baw_CurrentAnim )
				baw->baw_CurrentAnim->ba_Length = code;
				SussBawGadgets( baw );
			break;
		case GAD_SCROLLER:
//			if( baw->baw_CurrentAnim )
			printf(" GAD_SCROLLER\n");
			baw->baw_BlkViewLeft = code;
				//SussBawGadgets( baw );
			DrawBlkView( baw );
			break;
	}
}

/*****************  HandleSingleClick() *****************/

void HandleSingleClick( struct BlkAnimWindow *baw, WORD x, WORD y )
{
	struct Window *win;

	win = baw->baw_sw.sw_Window;

	if( y < baw->baw_BlkViewTopEdge || y > baw->baw_BlkViewTopEdge+BLKH+2 ) return;
	if( x < win->BorderLeft ) return;
	if( x > win->Width - win->BorderRight ) return;

	if( baw->baw_CurrentAnimNum != -1 && baw->baw_CurrentAnim )
	{
		x -= ( win->BorderLeft + 2 );
		x = x / (BLKW+2);
		x += baw->baw_BlkViewLeft;

		if( x < powers_of_two[ baw->baw_CurrentAnim->ba_Length + 1 ] )
		{
			baw->baw_SelectedBlk = x;
			DrawBlkView( baw );
			//SussBawGadgets( baw );
		}
	}
}

/*****************  HandleDoubleClick() *****************/

void HandleDoubleClick( struct BlkAnimWindow *baw, WORD x, WORD y )
{
	struct Window *win;
	WORD	oldsel;

	win = baw->baw_sw.sw_Window;

	if( y < baw->baw_BlkViewTopEdge || y > baw->baw_BlkViewTopEdge+BLKH+2 ) return;
	if( x < win->BorderLeft ) return;
	if( x > win->Width - win->BorderRight ) return;

	if( baw->baw_CurrentAnimNum != -1 && baw->baw_CurrentAnim )
	{
		x -= ( win->BorderLeft + 2 );
		x = x / (BLKW+2);
		x += baw->baw_BlkViewLeft;

		if( x < powers_of_two[ baw->baw_CurrentAnim->ba_Length + 1 ] )
		{
			oldsel = baw->baw_SelectedBlk;
			baw->baw_SelectedBlk = x;
			if( x==oldsel )
				baw->baw_CurrentAnim->ba_Data[x] = lmbblk;
			DrawBlkView( baw );
			//SussBawGadgets( baw );
		}
	}
}
