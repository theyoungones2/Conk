/*
    Construction kit Image editor ( TAB SIZE = 2 )
*/


#include <stdio.h>
//#include <stdarg.h>
#include <string.h>
#include <dos/dos.h>
#include <dos/dostags.h>
#include <exec/exec.h>
#include <intuition/intuition.h>
//#include <graphics/gfxbase.h>
//#include <rexx/rxslib.h>
//#include <libraries/gadtools.h>
//#include <libraries/diskfont.h>
//#include <libraries/asl.h>

#include <proto/exec.h>
#include <proto/dos.h>
#include <proto/intuition.h>
#include <proto/graphics.h>
#include <proto/diskfont.h>
#include <proto/asl.h>
//#include <clib/alib_protos.h>
#include <proto/gadtools.h>
#include <proto/rexxsyslib.h>
#include <proto/iffparse.h>
#include <proto/layers.h>

#include <rexx/rexxmain.h>
#include <rexx/RexxMain_rxcl.c>

#include <Global.h>
#include <ExternVars.h>

void ProjWindowHandler(struct SuperWindow *sw, struct IntuiMessage *imsg);

/**********************  GLOBAL VARIABLES  *********************************/


static void SetupDefaultConfig(struct Config *cg);
static void	HandleIDCMPStuff(void);
static BOOL	OpenStuff(void);
static void	CloseDown(void);

//static BYTE myprocsignalbit = -1;

#include <MyText.h>


/**********************   MAIN  ******************************************/


ULONG main()
{
	BOOL done = FALSE;

	if ( OpenStuff() )
	{
		//printf("Delaying...");
		//printf("Done\n");

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
				OpenNewProject(&glob.cg_WindowDefaults[SWTY_PROJECT],NULL);
				done = FALSE;
			}
		} while( (!IsListEmpty(&superwindows)) && (!done) );
	}
	else
		CloseDown();

	return(0);
}

/**********************  OpenStuff  ******************************************/

static BOOL OpenStuff(void)
{
	struct TagItem	subtasktags[] = {	NP_Entry,	0,
																		NP_Name,	0,
																	//NP_WindowPtr,	-1,
																	//NP_Priority,	1,
																		NP_StackSize,	16384,
																		TAG_END, TAG_END };
	//char		rexxcommand[256];
	//ULONG	sigset;
	struct IntuiText errIntuitionText[] = {
							{ 1,2,JAM2,20,5,NULL,Txt_Workbench2Required ,NULL },
							{ 0,1,JAM2,6,4,NULL,Txt_Abort,NULL } };

	topaztextattr.ta_Name = Txt_TopazFont;
	topaztextattr.ta_YSize = 8;
	topaztextattr.ta_Style = NULL;
	topaztextattr.ta_Flags = NULL;

	/* init some lists */
	NewList(&projects);
	NewList(&superwindows);
	InitAnimBrush(&animbrush);

	/* open up all the libraries we need */
	if (!(IntuitionBase = (struct IntuitionBase *)OpenLibrary(Txt_IntuitionLibrary,37L) ))
	{
		if (IntuitionBase = (struct IntuitionBase *)OpenLibrary(Txt_IntuitionLibrary,0L))
			AutoRequest( NULL,&errIntuitionText[0],NULL,&errIntuitionText[1],NULL,NULL,200,50 );
		return(FALSE);
	}
	if (!(GfxBase = (struct GfxBase *)OpenLibrary(Txt_GraphicsLibrary,37L) ))
	{
		GroovyReq(Txt_EditorError,Txt_CantOpenGraphicsLibrary,Txt_Abort);
		return(FALSE);
	}
	if (!(DOSBase = (struct DosLibrary *)OpenLibrary(Txt_DosLibrary,37L) ))
	{
		GroovyReq(Txt_EditorError,Txt_CantOpenDOSLibrary,Txt_Abort);
		return(FALSE);
	}
	if (!(GadToolsBase = OpenLibrary(Txt_GadToolsLibrary ,37L) ))
	{
		GroovyReq(Txt_EditorError,Txt_CantOpenGadToolsLibrary,Txt_Abort);
		return(FALSE);
	}
	if (!(RexxSysBase = OpenLibrary(Txt_RexxSysLibLibrary ,35L) ))
	{
		GroovyReq(Txt_EditorError,Txt_CantOpenRexxSysLibrary,Txt_Abort);
		return(FALSE);
	}
	if (!(DiskfontBase = OpenLibrary(Txt_DiskFontLibrary ,0L) ))
	{
		GroovyReq(Txt_EditorError,Txt_CantOpenDiskFontLibrary,Txt_Abort);
		return(FALSE);
	}
	if (!(AslBase = OpenLibrary(Txt_AslLibrary , 36L)) )
	{
		GroovyReq(Txt_EditorError,Txt_CantOpenASLLibrary,Txt_Abort);
		return(FALSE);
	}
	if (!(IFFParseBase = OpenLibrary(Txt_IFFParseLibrary , 36L)) )
	{
		GroovyReq(Txt_EditorError,Txt_CantOpenIFFParseLibrary,Txt_Abort);
		return(FALSE);
	}
	if (!(LayersBase = OpenLibrary("layers.library" , 36L)) )
	{
		GroovyReq(Txt_EditorError,"Can't Open layers.library",Txt_Abort);
		return(FALSE);
	}
/*
	if (!(UtilityBase = OpenLibrary("utility.library" , 0L)) )
	{
		GroovyReq(Txt_EditorError,"Can't Open utility.library",Txt_Abort);
		return(FALSE);
	}
 */
	if ( !(mainmsgport = (struct MsgPort *)CreateMsgPort()) )
	{
		GroovyReq(Txt_EditorError,Txt_NotEnoughMemory,Txt_Abort);
		return(FALSE);
	}

	globpalette = &kludgepalette;

	/* Get all the Stuff to do with shanghai'ing windows, _before_ we open our screen */
	GetDefaultPubScreen(OldDefaultPubScreenName);
	OldPubScreenModes = SetPubScreenModes(0);
	SetPubScreenModes(OldPubScreenModes);

	if(!(myrexxhost = SetupARexxHost(NULL,NULL)) )
	{
		GroovyReq(Txt_EditorError,Txt_CantOpenRexxPort,Txt_Abort);
		return(FALSE);
	}

	/* Save out the old requester window pointer for our process */
	taskbase = (struct Process *)FindTask(NULL);
	oldtaskwindowptr = taskbase->pr_WindowPtr;

	/* Do all the stuff needed to setup the subtask */

	/*  Clear the GOOD & BAD signals for the maintask cos the subtask
	 * will reply with a good or bad message saying whether it could
	 * set itself up or not. */
	//printf("main: SetSignal(0L,SIGF_GOOD | SIGF_BAD)\n");
	//SetSignal(0L,SIGF_GOOD | SIGF_BAD);
	//printf("main: CreateNewProcTags()\n");
	subtasktags[0].ti_Data = (ULONG)SubTaskStart;
	subtasktags[1].ti_Data = (ULONG)"Gonk Sub Task";
	if( !(subtaskproc = CreateNewProc(subtasktags)) )
	{
		printf("main: CreateNewProcTags() Failed\n");
		GroovyReq(Txt_EditorError,Txt_NotEnoughMemory,Txt_Abort);
		return(FALSE);
	}

	//Delay(50);

	//printf("main: SetTaskPri(taskbase) fred \n");
	SetTaskPri(&taskbase->pr_Task,1);

	//if( !(sigset = SetSignal(0L,SIGF_GOOD | SIGF_BAD)) )
	//	sigset = Wait(SIGF_GOOD | SIGF_BAD);
	//if(sigset == SIGF_BAD)
	//{
	//	GroovyReq(Txt_EditorError,Txt_NotEnoughMemory,Txt_Abort);
	//	return(FALSE);
	//}

	//printf("main: SetTaskPri(subtaskproc) \n");
	//SetTaskPri(&subtaskproc->pr_Task,1);

	while(!subtaskgoing)
	{
		Delay(50);
	}

	/* Set up a fallback font */
	if(!(topaztextfont = OpenDiskFont(&topaztextattr)) )
	{
		GroovyReq(Txt_EditorError,Txt_CantOpenFont,Txt_Abort);
		return(FALSE);
	}

	if( !SetupDefaultBrushes() )
	{
		GroovyReq("Gonk Error", "Can't Allocate Internal Brushes", "Abort");
		return(FALSE);
	}

	/* Allocate a few Asl requesters */

	filereq = AllocAslRequestTags(ASL_FileRequest,
		ASLFR_TitleText,			Txt_FileRequester,
		//ASLFR_InitialDrawer,	"ILBM:",
		ASLFR_InitialHeight,	256,
		//ASLFR_RejectIcons,		TRUE,
		ASLFR_DoPatterns,			TRUE,
		ASLFR_InitialPattern,	"~(#?.(info|bak))",
		TAG_END );

	screenmodereq = AllocAslRequestTags(ASL_ScreenModeRequest,
		ASLSM_TitleText,			Txt_ScreenModeRequester,
		ASLSM_DoWidth,				TRUE,
		ASLSM_DoHeight,				TRUE,
		ASLSM_DoDepth,				TRUE,
		ASLSM_DoOverscanType,	TRUE,
		ASLSM_DoAutoScroll,		TRUE,
		ASLSM_PropertyFlags,	0,
		ASLSM_PropertyMask,		DIPF_IS_DUALPF,
		TAG_END	);

	fontreq = AllocAslRequestTags(ASL_FontRequest,
		ASLFO_TitleText,			Txt_FontRequester,
		TAG_END	);

	/* make sure we got em all */
	if(!filereq || !screenmodereq || !fontreq)
	{
		GroovyReq(Txt_EditorError,Txt_CantOpenASLRequester,Txt_Abort);
		return( FALSE );
	}
	CreateDefaultPalette();

	SetupDefaultConfig(&glob);
	if(!LoadSettings(Txt_DefaultConfig ) )
	{
		if ( !(SetupScreen(&glob.cg_ScrConfig)) )
		{
			GroovyReq(Txt_EditorError,Txt_CantOpenScreen,Txt_Abort);
			return(FALSE);
		}
		OpenNewProject(&glob.cg_WindowDefaults[SWTY_PROJECT],NULL);
		OpenNewToolsWindow(&glob.cg_WindowDefaults[SWTY_TOOLS]);
		OpenNewPaletteWindow(&glob.cg_WindowDefaults[SWTY_PALETTE]);
	}

/*
	sprintf(rexxcommand,Txt_DefaultRexx,myrexxhost->portname);
	DoDOS(rexxcommand);
*/
	return(TRUE);
}





/**********************  Handle IDCMP Stuff *******************************/
//
// The Main IDCMP event handler. All IDCMP stuff comes in here, then it is
// passed on to the appropriate handler.
//


static VOID HandleIDCMPStuff()
{
	struct	IntuiMessage		*msg;
	ULONG		signalset,mainsignal,rexxsignal;
	struct	SuperWindow			*sw;

	mainsignal = 1L << mainmsgport->mp_SigBit;
	rexxsignal = 1L << myrexxhost->port->mp_SigBit;

	/* loop while there are still superwindows onscreen */
	while( !IsListEmpty(&superwindows) )
	{
		signalset = Wait( mainsignal | rexxsignal );

		if(signalset & mainsignal)
		{
			while(msg = (struct IntuiMessage *)GetMsg(mainmsgport) )
			{
				if(msg->Class == IDCMP_MOUSEMOVE)
					SafeActivateWindowUnderMouse();

				sw = (struct SuperWindow *)msg->IDCMPWindow->UserData;
				if( !FilterSubTaskMessages(msg) )
				{
					(*sw->sw_Handler)(sw,msg);
				}
			}
		}
		if(signalset & rexxsignal)
		{
			ARexxDispatch(myrexxhost);
		}
		if( (!IsListEmpty(&superwindows)) && (!CountSuperWindows()) )
		{
			if( !OpenNewRevealWindow(&glob.cg_WindowDefaults[SWTY_REVEAL],RWNS_WINDOWS) )
			{
				while( !CloseOurScreen(&glob.cg_ScrConfig) )
					GroovyReq("Gonk Error","Can't Close Screen","Retry");
				strcpy(glob.cg_ScrConfig.sg_BorrowedName, "Workbench");
				glob.cg_ScrConfig.sg_OwnScreen = FALSE;
				RemakeScreen();
				GroovyReq("Gonk Error","Not Enough Chip Memory","Continue");
			}
		}
	}
}



/**********************   CloseDown  ******************************************/

static void CloseDown(void)
{
	ULONG	sigset;

	/* Close stuff */

	FreeBrushBitMaps(&brushes[BR_DOT]);
	FreeBrushBitMaps(&brushes[BR_BRUSH1]);
	FreeBrushBitMaps(&brushes[BR_BRUSH2]);
	FreeBrushBitMaps(&brushes[BR_BRUSH3]);
	FreeBrushBitMaps(&brushes[BR_BRUSH4]);
	FreeBrushBitMaps(&brushes[BR_BRUSH5]);
	FreeBrushBitMaps(&brushes[BR_BRUSH6]);
	FreeBrushBitMaps(&brushes[BR_BRUSH7]);
	FreeBrushBitMaps(&brushes[BR_BRUSH8]);
	FreeBrushBitMaps(&brushes[BR_CUSTOM]);
	FreeAnimBrush(&animbrush);

	if (fontreq) FreeAslRequest(fontreq);
	if (screenmodereq) FreeAslRequest(screenmodereq);
	if (filereq) FreeAslRequest(filereq);
	if (topaztextfont) CloseFont(topaztextfont);
	if (oldtaskwindowptr != (APTR)-2)	taskbase->pr_WindowPtr = oldtaskwindowptr;

	if(subtaskport)
	{
		SetSignal(0L,SIGF_GOOD);
		Signal(&subtaskproc->pr_Task, SIGF_BAD);
/*	if( !(sigset = SetSignal(0L,SIGF_GOOD)) ) */
			sigset = Wait(SIGF_GOOD);
	}

	if (myrexxhost)		CloseDownARexxHost(myrexxhost);
	if (mainmsgport)	MyDeleteMsgPort(mainmsgport);

	/* Close librarys */

/*	if (UtilityBase)		CloseLibrary( UtilityBase ); */
	if (LayersBase)			CloseLibrary( LayersBase );
	if (IFFParseBase)		CloseLibrary( IFFParseBase );
	if (AslBase)				CloseLibrary( AslBase );
	if (DiskfontBase)		CloseLibrary( DiskfontBase );
	if (RexxSysBase)		CloseLibrary( RexxSysBase);
	if (GadToolsBase)		CloseLibrary( GadToolsBase);
	if (DOSBase)				CloseLibrary( (struct Library *)DOSBase);
	if (GfxBase)				CloseLibrary( (struct Library *)GfxBase);
	if (IntuitionBase)	CloseLibrary( (struct Library *)IntuitionBase);
}



/**********************   Ctrl/C Handler ****************************************/

VOID __regargs __chkabort(void)
{
}

/***************  SetupDefaultConfig()  ***************/
//
// Initialise a config struct with default values (lowres etc...)
//

static void SetupDefaultConfig(struct Config *cg)
{
	static struct WindowDefault crappywf = {0,11,200,180,SWTY_PROJECT,FALSE,FALSE,TRUE,TRUE,0};

	cg->cg_Dummy1 = FALSE;
	cg->cg_SaveChunky = FALSE;
	cg->cg_MakeBackups = TRUE;
	cg->cg_AutoActivate = TRUE;
	cg->cg_ExcludeBrush = TRUE;
	cg->cg_ShowWindowID = FALSE;
	cg->cg_CreateIcons = TRUE;
	cg->cg_KeepScreenMode = TRUE;

	SetupDefaultScreenConfig(&cg->cg_ScrConfig);

	Mystrncpy( cg->cg_ScrConfig.sg_SFNameBuf, Txt_TopazFont, MYMAXFONTNAMESIZE );
	cg->cg_ScrConfig.sg_SFYSize = 8;
	cg->cg_ScrConfig.sg_SFStyle = NULL;
	cg->cg_ScrConfig.sg_SFFlags = NULL;

	Mystrncpy( cg->cg_ScrConfig.sg_WFNameBuf, Txt_TopazFont, MYMAXFONTNAMESIZE );
	cg->cg_ScrConfig.sg_WFYSize = 8;
	cg->cg_ScrConfig.sg_WFStyle = NULL;
	cg->cg_ScrConfig.sg_WFFlags = NULL;

	cg->cg_WindowDefaults[SWTY_TOOLS] = crappywf;
	cg->cg_WindowDefaults[SWTY_TOOLS].wf_Left = cg->cg_ScrConfig.sg_DisplayWidth-50;
	cg->cg_WindowDefaults[SWTY_TOOLS].wf_Top = 11;
	cg->cg_WindowDefaults[SWTY_TOOLS].wf_Flags = WFF_TOOLSWINDOWVERTICAL;
	cg->cg_WindowDefaults[SWTY_PALETTE] = crappywf;
	cg->cg_WindowDefaults[SWTY_PALETTE].wf_Left = cg->cg_ScrConfig.sg_DisplayWidth-50;
	cg->cg_WindowDefaults[SWTY_PALETTE].wf_Top = cg->cg_WindowDefaults[SWTY_TOOLS].wf_Top+149;
	cg->cg_WindowDefaults[SWTY_PALETTE].wf_Width = 50;
	cg->cg_WindowDefaults[SWTY_PALETTE].wf_Height = cg->cg_ScrConfig.sg_DisplayHeight-cg->cg_WindowDefaults[SWTY_PALETTE].wf_Top;
	cg->cg_WindowDefaults[SWTY_PALETTE].wf_SWType = SWTY_PALETTE;
	cg->cg_WindowDefaults[SWTY_PROJECT] = crappywf;
	cg->cg_WindowDefaults[SWTY_PROJECT].wf_Width = cg->cg_ScrConfig.sg_DisplayWidth-50;
	cg->cg_WindowDefaults[SWTY_PROJECT].wf_Height = cg->cg_ScrConfig.sg_DisplayHeight-cg->cg_WindowDefaults[SWTY_PROJECT].wf_Top;
	cg->cg_WindowDefaults[SWTY_PROJECT].wf_SWType = SWTY_PROJECT;
	cg->cg_WindowDefaults[SWTY_PROJECT].wf_Flags = EDTY_FRAME;
	cg->cg_WindowDefaults[SWTY_ABOUT] = crappywf;
	cg->cg_WindowDefaults[SWTY_ABOUT].wf_SWType = SWTY_ABOUT;
	cg->cg_WindowDefaults[SWTY_REVEAL] = crappywf;
	cg->cg_WindowDefaults[SWTY_REVEAL].wf_SWType = SWTY_REVEAL;
	cg->cg_WindowDefaults[SWTY_SCREENCONFIG] = crappywf;
	cg->cg_WindowDefaults[SWTY_SCREENCONFIG].wf_SWType = SWTY_SCREENCONFIG;
	cg->cg_WindowDefaults[SWTY_SCREENCONFIG].wf_Width = 300;
	cg->cg_WindowDefaults[SWTY_USERCONFIG] = crappywf;
	cg->cg_WindowDefaults[SWTY_USERCONFIG].wf_SWType = SWTY_USERCONFIG;
	cg->cg_WindowDefaults[SWTY_EDITPALETTE] = crappywf;
	cg->cg_WindowDefaults[SWTY_EDITPALETTE].wf_SWType = SWTY_EDITPALETTE;
	cg->cg_WindowDefaults[SWTY_EDITPALETTE].wf_Width = 310;
	cg->cg_WindowDefaults[SWTY_EDITPALETTE].wf_Height = 95;
	cg->cg_WindowDefaults[SWTY_EDITPALETTE].wf_Top = cg->cg_ScrConfig.sg_DisplayHeight-cg->cg_WindowDefaults[SWTY_EDITPALETTE].wf_Height-1;
	cg->cg_WindowDefaults[SWTY_SEQUENCE] = crappywf;
	cg->cg_WindowDefaults[SWTY_SEQUENCE].wf_SWType = SWTY_SEQUENCE;
	cg->cg_WindowDefaults[SWTY_SEQUENCE].wf_Width = 320;
	cg->cg_WindowDefaults[SWTY_RANGE] = crappywf;
	cg->cg_WindowDefaults[SWTY_RANGE].wf_SWType = SWTY_RANGE;
	cg->cg_WindowDefaults[SWTY_RANGE].wf_Width = 260;
	cg->cg_WindowDefaults[SWTY_RANGE].wf_Height = 110;
}



BOOL InstallNewGlobalConfig(struct Config *cg)
{
	BOOL ret;

	glob = *cg;
	ret = UseNewScreenSettings(&cg->cg_ScrConfig);
	return(ret);
}
