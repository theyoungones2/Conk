/********************************************************/
//
// THAPWindow.c
//
/********************************************************/

#define THAPWINDOW_C

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>

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
#include <proto/iffparse.h>

#include "hoopy.h"
#include "global.h"

#define MKSIZE 2

#define VIEWLEFT ( thw->thw_Layout[ GID_VIEWSPACE ].ho_Attr0 >> 16 )
#define VIEWTOP ( thw->thw_Layout[ GID_VIEWSPACE ].ho_Attr0 & 0xFFFF )
#define VIEWWIDTH ( thw->thw_Layout[ GID_VIEWSPACE ].ho_Attr1 >> 16 )
#define VIEWHEIGHT ( thw->thw_Layout[ GID_VIEWSPACE ].ho_Attr1 & 0xFFFF )

#define WIN (thw->thw_sw.sw_Window)
#define RP (thw->thw_sw.sw_Window->RPort)

struct THAPWindow
{
	struct SuperWindow		thw_sw;
	struct Chunk					*thw_Chunk;
	struct HoopyObject		*thw_Layout;
	struct Path						*thw_CurrentPath;
	struct DrawMapBMArgs	thw_DMArgs;				/* mapdrawing stuff */
	/* names for blks and map, if they are set in DMArgs */
	UBYTE									thw_BlocksetName[ BLOCKSETNAMESIZE ];
	UBYTE									thw_MapName[ MAPNAMESIZE ];

	UWORD									thw_HighlightPen;

	UWORD									thw_Speed;

	struct PathSeg				*thw_DragSeg;		/* segment being dragged (NULL=none) */
	UWORD									thw_DragObj;		/* see below */
};

/* values for thw_DragObj: */
#define POBJ_NODE 0				/* dragging a endpoint node */
/* define ctrl point types here... */

#define GAD_BORING 0
#define GID_PICKPATH	2
#define GID_PATH			3
#define GID_NEWPATH		4
#define GID_RENAMEPATH	5
#define GID_DELPATH			6

#define GID_EDITGROUP 7

#define GID_VIEWSPACE 9
#define GID_VSCROLL		10

#define GID_HSCROLL		12

#define GID_ADDLINE		15
#define GID_TAGALL		16
#define	GID_DIVLINE		17
#define GID_DELLINE		19
#define GID_SPEED			20

#define GID_ZOOMIN		22
#define GID_ZOOMOUT		23
#define GID_MAP				24
#define GID_PICKMAP		25
#define GID_BLKS			26
#define GID_PICKBLKS	27

static struct HoopyObject gadlayout[] =
{
	{ HOTYPE_VGROUP, 100, 0,3,0,0,0, NULL, 0,0,GAD_BORING,0,0 },
		{ HOTYPE_HGROUP, 100, HOFLG_NOBORDER,5,0,0,0, NULL, 0,0,GAD_BORING,0,0 },
			{ HOTYPE_BUTTON, 100, HOFLG_NOBORDER, 0,0,0,0, "Path...",0,GID_PICKPATH,PLACETEXT_IN,0 },
			{ HOTYPE_TEXT, 100, HOFLG_NOBORDER|HOFLG_FREEWIDTH, NULL,0,0,0, NULL,0,GID_PATH,0,0 },
			{ HOTYPE_BUTTON, 100, HOFLG_NOBORDER, 0,0,0,0, "New",0,GID_NEWPATH,PLACETEXT_IN,0 },
			{ HOTYPE_BUTTON, 100, HOFLG_NOBORDER, 0,0,0,0, "Rename",0,GID_RENAMEPATH,PLACETEXT_IN,0 },
			{ HOTYPE_BUTTON, 100, HOFLG_NOBORDER, 0,0,0,0, "Del",0,GID_DELPATH,PLACETEXT_IN,0 },
		{ HOTYPE_VGROUP, 100, HOFLG_NOBORDER,3,0,0,0, NULL, 0,0,GID_EDITGROUP,0,0 },
			{ HOTYPE_HGROUP, 100, HOFLG_NOBORDER,2,0,0,0, NULL, 0,0,GAD_BORING,0,0 },
				{ HOTYPE_SPACE, 100, HOFLG_RAISEDBORDER|HOFLG_FREEWIDTH|HOFLG_FREEHEIGHT, 0,0,0,0, NULL,0,GID_VIEWSPACE,0,0 },
				{ HOTYPE_VSCROLLER, 100, HOFLG_NOBORDER|HOFLG_FREEHEIGHT, 0,0,0,0, NULL,0,GAD_BORING,0,0 },
			{ HOTYPE_HGROUP, 100, HOFLG_NOBORDER,2,0,0,0, NULL, 0,0,GAD_BORING,0,0 },
				{ HOTYPE_HSCROLLER, 100, HOFLG_NOBORDER|HOFLG_FREEWIDTH, 0,0,0,0, NULL,0,GAD_BORING,0,0 },
				{ HOTYPE_SCROLLYSPACE, 100, HOFLG_NOBORDER, 0,0,0,0, NULL,0,GAD_BORING,0,0 },
			{ HOTYPE_HGROUP, 100, HOFLG_NOBORDER,6,0,0,0, NULL, 0,0,GAD_BORING,0,0 },
				{ HOTYPE_BUTTON, 100, HOFLG_NOBORDER, 0,0,0,0, "Line",0,GID_ADDLINE,PLACETEXT_IN,0 },
				{ HOTYPE_BUTTON, 100, HOFLG_NOBORDER, 0,0,0,0, "All",0,GID_TAGALL,PLACETEXT_IN,0 },
				{ HOTYPE_BUTTON, 100, HOFLG_NOBORDER, 0,0,0,0, "Div",0,GID_DIVLINE,PLACETEXT_IN,0 },
				{ HOTYPE_SPACE, 100, HOFLG_NOBORDER|HOFLG_FREEWIDTH, 0,0,0,0, NULL,0,0,0,0 },
				{ HOTYPE_BUTTON, 100, HOFLG_NOBORDER, 0,0,0,0, "Del",0,GID_DELLINE,PLACETEXT_IN,0 },
				{ HOTYPE_FIXEDPOINT, 100, HOFLG_NOBORDER, 6,0,0,0, "Speed",0,GID_SPEED,PLACETEXT_LEFT,0 },
		{ HOTYPE_HGROUP, 100, HOFLG_NOBORDER,6,0,0,0, NULL, 0,0,GAD_BORING,0,0 },
			{ HOTYPE_BUTTON, 100, HOFLG_NOBORDER, 0,0,0,0, "In",0,GID_ZOOMIN,PLACETEXT_IN,0 },
			{ HOTYPE_BUTTON, 100, HOFLG_NOBORDER, 0,0,0,0, "Out",0,GID_ZOOMOUT,PLACETEXT_IN,0 },
			{ HOTYPE_TEXT, 100, HOFLG_NOBORDER|HOFLG_FREEWIDTH, 0,0,0,0, "Map",0,GID_MAP,PLACETEXT_LEFT,0 },
			{ HOTYPE_BUTTON, 100, HOFLG_NOBORDER, 0,0,0,0, "M",0,GID_PICKMAP,PLACETEXT_IN,0 },
			{ HOTYPE_TEXT, 100, HOFLG_NOBORDER|HOFLG_FREEWIDTH, 0,0,0,0, "Blks",0,GID_BLKS,PLACETEXT_LEFT,0 },
			{ HOTYPE_BUTTON, 100, HOFLG_NOBORDER, 0,0,0,0, "B",0,GID_PICKBLKS,PLACETEXT_IN,0 },
	{ HOTYPE_END }
};

static void THAPIDCMPHandler( struct SuperWindow *sw,
	struct IntuiMessage *imsg );
static void HideWindow( struct SuperWindow *sw );
static BOOL ShowWindow( struct SuperWindow *sw );
static void KillWindow( struct SuperWindow *sw );
static BOOL RefreshMenus( struct SuperWindow *sw );
static UBYTE *MakeTHAPWinTitle( struct SuperWindow *sw );

static VOID ClipToMapView( struct THAPWindow *thw );
static VOID DrawMapView( struct THAPWindow *thw );
static void ScrollMapView( struct THAPWindow *thw, WORD dx, WORD dy,
	UWORD qual );
static VOID thw_PickMap( struct THAPWindow *thw );
static VOID thw_PickBlockset( struct THAPWindow *thw );
static VOID thw_PickPath( struct THAPWindow *thw );
static VOID thw_SetCurrentPath( struct THAPWindow *thw, struct Path *path );
static VOID thw_NewPath( struct THAPWindow *thw );
static VOID thw_RenamePath( struct THAPWindow *thw );
static VOID thw_DeletePath( struct THAPWindow *thw );
static VOID DrawPath( struct THAPWindow *thw );
static VOID DrawPathSeg( struct THAPWindow *thw, struct PathSeg *psg );
static VOID EORPathSeg( struct THAPWindow *thw, struct PathSeg *psg );
static VOID UnDrawPath( struct THAPWindow *thw );
static VOID UnDrawPathSeg( struct THAPWindow *thw, struct PathSeg *psg );
static VOID AddLine( struct THAPWindow *thw );
static VOID MouseDown( struct THAPWindow *thw, WORD mx, WORD my, UWORD qual );
static VOID MouseUp( struct THAPWindow *thw, WORD mx, WORD my );
static VOID MouseMove( struct THAPWindow *thw, WORD mx, WORD my );
static struct PathSeg *PickSegNode( struct THAPWindow *thw, WORD mx, WORD my,
	UWORD *objtype );
static struct PathSeg *PickSegBody( struct THAPWindow *thw,
	WORD mx, WORD my, BOOL nontagged );
static void TagAllSegs( struct THAPWindow *thw, UBYTE state );
static void SetSpeed( struct THAPWindow *thw );
static void DeleteLines( struct THAPWindow *thw );
static void SubDivideLines( struct THAPWindow *thw );

/********************************************************/

BOOL OpenNewTHAPWindow( struct Chunk *cnk )
{
	struct SuperWindow *sw;
	struct THAPWindow *thw;
	BOOL success = FALSE;
	struct Chunk *cnktoo;

	assert( cnk != NULL );

	for( sw = (struct SuperWindow *)superwindows.lh_Head;
		sw->sw_Node.ln_Succ;
		sw = (struct SuperWindow *)sw->sw_Node.ln_Succ )
	{
		if( sw->sw_Type == SWTY_THAP )
		{
			if( ((struct THAPWindow *)sw)->thw_Chunk == cnk )
			{
				if( sw->sw_Window )
				{
					WindowToFront( sw->sw_Window );
					ActivateWindow( sw->sw_Window );
					return TRUE;
				}
			}
		}
	}

	LockChunk( cnk );

	if( thw = AllocVec( sizeof( struct THAPWindow ), MEMF_ANY|MEMF_CLEAR ) )
	{
		sw = &thw->thw_sw;
		sw->sw_Handler = THAPIDCMPHandler;
		sw->sw_HideWindow = HideWindow;
		sw->sw_ShowWindow = ShowWindow;
		sw->sw_BuggerOff = KillWindow;
		sw->sw_RefreshMenus = RefreshMenus;
		sw->sw_MakeTitle	= MakeTHAPWinTitle;
		sw->sw_Type				= SWTY_THAP;
		sw->sw_Window			= NULL;
		sw->sw_Gadgets		= NULL;
		sw->sw_VisualInfo	= NULL;
		sw->sw_WinDim			= zonkfig.SuperWindowDefs[ SWTY_THAP ];

		thw->thw_Chunk = cnk;
		thw->thw_DMArgs.MapX = 0;
		thw->thw_DMArgs.MapY = 0;
		thw->thw_DMArgs.ZoomFactor = 1;
		thw->thw_DMArgs.Map = NULL;
		thw->thw_DMArgs.Blockset = NULL;

		if( cnktoo = FindChunkType( NULL, ID_MAP ) )
		{
			if( cnktoo->ch_Data )
			{
				thw->thw_DMArgs.Map = (struct Map *)cnktoo->ch_Data;
				Mystrncpy( thw->thw_MapName,
					thw->thw_DMArgs.Map->map_Name,
					MAPNAMESIZE-1 );
			}
		}
		if( cnktoo = FindChunkType( NULL, ID_BLKS ) )
		{
			if( cnktoo->ch_Data )
			{
				thw->thw_DMArgs.Blockset = (struct Blockset *)cnktoo->ch_Data;
				Mystrncpy( thw->thw_BlocksetName,
					thw->thw_DMArgs.Blockset->bs_Name,
					BLOCKSETNAMESIZE-1 );
			}
		}

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
	struct THAPWindow *thw;
	struct Window *win;
	struct Screen *scr;
	struct Path *path;

	thw = (struct THAPWindow *)sw;

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
		WA_Title,				(*sw->sw_MakeTitle)( sw ),
		WA_CloseGadget,	TRUE,
		WA_DragBar,			TRUE,
		WA_SizeGadget,	TRUE,
		WA_DepthGadget,	TRUE,
		WA_SizeBRight,	TRUE,
		WA_NewLookMenus,TRUE,
		WA_ReportMouse,	TRUE,
		WA_Activate,		TRUE,
		TAG_DONE,				NULL);

	UnlockMainScreen( scr );

	if( sw->sw_Window = win )
	{
		if( !zonkfig.scfg.scfg_DefaultFont && mainwinfont )
			SetFont( win->RPort, mainwinfont );

		if( thw->thw_Layout = AllocVec( sizeof( gadlayout ), MEMF_ANY ) )
		{
			CopyMem( gadlayout, thw->thw_Layout, sizeof( gadlayout ) );
			if( sw->sw_hos = HO_GetHOStuff( win, win->RPort->Font ) )
			{

				/* Tell the window which superwindow it is attached to */
				/* (so we'll know which handler routine to call) */
				win->UserData = (BYTE *)sw;

				/* Set up some menus */
				sw->sw_VisualInfo = sw->sw_hos->hos_vi;
				GenericMakeMenus( sw, NULL );

				/* we want to use our global message port for IDCMP stuff */
				win->UserPort = mainmsgport;
				ModifyIDCMP( win, IDCMP_REFRESHWINDOW|IDCMP_NEWSIZE|
					IDCMP_CLOSEWINDOW|IDCMP_CHANGEWINDOW|
					IDCMP_RAWKEY|IDCMP_MOUSEBUTTONS|IDCMP_MOUSEMOVE|IDCMP_MENUPICK|
					TEXTIDCMP|BUTTONIDCMP );

				HO_SussWindowSizing( sw->sw_hos, thw->thw_Layout );

				/* init blockset/map text gadgets */
				if( thw->thw_DMArgs.Map )
					thw->thw_Layout[ GID_MAP ].ho_Attr0 = (LONG)thw->thw_DMArgs.Map->map_Name;
				else
					thw->thw_Layout[ GID_MAP ].ho_Attr0 = NULL;

				if( thw->thw_DMArgs.Blockset )
					thw->thw_Layout[ GID_BLKS ].ho_Attr0 = (LONG)thw->thw_DMArgs.Blockset->bs_Name;
				else
					thw->thw_Layout[ GID_BLKS ].ho_Attr0 = NULL;

				thw->thw_Speed = 0;
				thw->thw_Layout[ GID_SPEED ].ho_Value = (LONG)thw->thw_Speed;
				HO_MakeLayout( thw->thw_Layout, sw->sw_hos, win->BorderLeft, win->BorderTop,
					win->Width - win->BorderLeft - win->BorderRight,
					win->Height - win->BorderTop - win->BorderBottom );

				thw->thw_HighlightPen =
					thw->thw_sw.sw_hos->hos_dri->dri_Pens[ SHINEPEN ];

				/* bitmap for map drawing stuff - ok if it fails */
				thw->thw_DMArgs.bm = CreateBitMap(win->WScreen->Width,win->WScreen->Height,
					win->WScreen->RastPort.BitMap->Depth, TRUE );
				thw->thw_DMArgs.BMWidth = win->WScreen->Width;
				thw->thw_DMArgs.BMHeight = win->WScreen->Height;

				if( !IsListEmpty( &thw->thw_Chunk->ch_DataList ) )
				{
					path = (struct Path *)thw->thw_Chunk->ch_DataList.lh_Head;
					thw_SetCurrentPath( thw, path );
				}
				else
					thw_SetCurrentPath( thw, NULL );

				success = TRUE;
			}
		}
	}

	if( !success )
	{
		if( sw->sw_Window ) CloseWindow( sw->sw_Window );
		sw->sw_Window = NULL;
		if( thw->thw_Layout ) FreeVec( thw->thw_Layout );
		thw->thw_Layout = NULL;
	}

	return( success );
}


/********************************************************/

static void HideWindow( struct SuperWindow *sw )
{
	struct THAPWindow *thw;
	struct Window *win;

	thw = (struct THAPWindow *)sw;

	if( win = sw->sw_Window )
	{
		RememberWindow( sw );
		GenericZapMenus( sw );
		HO_KillLayout( sw->sw_hos );

		/* free work bitmap (used for map drawing) if it exists */
		if( thw->thw_DMArgs.bm )
		{
			DestroyBitMap( thw->thw_DMArgs.bm, win->WScreen->Width, win->WScreen->Height,
				win->WScreen->RastPort.BitMap->Depth );
			thw->thw_DMArgs.bm = NULL;
		}

		CloseWindowSafely( sw->sw_Window );
		HO_FreeHOStuff( sw->sw_hos );
		sw->sw_Window = NULL;
		FreeVec( thw->thw_Layout );
		thw->thw_Layout = NULL;
	}
}

/********************************************************/

static void KillWindow( struct SuperWindow *sw )
{
	struct THAPWindow *thw;

	if( (thw = (struct THAPWindow *)sw) && IsOKToClose( sw ) )
	{
		(*sw->sw_HideWindow)(sw);
		UnlockChunk( ((struct THAPWindow *)sw)->thw_Chunk );
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

static UBYTE *MakeTHAPWinTitle( struct SuperWindow *sw )
{
	struct Chunk *cnk;
	UBYTE *p;

	cnk = ((struct THAPWindow *)sw)->thw_Chunk;
	p = sw->sw_Name;

	if( cnk->ch_Modified )
		*p++ = MODCHAR;
	strcpy( p, "Paths" );

	return( sw->sw_Name );
}

/********************************************************/

static void THAPIDCMPHandler( struct SuperWindow *sw,
	struct IntuiMessage *origimsg )
{
	struct IntuiMessage *imsg;
	struct Window *win;
	struct THAPWindow *thw;

	ULONG class;
	UWORD code;
	UWORD qualifier;
	APTR iaddress;
	WORD mousex, mousey;
	ULONG seconds, micros;

	thw = (struct THAPWindow *)sw;
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
			case IDCMP_MOUSEMOVE:
				ReplyMsg( (struct Message *)origimsg );
				MouseMove( thw, mousex, mousey );
				break;
			case IDCMP_MOUSEBUTTONS:
				ReplyMsg( (struct Message *)origimsg );
				switch( code )
				{
					case SELECTDOWN:
						MouseDown( thw, mousex, mousey, qualifier );
						break;
					case SELECTUP:
						MouseUp( thw, mousex, mousey );
						break;
				}
				break;
			case IDCMP_CLOSEWINDOW:
				ReplyMsg( (struct Message *)origimsg );
				KillWindow( sw );
				break;
			case IDCMP_NEWSIZE:
				ClearWindow( win );
				HO_ReadGadgetStates( sw->sw_hos, thw->thw_Layout );
				HO_MakeLayout( thw->thw_Layout, sw->sw_hos, win->BorderLeft, win->BorderTop,
					win->Width - win->BorderLeft - win->BorderRight,
					win->Height - win->BorderTop - win->BorderBottom );
				DrawMapView( thw );
				DrawPath( thw );
				ReplyMsg( (struct Message *)origimsg );
				break;
			case IDCMP_CHANGEWINDOW:
				UpdateWinDef( sw );
				ReplyMsg( (struct Message *)origimsg );
				break;
			case IDCMP_GADGETUP:
				ReplyMsg( (struct Message *)origimsg );
				switch( ((struct Gadget*)iaddress)->GadgetID )
				{
					case GID_ZOOMIN:
						if( thw->thw_DMArgs.ZoomFactor < 16 )
							thw->thw_DMArgs.ZoomFactor += 1;
						DrawMapView( thw );
						DrawPath( thw );
						break;
					case GID_ZOOMOUT:
						if( thw->thw_DMArgs.ZoomFactor > 1 )
							thw->thw_DMArgs.ZoomFactor -= 1;
						DrawMapView( thw );
						DrawPath( thw );
						break;

					case GID_SPEED:
						ModifyChunk( thw->thw_Chunk );
						HO_ReadGadgetStates( sw->sw_hos, thw->thw_Layout );
						thw->thw_Speed = (UWORD)thw->thw_Layout[ GID_SPEED ].ho_Value;
						SetSpeed( thw );
						break;
					case GID_PICKMAP:
						thw_PickMap( thw );
						break;
					case GID_PICKBLKS:
						thw_PickBlockset( thw );
						break;
					case GID_PICKPATH:
						thw_PickPath( thw );
						break;
					case GID_NEWPATH:
						ModifyChunk( thw->thw_Chunk );
						thw_NewPath( thw );
						break;
					case GID_RENAMEPATH:
						ModifyChunk( thw->thw_Chunk );
						thw_RenamePath( thw );
						break;
					case GID_DELPATH:
						ModifyChunk( thw->thw_Chunk );
						thw_DeletePath( thw );
						break;
					case GID_ADDLINE:
						ModifyChunk( thw->thw_Chunk );
						AddLine( thw );
						break;
					case GID_DELLINE:
						ModifyChunk( thw->thw_Chunk );
						DeleteLines( thw );
						break;
					case GID_DIVLINE:
						ModifyChunk( thw->thw_Chunk );
						SubDivideLines( thw );
						break;
					case GID_TAGALL:
						ClipToMapView( thw );
						TagAllSegs( thw, 1 );
						UnclipWindow( WIN );
						break;
				}
				break;
			case IDCMP_RAWKEY:
				ReplyMsg( (struct Message *)origimsg );
				switch( code )
				{
					case RAWKEY_UP:
						ScrollMapView( thw, 0, -1, qualifier );
						DrawPath( thw );
						break;
					case RAWKEY_DOWN:
						ScrollMapView( thw, 0, 1, qualifier );
						DrawPath( thw );
						break;
					case RAWKEY_LEFT:
						ScrollMapView( thw, -1, 0, qualifier );
						DrawPath( thw );
						break;
					case RAWKEY_RIGHT:
						ScrollMapView( thw, 1, 0, qualifier );
						DrawPath( thw );
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
			default:
				ReplyMsg( (struct Message *)origimsg );
				break;
		}
	}
	else
		ReplyMsg( (struct Message *)origimsg );
}



/********************************************************/

static VOID thw_PickMap( struct THAPWindow *thw )
{
	struct Chunk *cnk;

	if( BlockAllSuperWindows() )
	{

		cnk = RequestChunk( thw->thw_sw.sw_Window, "Select Map",
			"Maps", ID_MAP, NULL );
		UnblockAllSuperWindows();
		if( cnk )
		{
			thw->thw_DMArgs.Map = (struct Map *)cnk->ch_Data;
			thw->thw_Layout[ GID_MAP ].ho_Attr0 = (LONG)thw->thw_DMArgs.Map->map_Name;
			HO_RefreshObject( &thw->thw_Layout[ GID_MAP ], thw->thw_sw.sw_hos );
			Mystrncpy( thw->thw_MapName,
				thw->thw_DMArgs.Map->map_Name,
				MAPNAMESIZE-1 );
		}
		else
		{
			thw->thw_DMArgs.Map = NULL;
			thw->thw_Layout[ GID_MAP ].ho_Attr0 = NULL;
			HO_RefreshObject( &thw->thw_Layout[ GID_MAP ], thw->thw_sw.sw_hos );
		}
		DrawMapView( thw );
		DrawPath( thw );
	}
}

/********************************************************/

static VOID thw_PickBlockset( struct THAPWindow *thw )
{
	struct Chunk *cnk;

	if( BlockAllSuperWindows() )
	{
		cnk = RequestChunk( thw->thw_sw.sw_Window, "Select Blockset",
			"Blocksets", ID_BLKS, NULL );
		UnblockAllSuperWindows();
		if( cnk )
		{
			thw->thw_DMArgs.Blockset = (struct Blockset *)cnk->ch_Data;
			thw->thw_Layout[ GID_BLKS ].ho_Attr0 = (LONG)thw->thw_DMArgs.Blockset->bs_Name;
			HO_RefreshObject( &thw->thw_Layout[ GID_BLKS ], thw->thw_sw.sw_hos );
			Mystrncpy( thw->thw_BlocksetName,
				thw->thw_DMArgs.Blockset->bs_Name,
				BLOCKSETNAMESIZE-1 );
		}
		else
		{
			thw->thw_DMArgs.Blockset = NULL;
			thw->thw_Layout[ GID_BLKS ].ho_Attr0 = NULL;
			HO_RefreshObject( &thw->thw_Layout[ GID_BLKS ], thw->thw_sw.sw_hos );
		}
		DrawMapView( thw );
		DrawPath( thw );
	}
}

/********************************************************/

static VOID thw_PickPath( struct THAPWindow *thw )
{
	struct Path *path = NULL;
	UWORD num;

	if( BlockAllSuperWindows() )
	{
		num = MyListViewRequest( thw->thw_sw.sw_Window, "Pickapath",
			"Paths", &thw->thw_Chunk->ch_DataList );
		UnblockAllSuperWindows();
		if( num != 0xFFFF )
		{
			path = (struct Path *)FindNthNode( &thw->thw_Chunk->ch_DataList, num );
			thw_SetCurrentPath( thw, path );
		}
	}
}

/********************************************************/

static VOID thw_NewPath( struct THAPWindow *thw )
{
	struct Path *path = NULL;
	UBYTE nambuf[ PATHNAMESIZE ];
	WORD x,y;

	Mystrncpy( nambuf, "-<UNTITLED>-", PATHNAMESIZE-1 );

	if( MyStringRequest( thw->thw_sw.sw_Window, "Create new path", "Enter name",
		nambuf, PATHNAMESIZE-1 ) )
	{
		/* create path with end node already in place */

		x = ( thw->thw_DMArgs.MapX * BLKW ) + ( VIEWWIDTH / 2 );
		y = ( thw->thw_DMArgs.MapY * BLKH ) + ( VIEWHEIGHT / 2 );

		if( path = CreatePath( TRUE, x ,y ) )
		{
			MakePackable( nambuf );
			Mystrncpy( path->pth_Name, nambuf, PATHNAMESIZE-1 );
			/* add to chunk and display */
			AddTail( &thw->thw_Chunk->ch_DataList, (struct Node *)path );
			thw_SetCurrentPath( thw, path );
		}
	}
}

/********************************************************/

static VOID thw_RenamePath( struct THAPWindow *thw )
{
	struct Path *path;
	UBYTE nambuf[ PATHNAMESIZE ];

	if( path = thw->thw_CurrentPath )
	{

		Mystrncpy( nambuf, path->pth_Name, PATHNAMESIZE-1 );

		if( MyStringRequest( thw->thw_sw.sw_Window, "Rename Path", "New name",
			nambuf, PATHNAMESIZE-1 ) )
		{
			MakePackable( nambuf );
			Mystrncpy( path->pth_Name, nambuf, PATHNAMESIZE-1 );
			thw_SetCurrentPath( thw, path );
		}
	}
}

/********************************************************/

static VOID thw_DeletePath( struct THAPWindow *thw )
{
	struct Path *path;

	if( path = thw->thw_CurrentPath )
	{
		if( PlebReq( thw->thw_sw.sw_Window, "Delete Path", "Are you sure?","Yep|No way!" ) )
		{
			Remove( &path->pth_Node );
			{
				FreePath( path );
				thw_SetCurrentPath( thw, NULL );
			}
		}
	}
}

/********************************************************/

static VOID ClipToMapView( struct THAPWindow *thw )
{
	struct Region *oldregion;

	if( oldregion = ClipWindow( WIN, VIEWLEFT, VIEWTOP,
		VIEWLEFT + VIEWWIDTH - 1, VIEWTOP + VIEWHEIGHT - 1 ) )
	{
		DisposeRegion( oldregion );
	}
}


/********************************************************/

static VOID DrawMapView( struct THAPWindow *thw )
{
	thw->thw_DMArgs.ViewWidth = thw->thw_Layout[ GID_VIEWSPACE ].ho_Attr1 >> 16;
	thw->thw_DMArgs.ViewHeight = thw->thw_Layout[ GID_VIEWSPACE ].ho_Attr1 & 0xFFFF;

	if( thw->thw_DMArgs.Map && thw->thw_DMArgs.Blockset )
	{

		DrawMapBM( &thw->thw_DMArgs );

		BltBitMapRastPort( thw->thw_DMArgs.bm, 0,0,thw->thw_sw.sw_Window->RPort,
			thw->thw_Layout[ GID_VIEWSPACE ].ho_Attr0 >> 16,
			thw->thw_Layout[ GID_VIEWSPACE ].ho_Attr0 & 0xFFFF,
			thw->thw_DMArgs.ViewWidth, thw->thw_DMArgs.ViewHeight, 0xC0 );
	}
	else
	{
		SetAPen( thw->thw_sw.sw_Window->RPort, 0 );
		RectFill( thw->thw_sw.sw_Window->RPort,
			thw->thw_Layout[ GID_VIEWSPACE ].ho_Attr0 >> 16,
			thw->thw_Layout[ GID_VIEWSPACE ].ho_Attr0 & 0xFFFF,
			(thw->thw_Layout[ GID_VIEWSPACE ].ho_Attr0 >> 16) +
			thw->thw_DMArgs.ViewWidth,
			(thw->thw_Layout[ GID_VIEWSPACE ].ho_Attr0 & 0xFFFF) +
			thw->thw_DMArgs.ViewHeight );
	}
}


/********************************************************/

static void ScrollMapView( struct THAPWindow *thw, WORD dx, WORD dy, UWORD qual )
{
	thw->thw_DMArgs.ViewWidth = thw->thw_Layout[ GID_VIEWSPACE ].ho_Attr1 >> 16;
	thw->thw_DMArgs.ViewHeight = thw->thw_Layout[ GID_VIEWSPACE ].ho_Attr1 & 0xFFFF;

	if( thw->thw_DMArgs.Map && thw->thw_DMArgs.Blockset )
	{
		ScrollMapBM( &thw->thw_DMArgs, dx, dy, qual );

		BltBitMapRastPort( thw->thw_DMArgs.bm, 0,0,thw->thw_sw.sw_Window->RPort,
			thw->thw_Layout[ GID_VIEWSPACE ].ho_Attr0 >> 16,
			thw->thw_Layout[ GID_VIEWSPACE ].ho_Attr0 & 0xFFFF,
			thw->thw_DMArgs.ViewWidth, thw->thw_DMArgs.ViewHeight, 0xC0 );
	}
	else
	{
		SetAPen( thw->thw_sw.sw_Window->RPort, 0 );
		RectFill( thw->thw_sw.sw_Window->RPort,
			thw->thw_Layout[ GID_VIEWSPACE ].ho_Attr0 >> 16,
			thw->thw_Layout[ GID_VIEWSPACE ].ho_Attr0 & 0xFFFF,
			(thw->thw_Layout[ GID_VIEWSPACE ].ho_Attr0 >> 16) +
			thw->thw_DMArgs.ViewWidth,
			(thw->thw_Layout[ GID_VIEWSPACE ].ho_Attr0 & 0xFFFF) +
			thw->thw_DMArgs.ViewHeight );
	}
}


/********************************************************/

static VOID thw_SetCurrentPath( struct THAPWindow *thw, struct Path *path )
{
	struct PathSeg *psg;

	thw->thw_CurrentPath = path;
	if( path )
	{
		thw->thw_Layout[ GID_PATH ].ho_Attr0 = (LONG)path->pth_Name;

		HO_RefreshObject( &thw->thw_Layout[ GID_PATH ], thw->thw_sw.sw_hos );

		HO_DisableObject( &thw->thw_Layout[ GID_PICKPATH ], thw->thw_sw.sw_hos,
			FALSE );
		HO_DisableObject( &thw->thw_Layout[ GID_RENAMEPATH ], thw->thw_sw.sw_hos,
			FALSE );
		HO_DisableObject( &thw->thw_Layout[ GID_DELPATH ], thw->thw_sw.sw_hos,
			FALSE );
		HO_DisableObject( &thw->thw_Layout[ GID_EDITGROUP ], thw->thw_sw.sw_hos,
			FALSE );

		if( psg = (struct PathSeg *)path->pth_Segments.mlh_Head )
		{
			thw->thw_DMArgs.MapX = ( psg->psg_XPos / BLKW ) -
				( ( VIEWWIDTH / BLKW ) / 2 );
			thw->thw_DMArgs.MapY = ( psg->psg_YPos / BLKH ) -
				( ( VIEWHEIGHT / BLKH ) / 2 );
		}
	}
	else
	{
		thw->thw_Layout[ GID_PATH ].ho_Attr0 = (LONG)NULL;

		HO_RefreshObject( &thw->thw_Layout[ GID_PATH ], thw->thw_sw.sw_hos );
		if( IsListEmpty( &thw->thw_Chunk->ch_DataList ) )
		{
			HO_DisableObject( &thw->thw_Layout[ GID_PICKPATH ], thw->thw_sw.sw_hos,
				TRUE );
		}
		else
		{
			HO_DisableObject( &thw->thw_Layout[ GID_PICKPATH ], thw->thw_sw.sw_hos,
				FALSE );
		}

		HO_DisableObject( &thw->thw_Layout[ GID_RENAMEPATH ], thw->thw_sw.sw_hos,
			TRUE );
		HO_DisableObject( &thw->thw_Layout[ GID_DELPATH ], thw->thw_sw.sw_hos,
			TRUE );
		HO_DisableObject( &thw->thw_Layout[ GID_EDITGROUP ], thw->thw_sw.sw_hos,
			TRUE );
	}
	DrawMapView( thw );
	DrawPath( thw );
}


/********************************************************/
// Draw the current path (if any) over the mapview.

static VOID DrawPath( struct THAPWindow *thw )
{
	struct Path *path;
	struct PathSeg *psg;

	if( path = thw->thw_CurrentPath )
	{
		ClipToMapView( thw );
		for( psg = (struct PathSeg *)path->pth_Segments.mlh_Head;
			psg->psg_Node.mln_Succ;
			psg = (struct PathSeg *)psg->psg_Node.mln_Succ )
		{
			DrawPathSeg( thw, psg );
		}
		UnclipWindow( WIN );
	}
}


/********************************************************/
static VOID UnDrawPath( struct THAPWindow *thw )
{
	struct Path *path;
	struct PathSeg *psg;

	if( path = thw->thw_CurrentPath )
	{
//		DrawMapView( thw );

		ClipToMapView( thw );
		for( psg = (struct PathSeg *)path->pth_Segments.mlh_Head;
			psg->psg_Node.mln_Succ;
			psg = (struct PathSeg *)psg->psg_Node.mln_Succ )
		{
			UnDrawPathSeg( thw, psg );
		}
		UnclipWindow( WIN );
	}
}


/********************************************************/
static VOID UnDrawPathSeg( struct THAPWindow *thw, struct PathSeg *psg )
{
	WORD x0,y0,x1,y1,tmp;
	struct PathSeg *predpsg;

	/* translate map coords to view coords */
	x0 = ( ( psg->psg_XPos - (thw->thw_DMArgs.MapX * BLKW ) ) *
		thw->thw_DMArgs.ZoomFactor ) + ( thw->thw_DMArgs.ZoomFactor / 2 );
	y0 = ( ( psg->psg_YPos - (thw->thw_DMArgs.MapY * BLKH ) ) *
		thw->thw_DMArgs.ZoomFactor ) + thw->thw_DMArgs.ZoomFactor/2;
	x0 += VIEWLEFT;
	y0 += VIEWTOP;

	switch( psg->psg_Type )
	{
		case PSGTYPE_LINE:
			if( thw->thw_DMArgs.bm && thw->thw_DMArgs.Blockset && thw->thw_DMArgs.Map )
			{
				BltBitMapRastPort( thw->thw_DMArgs.bm,
					x0-VIEWLEFT-MKSIZE, y0-VIEWTOP-MKSIZE, RP,
					x0 - MKSIZE, y0 - MKSIZE, ( MKSIZE * 2 ) + 1, ( MKSIZE * 2 ) + 1, 0xC0 );
			}
			else
			{
				SetAPen( RP, 0 );
				RectFill( RP, x0 - MKSIZE, y0 - MKSIZE, x0 + MKSIZE, y0 + MKSIZE );
			}

			predpsg = (struct PathSeg *)psg->psg_Node.mln_Pred;
			x1 = ( ( predpsg->psg_XPos - (thw->thw_DMArgs.MapX * BLKW ) ) *
				thw->thw_DMArgs.ZoomFactor ) + ( thw->thw_DMArgs.ZoomFactor / 2 );
			y1 = ( ( predpsg->psg_YPos - (thw->thw_DMArgs.MapY * BLKH ) ) *
				thw->thw_DMArgs.ZoomFactor ) + thw->thw_DMArgs.ZoomFactor/2;
			x1 += VIEWLEFT;
			y1 += VIEWTOP;

			if( thw->thw_DMArgs.bm && thw->thw_DMArgs.Blockset && thw->thw_DMArgs.Map )
			{
				if( x1 < x0 )
				{
					tmp = x1;
					x1 = x0;
					x0 = tmp;
				}

				if( y1 < y0 )
				{
					tmp = y1;
					y1 = y0;
					y0 = tmp;
				}

				BltBitMapRastPort( thw->thw_DMArgs.bm, x0-VIEWLEFT, y0-VIEWTOP, RP,
					x0, y0, 1+x1-x0, 1+y1-y0, 0xC0 );
			}
			else
			{
				/* no background map */
				SetAPen( RP, 0 );
				Move( RP, x0, y0 );
				Draw( RP, x1, y1 );
			}
			break;
		case PSGTYPE_HEAD:
			if( thw->thw_DMArgs.bm && thw->thw_DMArgs.Blockset && thw->thw_DMArgs.Map )
			{
				BltBitMapRastPort( thw->thw_DMArgs.bm,
					x0-VIEWLEFT-8, y0-VIEWTOP-8, RP,
					x0 - 8, y0 - 8, ( 8 * 2 ) + 1, ( 8 * 2 ) + 1, 0xC0 );
			}
			else
			{
				SetAPen( RP, 0 );
				RectFill( RP, x0 - 8, y0 - 8, x0 + 8, y0 + 8 );
			}
	}
}

/********************************************************/
static VOID DrawPathSeg( struct THAPWindow *thw, struct PathSeg *psg )
{
	WORD x0,y0,x1,y1;
	struct PathSeg *predpsg;

	/* translate map coords to view coords */
	x0 = ( ( psg->psg_XPos - (thw->thw_DMArgs.MapX * BLKW ) ) *
		thw->thw_DMArgs.ZoomFactor ) + ( thw->thw_DMArgs.ZoomFactor / 2 );
	y0 = ( ( psg->psg_YPos - (thw->thw_DMArgs.MapY * BLKH ) ) *
		thw->thw_DMArgs.ZoomFactor ) + thw->thw_DMArgs.ZoomFactor/2;
	x0 += VIEWLEFT;
	y0 += VIEWTOP;

	switch( psg->psg_Type )
	{
		case PSGTYPE_LINE:
			if( psg->psg_Tagged )
				SetAPen( RP, 1 );
			else
				SetAPen( RP, thw->thw_HighlightPen );

			predpsg = (struct PathSeg *)psg->psg_Node.mln_Pred;
			x1 = ( ( predpsg->psg_XPos - (thw->thw_DMArgs.MapX * BLKW ) ) *
				thw->thw_DMArgs.ZoomFactor ) + ( thw->thw_DMArgs.ZoomFactor / 2 );
			y1 = ( ( predpsg->psg_YPos - (thw->thw_DMArgs.MapY * BLKH ) ) *
				thw->thw_DMArgs.ZoomFactor ) + thw->thw_DMArgs.ZoomFactor/2;
			x1 += VIEWLEFT;
			y1 += VIEWTOP;
			Move( RP, x0, y0 );
			Draw( RP, x1, y1 );
			SetAPen( RP, thw->thw_HighlightPen );
			RectFill( RP, x0 - MKSIZE, y0 - MKSIZE, x0 + MKSIZE, y0 + MKSIZE );
			break;
		case PSGTYPE_HEAD:
			SetAPen( RP, thw->thw_HighlightPen );
			Move( RP, x0-8, y0-8 );
			Draw( RP, x0+8, y0+8 );
			Move( RP, x0-8, y0+8 );
			Draw( RP, x0+8, y0-8 );
			RectFill( RP, x0 - MKSIZE, y0 - MKSIZE, x0 + MKSIZE, y0 + MKSIZE );
			break;
	}
}


/********************************************************/
static VOID EORPathSeg( struct THAPWindow *thw, struct PathSeg *psg )
{
	WORD x0,y0,x1,y1;
	struct PathSeg *predpsg;

	/* translate map coords to view coords */
	x0 = ( ( psg->psg_XPos - (thw->thw_DMArgs.MapX * BLKW ) ) *
		thw->thw_DMArgs.ZoomFactor ) + ( thw->thw_DMArgs.ZoomFactor / 2 );
	y0 = ( ( psg->psg_YPos - (thw->thw_DMArgs.MapY * BLKH ) ) *
		thw->thw_DMArgs.ZoomFactor ) + thw->thw_DMArgs.ZoomFactor/2;
	x0 += VIEWLEFT;
	y0 += VIEWTOP;

	SetDrMd( RP, COMPLEMENT );
	switch( psg->psg_Type )
	{
		case PSGTYPE_LINE:
			predpsg = (struct PathSeg *)psg->psg_Node.mln_Pred;
			x1 = ( ( predpsg->psg_XPos - (thw->thw_DMArgs.MapX * BLKW ) ) *
				thw->thw_DMArgs.ZoomFactor ) + ( thw->thw_DMArgs.ZoomFactor / 2 );
			y1 = ( ( predpsg->psg_YPos - (thw->thw_DMArgs.MapY * BLKH ) ) *
				thw->thw_DMArgs.ZoomFactor ) + thw->thw_DMArgs.ZoomFactor/2;
			x1 += VIEWLEFT;
			y1 += VIEWTOP;
			Move( RP, x0, y0 );
			Draw( RP, x1, y1 );
			RectFill( RP, x0 - MKSIZE, y0 - MKSIZE, x0 + MKSIZE, y0 + MKSIZE );
			break;
		case PSGTYPE_HEAD:
			Move( RP, x0-8, y0-8 );
			Draw( RP, x0+8, y0+8 );
			Move( RP, x0-8, y0+8 );
			Draw( RP, x0+8, y0-8 );
			RectFill( RP, x0 - MKSIZE, y0 - MKSIZE, x0 + MKSIZE, y0 + MKSIZE );
			break;
	}

	SetDrMd( RP, JAM2 );
}



/********************************************************/

static VOID AddLine( struct THAPWindow *thw )
{
	struct Path *path;
	struct PathSeg *psg, *secondlastpsg, *lastpsg;

	if( path = thw->thw_CurrentPath )
	{
		if( psg = AllocVec( sizeof( struct PathSeg ), MEMF_ANY|MEMF_CLEAR ) )
		{
			ClipToMapView( thw );


			/* find end node */
			lastpsg = (struct PathSeg *)path->pth_Segments.mlh_TailPred;

			if( lastpsg->psg_Type == PSGTYPE_LINE )
			{
				secondlastpsg = (struct PathSeg *)lastpsg->psg_Node.mln_Pred;
				psg->psg_Type = PSGTYPE_LINE;
				psg->psg_XPos = lastpsg->psg_XPos +
					(lastpsg->psg_XPos - secondlastpsg->psg_XPos);
				psg->psg_YPos = lastpsg->psg_YPos +
					(lastpsg->psg_YPos - secondlastpsg->psg_YPos);
				psg->psg_Speed = lastpsg->psg_Speed;
			}
			else
			{
				psg->psg_Type = PSGTYPE_LINE;
				psg->psg_XPos = lastpsg->psg_XPos + 50;
				psg->psg_YPos = lastpsg->psg_YPos;
				psg->psg_Speed = 1*32;
			}

			AddTail( (struct List *)&path->pth_Segments, (struct Node *)psg );

			DrawPathSeg( thw, psg );
			UnclipWindow( WIN );
		}
	}
}

/********************************************************/

static VOID MouseDown( struct THAPWindow *thw, WORD mx, WORD my, UWORD qual )
{
	UWORD obj;
	struct Path *path;
	struct PathSeg *psg, *nextpsg;
	BOOL multiselect = FALSE;

	if( (qual & IEQUALIFIER_LSHIFT) || (qual & IEQUALIFIER_RSHIFT ) )
		multiselect = TRUE;

	if( path = thw->thw_CurrentPath )
	{
		if( psg = PickSegNode( thw, mx, my, &obj ) )
		{
			ClipToMapView( thw );
//			UnDrawPath( thw );
			thw->thw_DragSeg = psg;
			thw->thw_DragObj = obj;
			UnDrawPathSeg( thw, psg );

			nextpsg = (struct PathSeg *)psg->psg_Node.mln_Succ;
			if( nextpsg->psg_Node.mln_Succ )
			{
				UnDrawPathSeg( thw, nextpsg );
				EORPathSeg( thw, nextpsg );
			}

			EORPathSeg( thw, psg );
//			DrawPath( thw );
			UnclipWindow( WIN );
		}
		else
		{
			ClipToMapView( thw );

			if( !multiselect )
			{
				thw->thw_Speed = 0;
				thw->thw_Layout[ GID_SPEED ].ho_HOFlags |= HOFLG_DISABLED;
				TagAllSegs( thw, 0 );
			}

			if( psg = PickSegBody( thw, mx, my, TRUE ) )
			{
				/* tagged one! */
				psg->psg_Tagged = 1;
				DrawPathSeg( thw, psg );

				/* display speed in speed gadget */
				thw->thw_Speed = psg->psg_Speed;
				thw->thw_Layout[ GID_SPEED ].ho_HOFlags &= ~HOFLG_DISABLED;
			}

			UnclipWindow( WIN );
			thw->thw_Layout[ GID_SPEED ].ho_Value = (LONG)thw->thw_Speed;
			HO_RefreshObject( &thw->thw_Layout[ GID_SPEED ], thw->thw_sw.sw_hos );

		}
	}
}

/********************************************************/

static VOID MouseMove( struct THAPWindow *thw, WORD mx, WORD my )
{
	WORD x,y;
	struct Path *path;
	struct PathSeg *psg, *nextpsg;

	if( path = thw->thw_CurrentPath )
	{
		if( psg = thw->thw_DragSeg )
		{
			ModifyChunk( thw->thw_Chunk );

			nextpsg = (struct PathSeg *)psg->psg_Node.mln_Succ;

			/* transform back to pixel coords on the map */
			x = ( ( mx - VIEWLEFT ) / thw->thw_DMArgs.ZoomFactor ) +
				(thw->thw_DMArgs.MapX * BLKW );
			y = ( ( my - VIEWTOP ) / thw->thw_DMArgs.ZoomFactor ) +
				(thw->thw_DMArgs.MapY * BLKH );

			if( thw->thw_DragObj == POBJ_NODE )
			{
				ClipToMapView( thw );
				EORPathSeg( thw, psg );
				if( nextpsg->psg_Node.mln_Succ )
					EORPathSeg( thw, nextpsg );
				psg->psg_XPos = x;
				psg->psg_YPos = y;
				EORPathSeg( thw, psg );
				if( nextpsg->psg_Node.mln_Succ )
					EORPathSeg( thw, nextpsg );
				UnclipWindow( WIN );
			}
		}
	}
}

/********************************************************/

static VOID MouseUp( struct THAPWindow *thw, WORD mx, WORD my )
{
	struct PathSeg *psg, *nextpsg;

	if( psg = thw->thw_DragSeg )
	{
		ClipToMapView( thw );
		nextpsg = (struct PathSeg *)psg->psg_Node.mln_Succ;
		EORPathSeg( thw, psg );
		if( nextpsg->psg_Node.mln_Succ )
			EORPathSeg( thw, nextpsg );
		UnclipWindow( WIN );
		DrawPath( thw );
		thw->thw_DragSeg = NULL;
	}
}

/********************************************************/

static struct PathSeg *PickSegNode( struct THAPWindow *thw, WORD mx, WORD my,
	UWORD *objtype )
{
	struct PathSeg *psg, *match;
	struct Path *path;
	WORD x,y;

	match = NULL;

	/* transform back to pixel coords on the map */
	x = ( ( mx - VIEWLEFT ) / thw->thw_DMArgs.ZoomFactor ) +
		(thw->thw_DMArgs.MapX * BLKW );
	y = ( ( my - VIEWTOP ) / thw->thw_DMArgs.ZoomFactor ) +
		(thw->thw_DMArgs.MapY * BLKH );

	if( path = thw->thw_CurrentPath )
	{

		for( psg = (struct PathSeg *)path->pth_Segments.mlh_Head;
			psg->psg_Node.mln_Succ && !match;
			psg = (struct PathSeg *)psg->psg_Node.mln_Succ )
		{
			/* check for seg endpoint node */
			if( ( abs( x - psg->psg_XPos ) < MKSIZE ) &&
				( abs( y - psg->psg_YPos ) < MKSIZE ) )
			{
				match = psg;
				*objtype = POBJ_NODE;
			}

			/* check ctrl points etc here. */

		}
	}

	return( match );
}


/********************************************************/

static struct PathSeg *PickSegBody( struct THAPWindow *thw,
	WORD mx, WORD my, BOOL nontagged )
{
	struct PathSeg *psg, *predseg, *match;
	struct Path *path;
	WORD x,y;
	double dist,nx,ny;

	match = NULL;

	/* transform back to pixel coords on the map */
	x = ( ( mx - VIEWLEFT ) / thw->thw_DMArgs.ZoomFactor ) +
		(thw->thw_DMArgs.MapX * BLKW );
	y = ( ( my - VIEWTOP ) / thw->thw_DMArgs.ZoomFactor ) +
		(thw->thw_DMArgs.MapY * BLKH );

	if( path = thw->thw_CurrentPath )
	{

		for( psg = (struct PathSeg *)path->pth_Segments.mlh_Head;
			psg->psg_Node.mln_Succ && !match;
			psg = (struct PathSeg *)psg->psg_Node.mln_Succ )
		{
			switch( psg->psg_Type )
			{
				case PSGTYPE_HEAD:
					predseg = psg;
					break;
				case PSGTYPE_LINE:
					if( predseg )		//sanity check
					{
						if( !nontagged || ( psg->psg_Tagged == 0 ) )
						{
							/* minimax test first */
							if( ( (x+MKSIZE) >= min( psg->psg_XPos, predseg->psg_XPos ) ) &&
								( (x-MKSIZE) <=  max( psg->psg_XPos, predseg->psg_XPos ) ) &&
								( (y+MKSIZE) >= min( psg->psg_YPos, predseg->psg_YPos ) ) &&
								( (y-MKSIZE) <= max( psg->psg_YPos, predseg->psg_YPos ) ) )
							{
								/* now check distance from line */

								nx = 0.0-(double)( psg->psg_YPos - predseg->psg_YPos );
								ny = (double)( psg->psg_XPos - predseg->psg_XPos );
								dist = sqrt( (nx*nx) + (ny*ny) );
								nx = nx/dist;
								ny = ny/dist;

								dist = (double)(x-predseg->psg_XPos)*nx + (double)(y-predseg->psg_YPos)*ny;

								if( dist >= -4.0 && dist <= 4.0 )
								{
									match = psg;
								}
							}
						}
						predseg = psg;
					}
					break;
			}
		}
	}

	return( match );
}

/********************************************************/

static void TagAllSegs( struct THAPWindow *thw, UBYTE state )
{
	struct Path *path;
	struct PathSeg *psg;

	if( path = thw->thw_CurrentPath )
	{
		for( psg = (struct PathSeg *)path->pth_Segments.mlh_Head;
			psg->psg_Node.mln_Succ;
			psg = (struct PathSeg *)psg->psg_Node.mln_Succ )
		{
			if( psg->psg_Tagged != state && psg->psg_Type == PSGTYPE_LINE )
			{
				psg->psg_Tagged = state;
				DrawPathSeg( thw, psg );
			}
		}
	}
}

/********************************************************/

static void SetSpeed( struct THAPWindow *thw )
{
	struct Path *path;
	struct PathSeg *psg;

	// speed limiting here please...

	if( path = thw->thw_CurrentPath )
	{
		for( psg = (struct PathSeg *)path->pth_Segments.mlh_Head;
			psg->psg_Node.mln_Succ;
			psg = (struct PathSeg *)psg->psg_Node.mln_Succ )
		{
			if( psg->psg_Tagged == 1 )
				psg->psg_Speed = thw->thw_Speed;
		}
	}
}


/********************************************************/

static void DeleteLines( struct THAPWindow *thw )
{
	struct Path *path;
	struct PathSeg *psg, *nextpsg;

	if( path = thw->thw_CurrentPath )
	{
		UnDrawPath( thw );
		psg = (struct PathSeg *)path->pth_Segments.mlh_Head;
		while( psg->psg_Node.mln_Succ )
		{
			nextpsg = (struct PathSeg *)psg->psg_Node.mln_Succ;
			if( psg->psg_Tagged == 1 && psg->psg_Type == PSGTYPE_LINE )
			{
				Remove( (struct Node *)&psg->psg_Node );
				FreeVec( psg );
			}
			psg = nextpsg;
		}
		DrawPath( thw );
	}
}

/********************************************************/


static void SubDivideLines( struct THAPWindow *thw )
{
	struct Path *path;
	struct PathSeg *predpsg, *newpsg, *psg;

	if( path = thw->thw_CurrentPath )
	{
		UnDrawPath( thw );
		ClipToMapView( thw );
		predpsg = NULL;
		for( psg = (struct PathSeg *)path->pth_Segments.mlh_Head;
			psg->psg_Node.mln_Succ;
			psg = (struct PathSeg *)psg->psg_Node.mln_Succ )
		{
			if( psg->psg_Tagged == 1 && predpsg && psg->psg_Type == PSGTYPE_LINE )
			{
//				if( predpsg->psg_XPos != psg->psg_XPos &&
//					predpsg->psg_YPos != psg->psg_YPos )
//				{
					if( newpsg = AllocVec( sizeof( struct PathSeg ), MEMF_ANY|MEMF_CLEAR ) )
					{
						UnDrawPathSeg( thw, psg );

						newpsg->psg_Type = PSGTYPE_LINE;
						newpsg->psg_Tagged = 1;
						newpsg->psg_XPos = predpsg->psg_XPos +
							( ( psg->psg_XPos - predpsg->psg_XPos ) / 2 );
						newpsg->psg_YPos = predpsg->psg_YPos +
							( ( psg->psg_YPos - predpsg->psg_YPos ) / 2 );
						newpsg->psg_Speed = psg->psg_Speed;

						Insert( (struct List *)&path->pth_Segments,
							(struct Node *)newpsg, (struct Node *)predpsg );

					}
//				}
			}
			predpsg = psg;
		}
		UnclipWindow( WIN );
		DrawPath( thw );
	}
}

/********************************************************/

void RethinkTHAPBackgrounds( void )
{
	struct SuperWindow *sw;
	struct THAPWindow *thw;
	struct Chunk *cnk;

	for( sw = (struct SuperWindow *)superwindows.lh_Head;
		sw->sw_Node.ln_Succ;
		sw = (struct SuperWindow *)sw->sw_Node.ln_Succ )
	{
		if( sw->sw_Type == SWTY_THAP )
		{
			thw = (struct THAPWindow *)sw;

			if( thw->thw_DMArgs.Map )
			{
				cnk = FindMAPByName( thw->thw_MapName );
				if( cnk )
				{
					thw->thw_DMArgs.Map = (struct Map *)cnk->ch_Data;
					thw->thw_Layout[ GID_MAP ].ho_Attr0 = (LONG)thw->thw_DMArgs.Map->map_Name;
					HO_RefreshObject( &thw->thw_Layout[ GID_MAP ], thw->thw_sw.sw_hos );
				}
				else
				{
					thw->thw_DMArgs.Map = NULL;
					thw->thw_Layout[ GID_MAP ].ho_Attr0 = NULL;
					HO_RefreshObject( &thw->thw_Layout[ GID_MAP ], thw->thw_sw.sw_hos );
				}
			}

			if( thw->thw_DMArgs.Blockset )
			{
				cnk = FindBLKSByName( thw->thw_BlocksetName );
				if( cnk )
				{
					thw->thw_DMArgs.Blockset = (struct Blockset *)cnk->ch_Data;
					thw->thw_Layout[ GID_BLKS ].ho_Attr0 = (LONG)thw->thw_DMArgs.Blockset->bs_Name;
					HO_RefreshObject( &thw->thw_Layout[ GID_BLKS ], thw->thw_sw.sw_hos );
				}
				else
				{
					thw->thw_DMArgs.Blockset = NULL;
					thw->thw_Layout[ GID_BLKS ].ho_Attr0 = NULL;
					HO_RefreshObject( &thw->thw_Layout[ GID_BLKS ], thw->thw_sw.sw_hos );
				}
			}

			DrawMapView( thw );
			DrawPath( thw );
		}
	}
}
