/****************************************************************************/
//
// BONK: EditMenu.c
//
/****************************************************************************/


#include <stdio.h>
#include <string.h>
#include <dos/dos.h>
//#include <dos/dostags.h>
#include <exec/exec.h>
#include <intuition/intuition.h>
//#include <graphics/gfxbase.h>
//#include <rexx/rxslib.h>
//#include <libraries/gadtools.h>
//#include <libraries/diskfont.h>
#include <libraries/asl.h>
//#include <libraries/iffparse.h>
//#include "dh1:programming/stuff/include/libraries/asl.h"

#include <proto/exec.h>
#include <proto/dos.h>
#include <proto/intuition.h>
//#include <proto/graphics.h>
//#include <proto/diskfont.h>
#include <proto/asl.h>
#include <clib/alib_protos.h>
//#include <proto/gadtools.h>
//#include <proto/iffparse.h>

#include <global.h>
#include <ExternVars.h>

static void ED_LoadPalette( struct SuperWindow *sw );
static void ED_LoadBlocks( struct SuperWindow *sw );

/*************  DoEditMenu ***********/
//
//	Sort out everything to do with the Project Menu. If
// we closed a window then we return FALSE, else TRUE.
//
// 0 = All Ok , 1 = Stop Processing
// ********  FIX THE ABOVE COMMENT SOMETIME.  **********

UWORD DoEditMenu(struct SuperWindow *sw,UWORD id)
{
	UWORD itemnum, subnum;

	itemnum = ITEMNUM(id);
	subnum = SUBNUM(id);

	switch(itemnum)
	{
		case MNEDIT_UNDO:
			Undo(&undobuf);
			return(0);
			break;
		case MNEDIT_LOADPALETTE:
			ED_LoadPalette(sw);
			return(0);
			break;
		case MNEDIT_LOADBLOCKS:
			ED_LoadBlocks(sw);
			return(0);
			break;
		default:
			return(0);
	}
}


static void ED_LoadPalette( struct SuperWindow *sw )
{
	char name[256];
	struct List *rlist;
	BOOL wibble;

	if( !(rlist = BlockAllWindows() ) ) return;

	/* bring up the file requester */
	wibble = AslRequestTags( filereq,
		//ASLFR_Screen, mainscreen,
		ASLFR_Window, sw->sw_Window,
		ASLFR_TitleText, "Load Palette",
		ASLFR_DoSaveMode, FALSE,
		TAG_END );

	FreeBlockedWindows(rlist);

	if(wibble)
	{
		/* glue together the full pathname */
		if( AddPart( Mystrncpy(name, filereq->fr_Drawer, 256-1),
			filereq->fr_File,
			256 ) )
		{
			if( globpalette ) FreePalette( globpalette);
			globpalette = LoadPalette(name);
			/* suss out new preview colours if there are open previewwindows...*/
			if( FindSuperWindowByType( SWTY_PREVIEW ) )
				CalcBlockPreviewColours( globlocks );
			RemakeScreen();
		}
	}
}



static void ED_LoadBlocks( struct SuperWindow *sw )
{
	char name[256];
	struct List *rlist;
	BOOL wibble;
	struct BlockSet *bs;

	if( !(rlist = BlockAllWindows() ) ) return;

	/* bring up the file requester */
	wibble = AslRequestTags( filereq,
		//ASLFR_Screen, mainscreen,
		ASLFR_Window, sw->sw_Window,
		ASLFR_TitleText, "Load Blocks",
		ASLFR_DoSaveMode, FALSE,
		TAG_END );

	FreeBlockedWindows(rlist);

	if(wibble)
	{
		/* glue together the full pathname */
		if( AddPart( Mystrncpy(name, filereq->fr_Drawer, 256-1),
			filereq->fr_File,
			256 ) )
		{
			if( bs = LoadBlockSet( name ) )
			{
				FreeBlockSet( globlocks );
				globlocks = bs;
				//ResetSelector( (struct ProjectWindow *)sw );
				RedrawAllProjects();
				RedrawAllBlockWindows();
			}
		}
	}
}

