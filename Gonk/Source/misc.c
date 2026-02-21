#include <stdio.h>
//#include <stdarg.h>
#include <string.h>
//#include <dos/dos.h>
//#include <dos/dostags.h>
#include <exec/exec.h>
#include <intuition/intuition.h>
#include <intuition/intuitionbase.h>
#include <graphics/gfxbase.h>
//#include <rexx/rxslib.h>
#include <libraries/gadtools.h>
//#include <libraries/diskfont.h>
//#include <libraries/asl.h>
#include <libraries/iffparse.h>

#include <proto/exec.h>
#include <proto/dos.h>
#include <proto/intuition.h>
#include <proto/graphics.h>
//#include <proto/diskfont.h>
#include <proto/asl.h>
//#include <clib/alib_protos.h>
#include <proto/gadtools.h>
#include <proto/iffparse.h>
#include <proto/layers.h>
#include <datatypes/pictureclass.h>

#include <global.h>
#include <ExternVars.h>
#include <dos/dostags.h>
/* #include <intuition/intuitionbase.h> */

static UWORD chip crosshairimage[18*2] = 
{
	0x0000,0x0000,

	0x0200,0x0000,
	0x0000,0x0200,
	0x0200,0x0000,
	0x0000,0x0200,
	0x0200,0x0000,
	0x0000,0x0000,
	0xA8A8,0x5050,
	0x0000,0x0000,
	0x0200,0x0000,
	0x0000,0x0200,
	0x0200,0x0000,
	0x0000,0x0200,
	0x0200,0x0000,
	0x0000,0x0000,
	0x0000,0x0000,
	0x0000,0x0000,
	0x0000,0x0000
};

static UWORD chip fillimage[18*2] = 
{
	0x0000,0x0000,
	0x0000,0x0000,
	0x0070,0x0070,
	0x01F8,0x01C8,
	0x07F8,0x0608,
	0x0FFC,0x0804,
	0x07FC,0x0404,
	0x07FC,0x0404,
	0x07F8,0x0218,
	0x07E0,0x0260,
	0x0780,0x0180,
	0x0E00,0x0000,
	0x0C00,0x0000,
	0x0C00,0x0000,
	0x1C00,0x0000,
	0x7F00,0x0000,
	0xEF80,0xEE00,
	0x0000,0x0000,
};

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

/*******************  NodeNumber()  ******************/
//
//  Gives the number of the current Node (starting from 1)
//  Returns: 0 if node not found.

UWORD NodeNumber(struct List *list, struct Node *node)
{
	struct Node *ln;
	UWORD count = 1;

	ln = list->lh_Head;
	while( (ln != node) && (ln->ln_Succ) )
	{
		ln = ln->ln_Succ;
		count++;
	}
	if(ln != node)
		count = 0;

	return(count);
}

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
//
//  If the window size is under the given width and height, then we
// change the window font to 'topaz.font'.
//
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


/*********   MyStrDup *******************/
char *MyStrDup(char *source)
{
	char *dest;

	if( dest = AllocVec( strlen( source ) + 1 , MEMF_ANY) )
		strcpy(dest,source);
	return(dest);
}


/*********  HideAllWindows() **************/

void HideAllWindows(void)
{
	struct SuperWindow *sw;

	for( sw = (struct SuperWindow *)superwindows.lh_Head;
		sw->sw_Node.ln_Succ;
		sw = (struct SuperWindow *)sw->sw_Node.ln_Succ )
	{
		if ( sw->sw_Window )
		{
			(*sw->sw_HideWindow)(sw);
			sw->sw_Flags |= SWF_HIDING;
		}
	}
}

/*********  ListAndHideProjWindows() **************/
/*
struct List *ListAndHideProjWindows(struct Project *pj)
{
	struct List *winlist;
	struct ProjectWindow *pw;
	struct DataNode *dn;

	if (winlist = AllocVec( sizeof(struct List), MEMF_ANY | MEMF_CLEAR ) )
	{
		NewList(winlist);
		for( pw = pj->pj_ProjectWindows;
			pw;
			pw = pw->pw_NextProjWindow)
		{
			if ( pw->pw_sw.sw_Window )
			{
				if (dn = AllocVec( sizeof(struct DataNode), MEMF_ANY | MEMF_CLEAR ) )
				{
					dn->dn_Ptr = &pw->pw_sw;
					(*pw->pw_sw.sw_HideWindow)(&pw->pw_sw);
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

*/

void ReshowHiddenWindows(void)
{
	struct SuperWindow *sw;

	for( sw = (struct SuperWindow *)superwindows.lh_Head;
		sw->sw_Node.ln_Succ;
		sw = (struct SuperWindow *)sw->sw_Node.ln_Succ )
	{
		if( (!sw->sw_Window) && (sw->sw_Flags &= SWF_HIDING) )
		{
			(*sw->sw_ShowWindow)(sw);
			sw->sw_Flags &= ~SWF_HIDING;
		}
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

	struct TagItem tags[] = {	WA_BusyPointer,		TRUE,
														WA_PointerDelay,	TRUE,
														TAG_DONE};

	if(IntuitionBase->LibNode.lib_Version >= 39)
		SetWindowPointerA(win,tags);
	else
		SetPointer(win,busyimage,16,16,-6,0);
}


void ClrWait(struct Window *win)
{
	MyClearPointer(win);
}

void MySetPointer(struct Window *win, UWORD *sprite,UWORD width, UWORD height,
									WORD xoffset, WORD yoffset)
{
	//struct TagItem tags[] = {	WA_Pointer,				NULL,
	//													WA_PointerDelay,	TRUE,
	//													TAG_DONE};

	//if(IntuitionBase->LibNode.lib_Version >= 39)
	//{
	//	tags[0].ti_Data = (ULONG)sprite;
	//	SetWindowPointerA(win,tags);
	//}
	//else
		SetPointer(win,sprite,height,width,xoffset,yoffset);	
}

void MyClearPointer(struct Window *win)
{
	struct TagItem tags[] = {TAG_DONE};
	struct SuperWindow *sw;
	BOOL done = FALSE;

	if(IntuitionBase->LibNode.lib_Version >= 39)
		SetWindowPointerA(win,tags);
	else
		ClearPointer(win);

	for(sw = (struct SuperWindow *)superwindows.lh_Head;
			sw->sw_Node.ln_Succ && !done;
			sw = (struct SuperWindow *)sw->sw_Node.ln_Succ)
	{
		if(sw->sw_Window == win)
		{
			done = TRUE;
			if(sw->sw_Type == SWTY_PROJECT)
				SetCorrectPointer((struct ProjectWindow *)sw);
		}
	}

}

void SetCorrectPointer(struct ProjectWindow *pw)
{
	struct Window *win;

	if( (pw) && (win = pw->pw_sw.sw_Window) )
	{
		switch(currenttool)
		{
			case TL_FILL:
				MySetPointer(win, fillimage, 16, 16, -4, -15);
				break;
			default:
				MySetPointer(win, crosshairimage, 16, 16, -7, -6);
				break;
		}
	}
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

/*********  GetPubScreenName() *************/
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
	UBYTE *autocon=Txt_DefaultConsole;
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
			OpenNewProject(&glob.cg_WindowDefaults[SWTY_PROJECT],NULL);
			return(FALSE);
		}
	}
	return(FALSE);
}


/*********** Get Pot Value **********/
//  Get the value for a Prop Gadget Pot Value;
// Only valid for numlevels > 1

UWORD GetPotValue(UWORD numlevels,UWORD currentlevel)
{
	return( (UWORD)((MAXPOT * currentlevel) / (numlevels-1)) );
}

/********* Get Slider Level ****************/
//  Return the current level that the Slider Gadget is at.
//
UWORD GetSliderLevel(UWORD numlevels,UWORD pot)
{
	UWORD level;

	if(numlevels > 1)
	{
		level = (((ULONG)pot) * (numlevels-1) + MAXPOT/2) / MAXPOT;
	}
	else
	{
		level = 0;
	}
	return(level);
}

/**************** Get Gadget From ID ******************/
// Return NULL if no Gadget
//
struct Gadget *GetGadgetFromID(struct Gadget *gad,UWORD id)
{
	for( ; ((gad->GadgetID != id) && (gad)); gad = gad->NextGadget) {}
	return(gad);
}

/***************** Create BitMap **********************/
//  Given the right details it will create, initialise and
// allocate a bitmap structure. (and BitMap)
// Return: NULL = Not enuf memory.

struct BitMap *CreateBitMap(UWORD width, UWORD height, UWORD depth, BOOL interleaved)
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
			else
				ClearBitMap(bm);
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

/*
struct BitMap *CreateUndoBitMap(struct Project *proj,
																UWORD bmwidth, UWORD bmheight, UWORD bmdepth)
{
	UWORD scrwidth,scrheight;
	struct BitMap *scrbm;

	scrwidth = mainscreen->Width;
	scrheight = mainscreen->Height;
	scrbm = mainscreen->RastPort.BitMap;

	if(scrwidth > bmwidth)
		bmwidth = scrwidth;

	if(scrheight > bmheight)
		bmheight = scrheight;

	if(scrbm->Depth > bmdepth)
		bmdepth = scrbm->Depth;

	proj->pj_UndoBitMapWidth = bmwidth;
	proj->pj_UndoBitMapHeight = bmheight;
	proj->pj_UndoBitMapDepth = bmdepth;

	return( CreateBitMap(bmwidth,bmheight,bmdepth) );
}
*/
void ClearBitMap(struct BitMap *bm)
{
	ULONG	planesize;
	UWORD plane;

	planesize = bm->BytesPerRow * bm->Rows;

	for(plane = 0; plane < bm->Depth; plane++)
	{
		BltClear(bm->Planes[plane],planesize,1);
	}
}


/************** Reset Undo Buffer *****************/
void ResetUndoBuffer(struct Project *pj)
{
	if(pj->pj_Edable)
	{
		if(pj->pj_UndoLeft < 0) pj->pj_UndoLeft = 0;
		if(pj->pj_UndoRight >= pj->pj_Width) pj->pj_UndoRight = pj->pj_Width-1;
		if(pj->pj_UndoTop < 0) pj->pj_UndoTop = 0;
		if(pj->pj_UndoBottom >= pj->pj_Height) pj->pj_UndoBottom = pj->pj_Height-1;

		/*My*/BltBitMap(pj->pj_MainBitMap, pj->pj_UndoLeft, pj->pj_UndoTop,
								pj->pj_UndoBitMap, pj->pj_UndoLeft, pj->pj_UndoTop,
								pj->pj_UndoRight - pj->pj_UndoLeft + 1, pj->pj_UndoBottom - pj->pj_UndoTop + 1,
								0xC0,0xFFFF,NULL);
	}
}

void DoUndo(struct Project *pj)
{
	WORD bx,by,br,bb,bwidth,bheight,undowidth,undoheight;
	struct Window *win;
	struct ProjectWindow *pw;
	struct Magnify mg;

	if(pj->pj_Edable)
	{
		if(pj->pj_UndoLeft < 0) pj->pj_UndoLeft = 0;
		if(pj->pj_UndoRight >= pj->pj_Width) pj->pj_UndoRight = pj->pj_Width-1;
		if(pj->pj_UndoTop < 0) pj->pj_UndoTop = 0;
		if(pj->pj_UndoBottom >= pj->pj_Height) pj->pj_UndoBottom = pj->pj_Height-1;

		undowidth = pj->pj_UndoRight - pj->pj_UndoLeft + 1;
		undoheight = pj->pj_UndoBottom - pj->pj_UndoTop + 1;
		/*My*/BltBitMap(pj->pj_UndoBitMap, pj->pj_UndoLeft, pj->pj_UndoTop,
								pj->pj_MainBitMap, pj->pj_UndoLeft, pj->pj_UndoTop,
								undowidth, undoheight,
								0xC0,0xFFFF,NULL);
		
		mg.mg_SrcBitMap = pj->pj_MainBitMap;
		mg.mg_Scale = TRUE;
		mg.mg_ScaleBitMap = workbm;

		for(pw=pj->pj_ProjectWindows; pw; pw=pw->pw_NextProjWindow )
		{
			if(win = pw->pw_sw.sw_Window)
			{
				mg.mg_SrcXOffset = pj->pj_UndoLeft;
				mg.mg_SrcYOffset = pj->pj_UndoTop;

				bx = pj->pj_UndoLeft - pw->pw_XOffset;
				by = pj->pj_UndoTop - pw->pw_YOffset;

				if( (pw->pw_XOffset > pj->pj_UndoRight) ||
						(pw->pw_YOffset > pj->pj_UndoBottom) ||
						(pw->pw_XOffset+pw->pw_MagWidth < pj->pj_UndoLeft) ||
					  (pw->pw_YOffset+pw->pw_MagHeight < pj->pj_UndoTop) )
					return;

				bwidth = undowidth;
				bheight = undoheight;
				br = bx + bwidth;
				bb = by + bheight;

				if(bx < 0)
				{
					bwidth += bx;
					mg.mg_SrcXOffset -= bx;
					bx = 0;
				}
				if(by < 0)
				{
					bheight += by;
					mg.mg_SrcYOffset -= by;
					by = 0;
				}
				if((bwidth+bx) > pw->pw_MagWidth)
					bwidth = (pw->pw_MagWidth-bx);
				if((bheight+by) > pw->pw_MagHeight)
					bheight = (pw->pw_MagHeight-by);

				mg.mg_DestRastPort = win->RPort;
				mg.mg_DestXOffset = (bx * pw->pw_PixelSize) + win->BorderLeft;
				mg.mg_DestYOffset = (by * pw->pw_PixelSize) + win->BorderTop;
				mg.mg_DestWidth = (bwidth * pw->pw_PixelSize);
				mg.mg_DestHeight = (bheight * pw->pw_PixelSize);
				mg.mg_DestPixelSize = pw->pw_PixelSize;
				mg.mg_ClearRight = mg.mg_DestXOffset + mg.mg_DestWidth - 1;
				mg.mg_ClearBottom = mg.mg_DestYOffset + mg.mg_DestHeight - 1;
				MagnifyBitMap(&mg);
			}
		}
	}
}

void UpdateUndo(struct Project *pj, WORD x, WORD y)
{
	if(x < pj->pj_UndoLeft)
		pj->pj_UndoLeft = x;
	if(x > pj->pj_UndoRight)
		pj->pj_UndoRight = x;

	if(y < pj->pj_UndoTop)
		pj->pj_UndoTop = y;
	if(y > pj->pj_UndoBottom)
		pj->pj_UndoBottom = y;
}

void UpdateUndoBrush(struct Brush *br,struct Project *pj, WORD x, WORD y)
{
	WORD brushx,brushy;

	brushx = x - br->br_DrawBrush.bi_XHandle;
	brushy = y - br->br_DrawBrush.bi_YHandle;
	UpdateUndo(pj,brushx,brushy);
	UpdateUndo(pj,brushx+(br->br_DrawBrush.bi_Width-1),brushy+(br->br_DrawBrush.bi_Height-1));
}

void DrawIndentBox(struct Window *win,UWORD minx, UWORD miny, UWORD maxx, UWORD maxy)
{
	SetAPen(win->RPort,globpens[SHADOWPEN]);
	Move(win->RPort,minx,maxy);
	Draw(win->RPort,minx,miny);
	Draw(win->RPort,maxx,miny);
	SetAPen(win->RPort,globpens[SHINEPEN]);
	Move(win->RPort,minx+1,maxy);
	Draw(win->RPort,maxx,maxy);
	Draw(win->RPort,maxx,miny+1);	
}

/***************** My Delete Msg Port ********************/

void __asm MyDeleteMsgPort(register __a0 struct MsgPort *port)
{
	struct	IntuiMessage		*msg;

	Forbid();
	while(msg = (struct IntuiMessage *)GetMsg(port) )
	{
		if(port == subtaskport)
		{
			ReplySubTaskMsg((struct Message *)msg);
		}
		if(port == mainmsgport)
			ReplyMsg((struct Message *)msg);
	}
	DeleteMsgPort(port);
	Permit();
}


/*****************  SetProcessWindowPtr  ************/
//
// Set the process window pointer to the first superwindow
// in the list.

void	SetProcessWindowPtr()
{
	struct SuperWindow *sw;

	sw=GetOpenSuperWindow();
	if(sw)
		taskbase->pr_WindowPtr = sw->sw_Window;
	else
		taskbase->pr_WindowPtr = NULL;
}

/************************   Close Window Safely ******************************/
//
void CloseWindowSafely(struct Window *win)
{
		/* we forbid here to keep out of race conditions with Intuition */
	Forbid();

		/* send back any messages for this window 
		** that have not yet been processed
		*/
	StripIntuiMessages( win->UserPort, win );
		/* clear UserPort so Intuition will not free it */
	win->UserPort = NULL;

		/* tell Intuition to stop sending more messages */
	ModifyIDCMP( win, 0L );

		/* turn multitasking back on */
	Permit();

		/* and really close the window */
	CloseWindow( win );
}

/* remove and reply all IntuiMessages on a port that
 * have been sent to a particular window
 * (note that we don't rely on the ln_Succ pointer
 *  of a message after we have replied it) */

void StripIntuiMessages(struct MsgPort *mp, struct Window *win )
{
	struct IntuiMessage *msg;
	struct Node *succ;

	WaitForSubTaskDone(mainmsgport);

	msg = (struct IntuiMessage *) mp->mp_MsgList.lh_Head;

	while( succ =  msg->ExecMessage.mn_Node.ln_Succ )
	{
		if( msg->IDCMPWindow ==  win )
		{
			/* Intuition is about to free this message.
			** Make sure that we have politely sent it back.
			*/
			Remove( (struct Node *)msg );
			ReplyMsg( (struct Message *)msg );
		}
		msg = (struct IntuiMessage *)succ;
	}
}

/*******************   GetFile()   ***************************/
//
//  Bring up an ASL file requester, and exit. This routine
// blocks all the other windows inputs.
// Inputs:	win = Reference window for file requester
//					dosavemode: TRUE  = Requester is in save mode (no double click)
//											FALSE = Requester is in normal mode (double clicking active)
// Returns: True = All was ok,
//					False = Requester was cancel'd or not enough memory, or something.

BOOL GetFile(struct Window *win, char *title, char *initialdrawer, char *initialfile,
						 BOOL dosavemode, BOOL domultiselect)
{
	BOOL result = FALSE;
	char *name = NULL;
	BPTR fh = NULL;

	WaitForSubTaskDone(mainmsgport);

	if( BlockAllSuperWindows())
	{
		if( (initialdrawer) || (initialfile) )
		{
			/* bring up the file requester */
			result = AslRequestTags( filereq,
										ASLFR_Window,					win,
										ASLFR_SleepWindow,		FALSE,
										ASLFR_TitleText,			title ? title : "Select File",
										ASLFR_DoSaveMode,			dosavemode,
										ASLFR_DoMultiSelect,	domultiselect,
										ASLFR_InitialDrawer,	initialdrawer ? initialdrawer : "",
										ASLFR_InitialFile,		initialfile ? initialfile : "",
										TAG_END);
		}
		else
		{
			/* bring up the file requester */
			result = AslRequestTags( filereq,
										ASLFR_Window,					win,
										ASLFR_SleepWindow,		FALSE,
										ASLFR_TitleText,			title,
										ASLFR_DoSaveMode,			dosavemode,
										ASLFR_DoMultiSelect,	domultiselect,
										TAG_END);
		}
		if(result)
		{
			if(dosavemode)
			{
				if( name = MakeFullPathName(filereq->fr_Drawer, filereq->fr_File) )
				{
					/* check to see if the file already exists */
					if( fh = Open( name, MODE_OLDFILE ) )
					{
						Close(fh);
						result = GroovyReq("Gonk", "File already exists", "Replace|Cancel" );
					}
					FreeVec(name);
				}
			}
		}
		UnblockAllSuperWindows();
	}
	return(result);
}

/********************   MakeFullPathName()   *********************/
//
//  Given a pointer to a drawer, and a file, it will join the bits
// together, and return the resulting allocated string. When done
// with the string, just FreeVec() it. NULL is returned if something
// went wrong, ie not enough memory, invalid path,...

char *MakeFullPathName(char *drawer, char *file)
{
	ULONG len = 4;
	char *fullname;

	len += strlen(drawer)+strlen(file);

	if( fullname = AllocVec(len, MEMF_ANY|MEMF_CLEAR) )
	{
		Mystrncpy(fullname, drawer, len);
		/* glue together the full pathname */
		if( AddPart( fullname, file, len ) )
			return(fullname);
		else
		{
			FreeVec(fullname);
			return(NULL);
		}
	}
	else
		return(NULL);
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

struct Node *MyFindName(struct List *list, STRPTR name, struct Node *startnode)
{
	struct Node *node;

	if(IsListEmpty(list))
		return(NULL);

	if(!startnode)
		startnode = list->lh_Head;

	for(node = startnode; node->ln_Succ; node = node->ln_Succ)
	{
		if(!stricmp(node->ln_Name,name))
			return(node);
	}
	return(NULL);
}

/*********************   Rethink PaletteBox()   ********************/
//
//  This routine will fit the palette into the given rectangle.
//

void RethinkPaletteBox(struct PaletteBox *pb,
											UWORD boxleft, UWORD boxtop, UWORD boxwidth, UWORD boxheight)
{
	struct Window *win;
	UWORD colourswide,colourshigh;
	UWORD colourwidth,colourheight,x,y,countx,county,col;
	int numofcolours;

	win = pb->pb_Window;
	if(win)
	{
		numofcolours = pb->pb_Palette->pl_Count;

		switch(numofcolours)
		{
			case 2:
				colourswide = 1;
				colourshigh = 2;
				break;
			case 4:
				colourswide = 2;
				colourshigh = 2;
				break;
			case 8:
				colourswide = 2;
				colourshigh = 4;
				break;
			case 16:
				colourswide = 4;
				colourshigh = 4;
				break;
			case 32:
				colourswide = 4;
				colourshigh = 8;
				break;
			case 64:
				colourswide = 8;
				colourshigh = 8;
				break;
			case 128:
				colourswide = 8;
				colourshigh = 16;
				break;
			case 256:
				colourswide = 16;
				colourshigh = 16;
				break;
			default:
				colourswide = 8;
				colourshigh = 8;
				break;
		}
		colourwidth = boxwidth / colourswide;
		colourheight = boxheight / colourshigh;

		pb->pb_BoxLeft = boxleft;
		pb->pb_BoxTop = boxtop;
		pb->pb_BoxWidth = colourwidth * colourswide;
		pb->pb_BoxHeight = colourheight * colourshigh;
		pb->pb_ColourWidth = colourwidth;
		pb->pb_ColourHeight = colourheight;
		pb->pb_ColoursWide = colourswide;
		pb->pb_ColoursHigh = colourshigh;

		col = 0;
		x = pb->pb_BoxLeft;
		for(countx = 0; countx < colourswide; countx++)
		{
			y = pb->pb_BoxTop;
			for(county = 0; county < colourshigh; county++)
			{
				SetAPen(win->RPort,col);
				RectFill(win->RPort,x,y,x+colourwidth-1,y+colourheight-1);
				y += colourheight;
				col++;
			}
			x += colourwidth;
		}
	}
}


/**********************   BevelColour()   ********************/

void BevelColour(struct PaletteBox *pb, UWORD col)
{
	UWORD x,y;
	struct Window *win;

	if( (win = pb->pb_Window) && (col != 0xFFFF) &&
			(pb->pb_ColourWidth >= 3) && (pb->pb_ColourHeight >= 3) )
	{
		x = col/pb->pb_ColoursHigh;
		y = col-(x*pb->pb_ColoursHigh);

		x *= pb->pb_ColourWidth;
		y *= pb->pb_ColourHeight;

		x += pb->pb_BoxLeft;
		y += pb->pb_BoxTop;

		SetAPen(win->RPort,globpens[SHADOWPEN]);
		Move(win->RPort,x,y+pb->pb_ColourHeight-1);
		Draw(win->RPort,x,y);
		Draw(win->RPort,x+pb->pb_ColourWidth-1,y);
		SetAPen(win->RPort,globpens[SHINEPEN]);
		Draw(win->RPort,x+pb->pb_ColourWidth-1,y+pb->pb_ColourHeight-1);
		Draw(win->RPort,x,y+pb->pb_ColourHeight-1);
	}
}

/**********************   RestoreColour()   ********************/

void RestoreColour(struct PaletteBox *pb, UWORD col)
{
	UWORD x,y;
	struct Window *win;

	if( (win = pb->pb_Window) && (col != 0xFFFF) )
	{
		x = col/pb->pb_ColoursHigh;
		y = col-(x*pb->pb_ColoursHigh);

		x *= pb->pb_ColourWidth;
		y *= pb->pb_ColourHeight;

		x += pb->pb_BoxLeft;
		y += pb->pb_BoxTop;

		SetAPen(win->RPort,col);
		RectFill(win->RPort,x,y,x+pb->pb_ColourWidth-1,y+pb->pb_ColourHeight-1);

	}
}

/*********************  Mystrncpy()  ***********************/
//  This is just a slight varience from the original strncpy(),
// in that this clears the entire destination buffer, and then
// copies the string over top except for the last char, so that
// you always get a null ending.
//

char *Mystrncpy(char *to, char *from, int length)
{
	memset(to, 0x00, length);
	strncpy(to, from, length-1);
	return(to);
}

/*********************  FindMaxStrLen()  *********************/

UWORD FindMaxStrLen( STRPTR *str, struct TextFont *tf )
{
	UWORD i, len, maxlen = 0;

	for( i = 0; str[i]; i++ )
	{
			len = MyTextLength( tf, str[i] );
			if( len > maxlen ) maxlen = len;
	}
	return( maxlen );
}

/*********************  CountStrings()  *********************/
UWORD CountStrings( STRPTR *str )
{
	UWORD i=0;
	for( i = 0; str[i]; i++ );
	return( i );
}

/********************  ActivateWindowUnderMouse()  ************/

void ActivateWindowUnderMouse(void)
{
	struct Screen *scr;
	struct Window *win;
	struct Layer *layer;
	ULONG	lock;

    /* Find the Appropriate Screen */

	lock = LockIBase(0l);

	for(scr = IntuitionBase->FirstScreen;
			scr && (scr->TopEdge > 0) && (scr->MouseY < 0);
			scr = scr->NextScreen);

	if(scr == mainscreen)
	{
		/* Get layer */
		layer = WhichLayer(&scr->LayerInfo, (long)scr->MouseX, (long)scr->MouseY);

		if(layer && (layer != scr->BarLayer) )
		{
			/* Get window ptr */

			win = (struct Window *)layer->Window;

			if(win &&
				 win != IntuitionBase->ActiveWindow &&
				 !IntuitionBase->ActiveWindow->FirstRequest)
			{
				ActivateWindow(win);
			} /* if win */
		} /* if layer */
	} /* if scr */
	UnlockIBase(lock);
}

void SafeActivateWindowUnderMouse(void)
{
	if( glob.cg_AutoActivate && !toolstate)
	{
		ActivateWindowUnderMouse();
	}
}

UWORD Digits(UWORD num)
{
	UWORD digits = 1;

	while((num /= 10) != 0)
		digits++;

	return(digits);
}

/***********************   SimpleDoReq()   ***********************/
//
//  Open a new window with the given HoopyObject Structure. If
// parentwin is given it will open it just inside the window, else
// it will open it in the middle of the screen.
// NOTE: You have to look after blocking all the other windows.
// Returns: 0xFFFF if error.

UWORD HO_SimpleDoReq( struct HoopyObject *hob, char *title, struct Window *parentwin)
{
	struct Window *win;
	UWORD left,top,width,height,ret = 0xFFFF;

	width = HO_FindMinWidth(NULL, hob, windowtextfont);
	height = HO_FindMinHeight(NULL, hob, windowtextfont);

	if(parentwin)
	{
		left = parentwin->LeftEdge + parentwin->BorderLeft;
		top = parentwin->TopEdge + parentwin->BorderTop;
	}
	else
	{
		left = (mainscreen->Width/2) - (width/2);
		top = (mainscreen->Height/2) - (height/2);
	}

	if( win = OpenWindowTags( NULL,
		WA_Left, left,
		WA_Top, top,
		WA_InnerWidth, width,
		WA_InnerHeight, height,
		WA_MaxWidth, ~0,
		WA_MaxHeight, ~0,
		WA_PubScreen, mainscreen,
		WA_PubScreenFallBack, TRUE,
		WA_AutoAdjust,	TRUE,
		WA_Title,				title,
		WA_CloseGadget,	TRUE,
		WA_DragBar,			TRUE,
		WA_SizeGadget,	TRUE,
		WA_DepthGadget,	TRUE,
		WA_SizeBRight,	TRUE,
		WA_NewLookMenus,TRUE,
		WA_Activate,		TRUE,
		WA_IDCMP,				IDCMP_CLOSEWINDOW|IDCMP_NEWSIZE|IDCMP_REFRESHWINDOW|
			ARROWIDCMP|BUTTONIDCMP|CHECKBOXIDCMP|INTEGERIDCMP|LISTVIEWIDCMP|
			MXIDCMP|NUMBERIDCMP|CYCLEIDCMP|PALETTEIDCMP|SCROLLERIDCMP|SLIDERIDCMP|
			STRINGIDCMP,
		TAG_DONE,				NULL) )
	{
		ret = HO_DoReq( hob, win );
		CloseWindow( win );
	}
	return(ret);
}
