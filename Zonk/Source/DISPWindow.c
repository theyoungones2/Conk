/********************************************************/
//
// DISPWindow.c
//
/********************************************************/

#define DISPWINDOW_C

#include <stdio.h>
#include <stdlib.h>
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

#define NUMOFPAGES 4
#define PALETTEINFOSIZE 12	/* "xxx Colours" */

struct DISPWindow
{
	struct SuperWindow	dw_sw;
	struct Chunk				*dw_Chunk;
	struct HoopyObject	*dw_Layout[NUMOFPAGES];
	int									dw_Page;

	/* Page 0 buffers (gfx) */
	UBYTE								dw_PaletteInfoBuf[PALETTEINFOSIZE];
};

#define GID_BORING 0

#define GID_PAGESWITCH 2

static UBYTE *pagelabels[] = {
		(UBYTE *)"Graphics (1/4)",
		(UBYTE *)"Players (2/4)",
		(UBYTE *)"Misc (3/4)",
		(UBYTE *)"DefaultBorders (4/4)",
		NULL,
		(UBYTE *)"StatBar",
		NULL };

#define G0_MAP	6
#define G0_PICKMAP	7
#define G0_BLKS	9
#define G0_PICKBLKS	10
#define G0_BACKDROP	12
#define G0_PICKBACKDROP	13
#define G0_PALETTE	15
#define G0_PICKPALETTE	16
#define G0_IMPORTPALETTE	19
#define G0_OK 22

static struct HoopyObject gadlayout0[] =
{
	{ HOTYPE_VGROUP, 100, HOFLG_RAISEDBORDER,4,0,0,0, NULL, 0,GID_BORING+42,0,0,0 },
		{ HOTYPE_HGROUP, 100, 0,2,0,0,0, NULL, 0,GID_BORING+43,0,0,0 },
			{ HOTYPE_CYCLE, 100, 0, (ULONG)pagelabels,0,0,0, "Page:",0,GID_PAGESWITCH,PLACETEXT_LEFT,0,0 },
			{ HOTYPE_SPACE, 100, HOFLG_FREEWIDTH, 0,0,0,0, NULL,0,GID_BORING+44,0,0,0 },
		{ HOTYPE_VGROUP, 100, HOFLG_RAISEDBORDER,5,0,0,0, NULL, 0,GID_BORING+45,0,0,0 },
			{ HOTYPE_HGROUP, 100, 0,2,0,0,0, NULL, 0,GID_BORING+46,0,0,0 },
				{ HOTYPE_STRING, 100, HOFLG_NOBORDER|HOFLG_FREEWIDTH, 0,MAPNAMESIZE-1,0,0, "Map",0,G0_MAP,PLACETEXT_LEFT,0 },
				{ HOTYPE_BUTTON, 100, HOFLG_NOBORDER, 0,0,0,0, "?",0,G0_PICKMAP,PLACETEXT_IN,0,0 },
			{ HOTYPE_HGROUP, 100, 0,2,0,0,0, NULL, 0,GID_BORING+47,0,0,0 },
				{ HOTYPE_STRING, 100, HOFLG_NOBORDER|HOFLG_FREEWIDTH, 0,BLOCKSETNAMESIZE-1,0,0, "BlockSet",0,G0_BLKS,PLACETEXT_LEFT,0 },
				{ HOTYPE_BUTTON, 100, HOFLG_NOBORDER, 0,0,0,0, "?",0,G0_PICKBLKS,PLACETEXT_IN,0,0 },
			{ HOTYPE_HGROUP, 100, 0,2,0,0,0, NULL, 0,GID_BORING+48,0,0,0 },
				{ HOTYPE_STRING, 100, HOFLG_NOBORDER|HOFLG_FREEWIDTH, 0,GENERICNAMESIZE-1,0,0, "BackDropPic",0,G0_BACKDROP,PLACETEXT_LEFT,0 },
				{ HOTYPE_BUTTON, 100, HOFLG_NOBORDER, 0,0,0,0, "?",0,G0_PICKBACKDROP,PLACETEXT_IN,0,0 },
			{ HOTYPE_HGROUP, 100, 0,2,0,0,0, NULL, 0,GID_BORING+49,0,0,0 },
				{ HOTYPE_TEXT, 100, HOFLG_NOBORDER|HOFLG_FREEWIDTH, 0,PALETTEINFOSIZE-1,0,0, "Palette",0,G0_PALETTE,PLACETEXT_LEFT,0 },
				{ HOTYPE_BUTTON, 100, HOFLG_NOBORDER, 0,0,0,0, "?",0,G0_PICKPALETTE,PLACETEXT_IN,0,0 },
			{ HOTYPE_HGROUP, 100, 0,2,0,0,0, NULL, 0,GID_BORING+50,0,0,0 },
				{ HOTYPE_SPACE, 100, HOFLG_NOBORDER|HOFLG_FREEWIDTH, 0,0,0,0, NULL,0,GID_BORING+51,PLACETEXT_LEFT,0 },
				{ HOTYPE_BUTTON, 100, HOFLG_NOBORDER, 0,0,0,0, "Import Palette...",0,G0_IMPORTPALETTE,PLACETEXT_IN,0,0 },
		{ HOTYPE_SPACE, 100, HOFLG_NOBORDER|HOFLG_FREEWIDTH|HOFLG_FREEHEIGHT, 0,0,0,0, NULL,0,GID_BORING+52,0,0 },
		{ HOTYPE_HGROUP, 100, 0,2,0,0,0, NULL, 0,GID_BORING+53,0,0,0 },
			{ HOTYPE_BUTTON, 100, HOFLG_NOBORDER|HOFLG_FREEWIDTH, 0,0,0,0, "OK",0,G0_OK,0,0,0 },
			{ HOTYPE_SPACE, 200, HOFLG_NOBORDER|HOFLG_FREEWIDTH, 0,0,0,0, NULL,0,GID_BORING+54,PLACETEXT_LEFT,0 },
	{HOTYPE_END}
};

#define G1_P1INIT				6
#define G1_PICKP1INIT		7
#define G1_P2INIT				9
#define G1_PICKP2INIT		10
#define G1_P3INIT				12
#define G1_PICKP3INIT		13
#define G1_P4INIT				15
#define G1_PICKP4INIT		16
#define G1_OK						19

static struct HoopyObject gadlayout1[] =
{
	{ HOTYPE_VGROUP, 100, HOFLG_RAISEDBORDER,4,0,0,0, NULL, 0,GID_BORING,0,0,0 },
		{ HOTYPE_HGROUP, 100, 0,2,0,0,0, NULL, 0,GID_BORING,0,0,0 },
			{ HOTYPE_CYCLE, 100, 0, (ULONG)pagelabels,0,0,0, "Page:",0,GID_PAGESWITCH,PLACETEXT_LEFT,0,0 },
			{ HOTYPE_SPACE, 100, HOFLG_FREEWIDTH, 0,0,0,0, NULL,0,GID_BORING,0,0,0 },
		{ HOTYPE_VGROUP, 100, HOFLG_RAISEDBORDER,4,0,0,0, NULL, 0,GID_BORING,0,0,0 },
			{ HOTYPE_HGROUP, 100, 0,2,0,0,0, NULL, 0,GID_BORING,0,0,0 },
				{ HOTYPE_STRING, 100, HOFLG_NOBORDER|HOFLG_FREEWIDTH, 0,PROGNAMESIZE-1,0,0, "Player0Init",0,G1_P1INIT,PLACETEXT_LEFT,0 },
				{ HOTYPE_BUTTON, 100, HOFLG_NOBORDER, 0,0,0,0, "?",0,G1_PICKP1INIT,PLACETEXT_IN,0,0 },
			{ HOTYPE_HGROUP, 100, 0,2,0,0,0, NULL, 0,GID_BORING,0,0,0 },
				{ HOTYPE_STRING, 100, HOFLG_NOBORDER|HOFLG_FREEWIDTH, 0,PROGNAMESIZE-1,0,0, "Player1Init",0,G1_P2INIT,PLACETEXT_LEFT,0 },
				{ HOTYPE_BUTTON, 100, HOFLG_NOBORDER, 0,0,0,0, "?",0,G1_PICKP2INIT,PLACETEXT_IN,0,0 },
			{ HOTYPE_HGROUP, 100, 0,2,0,0,0, NULL, 0,GID_BORING,0,0,0 },
				{ HOTYPE_STRING, 100, HOFLG_NOBORDER|HOFLG_FREEWIDTH, 0,PROGNAMESIZE-1,0,0, "Player2Init",0,G1_P3INIT,PLACETEXT_LEFT,0 },
				{ HOTYPE_BUTTON, 100, HOFLG_NOBORDER, 0,0,0,0, "?",0,G1_PICKP3INIT,PLACETEXT_IN,0,0 },
			{ HOTYPE_HGROUP, 100, 0,2,0,0,0, NULL, 0,GID_BORING,0,0,0 },
				{ HOTYPE_STRING, 100, HOFLG_NOBORDER|HOFLG_FREEWIDTH, 0,PROGNAMESIZE-1,0,0, "Player3Init",0,G1_P4INIT,PLACETEXT_LEFT,0 },
				{ HOTYPE_BUTTON, 100, HOFLG_NOBORDER, 0,0,0,0, "?",0,G1_PICKP4INIT,PLACETEXT_IN,0,0 },
		{ HOTYPE_SPACE, 100, HOFLG_NOBORDER|HOFLG_FREEWIDTH|HOFLG_FREEHEIGHT, 0,0,0,0, NULL,0,GID_BORING,0,0 },
		{ HOTYPE_HGROUP, 100, 0,2,0,0,0, NULL, 0,GID_BORING,0,0,0 },
			{ HOTYPE_BUTTON, 100, HOFLG_NOBORDER|HOFLG_FREEWIDTH, 0,0,0,0, "OK",0,G1_OK,0,0,0 },
			{ HOTYPE_SPACE, 200, HOFLG_NOBORDER|HOFLG_FREEWIDTH, 0,0,0,0, NULL,0,GID_BORING,PLACETEXT_LEFT,0 },
	{HOTYPE_END}
};


static UBYTE *frameratelabels[] = {
		(UBYTE *)"Full (50 Frames/sec)",
		(UBYTE *)"Half (25 Frames/sec)",
		NULL };

#define G2_AGA								5
#define G2_FRAMERATE					6
#define G2_GAMEINIT						8
#define G2_PICKGAMEINIT				9
#define G2_GAMEUPDATE					11
#define G2_PICKGAMEUPDATE			12
#define G2_NUMOFPLAYEROBJS		13
#define G2_NUMOFBULLETOBJS		14
#define G2_NUMOFDUDEOBJS			15
#define G2_STATBOXPIC					17
#define G2_PICKSTATBOXPIC			18
#define G2_DRAWBARPIC					20
#define G2_PICKDRAWBARPIC			21
#define G2_OK									24

static struct HoopyObject gadlayout2[] =
{
	{ HOTYPE_VGROUP, 100, HOFLG_RAISEDBORDER,4,0,0,0, NULL, 0,GID_BORING,0,0,0 },
		{ HOTYPE_HGROUP, 100, 0,2,0,0,0, NULL, 0,GID_BORING,0,0,0 },
			{ HOTYPE_CYCLE, 100, 0, (ULONG)pagelabels,0,0,0, "Page:",0,GID_PAGESWITCH,PLACETEXT_LEFT,0,0 },
			{ HOTYPE_SPACE, 100, HOFLG_FREEWIDTH, 0,0,0,0, NULL,0,GID_BORING,0,0,0 },
		{ HOTYPE_VGROUP, 100, HOFLG_RAISEDBORDER,9,0,0,0, NULL, 0,GID_BORING,0,0,0 },
			{ HOTYPE_CHECKBOX, 100, 0, 0,0,0,0, "AGA Only?",0,G2_AGA,PLACETEXT_LEFT,0,0 },
			{ HOTYPE_CYCLE, 100, 0, (ULONG)frameratelabels,0,0,0, "FrameRate",0,G2_FRAMERATE,PLACETEXT_LEFT,0 },
			{ HOTYPE_HGROUP, 100, 0,2,0,0,0, NULL, 0,GID_BORING,0,0,0 },
				{ HOTYPE_STRING, 100, HOFLG_NOBORDER|HOFLG_FREEWIDTH, 0,PROGNAMESIZE-1,0,0, "LevelInit",0,G2_GAMEINIT,PLACETEXT_LEFT,0 },
				{ HOTYPE_BUTTON, 100, HOFLG_NOBORDER, 0,0,0,0, "?",0,G2_PICKGAMEINIT,PLACETEXT_IN,0,0 },
			{ HOTYPE_HGROUP, 100, 0,2,0,0,0, NULL, 0,GID_BORING,0,0,0 },
				{ HOTYPE_STRING, 100, HOFLG_NOBORDER|HOFLG_FREEWIDTH, 0,PROGNAMESIZE-1,0,0, "LevelUpdate",0,G2_GAMEUPDATE,PLACETEXT_LEFT,0 },
				{ HOTYPE_BUTTON, 100, HOFLG_NOBORDER, 0,0,0,0, "?",0,G2_PICKGAMEUPDATE,PLACETEXT_IN,0,0 },
			{ HOTYPE_INTEGER, 100, 0, 5,0,0,0, "NumOfPlayerObjs",0,G2_NUMOFPLAYEROBJS,PLACETEXT_LEFT,0 },
			{ HOTYPE_INTEGER, 100, 0, 5,0,0,0, "NumOfBulletObjs",0,G2_NUMOFBULLETOBJS,PLACETEXT_LEFT,0 },
			{ HOTYPE_INTEGER, 100, 0, 5,0,0,0, "NumOfBadDudeObjs",0,G2_NUMOFDUDEOBJS,PLACETEXT_LEFT,0 },
			{ HOTYPE_HGROUP, 100, 0,2,0,0,0, NULL, 0,GID_BORING,0,0,0 },
				{ HOTYPE_STRING, 100, HOFLG_NOBORDER|HOFLG_FREEWIDTH, 0,GENERICNAMESIZE-1,0,0, "StatBoxPic",0,G2_STATBOXPIC,PLACETEXT_LEFT,0 },
				{ HOTYPE_BUTTON, 100, HOFLG_NOBORDER, 0,0,0,0, "?",0,G2_PICKSTATBOXPIC,PLACETEXT_IN,0,0 },
			{ HOTYPE_HGROUP, 100, 0,2,0,0,0, NULL, 0,GID_BORING,0,0,0 },
				{ HOTYPE_STRING, 100, HOFLG_NOBORDER|HOFLG_FREEWIDTH, 0,GENERICNAMESIZE-1,0,0, "DrawBarPic",0,G2_DRAWBARPIC,PLACETEXT_LEFT,0 },
				{ HOTYPE_BUTTON, 100, HOFLG_NOBORDER, 0,0,0,0, "?",0,G2_PICKDRAWBARPIC,PLACETEXT_IN,0,0 },
		{ HOTYPE_SPACE, 100, HOFLG_NOBORDER|HOFLG_FREEWIDTH|HOFLG_FREEHEIGHT, 0,0,0,0, NULL,0,GID_BORING,0,0 },
		{ HOTYPE_HGROUP, 100, 0,2,0,0,0, NULL, 0,GID_BORING,0,0,0 },
			{ HOTYPE_BUTTON, 100, HOFLG_NOBORDER|HOFLG_FREEWIDTH, 0,0,0,0, "OK",0,G2_OK,0,0,0 },
			{ HOTYPE_SPACE, 200, HOFLG_NOBORDER|HOFLG_FREEWIDTH, 0,0,0,0, NULL,0,GID_BORING,PLACETEXT_LEFT,0 },
	{HOTYPE_END}
};


UBYTE *relativelabels[] = { "Map/Pic","View",NULL };
UBYTE *bordertypelabels[] = { "Off","Kill","Stop","Wrap","Bounce1","Bounce2",NULL };

#define G3_LEFTBORDER	6
#define G3_LEFTBORDERTYPE	7
#define G3_LEFTREL	10

#define G3_RIGHTBORDER	12
#define G3_RIGHTBORDERTYPE	13
#define G3_RIGHTREL	16

#define G3_TOPBORDER	18
#define G3_TOPBORDERTYPE	19
#define G3_TOPREL 22

#define G3_BOTTOMBORDER	24
#define G3_BOTTOMBORDERTYPE	25
#define G3_BOTTOMREL 28

#define G3_OK 31

static struct HoopyObject gadlayout3[] =
{
	{ HOTYPE_VGROUP, 100, HOFLG_RAISEDBORDER,4,0,0,0, NULL, 0,GID_BORING,0,0,0 },
		{ HOTYPE_HGROUP, 100, 0,2,0,0,0, NULL, 0,GID_BORING,0,0,0 },
			{ HOTYPE_CYCLE, 100, 0, (ULONG)pagelabels,0,0,0, "Page:",0,GID_PAGESWITCH,PLACETEXT_LEFT,0,0 },
			{ HOTYPE_SPACE, 100, HOFLG_FREEWIDTH, 0,0,0,0, NULL,0,GID_BORING,0,0,0 },
		{ HOTYPE_VGROUP, 100, HOFLG_RAISEDBORDER,8,0,0,0, NULL, 0,GID_BORING,0,0,0 },
			{ HOTYPE_HGROUP, 100, 0,2,0,0,0, NULL, 0,GID_BORING,0,0,0 },
				{ HOTYPE_FIXEDPOINT, 100, HOFLG_NOBORDER|HOFLG_FREEWIDTH, 12,0,0,0, "Left",0,G3_LEFTBORDER,PLACETEXT_LEFT,0,0 },
				{ HOTYPE_CYCLE, 100, HOFLG_NOBORDER, (LONG)&bordertypelabels,0,0,0, "",0,G3_LEFTBORDERTYPE,0,0,0 },
			{ HOTYPE_HGROUP, 100, 0,2,0,0,0, NULL, 0,GID_BORING,0,0,0 },
				{ HOTYPE_SPACE, 100, HOFLG_FREEWIDTH, 0,0,0,0, NULL,0,GID_BORING,0,0,0 },
				{ HOTYPE_CHECKBOX, 100, 0, 0,0,0,0, "ViewRelative?",0,G3_LEFTREL,PLACETEXT_LEFT,0,0 },		
			{ HOTYPE_HGROUP, 100, 0,2,0,0,0, NULL, 0,GID_BORING,0,0,0 },
				{ HOTYPE_FIXEDPOINT, 100, HOFLG_NOBORDER|HOFLG_FREEWIDTH, 12,0,0,0, "Right",0,G3_RIGHTBORDER,PLACETEXT_LEFT,0,0 },
				{ HOTYPE_CYCLE, 100, HOFLG_NOBORDER, (LONG)&bordertypelabels,0,0,0, "",0,G3_RIGHTBORDERTYPE,0,0,0 },
			{ HOTYPE_HGROUP, 100, 0,2,0,0,0, NULL, 0,GID_BORING,0,0,0 },
				{ HOTYPE_SPACE, 100, HOFLG_FREEWIDTH, 0,0,0,0, NULL,0,GID_BORING,0,0,0 },
				{ HOTYPE_CHECKBOX, 100, 0, 0,0,0,0, "ViewRelative?",0,G3_RIGHTREL,PLACETEXT_LEFT,0,0 },		
			{ HOTYPE_HGROUP, 100, 0,2,0,0,0, NULL, 0,GID_BORING,0,0,0 },
				{ HOTYPE_FIXEDPOINT, 100, HOFLG_NOBORDER|HOFLG_FREEWIDTH, 12,0,0,0, "Top",0,G3_TOPBORDER,PLACETEXT_LEFT,0,0 },
				{ HOTYPE_CYCLE, 100, HOFLG_NOBORDER, (LONG)&bordertypelabels,0,0,0, "",0,G3_TOPBORDERTYPE,0,0,0 },
			{ HOTYPE_HGROUP, 100, 0,2,0,0,0, NULL, 0,GID_BORING,0,0,0 },
				{ HOTYPE_SPACE, 100, HOFLG_FREEWIDTH, 0,0,0,0, NULL,0,GID_BORING,0,0,0 },
				{ HOTYPE_CHECKBOX, 100, 0, 0,0,0,0, "ViewRelative?",0,G3_TOPREL,PLACETEXT_LEFT,0,0 },		
			{ HOTYPE_HGROUP, 100, 0,2,0,0,0, NULL, 0,GID_BORING,0,0,0 },
				{ HOTYPE_FIXEDPOINT, 100, HOFLG_NOBORDER|HOFLG_FREEWIDTH, 12,0,0,0, "Bottom",0,G3_BOTTOMBORDER,PLACETEXT_LEFT,0,0 },
				{ HOTYPE_CYCLE, 100, HOFLG_NOBORDER, (LONG)&bordertypelabels,0,0,0, "",0,G3_BOTTOMBORDERTYPE,0,0,0 },
			{ HOTYPE_HGROUP, 100, 0,2,0,0,0, NULL, 0,GID_BORING,0,0,0 },
				{ HOTYPE_SPACE, 100, HOFLG_FREEWIDTH, 0,0,0,0, NULL,0,GID_BORING,0,0,0 },
				{ HOTYPE_CHECKBOX, 100, 0, 0,0,0,0, "ViewRelative?",0,G3_BOTTOMREL,PLACETEXT_LEFT,0,0 },		
		{ HOTYPE_SPACE, 100, HOFLG_NOBORDER|HOFLG_FREEWIDTH|HOFLG_FREEHEIGHT, 0,0,0,0, NULL,0,GID_BORING,0,0 },
		{ HOTYPE_HGROUP, 100, 0,2,0,0,0, NULL, 0,GID_BORING,0,0,0 },
			{ HOTYPE_BUTTON, 100, HOFLG_NOBORDER|HOFLG_FREEWIDTH, 0,0,0,0, "OK",0,G3_OK,0,0,0 },
			{ HOTYPE_SPACE, 200, HOFLG_NOBORDER|HOFLG_FREEWIDTH, 0,0,0,0, NULL,0,GID_BORING,PLACETEXT_LEFT,0 },
	{HOTYPE_END}
};


static void IDCMPHandler( struct SuperWindow *sw,
	struct IntuiMessage *imsg );
static void HideWindow( struct SuperWindow *sw );
static BOOL ShowWindow( struct SuperWindow *sw );
static void KillWindow( struct SuperWindow *sw );
static BOOL RefreshMenus( struct SuperWindow *sw );
static UBYTE *MakeDISPWinTitle( struct SuperWindow *sw );

static void NewPage( struct DISPWindow *dw, int page );
static void InitPageLayout( struct DISPWindow *dw, int page );
static BOOL	HandleGadgetUp( struct DISPWindow *dw, int gid );
static void PickAProgram( struct DISPWindow *dw, int gid );
static void PickPalette( struct DISPWindow *dw, int gid );
static VOID PickMap( struct DISPWindow *dw, int gid );
static VOID PickBlockset( struct DISPWindow *dw, int gid );
static void ImportPalette( struct DISPWindow *dw, int infogid );

/********************************************************/

BOOL OpenNewDISPWindow( struct Chunk *cnk )
{
	struct SuperWindow *sw;
	struct DISPWindow *dw;
	BOOL success = FALSE;

	assert( cnk != NULL );
	assert( cnk->ch_TypeID == ID_DISP );

	for( sw = (struct SuperWindow *)superwindows.lh_Head;
		sw->sw_Node.ln_Succ;
		sw = (struct SuperWindow *)sw->sw_Node.ln_Succ )
	{
		if( sw->sw_Type == SWTY_DISPLAY )
		{
			dw = (struct DISPWindow *)sw;
			if( dw->dw_Chunk == cnk )
			{
				if( sw->sw_Window )
				{
					WindowToFront( sw->sw_Window );
					ActivateWindow( sw->sw_Window );
					return TRUE;
				}
				else
					(*sw->sw_ShowWindow)( sw );
			}
		}
	}

	LockChunk( cnk );

	if( dw = AllocVec( sizeof( struct DISPWindow ), MEMF_ANY|MEMF_CLEAR ) )
	{
		sw = &dw->dw_sw;
		sw->sw_Handler = IDCMPHandler;
		sw->sw_HideWindow = HideWindow;
		sw->sw_ShowWindow = ShowWindow;
		sw->sw_BuggerOff = KillWindow;
		sw->sw_RefreshMenus = RefreshMenus;
		sw->sw_MakeTitle	= MakeDISPWinTitle;
		sw->sw_Type				= SWTY_DISPLAY;
		sw->sw_Window			= NULL;
		sw->sw_Gadgets		= NULL;
		sw->sw_VisualInfo	= NULL;
		sw->sw_WinDim			= zonkfig.SuperWindowDefs[ SWTY_DISPLAY ];

		dw->dw_Chunk = cnk;
		dw->dw_Page = 0;
		dw->dw_Layout[0] = AllocVec( sizeof( gadlayout0 ), MEMF_ANY );
		dw->dw_Layout[1] = AllocVec( sizeof( gadlayout1 ), MEMF_ANY );
		dw->dw_Layout[2] = AllocVec( sizeof( gadlayout2 ), MEMF_ANY );
		dw->dw_Layout[3] = AllocVec( sizeof( gadlayout3 ), MEMF_ANY );
		if( dw->dw_Layout[0] && dw->dw_Layout[1] &&
			dw->dw_Layout[2] && dw->dw_Layout[3] )
		{
			CopyMem( gadlayout0, dw->dw_Layout[0], sizeof( gadlayout0 ) );
			CopyMem( gadlayout1, dw->dw_Layout[1], sizeof( gadlayout1 ) );
			CopyMem( gadlayout2, dw->dw_Layout[2], sizeof( gadlayout2 ) );
			CopyMem( gadlayout3, dw->dw_Layout[3], sizeof( gadlayout3 ) );

			(*sw->sw_ShowWindow)( sw );
			AddTail( &superwindows, &sw->sw_Node );
			success = TRUE;
		}
		else
			FreeVec( dw );
	}
	return( success );
}

/********************************************************/

static BOOL ShowWindow( struct SuperWindow *sw )
{
	BOOL success = FALSE;
	struct DISPWindow *dw;
	struct Window *win;
	struct Screen *scr;

	dw = (struct DISPWindow *)sw;

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
			ModifyIDCMP( win, IDCMP_REFRESHWINDOW|IDCMP_NEWSIZE|IDCMP_CLOSEWINDOW|
				IDCMP_MENUPICK|IDCMP_CHANGEWINDOW|
				STRINGIDCMP|INTEGERIDCMP|SCROLLERIDCMP|BUTTONIDCMP );

			InitPageLayout( dw, dw->dw_Page );

			HO_SussWindowSizing( sw->sw_hos, dw->dw_Layout[dw->dw_Page] );

			HO_MakeLayout( dw->dw_Layout[ dw->dw_Page ], sw->sw_hos,
				win->BorderLeft, win->BorderTop,
				win->Width - win->BorderLeft - win->BorderRight,
				win->Height - win->BorderTop - win->BorderBottom );

			success = TRUE;
		}
	}

	if( !success )
	{
		if( sw->sw_Window )
			CloseWindow( sw->sw_Window );
		sw->sw_Window = NULL;
	}

	return( success );
}


/********************************************************/

static void HideWindow( struct SuperWindow *sw )
{
	struct DISPWindow *dw;

	dw = (struct DISPWindow *)sw;

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

/********************************************************/

static void KillWindow( struct SuperWindow *sw )
{
	int i;
	struct DISPWindow *dw;

	assert( sw != NULL );
	assert( sw->sw_Type == SWTY_DISPLAY );

	dw = (struct DISPWindow *)sw;

	if( sw && IsOKToClose( sw ) )
	{
		(*sw->sw_HideWindow)(sw);
		UnlockChunk( ((struct DISPWindow *)sw)->dw_Chunk );
		Remove( &sw->sw_Node );
		for( i=0; i<NUMOFPAGES; i++ )
		{
			FreeVec( dw->dw_Layout[i] );
			dw->dw_Layout[i] = NULL;
		}
		FreeVec( sw );
	}
}

/********************************************************/
static BOOL RefreshMenus( struct SuperWindow *sw )
{
	return( FALSE );
}


/********************************************************/

static UBYTE *MakeDISPWinTitle( struct SuperWindow *sw )
{
	struct Chunk *cnk;
	UBYTE *p;

	cnk = ((struct DISPWindow *)sw)->dw_Chunk;
	p = sw->sw_Name;

	if( cnk->ch_Modified )
		*p++ = MODCHAR;
	strcpy( p, "Display" );

	return( sw->sw_Name );
}



/********************************************************/

static void IDCMPHandler( struct SuperWindow *sw,
	struct IntuiMessage *origimsg )
{
	struct IntuiMessage *imsg;
	struct Window *win;
	struct DISPWindow *dw;

	ULONG class;
	UWORD code;
//	UWORD qualifier;
	APTR iaddress;
	WORD mousex, mousey;
	ULONG seconds, micros;

	dw = (struct DISPWindow *)sw;
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
				HO_ReadGadgetStates( dw->dw_sw.sw_hos,
					dw->dw_Layout[ dw->dw_Page ] );
				KillWindow( sw );
				break;
			case IDCMP_NEWSIZE:
				ClearWindow( win );
				HO_ReadGadgetStates( sw->sw_hos, dw->dw_Layout[dw->dw_Page] );
				HO_MakeLayout( dw->dw_Layout[dw->dw_Page], sw->sw_hos,
					win->BorderLeft, win->BorderTop,
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
				HO_ReadGadgetStates( dw->dw_sw.sw_hos,
					dw->dw_Layout[ dw->dw_Page ] );

				if( ((struct Gadget*)iaddress)->GadgetID == GID_PAGESWITCH )
				{
					NewPage( dw, code );
				}
				else
				{
					if( HandleGadgetUp( dw, ((struct Gadget*)iaddress)->GadgetID ) )
						KillWindow( sw );
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
static void NewPage( struct DISPWindow *dw, int page )
{
	struct Window *win;

	win = dw->dw_sw.sw_Window;
	assert( win != NULL );
	ClearWindow( win );
	dw->dw_Page = page;
	InitPageLayout( dw, dw->dw_Page );
	HO_SussWindowSizing( dw->dw_sw.sw_hos, dw->dw_Layout[dw->dw_Page] );
	HO_MakeLayout( dw->dw_Layout[dw->dw_Page], dw->dw_sw.sw_hos,
		win->BorderLeft, win->BorderTop,
		win->Width - win->BorderLeft - win->BorderRight,
		win->Height - win->BorderTop - win->BorderBottom );
}

/********************************************************/

static void InitPageLayout( struct DISPWindow *dw, int page )
{
	struct HoopyObject *ho;
	struct ZonkDisp *zdip;

	assert( dw != NULL );
	assert( dw->dw_Chunk != NULL );

	ho = dw->dw_Layout[page];
	zdip = (struct ZonkDisp *)dw->dw_Chunk->ch_Data;

	ho[ GID_PAGESWITCH ].ho_Value = page;

	switch( page )
	{
		case 0:	/* graphics page */
			ho[ G0_MAP ].ho_Attr0 = (LONG)zdip->zdp_Map;
			ho[ G0_BLKS ].ho_Attr0 = (LONG)zdip->zdp_BlockSet;
			ho[ G0_BACKDROP ].ho_Attr0 = (LONG)zdip->zdp_BackDrop;

			sprintf( dw->dw_PaletteInfoBuf, "%d Colours",
				zdip->zdp_Palette.pl_Count );
			ho[ G0_PALETTE ].ho_Attr0 = (LONG)dw->dw_PaletteInfoBuf;
			break;
		case 1:	/* players page */
			ho[ G1_P1INIT ].ho_Attr0 = (LONG)zdip->zdp_PlayerInitProgram1;
			ho[ G1_P2INIT ].ho_Attr0 = (LONG)zdip->zdp_PlayerInitProgram2;
			ho[ G1_P3INIT ].ho_Attr0 = (LONG)zdip->zdp_PlayerInitProgram3;
			ho[ G1_P4INIT ].ho_Attr0 = (LONG)zdip->zdp_PlayerInitProgram4;
			break;
		case 2:	/* misc page */
			ho[ G2_AGA ].ho_Value = (zdip->zdp_Flags & DPF_AGAONLY) ?
				TRUE : FALSE;
			ho[ G2_FRAMERATE ].ho_Value = zdip->zdp_FrameRate-1;
			ho[ G2_GAMEINIT ].ho_Attr0 = (LONG)zdip->zdp_InitProgram;
			ho[ G2_GAMEUPDATE ].ho_Attr0 = (LONG)zdip->zdp_StatUpdateProg;
			ho[ G2_NUMOFPLAYEROBJS ].ho_Value = (LONG)zdip->zdp_NumOfPlayerStructs;
			ho[ G2_NUMOFBULLETOBJS ].ho_Value = (LONG)zdip->zdp_NumOfBulletStructs;
			ho[ G2_NUMOFDUDEOBJS ].ho_Value = (LONG)zdip->zdp_NumOfBDStructs;
			ho[ G2_STATBOXPIC ].ho_Attr0 = (LONG)zdip->zdp_StatBackDropPicture;
			ho[ G2_DRAWBARPIC ].ho_Attr0 = (LONG)zdip->zdp_StatShieldBarImage;
			break;
		case 3:	/* DefaultBorders page */
			ho[ G3_LEFTBORDER ].ho_Value = zdip->zdp_DefaultBorderLeft;
			ho[ G3_LEFTBORDERTYPE ].ho_Value = zdip->zdp_DefaultBorderLeftType;
			ho[ G3_LEFTREL ].ho_Value =
				(zdip->zdp_DefaultBorderFlags & 1) ? 1 : 0;
			ho[ G3_RIGHTBORDER ].ho_Value = zdip->zdp_DefaultBorderRight;
			ho[ G3_RIGHTBORDERTYPE ].ho_Value = zdip->zdp_DefaultBorderRightType;
			ho[ G3_RIGHTREL ].ho_Value =
				(zdip->zdp_DefaultBorderFlags & 2) ? 1 : 0;
			ho[ G3_TOPBORDER ].ho_Value = zdip->zdp_DefaultBorderTop;
			ho[ G3_TOPBORDERTYPE ].ho_Value = zdip->zdp_DefaultBorderTopType;
			ho[ G3_TOPREL ].ho_Value =
				(zdip->zdp_DefaultBorderFlags & 4) ? 1 : 0;
			ho[ G3_BOTTOMBORDER ].ho_Value = zdip->zdp_DefaultBorderBottom;
			ho[ G3_BOTTOMBORDERTYPE ].ho_Value = zdip->zdp_DefaultBorderBottomType;
			ho[ G3_BOTTOMREL ].ho_Value =
				(zdip->zdp_DefaultBorderFlags & 8) ? 1 : 0;
			break;
	}
}



/********************************************************/

static BOOL HandleGadgetUp( struct DISPWindow *dw, int gid )
{
	struct HoopyObject *ho;
	struct ZonkDisp *zdip;
	BOOL done = FALSE;

	ho = dw->dw_Layout[ dw->dw_Page ];
	zdip = (struct ZonkDisp *)dw->dw_Chunk->ch_Data;

	switch( dw->dw_Page )
	{
		case 0:		/* gfx */
			switch( gid )
			{
				case G0_PICKPALETTE:
					PickPalette( dw, G0_PALETTE );
					break;
				case G0_IMPORTPALETTE:
					ImportPalette( dw, G0_PALETTE );
					break;
				case G0_MAP:
				case G0_BLKS:
				case G0_BACKDROP:
					if( MakePackable( (UBYTE *)ho[gid].ho_Attr0 ) )
						DisplayBeep( dw->dw_sw.sw_Window->WScreen );
					HO_RefreshObject( &ho[gid], dw->dw_sw.sw_hos );
					ModifyChunk( dw->dw_Chunk );
					break;
				case G0_PICKMAP:
					PickMap( dw, G0_MAP );
					break;
				case G0_PICKBLKS:
					PickBlockset( dw, G0_BLKS );
					break;
				case G0_OK:
					done = TRUE;
					break;
			}
			break;
		case 1:		/* Players */
			switch( gid )
			{
				case G1_P1INIT:
				case G1_P2INIT:
				case G1_P3INIT:
				case G1_P4INIT:
					if( MakePackable( (UBYTE *)ho[gid].ho_Attr0 ) )
						DisplayBeep( dw->dw_sw.sw_Window->WScreen );
					HO_RefreshObject( &ho[gid], dw->dw_sw.sw_hos );
					ModifyChunk( dw->dw_Chunk );
					break;
				case G1_PICKP1INIT:
				case G1_PICKP2INIT:
				case G1_PICKP3INIT:
				case G1_PICKP4INIT:
					PickAProgram( dw, gid-1 );
					break;
				case G1_OK:
					done = TRUE;
					break;
			}
			break;
		case 2:		/* Misc */
			switch( gid )
			{
				case G2_AGA:
					if( ho[gid].ho_Value )
						zdip->zdp_Flags |= DPF_AGAONLY;
					else
						zdip->zdp_Flags &= ~DPF_AGAONLY;
					ModifyChunk( dw->dw_Chunk );
					break;
				case G2_FRAMERATE:
					if( ho[gid].ho_Value == 1 )
						zdip->zdp_FrameRate = 2;
					else
						zdip->zdp_FrameRate = 1;
					ModifyChunk( dw->dw_Chunk );
					break;
				case G2_GAMEINIT:
				case G2_GAMEUPDATE:
				case G2_STATBOXPIC:
				case G2_DRAWBARPIC:
					if( MakePackable( (UBYTE *)ho[gid].ho_Attr0 ) )
						DisplayBeep( dw->dw_sw.sw_Window->WScreen );
					HO_RefreshObject( &ho[gid], dw->dw_sw.sw_hos );
					ModifyChunk( dw->dw_Chunk );
					break;
				case G2_PICKGAMEINIT:
				case G2_PICKGAMEUPDATE:
					PickAProgram( dw, gid-1 );
					break;
				/* really should put some sanity limits here... */
				case G2_NUMOFPLAYEROBJS:
					zdip->zdp_NumOfPlayerStructs = (UWORD)ho[gid].ho_Value;
					ModifyChunk( dw->dw_Chunk );
					break;
				case G2_NUMOFBULLETOBJS:
					zdip->zdp_NumOfBulletStructs = (UWORD)ho[gid].ho_Value;
					ModifyChunk( dw->dw_Chunk );
					break;
				case G2_NUMOFDUDEOBJS:
					zdip->zdp_NumOfBDStructs = (UWORD)ho[gid].ho_Value;
					ModifyChunk( dw->dw_Chunk );
					break;
				case G2_OK:
					done = TRUE;
					break;
			}
			break;
		case 3:		/* DefaultBorders */
			switch( gid )
			{
				case G3_LEFTBORDER:
				case G3_RIGHTBORDER:
				case G3_TOPBORDER:
				case G3_BOTTOMBORDER:
					zdip->zdp_DefaultBorderLeft = ho[G3_LEFTBORDER].ho_Value;
					zdip->zdp_DefaultBorderRight = ho[G3_RIGHTBORDER].ho_Value;
					zdip->zdp_DefaultBorderTop = ho[G3_TOPBORDER].ho_Value;
					zdip->zdp_DefaultBorderBottom = ho[G3_BOTTOMBORDER].ho_Value;
					ModifyChunk( dw->dw_Chunk );
					break;
				case G3_LEFTBORDERTYPE:
				case G3_RIGHTBORDERTYPE:
				case G3_TOPBORDERTYPE:
				case G3_BOTTOMBORDERTYPE:
					zdip->zdp_DefaultBorderLeftType =
						ho[G3_LEFTBORDERTYPE].ho_Value;
					zdip->zdp_DefaultBorderRightType =
						ho[G3_RIGHTBORDERTYPE].ho_Value;
					zdip->zdp_DefaultBorderTopType =
						ho[G3_TOPBORDERTYPE].ho_Value;
					zdip->zdp_DefaultBorderBottomType
						= ho[G3_BOTTOMBORDERTYPE].ho_Value;
					ModifyChunk( dw->dw_Chunk );
					break;
				case G3_LEFTREL:
					if( ho[G3_LEFTREL].ho_Value )
						zdip->zdp_DefaultBorderFlags |= 1;
					else
						zdip->zdp_DefaultBorderFlags &= ~1;
					ModifyChunk( dw->dw_Chunk );
					break;
				case G3_RIGHTREL:
					if( ho[G3_RIGHTREL].ho_Value )
						zdip->zdp_DefaultBorderFlags |= 2;
					else
						zdip->zdp_DefaultBorderFlags &= ~2;
					ModifyChunk( dw->dw_Chunk );
					break;
				case G3_TOPREL:
					if( ho[G3_TOPREL].ho_Value )
						zdip->zdp_DefaultBorderFlags |= 4;
					else
						zdip->zdp_DefaultBorderFlags &= ~4;
					ModifyChunk( dw->dw_Chunk );
					break;
				case G3_BOTTOMREL:
					if( ho[G3_BOTTOMREL].ho_Value )
						zdip->zdp_DefaultBorderFlags |= 8;
					else
						zdip->zdp_DefaultBorderFlags &= ~8;
					ModifyChunk( dw->dw_Chunk );
					break;
				case G3_OK:
					done = TRUE;
					break;
			}
			break;
	}
	return done;
}

/********************************************************/

static void PickPalette( struct DISPWindow *dw, int gid )
{
	struct Chunk *cnk;
	struct Palette *p;
	struct HoopyObject *ho;

	ho = dw->dw_Layout[ dw->dw_Page ];

	if( BlockAllSuperWindows() )
	{
		cnk = RequestChunk( dw->dw_sw.sw_Window, "Palette Request", "Pick Palette",
			ID_CMAP,NULL );
	}
	UnblockAllSuperWindows();

	if( cnk )
	{
		p = (struct Palette *)cnk->ch_Data;
		CopyMem( p,
			&((struct ZonkDisp *)(dw->dw_Chunk->ch_Data))->zdp_Palette,
			sizeof( struct Palette ) );
		sprintf( dw->dw_PaletteInfoBuf, "%d Colours",
			((struct ZonkDisp *)dw->dw_Chunk->ch_Data)->zdp_Palette.pl_Count );

		HO_RefreshObject( &ho[ gid ], dw->dw_sw.sw_hos );
		ModifyChunk( dw->dw_Chunk );
	}
}



/********************************************************/

static void PickAProgram( struct DISPWindow *dw, int gid )
{
	struct HoopyObject *ho;
	struct Program *prog = NULL;


	assert( dw != NULL );

	ho = dw->dw_Layout[ dw->dw_Page ];

	if( BlockAllSuperWindows() )
	{
		prog = RequestProgram( dw->dw_sw.sw_Window, NULL, "Pick Actionlist...",
			"Actionlists" );
	}
	UnblockAllSuperWindows();

	if( prog )
	{
		Mystrncpy( (UBYTE *)ho[gid].ho_Attr0, prog->pg_Name, PROGNAMESIZE-1 );
		HO_RefreshObject( &ho[gid], dw->dw_sw.sw_hos );
		ModifyChunk( dw->dw_Chunk );
	}
}


/********************************************************/

static VOID PickMap( struct DISPWindow *dw, int gid )
{
	struct Chunk *cnk;
	struct Map *map;
	struct HoopyObject *ho;

	ho = dw->dw_Layout[ dw->dw_Page ];

	if( BlockAllSuperWindows() )
	{

		cnk = RequestChunk( dw->dw_sw.sw_Window, "Select Map",
			"Maps", ID_MAP, NULL );
	}
	UnblockAllSuperWindows();
	if( cnk )
	{
		map = (struct Map *)cnk->ch_Data;
		Mystrncpy( (UBYTE *)ho[ gid ].ho_Attr0, map->map_Name, MAPNAMESIZE-1 );
		HO_RefreshObject( &ho[ gid ], dw->dw_sw.sw_hos );
		ModifyChunk( dw->dw_Chunk );
	}
}

/********************************************************/

static VOID PickBlockset( struct DISPWindow *dw, int gid )
{
	struct Chunk *cnk;
	struct Blockset *bs;
	struct HoopyObject *ho;

	ho = dw->dw_Layout[ dw->dw_Page ];

	if( BlockAllSuperWindows() )
	{
		cnk = RequestChunk( dw->dw_sw.sw_Window, "Select Blockset",
			"Blocksets", ID_BLKS, NULL );
	}
	UnblockAllSuperWindows();
	if( cnk )
	{
		bs = (struct Blockset *)cnk->ch_Data;
		Mystrncpy( (UBYTE *)ho[ gid ].ho_Attr0, bs->bs_Name, BLOCKSETNAMESIZE-1 );
		HO_RefreshObject( &ho[ gid ], dw->dw_sw.sw_hos );
		ModifyChunk( dw->dw_Chunk );
	}
}

/********************************************************/
static void ImportPalette( struct DISPWindow *dw, int infogid )
{
	char namebuf[512];
	struct HoopyObject *ho;
	struct ZonkDisp *zdip;

	assert( dw != NULL );
	assert( dw->dw_Chunk != NULL );

	ho = dw->dw_Layout[ dw->dw_Page ];

	zdip = (struct ZonkDisp *)dw->dw_Chunk->ch_Data;

	if( BlockAllSuperWindows() )
	{
		if( AslRequestTags( filereq,
			ASLFR_Window, dw->dw_sw.sw_Window,
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
				if( LoadPalette( namebuf, &zdip->zdp_Palette ) )
					ModifyChunk( dw->dw_Chunk );
			}
		}
		UnblockAllSuperWindows();
	}

	sprintf( dw->dw_PaletteInfoBuf, "%d Colours",
		((struct ZonkDisp *)dw->dw_Chunk->ch_Data)->zdp_Palette.pl_Count );
	HO_RefreshObject( &ho[ infogid ], dw->dw_sw.sw_hos );
}
