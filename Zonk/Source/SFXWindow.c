/********************************************************/
//
// SFXWindow.c
//
/********************************************************/

#define SFXWINDOW_C

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
#include <proto/iffparse.h>

#include "hoopy.h"
#include "global.h"

struct SFXWindow
{
	struct SuperWindow	fxw_sw;
	struct Chunk				*fxw_Chunk;
	struct HoopyObject	*fxw_Layout;
	struct SFXHeader		*fxw_CurrentEffect;
	UBYTE								fxw_NameBuf[ SFXNAMESIZE ];
};


/* `VHDR' header format. */

struct Voice8Header
{
		ULONG	oneShotHiSamples,	/* # samples in the high octave 1-shot part */
			repeatHiSamples,			/* # samples in the high octave repeat part */
			samplesPerHiCycle;		/* # samples/cycle in high octave, else 0 */
		UWORD	samplesPerSec;		/* data sampling rate */
		UBYTE	ctOctave,					/* # of octaves of waveforms */
			sCompression;					/* data compression technique used */
		LONG	volume;						/* playback nominal volume from 0 to Unity
														 * (full volume). Map this value into
														 * the output hardware's dynamic range.
														 */
};



#define GAD_BORING 0
#define GAD_1 1
#define GAD_2 2
#define GAD_3 3
#define GAD_4 4

#define	GAD_SFXLIST			2
#define	GAD_IMPORTRAW		3
#define	GAD_IMPORT8SVX	4
#define	GAD_EDITGROUP		6
#define	GAD_NAME				7
#define	GAD_LENGTH			8
#define	GAD_PRI					9
#define	GAD_VOLUME			10
#define	GAD_RATE			11
#define	GAD_CYCLES			12
#define	GAD_PLAY1				14
#define	GAD_PLAY2				15
#define	GAD_PLAY3				16
#define	GAD_PLAY4				17
#define	GAD_STOP				18
#define	GAD_DELETE			20


static struct HoopyObject gadlayout[] =
{
	{ HOTYPE_HGROUP, 100, HOFLG_NOBORDER,3,0,0,0, NULL, 0,GAD_BORING,0,0,0 },
		{ HOTYPE_VGROUP, 100, HOFLG_FREEWIDTH|HOFLG_FREEHEIGHT,3,0,0,0, NULL, 0,0,GAD_BORING,0,0 },
			{ HOTYPE_LISTVIEW, 100, HOFLG_NOBORDER, 0,0,0,0, "SFX",0,GAD_SFXLIST,PLACETEXT_ABOVE,0 },
			{ HOTYPE_BUTTON, 100, HOFLG_NOBORDER|HOFLG_FREEWIDTH, 0,0,0,0, "Raw...",0,GAD_IMPORTRAW,PLACETEXT_IN,0 },
			{ HOTYPE_BUTTON, 100, HOFLG_NOBORDER|HOFLG_FREEWIDTH, 0,0,0,0, "8SVX...",0,GAD_IMPORT8SVX,PLACETEXT_IN,0 },
		{ HOTYPE_VLINE, 100, 0,0,0,0, NULL, 0,0,GAD_BORING,0,0 },
		{ HOTYPE_VGROUP, 100, HOFLG_NOBORDER|HOFLG_FREEWIDTH|HOFLG_FREEHEIGHT,9,0,0,0, NULL, 0,GAD_EDITGROUP,0,0 },
			{ HOTYPE_STRING, 100, HOFLG_FREEWIDTH, 0,SFXNAMESIZE-1,0,0, "Name",0,GAD_NAME,PLACETEXT_RIGHT,0 },
			{ HOTYPE_NUMBER, 100, HOFLG_FREEWIDTH, 42,0,0,0, "Length",69,GAD_LENGTH,PLACETEXT_RIGHT,0 },
			{ HOTYPE_INTEGER, 100, 0, 4,0,0,0, "Pri (-128..127)",0,GAD_PRI,PLACETEXT_RIGHT,0 },
			{ HOTYPE_INTEGER, 100, 0, 3,0,0,0, "Vol (0-63)",0,GAD_VOLUME,PLACETEXT_RIGHT,0 },
			{ HOTYPE_INTEGER, 100, 0, 6,0,0,0, "Samples/Sec",0,GAD_RATE,PLACETEXT_RIGHT,0 },
			{ HOTYPE_INTEGER, 100, 0, 6,0,0,0, "Rep (-1=frvr)",0,GAD_CYCLES,PLACETEXT_RIGHT,0 },
			{ HOTYPE_HGROUP, 100, 0 ,5,0,0,0, NULL, 0,0,GAD_BORING,0,0 },
				{ HOTYPE_BUTTON, 100, HOFLG_NOBORDER, 0,0,0,0, "1",0,GAD_PLAY1,PLACETEXT_IN,0 },
				{ HOTYPE_BUTTON, 100, HOFLG_NOBORDER, 0,0,0,0, "2",0,GAD_PLAY2,PLACETEXT_IN,0 },
				{ HOTYPE_BUTTON, 100, HOFLG_NOBORDER, 0,0,0,0, "3",0,GAD_PLAY3,PLACETEXT_IN,0 },
				{ HOTYPE_BUTTON, 100, HOFLG_NOBORDER, 0,0,0,0, "4",0,GAD_PLAY4,PLACETEXT_IN,0 },
				{ HOTYPE_BUTTON, 100, HOFLG_NOBORDER, 0,0,0,0, ".",0,GAD_STOP,PLACETEXT_IN,0 },
			{ HOTYPE_SPACE, 100, HOFLG_FREEHEIGHT, 0,0,0,0, NULL,0,GAD_BORING,0,0 },
			{ HOTYPE_BUTTON, 100, 0, 0,0,0,0, "Del",0,GAD_DELETE,PLACETEXT_IN,0 },
		{HOTYPE_END}
};

static char dummybuf[SFXNAMESIZE] = { '\0' };

/* needed for spam */
struct Task *OurTaskBase;

static void IDCMPHandler( struct SuperWindow *sw,
	struct IntuiMessage *imsg );
static void HideWindow( struct SuperWindow *sw );
static BOOL ShowWindow( struct SuperWindow *sw );
static void KillWindow( struct SuperWindow *sw );
static BOOL RefreshMenus( struct SuperWindow *sw );

static VOID fxw_SetCurrentEffect( struct SFXWindow *fxw, struct SFXHeader *sfx );
static VOID fxw_ListClick( struct SFXWindow *fxw, UWORD num );
static VOID fxw_IntClick( struct SFXWindow *fxw );
static VOID fxw_NameClick( struct SFXWindow *fxw );
static VOID fxw_ImportRaw( struct SFXWindow *fxw );
static VOID fxw_Import8SVX( struct SFXWindow *fxw );
static VOID fxw_DeleteCurrentEffect( struct SFXWindow *fxw );
struct SFXHeader *LoadRawSample( STRPTR drawer, STRPTR file );
struct SFXHeader *Load8SVXSample( STRPTR drawer, STRPTR file );

/********************************************************/

BOOL OpenNewSFXWindow( struct Chunk *cnk )
{
	struct SuperWindow *sw;
	struct SFXWindow *fxw;
	BOOL success = FALSE;

	if( !cnk ) return( FALSE );
	if( FindSuperWindowByType( (struct SuperWindow *)superwindows.lh_Head,
		SWTY_SFX ) ) return( FALSE );

	LockChunk( cnk );

	if( fxw = AllocVec( sizeof( struct SFXWindow ), MEMF_ANY|MEMF_CLEAR ) )
	{
		sw = &fxw->fxw_sw;
		sw->sw_Handler = IDCMPHandler;
		sw->sw_HideWindow = HideWindow;
		sw->sw_ShowWindow = ShowWindow;
		sw->sw_BuggerOff = KillWindow;
		sw->sw_RefreshMenus = RefreshMenus;
		sw->sw_Type				= SWTY_SFX;
		sw->sw_Window			= NULL;
		sw->sw_Gadgets		= NULL;
		sw->sw_VisualInfo	= NULL;
		sw->sw_WinDim			= zonkfig.SuperWindowDefs[ SWTY_SFX ];

		fxw->fxw_Chunk = cnk;
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
	struct SFXWindow *fxw;
	struct Window *win;
	struct Screen *scr;

	fxw = (struct SFXWindow *)sw;

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
		WA_Title,				"SFXWindow",
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

		if( fxw->fxw_Layout = AllocVec( sizeof( gadlayout ), MEMF_ANY ) )
		{
			CopyMem( gadlayout, fxw->fxw_Layout, sizeof( gadlayout ) );
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
					IDCMP_CLOSEWINDOW|IDCMP_CHANGEWINDOW|IDCMP_MENUPICK|
					LISTVIEWIDCMP|STRINGIDCMP|INTEGERIDCMP|BUTTONIDCMP );

				HO_SussWindowSizing( sw->sw_hos, fxw->fxw_Layout );

				/* Start the Spam task */
				OurTaskBase = FindTask( NULL );
				StartPlayerSubTask();

				/* don't want null stringgadgets... */
				fxw->fxw_Layout[ GAD_NAME ].ho_Attr0 = (LONG)fxw->fxw_NameBuf;
				fxw->fxw_Layout[ GAD_NAME ].ho_Attr1 = SFXNAMESIZE-1;

				fxw->fxw_Layout[ GAD_SFXLIST ].ho_Attr0 = (LONG)&fxw->fxw_Chunk->ch_DataList;

				HO_MakeLayout( fxw->fxw_Layout, sw->sw_hos, win->BorderLeft, win->BorderTop,
					win->Width - win->BorderLeft - win->BorderRight,
					win->Height - win->BorderTop - win->BorderBottom );

				fxw_SetCurrentEffect( fxw, NULL );

				success = TRUE;
			}
		}
	}

	if( !success )
	{
		if( sw->sw_Window ) CloseWindow( sw->sw_Window );
		sw->sw_Window = NULL;
		if( fxw->fxw_Layout ) FreeVec( fxw->fxw_Layout );
		fxw->fxw_Layout = NULL;
	}

	return( success );
}


/********************************************************/

static void HideWindow( struct SuperWindow *sw )
{
	struct SFXWindow *fxw;

	fxw = (struct SFXWindow *)sw;


	if( sw->sw_Window )
	{
		RememberWindow( sw );

		/* end player task (Spam)*/
		EndPlayerSubTask();

		GenericZapMenus( sw );
		HO_KillLayout( sw->sw_hos );
		CloseWindowSafely( sw->sw_Window );
		HO_FreeHOStuff( sw->sw_hos );
		sw->sw_Window = NULL;
		FreeVec( fxw->fxw_Layout );
		fxw->fxw_Layout = NULL;
	}
}

/********************************************************/

static void KillWindow( struct SuperWindow *sw )
{
	if( sw && IsOKToClose( sw ) )
	{
		(*sw->sw_HideWindow)(sw);
		UnlockChunk( ((struct SFXWindow *)sw)->fxw_Chunk );
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

static void IDCMPHandler( struct SuperWindow *sw,
	struct IntuiMessage *origimsg )
{
	struct IntuiMessage *imsg;
	struct Window *win;
	struct SFXWindow *fxw;

	ULONG class;
	UWORD code;
//	UWORD qualifier;
	APTR iaddress;
	WORD mousex, mousey;
	ULONG seconds, micros;

	fxw = (struct SFXWindow *)sw;
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
				HO_ReadGadgetStates( sw->sw_hos, fxw->fxw_Layout );
				HO_MakeLayout( fxw->fxw_Layout, sw->sw_hos, win->BorderLeft, win->BorderTop,
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
				switch( ((struct Gadget*)iaddress)->GadgetID )
				{
					case GAD_PLAY1:
						if( fxw->fxw_CurrentEffect )
						{
							PlaySample( &fxw->fxw_CurrentEffect->sfx_Spam,
								(UBYTE *)(fxw->fxw_CurrentEffect+1), 0 );
						}
						break;
					case GAD_PLAY2:
						if( fxw->fxw_CurrentEffect )
						{
							PlaySample( &fxw->fxw_CurrentEffect->sfx_Spam,
								(UBYTE *)(fxw->fxw_CurrentEffect+1), 1 );
						}
						break;
					case GAD_PLAY3:
						if( fxw->fxw_CurrentEffect )
						{
							PlaySample( &fxw->fxw_CurrentEffect->sfx_Spam,
								(UBYTE *)(fxw->fxw_CurrentEffect+1), 2 );
						}
						break;
					case GAD_PLAY4:
						if( fxw->fxw_CurrentEffect )
						{
							PlaySample( &fxw->fxw_CurrentEffect->sfx_Spam,
								(UBYTE *)(fxw->fxw_CurrentEffect+1), 3 );
						}
						break;
					case GAD_SFXLIST:
						fxw_ListClick( fxw, code );
						break;
					case GAD_PRI:
					case GAD_VOLUME:
					case GAD_RATE:
					case GAD_CYCLES:
						fxw_IntClick( fxw );
						break;
					case GAD_NAME:
						fxw_NameClick( fxw );
						break;
					case GAD_IMPORTRAW:
						fxw_ImportRaw( fxw );
						break;
					case GAD_IMPORT8SVX:
						fxw_Import8SVX( fxw );
						break;
					case GAD_DELETE:
						fxw_DeleteCurrentEffect( fxw );
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
static VOID fxw_SetCurrentEffect( struct SFXWindow *fxw, struct SFXHeader *sfx )
{
	fxw->fxw_CurrentEffect = sfx;

	if( sfx )
	{
		Mystrncpy( fxw->fxw_NameBuf, sfx->sfx_Name, SFXNAMESIZE-1 );
		fxw->fxw_Layout[ GAD_LENGTH ].ho_Attr0 = (LONG)sfx->sfx_Spam.spam_Length;
		fxw->fxw_Layout[ GAD_PRI ].ho_Value = (LONG)sfx->sfx_Spam.spam_Pri;
		fxw->fxw_Layout[ GAD_VOLUME ].ho_Value = (LONG)sfx->sfx_Spam.spam_Volume;
		fxw->fxw_Layout[ GAD_RATE ].ho_Value = (LONG)sfx->sfx_Spam.spam_SampleRate;
		fxw->fxw_Layout[ GAD_CYCLES ].ho_Value = (LONG)sfx->sfx_Spam.spam_Repeats;

		HO_RefreshObject( &fxw->fxw_Layout[ GAD_EDITGROUP ], fxw->fxw_sw.sw_hos );
		HO_DisableObject( &fxw->fxw_Layout[ GAD_EDITGROUP ], fxw->fxw_sw.sw_hos,
			FALSE );
	}
	else
	{
		fxw->fxw_NameBuf[0] = '\0';
		fxw->fxw_Layout[ GAD_LENGTH ].ho_Attr0 = 0;
		fxw->fxw_Layout[ GAD_PRI ].ho_Value = 0;
		fxw->fxw_Layout[ GAD_VOLUME ].ho_Value = 0;
		fxw->fxw_Layout[ GAD_RATE ].ho_Value = 0;
		fxw->fxw_Layout[ GAD_CYCLES ].ho_Value = 0;

		HO_RefreshObject( &fxw->fxw_Layout[ GAD_EDITGROUP ], fxw->fxw_sw.sw_hos );
		HO_DisableObject( &fxw->fxw_Layout[ GAD_EDITGROUP ], fxw->fxw_sw.sw_hos,
			TRUE );
	}
}


/********************************************************/
// Handle clicks in the listview gadget
//

static VOID fxw_ListClick( struct SFXWindow *fxw, UWORD num )
{
	struct Chunk *cnk;
	struct SFXHeader *sfx = NULL;

	if( num != 0xFFFF && (cnk = fxw->fxw_Chunk) )
		sfx = (struct SFXHeader *)FindNthNode( &cnk->ch_DataList, num );
	fxw_SetCurrentEffect( fxw, sfx );
}

/********************************************************/
// Handle changes in any of the integer gadgets

static VOID fxw_IntClick( struct SFXWindow *fxw )
{

	if( fxw->fxw_CurrentEffect )
	{
		HO_ReadGadgetStates( fxw->fxw_sw.sw_hos, fxw->fxw_Layout );

		fxw->fxw_CurrentEffect->sfx_Spam.spam_Pri =
			(BYTE)fxw->fxw_Layout[ GAD_PRI ].ho_Value;
		fxw->fxw_CurrentEffect->sfx_Spam.spam_Volume =
			(UBYTE)fxw->fxw_Layout[ GAD_VOLUME ].ho_Value;
		fxw->fxw_CurrentEffect->sfx_Spam.spam_SampleRate =
			(UWORD)fxw->fxw_Layout[ GAD_RATE ].ho_Value;
		fxw->fxw_CurrentEffect->sfx_Spam.spam_Repeats =
			(WORD)fxw->fxw_Layout[ GAD_CYCLES ].ho_Value;
	}
}

/********************************************************/
// Handle changes in the name string gadget

static VOID fxw_NameClick( struct SFXWindow *fxw )
{
	if( fxw->fxw_CurrentEffect )
	{
		HO_ReadGadgetStates( fxw->fxw_sw.sw_hos, fxw->fxw_Layout );
		fxw->fxw_Layout[ GAD_SFXLIST ].ho_Attr0 = ~0;
		HO_RefreshObject( &fxw->fxw_Layout[ GAD_SFXLIST ], fxw->fxw_sw.sw_hos );
		MakePackable( fxw->fxw_NameBuf );
		Mystrncpy( fxw->fxw_CurrentEffect->sfx_Name, fxw->fxw_NameBuf, SFXNAMESIZE-1 );
		fxw->fxw_Layout[ GAD_SFXLIST ].ho_Attr0 = (ULONG)&fxw->fxw_Chunk->ch_DataList;
		HO_RefreshObject( &fxw->fxw_Layout[ GAD_SFXLIST ], fxw->fxw_sw.sw_hos );
		HO_RefreshObject( &fxw->fxw_Layout[ GAD_NAME ], fxw->fxw_sw.sw_hos );
	}
}

/********************************************************/
//

static VOID fxw_ImportRaw( struct SFXWindow *fxw )
{
	struct SFXHeader *sfx = NULL;

	if( BlockAllSuperWindows() )
	{
		if( AslRequestTags( filereq,
			ASLFR_Window, fxw->fxw_sw.sw_Window,
			ASLFR_TitleText, "Import Raw Sample",
			ASLFR_DoMultiSelect, FALSE,
			ASLFR_DoSaveMode, FALSE,
			TAG_DONE ) )
		{
			sfx = LoadRawSample( filereq->fr_Drawer,filereq->fr_File );
		}
	}
	UnblockAllSuperWindows();

	if( sfx )
	{
		fxw->fxw_Layout[ GAD_SFXLIST ].ho_Attr0 = ~0;
		HO_RefreshObject( &fxw->fxw_Layout[ GAD_SFXLIST ], fxw->fxw_sw.sw_hos );
		AddTail( &fxw->fxw_Chunk->ch_DataList, (struct Node *)sfx );
		fxw->fxw_Layout[ GAD_SFXLIST ].ho_Attr0 = (ULONG)&fxw->fxw_Chunk->ch_DataList;
		HO_RefreshObject( &fxw->fxw_Layout[ GAD_SFXLIST ], fxw->fxw_sw.sw_hos );
		fxw_SetCurrentEffect( fxw, sfx );
	}
}


/********************************************************/
//

static VOID fxw_Import8SVX( struct SFXWindow *fxw )
{
	struct SFXHeader *sfx = NULL;

	if( BlockAllSuperWindows() )
	{
		if( AslRequestTags( filereq,
			ASLFR_Window, fxw->fxw_sw.sw_Window,
			ASLFR_TitleText, "Import IFF 8SVX",
			ASLFR_RejectIcons, TRUE,
			ASLFR_DoSaveMode, FALSE,
			TAG_DONE ) )
		{
			sfx = Load8SVXSample( filereq->fr_Drawer,filereq->fr_File );
		}
	}
	UnblockAllSuperWindows();

	if( sfx )
	{
		fxw->fxw_Layout[ GAD_SFXLIST ].ho_Attr0 = ~0;
		HO_RefreshObject( &fxw->fxw_Layout[ GAD_SFXLIST ], fxw->fxw_sw.sw_hos );
		AddTail( &fxw->fxw_Chunk->ch_DataList, (struct Node *)sfx );
		fxw->fxw_Layout[ GAD_SFXLIST ].ho_Attr0 = (ULONG)&fxw->fxw_Chunk->ch_DataList;
		HO_RefreshObject( &fxw->fxw_Layout[ GAD_SFXLIST ], fxw->fxw_sw.sw_hos );
		fxw_SetCurrentEffect( fxw, sfx );
	}
}


struct SFXHeader *LoadRawSample( STRPTR drawer, STRPTR file )
{
	struct SFXHeader *sfx=NULL;
	char namebuf[512];
	BPTR fh;
	struct FileInfoBlock fib;
	ULONG len;

	Mystrncpy( namebuf, drawer, 512-1 );
	if( AddPart( namebuf, file, 512) )
	{
		if( fh = Open( namebuf, MODE_OLDFILE ) )
		{
			if( ExamineFH (fh, &fib) != DOSFALSE )
			{
				len = fib.fib_Size;
				if( len > 131072 )
					len = 131072;
				if( sfx = AllocVec( sizeof( struct SFXHeader ) + len,	MEMF_CHIP ) )
				{
					Read( fh, sfx+1, len );
					Mystrncpy( sfx->sfx_Name, file, SFXNAMESIZE-1 );
					MakePackable( sfx->sfx_Name );
					sfx->sfx_Node.ln_Name = sfx->sfx_Name;
					sfx->sfx_Spam.spam_Length = len;
					sfx->sfx_Spam.spam_Pri = 0;
					sfx->sfx_Spam.spam_Volume = 63;
					sfx->sfx_Spam.spam_SampleRate = 20000;
					sfx->sfx_Spam.spam_Flags = 0;
					sfx->sfx_Spam.spam_Repeats = 0;
					sfx->sfx_Spam.spam_RepeatStart = 0;
					sfx->sfx_Spam.spam_RepeatLength = sfx->sfx_Spam.spam_Length;
				}
			}
			Close( fh );
		}
	}

	return( sfx );
}


struct SFXHeader *Load8SVXSample( STRPTR drawer, STRPTR file )
{
	struct SFXHeader *sfx=NULL;
	char namebuf[512];
	struct IFFHandle *iff;
	struct StoredProperty *prop;
	struct Voice8Header *vheader;
	LONG ifferror, len;

	Mystrncpy( namebuf, drawer, 512-1 );
	if( AddPart( namebuf, file, 512) )
	{
		if( iff = AllocIFF() )
		{
			if( iff->iff_Stream = Open( namebuf, MODE_OLDFILE ) )
			{
				InitIFFasDOS( iff );
				ifferror = OpenIFF( iff, IFFF_READ );
				if( !ifferror )
				{
					if( !(ifferror = StopChunk( iff, ID_8SVX, ID_BODY ) ) &&
						!(ifferror = PropChunk( iff, ID_8SVX, ID_VHDR ) ) )
					{
						if( !ParseIFF( iff, IFFPARSE_SCAN ) )
						{
							if( prop = FindProp( iff, ID_8SVX, ID_VHDR ) )
							{
								vheader = (struct Voice8Header *)prop->sp_Data;

								if( !vheader->sCompression && vheader->ctOctave==1 )
								{
									if( vheader->oneShotHiSamples )
										len = vheader->oneShotHiSamples;
									else
										len = vheader->repeatHiSamples;
									if( len > 131072 ) len = 131072;

									if( sfx = AllocVec( sizeof( struct SFXHeader ) + len,	MEMF_CHIP ) )
									{
										ReadChunkBytes( iff, sfx+1, len );
										Mystrncpy( sfx->sfx_Name, file, SFXNAMESIZE-1 );
										MakePackable( sfx->sfx_Name );
										sfx->sfx_Node.ln_Name = sfx->sfx_Name;
										sfx->sfx_Spam.spam_Length = len;
										sfx->sfx_Spam.spam_Pri = 0;
										sfx->sfx_Spam.spam_Volume = ( vheader->volume * 64 ) / 0x10000;
										sfx->sfx_Spam.spam_SampleRate = vheader->samplesPerSec;
										sfx->sfx_Spam.spam_Flags = 0;
										sfx->sfx_Spam.spam_Repeats = 0;
										sfx->sfx_Spam.spam_RepeatStart = 0;
										sfx->sfx_Spam.spam_RepeatLength = sfx->sfx_Spam.spam_Length;	
									}
								}
							}
						}
					}
					CloseIFF( iff );
				}
				Close( iff->iff_Stream );
			}
			FreeIFF( iff );
		}
	}

	return( sfx );
}


static VOID fxw_DeleteCurrentEffect( struct SFXWindow *fxw )
{
	if( fxw->fxw_CurrentEffect )
	{
		fxw->fxw_Layout[ GAD_SFXLIST ].ho_Attr0 = ~0;
		HO_RefreshObject( &fxw->fxw_Layout[ GAD_SFXLIST ], fxw->fxw_sw.sw_hos );

		Remove( (struct Node *)fxw->fxw_CurrentEffect );
		FreeVec( fxw->fxw_CurrentEffect );

		fxw->fxw_Layout[ GAD_SFXLIST ].ho_Attr0 = (ULONG)&fxw->fxw_Chunk->ch_DataList;
		fxw->fxw_Layout[ GAD_SFXLIST ].ho_Value = 0xFFFF;
		HO_RefreshObject( &fxw->fxw_Layout[ GAD_SFXLIST ], fxw->fxw_sw.sw_hos );
		fxw_SetCurrentEffect( fxw, NULL );
	}
}
