/****************************************************************************/
//
// BONK: Misc.c
//
/****************************************************************************/


#include <stdio.h>
#include <string.h>
#include <ctype.h>

//#include <dos/dos.h>
#include <dos/dostags.h>
#include <exec/exec.h>
#include <intuition/intuition.h>
//#include <graphics/gfxbase.h>
//#include <rexx/rxslib.h>
#include <libraries/gadtools.h>
//#include <libraries/diskfont.h>
//#include <libraries/asl.h>
//#include <libraries/iffparse.h>

#include <proto/exec.h>
#include <proto/dos.h>
#include <proto/intuition.h>
#include <proto/graphics.h>
//#include <proto/diskfont.h>
//#include <proto/asl.h>
#include <clib/alib_protos.h>
#include <proto/gadtools.h>
//#include <proto/iffparse.h>

#include <global.h>
#include <ExternVars.h>

/*******************  FindNthNode  *******************/
//
// Returns a ptr to the Nth node in the specified list.
// The counting begins at zero (0 = the node after the head node).
// NULL is returned if the node doesn't exist.
//

struct Node *FindNthNode(struct List *list, UWORD num)
{
	struct Node *node;

	if( IsListEmpty(list) ) return(NULL);

	node = list->lh_Head;

	while( num )
	{
		node = node->ln_Succ;
		if(!node->ln_Succ) return(NULL);
		--num;
	}
	return(node);
}


/*******************  ClearWindow()  *******************/
//
// Clears the interior of a window without zapping crap
// out of the borders.
//
// Probably.
//

void ClearWindow( struct Window *win )
{
	SetAPen( win->RPort, 0 );
	RectFill(win->RPort,
		win->BorderLeft,
		win->BorderTop,
		win->Width - win->BorderRight - 1,
		win->Height - win->BorderBottom - 1 );
}


/**********  AdjustWindowFont  ********************/

void	AdjustWindowFont(struct SuperWindow *sw, UWORD width,UWORD height)
{
	struct Window *win;

	win = sw->sw_Window;

	width = width + win->BorderLeft + win->BorderRight;
	height = height + win->BorderTop + win->BorderBottom;

	if( (win->Width < width) || (win->Height < height) )
	{
		SetFont(win->RPort,topaztextfont);
		sw->sw_TextAttr = &topaztextattr;
		sw->sw_TextFont = topaztextfont;
	}
}

/*************  MyStrDup()  *************/
// Free string with FreeVec()


char *MyStrDup(char *source)
{
	char *dest;

	if( dest = AllocVec( strlen( source ) + 1 , MEMF_ANY) )
		strcpy(dest,source);
	return(dest);
}


struct List *ListAndHideWindows(void)
{
	struct List *winlist;
	struct SuperWindow *sw;
	struct DataNode *dn;

	if (winlist = AllocVec( sizeof(struct List), MEMF_ANY | MEMF_CLEAR ) )
	{
		NewList(winlist);
		for( sw = (struct SuperWindow *)superwindows.lh_Head;
			sw->sw_Node.ln_Succ;
			sw = (struct SuperWindow *)sw->sw_Node.ln_Succ )
		{
			if ( sw->sw_Window )
			{
				if (dn = AllocVec( sizeof(struct DataNode), MEMF_ANY | MEMF_CLEAR ) )
				{
					dn->dn_Ptr = sw;
					(*sw->sw_HideWindow)(sw);
					AddTail(winlist, (struct Node *)dn);
				}
				else
				{
					/* fucked up - couldn't alloc a DataNode, so clean up */
					/* and bugger off. */

					while( !IsListEmpty(winlist) )
					{
						dn = (struct DataNode *)winlist->lh_Head;
						Remove( (struct Node *)dn);
						FreeVec( (struct Node *)dn);
					}
					FreeVec(winlist);
					return(NULL);
				}
			}
		}
	}
	return(winlist);
}



void ReshowWindowList(struct List *winlist)
{
	struct DataNode *dn;
	struct SuperWindow *sw;

	if(winlist)
	{
		while(!IsListEmpty(winlist))
		{
			dn = (struct DataNode *)winlist->lh_Head;
			Remove( (struct Node *)dn);
			sw = dn->dn_Ptr;
			(*sw->sw_ShowWindow)(sw);
			FreeVec(dn);
		}
		FreeVec(winlist);
	}
}

/***********   GetDefaultScreen() ***********/
//
// Last ditch attempt to get a valid screen.
//

struct Screen *GetDefaultScreen()
{
	char name[MAXPUBSCREENNAME + 2];

	GetDefaultPubScreen(name);
	return( GetPubScreen(name)->psn_Screen );
}




/****************  DoEverything()  ****************/
//
//		NAME
//	DoEverything
//
//		SYNOPSIS
//	theanswer = DoEverything(void)
//
//		FUNCTION
//	This routine does everything, and returns the answer as an int.
//
//		INPUTS
//	none
//
//		RESULT
//	theanswer - The Answer.
//
//		BUGS
//	none - this routine is perfect.
//
//		SEE ALSO
//	"The Hitchhikers Guide to the Galaxy"
//		- Published by:
//			Megadodo Publications,
//			Light City,
//			Ursa Minor Beta.

int DoEveryThing(void)
{
	return(42);
}



/****************   BlockAllWindows()    *************/
//
// Blocks user input by opening requesters on all the
// currently-visible SuperWindows (The requesters are
// 0 by 0 in size). The returned list is used by the
// associated function FreeBlockedWindows() which removes
// the requesters.
//

struct List *BlockAllWindows(void)
{
	struct List						*list;
	struct SuperWindow		*oursw;
	struct DataNode				*dn;
	struct Requester			*req;
	ULONG idcmp;

	if(list = AllocVec( sizeof(struct List), MEMF_ANY | MEMF_CLEAR ))
	{
		NewList(list);
		for(oursw = (struct SuperWindow *)superwindows.lh_Head;
				oursw->sw_Node.ln_Succ;
				oursw = (struct SuperWindow *)oursw->sw_Node.ln_Succ )
		{
			if(oursw->sw_Window)
			{
				if(req = AllocVec( sizeof(struct Requester), MEMF_ANY | MEMF_CLEAR) )
				{
					if(dn = AllocVec( sizeof(struct DataNode), MEMF_ANY | MEMF_CLEAR) )
					{
						idcmp = oursw->sw_Window->IDCMPFlags;
						dn->dn_Node.ln_Name = (char *)idcmp;
						if( idcmp & ( IDCMP_MENUVERIFY|IDCMP_SIZEVERIFY|IDCMP_REQVERIFY ) )
						{
							idcmp &= ~( IDCMP_MENUVERIFY|IDCMP_SIZEVERIFY|IDCMP_REQVERIFY );
							if( !idcmp ) idcmp = IDCMP_CLOSEWINDOW;
							ModifyIDCMP( oursw->sw_Window, idcmp );
						}

						InitRequester(req);
						if(Request(req,oursw->sw_Window) )
						{
							dn->dn_Ptr = req;
							AddTail(list,&dn->dn_Node);
							SetWait(oursw->sw_Window);
						}
						else
						{
							FreeVec(dn);
							FreeVec(req);
						}
					}
					else
					{
						FreeVec(req);
					}
				}
			}
		}
	}
	return(list);
}



void FreeBlockedWindows(struct List *list)
{
	struct DataNode				*dn, *nextdn;
	struct Requester			*req;
	if(list)
	{
		dn = (struct DataNode *)list->lh_Head;
		while( !IsListEmpty(list) )
		{
			nextdn = (struct DataNode *)dn->dn_Node.ln_Succ;
			req = (struct Requester *)dn->dn_Ptr;
			ClrWait(req->RWindow);
			EndRequest(req,req->RWindow);
			if( (ULONG)dn->dn_Node.ln_Name != req->RWindow->IDCMPFlags )
				ModifyIDCMP( req->RWindow, (ULONG)dn->dn_Node.ln_Name );
			FreeVec(dn->dn_Ptr);
			Remove(&dn->dn_Node);
			FreeVec(dn);
			dn = nextdn;
		}
		FreeVec(list);
	}
}



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


/*	if(IntuitionBase->LibNode.lib_Version >= 39)
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
/*	if(IntuitionBase->LibNode.lib_Version >= 39)
		SetWindowPointer(win,TAG_DONE);
	else
 */
		ClearPointer(win);
}


/********   SetShanghaiState ***************/
//
//  Set the pub screen up for the appropriate Shanghai state.

void SetShanghaiState(struct Screen *scr, BOOL Shanghai)
{
	char			pubname[MAXPUBSCREENNAME];

	if (Shanghai)
	{
		if ( GetPubScreenName(scr,pubname) )
		{
			SetPubScreenModes(OldPubScreenModes | SHANGHAI);
			SetDefaultPubScreen(pubname);
		}
	}
	else
	{
		SetDefaultPubScreen(OldDefaultPubScreenName);
		SetPubScreenModes(OldPubScreenModes);
	}
}

//*********  GetPubScreenName() *************/
//
// Fill the name buffer with the name of the public screen

BOOL GetPubScreenName(struct Screen *scr, char *name)
{
	struct List								*publist;
	struct PubScreenNode			*pubnode;

	publist = LockPubScreenList();

	for(pubnode = (struct PubScreenNode *)publist->lh_Head;
		pubnode->psn_Node.ln_Succ;
		pubnode = (struct PubScreenNode *)pubnode->psn_Node.ln_Succ )
	{
		if(pubnode->psn_Screen == scr)
		{
			strcpy(name,pubnode->psn_Node.ln_Name);
			UnlockPubScreenList();
			return(TRUE);
		}
	}
	UnlockPubScreenList();
	return(FALSE);
}


/*************** DoDOS  ***************/
//
//

long DoDOS(char *cmd)
{
	UBYTE *autocon="CON:0/40/640/150/Output stuff/auto/close/wait";
	BPTR fh;
	long ret = -1;

  if(fh = Open(autocon, MODE_OLDFILE))
		ret = SystemTags(cmd, SYS_Input, fh, SYS_Output, NULL,
			SYS_UserShell, TRUE, SYS_Asynch, TRUE, TAG_END);
	return(ret);
}


/************* WaitForWindowMove() ********/
//
//  Wait for a ChangeWindow IDCMP event. All windows that can
// be referenced by AREXX _must_ have this IDCMP input.

void WaitForWindowMove(void)
{
	ULONG		mainsignal;
	struct	IntuiMessage		*msg;
	BOOL		quit = FALSE;

	mainsignal = 1L << mainmsgport->mp_SigBit;

	while(!quit)
	{
		Wait(mainsignal);
		while( (msg = (struct IntuiMessage *)GetMsg(mainmsgport)) && !quit )
		{
			if(msg->Class == IDCMP_CHANGEWINDOW)
			{
				ReplyMsg((struct Message *)msg);
				quit = TRUE;
			}
		}
	}
}

/************  SussOutQuiting() *************/
//
//	Bring up a Quit program requester if there are no windows left on
// screen. If User doesn't want to quit a new project is opened.
// returns: TRUE = User wants to Quit, FALSE = User wants to carry on.

BOOL SussOutQuiting(BOOL force)
{
	if ( IsListEmpty(&superwindows) )
	{
		if( force || QuitProgramRequester() )
		{
			return(TRUE);
		}
		else
		{
//			OpenNewProject(10,10);
			OpenNewToolsWindow();
			return(FALSE);
		}
	}
	return(FALSE);
}



void MyRectFill(struct RastPort *rp, WORD x1, WORD y1, WORD x2, WORD y2)
{
	WORD foo;

	if (x1>x2)
	{
		foo=x1;
		x1=x2;
		x2=foo;
	}
	if (y1>y2)
	{
		foo=y1;
		y1=y2;
		y2=foo;
	}

	RectFill(rp, x1, y1, x2, y2);
}


void WinPlot( struct Window *win, WORD x, WORD y, UBYTE col )
{
	if( ( x >= 0 ) && ( x < win->Width - win->BorderLeft - win->BorderRight ) &&
		( y >= 0 ) && ( y < win->Height - win->BorderTop - win->BorderBottom ) )
	{
		SetAPen( win->RPort, col );
		WritePixel( win->RPort, x + win->BorderLeft, y + win->BorderTop );
	}
}



struct BitMap *SetupWorkBitMap( struct Screen *scr )
{
	struct BitMap *bm;
	BOOL	err = FALSE;
	UWORD i;

	if( bm = AllocVec( sizeof( struct BitMap ), MEMF_ANY|MEMF_CLEAR ) )
	{
		bm->BytesPerRow = scr->RastPort.BitMap->BytesPerRow;
		bm->Rows = scr->RastPort.BitMap->Rows;
		bm->Flags = scr->RastPort.BitMap->Flags;
		bm->Depth = scr->RastPort.BitMap->Depth;

		for( i = 0; i < bm->Depth; i++ )
		{
			if( !( bm->Planes[i] = AllocRaster( bm->BytesPerRow*8, bm->Rows ) ) )
				err = TRUE;
		}

		if( err )
		{
			for( i = 0; i < bm->Depth; i++ )
				if( bm->Planes[i] ) FreeRaster( bm->Planes[i], bm->BytesPerRow*8, bm->Rows );
			FreeVec( bm );
			bm = NULL;
		}
	}
	return( bm );
}


void FreeWorkBitMap( struct BitMap *bm )
{
	UWORD i;
	if( bm )
	{
		for( i = 0; i < bm->Depth; i++ )
			if( bm->Planes[i] ) FreeRaster( bm->Planes[i], bm->BytesPerRow*8, bm->Rows );
		FreeVec( bm );
	}
}



/******************  MyTextLength()  ******************/
//
// Like the system TextLength() function, just doesn't
// need a Rastport to figure it out (so you can suss out
// stuff _before_ you open the window).
//
// inputs:
// tf:  TextFont struct of the font being used
// str: null-terminated string to be measured

WORD MyTextLength( struct TextFont *tf, char *str )
{
	WORD width = 0;

	if( tf->tf_Flags & FPF_PROPORTIONAL )
	{
		/* proportional font */
		for( ; *str != '\0'; str++ )
		{
			width += ((WORD *)tf->tf_CharKern)[ *str - tf->tf_LoChar ];
			width += ((WORD *)tf->tf_CharSpace)[ *str - tf->tf_LoChar ];
		}
	}
	else
	{
		/* fixed-space font */
		width = tf->tf_XSize * strlen( str );
	}
	return( width );
}

/******************  HandleGlobalRawKey()  ******************/
//
//


UWORD HandleGlobalRawKey( struct SuperWindow *sw, UWORD rawkey, UWORD qual )
{
	switch( rawkey )
	{
		case 0x0039:								/* . (full stop) */
			SetTool( TL_DRAW );
			rawkey = 0xFFFF;
			break;
		case 0x0028:								/* l */
			SetTool( TL_LINE );
			rawkey = 0xFFFF;
			break;
		case 0x0013:								/* r */
			SetTool( TL_BOX );
			rawkey = 0xFFFF;
			break;
//		case 0x0004:
//			SetTool( TL_FBOX );
//			rawkey = 0xFFFF;
//			break;
		case 0x0023:								/* f */
			SetTool( TL_FILL );
			rawkey = 0xFFFF;
			break;
		case 0x0035:								/* b */
			SetTool( TL_CUT );
			rawkey = 0xFFFF;
			break;
//		case 0x0007:
//			if( currentbrush )
//			{
//				SetTool( TL_PASTE );
//				rawkey = 0xFFFF;
//			}
//			break;
//		case 0x0008:
//			SetTool( TL_PICK );
//			rawkey = 0xFFFF;
//			break;
		case 0x0016:
			Undo( &undobuf );
			rawkey = 0xFFFF;
			break;
		case 0x0026:					/* j */
			JumpToNextProjectWindow( sw );
			rawkey = 0xFFFF;
			break;
	}
	return( rawkey );
}


/***************** Create BitMap **********************/
//  Given the right details it will create, initialise and
// allocate a bitmap structure. (and BitMap)
// Return: NULL = Not enuf memory.

struct BitMap *CreateBitMap(UWORD width, UWORD height, UWORD depth,
	BOOL interleaved)
{
	struct BitMap *bm;
	PLANEPTR allocated = (PLANEPTR) 1;
	UWORD i;
	ULONG flags;

	flags = interleaved ? BMF_INTERLEAVED|BMF_CLEAR : BMF_CLEAR;

	if(GfxBase->LibNode.lib_Version >= 39)
	{
		bm = AllocBitMap(width,height,depth,flags,NULL);
	}
	else
	{
		if(bm = AllocVec(sizeof(struct BitMap), MEMF_ANY|MEMF_CLEAR))
		{
			InitBitMap(bm,depth,width,height);
			for(i=0; ((i < depth) && (allocated)); i++)
				allocated = (bm->Planes[i] = AllocRaster(width,height));
	
			if (allocated == NULL)
			{
				DestroyBitMap(bm,width,height,depth);
				bm = NULL;
			}
//			else
//				ClearBitMap(bm);
		}
	}
	return(bm);
}

/************************** Destroy BitMap *******************/
// Counterpart to CreateBitMap
//
void DestroyBitMap(struct BitMap *bm, UWORD width, UWORD height, UWORD depth)
{
	int i;

	WaitBlit();
	if(bm)
	{
		if(GfxBase->LibNode.lib_Version >= 39)
		{
			FreeBitMap(bm);
		}
		else
		{
			for(i=0; (i<depth); i++)
			{
				if(bm->Planes[i])
					FreeRaster(bm->Planes[i], width, height);
			}
			FreeVec(bm);
		}
	}
}

/*********************  Mystrncpy()  ***********************/
//  This is just a slight varience from the original strncpy()
// in that this puts a null terminator on the string, after
// the last character, so that Mystrncpy(to,from,15) would copy
// the first 15 characters then put in a terminator.

char *Mystrncpy(char *to, char *from, int length)
{
	register char *dest = to;
	register char *src = from;

	while( length-- && *src )
		*dest++ = *src++;
	*dest = '\0';

	return( to );
}




/*******************  MakeBackupFile()  *******************/
//
// Tries to make a backup file (.bak) by renaming the given filename.
// If the file specified doesn't exist, the old backup (if any) is
// left untouched.
//
// Returns success - may fail if Rename() fails.

BOOL MakeBackupFile( STRPTR filename )
{
	BOOL allok;
	UBYTE bakname[256];
	BPTR lock;

	allok = TRUE;

	/* create backup name */
	Mystrncpy( bakname, filename, 256-4-1 );
	strcat( bakname,".bak" );

	if( lock = Lock( filename, ACCESS_READ ) )
	{
		/* file exists */
		UnLock( lock );
		DeleteFile( bakname );
		if( !Rename( filename, bakname ) )
			allok = FALSE;			/* fail if we couldn't make the backup */
	}
	/* if file doesn't exist just keep the old backup and exit */

	return( allok );
}


/*******************  MyStrToUpper()  *******************/

char *MyStrToUpper( char *str )
{
	register char *p = str;
	while( *p )
	{
		*p = toupper( *p );
		p++;
	}
	return( str );
}

