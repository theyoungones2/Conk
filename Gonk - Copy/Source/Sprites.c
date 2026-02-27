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

#define	ID_SPRT		MAKE_ID('S','P','R','T')
#define	ID_CONK		MAKE_ID('C','O','N','K')

extern long __far DefaultBobImage;

static BOOL WriteSpriteData(struct IFFHandle *handle, struct SpriteFileHeader *sfh, struct MyBob *bob);
static BOOL WriteSPRTChunk( struct IFFHandle *handle, struct Project *pj, char *name);
static BOOL LoadInSpriteImage(struct MyBob *bob, struct IFFHandle *handle);
static BOOL ReadKludgeSpriteImage(struct MyBob *bob, struct IFFHandle *handle);

/**************************   InitSpriteMode()   ***********************/
//
//  Put the given project into Sprite edit mode

BOOL InitSpriteMode(struct Project *pj, UWORD newdepth)
{
	struct Palette *pal;
	struct ProjectWindow *pw;

	if(pj->pj_EditType != EDTY_SPRITES)
	{
		WaitForSubTaskDone(mainmsgport);
		if( (pj->pj_EditType != EDTY_BOBS) && (pj->pj_ModifiedFlag) )
		{
			if( !GroovyReq("Gonk","Warning: This Will Erase\nThe Current Project","Kill Project|Cancel") )
			{
				RefreshAllMenus();
				return(FALSE);
			}
		}
		DeallocProjBitMaps(pj);

		pj->pj_Depth = newdepth;

		if( (pj->pj_MainRPort = AllocBitMapRPort(workbmwidth,workbmheight,pj->pj_Depth)) &&
				(pj->pj_UndoBitMap = CreateBitMap(workbmwidth,workbmheight,pj->pj_Depth,TRUE)) )
		{
			pj->pj_MainBitMap = pj->pj_MainRPort->BitMap;
			pj->pj_Width = workbmwidth;
			pj->pj_Height = workbmheight;
			pj->pj_EditType = EDTY_SPRITES;

			if( IsListEmpty(&pj->pj_BobList) )
				AllocNextBob(pj, &pj->pj_BobList);
			EditBob(pj, (struct MyBob *)pj->pj_BobList.lh_Head);
			/* Bring up the Bob Name Gadget (EditBob doesn't do this (for speed )) */
			for(pw = pj->pj_ProjectWindows; pw; pw = pw->pw_NextProjWindow)
				RedisplayProjWindow(pw,TRUE);
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


void GotoPreviousSprite(struct Project *pj)
{
	if(pj->pj_EditType == EDTY_SPRITES)
	{
		if( (pj->pj_CurrentBob->bb_Node.ln_Pred) &&
				(pj->pj_CurrentBob->bb_Node.ln_Pred->ln_Pred) )
		GotoNewSprite(pj, (struct MyBob *)pj->pj_CurrentBob->bb_Node.ln_Pred);
		RefreshProjectName(pj);
	}
}

void GotoNextSprite(struct Project *pj)
{
	if(pj->pj_EditType == EDTY_SPRITES)
	{
		if( (pj->pj_CurrentBob->bb_Node.ln_Succ) &&
				(pj->pj_CurrentBob->bb_Node.ln_Succ->ln_Succ) )
		GotoNewSprite(pj, (struct MyBob *)pj->pj_CurrentBob->bb_Node.ln_Succ);
		RefreshProjectName(pj);
	}
}

void GotoNewSprite(struct Project *pj, struct MyBob *bob)
{
	//WaitForSubTaskDone(mainmsgport);

	if(pj->pj_CurrentBob)
	{
		if(pj->pj_CurrentBobModified)
		{
			if(StoreSprite(pj,pj->pj_CurrentBob))
				EditBob(pj,bob);
		}
		else
			EditBob(pj,bob);
	}
}

BOOL StoreSprite(struct Project *pj, struct MyBob *bob)
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
		 (pos < pj->pj_Width) && (!TestColumnWord(pj->pj_MainBitMap,pos,0,pj->pj_Height-1));
			pos += 16) {}
	if(pos < pj->pj_Width)
	{
		pos = pos & 0xFFF0;
		for(finepos = pos;
				(finepos < pos+16) && (!TestColumn(pj->pj_MainBitMap,finepos,0,pj->pj_Height-1));
				finepos++) {}
		left = finepos;
	}
	else
		return(TRUE);

		/* Calc Right Pos */
	for(pos = pj->pj_Width-1;
		 (pos >= left) && (!TestColumnWord(pj->pj_MainBitMap,pos,0,pj->pj_Height-1));
			pos -= 16) {}
	pos = pos & 0xFFF0;
	for(finepos = pos+15;
			(finepos >= pos) && (!TestColumn(pj->pj_MainBitMap,finepos,0,pj->pj_Height-1));
			finepos--) {}
	right = finepos;

		/* Calc Top Pos */
	for(pos = 0;
		 (pos < pj->pj_Height-1) && (!TestRow(pj->pj_MainBitMap,left,pos,right-left+1));
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

static BOOL WriteSpriteData(struct IFFHandle *handle, struct SpriteFileHeader *sfh, struct MyBob *bob)
{
	struct BitMap *ibm;
	PLANEPTR ptr;
	BOOL abort = FALSE;
	UWORD plane,line;
	UWORD *imageptr,*sourceimage;
	ULONG size,ibmwordsperrow, buffersize;
	UBYTE	*buffermem = NULL, *destptr = NULL;

	ibm = bob->bb_Image;

	if(ibm)
	{
		sfh->sfh_X = bob->bb_Left;
		sfh->sfh_Y = bob->bb_Top;
		sfh->sfh_Width = bob->bb_Width;
		sfh->sfh_Height = bob->bb_Height;
		sfh->sfh_Flags = 0;
		sfh->sfh_SpriteNumber = 0;
		if(bob->bb_Flags & MBF_COLLISIONZONESET)
		{
			sfh->sfh_ColLeft = bob->bb_ColLeft - bob->bb_Left;
			sfh->sfh_ColTop = bob->bb_ColTop - bob->bb_Top;
			sfh->sfh_ColWidth = (bob->bb_ColRight - bob->bb_ColLeft) + 1;
			sfh->sfh_ColHeight = (bob->bb_ColBottom - bob->bb_ColTop) + 1;
			sfh->sfh_Flags |= BFHF_COLLISIONZONESET;
		}
		else
		{
			sfh->sfh_ColLeft = 0;
			sfh->sfh_ColTop = 0;
			sfh->sfh_ColWidth = bob->bb_Width;
			sfh->sfh_ColHeight = bob->bb_Height;
		}
		PackASCII(bob->bb_Node.ln_Name, sfh->sfh_Name, BOBNAMESIZE-1);
		sfh->sfh_Reserved[0] = 0;
	}
	else
	{
		sfh->sfh_X = 0;
		sfh->sfh_Y = 0;
		sfh->sfh_Width = 16;
		sfh->sfh_Height = 16;
		sfh->sfh_Flags = BFHF_NOREALIMAGE;
		sfh->sfh_SpriteNumber = 0;
		sfh->sfh_ColLeft = 0;
		sfh->sfh_ColTop = 0;
		sfh->sfh_ColWidth = 16;
		sfh->sfh_ColHeight = 16;
		PackASCII(bob->bb_Node.ln_Name, sfh->sfh_Name, BOBNAMESIZE-1);
		sfh->sfh_Reserved[0] = 0;
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
	if(!abort)
	{
		/* Ok, by here we should have an Image and a Mask bitmap, so all we have
			to do is buffer enough memory for the Image & Mask and  Save it */

		size = sizeof(struct SpriteFileHeader);
		if( WriteChunkBytes( handle, sfh, size ) != size )
			abort = TRUE;

		buffersize = (((bob->bb_Width+15)/8) & 0xFFFE) * bob->bb_Height * bob->bb_Depth;
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
			if( WriteChunkBytes( handle, buffermem, buffersize ) != buffersize )
				abort = TRUE;
		}

		if(buffermem)
			FreeVec(buffermem);
	
		if( (sfh->sfh_Flags & BFHF_NOREALIMAGE) && (ibm) )
		{
			DestroyBitMap(ibm,16,16,bob->bb_Depth);
			ibm = NULL;
			bob->bb_Image = NULL;
		}
	}
		return((BOOL)!abort);
}

static BOOL WriteSPRTChunk( struct IFFHandle *handle, struct Project *pj, char *name)
{
	ULONG size;
	BOOL success = FALSE,abort = FALSE;
	UWORD bobcount = 0;
	struct MyBob *bob;
	struct SpriteBankFileHeader sbfh;
	struct SpriteFileHeader sfh;

	for(bob = (struct MyBob *)pj->pj_BobList.lh_Head;
			bob->bb_Node.ln_Succ;
			bob = (struct MyBob *)bob->bb_Node.ln_Succ)
	{
		bobcount++;
	}
	Mystrncpy(sbfh.sbfh_BankName, pj->pj_BankName, 16);
	sbfh.sbfh_NumberOfSprites = bobcount-1;
	sbfh.sbfh_Depth = pj->pj_Depth;
	sbfh.sbfh_Pad = 0;
	sbfh.sbfh_Flags = 0;
	sbfh.sbfh_Reserved[0] = 0;
	sbfh.sbfh_Reserved[1] = 0;
	sbfh.sbfh_Reserved[2] = 0;
	sbfh.sbfh_Reserved[3] = 0;

	if( !PushChunk( handle, 0, ID_SPRT, IFFSIZE_UNKNOWN ) )
	{
		size = sizeof(sbfh);
		if( WriteChunkBytes( handle, &sbfh, size ) == size )
		{
			for(bob = (struct MyBob *)pj->pj_BobList.lh_Head;
				 (bob->bb_Node.ln_Succ) && (bob->bb_Node.ln_Succ->ln_Succ) && (!abort);
					bob = (struct MyBob *)bob->bb_Node.ln_Succ)
			{
				abort = !WriteSpriteData(handle,&sfh,bob);
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

BOOL SaveSprites(char *name, struct Project *pj, struct IFFHandle *handle)
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
			{ HOTYPE_CHECKBOX, 100, 0, 0,0,0,0, "Sprites",TRUE,0,PLACETEXT_RIGHT,0,0 },
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

	if(pj->pj_CurrentBobModified)
	{
		StoreSprite(pj,pj->pj_CurrentBob);
	}
	if( IsListEmpty(&pj->pj_BobList) )
		return(FALSE);

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
	hob[4].ho_Value = 1;		/* Sprites */
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

	if(	HO_SimpleDoReq( hob, "Save Sprites", NULL) != 0xFFFF)
	{
		palettetype = CheckPalettes(pj);
		if(palettetype)
		{
			if(success && hob[2].ho_Value)								/* Config */
				success = WriteGCFGChunk( handle, &glob);
			if(success && hob[3].ho_Value)								/* Palette */
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
				success = WriteSPRTChunk(handle, pj, name);
			if(success && gotanim && hob[5].ho_Value)
				success = WriteANIMChunk(handle, sq, pj->pj_BankName);
		}
	}
	return( success );
}

BOOL LoadSprites(struct Project *pj, struct IFFHandle *handle, char *name)
{
	struct SpriteBankFileHeader sbfh;
	struct SpriteFileHeader sfh;
	UWORD bobcount;
	BOOL abort = FALSE;
	struct MyBob *bob;

	if( ReadChunkBytes(handle,&sbfh,sizeof(sbfh)) != sizeof(sbfh))
		return(FALSE);

	MakePackable(sbfh.sbfh_BankName);
	Mystrncpy(pj->pj_BankName, sbfh.sbfh_BankName, 16);

	pj->pj_Palette.pl_Count = 1<<sbfh.sbfh_Depth;
	if( !NewEditType(pj, EDTY_SPRITES, sbfh.sbfh_Depth) )
		return(FALSE);

	for(bobcount = 0;
		 ( (bobcount < sbfh.sbfh_NumberOfSprites) && (!abort) );
			bobcount++)
	{
		if(ReadChunkBytes(handle,&sfh,sizeof(sfh)) == sizeof(sfh))
		{
			bob = (struct MyBob *)pj->pj_BobList.lh_TailPred;
			UnpackASCII(sfh.sfh_Name, bob->bb_Name, BOBNAMESIZE-1);
			bob->bb_Left = sfh.sfh_X;
			bob->bb_Top = sfh.sfh_Y;
			bob->bb_Width = sfh.sfh_Width;
			bob->bb_Height = sfh.sfh_Height;
			bob->bb_Depth = sbfh.sbfh_Depth;
			if(sfh.sfh_Flags & BFHF_COLLISIONZONESET)
			{
				bob->bb_ColLeft = sfh.sfh_ColLeft + bob->bb_Left;
				bob->bb_ColTop = sfh.sfh_ColTop + bob->bb_Top;
				bob->bb_ColRight = bob->bb_ColLeft + (sfh.sfh_ColWidth - 1);
				bob->bb_ColBottom = bob->bb_ColTop + (sfh.sfh_ColHeight - 1);
				bob->bb_Flags |= MBF_COLLISIONZONESET;
			}
			if( !(sfh.sfh_Flags & BFHF_NOREALIMAGE) )
				abort = !LoadInSpriteImage(bob,handle);
			else
				abort = !ReadKludgeSpriteImage(bob,handle);
		}
		else
			abort = TRUE;

		if( !AllocNextBob(pj, &pj->pj_BobList) )
			abort = TRUE;
	}
	if( (!abort) && (!IsListEmpty(&pj->pj_BobList)) )
		EditBob(pj, (struct MyBob *)pj->pj_BobList.lh_Head);

	return((BOOL)!abort);
}

static BOOL LoadInSpriteImage(struct MyBob *bob, struct IFFHandle *handle)
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

				for(line = 0;
						(line < bob->bb_Height);
						line++)
				{
					for(plane = 0;
						 (plane < bob->bb_Depth);
							plane++)
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

static BOOL ReadKludgeSpriteImage(struct MyBob *bob, struct IFFHandle *handle)
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
