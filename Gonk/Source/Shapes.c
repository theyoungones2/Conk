/*****************************************************************/
// Various routines to handle IFF Stuff.
//

#include <stdio.h>
//#include <stdarg.h>
//#include <string.h>
//#include <dos/dos.h>
//#include <dos/dostags.h>
//#include <exec/exec.h>
//#include <intuition/intuition.h>
//#include <graphics/gfxbase.h>
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
//#include <proto/gadtools.h>
#include <proto/iffparse.h>

#include <global.h>
#include <ExternVars.h>
#include <ilbm.h>
#include <packer.h>
#include <HoopyVision.h>

struct Shape
{
	UWORD		sh_PixWidth;		/* 00 Width in pixels */
	UWORD		sh_PixHeight;		/* 02 Height in pixels */
	UWORD		sh_Depth;				/* 04 number of bitplanes */
	UWORD		sh_EBWidth;			/* 06 even byte width (only 1bpl I think) */
	UWORD		sh_BltSize;			/* 08 BLTSIZE */
	UWORD		sh_XHandle;			/* 10 position of handle (0,0 by default) */
	UWORD		sh_YHandle;			/* 12 */
	ULONG		sh_Data;				/* 14 ptr to image data (meaningless on disk) */
	ULONG		sh_Cookie;			/* 18 ptr to mask data (meaningless on disk) */
	UWORD		sh_OneBPMem;		/* 22 mem needed for one bpl */
	UWORD		sh_OneBPMemX;		/* 24 mem needed for 1 bpl + extra word per plane per line */
	UWORD		sh_AllBPMem;		/* 26 mem needed for entire image (excludes this header) */
	UWORD		sh_AllBPMemX;		/* 28 mem needed for entire image + extra column of words */
	UBYTE		sh_Pad[2];			/* 30 think this is now used for some cookie flags, */
													/*    but not sure. Shouldn't matter. */
};

static BOOL LoadInBlitzImage(struct MyBob *bob, struct Shape *sh, BPTR doshandle);

/***********************   LoadShapesFile()   ****************************/

BOOL LoadShapesFile(char *drawer, char *file)
{
	char *name = NULL;
	struct Project *pj = NULL;
	struct File *fl = NULL;
	struct Shape sh;
	BPTR doshandle = NULL;
	struct MyBob *bob;
	BOOL success = FALSE,done = FALSE;

	if( name = MakeFullPathName(drawer, file))
	{
		if(doshandle = Open(name, MODE_OLDFILE))
		{
			if( fl = AllocNewFile())
			{
				SetFileName(fl, file, drawer);
		
				if( (pj = GetEmptyProject(fl)) && (BlockProjectWindows(pj)) )
				{
					memcpy( &pj->pj_Palette,globpalette,sizeof(struct Palette) );

					Mystrncpy(pj->pj_BankName, "-<UNTITLED>-", 16);

					if( Read(doshandle, &sh, sizeof(struct Shape)) == sizeof(struct Shape))
					{
						pj->pj_Palette.pl_Count = 1<<sh.sh_Depth;
						if( NewEditType(pj, EDTY_BOBS, sh.sh_Depth) )
						{
							do
							{
								if( (sh.sh_EBWidth * sh.sh_PixHeight == sh.sh_OneBPMem) &&
										(sh.sh_EBWidth * sh.sh_PixHeight * sh.sh_Depth == sh.sh_AllBPMem) )
								{
									bob = (struct MyBob *)pj->pj_BobList.lh_TailPred;
									bob->bb_Left = 16;
									bob->bb_Top = 16;
									bob->bb_Width = sh.sh_PixWidth;
									bob->bb_Height = sh.sh_PixHeight;
									bob->bb_Depth = sh.sh_Depth;
									if( !(done = !LoadInBlitzImage(bob, &sh, doshandle)) )
									{
										if( AllocNextBob(pj, &pj->pj_BobList))
										{
											if(Read(doshandle, &sh, sizeof(struct Shape)) != sizeof(struct Shape))
											{
												done = TRUE;
												success = TRUE;
											}
										}
										else
											done = TRUE;
									}
								}
								else
								{
									GroovyReq("Open File","Shape File Corrupt","Cancel");
									done = TRUE;
								}
							}	while(!done);
							if(success)
							{
								pj->pj_IsFile = TRUE;
								RefreshProjectName(pj);
								if( !IsListEmpty(&pj->pj_BobList))
									EditBob(pj, (struct MyBob *)pj->pj_BobList.lh_Head);
							}
						}
					}
				}
			}
			Close(doshandle);
			doshandle = NULL;
		}
		FreeVec(name);
	}
	if(success && pj)
		UseProjSettings(pj);

	return(success);
}

/***********************   LoadInBlitzImage()   ****************************/

static BOOL LoadInBlitzImage(struct MyBob *bob, struct Shape *sh, BPTR doshandle)
{
	BOOL ret = FALSE;
	UWORD line,plane;
	struct BitMap *bm;
	UBYTE		*buffermem = NULL, *srcptr = NULL;

	if(sh->sh_AllBPMem > 0)
	{
		if(buffermem = AllocVec(sh->sh_AllBPMem, MEMF_ANY))
		{
			srcptr = buffermem;
			if(Read(doshandle, buffermem, sh->sh_AllBPMem) == sh->sh_AllBPMem)
			{
				if(bob->bb_Image = CreateBitMap(bob->bb_Width, bob->bb_Height, bob->bb_Depth, TRUE))
				{
					bm = bob->bb_Image;
	
					for(plane = 0; plane < bob->bb_Depth; plane++)
					{
						for(line = 0; line < bob->bb_Height; line++)
						{
							memcpy(bm->Planes[plane]+(bm->BytesPerRow*line), srcptr, sh->sh_EBWidth );
							srcptr += sh->sh_EBWidth;
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
	}
	else
		ret = TRUE;

	return(ret);
}
