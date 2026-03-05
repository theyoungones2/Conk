/****************************************************************************/
//
// BONK: Main.c
//
/****************************************************************************/


#include <stdio.h>
#include <string.h>
#include <dos/dos.h>
#include <dos/dostags.h>
#include <exec/exec.h>
#include <intuition/intuition.h>
#include <graphics/gfxbase.h>
#include <libraries/gadtools.h>
#include <libraries/diskfont.h>
#include <libraries/asl.h>
//#include "dh1:programming/stuff/include/libraries/asl.h"
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

#include <global.h>
#include <ExternVars.h>

static const char *versionstring = "$VER: Bonk 0.01 (31.10.94)";

/**********************  GLOBAL VARIABLES  *********************************/

// struct IntuitionBase		*IntuitionBase					= NULL;
// struct GfxBase					*GfxBase								= NULL;
// struct DosLibrary				*DOSBase								= NULL;
// struct Library					*GadToolsBase						= NULL;
// struct Library					*DiskfontBase						= NULL;
// struct Library					*AslBase								= NULL;
// struct Library					*IFFParseBase						= NULL;

struct Process					*taskbase								= NULL;
struct MsgPort					*mainmsgport						= NULL;


struct Screen						*mainscreen							= NULL;
BOOL										ownscreen;
struct TextAttr					screentextattr;
struct TextFont					*screentextfont					= NULL;
struct TextAttr					windowtextattr;
struct TextFont					*windowtextfont					= NULL;
struct TextAttr					topaztextattr;
struct TextFont					*topaztextfont					= NULL;


struct ScreenModeRequester *screenmodereq				= NULL;
struct FontRequester 		*fontreq								= NULL;
struct FileRequester		*filereq								= NULL;

struct List							superwindows;
struct List							projects;

APTR		oldtaskwindowptr		= (APTR)-2;

char								OldDefaultPubScreenName[MAXPUBSCREENNAME];
UWORD								OldPubScreenModes						= NULL;

char								currentrexxwindow[6];

struct Config					glob;
struct BlockSet				*globlocks = NULL;
UBYTE									globlockpreviewcolours[256];
struct Palette				*globpalette = NULL;
struct UndoBuffer			undobuf = { NULL, 0, 0, NULL };
struct ProjectWindow	*prevprojwin = NULL;
UWORD lmbblk=1, mmbblk=2, rmbblk=0;
struct Brush					*currentbrush = NULL;
struct List						brushes;
struct BitMap					*scratchbm = NULL;
static void SetupDefaultConfig(struct Config *cg);


/**********************   MAIN  ******************************************/


void main()
{
	BOOL done = FALSE;

	if ( OpenStuff() )
	{
		OpenNewToolsWindow();

		do
		{
			HandleIDCMPStuff();

			if (CloseOurScreen(&glob.cg_ScrConfig))
			{
				CloseDown();
				done = TRUE;
			}
			else
			{
				OpenNewToolsWindow();
//				GUIOpenNewProject();
				done = FALSE;
			}
		} while( (!IsListEmpty(&superwindows)) && (!done) );
	}
	else
		CloseDown();
}

/**********************  OpenStuff  ******************************************/

BOOL OpenStuff(void)
{
	// struct IntuiText errIntuitionText[] = {
	// 						{ 1,2,JAM2,20,5,NULL,"WorkBench 2.0 Required" ,NULL },
	// 						{ 0,1,JAM2,6,4,NULL,"Abort",NULL } };

	topaztextattr.ta_Name = "topaz.font";
	topaztextattr.ta_YSize = 8;
	topaztextattr.ta_Style = NULL;
	topaztextattr.ta_Flags = NULL;

	/* open up all the libraries we need */
	/*
	if (!(IntuitionBase = (struct IntuitionBase *)OpenLibrary("intuition.library",37L) ))
	{
		if (IntuitionBase = (struct IntuitionBase *)OpenLibrary("intuition.library",0L))
			AutoRequest( NULL,&errIntuitionText[0],NULL,&errIntuitionText[1],NULL,NULL,200,50 );
		return(FALSE);
	}
	if (!(GfxBase = (struct GfxBase *)OpenLibrary("graphics.library",37L) ))
		return(FALSE);

	if (!(DOSBase = (struct DosLibrary *)OpenLibrary("dos.library",37L) ))
		return(FALSE);

	if (!(GadToolsBase = OpenLibrary("gadtools.library" ,37L) ))
	{
		GroovyReq( PROGNAME,"Can't Open gadtools.library","Abort");
		return(FALSE);
	}
	if (!(DiskfontBase = OpenLibrary("diskfont.library" ,0L) ))
	{
		GroovyReq( PROGNAME,"Can't Open diskfont.library","Abort");
		return(FALSE);
	}
	if (!(AslBase = OpenLibrary("asl.library" , 36L)) )
	{
		GroovyReq( PROGNAME, "Can't Open asl.library","Abort");
		return(FALSE);
	}
	if (!(IFFParseBase = OpenLibrary("iffparse.library" , 36L)) )
	{
		GroovyReq( PROGNAME, "Can't Open iffparse.library","Abort");
		return(FALSE);
	}
*/
	if ( !(mainmsgport = (struct MsgPort *)CreateMsgPort()) )
	{
		GroovyReq( PROGNAME,"Not Enough Memory","Abort");
		return(FALSE);
	}

	/* Get all the Stuff to do with shanghai'ing windows, _before_ we open our screen */
	GetDefaultPubScreen(OldDefaultPubScreenName);
	OldPubScreenModes = SetPubScreenModes(0);
	SetPubScreenModes(OldPubScreenModes);

	/* Save out the old requester window pointer for our process */
	taskbase = (struct Process *)FindTask(NULL);
	oldtaskwindowptr = taskbase->pr_WindowPtr;

	/* init some lists */
	NewList(&projects);
	NewList(&superwindows);
	NewList(&brushes);

	/* Set up a fallback font */
	if(!(topaztextfont = OpenDiskFont(&topaztextattr)) )
	{
		GroovyReq("Editor Error","Can't Open topaz.font","Abort");
		return(FALSE);
	}

	filereq = AllocAslRequestTags(ASL_FileRequest,
		ASLFR_TitleText,			"File Requester",
		//ASLFR_InitialDrawer,	"ILBM:",
		ASLFR_RejectIcons,		TRUE,
		ASLFR_DoPatterns,			TRUE,
		TAG_END );

	screenmodereq = AllocAslRequestTags(ASL_ScreenModeRequest,
		ASLSM_TitleText,			"Screen Mode Requester",
		ASLSM_DoWidth,				TRUE,
		ASLSM_DoHeight,				TRUE,
		ASLSM_DoDepth,				TRUE,
		ASLSM_DoOverscanType,	TRUE,
		ASLSM_DoAutoScroll,		TRUE,
		ASLSM_PropertyFlags,	0,
		ASLSM_PropertyMask,		DIPF_IS_DUALPF,
		TAG_END	);

	fontreq = AllocAslRequestTags(ASL_FontRequest,
		ASLFO_TitleText,			"Font Requester",
		TAG_END	);

	/* make sure we got em all */
	if(!filereq || !screenmodereq || !fontreq)
	{
		GroovyReq( PROGNAME,"Can't alloc ASL Requesters","Abort");
		return( FALSE );
	}

	globpalette = CreateDefaultPalette();

	if(!LoadSettings("PROGDIR:Bonk.cfg") )
	{
		SetupDefaultConfig(&glob);
		if ( !(SetupScreen(&glob.cg_ScrConfig)) )
		{
			GroovyReq( PROGNAME, "Can't Open Screen","Abort");
			return(FALSE);
		}
	}

	globlocks = CreateDummyBlockSet();
/*
	sprintf(rexxcommand,"rx StartUp.rexx %s\n",myrexxhost->portname);
	DoDOS(rexxcommand);
*/
	return(TRUE);
}


/**********************  Handle IDCMP Stuff *******************************/
//
// The Main IDCMP event handler. All IDCMP stuff comes in here, then is
// passed on to the appropriate handler.
//


VOID HandleIDCMPStuff()
{
struct	IntuiMessage		*msg;
ULONG		signalset,mainsignal;
struct	SuperWindow			*sw;

	mainsignal = 1L << mainmsgport->mp_SigBit;

	/* loop while there are still superwindows onscreen */

	while ( !IsListEmpty(&superwindows) )
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

		if( (!IsListEmpty(&superwindows)) && (!CountSuperWindows()) )
			OpenNewRevealWindow( RWNS_WINDOWS );
	}
}

/**********************   CloseDown  ******************************************/

void CloseDown(void)
{
	/* Close stuff */

	FreeUndoBuffer( &undobuf );
	if(globpalette) FreePalette(globpalette);
	if(globlocks) FreeBlockSet(globlocks);
	FreeAllBrushes();

	glob.cg_ScrConfig.sg_Shanghai = FALSE;
	SetShanghaiState(MAINSCREEN,glob.cg_ScrConfig.sg_Shanghai);

	if (oldtaskwindowptr != (APTR)-2)	taskbase->pr_WindowPtr = oldtaskwindowptr;
	if (mainmsgport)	DeleteMsgPort(mainmsgport);
	if (topaztextfont) CloseFont(topaztextfont);
	if (screenmodereq) FreeAslRequest(screenmodereq);
	if (fontreq) FreeAslRequest(fontreq);
	if (filereq) FreeAslRequest(filereq);

	/* Close librarys */
/*
	if (IFFParseBase)		CloseLibrary( IFFParseBase );
	if (AslBase)				CloseLibrary( AslBase );
	if (DiskfontBase)		CloseLibrary( DiskfontBase );
	if (GadToolsBase)		CloseLibrary( (struct Library *)GadToolsBase);
	if (DOSBase)				CloseLibrary( (struct Library *)DOSBase);
	if (GfxBase)				CloseLibrary( (struct Library *)GfxBase);
	if (IntuitionBase)	CloseLibrary( (struct Library *)IntuitionBase);
*/
}



/**********************   Ctrl/C Handler ****************************************/

VOID __regargs __chkabort(void)
{
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


/*****************  SetProcessWindowPtr  ************/
//
// Set the process window pointer to the first superwindow
// in the list.

void	SetProcessWindowPtr()
{
struct SuperWindow *sw;

	if( sw = GetOpenSuperWindow() )
		taskbase->pr_WindowPtr = sw->sw_Window;
	else
		taskbase->pr_WindowPtr = NULL;
}


/***************  SetupDefaultConfig()  ***************/
//
// Initialise a config struct with default values (lores etc...)
//

static void SetupDefaultConfig(struct Config *cg)
{
//	static struct TextAttr crappyta = { NULL, 7, NULL,NULL };
	static struct WindowDefault crappywf = {0,0,200,180,FALSE,FALSE,TRUE,TRUE};
	UWORD i;

	cg->cg_CreateIcons = FALSE;	//not used
	cg->cg_MakeBackups = TRUE;

	cg->cg_ScrConfig.sg_BorrowedName[0] = '\0';
	cg->cg_ScrConfig.sg_DisplayID = 0;
	cg->cg_ScrConfig.sg_DisplayWidth = 320;
	cg->cg_ScrConfig.sg_DisplayHeight = 256;
	cg->cg_ScrConfig.sg_DisplayDepth = 5;
	cg->cg_ScrConfig.sg_OverscanType = 0;
	cg->cg_ScrConfig.sg_OwnScreen = TRUE;
	cg->cg_ScrConfig.sg_AutoScroll = TRUE;
	cg->cg_ScrConfig.sg_Shanghai = FALSE;

	Mystrncpy( cg->cg_ScrConfig.sg_SFNameBuf, "topaz.font", MYMAXFONTNAMESIZE-1 );
	cg->cg_ScrConfig.sg_SFYSize = 8;
	cg->cg_ScrConfig.sg_SFStyle = NULL;
	cg->cg_ScrConfig.sg_SFFlags = NULL;

	Mystrncpy( cg->cg_ScrConfig.sg_WFNameBuf, "topaz.font", MYMAXFONTNAMESIZE-1 );
	cg->cg_ScrConfig.sg_WFYSize = 8;
	cg->cg_ScrConfig.sg_WFStyle = NULL;
	cg->cg_ScrConfig.sg_WFFlags = NULL;

	for( i = 0; i < SWTY_NUM; i++ )
		cg->cg_WinDefs[i] = crappywf;
/*
	cg->cg_WinDefs[SWTY_PROJECT] = crappywf;
	cg->cg_WinDefs[SWTY_ABOUT] = crappywf;
	cg->cg_WinDefs[SWTY_REVEAL] = crappywf;
	cg->cg_WinDefs[SWTY_SCREENCONFIG] = crappywf;
	cg->cg_WinDefs[SWTY_TOOLS] = crappywf;
*/
}

void InstallNewGlobalConfig(struct Config *cg)
{
	glob = *cg;
	UseNewScreenSettings(&cg->cg_ScrConfig);
}
