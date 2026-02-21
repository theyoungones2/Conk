
#define ZONKMAIN_C

#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <assert.h>

#include <dos/dos.h>
#include <dos/dostags.h>
#include <exec/exec.h>
#include <intuition/intuition.h>
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

#include "global.h"
#include "MenuID.h"

/********************************************************************/
// Global variables

struct IntuitionBase				*IntuitionBase					= NULL;
struct GfxBase							*GfxBase								= NULL;
struct DosLibrary						*DOSBase								= NULL;
struct Library							*GadToolsBase						= NULL;
struct RxsLib    			      *RexxSysBase						= NULL;
struct Library							*DiskfontBase						= NULL;
struct Library							*AslBase								= NULL;
struct Library							*IFFParseBase						= NULL;
struct Library							*LayersBase							= NULL;
struct ScreenModeRequester	*smreq									= NULL;
struct FileRequester				*filereq								= NULL;
struct FontRequester				*fontreq								= NULL;
struct ScreenConfig					globalscreenconfig;
struct ZonkConfig						zonkfig;

struct MsgPort					*mainmsgport = NULL;
//UBYTE mainscreenname[ MAXPUBSCREENNAME+1 ];
//UBYTE gfxscreenname[ MAXPUBSCREENNAME+1 ];
struct List							superwindows;

/* default namestring for anything (must be packable - see 'Wibble.s' ) */
UBYTE *defname = "-<UNNAMED>-";

/* fallback font */
struct TextFont					*topaztextfont = NULL;

/********************************************************************/
// Local variables

static struct Process					*taskbase = NULL;
/* oldtaskwindowptr starts at -2 so we know if it's been changed in startup */
static APTR										oldtaskwindowptr = (APTR)-2;

//struct TextFont *tempfont = NULL;

/********************************************************************/
// LOCAL PROTOS
//

static BOOL StartUp( void );
static void ShutDown( void );
static void MainIDCMPHandler( void );

void MakeUpDefaultScreenConfig( struct ScreenConfig *scfg );


/********************************************************************/
// main()
//



int main( void )
{
	struct ZonkConfig zcfg;
/*
	int i;

	printf("\nActions:\n");
	for( i=0; i<numofoperators; i++)
	{
		if(operators[i].od_Type == OPTYPE_ACTION )
		{
			if( operators[i].od_Name[0] != '*' )
				printf( "%s\n",operators[i].od_Name );
		}
	}

	printf("\nChannelRoutines:\n");
	for( i=0; i<numofoperators; i++)
	{
		if(operators[i].od_Type == OPTYPE_CHANNL )
		{
			if( operators[i].od_Name[0] != '*' )
				printf( "%s\n",operators[i].od_Name );
		}
	}
*/


	#ifndef NDEBUG
	VerifyOpParamSize();
	#endif

	if( StartUp() )
	{
		/* load in config file, if it exists... */
		if( !LoadZonkConfig( "PROGDIR:Zonk.cfg", &zcfg ) )
			MakeUpDefaultZonkConfig( &zcfg );
		InstallZonkConfig( &zcfg );

		OpenNewChunkWindow();
		MainIDCMPHandler();

		UninstallScreenConfig();
	}
	ShutDown();

	return 0;
}


/********************************************************************/
// StartUp()
// Open libraries etc...
// Returns success.

static BOOL StartUp( void )
{
	struct TextAttr topaztextattr = { ( STRPTR )"topaz.font", 8, 0x00, FPF_ROMFONT };
	struct IntuiText errIntuitionText[] = {
		{ 1,2,JAM2,20,5,NULL,"WorkBench 2.0+ Required" ,NULL },
		{ 0,1,JAM2,6,4,NULL,"Abort",NULL } };

	/* try and open v37+ intuition.library */
	if ( !( IntuitionBase =
		(struct IntuitionBase *)OpenLibrary( "intuition.library", 37L) ) )
	{
		/* try and open _any_ intuition.library to bring up a requester */
		if (IntuitionBase =
			(struct IntuitionBase *)OpenLibrary( "intuition.library", 0L ) )
		{
			/* bring up a pre-v37 requester */
			AutoRequest( NULL, &errIntuitionText[0], NULL, &errIntuitionText[1],
			NULL, NULL, 200, 50 );
		}
		return(FALSE);
	}

	DOSBase = (struct DosLibrary *)OpenLibrary("dos.library", 37L);
	if( !DOSBase )
		return(FALSE);

	if( !( GfxBase = (struct GfxBase *)MyOpenLibrary( "graphics.library",
		37L ) ) )
		return( FALSE );
	if( !( DiskfontBase = MyOpenLibrary( "diskfont.library", 0L ) ) )
		return( FALSE );
	if( !( LayersBase = MyOpenLibrary( "layers.library", 0L ) ) )
		return( FALSE );
	if( !( GadToolsBase = MyOpenLibrary( "gadtools.library", 37L ) ) )
		return( FALSE );
	if( !( AslBase = MyOpenLibrary( "asl.library", 38L ) ) )
		return( FALSE );
	if( !( IFFParseBase = MyOpenLibrary( "iffparse.library", 36L ) ) )
		return( FALSE );

	/* open up topaz as a fallback font. */
	if( !( topaztextfont = OpenDiskFont( &topaztextattr ) ) )
		return( FALSE );

	if( !( smreq = AllocAslRequestTags( ASL_ScreenModeRequest, TAG_END ) ) )
	{
		PlebReq( NULL, PROGNAME, "Can't allocate ASL ScreenMode requester", "OK" );
		return( FALSE );
	}

	if( !( filereq = AllocAslRequestTags( ASL_FileRequest, TAG_END ) ) )
	{
		PlebReq( NULL, PROGNAME, "Can't allocate ASL File requester", "OK" );
		return( FALSE );
	}

	if( !( fontreq = AllocAslRequestTags( ASL_FontRequest, TAG_END ) ) )
	{
		PlebReq( NULL, PROGNAME, "Can't allocate ASL Font requester", "OK" );
		return( FALSE );
	}


	/* Create a message port for all our IDCMP stuff */
	if ( !( mainmsgport = CreateMsgPort() ) )
	{
		return(FALSE);
	}

	/* Save out the old requester window pointer for our process */
	taskbase = (struct Process *)FindTask(NULL);
	oldtaskwindowptr = taskbase->pr_WindowPtr;

	NewList( &superwindows );

	/* set up the chunk and file tracking */
	InitChunkStorage();

	return( TRUE );
}


/********************************************************************/
// ShutDown()
// Shut down everything set up by StartUp().
//

static void ShutDown( void )
{
	/* free any leftover chunk/file data - should already be freed anyway */
	FreeAllChunks();

	if( oldtaskwindowptr != (APTR)-2 ) taskbase->pr_WindowPtr = oldtaskwindowptr;
	if( mainmsgport ) DeleteMsgPort( mainmsgport );

	if( topaztextfont ) CloseFont( topaztextfont );
	if( gfxscrfont ) CloseFont( gfxscrfont );

	if( smreq ) FreeAslRequest( smreq );
	if( filereq ) FreeAslRequest( filereq );
	if( fontreq ) FreeAslRequest( fontreq );

	if( IFFParseBase ) CloseLibrary( IFFParseBase );
	if( AslBase ) CloseLibrary( AslBase );
	if( GadToolsBase ) CloseLibrary( GadToolsBase );
	if( DiskfontBase ) CloseLibrary( DiskfontBase );
	if( LayersBase ) CloseLibrary( LayersBase );
	if( GfxBase ) CloseLibrary( (struct Library *)GfxBase );
	if( DOSBase ) CloseLibrary( (struct Library *)DOSBase );
	if( IntuitionBase ) CloseLibrary( (struct Library *)IntuitionBase );
}



/********************************************************************/
// MyOpenLibrary()
// Front for OpenLibrary with "retry|abort" requester.
// Returns library base or NULL.

struct Library *MyOpenLibrary( STRPTR libname, LONG version )
{
	struct Library *base;

	base = OpenLibrary( libname, version );
	if( !base )
	{
		while( !base && PlebReq( NULL, PROGNAME, "Can't open %s %ld+",
			"Retry|Abort", libname, version ) )
		{
			base = OpenLibrary( libname, version );
		}
	}
	return( base );
}



/************************   Close Window Safely ******************************/

void CloseWindowSafely(struct Window *win)
{
	/* we forbid here to keep out of race conditions with Intuition */
	Forbid();

	/* send back any messages for this window
	** that have not yet been processed
	*/
	StripIntuiMessages( win->UserPort, win );
	/* clear UserPort so Intuition will not free it */
	win->UserPort = NULL;

	/* tell Intuition to stop sending more messages */
	ModifyIDCMP( win, 0L );

	/* turn multitasking back on */
	Permit();

	/* and really close the window */
	CloseWindow( win );
}



void StripIntuiMessages(struct MsgPort *mp, struct Window *win )
/* remove and reply all IntuiMessages on a port that
 * have been sent to a particular window
 * (note that we don't rely on the ln_Succ pointer
 *  of a message after we have replied it)
 */
{
	struct IntuiMessage *msg;
	struct Node *succ;

	msg = (struct IntuiMessage *) mp->mp_MsgList.lh_Head;

	while( succ =  msg->ExecMessage.mn_Node.ln_Succ )
	{
		if( msg->IDCMPWindow ==  win )
		{
				/* Intuition is about to free this message.
				** Make sure that we have politely sent it back.
				*/
			Remove( (struct Node *)msg );
			ReplyMsg( (struct Message *)msg );
		}
		msg = (struct IntuiMessage *) succ;
	}
}



/********************************************************************/

static void MainIDCMPHandler( void )
{
	struct	IntuiMessage		*msg;
	ULONG		signalset, mainsignal;
	struct	SuperWindow			*sw;
	BOOL		done;

	mainsignal = 1L << mainmsgport->mp_SigBit;

	done = FALSE;
	while ( !done )
	{
		signalset = Wait( mainsignal );

		if (signalset & mainsignal)
		{
			while(msg = (struct IntuiMessage *)GetMsg(mainmsgport) )
			{
				sw = (struct SuperWindow *)msg->IDCMPWindow->UserData;
				(*sw->sw_Handler)(sw,(struct IntuiMessage *)msg);
			}
		}
		else
		{
			/* other signal - maybe arexx stuff here */
		}

		if( IsListEmpty(&superwindows) )
			done = TRUE;
	}
}



/********************************************************************/
//
// MakeUpDefaultScreenConfig()
//
// Install default settings into a given ScreenConfig struct.
//

void MakeUpDefaultScreenConfig( struct ScreenConfig *scfg )
{
	scfg->scfg_GFXScrType = 0;	/* 0=none, 1 = create new pubscreen */

	/* gfx screen parameters (only apply with type 1 above ) */
	scfg->scfg_Width = 640;
	scfg->scfg_Height = 256;
	scfg->scfg_Depth = 5;
	scfg->scfg_DisplayID = (PAL_MONITOR_ID | HIRES_KEY);
	scfg->scfg_AutoScroll = TRUE;

	/* fonts for the gfx screen */
	strcpy( scfg->scfg_GFXScrFontName, "topaz.font" );
	scfg->scfg_GFXScrFont.ta_Name = scfg->scfg_GFXScrFontName;
	scfg->scfg_GFXScrFont.ta_YSize = 8;
	scfg->scfg_GFXScrFont.ta_Style = 0;
	scfg->scfg_GFXScrFont.ta_Flags = 0;

	strcpy( scfg->scfg_GFXWinFontName, "topaz.font" );
	scfg->scfg_GFXWinFont.ta_Name = scfg->scfg_GFXWinFontName;
	scfg->scfg_GFXWinFont.ta_YSize = 8;
	scfg->scfg_GFXWinFont.ta_Style = 0;
	scfg->scfg_GFXWinFont.ta_Flags = 0;

	scfg->scfg_GFXPubScrName[ 0 ] = '\0';

	scfg->scfg_MainScrType = 0;				/* 0 = use default pubscreen */
																		/* 1 = use gfx screen */
	scfg->scfg_MainPubScrName[ 0 ] = '\0';

	/* if DefaultFont=true then use system font instead of mainwinfont */
	scfg->scfg_DefaultFont = FALSE;
	strcpy( scfg->scfg_MainWinFontName, "topaz.font" );
	scfg->scfg_MainWinFont.ta_Name = scfg->scfg_MainWinFontName;
	scfg->scfg_MainWinFont.ta_YSize = 8;
	scfg->scfg_MainWinFont.ta_Style = 0;
	scfg->scfg_MainWinFont.ta_Flags = 0;


	scfg->scfg_Palette.pl_Count = 4;
	scfg->scfg_Palette.pl_Colours[0].r = 170;
	scfg->scfg_Palette.pl_Colours[0].g = 170;
	scfg->scfg_Palette.pl_Colours[0].b = 170;
	scfg->scfg_Palette.pl_Colours[1].r = 0;
	scfg->scfg_Palette.pl_Colours[1].g = 0;
	scfg->scfg_Palette.pl_Colours[1].b = 0;
	scfg->scfg_Palette.pl_Colours[2].r = 255;
	scfg->scfg_Palette.pl_Colours[2].g = 255;
	scfg->scfg_Palette.pl_Colours[2].b = 255;
	scfg->scfg_Palette.pl_Colours[3].r = 102;
	scfg->scfg_Palette.pl_Colours[3].g = 136;
	scfg->scfg_Palette.pl_Colours[3].b = 187;

	/* not used (yet) */
	scfg->scfg_Flags = 0;
	scfg->scfg_Reserved[0] = 0;
	scfg->scfg_Reserved[1] = 0;
	scfg->scfg_Reserved[2] = 0;
	scfg->scfg_Reserved[3] = 0;
	scfg->scfg_Reserved[4] = 0;
}


/* comments here please... */

VOID MakeUpDefaultZonkConfig( struct ZonkConfig *zcfg )
{
	int i;

	MakeUpDefaultScreenConfig( &zcfg->scfg );

	/* setup other fields here */
	zcfg->SaveIcons = FALSE;
	zcfg->MakeBackups = TRUE;

	for( i=0; i<16; i++ )
	{
		zcfg->SuperWindowDefs[ i ].Left = 100;
		zcfg->SuperWindowDefs[ i ].Top = 100;
		zcfg->SuperWindowDefs[ i ].Width = 400;
		zcfg->SuperWindowDefs[ i ].Height = 120;
		zcfg->SuperWindowDefs[ i ].Reserved[0] = 0;
		zcfg->SuperWindowDefs[ i ].Reserved[1] = 0;
	}

	zcfg->OpReqW = 280;
	zcfg->OpReqH = 150;
}



BOOL InstallZonkConfig( struct ZonkConfig *zcfg )
{
	BOOL success;
	struct List *winlist;
	int i;

//	D(bug( "InstallZonkConfig()\n" ) );

	success = FALSE;
	if( winlist = ListAndHideWindows() )
	{

		if( InstallScreenConfig( &zcfg->scfg ) )
			success = TRUE;

		/* install other stuff... */
		CopyMem( &zcfg->MakeBackups, &zonkfig.MakeBackups,
			sizeof( struct ZonkConfig ) - sizeof( struct ScreenConfig ) );

		ShowHiddenWindows( winlist );
	}
	return( success );
}

