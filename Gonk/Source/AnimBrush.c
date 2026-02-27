 
#include <stdio.h>
//#include <stdarg.h>
//#include <string.h>
//#include <dos/dos.h>
//#include <dos/dostags.h>
#include <exec/exec.h>
#include <intuition/intuition.h>
//#include <graphics/gfxbase.h>
//#include <rexx/rxslib.h>
//#include <libraries/gadtools.h>
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

#include <graphics/gfx.h>
#include <global.h>
#include <ExternVars.h>
#include <HoopyVision.h>

/*************************    InitAnimBrush()    ***************************/
//
//  Call this at startup.

void InitAnimBrush(struct AnimBrush *ab)
{
	if(ab)
	{
		NewList(&ab->ab_ABrushList);
		ab->ab_Direction = ABDIR_FORWARD;
		ab->ab_PingPongDirection = ABDIR_FORWARD;
	}
}


/*************************    GUIGetAnimBrushFrames()    ***************************/
//
//  Bring up a requester to ask how many frames you want to cut out. Then call up
// GetAnimBrushFrames().

BOOL GUIGetAnimBrushFrames(struct AnimBrush *ab, struct ProjectWindow *pw,
													 WORD left, WORD top, WORD width, WORD height, UWORD button)
{
	BOOL ret = FALSE;
	static struct HoopyObject gadlayout[] =
	{
		{ HOTYPE_VGROUP, 100, HOFLG_FREEWIDTH|HOFLG_FREEHEIGHT,3,0,0,0, NULL, 0,0,0,0,0 },
			{ HOTYPE_INTEGER, 100, 0, 4,0,0,0, "# of Frames",1,0x8001,PLACETEXT_LEFT,0,0 },
			{ HOTYPE_SPACE, 10, HOFLG_NOBORDER|HOFLG_FREEWIDTH|HOFLG_FREEHEIGHT, 0,0,0,0, NULL,0,0,0,0,0 },
			{ HOTYPE_HGROUP, 100, HOFLG_NOBORDER|HOFLG_FREEWIDTH,3,0,0,0, NULL, 0,0,0,0,0 },
				{ HOTYPE_BUTTON, 100, HOFLG_FREEWIDTH, 0,0,0,0, "OK",0,0x8000,0,0,0 },
				{ HOTYPE_SPACE, 500, HOFLG_NOBORDER|HOFLG_FREEWIDTH, 0,0,0,0, NULL,0,0,0,0,0 },
				{ HOTYPE_BUTTON, 100, HOFLG_FREEWIDTH, 0,0,0,0, "Cancel",0,0xFFFF,0,0,0 },
		{HOTYPE_END}
	};

	if(HO_SimpleDoReq( gadlayout, "Pick Up Anim Brush", NULL ) != 0xFFFF)
		ret = GetAnimBrushFrames(ab, pw->pw_Project, gadlayout[1].ho_Value, left, top, width, height, button);
	return(ret);
}


/*************************    GetAnimBrushFrames()    ***************************/
//
// Cut out the animbrush from the given project.

BOOL GetAnimBrushFrames(struct AnimBrush *ab, struct Project *pj,
												UWORD numofframes, WORD left, WORD top, WORD width, WORD height,
												UWORD button)
{
	struct MyBob *bob;
	struct ABrush *abr;
	UWORD count, depth;
	BOOL abort = FALSE;

	if( (!ab) || (!pj) || (numofframes == 0) || (width <= 0) || (height <= 0) ||
		(left < 0) || (top < 0) ||
		(left+width > pj->pj_Width) && (top+height > pj->pj_Height) )
	{
		DisplayBeep(mainscreen);
		return(FALSE);
	}
	if( (pj->pj_EditType != EDTY_BOBS) && (pj->pj_EditType != EDTY_SPRITES) )
	{
		GroovyReq("Pick Up Anim Brush","Must be Editing Either Bobs or Sprites","Opps");
		return(FALSE);
	}

	if(pj->pj_EditBobMask)
		TurnOnBobImageEditing(pj);

	if(pj->pj_CurrentBobModified)
		StoreBob(pj, pj->pj_CurrentBob);

	if( (!pj->pj_BobList.lh_Head->ln_Succ) ||
			(!pj->pj_BobList.lh_Head->ln_Succ->ln_Succ) )
	{
		GroovyReq("Pick Up Anim Brush","No Bob/Sprites In Project","Oops");
		return(FALSE);
	}

	FreeAnimBrush(&animbrush);
	depth = pj->pj_Depth;
	ab->ab_Width = width;
	ab->ab_Height = height;
	ab->ab_Depth = depth;

	bob = pj->pj_CurrentBob;
	for(count = 0; (count < numofframes) && (!abort); count++)
	{
		if(abr = AllocABrush(width, height, depth))
		{
			AddTail(&ab->ab_ABrushList, &abr->ab_Node);
			BltBitMap(pj->pj_MainBitMap, left, top, abr->ab_Image, 0, 0, width, height, 0xC0, 0xFF, NULL);
			if(bob->bb_Mask)
				BltBitMap(pj->pj_MaskBitMap, left, top, abr->ab_Mask, 0, 0, width, height, 0xC0, 0xFF, NULL);
			else
				NastyMakeColourMask(abr->ab_Image, abr->ab_Mask, width, height, 0);
			if( button == IECODE_MBUTTON)
			{
				DoExtraSmartColSBox(pj->pj_ProjectWindows, NULL, left, top, left+width-1, top+height-1, mmbcol);
				SussOutModificationStuff(pj);
			}
			if( button == IECODE_RBUTTON)
			{
				DoExtraSmartColSBox(pj->pj_ProjectWindows, NULL, left, top, left+width-1, top+height-1, rmbcol);
				SussOutModificationStuff(pj);
			}
			if(count+1 < numofframes)
			{
				bob = (struct MyBob *)bob->bb_Node.ln_Succ;
				if(!bob->bb_Node.ln_Succ->ln_Succ)
					bob = (struct MyBob *)pj->pj_BobList.lh_Head;
				GotoNewBob(pj, bob);
				RefreshProjectName(pj);
			}
		}
		else
		{
			GroovyReq("Pick Up Anim Brush","Not Enough Memory","Bugger");
			abort = TRUE;
		}
	}

	brushes[BR_ANIMBRUSH].br_DrawBrush.bi_XHandle = ab->ab_Width/2;
	brushes[BR_ANIMBRUSH].br_DrawBrush.bi_YHandle = ab->ab_Height/2;

	if(!abort)
		SetAnimBrushFrame(ab, (struct ABrush *)ab->ab_ABrushList.lh_Head);
	else
		FreeAnimBrush(ab);
}

/*************************    SetAnimBrushFrame()    ***************************/
//
//  Change the animbrush current frame. Also updates the brush for plot routines.

void SetAnimBrushFrame(struct AnimBrush *ab, struct ABrush *abr)
{
	ab->ab_CurrentABrush = (struct ABrush *)abr;
	brushes[BR_ANIMBRUSH].br_DrawBrush.bi_Width = ab->ab_Width;
	brushes[BR_ANIMBRUSH].br_DrawBrush.bi_Height = ab->ab_Height;
	brushes[BR_ANIMBRUSH].br_DrawBrush.bi_Depth = ab->ab_Depth;
	brushes[BR_ANIMBRUSH].br_DrawBrush.bi_Image = abr->ab_Image;
	brushes[BR_ANIMBRUSH].br_DrawBrush.bi_Mask = abr->ab_Mask;
}

/*************************    AllocABrush()    ***************************/
//
//  Allocate a new ABrush structure. No linkage is performed.

struct ABrush *AllocABrush(UWORD width, UWORD height, UWORD depth)
{
	struct ABrush *abr = NULL;
	BOOL abort = FALSE;

	if(abr = AllocVec(sizeof(struct ABrush),MEMF_ANY|MEMF_CLEAR))
	{
		if(abr->ab_Image = CreateBitMap(width, height, depth, FALSE))
		{
			if(abr->ab_Mask = CreateBitMap(width, height, 1, FALSE))
			{
			}
			else
				abort = TRUE;
		}
		else
			abort = TRUE;
	}
	else
		abort = TRUE;

	if( (abort) && (abr) )
	{
		if(abr->ab_Image)
			DestroyBitMap(abr->ab_Image, width, height, depth);
		if(abr->ab_Mask)
			DestroyBitMap(abr->ab_Mask, width, height, 1);
		FreeVec(abr);
		abr = NULL;
	}
	return(abr);
}

/*************************    FreeABrush()    ***************************/
//
//  Unlinks and dealloc's the given ABrush.

void FreeAnimBrush(struct AnimBrush *ab)
{
	struct ABrush *abr;

	while(!IsListEmpty(&ab->ab_ABrushList))
	{
		abr = (struct ABrush *)ab->ab_ABrushList.lh_Head;
		Remove(&abr->ab_Node);
		FreeABrushBitMaps(ab, abr);
		FreeVec(abr);
	}
}

/*************************    AllocABrush()    ***************************/
//
//  Deallocate the ABrush BitMaps.

void FreeABrushBitMaps(struct AnimBrush *ab, struct ABrush *abr)
{
	if(abr->ab_Image)
		DestroyBitMap(abr->ab_Image, ab->ab_Width, ab->ab_Height, ab->ab_Depth);
	if(abr->ab_Mask)
		DestroyBitMap(abr->ab_Mask, ab->ab_Width, ab->ab_Height, 1);
}

/*************************    GotoNextAnimBrushFrame()    ***************************/
//
//  Goto the Next Frame, if at the end, then goto the start again.

void GotoNextAnimBrushFrame(struct AnimBrush *ab)
{
	UWORD direction;

	if(!IsListEmpty(&ab->ab_ABrushList))
	{
		direction = ab->ab_Direction;
		if(direction == ABDIR_PINGPONG)
			direction = ab->ab_PingPongDirection;

		switch(direction)
		{
			case ABDIR_FORWARD:
				if( (ab->ab_CurrentABrush->ab_Node.ln_Succ) && (ab->ab_CurrentABrush->ab_Node.ln_Succ->ln_Succ) )
					SetAnimBrushFrame(ab, (struct ABrush *)ab->ab_CurrentABrush->ab_Node.ln_Succ);
				else
				{
					if(ab->ab_Direction == ABDIR_PINGPONG)
					{
						if( (ab->ab_CurrentABrush->ab_Node.ln_Pred) && (ab->ab_CurrentABrush->ab_Node.ln_Pred->ln_Pred) )
							SetAnimBrushFrame(ab, (struct ABrush *)ab->ab_CurrentABrush->ab_Node.ln_Pred);
						ab->ab_PingPongDirection = ABDIR_REVERSE;
					}
					else
						SetAnimBrushFrame(ab, (struct ABrush *)ab->ab_ABrushList.lh_Head);
				}
				break;
			case ABDIR_REVERSE:
				if( (ab->ab_CurrentABrush->ab_Node.ln_Pred) && (ab->ab_CurrentABrush->ab_Node.ln_Pred->ln_Pred) )
					SetAnimBrushFrame(ab, (struct ABrush *)ab->ab_CurrentABrush->ab_Node.ln_Pred);
				else
				{
					if(ab->ab_Direction == ABDIR_PINGPONG)
					{
						if( (ab->ab_CurrentABrush->ab_Node.ln_Succ) && (ab->ab_CurrentABrush->ab_Node.ln_Succ->ln_Succ) )
							SetAnimBrushFrame(ab, (struct ABrush *)ab->ab_CurrentABrush->ab_Node.ln_Succ);
						ab->ab_PingPongDirection = ABDIR_FORWARD;
					}
					else
						SetAnimBrushFrame(ab, (struct ABrush *)ab->ab_ABrushList.lh_TailPred);
				}
				break;
		}
	}
}
