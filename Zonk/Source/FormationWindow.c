/********************************************************/
//
// FormationWindow.c
//
/********************************************************/

#define FORMATIONWINDOW_C

#include <stdio.h>
#include <string.h>
//#include <stdarg.h>
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
//#include <libraries/iffparse.h>

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

struct FormationWindow
{
	struct SuperWindow	fmw_sw;
	struct HoopyObject	*fmw_Layout;
	struct Chunk				*fmw_Chunk;

	struct Formation		*fmw_CurrentFormation;
	struct Dude					*fmw_SelectedDude;	// or NULL
	BOOL								fmw_MultiPick;		//In multi-pick mode?
	BOOL								fmw_DragFlag;			//Moving stuff about?
	LONG								fmw_PrevX;
	LONG								fmw_PrevY;

	LONG								fmw_SpaceLeft;		//Extent of space containing all dudes
	LONG								fmw_SpaceTop;
	LONG								fmw_SpaceRight;
	LONG								fmw_SpaceBottom;

	LONG								fmw_ViewXPos;			//Current view position in space coords
	LONG								fmw_ViewYPos;

	LONG								fmw_ViewLeft;			//View window in rastport coords
	LONG								fmw_ViewTop;
	LONG								fmw_ViewWidth;
	LONG								fmw_ViewHeight;

	LONG								fmw_TotalLeft;
	LONG								fmw_TotalTop;
	LONG								fmw_TotalRight;
	LONG								fmw_TotalBottom;
};



 
#define MARKERSIZE 4
#define SCROLLERSCALE 16

#define GAD_BORING					0
#define GAD_FORMATIONNAME		2
#define GAD_SELECTFORMATION	3
#define GAD_NEWFORMATION		4
#define GAD_DELETEFORMATION	5
#define GAD_VIEW						7
#define GAD_VSCROLLER				8
#define GAD_HSCROLLER				10

#define GAD_ADDDUDE					13
#define GAD_DELETEDUDE			14

#define GAD_SELECTGROUP			15
#define GAD_PROGSTRING			16
#define GAD_PICKPROG				17
#define GAD_XPOSINT					18
#define GAD_YPOSINT					19
#define GAD_DELAYINT				20

static struct HoopyObject gadlayout[] =
{
	{ HOTYPE_VGROUP, 100, HOFLG_FREEWIDTH,5,0,0,0, NULL, 0,0,GAD_BORING,0,0 },
		{ HOTYPE_HGROUP, 100, HOFLG_NOBORDER|HOFLG_FREEWIDTH,4,0,0,0, NULL, 0,0,GAD_BORING,0,0 },
			{ HOTYPE_TEXT, 100, HOFLG_NOBORDER|HOFLG_FREEWIDTH, 0,0,0,0, NULL,0,GAD_FORMATIONNAME,0,0 },
			{ HOTYPE_BUTTON, 100, HOFLG_NOBORDER, 0,0,0,0, "Select...",0,GAD_SELECTFORMATION,PLACETEXT_IN,0 },
			{ HOTYPE_BUTTON, 100, HOFLG_NOBORDER, 0,0,0,0, "New",0,GAD_NEWFORMATION,PLACETEXT_IN,0 },
			{ HOTYPE_BUTTON, 100, HOFLG_NOBORDER, 0,0,0,0, "Delete",0,GAD_DELETEFORMATION,PLACETEXT_IN,0 },
		{ HOTYPE_HGROUP, 100, HOFLG_NOBORDER|HOFLG_FREEWIDTH|HOFLG_FREEHEIGHT,2,0,0,0, NULL, 0,0,GAD_BORING,0,0 },
			{ HOTYPE_SPACE, 100, HOFLG_RAISEDBORDER|HOFLG_FREEWIDTH|HOFLG_FREEHEIGHT, 0,0,0,0, NULL,0,GAD_VIEW,0,0 },
			{ HOTYPE_VSCROLLER, 100, HOFLG_NOBORDER|HOFLG_FREEHEIGHT, 5,50,0,0, NULL,0,GAD_VSCROLLER,0,0 },
		{ HOTYPE_HGROUP, 100, HOFLG_NOBORDER|HOFLG_FREEWIDTH,2,0,0,0, NULL, 0,0,GAD_BORING,0,0 },
			{ HOTYPE_HSCROLLER, 100, HOFLG_NOBORDER|HOFLG_FREEWIDTH, 5,50,0,0, NULL,0,GAD_HSCROLLER,0,0 },
			{ HOTYPE_SCROLLYSPACE, 100, HOFLG_NOBORDER, 0,0,0,0, NULL,0,GAD_BORING,0,0 },

		{ HOTYPE_HGROUP, 100, HOFLG_NOBORDER|HOFLG_FREEWIDTH,2,0,0,0, NULL, 0,0,GAD_BORING,0,0 },
			{ HOTYPE_BUTTON, 100, HOFLG_NOBORDER, 0,0,0,0, "Add Dude",0,GAD_ADDDUDE,PLACETEXT_IN,0 },
			{ HOTYPE_BUTTON, 100, HOFLG_NOBORDER, 0,0,0,0, "Delete Dude(s)",0,GAD_DELETEDUDE,PLACETEXT_IN,0 },
		{ HOTYPE_HGROUP, 100, HOFLG_NOBORDER|HOFLG_FREEWIDTH,5,0,0,0, NULL, 0,0,GAD_BORING,0,0 },
			{ HOTYPE_STRING, 300, HOFLG_NOBORDER|HOFLG_FREEWIDTH, 0,PROGNAMESIZE-1,0,0,"Program",0,GAD_PROGSTRING,PLACETEXT_BELOW,0 },
			{ HOTYPE_BUTTON, 100, HOFLG_NOBORDER, 0,0,0,0, "P",0,GAD_PICKPROG,PLACETEXT_IN,0 },
			{ HOTYPE_INTEGER, 100, HOFLG_NOBORDER|HOFLG_FREEWIDTH, 8,0,0,0, "X",0,GAD_XPOSINT,PLACETEXT_BELOW,0 },
			{ HOTYPE_INTEGER, 100, HOFLG_NOBORDER|HOFLG_FREEWIDTH, 8,0,0,0, "Y",0,GAD_YPOSINT,PLACETEXT_BELOW,0 },
			{ HOTYPE_INTEGER, 100, HOFLG_NOBORDER|HOFLG_FREEWIDTH, 8,0,0,0, "Delay",0,GAD_DELAYINT,PLACETEXT_BELOW,0 },
	{HOTYPE_END}
};



static void IDCMPHandler( struct SuperWindow *sw,
	struct IntuiMessage *imsg );
static void HideWindow( struct SuperWindow *sw );
static BOOL ShowWindow( struct SuperWindow *sw );
static VOID KillWindow( struct SuperWindow *sw );
static BOOL RefreshMenus( struct SuperWindow *sw );
static UBYTE *MakeFormationWinTitle( struct SuperWindow *sw );

static VOID DrawAxes( struct FormationWindow *fmw );
static VOID ClearView( struct FormationWindow *fmw );
static VOID FindViewSize( struct FormationWindow *fmw );
static VOID RedrawView( struct FormationWindow *fmw );
static VOID SetScrollers( struct FormationWindow *fmw );
static VOID ReadVScroller( struct FormationWindow *fmw );
static VOID ReadHScroller( struct FormationWindow *fmw );
static BOOL fmw_AddDude( struct FormationWindow *fmw, LONG mousex, LONG mousey );
static VOID CalcSpaceEdges( struct FormationWindow *fmw );
static VOID SetCurrentFormation( struct FormationWindow *fmw,
	struct Formation *fm );
static BOOL fmw_AddFormation( struct FormationWindow *fmw );
static VOID fmw_DeleteFormation( struct FormationWindow *fmw );
static VOID fmw_SelectFormation( struct FormationWindow *fmw );
static VOID fmw_HandleMouseButtons( struct FormationWindow *fmw,
	UWORD code, WORD mousex, WORD mousey );
static VOID fmw_PickDude( struct FormationWindow *fmw, WORD mousex, WORD mousey );
static VOID fmw_SelectDude( struct FormationWindow *fmw, struct Dude *dude );
static VOID DrawDude( struct FormationWindow *fmw, struct Dude *dude );
static VOID UnDrawDude( struct FormationWindow *fmw, struct Dude *dude );
static VOID fmw_UnTagAllDudes( struct FormationWindow *fmw );
static VOID fmw_MoveTaggedDudes( struct FormationWindow *fmw,
	LONG xdelta, LONG ydelta );
static VOID fmw_HandleMouseMove( struct FormationWindow *fmw,
	UWORD code, WORD mousex, WORD mousey );
static VOID DrawTaggedDudes( struct FormationWindow *fmw );
static VOID fmw_DeleteDudes( struct FormationWindow *fmw );
static VOID UpdateSelectedDudeParams( struct FormationWindow *fmw );

//ULONG KPrintF( STRPTR, ... );

/********************************************************/

BOOL OpenNewFormationWindow( struct Chunk *cnk )
{
	struct SuperWindow *sw;
	struct FormationWindow *fmw;
	BOOL success = FALSE;

	assert( cnk != NULL );

	for( sw = (struct SuperWindow *)superwindows.lh_Head;
		sw->sw_Node.ln_Succ;
		sw = (struct SuperWindow *)sw->sw_Node.ln_Succ )
	{
		if( sw->sw_Type == SWTY_FORMATION )
		{
			fmw = (struct FormationWindow *)sw;
			if( fmw->fmw_Chunk == cnk )
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

	if( fmw = AllocVec( sizeof( struct FormationWindow ), MEMF_ANY|MEMF_CLEAR ) )
	{
		sw = &fmw->fmw_sw;
		sw->sw_Handler = IDCMPHandler;
		sw->sw_HideWindow = HideWindow;
		sw->sw_ShowWindow = ShowWindow;
		sw->sw_BuggerOff = KillWindow;
		sw->sw_RefreshMenus = RefreshMenus;
		sw->sw_MakeTitle	= MakeFormationWinTitle;
		sw->sw_Type				= SWTY_FORMATION;
		sw->sw_Window			= NULL;
		sw->sw_Gadgets		= NULL;
		sw->sw_VisualInfo	= NULL;
		sw->sw_WinDim			= zonkfig.SuperWindowDefs[ SWTY_FORMATION ];

		fmw->fmw_Chunk = cnk;
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
	struct FormationWindow *fmw;
	struct Window *win;
	struct Screen *scr;

	assert( sw != NULL );

	fmw = (struct FormationWindow *)sw;

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
		WA_ReportMouse,	TRUE,
		WA_Activate,		TRUE,
		TAG_DONE,				NULL);

	UnlockMainScreen( scr );

	if( sw->sw_Window = win )
	{
		if( !zonkfig.scfg.scfg_DefaultFont && mainwinfont )
			SetFont( win->RPort, mainwinfont );

		if( fmw->fmw_Layout = AllocVec( sizeof( gadlayout ), MEMF_ANY ) )
		{
			CopyMem( gadlayout, fmw->fmw_Layout, sizeof( gadlayout ) );
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
					STRINGIDCMP|INTEGERIDCMP|SCROLLERIDCMP|BUTTONIDCMP );

				HO_SussWindowSizing( sw->sw_hos, fmw->fmw_Layout );

				HO_MakeLayout( fmw->fmw_Layout, sw->sw_hos, win->BorderLeft, win->BorderTop,
					win->Width - win->BorderLeft - win->BorderRight,
					win->Height - win->BorderTop - win->BorderBottom );

				if( !IsListEmpty( &fmw->fmw_Chunk->ch_DataList ) )
					SetCurrentFormation( fmw, (struct Formation *)fmw->fmw_Chunk->ch_DataList.lh_Head );
				else
					SetCurrentFormation( fmw, NULL );

				fmw_SelectDude( fmw, NULL );
				CalcSpaceEdges( fmw );
				FindViewSize( fmw );
				RedrawView( fmw );
				SetScrollers( fmw );

				success = TRUE;
			}
			else
			{
			}
		}
	}

	if( !success )
	{
		if( sw->sw_Window ) CloseWindow( sw->sw_Window );
		sw->sw_Window = NULL;
		if( fmw->fmw_Layout ) FreeVec( fmw->fmw_Layout );
		fmw->fmw_Layout = NULL;
	}

	return( success );
}


/********************************************************/

static void HideWindow( struct SuperWindow *sw )
{
	struct FormationWindow *fmw;

	fmw = (struct FormationWindow *)sw;

	assert( sw != NULL );

	if( sw->sw_Window )
	{
		RememberWindow( sw );
		GenericZapMenus( sw );
		HO_KillLayout( sw->sw_hos );
		CloseWindowSafely( sw->sw_Window );
		HO_FreeHOStuff( sw->sw_hos );
		sw->sw_Window = NULL;
		FreeVec( fmw->fmw_Layout );
		fmw->fmw_Layout = NULL;
	}
}

/********************************************************/

static VOID KillWindow( struct SuperWindow *sw )
{
	if( sw && IsOKToClose( sw ) )
	{
		(*sw->sw_HideWindow)(sw);
		UnlockChunk( ((struct FormationWindow *)sw)->fmw_Chunk );
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

static UBYTE *MakeFormationWinTitle( struct SuperWindow *sw )
{
	struct Chunk *cnk;
	UBYTE *p;

	cnk = ((struct FormationWindow *)sw)->fmw_Chunk;
	p = sw->sw_Name;

	if( cnk->ch_Modified )
		*p++ = MODCHAR;
	strcpy( p, "Formations" );

	return( sw->sw_Name );
}


/********************************************************/

static void IDCMPHandler( struct SuperWindow *sw,
	struct IntuiMessage *origimsg )
{
	struct IntuiMessage *imsg;
	struct Window *win;
	struct FormationWindow *fmw;

	ULONG class;
	UWORD code;
//	UWORD qualifier;
	APTR iaddress;
	WORD mousex, mousey;
	ULONG seconds, micros;

	fmw = (struct FormationWindow *)sw;
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
				HO_ReadGadgetStates( sw->sw_hos, fmw->fmw_Layout );
				HO_MakeLayout( fmw->fmw_Layout, sw->sw_hos, win->BorderLeft, win->BorderTop,
					win->Width - win->BorderLeft - win->BorderRight,
					win->Height - win->BorderTop - win->BorderBottom );
				FindViewSize( fmw );
				RedrawView( fmw );
				SetScrollers( fmw );

				ReplyMsg( (struct Message *)origimsg );
				break;
			case IDCMP_CHANGEWINDOW:
				UpdateWinDef( sw );
				ReplyMsg( (struct Message *)origimsg );
				break;
			case IDCMP_MOUSEBUTTONS:
				ReplyMsg( (struct Message *)origimsg );
				fmw_HandleMouseButtons( fmw, code, mousex, mousey );
				break;
			case IDCMP_MOUSEMOVE:
				ReplyMsg( (struct Message *)origimsg );
				fmw_HandleMouseMove( fmw, code, mousex, mousey );
				break;
			case IDCMP_GADGETUP:
				ReplyMsg( (struct Message *)origimsg );
				switch( ((struct Gadget*)iaddress)->GadgetID )
				{
					case GAD_VSCROLLER:
						ReadVScroller( fmw );
						break;
					case GAD_HSCROLLER:
						ReadHScroller( fmw );
						break;
					case GAD_NEWFORMATION:
						ModifyChunk( fmw->fmw_Chunk );
						fmw_AddFormation( fmw );
						break;
					case GAD_DELETEFORMATION:
						ModifyChunk( fmw->fmw_Chunk );
						fmw_DeleteFormation( fmw );
						break;
					case GAD_SELECTFORMATION:
						fmw_SelectFormation( fmw );
						break;
					case GAD_DELETEDUDE:
						ModifyChunk( fmw->fmw_Chunk );
						fmw_DeleteDudes( fmw );
						break;
					case GAD_ADDDUDE:
						ModifyChunk( fmw->fmw_Chunk );
						fmw_AddDude( fmw, (LONG)mousex, (LONG)mousey );
						break;
					case GAD_PROGSTRING:
					case GAD_XPOSINT:
					case GAD_YPOSINT:
					case GAD_DELAYINT:
						ModifyChunk( fmw->fmw_Chunk );
						UpdateSelectedDudeParams( fmw );
						break;
				}
				break;
			case IDCMP_REFRESHWINDOW:
				GT_BeginRefresh(win);
				GT_EndRefresh(win,TRUE);
				ReplyMsg( (struct Message *)origimsg );
				break;
			case IDCMP_RAWKEY:
				ReplyMsg( (struct Message *)origimsg );
				switch( code )
				{
					case RAWKEY_UP:
						fmw->fmw_ViewYPos -= 8;
						break;
					case RAWKEY_DOWN:
						fmw->fmw_ViewYPos += 8;
						break;
					case RAWKEY_RIGHT:
						fmw->fmw_ViewXPos += 8;
						break;
					case RAWKEY_LEFT:
						fmw->fmw_ViewXPos -= 8;
						break;
					case RAWKEY_SPACE:
						ModifyChunk( fmw->fmw_Chunk );
						fmw_AddDude( fmw, (LONG)mousex, (LONG)mousey );
						break;
					case 0x60:											/* shift keys down*/
					case 0x61:
						fmw->fmw_MultiPick = TRUE;
						break;
					case 0x60 + IECODE_UP_PREFIX:		/* shift keys up */
					case 0x61 + IECODE_UP_PREFIX:
						fmw->fmw_MultiPick = FALSE;
						break;
				}
				RedrawView( fmw );
				SetScrollers( fmw );
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

static VOID ClearView( struct FormationWindow *fmw )
{
	struct Window *win;
	ULONG w,h,l,t;

	win = fmw->fmw_sw.sw_Window;

	SetAPen( win->RPort, 0 );
	SetDrMd( win->RPort, JAM1 );
	l = fmw->fmw_Layout[GAD_VIEW].ho_Attr0 >> 16;
	t = fmw->fmw_Layout[GAD_VIEW].ho_Attr0 & 0xFFFF;
	w = fmw->fmw_Layout[GAD_VIEW].ho_Attr1 >> 16;
	h = fmw->fmw_Layout[GAD_VIEW].ho_Attr1 & 0xFFFF;

	RectFill( win->RPort, l,t,l+w,t+h );
}

/********************************************************/

static VOID FindViewSize( struct FormationWindow *fmw )
{
	ULONG w,h,l,t;

	l = fmw->fmw_Layout[GAD_VIEW].ho_Attr0 >> 16;
	t = fmw->fmw_Layout[GAD_VIEW].ho_Attr0 & 0xFFFF;
	w = fmw->fmw_Layout[GAD_VIEW].ho_Attr1 >> 16;
	h = fmw->fmw_Layout[GAD_VIEW].ho_Attr1 & 0xFFFF;

	fmw->fmw_ViewLeft = l;
	fmw->fmw_ViewWidth = w;
	fmw->fmw_ViewTop = t;
	fmw->fmw_ViewHeight = h;

}

/********************************************************/

static VOID DrawDude( struct FormationWindow *fmw, struct Dude *dude )
{
	LONG x, y, start, end;
	struct RastPort *rp;

	rp = fmw->fmw_sw.sw_Window->RPort;

	if( dude == fmw->fmw_SelectedDude )
		SetAPen( fmw->fmw_sw.sw_Window->RPort, 2 );
	else
	{
		if( dude->du_Tagged )
				SetAPen( fmw->fmw_sw.sw_Window->RPort, 3 );
		else
			SetAPen( fmw->fmw_sw.sw_Window->RPort, 1 );
	}

	/* ENFORCER */
	x = dude->du_XPos - fmw->fmw_ViewXPos;
	y = dude->du_YPos - fmw->fmw_ViewYPos;

	/* Horizontal line */

	if( y >= 0 && y < fmw->fmw_ViewHeight )
	{
		start = x - MARKERSIZE;
		end = x + MARKERSIZE;
		if( start<0 ) start=0;
		if( end >= fmw->fmw_ViewWidth ) end = fmw->fmw_ViewWidth-1;
		if( start < fmw->fmw_ViewWidth && end >= 0 )
		{
			Move( rp, start + fmw->fmw_ViewLeft, y + fmw->fmw_ViewTop );
			Draw( rp, end + fmw->fmw_ViewLeft, y + fmw->fmw_ViewTop );
		}
	}

	/* vertical line */

	if( x >= 0 && x < fmw->fmw_ViewWidth )
	{
		start = y - MARKERSIZE;
		end = y + MARKERSIZE;
		if( start<0 ) start=0;
		if( end >= fmw->fmw_ViewHeight ) end = fmw->fmw_ViewHeight-1;
		if( start < fmw->fmw_ViewHeight && end >= 0 )
		{
			Move( rp,x + fmw->fmw_ViewLeft, start + fmw->fmw_ViewTop );
			Draw( rp,x + fmw->fmw_ViewLeft, end + fmw->fmw_ViewTop );
		}
	}
}


/********************************************************/

static VOID UnDrawDude( struct FormationWindow *fmw, struct Dude *dude )
{
	LONG x, y, start, end;
	struct RastPort *rp;

	rp = fmw->fmw_sw.sw_Window->RPort;

	SetAPen( fmw->fmw_sw.sw_Window->RPort, 0 );


	x = dude->du_XPos - fmw->fmw_ViewXPos;
	y = dude->du_YPos - fmw->fmw_ViewYPos;

	/* Horizontal line */

	if( y >= 0 && y < fmw->fmw_ViewHeight )
	{
		start = x - MARKERSIZE;
		end = x + MARKERSIZE;
		if( start<0 ) start=0;
		if( end >= fmw->fmw_ViewWidth ) end = fmw->fmw_ViewWidth-1;
		if( start < fmw->fmw_ViewWidth && end >= 0 )
		{
			Move( rp, start + fmw->fmw_ViewLeft, y + fmw->fmw_ViewTop );
			Draw( rp, end + fmw->fmw_ViewLeft, y + fmw->fmw_ViewTop );
		}
	}

	/* vertical line */

	if( x >= 0 && x < fmw->fmw_ViewWidth )
	{
		start = y - MARKERSIZE;
		end = y + MARKERSIZE;
		if( start<0 ) start=0;
		if( end >= fmw->fmw_ViewHeight ) end = fmw->fmw_ViewHeight-1;
		if( start < fmw->fmw_ViewHeight && end >= 0 )
		{
			Move( rp,x + fmw->fmw_ViewLeft, start + fmw->fmw_ViewTop );
			Draw( rp,x + fmw->fmw_ViewLeft, end + fmw->fmw_ViewTop );
		}
	}
}



/********************************************************/

static VOID DrawAxes( struct FormationWindow *fmw )
{
	LONG x,y;
	struct RastPort *rp;

	x = 0 - fmw->fmw_ViewXPos;
	y = 0 - fmw->fmw_ViewYPos;
	rp = fmw->fmw_sw.sw_Window->RPort;

	SetAPen( rp, 2 );
	SetDrMd( rp, JAM1 );
	SetDrPt( rp, 0x00FF );

	if( x>=0 && x<fmw->fmw_ViewWidth )
	{
		x += fmw->fmw_ViewLeft;
		Move( rp, x, fmw->fmw_ViewTop );
		Draw( rp, x, fmw->fmw_ViewTop + fmw->fmw_ViewHeight );
	}
	if( y>=0 && y<fmw->fmw_ViewHeight )
	{
		y += fmw->fmw_ViewTop;
		Move( rp, fmw->fmw_ViewLeft, y );
		Draw( rp, fmw->fmw_ViewLeft + fmw->fmw_ViewWidth, y );
	}
	SetDrPt( rp, 0xFFFF );

}

/********************************************************/

static VOID RedrawView( struct FormationWindow *fmw )
{
	struct Dude *dude;

	ClearView( fmw );

	if( fmw->fmw_CurrentFormation )
	{
		DrawAxes( fmw );

		SetDrMd( fmw->fmw_sw.sw_Window->RPort, JAM1 );

		for( dude = (struct Dude *)fmw->fmw_CurrentFormation->fm_DudeList.mlh_Head;
			dude->du_Node.mln_Succ;
			dude = (struct Dude *)dude->du_Node.mln_Succ )
		{
			DrawDude( fmw, dude );
		}
	}
}

/********************************************************/

static VOID DrawTaggedDudes( struct FormationWindow *fmw )
{
	struct Dude *dude;

	if( fmw->fmw_CurrentFormation )
	{
		for( dude = (struct Dude *)fmw->fmw_CurrentFormation->fm_DudeList.mlh_Head;
			dude->du_Node.mln_Succ;
			dude = (struct Dude *)dude->du_Node.mln_Succ )
		{
			if( dude->du_Tagged )
				DrawDude( fmw, dude );
		}
	}
}

/********************************************************/


static VOID SetScrollers( struct FormationWindow *fmw )
{
	ULONG total,visible,top;
	LONG l,t,r,b;

	/* do hscroller */

	l = fmw->fmw_ViewXPos;
	t = fmw->fmw_ViewYPos;
	r = l + fmw->fmw_ViewWidth;
	b = t + fmw->fmw_ViewHeight;

	total = max( r, fmw->fmw_SpaceRight ) -
		min( l, fmw->fmw_SpaceLeft );
	visible = fmw->fmw_ViewWidth;
	top = l - min( l, fmw->fmw_SpaceLeft );

	fmw->fmw_TotalLeft = min( l, fmw->fmw_SpaceLeft );
	fmw->fmw_TotalRight = max( r, fmw->fmw_SpaceRight );

	fmw->fmw_Layout[ GAD_HSCROLLER ].ho_Attr0 = visible/SCROLLERSCALE;
	fmw->fmw_Layout[ GAD_HSCROLLER ].ho_Attr1 = total/SCROLLERSCALE;
	fmw->fmw_Layout[ GAD_HSCROLLER ].ho_Value = top/SCROLLERSCALE;

	/* do vscroller */
	total = max( b, fmw->fmw_SpaceBottom ) -
		min( t, fmw->fmw_SpaceTop );
	visible = fmw->fmw_ViewHeight;
	top = t - min( t, fmw->fmw_SpaceTop );

	fmw->fmw_TotalTop = min( t, fmw->fmw_SpaceTop );
	fmw->fmw_TotalBottom = max( b, fmw->fmw_SpaceBottom );

	fmw->fmw_Layout[ GAD_VSCROLLER ].ho_Attr0 = visible/SCROLLERSCALE;
	fmw->fmw_Layout[ GAD_VSCROLLER ].ho_Attr1 = total/SCROLLERSCALE;
	fmw->fmw_Layout[ GAD_VSCROLLER ].ho_Value = top/SCROLLERSCALE;

	/* refresh 'em */

	HO_RefreshObject( &fmw->fmw_Layout[ GAD_HSCROLLER ] ,fmw->fmw_sw.sw_hos );
	HO_RefreshObject( &fmw->fmw_Layout[ GAD_VSCROLLER ] ,fmw->fmw_sw.sw_hos );

}
/********************************************************/

static VOID ReadVScroller( struct FormationWindow *fmw )
{
	LONG visible,total,top;

	visible  = fmw->fmw_Layout[ GAD_VSCROLLER ].ho_Attr0 * SCROLLERSCALE;
	total = fmw->fmw_Layout[ GAD_VSCROLLER ].ho_Attr1 * SCROLLERSCALE;
	top = fmw->fmw_Layout[ GAD_VSCROLLER ].ho_Value * SCROLLERSCALE;

	fmw->fmw_ViewYPos = fmw->fmw_TotalTop + top;
	RedrawView( fmw );
	SetScrollers(fmw);
}

/********************************************************/

static VOID ReadHScroller( struct FormationWindow *fmw )
{
	LONG visible,total,top;

	visible  = fmw->fmw_Layout[ GAD_HSCROLLER ].ho_Attr0 * SCROLLERSCALE;
	total = fmw->fmw_Layout[ GAD_HSCROLLER ].ho_Attr1 * SCROLLERSCALE;
	top = fmw->fmw_Layout[ GAD_HSCROLLER ].ho_Value * SCROLLERSCALE;

	fmw->fmw_ViewXPos = fmw->fmw_TotalLeft + top;
	RedrawView( fmw );
	SetScrollers(fmw);
}


/********************************************************/

static BOOL fmw_AddDude( struct FormationWindow *fmw, LONG mousex, LONG mousey )
{
	BOOL success = FALSE;
	struct Dude *dude;
	struct Formation *fm;

	if( fm = fmw->fmw_CurrentFormation )
	{
		dude = AllocVec( sizeof( struct Dude ),MEMF_ANY|MEMF_CLEAR );
		if( dude )
		{
			fmw_UnTagAllDudes( fmw );
			strcpy( dude->du_Program, "PROG" );
			dude->du_XPos = (LONG)mousex + fmw->fmw_ViewXPos - fmw->fmw_ViewLeft;
			dude->du_YPos = (LONG)mousey + fmw->fmw_ViewYPos - fmw->fmw_ViewTop;
			dude->du_Tagged = TRUE;
			fmw_SelectDude( fmw, dude );
			AddTail( (struct List *)&fm->fm_DudeList, (struct Node *)&dude->du_Node );
			fmw->fmw_DragFlag = TRUE;
			fmw->fmw_PrevX = mousex;
			fmw->fmw_PrevY = mousey;
			success = TRUE;
		}
		RedrawView( fmw );
	}
	return(success );
}

/********************************************************/

static VOID CalcSpaceEdges( struct FormationWindow *fmw )
{
	struct Dude *dude;
	struct Formation *fm;
	LONG l,r,t,b;

	b = r = -MAXINT;
	t = l = MAXINT;

	if( fm = fmw->fmw_CurrentFormation )
	{
		for( dude = (struct Dude *)fm->fm_DudeList.mlh_Head;
			dude->du_Node.mln_Succ;
			dude = (struct Dude *)dude->du_Node.mln_Succ )
		{
			if( dude->du_XPos - MARKERSIZE < l )
				l = dude->du_XPos - MARKERSIZE;
			if( dude->du_XPos + MARKERSIZE > r )
				r = dude->du_XPos + MARKERSIZE;
			if( dude->du_YPos - MARKERSIZE < t )
				t = dude->du_YPos - MARKERSIZE;
			if( dude->du_YPos + MARKERSIZE > b )
				b = dude->du_YPos + MARKERSIZE;
		}
	}
	fmw->fmw_SpaceLeft = l;
	fmw->fmw_SpaceRight = r;
	fmw->fmw_SpaceTop = t;
	fmw->fmw_SpaceBottom = b;
}


/********************************************************/

static BOOL fmw_AddFormation( struct FormationWindow *fmw )
{
	struct Formation *fm;
	struct Chunk *cnk;
	BOOL success;
	UBYTE name[ FORMATIONNAMESIZE ];

	if( BlockAllSuperWindows() )
	{
		Mystrncpy( name, defname, FORMATIONNAMESIZE-1 );
		if( MyStringRequest( fmw->fmw_sw.sw_Window, "New Formation", "Name:",
			name, FORMATIONNAMESIZE ) )
		{
			cnk = fmw->fmw_Chunk;
			if( fm = CreateFormation( name ) )
			{
				AddTail( &cnk->ch_DataList, &fm->fm_Node );
				SetCurrentFormation( fmw, fm );
				success = TRUE;
			}
		}
		UnblockAllSuperWindows();
	}
	return( success );
}

/********************************************************/

static VOID fmw_DeleteFormation( struct FormationWindow *fmw )
{
	struct Formation *fm, *otherfm;

	if( fm = fmw->fmw_CurrentFormation )
	{
		Remove( &fm->fm_Node );
		FreeFormation( fm );

		if( !IsListEmpty( &fmw->fmw_Chunk->ch_DataList ) )
			otherfm = (struct Formation *)fmw->fmw_Chunk->ch_DataList.lh_Head;
		else
			otherfm = NULL;

		SetCurrentFormation( fmw, otherfm );
	}
}

/********************************************************/

static VOID SetCurrentFormation( struct FormationWindow *fmw,
	struct Formation *fm )
{

	fmw_SelectDude( fmw, NULL );
	if( fm && fm->fm_Node.ln_Succ )
	{
		fmw->fmw_CurrentFormation = fm;
		fmw->fmw_Layout[ GAD_FORMATIONNAME ].ho_Attr0 = (ULONG)fm->fm_Name;
		HO_RefreshObject( &fmw->fmw_Layout[ GAD_FORMATIONNAME ], fmw->fmw_sw.sw_hos );
		RedrawView( fmw );
	}
	else
	{
		fmw->fmw_CurrentFormation = NULL;
		fmw->fmw_Layout[ GAD_FORMATIONNAME ].ho_Attr0 = NULL;
		HO_RefreshObject( &fmw->fmw_Layout[ GAD_FORMATIONNAME ], fmw->fmw_sw.sw_hos );
		ClearView( fmw );
		/* set disable flags here */
	}
}

/********************************************************/

static VOID fmw_SelectFormation( struct FormationWindow *fmw )
{
	struct Formation *fm;

	if( BlockAllSuperWindows() )
	{
		if( fm = MyFormationRequest( fmw->fmw_sw.sw_Window,fmw->fmw_Chunk ) )
		{
			SetCurrentFormation( fmw, fm );
		}
		UnblockAllSuperWindows();
	}
}


/********************************************************/
static VOID fmw_HandleMouseButtons( struct FormationWindow *fmw,
	UWORD code, WORD mousex, WORD mousey )
{
	switch( code )
	{
		case SELECTDOWN:
			if( ( mousex >= fmw->fmw_ViewLeft ) &&
				( mousex < fmw->fmw_ViewLeft + fmw->fmw_ViewWidth ) &&
				( mousey >= fmw->fmw_ViewTop ) &&
				( mousey < fmw->fmw_ViewTop + fmw->fmw_ViewHeight ) )
			{
				if( !fmw->fmw_MultiPick )
					fmw_UnTagAllDudes( fmw );
				fmw_PickDude( fmw, mousex, mousey );
				fmw->fmw_DragFlag = TRUE;
				fmw->fmw_PrevX = mousex;
				fmw->fmw_PrevY = mousey;
				break;
			}
		case SELECTUP:
			fmw->fmw_DragFlag = FALSE;
			CalcSpaceEdges( fmw );
			RedrawView( fmw );
			SetScrollers( fmw );
			break;
	}
}

/********************************************************/
static VOID fmw_HandleMouseMove( struct FormationWindow *fmw,
	UWORD code, WORD mousex, WORD mousey )
{
	struct Dude *dude;

	if( fmw->fmw_DragFlag && fmw->fmw_CurrentFormation )
	{
		ModifyChunk( fmw->fmw_Chunk );

		for( dude = (struct Dude *)fmw->fmw_CurrentFormation->fm_DudeList.mlh_Head;
			dude->du_Node.mln_Succ;
			dude = (struct Dude *)dude->du_Node.mln_Succ )
		{
			if( dude->du_Tagged )
				UnDrawDude( fmw, dude );
		}
		fmw_MoveTaggedDudes( fmw, mousex - fmw->fmw_PrevX, mousey - fmw->fmw_PrevY );
		for( dude = (struct Dude *)fmw->fmw_CurrentFormation->fm_DudeList.mlh_Head;
			dude->du_Node.mln_Succ;
			dude = (struct Dude *)dude->du_Node.mln_Succ )
		{
			if( dude->du_Tagged )
				DrawDude( fmw, dude );
		}
		fmw->fmw_PrevX = mousex;
		fmw->fmw_PrevY = mousey;
	}
}


/********************************************************/
static VOID fmw_SelectDude( struct FormationWindow *fmw, struct Dude *dude )
{
//	KPrintF("SelectDude\n");

	/* show old dude unselected */
	if( fmw->fmw_SelectedDude )
	{
		DrawDude( fmw, fmw->fmw_SelectedDude );
		fmw->fmw_SelectedDude = NULL;
	}

	fmw->fmw_SelectedDude = dude;
	if( dude )
	{
		DrawDude( fmw, dude );

		fmw->fmw_Layout[ GAD_PROGSTRING ].ho_Attr0 = (ULONG)dude->du_Program;
		fmw->fmw_Layout[ GAD_PROGSTRING ].ho_Attr1 = PROGNAMESIZE;
		fmw->fmw_Layout[ GAD_XPOSINT ].ho_Value = (LONG)dude->du_XPos;
		fmw->fmw_Layout[ GAD_YPOSINT ].ho_Value = (LONG)dude->du_YPos;
		fmw->fmw_Layout[ GAD_DELAYINT ].ho_Value = (ULONG)dude->du_Delay;
		HO_RefreshObject( &fmw->fmw_Layout[ GAD_SELECTGROUP ], fmw->fmw_sw.sw_hos );
		HO_DisableObject( &fmw->fmw_Layout[ GAD_SELECTGROUP ], fmw->fmw_sw.sw_hos, FALSE );
	}
	else
	{
		fmw->fmw_Layout[ GAD_PROGSTRING ].ho_Attr0 = NULL;
		fmw->fmw_Layout[ GAD_PROGSTRING ].ho_Attr1 = 0;
		fmw->fmw_Layout[ GAD_XPOSINT ].ho_Value = 0;
		fmw->fmw_Layout[ GAD_YPOSINT ].ho_Value = 0;
		fmw->fmw_Layout[ GAD_DELAYINT ].ho_Value = 0;
		HO_RefreshObject( &fmw->fmw_Layout[ GAD_SELECTGROUP ], fmw->fmw_sw.sw_hos );
		HO_DisableObject( &fmw->fmw_Layout[ GAD_SELECTGROUP ], fmw->fmw_sw.sw_hos, TRUE );
	}
}


/********************************************************/
static VOID fmw_PickDude( struct FormationWindow *fmw, WORD mousex, WORD mousey )
{
	struct Formation *fm;
	struct Dude *dude, *otherdude = NULL;
	LONG spacex,spacey;
	BOOL done = FALSE;

//	KPrintF("PickDude\n");

	if( fm = fmw->fmw_CurrentFormation )
	{

		spacex = mousex + fmw->fmw_ViewXPos - fmw->fmw_ViewLeft;
		spacey = mousey + fmw->fmw_ViewYPos - fmw->fmw_ViewTop;

		SetAPen( fmw->fmw_sw.sw_Window->RPort, 2 );

		for( dude = (struct Dude *)fm->fm_DudeList.mlh_Head;
			dude->du_Node.mln_Succ && !done;
			dude = (struct Dude *)dude->du_Node.mln_Succ )
		{
			if( ( spacex > ( dude->du_XPos-MARKERSIZE ) ) &&
				( spacex < ( dude->du_XPos+MARKERSIZE ) ) &&
				( spacey > ( dude->du_YPos-MARKERSIZE ) ) &&
				( spacey < ( dude->du_YPos+MARKERSIZE ) ) )
			{
				dude->du_Tagged = TRUE;
				otherdude = dude;
				if( !fmw->fmw_MultiPick )
					done = TRUE;
				else
					DrawDude( fmw, dude );
			}
		}
	}
	if( otherdude ) fmw_SelectDude( fmw, otherdude );
}


/********************************************************/
static VOID fmw_UnTagAllDudes( struct FormationWindow *fmw )
{
	struct Dude *dude;
	struct Formation *fm;

//	KPrintF("UnTagAllDudes\n");

	fmw_SelectDude( fmw, NULL );
	if( fm = fmw->fmw_CurrentFormation )
	{
		for( dude = (struct Dude *)fm->fm_DudeList.mlh_Head;
			dude->du_Node.mln_Succ;
			dude = (struct Dude *)dude->du_Node.mln_Succ )
		{
			if( dude->du_Tagged )
			{
				dude->du_Tagged = FALSE;
				DrawDude( fmw, dude );
			}
		}
	}
}


/********************************************************/
static VOID fmw_MoveTaggedDudes( struct FormationWindow *fmw,
	LONG xdelta, LONG ydelta )
{
	struct Formation *fm;
	struct Dude *dude;

	if( fm = fmw->fmw_CurrentFormation )
	{
		for( dude = (struct Dude *)fm->fm_DudeList.mlh_Head;
			dude->du_Node.mln_Succ;
			dude = (struct Dude *)dude->du_Node.mln_Succ )
		{
			if( dude->du_Tagged )
			{
				dude->du_XPos += xdelta;
				dude->du_YPos += ydelta;
				if( dude == fmw->fmw_SelectedDude )
				{
					fmw->fmw_Layout[ GAD_XPOSINT ].ho_Value = dude->du_XPos;
					fmw->fmw_Layout[ GAD_YPOSINT ].ho_Value = dude->du_YPos;
					HO_RefreshObject( &fmw->fmw_Layout[ GAD_XPOSINT ],
						fmw->fmw_sw.sw_hos );
					HO_RefreshObject( &fmw->fmw_Layout[ GAD_YPOSINT ],
						fmw->fmw_sw.sw_hos );
				}
			}
		}
	}
}

/********************************************************/
static VOID fmw_DeleteDudes( struct FormationWindow *fmw )
{
	struct Formation *fm;
	struct Dude *dude, *nextdude;

	if( fm = fmw->fmw_CurrentFormation )
	{
		dude = (struct Dude *)fm->fm_DudeList.mlh_Head;
		while( dude->du_Node.mln_Succ )
		{
			nextdude = (struct Dude *)dude->du_Node.mln_Succ;
			if( dude->du_Tagged )
			{
				Remove( (struct Node *)&dude->du_Node );
				FreeVec( dude );
			}
			dude = nextdude;
		}
		fmw_SelectDude( fmw, NULL );
		RedrawView( fmw );
	}
}


static VOID UpdateSelectedDudeParams( struct FormationWindow *fmw )
{
	struct Dude *dude;

	if( dude = fmw->fmw_SelectedDude )
	{
		HO_ReadGadgetStates( fmw->fmw_sw.sw_hos,
			&fmw->fmw_Layout[ GAD_SELECTGROUP ] );
		/* name automagically updated */
		if( MakePackable( dude->du_Program ) )
			DisplayBeep( fmw->fmw_sw.sw_Window->WScreen );
		HO_RefreshObject( &fmw->fmw_Layout[ GAD_PROGSTRING ], fmw->fmw_sw.sw_hos );
		dude->du_XPos = fmw->fmw_Layout[ GAD_XPOSINT ].ho_Value;
		dude->du_YPos = fmw->fmw_Layout[ GAD_YPOSINT ].ho_Value;
		dude->du_Delay = fmw->fmw_Layout[ GAD_DELAYINT ].ho_Value;
	}
}
