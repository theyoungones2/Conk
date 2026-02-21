/********************************************************/
//
// ScreenConfigWindow.c
//
/********************************************************/

#define SCRCONFIGWINDOW_C

#include <stdio.h>
#include <string.h>
#include <stdarg.h>

//#include <dos/dos.h>
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
//#include <proto/dos.h>
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
	UBYTE								sc_ScreenModeBuf[128];
};



static void IDCMPHandler( struct SuperWindow *sw,
	struct IntuiMessage *imsg );
static void HideWindow( struct SuperWindow *sw );
static BOOL ShowWindow( struct SuperWindow *sw );
static void KillWindow( struct SuperWindow *sw );
static BOOL RefreshMenus( struct SuperWindow *sw );
static void DoDisableStuff( struct ScreenConfigWindow *sc );
static void InitGadgets( struct ScreenConfigWindow *sc );
static void UpdateLocalConfig( struct ScreenConfigWindow *sc );
static VOID sc_PickPalette( struct ScreenConfigWindow *sc );

static UBYTE *gfxscrlabels[] = {
		(UBYTE *)"None",
		(UBYTE *)"New PubScreen",
		(UBYTE *)"Existing PubScreen",
		NULL };


static UBYTE *mainscrlabels[] = {
		(UBYTE *)"Use default PubScreen",
		(UBYTE *)"Use gfx screen",
		(UBYTE *)"Use PubScreen",
		NULL };

UBYTE gfxscreenname[ MAXPUBSCREENNAME+1 ];
UBYTE mainscreenname[ MAXPUBSCREENNAME+1 ];

#define ID_ROOTGROUP				0

#define	ID_HLINE						1
#define ID_GFXSCRTYPEGROUP	2
#define ID_GFXSCRTYPE				3
#define ID_SPACE1						4

#define ID_GFXSCRMODEGROUP			5
#define ID_GFXSCRMODEBUTTON		6
#define ID_GFXSCRMODETEXT			7

#define ID_GFXSCRFONTGROUP	8
#define ID_GFXSCRFONTBUTTON	9
#define ID_GFXSCRFONTNAME		10
#define ID_GFXSCRFONTSIZE		11

#define ID_GFXWINFONTGROUP	12
#define ID_GFXWINFONTBUTTON	13
#define ID_GFXWINFONTNAME		14
#define ID_GFXWINFONTSIZE		15

#define ID_PALETTEGROUP			16
#define ID_PICKPALETTE			17
#define ID_PALETTEINFO			18


#define ID_GFXPUBSCRNAMEGROUP	19
#define ID_GFXPUBSCRNAME			20
#define ID_GFXPUBSCRNAMESTR		21

#define ID_HLINE1							22

#define ID_HLINE2							23
#define ID_MAINSCRTYPEGROUP		24
#define ID_MAINSCRTYPE				25
#define ID_SPACE2							26

#define ID_MAINPUBSCRNAMEGROUP	27
#define ID_MAINPUBSCRNAMEBUTTON	28
#define ID_MAINPUBSCRNAMESTR		29

#define ID_MAINWINFONTGROUP			30
#define ID_MAINWINFONTBUTTON		31
#define ID_MAINWINFONTNAME			32
#define ID_MAINWINFONTSIZE			33

#define ID_DEFAULTFONT					34

#define ID_HLINE3								35
#define ID_SPACE3								36

#define ID_OKCANCELGROUP				37
#define ID_OK										38
#define ID_SPACE4								39
#define ID_CANCEL								40


/* The gadget layout for the ScreenConfigWindow */
/* Must run InitGadgets() over it before using. */

static struct HoopyObject gadlayout[] =
{
	{ HOTYPE_VGROUP, 100, HOFLG_NOBORDER|HOFLG_FREEWIDTH|HOFLG_FREEHEIGHT,16,0,0,0, NULL, 0,ID_ROOTGROUP,0,0,0 },

		{ HOTYPE_HLINE, 150, HOFLG_FREEWIDTH, 0,0,0,0, "Gfx Screen",0,ID_HLINE,0,0,0 },
		{ HOTYPE_HGROUP, 100, HOFLG_NOBORDER|HOFLG_FREEWIDTH,2,0,0,0, NULL, 0,ID_GFXSCRTYPEGROUP,0,0,0 },
			{ HOTYPE_CYCLE, 100, 0, (LONG)gfxscrlabels, 0,0,0, NULL,0,ID_GFXSCRTYPE,0,0,0 },
			{ HOTYPE_SPACE, 100, HOFLG_FREEWIDTH, 0, 0,0,0, NULL,0,ID_SPACE1,0,0,0 },

		{ HOTYPE_HGROUP, 100, HOFLG_NOBORDER|HOFLG_FREEWIDTH,2,0,0,0, NULL, 0,ID_GFXSCRMODEGROUP,0,0,0 },
			{ HOTYPE_BUTTON, 100, HOFLG_FREEWIDTH, 0,0,0,0, "ScreenMode...",0,ID_GFXSCRMODEBUTTON,PLACETEXT_IN,0,0 },
			{ HOTYPE_TEXT, 200, HOFLG_FREEWIDTH, 0,0,0,0, NULL,0,ID_GFXSCRMODETEXT,0,0,0 },

		{ HOTYPE_HGROUP, 100, HOFLG_NOBORDER|HOFLG_FREEWIDTH,3,0,0,0, NULL, 0,ID_GFXSCRFONTGROUP,0,0,0 },
			{ HOTYPE_BUTTON, 100, HOFLG_FREEWIDTH, 0,0,0,0, "ScreenFont...",0,ID_GFXSCRFONTBUTTON,PLACETEXT_IN,0,0 },
			{ HOTYPE_TEXT, 150, HOFLG_FREEWIDTH, 0,0,0,0, NULL,0,ID_GFXSCRFONTNAME,0,0,0 },
			{ HOTYPE_NUMBER, 50, HOFLG_FREEWIDTH, 0,0,0,0, NULL,0,ID_GFXSCRFONTSIZE,0,0,0 },

		{ HOTYPE_HGROUP, 100, HOFLG_NOBORDER|HOFLG_FREEWIDTH,3,0,0,0, NULL, 0,ID_GFXWINFONTGROUP,0,0,0 },
			{ HOTYPE_BUTTON, 100, HOFLG_FREEWIDTH, 0,0,0,0, "WindowFont...",0,ID_GFXWINFONTBUTTON,PLACETEXT_IN,0,0 },
			{ HOTYPE_TEXT, 150, HOFLG_FREEWIDTH, 0,0,0,0, NULL,0,ID_GFXWINFONTNAME,0,0,0 },
			{ HOTYPE_NUMBER, 50, HOFLG_FREEWIDTH, 0,0,0,0, NULL,0,ID_GFXWINFONTSIZE,0,0,0 },

		{ HOTYPE_HGROUP, 100, HOFLG_NOBORDER|HOFLG_FREEWIDTH,2,0,0,0, NULL, 0,ID_PALETTEGROUP,0,0,0 },
			{ HOTYPE_BUTTON, 100, HOFLG_FREEWIDTH, 0,0,0,0, "Palette..",0,ID_PICKPALETTE,PLACETEXT_IN,0,0 },
			{ HOTYPE_NUMBER, 50, HOFLG_FREEWIDTH, 0,0,0,0, NULL,0,ID_PALETTEINFO,0,0,0 },

		{ HOTYPE_HGROUP, 100, HOFLG_NOBORDER|HOFLG_FREEWIDTH,2,0,0,0, NULL, 0,ID_GFXPUBSCRNAMEGROUP,0,0,0 },
			{ HOTYPE_BUTTON, 100, HOFLG_FREEWIDTH, 0,0,0,0, "PubScreen...",0,ID_GFXPUBSCRNAME,PLACETEXT_IN,0,0 },
			{ HOTYPE_STRING, 200, HOFLG_FREEWIDTH, NULL, MAXPUBSCREENNAME,0,0, NULL,0,ID_GFXPUBSCRNAMESTR,0,0,0 },

		{ HOTYPE_HLINE, 100, HOFLG_NOBORDER|HOFLG_FREEWIDTH,0,0,0,0, NULL, 0,ID_HLINE1,0,0,0 },

		{ HOTYPE_HLINE, 150, HOFLG_NOBORDER|HOFLG_FREEWIDTH,0,0,0,0, "Main Screen",0,ID_HLINE2,0,0,0 },
		{ HOTYPE_HGROUP, 100, HOFLG_NOBORDER|HOFLG_FREEWIDTH,2,0,0,0, NULL, 0,ID_MAINSCRTYPEGROUP,0,0,0 },
			{ HOTYPE_CYCLE, 100, 0, (LONG)mainscrlabels, 0,0,0,NULL,0,ID_MAINSCRTYPE,0,0,0 },
			{ HOTYPE_SPACE, 100, HOFLG_FREEWIDTH, 0, 0,0,0, NULL,0,ID_SPACE2,0,0,0 },

		{ HOTYPE_HGROUP, 100, HOFLG_NOBORDER|HOFLG_FREEWIDTH,2,0,0,0, NULL, 0,ID_MAINPUBSCRNAMEGROUP,0,0,0 },
			{ HOTYPE_BUTTON, 100, HOFLG_FREEWIDTH, 0,0,0,0, "PubScreen...",0,ID_MAINPUBSCRNAMEBUTTON,PLACETEXT_IN,0,0 },
			{ HOTYPE_STRING, 200, HOFLG_FREEWIDTH, (LONG)mainscreenname, MAXPUBSCREENNAME,0,0, NULL,0,ID_MAINPUBSCRNAMESTR,0,0,0 },

		{ HOTYPE_HGROUP, 100, HOFLG_NOBORDER|HOFLG_FREEWIDTH,3,0,0,0, NULL, 0,ID_MAINWINFONTGROUP,0,0,0 },
			{ HOTYPE_BUTTON, 100, HOFLG_FREEWIDTH, 0,0,0,0, "WindowFont...",0,ID_MAINWINFONTBUTTON,PLACETEXT_IN,0,0 },
			{ HOTYPE_TEXT, 150, HOFLG_FREEWIDTH, 0,0,0,0, NULL,0,ID_MAINWINFONTNAME,0,0,0 },
			{ HOTYPE_NUMBER, 50, HOFLG_FREEWIDTH, 0,0,0,0, NULL,0,ID_MAINWINFONTSIZE,0,0,0 },

		{ HOTYPE_CHECKBOX, 100, HOFLG_FREEWIDTH, 0,0,0,0, "Use system default font",0,ID_DEFAULTFONT,PLACETEXT_LEFT,0,0 },

		{ HOTYPE_HLINE, 100, HOFLG_NOBORDER|HOFLG_FREEWIDTH,0,0,0,0, NULL, 0,ID_HLINE3,0,0,0 },
		{ HOTYPE_SPACE, 200, HOFLG_NOBORDER|HOFLG_FREEWIDTH|HOFLG_FREEHEIGHT, 0,0,0,0, NULL,0,ID_SPACE3,0,0,0 },

		{ HOTYPE_HGROUP, 100, HOFLG_NOBORDER|HOFLG_FREEWIDTH,3,0,0,0, NULL, 0,ID_OKCANCELGROUP,0,0,0 },
			{ HOTYPE_BUTTON, 100, HOFLG_FREEWIDTH, 0,0,0,0, "  OK  ",0,ID_OK,PLACETEXT_IN,0,0 },
			{ HOTYPE_SPACE, 200, HOFLG_NOBORDER|HOFLG_FREEWIDTH, 0,0,0,0, NULL,0,ID_SPACE4,0,0,0 },
			{ HOTYPE_BUTTON, 100, HOFLG_FREEWIDTH, 0,0,0,0, "Cancel",0,ID_CANCEL,PLACETEXT_IN,0,0 },

	{HOTYPE_END}
};


BOOL OpenNewScreenConfigWindow( void )
{
	struct SuperWindow *sw;
	struct ScreenConfigWindow *sc;
	BOOL success = FALSE;

//	printf("OpenScreenConfigWindow()\n");

	/* make sure no other screenconfig windows open... */
	if( sw = FindSuperWindowByType( (struct SuperWindow *)superwindows.lh_Head,
		SWTY_SCRCONFIG ) )
	{
		if( sw->sw_Window )
			WindowToFront( sw->sw_Window );
		else
			(*sw->sw_ShowWindow)( sw );
		return( FALSE );
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

//	printf("ShowScreenConfigWindow()\n");

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
				InitGadgets( sc );

				HO_MakeLayout( gadlayout, sw->sw_hos, win->BorderLeft, win->BorderTop,
					win->Width - win->BorderLeft - win->BorderRight,
					win->Height - win->BorderTop - win->BorderBottom );

				DoDisableStuff(sc);
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

//	printf("Hide ScreenConfigWindow\n");

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
//	printf("Kill ScreenConfigWindow\n");

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
				switch( ((struct Gadget*)iaddress)->GadgetID )
				{
					case ID_GFXSCRMODEBUTTON:
						if( AslRequestTags( smreq,
							ASLSM_Window, win,
							ASLSM_SleepWindow, TRUE,
//							ASLSM_TextAttr, &zonkfig.scfg.scfg_MainWinFont,
							ASLSM_DoWidth, TRUE,
							ASLSM_DoHeight, TRUE,
							ASLSM_DoDepth, TRUE,
							ASLSM_DoOverscanType, TRUE,
							ASLSM_DoAutoScroll, TRUE,
							ASLSM_InitialDisplayID, sc->sc_LocalConfig.scfg_DisplayID,
							ASLSM_InitialDisplayWidth, sc->sc_LocalConfig.scfg_Width,
							ASLSM_InitialDisplayHeight, sc->sc_LocalConfig.scfg_Height,
							ASLSM_InitialDisplayDepth, sc->sc_LocalConfig.scfg_Depth,
//							ASLSM_InitialOverscanType, sc->sc_LocalConfig.scfg_OverscanType,
//							ASLSM_InitialAutoScroll, sc->sc_LocalConfig.scfg_AutoScroll,
							ASLSM_PropertyFlags,0,
							ASLSM_PropertyMask, DIPF_IS_DUALPF,
							TAG_END ) )
						{
							sc->sc_LocalConfig.scfg_DisplayID = smreq->sm_DisplayID;
							sc->sc_LocalConfig.scfg_Width = smreq->sm_DisplayWidth;
							sc->sc_LocalConfig.scfg_Height = smreq->sm_DisplayHeight;
							sc->sc_LocalConfig.scfg_Depth = smreq->sm_DisplayDepth;
//							sc->sc_LocalConfig.scfg_DisplayID = smreq->sm_OverscanType;
//							sc->sc_LocalConfig.scfg_AutoScroll = smreq->sm_AutoScroll;
							ComposeScreenModeString( sc->sc_ScreenModeBuf,
								sc->sc_LocalConfig.scfg_DisplayID, sc->sc_LocalConfig.scfg_Depth );
							gadlayout[ ID_GFXSCRMODETEXT ].ho_Attr0 =
								(ULONG)sc->sc_ScreenModeBuf;
							HO_RefreshObject( &gadlayout[ ID_GFXSCRMODETEXT ], sw->sw_hos );
						}
						break;
					case ID_GFXSCRFONTBUTTON:
						if( AslRequestTags( fontreq,
							ASLFO_Window, win,
							ASLFO_SleepWindow, TRUE,
//							ASLFO_TextAttr, &zonkfig.scfg.scfg_MainWinFont,
							ASLFO_DoStyle, TRUE,
							ASLFO_InitialName, sc->sc_LocalConfig.scfg_GFXScrFontName,
							ASLFO_InitialSize, sc->sc_LocalConfig.scfg_GFXScrFont.ta_YSize,
							ASLFO_InitialStyle, sc->sc_LocalConfig.scfg_GFXScrFont.ta_Style,
							ASLFO_InitialFlags, sc->sc_LocalConfig.scfg_GFXScrFont.ta_Flags,
							TAG_END ) )
						{
							sc->sc_LocalConfig.scfg_GFXScrFont = fontreq->fo_Attr;
//							sc->sc_LocalConfig.scfg_GFXScrFont.ta_Name = NULL;
							strcpy( sc->sc_LocalConfig.scfg_GFXScrFontName,
								fontreq->fo_Attr.ta_Name );
							gadlayout[ ID_GFXSCRFONTNAME ].ho_Attr0 =
								(ULONG)sc->sc_LocalConfig.scfg_GFXScrFontName;
							gadlayout[ ID_GFXSCRFONTSIZE ].ho_Attr0 =
								(ULONG)sc->sc_LocalConfig.scfg_GFXScrFont.ta_YSize;
							HO_RefreshObject( &gadlayout[ ID_GFXSCRFONTGROUP ], sw->sw_hos );
						}
						break;
					case ID_GFXWINFONTBUTTON:
						if( AslRequestTags( fontreq,
							ASLFO_Window, win,
							ASLFO_SleepWindow, TRUE,
//							ASLFO_TextAttr, &zonkfig.scfg.scfg_MainWinFont,
							ASLFO_DoStyle, TRUE,
							ASLFO_InitialName, sc->sc_LocalConfig.scfg_GFXWinFontName,
							ASLFO_InitialSize, sc->sc_LocalConfig.scfg_GFXWinFont.ta_YSize,
							ASLFO_InitialStyle, sc->sc_LocalConfig.scfg_GFXWinFont.ta_Style,
							ASLFO_InitialFlags, sc->sc_LocalConfig.scfg_GFXWinFont.ta_Flags,
							TAG_END ) )
						{
							sc->sc_LocalConfig.scfg_GFXWinFont = fontreq->fo_Attr;
							sc->sc_LocalConfig.scfg_GFXWinFont.ta_Name = NULL;
							strcpy( sc->sc_LocalConfig.scfg_GFXWinFontName,
								fontreq->fo_Attr.ta_Name );
							gadlayout[ ID_GFXWINFONTNAME ].ho_Attr0 =
								(ULONG)sc->sc_LocalConfig.scfg_GFXWinFontName;
							gadlayout[ ID_GFXWINFONTSIZE ].ho_Attr0 =
								(ULONG)sc->sc_LocalConfig.scfg_GFXWinFont.ta_YSize;
							HO_RefreshObject( &gadlayout[ ID_GFXWINFONTGROUP ], sw->sw_hos );
						}
						break;
					case ID_MAINWINFONTBUTTON:
						if( AslRequestTags( fontreq,
							ASLFO_Window, win,
							ASLFO_SleepWindow, TRUE,
							ASLFO_TextAttr, &zonkfig.scfg.scfg_MainWinFont,
							ASLFO_DoStyle, TRUE,
							ASLFO_InitialName, sc->sc_LocalConfig.scfg_MainWinFontName,
							ASLFO_InitialSize, sc->sc_LocalConfig.scfg_MainWinFont.ta_YSize,
							ASLFO_InitialStyle, sc->sc_LocalConfig.scfg_MainWinFont.ta_Style,
							ASLFO_InitialFlags, sc->sc_LocalConfig.scfg_MainWinFont.ta_Flags,
							TAG_END ) )
						{
							sc->sc_LocalConfig.scfg_MainWinFont = fontreq->fo_Attr;
							sc->sc_LocalConfig.scfg_MainWinFont.ta_Name = NULL;
							strcpy( sc->sc_LocalConfig.scfg_MainWinFontName,
								fontreq->fo_Attr.ta_Name );
							gadlayout[ ID_MAINWINFONTNAME ].ho_Attr0 =
								(ULONG)sc->sc_LocalConfig.scfg_MainWinFontName;
							gadlayout[ ID_MAINWINFONTSIZE ].ho_Attr0 =
								(ULONG)sc->sc_LocalConfig.scfg_MainWinFont.ta_YSize;
							HO_RefreshObject( &gadlayout[ ID_MAINWINFONTGROUP ], sw->sw_hos );
						}
						break;
					case ID_GFXSCRTYPE:
					case ID_MAINSCRTYPE:
					case ID_DEFAULTFONT:
						/* enable/disable appropriate gadget groups */
						DoDisableStuff( sc );
						break;
					case ID_CANCEL:
						KillWindow( sw );
						break;
					case ID_OK:
//						HideWindow( sw );
						UpdateLocalConfig( sc );
						if( hiddenwindows = ListAndHideWindows() )
						{
							InstallScreenConfig( &sc->sc_LocalConfig );
							ShowHiddenWindows( hiddenwindows );
						}
						break;
					case ID_PICKPALETTE:
						sc_PickPalette( sc );
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
// DoDisableStuff()
//
// Checks the gadget values and disables/enables gadgets
// (or groups of gadgets) accordingly.
// The gadgets should already have been created and attached
// to the window when this routine is called.

static void DoDisableStuff( struct ScreenConfigWindow *sc )
{

	/* update any values which may be out of date */
	HO_ReadGadgetStates( sc->sc_sw.sw_hos, gadlayout );

	/* check gfx screen cycle gadget */
	switch( gadlayout[ ID_GFXSCRTYPE ].ho_Value )
	{
		/* no gfx screen */
		case 0:
			HO_DisableObject( &gadlayout[ ID_GFXSCRMODEGROUP ], sc->sc_sw.sw_hos, TRUE );
			HO_DisableObject( &gadlayout[ ID_GFXSCRFONTGROUP ], sc->sc_sw.sw_hos, TRUE );
			HO_DisableObject( &gadlayout[ ID_GFXWINFONTGROUP ], sc->sc_sw.sw_hos, TRUE );
			HO_DisableObject( &gadlayout[ ID_GFXPUBSCRNAMEGROUP ], sc->sc_sw.sw_hos, TRUE );
			break;
		/* new gfx screen */
		case 1:
			HO_DisableObject( &gadlayout[ ID_GFXSCRMODEGROUP ], sc->sc_sw.sw_hos, FALSE );
			HO_DisableObject( &gadlayout[ ID_GFXSCRFONTGROUP ], sc->sc_sw.sw_hos, FALSE );
			HO_DisableObject( &gadlayout[ ID_GFXWINFONTGROUP ], sc->sc_sw.sw_hos, FALSE );
			HO_DisableObject( &gadlayout[ ID_GFXPUBSCRNAMEGROUP ], sc->sc_sw.sw_hos, TRUE );
			break;
		/* use existing pubscreen */
		case 2:
			HO_DisableObject( &gadlayout[ ID_GFXSCRMODEGROUP ], sc->sc_sw.sw_hos, TRUE );
			HO_DisableObject( &gadlayout[ ID_GFXSCRFONTGROUP ], sc->sc_sw.sw_hos, TRUE );
			HO_DisableObject( &gadlayout[ ID_GFXWINFONTGROUP ], sc->sc_sw.sw_hos, FALSE );
			HO_DisableObject( &gadlayout[ ID_GFXPUBSCRNAMEGROUP ], sc->sc_sw.sw_hos, FALSE );
			break;
	}

	/* check mainscreen cyclegadget */
	switch( gadlayout[ ID_MAINSCRTYPE ].ho_Value )
	{
		/* default pubscreen */
		case 0:
			HO_DisableObject( &gadlayout[ ID_MAINPUBSCRNAMEGROUP ], sc->sc_sw.sw_hos, TRUE );
			break;
		/* use gfx screen */
		case 1:
			HO_DisableObject( &gadlayout[ ID_MAINPUBSCRNAMEGROUP ], sc->sc_sw.sw_hos, TRUE );
			break;
		/* pubscreen */
		case 2:
			HO_DisableObject( &gadlayout[ ID_MAINPUBSCRNAMEGROUP ], sc->sc_sw.sw_hos, FALSE );
			break;
	}

	/* check defaultfont check gadget */
	if( gadlayout[ ID_DEFAULTFONT ].ho_Value )
		HO_DisableObject( &gadlayout[ ID_MAINWINFONTGROUP ], sc->sc_sw.sw_hos, TRUE );
	else
		HO_DisableObject( &gadlayout[ ID_MAINWINFONTGROUP ], sc->sc_sw.sw_hos, FALSE );
}


/********************************************************/
// InitGadgets()
//
// Set up the hoopyobject array with values from our local
// screenconfig structure. Also point to string editing
// buffers etc...
// Basically anything that isn't stored statically in gadlayout.
// Should be called before HO_MakeLayout() is invoked.

static void InitGadgets( struct ScreenConfigWindow *sc )
{
	gadlayout[ ID_GFXSCRTYPE ].ho_Value = sc->sc_LocalConfig.scfg_GFXScrType;

	gadlayout[ ID_GFXSCRMODETEXT ].ho_Attr0 =
		(LONG)sc->sc_ScreenModeBuf;

	gadlayout[ ID_GFXSCRFONTNAME ].ho_Attr0 =
		(LONG)sc->sc_LocalConfig.scfg_GFXScrFontName;
	gadlayout[ ID_GFXSCRFONTSIZE ].ho_Attr0 =
		(LONG)sc->sc_LocalConfig.scfg_GFXScrFont.ta_YSize;
	gadlayout[ ID_GFXWINFONTNAME ].ho_Attr0 =
		(LONG)sc->sc_LocalConfig.scfg_GFXWinFontName;
	gadlayout[ ID_GFXWINFONTSIZE ].ho_Attr0 =
		(LONG)sc->sc_LocalConfig.scfg_GFXWinFont.ta_YSize;

	gadlayout[ ID_GFXPUBSCRNAMESTR ].ho_Attr0 =
		(LONG)sc->sc_LocalConfig.scfg_GFXPubScrName;

	gadlayout[ ID_MAINSCRTYPE ].ho_Value = sc->sc_LocalConfig.scfg_MainScrType;

	gadlayout[ ID_MAINPUBSCRNAMESTR ].ho_Attr0 =
		(LONG)sc->sc_LocalConfig.scfg_MainPubScrName;

	gadlayout[ ID_DEFAULTFONT ].ho_Value = sc->sc_LocalConfig.scfg_DefaultFont;

	gadlayout[ ID_MAINWINFONTNAME ].ho_Attr0 =
		(LONG)sc->sc_LocalConfig.scfg_MainWinFontName;
	gadlayout[ ID_MAINWINFONTSIZE ].ho_Attr0 =
		(LONG)sc->sc_LocalConfig.scfg_MainWinFont.ta_YSize;

	ComposeScreenModeString( sc->sc_ScreenModeBuf,
		sc->sc_LocalConfig.scfg_DisplayID, sc->sc_LocalConfig.scfg_Depth );
}



/********************************************************/
// UpdateLocalConfig()
//
// Copy values out of the gadlayout array into our local
// screenconfig structure. Some values in the local struct
// will always be kept up to date by the IDCMP handler, like
// the screen dimensions and depth, because that sort of stuff
// is not stored in the gadgets, but comes from other sources
// (such as an ASL requester).

static void UpdateLocalConfig( struct ScreenConfigWindow *sc )
{
	/* update any values which may be out of date */
	HO_ReadGadgetStates( sc->sc_sw.sw_hos, gadlayout );

	sc->sc_LocalConfig.scfg_GFXScrType = gadlayout[ ID_GFXSCRTYPE ].ho_Value;
	sc->sc_LocalConfig.scfg_MainScrType = gadlayout[ ID_MAINSCRTYPE ].ho_Value;
	sc->sc_LocalConfig.scfg_DefaultFont = gadlayout[ ID_DEFAULTFONT ].ho_Value;
}

static VOID sc_PickPalette( struct ScreenConfigWindow *sc )
{
	struct Chunk *cnk;
	
	if( BlockAllSuperWindows() )
	{
		cnk = RequestChunk( sc->sc_sw.sw_Window, "Palette Request", "Pick Palette",
			ID_CMAP,NULL );
		if( cnk )
		{
			CopyMem( cnk->ch_Data, &sc->sc_LocalConfig.scfg_Palette,
				sizeof( struct Palette ) );
		}
	}
	UnblockAllSuperWindows();
}
