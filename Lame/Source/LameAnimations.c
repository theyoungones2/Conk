#include <stdarg.h>
#include <clib/alib_stdio_protos.h>
#include <exec/exec.h>
#include <dos/dos.h>
#include <graphics/modeid.h>
#include <graphics/display.h>
#include <graphics/gfxbase.h>
#include <hardware/dmabits.h>
#include <hardware/intbits.h>
#include <hardware/custom.h>
#include <proto/exec.h>
#include <proto/dos.h>
#include <lame.h>
#include <game.h>
#include <mycustom.h>
#include <debug.h>
#include <protos.h>
#include <RawKeyCodes.h>

extern struct Custom custom;

static UWORD SetupCopperList(struct AnimInfoNode *ain);
static void DoAnimFrame(UBYTE *bitmap, struct FrameNode *frn, struct AnimInfoNode *ain);
static void InsertEvent(struct List *list, struct EventNode *en);
static UWORD InitSFXEvent(struct AnimInfoNode *ain, struct EventNode *ev);
static UWORD InitColourEvent(struct AnimInfoNode *ain, struct EventNode *ev);
static UWORD InitCyclesEvent(struct AnimInfoNode *ain, struct EventNode *ev);
static UWORD InitFrameRateEvent(struct AnimInfoNode *ain, struct EventNode *ev);

UWORD LoadAnimFile(BPTR fh, char *name)
{
	struct FileNode *fn;
	struct AnimInfoNode *ain;
	struct FrameNode *frn;
	ULONG buf[3],formsize,formcount;
	UWORD err = LAMERR_ALLOK;

	if( fn = MYALLOCVEC(sizeof(struct FileNode), MEMF_ANY|MEMF_CLEAR, "ANIM FileNode") )
	{
		NewList( &fn->fn_ChunkList );
		fn->fn_Node.ln_Name = name;
		fn->fn_Form = ID_ANIM;
		fn->fn_UsedInLevel = TRUE;
		fn->fn_Complete = TRUE;
		AddTail( &filelist, (struct Node *)fn );
		if( ain = MYALLOCVEC(sizeof(struct AnimInfoNode), MEMF_ANY|MEMF_CLEAR, "AnimInfoNode") )
		{
			fn->fn_Data = ain;
			NewList((struct List *)&ain->ain_FrameList);

			/* Read ILBM Header First */
			if( (Read(fh, buf, 12) == 12) && (err < LAME_FAILAT) )
			{
				if( (buf[0] == ID_FORM) && (buf[2] == ID_ILBM) )
				{
					formsize = buf[1];
					formcount = 4;						/* Read 4 bytes for the ID_ILBM */

					while( (formcount < formsize) && (err < LAME_FAILAT) )
					{
						if(Read( fh, buf, 8 ) == 8)
						{
							formcount += 8;
							if( buf[1] & 1 )
								buf[1]++;
							formcount += buf[1];

							switch( buf[0] )
							{
								case ID_BMHD:
									if( !(ain->ain_BMHD = ReadAnimFileChunk(fh, buf[1], name)) )
										err = LAME_FAILAT;
									break;
								case ID_CMAP:
									if( !(ain->ain_CMAP = ReadAnimFileChunk(fh, buf[1], name)) )
										err = LAME_FAILAT;
									break;
								case ID_BODY:
									if( !(ain->ain_BODY = ReadAnimFileChunk(fh, buf[1], name)) )
										err = LAME_FAILAT;
									break;
								default:
									Seek( fh, buf[1], OFFSET_CURRENT );
									break;
							}
						}
						else
						{
							err = LAMERR_READ;
							sprintf(errorstring, "Error Code: %ld\nFile: %s\nDescription: Read Error\n", err, name);
							DisplayError(errorstring);
						}
					}
				}
				else
				{
					err = LAMERR_MALFORMED_ANIMATION_FILE;
					sprintf(errorstring, "Error Code: %ld\nFile: %s\nDescription: Malformed Animation File\n", err, name);
					DisplayError(errorstring);
				}
			}
			while( (Read(fh, buf, 12) == 12) && (err < LAME_FAILAT) )
			{
				if( (buf[0] == ID_FORM) && (buf[2] == ID_ILBM) )
				{
					formsize = buf[1];
					formcount = 4;						/* Read 4 bytes for the ID_ILBM */
					if(frn = MYALLOCVEC(sizeof(struct FrameNode),MEMF_ANY|MEMF_CLEAR, "AnimFrameNode"))
					{
						AddTail((struct List *)&ain->ain_FrameList, (struct Node *)&frn->frn_Node);
						while( (formcount < formsize) && (err < LAME_FAILAT) )
						{
							if(Read( fh, buf, 8 ) == 8)
							{
								formcount += 8;
								if( buf[1] & 1 )
									buf[1]++;
								formcount += buf[1];
	
								switch( buf[0] )
								{
									case ID_ANHD:
										if( !(frn->frn_ANHD = ReadAnimFileChunk(fh, buf[1], name)) )
											err = LAME_FAILAT;
										break;
									case ID_DLTA:
										if( !(frn->frn_DLTA = ReadAnimFileChunk(fh, buf[1], name)) )
											err = LAME_FAILAT;
										break;
									default:
										Seek( fh, buf[1], OFFSET_CURRENT );
										break;
								}
							}
							else
							{
								err = LAMERR_READ;
								sprintf(errorstring, "Error Code: %ld\nFile: %s\nDescription: Read Error\n", err, name);
								DisplayError(errorstring);
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
				else
				{
					err = LAMERR_MALFORMED_ANIMATION_FILE;
					sprintf(errorstring, "Error Code: %ld\nFile: %s\nDescription: Malformed Animation File\n", err, name);
					DisplayError(errorstring);
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
	else
	{
		err = LAMERR_MEM;
		sprintf(errorstring, "Error Code: %ld\nDescription: Not Enough Memory\n", err);
		DisplayError(errorstring);
	}
	return(err);
}

void FreeAnimation(struct AnimInfoNode *ain)
{
	struct FrameNode *frn;

	if(ain)
	{
		if(ain->ain_BMHD)
		{
			MYFREEVEC(ain->ain_BMHD);
			ain->ain_BMHD = NULL;
		}
		if(ain->ain_CMAP)
		{
			MYFREEVEC(ain->ain_CMAP);
			ain->ain_CMAP = NULL;
		}
		if(ain->ain_BODY)
		{
			MYFREEVEC(ain->ain_BODY);
			ain->ain_BODY = NULL;
		}
		while(!IsListEmpty((struct List *)&ain->ain_FrameList))
		{
			frn = (struct FrameNode *)ain->ain_FrameList.mlh_Head;
			Remove((struct Node *)&frn->frn_Node);
			if(frn->frn_ANHD) MYFREEVEC(frn->frn_ANHD);
			if(frn->frn_DLTA) MYFREEVEC(frn->frn_DLTA);
			MYFREEVEC(frn);
		}
		MYFREEVEC(ain);
	}
}

UWORD CookAnimations( struct LameReq *lr )
{
	struct FileNode *fn;
	struct AnimInfoNode *ain;
	UWORD err = LAMERR_ALLOK;
	ULONG														 imagesize;
	struct BitMapHeader							*bmhd;

	for( fn = (struct FileNode *)filelist.lh_Head;
			 fn->fn_Node.ln_Succ && err < LAME_FAILAT;
			 fn = (struct FileNode *)fn->fn_Node.ln_Succ )
	{
		if( (fn->fn_UsedInLevel) && (fn->fn_Form == ID_ANIM) )
		{
			D(bug("--- Cook Animations ---\n"));
			if(ain = fn->fn_Data)
			{
				if((bmhd = ain->ain_BMHD) && ain->ain_CMAP && ain->ain_BODY)
				{
					ain->ain_Width = bmhd->bmh_Width;
					ain->ain_Height = bmhd->bmh_Height;
					ain->ain_Depth = bmhd->bmh_Depth;
					ain->ain_BMByteWidth = ((ain->ain_Width>>3)+7)&0xFFF8;
					ain->ain_BMBytesPerRow = ain->ain_BMByteWidth * ain->ain_Depth;
					D(bug("Alligned Byte Width: %ld\n",ain->ain_BMByteWidth));

					imagesize = (ain->ain_BMByteWidth * ain->ain_Height * ain->ain_Depth) + 8;
					if( (ain->ain_RealBitMap1 = MYALLOCVEC(imagesize, MEMF_CHIP|MEMF_CLEAR,"AnimBitMap1")) &&
							(ain->ain_RealBitMap2 = MYALLOCVEC(imagesize, MEMF_CHIP|MEMF_CLEAR,"AnimBitMap2")) )
					{
						ain->ain_BitMap1 = (UBYTE *)((LONG)(ain->ain_RealBitMap1+7)&0xFFFFFFF8);
						ain->ain_BitMap2 = (UBYTE *)((LONG)(ain->ain_RealBitMap2+7)&0xFFFFFFF8);

						if( (err = UnpackILBM(bmhd, ain->ain_BODY, ain->ain_BitMap1, ain->ain_BMByteWidth)) < LAME_FAILAT)
						{
							/* 'imagesize-8' is a fix for a mungwall hit. Beacuse the bitmap doesn't have to start */
							/* exactly at the beginning of the allocated memory, we have to take that into account */
							/* - Ben	 */
							CopyBitMap((ULONG *)ain->ain_BitMap1, (ULONG *)ain->ain_BitMap2, imagesize-8);
						}
					}
					else
					{
						err = LAMERR_CHIPMEM;
						sprintf(errorstring, "Error Code: %ld\nDescription: Not Chip Memory\n", err);
						DisplayError(errorstring);
					}
				}
				else
				{
					err = LAMERR_ILBM_MISSING_CHUNK;
					sprintf(errorstring, "Error Code: %ld\nFile: %s\nDescription: ANIM File Missing Chunk\n", err, fn->fn_Node.ln_Name);
					DisplayError(errorstring);
				}
			}
		}
	}
	return(err);
}

void FreeCookedAnimations(void)
{
	struct FileNode *fn;
	struct AnimInfoNode *ain;

	for(fn = (struct FileNode *)filelist.lh_Head;
		  fn->fn_Node.ln_Succ;
			fn = (struct FileNode *)fn->fn_Node.ln_Succ )
	{
		if( (fn->fn_Form == ID_ANIM) && (ain = fn->fn_Data) )
		{
			if(ain->ain_RealBitMap1)
			{
				MYFREEVEC(ain->ain_RealBitMap1);
				ain->ain_RealBitMap1 = NULL;
			}
			if(ain->ain_RealBitMap2)
			{
				MYFREEVEC(ain->ain_RealBitMap2);
				ain->ain_RealBitMap2 = NULL;
			}
		}
	}
}


UWORD DisplayAnimation(struct LevelNode *ln)
{
	struct FileNode *fn;
	struct AnimInfoNode *ain = NULL;
	UWORD err = LAMERR_ALLOK;
	UBYTE namebuffer[LEVELFULLNAMESIZE];
	struct FrameNode *frn;
	UBYTE *modulestart = NULL, *moduleend = NULL;
	ULONG *memptr;
	BOOL firstcycle = TRUE;
	UWORD *srcptr,*dstptr;
	UWORD done = 0;		/* 1=NextLevel, 2=NoNextLevel */

	D(bug("DisplayAnimtion()\n"));

	for(fn = (struct FileNode *)filelist.lh_Head;
		  fn->fn_Node.ln_Succ && !ain;
			fn = (struct FileNode *)fn->fn_Node.ln_Succ )
	{
		if( fn->fn_UsedInLevel && (fn->fn_Form == ID_ANIM) )
		{
			ain = fn->fn_Data;
		}
	}

	if(ain)
	{
		ain->ain_MainLoopDone = FALSE;
		ain->ain_FrameRate = 3;
		ain->ain_FrameRateCount = 0;
		ain->ain_ScreenToggle = 0;
		ain->ain_Frame = 0;
		CopperInterruptStruct.is_Node.ln_Type = NT_INTERRUPT;
		CopperInterruptStruct.is_Node.ln_Pri = 127;
		CopperInterruptStruct.is_Node.ln_Name = "AnimCopperInt";
		CopperInterruptStruct.is_Data = ain;
		CopperInterruptStruct.is_Code = AnimCopperIntHandler;
		custom.intena = INTF_COPER;
		ain->ain_OldIntVector = SetIntVector(INTB_COPER,&CopperInterruptStruct);
		custom.intena = INTF_SETCLR|INTF_COPER;

		ain->ain_Cycles = (UWORD)~0;

		if(TRUE) /* StartPlayerSubTask()) */
		{
			ModulePlayerEnable = 0;

			for(fn = (struct FileNode *)filelist.lh_Head;
					(fn->fn_Node.ln_Succ) && (err < LAME_FAILAT) && (!modulestart);
					fn = (struct FileNode *)fn->fn_Node.ln_Succ )
			{
				if(fn->fn_UsedInLevel && (fn->fn_Form == 0) && (fn->fn_FileType == FT_MODULE))
				{
					modulestart = fn->fn_Data;
					memptr = fn->fn_Data;
					memptr--;
					moduleend = modulestart + *memptr;
/*				InitCIAInterrupt(); */
				}
			}

			if(!modulestart || (modulestart && CIAABase))
			{
				if(modulestart)
					InitModulePlayer(modulestart, moduleend);

				if((err = SetupCopperList(ain)) < LAME_FAILAT)
				{
					KillSprites();
					SetBitplanePointers(ain, ain->ain_BitMap1);
					custom.cop1lc = (ULONG)ain->ain_CopperList;
		
					/* Screen 1 Starts out in front */
		
					if(modulestart)
						ModulePlayerEnable = 1;

					while(ain->ain_Cycles && !done)
					{
						ain->ain_Frame = 0;
						ain->ain_MainLoopDone = FALSE;
						ain->ain_NextEvent = (struct EventNode *)eventlist.lh_Head;
						frn = (struct FrameNode *)ain->ain_FrameList.mlh_Head;
						if(!firstcycle)
						{
							if(frn->frn_Node.mln_Succ)
								frn = (struct FrameNode *)frn->frn_Node.mln_Succ;
							ain->ain_Frame++;
						}
						firstcycle = FALSE;
						while( (((ain->ain_Cycles > 1) && frn->frn_Node.mln_Succ) ||
									  ((ain->ain_Cycles <= 1) && frn->frn_Node.mln_Succ && frn->frn_Node.mln_Succ->mln_Succ)) &&
										!done)
						{
							DoAnimFrame(ain->ain_ScreenToggle ? ain->ain_BitMap1 : ain->ain_BitMap2, frn, ain);
							ain->ain_MainLoopDone = TRUE;
							Wait(SIGBREAKF_CTRL_C);
							ain->ain_MainLoopDone = FALSE;
							SetSignal(0,SIGBREAKF_CTRL_C);
							//ain->ain_NextEvent = DoEvents(&eventlist, ain, ain->ain_NextEvent, ain->ain_Frame);
							//SetBitplanePointers(ain, ain->ain_ScreenToggle ? ain->ain_BitMap1 : ain->ain_BitMap2);
							ain->ain_ScreenToggle = ~ain->ain_ScreenToggle;
							frn = (struct FrameNode *)frn->frn_Node.mln_Succ;
							ain->ain_Frame++;
							if(JoystickDecode(1) & 0x0010)
								done = 1;
							if(RawKeyArray[KEY_ESC])
								done = 2;
						}
						if(ain->ain_Cycles != ~0)
							ain->ain_Cycles--;
					}
				}
				if(!done)
					done = 1;

				custom.cop1lc = (ULONG)ALittleCopperList;
				MYFREEVEC(ain->ain_CopperList);
				ain->ain_CopperList = NULL;
				ain->ain_CopperBitplanePointer = NULL;
				ain->ain_CopperColoursHi = NULL;
				ain->ain_CopperColoursLo = NULL;
				if(modulestart)
					StopModulePlayer();
/*			if(CIAABase)
					StopCIAInterrupt();
 */
			}
			else
			{
				err = LAMERR_CANT_ALLOC_CIA_INTERRUPT;
				sprintf(errorstring, "Error Code: %ld\nDescription: Can't Allocate Module Player Interrupts\n", err);
				DisplayError(errorstring);
			}
/*		EndPlayerSubTask(); */
		}
		if(ain->ain_OldIntVector)
		{
			custom.intena = INTF_COPER;
			SetIntVector(INTB_COPER, ain->ain_OldIntVector);
			custom.intena = INTF_SETCLR|INTF_COPER;
			ain->ain_OldIntVector = NULL;
		}
	}
	else
	{
		err = LAMERR_ANIMATION_NOT_FOUND;
		UnpackASCII(ln->ln_Node.ln_Name, namebuffer, LEVELFULLNAMESIZE-1);
		sprintf(errorstring, "Error Code: %ld\nDescription: No Animation File Found\nFor Level '%s'\n", err, namebuffer);
		DisplayError(errorstring);
	}
	if( (done == 1) && (ln->ln_Node.ln_Succ) && (ln->ln_Node.ln_Succ->ln_Succ) )
	{
		ln = (struct LevelNode *)ln->ln_Node.ln_Succ;
		srcptr = (UWORD *)ln->ln_Node.ln_Name;
		dstptr = (UWORD *)NextLevelBuffer;
		*dstptr++ = *srcptr++;
		*dstptr++ = *srcptr++;
		*dstptr++ = *srcptr++;
		*dstptr++ = *srcptr++;
		*dstptr++ = *srcptr++;
	}
	return(err);
}

static UWORD SetupCopperList(struct AnimInfoNode *ain)
{
	struct PaulsCopperHeader *pch;
	struct CopperPointer *cp;
	UWORD *cprptr;
	UWORD count,reg,colour;
	UBYTE *byteptr;
	UWORD err = LAMERR_ALLOK;
	UWORD mybplcon3,col32count,numofcolours;

	if(ain->ain_CopperList = MYALLOCVEC(10000,MEMF_CHIP,"AnimCopperList"))
	{
		pch = (struct PaulsCopperHeader *)ain->ain_CopperList;
		pch->ch_Bpl1Mod[0] = BPL1MOD;
		pch->ch_Bpl1Mod[1] = ((ain->ain_Depth-1) * ain->ain_BMByteWidth);
		pch->ch_Bpl2Mod[0] = BPL2MOD;
		pch->ch_Bpl2Mod[1] = pch->ch_Bpl1Mod[1];
		pch->ch_BplCon0[0] = BPLCON0;
		pch->ch_BplCon0[1] = (ain->ain_Depth<<12)|0x0200;
		pch->ch_BplCon1[0] = BPLCON1;
		pch->ch_BplCon1[1] = 0x0000;
		pch->ch_BplCon2[0] = BPLCON2;
		pch->ch_BplCon2[1] = 0x0224;
		pch->ch_BplCon3[0] = BPLCON3;
		pch->ch_BplCon3[1] = 0x0C40;
		pch->ch_BplCon4[0] = BPLCON4;
		pch->ch_BplCon4[1] = 0x0011;
		pch->ch_DdfStrt[0] = DDFSTRT;
		pch->ch_DdfStrt[1] = 0x0038;
		pch->ch_DdfStop[0] = DDFSTOP;
		pch->ch_DdfStop[1] = 0x00D0;
		pch->ch_DiwStrt[0] = DIWSTRT;
		pch->ch_DiwStrt[1] = 0x2C81;
		pch->ch_DiwStop[0] = DIWSTOP;
		pch->ch_DiwStop[1] = 0x2CC1;
		pch->ch_FMode[0] = FMODE;
		pch->ch_FMode[1] = 0x0000;

		cp = (struct CopperPointer *)(pch+1);
		ain->ain_CopperBitplanePointer = cp;
		reg = BPLPT;
		for(count = 0; count < ain->ain_Depth; count++)
		{
			cp->cp_HiRegister = reg;
			reg += 2;
			cp->cp_LoRegister = reg;
			reg += 2;
			cp++;
		}
		cprptr = (UWORD *)cp;
		*cprptr++ = DMACON;
		*cprptr++ = DMAF_SETCLR|DMAF_RASTER;
		ain->ain_CopperColoursHi = cprptr;

		mybplcon3 = 0x0C40;
		numofcolours = 1<<ain->ain_Depth;

		/* High Nibble Colour Stuff */
		mybplcon3 &= 0x1FFF;
		col32count = 0;
		byteptr = ain->ain_CMAP;
		for(count = 0; count < numofcolours; count++)
		{
			if(col32count == 0)
			{
				col32count = 32;
				*cprptr++ = BPLCON3;
				*cprptr++ = mybplcon3;
				mybplcon3 += 0x2000;		/* Goto Next Colour Bank */
				reg = COLOR;
			}
			colour = (byteptr[0]<<4)&0x0F00;
			colour |= (byteptr[1])&0x00F0;
			colour |= (byteptr[2]>>4)&0x000F;
			*cprptr++ = reg;
			reg += 2;
			*cprptr++ = colour;
			byteptr += 3;
			col32count--;
		}
		if(agacomputer)
		{
			/* Low Nibble Colour Stuff */
			ain->ain_CopperColoursLo = cprptr;
			mybplcon3 &= 0x1FFF;
			mybplcon3 |= 0x0200;	/* Set LOCT bit (lower nibble set) */
			col32count = 0;
			byteptr = ain->ain_CMAP;
			for(count = 0; count < numofcolours; count++)
			{
				if(col32count == 0)
				{
					col32count = 32;
					*cprptr++ = BPLCON3;
					*cprptr++ = mybplcon3;
					mybplcon3 += 0x2000;		/* Goto Next Colour Bank */
					reg = COLOR;
				}
				colour = (byteptr[0]<<8)&0x0F00;
				colour |= (byteptr[1]<<4)&0x00F0;
				colour |= (byteptr[2])&0x000F;
				*cprptr++ = reg;
				reg += 2;
				*cprptr++ = colour;
				byteptr += 3;
				col32count--;
			}
		}

		*cprptr++ = 0xFF01;
		*cprptr++ = 0xFF00;
		*cprptr++ = INTREQ;
		*cprptr++ = INTF_SETCLR|INTF_COPER;

		*cprptr++ = 0xFFFF;
		*cprptr++ = 0xFFFE;
		*cprptr++ = 0xFFFF;
		*cprptr++ = 0xFFFE;
	}
	else
	{
		err = LAMERR_CHIPMEM;
		sprintf(errorstring, "Error Code: %ld\nDescription: Not Enough Chip Memory\n", err);
		DisplayError(errorstring);
	}
	return(err);
}

void __asm SetBitplanePointers(register __a0 struct AnimInfoNode *ain,
															 register __a1 UBYTE *bitmap)
{
	struct CopperPointer *cp;
	UWORD count;

	if(cp = ain->ain_CopperBitplanePointer)
	{
		for(count = 0; count < ain->ain_Depth; count++)
		{
			cp->cp_HiPointer = (UWORD)(((ULONG)bitmap)>>16);
			cp->cp_LoPointer = (UWORD)(((ULONG)bitmap)&0xFFFF);
			bitmap += ain->ain_BMByteWidth;
			cp++;
		}
	}
}

static void SetCopperColour(struct AnimInfoNode *ain, UWORD colourreg, UBYTE r, UBYTE g, UBYTE b)
{
	UWORD colour, bplcon3count;
	UWORD *cprptr;

	bplcon3count = (colourreg/32)+1;
	if(cprptr = ain->ain_CopperColoursHi)
	{
		cprptr += ((bplcon3count+colourreg)*2)+1;

		colour = (r<<4)&0x0F00;
		colour |= g&0x00F0;
		colour |= (b>>4)&0x000F;
		*cprptr = colour;

		if(agacomputer && (cprptr = ain->ain_CopperColoursLo))
		{
			cprptr += ((bplcon3count+colourreg)*2)+1;
			colour = (r<<8)&0x0F00;
			colour |= (g<<4)&0x00F0;
			colour |= b&0x000F;
			*cprptr = colour;
		}
	}
}

static void DoAnimFrame(UBYTE *bitmap, struct FrameNode *frn, struct AnimInfoNode *ain)
{
	struct AnimHeader *ah;
	UBYTE *dlta;
	PLANEPTR destptr,bmptrs[8];
	UWORD	column,row,plane,bytewidth;
	UBYTE		*dltaptrs[8],*dltaptr;
	UBYTE		value,count;

	ah = frn->frn_ANHD;
	dlta = frn->frn_DLTA;
	if( (ah->ah_Operation == 5) && ((ah->ah_Interleave == 0) || (ah->ah_Interleave == 0)) )
	{
		bytewidth = (((ain->ain_Width+15)/8) & 0xFFFE);
		CopyMem(dlta,dltaptrs,ain->ain_Depth*4);
		destptr = bitmap;
		for(plane = 0; plane < ain->ain_Depth; plane++)
		{
			if(dltaptrs[plane])
				dltaptrs[plane] += (ULONG)dlta;
			bmptrs[plane] = destptr;	
			destptr += ain->ain_BMByteWidth;
		}
		for(plane = 0; plane < ain->ain_Depth; plane++)
		{
			dltaptr = dltaptrs[plane];
			if(dltaptr)
			{
				for(column = 0; column < bytewidth; column++)
				{
					destptr = bmptrs[plane] + column;
					value = *dltaptr++;				/* OpCount */
					if(value)
					{
						for(row = 0; row < ain->ain_Height; )
						{
							value = *dltaptr++;
							if(value == 0)				/* Repeat */
							{
								count = *dltaptr++;
								value = *dltaptr++;
								row += count;
								for(; count; count--)
								{
									*destptr = value;
									destptr += ain->ain_BMBytesPerRow;
								}
							}
							else
							{
								if(value & 0x80)		/* Dump */
								{
									value &= 0x7F;
									row += value;
									while(value)
									{
										*destptr = *dltaptr++;
										destptr += ain->ain_BMBytesPerRow;
										value--;
									}
								}
								else								/* Skip */
								{
									destptr += ((UWORD)value)*ain->ain_BMBytesPerRow;
									row += value;
								}
							}
						}
					}
				}
			}
		}
	}
	else
	{
		D(bug("Anim Operation %ld, Interleave %ld\n", ah->ah_Operation, ah->ah_Interleave));
	}
}


UWORD CookEvents( struct LameReq *lr )
{
	struct FileNode *fn;
	struct ChunkNode *cn;
	struct EventNode *ev;
	UWORD numofevents,eventnum;
	struct SFXNode *sfx;
	struct SFXEvent *se;
	struct ColourEvent *ce;
	UBYTE *p;
	UWORD err = LAMERR_ALLOK;
	UBYTE namebuffer[SFXFULLNAMESIZE];

	for(fn = (struct FileNode *)filelist.lh_Head;
			fn->fn_Node.ln_Succ && err < LAME_FAILAT;
			fn = (struct FileNode *)fn->fn_Node.ln_Succ )
	{
		if( fn->fn_UsedInLevel )
		{
			for(cn = (struct ChunkNode *)fn->fn_ChunkList.lh_Head;
					cn->cn_Node.ln_Succ && err < LAME_FAILAT;
					cn = (struct ChunkNode *)cn->cn_Node.ln_Succ )
			{
				if( cn->cn_TypeID == ID_EVNT )
				{
					D(bug("---  Cook EVNT --- %s\n"));
					p = cn->cn_Data;
					numofevents = *(UWORD *)p; p += 2;
					for(eventnum = 0; eventnum < numofevents; eventnum++)
					{
						if(ev = MYALLOCVEC(sizeof(struct EventNode), MEMF_ANY|MEMF_CLEAR, "Event Node"))
						{
							ev->ev_Frame = *(UWORD *)p; p += 2;
							ev->ev_Type = *(UWORD *)p; p += 2;
							InsertEvent(&eventlist, ev);
							switch(ev->ev_Type)
							{
								case EVT_SFX:
									ev->ev_InitRoutine = InitSFXEvent;
									if(se = MYALLOCVEC(sizeof(struct SFXEvent), MEMF_ANY|MEMF_CLEAR, "SFXEvent"))
									{
										ev->ev_EventData = se;
										se->se_Voice = *(UWORD *)p; p+=2;
										if(!NullName(p))
										{
											if( sfx = (struct SFXNode *)FindCompressedName(&sfxlist, p, NULL) )
												se->se_SpamParam = sfx->sfx_SpamParam;
											else
											{
												err = LAMERR_SFX_NOT_FOUND;
												UnpackASCII(stsrcptr, namebuffer, SFXFULLNAMESIZE-1);
												sprintf(errorstring, "Error Code: %ld\nDescription: Sound Effect '%s' For Event #%ld\n", err, namebuffer, eventnum);
												DisplayError(errorstring);
											}
										}
										else
										{
											se->se_SpamParam = NULL;
											err = LAMERR_EMPTY_NAME;
											sprintf(errorstring, "Warning %ld: No Sound Effect Given For Event #%ld\n", err, eventnum);
											DisplayWarning(errorstring);
										}
										p += SFXFILENAMESIZE;
									}
									else
										p += 2+SFXFILENAMESIZE;
									break;
								case EVT_COLOUR:
									ev->ev_InitRoutine = InitColourEvent;
									if(ce = MYALLOCVEC(sizeof(struct ColourEvent), MEMF_ANY|MEMF_CLEAR, "ColourEvent"))
									{
										ev->ev_EventData = ce;
										ce->ce_ColourReg = *p++;
										ce->ce_Red = *p++;
										ce->ce_Green = *p++;
										ce->ce_Blue = *p++;
									}
									break;
								case EVT_CYCLES:
									ev->ev_InitRoutine = InitCyclesEvent;
									ev->ev_EventData = (void *)(*(UWORD *)p); p += 2;
									break;
								case EVT_FRAMERATE:
									ev->ev_InitRoutine = InitFrameRateEvent;
									ev->ev_EventData = (void *)(*(UWORD *)p); p += 2;
									break;
							}
						}
						else
						{
							err = LAMERR_MEM;
							sprintf(errorstring, "Error Code: %ld\nDescription: Not Enough Memory\n", err);
							DisplayError(errorstring);
						}
					}
				}
			}
		}
	}
	return(err);
}

void FreeCookedEvents(void)
{
	struct EventNode *ev;

	while(!IsListEmpty(&eventlist))
	{
		ev = (struct EventNode *)eventlist.lh_Head;
		Remove(&ev->ev_Node);
		if(ev->ev_EventData)
		{
			switch(ev->ev_Type)
			{
				case EVT_SFX:
					if(ev->ev_EventData)
					{
						MYFREEVEC(ev->ev_EventData);
						ev->ev_EventData = NULL;
					}
					break;
				case EVT_COLOUR:
					if(ev->ev_EventData)
					{
						MYFREEVEC(ev->ev_EventData);
						ev->ev_EventData = NULL;
					}
					break;
				case EVT_CYCLES:
					break;
				case EVT_FRAMERATE:
					break;
			}
		}
		MYFREEVEC(ev);
	}
}

struct EventNode * __asm DoEvents(register __a0 struct List *list,
																	register __a1 struct AnimInfoNode *ain,
																	register __a2 struct EventNode *ev,
																	register __d0 UWORD frame)
{
	BOOL done = FALSE;

	while(ev->ev_Node.ln_Succ && !done)
	{
		if(ev->ev_Frame <= frame)
		{
			(*ev->ev_InitRoutine)(ain, ev);
			ev = (struct EventNode *)ev->ev_Node.ln_Succ;
		}
		else
			done = TRUE;
	}
	return(ev);
}

static void InsertEvent(struct List *list, struct EventNode *en)
{
	struct EventNode *ev;

	for(ev = (struct EventNode *)list->lh_TailPred;
			ev->ev_Node.ln_Pred;
			ev = (struct EventNode *)ev->ev_Node.ln_Pred)
	{
		if(ev->ev_Frame <= en->ev_Frame)
		{
			Insert(list, &en->ev_Node, &ev->ev_Node);
			return;
		}
	}
	AddHead(list, &en->ev_Node);
}

static UWORD InitSFXEvent(struct AnimInfoNode *ain, struct EventNode *ev)
{
	UWORD err = LAMERR_ALLOK;
	struct SpamParam *spam;
	struct SFXEvent *se;

	if(se = ev->ev_EventData)
	{
		if(spam = se->se_SpamParam)
			PlaySample(spam, (UBYTE *)spam+1, se->se_Voice);
	}
	return(err);
}

static UWORD InitColourEvent(struct AnimInfoNode *ain, struct EventNode *ev)
{
	UWORD err = LAMERR_ALLOK;
	struct ColourEvent *ce;

	if(ce = ev->ev_EventData)
		SetCopperColour(ain, ce->ce_ColourReg, ce->ce_Red, ce->ce_Green, ce->ce_Blue);

	return(err);
}

static UWORD InitCyclesEvent(struct AnimInfoNode *ain, struct EventNode *ev)
{
	UWORD err = LAMERR_ALLOK;

	ain->ain_Cycles = (ULONG)ev->ev_EventData;

	return(err);
}

static UWORD InitFrameRateEvent(struct AnimInfoNode *ain, struct EventNode *ev)
{
	UWORD err = LAMERR_ALLOK;

	ain->ain_FrameRate = (ULONG)ev->ev_EventData;

	return(err);
}
