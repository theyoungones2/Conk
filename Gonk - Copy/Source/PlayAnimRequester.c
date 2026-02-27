 #include <stdio.h>
//#include <stdarg.h>
#include <string.h>
//#include <dos/dos.h>
//#include <dos/dostags.h>
#include <exec/exec.h>
//#include <intuition/intuition.h>
//#include <graphics/gfxbase.h>
//#include <rexx/rxslib.h>
#include <libraries/gadtools.h>
//#include <libraries/diskfont.h>
//#include <libraries/asl.h>

#include <proto/exec.h>
//#include <proto/dos.h>
#include <proto/intuition.h>
#include <proto/graphics.h>
//#include <proto/diskfont.h>
//#include <proto/asl.h>
//#include <clib/alib_protos.h>
#include <proto/gadtools.h>
#include <proto/iffparse.h>

#include <global.h>
#include <ExternVars.h>
#include <HoopyVision.h>

#define MYGT_FASTER	0
#define MYGT_SLOWER	1
#define MYGT_DONE	0xFFFF

static void DisplayFrame(struct Window *win, WORD boxleft, WORD boxtop, WORD boxwidth,
												 WORD boxheight, struct Frame *fr);

void PlayAnimRequester(struct Sequence *sq, struct Anim *an)
{
	static struct HoopyObject gadlayout[] =
	{
		{ HOTYPE_VGROUP, 100, HOFLG_FREEWIDTH|HOFLG_FREEHEIGHT,2,0,0,0, NULL, 0,0,0,0,0 },
			{ HOTYPE_SPACE, 1000, HOFLG_FREEWIDTH|HOFLG_FREEHEIGHT|HOFLG_RAISEDBORDER, 0,0,0,0, NULL,0,0,0,0,0 },
			{ HOTYPE_HGROUP, 100, HOFLG_NOBORDER|HOFLG_FREEWIDTH,3,0,0,0, NULL, 0,0,0,0,0 },
				{ HOTYPE_BUTTON, 100, 0, 0,0,0,0, "Faster",0,MYGT_FASTER,0,0,0 },
				{ HOTYPE_BUTTON, 100, 0, 0,0,0,0, "Slower",0,MYGT_SLOWER,0,0,0 },
				{ HOTYPE_BUTTON, 100, 0, 0,0,0,0, "Done",0,MYGT_DONE,0,0,0 },
		{HOTYPE_END}
	};
	struct Frame *fr;
	struct HOStuff *hos;
	struct	IntuiMessage		*imsg, *gtimsg;
	ULONG	mainsignal;
	UWORD	width, height;
	ULONG class;
	UWORD code;
	APTR iaddress;
	ULONG seconds, micros;
	struct Window *win;
	UWORD gadid = 0;
	UWORD *attrptr;
	UWORD animboxleft, animboxtop, animboxwidth, animboxheight;
	UWORD playspeed, speedcount = 0;

	if( (!sq) || (!an) || (IsListEmpty(&an->an_Frames)) )
		return;

	fr = (struct Frame *)an->an_Frames.lh_Head;
	playspeed = an->an_Speed;

	width = HO_FindMinWidth(NULL, gadlayout, windowtextfont);
	height = HO_FindMinHeight(NULL, gadlayout, windowtextfont) + 32;

	if( win = OpenWindowTags( NULL,
		WA_Left, (mainscreen->Width/2)-(width/2),
		WA_Top,  (mainscreen->Height/2)-(height/2),
		WA_InnerWidth, width,
		WA_InnerHeight, height,
		WA_MaxWidth, ~0,
		WA_MaxHeight, ~0,
		WA_PubScreen, mainscreen,
		WA_PubScreenFallBack, TRUE,
		WA_AutoAdjust,	TRUE,
		WA_Title,				"Play Anim",
		WA_CloseGadget,	TRUE,
		WA_DragBar,			TRUE,
		WA_SizeGadget,	TRUE,
		WA_DepthGadget,	TRUE,
		WA_SizeBRight,	TRUE,
		WA_NewLookMenus,TRUE,
		WA_Activate,		TRUE,
		WA_IDCMP,				IDCMP_CLOSEWINDOW|IDCMP_NEWSIZE|IDCMP_REFRESHWINDOW|
			BUTTONIDCMP,
		TAG_DONE,				NULL) )
	{
		if( hos = HO_GetHOStuff( win, win->RPort->Font ) )
		{
			HO_SussWindowSizing( hos, gadlayout );
			ClearWindow( win );
			HO_MakeLayout( gadlayout, hos, win->BorderLeft, win->BorderTop,
				win->Width - win->BorderLeft - win->BorderRight,
				win->Height - win->BorderTop - win->BorderBottom );

			attrptr = (UWORD *)&gadlayout[1].ho_Attr0;
			animboxleft = *attrptr++;
			animboxtop = *attrptr++;
			animboxwidth = *attrptr++;
			animboxheight = *attrptr++;

			DisplayFrame(win, animboxleft, animboxtop, animboxwidth, animboxheight, fr);

			mainsignal = 1L << win->UserPort->mp_SigBit;
			while ( gadid < 0x8000 )
			{
				;WaitBOVP(&mainscreen->ViewPort);
				WaitTOF();

				while( imsg = (struct IntuiMessage *)GetMsg( win->UserPort ) )
				{
					if( imsg->IDCMPWindow == win )
					{
						if( gtimsg = GT_FilterIMsg( imsg ) )
						{
							HO_CheckIMsg( gtimsg );

							class = gtimsg->Class;
							code = gtimsg->Code;
							iaddress = gtimsg->IAddress;
							seconds = gtimsg->Seconds;
							micros = gtimsg->Micros;

							GT_PostFilterIMsg( gtimsg );

							switch( class )
							{
								case IDCMP_CLOSEWINDOW:
									gadid = 0xFFFF;
									ReplyMsg( (struct Message *)imsg );
									break;
								case IDCMP_NEWSIZE:
									ClearWindow( win );
									HO_ReadGadgetStates( hos, gadlayout );
									HO_MakeLayout( gadlayout, hos,
										win->BorderLeft, win->BorderTop,
										win->Width - win->BorderLeft - win->BorderRight,
										win->Height - win->BorderTop - win->BorderBottom );
									attrptr = (UWORD *)&gadlayout[1].ho_Attr0;
									animboxleft = *attrptr++;
									animboxtop = *attrptr++;
									animboxwidth = *attrptr++;
									animboxheight = *attrptr++;
									ReplyMsg( (struct Message *)imsg );
									break;
								case IDCMP_GADGETUP:
									gadid = ((struct Gadget *)iaddress)->GadgetID;
									ReplyMsg( (struct Message *)imsg );
									switch(gadid)
									{
										case MYGT_FASTER:
											if(playspeed > 1) playspeed--;
											break;
										case MYGT_SLOWER:
											playspeed++;
											break;
									}
									break;
								case IDCMP_REFRESHWINDOW:
									GT_BeginRefresh(win);
									GT_EndRefresh(win,TRUE);
									ReplyMsg( (struct Message *)imsg );
									break;
								default:
									ReplyMsg( (struct Message *)imsg );
									break;
							}
						}
						else
							ReplyMsg( (struct Message *)imsg );
					}
					else
						ReplyMsg( (struct Message *)imsg );
				}
				speedcount++;
				if(speedcount >= playspeed)
				{
					speedcount = 0;
					if(fr->fr_Node.ln_Succ->ln_Succ)
						fr = (struct Frame *)fr->fr_Node.ln_Succ;
					else
						fr = (struct Frame *)an->an_Frames.lh_Head;
					DisplayFrame(win, animboxleft, animboxtop, animboxwidth, animboxheight, fr);
				}
			}
			HO_ReadGadgetStates( hos, gadlayout );
			HO_KillLayout( hos );
			HO_FreeHOStuff( hos );
		}
		CloseWindow(win);
		win = NULL;
	}
}

static void DisplayFrame(struct Window *win, WORD boxleft, WORD boxtop, WORD boxwidth,
												 WORD boxheight, struct Frame *fr)
{
	struct MyBob *bob;
	WORD left,right,top,bottom,width,height;
	WORD animboxright,animboxbottom;
	WORD imleft=0,imtop=0;

	/* The the whole box */
	SetAPen(win->RPort,0);
	RectFill(win->RPort, boxleft, boxtop, boxleft+boxwidth-1, boxtop+boxheight-1);

	if( (!fr) || !(bob = fr->fr_Bob) || (!bob->bb_Image) )
		return;

	animboxright = boxwidth-1;
	animboxbottom = boxheight-1;
	left = fr->fr_X;
	top = fr->fr_Y;
	right = left+bob->bb_Width-1;
	bottom = top+bob->bb_Height-1;
	if( (left > animboxright) || (top > animboxbottom) ||
			(right < 0) || (bottom < 0) )
		return;

	if(left < 0)
	{
		imleft = -left;
		left = 0;
	}
	if(right > animboxright) right = animboxright;
	if(top < 0)
	{
		imtop = -top;
		top = 0;
	}
	if(bottom > animboxbottom) bottom = animboxbottom;

	width = right-left+1;
	height = bottom-top+1;
	BltBitMapRastPort(bob->bb_Image,imleft,imtop,win->RPort,left+boxleft,top+boxtop,
				width,height,0xE0);
}
