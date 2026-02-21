/********************************************************/
//
// TimedWaveWindow.c
//
/********************************************************/

#define TIMEDWAVEWINDOW_C

#include <stdio.h>
#include <string.h>
//#include <stdarg.h>
//#include <math.h>

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

struct TimedWaveWindow
{
	struct SuperWindow	tww_sw;
	struct Chunk				*tww_Chunk;
//	struct WaveInfoNode	*tww_CurrentWave;
	struct HoopyObject	*tww_Layout;
	/* The WaveInfoQueue is an viewable/selectable representation of the	*/
	/* TimedWave queue in the Chunk struct. 															*/
	struct List					tww_WaveInfoQueue;
	UBYTE								tww_NameBuf[ GENERICNAMESIZE ];
};

struct WaveInfoNode
{
	struct Node Noddy;
	UBYTE InfoString[ 8 + GENERICNAMESIZE ];
	struct TimedWave *TWave;
};


#define GID_BORING 0
#define GID_WAVELIST 1
#define GID_ADD 3
#define GID_DELETE 4
#define GID_EDITGROUP 5

#define GID_TIME 7


#define GID_TYPE 10
#define GID_EVENT 11
#define GID_PICKEVENT 12

#define GID_X 14
#define GID_Y 15

#define GID_VIEWRELATIVE 16

static UBYTE *typelabels[] = { "ActionList", "InitBadDude",
	"Formation", NULL };

static struct HoopyObject gadlayout[] =
{
	{ HOTYPE_VGROUP, 100, 0,3,0,0,0, NULL, 0,GID_BORING,0,0,0 },
		{ HOTYPE_LISTVIEW, 100, HOFLG_NOBORDER|HOFLG_FREEWIDTH|HOFLG_FREEHEIGHT, 0,0,0,0, "Timed Events",0,GID_WAVELIST,PLACETEXT_ABOVE,0,0 },
		{ HOTYPE_HGROUP, 100, HOFLG_NOBORDER,2,0,0,0, NULL, 0,GID_BORING,0,0,0 },
			{ HOTYPE_BUTTON, 100, HOFLG_NOBORDER, 0,0,0,0, "Add",0,GID_ADD,PLACETEXT_IN,0,0 },
			{ HOTYPE_BUTTON, 100, HOFLG_NOBORDER, 0,0,0,0, "Delete",0,GID_DELETE,PLACETEXT_IN,0,0 },
		{ HOTYPE_VGROUP, 100, HOFLG_NOBORDER,4,0,0,0, NULL, 0,GID_EDITGROUP,0,0,0 },
			{ HOTYPE_HGROUP, 100, HOFLG_FREEWIDTH,2,0,0,0, NULL, 0,GID_BORING,0,0,0 },
				{ HOTYPE_INTEGER, 100, HOFLG_NOBORDER, 6,0,0,0, "Time (Frame #)",0,GID_TIME,PLACETEXT_LEFT,0,0 },
				{ HOTYPE_SPACE, 100, HOFLG_FREEWIDTH|HOFLG_NOBORDER, 0,0,0,0, NULL,0,GID_BORING,0,0,0 },
			{ HOTYPE_HGROUP, 100, HOFLG_FREEWIDTH,3,0,0,0, NULL, 0,GID_BORING,0,0,0 },
				{ HOTYPE_CYCLE, 100, HOFLG_NOBORDER, (ULONG)typelabels,0,0,0, NULL,0,GID_TYPE,0,0,0 },
				{ HOTYPE_PACKEDSTRING, 100, HOFLG_NOBORDER, 0,GENERICNAMESIZE-1,0,0, NULL,0,GID_EVENT,0,0,0 },
				{ HOTYPE_BUTTON, 100, HOFLG_NOBORDER, 0,0,0,0, "?",0,GID_PICKEVENT,PLACETEXT_IN,0,0 },
			{ HOTYPE_HGROUP, 100, HOFLG_FREEWIDTH,2,0,0,0, NULL, 0,GID_BORING,0,0,0 },
				{ HOTYPE_FIXEDPOINT, 100, HOFLG_NOBORDER, 8,0,0,0, "X",0,GID_X,PLACETEXT_LEFT,0,0 },
				{ HOTYPE_FIXEDPOINT, 100, HOFLG_NOBORDER, 8,0,0,0, "Y",0,GID_Y,PLACETEXT_LEFT,0,0 },
			{ HOTYPE_CHECKBOX, 100, 0, 0,0,0,0, "ViewRelative?",0,GID_VIEWRELATIVE,PLACETEXT_LEFT,0,0 },
	{HOTYPE_END}
};


static void IDCMPHandler( struct SuperWindow *sw,
	struct IntuiMessage *imsg );
static void HideWindow( struct SuperWindow *sw );
static BOOL ShowWindow( struct SuperWindow *sw );
static void KillWindow( struct SuperWindow *sw );
static BOOL RefreshMenus( struct SuperWindow *sw );
static UBYTE *MakeTWAVWinTitle( struct SuperWindow *sw );

static struct WaveInfoNode *FindCurrentWave( struct TimedWaveWindow *tww );
static VOID FillInWaveInfoNode( struct WaveInfoNode *winfo,
	struct TimedWave *twave );
static VOID EnqueueWaveInfoNode( struct TimedWaveWindow *tww,
	struct WaveInfoNode *winfo );
static BOOL InitWaveInfoQueue( struct TimedWaveWindow *tww );
static VOID UpdateEditFields( struct TimedWaveWindow *tww );
static VOID ReadNewMiscFields( struct TimedWaveWindow *tww );
static VOID ReadNewEvent( struct TimedWaveWindow *tww );
static VOID ReadNewTime( struct TimedWaveWindow *tww );
static VOID AddWave( struct TimedWaveWindow *tww );
static VOID DeleteWave( struct TimedWaveWindow *tww );

//ULONG KPrintF( STRPTR, ... );

/********************************************************/

BOOL OpenNewTimedWaveWindow( struct Chunk *cnk )
{
	struct SuperWindow *sw;
	struct TimedWaveWindow *tww;
	BOOL success = FALSE;

	assert( cnk != NULL );
	assert( cnk->ch_TypeID == ID_TWAV );
	
	for( sw = (struct SuperWindow *)superwindows.lh_Head;
		sw->sw_Node.ln_Succ;
		sw = (struct SuperWindow *)sw->sw_Node.ln_Succ )
	{
		if( sw->sw_Type == SWTY_TIMEDWAVE )
		{
			tww = (struct TimedWaveWindow *)sw;
			if( tww->tww_Chunk == cnk )
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

	if( tww = AllocVec( sizeof( struct TimedWaveWindow ), MEMF_ANY|MEMF_CLEAR ) )
	{
		sw = &tww->tww_sw;
		sw->sw_Handler = IDCMPHandler;
		sw->sw_HideWindow = HideWindow;
		sw->sw_ShowWindow = ShowWindow;
		sw->sw_BuggerOff = KillWindow;
		sw->sw_RefreshMenus = RefreshMenus;
		sw->sw_MakeTitle	= MakeTWAVWinTitle;
		sw->sw_Type				= SWTY_TIMEDWAVE;
		sw->sw_Window			= NULL;
		sw->sw_Gadgets		= NULL;
		sw->sw_VisualInfo	= NULL;
		sw->sw_WinDim			= zonkfig.SuperWindowDefs[ SWTY_TIMEDWAVE ];

		tww->tww_Chunk = cnk;
		NewList( &tww->tww_WaveInfoQueue );

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
	struct TimedWaveWindow *tww;
	struct Window *win;
	struct Screen *scr;

	assert( sw != NULL );

	tww = (struct TimedWaveWindow *)sw;

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
		TAG_DONE,				NULL);

	UnlockMainScreen( scr );

	if( sw->sw_Window = win )
	{
		if( !zonkfig.scfg.scfg_DefaultFont && mainwinfont )
			SetFont( win->RPort, mainwinfont );

		if( tww->tww_Layout = AllocVec( sizeof( gadlayout ), MEMF_ANY ) )
		{
			CopyMem( gadlayout, tww->tww_Layout, sizeof( gadlayout ) );
			if( sw->sw_hos = HO_GetHOStuff( win, win->RPort->Font ) )
			{

				/* Tell the window which superwindow it is attached to */
				/* (so we'll know which handler routine to call) */
				win->UserData = (BYTE *)sw;

				/* Set up some menus */
				sw->sw_VisualInfo = sw->sw_hos->hos_vi;
				GenericMakeMenus( sw, NULL );

				/* set up the WaveInfoNode queue for the Listview */
				InitWaveInfoQueue( tww );
				tww->tww_Layout[ GID_WAVELIST ].ho_Attr0 = (LONG)&tww->tww_WaveInfoQueue;
				tww->tww_Layout[ GID_WAVELIST ].ho_Value = ~0;

				/* set up buffer for event name */
				tww->tww_NameBuf[0] = '\0';
				tww->tww_Layout[ GID_EVENT ].ho_Attr0 = (LONG)tww->tww_NameBuf;

				/* we want to use our global message port for IDCMP stuff */
				win->UserPort = mainmsgport;
				ModifyIDCMP( win, IDCMP_REFRESHWINDOW|IDCMP_NEWSIZE|
					IDCMP_CLOSEWINDOW|IDCMP_CHANGEWINDOW|IDCMP_MENUPICK|
					STRINGIDCMP|INTEGERIDCMP|LISTVIEWIDCMP|BUTTONIDCMP );

				HO_SussWindowSizing( sw->sw_hos, tww->tww_Layout );

				HO_MakeLayout( tww->tww_Layout, sw->sw_hos, win->BorderLeft, win->BorderTop,
					win->Width - win->BorderLeft - win->BorderRight,
					win->Height - win->BorderTop - win->BorderBottom );

				UpdateEditFields( tww );

				success = TRUE;
			}
		}
	}

	if( !success )
	{
		if( sw->sw_Window ) CloseWindow( sw->sw_Window );
		sw->sw_Window = NULL;
		if( tww->tww_Layout ) FreeVec( tww->tww_Layout );
		tww->tww_Layout = NULL;
	}

	return( success );
}


/********************************************************/

static void HideWindow( struct SuperWindow *sw )
{
	struct TimedWaveWindow *tww;

	assert( sw != NULL );

	tww = (struct TimedWaveWindow *)sw;

	if( sw->sw_Window )
	{
		RememberWindow( sw );
		GenericZapMenus( sw );
		HO_KillLayout( sw->sw_hos );
		CloseWindowSafely( sw->sw_Window );
		HO_FreeHOStuff( sw->sw_hos );
		sw->sw_Window = NULL;
		FreeVec( tww->tww_Layout );
		tww->tww_Layout = NULL;

		FreeNodes( &tww->tww_WaveInfoQueue );
	}
}

/********************************************************/

static void KillWindow( struct SuperWindow *sw )
{

	assert( sw != NULL );

	if( IsOKToClose( sw ) )
	{
		(*sw->sw_HideWindow)(sw);
		UnlockChunk( ((struct TimedWaveWindow *)sw)->tww_Chunk );
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

static UBYTE *MakeTWAVWinTitle( struct SuperWindow *sw )
{
	struct Chunk *cnk;
	UBYTE *p;

	cnk = ((struct TimedWaveWindow *)sw)->tww_Chunk;
	p = sw->sw_Name;

	if( cnk->ch_Modified )
		*p++ = MODCHAR;
	strcpy( p, "Timed Waves" );

	return( sw->sw_Name );
}

/********************************************************/

static void IDCMPHandler( struct SuperWindow *sw,
	struct IntuiMessage *origimsg )
{
	struct IntuiMessage *imsg;
	struct Window *win;
	struct TimedWaveWindow *tww;

	ULONG class;
	UWORD code;
//	UWORD qualifier;
	APTR iaddress;
	WORD mousex, mousey;
	ULONG seconds, micros;

	tww = (struct TimedWaveWindow *)sw;
	win = sw->sw_Window;

	if( imsg = GT_FilterIMsg( origimsg ) )
	{
		HO_CheckIMsg( imsg );

		class = imsg->Class;
		code = imsg->Code;
//		qualifier = imsg->Qualifier;
		iaddress = imsg->IAddress;
		mousex = imsg->MouseX;
		mousey = imsg->MouseY;
		seconds = imsg->Seconds;
		micros = imsg->Micros;

		GT_PostFilterIMsg( imsg );

		switch( class )
		{
			case IDCMP_CLOSEWINDOW:
				ReplyMsg( (struct Message *)origimsg );
				KillWindow( sw );
				break;
			case IDCMP_NEWSIZE:
				ClearWindow( win );
				HO_ReadGadgetStates( sw->sw_hos, tww->tww_Layout );
				HO_MakeLayout( tww->tww_Layout, sw->sw_hos, win->BorderLeft, win->BorderTop,
					win->Width - win->BorderLeft - win->BorderRight,
					win->Height - win->BorderTop - win->BorderBottom );
				ReplyMsg( (struct Message *)origimsg );
				break;
			case IDCMP_CHANGEWINDOW:
				UpdateWinDef( sw );
				ReplyMsg( (struct Message *)origimsg );
				break;
			case IDCMP_GADGETUP:
				ReplyMsg( (struct Message *)origimsg );
				HO_ReadGadgetStates( sw->sw_hos, tww->tww_Layout );
				switch( ((struct Gadget*)iaddress)->GadgetID )
				{
					case GID_WAVELIST:
						UpdateEditFields( tww );
						break;
					case GID_ADD:
						ModifyChunk( tww->tww_Chunk );
						AddWave( tww );
						break;
					case GID_DELETE:
						ModifyChunk( tww->tww_Chunk );
						DeleteWave( tww );
						break;
					case GID_EVENT:
						ModifyChunk( tww->tww_Chunk );
						ReadNewEvent( tww );
						break;
					case GID_PICKEVENT:
						ModifyChunk( tww->tww_Chunk );
						break;
					case GID_TIME :
						ModifyChunk( tww->tww_Chunk );
						ReadNewTime( tww );
						break;
					case GID_X :
					case GID_Y :
					case GID_VIEWRELATIVE:
					case GID_TYPE:
						ModifyChunk( tww->tww_Chunk );
						ReadNewMiscFields( tww );
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
//
// CreateWaveInfoNode() - creates a WaveInfo struct for display
// in the listview. Free with FreeVec().
//

static struct WaveInfoNode *OBSCreateWaveInfoNode( struct TimedWave *twave )
{
	struct WaveInfoNode *winfo = NULL;

	if( winfo = AllocVec( sizeof( struct WaveInfoNode ), MEMF_ANY|MEMF_CLEAR ) )
	{
		sprintf( winfo->InfoString, "%6d %s", twave->Time, twave->Event );
		winfo->TWave = twave;
		winfo->Noddy.ln_Name = winfo->InfoString;
	}

	return( winfo );
}



/********************************************************/

static VOID FillInWaveInfoNode( struct WaveInfoNode *winfo,
	struct TimedWave *twave )
{
	sprintf( winfo->InfoString, "%6d %s", twave->Time, twave->Event );
	winfo->TWave = twave;
	winfo->Noddy.ln_Name = winfo->InfoString;
}



/********************************************************/
//
// InitWaveInfoQueue() - Sets up the WaveInfoQueue so it can be used
// in the listview. Inits the List struct, Creates a WaveInfo struct
// for each existing TimedWave and adds them in. It's assumed that the
// TimedWaves are already sorted. The queue must always reflect the
// the order of the Chunk TimedWaves queue.
// Can free the Queue with FreeNodes();

static BOOL InitWaveInfoQueue( struct TimedWaveWindow *tww )
{
	struct TimedWave *twave;
	struct Chunk *cnk;
	struct WaveInfoNode *winfo;
	BOOL allok;

	allok = TRUE;
	cnk = tww->tww_Chunk;
	NewList( &tww->tww_WaveInfoQueue );

	for( twave = (struct TimedWave *)cnk->ch_DataList.lh_Head;
		twave->Noddy.ln_Succ && allok;
		twave = (struct TimedWave *)twave->Noddy.ln_Succ )
	{
		if( winfo = AllocVec( sizeof( struct WaveInfoNode ), MEMF_ANY|MEMF_CLEAR ) )
		{
			FillInWaveInfoNode( winfo, twave );
			AddTail( &tww->tww_WaveInfoQueue, &winfo->Noddy );
		}
		else
			allok = FALSE;
	}

	if( !allok )
		FreeNodes( &tww->tww_WaveInfoQueue );

	return( allok );
}



/********************************************************/
//
// Enqueue a WaveInfoNode according to its Time field.
// Corresponds to, and should be matched with, EnqueueTimedWave().
// (see TWAVStuff.c)

static VOID EnqueueWaveInfoNode( struct TimedWaveWindow *tww,
	struct WaveInfoNode *winfo )
{
	struct WaveInfoNode *prevwinfo;

	prevwinfo = (struct WaveInfoNode *)tww->tww_WaveInfoQueue.lh_Head;
	while( prevwinfo->Noddy.ln_Succ &&
		prevwinfo->TWave->Time < winfo->TWave->Time )
	{
		prevwinfo = (struct WaveInfoNode *)prevwinfo->Noddy.ln_Succ;
	}
	Insert( &tww->tww_WaveInfoQueue, &winfo->Noddy, prevwinfo->Noddy.ln_Pred );
}


/********************************************************/
//
//

static VOID UpdateEditFields( struct TimedWaveWindow *tww )
{
	struct WaveInfoNode *winfo;
	struct TimedWave *twave;

	if( winfo = FindCurrentWave( tww ) )
	{
		twave = winfo->TWave;

		Mystrncpy( tww->tww_NameBuf, twave->Event,
			GENERICNAMESIZE-1 );

		tww->tww_Layout[ GID_TIME ].ho_Value = (LONG)twave->Time;
		tww->tww_Layout[ GID_X ].ho_Value = (LONG)twave->X;
		tww->tww_Layout[ GID_Y ].ho_Value = (LONG)twave->Y;
		tww->tww_Layout[ GID_VIEWRELATIVE ].ho_Value =
			(LONG)( (twave->Flags & TWFLAG_VIEWRELATIVE) ? TRUE : FALSE );
		tww->tww_Layout[ GID_TYPE ].ho_Value = (LONG)twave->Type;

		HO_RefreshObject( &tww->tww_Layout[ GID_EDITGROUP ], tww->tww_sw.sw_hos );

		HO_DisableObject( &tww->tww_Layout[ GID_EDITGROUP ], tww->tww_sw.sw_hos,
			FALSE );
		HO_DisableObject( &tww->tww_Layout[ GID_DELETE ], tww->tww_sw.sw_hos,
			FALSE );
	}
	else
	{
		Mystrncpy( tww->tww_NameBuf, defname,
			GENERICNAMESIZE-1 );

		HO_DisableObject( &tww->tww_Layout[ GID_EDITGROUP ], tww->tww_sw.sw_hos,
			TRUE );
		HO_DisableObject( &tww->tww_Layout[ GID_DELETE ], tww->tww_sw.sw_hos,
			TRUE );
	}
}

/********************************************************/
static struct WaveInfoNode *FindCurrentWave( struct TimedWaveWindow *tww )
{
	UWORD num;
	struct WaveInfoNode *winfo;

	num = (UWORD)tww->tww_Layout[ GID_WAVELIST ].ho_Value;

	winfo = NULL;
	if( num != ~0 )
		winfo = (struct WaveInfoNode *)FindNthNode( &tww->tww_WaveInfoQueue, num );

	return( winfo );
}

/********************************************************/

static VOID ReadNewMiscFields( struct TimedWaveWindow *tww )
{
	struct TimedWave *twave;
	struct WaveInfoNode *winfo;

	if( winfo = FindCurrentWave( tww ) )
	{
		twave = winfo->TWave;
		twave->X = tww->tww_Layout[ GID_X ].ho_Value;
		twave->Y = tww->tww_Layout[ GID_Y ].ho_Value;
		if( tww->tww_Layout[ GID_VIEWRELATIVE ].ho_Value )
			twave->Flags |= TWFLAG_VIEWRELATIVE;
		else
			twave->Flags &= ~TWFLAG_VIEWRELATIVE;

		twave->Type = (UBYTE)tww->tww_Layout[ GID_TYPE ].ho_Value;
	}
}


/********************************************************/

static VOID ReadNewEvent( struct TimedWaveWindow *tww )
{
	struct WaveInfoNode *winfo;

	if( winfo = FindCurrentWave( tww ) )
	{
		/* detach list from listview gadget before fucking around with it... */
		tww->tww_Layout[ GID_WAVELIST ].ho_Attr0 = ~0;
		HO_RefreshObject( &tww->tww_Layout[ GID_WAVELIST ], tww->tww_sw.sw_hos );
		
		MakePackable( tww->tww_NameBuf );
		Mystrncpy( winfo->TWave->Event, tww->tww_NameBuf,
			GENERICNAMESIZE-1 );
		FillInWaveInfoNode( winfo, winfo->TWave );

		HO_RefreshObject( &tww->tww_Layout[ GID_EVENT ], tww->tww_sw.sw_hos );

		/* redisplay list */
		tww->tww_Layout[ GID_WAVELIST ].ho_Attr0 = (LONG)&tww->tww_WaveInfoQueue;
		HO_RefreshObject( &tww->tww_Layout[ GID_WAVELIST ], tww->tww_sw.sw_hos );
	}
}

/********************************************************/

static VOID ReadNewTime( struct TimedWaveWindow *tww )
{
	struct TimedWave *twave;
	struct WaveInfoNode *winfo;

	if( winfo = FindCurrentWave( tww ) )
	{
		/* detach list from listview gadget before fucking around with it... */
		tww->tww_Layout[ GID_WAVELIST ].ho_Attr0 = ~0;
		HO_RefreshObject( &tww->tww_Layout[ GID_WAVELIST ], tww->tww_sw.sw_hos );

		twave = winfo->TWave;

		/* remove wave from lists */
		Remove( &winfo->Noddy );
		Remove( &twave->Noddy );

		/* update time field */
		twave->Time = (UWORD)tww->tww_Layout[ GID_TIME ].ho_Value;
		FillInWaveInfoNode( winfo, winfo->TWave );

		/* put wave back at new position */
		EnqueueTimedWave( &tww->tww_Chunk->ch_DataList, twave );
		EnqueueWaveInfoNode( tww, winfo );

		/* redisplay list */
		tww->tww_Layout[ GID_WAVELIST ].ho_Value =
			FindNodeNum( &tww->tww_WaveInfoQueue, &winfo->Noddy );
		tww->tww_Layout[ GID_WAVELIST ].ho_Attr0 = (LONG)&tww->tww_WaveInfoQueue;
		HO_RefreshObject( &tww->tww_Layout[ GID_WAVELIST ], tww->tww_sw.sw_hos );

		UpdateEditFields( tww );

	}
}




/********************************************************/

static VOID AddWave( struct TimedWaveWindow *tww )
{
	struct TimedWave *twav;
	struct WaveInfoNode *winfo;

	/* detach list from listview gadget before fucking around with it... */
	tww->tww_Layout[ GID_WAVELIST ].ho_Attr0 = ~0;
	HO_RefreshObject( &tww->tww_Layout[ GID_WAVELIST ], tww->tww_sw.sw_hos );

	if( twav = AllocVec( sizeof( struct TimedWave ),MEMF_ANY|MEMF_CLEAR ) )
	{
		/* time=0, so put at start of queue */
		twav->Event[0] = '\0';

		if( winfo = AllocVec( sizeof( struct WaveInfoNode ), MEMF_ANY|MEMF_CLEAR ) )
		{
			FillInWaveInfoNode( winfo, twav );
			AddHead( &tww->tww_Chunk->ch_DataList, &twav->Noddy );
			AddHead( &tww->tww_WaveInfoQueue, &winfo->Noddy );
		}
		else
			FreeVec( twav );
	}

	/* reshow list */
	tww->tww_Layout[ GID_WAVELIST ].ho_Value = 0;					/* select new wave */
	tww->tww_Layout[ GID_WAVELIST ].ho_Attr0 = (ULONG)&tww->tww_WaveInfoQueue;
	HO_RefreshObject( &tww->tww_Layout[ GID_WAVELIST ], tww->tww_sw.sw_hos );

	UpdateEditFields( tww );
}



/********************************************************/

static VOID DeleteWave( struct TimedWaveWindow *tww )
{
	struct TimedWave *twave;
	struct WaveInfoNode *winfo;
	UWORD num;

	if( winfo = FindCurrentWave( tww ) )
	{
		/* detach list from listview gadget before fucking around with it... */
		tww->tww_Layout[ GID_WAVELIST ].ho_Attr0 = ~0;
		HO_RefreshObject( &tww->tww_Layout[ GID_WAVELIST ], tww->tww_sw.sw_hos );

		twave = winfo->TWave;

		/* remove wave from lists */
		Remove( &winfo->Noddy );
		Remove( &twave->Noddy );

		FreeVec( winfo );
		FreeVec( twave );

		/* figure out which wave should now be selected */
		num = CountNodes( &tww->tww_WaveInfoQueue );
		if( tww->tww_Layout[ GID_WAVELIST ].ho_Value >= num )
		{
			if( num > 0 )
				tww->tww_Layout[ GID_WAVELIST ].ho_Value = num-1;
			else
				tww->tww_Layout[ GID_WAVELIST ].ho_Value = ~0;
		}

		/* redisplay list */
		tww->tww_Layout[ GID_WAVELIST ].ho_Attr0 = (LONG)&tww->tww_WaveInfoQueue;
		HO_RefreshObject( &tww->tww_Layout[ GID_WAVELIST ], tww->tww_sw.sw_hos );

		/* show the selected wave */
		UpdateEditFields( tww );
	}
}
