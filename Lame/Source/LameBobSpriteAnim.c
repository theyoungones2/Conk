#include <stdarg.h>
#include <clib/alib_stdio_protos.h>
#include <exec/exec.h>
#include <dos/dos.h>
#include <graphics/modeid.h>
#include <graphics/display.h>
#include <graphics/gfxbase.h>
#include <hardware/intbits.h>
#include <proto/exec.h>
#include <proto/dos.h>
#include <lame.h>
#include <mycustom.h>
#include <debug.h>
#include <protos.h>

static UWORD CookSpriteImage(struct SpriteFileHeader *sfh, UWORD spritebytewidth, BOOL attached,
														 struct PaulsSpriteImage *sp);

/*****************   CookBobs()   *****************************/
//
// Goes through all the loaded BOBS chunks and makes up named bob
// image structures and adds them into the bob list.
// Needs access to display information (currently in DISP chunk)
// because we set up some display-dependent blitterstuff.

UWORD CookBobs( struct LameReq *lr )
{
	struct FileNode *fn;
	struct ChunkNode *cn;
	struct BobBankFileHeader *bankheader = NULL;
	struct BobHeader *bf;
	struct BobNode *bn;
	struct Display *disp;
	UWORD err = LAMERR_ALLOK;
	UWORD bufwidth, imwidth, i;
	UBYTE *p;
	BOOL	firstbob;
	char	namebuffer[BOBFULLNAMESIZE];

	/* look for display information */
	if( !( cn = FindChunkByType( ID_DISP ) ) )
	{
		err = LAMERR_NODISPLAY;
		sprintf(errorstring, "Error Code: %ld\nDescription: No Display Chunk Found\n", err);
		DisplayError(errorstring);
		return(err);
	}

	disp = (struct Display *)cn->cn_Data;
	if( disp->dp_DisplayID & HIRES_KEY )
		bufwidth = 88;
	else
		bufwidth = 48;

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
				if( cn->cn_TypeID == ID_BOBS )
				{
					/* Found a BOBS chunk */

					D(bug("---  Cook BOBS --- %s\n",cn->cn_Node.ln_Name));
					bankheader = cn->cn_Data;

					D2(bug("  bufwidth = %ld\n", bufwidth ));
					D2(bug("  BobBank: %s\n",bankheader->bf_BankName ));
					D2(bug("  numofbobs: %ld\n",bankheader->bf_NumberOfBobs ));
					D2(bug("  depth: %ld\n",bankheader->bf_Depth ));

					firstbob = TRUE;
					p = (UBYTE *)bankheader + sizeof( struct BobBankFileHeader );
					for( i=0; i < bankheader->bf_NumberOfBobs && err < LAME_FAILAT; i++ )
					{
						bf = (struct BobHeader *)p;

						/* calc image width in bytes, rounded up to nearest word */
						imwidth = ( ( bf->bf_Width+15 ) >> 3) & 0xFFFE;


						if( bn = MYALLOCVEC(sizeof(struct BobNode), MEMF_ANY|MEMF_CLEAR,"BobNode") )
						{
							/* add the bobnode to the boblist */
							AddTail( &boblist, &bn->bn_Node );

							if( firstbob )
							{
								/* say where this bank is in the global boblist */
								firstbob=FALSE;
								cn->cn_FirstItem = bn;
							}

							/* set bob name */
							bn->bn_Node.ln_Name = bf->bf_Name;
							D2(UnpackASCII(bn->bn_Node.ln_Name, namebuffer, BOBFULLNAMESIZE-1));
							D2(bug("%s",namebuffer));

							bn->bn_ImageHeader.im_Type = IMTY_BOB;
	
							/* skip to image data and set image ptr */
							p += sizeof(struct BobHeader);
							bn->bn_ImageHeader.im_Image = (ULONG)p;
	
							/* jump to mask data, set mask ptr */
							p += imwidth * bf->bf_Height * bankheader->bf_Depth;
							bn->bn_ImageHeader.im_Mask = (ULONG)p;
	
							/* jump to beginning of next bobheader */
							p += imwidth * bf->bf_Height * bankheader->bf_Depth;
	
							/* set up all the collisionzone data */
							bn->bn_ImageHeader.im_ColLeft = bf->bf_ColLeft << 5;
							bn->bn_ImageHeader.im_ColTop = bf->bf_ColTop << 5;
							bn->bn_ImageHeader.im_ColWidth  = ( bf->bf_ColWidth-1  ) << 5;
							bn->bn_ImageHeader.im_ColHeight = ( bf->bf_ColHeight-1 ) << 5;
							bn->bn_ImageHeader.im_ColRight  = ( bf->bf_ColLeft + bf->bf_ColWidth - 1 ) << 5;
							bn->bn_ImageHeader.im_ColBottom = ( bf->bf_ColTop + bf->bf_ColHeight - 1 ) << 5;
	
							/* Set Bob Number.  Numbers start at one (this allows for 0 terminator
								of animation lists). */
							bn->bn_ImageHeader.im_Number = i+1;
	
							/* setup Blitter D modulo */
							bn->bn_ImageHeader.im_DMod = bufwidth - 2 - imwidth;
	
							/* setup BLTSIZE word */
							bn->bn_ImageHeader.im_Size =
								( ( bf->bf_Height * bankheader->bf_Depth ) << 6 ) |
								( (imwidth+2)>>1 );
	
							bn->bn_ImageHeader.im_Height = bf->bf_Height;
							bn->bn_ImageHeader.im_Width = (bf->bf_Width + 15) & 0xFFF0;
						}
						else
						{
							err = LAMERR_MEM;
							sprintf(errorstring, "Error Code: %ld\nDescription: Not Enough Memory\n", err);
							DisplayError(errorstring);
						}
						D2(bug("\n"));
						cn->cn_Cooked = TRUE;
					}
				}
			}
		}
	}
	return(err);
}



/****************   FreeCookedBobs()   ************************/

void 	FreeCookedBobs( void )
{
	struct Node *node;
	while( !IsListEmpty( &boblist ) )
	{
		node = boblist.lh_Head;
		Remove( node );
		MYFREEVEC( node );
	}
}

/*****************   CookSprites()   *****************************/
//
// Goes through all the loaded SPRT chunks and makes up named sprite
// image structures and adds them into the sprite list.
// Needs access to display information (currently in DISP chunk)
// because we set up some display dependent sprite size stuff.

UWORD CookSprites( struct LameReq *lr )
{
	struct FileNode *fn;
	struct ChunkNode *cn;
	struct SpriteBankFileHeader *sbfh = NULL;
	struct SpriteFileHeader *sfh;
	struct SpriteNode *sn;
	struct Display *disp;
	UWORD err = LAMERR_ALLOK;
	UWORD imwidth, i, spritebytewidth;
	UBYTE *p;
	BOOL	firstsprite, attached;
	char	namebuffer[BOBFULLNAMESIZE];

	/* look for display information */
	if( !( cn = FindChunkByType( ID_DISP ) ) )
	{
		err = LAMERR_NODISPLAY;
		sprintf(errorstring, "Error Code: %ld\nDescription: No Display Chunk Found\n", err);
		DisplayError(errorstring);
		return(err);
	}

	disp = (struct Display *)cn->cn_Data;

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
				if( cn->cn_TypeID == ID_SPRT )
				{
					/* Found a SPRT chunk */

					D(bug("---  Cook SPRT --- %s\n",cn->cn_Node.ln_Name));
					sbfh = cn->cn_Data;

					D2(bug("  Sprite Bank: %s\n", sbfh->sbfh_BankName ));
					D2(bug("  Num Of Sprites: %ld\n", sbfh->sbfh_NumberOfSprites ));
					D2(bug("  Depth: %ld\n", sbfh->sbfh_Depth ));

					switch(sbfh->sbfh_Depth)
					{
						case 2:
							attached = FALSE;
							break;
						case 4:
							attached = TRUE;
							break;
						default:
							err = LAMERR_SPRITE_DEPTH_WRONG;
							sprintf(errorstring, "Error Code: %ld\nDescription: Sprite Depth Incorrect\n", err);
							DisplayError(errorstring);
							break;
					}
					if(err < LAME_FAILAT)
					{
						switch(disp->dp_SpriteBurstMode)
						{
							case 0:
								spritebytewidth = 2;
								break;
							case 1:
								spritebytewidth = 4;
								break;
							case 2:
								spritebytewidth = 8;
								break;
							default:
								err = LAMERR_SPRITE_BURST_WRONG;
								sprintf(errorstring, "Error Code: %ld\nDescription: Sprite Burst Mode Incorrect. Number %ld\n", err, disp->dp_SpriteBurstMode);
								DisplayError(errorstring);
								break;
						}
						if(err < LAME_FAILAT)
						{
							/******************/
							firstsprite = TRUE;
							p = (UBYTE *)sbfh + sizeof( struct SpriteBankFileHeader );
							for( i=0; (i < sbfh->sbfh_NumberOfSprites) && (err < LAME_FAILAT); i++ )
							{
								sfh = (struct SpriteFileHeader *)p;
		
								/* calc image width in bytes, rounded up to nearest word */
								imwidth = ( ( sfh->sfh_Width+15 ) >> 3) & 0xFFFE;
		
								if( sn = MYALLOCVEC(sizeof(struct SpriteNode), MEMF_ANY|MEMF_CLEAR,"SpriteNode") )
								{
									AddTail( &spritelist, &sn->sn_Node );
		
									if( firstsprite )
									{
										/* Say where this bank is in the global spritelist,
											this is used for finding the data in the CookAnim() routine */
										firstsprite = FALSE;
										cn->cn_FirstItem = sn;
									}
			
									/* set sprite name */
									sn->sn_Node.ln_Name = sfh->sfh_Name;
									D2(UnpackASCII(sn->sn_Node.ln_Name, namebuffer, BOBFULLNAMESIZE-1));
									D2(bug(" %s\n",namebuffer));

									if( (err = CookSpriteImage(sfh, spritebytewidth, attached, &sn->sn_ImageHeader)) < LAME_FAILAT )
									{
										sn->sn_ImageHeader.sp_Type = IMTY_SPRITE;
										sn->sn_ImageHeader.sp_StartSprite = sfh->sfh_SpriteNumber;
										/* set up all the collisionzone data */
										sn->sn_ImageHeader.sp_ColLeft = sfh->sfh_ColLeft << 5;
										sn->sn_ImageHeader.sp_ColTop = sfh->sfh_ColTop << 5;
										sn->sn_ImageHeader.sp_ColWidth = ( sfh->sfh_ColWidth-1 ) << 5;
										sn->sn_ImageHeader.sp_ColHeight = ( sfh->sfh_ColHeight-1 ) << 5;
										sn->sn_ImageHeader.sp_ColRight =
											( sfh->sfh_ColLeft + sfh->sfh_ColWidth - 1 ) << 5;
										sn->sn_ImageHeader.sp_ColBottom =
											( sfh->sfh_ColTop + sfh->sfh_ColHeight - 1 ) << 5;
		
										p += sizeof(struct SpriteFileHeader) + (imwidth * sfh->sfh_Height * sbfh->sbfh_Depth);
			
										/* Set Sprite Number.  Numbers start at one (this allows for 0 terminator
											of animation lists). */
										sn->sn_ImageHeader.sp_Number = i+1;
									}
								}
								else
								{
									err = LAMERR_MEM;
									sprintf(errorstring, "Error Code: %ld\nDescription: Not Enough Memory\n", err);
									DisplayError(errorstring);
								}
							}
							D2(bug("\n"));
							cn->cn_Cooked = TRUE;
						}
					}
				}
			}
		}
	}
	return(err);
}

static UWORD CookSpriteImage(struct SpriteFileHeader *sfh, UWORD spritebytewidth, BOOL attached,
														 struct PaulsSpriteImage *sp)
{
	UWORD err = LAMERR_ALLOK;
	UWORD spriteswide,totalbytewidth, spritewordwidth, srcbytemodulo;
	ULONG spritesize, totalmemsize;
	UBYTE *spritebase, *spritesrcptr, *spritedestptr, *spritedestptr2;
	UWORD	sprite,height,word;

	if( (sfh) && (sp) )
	{
		totalbytewidth = ( ( sfh->sfh_Width+15 ) >> 3) & 0xFFFE;
		spriteswide = (totalbytewidth + (spritebytewidth-1)) / spritebytewidth;
		/* The Size of the sprite is given by the (Sprite Byte Width * Height) + (1 control
			'word'(gets doubled later)) + (ending line of NULL's) all times 2 for 2 bit planes */
		spritesize = ((spritebytewidth * sfh->sfh_Height) + (spritebytewidth*2)) * 2;
		spritewordwidth = spritebytewidth>>1;
		srcbytemodulo = totalbytewidth - spritebytewidth;

		totalmemsize = spritesize * spriteswide;
		if(attached) totalmemsize *= 2;

		if(spritebase = MYALLOCVEC(totalmemsize, MEMF_CHIP|MEMF_CLEAR,"SpriteCookedImageMemory"))
		{
			sp->sp_Image = spritebase;
			sp->sp_Size = spritesize;
			sp->sp_NumberUsed = (attached) ? spriteswide*2 : spriteswide;
			sp->sp_Flags  = (attached) ? SPF_ATTACHED : 0;
			sp->sp_Height = sfh->sfh_Height;
			sp->sp_Width = spritebytewidth<<3;		//(sfh->sfh_Width + 15) & 0xFFF0;

			for(sprite = 0; sprite < spriteswide; sprite++)
			{
				spritesrcptr = (UBYTE *)(sfh + 1);
				spritesrcptr += (sprite * spritebytewidth);

				spritedestptr = spritebase;
				if(attached)
				{
					spritedestptr += (spritesize * sprite * 2);
					spritedestptr2 = spritedestptr + spritesize;
					spritedestptr += (spritebytewidth*2);
					spritedestptr2 += (spritebytewidth*2);
				}
				else
				{
					spritedestptr += (spritesize * sprite);
					spritedestptr += (spritebytewidth*2);
				}

				for(height = 0; height < sp->sp_Height; height++)
				{
					/* Plane 1 */
					for(word = 0; word < spritewordwidth; word++)
					{
						*(UWORD *)spritedestptr = *(UWORD *)spritesrcptr;
						spritedestptr += 2;
						spritesrcptr += 2;
					}
					spritesrcptr += srcbytemodulo;
					/* Plane 2 */
					for(word = 0; word < spritewordwidth; word++)
					{
						*(UWORD *)spritedestptr = *(UWORD *)spritesrcptr;
						spritedestptr += 2;
						spritesrcptr += 2;
					}
					spritesrcptr += srcbytemodulo;

					if(attached)
					{
						/* Plane 3 */
						for(word = 0; word < spritewordwidth; word++)
						{
							*(UWORD *)spritedestptr2 = *(UWORD *)spritesrcptr;
							spritedestptr2 += 2;
							spritesrcptr += 2;
						}
						spritesrcptr += srcbytemodulo;
						/* Plane 4 */
						for(word = 0; word < spritewordwidth; word++)
						{
							*(UWORD *)spritedestptr2 = *(UWORD *)spritesrcptr;
							spritedestptr2 += 2;
							spritesrcptr += 2;
						}
						spritesrcptr += srcbytemodulo;
					}
				}
			}
		}
		else
		{
			/* Couldn't Allocate BitMaps */
			err = LAMERR_CHIPMEM;
			sprintf(errorstring, "Error Code: %ld\nDescription: Not Enough Chip Memory\n", err);
			DisplayError(errorstring);
		}
	}
	return(err);
}


/****************   FreeCookedSprites()   ************************/

void 	FreeCookedSprites( void )
{
	struct SpriteNode *sn;
	while( !IsListEmpty( &spritelist ) )
	{
		sn = (struct SpriteNode *)spritelist.lh_Head;
		Remove( &sn->sn_Node );
		if(sn->sn_ImageHeader.sp_Image) MYFREEVEC(sn->sn_ImageHeader.sp_Image);
		MYFREEVEC( sn );
	}
}


/****************   CheckUniqueImageNames()   ************/
//
//  Goes through all the bobs and sprites and makes sure
// that all the names are unique.

void CheckUniqueImageNames(void)
{
	struct SpriteNode *sn;
	struct BobNode *bn;
	UBYTE	namebuffer[BOBFULLNAMESIZE];
	UWORD err = LAMERR_ALLOK;

	for(bn = (struct BobNode *)boblist.lh_Head;
			bn->bn_Node.ln_Succ && bn->bn_Node.ln_Succ->ln_Succ;
			bn = (struct BobNode *)bn->bn_Node.ln_Succ)
	{
		if(!NullName(bn->bn_Node.ln_Name))
		{
			if(FindCompressedName(&boblist, bn->bn_Node.ln_Name, bn->bn_Node.ln_Succ))
			{
				err = LAMERR_NAME_NOT_UNIQUE;
				UnpackASCII(bn->bn_Node.ln_Name, namebuffer, BOBFULLNAMESIZE-1);
				sprintf(errorstring, "Warning %ld: Bob Name '%s' Not Unique\n", err, namebuffer);
				DisplayWarning(errorstring);
			}
			if(FindCompressedName(&spritelist, bn->bn_Node.ln_Name, NULL))
			{
				err = LAMERR_NAME_NOT_UNIQUE;
				UnpackASCII(bn->bn_Node.ln_Name, namebuffer, BOBFULLNAMESIZE-1);
				sprintf(errorstring, "Warning %ld: Bob Name '%s' Not Unique\nAlso used in Sprites.\n", err, namebuffer);
				DisplayWarning(errorstring);
			}
		}
	}
	for(sn = (struct SpriteNode *)spritelist.lh_Head;
			sn->sn_Node.ln_Succ && sn->sn_Node.ln_Succ->ln_Succ;
			sn = (struct SpriteNode *)sn->sn_Node.ln_Succ)
	{
		if(!NullName(sn->sn_Node.ln_Name))
		{
			if(FindCompressedName(&spritelist, sn->sn_Node.ln_Name, sn->sn_Node.ln_Succ))
			{
				err = LAMERR_NAME_NOT_UNIQUE;
				UnpackASCII(sn->sn_Node.ln_Name, namebuffer, BOBFULLNAMESIZE-1);
				sprintf(errorstring, "Warning %ld: Sprite Name '%s' Not Unique\n", err, namebuffer);
				DisplayWarning(errorstring);
			}
		}
	}
}

/****************   CookAnims()   ************************/
//
//  This modifies the data in place (Changes the bobnumber
// to a pointer and shifts the offsets. This is restored in
// FreeCookedAnims().

UWORD CookAnims( struct LameReq *lr )
{
	struct FileNode *fn;
	struct ChunkNode *cn, *bobchunk, *spritechunk;
	struct SequenceFileHeader *sfh;
	struct AnimFileHeader *afh;
	struct FrameFileHeader *ffh;
	struct BobNode *bn;
	struct SpriteNode *sn;
	struct AnimNode *an;
	UBYTE *p;
	UWORD err = LAMERR_ALLOK;
	ULONG seq,frame,bobnum,spritenum;

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
				if( cn->cn_TypeID == ID_ANIM )
				{
					D(bug("---  Cook ANIM Chunk --- \n"));

					p = cn->cn_Data;
					sfh = (struct SequenceFileHeader *)p;
					p += sizeof( struct SequenceFileHeader );

					/* look for the bobchunk we're attached to */
					if( bobchunk = FindChunkByName( ID_BOBS, sfh->sfh_BobBankName ) )
					{
						/* go through each animsequence */
						for( seq=0; (seq < sfh->sfh_NumOfAnims) && (err < LAME_FAILAT); seq++ )
						{
							/* find the sequence header */
							afh = (struct AnimFileHeader *)p;
							p += sizeof( struct AnimFileHeader );

							/* go through each frame in the animsequence */
							for( frame=0; (frame < afh->afh_NumOfFrames) && (err < LAME_FAILAT); frame++ )
							{
								/* find the frame data */
								ffh = (struct FrameFileHeader *)p;
								p += sizeof( struct FrameFileHeader );

								ffh->ffh_XOffset = ffh->ffh_XOffset<<5;
								ffh->ffh_YOffset = ffh->ffh_YOffset<<5;

								/* starting at first bobnode, look for nth bob */
								/* (first bob is bob number 1!) */
								bobnum=0;
								for(bn = bobchunk->cn_FirstItem;
										(++bobnum != ffh->ffh_BobNumber) && (bn->bn_Node.ln_Succ);
										bn = (struct BobNode *)bn->bn_Node.ln_Succ);
								if(!bn->bn_Node.ln_Succ)
								{
									err = LAMERR_ANIM_IMAGE_NOT_FOUND;
									sprintf(errorstring, "Error Code: %ld\nDescription: Image Number %ld\nNot Found in Bank %s\n", err, ffh->ffh_BobNumber, sfh->sfh_BobBankName);
									DisplayError(errorstring);										
								}
								ffh->ffh_BobNumber = (ULONG)&bn->bn_ImageHeader;
							}
							/* skip the delimiter frame */
							p += sizeof( struct FrameFileHeader );

							/* add the anim to the animlist */

							if( an = MYALLOCVEC( sizeof(struct AnimNode), MEMF_ANY|MEMF_CLEAR,"AnimNode") )
							{
								an->an_Node.ln_Name = afh->afh_AnimName;
								an->an_AnimHeader = afh;
								AddTail( &animlist, &an->an_Node );
							}
							else
							{
								err = LAMERR_MEM;
								sprintf(errorstring, "Error Code: %ld\nDescription: Not Enough Memory\n", err);
								DisplayError(errorstring);
							}
						}
					}
					else
					{
						if( spritechunk = FindChunkByName( ID_SPRT, sfh->sfh_BobBankName ) )
						{
							/* go through each animsequence */
							for( seq = 0; (seq < sfh->sfh_NumOfAnims) && (err < LAME_FAILAT); seq++ )
							{
								/* find the sequence header */
								afh = (struct AnimFileHeader *)p;
								p += sizeof( struct AnimFileHeader );
	
								/* go through each frame in the animsequence */
								for( frame=0; (frame < afh->afh_NumOfFrames) && (err < LAME_FAILAT); frame++ )
								{
									/* find the frame data */
									ffh = (struct FrameFileHeader *)p;
									p += sizeof( struct FrameFileHeader );
	
									ffh->ffh_XOffset = ffh->ffh_XOffset<<5;
									ffh->ffh_YOffset = ffh->ffh_YOffset<<5;
	
									/* starting at first sprite node, look for nth sprite */
									/* (first sprite is sprite number 1!) */
									spritenum=0;
									for(sn = spritechunk->cn_FirstItem;
											(++spritenum != ffh->ffh_BobNumber) && (sn->sn_Node.ln_Succ);
											sn = (struct SpriteNode *)sn->sn_Node.ln_Succ);
									if(!sn->sn_Node.ln_Succ)
									{
										err = LAMERR_ANIM_IMAGE_NOT_FOUND;
										sprintf(errorstring, "Error Code: %ld\nDescription: Image Number %ld\nNot Found in Bank %s\n", err, ffh->ffh_BobNumber, sfh->sfh_BobBankName);
										DisplayError(errorstring);
									}
									ffh->ffh_BobNumber = (ULONG)&sn->sn_ImageHeader;
								}
								/* skip the delimiter frame */
								p += sizeof( struct FrameFileHeader );
	
								/* add the anim to the animlist */
	
								if( an = MYALLOCVEC( sizeof(struct AnimNode), MEMF_ANY|MEMF_CLEAR,"AnimNode") )
								{
									an->an_Node.ln_Name = afh->afh_AnimName;
									an->an_AnimHeader = afh;
									AddTail( &animlist, &an->an_Node );
								}
								else
								{
									err = LAMERR_MEM;
									sprintf(errorstring, "Error Code: %ld\nDescription: Not Enough Memory\n", err);
									DisplayError(errorstring);
								}
							}
						}
						else
						{
							err = LAMERR_ANIM_IMAGE_CHUNK_NOT_FOUND;
							sprintf(errorstring, "Error Code: %ld\nChunk Name: %s\nDescription: Anim Image Chunk Not Found\n", err, sfh->sfh_BobBankName);
							DisplayError(errorstring);
						}
					}
				}
			}
		}
	}
	return(err);
}


/****************   FreeCookedAnims()   ************************/

void FreeCookedAnims( void )
{
	struct AnimNode *animnode;
	struct FrameFileHeader *ffh;
	struct AnimFileHeader *afh;
	struct PaulsBobImage *im;
	UWORD framenum;

	while( !IsListEmpty( &animlist ) )
	{
		/* The BobNumber field in the 'FrameFileHeader's has been changed in place
			to point to the 'PaulsBobImage' structure. We must change this back to the
			bob number so that when we cook it again it will be correct */
		animnode = (struct AnimNode *)animlist.lh_Head;
		afh = animnode->an_AnimHeader;
		ffh = (struct FrameFileHeader *)(afh+1);
		for(framenum = 0; framenum < afh->afh_NumOfFrames; framenum++)
		{
			im = (struct PaulsBobImage *)ffh->ffh_BobNumber;
			ffh->ffh_BobNumber = im->im_Number;
			ffh->ffh_XOffset = ffh->ffh_XOffset>>5;
			ffh->ffh_YOffset = ffh->ffh_YOffset>>5;
			ffh++;
		}
		Remove( &animnode->an_Node );
		MYFREEVEC( animnode );
	}
}

/****************   CheckUniqueAnimNames()   ************/
//
//  Goes through all the anims and makes sure
// that all the names are unique.

void CheckUniqueAnimNames(void)
{
	struct AnimNode *an;
	UBYTE	namebuffer[ANIMFULLNAMESIZE];
	UWORD err = LAMERR_ALLOK;

	for(an = (struct AnimNode *)animlist.lh_Head;
			an->an_Node.ln_Succ && an->an_Node.ln_Succ->ln_Succ;
			an = (struct AnimNode *)an->an_Node.ln_Succ)
	{
		if(!NullName(an->an_Node.ln_Name))
		{
			if(FindCompressedName(&animlist, an->an_Node.ln_Name, an->an_Node.ln_Succ))
			{
				err = LAMERR_NAME_NOT_UNIQUE;
				UnpackASCII(an->an_Node.ln_Name, namebuffer, ANIMFULLNAMESIZE-1);
				sprintf(errorstring, "Warning %ld: Anim Name '%s' Not Unique\n", err, namebuffer);
				DisplayWarning(errorstring);
			}
		}
		else
		{
			err = LAMERR_EMPTY_NAME;
			sprintf(errorstring, "Warning %ld: Anim Has No Name\n", err);
			DisplayWarning(errorstring);
		}
	}
}
