/********************************************************/
//
// ChunkWindow.c
//
/********************************************************/

#define CHUNKWINDOW_C

#include <stdio.h>
#include <string.h>
#include <stdarg.h>

#include <dos/dos.h>
#include <dos/dostags.h>
#include <exec/exec.h>
#include <intuition/intuition.h>
#include <intuition/imageclass.h>
//#include <graphics/gfxbase.h>
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
#include "MenuID.h"


struct ChunkWindow
{
	struct SuperWindow	cw_sw;
//	struct HOStuff			*cw_hos;
	struct List					cw_ChunkList;
	char								cw_InfoString[ CHUNKINFOSTRINGSIZE ];
	ULONG								cw_lastclicksecs;
	ULONG								cw_lastclickmicros;
	UWORD								cw_lastclicknum;
};

#define CHUNKMENU CUSTMENU
enum { CHUNKMENU_INFO = (CHUNKMENU<<16) + 1 };

static struct NewMenu chunkmenu[] = {
		{ NM_TITLE,	"Chunk",								0,	0,	0,	(APTR)CHUNKMENU },
		{ NM_ITEM,	"Info...",							0,	0,	0,	(APTR)CHUNKMENU_INFO },
		{ NM_END, NULL, 0,	0,	0,	0,},
	};



static void IDCMPHandler( struct SuperWindow *sw,
	struct IntuiMessage *imsg );
static void HideWindow( struct SuperWindow *sw );
static BOOL ShowWindow( struct SuperWindow *sw );
static VOID KillWindow( struct SuperWindow *sw );
static BOOL RefreshMenus( struct SuperWindow *sw );
static BOOL chunkmenuhandler( struct SuperWindow *sw, ULONG id );

static BOOL CreateChunkList( struct ChunkWindow *cw );
static VOID ZapChunkList( struct ChunkWindow *cw );
static VOID DisplayChunkInfo( struct ChunkWindow *cw, UWORD num );
static VOID ToggleChunk( struct ChunkWindow *cw, UWORD num );
static VOID InitGadgets( struct ChunkWindow *cw );
static VOID cw_EditChunk( struct ChunkWindow *cw );
static VOID cw_FreeChunk( struct ChunkWindow *cw );

static void MakeChunkBriefDesc( struct Chunk *cnk, STRPTR str );

void SaveChunks( struct ChunkWindow *cw );



/************* OBSOLETE ****************/
static VOID HandleMenuPick( struct ChunkWindow *cw, UWORD menunum );


//#define GAD_WIBBLE			0
#define GAD_BORING			0
#define GAD_CHUNKLIST		2
#define GAD_INFO				3
#define GAD_NEW					5
#define GAD_EDIT				6
#define GAD_FREE				7
#define GAD_LOAD				9
#define GAD_SAVE				10
#define GAD_LOADCFG			11
#define GAD_SAVECFG			12

static struct HoopyObject gadlayout[] =
{
	{ HOTYPE_VGROUP, 100, HOFLG_RAISEDBORDER|HOFLG_FREEWIDTH|HOFLG_FREEHEIGHT,3,0,0,0, NULL, 0,GAD_BORING,0,0,0 },
		{ HOTYPE_VGROUP, 100, HOFLG_NOBORDER|HOFLG_FREEWIDTH|HOFLG_FREEHEIGHT,2,0,0,0, NULL, 0,GAD_BORING,0,0,0 },
			{ HOTYPE_LISTVIEW, 200, HOFLG_FREEWIDTH|HOFLG_FREEHEIGHT,NULL,0,0,0,"Chunks",0,GAD_CHUNKLIST,PLACETEXT_ABOVE,0 },
			{ HOTYPE_TEXT, 100, HOFLG_FREEWIDTH, 0,0,0,0, "Info:",0,GAD_INFO,PLACETEXT_LEFT,0 },
		{ HOTYPE_HGROUP, 100, HOFLG_NOBORDER|HOFLG_FREEWIDTH,3,0,0,0, NULL, 0,GAD_BORING,0,0,0 },
			{ HOTYPE_BUTTON, 100, HOFLG_NOBORDER, 0,0,0,0, "New...",0,GAD_NEW,0,0 },
			{ HOTYPE_BUTTON, 100, HOFLG_NOBORDER, 0,0,0,0, "Edit...",0,GAD_EDIT,0,0 },
			{ HOTYPE_BUTTON, 100, HOFLG_NOBORDER, 0,0,0,0, "Free...",0,GAD_FREE,0,0 },
		{ HOTYPE_HGROUP, 100, HOFLG_NOBORDER|HOFLG_FREEWIDTH,2,0,0,0, NULL, 0,GAD_BORING,0,0,0 },
			{ HOTYPE_BUTTON, 100, HOFLG_NOBORDER, 0,0,0,0, "Load...",0,GAD_LOAD,0,0 },
			{ HOTYPE_BUTTON, 100, HOFLG_NOBORDER, 0,0,0,0, "Save...",0,GAD_SAVE,0,0 },
	{HOTYPE_END}
};



BOOL OpenNewChunkWindow( VOID )
{
	struct SuperWindow *sw;
	struct ChunkWindow *cw;
	BOOL success = FALSE;

	/* make sure no other chunk windows open... */
	if( sw = FindSuperWindowByType( (struct SuperWindow *)superwindows.lh_Head,
		SWTY_CHUNK ) )
	{
		/* bring to front */
		if( sw->sw_Window )
			WindowToFront( sw->sw_Window );
		else
			(*sw->sw_ShowWindow)( sw );

		return( FALSE );
	}

	if( cw = AllocVec( sizeof( struct ChunkWindow ), MEMF_ANY|MEMF_CLEAR ) )
	{
		sw = &cw->cw_sw;
		sw->sw_Handler = IDCMPHandler;
		sw->sw_HideWindow = HideWindow;
		sw->sw_ShowWindow = ShowWindow;
		sw->sw_BuggerOff = KillWindow;
		sw->sw_RefreshMenus = RefreshMenus;
		sw->sw_Type				= SWTY_CHUNK;
		sw->sw_Window			= NULL;
		sw->sw_Gadgets		= NULL;
		sw->sw_VisualInfo	= NULL;
/*
		sw->sw_Left				= 100;
		sw->sw_Top				= 100;
		sw->sw_Width			= 400;
		sw->sw_Height			= 120;
*/
		sw->sw_WinDim			= zonkfig.SuperWindowDefs[ SWTY_CHUNK ];

		NewList( &cw->cw_ChunkList );

		(*sw->sw_ShowWindow)( sw );
		AddTail( &superwindows, &sw->sw_Node );
		success = TRUE;
	}
	return( success );
}


static BOOL ShowWindow( struct SuperWindow *sw )
{
	BOOL success = FALSE;
	struct ChunkWindow *cw;
	struct Window *win;
//	ULONG minw,minh;
	struct Screen *scr;

	cw = (struct ChunkWindow *)sw;

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
		WA_Title,				"ChunkWindow",
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
			GenericMakeMenus( sw, chunkmenu );

			/* we want to use our global message port for IDCMP stuff */
			win->UserPort = mainmsgport;
			ModifyIDCMP( win, IDCMP_REFRESHWINDOW|IDCMP_NEWSIZE|IDCMP_CLOSEWINDOW|
				IDCMP_MENUPICK|IDCMP_CHANGEWINDOW|
				LISTVIEWIDCMP|BUTTONIDCMP|IDCMP_INTUITICKS );

			HO_SussWindowSizing( sw->sw_hos, gadlayout );

			InitGadgets( cw );

			HO_MakeLayout( gadlayout, sw->sw_hos, win->BorderLeft, win->BorderTop,
				win->Width - win->BorderLeft - win->BorderRight,
				win->Height - win->BorderTop - win->BorderBottom );

//			DoDisableStuff(sc);

			success = TRUE;
		}
		else
		{
			CloseWindow( win );
			sw->sw_Window = NULL;
		}
	}

	return( success );
}



static void HideWindow( struct SuperWindow *sw )
{
	struct ChunkWindow *cw;

	cw = (struct ChunkWindow *)sw;

	if( sw->sw_Window )
	{
		RememberWindow( sw );
		GenericZapMenus( sw );

//		HO_ReadGadgetStates( sc->sc_hos, gadlayout );
		HO_KillLayout( sw->sw_hos );
		CloseWindowSafely( sw->sw_Window );
		HO_FreeHOStuff( sw->sw_hos );
		sw->sw_Window = NULL;

		ZapChunkList( cw );

	}
}


static VOID KillWindow( struct SuperWindow *sw )
{
	if( sw && IsOKToClose( sw ) )
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


static BOOL chunkmenuhandler( struct SuperWindow *sw, ULONG id )
{
	printf("ChunkMenu! id=%ld\n",id );
	return( FALSE );
}


/********************************************************/

static void IDCMPHandler( struct SuperWindow *sw,
	struct IntuiMessage *origimsg )
{
	struct IntuiMessage *imsg;
	struct Window *win;
	struct ChunkWindow *cw;

	ULONG class;
	UWORD code;
//	UWORD qualifier;
	APTR iaddress;
//	WORD mousex, mousey;
	ULONG seconds, micros;

	cw = (struct ChunkWindow *)sw;
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
				HO_ReadGadgetStates( sw->sw_hos, gadlayout );
				HO_MakeLayout( gadlayout, sw->sw_hos, win->BorderLeft, win->BorderTop,
					win->Width - win->BorderLeft - win->BorderRight,
					win->Height - win->BorderTop - win->BorderBottom );
				//HO_DisableObject( &gadlayout[0], sc->sc_hos, FALSE );
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
					case GAD_EDIT:
						cw_EditChunk( cw );
						break;
					case GAD_NEW:
						NewChunk( &cw->cw_sw );
						break;
					case GAD_SAVE:
						SaveChunks( cw );
						break;
					case GAD_FREE:
						cw_FreeChunk( cw );
						break;
					case GAD_LOAD:
						LoadChunks( &cw->cw_sw );
						break;
					case GAD_CHUNKLIST:
						if( cw->cw_lastclicknum != code )
						{
							cw->cw_lastclicknum = code;
							cw->cw_lastclicksecs = 0;
							cw->cw_lastclickmicros = 0;
						}

						if( DoubleClick( cw->cw_lastclicksecs, cw->cw_lastclickmicros, seconds, micros ) )
						{
							/* did a doubleclick */
							ToggleChunk( cw, code );
							cw->cw_lastclicksecs = 0;
							cw->cw_lastclickmicros = 0;
						}
						else
						{
							/* 'tis just a singleclick */
							DisplayChunkInfo( cw, code );
							cw->cw_lastclicksecs = seconds;
							cw->cw_lastclickmicros = micros;
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
				GenericHandleMenuPick( sw, code, chunkmenuhandler );
				break;
			default:
				ReplyMsg( (struct Message *)origimsg );
				break;
		}
	}
	else
		ReplyMsg( (struct Message *)origimsg );
}



VOID RefreshChunkListings( VOID )
{
	struct SuperWindow *sw;
	struct ChunkWindow *cw;
	ULONG num;

	for( sw = (struct SuperWindow *)superwindows.lh_Head;
		sw->sw_Node.ln_Succ;
		sw = (struct SuperWindow *)sw->sw_Node.ln_Succ )
	{
		if( sw->sw_Type == SWTY_CHUNK )
		{
			cw = (struct ChunkWindow *)sw;
			gadlayout[ GAD_CHUNKLIST ].ho_Attr0 = ~0;
			HO_RefreshObject( &gadlayout[ GAD_CHUNKLIST ], sw->sw_hos );
			ZapChunkList( cw );
			CreateChunkList( cw );
			gadlayout[ GAD_CHUNKLIST ].ho_Attr0 = (ULONG)&cw->cw_ChunkList;

			/* if we're off the end of the list, step back to point to the last item */
			num = CountNodes( &cw->cw_ChunkList );
			if( gadlayout[ GAD_CHUNKLIST ].ho_Value >= num )
				gadlayout[ GAD_CHUNKLIST ].ho_Value = (num==0) ? ~0 : num-1;
			HO_RefreshObject( &gadlayout[ GAD_CHUNKLIST ], sw->sw_hos );

			/* redisplay the info string for any selected chunk */
			if( gadlayout[ GAD_CHUNKLIST ].ho_Value != ~0 )
				DisplayChunkInfo( cw, gadlayout[ GAD_CHUNKLIST ].ho_Value );
		}
	}
}


static BOOL CreateChunkList( struct ChunkWindow *cw )
{
	struct DataNode *noddy;
	struct Chunk *cnk;
	BOOL allrighty = TRUE;
	char workbuf[512];

	ZapChunkList( cw );			// just in case...

	for( cnk = (struct Chunk *)chunklist.lh_Head;
		cnk->ch_Node.ln_Succ && allrighty;
		cnk = (struct Chunk *)cnk->ch_Node.ln_Succ )
	{
		if( noddy = AllocVec( sizeof( struct DataNode ), MEMF_ANY|MEMF_CLEAR ) )
		{
			AddTail( &cw->cw_ChunkList, (struct Node *)noddy );
/*
			if( cnk->ch_ParentFile )
				sprintf( workbuf, "  zzzz from %s", &cnk->ch_ParentFile->ft_File );
			else
				sprintf( workbuf, "  zzzz" );

			*(ULONG *)(&workbuf[2]) = cnk->ch_TypeID;
*/
			workbuf[0] = ' ';
			workbuf[1] = ' ';

			MakeChunkBriefDesc( cnk, &workbuf[2] );

			noddy->dn_Data = (APTR)cnk;

			if( !(noddy->dn_Node.ln_Name = MyStrDup( workbuf ) ) )
				allrighty = FALSE;
		}
	}

	if( !allrighty ) ZapChunkList( cw );

	return( allrighty );
}



static void MakeChunkBriefDesc( struct Chunk *cnk, STRPTR str )
{

	if( cnk->ch_Modified )
		*str++ = MODCHAR;

	switch( cnk->ch_TypeID )
	{
		case ID_MAP	:
			str = stpcpy( str, "MAP    " );
			break;
		case ID_BLKS:
			str = stpcpy( str, "BLKSET " );
			break;
		case ID_BOBS:
			str = stpcpy( str, "BOBS   " );
			break;
		case ID_ANIM:
			str = stpcpy( str, "ANIMS  " );
			break;
		case ID_PROG:
			str = stpcpy( str, "ALISTS " );
			break;
		case ID_DISP:
			str = stpcpy( str, "DISPLAY" );
			break;
		case ID_DFRM:
			str = stpcpy( str, "FRMTNS " );
			break;
		case ID_TWAV:
			str = stpcpy( str, "TWAVES " );
			break;
		case ID_SWAV:
			str = stpcpy( str, "SWAVES " );
			break;
		case ID_WEAP:
			str = stpcpy( str, "WEAPONS" );
			break;
		case ID_SFX	:
			str = stpcpy( str, "SFX    " );
			break;
		case ID_THAP:
			str = stpcpy( str, "PATHS  " );
			break;
		case ID_ZCFG:
			str = stpcpy( str, "ZONKCFG" );
			break;
		case ID_SPRT:
			str = stpcpy( str, "SPRITES" );
			break;
		case ID_EOTR:
			str = stpcpy( str, "EOTR   " );
			break;
		case ID_GAME:
			str = stpcpy( str, "GAME   " );
			break;
		case ID_CMAP:
			str = stpcpy( str, "PALETTE" );
			break;
	}

	if( cnk->ch_ParentFile )
		sprintf( str," '%s'", cnk->ch_ParentFile->ft_File );
}


static VOID ZapChunkList( struct ChunkWindow *cw )
{
	struct DataNode *noddy;

	while( !IsListEmpty( &cw->cw_ChunkList ) )
	{
		noddy = (struct DataNode *)cw->cw_ChunkList.lh_Head;
		Remove( (struct Node *)noddy );
		if( noddy->dn_Node.ln_Name ) FreeVec( noddy->dn_Node.ln_Name );
		FreeVec( (struct Node *)noddy );
	}
}



static VOID DisplayChunkInfo( struct ChunkWindow *cw, UWORD num )
{
	struct Chunk *cnk;
	struct DataNode *dn;
	char *str;

	if( num == ~0 || IsListEmpty( &cw->cw_ChunkList ) )
	{
		gadlayout[ GAD_INFO ].ho_Attr0 = NULL;
		HO_RefreshObject( &gadlayout[ GAD_INFO ], cw->cw_sw.sw_hos );
		return;
	}

	gadlayout[ GAD_INFO ].ho_Attr0 = NULL;
	HO_RefreshObject( &gadlayout[ GAD_INFO ], cw->cw_sw.sw_hos );

	str = cw->cw_InfoString;
	str[0] = '\0';
	for( dn = (struct DataNode *)cw->cw_ChunkList.lh_Head; num; --num )
		dn = (struct DataNode *)dn->dn_Node.ln_Succ;

	if( cnk = (struct Chunk*)dn->dn_Data )
	{
		if( cnk->ch_MakeInfoString )
		{
			if( !(*cnk->ch_MakeInfoString)( cnk, str ) )
				str[0] = '\0';
		}
		else
			strcpy( str, "A Chunk" );
	}
	gadlayout[ GAD_INFO ].ho_Attr0 = (ULONG)cw->cw_InfoString;
	HO_RefreshObject( &gadlayout[ GAD_INFO ], cw->cw_sw.sw_hos );
}



static VOID ToggleChunk( struct ChunkWindow *cw, UWORD num )
{
	struct DataNode *dn;
	char *name;
	struct Filetracker *ft;

	gadlayout[ GAD_CHUNKLIST ].ho_Attr0 = ~0;
	HO_RefreshObject( &gadlayout[ GAD_CHUNKLIST ], cw->cw_sw.sw_hos );

	for( dn = (struct DataNode *)cw->cw_ChunkList.lh_Head; num; --num )
		dn = (struct DataNode *)dn->dn_Node.ln_Succ;

	if( name = dn->dn_Node.ln_Name )
	{
		if( name[0] == ' ' )
		{
			if( ft = ((struct Chunk *)dn->dn_Data)->ch_ParentFile )
			{
				for( dn = (struct DataNode *)cw->cw_ChunkList.lh_Head;
					dn->dn_Node.ln_Succ;
					dn = (struct DataNode *)dn->dn_Node.ln_Succ )
				{
					if( ((struct Chunk *)dn->dn_Data)->ch_ParentFile == ft )
					{
						dn->dn_Node.ln_Name[0] = '*';
					}
				}
			}
			else
				name[0] = '*';
		}
		else
			name[0] = ' ';
	}

	gadlayout[ GAD_CHUNKLIST ].ho_Attr0 = (ULONG)&cw->cw_ChunkList;
	HO_RefreshObject( &gadlayout[ GAD_CHUNKLIST ], cw->cw_sw.sw_hos );
}




static VOID InitGadgets( struct ChunkWindow *cw )
{
	CreateChunkList( cw );
	gadlayout[GAD_CHUNKLIST].ho_Attr0 = (ULONG)&cw->cw_ChunkList;
	gadlayout[GAD_INFO].ho_Attr0 = (ULONG)cw->cw_InfoString;
}




void SaveChunks( struct ChunkWindow *cw )
{
	struct DataNode *dn;
	char namebuf[512];
	struct Chunk *cnk, **cnkarray, **cnkpp;
	BOOL allok;
	struct Filetracker *ft;
	UWORD count;

	allok = TRUE;
	/* count the number of chunks to save, and figure out a default file */
	ft = NULL;
	count = 0;
	for( dn = (struct DataNode *)cw->cw_ChunkList.lh_Head;
		dn->dn_Node.ln_Succ && allok;
		dn = (struct DataNode *)dn->dn_Node.ln_Succ )
	{
		if( dn->dn_Node.ln_Name[0] == '*' )
		{
			count++;
			cnk = dn->dn_Data;
			if( !ft )
				ft = cnk->ch_ParentFile;
		}
	}

	if( BlockAllSuperWindows() )
	{
		if( count )
		{
			if( cnkarray = AllocVec( count * sizeof( struct Chunk *), MEMF_ANY ) )
			{
				/* set up chunk ptr array for SaveFile() */
				cnkpp = cnkarray;
				for( dn = (struct DataNode *)cw->cw_ChunkList.lh_Head;
					dn->dn_Node.ln_Succ && allok;
					dn = (struct DataNode *)dn->dn_Node.ln_Succ )
				{
					if( dn->dn_Node.ln_Name[0] == '*' )
						*cnkpp++ = dn->dn_Data;
				}

				if( AslRequestTags( filereq,
					ASLFR_Window, cw->cw_sw.sw_Window,
					ASLFR_TitleText, "Save chunks",
					ASLFR_RejectIcons, TRUE,
					ASLFR_DoMultiSelect, FALSE,
					ASLFR_DoPatterns, FALSE,
					ASLFR_DoSaveMode, TRUE,
					ASLFR_InitialFile, ft ? ft->ft_File : NULL,
					ASLFR_InitialDrawer, ft ? ft->ft_Drawer : NULL,
					TAG_DONE ) )
				{
					Mystrncpy( namebuf, filereq->fr_Drawer, 512-1 );
					if( AddPart( namebuf, filereq->fr_File, 512) )
					{
						if( zonkfig.MakeBackups )
							allok = MakeBackupFile( namebuf );

						if( allok )
						{
							allok = SaveFile( filereq->fr_Drawer, filereq->fr_File, cnkarray, count );
						}
						else
							PlebReq( cw->cw_sw.sw_Window, "Save Error", "Couldn't backup old file!", "Bugger!" );
					}
				}
			}
			else
				D(bug( "AllocVec Failed!\n") );
		}
		else
			PlebReq( cw->cw_sw.sw_Window, "Save Error", "No chunks selected!", "Doh!" );

		if( !allok )
			PlebReq( cw->cw_sw.sw_Window, "Save Error", "Save stuffed up!", "Bugger!" );
	}
	UnblockAllSuperWindows();
	RefreshSuperWindowTitles();
	RefreshChunkListings();
}






static VOID cw_EditChunk( struct ChunkWindow *cw )
{
	struct DataNode *dn;
	struct Chunk *cnk;

	if( !IsListEmpty( &cw->cw_ChunkList ) )
	{
		HO_ReadGadgetStates( cw->cw_sw.sw_hos, gadlayout );

		if( dn = (struct DataNode *)FindNthNode( &cw->cw_ChunkList,
			gadlayout[ GAD_CHUNKLIST ].ho_Value ) )
		{
			cnk = (struct Chunk *)dn->dn_Data;

			if( cnk->ch_Edit )
				(*cnk->ch_Edit)(cnk);
		}
	}
}


static VOID cw_FreeChunk( struct ChunkWindow *cw )
{
	struct DataNode *dn;
	struct Chunk *cnk;
	BOOL oktokill;

	oktokill = FALSE;

	if( !IsListEmpty( &cw->cw_ChunkList ) )
	{
		HO_ReadGadgetStates( cw->cw_sw.sw_hos, gadlayout );

		if( dn = (struct DataNode *)FindNthNode( &cw->cw_ChunkList,
			gadlayout[ GAD_CHUNKLIST ].ho_Value ) )
		{
			if( cnk = (struct Chunk *)dn->dn_Data )
			{
				/* anyone still using this chunk? */
				if( !cnk->ch_LockCount )
				{
					/* check modified flag... */

					if( cnk->ch_Modified )
					{
						if( BlockAllSuperWindows() )
						{
							oktokill = PlebReq( cw->cw_sw.sw_Window, "Zonk Error", "Chunk has been modified - Free anyway?", "Ok|Cancel" );
						}
						UnblockAllSuperWindows();
					}
					else
						oktokill = TRUE;

					if( oktokill )
					{
						Remove( &cnk->ch_Node );
						(*cnk->ch_Free)( cnk );
						RefreshChunkListings();
					}
				}
				else
				{
					if( BlockAllSuperWindows() )
						PlebReq( cw->cw_sw.sw_Window, "Zonk Error", "Chunk still in use!", "Doh!" );
					UnblockAllSuperWindows();
				}
			}
		}
	}
}


void NewChunk( struct SuperWindow *sw )
{
	struct Chunk *cnk = NULL;
	ULONG id;

	if( BlockAllSuperWindows() )
		id = RequestChunktype( sw->sw_Window, "Select new chunk type" );
	UnblockAllSuperWindows();

	switch( id )
	{
		case ID_PROG:
			cnk = CreatePROGChunk(NULL);
			break;
		case ID_DFRM:
			cnk = CreateDFRMChunk(NULL);
			break;
		case ID_WEAP:
			cnk = CreateWEAPChunk(NULL);
			break;
		case ID_TWAV:
			cnk = CreateTWAVChunk(NULL);
			break;
		case ID_SFX:
			cnk = CreateSFXChunk(NULL);
			break;
		case ID_THAP:
			cnk = CreateTHAPChunk(NULL);
			break;
		case ID_SWAV:
			cnk = CreateSWAVChunk(NULL);
			break;
		case ID_GAME:
			cnk = CreateGAMEChunk(NULL);
			break;
		case ID_DISP:
			cnk = CreateDISPChunk(NULL);
			break;
		case ID_EOTR:
			cnk = CreateEOTRChunk(NULL);
			break;
	}

	if( cnk )
	{
		AddTail( &chunklist, &cnk->ch_Node );
		RefreshChunkListings();
	}
}


/************* OBSOLETE ****************/

static VOID HandleMenuPick( struct ChunkWindow *cw, UWORD menunum )
{
	BOOL stop;
	struct MenuItem *item;
	ULONG id;
	UWORD menuid;


	stop = FALSE;
	while( menunum != MENUNULL && !stop )
	{
		item = ItemAddress( cw->cw_sw.sw_ProjectMenu, menunum );
//		printf(":%ld\n", (ULONG)GTMENUITEM_USERDATA( item ) );
		id = ( ULONG )GTMENUITEM_USERDATA( item );
		menuid = (UWORD)( id >> 16 );

		switch( menuid )
		{
			case PROJMENU:
				stop = HandleProjectMenu( &cw->cw_sw, id );
				break;
			case WINMENU:
				stop = HandleWindowMenu( &cw->cw_sw, id );
				break;
			case SETMENU:
				stop = HandleSettingsMenu( &cw->cw_sw, id );
				break;
		}
		menunum = item->NextSelect;
	}
}


void LoadChunks( struct SuperWindow *sw )
{
	int i;

	if( BlockAllSuperWindows() )
	{
		if( AslRequestTags( filereq,
			ASLFR_Window, sw->sw_Window,
			ASLFR_TitleText, "Load Chunks",
			ASLFR_DoPatterns, TRUE,
			ASLFR_InitialPattern, "~(#?.info|#?.bak)",
			ASLFR_DoMultiSelect, TRUE,
			ASLFR_DoSaveMode, FALSE,
			TAG_DONE ) )
		{
			for( i=0; i< filereq->fr_NumArgs; i++ )
				LoadFile( filereq->fr_Drawer, filereq->fr_ArgList[i].wa_Name );
			RefreshChunkListings();
		}
		UnblockAllSuperWindows();
	}
}
