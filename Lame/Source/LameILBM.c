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


UWORD UnpackILBM(struct BitMapHeader *bmhd, UBYTE *body, UBYTE *destimage, UWORD allignedbytewidth)
{
	UWORD err = LAMERR_ALLOK;
	UWORD evenbytewidth;
	BYTE		count;
	UBYTE		*bodyptr, *imageptr,repeatbyte;
	UWORD plane,bytepos,line;
	UBYTE *linebase;

	evenbytewidth = ((bmhd->bmh_Width+15)>>3)&0xFFFE;

	bodyptr = body;

	switch(bmhd->bmh_Compression)
	{
		case cmpNone:
			linebase = destimage;
			for(line = 0; line < bmhd->bmh_Height; line++)
			{
				for(plane = 0; plane < bmhd->bmh_Depth; plane++)
				{
					imageptr = linebase;
					for(bytepos = 0; bytepos < evenbytewidth; bytepos++)
					{
						*imageptr++ = *bodyptr++;
					}
					linebase += allignedbytewidth;
				}
			}
			err = FALSE;
			break;
		case cmpByteRun1:
			linebase = destimage;
			for(line = 0; line < bmhd->bmh_Height; line++)
			{
				for(plane = 0; plane < bmhd->bmh_Depth; plane++)
				{
					imageptr = linebase;
					bytepos = 0;
					while(bytepos < evenbytewidth)
					{
						count = *bodyptr++;
						if(count != -128)
						{
							if(count >= 0)
							{
								/* copy the next count+1 bytes literally */
								bytepos += count+1;
								while(count-- >= 0)			/* include 0 so that makes it count+1 */
								{
									*imageptr++ = *bodyptr++;
								}
							}
							else
							{
								/* repeat the next byte count+1 times */
								repeatbyte = *bodyptr++;
								count *= -1;									/* make positive */
								bytepos += count+1;
								while(count-- >= 0)			/* include 0 so that makes it count+1 */
								{
									*imageptr++ = repeatbyte;
								}
							}
						}
					}				/* BytePos */
					linebase += allignedbytewidth;
				}				/* Plane */
			}				/* Line */
			break;
		default:
			printf("Unknown Compression Type\n");
			break;
	}
	return(err);
}

/*******************  CookILBMs()   ****************************/

UWORD CookILBMs( struct LameReq *lr )
{
	struct FileNode									*fn = NULL;
	struct ChunkNode								*cn = NULL, *dispcn = NULL;
	struct ChunkNode								*bmhdcn = NULL, *cmapcn = NULL, *bodycn = NULL;
	struct BitMapHeader							*bmhd = NULL;
	UBYTE														*cmap = NULL, *body = NULL;
	UWORD														 err = LAMERR_ALLOK;
	struct ILBMNode									*ilbmnode = NULL;
	UWORD														 allignedbytewidth;
	ULONG														 imagesize;
	UBYTE														*imagebase;
	UWORD														pixwidth;
	UBYTE														packbuf[ BOBFILENAMESIZE ];

	for( fn = (struct FileNode *)filelist.lh_Head;
			 fn->fn_Node.ln_Succ && err < LAME_FAILAT;
			 fn = (struct FileNode *)fn->fn_Node.ln_Succ )
	{
		if( fn->fn_UsedInLevel && (fn->fn_Form == ID_ILBM))
		{
			/* Found an ILBM chunk */
			D(bug("--- Cook ILBM's ---\n"));

			for( cn = (struct ChunkNode *)fn->fn_ChunkList.lh_Head;
					 cn->cn_Node.ln_Succ && err < LAME_FAILAT;
					 cn = (struct ChunkNode *)cn->cn_Node.ln_Succ )
			{
				switch(cn->cn_TypeID)
				{
					case ID_BMHD:
						D(bug(" Found BMHD Chunk\n"));
						bmhdcn = cn;
						bmhd = bmhdcn->cn_Data;
						break;
					case ID_CMAP:
						D(bug(" Found CMAP Chunk\n"));
						cmapcn = cn;
						cmap = cmapcn->cn_Data;
						break;
					case ID_BODY:
						D(bug(" Found BODY Chunk\n"));
						bodycn = cn;
						body = bodycn->cn_Data;
						break;
				}
			}
			if(bmhd && cmap && body)
			{
				if(ilbmnode = MYALLOCVEC(sizeof(struct ILBMNode), MEMF_ANY|MEMF_CLEAR,"ILBM Node"))
				{
					AddTail(&ilbmlist, &ilbmnode->ilbm_Node);
					pixwidth = bmhd->bmh_Width;
					allignedbytewidth = (((bmhd->bmh_Width>>3)+7)&0xFFF8);
					D(bug("Alligned Byte Width: %ld\n",allignedbytewidth));

					/* pack the picture name */
					PackASCII(MyFilePart(fn->fn_Node.ln_Name), packbuf, BOBFULLNAMESIZE-1);

					/* the next bit allows for an extra 8 bytes off the side of the bitmap */
					/* to allow for bob clipping and stuff like that - BC */

					/* Put some sort of check in here to make sure this is a game-type level!!!! */

					imagesize = 0;

					/* peek at the display chunk */
					if( dispcn = FindChunkByType(ID_DISP) )
					{
						if( CompressedStringSame( (UWORD *)((struct Display *)dispcn->cn_Data)->dp_BackDrop,
							(UWORD *)packbuf ) )
						{
							allignedbytewidth += 8;
							pixwidth += 64;
							imagesize += 8;		/* so the screen can be scrolled the full length (the extra 8 bytes off the */
																/* right that you never see */

							/* check against min size */
							if( ((struct Display *)dispcn->cn_Data)->dp_DisplayID & HIRES_KEY )
							{
								/* hires mode - min size 88 bytes */
								if( allignedbytewidth < 88 )
								{
									allignedbytewidth = 88;
									pixwidth = 88*8;
								}
							}
							else
							{
								/* lowres mode - min size 48 bytes */
								if( allignedbytewidth < 48 )
								{
									allignedbytewidth = 48;
									pixwidth = 48*8;
								}
							}
						}
						D(bug("Found DISP -> new Alligned Byte Width: %ld\n",allignedbytewidth));
					}
					/* no display chunk? oh well, too bad - just assume ilbm is needed without the 8 bytes */

					imagesize += (allignedbytewidth * bmhd->bmh_Height * bmhd->bmh_Depth) + 8;
					
					if(imagebase = MYALLOCVEC(imagesize, MEMF_CHIP|MEMF_CLEAR, "ILBM ImageBase"))
					{
						ilbmnode->ilbm_Node.ln_Name = ilbmnode->ilbm_NameBuffer;
//						PackASCII(MyFilePart(fn->fn_Node.ln_Name), ilbmnode->ilbm_NameBuffer, BOBFULLNAMESIZE-1);
						CopyMem( packbuf, ilbmnode->ilbm_NameBuffer, BOBFULLNAMESIZE-1 );
						ilbmnode->ilbm_Image = imagebase;
						ilbmnode->ilbm_PaulsPictureImage.ppi_Type = IMTY_PICTURE;
						ilbmnode->ilbm_PaulsPictureImage.ppi_Width = pixwidth;
						ilbmnode->ilbm_PaulsPictureImage.ppi_Height = bmhd->bmh_Height;
						ilbmnode->ilbm_PaulsPictureImage.ppi_Depth = bmhd->bmh_Depth;
						ilbmnode->ilbm_PaulsPictureImage.ppi_DisplayID = 0;
						ilbmnode->ilbm_PaulsPictureImage.ppi_Palette = cmap;
						ilbmnode->ilbm_PaulsPictureImage.ppi_Image = (UBYTE *)((LONG)(imagebase+7)&0xFFFFFFF8);
						if( (err = UnpackILBM(bmhd, body, ilbmnode->ilbm_PaulsPictureImage.ppi_Image, allignedbytewidth)) < LAME_FAILAT)
						{
						}
					}
				}
				else
				{
					err = LAMERR_MEM;
					sprintf(errorstring, "Error Code: %ld\nDescription: Not Enough Memory\n", err);
					DisplayError(errorstring);
					return(err);
				}
				bmhdcn->cn_Cooked = TRUE;
				cmapcn->cn_Cooked = TRUE;
				bodycn->cn_Cooked = TRUE;
			}
			else
			{
				err = LAMERR_ILBM_MISSING_CHUNK;
				sprintf(errorstring, "Error Code: %ld\nFile: %s\nDescription: ILBM File Missing Chunk\n", err, fn->fn_Node.ln_Name);
				DisplayError(errorstring);
			}
		}
	}
	return(err);
}


/****************   FreeCookedILBMs()   ************************/

void FreeCookedILBMs( void )
{
	struct ILBMNode *ilbmnode;
	while( !IsListEmpty( &ilbmlist ) )
	{
		ilbmnode = (struct ILBMNode *)ilbmlist.lh_Head;
		if(ilbmnode->ilbm_Image)
			MYFREEVEC(ilbmnode->ilbm_Image);
		Remove( &ilbmnode->ilbm_Node );
		MYFREEVEC( ilbmnode );
	}
}
