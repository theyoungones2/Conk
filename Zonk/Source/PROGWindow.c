/********************************************************/
//
// PROGWindow.c
//
/********************************************************/

#define PROGWINDOW_C

#include <stdio.h>
#include <string.h>
#include <stdarg.h>

#include <dos/dos.h>
#include <dos/dostags.h>
#include <exec/exec.h>
#include <intuition/intuition.h>
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
//#include <proto/iffparse.h>

#include "hoopy.h"
#include "global.h"
#include "MenuID.h"


struct PROGWindow
{
	struct SuperWindow	pgw_sw;
	struct Chunk				*pgw_Chunk;
	UWORD								pgw_Mode;					/* select or edit */
	UWORD								pgw_lastclicknum;
	ULONG								pgw_lastclicksecs;
	ULONG								pgw_lastclickmicros;

	/* program selection mode stuff */
	struct HoopyObject	*pgw_SelectLayout;

	/* edit mode stuff */
	struct HoopyObject	*pgw_EditLayout;
	struct Program			*pgw_CurrentProg;
	char								pgw_OpListTitle[32];
	struct List					pgw_OpList;
};

#define PGW_SELECTMODE 0
#define PGW_EDITMODE 1



static void IDCMPHandler( struct SuperWindow *sw,
	struct IntuiMessage *imsg );
static void HideWindow( struct SuperWindow *sw );
static BOOL ShowWindow( struct SuperWindow *sw );
static void KillWindow( struct SuperWindow *sw );
static BOOL RefreshMenus( struct SuperWindow *sw );
static UBYTE *MakePROGWinTitle( struct SuperWindow *sw );
static VOID InitSelectMode( struct PROGWindow *pgw );
static VOID InitEditMode( struct PROGWindow *pgw, UWORD prognum );
static VOID ExitEditMode( struct PROGWindow *pgw );
static BOOL PGW_AddNewProgram( struct PROGWindow *pgw );
static VOID PGW_DeleteProgram( struct PROGWindow *pgw );
static VOID PGW_BubbleProgram( struct PROGWindow *pgw, WORD dir );
static VOID PGW_RenameProgram( struct PROGWindow *pgw );
static VOID PGW_CopyProgram( struct PROGWindow *pgw );

static VOID RefreshOperatorList( struct PROGWindow *pgw );
static VOID PGW_EditOperator( struct PROGWindow *pgw );
static VOID PGW_DeleteOperator( struct PROGWindow *pgw );
static VOID PGW_AddOperator( struct PROGWindow *pgw );
static VOID PGW_InsertOperator( struct PROGWindow *pgw );
static VOID PGW_UpOperator( struct PROGWindow *pgw );
static VOID PGW_DownOperator( struct PROGWindow *pgw );

static VOID HandleMenuPick( struct PROGWindow *pgw, UWORD menunum );


#define SGAD_GROUP0		0
#define SGAD_PROGLIST	1
//#define SGAD_PROGNAME	2
#define SGAD_GROUP1		2
#define SGAD_EDIT			3
#define SGAD_NEW			4
#define SGAD_DEL			5
#define SGAD_RENAME		6
#define SGAD_COPY			7
#define SGAD_UP				8
#define SGAD_DN				9

char buf[16];

static struct HoopyObject selectgadlayout[] =
{
	{ HOTYPE_VGROUP, 100, HOFLG_RAISEDBORDER|HOFLG_FREEWIDTH|HOFLG_FREEHEIGHT,2,0,0,0, NULL, 0,SGAD_GROUP0,0,0,0 },
		{ HOTYPE_LISTVIEW, 200, HOFLG_NOBORDER|HOFLG_FREEWIDTH|HOFLG_FREEHEIGHT,NULL,0,0,0,"ActionLists",0,SGAD_PROGLIST,PLACETEXT_ABOVE,0 },
		{ HOTYPE_HGROUP, 100, HOFLG_FREEWIDTH,7,0,0,0, NULL, 0,SGAD_GROUP1,0,0,0 },
			{ HOTYPE_BUTTON, 100, HOFLG_NOBORDER, 0,0,0,0, "Edit",0,SGAD_EDIT,0,0 },
			{ HOTYPE_BUTTON, 100, HOFLG_NOBORDER, 0,0,0,0, "New",0,SGAD_NEW,0,0 },
			{ HOTYPE_BUTTON, 100, HOFLG_NOBORDER, 0,0,0,0, "Del",0,SGAD_DEL,0,0 },
			{ HOTYPE_BUTTON, 100, HOFLG_NOBORDER, 0,0,0,0, "Rename",0,SGAD_RENAME,0,0 },
			{ HOTYPE_BUTTON, 100, HOFLG_NOBORDER, 0,0,0,0, "Copy",0,SGAD_COPY,0,0 },
			{ HOTYPE_BUTTON, 100, HOFLG_NOBORDER, 0,0,0,0, "Up",0,SGAD_UP,0,0 },
			{ HOTYPE_BUTTON, 100, HOFLG_NOBORDER, 0,0,0,0, "Dn",0,SGAD_DN,0,0 },
	{HOTYPE_END}
};

#define EGAD_GROUP0		0
#define EGAD_OPLIST		1
#define EGAD_GROUP1		2
#define EGAD_EDIT			3
#define EGAD_ADD			4
#define EGAD_INSERT		5
#define EGAD_DELETE		6
#define EGAD_UP				7
#define EGAD_DOWN			8
#define EGAD_SELECT		9



static struct HoopyObject editgadlayout[] =
{
	{ HOTYPE_VGROUP, 100, HOFLG_FREEWIDTH|HOFLG_FREEHEIGHT,3,0,0,0, NULL, 0,EGAD_GROUP0,0,0,0 },
		{ HOTYPE_LISTVIEW, 200, HOFLG_NOBORDER|HOFLG_FREEWIDTH|HOFLG_FREEHEIGHT,NULL,0,0,0,"ActionList",0,EGAD_OPLIST,PLACETEXT_ABOVE,0 },
		{ HOTYPE_HGROUP, 100, HOFLG_NOBORDER|HOFLG_FREEWIDTH,6,0,0,0, NULL, 0,EGAD_GROUP1,0,0,0 },
			{ HOTYPE_BUTTON, 100, HOFLG_NOBORDER, 0,0,0,0, "Edit",0,EGAD_EDIT,0,0 },
			{ HOTYPE_BUTTON, 100, HOFLG_NOBORDER, 0,0,0,0, "Add",0,EGAD_ADD,0,0 },
			{ HOTYPE_BUTTON, 100, HOFLG_NOBORDER, 0,0,0,0, "Ins",0,EGAD_INSERT,0,0 },
			{ HOTYPE_BUTTON, 100, HOFLG_NOBORDER, 0,0,0,0, "Del",0,EGAD_DELETE,0,0 },
			{ HOTYPE_BUTTON, 100, HOFLG_NOBORDER, 0,0,0,0, "Up",0,EGAD_UP,0,0 },
			{ HOTYPE_BUTTON, 100, HOFLG_NOBORDER, 0,0,0,0, "Down",0,EGAD_DOWN,0,0 },
		{ HOTYPE_BUTTON, 100, HOFLG_NOBORDER, 0,0,0,0, "Done",0,EGAD_SELECT,0,0 },


	{HOTYPE_END}
};



BOOL OpenNewPROGWindow( struct Chunk *cnk )
{
	struct SuperWindow *sw;
	struct PROGWindow *pgw;
	BOOL success = FALSE;

	assert( cnk != NULL );

	LockChunk( cnk );

	if( pgw = AllocVec( sizeof( struct PROGWindow ), MEMF_ANY|MEMF_CLEAR ) )
	{
		sw = &pgw->pgw_sw;
		sw->sw_Handler = IDCMPHandler;
		sw->sw_HideWindow = HideWindow;
		sw->sw_ShowWindow = ShowWindow;
		sw->sw_BuggerOff = KillWindow;
		sw->sw_RefreshMenus = RefreshMenus;
		sw->sw_MakeTitle = MakePROGWinTitle;

		sw->sw_Type				= SWTY_PROG;
		sw->sw_Window			= NULL;
//		sw->sw_MenuStrip	= NULL;
		sw->sw_Gadgets		= NULL;
		sw->sw_VisualInfo	= NULL;
		sw->sw_WinDim			= zonkfig.SuperWindowDefs[ SWTY_PROG ];

		pgw->pgw_Chunk = cnk;
		/* make copies of the two hoopyvision object layout arrays */
		/* and set them up... */
		pgw->pgw_SelectLayout = AllocVec( sizeof( selectgadlayout ), MEMF_ANY );
		pgw->pgw_EditLayout = AllocVec( sizeof( editgadlayout ), MEMF_ANY );
		if( pgw->pgw_SelectLayout && pgw->pgw_EditLayout )
		{
			CopyMem( selectgadlayout, pgw->pgw_SelectLayout, sizeof( selectgadlayout ) );
			CopyMem( editgadlayout, pgw->pgw_EditLayout, sizeof( editgadlayout ) );
//			NewList( &pgw->pgw_ProgramList );
			NewList( &pgw->pgw_OpList );
			success = TRUE;
		}

		if( success )
		{
			(*sw->sw_ShowWindow)( sw );
			AddTail( &superwindows, &sw->sw_Node );
		}
		else
		{
			if( pgw->pgw_SelectLayout ) FreeVec( pgw->pgw_SelectLayout );
			if( pgw->pgw_EditLayout ) FreeVec( pgw->pgw_EditLayout );
			FreeVec( pgw );
		}
	}
	return( success );
}



static BOOL ShowWindow( struct SuperWindow *sw )
{
	BOOL success = FALSE;
	struct PROGWindow *pgw;
	struct Window *win;
	struct Screen *scr;

	assert( sw != NULL );

	pgw = (struct PROGWindow *)sw;

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
				IDCMP_CLOSEWINDOW|IDCMP_CHANGEWINDOW|
				IDCMP_MENUPICK|LISTVIEWIDCMP|BUTTONIDCMP );

			InitSelectMode( pgw );

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
	struct PROGWindow *pgw;

	assert( sw != NULL );

	pgw = (struct PROGWindow *)sw;

	if( sw->sw_Window )
	{
		if( pgw->pgw_Mode == PGW_EDITMODE )
		{
			ExitEditMode( pgw );
		}


		RememberWindow( sw );
		GenericZapMenus( sw );

//		HO_ReadGadgetStates( sc->sc_hos, gadlayout );
		HO_KillLayout( sw->sw_hos );
		CloseWindowSafely( sw->sw_Window );
		HO_FreeHOStuff( sw->sw_hos );
		sw->sw_Window = NULL;

//		FreeProgramList( &pgw->pgw_ProgramList );
		FreeOperatorList( &pgw->pgw_OpList );
	}
}


static void KillWindow( struct SuperWindow *sw )
{
	struct PROGWindow *pgw;

	assert( sw != NULL );

	if( IsOKToClose( sw ) )
	{
		pgw = (struct PROGWindow *)sw;
		(*sw->sw_HideWindow)(sw);
		Remove( &sw->sw_Node );
		if( pgw->pgw_SelectLayout ) FreeVec( pgw->pgw_SelectLayout );
		if( pgw->pgw_EditLayout ) FreeVec( pgw->pgw_EditLayout );
		UnlockChunk( pgw->pgw_Chunk );
		FreeVec( sw );
	}
}

static BOOL RefreshMenus( struct SuperWindow *sw )
{
	return( FALSE );
}


static UBYTE *MakePROGWinTitle( struct SuperWindow *sw )
{
	struct Chunk *cnk;
	UBYTE *p;

	cnk = ((struct PROGWindow *)sw)->pgw_Chunk;
	p = sw->sw_Name;

	if( cnk->ch_Modified )
		*p++ = MODCHAR;

	strcpy( p,"ActionLists" );

	return( sw->sw_Name );
}


/********************************************************/

static void IDCMPHandler( struct SuperWindow *sw,
	struct IntuiMessage *origimsg )
{
	struct IntuiMessage *imsg;
	struct Window *win;
	struct PROGWindow *pgw;
	struct Chunk *cnk;

	ULONG class;
	UWORD code;
//	UWORD qualifier;
	APTR iaddress;
//	WORD mousex, mousey;
	ULONG seconds, micros;

	pgw = (struct PROGWindow *)sw;
	cnk = pgw->pgw_Chunk;
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

				if( pgw->pgw_Mode == PGW_EDITMODE )
				{
					ExitEditMode( pgw );
					InitSelectMode( pgw );
				}
				else
					KillWindow( sw );
				break;
			case IDCMP_NEWSIZE:
				ClearWindow( win );
				switch( pgw->pgw_Mode )
				{
					case PGW_SELECTMODE:
						HO_ReadGadgetStates( sw->sw_hos, pgw->pgw_SelectLayout );
						HO_MakeLayout( pgw->pgw_SelectLayout, sw->sw_hos,
							win->BorderLeft, win->BorderTop,
							win->Width - win->BorderLeft - win->BorderRight,
							win->Height - win->BorderTop - win->BorderBottom );
						break;

					case PGW_EDITMODE:
						HO_ReadGadgetStates( sw->sw_hos, pgw->pgw_EditLayout );
						HO_MakeLayout( pgw->pgw_EditLayout, sw->sw_hos,
							win->BorderLeft, win->BorderTop,
							win->Width - win->BorderLeft - win->BorderRight,
							win->Height - win->BorderTop - win->BorderBottom );
						break;
				}
				ReplyMsg( (struct Message *)origimsg );
				break;
			case IDCMP_CHANGEWINDOW:
				UpdateWinDef( sw );
				ReplyMsg( (struct Message *)origimsg );
				break;
			case IDCMP_GADGETUP:
				ReplyMsg( (struct Message *)origimsg );
				switch( pgw->pgw_Mode )
				{
					case PGW_SELECTMODE:
						switch( ((struct Gadget*)iaddress)->GadgetID )
						{
							case SGAD_EDIT:
								HO_ReadGadgetStates( sw->sw_hos, pgw->pgw_SelectLayout );
								InitEditMode( pgw,
									pgw->pgw_SelectLayout[ SGAD_PROGLIST ].ho_Value );
								break;
							case SGAD_NEW:
								/* create new program */
								PGW_AddNewProgram( pgw );
								ModifyChunk( cnk );
								break;
							case SGAD_DEL:
								/* kill program */
								PGW_DeleteProgram( pgw );
								ModifyChunk( cnk );
								break;
							case SGAD_RENAME:
								PGW_RenameProgram( pgw );
								ModifyChunk( cnk );
								break;
							case SGAD_COPY:
								PGW_CopyProgram( pgw );
								ModifyChunk( cnk );
								break;
							case SGAD_UP:
								PGW_BubbleProgram( pgw, -1 );
								ModifyChunk( cnk );
								break;
							case SGAD_DN:
								PGW_BubbleProgram( pgw, 1 );
								ModifyChunk( cnk );
								break;
							case SGAD_PROGLIST:
								if( pgw->pgw_lastclicknum != code )
								{
									pgw->pgw_lastclicknum = code;
									pgw->pgw_lastclicksecs = 0;
									pgw->pgw_lastclickmicros = 0;
								}

								if( DoubleClick( pgw->pgw_lastclicksecs, pgw->pgw_lastclickmicros, seconds, micros ) )
								{
									/* did a doubleclick */
									InitEditMode( pgw, code );
									pgw->pgw_lastclicksecs = 0;
									pgw->pgw_lastclickmicros = 0;
								}
								else
								{
									/* 'tis just a singleclick */
									pgw->pgw_lastclicksecs = seconds;
									pgw->pgw_lastclickmicros = micros;
								}
								break;
						}
						break;
					case PGW_EDITMODE:
						switch( ((struct Gadget*)iaddress)->GadgetID )
						{
							case EGAD_SELECT:
								ExitEditMode( pgw );
								InitSelectMode( pgw );
								break;
							case EGAD_EDIT:
								PGW_EditOperator( pgw );
								ModifyChunk( cnk );
								break;
							case EGAD_ADD:
								PGW_AddOperator( pgw );
								ModifyChunk( cnk );
								break;
							case EGAD_INSERT:
								PGW_InsertOperator( pgw );
								ModifyChunk( cnk );
								break;
							case EGAD_DELETE:
								PGW_DeleteOperator( pgw );
								ModifyChunk( cnk );
								break;
							case EGAD_UP:
								PGW_UpOperator( pgw );
								ModifyChunk( cnk );
								break;
							case EGAD_DOWN:
								PGW_DownOperator( pgw );
								ModifyChunk( cnk );
								break;
							case EGAD_OPLIST:
								if( pgw->pgw_lastclicknum != code )
								{
									pgw->pgw_lastclicknum = code;
									pgw->pgw_lastclicksecs = 0;
									pgw->pgw_lastclickmicros = 0;
								}

								if( DoubleClick( pgw->pgw_lastclicksecs, pgw->pgw_lastclickmicros, seconds, micros ) )
								{
									/* did a doubleclick */
									PGW_EditOperator( pgw );
									ModifyChunk( cnk );
									pgw->pgw_lastclicksecs = 0;
									pgw->pgw_lastclickmicros = 0;
								}
								else
								{
									/* 'tis just a singleclick */
									pgw->pgw_lastclicksecs = seconds;
									pgw->pgw_lastclickmicros = micros;
								}
								break;

						}
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

static VOID RefreshOperatorList( struct PROGWindow *pgw )
{
	/* detach old list */
	pgw->pgw_EditLayout[ EGAD_OPLIST ].ho_Attr0 = ~0;
	HO_RefreshObject( &pgw->pgw_EditLayout[ EGAD_OPLIST ], pgw->pgw_sw.sw_hos );
	/* remake list and redisplay */
	MakeOperatorList( pgw->pgw_CurrentProg, &pgw->pgw_OpList );
	pgw->pgw_EditLayout[ EGAD_OPLIST ].ho_Attr0 = (ULONG)&pgw->pgw_OpList;
	HO_RefreshObject( &pgw->pgw_EditLayout[ EGAD_OPLIST ], pgw->pgw_sw.sw_hos );
}




/********************************************************/

static VOID InitSelectMode( struct PROGWindow *pgw )
{
	struct Window *win;

//	HO_SussWindowSizing( pgw->pgw_sw.sw_hos, pgw->pgw_SelectLayout );

	pgw->pgw_Mode = PGW_SELECTMODE;

	win = pgw->pgw_sw.sw_Window;
	HO_KillLayout( pgw->pgw_sw.sw_hos );
	ClearWindow( win );

//	MakeProgramList( pgw->pgw_Chunk, &pgw->pgw_ProgramList );
//	pgw->pgw_SelectLayout[ SGAD_PROGLIST ].ho_Attr0 = (ULONG)&pgw->pgw_ProgramList;
	pgw->pgw_SelectLayout[ SGAD_PROGLIST ].ho_Attr0 = (ULONG)&pgw->pgw_Chunk->ch_DataList;

	HO_SussWindowSizing( pgw->pgw_sw.sw_hos, pgw->pgw_SelectLayout );

	HO_MakeLayout( pgw->pgw_SelectLayout, pgw->pgw_sw.sw_hos,
		win->BorderLeft, win->BorderTop,
		win->Width - win->BorderLeft - win->BorderRight,
		win->Height - win->BorderTop - win->BorderBottom );
}

/********************************************************/

static VOID InitEditMode( struct PROGWindow *pgw, UWORD prognum )
{
	struct Window *win;
	struct Program *prog;

	if( prognum != ~0 && !IsListEmpty( &pgw->pgw_Chunk->ch_DataList ) )
	{
		for( prog =
				(struct Program *)pgw->pgw_Chunk->ch_DataList.lh_Head;
			prognum;
			--prognum )
		{
			prog = (struct Program *)prog->pg_Node.ln_Succ;
		}

		if( !prog->pg_Locked )
		{

			pgw->pgw_CurrentProg = prog;

			pgw->pgw_Mode = PGW_EDITMODE;

//			printf("LockProg: %s\n",pgw->pgw_CurrentProg->pg_Name );
			pgw->pgw_CurrentProg->pg_Locked = TRUE;

			win = pgw->pgw_sw.sw_Window;
			HO_KillLayout( pgw->pgw_sw.sw_hos );
			ClearWindow( win );

			sprintf( pgw->pgw_OpListTitle, "Listing of '%s'",
				((struct Program *)pgw->pgw_CurrentProg)->pg_Name );
			MakeOperatorList( pgw->pgw_CurrentProg, &pgw->pgw_OpList );
			pgw->pgw_EditLayout[ EGAD_OPLIST ].ho_Attr0 = (ULONG)&pgw->pgw_OpList;
			pgw->pgw_EditLayout[ EGAD_OPLIST ].ho_Text = pgw->pgw_OpListTitle;

			HO_SussWindowSizing( pgw->pgw_sw.sw_hos, pgw->pgw_EditLayout );

			HO_MakeLayout( pgw->pgw_EditLayout, pgw->pgw_sw.sw_hos,
				win->BorderLeft, win->BorderTop,
				win->Width - win->BorderLeft - win->BorderRight,
				win->Height - win->BorderTop - win->BorderBottom );
		}
		else
		{
			if( BlockAllSuperWindows() )
			{
				PlebReq( pgw->pgw_sw.sw_Window, "Zonk Error",
					"ActionList is already being edited", "Doh!" );
			}
			UnblockAllSuperWindows();
		}
	}
}

/********************************************************/
// Must be paired with InitEditMode()!

static VOID ExitEditMode( struct PROGWindow *pgw )
{
	if( pgw->pgw_CurrentProg )
	{
//		printf("UnlockProg: %s\n",pgw->pgw_CurrentProg->pg_Name );
		pgw->pgw_CurrentProg->pg_Locked = FALSE;
	}
}



/********************************************************/

static BOOL PGW_AddNewProgram( struct PROGWindow *pgw )
{
	char newname[ PROGNAMESIZE ];
	struct Chunk *cnk;
	BOOL yepflag = FALSE;

	Mystrncpy( newname, defname, PROGNAMESIZE-1 );

	cnk = pgw->pgw_Chunk;

	/* detach proglist from listview */
	pgw->pgw_SelectLayout[ SGAD_PROGLIST ].ho_Attr0 = ~0;
	HO_RefreshObject( &pgw->pgw_SelectLayout[ SGAD_PROGLIST ], pgw->pgw_sw.sw_hos );

	if( BlockAllSuperWindows() )
	{
		if( MyPackedStringRequest( pgw->pgw_sw.sw_Window, "New ActionList", "Name:", newname, PROGNAMESIZE-1 ) )
		{
			/* maybe do some name-checking here... */

			if( AddNewProgram( cnk, newname ) )
				yepflag = TRUE;
		}
		UnblockAllSuperWindows();
	}

	/* redisplay proglist */
	pgw->pgw_SelectLayout[ SGAD_PROGLIST ].ho_Attr0 = (ULONG)&cnk->ch_DataList;
	HO_RefreshObject( &pgw->pgw_SelectLayout[ SGAD_PROGLIST ], pgw->pgw_sw.sw_hos );

	return( yepflag );
}




/********************************************************/
// Delete the program currently highlighted in the ProgList
// listview.

static VOID PGW_DeleteProgram( struct PROGWindow *pgw )
{
	struct Chunk *cnk;
	struct Program *prog;
	UWORD num;
	BOOL wibble = TRUE;
	struct EasyStruct	easy;

	easy.es_StructSize	= sizeof(struct EasyStruct);
	easy.es_Flags		= NULL;
	easy.es_Title		= "ZONK";
	easy.es_TextFormat	= "Delete ActionList - Are you sure?";
	easy.es_GadgetFormat	= "Yep|Nope";

//	printf("PGW_DeleteProgram()\n");
	HO_ReadGadgetStates( pgw->pgw_sw.sw_hos, pgw->pgw_SelectLayout );
	cnk = pgw->pgw_Chunk;

	num = (UWORD)pgw->pgw_SelectLayout[ SGAD_PROGLIST ].ho_Value;

	if( !IsListEmpty( &cnk->ch_DataList ) && num != ~0 )
	{

//		printf("num: %d\n",num);

		if( prog = (struct Program *)FindNthNode( &cnk->ch_DataList, num ) )
		{

			if( !prog->pg_Locked )
			{

				/* detach proglist from listview */
				pgw->pgw_SelectLayout[ SGAD_PROGLIST ].ho_Attr0 = ~0;
				HO_RefreshObject( &pgw->pgw_SelectLayout[ SGAD_PROGLIST ], pgw->pgw_sw.sw_hos );


				if( !IsListEmpty( (struct List *)&prog->pg_OpList ) )
					wibble = EasyRequestArgs( pgw->pgw_sw.sw_Window, &easy, 0, NULL );

				if( wibble )
				{
					Remove( &prog->pg_Node );
					FreeProgram( prog );
				}

				/* redisplay proglist */
				pgw->pgw_SelectLayout[ SGAD_PROGLIST ].ho_Attr0 = (ULONG)&cnk->ch_DataList;
				HO_RefreshObject( &pgw->pgw_SelectLayout[ SGAD_PROGLIST ], pgw->pgw_sw.sw_hos );
			}
			else
			{
				if( BlockAllSuperWindows() )
				{
					PlebReq( pgw->pgw_sw.sw_Window, "Zonk Error",
						"ActionList is in use", "Doh!" );
				}
				UnblockAllSuperWindows();
			}
		}
	}
}


/********************************************************/
// Rename the program currently highlighted in the ProgList
// listview.

static VOID PGW_RenameProgram( struct PROGWindow *pgw )
{
	struct Chunk *cnk;
	struct Program *prog;
	UWORD num;
	char namebuf[ PROGNAMESIZE ];

	HO_ReadGadgetStates( pgw->pgw_sw.sw_hos, pgw->pgw_SelectLayout );
	cnk = pgw->pgw_Chunk;

	num = (UWORD)pgw->pgw_SelectLayout[ SGAD_PROGLIST ].ho_Value;

	if( !IsListEmpty( &cnk->ch_DataList ) && num != ~0 )
	{

		if( prog = (struct Program *)FindNthNode( &cnk->ch_DataList, num ) )
		{
			if( !prog->pg_Locked )
			{

				/* detach proglist from listview */
				pgw->pgw_SelectLayout[ SGAD_PROGLIST ].ho_Attr0 = ~0;
				HO_RefreshObject( &pgw->pgw_SelectLayout[ SGAD_PROGLIST ], pgw->pgw_sw.sw_hos );

				Mystrncpy( namebuf, prog->pg_Name, PROGNAMESIZE-1 );

				if( BlockAllSuperWindows() )
				{
					if( MyPackedStringRequest( pgw->pgw_sw.sw_Window, "Rename ActionList", "Name:", namebuf, PROGNAMESIZE-1 ) )
					{
						MakePackable( namebuf );
						Mystrncpy( prog->pg_Name, namebuf, PROGNAMESIZE-1 );
					}
					UnblockAllSuperWindows();
				}

				/* redisplay proglist */
				pgw->pgw_SelectLayout[ SGAD_PROGLIST ].ho_Attr0 = (ULONG)&cnk->ch_DataList;
				HO_RefreshObject( &pgw->pgw_SelectLayout[ SGAD_PROGLIST ], pgw->pgw_sw.sw_hos );
			}
			else
			{
				if( BlockAllSuperWindows() )
				{
					PlebReq( pgw->pgw_sw.sw_Window, "Zonk Error",
						"ActionList is being edited", "Doh!" );
				}
				UnblockAllSuperWindows();
			}
		}
	}
}


/********************************************************/

static VOID PGW_CopyProgram( struct PROGWindow *pgw )
{
	struct Chunk *cnk;
	struct Program *srcprog, *destprog;

	cnk = pgw->pgw_Chunk;

	if( BlockAllSuperWindows() )
	{
		srcprog = RequestProgram( pgw->pgw_sw.sw_Window, NULL, "Copy Actionlist...",
			"Actionlists" );
	}
	UnblockAllSuperWindows();

	if( srcprog )
	{
		if( destprog = CopyProgram( srcprog ) )
		{
			/* detach proglist from listview */
			pgw->pgw_SelectLayout[ SGAD_PROGLIST ].ho_Attr0 = ~0;
			HO_RefreshObject( &pgw->pgw_SelectLayout[ SGAD_PROGLIST ], pgw->pgw_sw.sw_hos );

			AddTail( &cnk->ch_DataList, (struct Node *)destprog );

			/* redisplay proglist */
			pgw->pgw_SelectLayout[ SGAD_PROGLIST ].ho_Attr0 = (ULONG)&cnk->ch_DataList;
			HO_RefreshObject( &pgw->pgw_SelectLayout[ SGAD_PROGLIST ], pgw->pgw_sw.sw_hos );
		}
	}
}



/********************************************************/
// Bubble the currently selected program up or down the list
// (-1 = up, 1=down )

static VOID PGW_BubbleProgram( struct PROGWindow *pgw, WORD dir )
{
	struct Chunk *cnk;
	struct Program *prog;
	UWORD num;

	HO_ReadGadgetStates( pgw->pgw_sw.sw_hos, pgw->pgw_SelectLayout );
	cnk = pgw->pgw_Chunk;

	num = (UWORD)pgw->pgw_SelectLayout[ SGAD_PROGLIST ].ho_Value;

	if( !IsListEmpty( &cnk->ch_DataList ) && num != ~0 )
	{

		if( prog = (struct Program *)FindNthNode( &cnk->ch_DataList, num ) )
		{
			/* detach proglist from listview */
			pgw->pgw_SelectLayout[ SGAD_PROGLIST ].ho_Attr0 = ~0;
			HO_RefreshObject( &pgw->pgw_SelectLayout[ SGAD_PROGLIST ], pgw->pgw_sw.sw_hos );

			switch( dir )
			{
				case -1:
					if( BubbleUp( &cnk->ch_DataList, &prog->pg_Node ) )
						pgw->pgw_SelectLayout[ SGAD_PROGLIST ].ho_Value--;
					break;
				case 1:
					if( BubbleDown( &cnk->ch_DataList, &prog->pg_Node ) )
						pgw->pgw_SelectLayout[ SGAD_PROGLIST ].ho_Value++;
					break;
			}

			/* redisplay proglist */
			pgw->pgw_SelectLayout[ SGAD_PROGLIST ].ho_Attr0 = (ULONG)&cnk->ch_DataList;
			HO_RefreshObject( &pgw->pgw_SelectLayout[ SGAD_PROGLIST ], pgw->pgw_sw.sw_hos );
		}
	}
}


/********************************************************/
// Bring up an EditOperator() requester to edit the currently
// highlighted operator.

static VOID PGW_EditOperator( struct PROGWindow *pgw )
{
	struct OpNode *on;
	struct DataNode *dn;
	UBYTE buf[512];

	/* make sure there actually are some operators */

	if( !IsListEmpty( &pgw->pgw_OpList ) )
	{
		/* make sure the HoopyObject array values are up to date */
		HO_ReadGadgetStates( pgw->pgw_sw.sw_hos, pgw->pgw_EditLayout );

		if( dn = (struct DataNode *)FindNthNode( &pgw->pgw_OpList,
			pgw->pgw_EditLayout[ EGAD_OPLIST ].ho_Value ) )
		{
			if( BlockAllSuperWindows() )
			{
				on = dn->dn_Data;
				/* do it! */
				EditOperator( pgw->pgw_sw.sw_Window,
					((struct OpNode *)dn->dn_Data)->on_OpData );

				UnblockAllSuperWindows();

				/* detach oplist from listview */
				pgw->pgw_EditLayout[ EGAD_OPLIST ].ho_Attr0 = ~0;
				HO_RefreshObject( &pgw->pgw_EditLayout[ EGAD_OPLIST ], pgw->pgw_sw.sw_hos );

				FreeVec( dn->dn_Node.ln_Name );
				DescribeOperator( buf, on->on_OpData );
				dn->dn_Node.ln_Name = MyStrDup( buf );

				/* redisplay oplist */
				pgw->pgw_EditLayout[ EGAD_OPLIST ].ho_Attr0 = (ULONG)&pgw->pgw_OpList;
				HO_RefreshObject( &pgw->pgw_EditLayout[ EGAD_OPLIST ], pgw->pgw_sw.sw_hos );

			}
		}
	}
}


/********************************************************/
// Delete the currently highlighted operator.

static VOID PGW_DeleteOperator( struct PROGWindow *pgw )
{
	struct OpNode *on;
	struct DataNode *dn;
//	struct EasyStruct es = { sizeof( struct EasyStruct ), 0 ,
//		"Delete line","Are you sure?","Yep|Nope" };
	UWORD foo;

	/* make sure there actually are some operators */

	if( !IsListEmpty( &pgw->pgw_OpList ) )
	{

		/* make sure the HoopyObject array values are up to date */
		HO_ReadGadgetStates( pgw->pgw_sw.sw_hos, pgw->pgw_EditLayout );

		if( BlockAllSuperWindows() )
		{
//			foo = EasyRequest( pgw->pgw_sw.sw_Window, &es, NULL );
			foo = 1;
			UnblockAllSuperWindows();

			if( foo )
			{
				/* find the currently selected node */
				if( dn = (struct DataNode *)FindNthNode( &pgw->pgw_OpList,
					pgw->pgw_EditLayout[ EGAD_OPLIST ].ho_Value ) )
				{
					/* see which opnode it relates to */
					on = dn->dn_Data;
					Remove( (struct Node *)on );
					FreeOperatorData( on->on_OpData );
					FreeVec( on );
					RefreshOperatorList( pgw );
				}
			}
		}
	}
}



/********************************************************/

static VOID PGW_AddOperator( struct PROGWindow *pgw )
{
	struct OpNode *noddy;
	UWORD op;

	if( BlockAllSuperWindows() )
	{
//		op = MyOperatorRequest( pgw->pgw_sw.sw_Window, OPTYPE_ACTION );
		op = OpReq( pgw->pgw_sw.sw_Window, OPTYPE_ACTION );
		if( op != 0xFFFF )
		{
//			InsertOperator( pgw->pgw_CurrentProg, op,
//				CountNodes( (struct List *)&pgw->pgw_CurrentProg->pg_OpList ) );

			if( noddy = AllocOperator( op ) )
			{
				AddTail( (struct List *)&pgw->pgw_CurrentProg->pg_OpList,
					(struct Node *)noddy );
				EditOperator( pgw->pgw_sw.sw_Window, noddy->on_OpData );
			}
			RefreshOperatorList( pgw );
		}
		UnblockAllSuperWindows();
	}
}


/********************************************************/

static VOID PGW_InsertOperator( struct PROGWindow *pgw )
{
	UWORD op;
	WORD pos;
	struct OpNode *noddy;
	struct Node *pred;

	if( IsListEmpty( &pgw->pgw_OpList ) )
		return;

	/* make sure the HoopyObject array values are up to date */
	HO_ReadGadgetStates( pgw->pgw_sw.sw_hos, pgw->pgw_EditLayout );

	if( BlockAllSuperWindows() )
	{
//		op = MyOperatorRequest( pgw->pgw_sw.sw_Window, OPTYPE_ACTION );
		op = OpReq( pgw->pgw_sw.sw_Window, OPTYPE_ACTION );
		if( op != 0xFFFF )
		{
			if( !IsListEmpty( &pgw->pgw_OpList ) )
				pos = (WORD)pgw->pgw_EditLayout[ EGAD_OPLIST ].ho_Value;
			else
				pos = 0;

			pred = FindNthNode( (struct List *)&pgw->pgw_CurrentProg->pg_OpList, pos );
			pred = pred->ln_Pred;

//			InsertOperator( pgw->pgw_CurrentProg, op, pos );

			if( noddy = AllocOperator( op ) )
			{
//				AddTail( (struct List *)&pgw->pgw_CurrentProg->pg_OpList,
//					(struct Node *)noddy );
					EditOperator( pgw->pgw_sw.sw_Window, noddy->on_OpData );

				Insert( (struct List *)&pgw->pgw_CurrentProg->pg_OpList,
					(struct Node *)noddy, pred );
			}

			RefreshOperatorList( pgw );
		}
		UnblockAllSuperWindows();
	}
}


/********************************************************/

static VOID PGW_UpOperator( struct PROGWindow *pgw )
{
	struct DataNode *dn;
	struct OpNode *on;

	if( !IsListEmpty( &pgw->pgw_OpList ) )
	{
		/* make sure the HoopyObject array values are up to date */
		HO_ReadGadgetStates( pgw->pgw_sw.sw_hos, pgw->pgw_EditLayout );

		/* find the currently selected node */
		if( dn = (struct DataNode *)FindNthNode( &pgw->pgw_OpList,
			pgw->pgw_EditLayout[ EGAD_OPLIST ].ho_Value ) )
		{
			/* see which opnode it relates to */
			on = dn->dn_Data;
			if( BubbleUp( (struct List *)&pgw->pgw_CurrentProg->pg_OpList,
				(struct Node *)on ) )
			{
				pgw->pgw_EditLayout[ EGAD_OPLIST ].ho_Value--;
				RefreshOperatorList( pgw );
			}
		}
	}
}

/********************************************************/

static VOID PGW_DownOperator( struct PROGWindow *pgw )
{
	struct DataNode *dn;
	struct OpNode *on;

	if( !IsListEmpty( &pgw->pgw_OpList ) )
	{
		/* make sure the HoopyObject array values are up to date */
		HO_ReadGadgetStates( pgw->pgw_sw.sw_hos, pgw->pgw_EditLayout );

		/* find the currently selected node */
		if( dn = (struct DataNode *)FindNthNode( &pgw->pgw_OpList,
			pgw->pgw_EditLayout[ EGAD_OPLIST ].ho_Value ) )
		{
			/* see which opnode it relates to */
			on = dn->dn_Data;
			if( BubbleDown( (struct List *)&pgw->pgw_CurrentProg->pg_OpList,
				(struct Node *)on ) )
			{
				pgw->pgw_EditLayout[ EGAD_OPLIST ].ho_Value++;
				RefreshOperatorList( pgw );
			}
		}
	}
}

/********************************************************/

static VOID HandleMenuPick( struct PROGWindow *pgw, UWORD menunum )
{
	BOOL stop;
	struct MenuItem *item;
	ULONG id;
	UWORD menuid;

	stop = FALSE;
	while( menunum != MENUNULL && !stop )
	{
		item = ItemAddress( pgw->pgw_sw.sw_ProjectMenu, menunum );
//		printf(":%ld\n", (ULONG)GTMENUITEM_USERDATA( item ) );
		id = ( ULONG )GTMENUITEM_USERDATA( item );
		menuid = (UWORD)( id >> 16 );

		switch( menuid )
		{
			case PROJMENU:
				stop = HandleProjectMenu( &pgw->pgw_sw, id );
				break;
			case WINMENU:
				stop = HandleWindowMenu( &pgw->pgw_sw, id );
				break;
			case SETMENU:
				stop = HandleSettingsMenu( &pgw->pgw_sw, id );
				break;
		}
		menunum = item->NextSelect;
	}
}

