
/********************************************************/
//
// GAMEWindow.c
//
/********************************************************/

#define GAMEWINDOW_C

#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <assert.h>

#include <dos/dos.h>
#include <dos/dostags.h>
#include <exec/exec.h>
#include <intuition/intuition.h>
//#include <graphics/gfxbase.h>
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
//#include <clib/alib_protos.h>
#include <proto/gadtools.h>
//#include <proto/iffparse.h>

#include "hoopy.h"
#include "global.h"
#include "MenuID.h"


struct GAMEWindow
{
	struct SuperWindow	gw_sw;
	struct Chunk				*gw_Chunk;
	UWORD								gw_Mode;					/* see below */
	/* stuff for tracking doubleclicks in listviews */
	UWORD								gw_lastclicknum;
	ULONG								gw_lastclicksecs;
	ULONG								gw_lastclickmicros;

	/* game config stuff */
	struct HoopyObject	*gw_GameLayout;
	struct LevelConfig	*gw_SelectedLevel;	/* or NULL */

	/* level config stuff */
	struct HoopyObject	*gw_LevelLayout;
	struct LFile				*gw_SelectedFile;		/* or NULL */
};

#define GWMODE_NONE 0
#define GWMODE_GAME 1
#define GWMODE_LEVEL 2
#define GWMODE_ANIMOPTIONS 3
#define GWMODE_PICOPTIONS 4



#define GID_BORING 0

#define GGID_TITLE 1
#define GGID_AUTHOR 2
#define GGID_COMMENT 3
#define GGID_LEVELS 4

#define GGID_EDIT		6
#define GGID_ADD		7
#define GGID_DELETE	8
#define GGID_UP			9
#define GGID_DOWN		10


static struct HoopyObject gamegadlayout[] =
{
	{ HOTYPE_VGROUP, 100, HOFLG_RAISEDBORDER|HOFLG_FREEWIDTH|HOFLG_FREEHEIGHT,5,0,0,0, NULL, 0,GID_BORING,0,0,0 },
		{ HOTYPE_STRING, 100, HOFLG_FREEWIDTH, NULL,64-1,0,0, "Title",0,GGID_TITLE,PLACETEXT_LEFT,0,0 },
		{ HOTYPE_STRING, 100, HOFLG_FREEWIDTH, NULL,64-1,0,0, "Author",0,GGID_AUTHOR,PLACETEXT_LEFT,0,0 },
		{ HOTYPE_STRING, 100, HOFLG_FREEWIDTH, NULL,128-1,0,0, "Comment",0,GGID_COMMENT,PLACETEXT_LEFT,0,0 },
		{ HOTYPE_LISTVIEW, 200, HOFLG_NOBORDER|HOFLG_FREEWIDTH|HOFLG_FREEHEIGHT,NULL,0,0,0,"Levels",0,GGID_LEVELS,PLACETEXT_ABOVE,0,0 },
		{ HOTYPE_HGROUP, 100, HOFLG_NOBORDER|HOFLG_FREEWIDTH,5,0,0,0, NULL, 0,GID_BORING,0,0,0 },
			{ HOTYPE_BUTTON, 100, HOFLG_NOBORDER, 0,0,0,0, "Edit",0,GGID_EDIT,0,0,0 },
			{ HOTYPE_BUTTON, 100, HOFLG_NOBORDER, 0,0,0,0, "New",0,GGID_ADD,0,0,0 },
			{ HOTYPE_BUTTON, 100, HOFLG_NOBORDER, 0,0,0,0, "Del",0,GGID_DELETE,0,0,0 },
			{ HOTYPE_BUTTON, 100, HOFLG_NOBORDER, 0,0,0,0, "Up",0,GGID_UP,0,0,0 },
			{ HOTYPE_BUTTON, 100, HOFLG_NOBORDER, 0,0,0,0, "Down",0,GGID_DOWN,0,0,0 },
	{HOTYPE_END}
};

#define LGID_NAME			1
#define LGID_COMMENT	2
#define LGID_LEVELTYPE	3

#define LGID_FILES		4

#define LGID_FILETYPE	6
#define LGID_FILE			7

#define LGID_ADD			9
#define LGID_DELETE		10
#define LGID_UP				11
#define LGID_DOWN			12

#define LGID_DONE			13

/* see filetypes in Game.h */
UBYTE *filetypes[] = { "????","CONK", "ILBM","MOD","ANIM",NULL };
/* see leveltypes in Game.h */
UBYTE *leveltypes[] = { "Game", "Pic","Anim",NULL };

static struct HoopyObject levelgadlayout[] =
{
	{ HOTYPE_VGROUP, 100, HOFLG_RAISEDBORDER|HOFLG_FREEWIDTH|HOFLG_FREEHEIGHT,7,0,0,0, NULL, 0,GID_BORING,0,0,0 },
		{ HOTYPE_STRING, 100, HOFLG_FREEWIDTH, NULL,LEVELFULLNAMESIZE-1,0,0, "LevelName",0,LGID_NAME,PLACETEXT_LEFT,0,0 },
		{ HOTYPE_STRING, 100, HOFLG_FREEWIDTH, NULL,64-1,0,0, "Comment",0,LGID_COMMENT,PLACETEXT_LEFT,0,0 },
		{ HOTYPE_CYCLE, 100, 0, (LONG)leveltypes,0,0,0, "Level Type",0,LGID_LEVELTYPE,PLACETEXT_LEFT,0,0 },
		{ HOTYPE_LISTVIEW, 200, HOFLG_NOBORDER|HOFLG_FREEWIDTH|HOFLG_FREEHEIGHT,NULL,0,0,0,"Files",0,LGID_FILES,PLACETEXT_ABOVE,0,0 },
		{ HOTYPE_HGROUP, 100, HOFLG_NOBORDER|HOFLG_FREEWIDTH,2,0,0,0, NULL, 0,GID_BORING,0,0,0 },
			{ HOTYPE_CYCLE, 100, HOFLG_NOBORDER, (LONG)filetypes,0,0,0, NULL,0,LGID_FILETYPE,0,0,0 },
			{ HOTYPE_STRING, 100, HOFLG_NOBORDER|HOFLG_FREEWIDTH, NULL,MAXFILENAMESIZE-1,0,0, NULL,0,LGID_FILE,0,0,0 },
		{ HOTYPE_HGROUP, 100, HOFLG_FREEWIDTH,4,0,0,0, NULL, 0,GID_BORING,0,0,0 },
			{ HOTYPE_BUTTON, 100, HOFLG_NOBORDER, 0,0,0,0, "Add...",0,LGID_ADD,0,0,0 },
			{ HOTYPE_BUTTON, 100, HOFLG_NOBORDER, 0,0,0,0, "Drop",0,LGID_DELETE,0,0,0 },
			{ HOTYPE_BUTTON, 100, HOFLG_NOBORDER, 0,0,0,0, "Up",0,LGID_UP,0,0,0 },
			{ HOTYPE_BUTTON, 100, HOFLG_NOBORDER, 0,0,0,0, "Down",0,LGID_DOWN,0,0,0 },
		{ HOTYPE_BUTTON, 100, 0, 0,0,0,0, "Done",0,LGID_DONE,0,0,0 },
	{HOTYPE_END}
};



static void IDCMPHandler( struct SuperWindow *sw,
	struct IntuiMessage *imsg );
static void HideWindow( struct SuperWindow *sw );
static BOOL ShowWindow( struct SuperWindow *sw );
static void KillWindow( struct SuperWindow *sw );
static BOOL RefreshMenus( struct SuperWindow *sw );
static UBYTE *MakeGAMEWinTitle( struct SuperWindow *sw );

static BOOL SetGameWinMode( struct GAMEWindow *gw, UWORD mode );

static VOID PickLevel( struct GAMEWindow *gw, UWORD num );
static VOID AddLevel( struct GAMEWindow *gw );
static VOID DeleteLevel( struct GAMEWindow *gw );
static VOID EditLevel( struct GAMEWindow *gw );
static VOID BubbleLevel( struct GAMEWindow *gw, WORD dir );

static VOID PickFile( struct GAMEWindow *gw, UWORD num );
static VOID AddFiles( struct GAMEWindow *gw );
static VOID FuckOffFile( struct GAMEWindow *gw );
static VOID BubbleFile( struct GAMEWindow *gw, WORD dir );

/********************************************************/
BOOL OpenNewGAMEWindow( struct Chunk *cnk )
{
	struct SuperWindow *sw;
	struct GAMEWindow *gw;
	BOOL success;

	assert( cnk != NULL );
	assert( cnk->ch_TypeID == ID_GAME );

	success = FALSE;
	LockChunk( cnk );

	if( gw = AllocVec( sizeof( struct GAMEWindow ), MEMF_ANY|MEMF_CLEAR ) )
	{
		sw = &gw->gw_sw;
		sw->sw_Handler = IDCMPHandler;
		sw->sw_HideWindow = HideWindow;
		sw->sw_ShowWindow = ShowWindow;
		sw->sw_BuggerOff = KillWindow;
		sw->sw_RefreshMenus = RefreshMenus;
		sw->sw_MakeTitle	= MakeGAMEWinTitle;
		sw->sw_Type				= SWTY_GAME;
		sw->sw_Window			= NULL;
		sw->sw_Gadgets		= NULL;
		sw->sw_VisualInfo	= NULL;
		sw->sw_WinDim			= zonkfig.SuperWindowDefs[ SWTY_GAME ];

		gw->gw_Chunk = cnk;

		/* make copies of hoopyvision object layout arrays */
		gw->gw_GameLayout = AllocVec( sizeof( gamegadlayout ), MEMF_ANY );
		gw->gw_LevelLayout = AllocVec( sizeof( levelgadlayout ), MEMF_ANY );
		if( gw->gw_GameLayout && gw->gw_LevelLayout )
		{
			CopyMem( gamegadlayout, gw->gw_GameLayout, sizeof( gamegadlayout ) );
			CopyMem( levelgadlayout, gw->gw_LevelLayout, sizeof( levelgadlayout ) );
			success = TRUE;
		}
			if( success )
		{
			(*sw->sw_ShowWindow)( sw );
			AddTail( &superwindows, &sw->sw_Node );
		}
		else
		{
			if( gw->gw_GameLayout ) FreeVec( gw->gw_GameLayout );
			if( gw->gw_LevelLayout ) FreeVec( gw->gw_LevelLayout );
			FreeVec( gw );
		}
	}

	return( success );
}



/********************************************************/
static BOOL ShowWindow( struct SuperWindow *sw )
{
	BOOL success = FALSE;
	struct GAMEWindow *gw;
	struct Window *win;
	struct Screen *scr;

	gw = (struct GAMEWindow *)sw;

	scr = LockMainScreen();

	win = OpenWindowTags( NULL,
		WA_Left, sw->sw_WinDim.Left,
		WA_Top, sw->sw_WinDim.Top,
		WA_Width, sw->sw_WinDim.Width,
		WA_Height, sw->sw_WinDim.Height,
		WA_MaxWidth, ~0,
		WA_MaxHeight, ~0,
		WA_PubScreen, scr,
		WA_PubScreenFallBack, TRUE,
		WA_AutoAdjust,	TRUE,
		WA_Title,				(*sw->sw_MakeTitle)(sw),
		WA_CloseGadget,	TRUE,
		WA_DragBar,			TRUE,
		WA_SizeGadget,	TRUE,
		WA_DepthGadget,	TRUE,
		WA_SizeBRight,	TRUE,
		WA_NewLookMenus,TRUE,
		WA_Activate,		TRUE,
		TAG_DONE,				NULL);

	UnlockMainScreen( scr );

	if( sw->sw_Window = win )
	{
		if( !zonkfig.scfg.scfg_DefaultFont && mainwinfont )
			SetFont( win->RPort, mainwinfont );

		if( sw->sw_hos = HO_GetHOStuff( win, win->RPort->Font ) )
		{
			/* Tell the window which superwindow it is attached to */
			/* (so we'll know which handler routine to call) */
			win->UserData = (BYTE *)sw;

			sw->sw_VisualInfo = sw->sw_hos->hos_vi;
			GenericMakeMenus( sw, NULL );

			/* we want to use our global message port for IDCMP stuff */
			win->UserPort = mainmsgport;
			ModifyIDCMP( win, IDCMP_REFRESHWINDOW|IDCMP_NEWSIZE|
				IDCMP_CLOSEWINDOW|IDCMP_CHANGEWINDOW|
				IDCMP_MENUPICK|LISTVIEWIDCMP|BUTTONIDCMP|STRINGIDCMP );

			success =  SetGameWinMode( gw, GWMODE_GAME );
		}

		if( !success )
		{
			CloseWindow( win );
			sw->sw_Window = NULL;
		}
	}

	return( success );
}



/********************************************************/
static void HideWindow( struct SuperWindow *sw )
{
	struct GAMEWindow *gw;

	gw = (struct GAMEWindow *)sw;

	if( sw->sw_Window )
	{
		RememberWindow( sw );
		GenericZapMenus( sw );

		/* clean up the gad layout */
		SetGameWinMode( gw, GWMODE_NONE );

		CloseWindowSafely( sw->sw_Window );
		HO_FreeHOStuff( sw->sw_hos );
		sw->sw_Window = NULL;
	}
}


/********************************************************/
static void KillWindow( struct SuperWindow *sw )
{
	struct GAMEWindow *gw;

	if( sw && IsOKToClose( sw ) )
	{
		gw = (struct GAMEWindow *)sw;
		(*sw->sw_HideWindow)(sw);
		Remove( &sw->sw_Node );
		if( gw->gw_GameLayout ) FreeVec( gw->gw_GameLayout );
		if( gw->gw_LevelLayout ) FreeVec( gw->gw_LevelLayout );
		UnlockChunk( gw->gw_Chunk );
		FreeVec( sw );
	}
}

/********************************************************/
static BOOL RefreshMenus( struct SuperWindow *sw )
{
	return( FALSE );
}

/********************************************************/

static UBYTE *MakeGAMEWinTitle( struct SuperWindow *sw )
{
	struct Chunk *cnk;
	UBYTE *p;

	cnk = ((struct GAMEWindow *)sw)->gw_Chunk;
	p = sw->sw_Name;

	if( cnk->ch_Modified )
		*p++ = MODCHAR;
	strcpy( p, "Game Config" );

	return( sw->sw_Name );
}


/********************************************************/

static void IDCMPHandler( struct SuperWindow *sw,
	struct IntuiMessage *origimsg )
{
	struct IntuiMessage *imsg;
	struct Window *win;
	struct GAMEWindow *gw;

	ULONG class;
	UWORD code;
//	UWORD qualifier;
	APTR iaddress;
//	WORD mousex, mousey;
	ULONG seconds, micros;

	gw = (struct GAMEWindow *)sw;
	win = sw->sw_Window;

	if( imsg = GT_FilterIMsg( origimsg ) )
	{
		HO_CheckIMsg( imsg );

		class = imsg->Class;
		code = imsg->Code;
//		qualifier = imsg->Qualifier;
		iaddress = imsg->IAddress;
//		mousex = imsg->MouseX;
//		mousey = imsg->MouseY;
		seconds = imsg->Seconds;
		micros = imsg->Micros;

		GT_PostFilterIMsg( imsg );

		switch( class )
		{
			case IDCMP_NEWSIZE:
				ClearWindow( win );
				switch( gw->gw_Mode )
				{
					case GWMODE_GAME:
						HO_ReadGadgetStates( sw->sw_hos, gw->gw_GameLayout );
						HO_MakeLayout( gw->gw_GameLayout, sw->sw_hos,
							win->BorderLeft, win->BorderTop,
							win->Width - win->BorderLeft - win->BorderRight,
							win->Height - win->BorderTop - win->BorderBottom );
						break;

					case GWMODE_LEVEL:
						HO_ReadGadgetStates( sw->sw_hos, gw->gw_LevelLayout );
						HO_MakeLayout( gw->gw_LevelLayout, sw->sw_hos,
							win->BorderLeft, win->BorderTop,
							win->Width - win->BorderLeft - win->BorderRight,
							win->Height - win->BorderTop - win->BorderBottom );
						break;
				}
				ReplyMsg( (struct Message *)origimsg );
				break;
			case IDCMP_CHANGEWINDOW:
				UpdateWinDef( sw );
				ReplyMsg( (struct Message *)origimsg );
				break;
			case IDCMP_GADGETUP:
				ReplyMsg( (struct Message *)origimsg );
				switch( gw->gw_Mode )
				{
					case GWMODE_GAME:
						switch( ((struct Gadget*)iaddress)->GadgetID )
						{
							case GGID_LEVELS:
								PickLevel( gw, code );
								break;
							case GGID_UP:
								ModifyChunk( gw->gw_Chunk );
								BubbleLevel( gw, -1 );
								break;
							case GGID_DOWN:
								ModifyChunk( gw->gw_Chunk );
								BubbleLevel( gw, 1 );
								break;
							case GGID_ADD:
								ModifyChunk( gw->gw_Chunk );
								AddLevel( gw );
								break;
							case GGID_DELETE:
								ModifyChunk( gw->gw_Chunk );
								DeleteLevel( gw );
								break;
							case GGID_EDIT:
								EditLevel( gw );
								break;
							case GGID_TITLE:
							case GGID_AUTHOR:
							case GGID_COMMENT:
								ModifyChunk( gw->gw_Chunk );
								HO_ReadGadgetStates( sw->sw_hos, gw->gw_GameLayout );
								break;
						}
						break;
					case GWMODE_LEVEL:
						switch( ((struct Gadget*)iaddress)->GadgetID )
						{
							case LGID_FILES:
								PickFile( gw, code );
								break;
							case LGID_ADD:
								ModifyChunk( gw->gw_Chunk );
								AddFiles( gw );
								break;
							case LGID_DELETE:
								ModifyChunk( gw->gw_Chunk );
								FuckOffFile( gw );
								break;
							case LGID_UP:
								ModifyChunk( gw->gw_Chunk );
								BubbleFile( gw, -1 );
								break;
							case LGID_DOWN:
								ModifyChunk( gw->gw_Chunk );
								BubbleFile( gw, 1 );
								break;
							case LGID_LEVELTYPE:
								ModifyChunk( gw->gw_Chunk );
								gw->gw_SelectedLevel->lc_LevelFileHeader.lfh_LevelType = code;
								break;
							case LGID_FILETYPE:
								if( gw->gw_SelectedFile )
								{
									ModifyChunk( gw->gw_Chunk );
									gw->gw_SelectedFile->lf_Type = code;
								}
								break;
							case LGID_NAME:
								ModifyChunk( gw->gw_Chunk );
								HO_ReadGadgetStates( sw->sw_hos, gw->gw_LevelLayout );
								MakePackable( gw->gw_SelectedLevel->lc_Name );
								HO_RefreshObject( &gw->gw_LevelLayout[ LGID_NAME ] , gw->gw_sw.sw_hos );
								break;
							case LGID_COMMENT:
								ModifyChunk( gw->gw_Chunk );
								HO_ReadGadgetStates( sw->sw_hos, gw->gw_LevelLayout );
								break;
							case LGID_FILE:
								/* detach list from listview */
								gw->gw_LevelLayout[ LGID_FILES ].ho_Attr0 = (LONG)~0;
								HO_RefreshObject( &gw->gw_LevelLayout[ LGID_FILES ] , gw->gw_sw.sw_hos );

								HO_ReadGadgetStates( sw->sw_hos, gw->gw_LevelLayout );

								/* update listview */
								gw->gw_LevelLayout[ LGID_FILES ].ho_Attr0 =
									(LONG)&gw->gw_SelectedLevel->lc_Files;
								HO_RefreshObject( &gw->gw_LevelLayout[ LGID_FILES ] , gw->gw_sw.sw_hos );
								break;
							case LGID_DONE:
								SetGameWinMode( gw, GWMODE_GAME );
								break;
						}
						break;
				}
				break;
			case IDCMP_REFRESHWINDOW:
				GT_BeginRefresh(win);
				GT_EndRefresh(win,TRUE);
				ReplyMsg( (struct Message *)origimsg );
				break;
			case IDCMP_MENUPICK:
				ReplyMsg( (struct Message *)origimsg );
				GenericHandleMenuPick( sw, code, NULL );
				break;
			case IDCMP_CLOSEWINDOW:
				ReplyMsg( (struct Message *)origimsg );
				switch( gw->gw_Mode )
				{
					case GWMODE_LEVEL:
						SetGameWinMode( gw, GWMODE_GAME );
						break;
					case GWMODE_GAME:
						KillWindow( sw );
						break;
				}
				break;
			default:
				ReplyMsg( (struct Message *)origimsg );
				break;
		}
	}
	else
		ReplyMsg( (struct Message *)origimsg );
}

static BOOL SetGameWinMode( struct GAMEWindow *gw, UWORD mode )
{
	BOOL success;
	struct Window *win;
	struct GameConfig *gc;

	gc = (struct GameConfig *)gw->gw_Chunk->ch_Data;

	/* sanity check - must have a selected level if going into level mode */
	if( mode == GWMODE_LEVEL && !gw->gw_SelectedLevel )
		return( FALSE );

	/* clean up after previous mode */

	switch( gw->gw_Mode )
	{
		case GWMODE_GAME:
		case GWMODE_LEVEL:
			HO_KillLayout( gw->gw_sw.sw_hos );
			break;
	}

	/* now switch to new mode */

	win = gw->gw_sw.sw_Window;
	ClearWindow( win );

	switch( mode )
	{
		case GWMODE_GAME:
			if( HO_SussWindowSizing( gw->gw_sw.sw_hos, gw->gw_GameLayout ) )
			{
				gw->gw_GameLayout[ GGID_TITLE ].ho_Attr0 =
					(LONG)gc->gc_GameFileHeader.gfh_GameName;
				gw->gw_GameLayout[ GGID_AUTHOR ].ho_Attr0 =
					(LONG)gc->gc_GameFileHeader.gfh_Author;
				gw->gw_GameLayout[ GGID_COMMENT ].ho_Attr0 =
					(LONG)gc->gc_GameFileHeader.gfh_Comment;
				gw->gw_GameLayout[ GGID_LEVELS ].ho_Attr0 =
					(LONG)&gc->gc_Levels;

				PickLevel( gw, 0 );

				HO_MakeLayout( gw->gw_GameLayout, gw->gw_sw.sw_hos,
					win->BorderLeft, win->BorderTop,
					win->Width - win->BorderLeft - win->BorderRight,
					win->Height - win->BorderTop - win->BorderBottom );
				gw->gw_Mode = GWMODE_GAME;
				success = TRUE;
			}
			break;
		case GWMODE_LEVEL:
			if( HO_SussWindowSizing( gw->gw_sw.sw_hos, gw->gw_LevelLayout ) )
			{
				gw->gw_LevelLayout[ LGID_NAME ].ho_Attr0 =
					(LONG)gw->gw_SelectedLevel->lc_Name;
				gw->gw_LevelLayout[ LGID_COMMENT ].ho_Attr0 =
					(LONG)gw->gw_SelectedLevel->lc_LevelFileHeader.lfh_Comment;
				gw->gw_LevelLayout[ LGID_FILES ].ho_Attr0 =
					(LONG)&gw->gw_SelectedLevel->lc_Files;

				gw->gw_LevelLayout[ LGID_LEVELTYPE ].ho_Value =
					gw->gw_SelectedLevel->lc_LevelFileHeader.lfh_LevelType;

				PickFile( gw, 0 );

				HO_MakeLayout( gw->gw_LevelLayout, gw->gw_sw.sw_hos,
					win->BorderLeft, win->BorderTop,
					win->Width - win->BorderLeft - win->BorderRight,
					win->Height - win->BorderTop - win->BorderBottom );

				gw->gw_Mode = GWMODE_LEVEL;
				success = TRUE;
			}
			break;
	}

	return( success );
}


/********************************************************/
/* Stuff for GWMODE_GAME																*/
/********************************************************/

static VOID PickLevel( struct GAMEWindow *gw, UWORD num )
{
	struct GameConfig *gc;
	struct LevelConfig *lc;

	gc = (struct GameConfig *)gw->gw_Chunk->ch_Data;

	lc = NULL;
	if( num != 0xFFFF )
		lc = (struct LevelConfig *)FindNthNode( &gc->gc_Levels, num );

	gw->gw_SelectedLevel = lc;
}


static VOID AddLevel( struct GAMEWindow *gw )
{
	struct GameConfig *gc;
	struct LevelConfig *lc;
	UWORD foo;

	gc = (struct GameConfig *)gw->gw_Chunk->ch_Data;

	if( lc = AllocVec( sizeof( struct LevelConfig ), MEMF_ANY|MEMF_CLEAR ) )
	{
		/* detach level list from listview */
		gw->gw_GameLayout[ GGID_LEVELS ].ho_Attr0 =
			(LONG)~0;
		HO_RefreshObject( &gw->gw_GameLayout[ GGID_LEVELS ] , gw->gw_sw.sw_hos );

		Mystrncpy( lc->lc_Name, defname, LEVELFULLNAMESIZE-1 );
		lc->lc_Node.ln_Name = lc->lc_Name;
		NewList( &lc->lc_Files );

		AddTail( &gc->gc_Levels, &lc->lc_Node );

		foo = CountNodes( &gc->gc_Levels );
		PickLevel( gw, foo-1 );
		gw->gw_GameLayout[ GGID_LEVELS ].ho_Value = (LONG)foo-1;

		/* update listview */
		gw->gw_GameLayout[ GGID_LEVELS ].ho_Attr0 =
			(LONG)&gc->gc_Levels;
		HO_RefreshObject( &gw->gw_GameLayout[ GGID_LEVELS ] , gw->gw_sw.sw_hos );
	}
}

static VOID EditLevel( struct GAMEWindow *gw )
{
	if( gw->gw_SelectedLevel )
		SetGameWinMode( gw, GWMODE_LEVEL );
}


static VOID DeleteLevel( struct GAMEWindow *gw )
{
	struct GameConfig *gc;
	struct LevelConfig *lc;

	gc = (struct GameConfig *)gw->gw_Chunk->ch_Data;
	assert( gc != NULL );

	/* detach list from listview */
	gw->gw_GameLayout[ GGID_LEVELS ].ho_Attr0 = (LONG)~0;
	HO_RefreshObject( &gw->gw_GameLayout[ GGID_LEVELS ] , gw->gw_sw.sw_hos );

	if( lc = gw->gw_SelectedLevel )
	{
		Remove( &lc->lc_Node );
		FreeNodes( &lc->lc_Files );
		FreeVec( lc );
		gw->gw_GameLayout[ GGID_LEVELS ].ho_Value = (LONG)~0;
		PickLevel( gw, 0xFFFF );
	}

	/* update listview */
	gw->gw_GameLayout[ GGID_LEVELS ].ho_Attr0 =
		(LONG)&gc->gc_Levels;
	HO_RefreshObject( &gw->gw_GameLayout[ GGID_LEVELS ] , gw->gw_sw.sw_hos );
}



static VOID BubbleLevel( struct GAMEWindow *gw, WORD dir )
{
	struct GameConfig *gc;
	struct LevelConfig *lc;
	UWORD foo;

	gc = (struct GameConfig *)gw->gw_Chunk->ch_Data;
	assert( gc != NULL );
	assert( (dir == 1) || (dir == -1) );

	/* detach list from listview */
	gw->gw_GameLayout[ GGID_LEVELS ].ho_Attr0 = (LONG)~0;
	HO_RefreshObject( &gw->gw_GameLayout[ GGID_LEVELS ] , gw->gw_sw.sw_hos );

	if( lc = gw->gw_SelectedLevel )
	{
		if( dir == 1 )
			BubbleDown( &gc->gc_Levels, &lc->lc_Node );
		else
			BubbleUp( &gc->gc_Levels, &lc->lc_Node );
		foo = FindNodeNum( &gc->gc_Levels, &lc->lc_Node );
		gw->gw_GameLayout[ GGID_LEVELS ].ho_Value = (LONG)foo;
		PickLevel( gw, foo );
	}

	/* update listview */
	gw->gw_GameLayout[ GGID_LEVELS ].ho_Attr0 =
		(LONG)&gc->gc_Levels;
	HO_RefreshObject( &gw->gw_GameLayout[ GGID_LEVELS ] , gw->gw_sw.sw_hos );
}





/********************************************************/
/* Stuff for GWMODE_LEVEL																*/
/********************************************************/

static VOID PickFile( struct GAMEWindow *gw, UWORD num )
{
	struct LevelConfig *lc;
	struct LFile *lf;

	lc = gw->gw_SelectedLevel;
	assert( lc != NULL );

	lf = NULL;
	if( num != 0xFFFF )
		lf = (struct LFile *)FindNthNode( &lc->lc_Files, num );

	if( gw->gw_SelectedFile = lf )
	{
		gw->gw_LevelLayout[ LGID_FILE ].ho_Attr0 = (LONG)lf->lf_Name;
		gw->gw_LevelLayout[ LGID_FILE ].ho_HOFlags &= ~HOFLG_DISABLED;
		HO_RefreshObject( &gw->gw_LevelLayout[ LGID_FILE ] , gw->gw_sw.sw_hos );

		gw->gw_LevelLayout[ LGID_FILETYPE ].ho_Value = (LONG)lf->lf_Type;
		gw->gw_LevelLayout[ LGID_FILETYPE ].ho_HOFlags &= ~HOFLG_DISABLED;
		HO_RefreshObject( &gw->gw_LevelLayout[ LGID_FILETYPE ] , gw->gw_sw.sw_hos );
	}
	else
	{
		gw->gw_LevelLayout[ LGID_FILE ].ho_Attr0 = NULL;
		gw->gw_LevelLayout[ LGID_FILE ].ho_HOFlags |= HOFLG_DISABLED;
		HO_RefreshObject( &gw->gw_LevelLayout[ LGID_FILE ] , gw->gw_sw.sw_hos );

		gw->gw_LevelLayout[ LGID_FILETYPE ].ho_Value = 0;
		gw->gw_LevelLayout[ LGID_FILETYPE ].ho_HOFlags |= HOFLG_DISABLED;
		HO_RefreshObject( &gw->gw_LevelLayout[ LGID_FILETYPE ] , gw->gw_sw.sw_hos );

	}
}

static VOID AddFiles( struct GAMEWindow *gw )
{
	struct LevelConfig *lc;
	struct LFile *lf;
	int i;
	BOOL allok;

	lc = gw->gw_SelectedLevel;
	assert( lc != NULL );

	/* detach list from listview */
	gw->gw_LevelLayout[ LGID_FILES ].ho_Attr0 = (LONG)~0;
	HO_RefreshObject( &gw->gw_LevelLayout[ LGID_FILES ] , gw->gw_sw.sw_hos );

	if( BlockAllSuperWindows() )
	{
		if( AslRequestTags( filereq,
			ASLFR_Window, gw->gw_sw.sw_Window,
			ASLFR_TitleText, "Add Files",
			ASLFR_DoPatterns, TRUE,
			ASLFR_InitialPattern, "~(#?.info|#?.bak)",
			ASLFR_DoMultiSelect, TRUE,
			ASLFR_DoSaveMode, FALSE,
			TAG_DONE ) )
		{
			for( i=0; i< filereq->fr_NumArgs; i++ )
			{
				if( lf = AllocVec( sizeof( struct LFile ), MEMF_ANY|MEMF_CLEAR ) )
				{
					Mystrncpy( lf->lf_Name, filereq->fr_Drawer, MAXFILENAMESIZE-1 );
					if( AddPart( lf->lf_Name, filereq->fr_ArgList[i].wa_Name,
						MAXFILENAMESIZE-1) )
					{
						lf->lf_Node.ln_Name = lf->lf_Name;
						lf->lf_Type = IDFile( lf->lf_Name );
						AddTail( &lc->lc_Files, &lf->lf_Node );
					}
					else
						FreeVec(lf);
				}
			}
		}
		UnblockAllSuperWindows();
	}
	/* update listview */
	gw->gw_LevelLayout[ LGID_FILES ].ho_Attr0 =
		(LONG)&lc->lc_Files;
	HO_RefreshObject( &gw->gw_LevelLayout[ LGID_FILES ] , gw->gw_sw.sw_hos );

	PickFile( gw, (WORD)gw->gw_LevelLayout[ LGID_FILES ].ho_Value );
}


static VOID FuckOffFile( struct GAMEWindow *gw )
{
	struct LevelConfig *lc;
	struct LFile *lf;

	lc = gw->gw_SelectedLevel;
	assert( lc != NULL );

	/* detach list from listview */
	gw->gw_LevelLayout[ LGID_FILES ].ho_Attr0 = (LONG)~0;
	HO_RefreshObject( &gw->gw_LevelLayout[ LGID_FILES ] , gw->gw_sw.sw_hos );

	if( lf = gw->gw_SelectedFile )
	{
		Remove( &lf->lf_Node );
		FreeVec( lf );
		gw->gw_LevelLayout[ LGID_FILES ].ho_Value = (LONG)~0;
		PickFile( gw, 0xFFFF );
	}

	/* update listview */
	gw->gw_LevelLayout[ LGID_FILES ].ho_Attr0 =
		(LONG)&lc->lc_Files;
	HO_RefreshObject( &gw->gw_LevelLayout[ LGID_FILES ] , gw->gw_sw.sw_hos );

}


static VOID BubbleFile( struct GAMEWindow *gw, WORD dir )
{
	struct LevelConfig *lc;
	struct LFile *lf;
	UWORD foo;

	lc = gw->gw_SelectedLevel;
	assert( lc != NULL );
	assert( ( dir == 1 ) || ( dir == -1 ) );

	/* detach list from listview */
	gw->gw_LevelLayout[ LGID_FILES ].ho_Attr0 = (LONG)~0;
	HO_RefreshObject( &gw->gw_LevelLayout[ LGID_FILES ] , gw->gw_sw.sw_hos );

	if( lf = gw->gw_SelectedFile )
	{
		if( dir == 1 )
			BubbleDown( &lc->lc_Files, &lf->lf_Node );
		else
			BubbleUp( &lc->lc_Files, &lf->lf_Node );

		foo = FindNodeNum( &lc->lc_Files, &lf->lf_Node );
		gw->gw_LevelLayout[ LGID_FILES ].ho_Value = (LONG)foo;
		PickFile( gw, foo );
	}

	/* update listview */
	gw->gw_LevelLayout[ LGID_FILES ].ho_Attr0 =
		(LONG)&lc->lc_Files;
	HO_RefreshObject( &gw->gw_LevelLayout[ LGID_FILES ] , gw->gw_sw.sw_hos );

}


