/****************************************************************************/
//
// BONK: PubScreenRequester.c
//
/****************************************************************************/


#include <stdio.h>
#include <string.h>
#include <dos/dos.h>
#include <dos/dostags.h>
#include <exec/exec.h>
#include <intuition/intuition.h>
#include <graphics/gfxbase.h>
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

#include <global.h>
#include <ExternVars.h>

#define MYLV_NAMELIST 0
#define MYBT_OK				1
#define MYBT_CANCEL   2
#define NUMOFGADS			3

static struct Gadget			*gadgets[NUMOFGADS];
static struct Gadget			*glist;
static void								*vi;
static struct List				labels;
static struct Gadget			*lvgadget;
static struct TextAttr		*textattr;
static UWORD							currentselection;

static void IDCMPHandle(struct Window *win);
static void PubReqAdjustFont(struct Window *win, UWORD width, UWORD height);
static void RemakePubScreenList(struct Window *win);
static void FreePubScreenList(void);
static void DoGadgets(struct Window *win);


int PubScreenRequester(struct Window *parent, char *namebuffer)
{
//	struct Requester			req;
	struct Window					*win;
	UWORD									minh;

//	InitRequester(&req);
//  Request(&req,parent);
//	SetWait(parent);

	/* Set up the Visual Info for GadTools */
	vi = GetVisualInfo( parent->WScreen, TAG_END );

	minh = parent->WScreen->WBorTop + parent->WScreen->Font->ta_YSize + 11 + (4*8) + 16;

	win = OpenWindowTags(NULL,
											WA_Left,				100,
											WA_Top,					50,
											WA_Width,				200,
											WA_Height,			100,
											WA_MinWidth,		(((6 * 8) + 8) * 2) + 20,
											WA_MinHeight,		minh,
											WA_MaxWidth,		-1,
											WA_MaxHeight,		-1,
											WA_AutoAdjust,	TRUE,
											WA_Title,				"Public Screen Requester",
											WA_IDCMP,				NULL,
											WA_Flags,				WFLG_SIZEGADGET | WFLG_DRAGBAR |
																			WFLG_ACTIVATE | WFLG_DEPTHGADGET |
																			WFLG_CLOSEGADGET | WFLG_SIZEBBOTTOM,
											WA_CustomScreen,	parent->WScreen,
											TAG_DONE,				NULL);

	win->UserPort = parent->UserPort;
	ModifyIDCMP(win,
		IDCMP_CLOSEWINDOW | IDCMP_REFRESHWINDOW |	LISTVIEWIDCMP | IDCMP_NEWSIZE );

	SetFont(win->RPort, windowtextfont);

	NewList(&labels);
	lvgadget = NULL;
	currentselection = 0xFFFF;
	glist = NULL;

	RemakePubScreenList(win);

	DoGadgets(win);

	IDCMPHandle(win);
	CloseWindowSafely(win);

	FreeGadgets(glist);
	glist = NULL;

	FreeVisualInfo(vi);

	if(currentselection != 0xFFFF)
		strcpy(namebuffer,FindNthNode(&labels,currentselection)->ln_Name );

	FreePubScreenList();

//	ClrWait(parent);
//	EndRequest(&req,parent);

	if(currentselection == 0xFFFF)
		return(FALSE);
	else
		return(TRUE);
}



static void IDCMPHandle(struct Window *win)
{
	struct	IntuiMessage		*imsg;
	ULONG		mainsignal;
	int 		done 											= FALSE;

	mainsignal = 1L << win->UserPort->mp_SigBit;

	while(!done)
	{
		Wait(mainsignal);
		while ( !done && (imsg = GT_GetIMsg(win->UserPort)) )
		{
			switch (imsg->Class)
			{
				case IDCMP_REFRESHWINDOW:
					GT_BeginRefresh(win);
					GT_EndRefresh(win,TRUE);
					break;
				case IDCMP_CLOSEWINDOW:
					done = TRUE;
					currentselection = 0xFFFF;
					break;
				case IDCMP_NEWSIZE:
					DoGadgets(win);
					break;
				case IDCMP_GADGETUP:
					switch( ((struct Gadget *)imsg->IAddress)->GadgetID )
					{
						case MYLV_NAMELIST:
							/* listview */
							currentselection = imsg->Code;
							break;
						case MYBT_OK:
							done = TRUE;
							break;
						case MYBT_CANCEL:
							done = TRUE;
							currentselection = 0xFFFF;
							break;
					}
			}
			GT_ReplyIMsg(imsg);
		}
	}
}

/************** DoGadgets **************/
//
// Sets up and formats all the gadgets within the Pub Screen Requester.
// Adapts to font and window sizes.
// Call at window opening, and again at NEWSIZE events.

static void DoGadgets(struct Window *win)
{
	struct Gadget *gad;
	struct NewGadget ng;
	UWORD buttonh,buttonw;

	/* first, kill any old gadgets that may be active */
	if(glist)
	{
		RemoveGList(win, glist, -1);
		FreeGadgets(glist);
		glist = NULL;
	}

	ClearWindow( win );


	/* use window font */
	SetFont(win->RPort, windowtextfont);
	textattr = &windowtextattr;

	buttonw = TextLength(win->RPort,"Cancel",6) + 8;

	/* fall back to Topaz 8 if the window is too small */
	PubReqAdjustFont(win,(buttonw * 2) + 12,
									(windowtextfont->tf_YSize * 4) + 18);

	/* init gadget stuff */
	gad = CreateContext(&glist);

	buttonw = TextLength(win->RPort,"Cancel",6) + 8;
	buttonh = win->RPort->Font->tf_YSize + 3;

	/* create a ListView gadget */
	ng.ng_TextAttr = textattr;
	ng.ng_VisualInfo = vi;
	ng.ng_LeftEdge = win->BorderLeft + 4;
	ng.ng_TopEdge = win->BorderTop + 2;
	ng.ng_Width = win->Width - win->BorderLeft - win->BorderRight - 8;
	ng.ng_Height = win->Height - win->BorderTop - win->BorderBottom - buttonh - 8;
	ng.ng_GadgetText = NULL;
	ng.ng_GadgetID = MYLV_NAMELIST;
	ng.ng_Flags = 0;

	/* reattach the existing label list */
	gad = CreateGadget( LISTVIEW_KIND, gad, &ng,
		GTLV_Labels,&labels,
		GTLV_ShowSelected, NULL,
		TAG_END );

	/* want to remember listview gadget... */
	lvgadget = gad;

	/* Create Button Gadget ("OK") */
	ng.ng_LeftEdge = win->BorderLeft + 4;
	ng.ng_TopEdge = win->Height - win->BorderBottom - buttonh - 2;
	ng.ng_Width = buttonw;
	ng.ng_Height = buttonh;
	ng.ng_GadgetText = "OK";
	ng.ng_GadgetID = MYBT_OK;
	ng.ng_Flags = 0;
	gad = CreateGadget( BUTTON_KIND, gad, &ng, TAG_END );

	/* Create Button Gadget ("Cancel") */
	ng.ng_LeftEdge = win->Width - win->BorderRight -
		buttonw - 4;
	ng.ng_TopEdge = win->Height - win->BorderBottom - buttonh - 2;
	ng.ng_Width = buttonw;
	ng.ng_Height = buttonh;
	ng.ng_GadgetText = "Cancel";
	ng.ng_GadgetID = MYBT_CANCEL;
	ng.ng_Flags = 0;
	gad = CreateGadget( BUTTON_KIND, gad, &ng, TAG_END );

	/* check for error */
	if (!gad)
	{
		FreeGadgets(glist);
		return;
	}

	/* attach our groovy new gadgets to the window */
	AddGList(win, glist, -1, -1, NULL);
//	RefreshGList(glist, win, NULL, -1);
	GT_RefreshWindow(win, NULL);

	/* need to redraw the frame coz we're not using GimmeZeroZero */
	RefreshWindowFrame(win);
}


static void PubReqAdjustFont(struct Window *win, UWORD width, UWORD height)
{
	width = width + win->BorderLeft + win->BorderRight;
	height = height + win->BorderTop + win->BorderBottom;

	if( (win->Width < width) || (win->Height < height) )
	{
		SetFont(win->RPort,topaztextfont);
		textattr = &topaztextattr;
	}
}

static void RemakePubScreenList(struct Window *win)
{
	struct List								*publist;
	struct PubScreenNode			*pubnode;
	struct Node								*no;

	/* detach old labels from the gadget */
	GT_SetGadgetAttrs(lvgadget, win, NULL,
		GTLV_Labels,~0,TAG_END);

	/* free old label list if it exists */
	FreePubScreenList();

	/* create new labels */
	publist = LockPubScreenList();
	for(pubnode = (struct PubScreenNode *)publist->lh_Head;
		pubnode->psn_Node.ln_Succ;
		pubnode = (struct PubScreenNode *)pubnode->psn_Node.ln_Succ )
	{
		if( !ownscreen || ( pubnode->psn_Screen != MAINSCREEN ) )
		{
			no = AllocVec( sizeof(struct Node), MEMF_CLEAR);
			if(no)
			{
				no->ln_Name = MyStrDup(pubnode->psn_Node.ln_Name);
				AddTail( &labels, no );
			}
		}
	}
	/* 0xFFFF indicates no selection */
	currentselection = 0xFFFF;
	UnlockPubScreenList();

	/* attach new labels */
	GT_SetGadgetAttrs(lvgadget, win, NULL,
		GTLV_Labels,&labels,
		GTLV_Selected,~0,TAG_END);
}


/**********  FreePubScreenList()  **********/
//

static void FreePubScreenList(void)
{
	struct Node *no,*nextno;

	no = labels.lh_Head;
	while( !IsListEmpty(&labels) )
	{
		nextno = no->ln_Succ;
		Remove( no );
		FreeVec(no->ln_Name);
		FreeVec(no);
		no = nextno;
	}
}
