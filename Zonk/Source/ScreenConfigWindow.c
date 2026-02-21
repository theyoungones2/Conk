/********************************************************/
//
// ScreenConfigWindow.c
//
/********************************************************/

#define SCRCONFIGWINDOW_C

#include <stdio.h>
#include <string.h>
#include <stdarg.h>

#include <dos/dos.h>
//#include <dos/dostags.h>
#include <exec/exec.h>
#include <intuition/intuition.h>
//#include <graphics/gfxbase.h>
//#include <rexx/rxslib.h>
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

//#include <rexx/rexxmain.h>
//#include <rexx/RexxMain_rxcl.c>

#include "hoopy.h"
#include "global.h"

struct ScreenConfigWindow
{
	struct SuperWindow	sc_sw;
//	struct HOStuff			*sc_hos;
	struct ScreenConfig	sc_LocalConfig;
	UBYTE								sc_ScreenModeInfoBuf[128];
	UBYTE								sc_PaletteInfoBuf[16];
};

static void IDCMPHandler( struct SuperWindow *sw,
	struct IntuiMessage *imsg );
static void HideWindow( struct SuperWindow *sw );
static BOOL ShowWindow( struct SuperWindow *sw );
static void KillWindow( struct SuperWindow *sw );
static BOOL RefreshMenus( struct SuperWindow *sw );

static void DisableStuff( struct ScreenConfigWindow *sc );
static void UpdatePaletteInfo( struct ScreenConfigWindow *sc );
static void UpdateScreenModeInfo( struct ScreenConfigWindow *sc );
static void PickScreenMode(struct ScreenConfigWindow *sc );
static void PickScrFont(struct ScreenConfigWindow *sc );
static void PickWinFont(struct ScreenConfigWindow *sc );
static void PickPalette( struct ScreenConfigWindow *sc );
static void ReadyConfig( struct ScreenConfigWindow *sc );
static void ImportPalette( struct ScreenConfigWindow *sc );


static UBYTE *scrtypelabels[] = {
		(UBYTE *)"Default Pubscreen",
		(UBYTE *)"New PubScreen",
		(UBYTE *)"Existing Pubscreen",
		NULL };

#define GID_BORING				0
#define GID_SCRTYPE				2

#define GID_PICKSCRNAME		4
#define GID_SCRNAME				5

#define GID_PICKSCRMODE		7
#define GID_SCRMODE				8

#define GID_IMPORTPALETTE	10
#define GID_PALETTEINFO		11
#define GID_PICKPALETTE		12

#define GID_PICKSCRFONT		14
#define GID_SCRFONT				15
#define GID_SCRFONTSIZE		16

#define GID_PICKWINFONT		18
#define GID_WINFONT				19
#define GID_WINFONTSIZE		20

#define GID_OK						23
#define	GID_CANCEL				24



/* The gadget layout for the ScreenConfigWindow */

static struct HoopyObject gadlayout[] =
{
	{ HOTYPE_VGROUP, 100, HOFLG_NOBORDER,3,0,0,0, NULL, 0,GID_BORING,0,0,0 },
		{ HOTYPE_VGROUP, 100, HOFLG_RAISEDBORDER,6,0,0,0, NULL, 0,GID_BORING,0,0,0 },
			{ HOTYPE_CYCLE, 100, 0, (LONG)&scrtypelabels,0,0,0, "Screen",0,GID_SCRTYPE,PLACETEXT_LEFT,0,0 },
			{ HOTYPE_HGROUP, 100, 0,2,0,0,0, NULL, 0,GID_BORING,0,0,0 },
				{ HOTYPE_BUTTON, 100, HOFLG_NOBORDER, 0,0,0,0, "PubScreen...",0,GID_PICKSCRNAME,PLACETEXT_IN,0,0 },
				{ HOTYPE_STRING, 100, HOFLG_NOBORDER|HOFLG_FREEWIDTH, NULL,MAXPUBSCREENNAME-1,0,0, NULL,0,GID_SCRNAME,0,0,0 },
			{ HOTYPE_HGROUP, 100, 0,2,0,0,0, NULL, 0,GID_BORING,0,0,0 },
				{ HOTYPE_BUTTON, 100, HOFLG_NOBORDER, 0,0,0,0, "ScreenMode...",0,GID_PICKSCRMODE,PLACETEXT_IN,0,0 },
				{ HOTYPE_TEXT, 100, HOFLG_NOBORDER|HOFLG_FREEWIDTH, NULL,0,0,0, NULL,0,GID_SCRMODE,0,0,0 },
			{ HOTYPE_HGROUP, 100, 0,3,0,0,0, NULL, 0,GID_BORING,0,0,0 },
				{ HOTYPE_BUTTON, 100, HOFLG_NOBORDER, 0,0,0,0, "Palette...",0,GID_IMPORTPALETTE,PLACETEXT_IN,0,0 },
				{ HOTYPE_TEXT, 100, HOFLG_NOBORDER|HOFLG_FREEWIDTH, NULL,0,0,0, NULL,0,GID_PALETTEINFO,0,0,0 },
				{ HOTYPE_BUTTON, 100, HOFLG_NOBORDER, 0,0,0,0, "?",0,GID_PICKPALETTE,PLACETEXT_IN,0,0 },
			{ HOTYPE_HGROUP, 100, 0,3,0,0,0, NULL, 0,GID_BORING,0,0,0 },
				{ HOTYPE_BUTTON, 100, HOFLG_NOBORDER, 0,0,0,0, "ScreenFont...",0,GID_PICKSCRFONT,PLACETEXT_IN,0,0 },
				{ HOTYPE_TEXT, 500, HOFLG_NOBORDER|HOFLG_FREEWIDTH, NULL,0,0,0, NULL,0,GID_SCRFONT,0,0,0 },
				{ HOTYPE_NUMBER, 100, HOFLG_NOBORDER|HOFLG_FREEWIDTH, NULL,0,0,0, NULL,0,GID_SCRFONTSIZE,0,0,0 },
			{ HOTYPE_HGROUP, 100, 0,3,0,0,0, NULL, 0,GID_BORING,0,0,0 },
				{ HOTYPE_BUTTON, 100, HOFLG_NOBORDER, 0,0,0,0, "WindowFont...",0,GID_PICKWINFONT,PLACETEXT_IN,0,0 },
				{ HOTYPE_TEXT, 500, HOFLG_NOBORDER|HOFLG_FREEWIDTH, NULL,0,0,0, NULL,0,GID_WINFONT,0,0,0 },
				{ HOTYPE_NUMBER, 100, HOFLG_NOBORDER|HOFLG_FREEWIDTH, NULL,0,0,0, NULL,0,GID_WINFONTSIZE,0,0,0 },
		{ HOTYPE_SPACE, 100, HOFLG_NOBORDER|HOFLG_FREEHEIGHT|HOFLG_FREEWIDTH, 0,0,0,0, NULL,0,GID_BORING,0,0,0 },
		{ HOTYPE_HGROUP, 100, 0,3,0,0,0, NULL, 0,GID_BORING,0,0,0 },
			{ HOTYPE_BUTTON, 100, HOFLG_NOBORDER|HOFLG_FREEWIDTH, 0,0,0,0, "OK",0,GID_OK,PLACETEXT_IN,0,0 },
			{ HOTYPE_SPACE, 200, HOFLG_NOBORDER|HOFLG_FREEWIDTH, 0,0,0,0, NULL,0,GID_BORING,0,0,0 },
			{ HOTYPE_BUTTON, 100, HOFLG_NOBORDER|HOFLG_FREEWIDTH, 0,0,0,0, "Cancel",0,GID_CANCEL,PLACETEXT_IN,0,0 },
	{HOTYPE_END}
};


BOOL OpenNewScreenConfigWindow( void )
{
	struct SuperWindow *sw;
	struct ScreenConfigWindow *sc;
	BOOL success = FALSE;

	/* make sure no other screenconfig windows open... */
	/* can only have one cos we don't copy out the gadlayout */
	if( sw = FindSuperWindowByType( (struct SuperWindow *)superwindows.lh_Head,
		SWTY_SCRCONFIG ) )
	{
		if( sw->sw_Window )
			WindowToFront( sw->sw_Window );
		else
			(*sw->sw_ShowWindow)( sw );
		return FALSE;
	}

	if( sc = AllocVec( sizeof( struct ScreenConfigWindow ), MEMF_ANY|MEMF_CLEAR ) )
	{
		sw = &sc->sc_sw;
		sw->sw_Handler = IDCMPHandler;
		sw->sw_HideWindow = HideWindow;
		sw->sw_ShowWindow = ShowWindow;
		sw->sw_BuggerOff = KillWindow;
		sw->sw_RefreshMenus = RefreshMenus;

		sw->sw_Type				= SWTY_SCRCONFIG;
		sw->sw_Window			= NULL;
//		sw->sw_MenuStrip	= NULL;
		sw->sw_Gadgets		= NULL;
		sw->sw_VisualInfo	= NULL;

		sw->sw_WinDim			= zonkfig.SuperWindowDefs[ SWTY_SCRCONFIG ];

		/* take a copy of the global screen config */
		sc->sc_LocalConfig = zonkfig.scfg;

		(*sw->sw_ShowWindow)( sw );

		AddTail( &superwindows, &sw->sw_Node );

		success = TRUE;

	}
	return( success );
}



static BOOL ShowWindow( struct SuperWindow *sw )
{
	BOOL success = FALSE;
	struct ScreenConfigWindow *sc;
	struct Window *win;
//	ULONG minw,minh;
	struct Screen *scr;
	struct ScreenConfig *scfg;

	assert( sw != NULL );

	sc = (struct ScreenConfigWindow *)sw;

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
		WA_Title,				"ScreenConfigWindow",

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
				IDCMP_CLOSEWINDOW|IDCMP_CHANGEWINDOW|IDCMP_MENUPICK|
				CYCLEIDCMP|BUTTONIDCMP|STRINGIDCMP|TEXTIDCMP|NUMBERIDCMP|CHECKBOXIDCMP );

			if( HO_SussWindowSizing( sw->sw_hos, gadlayout ) )
			{
				scfg = &sc->sc_LocalConfig;

				gadlayout[ GID_SCRTYPE ].ho_Value =	(LONG)scfg->scfg_MainScrType;

				gadlayout[ GID_SCRNAME ].ho_Attr0 =	(LONG)scfg->scfg_MainPubScrName;
				gadlayout[ GID_SCRMODE ].ho_Attr0 =	(LONG)sc->sc_ScreenModeInfoBuf;
				gadlayout[ GID_PALETTEINFO ].ho_Attr0 =	(LONG)sc->sc_PaletteInfoBuf;
				gadlayout[ GID_SCRFONT ].ho_Attr0 =	(LONG)scfg->scfg_GFXScrFontName;
				gadlayout[ GID_WINFONT ].ho_Attr0 =	(LONG)scfg->scfg_MainWinFontName;

				gadlayout[ GID_SCRFONTSIZE ].ho_Attr0 =	(LONG)scfg->scfg_GFXScrFont.ta_YSize;
				gadlayout[ GID_WINFONTSIZE ].ho_Attr0 =	(LONG)scfg->scfg_MainWinFont.ta_YSize;

				UpdatePaletteInfo( sc );
				UpdateScreenModeInfo( sc );

				HO_MakeLayout( gadlayout, sw->sw_hos, win->BorderLeft, win->BorderTop,
					win->Width - win->BorderLeft - win->BorderRight,
					win->Height - win->BorderTop - win->BorderBottom );

				DisableStuff( sc );

				success = TRUE;
			}
		}
	}

	if( !success )
	{
		if( win )
		{
			CloseWindow( win );
			sw->sw_Window = NULL;

			if( sw->sw_hos )
			{
				HO_FreeHOStuff( sw->sw_hos );
				sw->sw_hos = NULL;
			}
		}
	}

	return( success );
}



static void HideWindow( struct SuperWindow *sw )
{
	struct ScreenConfigWindow *sc;

	sc = (struct ScreenConfigWindow *)sw;

	assert( sw != NULL );
	if( sw->sw_Window )
	{
		RememberWindow( sw );
		GenericZapMenus( sw );
		HO_KillLayout( sw->sw_hos );
		CloseWindowSafely( sw->sw_Window );
		HO_FreeHOStuff( sw->sw_hos );
		sw->sw_Window = NULL;
	}
}


static void KillWindow( struct SuperWindow *sw )
{
	assert( sw != NULL );

	if( IsOKToClose( sw ) )
	{
		(*sw->sw_HideWindow)(sw);
		Remove( &sw->sw_Node );
		FreeVec( sw );
	}
}

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
	struct ScreenConfigWindow *sc;
	struct List *hiddenwindows;
	struct ScreenConfig *scfg;


	ULONG class;
	UWORD code;
//	UWORD qualifier;
	APTR iaddress;
//	WORD mousex, mousey;
//	ULONG seconds, micros;

	sc = (struct ScreenConfigWindow *)sw;
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
//		seconds = imsg->Seconds;
//		micros = imsg->Micros;

		GT_PostFilterIMsg( imsg );

		switch( class )
		{
			case IDCMP_CLOSEWINDOW:
				ReplyMsg( (struct Message *)origimsg );
				KillWindow( sw );
				break;
			case IDCMP_NEWSIZE:
				ClearWindow( win );
				HO_ReadGadgetStates( sw->sw_hos, gadlayout );

				HO_MakeLayout( gadlayout, sw->sw_hos, win->BorderLeft, win->BorderTop,
					win->Width - win->BorderLeft - win->BorderRight,
					win->Height - win->BorderTop - win->BorderBottom );

				//HO_DisableObject( &gadlayout[0], sw->sw_hos, FALSE );
				//DoDisableStuff( sc );

				ReplyMsg( (struct Message *)origimsg );
				break;
			case IDCMP_CHANGEWINDOW:
				UpdateWinDef( sw );
				ReplyMsg( (struct Message *)origimsg );
				break;
			case IDCMP_GADGETUP:
				ReplyMsg( (struct Message *)origimsg );
				HO_ReadGadgetStates( sw->sw_hos, gadlayout );
				scfg = &sc->sc_LocalConfig;
				switch( ((struct Gadget*)iaddress)->GadgetID )
				{
					case GID_SCRTYPE:
						scfg->scfg_MainScrType = code;
						DisableStuff( sc );
						break;
					case GID_PICKSCRMODE:
						PickScreenMode( sc );
						UpdateScreenModeInfo( sc );
						HO_RefreshObject( &gadlayout[ GID_SCRMODE ], sw->sw_hos );
						break;
					case GID_PICKSCRFONT:
						PickScrFont( sc );
						gadlayout[ GID_SCRFONT ].ho_Attr0 =
							(ULONG)scfg->scfg_GFXScrFontName;
						gadlayout[ GID_SCRFONTSIZE ].ho_Attr0 =
							(ULONG)scfg->scfg_GFXScrFont.ta_YSize;
						HO_RefreshObject( &gadlayout[ GID_SCRFONT ], sw->sw_hos );
						HO_RefreshObject( &gadlayout[ GID_SCRFONTSIZE ], sw->sw_hos );
						break;
					case GID_PICKWINFONT:
						PickWinFont( sc );
						gadlayout[ GID_WINFONT ].ho_Attr0 =
							(ULONG)scfg->scfg_MainWinFontName;
						gadlayout[ GID_WINFONTSIZE ].ho_Attr0 =
							(ULONG)scfg->scfg_MainWinFont.ta_YSize;
						HO_RefreshObject( &gadlayout[ GID_WINFONT ], sw->sw_hos );
						HO_RefreshObject( &gadlayout[ GID_WINFONTSIZE ], sw->sw_hos );
						break;
					case GID_PICKPALETTE:
						PickPalette( sc );
						UpdatePaletteInfo( sc );
						HO_RefreshObject( &gadlayout[ GID_PALETTEINFO ], sw->sw_hos );
						break;
					case GID_IMPORTPALETTE:
						ImportPalette( sc );
						UpdatePaletteInfo( sc );
						HO_RefreshObject( &gadlayout[ GID_PALETTEINFO ], sw->sw_hos );
						break;
					case GID_CANCEL:
						KillWindow( sw );
						break;
					case GID_OK:
//						HideWindow( sw );
						ReadyConfig( sc );
						if( hiddenwindows = ListAndHideWindows() )
						{
							InstallScreenConfig( &sc->sc_LocalConfig );
							ShowHiddenWindows( hiddenwindows );
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
			default:
				ReplyMsg( (struct Message *)origimsg );
				break;
		}
	}
	else
		ReplyMsg( (struct Message *)origimsg );
}




static void DisableStuff( struct ScreenConfigWindow *sc )
{
	struct ScreenConfig *scfg;
	struct HoopyObject *hob;
	struct HOStuff *hos;

	hob = gadlayout;
	hos = sc->sc_sw.sw_hos;
	scfg = &sc->sc_LocalConfig;

	switch( scfg->scfg_MainScrType )
	{
		case 0:	/* use default screen */
			HO_DisableObject( &hob[GID_PICKSCRNAME], hos, TRUE );
			HO_DisableObject( &hob[GID_SCRNAME], hos, TRUE );
			HO_DisableObject( &hob[GID_PICKSCRMODE], hos, TRUE );
			HO_DisableObject( &hob[GID_IMPORTPALETTE], hos, TRUE );
			HO_DisableObject( &hob[GID_PICKPALETTE], hos, TRUE );
			HO_DisableObject( &hob[GID_PICKSCRFONT], hos, TRUE );
			break;
		case 1:	/* new screen */
			HO_DisableObject( &hob[GID_PICKSCRNAME], hos, TRUE );
			HO_DisableObject( &hob[GID_SCRNAME], hos, TRUE );
			HO_DisableObject( &hob[GID_PICKSCRMODE], hos, FALSE );
			HO_DisableObject( &hob[GID_IMPORTPALETTE], hos, FALSE );
			HO_DisableObject( &hob[GID_PICKPALETTE], hos, FALSE );
			HO_DisableObject( &hob[GID_PICKSCRFONT], hos, FALSE );
			break;
		case 2:	/* use public screen */
			HO_DisableObject( &hob[GID_PICKSCRNAME], hos, FALSE );
			HO_DisableObject( &hob[GID_SCRNAME], hos, FALSE );
			HO_DisableObject( &hob[GID_PICKSCRMODE], hos, TRUE );
			HO_DisableObject( &hob[GID_IMPORTPALETTE], hos, TRUE );
			HO_DisableObject( &hob[GID_PICKPALETTE], hos, TRUE );
			HO_DisableObject( &hob[GID_PICKSCRFONT], hos, TRUE );
			break;
	}
}


static void UpdatePaletteInfo( struct ScreenConfigWindow *sc )
{
	if( sc->sc_LocalConfig.scfg_Palette.pl_Count )
	{
		sprintf( sc->sc_PaletteInfoBuf, "%d Colours",
			sc->sc_LocalConfig.scfg_Palette.pl_Count );
	}
	else
	{
		strcpy( sc->sc_PaletteInfoBuf, "No Palette" );
	}
}


static void UpdateScreenModeInfo( struct ScreenConfigWindow *sc )
{
	struct ScreenConfig *scfg;

	scfg = &sc->sc_LocalConfig;

	ComposeScreenModeString( sc->sc_ScreenModeInfoBuf,
		scfg->scfg_DisplayID, scfg->scfg_Depth );
}



static void PickScreenMode(struct ScreenConfigWindow *sc )
{
	struct ScreenConfig *scfg;

	scfg = &sc->sc_LocalConfig;

	if( BlockAllSuperWindows() )
	{
		if( AslRequestTags( smreq,
			ASLSM_Window, sc->sc_sw.sw_Window,
//			ASLSM_SleepWindow, TRUE,
//			ASLSM_TextAttr, &zonkfig.scfg.scfg_MainWinFont,
			ASLSM_DoWidth, TRUE,
			ASLSM_DoHeight, TRUE,
			ASLSM_DoDepth, TRUE,
			ASLSM_DoOverscanType, TRUE,
			ASLSM_DoAutoScroll, TRUE,
			ASLSM_InitialDisplayID, sc->sc_LocalConfig.scfg_DisplayID,
			ASLSM_InitialDisplayWidth, sc->sc_LocalConfig.scfg_Width,
			ASLSM_InitialDisplayHeight, sc->sc_LocalConfig.scfg_Height,
			ASLSM_InitialDisplayDepth, sc->sc_LocalConfig.scfg_Depth,
//		ASLSM_InitialOverscanType, sc->sc_LocalConfig.scfg_OverscanType,
//		ASLSM_InitialAutoScroll, sc->sc_LocalConfig.scfg_AutoScroll,
			ASLSM_PropertyFlags,0,
			ASLSM_PropertyMask, DIPF_IS_DUALPF,
			TAG_END ) )
		{
			sc->sc_LocalConfig.scfg_DisplayID = smreq->sm_DisplayID;
			sc->sc_LocalConfig.scfg_Width = smreq->sm_DisplayWidth;
			sc->sc_LocalConfig.scfg_Height = smreq->sm_DisplayHeight;
			sc->sc_LocalConfig.scfg_Depth = smreq->sm_DisplayDepth;
		}
	}
	UnblockAllSuperWindows();
}




static void PickScrFont(struct ScreenConfigWindow *sc )
{
	struct ScreenConfig *scfg;

	scfg = &sc->sc_LocalConfig;

	if( BlockAllSuperWindows() )
	{
		if( AslRequestTags( fontreq,
			ASLFO_Window, sc->sc_sw.sw_Window,
//			ASLFO_SleepWindow, TRUE,
			ASLFO_TextAttr, &zonkfig.scfg.scfg_MainWinFont,
			ASLFO_DoStyle, TRUE,
			ASLFO_InitialName, sc->sc_LocalConfig.scfg_GFXScrFontName,
			ASLFO_InitialSize, sc->sc_LocalConfig.scfg_GFXScrFont.ta_YSize,
			ASLFO_InitialStyle, sc->sc_LocalConfig.scfg_GFXScrFont.ta_Style,
			ASLFO_InitialFlags, sc->sc_LocalConfig.scfg_GFXScrFont.ta_Flags,
			TAG_END ) )
		{
			scfg->scfg_GFXScrFont = fontreq->fo_Attr;
			strcpy( sc->sc_LocalConfig.scfg_GFXScrFontName,
				fontreq->fo_Attr.ta_Name );
			scfg->scfg_GFXScrFont.ta_Name =
				scfg->scfg_GFXScrFontName;
		}
	}
	UnblockAllSuperWindows();
}



static void PickWinFont(struct ScreenConfigWindow *sc )
{
	struct ScreenConfig *scfg;

	scfg = &sc->sc_LocalConfig;

	if( BlockAllSuperWindows() )
	{
		if( AslRequestTags( fontreq,
			ASLFO_Window, sc->sc_sw.sw_Window,
//			ASLFO_SleepWindow, TRUE,
			ASLFO_TextAttr, &zonkfig.scfg.scfg_MainWinFont,
			ASLFO_DoStyle, TRUE,
			ASLFO_InitialName, sc->sc_LocalConfig.scfg_MainWinFontName,
			ASLFO_InitialSize, sc->sc_LocalConfig.scfg_MainWinFont.ta_YSize,
			ASLFO_InitialStyle, sc->sc_LocalConfig.scfg_MainWinFont.ta_Style,
			ASLFO_InitialFlags, sc->sc_LocalConfig.scfg_MainWinFont.ta_Flags,
			TAG_END ) )
		{
			scfg->scfg_MainWinFont = fontreq->fo_Attr;
			strcpy( sc->sc_LocalConfig.scfg_MainWinFontName,
				fontreq->fo_Attr.ta_Name );
			scfg->scfg_MainWinFont.ta_Name =
				scfg->scfg_MainWinFontName;
		}
	}
	UnblockAllSuperWindows();
}



static void PickPalette( struct ScreenConfigWindow *sc )
{
	struct Chunk *cnk;
	
	if( BlockAllSuperWindows() )
	{
		cnk = RequestChunk( sc->sc_sw.sw_Window, "Install Palette", "Pick Palette",
			ID_CMAP,NULL );
		if( cnk )
		{
			CopyMem( cnk->ch_Data, &sc->sc_LocalConfig.scfg_Palette,
				sizeof( struct Palette ) );
		}
	}
	UnblockAllSuperWindows();
}

/* get the local config ready for installation */

static void ReadyConfig( struct ScreenConfigWindow *sc )
{
	struct ScreenConfig *scfg;

	scfg = &sc->sc_LocalConfig;

	/* hack up gfx screen settings from mainscreen settings */

	scfg->scfg_GFXScrType = scfg->scfg_MainScrType;
	if( scfg->scfg_MainScrType == 2 )
	{
		/* use existing pubscreen */

		Mystrncpy( scfg->scfg_GFXPubScrName, scfg->scfg_MainPubScrName,
			MAXPUBSCREENNAME-1 );

	}

	scfg->scfg_DefaultFont = FALSE;

	scfg->scfg_GFXWinFont = scfg->scfg_MainWinFont;
	strcpy( scfg->scfg_GFXWinFontName, scfg->scfg_MainWinFontName );
	scfg->scfg_GFXWinFont.ta_Name = scfg->scfg_GFXWinFontName;
}



/********************************************************/
static void ImportPalette( struct ScreenConfigWindow *sc )
{
	char namebuf[512];

	if( BlockAllSuperWindows() )
	{
		if( AslRequestTags( filereq,
			ASLFR_Window, sc->sc_sw.sw_Window,
			ASLFR_TitleText, "Import Palette",
			ASLFR_DoPatterns, TRUE,
			ASLFR_InitialPattern, "~(#?.info|#?.bak)",
			ASLFR_DoMultiSelect, FALSE,
			ASLFR_DoSaveMode, FALSE,
			TAG_DONE ) )
		{
			Mystrncpy( namebuf, filereq->fr_Drawer, 512-1 );
			if( AddPart( namebuf, filereq->fr_File, 512 ) )
			{
				LoadPalette( namebuf, &sc->sc_LocalConfig.scfg_Palette );
			}
		}
		UnblockAllSuperWindows();
	}
}
