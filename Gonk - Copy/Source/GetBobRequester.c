#include <stdio.h>
//#include <stdarg.h>
#include <string.h>
//#include <dos/dos.h>
//#include <dos/dostags.h>
#include <exec/exec.h>
//#include <intuition/intuition.h>
//#include <graphics/gfxbase.h>
//#include <rexx/rxslib.h>
//#include <libraries/gadtools.h>
//#include <libraries/diskfont.h>
//#include <libraries/asl.h>

#include <proto/exec.h>
//#include <proto/dos.h>
#include <proto/intuition.h>
#include <proto/graphics.h>
#include <proto/diskfont.h>
//#include <proto/asl.h>
//#include <clib/alib_protos.h>
#include <proto/gadtools.h>

#include <global.h>
#include <ExternVars.h>

#define MYLV_NAMELIST 0
#define MYBT_OK				1
#define MYBT_CANCEL   2
#define NUMOFGADS			3

static struct Gadget			*gadgets[NUMOFGADS];
static struct Gadget			*glist;
static void								*vi;
static struct Gadget			*lvgadget;
static struct TextAttr		*textattr;
static struct List				 labels;
static UWORD							 currentselection;

static void IDCMPHandle(struct Window *win);
static void BobReqAdjustFont(struct Window *win, UWORD width, UWORD height);
static void RemakeBobList(struct Window *win, struct Project *pj);
static void FreeBobList(void);
static void DoGadgets(struct Window *win);


struct MyBob *GetBobRequester(struct ProjectWindow *pw, char *title)
{
	struct Window					*win,*parent;
	struct Project				*pj;
	struct MyBob					*bob;
	UWORD									 minh;

		title = "Select Bob";
	pj = pw->pw_Project;
	parent = pw->pw_sw.sw_Window;
	if(!parent)
		return(NULL);

	/* Set up the Visual Info for GadTools */
	vi = GetVisualInfo( parent->WScreen, TAG_END );

	minh = parent->WScreen->WBorTop + parent->WScreen->Font->ta_YSize + 11 + (4*8) + 16;

	win = OpenWindowTags(NULL,
											WA_Left,				100,
											WA_Top,					50,
											WA_Width,				200,
											WA_Height,			256,
											WA_MinWidth,		(((6 * 8) + 8) * 2) + 20,
											WA_MinHeight,		minh,
											WA_MaxWidth,		-1,
											WA_MaxHeight,		-1,
											WA_AutoAdjust,	TRUE,
											WA_Title,				title,
											WA_IDCMP,				NULL,
											WA_Flags,				WFLG_SIZEGADGET | WFLG_DRAGBAR |
																			WFLG_ACTIVATE | WFLG_DEPTHGADGET |
																			WFLG_CLOSEGADGET | WFLG_SIZEBBOTTOM,
											WA_CustomScreen,	parent->WScreen,
											TAG_DONE,				NULL);

	win->UserPort = parent->UserPort;
	ModifyIDCMP(win,
		IDCMP_CLOSEWINDOW | IDCMP_REFRESHWINDOW |	LISTVIEWIDCMP | BUTTONIDCMP | IDCMP_NEWSIZE );

	SetFont(win->RPort, windowtextfont);

	NewList(&labels);
	lvgadget = NULL;
	currentselection = 0xFFFF;
	glist = NULL;

	RemakeBobList(win,pj);

	DoGadgets(win);

	IDCMPHandle(win);
	CloseWindowSafely(win);

	FreeGadgets(glist);
	glist = NULL;

	FreeVisualInfo(vi);

	if(currentselection != 0xFFFF)
		bob = (struct MyBob *)((struct DataNode *)FindNthNode(&labels,currentselection))->dn_Ptr;

	FreeBobList();

	if(currentselection == 0xFFFF)
		return(NULL);
	else
		return(bob);
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

	buttonw = TextLength(win->RPort,Txt_Cancel,6) + 8;

	/* fall back to Topaz 8 if the window is too small */
	BobReqAdjustFont(win,(buttonw * 2) + 12,
										(windowtextfont->tf_YSize * 4) + 18);

	/* init gadget stuff */
	gad = CreateContext(&glist);

	buttonw = TextLength(win->RPort,Txt_Cancel,6) + 8;
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

	/* Create Button Gadget (Txt_OK) */
	ng.ng_LeftEdge = win->BorderLeft + 4;
	ng.ng_TopEdge = win->Height - win->BorderBottom - buttonh - 2;
	ng.ng_Width = buttonw;
	ng.ng_Height = buttonh;
	ng.ng_GadgetText = Txt_OK;
	ng.ng_GadgetID = MYBT_OK;
	ng.ng_Flags = 0;
	gad = CreateGadget( BUTTON_KIND, gad, &ng, TAG_END );

	/* Create Button Gadget (Txt_Cancel) */
	ng.ng_LeftEdge = win->Width - win->BorderRight -
		buttonw - 4;
	ng.ng_TopEdge = win->Height - win->BorderBottom - buttonh - 2;
	ng.ng_Width = buttonw;
	ng.ng_Height = buttonh;
	ng.ng_GadgetText = Txt_Cancel;
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


static void BobReqAdjustFont(struct Window *win, UWORD width, UWORD height)
{
	width = width + win->BorderLeft + win->BorderRight;
	height = height + win->BorderTop + win->BorderBottom;

	if( (win->Width < width) || (win->Height < height) )
	{
		SetFont(win->RPort,topaztextfont);
		textattr = &topaztextattr;
	}
}

static void RemakeBobList(struct Window *win, struct Project *pj)
{
	struct DataNode						*dn;
	struct MyBob							*bob;

	/* detach old labels from the gadget */
	GT_SetGadgetAttrs(lvgadget, win, NULL,
		GTLV_Labels,~0,TAG_END);

	/* free old label list if it exists */
	FreeBobList();

	/* create new labels */
	for(bob = (struct MyBob *)pj->pj_BobList.lh_Head;
			bob->bb_Node.ln_Succ;
			bob = (struct MyBob *)bob->bb_Node.ln_Succ)
	{
		if( dn = AllocVec( sizeof(struct DataNode), MEMF_CLEAR) )
		{
			if(dn->dn_Node.ln_Name = AllocVec( 4+14, MEMF_ANY|MEMF_CLEAR))
			{
				sprintf(dn->dn_Node.ln_Name, "%ld: %s",bob->bb_Number, bob->bb_Node.ln_Name);
				dn->dn_Ptr = bob;
				AddTail( &labels, &dn->dn_Node );
			}
		}
	}
	/* 0xFFFF indicates no selection */
	currentselection = 0xFFFF;

	/* attach new labels */
	GT_SetGadgetAttrs(lvgadget, win, NULL,
		GTLV_Labels,&labels,
		GTLV_Selected,~0,TAG_END);
}


/**********  FreeBobList()  **********/
//

static void FreeBobList(void)
{
	struct DataNode *dn,*nextdn;

	dn = (struct DataNode *)labels.lh_Head;
	while( !IsListEmpty(&labels) )
	{
		nextdn = (struct DataNode *)dn->dn_Node.ln_Succ;
		Remove( &dn->dn_Node );
		if(dn->dn_Node.ln_Name) FreeVec(dn->dn_Node.ln_Name);
		FreeVec(dn);
		dn = nextdn;
	}
}
