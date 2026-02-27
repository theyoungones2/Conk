#include <stdio.h>
//#include <stdarg.h>
//#include <string.h>
//#include <dos/dos.h>
//#include <dos/dostags.h>
#include <exec/exec.h>
//#include <intuition/intuition.h>
//#include <graphics/gfxbase.h>
//#include <rexx/rxslib.h>
//#include <libraries/gadtools.h>
//#include <libraries/diskfont.h>
//#include <libraries/asl.h>
#include <hardware/custom.h>

#include <proto/exec.h>
#include <proto/dos.h>
#include <proto/intuition.h>
#include <proto/graphics.h>
//#include <proto/diskfont.h>
#include <proto/asl.h>
//#include <clib/alib_protos.h>
#include <proto/gadtools.h>
#include <proto/iffparse.h>

#include <graphics/modeid.h>

#include <global.h>
#include <ExternVars.h>
#include <HoopyVision.h>

#define	ID_BOBS		MAKE_ID('B','O','B','S')
#define	ID_CONK		MAKE_ID('C','O','N','K')

extern long __far DefaultBobImage;

static BOOL WriteChunkyBobData(struct MyBob *bob, struct IFFHandle *handle);
static BOOL WritePlanerBobData(struct MyBob *bob, struct IFFHandle *handle);
static BOOL WriteBobData(struct IFFHandle *handle, struct BobFileHeader *bh, struct MyBob *bob);
static BOOL WriteBOBSChunk( struct IFFHandle *handle, struct Project *pj, char *name);
static BOOL LoadInPlanerImage(struct MyBob *bob, struct IFFHandle *handle);
static BOOL LoadInChunkyImage(struct MyBob *bob, struct IFFHandle *handle);
static BOOL LoadInMask(struct MyBob *bob, struct IFFHandle *handle);
static BOOL ReadPlanerKludgeImage(struct MyBob *bob, struct IFFHandle *handle);
static BOOL ReadChunkyKludgeImage(struct MyBob *bob, struct IFFHandle *handle);
static BOOL ReadKludgeMask(struct MyBob *bob, struct IFFHandle *handle);
static void RenumberBobs(struct List *list);

BOOL TestRow(struct BitMap *bm,UWORD x, UWORD y, UWORD width)
{
	LONG bmoffset;
	UWORD xcount,plane;

	width = (width+15)/8;

	bmoffset = (y * bm->BytesPerRow) + ((x / 8) & 0xFFFE);

	for(plane = 0; plane < bm->Depth; plane++)
	{
		for(xcount = 0; xcount < width; xcount+=2)
		{
			if( *(UWORD *)(bm->Planes[plane] + bmoffset + xcount) )
				return(TRUE);
		}
	}
	return(FALSE);
}

BOOL TestColumn(struct BitMap *bm, UWORD x, UWORD y, UWORD height)
{
	LONG bmoffset;
	UWORD wordmask;
	UWORD ycount,plane;

	bmoffset = (y * bm->BytesPerRow) + ((x / 8) & 0xFFFE);
	wordmask = 1<<(15 - (x & 0x000F));

	for(ycount = 0; ycount < height; ycount++)
	{
		for(plane = 0; plane < bm->Depth; plane++)
		{
			if( wordmask & *(UWORD *)(bm->Planes[plane] + bmoffset) )
				return(TRUE);
		}
		bmoffset += bm->BytesPerRow;
	}
	return(FALSE);
}

BOOL TestColumnWord(struct BitMap *bm, UWORD x, UWORD y, UWORD height)
{
	LONG bmoffset;
	UWORD ycount,plane;

	bmoffset = (y * bm->BytesPerRow) + ((x / 8) & 0xFFFE);

	for(ycount = 0; ycount < height; ycount++)
	{
		for(plane = 0; plane < bm->Depth; plane++)
		{
			if( *(UWORD *)(bm->Planes[plane] + bmoffset) )
				return(TRUE);
		}
		bmoffset += bm->BytesPerRow;
	}
	return(FALSE);
}

/**************************   InitBobMode()   ***********************/
//
//  Put the given project into Bob edit mode

BOOL InitBobMode(struct Project *pj, UWORD newdepth)
{
	struct Palette *pal;
	struct ProjectWindow *pw;
	UWORD width,height;

	if(pj->pj_EditType != EDTY_BOBS)
	{
		WaitForSubTaskDone(mainmsgport);
		if(pj->pj_ModifiedFlag)
		{
			if( !GroovyReq("Gonk","Warning: This Will Erase\nThe Current Project","Kill Project|Cancel") )
			{
				RefreshAllMenus();
				return(FALSE);
			}
		}
		DeallocProjBitMaps(pj);

		pj->pj_Depth = newdepth;

		width = mainscreen->Width;
		height = mainscreen->Height;

		if( !glob.cg_SaveChunky )
		{
			if(width > BOB_MAXWIDTH)
				width = BOB_MAXWIDTH;
			if(height > (BOB_MAXHEIGHT/pj->pj_Depth))
				height = BOB_MAXHEIGHT/pj->pj_Depth;
		}

		if( (pj->pj_MainRPort = AllocBitMapRPort(width, height, pj->pj_Depth)) &&
				(pj->pj_UndoBitMap = CreateBitMap(width, height, pj->pj_Depth, TRUE)) &&
				(pj->pj_MaskBitMap = CreateBitMap(width, height, 1, TRUE)) )
		{
			pj->pj_MainBitMap = pj->pj_MainRPort->BitMap;
			pj->pj_Width = width;
			pj->pj_Height = height;
			pj->pj_EditType = EDTY_BOBS;

			if( IsListEmpty(&pj->pj_BobList) )
				AllocNextBob(pj, &pj->pj_BobList);
			EditBob(pj, (struct MyBob *)pj->pj_BobList.lh_Head);
			/* Bring up the Bob Name Gadget (EditBob doesn't do this (for speed )) */
			for(pw = pj->pj_ProjectWindows; pw; pw = pw->pw_NextProjWindow)
				RedisplayProjWindow(pw,TRUE);
			RefreshProjectName(pj);
			return(TRUE);
		}
		else
		{
			DeallocProjBitMaps(pj);
			GroovyReq("Gonk","Not Enough Memory\nTerminating Project","Continue");
			if( pal = CloseProject(pj) )
				InstallNewGlobPalette(pal);
		}
	}
	return(FALSE);
}

struct MyBob *AllocBob(struct Project *pj, struct List *boblist)
{
	struct MyBob *bob;

	if( bob = AllocVec(sizeof(struct MyBob),MEMF_ANY|MEMF_CLEAR) )
	{
		bob->bb_Node.ln_Name = bob->bb_Name;
		bob->bb_Depth = pj->pj_Depth;
		bob->bb_Flags = 0;
		bob->bb_SpriteNumber = 0;
		return(bob);
	}
	return(NULL);
}

struct MyBob *AllocNextBob(struct Project *pj, struct List *boblist)
{
	struct MyBob *bob;

	if( bob = AllocBob(pj, boblist) )
		AppendBobToList(boblist,bob);
	return(bob);
}

void AppendBobToList(struct List *boblist, struct MyBob *bob)
{
	if( IsListEmpty(boblist) )
	{
		AddTail(boblist,&bob->bb_Node);
		bob->bb_Number = 1;
	}
	else
	{
		AddTail(boblist,&bob->bb_Node);
		bob->bb_Number = (((struct MyBob *)bob->bb_Node.ln_Pred)->bb_Number) + 1;
	}
}

void FreeBobBitMaps(struct MyBob *bob)
{
	if(bob->bb_Image)
	{
		DestroyBitMap(bob->bb_Image, bob->bb_Width, bob->bb_Height, bob->bb_Depth);
		bob->bb_Image = NULL;
	}
	if(bob->bb_Mask)
	{
		DestroyBitMap(bob->bb_Mask, bob->bb_Width, bob->bb_Height, 1);
		bob->bb_Mask = NULL;
	}
}

void EditBob(struct Project *pj,struct MyBob *bob)
{
	struct ProjectWindow *pw;
	UBYTE olddepth;
	WORD left,top,width,height;

	if(lastprojectwindow)
		EraseSelector(lastprojectwindow);

	pj->pj_CurrentBob = bob;
	pj->pj_CurrentBobModified = FALSE;

	BltBitMap(pj->pj_MainBitMap,0,0,
						pj->pj_MainBitMap,0,0,pj->pj_Width, pj->pj_Height,0x00,0xFF,NULL);
	BltBitMap(pj->pj_MaskBitMap,0,0,
						pj->pj_MaskBitMap,0,0,pj->pj_Width, pj->pj_Height,0x00,0xFF,NULL);

	if(bob->bb_Image || bob->bb_Mask)
	{
		width = bob->bb_Width;
		if(width > pj->pj_Width)
		{
			DisplayBeep(mainscreen);
			width = pj->pj_Width;
		}
		height = bob->bb_Height;
		if(height > pj->pj_Height)
		{
			DisplayBeep(mainscreen);
			height = pj->pj_Height;
		}
		left = bob->bb_Left;
		if((left+width) > pj->pj_Width)
			left = pj->pj_Width - width;
		if(left < 0)
			left = 0;
		top = bob->bb_Top;
		if((top+height) > pj->pj_Height)
			top = pj->pj_Height - height;
		if(top < 0)
			top = 0;
	}

	if(!pj->pj_EditBobMask)
	{
		if(bob->bb_Image)
		{
			BltBitMap(bob->bb_Image, 0, 0,
								pj->pj_MainBitMap, left, top, width, height, 0xC0, 0xFF, NULL);
		}
		if(bob->bb_Mask)
		{
			BltBitMap(bob->bb_Mask, 0, 0,
								pj->pj_MaskBitMap, left, top, width, height, 0xC0, 0xFF, NULL);
			pj->pj_BobHasMask = TRUE;
		}
		else
			pj->pj_BobHasMask = FALSE;
	}
	else
	{
		if(bob->bb_Mask)
		{
			BltBitMap(bob->bb_Mask, 0, 0,
								pj->pj_MaskBitMap, left, top, width, height, 0xC0, 0xFF, NULL);

			olddepth = pj->pj_MainBitMap->Depth;
			pj->pj_MainBitMap->Depth = 1;
			BltBitMap(pj->pj_MaskBitMap,0,0,
								pj->pj_MainBitMap,0,0,pj->pj_Width, pj->pj_Height,0xC0,0xFF,NULL);
			WaitBlit();
			pj->pj_MainBitMap->Depth = olddepth;

			pj->pj_BobHasMask = TRUE;
		}
		else
		{
			if(bob->bb_Image)
			{
				BltBitMap(bob->bb_Image, 0, 0,
									pj->pj_MainBitMap, left, top, width, height, 0xC0, 0xFF, NULL);
				WaitBlit();

				NastyMakeColourMask(pj->pj_MainBitMap,pj->pj_MaskBitMap,pj->pj_Width,pj->pj_Height,0);
	
				BltBitMap(pj->pj_MainBitMap,0,0,
									pj->pj_MainBitMap,0,0,pj->pj_Width,pj->pj_Height,0x00,0xFF,NULL);

				WaitBlit();
				olddepth = pj->pj_MainBitMap->Depth;
				pj->pj_MainBitMap->Depth = 1;
				BltBitMap(pj->pj_MaskBitMap,0,0,
									pj->pj_MainBitMap,0,0,pj->pj_Width,pj->pj_Height,0xC0,0xFF,NULL);
				WaitBlit();
				pj->pj_MainBitMap->Depth = olddepth;
	
				pj->pj_BobHasMask = TRUE;
				pj->pj_CurrentBobModified = TRUE;
			}	
		}
	}
	BltBitMap(pj->pj_MainBitMap,0,0,
						pj->pj_UndoBitMap,0,0, pj->pj_Width, pj->pj_Height,0xC0,0xFF,NULL);

	for(pw = pj->pj_ProjectWindows; pw; pw = pw->pw_NextProjWindow)
		RedisplayProjWindow(pw,FALSE);

	if(lastprojectwindow)
		DrawSelector(lastprojectwindow, lastprojectwindow->pw_SelX0, lastprojectwindow->pw_SelY0);
}


void GotoPreviousBob(struct Project *pj)
{
	if(pj->pj_EditType == EDTY_BOBS)
	{
		if( (pj->pj_CurrentBob->bb_Node.ln_Pred) &&
				(pj->pj_CurrentBob->bb_Node.ln_Pred->ln_Pred) )
		GotoNewBob(pj, (struct MyBob *)pj->pj_CurrentBob->bb_Node.ln_Pred);
		RefreshProjectName(pj);
	}
}

void GotoNextBob(struct Project *pj)
{
	if(pj->pj_EditType == EDTY_BOBS)
	{
		if( (pj->pj_CurrentBob->bb_Node.ln_Succ) &&
				(pj->pj_CurrentBob->bb_Node.ln_Succ->ln_Succ) )
		GotoNewBob(pj, (struct MyBob *)pj->pj_CurrentBob->bb_Node.ln_Succ);
		RefreshProjectName(pj);
	}
}

void GotoNewBob(struct Project *pj, struct MyBob *bob)
{
	if(pj->pj_CurrentBob)
	{
		if(pj->pj_CurrentBobModified)
		{
			if(StoreBob(pj, pj->pj_CurrentBob))
			{
				EditBob(pj,bob);
			}
		}
		else
		{
			EditBob(pj,bob);
		}
	}
}

BOOL StoreBobImage(struct Project *pj, struct MyBob *bob)
{
	UWORD top,bottom,left,right;
	WORD pos,finepos;

	if(bob->bb_Image)
	{
		DestroyBitMap(bob->bb_Image, bob->bb_Width, bob->bb_Height, bob->bb_Depth);
		bob->bb_Image = NULL;
	}

		/* Calc Left Pos */
	for(pos = 0;
		 (pos < pj->pj_Width) && (!TestColumnWord(pj->pj_MainBitMap,pos,0,pj->pj_Height));
			pos += 16) {}
	if(pos < pj->pj_Width)
	{
		pos = pos & 0xFFF0;
		for(finepos = pos;
				(finepos < pos+16) && (!TestColumn(pj->pj_MainBitMap,finepos,0,pj->pj_Height));
				finepos++) {}
		left = finepos;
	}
	else
		return(TRUE);

		/* Calc Right Pos */
	for(pos = pj->pj_Width-1;
		 (pos >= left) && (!TestColumnWord(pj->pj_MainBitMap,pos,0,pj->pj_Height));
			pos -= 16) {}
	pos = pos & 0xFFF0;
	for(finepos = pos+15;
			(finepos >= pos) && (!TestColumn(pj->pj_MainBitMap,finepos,0,pj->pj_Height));
			finepos--) {}
	right = finepos;

		/* Calc Top Pos */
	for(pos = 0;
		 (pos < pj->pj_Height) && (!TestRow(pj->pj_MainBitMap,left,pos,right-left+1));
			pos++) {}
	top = pos;

		/* Calc Bottom Pos */
	for(pos = pj->pj_Height-1;
		 (pos >= top) && (!TestRow(pj->pj_MainBitMap,left,pos,right-left+1));
			pos--) {}
	bottom = pos;

	if( bob->bb_Image = CreateBitMap(right-left+1,bottom-top+1,pj->pj_Depth,TRUE) )
	{
		bob->bb_Left = left;
		bob->bb_Top = top;
		bob->bb_Width = right-left+1;
		bob->bb_Height = bottom-top+1;
		bob->bb_Depth = pj->pj_Depth;
		BltBitMap(pj->pj_MainBitMap,left,top,
							bob->bb_Image, 0, 0, right-left+1,bottom-top+1,0xC0,0xFF,NULL);
		return(TRUE);
	}
	else
	{
		FreeBobBitMaps(bob);
		GroovyReq("Gonk Error","Not Enough Memory\nTo Store Bob","Continue");
		return(FALSE);
	}
}

BOOL StoreBobMask(struct Project *pj, struct MyBob *bob)
{
	UBYTE olddepth;
	WORD pos;

	for(pos = 0;
		 (pos < pj->pj_Width) && (!TestColumnWord(pj->pj_MainBitMap,pos,0,pj->pj_Height));
			pos += 16) {}
	if(pos < pj->pj_Width)
	{
		olddepth = pj->pj_MainBitMap->Depth;
		pj->pj_MainBitMap->Depth = 1;
		BltBitMap(pj->pj_MainBitMap,0,0,
							pj->pj_MaskBitMap,0,0,pj->pj_Width,pj->pj_Height,0xC0,0xFF,NULL);
		WaitBlit();
		pj->pj_MainBitMap->Depth = olddepth;
		pj->pj_BobHasMask = TRUE;
	}
	else
		pj->pj_BobHasMask = FALSE;

	return(TRUE);
}

BOOL StoreBob(struct Project *pj, struct MyBob *bob)
{
	UWORD itop,ibottom,ileft,iright,mtop,mbottom,mleft,mright;
	WORD pos,finepos;
	BOOL hasimage=FALSE,hasmask=FALSE;
	UBYTE olddepth;

	/* If the Bob has no mask, then clear the mask bitmap, just for safety sake */
	if(!pj->pj_BobHasMask)
	{
		BltBitMap(pj->pj_MaskBitMap,0,0,
							pj->pj_MaskBitMap,0,0,pj->pj_Width,pj->pj_Height,0x00,0xFF,NULL);		
	}

	/* If editing the mask, then we have to copy the mask out into the mask bitmap
		, then copy the image back into the main bitmap */
	if(pj->pj_EditBobMask)
	{
		olddepth = pj->pj_MainBitMap->Depth;
		pj->pj_MainBitMap->Depth = 1;
		BltBitMap(pj->pj_MainBitMap,0,0,
							pj->pj_MaskBitMap,0,0,pj->pj_Width,pj->pj_Height,0xC0,0xFF,NULL);
		WaitBlit();
		pj->pj_MainBitMap->Depth = olddepth;

		/* Clear The MainBitMap */
		BltBitMap(pj->pj_MainBitMap,0,0,
							pj->pj_MainBitMap,0,0,pj->pj_Width,pj->pj_Height,0x00,0xFF,NULL);

		if(bob->bb_Image)
		{
			BltBitMap(bob->bb_Image,0,0,
								pj->pj_MainBitMap,bob->bb_Left,bob->bb_Top,
								bob->bb_Width,bob->bb_Height,0xC0,0xFF,NULL);
		}

	}
	/* Make sure blitter is done before we start processing any bitmaps */
	WaitBlit();

	FreeBobBitMaps(bob);

	/*  At this stage, the main bitmap should have the image in it, and the mask
	 * bitmap should have the mask in it (if any) */

	/*** Calculate the Image bounderies ***/

		/* Calc Left Pos */
	for(pos = 0;
		 (pos < pj->pj_Width) && (!TestColumnWord(pj->pj_MainBitMap,pos,0,pj->pj_Height));
			pos += 16) {}
	if(pos < pj->pj_Width)
	{
		pos = pos & 0xFFF0;
		for(finepos = pos;
				(finepos < pos+16) && (!TestColumn(pj->pj_MainBitMap,finepos,0,pj->pj_Height));
				finepos++) {}
		ileft = finepos;

		hasimage = TRUE;

			/* Calc Right Pos */
		for(pos = pj->pj_Width-1;
			 (pos >= ileft) && (!TestColumnWord(pj->pj_MainBitMap,pos,0,pj->pj_Height));
				pos -= 16) {}
		pos = pos & 0xFFF0;
		for(finepos = pos+15;
				(finepos >= pos) && (!TestColumn(pj->pj_MainBitMap,finepos,0,pj->pj_Height));
				finepos--) {}
		iright = finepos;
	
			/* Calc Top Pos */
		for(pos = 0;
			 (pos < pj->pj_Height) && (!TestRow(pj->pj_MainBitMap,ileft,pos,iright-ileft+1));
				pos++) {}
		itop = pos;
	
			/* Calc Bottom Pos */
		for(pos = pj->pj_Height-1;
			 (pos >= itop) && (!TestRow(pj->pj_MainBitMap,ileft,pos,iright-ileft+1));
				pos--) {}
		ibottom = pos;
	}

	/*** Calculate the Mask bounderies ***/

		/* Calc Left Pos */
	for(pos = 0;
		 (pos < pj->pj_Width) && (!TestColumnWord(pj->pj_MaskBitMap,pos,0,pj->pj_Height));
			pos += 16) {}
	if(pos < pj->pj_Width)
	{
		pos = pos & 0xFFF0;
		for(finepos = pos;
				(finepos < pos+16) && (!TestColumn(pj->pj_MaskBitMap,finepos,0,pj->pj_Height));
				finepos++) {}
		mleft = finepos;

		hasmask = TRUE;

			/* Calc Right Pos */
		for(pos = pj->pj_Width-1;
			 (pos >= mleft) && (!TestColumnWord(pj->pj_MaskBitMap,pos,0,pj->pj_Height));
				pos -= 16) {}
		pos = pos & 0xFFF0;
		for(finepos = pos+15;
				(finepos >= pos) && (!TestColumn(pj->pj_MaskBitMap,finepos,0,pj->pj_Height));
				finepos--) {}
		mright = finepos;
	
			/* Calc Top Pos */
		for(pos = 0;
			 (pos < pj->pj_Height) && (!TestRow(pj->pj_MaskBitMap,mleft,pos,mright-mleft+1));
				pos++) {}
		mtop = pos;
	
			/* Calc Bottom Pos */
		for(pos = pj->pj_Height-1;
			 (pos >= mtop) && (!TestRow(pj->pj_MaskBitMap,mleft,pos,mright-mleft+1));
				pos--) {}
		mbottom = pos;

		/* Make the image bouderies the minimax of the mask and image bitmaps */

		if(hasimage)
		{
			if(mtop < itop) itop = mtop;
			if(mbottom > ibottom) ibottom = mbottom;
			if(mleft < ileft) ileft = mleft;
			if(mright > iright) iright = mright;
		}
		else
		{
			itop = mtop;
			ibottom = mbottom;
			ileft = mleft;
			iright = mright;
			hasimage = TRUE;
		}
	}

	if(hasimage)
	{
		if( bob->bb_Image = CreateBitMap(iright-ileft+1,ibottom-itop+1,pj->pj_Depth,TRUE) )
		{
			bob->bb_Left = ileft;
			bob->bb_Top = itop;
			bob->bb_Width = iright-ileft+1;
			bob->bb_Height = ibottom-itop+1;
			bob->bb_Depth = pj->pj_Depth;
			BltBitMap(pj->pj_MainBitMap,ileft,itop,
								bob->bb_Image, 0, 0, iright-ileft+1,ibottom-itop+1,0xC0,0xFF,NULL);
		}
		else
		{
			FreeBobBitMaps(bob);
			GroovyReq("Gonk Error","Not Enough Memory\nTo Store Bob","Continue");
			return(FALSE);
		}
	}
	if(hasmask)
	{
		if( bob->bb_Mask = CreateBitMap(iright-ileft+1,ibottom-itop+1,1,TRUE) )
		{
			/*  If the bob has a mask, then it also has an image, so there's no need
			 * to store the width, height,... */

			BltBitMap(pj->pj_MaskBitMap,ileft,itop,
								bob->bb_Mask, 0, 0, iright-ileft+1,ibottom-itop+1,0xC0,0xFF,NULL);
		}
		else
		{
			FreeBobBitMaps(bob);
			GroovyReq("Gonk Error","Not Enough Memory\nTo Store Bob","Continue");
			return(FALSE);
		}
	}
	return(TRUE);
}

void TurnOnBobMaskEditing(struct Project *pj)
{
	struct ProjectWindow *pw;
	UBYTE olddepth;

	WaitForSubTaskDone(mainmsgport);

	if(!pj->pj_EditBobMask)
	{
		if( StoreBobImage(pj, pj->pj_CurrentBob) )
		{
			if(!pj->pj_BobHasMask)
			{
				/* Make up a default mask */
				NastyMakeColourMask(pj->pj_MainBitMap,pj->pj_MaskBitMap,pj->pj_Width,pj->pj_Height,0);
				pj->pj_BobHasMask = TRUE;
				pj->pj_CurrentBobModified = TRUE;
			}

			BltBitMap(pj->pj_MainBitMap,0,0,
								pj->pj_MainBitMap,0,0,pj->pj_Width,pj->pj_Height,0x00,0xFF,NULL);

			WaitBlit();
			olddepth = pj->pj_MainBitMap->Depth;
			pj->pj_MainBitMap->Depth = 1;
			BltBitMap(pj->pj_MaskBitMap,0,0,
								pj->pj_MainBitMap,0,0,pj->pj_Width,pj->pj_Height,0xC0,0xFF,NULL);
			WaitBlit();
			pj->pj_MainBitMap->Depth = olddepth;

			BltBitMap(pj->pj_MainBitMap,0,0,
								pj->pj_UndoBitMap,0,0,pj->pj_Width,pj->pj_Height,0xC0,0xFF,NULL);
	
			pj->pj_EditBobMask = TRUE;
			for(pw = pj->pj_ProjectWindows; pw; pw = pw->pw_NextProjWindow)
				RedisplayProjWindow(pw,TRUE);

			if(lastprojectwindow)
				DrawSelector(lastprojectwindow, lastprojectwindow->pw_SelX0, lastprojectwindow->pw_SelY0);

			RefreshProjectName(pj);
		}
	}
	
}

/********************************   TurnOnBobImageEditing()    **************************/
//
//  Be sure to call WaitForSubTaskDone(), before calling this if running from a non-subtask
// context.

void TurnOnBobImageEditing(struct Project *pj)
{
	struct ProjectWindow *pw;
	struct MyBob *bob;

	if(pj->pj_EditBobMask)
	{
		bob = pj->pj_CurrentBob;
		if( StoreBobMask(pj,bob) )
		{
			pj->pj_EditBobMask = FALSE;
	
			BltBitMap(pj->pj_MainBitMap,0,0,
								pj->pj_MainBitMap,0,0,pj->pj_Width,pj->pj_Height,0x00,0xFF,NULL);
	
			if(bob->bb_Image)
			{
				BltBitMap(bob->bb_Image,0,0,
									pj->pj_MainBitMap,bob->bb_Left,bob->bb_Top,
									bob->bb_Width,bob->bb_Height,0xC0,0xFF,NULL);
			}
			BltBitMap(pj->pj_MainBitMap,0,0,
								pj->pj_UndoBitMap,0,0,pj->pj_Width,pj->pj_Height,0xC0,0xFF,NULL);
	
			for(pw = pj->pj_ProjectWindows; pw; pw = pw->pw_NextProjWindow)
				RedisplayProjWindow(pw,TRUE);
			if(lastprojectwindow)
				DrawSelector(lastprojectwindow, lastprojectwindow->pw_SelX0, lastprojectwindow->pw_SelY0);
			RefreshProjectName(pj);
		}
	}
}

static BOOL WriteBobData(struct IFFHandle *handle, struct BobFileHeader *bh, struct MyBob *bob)
{
	UWORD line;
	struct BitMap *ibm,*mbm;
	UWORD *imageptr,*sourceimage;
	ULONG size,ibmwordsperrow;
	BOOL abort = FALSE;

	ibm = bob->bb_Image;
	mbm = bob->bb_Mask;

	if(ibm)
	{
		bh->bf_X = bob->bb_Left;
		bh->bf_Y = bob->bb_Top;
		bh->bf_Width = bob->bb_Width;
		bh->bf_Height = bob->bb_Height;
		bh->bf_Flags = mbm ? BFHF_HASMASK : 0;
		if(bob->bb_Flags & MBF_COLLISIONZONESET)
		{
			bh->bf_ColLeft = bob->bb_ColLeft - bob->bb_Left;
			bh->bf_ColTop = bob->bb_ColTop - bob->bb_Top;
			bh->bf_ColWidth = (bob->bb_ColRight - bob->bb_ColLeft) + 1;
			bh->bf_ColHeight = (bob->bb_ColBottom - bob->bb_ColTop) + 1;
			bh->bf_Flags |= BFHF_COLLISIONZONESET;
		}
		else
		{
			bh->bf_ColLeft = 0;
			bh->bf_ColTop = 0;
			bh->bf_ColWidth = bob->bb_Width;
			bh->bf_ColHeight = bob->bb_Height;
		}
		PackASCII(bob->bb_Node.ln_Name, bh->bf_Name, BOBNAMESIZE-1);
		bh->bf_Reserved[0] = 0;
	}
	else
	{
		bh->bf_X = 0;
		bh->bf_Y = 0;
		bh->bf_Width = 16;
		bh->bf_Height = 16;
		bh->bf_Flags = BFHF_NOREALIMAGE;
		bh->bf_ColLeft = 0;
		bh->bf_ColTop = 0;
		bh->bf_ColWidth = 16;
		bh->bf_ColHeight = 16;
		PackASCII(bob->bb_Node.ln_Name, bh->bf_Name, BOBNAMESIZE-1);
		bh->bf_Reserved[0] = 0;
		if( ibm = CreateBitMap(16,16,bob->bb_Depth,FALSE) )
		{
			bob->bb_Image = ibm;
			bob->bb_Width = 16;
			bob->bb_Height = 16;

			imageptr = (UWORD *)ibm->Planes[0];
			sourceimage = (UWORD *)&DefaultBobImage;
			ibmwordsperrow = ibm->BytesPerRow/2;

			for(line = 0; line<16; line++)
			{
				imageptr[0] = sourceimage[line];
				imageptr += ibmwordsperrow;
			}
		}
		else
			abort = TRUE;
	}

	size = sizeof(struct BobFileHeader);
	if( WriteChunkBytes( handle, bh, size ) != size )
		abort = TRUE;

	if(!abort)
	{
		if(glob.cg_SaveChunky)
			abort = !WriteChunkyBobData(bob,handle);
		else
			abort = !WritePlanerBobData(bob,handle);
	}

	if( (bh->bf_Flags & BFHF_NOREALIMAGE) && (ibm) )
	{
		DestroyBitMap(ibm,16,16,bob->bb_Depth);
		ibm = NULL;
		bob->bb_Image = NULL;
	}

	if(abort)
		return(FALSE);
	else
		return(TRUE);
}

/*******************   WriteChunkyBobData()   *******************/
//
// Returns: TRUE - All Saved Ok
//					FALSE - Failed

static BOOL WriteChunkyBobData(struct MyBob *bob, struct IFFHandle *handle)
{
	struct BitMap *ibm;
	UWORD x,y;
	ULONG buffersize;
	BOOL abort = FALSE;
	UBYTE	*buffermem = NULL, *destptr = NULL;

	ibm = bob->bb_Image;

	/* Ok, by here we should have an Image and a Mask bitmap, so all we have
		to do is buffer enough memory for the Image & Mask and Save it */

	buffersize = bob->bb_Width * bob->bb_Height;
	if( !(buffermem = AllocVec(buffersize, MEMF_ANY)) )
		abort = TRUE;
	destptr = buffermem;

	for(y = 0; (y < bob->bb_Height) && (!abort); y++)
	{
		for(x = 0; (x < bob->bb_Width) && (!abort); x++)
		{
			*destptr++ = GetPixelColour(ibm, x, y);
		}
	}
	if(!abort)
	{
		if( WriteChunkBytes( handle, buffermem, buffersize ) != buffersize )
			abort = TRUE;
	}

	if(buffermem)
		FreeVec(buffermem);
	
	return((BOOL)!abort);
}

/*******************   WritePlanerBobData()   *******************/
//
// Returns: TRUE - All Saved Ok
//					FALSE - Failed

static BOOL WritePlanerBobData(struct MyBob *bob, struct IFFHandle *handle)
{
	struct BitMap *ibm,*mbm;
	PLANEPTR ptr;
	UWORD plane,line;
	ULONG size, buffersize;
	BOOL abort = FALSE;
	UBYTE	*buffermem = NULL, *destptr = NULL;
	BOOL hasmask = TRUE;

	ibm = bob->bb_Image;
	mbm = bob->bb_Mask;

	/* Ok, by here we should have an Image and a Mask bitmap, so all we have
		to do is buffer enough memory for the Image & Mask and Save it */

	buffersize = (((bob->bb_Width+15)/8) & 0xFFFE) * bob->bb_Height * bob->bb_Depth * 2;
	if( !(buffermem = AllocVec(buffersize, MEMF_ANY)) )
		abort = TRUE;
	destptr = buffermem;

	size = (((bob->bb_Width+15)/8) & 0xFFFE);
	for(line = 0; (line<bob->bb_Height) && (!abort); line++)
	{
		for(plane = 0; (plane<bob->bb_Depth) && (!abort); plane++)
		{
			ptr = ibm->Planes[plane]+(ibm->BytesPerRow*line);
			memcpy(destptr,ptr,size);
			destptr += size;
		}
	}
	if(!abort)
	{
		if(!bob->bb_Mask)
		{
			hasmask = FALSE;
			if( mbm = CreateBitMap(bob->bb_Width,bob->bb_Height,1,FALSE) )
			{
				bob->bb_Mask = mbm;
				NastyMakeColourMask(ibm,mbm,bob->bb_Width,bob->bb_Height,0);
			}
			else
				abort = TRUE;
		}
		WaitBlit();
		size = (((bob->bb_Width+15)/8) & 0xFFFE);
		ptr = mbm->Planes[0];
		for(line = 0; (line<bob->bb_Height) && (!abort); line++)
		{
			for(plane = 0; (plane<bob->bb_Depth) && (!abort); plane++)
			{
				memcpy(destptr,ptr,size);
				destptr += size;
			}
			ptr += mbm->BytesPerRow;
		}
	}
	if(!abort)
	{
		if( WriteChunkBytes( handle, buffermem, buffersize ) != buffersize )
			abort = TRUE;
	}

	if(buffermem)
		FreeVec(buffermem);
	
	if( (!hasmask) && (mbm) )
	{
		DestroyBitMap(mbm,bob->bb_Width,bob->bb_Height,bob->bb_Depth);
		mbm = NULL;
		bob->bb_Mask = NULL;
	}
	return((BOOL)!abort);
}

static BOOL WriteBOBSChunk( struct IFFHandle *handle, struct Project *pj, char *name)
{
	ULONG size;
	BOOL success = FALSE,abort = FALSE;
	UWORD bobcount=0;
	struct MyBob *bob;
	struct BobBankFileHeader bbfh;
	struct BobFileHeader bh;

	for(bob = (struct MyBob *)pj->pj_BobList.lh_Head;
			bob->bb_Node.ln_Succ;
			bob = (struct MyBob *)bob->bb_Node.ln_Succ)
	{
		bobcount++;
	}
	Mystrncpy(bbfh.bf_BankName, pj->pj_BankName, 16);
	bbfh.bf_NumberOfBobs = bobcount-1;
	bbfh.bf_Depth = pj->pj_Depth;
	bbfh.bf_Pad = 0;
	bbfh.bf_Flags = glob.cg_SaveChunky ? BBFHF_CHUNKY : 0;
	bbfh.bf_Reserved[0] = 0;
	bbfh.bf_Reserved[1] = 0;
	bbfh.bf_Reserved[2] = 0;
	bbfh.bf_Reserved[3] = 0;

	if( !PushChunk( handle, ID_CONK, ID_BOBS, IFFSIZE_UNKNOWN ) )
	{
		size = sizeof(struct BobBankFileHeader);
		if( WriteChunkBytes( handle, &bbfh, size ) == size )
		{
			for(bob = (struct MyBob *)pj->pj_BobList.lh_Head;
				 (bob->bb_Node.ln_Succ) && (bob->bb_Node.ln_Succ->ln_Succ) && (!abort);
					bob = (struct MyBob *)bob->bb_Node.ln_Succ)
			{
				abort = !WriteBobData(handle,&bh,bob);
			}
		}
		else
			abort = TRUE;

		if(!abort)
		{
			if( PopChunk(handle) )
				 success = FALSE;
			else
				success = TRUE;
		}
		else
			success = FALSE;
	}
	return( success );
}

BOOL SaveBobs(char *name, struct Project *pj, struct IFFHandle *handle)
{
	BOOL success = TRUE;
	struct Sequence *sq;
	BOOL gotanim = FALSE;
	struct Anim *an;
	int palettetype;
	UWORD col;
	static struct HoopyObject hob[] =
	{
		{ HOTYPE_VGROUP, 100, HOFLG_FREEWIDTH|HOFLG_FREEHEIGHT,7,0,0,0, NULL, 0,0,0,0,0 },
			{ HOTYPE_TEXT, 100, HOFLG_NOBORDER, (ULONG)"Save Chunks?",0,0,0, NULL,0,0,0,0,0 },
			{ HOTYPE_CHECKBOX, 100, 0, 0,0,0,0, "Config",FALSE,0,PLACETEXT_RIGHT,0,0 },
			{ HOTYPE_CHECKBOX, 100, 0, 0,0,0,0, "Palette",TRUE,0,PLACETEXT_RIGHT,0,0 },
			{ HOTYPE_CHECKBOX, 100, 0, 0,0,0,0, "Bobs",TRUE,0,PLACETEXT_RIGHT,0,0 },
			{ HOTYPE_CHECKBOX, 100, 0, 0,0,0,0, "Anim",TRUE,0,PLACETEXT_RIGHT,0,0 },
			{ HOTYPE_SPACE, 10, HOFLG_NOBORDER|HOFLG_FREEWIDTH|HOFLG_FREEHEIGHT, 0,0,0,0, NULL,0,0,0,0,0 },
			{ HOTYPE_HGROUP, 100, HOFLG_NOBORDER|HOFLG_FREEWIDTH,3,0,0,0, NULL, 0,0,0,0,0 },
				{ HOTYPE_BUTTON, 100, 0, 0,0,0,0, "  OK  ",0,0x8000,0,0,0 },
				{ HOTYPE_SPACE, 100, HOFLG_NOBORDER|HOFLG_FREEWIDTH, 0,0,0,0, NULL,0,0,0,0,0 },
				{ HOTYPE_BUTTON, 100, 0, 0,0,0,0, "Cancel",0,0xFFFF,0,0,0 },
		{HOTYPE_END}
	};

	if(stricmp(pj->pj_BankName,"-<UNTITLED>-") == 0)
		SetChunkName(pj);

	if( IsListEmpty(&pj->pj_BobList) )
		return(FALSE);

	if(pj->pj_CurrentBobModified)
		StoreBob(pj,pj->pj_CurrentBob);

	if( sq = pj->pj_Sequence )
	{
		for(an = (struct Anim *)sq->sq_Anims.lh_Head;
				(an->an_Node.ln_Succ) && (!gotanim);
				an = (struct Anim *)an->an_Node.ln_Succ)
			gotanim = !IsListEmpty(&an->an_Frames);
	}
	else
		gotanim = FALSE;

	hob[2].ho_Value = 0;		/* Config */
	hob[3].ho_Value = 1;		/* Palette */
	hob[4].ho_Value = 1;		/* Bobs */
	if(gotanim)
	{
		hob[5].ho_Type = HOTYPE_CHECKBOX;
		hob[5].ho_Weight = 100;
		hob[5].ho_Value = 1;		/* Anim */
	}
	else
	{
		hob[5].ho_Type = HOTYPE_SPACE;
		hob[5].ho_Weight = 10;
		hob[5].ho_Value = 0;		/* Anim */
	}
	if(	HO_SimpleDoReq( hob, "Save Bobs", NULL) != 0xFFFF)
	{
		palettetype = CheckPalettes(pj);
		if(palettetype)
		{
			if(hob[2].ho_Value)										/* Config */
				success = WriteGCFGChunk( handle, &glob);
			if(success && hob[3].ho_Value)					/* Palette */
			{
				switch(palettetype)
				{
					case 1:
						success = WriteCMAPChunk( handle, &pj->pj_Palette);
						break;
					case 2:
						success = WriteCMAPChunk( handle, globpalette);
						break;
					case 3:
						success = WriteCMAPChunk( handle, globpalette);
						for(col = 0; col < 256; col++)
						{
							pj->pj_Palette.pl_Colours[col].r = globpalette->pl_Colours[col].r;
							pj->pj_Palette.pl_Colours[col].g = globpalette->pl_Colours[col].g;
							pj->pj_Palette.pl_Colours[col].b = globpalette->pl_Colours[col].b;
						}
						break;
				}
			}
			if(success && hob[4].ho_Value)
				success = WriteBOBSChunk(handle, pj, name);
			if(success && gotanim && hob[5].ho_Value)
				success = WriteANIMChunk(handle, sq, pj->pj_BankName);
		}
	}
	return( success );
}

BOOL LoadBobs(struct Project *pj, struct IFFHandle *handle, char *name)
{
	struct BobBankFileHeader bbfh;
	struct BobFileHeader bh;
	UWORD bobcount;
	BOOL abort = FALSE;
	struct MyBob *bob;

	if( ReadChunkBytes(handle,&bbfh,sizeof(struct BobBankFileHeader)) != sizeof(struct BobBankFileHeader))
		return(FALSE);

	MakePackable(bbfh.bf_BankName);
	Mystrncpy(pj->pj_BankName, bbfh.bf_BankName, 16);

	pj->pj_Palette.pl_Count = 1<<bbfh.bf_Depth;
	if( !NewEditType(pj, EDTY_BOBS, bbfh.bf_Depth) )
		return(FALSE);

	for(bobcount = 0;
		 ( (bobcount < bbfh.bf_NumberOfBobs) && (!abort) );
			bobcount++)
	{
		if(ReadChunkBytes(handle, &bh, sizeof(struct BobFileHeader)) == sizeof(struct BobFileHeader))
		{
			bob = (struct MyBob *)pj->pj_BobList.lh_TailPred;
			UnpackASCII(bh.bf_Name, bob->bb_Name, BOBNAMESIZE-1);
			bob->bb_Left = bh.bf_X;
			bob->bb_Top = bh.bf_Y;
			bob->bb_Width = bh.bf_Width;
			bob->bb_Height = bh.bf_Height;
			bob->bb_Depth = bbfh.bf_Depth;
			if(bh.bf_Flags & BFHF_COLLISIONZONESET)
			{
				bob->bb_ColLeft = bh.bf_ColLeft + bob->bb_Left;
				bob->bb_ColTop = bh.bf_ColTop + bob->bb_Top;
				bob->bb_ColRight = bob->bb_ColLeft + (bh.bf_ColWidth - 1);
				bob->bb_ColBottom = bob->bb_ColTop + (bh.bf_ColHeight - 1);
				bob->bb_Flags |= MBF_COLLISIONZONESET;
			}
			if( !(bh.bf_Flags & BFHF_NOREALIMAGE) )
			{
				if(bbfh.bf_Flags & BBFHF_CHUNKY)
					abort = !LoadInChunkyImage(bob,handle);
				else
					abort = !LoadInPlanerImage(bob,handle);
			}
			else
			{
				if(bbfh.bf_Flags & BBFHF_CHUNKY)
					abort = !ReadChunkyKludgeImage(bob,handle);
				else
					abort = !ReadPlanerKludgeImage(bob,handle);
			}
			if(!abort)
			{
				if(!(bbfh.bf_Flags & BBFHF_CHUNKY))
				{
					if(bh.bf_Flags & BFHF_HASMASK)
						abort = !LoadInMask(bob,handle);
					else
						abort = !ReadKludgeMask(bob,handle);
				}
			}
		}
		else
			abort = TRUE;

		if( !AllocNextBob(pj, &pj->pj_BobList) )
			abort = TRUE;
	}
	if( (!abort) && (!IsListEmpty(&pj->pj_BobList)) )
		EditBob(pj,(struct MyBob *)pj->pj_BobList.lh_Head);

	return((BOOL)!abort);
}

static BOOL LoadInPlanerImage(struct MyBob *bob, struct IFFHandle *handle)
{
	BOOL ret = FALSE;
	UWORD line,plane,bytewidth;
	struct BitMap *bm;
	UBYTE		*buffermem = NULL, *srcptr = NULL;
	ULONG size;

	size = (((bob->bb_Width+15)/8) & 0xFFFE) * bob->bb_Height * bob->bb_Depth;
	if(buffermem = AllocVec(size,MEMF_ANY))
	{
		srcptr = buffermem;
		if(ReadChunkBytes(handle,buffermem,size) == size)
		{
			if(bob->bb_Image = CreateBitMap(bob->bb_Width,bob->bb_Height,bob->bb_Depth,TRUE))
			{
				bytewidth = (((bob->bb_Width+15)/8) & 0xFFFE);
				bm = bob->bb_Image;

				for(line = 0; line < bob->bb_Height; line++)
				{
					for(plane = 0; plane < bob->bb_Depth; plane++)
					{
						memcpy(bm->Planes[plane]+(bm->BytesPerRow*line), srcptr, bytewidth );
						srcptr += bytewidth;
					}
				}
				ret = TRUE;
			}
		}
	}

	if( (!ret) && (bob->bb_Image) )
		DestroyBitMap(bob->bb_Image,bob->bb_Width,bob->bb_Height,bob->bb_Depth);

	if(buffermem)
		FreeVec(buffermem);

	return(ret);
}

static BOOL LoadInChunkyImage(struct MyBob *bob, struct IFFHandle *handle)
{
	BOOL ret = FALSE;
	UWORD x,y;
	struct BitMap *bm;
	UBYTE		*buffermem = NULL, *srcptr = NULL;
	ULONG size;

	size = bob->bb_Width * bob->bb_Height;
	if(buffermem = AllocVec(size, MEMF_ANY))
	{
		srcptr = buffermem;
		if(ReadChunkBytes(handle, buffermem, size) == size)
		{
			if(bob->bb_Image = CreateBitMap(bob->bb_Width, bob->bb_Height, bob->bb_Depth, TRUE))
			{
				bm = bob->bb_Image;

				for(y = 0; y < bob->bb_Height; y++)
				{
					for(x = 0; x < bob->bb_Width; x++)
					{
						PutPixelColour(bm, x, y, *srcptr++);
					}
				}
				ret = TRUE;
			}
		}
	}

	if( (!ret) && (bob->bb_Image) )
		DestroyBitMap(bob->bb_Image, bob->bb_Width, bob->bb_Height, bob->bb_Depth);

	if(buffermem)
		FreeVec(buffermem);

	return(ret);
}

static BOOL LoadInMask(struct MyBob *bob, struct IFFHandle *handle)
{
	BOOL ret = FALSE;
	UWORD line,bytewidth;
	struct BitMap *bm;
	UBYTE		*buffermem = NULL, *srcptr = NULL;
	ULONG size;

	size = (((bob->bb_Width+15)/8) & 0xFFFE) * bob->bb_Height * bob->bb_Depth;
	if(buffermem = AllocVec(size,MEMF_ANY))
	{
		srcptr = buffermem;
		if(ReadChunkBytes(handle,buffermem,size) == size)
		{
			if(bob->bb_Mask = CreateBitMap(bob->bb_Width,bob->bb_Height,1,TRUE))
			{
				bytewidth = (((bob->bb_Width+15)/8) & 0xFFFE);
				bm = bob->bb_Mask;

				for(line = 0;
					 (line < bob->bb_Height);
						line++)
				{
					memcpy(bm->Planes[0]+(bm->BytesPerRow*line), srcptr, bytewidth );
					srcptr += bytewidth * bob->bb_Depth;
				}
				ret = TRUE;
			}
		}
	}

	if( (!ret) && (bob->bb_Image) )
		DestroyBitMap(bob->bb_Image,bob->bb_Width,bob->bb_Height,bob->bb_Depth);

	if(buffermem)
		FreeVec(buffermem);

	return(ret);
}

static BOOL ReadPlanerKludgeImage(struct MyBob *bob, struct IFFHandle *handle)
{
	LONG size;
	APTR crapmem;
	BOOL ret = FALSE;

	size = (((bob->bb_Width+15)/8) & 0xFFFE) * bob->bb_Height * bob->bb_Depth;

	if(crapmem = AllocVec(size,MEMF_ANY))
	{
		if(ReadChunkBytes(handle,crapmem,size) == size)
			ret = TRUE;

		FreeVec(crapmem);
	}
	return(ret);
}

static BOOL ReadChunkyKludgeImage(struct MyBob *bob, struct IFFHandle *handle)
{
	LONG size;
	APTR crapmem;
	BOOL ret = FALSE;

	size = bob->bb_Width * bob->bb_Height;

	if(crapmem = AllocVec(size, MEMF_ANY))
	{
		if(ReadChunkBytes(handle, crapmem, size) == size)
			ret = TRUE;

		FreeVec(crapmem);
	}
	return(ret);
}

static BOOL ReadKludgeMask(struct MyBob *bob, struct IFFHandle *handle)
{
	LONG size;
	APTR crapmem;
	BOOL ret = FALSE;

	size = (((bob->bb_Width+15)/8) & 0xFFFE) * bob->bb_Height * bob->bb_Depth;

	if(crapmem = AllocVec(size,MEMF_ANY))
	{
		if(ReadChunkBytes(handle,crapmem,size) == size)
			ret = TRUE;

		FreeVec(crapmem);
	}
	return(ret);
}

void UpdateBobNameGadgets(struct Project *pj, struct MyBob *bob)
{
	struct ProjectWindow *pw;
	struct Gadget *bobnamegadget;

	for(pw = pj->pj_ProjectWindows; pw; pw = pw->pw_NextProjWindow)
	{
		if( (pw->pw_sw.sw_Window) && (bobnamegadget = pw->pw_GTGadgets[0]) )
		{
			GT_SetGadgetAttrs(bobnamegadget,pw->pw_sw.sw_Window,NULL,
				GTST_String,		bob->bb_Node.ln_Name,
				TAG_END);
		}
	}
}

struct MyBob *FindBobByNumber(struct List *boblist,UWORD number)
{
	struct MyBob *bob;

	if(IsListEmpty(boblist))
		return(NULL);

	for(bob = (struct MyBob *)boblist->lh_Head;
			bob->bb_Node.ln_Succ;
			bob = (struct MyBob *)bob->bb_Node.ln_Succ)
	{
		if(bob->bb_Number == number)
			return(bob);
	}
	return(NULL);
}

/******************   GUIDeleteBob   **********************************/

BOOL GUIDeleteBobs( struct Project *pj, struct Window *parentwin )
{
	WORD first, last;
	BOOL abort = FALSE;

	static struct HoopyObject gadlayout[] =
	{
		{ HOTYPE_VGROUP, 100, HOFLG_FREEWIDTH|HOFLG_FREEHEIGHT,4,0,0,0, NULL, 0,0,0,0,0 },
			{ HOTYPE_CYCLE, 100, 0, 0,0,0,0, NULL,0,0,0,0,0 },
			{ HOTYPE_HGROUP, 100, HOFLG_NOBORDER|HOFLG_FREEWIDTH,2,0,0,0, NULL, 0,0,0,0,0 },
				{ HOTYPE_INTEGER, 100, 0, 4,0,0,0, NULL,1,0,0,0,0 },
				{ HOTYPE_INTEGER, 100, 0, 4,0,0,0, "to",10,0,PLACETEXT_LEFT,0,0 },
			{ HOTYPE_SPACE, 10, HOFLG_NOBORDER|HOFLG_FREEWIDTH|HOFLG_FREEHEIGHT, 0,0,0,0, NULL,0,0,0,0,0 },
			{ HOTYPE_HGROUP, 100, HOFLG_NOBORDER|HOFLG_FREEWIDTH,3,0,0,0, NULL, 0,0,0,0,0 },
				{ HOTYPE_BUTTON, 100, 0, 0,0,0,0, "OK",0,0x8000,0,0,0 },
				{ HOTYPE_SPACE, 100, HOFLG_NOBORDER|HOFLG_FREEWIDTH, 0,0,0,0, NULL,0,0,0,0,0 },
				{ HOTYPE_BUTTON, 100, 0, 0,0,0,0, "Cancel",0,0xFFFF,0,0,0 },
		{HOTYPE_END}
	};
	char		*labels[] = {"Current Frame", "Range", "All Frames", NULL};

	gadlayout[1].ho_Attr0 = (ULONG)labels;
	gadlayout[3].ho_Value = ((struct MyBob *)pj->pj_BobList.lh_Head)->bb_Number;
	gadlayout[4].ho_Value = ((struct MyBob *)pj->pj_BobList.lh_TailPred)->bb_Number;

	if(HO_SimpleDoReq(gadlayout, "Delete Bob", parentwin) != 0xFFFF )
	{
		switch(gadlayout[1].ho_Value)
		{
			case 0:		/* Current Frame */
				if(pj->pj_CurrentBob)
				{
					first = pj->pj_CurrentBob->bb_Number;
					last = first;
				}
				else
					abort = TRUE;
				break;
			case 1:		/* Range */
				first = gadlayout[3].ho_Value;
				last = gadlayout[4].ho_Value;
				if(first < 1)
					first = 1;
				if(last > ((struct MyBob *)pj->pj_BobList.lh_TailPred)->bb_Number)
					last = ((struct MyBob *)pj->pj_BobList.lh_TailPred)->bb_Number;
				break;
			case 2:		/* All Frames */
				if(!IsListEmpty(&pj->pj_BobList))
				{
					first = 1;
					last = ((struct MyBob *)pj->pj_BobList.lh_TailPred)->bb_Number;
				}
				else
					abort = TRUE;
				break;
		}
	}
	if(!abort)
		abort = !DeleteBobs(pj, first, last);

	return( (BOOL)!abort );
}

BOOL DeleteBobs(struct Project *pj, UWORD first, UWORD last)
{
	UWORD count;
	struct MyBob *bob, *nextbob;

	WaitForSubTaskDone(mainmsgport);

	if( IsListEmpty(&pj->pj_BobList) )
		return(FALSE);

	bob = (struct MyBob *)pj->pj_BobList.lh_Head;
	for(count = 1; (count < first) && (bob->bb_Node.ln_Succ); count++)
		bob = (struct MyBob *)bob->bb_Node.ln_Succ;

	for(; (count <= last) && (bob->bb_Node.ln_Succ); count++)
	{
		nextbob = (struct MyBob *)bob->bb_Node.ln_Succ;
		/* Must always keep the last 'other' frame */
		if(nextbob->bb_Node.ln_Succ)
		{
			MakeSureSeqWindowsDontViewBob(pj->pj_Sequence, bob);
			DeleteBobFromAllAnims(pj->pj_Sequence, bob);
			Remove(&bob->bb_Node);
			if(pj->pj_CurrentBob == bob)
			{
				GotoNewBob(pj, nextbob);
				RefreshProjectName(pj);
			}
			FreeBobBitMaps(bob);
			FreeVec(bob);
		}
		bob = nextbob;
	}
	if( IsListEmpty(&pj->pj_BobList) )
	{
		AllocNextBob(pj, &pj->pj_BobList);
		GotoNewBob(pj, (struct MyBob *)pj->pj_BobList.lh_Head);
	}
	RenumberBobs(&pj->pj_BobList);
	GotoNewBob(pj, pj->pj_CurrentBob);
	RefreshProjectName(pj);
}

static void RenumberBobs(struct List *list)
{
	struct MyBob *bob;
	UWORD count = 1;

	if(IsListEmpty(list))
		return;

	for(bob = (struct MyBob *)list->lh_Head;
			bob->bb_Node.ln_Succ;
			bob = (struct MyBob *)bob->bb_Node.ln_Succ)
		bob->bb_Number = count++;
}

BOOL GUIInsertBobs(struct Project *pj, struct Window *parentwin)
{
	WORD numofframes,beforeframe;
	UWORD result;
	BOOL abort = FALSE;

	static struct HoopyObject gadlayout[] =
	{
		{ HOTYPE_VGROUP, 100, HOFLG_FREEWIDTH|HOFLG_FREEHEIGHT,4,0,0,0, NULL, 0,0,0,0,0 },
			{ HOTYPE_INTEGER, 100, 0, 4,0,0,0, "# of Frames",1,0,PLACETEXT_LEFT,0,0 },
			{ HOTYPE_INTEGER, 100, 0, 4,0,0,0, "Before Frame",1,0,PLACETEXT_LEFT,0,0 },
			{ HOTYPE_SPACE, 10, HOFLG_NOBORDER|HOFLG_FREEWIDTH|HOFLG_FREEHEIGHT, 0,0,0,0, NULL,0,0,0,0,0 },
			{ HOTYPE_HGROUP, 100, HOFLG_NOBORDER|HOFLG_FREEWIDTH,3,0,0,0, NULL, 0,0,0,0,0 },
				{ HOTYPE_BUTTON, 100, 0, 0,0,0,0, "OK",0,0x8000,0,0,0 },
				{ HOTYPE_SPACE, 100, HOFLG_NOBORDER|HOFLG_FREEWIDTH, 0,0,0,0, NULL,0,0,0,0,0 },
				{ HOTYPE_BUTTON, 100, 0, 0,0,0,0, "Cancel",0,0xFFFF,0,0,0 },
		{HOTYPE_END}
	};

	gadlayout[1].ho_Value = 1;
	gadlayout[2].ho_Value = pj->pj_CurrentBob->bb_Number;

	if(result = HO_SimpleDoReq( gadlayout, "Insert Frames", parentwin ) != 0xFFFF)
	{
		numofframes = gadlayout[1].ho_Value;
		beforeframe = gadlayout[2].ho_Value;
		if(beforeframe < 1) beforeframe = 1;
		if(beforeframe > ((struct MyBob *)pj->pj_BobList.lh_TailPred)->bb_Number)
			beforeframe = ((struct MyBob *)pj->pj_BobList.lh_TailPred)->bb_Number;
		if(numofframes > 0)
			abort = !InsertBobs(pj, numofframes, beforeframe);
	}
	return( (BOOL)!abort );
}

BOOL InsertBobs(struct Project *pj, UWORD numofframes, UWORD beforeframe)
{
	UWORD count;
	struct MyBob *bob, *newbob;

	WaitForSubTaskDone(mainmsgport);

	bob = (struct MyBob *)pj->pj_BobList.lh_Head;
	for(count = 1; (count < beforeframe) && (bob->bb_Node.ln_Succ); count++)
		bob = (struct MyBob *)bob->bb_Node.ln_Succ;

	if(!bob->bb_Node.ln_Succ)
		return(FALSE);

	for(count = 1; count <= numofframes; count++)
	{
		if(newbob = AllocBob(pj, &pj->pj_BobList))
		{
			if(IsListEmpty(&pj->pj_BobList))
				AddHead(&pj->pj_BobList, &newbob->bb_Node);
			else
				Insert(&pj->pj_BobList, &newbob->bb_Node, bob->bb_Node.ln_Pred);
		}
		else
		{
			DisplayBeep(mainscreen);
			return(FALSE);
		}
	}
	RenumberBobs(&pj->pj_BobList);
	GotoNewBob(pj, pj->pj_CurrentBob);
	RefreshProjectName(pj);
	return(TRUE);
}

BOOL GUICopyBobs(struct Project *pj, struct Window *parentwin)
{
	struct Window *win;
	UWORD width, height, result;
	WORD first, last, before;
	BOOL abort = FALSE, done = FALSE;
	char	*labels[] = {"Current Bob To Range","Current Bob To All Bobs","Range Before Bob",NULL};
	struct HoopyObject *currentho;

	static struct HoopyObject cbtrgadlayout[] =
	{
		{ HOTYPE_VGROUP, 100, HOFLG_FREEWIDTH|HOFLG_FREEHEIGHT,4,0,0,0, NULL, 0,0,0,0,0 },
			{ HOTYPE_CYCLE, 100, 0, 4,0,0,0, "Copy",0,0x8001,PLACETEXT_LEFT,0,0 },
			{ HOTYPE_HGROUP, 100, HOFLG_NOBORDER|HOFLG_FREEWIDTH,2,0,0,0, NULL, 0,0,0,0,0 },
				{ HOTYPE_INTEGER, 100, 0, 4,0,0,0, "Frames",1,0,PLACETEXT_LEFT,0,0 },
				{ HOTYPE_INTEGER, 100, 0, 4,0,0,0, "To",1,0,PLACETEXT_LEFT,0,0 },
			{ HOTYPE_SPACE, 10, HOFLG_NOBORDER|HOFLG_FREEWIDTH|HOFLG_FREEHEIGHT, 0,0,0,0, NULL,0,0,0,0,0 },
			{ HOTYPE_HGROUP, 100, HOFLG_NOBORDER|HOFLG_FREEWIDTH,3,0,0,0, NULL, 0,0,0,0,0 },
				{ HOTYPE_BUTTON, 100, HOFLG_FREEWIDTH, 0,0,0,0, "OK",0,0x8000,0,0,0 },
				{ HOTYPE_SPACE, 500, HOFLG_NOBORDER|HOFLG_FREEWIDTH, 0,0,0,0, NULL,0,0,0,0,0 },
				{ HOTYPE_BUTTON, 100, HOFLG_FREEWIDTH, 0,0,0,0, "Cancel",0,0xFFFF,0,0,0 },
		{HOTYPE_END}
	};

	static struct HoopyObject cbtabgadlayout[] =
	{
		{ HOTYPE_VGROUP, 100, HOFLG_FREEWIDTH|HOFLG_FREEHEIGHT,3,0,0,0, NULL, 0,0,0,0,0 },
			{ HOTYPE_CYCLE, 100, 0, 4,0,0,0, "Copy",0,0x8001,PLACETEXT_LEFT,0,0 },
			{ HOTYPE_SPACE, 10, HOFLG_NOBORDER|HOFLG_FREEWIDTH|HOFLG_FREEHEIGHT, 0,0,0,0, NULL,0,0,0,0,0 },
			{ HOTYPE_HGROUP, 100, HOFLG_NOBORDER|HOFLG_FREEWIDTH,3,0,0,0, NULL, 0,0,0,0,0 },
				{ HOTYPE_BUTTON, 100, HOFLG_FREEWIDTH, 0,0,0,0, "OK",0,0x8000,0,0,0 },
				{ HOTYPE_SPACE, 500, HOFLG_NOBORDER|HOFLG_FREEWIDTH, 0,0,0,0, NULL,0,0,0,0,0 },
				{ HOTYPE_BUTTON, 100, HOFLG_FREEWIDTH, 0,0,0,0, "Cancel",0,0xFFFF,0,0,0 },
		{HOTYPE_END}
	};

	static struct HoopyObject rbbgadlayout[] =
	{
		{ HOTYPE_VGROUP, 100, HOFLG_FREEWIDTH|HOFLG_FREEHEIGHT,5,0,0,0, NULL, 0,0,0,0,0 },
			{ HOTYPE_CYCLE, 100, 0, 4,0,0,0, "Copy",0,0x8001,PLACETEXT_LEFT,0,0 },
			{ HOTYPE_HGROUP, 100, HOFLG_NOBORDER|HOFLG_FREEWIDTH,2,0,0,0, NULL, 0,0,0,0,0 },
				{ HOTYPE_INTEGER, 100, 0, 4,0,0,0, "Frames",1,0,PLACETEXT_LEFT,0,0 },
				{ HOTYPE_INTEGER, 100, 0, 4,0,0,0, "To",1,0,PLACETEXT_LEFT,0,0 },
			{ HOTYPE_INTEGER, 100, 0, 4,0,0,0, "Insert Before Frame:",1,0,PLACETEXT_LEFT,0,0 },
			{ HOTYPE_SPACE, 10, HOFLG_NOBORDER|HOFLG_FREEWIDTH|HOFLG_FREEHEIGHT, 0,0,0,0, NULL,0,0,0,0,0 },
			{ HOTYPE_HGROUP, 100, HOFLG_NOBORDER|HOFLG_FREEWIDTH,3,0,0,0, NULL, 0,0,0,0,0 },
				{ HOTYPE_BUTTON, 100, HOFLG_FREEWIDTH, 0,0,0,0, "OK",0,0x8000,0,0,0 },
				{ HOTYPE_SPACE, 500, HOFLG_NOBORDER|HOFLG_FREEWIDTH, 0,0,0,0, NULL,0,0,0,0,0 },
				{ HOTYPE_BUTTON, 100, HOFLG_FREEWIDTH, 0,0,0,0, "Cancel",0,0xFFFF,0,0,0 },
		{HOTYPE_END}
	};

	if(!parentwin)
		return(0);

	currentho = cbtrgadlayout;

	currentho[1].ho_Attr0 = (ULONG)labels;
	currentho[1].ho_Value = 0;
	currentho[3].ho_Value = 1;
	currentho[4].ho_Value = ((struct MyBob *)pj->pj_BobList.lh_TailPred)->bb_Number;

	rbbgadlayout[1].ho_Attr0 = (ULONG)labels;
	width = HO_FindMinWidth(NULL, rbbgadlayout, windowtextfont);
	height = HO_FindMinHeight(NULL, rbbgadlayout, windowtextfont);

	if( win = OpenWindowTags( NULL,
		WA_Left, parentwin->LeftEdge + parentwin->BorderLeft,
		WA_Top, parentwin->TopEdge + parentwin->BorderTop,
		WA_InnerWidth, width,
		WA_InnerHeight, height,
		WA_MaxWidth, ~0,
		WA_MaxHeight, ~0,
		WA_PubScreen, parentwin->WScreen,
		WA_PubScreenFallBack, TRUE,
		WA_AutoAdjust,	TRUE,
		WA_Title,				"Copy Frames",
		WA_CloseGadget,	TRUE,
		WA_DragBar,			TRUE,
		WA_SizeGadget,	TRUE,
		WA_DepthGadget,	TRUE,
		WA_SizeBRight,	TRUE,
		WA_NewLookMenus,TRUE,
		WA_Activate,		TRUE,
		WA_IDCMP,				IDCMP_CLOSEWINDOW|IDCMP_NEWSIZE|IDCMP_REFRESHWINDOW|
			BUTTONIDCMP|INTEGERIDCMP|CYCLEIDCMP,
		TAG_DONE,				NULL) )
	{
		while( (!done) && (!abort) )
		{
			result = HO_DoReq( currentho, win );
			if(result == 0x8001)
			{
				if(currentho == cbtrgadlayout)
				{
					currentho = cbtabgadlayout;
					currentho[1].ho_Attr0 = (ULONG)labels;
					currentho[1].ho_Value = 1;
				}
				else
				{
					if(currentho == cbtabgadlayout)
					{
						currentho = rbbgadlayout;
						currentho[1].ho_Attr0 = (ULONG)labels;
						currentho[1].ho_Value = 2;
						currentho[3].ho_Value = 1;
						currentho[4].ho_Value = ((struct MyBob *)pj->pj_BobList.lh_TailPred)->bb_Number;
						currentho[5].ho_Value = pj->pj_CurrentBob->bb_Number;
					}
					else
					{
						if(currentho == rbbgadlayout)
						{
							currentho = cbtrgadlayout;
							currentho[1].ho_Attr0 = (ULONG)labels;
							currentho[1].ho_Value = 0;
							currentho[3].ho_Value = 1;
							currentho[4].ho_Value = ((struct MyBob *)pj->pj_BobList.lh_TailPred)->bb_Number;
						}
					}
				}
			}
			else
				done = TRUE;
		}
		CloseWindow( win );

		if(result == 0x8000)
		{
			switch(currentho[1].ho_Value)
			{
				case 0:				/* Current Bob To Range */
					first = currentho[3].ho_Value;
					last = currentho[4].ho_Value;
					if(first < last)
					{
						if(first < 1) first = 1;
						if(last > ((struct MyBob *)pj->pj_BobList.lh_TailPred)->bb_Number)
							last = ((struct MyBob *)pj->pj_BobList.lh_TailPred)->bb_Number;
						CopyCurrentBobToRange(pj, first, last);
					}
					else
						DisplayBeep(mainscreen);
					break;
				case 1:				/* Current Bob To All */
					first = 1;
					last = ((struct MyBob *)pj->pj_BobList.lh_TailPred)->bb_Number;
					CopyCurrentBobToRange(pj, first, last);
					break;
				case 2:				/* Range Before Bob */
					first = currentho[3].ho_Value;
					last = currentho[4].ho_Value;
					before = currentho[5].ho_Value;
					if(first < last)
					{
						if(first < 1) first = 1;
						if(last > ((struct MyBob *)pj->pj_BobList.lh_TailPred)->bb_Number)
							last = ((struct MyBob *)pj->pj_BobList.lh_TailPred)->bb_Number;
						if(before < 1) before = 1;
						if(before > ((struct MyBob *)pj->pj_BobList.lh_TailPred)->bb_Number)
							before = ((struct MyBob *)pj->pj_BobList.lh_TailPred)->bb_Number;
						CopyRangeBeforeBob(pj, first, last, before);
					}
					break;
			}
		}
	}
	return((BOOL)!abort);
}

BOOL CopyCurrentBobToRange(struct Project *pj, UWORD first, UWORD last)
{
	struct MyBob *bob,*srcbob;
	UWORD count;
	BOOL abort = FALSE;

	WaitForSubTaskDone(mainmsgport);

	if( !(srcbob = pj->pj_CurrentBob) )
		return(FALSE);

	bob = (struct MyBob *)pj->pj_BobList.lh_Head;
	for(count = 1; (count < first) && (bob->bb_Node.ln_Succ); count++)
		bob = (struct MyBob *)bob->bb_Node.ln_Succ;

	if(!bob->bb_Node.ln_Succ)
		return(FALSE);

	for(; (count <= last) && (!abort) && (bob->bb_Node.ln_Succ); count++)
	{
		if(bob != srcbob)
		{
			FreeBobBitMaps(bob);
			bob->bb_Top = srcbob->bb_Top;
			bob->bb_Left = srcbob->bb_Left;
			bob->bb_Width = srcbob->bb_Width;
			bob->bb_Height = srcbob->bb_Height;
			bob->bb_Depth = srcbob->bb_Depth;

			if(srcbob->bb_Image)
			{
				if( bob->bb_Image = CreateBitMap(bob->bb_Width, bob->bb_Height, bob->bb_Depth, TRUE) )
				{
					BltBitMap(srcbob->bb_Image, 0, 0,
										bob->bb_Image, 0, 0,
										bob->bb_Width, bob->bb_Height, 0xC0, 0xFF, NULL);
				}
				else
					abort = TRUE;
			}
			if(srcbob->bb_Mask)
			{
				if( bob->bb_Mask = CreateBitMap(bob->bb_Width, bob->bb_Height, 1, TRUE) )
				{
					BltBitMap(srcbob->bb_Mask, 0, 0,
										bob->bb_Mask, 0, 0,
										bob->bb_Width, bob->bb_Height, 0xC0, 0xFF, NULL);
				}
				else
					abort = TRUE;
			}
		}
		bob = (struct MyBob *)bob->bb_Node.ln_Succ;
	}
	if(!bob->bb_Node.ln_Succ)
		AllocNextBob(pj, &pj->pj_BobList);

	RenumberBobs(&pj->pj_BobList);
	GotoNewBob(pj, pj->pj_CurrentBob);
	RefreshProjectName(pj);
	return((BOOL)!abort);
}


/************************   Copy RangeBeforeBob()   ****************************/
//
//  Insert a range of bobs before another bob.
//  How it works: We copy the bobs into a tmplist, and then insert this tmplist
// into the proper range of bobs. We have to do the tmplist shit 'cos if we
// just inserted the bobs straight, then we would end up with huge problems if
// you inserted the range of bobs into itself.

BOOL CopyRangeBeforeBob(struct Project *pj, UWORD first, UWORD last, UWORD beforeframe)
{
	UWORD count;
	struct MyBob *beforebob, *copybob, *newbob;
	struct List tmplist;
	BOOL abort = FALSE;

	WaitForSubTaskDone(mainmsgport);

	NewList(&tmplist);

	beforebob = (struct MyBob *)pj->pj_BobList.lh_Head;
	for(count = 1; (count < beforeframe) && (beforebob->bb_Node.ln_Succ); count++)
		beforebob = (struct MyBob *)beforebob->bb_Node.ln_Succ;

	if(!beforebob->bb_Node.ln_Succ)
		return(FALSE);

	copybob = (struct MyBob *)pj->pj_BobList.lh_Head;
	for(count = 1; (count < first) && (copybob->bb_Node.ln_Succ); count++)
		copybob = (struct MyBob *)copybob->bb_Node.ln_Succ;

	if(!copybob->bb_Node.ln_Succ)
		return(FALSE);

	/* Copy the bobs into tmplist */
	for(; (count <= last) && (copybob->bb_Node.ln_Succ) && (!abort); count++)
	{
		if(newbob = AllocBob(pj, &tmplist))
		{
			AddTail(&tmplist, &newbob->bb_Node);
			newbob->bb_Top = copybob->bb_Top;
			newbob->bb_Left = copybob->bb_Left;
			newbob->bb_Width = copybob->bb_Width;
			newbob->bb_Height = copybob->bb_Height;
			newbob->bb_Depth = copybob->bb_Depth;

			if(copybob->bb_Image)
			{
				if( newbob->bb_Image = CreateBitMap(copybob->bb_Width, copybob->bb_Height, copybob->bb_Depth, TRUE) )
				{
					BltBitMap(copybob->bb_Image, 0, 0,
										newbob->bb_Image, 0, 0,
										copybob->bb_Width, copybob->bb_Height, 0xC0, 0xFF, NULL);
				}
				else
					abort = TRUE;
			}
			if(copybob->bb_Mask)
			{
				if( newbob->bb_Mask = CreateBitMap(copybob->bb_Width, copybob->bb_Height, 1, TRUE) )
				{
					BltBitMap(copybob->bb_Mask, 0, 0,
										newbob->bb_Mask, 0, 0,
										copybob->bb_Width, copybob->bb_Height, 0xC0, 0xFF, NULL);
				}
				else
					abort = TRUE;
			}
		}
		else
		{
			DisplayBeep(mainscreen);
			abort = TRUE;
		}
		copybob = (struct MyBob *)copybob->bb_Node.ln_Succ;
	}

	/* Copy the bobs back into the projects bob list */
	if(!abort)
	{
		while(!IsListEmpty(&tmplist))
		{
			newbob = (struct MyBob *)tmplist.lh_Head;
			Remove(&newbob->bb_Node);
			Insert(&pj->pj_BobList, &newbob->bb_Node, beforebob->bb_Node.ln_Pred);
		}
	}
	if(abort)
	{
		/* Free bobs associated with tmplist */
		while(!IsListEmpty(&tmplist))
		{
			newbob = (struct MyBob *)tmplist.lh_Head;
			Remove(&newbob->bb_Node);
			FreeBobBitMaps(newbob);
			FreeVec(newbob);
		}
	}
	RenumberBobs(&pj->pj_BobList);
	GotoNewBob(pj, pj->pj_CurrentBob);
	RefreshProjectName(pj);
	return(TRUE);
}

void DeleteMask(struct Project *pj, struct MyBob *bob)
{
	struct ProjectWindow *pw;

	if(bob->bb_Mask)
	{
		DestroyBitMap(bob->bb_Mask, bob->bb_Width, bob->bb_Height, 1);
		bob->bb_Mask = NULL;
	}

	if(bob == pj->pj_CurrentBob)
	{
		BltBitMap(pj->pj_MaskBitMap,0,0,
							pj->pj_MaskBitMap,0,0,pj->pj_Width,pj->pj_Height,0x00,0xFF,NULL);

		if(pj->pj_EditBobMask)
		{
			BltBitMap(pj->pj_MainBitMap,0,0,
								pj->pj_MainBitMap,0,0,pj->pj_Width,pj->pj_Height,0x00,0xFF,NULL);
			for(pw = pj->pj_ProjectWindows; pw; pw = pw->pw_NextProjWindow)
				RedisplayProjWindow(pw,FALSE);
		}
		RefreshProjectName(pj);
	}
}

void RemakeMask(struct Project *pj, struct MyBob *bob)
{
	if(bob == pj->pj_CurrentBob)
	{
		BltBitMap(pj->pj_MaskBitMap,0,0, pj->pj_MaskBitMap,0,0,pj->pj_Width,pj->pj_Height,0x00,0xFF,NULL);

		if(pj->pj_EditBobMask)
			BltBitMap(pj->pj_MainBitMap,0,0, pj->pj_MainBitMap,0,0,pj->pj_Width,pj->pj_Height,0x00,0xFF,NULL);
			
		WaitBlit();

		StoreBob(pj, bob);
	}
	if(bob->bb_Mask)
		DestroyBitMap(bob->bb_Mask, bob->bb_Width, bob->bb_Height, 1);

	if( bob->bb_Mask = CreateBitMap(bob->bb_Width, bob->bb_Height, 1, TRUE) )
	{
		NastyMakeColourMask(bob->bb_Image, bob->bb_Mask, bob->bb_Width, bob->bb_Height, 0);
	}
			
	if(bob == pj->pj_CurrentBob)
		EditBob(pj, bob);
	RefreshProjectName(pj);
}

BOOL BobCollisionZoneOK(struct Project *pj)
{
	BOOL	ret = FALSE;
	struct MyBob *bob;

	if( (pj->pj_EditType == EDTY_BOBS) || (pj->pj_EditType == EDTY_SPRITES) )
	{
		if(bob = pj->pj_CurrentBob)
		{
			ret = TRUE;
			if( !(bob->bb_Flags & MBF_COLLISIONZONESET) )
			{
				CreateDefaultHitZone(bob);
				RefreshProjectName(pj);
			}
		}
	}
	return(ret);
}

void CreateDefaultHitZone(struct MyBob *bob)
{
	if(bob)
	{
		bob->bb_ColLeft = bob->bb_Left;
		bob->bb_ColRight = bob->bb_Left + bob->bb_Width - 1;
		bob->bb_ColTop = bob->bb_Top;
		bob->bb_ColBottom = bob->bb_Top + bob->bb_Height - 1;
		bob->bb_Flags |= MBF_COLLISIONZONESET;
	}
}

/*********************   SetBobName()   **************************/
//
//  Copies the name to the bob, so the name can be freed after calling
// this routine.
// NOTE: Should call UpdateBobNameGadgets() after this calling
//       this routine.

void SetBobName(struct Project *pj, struct MyBob *bob, char *name)
{
	strncpy(bob->bb_Name,name,BOBNAMESIZE-1);
	bob->bb_Name[BOBNAMESIZE-1] = 0;
	if(MakePackable(bob->bb_Name))
		DisplayBeep(mainscreen);
	SussOutModificationStuff(pj);
}
