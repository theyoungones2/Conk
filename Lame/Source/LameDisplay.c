#include <stdarg.h>
#include <clib/alib_stdio_protos.h>
#include <exec/exec.h>
#include <dos/dos.h>
#include <graphics/modeid.h>
#include <graphics/display.h>
#include <graphics/gfxbase.h>
#include <hardware/intbits.h>
#include <hardware/dmabits.h>
#include <proto/exec.h>
#include <proto/dos.h>
#include <lame.h>
#include <mycustom.h>
#include <debug.h>
#include <protos.h>

void __asm GC_InitGroovyConStruct(register __a0 struct GroovyCon *gc);
static void InsertCopperWait(struct MinList *list, struct CopperWaitNode *cwn);
static UWORD ChopUpBarImages(struct LameReq *lr, struct PaulsPictureImage *ppi);

extern UWORD *CopperIntPtr;
extern struct GroovyCon StatBoxGroovyCon;
extern struct TextFont *TopazTextFont;

/* Display Data */
/* gametop & stattop start at 'displaytop'==0x2C */
static WORD gametop,gameheight;
static WORD stattop,statheight;

/*************************   AllocNewSliceNode()   *****************************/

UWORD AllocNewSliceNode(	UWORD bitmapwidth, UWORD bitmapheight, struct Display *disp,
													struct PaulsPictureImage *ppi)
{
	struct SliceNode						*sn								= NULL;
	struct GameSlice						*slice						= NULL;
	struct SliceHeader					*sh								= NULL;
	struct CopperWaitNode				*cwn							= NULL;
	UWORD												 bufferwidth			= 0;
	ULONG												 bitmapsize				= 0;
	struct ProgNode							*prognode					= NULL;
	UWORD												 err							= LAMERR_ALLOK;
	char												 namebuffer[PROGFULLNAMESIZE];
	WORD												 gamebottom;

	if( sn = MYALLOCVEC( sizeof(struct SliceNode), MEMF_ANY|MEMF_CLEAR,"SliceNode") )
	{
		if( slice = MYALLOCVEC( sizeof(struct GameSlice), MEMF_ANY|MEMF_CLEAR,"Slice") )
		{
			sn->sn_SliceData = slice;
			sn->sn_Type = SNTYPE_GAMESLICE;
			AddTail(&slicelist, &sn->sn_Node);
	
			sh = &slice->ss_SliceHeader;
			sh->sh_Next = NULL;
			sh->sh_DisplayID = disp->dp_DisplayID;
	
			bufferwidth = (disp->dp_DisplayID & HIRES_KEY) ? 88 : 48;
			bitmapsize = bufferwidth * bitmapheight;
			bitmapsize += ((bitmapwidth-(bufferwidth*8))+7)/8;
			bitmapsize *= disp->dp_LevelDepth;
			bitmapsize += 8+8;		/* 8 bytes for allignment & 8 for the extra stuff off the right of the */
														/* map that you'll never see */
			/* XYZZY - might need a few more bytes for scrolling right to edge - Ben. */

			if( (bitmapsize == 0) ||
				( (bitmapsize != 0) && (err = AllocBitMaps(sn, bitmapsize, ppi)) < LAME_FAILAT) )
			{
				sh->sh_Planes = disp->dp_LevelDepth;
				if((sh->sh_Colours = disp->dp_NumOfColours) >= 2)
				{
					sh->sh_ColourPointers = disp->dp_Palette;
	
					bufferwidth = (disp->dp_DisplayID & HIRES_KEY) ? 88 : 48;
					sh->sh_ColourOffset = 0;
					sh->sh_Flags = 0;
					sh->sh_ScreenWidth = (disp->dp_DisplayID & HIRES_KEY) ? 640<<5 : 320<<5;
					sh->sh_BufferWidth = bufferwidth;
					sh->sh_BufferWidthx16xPlanes = bufferwidth * 16 * disp->dp_LevelDepth;
					sh->sh_BufferWidthxPlanes = bufferwidth * disp->dp_LevelDepth;
					if(disp->dp_DisplayID & LACE_KEY)
						sh->sh_BufferWidthxIPlanes = bufferwidth * disp->dp_LevelDepth * 2;
					else
						sh->sh_BufferWidthxIPlanes = bufferwidth * disp->dp_LevelDepth;
					sh->sh_BufferHeight = bitmapheight;
	
					if( (err = AllocCopperHeader(sh, disp->dp_LevelDepth, disp->dp_DisplayID)) < LAME_FAILAT)
					{
						gametop = displaytop;
						gamebottom = bitmapheight;
						if(gamebottom > displayheight) gamebottom = displayheight;
						if(!NullName(disp->dp_StatBackDropPicture))
						{
							if(disp->dp_Flags & DPF_STATBOXATTOP)
							{
								gametop += statheight+disp->dp_GameHeaderHeight;
							}
							else
							{
								if((gamebottom+disp->dp_StatHeaderHeight+statheight) > displayheight)
									gamebottom = displayheight-(disp->dp_StatHeaderHeight+statheight);
							}
						}
						if(cwn = AllocCopperWait(gametop, DMACON, DMAF_SETCLR|DMAF_RASTER|DMAF_SPRITE))
						{
							sh->sh_CopperDMAOn = cwn;
							if(cwn = AllocCopperWait(gamebottom+displaytop, DMACON, DMAF_RASTER))
							{
								sh->sh_CopperDMAOff = cwn;
								gameheight = gamebottom;
								/* Slice Header Done, Now do the Main Slice */
			
								if(disp->dp_DisplayID & LACE_KEY)
									AllocInterlaceData(slice);
								else
									slice->ss_InterlaceData = NULL;
			
								if(!NullName(disp->dp_InitProgram))
								{
									if( prognode = (struct ProgNode *)FindCompressedName(&proglist, disp->dp_InitProgram, NULL) )
									{
										slice->ss_Channel0 = prognode->pn_Prog;
									}
									else
									{
										err = LAMERR_PROGNOTFOUND;
										UnpackASCII(disp->dp_InitProgram, namebuffer, PROGFULLNAMESIZE-1);
										sprintf(errorstring, "Error Code: %ld\nAction List: %s\nDescription: Level Init Action List Not Found\n", err, namebuffer);
										DisplayError(errorstring);
									}
								}
								else
								{
									slice->ss_Channel0 = NULL;
									D2(bug(" InitProgram: NULL\n"));
								}
							}
							else
							{
								/* Couldn't Allocate DMAOffCopperWait Structure */
								err = LAMERR_MEM;
								sprintf(errorstring, "Error Code: %ld\nDescription: Not Enough Memory\n", err);
								DisplayError(errorstring);
							}
						}
						else
						{
							/* Couldn't Allocate DMAOnCopperWait Structure */
							err = LAMERR_MEM;
							sprintf(errorstring, "Error Code: %ld\nDescription: Not Enough Memory\n", err);
							DisplayError(errorstring);
						}
					}
				}
				else
				{
					/* Display Palette Too Small */
					err = LAMERR_DISPLAY_PALETTE_TOO_SMALL;
					sprintf(errorstring, "Error Code: %ld\nDescription: Palette In Display Chunk Too Small\nMust Be At Least Two Colours", err);
					DisplayError(errorstring);
				}
			}
		}
		else
		{
			/* Couldn't Allocate GameSlice Structure */
			err = LAMERR_MEM;
			sprintf(errorstring, "Error Code: %ld\nDescription: Not Enough Memory\n", err);
			DisplayError(errorstring);
		}
	}
	else
	{
		/* Couldn't Allocate Slice Node */
		err = LAMERR_MEM;
		sprintf(errorstring, "Error Code: %ld\nDescription: Not Enough Memory\n", err);
		DisplayError(errorstring);
	}
	return(err);
}

void FreeSliceNode(struct SliceNode *sn)
{
	struct GameSlice		*slice;
	struct SliceHeader	*sh;

	if(sn)
	{
		Remove(&sn->sn_Node);
		if(slice = sn->sn_SliceData)
		{
			sh = &slice->ss_SliceHeader;

			if(slice->ss_InterlaceData)
				FreeInterlaceData(slice);

			if(sh->sh_CopperDMAOn)
				FreeCopperWait(sh->sh_CopperDMAOn);

			if(sh->sh_CopperDMAOff)
				FreeCopperWait(sh->sh_CopperDMAOff);

			if(sh->sh_CopperHeader)
				FreeCopperHeader(sh);

			DeallocBitMaps(sn);

			MYFREEVEC(slice);
		}
		MYFREEVEC(sn);
	}
}

/***************************   AllocScroll()   *******************************/

UWORD AllocScroll(struct GameSlice *slice, struct MAPHeader *mapheader,
								 struct BLKSHeader *blksheader, UWORD bufferwidth, UWORD bufferheight)
{
	struct PaulsScroll *scroll		= NULL;
	ULONG												 dlta,xpos,blkshigh;
	UWORD								err				= NULL;
	UWORD								count			= 0;

	if( scroll = MYALLOCVEC( sizeof(struct PaulsScroll), MEMF_ANY|MEMF_CLEAR,"ScrollStruct") )
	{
		slice->ss_Scroll = scroll;

		scroll->sc_CopperScrollANDBits = 0xFFFF;
	
		if(mapheader)
		{
			scroll->sc_MapBank = mapheader->mh_Data;
			scroll->sc_MapBlocksHigh = mapheader->mh_BlocksHigh;
			scroll->sc_MapBlocksWide = mapheader->mh_BlocksWide;
		}
		scroll->sc_BufferBlocksHigh = bufferheight/16;
		scroll->sc_BufferBlocksWide = bufferwidth/2;
		if(blksheader)
		{
			scroll->sc_BlockBank = blksheader->blks_Data;
			scroll->sc_BlockSize = 2 * 16 * slice->ss_SliceHeader.sh_Planes;
		}
		/* rightborder = ((mapblockswide*16) - bufferwidth)<<5 */
		if( (scroll->sc_RightBorder = ((scroll->sc_MapBlocksWide << 4) - (scroll->sc_BufferBlocksWide<<4))<<5) < 0)
			scroll->sc_RightBorder = 0;
		else
		{
			/* BC - added to allow scrolling to right edge of map */
			/* bit kludgy, but seems to work ok */
			scroll->sc_RightBorder += ((4*16)-1)<<5;
		}


		if( scroll->sc_LogPosToDraw = MYALLOCVEC( scroll->sc_BufferBlocksHigh * 4, MEMF_ANY|MEMF_CLEAR,"LogPosToDraw") )
		{
			if( scroll->sc_LogBlocksToDraw = MYALLOCVEC( scroll->sc_BufferBlocksHigh * 4, MEMF_ANY|MEMF_CLEAR,"LogBlocksToDraw") )
			{
				scroll->sc_BlockSizexBufferWidth = (scroll->sc_BlockSize/2) * bufferwidth;
	
				D(bug(" BufferBlocksHigh: %lu\n",scroll->sc_BufferBlocksHigh));
	
				blkshigh = ((ULONG)scroll->sc_BufferBlocksHigh)<<12;
				dlta =  blkshigh / 16;
				xpos = 0;
				D2(bug(" Table: "));
				for(count = 0; count < 15; count++)
				{
					xpos += dlta;
					scroll->sc_NeededDrawnTable[count] = (UWORD)(xpos>>12);
					D2(bug("%lu  ",scroll->sc_NeededDrawnTable[count] ));
				}
				scroll->sc_NeededDrawnTable[15] = blkshigh>>12;
				D2(bug("%lu\n",scroll->sc_NeededDrawnTable[15]));
			}
			else
			{
				/* Can't allocate Log Blocks to draw */
				err = LAMERR_MEM;
				sprintf(errorstring, "Error Code: %ld\nDescription: Not Enough Memory\n", err);
				DisplayError(errorstring);
			}
		}
		else
		{
			/* Can't alloc Log pos to draw */
			err = LAMERR_MEM;
			sprintf(errorstring, "Error Code: %ld\nDescription: Not Enough Memory\n", err);
			DisplayError(errorstring);
		}
	}
	else
	{
		/* Can't alloc Scroll Struct */
		err = LAMERR_MEM;
		sprintf(errorstring, "Error Code: %ld\nDescription: Not Enough Memory\n", err);
		DisplayError(errorstring);
	}
	return(err);
}

void FreeScroll(struct GameSlice *slice)
{
	struct PaulsScroll	*scroll;

	if(scroll = slice->ss_Scroll)
	{
		if(scroll->sc_LogPosToDraw)
			MYFREEVEC(scroll->sc_LogPosToDraw);
		if(scroll->sc_LogBlocksToDraw)
			MYFREEVEC(scroll->sc_LogBlocksToDraw);
		MYFREEVEC(scroll);
		slice->ss_Scroll = NULL;
	}
}

/*******************  CookDisplay()   ****************************/

UWORD CookDisplay( struct LameReq *lr )
{
	struct ChunkNode						*cn, *tmpcn;
	struct Display							*disp							= NULL;
	struct BLKSHeader						*blksheader				= NULL;
	struct MAPHeader						*mapheader				= NULL;
	struct GameSlice						*slice						= NULL;
	struct BulletNode						*bn								= NULL;
	struct BadDudeNode					*bdn							= NULL;
	struct PaulsObject					*prevobject				= NULL;
	struct TimedWaveNode				*twn							= NULL;
	struct ILBMNode							*ilbmnode					= NULL;
	struct PaulsPictureImage		*ppi							= NULL;
	UWORD 											 err							= LAMERR_ALLOK;
	UWORD												 bitmapwidth, bitmapheight, count;
	char												 namebuffer[PROGFULLNAMESIZE];
	struct ProgNode *prognode;

	if(cn = FindChunkByType(ID_DISP))
	{
		/* Found a DISP chunk */
		D(bug("--- Cook DISP ---\n"));

		disp = (struct Display *)cn->cn_Data;

		lr->lr_FrameRate = disp->dp_FrameRate;

		lr->lr_LevelUpdateProg = NULL;
		if( !NullName(disp->dp_StatUpdateProg) )
		{
			if( prognode = (struct ProgNode *)FindCompressedName(&proglist, disp->dp_StatUpdateProg, NULL) )
				lr->lr_LevelUpdateProg = prognode->pn_Prog;
			else
			{
				err = LAMERR_PROGNOTFOUND;
				UnpackASCII(disp->dp_StatUpdateProg, namebuffer, PROGFULLNAMESIZE-1);
				sprintf(errorstring, "Error Code: %ld\nAction List: %s\nDescription: LevelUpdate ActionList Not Found\n", err, namebuffer);
				DisplayError(errorstring);
				goto abort;
			}
		}


		/* Sort out the maps (if any) */
		if(disp->dp_Map[0] != 0)
		{
			if( tmpcn = FindChunkByName(ID_MAP,disp->dp_Map) )
			{
				mapheader = tmpcn->cn_Data;
				D2(bug(" MapName: %s  ",mapheader->mh_Name));

				if(disp->dp_BlockSet[0] != 0)
				{
					if( tmpcn = FindChunkByName(ID_BLKS,disp->dp_BlockSet) )
					{
						blksheader = tmpcn->cn_Data;
						disp->dp_LevelDepth = blksheader->blks_Depth;
						D2(bug(" BlockName: %s\n",blksheader->blks_Name));
					}
					else
					{
						err = LAMERR_BLOCKSNOTFOUND;
						sprintf(errorstring, "Error Code: %ld\nBlock Set: %s\nDescription: Block Set Not Found\n", err, disp->dp_BlockSet);
						DisplayError(errorstring);
					}
				}
				else
				{
					err = LAMERR_UNDEFINEDBLOCKS;
					sprintf(errorstring, "Error Code: %ld\nDescription: No Block Set Defined in Display Chunk\n", err);
					DisplayError(errorstring);
				}
			}
			else
			{
				err = LAMERR_MAPNOTFOUND;
				sprintf(errorstring, "Error Code: %ld\nMap Name: %s\nDescription: Map Not Found\n", err, disp->dp_Map);
				DisplayError(errorstring);
			}
		}
		else
		{
			/* No Map so check for a Backdrop Image */
			if(!NullName(disp->dp_BackDrop))
			{
				if( ilbmnode = (struct ILBMNode *)FindCompressedName(&ilbmlist, disp->dp_BackDrop, NULL) )
				{
					ppi = &ilbmnode->ilbm_PaulsPictureImage;
					disp->dp_LevelDepth = ppi->ppi_Depth;
				}
				else
				{
					err = LAMERR_IMAGENOTFOUND;
					UnpackASCII(disp->dp_BackDrop, namebuffer, BOBFULLNAMESIZE-1);
					sprintf(errorstring, "Error Code: %ld\nDescription: Picture '%s' Not Found\nFor Game Backdrop\n", err, namebuffer);
					DisplayError(errorstring);
				}
			}
			else
				D(bug(" NULL Backdrop Name\n"));
		}

		if( err >= LAME_FAILAT)
			goto abort;

		if(mapheader)
		{
			bitmapwidth = mapheader->mh_BlocksWide * 16;
			bitmapheight = mapheader->mh_BlocksHigh * 16;
		}
		else
		{
			if(ppi)
			{
				bitmapwidth = ppi->ppi_Width;			/* ((ppi->ppi_Width>>3)+7)&0xFFF8; */
				bitmapheight = ppi->ppi_Height;
			}
			else
			{
				bitmapwidth = disp->dp_LevelWidth;
				bitmapheight = disp->dp_LevelHeight;
			}
		}
		if( (err = AllocNewSliceNode(bitmapwidth, bitmapheight, disp,
					ppi)) < LAME_FAILAT)
		{
			slice = ((struct SliceNode *)slicelist.lh_TailPred)->sn_SliceData;

						if(IsListEmpty(&playerlist))
							slice->ss_Players = NULL;
						else
							slice->ss_Players = &((struct PlayerNode *)playerlist.lh_Head)->pn_Bob.bb_PaulsObject;
						slice->ss_NumOfPlayers = disp->dp_NumOfPlayers;

						D(bug(" NumOfBulletStructs: %lu\n Count: ",disp->dp_NumOfBulletStructs));
						prevobject = NULL;
						for(count = 0;
							 (count < disp->dp_NumOfBulletStructs) && (err < LAME_FAILAT);
								count++)
						{
							if( !(bn = MYALLOCVEC(sizeof(struct BulletNode),MEMF_ANY|MEMF_CLEAR,"BulletBobNode")) )
							{
								err = LAMERR_MEM;
								sprintf(errorstring, "Error Code: %ld\nDescription: Not Enough Memory\n", err);
								DisplayError(errorstring);
								goto abort;
							}
							bn->bn_Bob.bb_PaulsObject.ob_ScreenOffset1 = -1;
							bn->bn_Bob.bb_PaulsObject.ob_ScreenOffset2 = -1;
							AddTail( &bulletlist, &bn->bn_Node );
							if(count == 0)
								slice->ss_PlayerBullets = &bn->bn_Bob.bb_PaulsObject;
							bn->bn_Bob.bb_PaulsObject.ob_Previous = prevobject;
							if(prevobject)
								prevobject->ob_Next = &bn->bn_Bob.bb_PaulsObject;

							prevobject = &bn->bn_Bob.bb_PaulsObject;
						}

						D(bug(" NumOfBDStructs: %lu\n",disp->dp_NumOfBDStructs));
						prevobject = NULL;
						for(count = 0;
							 (count < disp->dp_NumOfBDStructs) && (err < LAME_FAILAT);
								count++)
						{
							if( !(bdn = MYALLOCVEC(sizeof(struct BadDudeNode),MEMF_ANY|MEMF_CLEAR,"BadDudeBobNode")) )
							{
								err = LAMERR_MEM;
								sprintf(errorstring, "Error Code: %ld\nDescription: Not Enough Memory\n", err);
								DisplayError(errorstring);
								goto abort;
							}
							bdn->bdn_Bob.bb_PaulsObject.ob_ScreenOffset1 = -1;
							bdn->bdn_Bob.bb_PaulsObject.ob_ScreenOffset2 = -1;
							AddTail( &baddudelist, &bdn->bdn_Node );
							if(count == 0)
								slice->ss_BadDudes = &bdn->bdn_Bob.bb_PaulsObject;
							bdn->bdn_Bob.bb_PaulsObject.ob_Previous = prevobject;
							if(prevobject)
								prevobject->ob_Next = &bdn->bdn_Bob.bb_PaulsObject;

							prevobject = &bdn->bdn_Bob.bb_PaulsObject;
						}

			slice->ss_NextRightScrollAttackWave = firstleftswav;
			slice->ss_NextLeftScrollAttackWave = firstrightswav;
			if(!IsListEmpty(&timedwavelist))
			{
				twn = (struct TimedWaveNode *)timedwavelist.lh_Head;
				slice->ss_NextTimeAttackWave = twn->twn_PaulsTimedWave;
				slice->ss_TimedAttackWavesToGo = twn->twn_NumberOfTimedWaves;
			}
			if(slice->ss_ActiveAttackWaves = MYALLOCVEC(sizeof(struct PaulsActiveWaveData)*50,MEMF_ANY|MEMF_CLEAR,"ActiveAttackWaves"))
			{
				slice->ss_DefaultBorderLeft = disp->dp_DefaultBorderLeft;
				slice->ss_DefaultBorderLeftType = disp->dp_DefaultBorderLeftType;
				slice->ss_DefaultBorderRight = disp->dp_DefaultBorderRight;
				slice->ss_DefaultBorderRightType = disp->dp_DefaultBorderRightType;
				slice->ss_DefaultBorderTop = disp->dp_DefaultBorderTop;
				slice->ss_DefaultBorderTopType = disp->dp_DefaultBorderTopType;
				slice->ss_DefaultBorderBottom = disp->dp_DefaultBorderBottom;
				slice->ss_DefaultBorderBottomType = disp->dp_DefaultBorderBottomType;
				slice->ss_DefaultBorderFlags = disp->dp_DefaultBorderFlags;
	
				slice->ss_PushScrollLeft = 10<<5;
				slice->ss_PushScrollRight = 310<<5;
				slice->ss_PushScrollTop = 10<<5;
				slice->ss_PushScrollBottom = 190<<5;
		
				slice->ss_CenterLine = gameheight / 2;
				slice->ss_BottomScreenPos = bitmapheight-gameheight;
				if(slice->ss_BottomScreenPos < 0)
					slice->ss_BottomScreenPos = 0;
				slice->ss_ClipLeft = 0;
				slice->ss_ClipRight = (disp->dp_DisplayID & HIRES_KEY) ? 640<<5 : 320<<5;
				slice->ss_ClipTop = 0;
				slice->ss_ClipBottom = bitmapheight<<5;
			
				/* Slice Done, now do the Scroll Thingy */
	
				D(bug("About to alloc scroll\n"));
				if( (err = AllocScroll(slice, mapheader, blksheader, (disp->dp_DisplayID & HIRES_KEY) ? 88 : 48, bitmapheight)) < LAME_FAILAT)
				{
					D(bug("AllocScroll Done\n"));
					D(bug("About to AllocCopperList\n"));
					if( (err = AllocCopperList(slice,disp,&displaynode)) < LAME_FAILAT )
					{
						D(bug("AllocCopperList Done\n"));
					}
				}
			}
			else
			{
				err = LAMERR_MEM;
				sprintf(errorstring, "Error Code: %ld\nDescription: Not Enough Memory\n", err);
				DisplayError(errorstring);
			}
		}
		cn->cn_Cooked = TRUE;
	}
abort:
	return(err);
}

/****************   FreeCookedDisplay()   ************************/

void 	FreeCookedDisplay( void )
{
	struct	SliceNode	*sn,*nextsn;
	struct GameSlice			*slice;
	struct SliceHeader *sh;
	struct BulletNode *bn;
	struct BadDudeNode *bdn;

	while(!IsListEmpty(&baddudelist))
	{
		bdn = (struct BadDudeNode *)baddudelist.lh_TailPred;
		Remove(&bdn->bdn_Node);
		MYFREEVEC(bdn);
	}

	while(!IsListEmpty(&bulletlist))
	{
		bn = (struct BulletNode *)bulletlist.lh_TailPred;
		Remove(&bn->bn_Node);
		MYFREEVEC(bn);
	}

	sn = (struct SliceNode *)slicelist.lh_Head;
	while(sn->sn_Node.ln_Succ)
	{
		nextsn = (struct SliceNode *)sn->sn_Node.ln_Succ;
		if(sn->sn_Type == SNTYPE_GAMESLICE)
		{
			Remove(&sn->sn_Node);
			slice = sn->sn_SliceData;
			sh = &slice->ss_SliceHeader;

			if(slice->ss_ActiveAttackWaves) MYFREEVEC(slice->ss_ActiveAttackWaves);

			DeallocCopperList(&displaynode);

			FreeScroll(slice);

			FreeSliceNode(sn);
		}
		sn = nextsn;
	}
}

void CheckUniqueDisplayChunk(void)
{
	struct FileNode *fn;
	struct ChunkNode *cn;
	UWORD err = LAMERR_ALLOK;
	UWORD displaychunks = 0;

	for( fn = (struct FileNode *)filelist.lh_Head;
		fn->fn_Node.ln_Succ && err < LAME_FAILAT;
		fn = (struct FileNode *)fn->fn_Node.ln_Succ )
	{
		if( fn->fn_UsedInLevel )
		{
			for( cn = (struct ChunkNode *)fn->fn_ChunkList.lh_Head;
				cn->cn_Node.ln_Succ && err < LAME_FAILAT;
				cn = (struct ChunkNode *)cn->cn_Node.ln_Succ )
			{
				if( cn->cn_TypeID == ID_DISP )
					displaychunks++;
			}
		}
	}
	if(displaychunks > 1)
	{
		err = LAMERR_MULTIPLE_DISP_CHUNKS;
		sprintf(errorstring, "Warning %ld: Multiple Display Chunks Found\nUsing First Chunk\n", err);
		DisplayWarning(errorstring);
	}
}

/****************   PreCookStatBox()   *********************/
//
//  This routine is used to simply calculate the statheight, and
// statheaderheight. This makes it a lot easier to calculate the
// gameslice stuff.
//  If there's no stat slice then it zero's the height fields.

UWORD PreCookStatBox( struct LameReq *lr )
{
	struct ChunkNode						*cn;
	struct Display							*disp							= NULL;
	struct PaulsPictureImage		*ppi							= NULL;
	struct ILBMNode							*ilbmnode					= NULL;
	UWORD 											 err							= LAMERR_ALLOK;
	UBYTE												 namebuffer[BOBFULLNAMESIZE];

	statheight = 0;
	if(cn = FindChunkByType(ID_DISP))
	{
		/* Found a DISP chunk */
		D(bug("--- PreCook STAT ---\n"));

		disp = (struct Display *)cn->cn_Data;

		if(!NullName(disp->dp_StatBackDropPicture))
		{
			if( ilbmnode = (struct ILBMNode *)FindCompressedName(&ilbmlist, disp->dp_StatBackDropPicture, NULL) )
			{
				ppi = &ilbmnode->ilbm_PaulsPictureImage;
				statheight = ppi->ppi_Height;
			}
			else
			{
				err = LAMERR_IMAGENOTFOUND;
				UnpackASCII(disp->dp_StatBackDropPicture, namebuffer, BOBFULLNAMESIZE-1);
				sprintf(errorstring, "Error Code: %ld\nDescription: Picture '%s' Not Found\nFor Stat Box\n", err, namebuffer);
				DisplayError(errorstring);
			}
		}
		else
			D(bug(" No Stat Box Picture\n"));
	}
	return(err);
}

/*******************  CookStatBox()   ****************************/

UWORD CookStatBox( struct LameReq *lr )
{
	struct ChunkNode						*cn;
	struct Display							*disp							= NULL;
	struct PaulsPictureImage		*ppi							= NULL;
	struct StatSlice						*slice						= NULL;
	struct ILBMNode							*ilbmnode					= NULL;
	struct SliceNode						*sn								= NULL;
	struct SliceHeader					*sh = NULL, *prevsh = NULL;
	struct CopperWaitNode				*cwn							= NULL;
	struct ProgNode							*prognode					= NULL;
	UWORD												 bitmapwidth, bitmapdepth;
	ULONG												 bitmapsize;
	UWORD 											 err							= LAMERR_ALLOK;
	UBYTE												 namebuffer[BOBFULLNAMESIZE];

	if(cn = FindChunkByType(ID_DISP))
	{
		/* Found a DISP chunk */
		D(bug("--- Cook STAT ---\n"));

		disp = (struct Display *)cn->cn_Data;

		if(!NullName(disp->dp_StatBackDropPicture))
		{
			if( ilbmnode = (struct ILBMNode *)FindCompressedName(&ilbmlist, disp->dp_StatBackDropPicture, NULL) )
			{
				ppi = &ilbmnode->ilbm_PaulsPictureImage;
				bitmapwidth = ((ppi->ppi_Width>>3)+7)&0xFFF8;
				statheight = ppi->ppi_Height;
				bitmapdepth = ppi->ppi_Depth;

				if(disp->dp_Flags & DPF_STATBOXATTOP)
					stattop = displaytop;
				else
				{
					stattop = gameheight+disp->dp_StatHeaderHeight;
					/* No need to check for going off the bottom of the display, because
						gameheight takes all that into account in AllocSliceNode() */
					stattop += displaytop;
				}
	
				D(bug("BitMap Byte Width: %ld  BitMap Height: %ld  BitMap Depth %ld\n",bitmapwidth, statheight, bitmapdepth));
	
				if( sn = MYALLOCVEC( sizeof(struct SliceNode), MEMF_ANY|MEMF_CLEAR,"StatBoxSliceNode") )
				{
					if( slice = MYALLOCVEC( sizeof(struct StatSlice), MEMF_ANY|MEMF_CLEAR,"StatBoxSlice") )
					{
						sn->sn_SliceData = slice;
						sn->sn_Type = SNTYPE_STATBOX;
						/* We can just add on the tail here, 'cos if the stat box is at the top
							then 'lamemain' will cook the stat first, then the game */
						AddTail(&slicelist, &sn->sn_Node);
						sh = &slice->st_SliceHeader;
						if(sn->sn_Node.ln_Pred->ln_Pred)
						{
							prevsh = ((struct SliceNode *)sn->sn_Node.ln_Pred)->sn_SliceData;
							prevsh->sh_Next = sh;
						}
						else
							prevsh = NULL;	
						sh->sh_Next = NULL;

						sh->sh_DisplayID = disp->dp_StatDisplayID;
		
						bitmapsize = (bitmapwidth * statheight * bitmapdepth) + 7;
						D(bug("BitMapSize: %ld\n",bitmapsize));
	
						if(bitmapsize > 7)
						{
							sn->sn_BitMap1 = ppi->ppi_Image;
							sn->sn_BitMap3 = 0;				/* BitMap Not allocated */
							sh->sh_PhyBase = ppi->ppi_Image;
	
							sh->sh_Planes = bitmapdepth;
							sh->sh_ColourPointers = ppi->ppi_Palette;
							sh->sh_Colours = 1<<ppi->ppi_Depth;
							D(bug(" Colours: %ld\n",sh->sh_Colours));
	
							sh->sh_ColourOffset = 0;
							sh->sh_Flags = 0;
							sh->sh_BufferWidth = bitmapwidth;
							sh->sh_BufferWidthx16xPlanes = bitmapwidth * 16 * bitmapdepth;
							sh->sh_BufferWidthxPlanes = bitmapwidth * bitmapdepth;
							if(disp->dp_StatDisplayID & LACE_KEY)
								sh->sh_BufferWidthxIPlanes = bitmapwidth * bitmapdepth * 2;
							else
								sh->sh_BufferWidthxIPlanes = bitmapwidth * bitmapdepth;
							sh->sh_BufferHeight = statheight;

							if( (err = AllocCopperHeader(sh, bitmapdepth, disp->dp_StatDisplayID)) < LAME_FAILAT)
							{
								if( (cwn = AllocCopperWait(stattop, DMACON, DMAF_SETCLR|DMAF_RASTER|DMAF_SPRITE)) )
			 					{
									sh->sh_CopperDMAOn = cwn;
									if( (cwn = AllocCopperWait(stattop+statheight, DMACON, DMAF_RASTER)) )
				 					{
										sh->sh_CopperDMAOff = cwn;
		
										/* Slice Header Done, Now do the Main Slice */
		
										//if(disp->dp_StatDisplayID & LACE_KEY)
										//	err = AllocInterlaceData(slice);
										//else
											slice->st_InterlaceData = NULL;
		
										if(err < LAME_FAILAT)
										{
											/* Set up GroovyCon Stuff */
											GC_InitGroovyConStruct(&StatBoxGroovyCon);
											StatBoxGroovyCon.gc_GfxMem = sh->sh_PhyBase;
											StatBoxGroovyCon.gc_BytesPerPlane = sh->sh_BufferWidth;
											StatBoxGroovyCon.gc_BytesPerRow = sh->sh_BufferWidthxPlanes;
											StatBoxGroovyCon.gc_Depth = sh->sh_Planes;
											StatBoxGroovyCon.gc_ConWidth = sh->sh_BufferWidth;
											StatBoxGroovyCon.gc_ConHeight = sh->sh_BufferHeight>>3;
											StatBoxGroovyCon.gc_TextFont = TopazTextFont;
	
											if( !NullName(disp->dp_StatUpdateProg) )
											{
												if( prognode = (struct ProgNode *)FindCompressedName(&proglist, disp->dp_StatUpdateProg, NULL) )
												{
													slice->st_Channel0 = prognode->pn_Prog;
												}
												else
												{
													err = LAMERR_PROGNOTFOUND;
													UnpackASCII(disp->dp_StatUpdateProg, namebuffer, PROGFULLNAMESIZE-1);
													sprintf(errorstring, "Error Code: %ld\nAction List: %s\nDescription: Stat Bar Update Action List Not Found\n", err, namebuffer);
													DisplayError(errorstring);
												}
											}
											else
											{
												slice->st_Channel0 = NULL;
												D2(bug(" InitProgram: NULL\n"));
											}
										}
									}
									else
									{
										/* Can't Alloc Copper Footer, error already displayed */
										err = LAMERR_MEM;
									}
								}
								else
								{
									/* Can't Alloc Copper DMA On, error already displayed */
									err = LAMERR_MEM;
								}
							}
						}
						else
						{
							/* BitMap Size == 0 */
							err = LAMERR_BITMAP_BAD_SIZE;
							sprintf(errorstring, "Error Code: %ld\nDescription: Bitmap Size = 0 for Stat Slice\n", err);
							DisplayError(errorstring);
						}
					}
					else
					{
						/* Couldn't Allocate Slice Structure */
						err = LAMERR_MEM;
						sprintf(errorstring, "Error Code: %ld\nDescription: Not Enough Memory\n", err);
						DisplayError(errorstring);
					}
				}
				else
				{
					/* Couldn't Allocate StatSlice Node */
					err = LAMERR_MEM;
					sprintf(errorstring, "Error Code: %ld\nDescription: Not Enough Memory\n", err);
					DisplayError(errorstring);
				}
			}
			else
			{
				err = LAMERR_IMAGENOTFOUND;
				UnpackASCII(disp->dp_StatBackDropPicture, namebuffer, BOBFULLNAMESIZE-1);
				sprintf(errorstring, "Error Code: %ld\nDescription: Picture '%s' Not Found\nFor Stat Box\n", err, namebuffer);
				DisplayError(errorstring);
			}
		}
		else
			D(bug(" NULL Backdrop Name\n"));

		cn->cn_Cooked = TRUE;
	}
	return(err);
}

/****************   FreeCookedStatBox()   ************************/

void 	FreeCookedStatBox( void )
{
	struct	SliceNode					*sn, *nextsn;
	struct StatSlice					*slice;
	struct SliceHeader				*sh;

	sn = (struct SliceNode *)slicelist.lh_Head;
	while(sn->sn_Node.ln_Succ)
	{
		nextsn = (struct SliceNode *)sn->sn_Node.ln_Succ;
		if(sn->sn_Type == SNTYPE_STATBOX)
		{
			Remove(&sn->sn_Node);
			if(slice = sn->sn_SliceData)
			{
				sh = &slice->st_SliceHeader;

				//if(slice->st_InterlaceData)
				//	FreeInterlaceData(slice);

				if(sh->sh_CopperDMAOn)
					FreeCopperWait(sh->sh_CopperDMAOn);

				if(sh->sh_CopperDMAOff)
					FreeCopperWait(sh->sh_CopperDMAOff);

				if(sh->sh_CopperHeader)
					FreeCopperHeader(sh);

				MYFREEVEC(sn->sn_SliceData);
				sn->sn_SliceData = NULL;
			}
			MYFREEVEC(sn);
		}
		sn = nextsn;
	}
}

UWORD	AllocInterlaceData(struct GameSlice *slice)
{
	UWORD	err = LAMERR_ALLOK;
	struct SliceHeader	*osh, *nsh;

	if( !(nsh = MYALLOCVEC(sizeof(struct SliceHeader),MEMF_ANY|MEMF_CLEAR,"InterlaceSliceHeader")) )
	{
		err = LAMERR_MEM;
		sprintf(errorstring, "Error Code: %ld\nDescription: Not Enough Memory\n", err);
		DisplayError(errorstring);
		return(err);
	}

	slice->ss_InterlaceData = nsh;
	osh = &slice->ss_SliceHeader;

	nsh->sh_Next = NULL;
	nsh->sh_DisplayID = osh->sh_DisplayID;
	nsh->sh_LogBase = osh->sh_LogBase + osh->sh_BufferWidthxPlanes;
	nsh->sh_PhyBase = osh->sh_PhyBase + osh->sh_BufferWidthxPlanes;
	nsh->sh_TriBase = osh->sh_TriBase + osh->sh_BufferWidthxPlanes;

	nsh->sh_CopperList = osh->sh_CopperList;
	nsh->sh_CopperHeader = osh->sh_CopperHeader;
	nsh->sh_CopperHeaderSize = osh->sh_CopperHeaderSize;
	nsh->sh_CopperDMAOn = osh->sh_CopperDMAOn;
	nsh->sh_CopperDMAOff = osh->sh_CopperDMAOff;

	nsh->sh_Planes = osh->sh_Planes;

	nsh->sh_ColourPointers = osh->sh_ColourPointers;
	nsh->sh_Colours = osh->sh_Colours;
	nsh->sh_ColourOffset = osh->sh_ColourOffset;

	nsh->sh_Flags = osh->sh_Flags;
	nsh->sh_ScreenToggle = osh->sh_ScreenToggle;

	nsh->sh_BufferWidth = osh->sh_BufferWidth;
	nsh->sh_BufferWidthx16xPlanes = osh->sh_BufferWidthx16xPlanes;
	nsh->sh_BufferWidthxPlanes = osh->sh_BufferWidthxPlanes;
	nsh->sh_BufferWidthxIPlanes = osh->sh_BufferWidthxIPlanes;
	nsh->sh_BufferHeight = osh->sh_BufferHeight;
}

void FreeInterlaceData(struct GameSlice *slice)
{
	struct SliceHeader *sh;

	if(sh = slice->ss_InterlaceData)
	{
		MYFREEVEC(sh);
		slice->ss_InterlaceData = NULL;
	}
}

void CopyBitMap(ULONG *srcptr, ULONG *destptr, ULONG bmbytesize)
{
	CopyMem(srcptr,destptr,bmbytesize);

/*
	ULONG bmlongsize, count;

	bmlongsize = bmbytesize/4;

	for(count = 0; count < bmlongsize; count++)
	{
		*destptr++ = *srcptr++;
	}
*/
}

UWORD AllocBitMaps(struct SliceNode *sn, ULONG bmsize, struct PaulsPictureImage *ppi)
{
	UWORD err = LAMERR_CHIPMEM;
	struct GameSlice *slice;

	slice = sn->sn_SliceData;

	D2(bug("BitMapSize: %ld\n",bmsize));
	sn->sn_BitMap2 = NULL; 		/* need this for error cleanup */
	if(sn->sn_BitMap1 = MYALLOCVEC(bmsize, MEMF_CHIP|MEMF_CLEAR,"AllocBitMaps() BitMap1") )
	{
		slice->ss_SliceHeader.sh_LogBase = (UBYTE *)( (((ULONG)sn->sn_BitMap1) + 7) & 0xFFFFFFF8 );
		D2(bug(" BitMap1: %08lx ", sn->sn_BitMap1));
		if(sn->sn_BitMap2 = MYALLOCVEC(bmsize, MEMF_CHIP|MEMF_CLEAR,"AllocBitMaps() BitMap2") )
		{
			slice->ss_SliceHeader.sh_PhyBase = (UBYTE *)( (((ULONG)sn->sn_BitMap2) + 7) & 0xFFFFFFF8 );
			D2(bug(" BitMap2: %08lx ",sn->sn_BitMap2));
			if( ppi )
			{
				sn->sn_BitMap3 = NULL;
				slice->ss_SliceHeader.sh_TriBase = ppi->ppi_Image;
				err = LAMERR_ALLOK;

				/* note the 'bmsize-8' below -> this is a fix for a mungwall hit we were getting, where */
				/* CopyBitMap() was writing over four bytes after our allocation beacuse it wasn't */
				/* allowing for the fact that the bitmap doesn't start exactly on the start of the */
				/* allocated memory, but on the closest 8byte boundary - Ben */
				CopyBitMap((ULONG *)slice->ss_SliceHeader.sh_TriBase, (ULONG *)slice->ss_SliceHeader.sh_LogBase, bmsize-8);
				CopyBitMap((ULONG *)slice->ss_SliceHeader.sh_TriBase, (ULONG *)slice->ss_SliceHeader.sh_PhyBase, bmsize-8);
				D2(bug("BitMap3 An Image\n"));
			}
			else
			{
				if( sn->sn_BitMap3 = MYALLOCVEC(bmsize, MEMF_CHIP|MEMF_CLEAR,"AllocBitMaps() BitMap3") )
				{
					D2(bug(" BitMap3: %08lx\n", sn->sn_BitMap3));
					slice->ss_SliceHeader.sh_TriBase = (UBYTE *)( (((ULONG)sn->sn_BitMap3) + 7) & 0xFFFFFFF8 );
					err = LAMERR_ALLOK;
				}
			}
		}	
	}
	if(err == LAMERR_CHIPMEM)
	{
		/* Grant, you dork, you forgot this: */
		if( !sn->sn_BitMap1 )
			MYFREEVEC( sn->sn_BitMap1 );
		if( !sn->sn_BitMap2 )
			MYFREEVEC( sn->sn_BitMap2 );
		/* we were losing chipmem. */

		sprintf(errorstring, "Error Code: %ld\nDescription: Not Enough Chip Memory\n", err);
		DisplayError(errorstring);
	}
	return(err);
}

void DeallocBitMaps(struct SliceNode *sn)
{
	if(sn->sn_BitMap1) MYFREEVEC(sn->sn_BitMap1);
	if(sn->sn_BitMap2) MYFREEVEC(sn->sn_BitMap2);
	if(sn->sn_BitMap3) MYFREEVEC(sn->sn_BitMap3);
	sn->sn_BitMap1 = NULL;
	sn->sn_BitMap2 = NULL;
	sn->sn_BitMap3 = NULL;
}

UWORD AllocCopperHeader(struct SliceHeader *sh, UWORD depth, ULONG displayid)
{
	UWORD err = LAMERR_ALLOK;
	struct PaulsCopperHeader *ch;
	UWORD	mybplcon0 = 0;

	if( ch = MYALLOCVEC(sizeof(struct PaulsCopperHeader),MEMF_ANY,"PaulsCopperHeader") )
	{
		D(bug("CopperHeader: %lx\n",ch));
		sh->sh_CopperHeader = (UWORD *)ch;
		ch->ch_Bpl1Mod[0] = BPL1MOD;
		ch->ch_Bpl2Mod[0] = BPL2MOD;
		ch->ch_BplCon0[0] = BPLCON0;
		ch->ch_BplCon1[0] = BPLCON1;
		ch->ch_BplCon2[0] = BPLCON2;
		ch->ch_BplCon3[0] = BPLCON3;
		ch->ch_BplCon4[0] = BPLCON4;
		ch->ch_DdfStrt[0] = DDFSTRT;
		ch->ch_DdfStop[0] = DDFSTOP;
		ch->ch_DiwStrt[0] = DIWSTRT;
		ch->ch_DiwStop[0] = DIWSTOP;
		ch->ch_FMode[0] = FMODE;

		mybplcon0 = 0;
		if(displayid & HIRES_KEY)
		{
			D2(bug(" - Hires Copper Header -\n"));
			//ch->ch_Bpl1Mod[1] = (88 * (depth - 1)) + 4;
			//ch->ch_Bpl2Mod[1] = (88 * (depth - 1)) + 4;

			ch->ch_Bpl1Mod[1] = ((depth - 1) * 88) - 8;
			ch->ch_Bpl2Mod[1] = ((depth - 1) * 88) - 8;
			if(displayid & LACE_KEY)
			{
				ch->ch_Bpl1Mod[1] += (depth * 88);
				ch->ch_Bpl2Mod[1] += (depth * 88);
			}
			mybplcon0 |= MODE_640 | COLORON;
			if(displayid & LACE_KEY)
				mybplcon0 |= INTERLACE;
			if(depth < 8)
				ch->ch_BplCon0[1] = mybplcon0 | depth<<12;
			else
				ch->ch_BplCon0[1] = mybplcon0 | 0x0010;
			ch->ch_BplCon1[1] = 0x4444;
			ch->ch_BplCon2[1] = 0x0224;
			ch->ch_BplCon3[1] = 0x0C40;	//0x1000;
			ch->ch_BplCon4[1] = 0x0011;	//0x0000;
			ch->ch_DdfStrt[1] = 0x0028;	//agaonlygame ? 0x0030 : 0x0034;
			ch->ch_DdfStop[1] = 0x00D8;	//0x00D0;
			ch->ch_DiwStrt[1] = 0x1D75;	//0x2C81;
			ch->ch_DiwStop[1] = 0x27C4;	//0x2CC1;
			ch->ch_FMode[1]   = 0x0003;	//agaonlygame ? 0x0001 : 0x0000;
		}
		else
		{
			/* LOWRES */

			/*XYZZY - statbox appears as crap in AGA games - cos of burst mode? */

			D(bug(" - Lowres Copper Header -\n"));
			if(agaonlygame)
			{
				ch->ch_Bpl1Mod[1] = ((depth-1) * sh->sh_BufferWidth);
				ch->ch_Bpl2Mod[1] = ((depth-1) * sh->sh_BufferWidth);

				if(sh->sh_BufferWidth == 48)		/* StatBox can be 40 bytes wide */
				{
					ch->ch_Bpl1Mod[1] -= 8;
					ch->ch_Bpl2Mod[1] -= 8;
				}
			}
			else
			{
				ch->ch_Bpl1Mod[1] = ((depth-1) * sh->sh_BufferWidth);
				ch->ch_Bpl2Mod[1] = ((depth-1) * sh->sh_BufferWidth);
				if(sh->sh_BufferWidth == 48)		/* StatBox can be 40 bytes wide */
				{
					ch->ch_Bpl1Mod[1] += 6;
					ch->ch_Bpl2Mod[1] += 6;
				}
			}
			if(displayid & LACE_KEY)
			{
				ch->ch_Bpl1Mod[1] += (depth * sh->sh_BufferWidth);
				ch->ch_Bpl2Mod[1] += (depth * sh->sh_BufferWidth);
			}
			mybplcon0 |= COLORON;
			if(displayid & LACE_KEY)
				mybplcon0 |= INTERLACE;
			if(depth < 8)
				ch->ch_BplCon0[1] = mybplcon0 | (depth<<12);
			else
				ch->ch_BplCon0[1] =  mybplcon0 | 0x0010;

			ch->ch_BplCon1[1] = 0x0000;
			ch->ch_BplCon2[1] = 0x0224;
			ch->ch_BplCon3[1] = 0x0C40;
			ch->ch_BplCon4[1] = 0x0011;
			if(sh->sh_BufferWidth == 48)		/* StatBox can be 40 bytes wide */
			{
				ch->ch_DdfStrt[1] = agaonlygame ? 0x0018 : 0x0030;
				ch->ch_DdfStop[1] = agaonlygame ? 0x00D8 : 0x00D0;
			}
			else
			{
				ch->ch_DdfStrt[1] = agaonlygame ? 0x0018 : 0x0038;
				ch->ch_DdfStop[1] = agaonlygame ? 0x00B8 : 0x00D0;
			}
			ch->ch_DiwStrt[1] = 0x2C81;				/* VSTRT, HSTRT */
			ch->ch_DiwStop[1] = 0x2CC1;
//			ch->ch_FMode[1]   = agaonlygame ? 0x0003 : 0x0000;

			/* nasty little hack time: if we're displaying a 320 wide statbox */
			/* in an aga game we turn burst mode off and shrink the datafetch. */
			if( agaonlygame )
			{
				if( sh->sh_BufferWidth == 48 )
					ch->ch_FMode[1] = 0x0003;
				else
				{
					/* statbox, 40 bytes wide - kill burst and reset fetch start/stop */
					ch->ch_FMode[1] = 0x0000;
					ch->ch_DdfStrt[1] = 0x0038;
					ch->ch_DdfStop[1] = 0x00D0;
				}
			}
			else
				ch->ch_FMode[1] = 0x0000;

//				ch->ch_FMode[1]   = agaonlygame ? 0x0003 : 0x0000;
			D(bug("  bpl1mod: %ld\n",ch->ch_Bpl1Mod[1]));
			D(bug("  bpl2mod: %ld\n",ch->ch_Bpl2Mod[1]));
			D(bug("  bplcon0: %04lx\n",ch->ch_BplCon0[1]));
			D(bug("  bplcon1: %04lx\n",ch->ch_BplCon1[1]));
			D(bug("  bplcon2: %04lx\n",ch->ch_BplCon2[1]));
			D(bug("  bplcon3: %04lx\n",ch->ch_BplCon3[1]));
			D(bug("  bplcon4: %04lx\n",ch->ch_BplCon4[1]));
			D(bug("  ddfstrt: %04lx\n",ch->ch_DdfStrt[1]));
			D(bug("  ddfstop: %04lx\n",ch->ch_DdfStop[1]));
			D(bug("  diwstrt: %04lx\n",ch->ch_DiwStrt[1]));
			D(bug("  diwstop: %04lx\n",ch->ch_DiwStop[1]));
			D(bug("  fmode:   %04lx\n",ch->ch_FMode[1]));
		}
		err = LAMERR_ALLOK;
	}
	else
	{
		err = LAMERR_MEM;
		sprintf(errorstring, "Error Code: %ld\nDescription: Not Enough Memory\n", err);
		DisplayError(errorstring);
	}
	sh->sh_CopperHeaderSize = 12;
	return(err);
}

void FreeCopperHeader(struct SliceHeader *sh)
{
	if(sh->sh_CopperHeader)
	{
		MYFREEVEC(sh->sh_CopperHeader);
		sh->sh_CopperHeader = NULL;
		sh->sh_CopperHeaderSize = 0;
	}
}

/**********************   AllocCopperWait()   ***********************/
//
//  Allocates a CopperWaitNode and returns it, if failed it will return
// NULL, so using a LAMERR_MEM is most probably safe.
// Inputs:	line = the hardware line you want to wait for (can be over 255)
//					commandinst = the copper command you want to execute when you get there,
//												can be anything like intreq, or noop if nothing wanted.
//					commandvalue = the word after commandinst.

struct CopperWaitNode *AllocCopperWait(UWORD line, UWORD commandinst, UWORD commandvalue)
{
	UWORD err = LAMERR_ALLOK;
	struct CopperWaitNode *cwn = NULL;

	if( cwn = MYALLOCVEC(sizeof(struct CopperWaitNode),MEMF_ANY,"CopperWaitNode") )
	{
		D(bug(" - Copper Wait -\n"));
		cwn->cwn_Line = line;
		cwn->cwn_Command1[0] = commandinst;
		cwn->cwn_Command1[1] = commandvalue;
		InsertCopperWait( &copperwaitlist, cwn);
	}
	else
	{
		err = LAMERR_MEM;
		sprintf(errorstring, "Error Code: %ld\nDescription: Not Enough Memory\n", err);
		DisplayError(errorstring);
	}
	return(cwn);
}

UWORD CookCopperWaits(  struct LameReq *lr )
{
	UWORD err = LAMERR_ALLOK;
	BOOL inpalarea = FALSE;
	struct CopperWaitNode *cwn;

	for(cwn = (struct CopperWaitNode *)copperwaitlist.mlh_Head;
			cwn->cwn_Node.mln_Succ;
			cwn = (struct CopperWaitNode *)cwn->cwn_Node.mln_Succ)
	{
		cwn->cwn_Wait1[0] = ((cwn->cwn_Line & 0x00FF)<<8)|0x01;
		cwn->cwn_Wait1[1] = 0xFF00;
		cwn->cwn_InstructionNum = 2;
		if( (cwn->cwn_Line & 0x0100) && !inpalarea )
		{
			cwn->cwn_Wait2[0] = cwn->cwn_Wait1[0];
			cwn->cwn_Wait2[1] = cwn->cwn_Wait1[1];
			cwn->cwn_Wait1[0] = 0xFFDF;		/* Pal Area Wait */
			cwn->cwn_Wait1[1] = 0xFFFE;
			cwn->cwn_Command2[0] = cwn->cwn_Command1[0];
			cwn->cwn_Command2[1] = cwn->cwn_Command1[1];
			cwn->cwn_Command1[0] = NOOP;
			cwn->cwn_Command1[1] = 0;
			cwn->cwn_InstructionNum = 4;
			inpalarea = TRUE;
		}
	}
	return(err);
}

void FreeCopperWait(struct CopperWaitNode *cwn)
{
	if(cwn)
	{
		Remove((struct Node *)&cwn->cwn_Node);
		MYFREEVEC(cwn);
	}
}

UWORD AllocCopperList(struct GameSlice *slice, struct Display *dp,
											struct DisplayNode *dn)
{
	UWORD err = LAMERR_CHIPMEM;

	if( dn->dn_CopperList1 = MYALLOCVEC(10000,MEMF_CHIP,"AllocCopperList() CopperList1") )
	{
		if(dp->dp_DisplayID & LACE_KEY)
		{
			if( dn->dn_CopperList2 = MYALLOCVEC(10000,MEMF_CHIP,"AllocCopperList() CopperList2") )
				err = LAMERR_ALLOK;
		}
		else
			err = LAMERR_ALLOK;
	}
	if(err == LAMERR_CHIPMEM)
	{
		sprintf(errorstring, "Error Code: %ld\nDescription: Not Enough Chip Memory\n", err);
		DisplayError(errorstring);
	}
	return(err);
}

void DeallocCopperList( struct DisplayNode *dn)
{
	if(dn->dn_CopperList1)
		MYFREEVEC(dn->dn_CopperList1);
	if(dn->dn_CopperList2)
		MYFREEVEC(dn->dn_CopperList2);
}

static void InsertCopperWait(struct MinList *list, struct CopperWaitNode *cwn)
{
	struct CopperWaitNode *pcwn;

	for(pcwn = (struct CopperWaitNode *)list->mlh_TailPred;
			pcwn->cwn_Node.mln_Pred;
			pcwn = (struct CopperWaitNode *)pcwn->cwn_Node.mln_Pred)
	{
		if(pcwn->cwn_Line <= cwn->cwn_Line)
		{
			Insert((struct List *)list, (struct Node *)&cwn->cwn_Node, (struct Node *)&pcwn->cwn_Node);
			return;
		}
	}
	AddHead((struct List *)list, (struct Node *)&cwn->cwn_Node);
}

UWORD AllocInterruptCopperWait(struct LameReq *lr)
{
	UWORD err = LAMERR_ALLOK;
	struct CopperWaitNode *cwn;

	if(cwn = AllocCopperWait(displaytop+displayheight+1, INTREQ, INTF_SETCLR|INTF_COPER))
		lr->lr_InterruptCopperWait = cwn;
	else
	{
		err = LAMERR_MEM;
		sprintf(errorstring, "Error Code: %ld\nDescription: Not Enough Memory\n", err);
		DisplayError(errorstring);
		lr->lr_InterruptCopperWait = NULL;
	}
	return(err);
}

void FreeExtraCopperWaits(void)
{
	while(!IsListEmpty((struct List *)&copperwaitlist))
	{
		FreeCopperWait((struct CopperWaitNode *)copperwaitlist.mlh_Head);
	}
}

/***********************   CookShieldBarImage()   *******************/

UWORD CookShieldBarImage(struct LameReq *lr)
{
	struct ChunkNode						*cn;
	struct Display							*disp							= NULL;
	struct PaulsPictureImage		*ppi							= NULL;
	struct ILBMNode							*ilbmnode					= NULL;
	UWORD												 statdepth;
	UWORD 											 err							= LAMERR_ALLOK;
	UWORD												 num;
	UBYTE												 namebuffer[BOBFULLNAMESIZE];

	for(num = 0; num < 17; num++)
		lr->lr_ShieldBarImagePtrs[num] = NULL;

	if(cn = FindChunkByType(ID_DISP))
	{
		/* Found a DISP chunk */
		D(bug("--- Cook Shield Bar ---\n"));

		disp = (struct Display *)cn->cn_Data;

		if(!NullName(disp->dp_StatBackDropPicture))
		{
			if( ilbmnode = (struct ILBMNode *)FindCompressedName(&ilbmlist, disp->dp_StatBackDropPicture, NULL) )
			{
				ppi = &ilbmnode->ilbm_PaulsPictureImage;
				statdepth = ppi->ppi_Depth;

				if(!NullName(disp->dp_StatShieldBarImage))
				{
					if( ilbmnode = (struct ILBMNode *)FindCompressedName(&ilbmlist, disp->dp_StatShieldBarImage, NULL) )
					{
						ppi = &ilbmnode->ilbm_PaulsPictureImage;
						if(ppi->ppi_Depth == statdepth)
						{
							err = ChopUpBarImages(lr, ppi);
						}
						else
						{
							err = LAMERR_SHIELD_BAR_WRONG_DEPTH;
							sprintf(errorstring, "Error Code: %ld\nDescription: Shield Bar Depth Different To Stat Box Depth\n", err);
							DisplayError(errorstring);
						}
					}
				}
			}
			else
			{
				err = LAMERR_IMAGENOTFOUND;
				UnpackASCII(disp->dp_StatBackDropPicture, namebuffer, BOBFULLNAMESIZE-1);
				sprintf(errorstring, "Error Code: %ld\nDescription: Picture '%s' Not Found\nFor Stat Box\n", err, namebuffer);
				DisplayError(errorstring);
			}
		}
	}
	return(err);
}

/**********************   ChopUpBarImages()   **********************/
//
//  Slice and Dice the ILBM in 17 word images.

static UWORD ChopUpBarImages(struct LameReq *lr, struct PaulsPictureImage *ppi)
{
	UWORD err = LAMERR_ALLOK;
	UWORD x, line, wordwidth;
	UWORD *srcimageptr,*srcimageptrbase,*dstimageptr;

	if(shieldbarimages = MYALLOCVEC((2*ppi->ppi_Height*ppi->ppi_Depth)*17, MEMF_ANY|MEMF_CLEAR, "Shield Bar Images"))
	{
		lr->lr_ShieldBarHeight = ppi->ppi_Height;
		wordwidth = (((ppi->ppi_Width>>4)+3)&0xFFFC);
		if(wordwidth >= 17)
		{
			srcimageptrbase = (UWORD *)ppi->ppi_Image;
			dstimageptr = shieldbarimages;

			for(x = 0; x < 17; x++)
			{
				srcimageptr = srcimageptrbase++;
				lr->lr_ShieldBarImagePtrs[x] = dstimageptr;
				for(line = 0; line < ppi->ppi_Height*ppi->ppi_Depth; line++)
				{
					*dstimageptr++ = *srcimageptr;
					srcimageptr += wordwidth;
				}
			}
		}
		else
		{
			err = LAMERR_SHIELD_IMAGE_TOO_SHORT;
			sprintf(errorstring, "Error Code: %ld\nDescription: Shield Images Too Short\nShould be 17 images accross", err);
			DisplayError(errorstring);
		}
	}
	else
	{
		/* Couldn't Allocate DMAOffCopperWait Structure */
		err = LAMERR_MEM;
		sprintf(errorstring, "Error Code: %ld\nDescription: Not Enough Memory\n", err);
		DisplayError(errorstring);
	}
	return(err);
}

void FreeCookedShieldBarImage(void)
{
	if(shieldbarimages)
	{
		MYFREEVEC(shieldbarimages);
		shieldbarimages = NULL;
	}
}
