/****************************************************************************/
//
// BONK: BlockWindow.c
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

static struct NewMenu	blockmenu[] =
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

		{ NM_TITLE,	"Blockset",							0,	0,	0,	0,},
		{  NM_ITEM,	"Rename...",						0,	0,	0,	0,},

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
#define MN_BLOCKSET					2
#define MN_WINDOW						3
#define MN_SETTINGS					4
#define MN_USER							5

#define MN_EDIT_IDX					13
#define MN_BLOCKSET_IDX			18
#define MN_WINDOW_IDX				20
#define MN_SETTINGS_IDX			39
#define MN_USER_IDX					47

//#define MN_WINDOW_IDX				18
//#define MN_SETTINGS_IDX			37
//#define MN_USER_IDX					45

/************ Prototypes for functions local to this file ************/

static BOOL ShowBlockWindow(struct SuperWindow *sw);
static BOOL SetupBlockWinMenus( struct SuperWindow *sw );
static void HideBlockWindow(struct SuperWindow *sw);
static void KillOffBlockWindow(struct SuperWindow *sw);
static void BlockWindowHandler(struct SuperWindow *sw,	struct IntuiMessage *imsg);
static void DisplayBlocks(struct BlockWindow *bw);
static void ScrollUp(struct BlockWindow *bw, WORD dist);
static void ScrollDown(struct BlockWindow *bw, WORD dist);
static void ScrollLeft(struct BlockWindow *bw, WORD dist);
static void ScrollRight(struct BlockWindow *bw, WORD dist);
static void SortOutBlockWindowMenus(struct SuperWindow *sw,
	struct IntuiMessage *imsg, UWORD code);
static UWORD PickUpBlock(struct BlockWindow *bw, WORD blkx, WORD blky );
static void BW_DrawSelector(struct BlockWindow *bw, WORD blkx, WORD blky );
static void BW_EraseSelector(struct BlockWindow *bw );
static void MouseDown( struct BlockWindow *bw, WORD x, WORD y, WORD button);
static void MouseUp( struct BlockWindow *bw, WORD x, WORD y, WORD button);
static void MouseMove( struct BlockWindow *bw, WORD x, WORD y );
static void RenameBlockset( struct SuperWindow *sw );


/************ OpenNewBlockWindow ************/
//
// Opens a BlockWindow. The window displays a list of all the
// currently hidden SuperWindows or Projects, allowing the user
// to make them reappear.
//

BOOL OpenNewBlockWindow(void)
{

	struct	BlockWindow	*bw;
	struct	WindowDefault	*wf;

	struct SuperWindow *sw;

	/* only one blockwindow */

	for( sw = (struct SuperWindow *)superwindows.lh_Head;
		sw->sw_Node.ln_Succ;
		sw = (struct SuperWindow *)sw->sw_Node.ln_Succ )
	{
		if( sw->sw_Type == SWTY_BLOCK && sw->sw_Window )
		{
			WindowToFront( sw->sw_Window );
			ActivateWindow( sw->sw_Window );
			return TRUE;
		}
	}




	/* grab mem for the BlockWindow struct */
	if ( !( bw= AllocVec(sizeof(struct BlockWindow), MEMF_CLEAR ) ) )
		return(FALSE);

	/* figure out a unique ID for this window (mainly for AREXX use) */
	InstallUniqueWindowID((struct SuperWindow *)bw);
	bw->bw_sw.sw_Node.ln_Name = (char *)&(bw->bw_sw.sw_idstring);

	/* find the Default settings for this type of SuperWindow */
	wf = &glob.cg_WinDefs[SWTY_BLOCK];

	/* set up handler routines and assorted SuperWindow data */
	bw->bw_sw.sw_Handler = BlockWindowHandler;
	bw->bw_sw.sw_HideWindow = HideBlockWindow;
	bw->bw_sw.sw_ShowWindow = ShowBlockWindow;
	bw->bw_sw.sw_BuggerOff = KillOffBlockWindow;
	bw->bw_sw.sw_RefreshMenus = SetupBlockWinMenus;
	bw->bw_sw.sw_Window = NULL;
	bw->bw_sw.sw_MenuStrip = NULL;
	bw->bw_sw.sw_GadList = NULL;
	bw->bw_sw.sw_Type = SWTY_BLOCK;
	bw->bw_sw.sw_VisualInfo = NULL;

	/* set up stuff for opening the actual intuition window */
	bw->bw_sw.sw_Left = wf->wf_Left;
	bw->bw_sw.sw_Top = wf->wf_Top;
	bw->bw_sw.sw_Width = wf->wf_Width;
	bw->bw_sw.sw_Height = wf->wf_Height;
	bw->bw_sw.sw_MinWidth = 100;					/* minwidth set for topaz8 */
	bw->bw_sw.sw_MaxWidth = 0xFFFF;
	bw->bw_sw.sw_MaxHeight = 0XFFFF;
	bw->bw_sw.sw_MenuStrip = NULL;

	bw->bw_sw.sw_FlgBackDrop = wf->wf_FlgBackDrop;
	bw->bw_sw.sw_FlgBorderless = wf->wf_FlgBorderless;
	bw->bw_sw.sw_FlgTitle = wf->wf_FlgTitle;
	bw->bw_sw.sw_FlgBorderGadgets = wf->wf_FlgBorderGadgets;

	/* setup SuperWindow basename */
	bw->bw_sw.sw_Title = "BlockSet";

	/* add window into the SuperWindows list */
	AddTail(&superwindows, &bw->bw_sw.sw_Node);

	/* open the intuition window (if the window doesn't open, */
	/* it is just left hidden - SEP). */
	ShowBlockWindow(&bw->bw_sw);

	return(TRUE);
}



/************ ShowBlockWindow ************/
//

static BOOL ShowBlockWindow(struct SuperWindow *sw)
{
	struct BlockWindow *bw;
	UWORD minh;
	ULONG flags;

	bw = (struct BlockWindow *)sw;

	if(!sw->sw_Window)				/* make sure it isn't already open */
	{
		/* Set up the Visual Info for GadTools */
		if (!(bw->bw_sw.sw_VisualInfo = GetVisualInfo(MAINSCREEN,TAG_END)))
			return(FALSE);

		minh = mainscreen->Font->ta_YSize + 32 +
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
											WA_CustomScreen,	mainscreen,
											WA_AutoAdjust,	TRUE,
											WA_ReportMouse,	TRUE,
											WA_Title,				sw->sw_FlgTitle ?
												BuildWindowTitle(sw) : NULL,
											TAG_DONE,				NULL);

		if( !(sw->sw_Window)) return(FALSE);

		/* Tell the window which SuperWindow it is attached to */
		/* (so we'll know which handler routine to call) */
		sw->sw_Window->UserData = (BYTE *)bw;

		/* Set up and attach menus */
		SetupBlockWinMenus( sw );

		/* we want to use our global message port for IDCMP stuff */
		sw->sw_Window->UserPort = mainmsgport;
		ModifyIDCMP(sw->sw_Window, IDCMP_CLOSEWINDOW | IDCMP_MENUPICK |
			IDCMP_NEWSIZE | IDCMP_RAWKEY| IDCMP_REFRESHWINDOW |
			IDCMP_MENUVERIFY | IDCMP_MOUSEBUTTONS | IDCMP_MOUSEMOVE |
			IDCMP_CHANGEWINDOW | IDCMP_INACTIVEWINDOW | IDCMP_ACTIVEWINDOW );

		DisplayBlocks(bw);
	}

	return(TRUE);
}




/************  SetupBlockWinMenus()  ************/
//

static BOOL SetupBlockWinMenus( struct SuperWindow *sw )
{
	BOOL success = FALSE;

	if (sw->sw_MenuStrip)						/* Clear the menu strip if it exists */
	{
		ClearMenuStrip(sw->sw_Window);
		FreeMenus(sw->sw_MenuStrip);
		sw->sw_MenuStrip = NULL;
	}

	/* Set up some menus */
	SetupWindowMenu( sw, &blockmenu[MN_WINDOW_IDX] );
	SetupSettingsMenu( &blockmenu[MN_SETTINGS_IDX] );
	LinkInUserMenu( &blockmenu[MN_USER_IDX] );

	if( sw->sw_MenuStrip = CreateMenus( blockmenu, TAG_END ) )
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




/******************  HideBlockWindow()  ******************/
//
// Hides the given BlockWindow.
//

static void HideBlockWindow(struct SuperWindow *sw)
{
	struct BlockWindow *bw;

	bw = (struct BlockWindow *)sw;

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


/******************  KillOffBlockWindow()  ******************/
//
// Shuts down a BlockWindow and removes it from the SuperWindows
// list.
//

static void KillOffBlockWindow(struct SuperWindow *sw)
{
	struct BlockWindow *bw;

	bw = (struct BlockWindow *)sw;

	/* close/free the SuperWindow */
	HideBlockWindow(sw);												/* shut down the Intuition window */
	Remove(&(sw->sw_Node));											/* remove from SuperWindows list */
	FreeVec(bw);																/* free BlockWindow struct */
}

/******************  BlockWindowHandler  ******************/
//
// Processes IDCMP messages coming into any BlockWindows,
// dispatching appropriate bits of code.
//

static void BlockWindowHandler(struct SuperWindow *sw,
	struct IntuiMessage *imsg)
{
	ULONG	class;
	UWORD	code, qual;
	WORD	blkx, blky, mx, my;

	struct IntuiMessage *gtimsg;
	struct Gadget *gad;
	struct BlockWindow *bw;
	struct Window *win;

	bw = (struct BlockWindow *)sw;
	if( gtimsg = GT_FilterIMsg(imsg) )
	{

		/* read out all we need */
		class = gtimsg->Class;
		code = gtimsg->Code;
		qual = gtimsg->Qualifier;
		gad = (struct Gadget *)gtimsg->IAddress;
		mx = gtimsg->MouseX;
		my = gtimsg->MouseY;

		GT_PostFilterIMsg(gtimsg);

		win = bw->bw_sw.sw_Window;

		if( (blkx = mx	- (WORD)win->BorderLeft) < 0 )
			blkx -= 16;
		blkx /= BLKW;

		if( (blky = my - (WORD)win->BorderTop) < 0 )
			blky -= 16;
		blky /= BLKH;

		switch(class)
		{
			case IDCMP_CLOSEWINDOW:
				/* tell this window to bugger off */
				ReplyMsg((struct Message *)imsg);
				KillOffBlockWindow(sw);
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
				SortOutBlockWindowMenus( sw,imsg,code );
				break;
			case IDCMP_NEWSIZE:
				/* resuss all the gadget placements/font stuff */
				/* according to the new windowsize. */
//				SetupRevealGadgets((struct BlockWindow *)sw);
				ReplyMsg((struct Message *)imsg);
				BW_EraseSelector( bw );
				DisplayBlocks(bw);
				break;
			case IDCMP_CHANGEWINDOW:
				ReplyMsg((struct Message *)imsg);
				MakeDefault( sw );
				break;
			case IDCMP_RAWKEY:
				ReplyMsg((struct Message *)imsg);
				BW_EraseSelector( bw );

				code = HandleGlobalRawKey( sw, code, qual );

				switch (code)
				{
					case 0x40:		/* Spacebar */
						if( prevprojwin )
						{
							WindowToBack( bw->bw_sw.sw_Window );
							ActivateWindow( prevprojwin->pw_sw.sw_Window);
						}
						break;
					case 0x4C:		/* cursor up */
						if( qual & IEQUALIFIER_RALT )
							ScrollUp( bw, 4 );
						else
							ScrollUp( bw, 1 );
						break;
					case 0x4D:		/* cursor down */
						if( qual & IEQUALIFIER_RALT )
							ScrollDown( bw, 4 );
						else
							ScrollDown( bw, 1 );
						break;
					case 0x4F:		/* cursor left */
						if( qual & IEQUALIFIER_RALT )
							ScrollLeft( bw, 4 );
						else
							ScrollLeft( bw, 1 );
						break;
					case 0x4E:		/* cursor right */
						if( qual & IEQUALIFIER_RALT )
							ScrollRight( bw, 4 );
						else
							ScrollRight( bw, 1 );
						break;
				}
				break;

			case IDCMP_MOUSEMOVE:
				ReplyMsg((struct Message *)imsg);
				blkx = ClipToWindowBlkX(bw->bw_sw.sw_Window, blkx );
				blky = ClipToWindowBlkY(bw->bw_sw.sw_Window, blky );
				MouseMove( bw, blkx, blky );
				break;

			case IDCMP_MOUSEBUTTONS:
				ReplyMsg((struct Message *)imsg);
				if( ( code & IECODE_UP_PREFIX ) )
				{
					MouseUp( bw, blkx, blky, code & ~IECODE_UP_PREFIX );
				}
				else
					MouseDown( bw, blkx, blky, code );
				break;
			case IDCMP_MENUVERIFY:
				if(code == MENUHOT)
				{
					/* the menu event came from this window, so we can cancel it */
					/* if we feel the inclination. */

					/* check mouse position against window boundaries */
					if( (imsg->MouseX > 0) && (imsg->MouseX < win->Width) &&
						(imsg->MouseY > 0) && (imsg->MouseY < win->Height) )
					{
						/* RMB down within window - cancel the menu, */
						/* and send the event on as a SELECTDOWN */
						imsg->Code = MENUCANCEL;
						ReplyMsg((struct Message *)imsg);
						//Tool_Down(pw, blkx, blky, IECODE_RBUTTON);
						MouseDown( bw, blkx, blky, IECODE_RBUTTON );
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
				break;

			case IDCMP_INACTIVEWINDOW:
				ReplyMsg((struct Message *)imsg);
				BW_EraseSelector( bw );
				break;
			case IDCMP_ACTIVEWINDOW:
				MakeDefault( sw );
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


/************* SortOutBlockWindowMenus() **************/
//
// Handle MENUPICK IDCMP messages - note the reply() is given
// here rather than in the main loop.
// Why is this so? Well... who can tell?
//

static void SortOutBlockWindowMenus(struct SuperWindow *sw,
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


	/* Go through our little array of menunumbers and process them */
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
			case MN_BLOCKSET:
				if( itemnum == 0 )		/* rename blockset? */
					RenameBlockset( sw );
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


void RedrawAllBlockWindows( void )
{
	struct SuperWindow *sw;

	for( sw = (struct SuperWindow *)superwindows.lh_Head;
		sw->sw_Node.ln_Succ;
		sw = (struct SuperWindow *)sw->sw_Node.ln_Succ )
	{
		if( sw->sw_Type == SWTY_BLOCK )
			DisplayBlocks( (struct BlockWindow *)sw );
	}
}



static void DisplayBlocks(struct BlockWindow *bw)
{
	WORD x, y;
	UWORD num, blk, width, height;
	struct Window *win;

	if( win = bw->bw_sw.sw_Window )
	{
		width = ( win->Width - win->BorderLeft - win->BorderRight ) / BLKW;
		height = ( win->Height - win->BorderTop - win->BorderBottom ) / BLKH;

		blk = 0;
		num = globlocks->bs_NumOfBlocks;
		y=0;
		while( y <= height )
		{
			x=0;
			while( x <= width )
			{
				blk = ( ( bw->bw_WinY + y ) *globlocks->bs_LayoutWidth ) + x + bw->bw_WinX;
				if( x+bw->bw_WinX < globlocks->bs_LayoutWidth && blk < num )
					DrawBlock( win, x, y, blk);
				else
					DrawBlock( win, x, y, 0);
				x++;
			}
			y++;
		}

		/* do grid */

		SetDrMd( win->RPort, COMPLEMENT );
		for( y=0; y<height; y++ )
		{
			for( x=0; x<width; x++ )
			{
				WritePixel( win->RPort, win->BorderLeft + x*BLKW, win->BorderTop + y*BLKH);
				WritePixel( win->RPort, win->BorderLeft + x*BLKW +15, win->BorderTop + y*BLKH);
				WritePixel( win->RPort, win->BorderLeft + x*BLKW, win->BorderTop + y*BLKH + 15);
				WritePixel( win->RPort, win->BorderLeft + x*BLKW + 15, win->BorderTop + y*BLKH + 15);
			}
		}
	}
}


static void ScrollUp(struct BlockWindow *bw, WORD dist)
{
	bw->bw_WinY -= dist;
	if( bw->bw_WinY < 0 ) bw->bw_WinY = 0;
	DisplayBlocks(bw);
}

static void ScrollDown(struct BlockWindow *bw, WORD dist)
{
	WORD h;
	UWORD bh;
	struct Window *win;

	win = bw->bw_sw.sw_Window;
	h = ( ( win->Height - win->BorderTop - win->BorderBottom ) / BLKH );
	bh = globlocks->bs_NumOfBlocks / globlocks->bs_LayoutWidth + 1;
	bw->bw_WinY += dist;
	if( ( bw->bw_WinY + h ) > bh-1 )
		bw->bw_WinY = bh - h;
	if( bw->bw_WinY < 0 ) bw->bw_WinY=0;
	DisplayBlocks(bw);
}

static void ScrollLeft(struct BlockWindow *bw, WORD dist)
{
	bw->bw_WinX -= dist;
	if( bw->bw_WinX < 0 ) bw->bw_WinX = 0;
	DisplayBlocks(bw);
}

static void ScrollRight(struct BlockWindow *bw, WORD dist)
{
	WORD w;
	struct Window *win;

	win = bw->bw_sw.sw_Window;
	w = (( win->Width - win->BorderLeft - win->BorderRight ) / BLKW);
	bw->bw_WinX += dist;
	if( ( bw->bw_WinX + w ) > globlocks->bs_LayoutWidth-1 )
		bw->bw_WinX = globlocks->bs_LayoutWidth - w;

	if( bw->bw_WinX < 0 ) bw->bw_WinX=0;
	DisplayBlocks(bw);
}


static UWORD PickUpBlock(struct BlockWindow *bw, WORD x, WORD y )
{
	UWORD blk;

	blk = ( y * globlocks->bs_LayoutWidth ) + x;

	if( x >= 0 && x < globlocks->bs_LayoutWidth && blk < globlocks->bs_NumOfBlocks )
		return( blk );
	else
		return( (WORD)~0 );
}


static void BW_DrawSelector(struct BlockWindow *bw, WORD blkx, WORD blky )
{
	WORD x0, y0, blockswide, blockshigh;
	struct Window *win;

	win = bw->bw_sw.sw_Window;
	/* suss out window dimensions */
	blockswide = ( win->Width - win->BorderLeft - win->BorderRight ) / BLKW;
	blockshigh = ( win->Height - win->BorderTop - win->BorderBottom ) / BLKH;

	if(bw->bw_DragFlag)
	{
		x0 = bw->bw_AnchorX - bw->bw_WinX;
		y0 = bw->bw_AnchorY - bw->bw_WinY;
	}
	else
	{
		x0 = blkx;
		y0 = blky;
	}

	if( x0<0 ) x0 = 0;
	if( y0<0 ) y0 = 0;
	if( x0>blockswide ) x0 = blockswide;
	if( y0>blockshigh ) y0 = blockshigh;

	bw->bw_SelX0 = x0;
	bw->bw_SelY0 = y0;
	bw->bw_SelX1 = blkx;
	bw->bw_SelY1 = blky;

	SetDrMd( bw->bw_sw.sw_Window->RPort, COMPLEMENT);
	BlkRectFill( bw->bw_sw.sw_Window, x0, y0, blkx, blky );
	bw->bw_SelState = TRUE;
}


static void BW_EraseSelector(struct BlockWindow *bw )
{
	if( bw->bw_SelState )
	{
		SetDrMd( bw->bw_sw.sw_Window->RPort, COMPLEMENT);
		BlkRectFill( bw->bw_sw.sw_Window, bw->bw_SelX0, bw->bw_SelY0,
			bw->bw_SelX1, bw->bw_SelY1 );
		bw->bw_SelState = FALSE;
	}
}


void OldBlockWindowToFront( void )
{
	struct SuperWindow *sw;

	sw = (struct SuperWindow *)superwindows.lh_Head;
	while( sw->sw_Node.ln_Succ )
	{
		if( sw->sw_Type == SWTY_BLOCK && sw->sw_Window )
		{
			WindowToFront( sw->sw_Window );
			ActivateWindow( sw->sw_Window );
		}
		sw = (struct SuperWindow *)sw->sw_Node.ln_Succ;
	}
}


void BlockWindowToFront( void )
{
	struct SuperWindow *sw;

	sw = FindSuperWindowByType( SWTY_BLOCK );

	if(sw)
	{
		if( sw->sw_Window )
		{
			WindowToFront( sw->sw_Window );
			ActivateWindow( sw->sw_Window );
		}
		else
		{
			(*sw->sw_ShowWindow)(sw);
			RV_RemakeAll();
		}
	}
	else
		OpenNewBlockWindow();
}


static void MouseDown( struct BlockWindow *bw, WORD x, WORD y, WORD button)
{
	if( button == IECODE_LBUTTON )
	{
		bw->bw_DragFlag = TRUE;
		BW_EraseSelector( bw );
		bw->bw_AnchorX = x + bw->bw_WinX;
		bw->bw_AnchorY = y + bw->bw_WinY;
		BW_DrawSelector( bw, x, y );
	}
}


static void MouseUp( struct BlockWindow *bw, WORD x, WORD y, WORD button)
{
	UWORD blk;
	struct Brush *tempbrush;

	BW_EraseSelector( bw );

	x = x + bw->bw_WinX;
	y = y + bw->bw_WinY;

	switch( button )
	{
		case IECODE_LBUTTON:
			bw->bw_DragFlag = FALSE;
			if( x == bw->bw_AnchorX && y == bw->bw_AnchorY )
			{
				blk = PickUpBlock( bw,x,y);
				if( blk != ~0 ) lmbblk = blk;
				if( ReadTool() == TL_PASTE) SetTool( TL_DRAW );
			}
			else
			{
				//FreeBrush( currentbrush );
				tempbrush = GrabBWBrush( bw, bw->bw_AnchorX, bw->bw_AnchorY, x, y );
				if( tempbrush )
				{
					Insert( &brushes, &tempbrush->br_Node, &currentbrush->br_Node );
					currentbrush = tempbrush;
					RefreshAllBrushWindows();
				}
				SetTool( TL_PASTE );
			}
			break;
		case IECODE_MBUTTON:
			blk = PickUpBlock( bw,x,y);
			if( blk != ~0 ) mmbblk = blk;
			if( ReadTool() == TL_PASTE) SetTool( TL_DRAW );
			break;
		case IECODE_RBUTTON:
			blk = PickUpBlock( bw,x,y);
			if( blk != ~0 ) rmbblk = blk;
			if( ReadTool() == TL_PASTE) SetTool( TL_DRAW );
			break;
	}

	if( prevprojwin )
	{
		WindowToBack( bw->bw_sw.sw_Window );
		ActivateWindow( prevprojwin->pw_sw.sw_Window);
	}
}



static void MouseMove( struct BlockWindow *bw, WORD x, WORD y )
{
	if( x != bw->bw_PrevX || y != bw->bw_PrevY )
	{
		bw->bw_PrevX = x;
		bw->bw_PrevY = y;
		BW_EraseSelector( bw );
		BW_DrawSelector( bw, x, y );
	}
}




static void RenameBlockset( struct SuperWindow *sw )
{
	static struct ReqCrap req[] = {
		{ "Name:", HOOPYREQTYPE_STRING, NULL, BLOCKSETNAMESIZE-1, NULL }
	};
	UBYTE namebuf[ BLOCKSETNAMESIZE ];
	char *oldtitle;

	Mystrncpy( namebuf, globlocks->bs_Name, BLOCKSETNAMESIZE-1 );
	req[0].rc_Data = namebuf;

	if( DoHoopyReq( "Rename Blockset", req, 1 ) )
	{
		MyStrToUpper( namebuf );
		Mystrncpy( globlocks->bs_Name, namebuf, BLOCKSETNAMESIZE-1 );

		if( sw->sw_Window )
		{
			oldtitle = sw->sw_Window->Title;
			/* create and set up new title */
			SetWindowTitles( sw->sw_Window,
				sw->sw_FlgTitle ? BuildWindowTitle( sw ) : NULL,
				(char *)-1 );
			/* free old title string */
			if(oldtitle)
				FreeVec(oldtitle);
		}
	}
}
