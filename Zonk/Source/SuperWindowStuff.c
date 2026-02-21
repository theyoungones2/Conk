/********************************************************/
//
// SuperWindowStuff.c
//
/********************************************************/

#define SUPERWINDOWSTUFF_C

#include <stdio.h>
#include <string.h>
#include <stdarg.h>

#include <dos/dos.h>
#include <dos/dostags.h>
#include <exec/exec.h>
#include <intuition/intuition.h>
#include <intuition/intuitionbase.h>
//#include <graphics/gfxbase.h>
//#include <libraries/gadtools.h>
//#include <libraries/diskfont.h>
//#include <libraries/asl.h>
//#include <libraries/iffparse.h>

#include <proto/exec.h>
#include <proto/dos.h>
#include <proto/intuition.h>
//#include <proto/graphics.h>
//#include <proto/diskfont.h>
#include <proto/asl.h>
#include <clib/alib_protos.h>
//#include <proto/gadtools.h>
//#include <proto/iffparse.h>

#include "hoopy.h"
#include "global.h"


struct BlockingReqInfo
{
	struct Requester	Req;
	ULONG							OldIDCMP;
};

//extern struct IntuitionBase *IntuitionBase;


void SetWait(struct Window *win)
{
	static UWORD chip busyimage[18 * 2] =
	{
		0x0000,0x0000,

		0x0400,0x07C0,
		0x0000,0x07C0,
		0x0100,0x0380,
		0x0000,0x07E0,
		0x07C0,0x1FF8,
		0x1FF0,0x3FEC,
		0x3FF8,0x7FDE,
		0x3FF8,0x7FBE,
		0x7FFC,0xFF7F,
		0x7EFC,0xFFFF,
		0x7FFC,0xFFFF,
		0x3FF8,0x7FFE,
		0x3FF8,0x7FFE,
		0x1FF0,0x3FFC,
		0x07C0,0x1FF8,
		0x0000,0x07E0,

		0x0000,0x0000
	};

//	printf("SetWait()\n");
/*
	if( IntuitionBase->LibNode.lib_Version >= 39 )
	{
		SetWindowPointer(win,
			WA_BusyPointer,		TRUE,
			WA_PointerDelay,	TRUE,
		TAG_DONE);
	}
	else
*/
		SetPointer(win,busyimage,16,16,-6,0);
}


void ClrWait(struct Window *win)
{
/*
	if( IntuitionBase->LibNode.lib_Version >= 39 )
		SetWindowPointer(win,TAG_DONE);
	else
*/
		ClearPointer(win);
}





APTR BlockWin( struct Window *win )
{
	struct BlockingReqInfo *bri;
	ULONG idcmp;

	if( bri = AllocVec( sizeof(struct BlockingReqInfo), MEMF_ANY|MEMF_CLEAR ) )
	{
		idcmp = win->IDCMPFlags;
		bri->OldIDCMP = idcmp;
		if( idcmp & ( IDCMP_MENUVERIFY|IDCMP_SIZEVERIFY|IDCMP_REQVERIFY ) )
		{
			idcmp &= ~( IDCMP_MENUVERIFY|IDCMP_SIZEVERIFY|IDCMP_REQVERIFY );
			if( !idcmp ) idcmp = IDCMP_CLOSEWINDOW;
			ModifyIDCMP( win, idcmp );
		}

		InitRequester( &bri->Req );
		if( Request( &bri->Req, win ) )
		{
			SetWait( win );
		}
		else
		{
			if( bri->OldIDCMP != idcmp ) ModifyIDCMP( win, bri->OldIDCMP );
			FreeVec( bri );
			bri = NULL;
		}
	}
	return( (APTR)bri );
}



VOID UnblockWin( APTR blockwinhandle )
{
	struct BlockingReqInfo *bri;
	struct Requester *req;

	bri = (struct BlockingReqInfo *)blockwinhandle;
	req = &bri->Req;
	ClrWait( req->RWindow );
	EndRequest( req, req->RWindow );
	if( bri->OldIDCMP != req->RWindow->IDCMPFlags )
		ModifyIDCMP( req->RWindow, bri->OldIDCMP );
	FreeVec( bri );
}



BOOL BlockAllSuperWindows( VOID )
{
	struct SuperWindow *sw;
	BOOL success = TRUE;

	for( sw = (struct SuperWindow *)superwindows.lh_Head;
		sw->sw_Node.ln_Succ && success;
		sw = (struct SuperWindow *)sw->sw_Node.ln_Succ )
	{
		if( sw->sw_Window && !sw->sw_BlockingHandle )
		{
			sw->sw_BlockingHandle = BlockWin( sw->sw_Window );
			if( !sw->sw_BlockingHandle )
				success = FALSE;
		}
	}

	if( !success )
		UnblockAllSuperWindows();

	return( success );
}


VOID UnblockAllSuperWindows( VOID )
{
	struct SuperWindow *sw;

	for( sw = (struct SuperWindow *)superwindows.lh_Head;
		sw->sw_Node.ln_Succ;
		sw = (struct SuperWindow *)sw->sw_Node.ln_Succ )
	{
		if( sw->sw_Window && sw->sw_BlockingHandle )
		{
			UnblockWin( sw->sw_BlockingHandle );
			sw->sw_BlockingHandle = NULL;
		}
	}
}



/*********************  RememberWindow()  *********************/

void RememberWindow( struct SuperWindow *sw )
{
	struct Window *win;

	assert( sw != NULL );
	assert( sw->sw_Window != NULL );

	win = sw->sw_Window;

	sw->sw_WinDim.Left = win->LeftEdge;
	sw->sw_WinDim.Top = win->TopEdge;
	sw->sw_WinDim.Width = win->Width;
	sw->sw_WinDim.Height = win->Height;

	UpdateWinDef( sw );

}

/*********************  UpdateWinDef()  *********************/

void UpdateWinDef( struct SuperWindow *sw )
{
	struct Window *win;

	assert( sw != NULL );
	assert( sw->sw_Window != NULL );

	win = sw->sw_Window;

	zonkfig.SuperWindowDefs[ sw->sw_Type ].Left = win->LeftEdge;
	zonkfig.SuperWindowDefs[ sw->sw_Type ].Top = win->TopEdge;
	zonkfig.SuperWindowDefs[ sw->sw_Type ].Width = win->Width;
	zonkfig.SuperWindowDefs[ sw->sw_Type ].Height = win->Height;
}


/*******************************************************/
// FindSuperWindowByType()
//
// Looks for a superwindow of the given type, starting at the
// specified window.
//

struct SuperWindow *FindSuperWindowByType( struct SuperWindow *sw, UWORD type )
{
	struct SuperWindow *ret = NULL;

	while( sw->sw_Node.ln_Succ && !ret )
	{
		if( sw->sw_Type == type ) ret = sw;
		sw = (struct SuperWindow *)sw->sw_Node.ln_Succ;
	}
	return( ret );
}


/*******************************************************/

struct List *ListAndHideWindows( VOID )
{
	struct List *winlist;
	struct SuperWindow *sw;
	struct DataNode *dn;

	if( winlist = AllocVec( sizeof( struct List ), MEMF_ANY|MEMF_CLEAR ) )
	{
		NewList( winlist );

		for( sw = (struct SuperWindow *)superwindows.lh_Head;
			sw->sw_Node.ln_Succ;
			sw = (struct SuperWindow *)sw->sw_Node.ln_Succ )
		{
			if( sw->sw_Window )
			{
				if( dn = AllocVec( sizeof( struct DataNode ), MEMF_ANY ) )
				{
					dn->dn_Data = sw;
					(*sw->sw_HideWindow)( sw );
					AddTail( winlist, (struct Node *)dn );
				}
			}
		}
	}
	return( winlist );
}

/*******************************************************/
VOID ShowHiddenWindows( struct List *winlist )
{
	struct DataNode *dn, *nextdn;
	struct SuperWindow *sw;

	if( winlist )
	{
		dn = (struct DataNode *)winlist->lh_Head;
		while( dn->dn_Node.ln_Succ )
		{
			nextdn = (struct DataNode *)dn->dn_Node.ln_Succ;
			sw = (struct SuperWindow *)dn->dn_Data;
			(*sw->sw_ShowWindow)( sw );
			Remove( (struct Node *)dn );
			FreeVec( dn );
			dn = nextdn;
		}
		FreeVec( winlist );
	}
}

/*******************************************************/
//
// Remake the title string for each open superwindow.
//

void RefreshSuperWindowTitles( void )
{
	struct SuperWindow *sw;

	for( sw = (struct SuperWindow *)superwindows.lh_Head;
		sw->sw_Node.ln_Succ;
		sw = (struct SuperWindow *)sw->sw_Node.ln_Succ )
	{
		if( sw->sw_Window && sw->sw_MakeTitle )
			SetWindowTitles( sw->sw_Window, (*sw->sw_MakeTitle)(sw), (UBYTE *)-1 );
	}
}

/*******************************************************/
//
// Check if it's OK to close a superwindow.
// (The only time it might not be ok to close is when
// it's the last window open and there is unsaved data about)

BOOL IsOKToClose( struct SuperWindow *sw )
{
	BOOL yepsureisdude;
	BOOL modflag;
	struct Chunk *cnk;

	if( CountNodes( &superwindows ) == 1 )
	{
		modflag = FALSE;
		for( cnk = (struct Chunk *)chunklist.lh_Head;
			cnk->ch_Node.ln_Succ && !modflag;
			cnk = (struct Chunk *)cnk->ch_Node.ln_Succ )
		{
			if( cnk->ch_Modified )
				modflag = TRUE;
		}

		if( modflag )
		{
			yepsureisdude = PlebReq( sw->sw_Window, PROGNAME,
				"There are unsaved changes.\nQuit anyway?", "Quit|Cancel" );
		}
		else
			yepsureisdude = TRUE;
	}
	else
		yepsureisdude = TRUE;

	return( yepsureisdude );
}

/**************************************************************/
//
// Grab a pointer to the first open superwindow (if any)
//

struct SuperWindow *FensterSnarf( void )
{
	struct SuperWindow *sw;

	for( sw = (struct SuperWindow *)superwindows.lh_Head;
		sw->sw_Node.ln_Succ;
		sw = (struct SuperWindow *)sw->sw_Node.ln_Succ )
	{
		if( sw->sw_Window )
			return( sw );
	}

	return( NULL );
}
