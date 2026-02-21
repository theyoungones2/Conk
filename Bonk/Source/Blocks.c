/****************************************************************************/
//
// BONK: Blocks.c
//
/****************************************************************************/

#include <stdio.h>
#include <string.h>
#include <dos/dos.h>
#include <dos/dostags.h>
#include <exec/exec.h>
#include <intuition/intuition.h>
#include <graphics/gfxbase.h>
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



static UBYTE OldCalcBlockColour( struct BlockSet *bs, UWORD blk );



/*************  CreateDummyBlockSet()  *************/
//
// Creates a blockset consisting of two 1-bitplane blocks
// (first one blank, second one full).
// A Kludge - assumes wordwidth block images (but always allocs
// enough memory for full blocks, so don't panic).
// Free with FreeBlockSet(), below.
//
// Returns new BlockSet, or NULL if there were fuckups.

struct BlockSet *CreateDummyBlockSet(void)
{
	struct BlockSet *bs;
	UWORD	i;
	UWORD *data;

	if ( bs = AllocVec( sizeof(struct BlockSet), MEMF_ANY | MEMF_CLEAR ) )
	{
		/* alloc space for 2 blocks */
		if( data = AllocVec( BLKWORDW * 2 * BLKH * 2, MEMF_CHIP | MEMF_CLEAR ) )
		{
			/* fill second block with $FFFFs */
			/* (this is the one-word-only kludge) */
			for( i = BLKH; i < BLKH*2; i++)
				data[i] = 0xFFFF;

			/* setup structure values */
			bs->bs_Depth = 1;
			bs->bs_BytesPerRow = BLKWORDW * 2;
			bs->bs_BlkSize = BLKH * BLKWORDW * 2;
			bs->bs_NumOfBlocks = 2;
			bs->bs_Data = (UBYTE *)data;

			/* suss out preview blockcolours */
			CalcBlockPreviewColours( bs );
		}
		else
		{
			FreeVec(bs);
			bs = NULL;
		}
	}
//	else
//		printf("Not enough memory.\n");
	return(bs);
}


/*************  FreeBlockSet()  *************/
//
// Frees a BlockSet structure, and the block image
// data associated with it.
//

void FreeBlockSet( struct BlockSet *bs )
{
	if( bs )
	{
		if( bs->bs_Data )
			FreeVec( bs->bs_Data );
		if( bs->bs_PreviewColours )
			FreeVec( bs->bs_PreviewColours );
		FreeVec( bs );
	}
}





/* use block coords */
void DrawBlock( struct Window *win, LONG xpos, LONG ypos, UWORD blk )
{
	struct BitMap bm;
	struct BlockSet *bs;
	UWORD i;
	UBYTE *ptr;

	LONG	xsize = BLKW;
	LONG	ysize = BLKH;
	LONG	redge, bedge;

	xpos = (xpos*BLKW) + win->BorderLeft;
	ypos = (ypos*BLKH) + win->BorderTop;

	if( bs = globlocks )
	{
		if( blk >= bs->bs_NumOfBlocks )
			blk = 0;

		/* clip block against bottom and right borders */

		redge = win->Width - win->BorderRight;
		bedge = win->Height - win->BorderBottom;
		if(( xpos + xsize ) > redge )
		{
			xsize = xsize - (xpos + xsize - redge);
			if (xsize < 0) return((void)0);
		}
		if(( ypos + ysize ) > bedge )
		{
			ysize = ysize - (ypos + ysize - bedge);
			if (ysize < 0) return((void)0);
		}

		bm.BytesPerRow = bs->bs_BytesPerRow;
		bm.Rows = BLKH;
		bm.Flags = 0;
		bm.Depth = bs->bs_Depth;
		bm.pad = 0;

		ptr = bs->bs_Data + (blk * bs->bs_BlkSize);
		for( i=0; i<bs->bs_Depth; i++)
		{
			bm.Planes[i] = (char *)ptr;
			ptr += (BLKWORDW * 2);						//bs->bs_BlkW;
		}

		BltBitMapRastPort( &bm,0,0,win->RPort,xpos,ypos,xsize,ysize,0xC0);
	}
}


void BltBlockWindow( struct Window *win, LONG xpos, LONG ypos, UWORD blk )
{
	struct BitMap bm;
	struct BlockSet *bs;
	UWORD i;
	UBYTE *ptr;

	LONG	xsize = BLKW;
	LONG	ysize = BLKH;
	LONG	redge, bedge;

//	xpos = (xpos*BLKW) + win->BorderLeft;
//	ypos = (ypos*BLKH) + win->BorderTop;
//	xpos += win->BorderLeft;
//	ypos += win->BorderTop;

	if( bs = globlocks )
	{
		if( blk >= bs->bs_NumOfBlocks )
			blk = 0;

		/* clip block against bottom and right borders */

		redge = win->Width - win->BorderRight;
		bedge = win->Height - win->BorderBottom;
		if(( xpos + xsize ) > redge )
		{
			xsize = xsize - (xpos + xsize - redge);
			if (xsize < 0) return((void)0);
		}
		if(( ypos + ysize ) > bedge )
		{
			ysize = ysize - (ypos + ysize - bedge);
			if (ysize < 0) return((void)0);
		}

		bm.BytesPerRow = bs->bs_BytesPerRow;
		bm.Rows = BLKH;
		bm.Flags = 0;
		bm.Depth = bs->bs_Depth;
		bm.pad = 0;

		ptr = bs->bs_Data + (blk * bs->bs_BlkSize);
		for( i=0; i<bs->bs_Depth; i++)
		{
			bm.Planes[i] = (char *)ptr;
			ptr += (BLKWORDW * 2);						//bs->bs_BlkW;
		}

		BltBitMapRastPort( &bm,0,0,win->RPort,xpos,ypos,xsize,ysize,0xC0);
	}
}




void OldDrawMap(struct ProjectWindow *pw)
{
//	struct BlockSet *bs;
	struct Project	*proj;
	struct Window		*win;

	UWORD winx, winy, blk;
	WORD rightedge, bottomedge;
	UWORD *base, *ptr;
	WORD xpos, ypos;

	/* make sure the window is all cool and groovy */
	if ( win = pw->pw_sw.sw_Window )
	{
		proj = pw->pw_Project;
		base = proj->pj_Map;
		/* find first line of visible map */
		base += ( (pw->pw_MapY * proj->pj_MapW) + pw->pw_MapX );

		/* see which bits appear on the right and bottom of the view area */
		rightedge = ( (win->Width - win->BorderLeft - win->BorderRight) / BLKW ) +
			pw->pw_MapX + 1;
		bottomedge = ( (win->Height - win->BorderTop - win->BorderBottom) / BLKH ) +
			pw->pw_MapY + 1;

		/* start drawing at top of window */

		winy = 0;

		/* main drawing loop */
		for(ypos = pw->pw_MapY; ypos < bottomedge; ypos++)
		{
			/* start drawing at inner leftedge of window */
			winx = 0;
			/* init map ptr to beginning of row */
			ptr = base;

			for(xpos = pw->pw_MapX; xpos < rightedge; xpos++)
			{
				/* read the block and make sure we're still within the map limits */
				if( (xpos < proj->pj_MapW) &&
					(ypos < proj->pj_MapH) )
					blk = *ptr++;
				else
					blk = 0;
				/* draw the block */
				DrawBlock(win, winx, winy, blk );
				/* update drawing position */
				winx++;
			}
			winy++;
			/* jump down to next line of mapdata */
			base += proj->pj_MapW;
		}
	}
}


/*************  LoadFile()  *************/
//
// A general-purpose loader routine. Loads the specified file
// into memory. Use FreeVec() to free the file when done.
//

UBYTE *LoadFile( char *name, ULONG memtype )
{
	BPTR fh;
	ULONG len;
	UBYTE *dat = NULL;

	if( fh = Open( name, MODE_OLDFILE ) )
	{
		/* find file length */
		Seek( fh, 0, OFFSET_END );
		len = Seek( fh, 0, OFFSET_BEGINNING );

		if( dat = AllocVec( len, memtype ) )
			Read( fh, dat, len );
		Close( fh );
	}
	return( dat );
}



void FreeMap(struct Project *proj)
{
	if( proj->pj_Map )
	{
		FreeVec( proj->pj_Map );
		proj->pj_MapW = 0;
		proj->pj_MapH = 0;
	}
}









/************  InWindow()  ************/
//
// Returns TRUE if the specified block position will be visible
// in the given window (Block coords start at the inner border
// of the window).

BOOL InWindow(struct Window *win, WORD x, WORD y)
{
	WORD bw,bh;

	bw=win->Width - win->BorderLeft - win->BorderRight;
	bh=win->Height - win->BorderTop - win->BorderBottom;
	if( x>=0 && x<=( bw/BLKW ) &&
		y>=0 && y<=( bh/BLKH ) )
		return(TRUE);
	else
		return(FALSE);
}


/***************  BlkRectFill()  ***************/
//
// Draws a rectangle into the given projectwindow.
// x0,y0,x1,y1 blk coords, relative to window topleft.
// automagically flips rectangle corners if needed.
// does minimal clipping against right and bottom window borders.
//


void BlkRectFill(struct Window *win, UWORD x0, UWORD y0,
	UWORD x1, UWORD y1)
{
	UWORD pleb;

	if( x0 > x1 )
	{
		pleb=x0;
		x0=x1;
		x1=pleb;
	}
	if( y0 > y1 )
	{
		pleb=y0;
		y0=y1;
		y1=pleb;
	}

	x0 = (x0 * BLKW) + win->BorderLeft;
	x1 = (x1 * BLKW) + win->BorderLeft + BLKW -1;
	y0 = (y0 * BLKH) + win->BorderTop;
	y1 = (y1 * BLKH) + win->BorderTop + BLKH - 1;

	/* clip to right and bottom */
	if( x1 >= win->Width - win->BorderRight )
		x1 = win->Width - win->BorderRight - 1;
	if( y1 >= win->Height - win->BorderBottom )
		y1 = win->Height - win->BorderBottom - 1;

	if( x1>=x0 && y1>=y0 )
		RectFill(win->RPort, x0, y0, x1, y1 );
}


/***************  BlkRectOutline()  ***************/
//
// Draws a rectangle outline into the given projectwindow.
// x0,y0,x1,y1 blk coords, relative to window topleft.
// automagically flips rectangle corners if needed.
//

void BlkRectOutline(struct Window *win, UWORD x0, UWORD y0,
	UWORD x1, UWORD y1)
{
	UWORD pleb;
	UWORD wx0,wy0,wx1,wy1;

	if( x0 > x1 )
	{
		pleb=x0;
		x0=x1;
		x1=pleb;
	}
	if( y0 > y1 )
	{
		pleb=y0;
		y0=y1;
		y1=pleb;
	}

	wx0 = (x0 * BLKW) + win->BorderLeft;
	wx1 = (x1 * BLKW) + win->BorderLeft;
	wy0 = (y0 * BLKH) + win->BorderTop;
	wy1 = (y1 * BLKH) + win->BorderTop;

//	RectFill(win->RPort, wx0, wy0, wx1 + BLKW - 1, wy1 + BLKH - 1 );
	Move( win->RPort, wx0, wy0 );
	Draw( win->RPort, wx1 + BLKW - 1 , wy0 );
	Draw( win->RPort, wx1 + BLKW - 1 , wy1 + BLKH - 1 );
	Draw( win->RPort, wx0 , wy1 + BLKH - 1 );
	Draw( win->RPort, wx0, wy0 + 1 );
}


/***************  ClipToWindowBlkX()  ***************/
//
// Clips blkx so that it is within the window boundaries.
// blkx = block coord, relative to window topleft.

WORD ClipToWindowBlkX( struct Window *win, WORD blkx )
{
	WORD blockswide;

	blockswide = ( win->Width - win->BorderLeft - win->BorderRight ) / BLKW;
	if( blkx < 0 ) blkx = 0;
	if( blkx > blockswide ) blkx = blockswide;
	return( blkx );
}


/***************  ClipToWindowBlkY()  ***************/
//
// Clips blky so that it is within the window boundaries.
// blky = block coord, relative to window topleft.

WORD ClipToWindowBlkY( struct Window *win, WORD blky )
{
	WORD blockshigh;

	blockshigh = ( win->Height - win->BorderTop - win->BorderBottom ) / BLKH;
	if( blky < 0 ) blky = 0;
	if( blky > blockshigh ) blky = blockshigh;
	return( blky );
}

/***************  FindBlockColour()  ***************/
//

UBYTE FindBlockColour( struct BlockSet *bs, UWORD blk )
{
	UBYTE c = 0;

	if( bs )
	{
		if( bs->bs_PreviewColours )
		{
			c = bs->bs_PreviewColours[ blk ];
		}
	}
	return( c );
}


/***************  CalcBlockColour()  ***************/
//

static UBYTE OldCalcBlockColour( struct BlockSet *bs, UWORD blk )
{
	UWORD x, y;
	ULONG	red=0, grn=0, blu=0,count=0;
	UBYTE	*p, c;

	//printf("CalcBlockColour(), bs = %ld, %s\n",&bs,bs->bs_Name);
	//printf("blk: %d\n",blk);
	//red = BlkPickPixel( bs, x, y, blk );
	//printf("blk: %d\n",blk);

	p = globpalette->pl_Colours;

	for( y=0; y<BLKH; y++ )
	{
		for( x=0; x<BLKW; x++ )
		{
//			printf("BlkPickPixel....");
			c = BlkPickPixel( bs, x, y, blk );
//			printf("%d!\n",c);
//			c=blk?2:0;
			if( c < globpalette->pl_Count )
			{
				red += p[ c*3 ];
				grn += p[ (c*3)+1 ];
				blu += p[ (c*3)+2 ];
				count++;
			}
		}
	}
	if( count )
	{
//		printf("blk: %d, count: %d, r: %d, g: %d, b: %d\n",
//			blk,count,red/count,grn/count,blu/count );

		return( (UBYTE)FindnBestPen( globpalette, globpalette->pl_Count,
			(UBYTE)(red/count), (UBYTE)(grn/count), (UBYTE)(blu/count) ) );
	}
	else
	{
		/* fucked */
		if( blk==0 )
			return(0);
		else
			return(1);
	}
}


/***************  CalcBlockPreviewColours()  ***************/
//

void CalcBlockPreviewColours( struct BlockSet *bs )
{
	struct List	*wlist;
	UWORD b;

	if(wlist = BlockAllWindows())
	{

		if( bs )			/* the safe side */
		{
			/* free old array, if any */
			if( bs->bs_PreviewColours )
				FreeVec( bs->bs_PreviewColours );

			/* alloc mem for array */
			if( bs->bs_PreviewColours = AllocVec( bs->bs_NumOfBlocks, MEMF_ANY ) )
			{
				for( b=0; b < bs->bs_NumOfBlocks; b++ )
				{
					bs->bs_PreviewColours[b] = CalcBlockColour( bs, globpalette, b );
				}
			}
		}
		FreeBlockedWindows( wlist );
	}
}


void FreeBlockPreviewColours( struct BlockSet *bs )
{
	if(bs)
	{
		if( bs->bs_PreviewColours )
		{
			FreeVec( bs->bs_PreviewColours );
			bs->bs_PreviewColours = NULL;
		}
	}
}
