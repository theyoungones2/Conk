/****************************************************************************/
//
// BONK: ProjectWindows.c
//
/****************************************************************************/



#include <stdio.h>
#include <string.h>
#include <dos/dos.h>
#include <dos/dostags.h>
#include <exec/exec.h>
#include <intuition/intuition.h>
#include <intuition/gadgetclass.h>
#include <intuition/icclass.h>
#include <intuition/imageclass.h>
#include <graphics/gfxbase.h>
#include <libraries/gadtools.h>
#include <libraries/diskfont.h>
#include <libraries/asl.h>
#include <libraries/iffparse.h>

#include <proto/exec.h>
#include <proto/dos.h>
#include <proto/intuition.h>
#include <proto/graphics.h>
#include <proto/diskfont.h>
#include <proto/asl.h>
#include <clib/alib_protos.h>
#include <proto/gadtools.h>
#include <proto/iffparse.h>
#include <proto/utility.h>

#include <global.h>
#include <ExternVars.h>



/* scroller crap */


enum	{	GAD_NONE, GAD_VSCROLLER, GAD_HSCROLLER, GAD_UP, GAD_DOWN,
			GAD_LEFT, GAD_RIGHT };

	/* The arrow image height. */

#define ARROW_HEIGHT	11
#define ARROW_WIDTH		16
//(sizewidth-1)
/*
STATIC struct Gadget		*Scroller,
				*UpArrow,
				*DownArrow;

STATIC UWORD			 RightBorderWidth;

STATIC struct Image		*UpImage,
				*DownImage;
*/
static BOOL SetupScrollers( struct ProjectWindow *pw );
static void DeleteScrollers( struct ProjectWindow *pw );

/* end of scrollcrap */

static void HandleCursorKeys( struct ProjectWindow *pw,
	UWORD rawkey, UWORD qual );

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
		{  NM_ITEM,	"About...",							0,	0,	0,	0,},
		{  NM_ITEM,	"Quit Program...",		 "Q",	0,	0,	0,},

		{ NM_TITLE,	"Edit",									0,	0,	0,	0,},
		{  NM_ITEM,	"Undo",									"Z",	0,	0,	0,},
		{  NM_ITEM,	NM_BARLABEL,						0,	0,	0,	0,},
		{  NM_ITEM,	"Load Blocks...",				0,	0,	0,	0,},
		{  NM_ITEM,	"Load Palette...",			0,	0,	0,	0,},

		{ NM_TITLE,	"Map",									0,	0,	0,	0,},
		{  NM_ITEM,	"Fix Background?",			0,	CHECKIT|MENUTOGGLE,	0,	0,},
		{  NM_ITEM,	"Resize Map...",				0,	0,	0,	0,},
		{  NM_ITEM,	"Rename Map",						0,	0,	0,	0,},
		{  NM_ITEM,	"Go to Bookmark",				0,	0,	0,	0,},
		{   NM_SUB,	"Go to 1",						 	0,	0,	0,	0,},
		{   NM_SUB,	"Go to 2",						 	0,	0,	0,	0,},
		{   NM_SUB,	"Go to 3",						 	0,	0,	0,	0,},
		{   NM_SUB,	"Go to 4",						 	0,	0,	0,	0,},
		{   NM_SUB,	"Go to 5",						 	0,	0,	0,	0,},
		{   NM_SUB,	"Go to 6",						 	0,	0,	0,	0,},
		{   NM_SUB,	"Go to 7",						 	0,	0,	0,	0,},
		{   NM_SUB,	"Go to 8",						 	0,	0,	0,	0,},
		{   NM_SUB,	"Go to 9",						 	0,	0,	0,	0,},
		{   NM_SUB,	"Go to 10",						 	0,	0,	0,	0,},


		{  NM_ITEM,	"Set BookMark",					0,	0,	0,	0,},
		{   NM_SUB,	"Set 1",							 	0,	0,	0,	0,},
		{   NM_SUB,	"Set 2",							 	0,	0,	0,	0,},
		{   NM_SUB,	"Set 3",							 	0,	0,	0,	0,},
		{   NM_SUB,	"Set 4",							 	0,	0,	0,	0,},
		{   NM_SUB,	"Set 5",							 	0,	0,	0,	0,},
		{   NM_SUB,	"Set 6",							 	0,	0,	0,	0,},
		{   NM_SUB,	"Set 7",							 	0,	0,	0,	0,},
		{   NM_SUB,	"Set 8",							 	0,	0,	0,	0,},
		{   NM_SUB,	"Set 9",							 	0,	0,	0,	0,},
		{   NM_SUB,	"Set 10",							 	0,	0,	0,	0,},

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
#define MN_MAP							2
#define MN_WINDOW						3
#define MN_SETTINGS					4
#define MN_USER							5

//#define MN_EDIT_IDX					19
#define MN_MAP_IDX					18
#define MN_WINDOW_IDX				44
#define MN_SETTINGS_IDX			63
#define MN_USER_IDX					71

/* prototypes local to this source file: */

BOOL ShowProjWindow(struct SuperWindow *mw);
void HideProjWindow(struct SuperWindow *sw);
void KillOffProjWindow(struct SuperWindow *sw);
void ProjWindowHandler(struct SuperWindow *sw, struct IntuiMessage *imsg);
BOOL SetupProjWinMenus( struct SuperWindow *sw );
void SortOutProjWindowMenus(struct SuperWindow *sw,
													 struct IntuiMessage *imsg,
													 UWORD code);
void SussOutCloseGadget(struct SuperWindow *sw);
static void ScrollLeft( struct ProjectWindow *pw, UWORD dist );
static void ScrollRight( struct ProjectWindow *pw, UWORD dist );
static void ScrollUp( struct ProjectWindow *pw, UWORD dist );
static void ScrollDown( struct ProjectWindow *pw, UWORD dist );



/**************** OpenNewProjWindow() ****************/
//
// Creates a new ProjectWindow, and attaches it to a Project.
//

BOOL OpenNewProjWindow(struct Project *proj)
{

	struct	ProjectWindow	*pw;
	struct	WindowDefault	*wf;

	/* grab mem for the ProjectWindow struct */
	if ( !( pw = AllocVec(sizeof(struct ProjectWindow), MEMF_CLEAR ) ) )
		return(FALSE);

	/* figure out a unique ID for this window (mainly for AREXX use) */
	InstallUniqueWindowID((struct SuperWindow *)pw);
	pw->pw_sw.sw_Node.ln_Name = (char *)&(pw->pw_sw.sw_idstring);

	/* find the Default settings for this type of SuperWindow */
	wf = &glob.cg_WinDefs[SWTY_PROJECT];

	/* set up ProjectWindow stuff */
	pw->pw_Project = proj;											/* point to project (if any) */
	pw->pw_MapX	= 0;
	pw->pw_MapY = 0;
	//ResetSelector( pw );
	//pw->pw_SelState = FALSE;
	//pw->pw_PrevX = -1;
	//pw->pw_PrevY = -1;

	/* set up general SuperWindow stuff */
	pw->pw_sw.sw_Handler = ProjWindowHandler;
	pw->pw_sw.sw_HideWindow = HideProjWindow;
	pw->pw_sw.sw_ShowWindow = ShowProjWindow;
	pw->pw_sw.sw_BuggerOff = KillOffProjWindow;
	pw->pw_sw.sw_RefreshMenus = SetupProjWinMenus;

	pw->pw_sw.sw_Window = NULL;
	pw->pw_sw.sw_MenuStrip = NULL;
	pw->pw_sw.sw_GadList = NULL;
	pw->pw_sw.sw_Type = SWTY_PROJECT;
	pw->pw_sw.sw_VisualInfo = NULL;

	/* set up stuff for opening the actual intuition window */
	pw->pw_sw.sw_Left = wf->wf_Left;
	pw->pw_sw.sw_Top = wf->wf_Top;
	pw->pw_sw.sw_Width = wf->wf_Width;
	pw->pw_sw.sw_Height = wf->wf_Height;
	pw->pw_sw.sw_MinWidth = 50;
	pw->pw_sw.sw_MinHeight = 50;
	pw->pw_sw.sw_MaxWidth = 0xFFFF;
	pw->pw_sw.sw_MaxHeight = 0xFFFF;
	pw->pw_sw.sw_MenuStrip = NULL;

	pw->pw_sw.sw_FlgBackDrop = wf->wf_FlgBackDrop;
	pw->pw_sw.sw_FlgBorderless = wf->wf_FlgBorderless;
	pw->pw_sw.sw_FlgTitle = wf->wf_FlgTitle;
	pw->pw_sw.sw_FlgBorderGadgets = wf->wf_FlgBorderGadgets;

	pw->pw_sw.sw_Title = pw->pw_Project->pj_Node.ln_Name;

	/* add ProjectWindow into the superwindows list */
	AddTail(&superwindows, &pw->pw_sw.sw_Node);

	/* link to project */
	pw->pw_NextProjWindow = proj->pj_ProjectWindows;
	proj->pj_ProjectWindows = pw;

	/* open the intuition window (if the window doesn't open, */
	/* it is just left hidden - SEP). */

	ShowProjWindow(&pw->pw_sw);

	return(TRUE);
}



BOOL ShowProjWindow(struct SuperWindow *sw)
{
	struct ProjectWindow *pw;
	ULONG flags;

	pw = (struct ProjectWindow *) sw;

	if(!sw->sw_Window)				/* make sure it isn't already open */
	{
		/* Set up the Visual Info for GadTools */
		if (!(sw->sw_VisualInfo = GetVisualInfo( mainscreen, TAG_END )))
			return(FALSE);

		/* zero all the gadget stuff */
		pw->pw_VScroller = NULL;
		pw->pw_UpArrow = NULL;
		pw->pw_DownArrow = NULL;
		pw->pw_HScroller = NULL;
		pw->pw_LeftArrow = NULL;
		pw->pw_RightArrow = NULL;
		pw->pw_UpImage = NULL;
		pw->pw_DownImage = NULL;
		pw->pw_LeftImage = NULL;
		pw->pw_RightImage = NULL;

		flags = WFLG_ACTIVATE | WFLG_NEWLOOKMENUS;
		if(sw->sw_FlgTitle)
			flags = flags | WFLG_DRAGBAR | WFLG_DEPTHGADGET | WFLG_CLOSEGADGET;
		if(sw->sw_FlgBorderGadgets)
		{
			flags = flags | WFLG_SIZEGADGET | WFLG_SIZEBRIGHT | WFLG_SIZEBBOTTOM;
			SetupScrollers(pw);
		}


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
											WA_Backdrop,		sw->sw_FlgBackDrop,
											WA_Borderless,	sw->sw_FlgBorderless,
											WA_CustomScreen, mainscreen,
											WA_AutoAdjust,	TRUE,
											WA_ReportMouse,	TRUE,
//											WA_RMBTrap,			TRUE,
											WA_Title,				sw->sw_FlgTitle ?
												BuildWindowTitle(sw) : NULL,
											WA_Gadgets, pw->pw_VScroller,
											TAG_DONE,				NULL);

		if( !(sw->sw_Window)) return(FALSE);

		/* Tell the window which superwindow it is attached to */
		/* (so we'll know which handler routine to call) */
		sw->sw_Window->UserData = (BYTE *)pw;

		/* Set up some menus */
		SetupProjWinMenus( &pw->pw_sw );

		/* we want to use our global message port for IDCMP stuff */
		sw->sw_Window->UserPort = mainmsgport;
		ModifyIDCMP(sw->sw_Window, IDCMP_CLOSEWINDOW | IDCMP_RAWKEY |
			IDCMP_MENUPICK | IDCMP_CHANGEWINDOW | IDCMP_MOUSEBUTTONS |
			IDCMP_MENUVERIFY | IDCMP_ACTIVEWINDOW | IDCMP_INACTIVEWINDOW |
			IDCMP_MOUSEMOVE | IDCMP_REFRESHWINDOW | IDCMP_NEWSIZE |
			IDCMP_IDCMPUPDATE | IDCMP_GADGETDOWN | IDCMP_GADGETUP);	// | SCROLLERIDCMP );

		ResetSelector( pw );								/* init selector vars */
		DrawMap(pw);
		UpdatePWScrollers(pw);
		prevprojwin = pw;
		pw->pw_ScrollyState = GAD_NONE;

	}
	return(TRUE);
}



BOOL SetupProjWinMenus( struct SuperWindow *sw )
{
	BOOL success = FALSE;

	if (sw->sw_MenuStrip)						/* Clear the menu strip if it exists */
	{
		ClearMenuStrip(sw->sw_Window);
		FreeMenus(sw->sw_MenuStrip);
		sw->sw_MenuStrip = NULL;
	}

	/* Set up some menus */
	SetupMapMenu( (struct ProjectWindow *)sw, &mapmenu[MN_MAP_IDX]);
	SetupWindowMenu( sw, &mapmenu[MN_WINDOW_IDX] );
	SetupSettingsMenu( &mapmenu[MN_SETTINGS_IDX] );
	LinkInUserMenu( &mapmenu[MN_USER_IDX] );

	if( sw->sw_MenuStrip = CreateMenus(mapmenu,TAG_END) )
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




void HideProjWindow(struct SuperWindow *sw)
{
	if (sw->sw_Window)												/* make sure it's actually open */
	{
		/* check to see if it's the most recently active one */
		if( prevprojwin )
		{
			if( sw->sw_id == prevprojwin->pw_sw.sw_id )
				prevprojwin = NULL;
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

		DeleteScrollers( (struct ProjectWindow *)sw );

		if (sw->sw_VisualInfo) FreeVisualInfo(sw->sw_VisualInfo);
		sw->sw_VisualInfo = NULL;
		SetProcessWindowPtr();
	}
}



void KillOffProjWindow(struct SuperWindow *sw)
{
	struct ProjectWindow *pw;
	struct Project *proj;

	pw = (struct ProjectWindow *)sw;

	/* see which project we're attached to */
	proj = pw->pw_Project;

	UnlinkProjectWindow(pw);

	/* close/free the superwindow */
	HideProjWindow(sw);												/* shut down the Intuition window */
	Remove(&(sw->sw_Node));										/* remove from superwindows list */
	FreeVec(pw);															/* free ProjectWindow struct */

	if (!proj->pj_ProjectWindows)
	{
		/* we've just shut down the last window attached to the */
		/* project, so we also want to kill off the project */

		RemoveProject(proj);
	}
}



void ProjWindowHandler(struct SuperWindow *sw, struct IntuiMessage *imsg)
{
	ULONG	class, temp;
	UWORD	code, qual;
	struct ProjectWindow *pw;
	WORD	blkx,blky,mx,my;
	struct IntuiMessage *gtimsg;
	struct Gadget *gad;
	struct TagItem *taglist;

//	char name[256];
	struct Window *win;

//	struct Project *proj;

//	UBYTE *autocon="CON:0/40/640/150/Auto CON Window Opens if Needed/auto/close/wait";
//	BPTR file;


	pw = (struct ProjectWindow *)sw;
	win = sw->sw_Window;

	if( gtimsg = GT_FilterIMsg(imsg) )
	{
		class = gtimsg->Class;
		code = gtimsg->Code;
		qual = gtimsg->Qualifier;
		gad = (struct Gadget *)gtimsg->IAddress;
		mx = gtimsg->MouseX;
		my = gtimsg->MouseY;

		GT_PostFilterIMsg(gtimsg);

		if( (blkx = mx	- (WORD)win->BorderLeft) < 0 )
			blkx -= 16;
		blkx /= BLKW;

		if( (blky = my - (WORD)win->BorderTop) < 0 )
			blky -= 16;
		blky /= BLKH;

		blkx = ClipToWindowBlkX( pw->pw_sw.sw_Window, blkx );
		blky = ClipToWindowBlkY( pw->pw_sw.sw_Window, blky );

		switch(class)
		{
			case IDCMP_CLOSEWINDOW:
				ReplyMsg((struct Message *)imsg);
				SussOutCloseGadget(sw);
				break;
			case IDCMP_RAWKEY:
				ReplyMsg((struct Message *)imsg);
				EraseSelector(pw);

				code = HandleGlobalRawKey( sw, code, qual );

				switch (code)
				{
					case 0x37:		/* M key */
						GUIResizeProject( pw->pw_Project );
						break;
					case 0x40:		/* Spacebar */
						//pw->pw_Project->pj_ModifiedFlag = TRUE;
						//KludgeLoadBlocks();
						//Undo( &undobuf );
						//DoHoopyReq();
						BlockWindowToFront();
						break;
					case 0x4C:
					case 0x4D:
					case 0x4E:
					case 0x4F:
						HandleCursorKeys( pw, code, qual );
						UpdatePWScrollers( pw );
						break;
					case 0x01:
					case 0x02:
					case 0x03:
					case 0x04:
					case 0x05:
					case 0x06:
					case 0x07:
					case 0x08:
					case 0x09:
					case 0x0A:
						if( qual & IEQUALIFIER_LSHIFT || qual & IEQUALIFIER_RSHIFT )
							SetBookMark( pw, code - 0x01 );
						else
							GotoBookMark( pw, code - 0x01 );
						break;
				}
				break;

			case IDCMP_MENUPICK:
				SortOutProjWindowMenus( sw,imsg,code );
				break;
			case IDCMP_ACTIVEWINDOW:
				ReplyMsg((struct Message *)imsg);
				prevprojwin = pw;
				MakeDefault( sw );
				break;
			case IDCMP_INACTIVEWINDOW:
				ReplyMsg((struct Message *)imsg);
				EraseSelector(pw);
				break;
			case IDCMP_MOUSEMOVE:


/*
					if(ReviewGlobalLines > ReviewLines)
					{
						LONG NewTop;

						if(GetAttr(PGA_Top,Scroller,&NewTop))
							ScrollReview(NewTop);
					}
*/



				ReplyMsg((struct Message *)imsg);
				switch( pw->pw_ScrollyState )
				{
					case GAD_NONE:
						Tool_Move( pw, blkx, blky );
						break;
					case GAD_VSCROLLER:
						if(GetAttr(PGA_Top,pw->pw_VScroller,&temp))
						{
							pw->pw_MapY = temp;
							DrawMap( pw );
						}
						break;
					case GAD_HSCROLLER:
						if(GetAttr(PGA_Top,pw->pw_HScroller,&temp))
						{
							pw->pw_MapX = temp;
							DrawMap( pw );
						}
						break;
				}
				break;
			case IDCMP_MOUSEBUTTONS:
				ReplyMsg((struct Message *)imsg);
				//printf("MouseX:%d, MouseY:%d\n",imsg->MouseX,imsg->MouseY);
				if(code & IECODE_UP_PREFIX)
					Tool_Up( pw, blkx, blky, code & ~IECODE_UP_PREFIX);
				else
					Tool_Down( pw, blkx, blky, code);
				break;
			case IDCMP_MENUVERIFY:
				if(code == MENUHOT)
				{
					/* the menu event came from this window, so we can cancel it */
					/* if we feel the inclination. */

					//printf("MENUVERIFY, MENUHOT\n");
					/* check mouse position against window boundaries */
					if( (imsg->MouseX > 0) && (imsg->MouseX < win->Width) &&
						(imsg->MouseY > 0) && (imsg->MouseY < win->Height) )
					{
						/* RMB down within window - cancel the menu, */
						/* and send the event on as a SELECTDOWN */
						imsg->Code = MENUCANCEL;
						ReplyMsg((struct Message *)imsg);
						Tool_Down(pw, blkx, blky, IECODE_RBUTTON);
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
					//printf("MENUVERIFY, not MENUHOT\n");
					ReplyMsg((struct Message *)imsg);
				}
				break;
			case IDCMP_CHANGEWINDOW:
				ReplyMsg((struct Message *)imsg);
				MakeDefault( sw );
				break;
			case IDCMP_NEWSIZE:
				ReplyMsg((struct Message *)imsg);
				EraseSelector(pw);
				DrawMap(pw);
				UpdatePWScrollers(pw);
				break;
			case IDCMP_REFRESHWINDOW:
				/* repair GadTool renderings */
				ReplyMsg((struct Message *)imsg);
				GT_BeginRefresh(sw->sw_Window);
				GT_EndRefresh(sw->sw_Window,TRUE);
				break;
			case IDCMP_IDCMPUPDATE:
				taglist		= (struct TagItem *)imsg->IAddress;
				ReplyMsg((struct Message *)imsg);
				EraseSelector(pw);
				switch( GetTagData( GA_ID, 0, taglist ) )
				{
					case GAD_UP:	ScrollUp(pw,1);
						break;
					case GAD_DOWN:	ScrollDown(pw,1);
						break;
					case GAD_LEFT:	ScrollLeft(pw,1);
						break;
					case GAD_RIGHT:	ScrollRight(pw,1);
						break;
				}
				UpdatePWScrollers( pw );
				break;
			case IDCMP_GADGETDOWN:
//				taglist		= (struct TagItem *)imsg->IAddress;
				gad = (struct Gadget *)imsg->IAddress;
				ReplyMsg((struct Message *)imsg);
				EraseSelector(pw);

//				switch( GetTagData( GA_ID, 0, taglist ) )
				switch( gad->GadgetID )
				{
					case GAD_VSCROLLER:
						pw->pw_ScrollyState = GAD_VSCROLLER;
						break;
					case GAD_HSCROLLER:
						pw->pw_ScrollyState = GAD_HSCROLLER;
						break;
					default:
						pw->pw_ScrollyState = GAD_NONE;
						break;
				}
				break;
			case IDCMP_GADGETUP:
//				taglist		= (struct TagItem *)imsg->IAddress;
				gad = (struct Gadget *)imsg->IAddress;
				ReplyMsg((struct Message *)imsg);
				EraseSelector(pw);
//				switch( GetTagData( GA_ID, 0, taglist ) )
				switch( gad->GadgetID )
				{
					case GAD_VSCROLLER:
					case GAD_HSCROLLER:
						if(GetAttr(PGA_Top,pw->pw_VScroller,&temp))
						{
							pw->pw_MapY = temp;
							if(GetAttr(PGA_Top,pw->pw_HScroller,&temp))
							{
								pw->pw_MapX = temp;
//								DrawMap( pw );
							}
						}
						pw->pw_ScrollyState = GAD_NONE;
						break;
				}
				break;
			default:
				ReplyMsg((struct Message *)imsg);
				break;
		}
	}
	else
		ReplyMsg((struct Message *)imsg);
}


static void HandleCursorKeys( struct ProjectWindow *pw,
	UWORD rawkey, UWORD qual )
{
	UWORD xstep, ystep;

	xstep = 1;
	ystep = 1;
	if( ( qual & IEQUALIFIER_LALT ) || ( qual & IEQUALIFIER_RALT ) )
	{
		xstep = 4;
		ystep = 4;
	}
	if( ( qual & IEQUALIFIER_LSHIFT ) || ( qual & IEQUALIFIER_RSHIFT ) )
	{
		xstep = ( pw->pw_sw.sw_Window->Width / BLKW ) - 2;
		ystep = ( pw->pw_sw.sw_Window->Height / BLKH ) - 2;
	}

	if( qual & IEQUALIFIER_CONTROL )
	{
		xstep = pw->pw_Project->pj_MapW;
		ystep = pw->pw_Project->pj_MapH;
	}

	switch( rawkey )
	{
		case 0x4C:		/* cursor up */
				ScrollUp( pw, ystep );
			break;
		case 0x4D:		/* cursor down */
				ScrollDown( pw, ystep );
			break;
		case 0x4F:		/* cursor left */
				ScrollLeft( pw, xstep );
			break;
		case 0x4E:		/* cursor right */
				ScrollRight( pw, xstep );
			break;
	}
}




/********** SussOutCloseGadget **************/
void SussOutCloseGadget(struct SuperWindow *sw)
{
	MaybeCloseProjectWindow( (struct ProjectWindow *)sw );
}


void MaybeCloseProjectWindow( struct ProjectWindow *pw )
{
	struct Project *proj;

	proj = pw->pw_Project;

	if( !proj->pj_ProjectWindows->pw_NextProjWindow )
	{
		if (proj->pj_ModifiedFlag)
		{
			switch( ModifiedProjectRequester( pw->pw_Project ) )
			{
				case 1:
					/* Save the Project */
//					PM_Save( proj, pw->pw_sw.sw_Window );
					if( !proj->pj_ModifiedFlag )
						KillOffProjWindow( &pw->pw_sw );
					break;
				case 2:
					KillOffProjWindow( &pw->pw_sw );
					break;
			}
		}
		else
			KillOffProjWindow( &pw->pw_sw );
	}
	else
		KillOffProjWindow( &pw->pw_sw );
}


/********** SORT OUT PROJECT MENUS **************/

void SortOutProjWindowMenus(struct SuperWindow *sw,
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
			case MN_MAP:
				status |= DoMapMenu(sw,menunumber[i]);
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


static void ScrollLeft( struct ProjectWindow *pw, UWORD dist )
{
	UWORD winh, winw, winx, winy, *ptr;
	struct Window *win;
	struct Project *proj;

	/* track down window and project structs */
	proj = pw->pw_Project;
	win = pw->pw_sw.sw_Window;

	/* see how many blocks we can see in this window */
	winw = ( ( win->Width - win->BorderLeft - win->BorderRight ) / BLKW ) + 1;
	winh = ( ( win->Height - win->BorderTop - win->BorderBottom ) / BLKH ) + 1;

	/* trying to scroll off end of map? */
	if( dist > pw->pw_MapX )
		dist = pw->pw_MapX;

	/* update viewpoint */
	pw->pw_MapX -= dist;

	if( dist >= winw )
	{
		/* just redraw the whole window */
		DrawMap(pw);
	}
	else
	{

		/* scroll existing blocks off */
		ScrollRaster( win->RPort,
			0-(dist*BLKW),0,
			win->BorderLeft,
			win->BorderTop,
			win->Width - win->BorderRight - 1,
			win->Height - win->BorderBottom - 1);

		/* draw a new strip of blocks on the left    */
		/* (we may want to draw more than one strip) */
		for( winx = 0; winx < dist; winx++ )
		{
			/* find position in mapdata */
			ptr = proj->pj_Map;
			ptr += ( (pw->pw_MapY * proj->pj_MapW ) + pw->pw_MapX + winx );

			/* loop through and draw one vertical strip */
			for( winy=0; winy < winh; winy++)
			{
				if( ( winx + pw->pw_MapX ) < proj->pj_MapW  &&
					( winy +pw->pw_MapY ) < proj->pj_MapH )
					/* still on map */
					DrawBlock( win, winx, winy, *ptr);
				else
					/* off the map - just draw a blank */
					DrawBlock( win, winx, winy, 0);
				/* down to next line */
				ptr += proj->pj_MapW;
			}
		}
	}
}


static void ScrollRight( struct ProjectWindow *pw, UWORD dist )
{
	UWORD winh, winw, winx, winy, *ptr;
	struct Window *win;
	struct Project *proj;

	/* track down window and project structs */
	proj = pw->pw_Project;
	win = pw->pw_sw.sw_Window;

	/* see how many blocks we can see in this window */
	winw = ( ( win->Width - win->BorderLeft - win->BorderRight ) / BLKW ) + 1;
	winh = ( ( win->Height - win->BorderTop - win->BorderBottom ) / BLKH ) + 1;

	/* trying to scroll off end of map? */
	if( pw->pw_MapX + dist + (winw - 1) > proj->pj_MapW )
		dist = proj->pj_MapW - (winw - 1) - pw->pw_MapX;
	if( (WORD)dist < 0 ) dist = 0;

	/* update viewpoint */
	pw->pw_MapX += dist;

	if( dist >= winw )
	{
		/* just redraw the whole window */
		DrawMap(pw);
	}
	else
	{

		/* scroll existing blocks off */
		ScrollRaster( win->RPort,
			dist * BLKW,0,
			win->BorderLeft,
			win->BorderTop,
			win->Width - win->BorderRight - 1,
			win->Height - win->BorderBottom - 1);

		/* draw a new strip of blocks on the right   */
		/* (we may want to draw more than one strip) */
		for( winx = winw-dist-1; winx < winw; winx++ )
		{
			/* find position in mapdata */
			ptr = proj->pj_Map;
			ptr += ( (pw->pw_MapY * proj->pj_MapW ) + pw->pw_MapX + winx );

			/* loop through and draw one vertical strip */
			for( winy=0; winy < winh; winy++)
			{
				if( ( winy + pw->pw_MapY ) < proj->pj_MapH  &&
					( winx +pw->pw_MapX ) < proj->pj_MapW )
					/* still on map */
					DrawBlock( win, winx, winy, *ptr);
				else
					/* off the map - just draw a blank */
					DrawBlock( win, winx, winy, 0);
				/* down to next line */
				ptr += proj->pj_MapW;
			}
		}
	}
}

static void ScrollUp( struct ProjectWindow *pw, UWORD dist )
{
	UWORD winh, winw, winx, winy, *ptr;
	struct Window *win;
	struct Project *proj;

	/* track down window and project structs */
	proj = pw->pw_Project;
	win = pw->pw_sw.sw_Window;

	/* see how many blocks we can see in this window */
	winw = ( ( win->Width - win->BorderLeft - win->BorderRight ) / BLKW ) + 1;
	winh = ( ( win->Height - win->BorderTop - win->BorderBottom ) / BLKH ) + 1;

	/* trying to scroll off top of map? */
	if( dist > pw->pw_MapY )
		dist = pw->pw_MapY;

	/* update viewpoint */
	pw->pw_MapY -= dist;

	if( dist >= winh )
	{
		/* just redraw the whole window */
		DrawMap(pw);
	}
	else
	{

		/* scroll existing blocks off */
		ScrollRaster( win->RPort,
			0,0-(dist*BLKW),
			win->BorderLeft,
			win->BorderTop,
			win->Width - win->BorderRight - 1,
			win->Height - win->BorderBottom - 1);

		/* draw a new strip of blocks at the top     */
		/* (we may want to draw more than one strip) */
		for( winy = 0; winy < dist; winy++ )
		{
			/* find position in mapdata */
			ptr = proj->pj_Map;
			ptr += ( ( (pw->pw_MapY + winy) * proj->pj_MapW ) + pw->pw_MapX );

			/* loop through and draw one horizontal strip */
			for( winx=0; winx < winw; winx++)
			{
				if( ( winx + pw->pw_MapX ) < proj->pj_MapW  &&
					( winy +pw->pw_MapY ) < proj->pj_MapH )

					/* still on map */
					DrawBlock( win, winx, winy, *ptr);
				else
					/* off the map - just draw a blank */
					DrawBlock( win, winx, winy, 0);
				/* move right a block */
				ptr++;
			}
		}
	}
}

static void ScrollDown( struct ProjectWindow *pw, UWORD dist )
{
	UWORD winh, winw, winx, winy, *ptr;
	struct Window *win;
	struct Project *proj;

	/* track down window and project structs */
	proj = pw->pw_Project;
	win = pw->pw_sw.sw_Window;

	/* see how many blocks we can see in this window */
	winw = ( ( win->Width - win->BorderLeft - win->BorderRight ) / BLKW ) + 1;
	winh = ( ( win->Height - win->BorderTop - win->BorderBottom ) / BLKH ) + 1;

	/* trying to scroll off end of map? */
	if( pw->pw_MapY + dist + (winh - 1) > proj->pj_MapH )
		dist = proj->pj_MapH - (winh - 1) - pw->pw_MapY;
	if( (WORD)dist < 0 ) dist = 0;

	/* update viewpoint */
	pw->pw_MapY += dist;

	if( dist >= winh )
	{
		/* just redraw the whole window */
		DrawMap(pw);
	}
	else
	{
		/* scroll existing blocks off */
		ScrollRaster( win->RPort,
			0, dist * BLKW,
			win->BorderLeft,
			win->BorderTop,
			win->Width - win->BorderRight - 1,
			win->Height - win->BorderBottom - 1);

		/* draw a new strip of blocks at the bottom  */
		/* (we may want to draw more than one strip) */
		for( winy = winh-dist-1; winy < winh; winy++ )
		{
			/* find position in mapdata */
			ptr = proj->pj_Map;
			ptr += ( ( (pw->pw_MapY + winy) * proj->pj_MapW ) + pw->pw_MapX );

			/* loop through and draw one horizontal strip */
			for( winx=0; winx < winw; winx++)
			{
				if( ( winx + pw->pw_MapX ) < proj->pj_MapW  &&
					( winy +pw->pw_MapY ) < proj->pj_MapH )
					/* still on map */
					DrawBlock( win, winx, winy, *ptr);
				else
					/* off the map - just draw a blank */
					DrawBlock( win, winx, winy, 0);
				/* next block */
				ptr++;
			}
		}
	}
}



static BOOL SetupScrollers( struct ProjectWindow *pw )
{
	struct DrawInfo	*dri;
	BOOL	result = FALSE;
	LONG	sizewidth, sizeheight, rborder;
	UWORD	sizetype;

	if(dri = GetScreenDrawInfo( mainscreen ))
	{

		if( mainscreen->Flags & SCREENHIRES )
		{
			sizewidth = 18;
			sizeheight = 10;
			sizetype = SYSISIZE_MEDRES;
		}
		else
		{
			sizewidth = 13;
			sizeheight = 11;
			sizetype = SYSISIZE_LOWRES;
		}

		rborder = sizewidth;

		if( pw->pw_UpImage = (struct Image *)NewObject(NULL,"sysiclass",
			SYSIA_Size,	sizetype,
			SYSIA_Which,	UPIMAGE,
			SYSIA_DrawInfo,	dri,
		TAG_DONE) )
		{
			if( pw->pw_DownImage = (struct Image *)NewObject(NULL,"sysiclass",
				SYSIA_Size,	sizetype,
				SYSIA_Which,	DOWNIMAGE,
				SYSIA_DrawInfo,	dri,
			TAG_DONE))
			{
				if(pw->pw_VScroller = NewObject(NULL,"propgclass",
					GA_ID,		GAD_VSCROLLER,
					GA_Top, mainscreen->WBorTop + mainscreen->Font->ta_YSize + 2,
					GA_RelHeight,	-(mainscreen->WBorTop + mainscreen->Font->ta_YSize +
						2 + sizeheight + 1 + 2 * ARROW_HEIGHT ),
					GA_Width, sizetype	== SYSISIZE_LOWRES ? sizewidth-4 : sizewidth-8,
					GA_RelRight,	sizetype	== SYSISIZE_LOWRES ?
						-(sizewidth - 3) : -(sizewidth - 5),

					GA_Immediate,	TRUE,
					GA_FollowMouse,	TRUE,
					GA_RelVerify,	TRUE,
					GA_RightBorder,	TRUE,

					PGA_Freedom,	FREEVERT,
					PGA_NewLook,	TRUE,
					PGA_Borderless,	TRUE,

					PGA_Visible,	1,
					PGA_Total,	1,
				TAG_DONE))
				{
					STATIC struct TagItem ArrowMappings[] =
					{
						GA_ID,	GA_ID,
						TAG_END
					};

					if(pw->pw_UpArrow = NewObject(NULL,"buttongclass",
						GA_ID, GAD_UP,
						GA_Image,	pw->pw_UpImage,
						GA_RelRight,	-(sizewidth - 1),
						GA_RelBottom,	-(sizeheight - 1 + 2 * ARROW_HEIGHT),
						GA_Height,	ARROW_HEIGHT,
						GA_Width,	sizewidth,
						GA_Immediate,	TRUE,
						GA_RelVerify,	TRUE,
						GA_Previous,	pw->pw_VScroller,
						GA_RightBorder,	TRUE,
						ICA_TARGET,	ICTARGET_IDCMP,
						ICA_MAP,	ArrowMappings,
					TAG_DONE))
					{
						if(pw->pw_DownArrow = NewObject(NULL,"buttongclass",
							GA_ID, GAD_DOWN,
							GA_Image,	pw->pw_DownImage,
							GA_RelRight,	-(sizewidth - 1),
							GA_RelBottom,	-(sizeheight - 1 + ARROW_HEIGHT),
							GA_Height,	ARROW_HEIGHT,
							GA_Width,	sizewidth,
							GA_Immediate,	TRUE,
							GA_RelVerify,	TRUE,
							GA_Previous,	pw->pw_UpArrow,
							GA_RightBorder,	TRUE,
							ICA_TARGET,	ICTARGET_IDCMP,
							ICA_MAP,	ArrowMappings,
						TAG_DONE))
						{

							if( pw->pw_LeftImage = (struct Image *)NewObject(NULL,"sysiclass",
								SYSIA_Size,	sizetype,
								SYSIA_Which,	LEFTIMAGE,
								SYSIA_DrawInfo,	dri,
							TAG_DONE) )
							{
								if( pw->pw_RightImage = (struct Image *)NewObject(NULL,"sysiclass",
									SYSIA_Size,	sizetype,
									SYSIA_Which,	RIGHTIMAGE,
									SYSIA_DrawInfo,	dri,
								TAG_DONE))
								{
									if(pw->pw_HScroller = NewObject(NULL,"propgclass",
										GA_ID, GAD_HSCROLLER,
										GA_Left, mainscreen->WBorLeft,
										GA_RelWidth,	-(mainscreen->WBorLeft +
											2 + sizewidth + 1 + 2 * ARROW_WIDTH ),
										GA_Height, sizeheight-4,	//sizetype	== SYSISIZE_LOWRES ? sizeheight-4 : sizeheight-8,
										GA_RelBottom, -(sizeheight - 3),

//	sizetype	== SYSISIZE_LOWRES ?
//											-(sizeheight - 3) : -(sizeheight - 5),

										GA_Immediate,	TRUE,
										GA_FollowMouse,	TRUE,
										GA_RelVerify,	TRUE,
										GA_BottomBorder,	TRUE,
										GA_Previous,	pw->pw_DownArrow,

										PGA_Freedom,	FREEHORIZ,
										PGA_NewLook,	TRUE,
										PGA_Borderless,	TRUE,
					
										PGA_Visible,	10,
										PGA_Total,	100,
									TAG_DONE))
									{
										if(pw->pw_LeftArrow = NewObject(NULL,"buttongclass",
											GA_ID, GAD_LEFT,
											GA_Image,	pw->pw_LeftImage,
											GA_RelRight,	-(sizewidth - 1 + 2 * ARROW_WIDTH ),
											GA_RelBottom,	-(sizeheight - 1 ),
											GA_Height, sizeheight,
											GA_Width,	ARROW_WIDTH,
											GA_Immediate,	TRUE,
											GA_RelVerify,	TRUE,
											GA_Previous,	pw->pw_HScroller,
											GA_BottomBorder,	TRUE,
											ICA_TARGET,	ICTARGET_IDCMP,
											ICA_MAP,	ArrowMappings,
										TAG_DONE))
										{
											if(pw->pw_RightArrow = NewObject(NULL,"buttongclass",
												GA_ID, GAD_RIGHT,
												GA_Image,	pw->pw_RightImage,
												GA_RelRight,	-(sizewidth - 1 + ARROW_WIDTH ),
												GA_RelBottom,	-(sizeheight - 1 ),
												GA_Height,	sizeheight,
												GA_Width,	ARROW_WIDTH,
												GA_Immediate,	TRUE,
												GA_RelVerify,	TRUE,
												GA_Previous,	pw->pw_LeftArrow,
												GA_BottomBorder,	TRUE,
												ICA_TARGET,	ICTARGET_IDCMP,
												ICA_MAP,	ArrowMappings,
											TAG_DONE))
												result = TRUE;
										}
									}
								}
							}
						}
					}
				}
			}
		}

		FreeScreenDrawInfo( mainscreen, dri );
	}
	return(result);
}



void UpdatePWScrollers(struct ProjectWindow *pw)
{
	struct Window *win;
	LONG	pleb;

	if( win = pw->pw_sw.sw_Window )
	{
		pleb = ( win->Height - win->BorderTop - win->BorderBottom ) / BLKH;
//		if( pleb + pw->pw_MapY >= pw->pw_Project->pj_MapH )
//			pleb = pw->pw_Project->pj_MapH - pw->pw_MapY;

		SetGadgetAttrs(pw->pw_VScroller, pw->pw_sw.sw_Window, NULL,
			PGA_Top,	pw->pw_MapY,
			PGA_Visible,	pleb,
			PGA_Total,	pw->pw_Project->pj_MapH,
		TAG_DONE);

		pleb = ( win->Width - win->BorderLeft - win->BorderRight ) / BLKW;
//		if( pleb + pw->pw_MapX >= pw->pw_Project->pj_MapW )
//			pleb = pw->pw_Project->pj_MapW - pw->pw_MapX;

		SetGadgetAttrs(pw->pw_HScroller, pw->pw_sw.sw_Window, NULL,
			PGA_Top,	pw->pw_MapX,
			PGA_Visible,	pleb,
			PGA_Total,	pw->pw_Project->pj_MapW,
		TAG_DONE);
	}
}


static void DeleteScrollers( struct ProjectWindow *pw )
{
	if(pw->pw_VScroller)
	{
		DisposeObject( pw->pw_VScroller );
		pw->pw_VScroller = NULL;
	}

	if(pw->pw_UpArrow)
	{
		DisposeObject(pw->pw_UpArrow);
		pw->pw_UpArrow = NULL;
	}

	if(pw->pw_DownArrow)
	{
		DisposeObject(pw->pw_DownArrow);
		pw->pw_DownArrow = NULL;
	}

	if(pw->pw_UpImage)
	{
		DisposeObject(pw->pw_UpImage);
		pw->pw_UpImage = NULL;
	}

	if(pw->pw_DownImage)
	{
		DisposeObject(pw->pw_DownImage);
		pw->pw_DownImage = NULL;
	}

	if(pw->pw_HScroller)
	{
		DisposeObject( pw->pw_HScroller );
		pw->pw_HScroller = NULL;
	}

	if(pw->pw_LeftArrow)
	{
		DisposeObject(pw->pw_LeftArrow);
		pw->pw_LeftArrow = NULL;
	}

	if(pw->pw_RightArrow)
	{
		DisposeObject(pw->pw_RightArrow);
		pw->pw_RightArrow = NULL;
	}

	if(pw->pw_LeftImage)
	{
		DisposeObject(pw->pw_LeftImage);
		pw->pw_LeftImage = NULL;
	}

	if(pw->pw_RightImage)
	{
		DisposeObject(pw->pw_RightImage);
		pw->pw_RightImage = NULL;
	}
}





void DrawMap(struct ProjectWindow *pw)
{
//	struct BlockSet *bs;
	struct Project	*proj;
	struct Window		*win;

	UWORD winx, winy, blk;
	WORD rightedge, bottomedge;
	UWORD *base, *ptr;
	WORD xpos, ypos;

	/* make sure the window is all cool and groovy */
	if ( win = pw->pw_sw.sw_Window )
	{
		proj = pw->pw_Project;
		base = proj->pj_Map;
		/* find first line of visible map */
		base += ( (pw->pw_MapY * proj->pj_MapW) + pw->pw_MapX );

		/* see which bits appear on the right and bottom of the view area */
		rightedge = ( (win->Width - win->BorderLeft - win->BorderRight) / BLKW ) +
			pw->pw_MapX + 1;
		bottomedge = ( (win->Height - win->BorderTop - win->BorderBottom) / BLKH ) +
			pw->pw_MapY + 1;

		/* start drawing at top of window */

		winy = 0;

		/* main drawing loop */
		for(ypos = pw->pw_MapY; ypos < bottomedge; ypos++)
		{
			/* start drawing at inner leftedge of window */
			winx = 0;
			/* init map ptr to beginning of row */
			ptr = base;

			for(xpos = pw->pw_MapX; xpos < rightedge; xpos++)
			{
				/* read the block and make sure we're still within the map limits */
				if( (xpos < proj->pj_MapW) &&
					(ypos < proj->pj_MapH) )
					blk = *ptr++;
				else
					blk = 0;
				/* draw the block */
				DrawBlock(win, winx, winy, blk );
				/* update drawing position */
				winx++;
			}
			winy++;
			/* jump down to next line of mapdata */
			base += proj->pj_MapW;
		}
	}
}



void JumpToNextProjectWindow( struct SuperWindow *thissw )
{
	struct SuperWindow *sw, *newsw = NULL;

	if( thissw->sw_Type != SWTY_PROJECT )
	{
		if( prevprojwin )
			thissw = (struct SuperWindow *)prevprojwin;
	}

	sw = (struct SuperWindow *)thissw->sw_Node.ln_Succ;
	while( sw->sw_Node.ln_Succ && !newsw )
	{
		if( sw->sw_Type == SWTY_PROJECT && sw->sw_Window )
		{
			newsw = sw;
		}
		sw = (struct SuperWindow *)sw->sw_Node.ln_Succ;
	}

	sw = (struct SuperWindow *)superwindows.lh_Head;
	while( sw->sw_Node.ln_Succ && ( sw != thissw ) && !newsw )
	{
		if( sw->sw_Type == SWTY_PROJECT && sw->sw_Window )
		{
			newsw = sw;
		}
		sw = (struct SuperWindow *)sw->sw_Node.ln_Succ;
	}

	if( newsw )
	{
		WindowToFront( newsw->sw_Window );
		ActivateWindow( newsw->sw_Window );
	}
}

