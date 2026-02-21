/********************************************************/
//
// EOTRWindow.c
//
/********************************************************/

#define EOTRWINDOW_C

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
#include <graphics/text.h>
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

struct EOTRWindow
{
	struct SuperWindow	ew_sw;
	struct Chunk				*ew_Chunk;
	struct HoopyObject	*ew_Layout;
	struct Chunk				*ew_BlocksetChunk;
	WORD 								ew_XPos;
	WORD 								ew_YPos;
	UBYTE								ew_LMBValue;
	UBYTE								ew_Pad;
	BOOL								ew_Drawing;
};

#define VIEWLEFT ( ew->ew_Layout[ GID_VIEW ].ho_Attr0 >> 16 )
#define VIEWTOP ( ew->ew_Layout[ GID_VIEW ].ho_Attr0 & 0xFFFF )
#define VIEWWIDTH ( ew->ew_Layout[ GID_VIEW ].ho_Attr1 >> 16 )
#define VIEWHEIGHT ( ew->ew_Layout[ GID_VIEW ].ho_Attr1 & 0xFFFF )
#define REFRESHGID( gid ) HO_RefreshObject( &ew->ew_Layout[ gid ], ew->ew_sw.sw_hos )

UBYTE *editmodelabels[] = { "Boolean","Value",NULL };

#define GID_BORING 0

#define GID_NAME 			2
#define GID_VIEW 			4
#define GID_LMBVALUE	7
#define GID_BLKS			10
#define GID_PICKBLKS	11

static void FontInit_ConkTinyNumbers5( void );

static struct HoopyObject gadlayout[] =
{
	{ HOTYPE_VGROUP, 100, HOFLG_NOBORDER,3,0,0,0, NULL, 0,GID_BORING,0,0,0 },
		{ HOTYPE_HGROUP, 100, 0,2,0,0,0, NULL, 0,GID_BORING,0,0,0 },
			{ HOTYPE_PACKEDSTRING, 100, HOFLG_NOBORDER, 0,EOTRNAMESIZE-1,0,0, "Name",0,GID_NAME,PLACETEXT_LEFT,0 },
			{ HOTYPE_SPACE, 100, HOFLG_NOBORDER|HOFLG_FREEWIDTH, 0,0,0,0, "",0,GID_BORING,0,0 },
		{ HOTYPE_SPACE, 100, HOFLG_RAISEDBORDER|HOFLG_FREEWIDTH|HOFLG_FREEHEIGHT, 0,0,0,0, "",0,GID_VIEW,0,0 },
		{ HOTYPE_VGROUP, 100, HOFLG_RAISEDBORDER,2,0,0,0, NULL, 0,GID_BORING,0,0,0 },
			{ HOTYPE_HGROUP, 100, 0,2,0,0,0, NULL, 0,GID_BORING,0,0,0 },
				{ HOTYPE_INTEGER, 100, HOFLG_NOBORDER, 3,0,0,0, "Value",0,GID_LMBVALUE,PLACETEXT_LEFT,0 },
				{ HOTYPE_SPACE, 100, HOFLG_NOBORDER|HOFLG_FREEWIDTH, 0,0,0,0, "",0,GID_BORING,0,0 },
			{ HOTYPE_HGROUP, 100, 0,2,0,0,0, NULL, 0,GID_BORING,0,0,0 },
				{ HOTYPE_PACKEDSTRING, 100, HOFLG_NOBORDER|HOFLG_FREEWIDTH, 0,BLOCKSETNAMESIZE-1,0,0, "Blockset",0,GID_BLKS,PLACETEXT_LEFT,0 },
				{ HOTYPE_BUTTON, 100, HOFLG_NOBORDER, 0,0,0,0, "?",0,GID_PICKBLKS,PLACETEXT_IN,0 },
	{HOTYPE_END}
};

/********************************************************/

/* used: */
/*  FontConverter_V2.0 © 1992 by Andreas Baum  */

static struct TextFont ConkTinyNumbers5Font;

static BOOL fontready = FALSE;

static chip UWORD ConkTinyNumbers5dump[] =
  {
	0xe4ee,	0x8e8e,	0xee00,	0xac22,	0x8882,	0xaae0,	0xa4ee,	0xcee2,
	0xeea0,	0xa482,	0xe2a2,	0xa2e0,	0xeeee,	0x4ee2,	0xe200,	0x0000,
	0x0004,	0x0004,	0x0004,	0x0008,	0x0004,	0x000c,	0x0004,	0x0010,
	0x0004,	0x0014,	0x0004,	0x0018,	0x0004,	0x001c,	0x0004,	0x0020,
	0x0004,	0x0024,	0x0004,	0x0028,	0x0004
  };

/********************************************************/

static void IDCMPHandler( struct SuperWindow *sw,
	struct IntuiMessage *imsg );
static void HideWindow( struct SuperWindow *sw );
static BOOL ShowWindow( struct SuperWindow *sw );
static void KillWindow( struct SuperWindow *sw );
static BOOL RefreshMenus( struct SuperWindow *sw );
static UBYTE *MakeEOTRWinTitle( struct SuperWindow *sw );

static void RelinquishBlockset( struct EOTRWindow *ew );
static void SnarfBlockset( struct EOTRWindow *ew, struct Chunk *bscnk );
static void ClipToView( struct EOTRWindow *ew );
static void PickBlockset( struct EOTRWindow *ew );
static void DrawView( struct EOTRWindow *ew );
static void DrawOverlay( struct EOTRWindow *ew );

static void FontInit_ConkTinyNumbers5( void );
static BOOL PlonkValue( struct EOTRWindow *ew, WORD mousex, WORD mousey );






/********************************************************/

BOOL OpenNewEOTRWindow( struct Chunk *cnk )
{
	struct SuperWindow *sw;
	struct EOTRWindow *ew;
	BOOL success = FALSE;

	assert( cnk != NULL );
	assert( cnk->ch_TypeID == ID_EOTR );
	assert( cnk->ch_Data != NULL );

	if( FindSuperWindowByType( (struct SuperWindow *)superwindows.lh_Head,
		SWTY_EOTR ) ) return( FALSE );

	LockChunk( cnk );

	if( ew = AllocVec( sizeof( struct EOTRWindow ), MEMF_ANY|MEMF_CLEAR ) )
	{
		sw = &ew->ew_sw;
		sw->sw_Handler = IDCMPHandler;
		sw->sw_HideWindow = HideWindow;
		sw->sw_ShowWindow = ShowWindow;
		sw->sw_BuggerOff = KillWindow;
		sw->sw_RefreshMenus = RefreshMenus;
		sw->sw_MakeTitle	= MakeEOTRWinTitle;
		sw->sw_Type				= SWTY_EOTR;
		sw->sw_Window			= NULL;
		sw->sw_Gadgets		= NULL;
		sw->sw_VisualInfo	= NULL;
		sw->sw_WinDim			= zonkfig.SuperWindowDefs[ SWTY_EOTR ];

		ew->ew_Chunk = cnk;
		(*sw->sw_ShowWindow)( sw );
		AddTail( &superwindows, &sw->sw_Node );
		success = TRUE;

		FontInit_ConkTinyNumbers5();

	}
	return( success );
}

/********************************************************/

static BOOL ShowWindow( struct SuperWindow *sw )
{
	BOOL success = FALSE;
	struct EOTRWindow *ew;
	struct Window *win;
	struct Screen *scr;
	struct Chunk *bscnk;
	struct EdgeOfTheRoad *eotr;

	ew = (struct EOTRWindow *)sw;

	scr = LockGFXScreen();

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
		WA_ReportMouse,	TRUE,
		WA_Activate,		TRUE,
		TAG_DONE,				NULL);

	UnlockMainScreen( scr );

	if( sw->sw_Window = win )
	{
		if( !zonkfig.scfg.scfg_DefaultFont && gfxwinfont )
			SetFont( win->RPort, gfxwinfont );

		if( ew->ew_Layout = AllocVec( sizeof( gadlayout ), MEMF_ANY ) )
		{
			CopyMem( gadlayout, ew->ew_Layout, sizeof( gadlayout ) );
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
					IDCMP_RAWKEY|IDCMP_MOUSEBUTTONS|IDCMP_MOUSEMOVE|
					CYCLEIDCMP|STRINGIDCMP|INTEGERIDCMP|BUTTONIDCMP );

				eotr = (struct EdgeOfTheRoad *)ew->ew_Chunk->ch_Data;
				ew->ew_Layout[ GID_BLKS ].ho_Attr0 =
					(LONG)eotr->BlocksetName;
				ew->ew_Layout[ GID_NAME ].ho_Attr0 =
					(LONG)eotr->Name;

				bscnk = FindBLKSByName( eotr->BlocksetName );
				SnarfBlockset( ew, bscnk );

				HO_SussWindowSizing( sw->sw_hos, ew->ew_Layout );

				HO_MakeLayout( ew->ew_Layout, sw->sw_hos, win->BorderLeft, win->BorderTop,
					win->Width - win->BorderLeft - win->BorderRight,
					win->Height - win->BorderTop - win->BorderBottom );

				DrawView( ew );
				DrawOverlay( ew );

				success = TRUE;
			}
		}
	}

	if( !success )
	{
		if( sw->sw_Window ) CloseWindow( sw->sw_Window );
		sw->sw_Window = NULL;
		if( ew->ew_Layout ) FreeVec( ew->ew_Layout );
		ew->ew_Layout = NULL;
	}

	return( success );
}


/********************************************************/

static void HideWindow( struct SuperWindow *sw )
{
	struct EOTRWindow *ew;

	ew = (struct EOTRWindow *)sw;

	if( sw->sw_Window )
	{
		RememberWindow( sw );
		GenericZapMenus( sw );
		HO_KillLayout( sw->sw_hos );
		CloseWindowSafely( sw->sw_Window );
		HO_FreeHOStuff( sw->sw_hos );
		sw->sw_Window = NULL;
		FreeVec( ew->ew_Layout );
		ew->ew_Layout = NULL;

		if( ew->ew_BlocksetChunk )
			RelinquishBlockset( ew );
	}
}

/********************************************************/

static void KillWindow( struct SuperWindow *sw )
{
	if( sw && IsOKToClose( sw ) )
	{
		(*sw->sw_HideWindow)(sw);
		UnlockChunk( ((struct EOTRWindow *)sw)->ew_Chunk );
		Remove( &sw->sw_Node );
		FreeVec( sw );
	}
}


/********************************************************/

static UBYTE *MakeEOTRWinTitle( struct SuperWindow *sw )
{
	struct Chunk *cnk;
	UBYTE *p;

	cnk = ((struct EOTRWindow *)sw)->ew_Chunk;
	p = sw->sw_Name;

	if( cnk->ch_Modified )
		*p++ = MODCHAR;
	strcpy( p, "EdgeOfTheRoad" );

	return( sw->sw_Name );
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
	struct EOTRWindow *ew;
	struct EdgeOfTheRoad *eotr;

	ULONG class;
	UWORD code;
//	UWORD qualifier;
	APTR iaddress;
	WORD mousex, mousey;
	ULONG seconds, micros;

	ew = (struct EOTRWindow *)sw;
	win = sw->sw_Window;
	eotr = (struct EdgeOfTheRoad *)ew->ew_Chunk->ch_Data;

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
				HO_ReadGadgetStates( sw->sw_hos, ew->ew_Layout );
				HO_MakeLayout( ew->ew_Layout, sw->sw_hos, win->BorderLeft, win->BorderTop,
					win->Width - win->BorderLeft - win->BorderRight,
					win->Height - win->BorderTop - win->BorderBottom );
				DrawView( ew );
				DrawOverlay( ew );

				ReplyMsg( (struct Message *)origimsg );
				break;
			case IDCMP_CHANGEWINDOW:
				UpdateWinDef( sw );
				ReplyMsg( (struct Message *)origimsg );
				break;
			case IDCMP_GADGETUP:
				HO_ReadGadgetStates( sw->sw_hos, ew->ew_Layout );
				ReplyMsg( (struct Message *)origimsg );
				switch( ((struct Gadget*)iaddress)->GadgetID )
				{
					case GID_BORING:
						break;
					case GID_NAME:
						ModifyChunk( ew->ew_Chunk );
						break;
					case GID_LMBVALUE:
						ew->ew_LMBValue = (UBYTE)ew->ew_Layout[ GID_LMBVALUE ].ho_Value;
						break;
					case GID_BLKS:
						SnarfBlockset( ew, FindBLKSByName( eotr->BlocksetName ) );
						DrawView( ew );
						DrawOverlay( ew );
						ModifyChunk( ew->ew_Chunk );
						break;
					case GID_PICKBLKS:
						PickBlockset( ew );
						REFRESHGID( GID_BLKS );
						DrawView( ew );
						DrawOverlay( ew );
						ModifyChunk( ew->ew_Chunk );
						break;
				}
				break;
			case IDCMP_MOUSEBUTTONS:
				switch( code )
				{
					case SELECTDOWN:
						if( PlonkValue( ew, mousex, mousey ) )
						{
							ModifyChunk( ew->ew_Chunk );
							ew->ew_Drawing = TRUE;
						}
						break;
					case SELECTUP:
						ew->ew_Drawing = FALSE;
						break;
				}
				break;
			case IDCMP_MOUSEMOVE:
				ReplyMsg( (struct Message *)origimsg );
				if( ew->ew_Drawing )
				{
					ModifyChunk( ew->ew_Chunk );
					PlonkValue( ew, mousex, mousey );
				}
				break;
			case IDCMP_RAWKEY:
				ReplyMsg( (struct Message *)origimsg );

				switch( code )
				{
					case RAWKEY_UP:
						ew->ew_YPos--;
						break;
					case RAWKEY_DOWN:
						ew->ew_YPos++;
						break;
					case RAWKEY_LEFT:
						ew->ew_XPos--;
						break;
					case RAWKEY_RIGHT:
						ew->ew_XPos++;
						break;
				}

				if( !(code & IECODE_UP_PREFIX) )
				{
					DrawView( ew );
					DrawOverlay( ew );
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

static void PickBlockset( struct EOTRWindow *ew )
{
	struct Chunk *bscnk;
	struct EdgeOfTheRoad *eotr;

	assert( ew != NULL );
	assert( ew->ew_Chunk != NULL );
	assert( ew->ew_Chunk->ch_Data != NULL );

	eotr = (struct EdgeOfTheRoad *)ew->ew_Chunk->ch_Data;

	if( BlockAllSuperWindows() )
	{
		bscnk = RequestChunk( ew->ew_sw.sw_Window, "Select Blockset",
			"Blocksets", ID_BLKS, NULL );
		if( bscnk )
			SnarfBlockset( ew, bscnk );
	}
	UnblockAllSuperWindows();
}

/********************************************************/

static void SnarfBlockset( struct EOTRWindow *ew, struct Chunk *bscnk )
{
	struct EdgeOfTheRoad *eotr;
	struct Blockset *bs;

	assert ( ew != NULL );

	if( ew->ew_BlocksetChunk )
		RelinquishBlockset( ew );

	if( bscnk )
	{
		ew->ew_BlocksetChunk = bscnk;
		bs = (struct Blockset *)bscnk->ch_Data;
		eotr = (struct EdgeOfTheRoad *)ew->ew_Chunk->ch_Data;
		eotr->NumOfBlocks = bs->bs_NumOfBlocks;
		Mystrncpy( eotr->BlocksetName, bs->bs_Name, BLOCKSETNAMESIZE-1 );
		LockChunk( bscnk );
	}
	else
	{
		ew->ew_BlocksetChunk = NULL;
	}
}

/********************************************************/


static void RelinquishBlockset( struct EOTRWindow *ew )
{
	struct EdgeOfTheRoad *eotr;

	assert( ew != NULL );

	eotr = (struct EdgeOfTheRoad *)ew->ew_Chunk->ch_Data;

	if( ew->ew_BlocksetChunk )
	{
		UnlockChunk( ew->ew_BlocksetChunk );
		ew->ew_BlocksetChunk = NULL;
//		eotr->NumOfBlocks = 0;
	}
}


/********************************************************/


static void ClipToView( struct EOTRWindow *ew )
{
	struct Region *oldregion;

	if( oldregion = ClipWindow( ew->ew_sw.sw_Window, VIEWLEFT, VIEWTOP,
		VIEWLEFT + VIEWWIDTH - 1, VIEWTOP + VIEWHEIGHT - 1 ) )
	{
		DisposeRegion( oldregion );
	}
}


/********************************************************/

static void DrawView( struct EOTRWindow *ew )
{
	struct Blockset *bs;
	struct RastPort *rp;
	WORD x,y,wx,wy;
	UWORD blk;

	assert( ew != NULL );
	assert( ew->ew_Chunk != NULL );

	rp = ew->ew_sw.sw_Window->RPort;
	if( ew->ew_BlocksetChunk )
	{
		bs = (struct Blockset *)ew->ew_BlocksetChunk->ch_Data;

		ClipToView( ew );

		for( y=ew->ew_YPos,wy=VIEWTOP; wy < VIEWTOP+VIEWHEIGHT; y++,wy += BLKH )
		{
			for( x=ew->ew_XPos,wx=VIEWLEFT; wx < VIEWLEFT+VIEWWIDTH; x++,wx += BLKW )
			{
				blk = (y * bs->bs_LayoutWidth) + x;
				if( (x >= bs->bs_LayoutWidth) || ( x < 0 ) ||
					(blk >= bs->bs_NumOfBlocks) )
				{
					blk = 0;
				}
				UnclippedDrawBlockRP( rp, wx, wy, bs, blk );
			}
		}

		UnclipWindow( ew->ew_sw.sw_Window );
	}
	else
	{
		SetAPen( rp, 0 );
		RectFill(rp, VIEWLEFT, VIEWTOP,
			VIEWLEFT + VIEWWIDTH - 1, VIEWTOP + VIEWHEIGHT - 1 );
	}

}

/********************************************************/
static void DrawOverlay( struct EOTRWindow *ew )
{
	struct Blockset *bs;
	struct RastPort *rp;
	WORD x,y,wx,wy;
	UWORD blk;
	struct TextFont *oldfont;
	struct EdgeOfTheRoad *eotr;
	char buf[4];
	WORD layoutwidth,l;
	static UBYTE *picktext = "Pick a BlockSet";

	assert( ew != NULL );
	assert( ew->ew_Chunk != NULL );

	rp = ew->ew_sw.sw_Window->RPort;
	eotr = (struct EdgeOfTheRoad *)ew->ew_Chunk->ch_Data;

	SetAPen( rp, ew->ew_sw.sw_hos->hos_dri->dri_Pens[ SHINEPEN ] );

	ClipToView( ew );

	if( ew->ew_BlocksetChunk )
		bs = (struct Blockset *)ew->ew_BlocksetChunk->ch_Data;
	else
		bs = NULL;

	if( bs && eotr->NumOfBlocks && eotr->Data )
	{
		oldfont = rp->Font;
		SetFont( rp, &ConkTinyNumbers5Font );

		layoutwidth = bs->bs_LayoutWidth;

		for( y = ew->ew_YPos, wy=VIEWTOP;
			wy < VIEWTOP+VIEWHEIGHT;
			y++, wy += BLKH )
		{
			for( x = ew->ew_XPos, wx=VIEWLEFT;
				wx < VIEWLEFT+VIEWWIDTH;
				x++, wx += BLKW )
			{
				blk = (y * layoutwidth) + x;
				if( ( x >= 0 ) && (x < layoutwidth) &&
					(blk < eotr->NumOfBlocks) )
				{
					if( eotr->Data[ blk ] != 0 )
					{
						sprintf( buf, "%d", (ULONG)eotr->Data[blk] );
						Move( rp, wx+2, wy+8 );
						Text( rp, buf, strlen(buf) );
					}
					WritePixel( rp, wx, wy );
				}
			}
		}
		if( ConkTinyNumbers5Font.tf_Extension )
			StripFont( &ConkTinyNumbers5Font );
		SetFont( rp, oldfont );
	}
	else
	{
		l = TextLength( rp, picktext, 15 );
		Move( rp, VIEWLEFT+(VIEWWIDTH/2) - (l/2),
			VIEWTOP+(VIEWHEIGHT/2)+(rp->Font->tf_YSize/2) );
		Text( rp, picktext, 15 );
	}
	UnclipWindow( ew->ew_sw.sw_Window );
}

/********************************************************/


static void FontInit_ConkTinyNumbers5( void )
{
	if( !fontready )
	{
		ConkTinyNumbers5Font.tf_Message.mn_Node.ln_Name = "ConkTinyNumbers5.font";
		ConkTinyNumbers5Font.tf_YSize = 0x0005;
		ConkTinyNumbers5Font.tf_Style = 0x00;
		ConkTinyNumbers5Font.tf_Flags = 0x40;
		ConkTinyNumbers5Font.tf_XSize = 0x0004;
		ConkTinyNumbers5Font.tf_Baseline = 0x0004;
		ConkTinyNumbers5Font.tf_BoldSmear = 0x0001;
		ConkTinyNumbers5Font.tf_Accessors = 0x0000;
		ConkTinyNumbers5Font.tf_LoChar = 0x30;
		ConkTinyNumbers5Font.tf_HiChar = 0x39;
		ConkTinyNumbers5Font.tf_CharData = (APTR)((char *)&ConkTinyNumbers5dump[0]+0x00000000);
		ConkTinyNumbers5Font.tf_Modulo = 0x0006;
		ConkTinyNumbers5Font.tf_CharLoc = (APTR)((char *)&ConkTinyNumbers5dump[0]+0x0000001e);
		ConkTinyNumbers5Font.tf_CharKern = 0x00000000;
		ConkTinyNumbers5Font.tf_CharSpace = 0x00000000;
		fontready = TRUE;
	}
}

/********************************************************/

static BOOL PlonkValue( struct EOTRWindow *ew, WORD mousex, WORD mousey )
{
	BOOL plonker = FALSE;
	WORD bx,by,wx,wy;
	UWORD blk;
	struct Blockset *bs;
	struct EdgeOfTheRoad *eotr;
	struct RastPort *rp;
	struct TextFont *oldfont;
	UBYTE buf[16];

	rp = ew->ew_sw.sw_Window->RPort;
	SetAPen( rp, ew->ew_sw.sw_hos->hos_dri->dri_Pens[ SHINEPEN ] );
	oldfont = rp->Font;
	SetFont( rp, &ConkTinyNumbers5Font );

	if( ew->ew_BlocksetChunk )
	{
		bs = (struct Blockset *)ew->ew_BlocksetChunk->ch_Data;
		eotr = (struct EdgeOfTheRoad *)ew->ew_Chunk->ch_Data;

		if( ( mousex >= VIEWLEFT ) &&
			( mousex < VIEWLEFT + VIEWWIDTH ) &&
			( mousey >= VIEWTOP ) &&
			( mousey < VIEWTOP + VIEWHEIGHT ) )
		{
			ClipToView( ew );

			bx = ( (mousex - VIEWLEFT) / BLKW ) + ew->ew_XPos;
			by = ( (mousey - VIEWTOP) / BLKH ) + ew->ew_YPos;

			if( bx >= 0 && bx < bs->bs_LayoutWidth && by >=0 )
			{
				blk = ( by * bs->bs_LayoutWidth) + bx;
				if( blk < eotr->NumOfBlocks )
				{
					wx = VIEWLEFT + ( (bx - ew->ew_XPos) * BLKW );
					wy = VIEWTOP + ( (by - ew->ew_YPos) * BLKH );

					eotr->Data[ blk ] = ew->ew_LMBValue;
					UnclippedDrawBlockRP( rp, wx, wy, bs, blk );

					if( eotr->Data[blk] != 0 )
					{
						sprintf( buf, "%d", (ULONG)eotr->Data[blk] );
						Move( rp, wx+2, wy+8 );
						Text( rp, buf, strlen(buf) );
					}
					WritePixel( rp, wx, wy );
					plonker = TRUE;
				}
			}
		}

		UnclipWindow( ew->ew_sw.sw_Window );
	}

	if( ConkTinyNumbers5Font.tf_Extension )
		StripFont( &ConkTinyNumbers5Font );
	SetFont( rp, oldfont );

	return plonker;
}
