/********************************************************/
//
// SWaveWindow.c
//
/********************************************************/

#define SWAVEWINDOW_C

#include <stdio.h>
#include <string.h>
#include <math.h>
#include <assert.h>

#include <dos/dos.h>
#include <dos/dostags.h>
#include <exec/exec.h>
#include <intuition/intuition.h>
#include <intuition/imageclass.h>
#include <graphics/gfxbase.h>
#include <graphics/gfx.h>
#include <graphics/gfxmacros.h>
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
//#include <proto/iffparse.h>

#include "hoopy.h"
#include "global.h"
#include "MenuID.h"

struct SWaveWindow
{
	struct SuperWindow		svw_sw;
	struct HoopyObject		*svw_Layout;
	struct Chunk					*svw_Chunk;
	struct DrawMapBMArgs	svw_DMArgs;				/* mapdrawing stuff */
	/* name of map (if svw_DMArgs.Map is set) */
	UBYTE									svw_MapName[ MAPNAMESIZE ];
	/* name of map (if svw_DMArgs.Blockset is set) */
	UBYTE									svw_BlocksetName[ BLOCKSETNAMESIZE ];

	struct ZScrollWave		*svw_SelectedWave;
	WORD									svw_DragObject;			/* see below */
	WORD									svw_DragXOffset;
	WORD									svw_DragYOffset;
	
	UWORD									svw_DefTrigFlags;
	UBYTE									svw_DefName[ GENERICNAMESIZE ];
	UBYTE									svw_DefFlags;
	UBYTE									svw_DefAppearCount;
};

/* types for svw_DragObject */
#define DRAGOB_NONE 0
#define DRAGOB_POS 1
#define DRAGOB_LTRIG 2
#define DRAGOB_RTRIG 3

#define MKSIZE 5

#define VIEWLEFT ( svw->svw_Layout[ GID_VIEW ].ho_Attr0 >> 16 )
#define VIEWTOP ( svw->svw_Layout[ GID_VIEW ].ho_Attr0 & 0xFFFF )
#define VIEWWIDTH ( svw->svw_Layout[ GID_VIEW ].ho_Attr1 >> 16 )
#define VIEWHEIGHT ( svw->svw_Layout[ GID_VIEW ].ho_Attr1 & 0xFFFF )

#define GID_BORING 0
#define GID_VIEW 2
#define GID_VSCROLLER 3
#define GID_HSCROLLER 5

#define GID_EDITGROUP 7

#define GID_NEW 9
#define GID_DEL 10

#define GID_TRIGGERS 11
#define GID_APPEARS 12

#define GID_TYPE 14
#define GID_NAME 15
#define GID_PICKNAME 16

static STRPTR typelabels[] = { "Dude","Fmtn", NULL };
static STRPTR triggerlabels[] = { "Both", "Left", "Right", NULL };

static struct HoopyObject gadlayout[] =
{
	{ HOTYPE_VGROUP, 100, HOFLG_RAISEDBORDER|HOFLG_FREEWIDTH,3,0,0,0, NULL, 0,GID_BORING,0,0,0 },
		{ HOTYPE_HGROUP, 100, HOFLG_NOBORDER|HOFLG_FREEWIDTH|HOFLG_FREEHEIGHT,2,0,0,0, NULL, 0,GID_BORING,0,0,0 },
			{ HOTYPE_SPACE, 100, HOFLG_RAISEDBORDER|HOFLG_FREEWIDTH|HOFLG_FREEHEIGHT, 0,0,0,0, NULL,0,GID_VIEW,0,0,0 },
			{ HOTYPE_VSCROLLER, 100, HOFLG_NOBORDER|HOFLG_FREEHEIGHT, 5,50,0,0, NULL,0,GID_VSCROLLER,0,0,0 },
		{ HOTYPE_HGROUP, 100, HOFLG_NOBORDER|HOFLG_FREEWIDTH,2,0,0,0, NULL, 0,GID_BORING,0,0,0 },
			{ HOTYPE_HSCROLLER, 100, HOFLG_NOBORDER|HOFLG_FREEWIDTH, 5,50,0,0, NULL,0,GID_HSCROLLER,0,0,0 },
			{ HOTYPE_SCROLLYSPACE, 100, HOFLG_NOBORDER, 0,0,0,0, NULL,0,GID_BORING,0,0,0 },
		{ HOTYPE_VGROUP, 100, HOFLG_NOBORDER|HOFLG_FREEWIDTH,2,0,0,0, NULL, 0,GID_EDITGROUP,0,0,0 },
			{ HOTYPE_HGROUP, 100, HOFLG_NOBORDER|HOFLG_FREEWIDTH,4,0,0,0, NULL, 0,GID_BORING,0,0,0 },
				{ HOTYPE_BUTTON, 100, HOFLG_NOBORDER,0,0,0,0, "New", 0,GID_NEW,0,0,0 },
				{ HOTYPE_BUTTON, 100, HOFLG_NOBORDER,0,0,0,0, "Del", 0,GID_DEL,0,0,0 },
				{ HOTYPE_CYCLE, 100, HOFLG_NOBORDER,(LONG)triggerlabels,0,0,0, "Trg", 0,GID_TRIGGERS,PLACETEXT_LEFT,0,0 },
				{ HOTYPE_INTEGER, 100, HOFLG_NOBORDER,3,0,0,0, "ApCnt", 0,GID_APPEARS,PLACETEXT_LEFT,0,0 },
			{ HOTYPE_HGROUP, 100, HOFLG_NOBORDER,3,0,0,0, NULL, 0,GID_BORING,0,0,0 },
				{ HOTYPE_CYCLE, 100, HOFLG_NOBORDER,(LONG)typelabels,0,0,0, NULL, 0,GID_TYPE,0,0,0 },
				{ HOTYPE_STRING, 100, HOFLG_NOBORDER,NULL,GENERICNAMESIZE-1,0,0, NULL, 0,GID_NAME,0,0,0 },
				{ HOTYPE_BUTTON, 100, HOFLG_NOBORDER,0,0,0,0, "?", 0,GID_PICKNAME,0,0,0 },
	{HOTYPE_END}
};

/* the custom menu for this window... */

#define SWAVEMENU CUSTMENU
enum { SWAVEMENU_SETREPEATS = (SWAVEMENU<<16) + 1 };

static struct NewMenu swavemenu[] = {
		{ NM_TITLE,	"SWaves",								0,	0,	0,	(APTR)SWAVEMENU },
		{ NM_ITEM,	"Set AppearCount...",	0,	0,	0,	(APTR)SWAVEMENU_SETREPEATS },
		{ NM_END, NULL, 0,	0,	0,	0,},
	};


static void IDCMPHandler( struct SuperWindow *sw,
	struct IntuiMessage *imsg );
static void HideWindow( struct SuperWindow *sw );
static BOOL ShowWindow( struct SuperWindow *sw );
static VOID KillWindow( struct SuperWindow *sw );
static BOOL RefreshMenus( struct SuperWindow *sw );
static UBYTE *MakeSWaveWinTitle( struct SuperWindow *sw );
static void ModifySWave( struct SWaveWindow *svw );
static BOOL SWaveMenuHandler( struct SuperWindow *sw, ULONG id );

static VOID DrawMapView( struct SWaveWindow *svw );
static VOID DrawOverlays( struct SWaveWindow *svw );
static VOID AddNewWave( struct SWaveWindow *svw );
static VOID SelectWave( struct SWaveWindow *svw, struct ZScrollWave *zsw );
static struct ZScrollWave *WhichWave( struct SWaveWindow *svw, WORD x, WORD y );
static VOID DrawWaveOverlay( struct SWaveWindow *svw, struct ZScrollWave *zsw );
static VOID UnDrawOverlays( struct SWaveWindow *svw );
static VOID UnDrawWaveOverlay( struct SWaveWindow *svw, struct ZScrollWave *zsw );
static VOID HandleMouseDown( struct SWaveWindow *svw, WORD mousex, WORD mousey );
static VOID HandleMouseMove( struct SWaveWindow *svw, WORD mousex, WORD mousey );
static VOID HandleMouseUp( struct SWaveWindow *svw, WORD mousex, WORD mousey );
static VOID ClipToMapView( struct SWaveWindow *svw );
static VOID ScrollMapView( struct SWaveWindow *svw, WORD dx, WORD dy );
static VOID HandleNameGadget( struct SWaveWindow *svw );
static VOID PickName( struct SWaveWindow *svw );
static VOID TrySussImage( struct SWaveWindow *svw, struct ZScrollWave *zsw );
static void TrySussAllImages( struct SWaveWindow *svw );

/********************************************************/

BOOL OpenNewSWaveWindow( struct Chunk *cnk )
{
	struct SuperWindow *sw;
	struct SWaveWindow *svw;
	BOOL success = FALSE;

	for( sw = (struct SuperWindow *)superwindows.lh_Head;
		sw->sw_Node.ln_Succ;
		sw = (struct SuperWindow *)sw->sw_Node.ln_Succ )
	{
		if( sw->sw_Type == SWTY_SWAVE )
		{
			svw = (struct SWaveWindow *)sw;
			if( svw->svw_Chunk == cnk )
			{
				if( sw->sw_Window )
				{
					WindowToFront( sw->sw_Window );
					ActivateWindow( sw->sw_Window );
					return( TRUE );
				}
				else
					(*sw->sw_ShowWindow)( sw );
			}
		}
	}

	LockChunk( cnk );

	if( svw = AllocVec( sizeof( struct SWaveWindow ), MEMF_ANY|MEMF_CLEAR ) )
	{
		sw = &svw->svw_sw;
		sw->sw_Handler = IDCMPHandler;
		sw->sw_HideWindow = HideWindow;
		sw->sw_ShowWindow = ShowWindow;
		sw->sw_BuggerOff	= KillWindow;
		sw->sw_RefreshMenus = RefreshMenus;
		sw->sw_MakeTitle	= MakeSWaveWinTitle;
		sw->sw_Type				= SWTY_SWAVE;
		sw->sw_Window			= NULL;
		sw->sw_Gadgets		= NULL;
		sw->sw_VisualInfo	= NULL;
		sw->sw_WinDim			= zonkfig.SuperWindowDefs[ SWTY_SWAVE ];

		svw->svw_Chunk = cnk;
		(*sw->sw_ShowWindow)( sw );
		AddTail( &superwindows, &sw->sw_Node );
		success = TRUE;
	}
	return( success );
}

/********************************************************/

static BOOL ShowWindow( struct SuperWindow *sw )
{
	BOOL success = FALSE;
	struct SWaveWindow *svw;
	struct Window *win;
	struct Screen *scr;
	struct Chunk *cnktoo;

	svw = (struct SWaveWindow *)sw;

	scr = LockGFXScreen();

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
		WA_ReportMouse,	TRUE,
		TAG_DONE,				NULL);

	UnlockGFXScreen( scr );

	if( sw->sw_Window = win )
	{
		if( !zonkfig.scfg.scfg_DefaultFont && mainwinfont )
			SetFont( win->RPort, mainwinfont );

		if( svw->svw_Layout = AllocVec( sizeof( gadlayout ), MEMF_ANY ) )
		{
			CopyMem( gadlayout, svw->svw_Layout, sizeof( gadlayout ) );
			if( sw->sw_hos = HO_GetHOStuff( win, win->RPort->Font ) )
			{

				/* Tell the window which superwindow it is attached to */
				/* (so we'll know which handler routine to call) */
				win->UserData = (BYTE *)sw;

				sw->sw_VisualInfo = sw->sw_hos->hos_vi;
				GenericMakeMenus( sw, swavemenu );

				/* we want to use our global message port for IDCMP stuff */
				win->UserPort = mainmsgport;
				ModifyIDCMP( win, IDCMP_REFRESHWINDOW|IDCMP_NEWSIZE|
					IDCMP_CLOSEWINDOW|IDCMP_CHANGEWINDOW|
					IDCMP_RAWKEY|IDCMP_MOUSEBUTTONS|IDCMP_MOUSEMOVE|IDCMP_MENUPICK|
					IDCMP_INTUITICKS|STRINGIDCMP|INTEGERIDCMP|SCROLLERIDCMP|BUTTONIDCMP );

				HO_SussWindowSizing( sw->sw_hos, svw->svw_Layout );

				/* bitmap for map drawing stuff - ok if it fails */
				svw->svw_DMArgs.bm = CreateBitMap(win->WScreen->Width,win->WScreen->Height,
					win->WScreen->RastPort.BitMap->Depth, TRUE );
				svw->svw_DMArgs.BMWidth = win->WScreen->Width;
				svw->svw_DMArgs.BMHeight = win->WScreen->Height;
				svw->svw_DMArgs.MapX = 0;
				svw->svw_DMArgs.MapY = 0;
				svw->svw_DMArgs.Flags = 0;						/* not used yet? */
				svw->svw_DMArgs.ZoomFactor = 1;				/* no zooming */

				svw->svw_DMArgs.Blockset = NULL;
				svw->svw_DMArgs.Map = NULL;

				if( cnktoo = FindChunkType( NULL, ID_MAP ) )
				{
					if( cnktoo->ch_Data )
					{
						svw->svw_DMArgs.Map = (struct Map *)cnktoo->ch_Data;
						Mystrncpy( svw->svw_MapName,
							svw->svw_DMArgs.Map->map_Name,
							MAPNAMESIZE-1 );
					}
				}
				if( cnktoo = FindChunkType( NULL, ID_BLKS ) )
				{
					if( cnktoo->ch_Data )
					{
						svw->svw_DMArgs.Blockset = (struct Blockset *)cnktoo->ch_Data;
						Mystrncpy( svw->svw_BlocksetName,
							svw->svw_DMArgs.Blockset->bs_Name,
							BLOCKSETNAMESIZE-1 );
					}
				}

				HO_MakeLayout( svw->svw_Layout, sw->sw_hos, win->BorderLeft, win->BorderTop,
					win->Width - win->BorderLeft - win->BorderRight,
					win->Height - win->BorderTop - win->BorderBottom );

				TrySussAllImages( svw );
				DrawMapView( svw );
				SelectWave( svw, NULL );
				DrawOverlays( svw );

				success = TRUE;
			}
		}
	}

	if( !success )
	{
		if( sw->sw_Window ) CloseWindow( sw->sw_Window );
		sw->sw_Window = NULL;
		if( svw->svw_Layout ) FreeVec( svw->svw_Layout );
		svw->svw_Layout = NULL;
	}

	return( success );
}


/********************************************************/

static void HideWindow( struct SuperWindow *sw )
{
	struct SWaveWindow *svw;
	struct Window *win;

	svw = (struct SWaveWindow *)sw;

	if( win = sw->sw_Window )
	{
		RememberWindow( sw );
		GenericZapMenus( sw );
		HO_KillLayout( sw->sw_hos );

		/* free work bitmap (used for map drawing) if it exists */
		if( svw->svw_DMArgs.bm )
		{
			DestroyBitMap( svw->svw_DMArgs.bm, win->WScreen->Width, win->WScreen->Height,
				win->WScreen->RastPort.BitMap->Depth );
			svw->svw_DMArgs.bm = NULL;
		}

		CloseWindowSafely( sw->sw_Window );
		HO_FreeHOStuff( sw->sw_hos );
		sw->sw_Window = NULL;
		FreeVec( svw->svw_Layout );
		svw->svw_Layout = NULL;
	}
}

/********************************************************/

static VOID KillWindow( struct SuperWindow *sw )
{
	if( sw && IsOKToClose( sw ) )
	{
		(*sw->sw_HideWindow)(sw);
		UnlockChunk( ((struct SWaveWindow *)sw)->svw_Chunk );
		Remove( &sw->sw_Node );
		FreeVec( sw );
	}
}

/********************************************************/
static BOOL RefreshMenus( struct SuperWindow *sw )
{
	return( FALSE );
}


/********************************************************/

static UBYTE *MakeSWaveWinTitle( struct SuperWindow *sw )
{
	struct Chunk *cnk;
	UBYTE *p;

	cnk = ((struct SWaveWindow *)sw)->svw_Chunk;
	p = sw->sw_Name;

	if( cnk->ch_Modified )
		*p++ = MODCHAR;
	strcpy( p, "Scrollwaves" );

	return( sw->sw_Name );
}



/********************************************************/
/* Replace with ModifyChunk */

static void ModifySWave( struct SWaveWindow *svw )
{
	struct Chunk *cnk;

	cnk = svw->svw_Chunk;
	if( !cnk->ch_Modified )
	{
		cnk->ch_Modified = TRUE;
		RefreshSuperWindowTitles();
	}
}



/********************************************************/

static BOOL SWaveMenuHandler( struct SuperWindow *sw, ULONG id )
{
	BOOL stop;
	LONG val;
	struct SWaveWindow *svw;
	struct ZScrollWave *zsw;

	stop = FALSE;
//	printf("id=%ld\n",id );

	svw = (struct SWaveWindow *)sw;

	switch( id )
	{
		case SWAVEMENU_SETREPEATS:
			val = svw->svw_DefAppearCount;
			if( BlockAllSuperWindows() )
			{
				if( MyIntRequest( sw->sw_Window, "Global Change",
					"Appear Count", &val ) )
				{

//					UnDrawOverlays( svw );

					for( zsw = (struct ZScrollWave *)svw->svw_Chunk->ch_DataList.lh_Head;
						zsw->zsw_Node.mln_Succ;
						zsw = (struct ZScrollWave *)zsw->zsw_Node.mln_Succ )
					{
						zsw->zsw_AppearCount = val;
					}

//					DrawOverlays( svw );
				}
				UnblockAllSuperWindows();
				/* force a redraw of the gadget values */
				SelectWave( svw, svw->svw_SelectedWave );
			}
			ModifySWave( svw );
			break;
	}

	return( stop );
}



/********************************************************/

static void IDCMPHandler( struct SuperWindow *sw,
	struct IntuiMessage *origimsg )
{
	struct IntuiMessage *imsg;
	struct Window *win;
	struct SWaveWindow *svw;

	struct ZScrollWave *zsw;

	ULONG class;
	UWORD code;
	UWORD qualifier;
	APTR iaddress;
	WORD mousex, mousey;
	ULONG seconds, micros;
	LONG temp;

	svw = (struct SWaveWindow *)sw;
	win = sw->sw_Window;

	if( imsg = GT_FilterIMsg( origimsg ) )
	{
		HO_CheckIMsg( imsg );

		class = imsg->Class;
		code = imsg->Code;
		qualifier = imsg->Qualifier;
		iaddress = imsg->IAddress;
		mousex = imsg->MouseX;
		mousey = imsg->MouseY;
		seconds = imsg->Seconds;
		micros = imsg->Micros;

		GT_PostFilterIMsg( imsg );

		switch( class )
		{
			case IDCMP_REFRESHWINDOW:
				GT_BeginRefresh(win);
				GT_EndRefresh(win,TRUE);
				ReplyMsg( (struct Message *)origimsg );
				break;
			case IDCMP_MOUSEMOVE:
				ReplyMsg( (struct Message *)origimsg );
				HandleMouseMove( svw, mousex, mousey );
				break;
			case IDCMP_INTUITICKS:
				ReplyMsg( (struct Message *)origimsg );
//				HandleMouseMove( svw, mousex, mousey );
				break;
			case IDCMP_NEWSIZE:
				ClearWindow( win );
				HO_ReadGadgetStates( sw->sw_hos, svw->svw_Layout );
				HO_MakeLayout( svw->svw_Layout, sw->sw_hos, win->BorderLeft, win->BorderTop,
					win->Width - win->BorderLeft - win->BorderRight,
					win->Height - win->BorderTop - win->BorderBottom );

				DrawMapView( svw );
				DrawOverlays( svw );

				ReplyMsg( (struct Message *)origimsg );
				break;
			case IDCMP_CHANGEWINDOW:
				UpdateWinDef( sw );
				ReplyMsg( (struct Message *)origimsg );
				break;
			case IDCMP_CLOSEWINDOW:
				ReplyMsg( (struct Message *)origimsg );
				KillWindow( sw );
				break;
			case IDCMP_MOUSEBUTTONS:
				ReplyMsg( (struct Message *)origimsg );
				switch( code )
				{
					case SELECTDOWN:
						HandleMouseDown( svw, mousex, mousey );
						break;
					case SELECTUP:
						HandleMouseUp( svw, mousex, mousey );
						break;
				}
				break;
			case IDCMP_GADGETUP:
				ReplyMsg( (struct Message *)origimsg );
				HO_ReadGadgetStates( sw->sw_hos, svw->svw_Layout );
				switch( ((struct Gadget*)iaddress)->GadgetID )
				{
					case GID_NAME:
						HandleNameGadget( svw );
						ModifySWave( svw );
						break;
					case GID_NEW:
						AddNewWave( svw );
						ModifySWave( svw );
						break;
					case GID_DEL:
						if( svw->svw_SelectedWave )
						{
							UnDrawWaveOverlay( svw, svw->svw_SelectedWave );
							Remove( (struct Node *)&svw->svw_SelectedWave->zsw_Node );
							FreeZScrollWave( svw->svw_SelectedWave );
							SelectWave( svw, NULL );
							ModifySWave( svw );
						}
						break;
					case GID_APPEARS:
						if( zsw = svw->svw_SelectedWave )
						{
							zsw->zsw_AppearCount = (UBYTE)svw->svw_Layout[ GID_APPEARS ].ho_Value;
							ModifySWave( svw );
						}
						break;
					case GID_TYPE:
						if( zsw = svw->svw_SelectedWave )
						{
							zsw->zsw_Flags = code ? zsw->zsw_Flags | 1 : zsw->zsw_Flags & ~1;
							ModifySWave( svw );
						}
						break;
					case GID_TRIGGERS:
						if( zsw = svw->svw_SelectedWave )
						{
							UnDrawWaveOverlay( svw, svw->svw_SelectedWave );
							zsw->zsw_TrigFlags = ( ~code ) & 3;
							DrawWaveOverlay( svw, svw->svw_SelectedWave );
							ModifySWave( svw );
						}
						break;
					case GID_PICKNAME:
						PickName( svw );
						ModifySWave( svw );
						break;
				}
				break;
			case IDCMP_RAWKEY:
				ReplyMsg( (struct Message *)origimsg );

				temp = 1;
				if( qualifier & IEQUALIFIER_LALT || qualifier & IEQUALIFIER_RALT )
					temp = 4;
				if( qualifier & IEQUALIFIER_LSHIFT || qualifier & IEQUALIFIER_RSHIFT )
					temp = 8;

				switch( code )
				{
					case RAWKEY_UP:
						ScrollMapView( svw, 0, 0-temp );
						break;
					case RAWKEY_DOWN:
						ScrollMapView( svw, 0, temp );
						break;
					case RAWKEY_LEFT:
						ScrollMapView( svw, 0-temp, 0 );
						break;
					case RAWKEY_RIGHT:
						ScrollMapView( svw, temp, 0 );
						break;
				}
				break;
			case IDCMP_MENUPICK:
				ReplyMsg( (struct Message *)origimsg );
				GenericHandleMenuPick( sw, code, SWaveMenuHandler );
				break;
			default:
				ReplyMsg( (struct Message *)origimsg );
				break;
		}
	}
	else
		ReplyMsg( (struct Message *)origimsg );
}

/********************************************************/

static VOID DrawMapView( struct SWaveWindow *svw )
{
	if( svw->svw_DMArgs.Blockset && svw->svw_DMArgs.Map && svw->svw_DMArgs.bm )
	{
		svw->svw_DMArgs.ViewWidth = VIEWWIDTH;
		svw->svw_DMArgs.ViewHeight = VIEWHEIGHT;
		DrawMapBM( &svw->svw_DMArgs );
		BltBitMapRastPort( svw->svw_DMArgs.bm, 0,0,svw->svw_sw.sw_Window->RPort,
			VIEWLEFT, VIEWTOP, VIEWWIDTH, VIEWHEIGHT, 0xC0 );
	}
	else
	{
		SetAPen( svw->svw_sw.sw_Window->RPort, 0 );
		RectFill( svw->svw_sw.sw_Window->RPort, VIEWLEFT, VIEWTOP,
			VIEWLEFT + VIEWWIDTH, VIEWTOP + VIEWHEIGHT );
	}

}

/********************************************************/

static VOID DrawOverlays( struct SWaveWindow *svw )
{
	struct ZScrollWave *zsw;
	struct RastPort *rp;
	struct Chunk *cnk;

	ClipToMapView( svw );

	rp = svw->svw_sw.sw_Window->RPort;
	if( cnk = svw->svw_Chunk )
	{
		for( zsw = (struct ZScrollWave *)cnk->ch_DataList.lh_Head;
			zsw->zsw_Node.mln_Succ;
			zsw = (struct ZScrollWave *)zsw->zsw_Node.mln_Succ )
		{
			DrawWaveOverlay( svw, zsw );
		}
	}
	UnclipWindow( svw->svw_sw.sw_Window );
}


/********************************************************/

static VOID DrawWaveOverlay( struct SWaveWindow *svw, struct ZScrollWave *zsw )
{
	struct RastPort *rp;
	WORD x,y;

	rp = svw->svw_sw.sw_Window->RPort;

	if( zsw == svw->svw_SelectedWave )
	{
		SetAPen( rp, 1 );
//svw->svw_DragObject
//#define DRAGOB_NONE 0
//#define DRAGOB_POS 1
//#define DRAGOB_LTRIG 2
//#define DRAGOB_RTRIG 3

		if( zsw->zsw_TrigFlags & 2 )
		{
			x = VIEWLEFT + zsw->zsw_LfTrigger - ( svw->svw_DMArgs.MapX * BLKW );
			Move( rp, x, VIEWTOP );
			Draw( rp, x, VIEWTOP+VIEWHEIGHT );
			RectFill(rp, x,VIEWTOP, x+MKSIZE, VIEWTOP+MKSIZE );
		}
		if( zsw->zsw_TrigFlags & 1 )
		{
			x = VIEWLEFT + zsw->zsw_RtTrigger - ( svw->svw_DMArgs.MapX * BLKW );
			Move( rp, x, VIEWTOP );
			Draw( rp, x, VIEWTOP+VIEWHEIGHT );
			RectFill(rp, x-MKSIZE, VIEWTOP+MKSIZE+1, x, VIEWTOP+MKSIZE+MKSIZE+1 );
		}
	}
	else
		SetAPen( rp, 2 );

	x = VIEWLEFT + zsw->zsw_XPos - ( svw->svw_DMArgs.MapX * BLKW );
	y = VIEWTOP + zsw->zsw_YPos - ( svw->svw_DMArgs.MapY * BLKH );

	if( zsw->zsw_Image )
	{
		x += zsw->zsw_ImageXOffset;
		y += zsw->zsw_ImageYOffset;
		BltBobRastPort( rp, zsw->zsw_Image, x, y );
		zsw->zsw_ImageW = zsw->zsw_Image->bin_BobHeader->bf_Width;
		zsw->zsw_ImageH = zsw->zsw_Image->bin_BobHeader->bf_Height;
	}
	else
	{
		Move( rp, x, y-MKSIZE );
		Draw( rp, x, y+MKSIZE );
		Move( rp, x-MKSIZE, y );
		Draw( rp, x+MKSIZE, y );
	}
}

/********************************************************/

static VOID UnDrawOverlays( struct SWaveWindow *svw )
{
	struct ZScrollWave *zsw;
	struct RastPort *rp;
	struct Chunk *cnk;

	ClipToMapView( svw );

	rp = svw->svw_sw.sw_Window->RPort;
	if( cnk = svw->svw_Chunk )
	{
		for( zsw = (struct ZScrollWave *)cnk->ch_DataList.lh_Head;
			zsw->zsw_Node.mln_Succ;
			zsw = (struct ZScrollWave *)zsw->zsw_Node.mln_Succ )
		{
			UnDrawWaveOverlay( svw, zsw );
		}
	}
	UnclipWindow( svw->svw_sw.sw_Window );
}

/********************************************************/

static VOID UnDrawWaveOverlay( struct SWaveWindow *svw, struct ZScrollWave *zsw )
{
	struct RastPort *rp;
	WORD x,y;

	rp = svw->svw_sw.sw_Window->RPort;

	if( svw->svw_DMArgs.bm && svw->svw_DMArgs.Blockset && svw->svw_DMArgs.Map )
	{

		if( svw->svw_SelectedWave == zsw )
		{
			if( zsw->zsw_TrigFlags & 2 )
			{
				x = zsw->zsw_LfTrigger - ( svw->svw_DMArgs.MapX * BLKW );
				BltBitMapRastPort( svw->svw_DMArgs.bm, x,0,svw->svw_sw.sw_Window->RPort,
					VIEWLEFT+x, VIEWTOP, MKSIZE+1, VIEWHEIGHT, 0xC0 );
			}
			if( zsw->zsw_TrigFlags & 1 )
			{
				x = zsw->zsw_RtTrigger - ( svw->svw_DMArgs.MapX * BLKW );
				BltBitMapRastPort( svw->svw_DMArgs.bm, x-MKSIZE,0,svw->svw_sw.sw_Window->RPort,
					VIEWLEFT+x-MKSIZE, VIEWTOP, MKSIZE+1, VIEWHEIGHT, 0xC0 );
			}
		}

		x = zsw->zsw_XPos - ( svw->svw_DMArgs.MapX * BLKW );
		y = zsw->zsw_YPos - ( svw->svw_DMArgs.MapY * BLKH );

		if( zsw->zsw_Image )
		{
			x += zsw->zsw_ImageXOffset;
			y += zsw->zsw_ImageYOffset;
			BltBitMapRastPort( svw->svw_DMArgs.bm, x, y, rp,
				VIEWLEFT+x, VIEWTOP+y, zsw->zsw_ImageW, zsw->zsw_ImageH, 0xC0 );
		}
		else
		{
			BltBitMapRastPort( svw->svw_DMArgs.bm, x-MKSIZE,y-MKSIZE, rp,
				VIEWLEFT+x-MKSIZE, VIEWTOP+y-MKSIZE, MKSIZE+MKSIZE+1, MKSIZE+MKSIZE+1, 0xC0 );
		}
	}
	else
	{
		/* no background bitmap */

		SetAPen( rp, 0 );
		if( zsw == svw->svw_SelectedWave )
		{
			if( zsw->zsw_TrigFlags & 2 )
			{
				x = VIEWLEFT + zsw->zsw_LfTrigger - ( svw->svw_DMArgs.MapX * BLKW );
				Move( rp, x, VIEWTOP );
				Draw( rp, x, VIEWTOP+VIEWHEIGHT );
				RectFill(rp, x,VIEWTOP, x+MKSIZE, VIEWTOP+MKSIZE );
			}
			if( zsw->zsw_TrigFlags & 1 )
			{
				x = VIEWLEFT + zsw->zsw_RtTrigger - ( svw->svw_DMArgs.MapX * BLKW );
				Move( rp, x, VIEWTOP );
				Draw( rp, x, VIEWTOP+VIEWHEIGHT );
				RectFill(rp, x-MKSIZE, VIEWTOP+MKSIZE+1, x, VIEWTOP+MKSIZE+MKSIZE+1 );
			}
		}
		x = VIEWLEFT + zsw->zsw_XPos - ( svw->svw_DMArgs.MapX * BLKW );
		y = VIEWTOP + zsw->zsw_YPos - ( svw->svw_DMArgs.MapY * BLKH );

		if( zsw->zsw_Image )
		{
			RectFill(rp, x, y, x + zsw->zsw_ImageW, y + zsw->zsw_ImageH );
		}
		else
		{
			Move( rp, x, y-MKSIZE );
			Draw( rp, x, y+MKSIZE );
			Move( rp, x-MKSIZE, y );
			Draw( rp, x+MKSIZE, y );
		}

	}
}



/********************************************************/
// luser clicked on the New gadget

static VOID AddNewWave( struct SWaveWindow *svw )
{
	struct ZScrollWave *zsw;

//	printf("AddNewWave()\n");

	/* read out default values */
	if( svw->svw_SelectedWave )
		SelectWave( svw, svw->svw_SelectedWave );

	if( zsw = AllocZScrollWave() )
	{
//		printf("Done Alloc\n");

		zsw->zsw_XPos = (svw->svw_DMArgs.MapX * BLKW) + VIEWWIDTH/2;
		zsw->zsw_YPos = (svw->svw_DMArgs.MapY * BLKH) + VIEWHEIGHT/2;
		zsw->zsw_LfTrigger = zsw->zsw_XPos - 32;
		zsw->zsw_RtTrigger = zsw->zsw_XPos + 32;
		/* set up default values */
		zsw->zsw_TrigFlags = svw->svw_DefTrigFlags;
		Mystrncpy( zsw->zsw_Name, svw->svw_DefName, GENERICNAMESIZE-1 );
		zsw->zsw_Flags = svw->svw_DefFlags;
		zsw->zsw_AppearCount = svw->svw_DefAppearCount;

		UnDrawOverlays( svw );
		AddTail( &svw->svw_Chunk->ch_DataList, (struct Node *)&zsw->zsw_Node );
		TrySussImage( svw, zsw );
		SelectWave( svw, zsw );
		DrawOverlays( svw );

		/* redraw stuff here */
	}
}


/********************************************************/

static VOID SelectWave( struct SWaveWindow *svw, struct ZScrollWave *zsw )
{
	struct Chunk *cnk;
	struct HOStuff *hos;

	svw->svw_SelectedWave = zsw;
	hos = svw->svw_sw.sw_hos;

	if( zsw )
	{
		/* move to tail of list */
		cnk = svw->svw_Chunk;
		Remove( (struct Node *)&zsw->zsw_Node );
		AddTail( &cnk->ch_DataList, (struct Node *)&zsw->zsw_Node );

		svw->svw_Layout[ GID_DEL ].ho_HOFlags &= ~HOFLG_DISABLED;
		svw->svw_Layout[ GID_TRIGGERS ].ho_HOFlags &= ~HOFLG_DISABLED;
		svw->svw_Layout[ GID_APPEARS ].ho_HOFlags &= ~HOFLG_DISABLED;
		svw->svw_Layout[ GID_TYPE ].ho_HOFlags &= ~HOFLG_DISABLED;
		svw->svw_Layout[ GID_NAME ].ho_HOFlags &= ~HOFLG_DISABLED;
		svw->svw_Layout[ GID_PICKNAME ].ho_HOFlags &= ~HOFLG_DISABLED;

		svw->svw_Layout[ GID_NAME ].ho_Attr0 = (LONG)zsw->zsw_Name;
		svw->svw_Layout[ GID_APPEARS ].ho_Value = (LONG)zsw->zsw_AppearCount;
		svw->svw_Layout[ GID_TRIGGERS ].ho_Value = ( ~zsw->zsw_TrigFlags ) & 3;
		svw->svw_Layout[ GID_TYPE ].ho_Value = ( zsw->zsw_Flags ) & 1;

		HO_RefreshObject( &svw->svw_Layout[ GID_EDITGROUP ], hos );

		/* set default values */
		svw->svw_DefTrigFlags = zsw->zsw_TrigFlags;
		Mystrncpy( svw->svw_DefName, zsw->zsw_Name, GENERICNAMESIZE-1 );
		svw->svw_DefFlags = zsw->zsw_Flags;
		svw->svw_DefAppearCount = zsw->zsw_AppearCount;
	}
	else
	{
		svw->svw_Layout[ GID_DEL ].ho_HOFlags |= HOFLG_DISABLED;
		svw->svw_Layout[ GID_TRIGGERS ].ho_HOFlags |= HOFLG_DISABLED;
		svw->svw_Layout[ GID_APPEARS ].ho_HOFlags |= HOFLG_DISABLED;
		svw->svw_Layout[ GID_TYPE ].ho_HOFlags |= HOFLG_DISABLED;
		svw->svw_Layout[ GID_NAME ].ho_HOFlags |= HOFLG_DISABLED;
		svw->svw_Layout[ GID_PICKNAME ].ho_HOFlags |= HOFLG_DISABLED;

		svw->svw_Layout[ GID_NAME ].ho_Attr0 = NULL;
		svw->svw_Layout[ GID_APPEARS ].ho_Value = 0L;
		svw->svw_Layout[ GID_TYPE ].ho_Value = 0;

		HO_RefreshObject( &svw->svw_Layout[ GID_EDITGROUP ], hos );
	}
}



/********************************************************/

static struct ZScrollWave *WhichWave( struct SWaveWindow *svw, WORD x,
	WORD y )
{
	struct ZScrollWave *zsw, *match;
	struct Chunk *cnk;

	cnk = svw->svw_Chunk;
	match = NULL;

	for( zsw = (struct ZScrollWave *)cnk->ch_DataList.lh_Head;
		zsw->zsw_Node.mln_Succ && !match;
		zsw = (struct ZScrollWave *)zsw->zsw_Node.mln_Succ )
	{
		if( zsw->zsw_Image )
		{
			if( x >= ( zsw->zsw_XPos + zsw->zsw_ImageXOffset ) &&
				x <= ( zsw->zsw_XPos  + zsw->zsw_ImageXOffset + zsw->zsw_ImageW ) &&
				y >= ( zsw->zsw_YPos + zsw->zsw_ImageYOffset) &&
				y <= ( zsw->zsw_YPos + zsw->zsw_ImageYOffset + zsw->zsw_ImageH ) )
			{
				match = zsw;
			}
		}
		else
		{
			if( x >= ( zsw->zsw_XPos - MKSIZE ) &&
				x <= ( zsw->zsw_XPos + MKSIZE ) &&
				y >= ( zsw->zsw_YPos - MKSIZE ) &&
				y <= ( zsw->zsw_YPos + MKSIZE ) )
			{
				match = zsw;
			}
		}
	}
	return( match );
}


/********************************************************/

static VOID HandleMouseDown( struct SWaveWindow *svw, WORD mousex, WORD mousey )
{
	struct ZScrollWave *zsw;
	WORD x,y;
	BOOL emptyspace;

	emptyspace = TRUE;
	x = mousex - VIEWLEFT + ( svw->svw_DMArgs.MapX * BLKW );
	y = mousey - VIEWTOP + ( svw->svw_DMArgs.MapY * BLKH );

	/* clicked within mapview */

	if( mousex >= VIEWLEFT && mousex < VIEWLEFT+VIEWWIDTH &&
		mousey >= VIEWTOP && mousey < VIEWTOP+VIEWHEIGHT )
	{

		if( zsw = WhichWave( svw, x, y ) )
		{
			/* wave under mouse */
			emptyspace = FALSE;
			if( zsw != svw->svw_SelectedWave )
			{
				UnDrawOverlays( svw );
				SelectWave( svw, zsw );
				DrawOverlays( svw );
			}
			svw->svw_DragObject = DRAGOB_POS;
			svw->svw_DragXOffset = zsw->zsw_XPos - x;
			svw->svw_DragYOffset = zsw->zsw_YPos - y;
		}
		else
		{
			/* no wave under mouse */
			if( zsw = svw->svw_SelectedWave )
			{
				/* check for left trigger dragtag */
				if( ( zsw->zsw_TrigFlags & 2 ) &&
					x >= zsw->zsw_LfTrigger && x < zsw->zsw_LfTrigger + MKSIZE &&
					mousey >= VIEWTOP && mousey < VIEWTOP + MKSIZE )
				{
					emptyspace = FALSE;
					svw->svw_DragObject = DRAGOB_LTRIG;
					svw->svw_DragXOffset = zsw->zsw_LfTrigger - x;
				}

				/* check for right trigger dragtag */
				if( ( zsw->zsw_TrigFlags & 1 ) &&
					x >= zsw->zsw_RtTrigger-MKSIZE && x <= zsw->zsw_RtTrigger &&
					mousey >= VIEWTOP+MKSIZE+1 && mousey < VIEWTOP + MKSIZE+MKSIZE+1 )
				{
					emptyspace = FALSE;
					svw->svw_DragObject = DRAGOB_RTRIG;
					svw->svw_DragXOffset = zsw->zsw_RtTrigger - x;
				}
				emptyspace = FALSE;
			}
		}

		if( emptyspace )
		{
			/* clicked on empty space */
			UnDrawOverlays( svw );
			SelectWave( svw, NULL );
			DrawOverlays( svw );
		}
	}
}

/********************************************************/

static VOID HandleMouseMove( struct SWaveWindow *svw, WORD mousex, WORD mousey )
{
	struct ZScrollWave *zsw;
	LONG temp;

	if( svw->svw_DragObject && ( zsw = svw->svw_SelectedWave ) )
	{
		ClipToMapView( svw );
		UnDrawWaveOverlay( svw, zsw );
		switch( svw->svw_DragObject )
		{
			case DRAGOB_POS:
				zsw->zsw_XPos = svw->svw_DragXOffset +
					mousex - VIEWLEFT + ( svw->svw_DMArgs.MapX * BLKW );
				zsw->zsw_YPos = svw->svw_DragYOffset +
					mousey - VIEWTOP + ( svw->svw_DMArgs.MapY * BLKH );
				break;
			case DRAGOB_LTRIG:
				temp = svw->svw_DragXOffset +
					mousex - VIEWLEFT + ( svw->svw_DMArgs.MapX * BLKW );
				if( temp < 0 ) temp = 0;
				if( temp > 0xFFFF ) temp = 0xFFFF;
				zsw->zsw_LfTrigger = (UWORD)temp;
				break;
			case DRAGOB_RTRIG:
				temp = svw->svw_DragXOffset +
					mousex - VIEWLEFT + ( svw->svw_DMArgs.MapX * BLKW );
				if( temp < 0 ) temp = 0;
				if( temp > 0xFFFF ) temp = 0xFFFF;
				zsw->zsw_RtTrigger = (UWORD)temp;
				break;
		}
		DrawWaveOverlay( svw, zsw );
		UnclipWindow( svw->svw_sw.sw_Window );
		ModifySWave( svw );
	}
}

/********************************************************/

static VOID HandleMouseUp( struct SWaveWindow *svw, WORD mousex, WORD mousey )
{
	if( svw->svw_DragObject )
	{
		UnDrawOverlays( svw );
		DrawOverlays( svw );
		svw->svw_DragObject = DRAGOB_NONE;
	}
}

/********************************************************/

static VOID ClipToMapView( struct SWaveWindow *svw )
{
	struct Region *oldregion;

	if( oldregion = ClipWindow( svw->svw_sw.sw_Window, VIEWLEFT, VIEWTOP,
		VIEWLEFT + VIEWWIDTH - 1, VIEWTOP + VIEWHEIGHT - 1 ) )
	{
		DisposeRegion( oldregion );
	}
}

/********************************************************/

static VOID ScrollMapView( struct SWaveWindow *svw, WORD dx, WORD dy )
{

	svw->svw_DMArgs.MapX += dx;
	svw->svw_DMArgs.MapY += dy;

	DrawMapView( svw );
	DrawOverlays( svw );
}

/********************************************************/
static void TrySussAllImages( struct SWaveWindow *svw )
{
	struct ZScrollWave *zsw;
	struct Chunk *cnk;

	cnk = svw->svw_Chunk;
	assert( cnk != NULL );
	assert( cnk->ch_TypeID == ID_SWAV );
	for( zsw = (struct ZScrollWave *)cnk->ch_DataList.lh_Head;
		zsw->zsw_Node.mln_Succ;
		zsw = (struct ZScrollWave *)zsw->zsw_Node.mln_Succ )
	{
		TrySussImage( svw, zsw );
	}
}



/********************************************************/
static VOID TrySussImage( struct SWaveWindow *svw, struct ZScrollWave *zsw )
{
	struct Program *prog;
	struct BobInfoNode *bin;
	LONG offsetbuffer[ 2 ];

	if( !(zsw->zsw_Flags & 1) )
	{
		/* zsw_Name is a program */
		if( prog = GlobalFindProg( zsw->zsw_Name ) )
		{
			if( bin = FindImageFromProgram( prog, offsetbuffer, TRUE ) )
			{
//				D(bug( "  image: '%s'\n", bin->bin_Name ) );
				zsw->zsw_Image = bin;
				zsw->zsw_ImageXOffset = (WORD)offsetbuffer[0];
				zsw->zsw_ImageYOffset = (WORD)offsetbuffer[1];
			}
			else
				zsw->zsw_Image = NULL;
		}
	}
}

/********************************************************/
static VOID HandleNameGadget( struct SWaveWindow *svw )
{
	// struct Program *prog;
	struct BobInfoNode *bin;
	struct ZScrollWave *zsw;
	// LONG offsetbuffer[ 2 ];

	if( zsw = svw->svw_SelectedWave )
	{
		bin = NULL;
		MakePackable( zsw->zsw_Name );
		HO_RefreshObject( &svw->svw_Layout[ GID_NAME ], svw->svw_sw.sw_hos );
		ClipToMapView( svw );
		UnDrawWaveOverlay( svw, zsw );
		TrySussImage( svw, zsw );
		DrawWaveOverlay( svw, zsw );
		UnclipWindow( svw->svw_sw.sw_Window );
	}
}


/********************************************************/

static VOID PickName( struct SWaveWindow *svw )
{
	// struct Formation *fm;
	struct Program *prog;
	struct ZScrollWave *zsw;
	BOOL	refresh;

	refresh = FALSE;
	if( zsw = svw->svw_SelectedWave )
	{
		if( BlockAllSuperWindows() )
		{
			if( !( zsw->zsw_Flags & 1 ) )
			{
				if( prog = RequestProgram( svw->svw_sw.sw_Window, NULL, "Pick Program", "Programs" ) )
				{
					Mystrncpy( zsw->zsw_Name, prog->pg_Name, GENERICNAMESIZE-1 );
					refresh = TRUE;
				}
			}
			UnblockAllSuperWindows();
		}
	}
	if( refresh )
		HandleNameGadget( svw );
}



/********************************************************/
// Go through all the SWave windows and call TrySussAllImages()

void RethinkSWaveBobImages( void )
{
	struct SuperWindow *sw;

	for( sw = (struct SuperWindow *)superwindows.lh_Head;
		sw->sw_Node.ln_Succ;
		sw = (struct SuperWindow *)sw->sw_Node.ln_Succ )
	{
		if( sw->sw_Type == SWTY_SWAVE )
		{
			TrySussAllImages( (struct SWaveWindow *)sw );
			DrawMapView( (struct SWaveWindow *)sw );
			DrawOverlays( (struct SWaveWindow *)sw );
		}
	}
}


void RethinkSWaveBackgrounds( void )
{
	struct SuperWindow *sw;
	struct SWaveWindow *svw;
	struct Chunk *cnk;

	for( sw = (struct SuperWindow *)superwindows.lh_Head;
		sw->sw_Node.ln_Succ;
		sw = (struct SuperWindow *)sw->sw_Node.ln_Succ )
	{
		if( sw->sw_Type == SWTY_SWAVE )
		{
			svw = (struct SWaveWindow *)sw;

			if( svw->svw_DMArgs.Map )
			{
				cnk = FindMAPByName( svw->svw_MapName );
				if( cnk )
					svw->svw_DMArgs.Map = (struct Map *)cnk->ch_Data;
				else
					svw->svw_DMArgs.Map = NULL;
			}

			if( svw->svw_DMArgs.Blockset )
			{
				cnk = FindBLKSByName( svw->svw_BlocksetName );
				if( cnk )
					svw->svw_DMArgs.Blockset = (struct Blockset *)cnk->ch_Data;
				else
					svw->svw_DMArgs.Blockset = NULL;
			}

			DrawMapView( svw );
			DrawOverlays( svw );
		}
	}
}
