/********************************************************/
//
// WeaponWindow.c
//
/********************************************************/

#define WEAPONWINDOW_C

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

struct WeaponWindow
{
	struct SuperWindow	ww_sw;
	struct Chunk				*ww_Chunk;
	struct HoopyObject	*ww_Layout;
	struct WeaponDef		*ww_CurrentWeapon;				/* NULL = none */
};


#define GID_BORING 0

#define GID_PICKWEAPON 2
#define GID_WEAPONNAME 3
#define GID_NEW 4
#define GID_DEL 5
#define GID_EDITGROUP 7
#define GID_FIREPROG 9
#define GID_PICKPROG 10

#define GID_MAXBULLETS 11
#define GID_FIREDELAY 12
#define GID_POWERUP 14
#define GID_PICKPOWERUP 15
#define GID_POWERDOWN 17
#define GID_PICKPOWERDOWN 18

#define	GID_CREATEBULLET 19

static struct HoopyObject gadlayout[] =
{
	{ HOTYPE_VGROUP, 100, HOFLG_RAISEDBORDER,3,0,0,0, NULL, 0,GID_BORING,0,0,0 },
		{ HOTYPE_HGROUP, 100, HOFLG_FREEWIDTH,4,0,0,0, NULL, 0,GID_BORING,0,0,0 },
			{ HOTYPE_BUTTON, 100, HOFLG_NOBORDER, 0,0,0,0, "Weapon",0,GID_PICKWEAPON,PLACETEXT_IN,0,0 },
			{ HOTYPE_PACKEDSTRING, 100, HOFLG_NOBORDER, NULL,WEAPONNAMESIZE-1,0,0, NULL,0,GID_WEAPONNAME,0,0,0 },
			{ HOTYPE_BUTTON, 100, HOFLG_NOBORDER, 0,0,0,0, "New",0,GID_NEW,PLACETEXT_IN,0,0 },
			{ HOTYPE_BUTTON, 100, HOFLG_NOBORDER, 0,0,0,0, "Del",0,GID_DEL,PLACETEXT_IN,0,0 },
		{ HOTYPE_HLINE, 100, HOFLG_NOBORDER|HOFLG_FREEWIDTH, 0,0,0,0, NULL,0,GID_BORING,PLACETEXT_IN,0,0 },
		{ HOTYPE_VGROUP, 100, HOFLG_NOBORDER|HOFLG_FREEWIDTH,6,0,0,0, NULL, 0,GID_EDITGROUP,0,0,0 },
			{ HOTYPE_HGROUP, 100, HOFLG_FREEWIDTH,2,0,0,0, NULL, 0,GID_BORING,0,0,0 },
				{ HOTYPE_PACKEDSTRING, 100, HOFLG_NOBORDER, NULL,PROGNAMESIZE-1,0,0, "ActionList",0,GID_FIREPROG,PLACETEXT_LEFT,0,0 },
				{ HOTYPE_BUTTON, 100, HOFLG_NOBORDER, 0,0,0,0, "?",0,GID_PICKPROG,PLACETEXT_IN,0,0 },
			{ HOTYPE_INTEGER, 100, HOFLG_FREEWIDTH, 6,0,0,0, "MaxBullets",0,GID_MAXBULLETS,PLACETEXT_LEFT,0,0 },
			{ HOTYPE_INTEGER, 100, HOFLG_FREEWIDTH, 6,0,0,0, "FireDelay",0,GID_FIREDELAY,PLACETEXT_LEFT,0,0 },
			{ HOTYPE_HGROUP, 100, HOFLG_FREEWIDTH,2,0,0,0, NULL, 0,GID_BORING,0,0,0 },
				{ HOTYPE_PACKEDSTRING, 100, HOFLG_NOBORDER, NULL,WEAPONNAMESIZE-1,0,0, "PowerUp",0,GID_POWERUP,PLACETEXT_LEFT,0,0 },
				{ HOTYPE_BUTTON, 100, HOFLG_NOBORDER, 0,0,0,0, "?",0,GID_PICKPOWERUP,PLACETEXT_IN,0,0 },
			{ HOTYPE_HGROUP, 100, HOFLG_FREEWIDTH,2,0,0,0, NULL, 0,GID_BORING,0,0,0 },
				{ HOTYPE_PACKEDSTRING, 100, HOFLG_NOBORDER, NULL,WEAPONNAMESIZE-1,0,0, "PowerDown",0,GID_POWERDOWN,PLACETEXT_LEFT,0,0 },
				{ HOTYPE_BUTTON, 100, HOFLG_NOBORDER, 0,0,0,0, "?",0,GID_PICKPOWERDOWN,PLACETEXT_IN,0,0 },
			{ HOTYPE_CHECKBOX, 100, HOFLG_NOBORDER, 0,0,0,0, "Create bullet?",0,GID_CREATEBULLET,PLACETEXT_RIGHT,0,0 },
	{HOTYPE_END}
};



static void IDCMPHandler( struct SuperWindow *sw,
	struct IntuiMessage *imsg );
static void HideWindow( struct SuperWindow *sw );
static BOOL ShowWindow( struct SuperWindow *sw );
static void KillWindow( struct SuperWindow *sw );
static BOOL RefreshMenus( struct SuperWindow *sw );
static UBYTE *MakeWeaponWinTitle( struct SuperWindow *sw );

static VOID NewWeapon( struct WeaponWindow *ww );
static VOID DelWeapon( struct WeaponWindow *ww );
static VOID SelectWeapon( struct WeaponWindow *ww, struct WeaponDef *wd );
static VOID ReadGads( struct WeaponWindow *ww );
static VOID PickFirstWeapon( struct WeaponWindow *ww );
static void PickProg( struct WeaponWindow *ww );
static void PickPowerUpDown( struct WeaponWindow *ww, int down );
static void PickWeapon( struct WeaponWindow *ww );

/********************************************************/

BOOL OpenNewWeaponWindow( struct Chunk *cnk )
{
	struct SuperWindow *sw;
	struct WeaponWindow *ww;
	BOOL success = FALSE;

	assert( cnk != NULL );
	assert( cnk->ch_TypeID == ID_WEAP );

	LockChunk( cnk );

	if( ww = AllocVec( sizeof( struct WeaponWindow ), MEMF_ANY|MEMF_CLEAR ) )
	{
		sw = &ww->ww_sw;
		sw->sw_Handler = IDCMPHandler;
		sw->sw_HideWindow = HideWindow;
		sw->sw_ShowWindow = ShowWindow;
		sw->sw_BuggerOff = KillWindow;
		sw->sw_RefreshMenus = RefreshMenus;
		sw->sw_MakeTitle	= MakeWeaponWinTitle;
		sw->sw_Type				= SWTY_WEAPON;
		sw->sw_Window			= NULL;
		sw->sw_Gadgets		= NULL;
		sw->sw_VisualInfo	= NULL;
		sw->sw_WinDim			= zonkfig.SuperWindowDefs[ SWTY_WEAPON ];

		ww->ww_Chunk = cnk;
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
	struct WeaponWindow *ww;
	struct Window *win;
	struct Screen *scr;

	ww = (struct WeaponWindow *)sw;

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
		WA_Title,				(*sw->sw_MakeTitle)( sw ),
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

		if( ww->ww_Layout = AllocVec( sizeof( gadlayout ), MEMF_ANY ) )
		{
			CopyMem( gadlayout, ww->ww_Layout, sizeof( gadlayout ) );
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
					STRINGIDCMP|INTEGERIDCMP|SCROLLERIDCMP|BUTTONIDCMP );

				HO_SussWindowSizing( sw->sw_hos, ww->ww_Layout );

				HO_MakeLayout( ww->ww_Layout, sw->sw_hos, win->BorderLeft, win->BorderTop,
					win->Width - win->BorderLeft - win->BorderRight,
					win->Height - win->BorderTop - win->BorderBottom );

				PickFirstWeapon( ww );

				success = TRUE;
			}
		}
	}

	if( !success )
	{
		if( sw->sw_Window ) CloseWindow( sw->sw_Window );
		sw->sw_Window = NULL;
		if( ww->ww_Layout ) FreeVec( ww->ww_Layout );
		ww->ww_Layout = NULL;
	}

	return( success );
}


/********************************************************/

static void HideWindow( struct SuperWindow *sw )
{
	struct WeaponWindow *ww;

	ww = (struct WeaponWindow *)sw;

	if( sw->sw_Window )
	{
		RememberWindow( sw );
		GenericZapMenus( sw );
		HO_KillLayout( sw->sw_hos );
		CloseWindowSafely( sw->sw_Window );
		HO_FreeHOStuff( sw->sw_hos );
		sw->sw_Window = NULL;
		FreeVec( ww->ww_Layout );
		ww->ww_Layout = NULL;
	}
}

/********************************************************/

static void KillWindow( struct SuperWindow *sw )
{
	if( sw && IsOKToClose( sw ) )
	{
		(*sw->sw_HideWindow)(sw);
		UnlockChunk( ((struct WeaponWindow *)sw)->ww_Chunk );
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

static UBYTE *MakeWeaponWinTitle( struct SuperWindow *sw )
{
	struct Chunk *cnk;
	UBYTE *p;

	cnk = ((struct WeaponWindow *)sw)->ww_Chunk;
	p = sw->sw_Name;

	if( cnk->ch_Modified )
		*p++ = MODCHAR;
	strcpy( p, "Weapons" );

	return( sw->sw_Name );
}



/********************************************************/

static void IDCMPHandler( struct SuperWindow *sw,
	struct IntuiMessage *origimsg )
{
	struct IntuiMessage *imsg;
	struct Window *win;
	struct WeaponWindow *ww;

	ULONG class;
	UWORD code;
//	UWORD qualifier;
	APTR iaddress;
	WORD mousex, mousey;
	ULONG seconds, micros;

	ww = (struct WeaponWindow *)sw;
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
				HO_ReadGadgetStates( sw->sw_hos, ww->ww_Layout );
				HO_MakeLayout( ww->ww_Layout, sw->sw_hos, win->BorderLeft, win->BorderTop,
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
				HO_ReadGadgetStates( ww->ww_sw.sw_hos, ww->ww_Layout );
				switch( ((struct Gadget*)iaddress)->GadgetID )
				{
					case GID_NEW:
						ModifyChunk( ww->ww_Chunk );
						NewWeapon( ww );
						break;
					case GID_DEL:
						ModifyChunk( ww->ww_Chunk );
						DelWeapon( ww );
						break;
					case GID_PICKWEAPON:
						PickWeapon( ww );
						break;
					case GID_PICKPROG:
						PickProg( ww );
						ModifyChunk( ww->ww_Chunk );
						break;
					case GID_PICKPOWERUP:
						PickPowerUpDown( ww, 0 );
						ModifyChunk( ww->ww_Chunk );
						break;
					case GID_PICKPOWERDOWN:
						PickPowerUpDown( ww, 1 );
						ModifyChunk( ww->ww_Chunk );
						break;
					case GID_WEAPONNAME:
					case GID_FIREPROG:
					case GID_POWERUP:
					case GID_POWERDOWN:
					case GID_MAXBULLETS:
					case GID_FIREDELAY:
					case GID_CREATEBULLET:
						ModifyChunk( ww->ww_Chunk );
						ReadGads( ww );
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

static VOID SelectWeapon( struct WeaponWindow *ww, struct WeaponDef *wd )
{
	ww->ww_CurrentWeapon = wd;
	if( wd )
	{
		MakePackable( wd->wd_Name );
		MakePackable( wd->wd_FireProgram );
		MakePackable( wd->wd_PowerUp );
		MakePackable( wd->wd_PowerDown );

		ww->ww_Layout[ GID_WEAPONNAME ].ho_Attr0 = (LONG)wd->wd_Name;
		ww->ww_Layout[ GID_FIREPROG ].ho_Attr0 = (LONG)wd->wd_FireProgram;
		ww->ww_Layout[ GID_POWERUP ].ho_Attr0 = (LONG)wd->wd_PowerUp;
		ww->ww_Layout[ GID_POWERDOWN ].ho_Attr0 = (LONG)wd->wd_PowerDown;
		ww->ww_Layout[ GID_MAXBULLETS ].ho_Value = (LONG)wd->wd_MaxBullets;
		ww->ww_Layout[ GID_FIREDELAY ].ho_Value = (LONG)wd->wd_FireDelay;
		ww->ww_Layout[ GID_CREATEBULLET ].ho_Value =
			(LONG)((wd->wd_Flags & 1) ? TRUE:FALSE );

		HO_RefreshObject( &ww->ww_Layout[ GID_WEAPONNAME ], ww->ww_sw.sw_hos );
		HO_RefreshObject( &ww->ww_Layout[ GID_EDITGROUP ], ww->ww_sw.sw_hos );
		HO_DisableObject( &ww->ww_Layout[ GID_WEAPONNAME ], ww->ww_sw.sw_hos,
			FALSE );
		HO_DisableObject( &ww->ww_Layout[ GID_PICKWEAPON ], ww->ww_sw.sw_hos,
			FALSE );
		HO_DisableObject( &ww->ww_Layout[ GID_DEL ], ww->ww_sw.sw_hos,
			FALSE );
		HO_DisableObject( &ww->ww_Layout[ GID_EDITGROUP ], ww->ww_sw.sw_hos,
			FALSE );
	}
	else
	{
		ww->ww_Layout[ GID_WEAPONNAME ].ho_Attr0 = NULL;
		ww->ww_Layout[ GID_FIREPROG ].ho_Attr0 = NULL;
		ww->ww_Layout[ GID_POWERUP ].ho_Attr0 = NULL;
		ww->ww_Layout[ GID_POWERDOWN ].ho_Attr0 = NULL;
		ww->ww_Layout[ GID_MAXBULLETS ].ho_Value = 0;
		ww->ww_Layout[ GID_FIREDELAY ].ho_Value = 0;
		ww->ww_Layout[ GID_CREATEBULLET ].ho_Value = FALSE;

		HO_RefreshObject( &ww->ww_Layout[ GID_WEAPONNAME ], ww->ww_sw.sw_hos );
		HO_RefreshObject( &ww->ww_Layout[ GID_EDITGROUP ], ww->ww_sw.sw_hos );

		HO_DisableObject( &ww->ww_Layout[ GID_WEAPONNAME ], ww->ww_sw.sw_hos,
			TRUE );

		if( IsListEmpty( &ww->ww_Chunk->ch_DataList ) )
		{
			HO_DisableObject( &ww->ww_Layout[ GID_PICKWEAPON ], ww->ww_sw.sw_hos,
				TRUE );
		}
		else
		{
			HO_DisableObject( &ww->ww_Layout[ GID_PICKWEAPON ], ww->ww_sw.sw_hos,
				FALSE );
		}

		HO_DisableObject( &ww->ww_Layout[ GID_DEL ], ww->ww_sw.sw_hos,
			TRUE );
		HO_DisableObject( &ww->ww_Layout[ GID_EDITGROUP ], ww->ww_sw.sw_hos,
			TRUE );
	}
}

/********************************************************/
static VOID NewWeapon( struct WeaponWindow *ww )
{
	struct WeaponDef *wd;

	if( wd = AllocVec( sizeof( struct WeaponDef ), MEMF_ANY|MEMF_CLEAR ) )
	{
		Mystrncpy( wd->wd_Name, defname, WEAPONNAMESIZE-1 );
		wd->wd_Node.ln_Name = wd->wd_Name;
		AddTail( &ww->ww_Chunk->ch_DataList, (struct Node *)wd );
		SelectWeapon( ww, wd );
	}
}

/********************************************************/
static VOID DelWeapon( struct WeaponWindow *ww )
{
	struct WeaponDef *wd;

	if( wd = ww->ww_CurrentWeapon )
	{
		Remove( (struct Node *)wd );
		PickFirstWeapon( ww );
		FreeVec( wd );
	}
}

/********************************************************/
static VOID PickFirstWeapon( struct WeaponWindow *ww )
{
	if( !IsListEmpty( &ww->ww_Chunk->ch_DataList ) )
		SelectWeapon( ww, (struct WeaponDef *)ww->ww_Chunk->ch_DataList.lh_Head );
	else
		SelectWeapon( ww, NULL );
}

/********************************************************/
static VOID ReadGads( struct WeaponWindow *ww )
{
	struct WeaponDef *wd;

	if( wd = ww->ww_CurrentWeapon )
	{
		MakePackable( wd->wd_Name );
		MakePackable( wd->wd_FireProgram );
		MakePackable( wd->wd_PowerUp );
		MakePackable( wd->wd_PowerDown );

		wd->wd_MaxBullets = (UWORD)ww->ww_Layout[ GID_MAXBULLETS ].ho_Value;
		wd->wd_FireDelay = (UWORD)ww->ww_Layout[ GID_FIREDELAY ].ho_Value;

		if( ww->ww_Layout[ GID_CREATEBULLET ].ho_Value )
			wd->wd_Flags |= 1;
		else
			wd->wd_Flags &= ~1;

		HO_RefreshObject( &ww->ww_Layout[ GID_WEAPONNAME ], ww->ww_sw.sw_hos );
		HO_RefreshObject( &ww->ww_Layout[ GID_FIREPROG ], ww->ww_sw.sw_hos );
		HO_RefreshObject( &ww->ww_Layout[ GID_POWERUP ], ww->ww_sw.sw_hos );
		HO_RefreshObject( &ww->ww_Layout[ GID_POWERDOWN ], ww->ww_sw.sw_hos );
	}
}

/********************************************************/
static void PickProg( struct WeaponWindow *ww )
{
	struct Program *prog = NULL;
	struct WeaponDef *wd;

	if( wd = ww->ww_CurrentWeapon )
	{
		if( BlockAllSuperWindows() )
		{
			prog = RequestProgram( ww->ww_sw.sw_Window, ww->ww_Chunk,
				"Pick ActionList", "ActionLists" );
		}
		UnblockAllSuperWindows();

		if( prog )
		{
			Mystrncpy( wd->wd_FireProgram, prog->pg_Name, PROGNAMESIZE-1 );
			HO_RefreshObject( &ww->ww_Layout[ GID_FIREPROG ], ww->ww_sw.sw_hos );
		}
	}
}

/********************************************************/
static void PickPowerUpDown( struct WeaponWindow *ww, int down )
{
	struct WeaponDef *wd, *powerweap = NULL;

	if( wd = ww->ww_CurrentWeapon )
	{
		if( BlockAllSuperWindows() )
		{
			powerweap = RequestWeapon( ww->ww_sw.sw_Window,
				down ? "PowerDown Weapon" : "PowerUp Weapon", ww->ww_Chunk );
		}
		UnblockAllSuperWindows();

		if( powerweap )
		{
			if( down == 0 )
			{
				Mystrncpy( wd->wd_PowerUp, powerweap->wd_Name,
					WEAPONNAMESIZE-1 );
				HO_RefreshObject( &ww->ww_Layout[ GID_POWERUP ], ww->ww_sw.sw_hos );
			}
			else
			{
				Mystrncpy( wd->wd_PowerDown, powerweap->wd_Name,
					WEAPONNAMESIZE-1 );
				HO_RefreshObject( &ww->ww_Layout[ GID_POWERDOWN ], ww->ww_sw.sw_hos );

			}
		}
	}
}


/********************************************************/

static void PickWeapon( struct WeaponWindow *ww )
{
	struct WeaponDef *wd;

	if( BlockAllSuperWindows() )
	{
		wd = RequestWeapon( ww->ww_sw.sw_Window,
							"Pick Weapon", ww->ww_Chunk );
	}
	UnblockAllSuperWindows();

	if( wd )
		SelectWeapon( ww, wd );

}

