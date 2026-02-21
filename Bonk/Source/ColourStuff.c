/****************************************************************************/
//
// BONK: ColourStuff.c
//
/****************************************************************************/



#include <stdio.h>
#include <string.h>
#include <dos/dos.h>
#include <dos/dostags.h>
#include <exec/exec.h>
#include <intuition/intuition.h>
#include <graphics/gfxbase.h>
#include <rexx/rxslib.h>
#include <libraries/gadtools.h>
#include <libraries/diskfont.h>
#include <libraries/asl.h>
#include <libraries/iffparse.h>

#include <proto/exec.h>
#include <proto/dos.h>
#include <proto/intuition.h>
#include <proto/graphics.h>
#include <proto/diskfont.h>
#include <proto/asl.h>
#include <clib/alib_protos.h>
#include <proto/gadtools.h>
#include <proto/iffparse.h>

//#include <datatypes/pictureclass.h>

#include <global.h>
#include <ExternVars.h>


/****************  FreePalette()  ****************/
//
// Frees a palette structure and it's associated RGB data stuff.
//

void FreePalette( struct Palette *pal )
{
	if(pal)
	{
		if( pal->pl_Colours )
		{
			FreeVec( pal->pl_Colours );
		}
		FreeVec(pal);
	}
}



/****************  CreateDefaultPalette()  ****************/
//
// Makes up a default 4 colour palette structure
// ( use FreePalette() when finished ).

struct Palette *CreateDefaultPalette(void)
{
	struct Palette *pal;
//	static UBYTE colours[4*3] = { 64,64,224, 0,0,0, 128,128,255, 96,128,224 };

//	static UBYTE colours[4*3] =	{ 0x80,0x8E,0xB8, 0x3A,0x3D,0x7D,
//																0xD6,0xDC,0xFF, 0x51,0x88,0xBE };

//	static UBYTE colours[4*3] = { 154,110,194, 12,0,52, 226,217,255, 52,119,223 };

	static UBYTE colours[4*3] =	{ 0x00,0x00,0x00, 0x55,0x55,0x88,
		0xEE,0xEE,0xFF, 0x88, 0x99, 0xBB	};		//0x77,0x77,0x99 };

	if( pal = AllocVec( sizeof( struct Palette ), MEMF_ANY ) )
	{
		if( pal->pl_Colours = AllocVec( 4*3, MEMF_ANY ) )
		{
			pal->pl_Count = 4;
			CopyMem( colours, pal->pl_Colours, 4*3 );
		}
		else
		{
			/* fuckedup */
			FreeVec( pal );
			pal = NULL;
		}
	}
	return( pal );
}

/****************  LoadPalette()  ****************/
//
// Create a palette structure with RGB data extracted from
// an iff file. Use FreePalette() to... um... free the palette.
// Checks for CMAP chunks in CONK and ILBM files...
//
// nb: Resuss the error handling here some time, ok?
// (it really sucks)

struct Palette *LoadPalette(char *name)
{
	struct IFFHandle *iff = NULL;
	struct Palette *pal = NULL;
	struct ContextNode *cn;

	if(iff = AllocIFF())
	{
		if( iff->iff_Stream = Open( name, MODE_OLDFILE) )
		{
			InitIFFasDOS(iff);
			if(!OpenIFF( iff, IFFF_READ ))
			{
			if(!StopChunk(iff, ID_ILBM, ID_CMAP) && !StopChunk(iff, ID_CONK, ID_CMAP))
				{
					if( !ParseIFF(iff, IFFPARSE_SCAN) )
					{
						if(cn = CurrentChunk(iff))
						{
							if(cn->cn_Type == ID_ILBM || cn->cn_Type == ID_CONK)
							{
								if( pal = AllocVec( sizeof(struct Palette), MEMF_ANY|MEMF_CLEAR ) )
								{
									if( pal->pl_Colours = AllocVec( cn->cn_Size, MEMF_ANY|MEMF_CLEAR ) )
									{
										pal->pl_Count = cn->cn_Size / 3;
										ReadChunkBytes( iff, pal->pl_Colours, cn->cn_Size );
									}
									else
									{
										/* damage control */
										FreeVec(pal);
										pal = NULL;
//										printf("Couldn't alloc palette struct memory\n");
									}
								}
//								else
//								{
									/* lets face it - if we can't allocate 6 measly bytes */
									/* for a lowly Palette structure, then we are quite   */
									/* probably not going to have a very nice day. */
//									printf("Couldn't alloc RGB data memory\n");
//								}
							}
						}
					}
				}
				CloseIFF(iff);
			}
			Close(iff->iff_Stream);
		}
		FreeIFF(iff);
	}
	return(pal);
}


/*************  InstallPalette()  *************/
//
// Installs the given palette into a screen.
//

void InstallPalette(struct Screen *scr, struct Palette *pal)
{
	ULONG			gun;
	UWORD			*colourtable4;
	ULONG			*colourtable32;
	UWORD			count,tableoffset;
	UBYTE			*p;


	p = pal->pl_Colours;

	if( GfxBase->LibNode.lib_Version >= 39 )
	{
		if( colourtable32 = AllocVec(2+2+((pal->pl_Count)*(4*3))+4,MEMF_ANY|MEMF_CLEAR) )
		{
			/* Copy the 8 bit colour table to the 32 bit table. */
			tableoffset=1;
			for(count=0; count<(pal->pl_Count); count++)
			{
				colourtable32[tableoffset++] = *p++;
				colourtable32[tableoffset++] = *p++;
				colourtable32[tableoffset++] = *p++;
			}
	
			/* Now scale to 32 bits */
			tableoffset = 1;
			count=pal->pl_Count;
			while(count--)
			{
				gun = colourtable32[tableoffset];
				colourtable32[tableoffset++] |= ((gun << 24) | (gun << 16) | (gun << 8));
				gun = colourtable32[tableoffset];
				colourtable32[tableoffset++] |= ((gun << 24) | (gun << 16) | (gun << 8));
				gun = colourtable32[tableoffset];
				colourtable32[tableoffset++] |= ((gun << 24) | (gun << 16) | (gun << 8));
			}
			colourtable32[0] = ((pal->pl_Count) << 16)+0;
			colourtable32[1+((pal->pl_Count)*3)]=0;
			LoadRGB32(&scr->ViewPort,colourtable32);
			FreeVec(colourtable32);
		}
	}
	else
	{
		/* If we get here, then we're not running V39 or greater, bummer, so we use
				the standard SetRGB4. */

		if( colourtable4 = AllocVec((pal->pl_Count)*2,MEMF_ANY|MEMF_CLEAR) )
		{
			for(count=0; count<(pal->pl_Count); count++)
				colourtable4[count] = (( *p++ & 0xF0 ) << 4 ) |
															( *p++ & 0xF0 ) |
															( *p++ >> 4 );
			LoadRGB4(&scr->ViewPort,colourtable4,pal->pl_Count);
			FreeVec(colourtable4);
		}
	}
}



/*************  OldInstallPalette()  *************/
//
// Installs the given palette into a screen.
// Should make it AGA-aware sometime and suss out using
// 32 bit palette functions but, well, there's just sooooooo
// much other fun stuff to do, so it'll have to wait.
//

void OldInstallPalette(struct Screen *scr, struct Palette *pal)
{
	UWORD i, colcnt;
	UBYTE *p;

	colcnt = 1 << ( scr->RastPort.BitMap->Depth );

	p = pal->pl_Colours;

//	if( GfxBase->LibNode.lib_Version >= 39 )
//	{
		/* using version 39 or higher (wb3+) */
		/* so we can use groovy 32bit palette functions */

//		for( i=0; ( i < colcnt) && ( i < pal->pl_Count ); i++ )
//		{
//			SetRGB32( &scr->ViewPort, i,
//				(*p++ & 0x0F0) << 24,
//				(*p++ & 0x0F0) << 24,
//				(*p++ & 0x0F0) << 24 );
//		}
//	}
//	else
//	{
		/* < version 39 */
		/* go through palette and install as 4bit (ECS) RGB values */

		for( i=0; ( i < colcnt) && ( i < pal->pl_Count ); i++ )
		{
			SetRGB4( &scr->ViewPort, i,
				(*p++ & 0x0F0) >> 4,
				(*p++ & 0x0F0) >> 4,
				(*p++ & 0x0F0) >> 4 );
		}
//	}
}



/*******************  OldFindnBestPen()  *******************/
//
// Go through a Palette looking for the closest matching
// entry to the given (24 bit) rgb value. Returns palette
// index. Can also potentially return 0xFFFF if stuff is
// seriously out to lunch.
//
// inputs:
//  pal = palette to pick from
//  numpens = how many pens available
//  r,g,b = desired colour
//
// Known bugs: matching method arrived at via complete
//             guesswork - crap results highly probable.

UWORD OldFindnBestPen( struct Palette *pal, UWORD numpens, UBYTE r, UBYTE g, UBYTE b )
{
	UBYTE *p;
	UWORD i, bestpen = 0xFFFF;
	ULONG delta, bestdelta = 0xFFFFFFFF;

	p = pal->pl_Colours;
	for( i=0; ( i < pal->pl_Count ) && ( i < numpens ); i++)
	{
		delta = ( (r - *p) * (r-*p++) ) +
			( (g - *p) * (g-*p++) ) +
			( (b - *p) * (b-*p++) );
		if( delta < bestdelta )
		{
			bestpen = i;
			bestdelta = delta;
		}
	}
	return(bestpen);
}


/********************  SussDrawInfoPens()  ********************/
//
// Sets up a drawinfo pen array by selecting colours in the
// given palette. Tries to keep things looking relatively sane,
// bas-relief and readable, but this really depends on the colours
// available. Uses different 'ideals' for dark, medium and bright
// background colours.
//
// params:
//  pens    = ptr to uninitialized pen array (+ room for 0xFFFF terminator)
//  pal     = palette to choose the colours from
//  numcols = number of colours to consider (ie num of colours on screen)

void SussDrawInfoPens( UWORD *pens, struct Palette *pal, UWORD numcols )
{
	UBYTE *p;
	ULONG brightness;

	p = pal->pl_Colours;
	brightness = (*p * *p++) + (*p * *p++) + (*p * *p++);

	if (brightness < 12288)
	{
		/* dark background */
		/* so avoid using dark colours */

		/* optimise some of this shit sometime... */
		pens[DETAILPEN] = FindnBestPen( pal, numcols, 255, 255, 255 ); //128
		pens[BLOCKPEN] = FindnBestPen( pal, numcols, 128, 128, 128 );
		pens[TEXTPEN] = FindnBestPen( pal, numcols, 255, 255, 255 ); //128
		pens[SHINEPEN] = FindnBestPen( pal, numcols, 255, 255, 255);
		pens[SHADOWPEN] = FindnBestPen( pal, numcols, 128, 128, 128);
		pens[FILLPEN] = FindnBestPen( pal, numcols, 102, 136, 187);
		pens[FILLTEXTPEN] = FindnBestPen( pal, numcols, 255, 255, 255); //128
		pens[BACKGROUNDPEN] = 0xFFFF;
	}
	else
	{
		if(brightness > 110592)
		{
			/* bright background */
			/* - avoid really bright colours */

			pens[DETAILPEN] = FindnBestPen( pal, numcols, 128, 128, 128 );
			pens[BLOCKPEN] = FindnBestPen( pal, numcols, 0, 0, 0 );
			pens[TEXTPEN] = FindnBestPen( pal, numcols, 0, 0, 0 );
			pens[SHINEPEN] = FindnBestPen( pal, numcols, 192, 192, 192);
			pens[SHADOWPEN] = FindnBestPen( pal, numcols, 0, 0, 0);
			pens[FILLPEN] = FindnBestPen( pal, numcols, 102, 136, 187);
			pens[FILLTEXTPEN] = FindnBestPen( pal, numcols, 0, 0, 0);
			pens[BACKGROUNDPEN] = 0xFFFF;
		}
		else
		{
			/* normal (workbenchesque) pen selections */
			/* aim for standard workbench default palette (almost) */

			pens[DETAILPEN] = FindnBestPen( pal, numcols, 128, 128, 128 );
			pens[BLOCKPEN] = FindnBestPen( pal, numcols, 0, 0, 0 );
			pens[TEXTPEN] = FindnBestPen( pal, numcols, 0, 0, 0 );
			pens[SHINEPEN] = FindnBestPen( pal, numcols, 255, 255, 255);
			pens[SHADOWPEN] = FindnBestPen( pal, numcols, 0, 0, 0);
			pens[FILLPEN] = FindnBestPen( pal, numcols, 102, 136, 187);
			pens[FILLTEXTPEN] = FindnBestPen( pal, numcols, 0, 0, 0);
			pens[BACKGROUNDPEN] = 0xFFFF;
		}
	}
}

