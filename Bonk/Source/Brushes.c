/****************************************************************************/
//
// BONK: Brushes.c
//
/****************************************************************************/



#include <stdio.h>
//#include <devices/inputevent.h>
//#include <graphics/gfx.h>
#include <exec/exec.h>

//#include <proto/graphics.h>
#include <proto/exec.h>

#include <global.h>
#include <ExternVars.h>



/***************  AllocBrush  ***************/
//
// Allocates an empty Brush structure and space to store a block array.
// Just copy in the block data, link into the brushlist and call FreeBrush()
// when you've finished.

struct Brush *AllocBrush( UWORD width, UWORD height )
{
	struct Brush *br;

	/* alloc data struct */
	if( br = AllocVec( sizeof( struct Brush ), MEMF_ANY|MEMF_CLEAR ) )
	{
		br->br_Width = width;
		br->br_Height = height;
		/* alloc block array memory */
		if( !( br->br_Data =
			AllocVec( width * height * sizeof( UWORD ), MEMF_ANY ) ) )
		{
			/* not enough memory - abort */
			FreeVec(br);
			br = NULL;
		}
	}
	return( br );
}


/***************  FreeBrush()  ***************/
//
// Frees a Brush structure and its associated block data (if any).
//

void FreeBrush( struct Brush *br )
{
	if( br )				/* sanity check */
	{
		/* free the block array */
		if( br->br_Data )
			FreeVec( br->br_Data );
		/* free the data struct */
		FreeVec( br );
	}
}


/***************  GrabBrush  ***************/
//
// Pick up a brush from the map.
// x0,y0,x1,y1 in map coords.
//
// returns a filled-out brush structure (with blockdata attached)
// ready for linking into the brushlist.

struct Brush *GrabBrush( struct Project *proj, WORD x0, WORD y0,
	WORD x1, WORD y1)
{
	struct Brush *br;
	WORD x,y,pleb;
	UWORD *p;

	/* clip to map boundaries */
	if( x0 < 0 ) x0 = 0;
	if( y0 < 0 ) y0 = 0;
	if( x1 < 0 ) x1 = 0;
	if( y1 < 0 ) y1 = 0;
	if( x0 >= proj->pj_MapW ) x0 = proj->pj_MapW - 1;
	if( y0 >= proj->pj_MapH ) y0 = proj->pj_MapH - 1;
	if( x1 >= proj->pj_MapW ) x1 = proj->pj_MapW - 1;
	if( y1 >= proj->pj_MapH ) y1 = proj->pj_MapH - 1;

	/* make sure x0,y0 at topleft */
	if( x1 < x0 )
	{
		pleb=x0;
		x0=x1;
		x1=pleb;
	}
	if( y1 < y0 )
	{
		pleb=y0;
		y0=y1;
		y1=pleb;
	}

	if( br = AllocBrush( x1 - x0 + 1, y1 - y0 + 1 ) )
	{
		p = br->br_Data;

		for( y=y0; y<=y1; y++)
		{
			for( x=x0; x<=x1; x++)
			{
				*p++ = PeekBlock( proj, x, y );
			}
		}
	}
	return( br );
}


void PasteBrush( struct ProjectWindow *pw, WORD x0, WORD y0, struct Brush *br )
{
	struct Project *proj;
	WORD x,y,x1,y1;
	UWORD *p;

	proj = pw->pw_Project;
	p = br->br_Data;

	x1 = x0 + br->br_Width;
	y1 = y0 + br->br_Height;

	for( y = y0; y < y1; y++ )
	{
		for( x = x0; x < x1; x++ )
		{
			if( x >= 0 && x < proj->pj_MapW &&
				y >= 0 && y < proj->pj_MapH )
			{
				if( *p != rmbblk )
					RPlotBlock( pw, x, y, *p );
			}
			p++;
		}
	}
}




/***************  GrabBWBrush()  ***************/
// Pick up a brush from a BlockWindow.
// x0,y0,x1,y1 in map coords.
//
// returns a filled-out brush structure (with blockdata attached)
// ready for linking into the brushlist.

struct Brush *GrabBWBrush( struct BlockWindow *bw, WORD x0, WORD y0,
	WORD x1, WORD y1)
{
	struct Brush *br;
	WORD x,y,pleb;
	UWORD *p;

	/* clip to left and top boundaries */
	if( x0 < 0 ) x0 = 0;
	if( y0 < 0 ) y0 = 0;
	if( x1 < 0 ) x1 = 0;
	if( y1 < 0 ) y1 = 0;

	/* make sure x0,y0 at topleft */
	if( x1 < x0 )
	{
		pleb=x0;
		x0=x1;
		x1=pleb;
	}
	if( y1 < y0 )
	{
		pleb=y0;
		y0=y1;
		y1=pleb;
	}

	if( br = AllocBrush( x1 - x0 + 1, y1 - y0 + 1 ) )
	{
		p = br->br_Data;

		for( y=y0; y<=y1; y++)
		{
			for( x=x0; x<=x1; x++)
			{
				if( x < globlocks->bs_LayoutWidth &&
					(y * globlocks->bs_LayoutWidth ) + x <= globlocks->bs_NumOfBlocks )
					*p = y * globlocks->bs_LayoutWidth + x;
				p++;
			}
		}
	}
	return( br );
}


void FreeAllBrushes( void )
{
	struct Brush *br;
	while( !IsListEmpty( &brushes ) )
	{
		br = (struct Brush *)brushes.lh_Head;
		Remove( &br->br_Node );
		FreeBrush( br );
	}
}


/***************  FreeOldBrushes()  ***************/
void FreeOldBrushes( void )
{
	struct Node *n,*pred;
	int i;

	n = brushes.lh_TailPred;
	i = 0;
	while( n->ln_Pred )
	{
		pred = n->ln_Pred;

		if( i >= BRUSHHISTORY )
		{
			Remove( n );
			FreeBrush( (struct Brush *)n );
		}
		n = pred;
		i++;
	}
}

