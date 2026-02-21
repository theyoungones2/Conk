/********************************************************/
//
// BLKSStuff.c
//
/********************************************************/

#define BLKSSTUFF_C

#include <stdio.h>
#include <strings.h>

#include <exec/exec.h>
#include <dos/dos.h>
#include <libraries/iffparse.h>
#include <proto/exec.h>
#include <proto/dos.h>
#include <proto/iffparse.h>
#include <proto/graphics.h>

#include "global.h"

/* BLKSHeader struct - saved at beginning of BLKS chunk */

struct BLKSHeader
{
	UBYTE	blks_Name[ BLOCKSETNAMESIZE ];
	UWORD	blks_NumOfBlocks;
	UWORD blks_Depth;
	UWORD	blks_LayoutWidth;	// how many blocks across in the Blocks window
													// (20 for lores)
	/* future expansion stuff */
	UWORD	blks_Flags;				// set to 0!
	UWORD	blks_PixW;				// width in pixels (currently 16)
	UWORD	blks_PixH;				// height (currently 16)
	UWORD blks_BitMapHeight;// How high the bitmap is to be made.
	UWORD	blks_Reserved[3];	// set to 0!
};


static BOOL MakeBLKSInfoString( struct Chunk *cnk, UBYTE *buf );

/**************************************************************/

struct Chunk *CreateBLKSChunk( struct Filetracker *ft )
{
	struct Chunk *cnk;
	if( cnk = AllocVec( sizeof( struct Chunk ), MEMF_ANY|MEMF_CLEAR ) )
	{
		if( ft )
			ft->ft_ChunkCount++;
		cnk->ch_TypeID = ID_BLKS;
		cnk->ch_Edit = NULL;
		cnk->ch_Free = FreeBLKSChunk;
		cnk->ch_MakeInfoString = MakeBLKSInfoString;
		cnk->ch_Saver = BLKSSaver;
		cnk->ch_ParentFile = ft;				/* can be NULL */
		cnk->ch_Modified = FALSE;

		if( !( cnk->ch_Data = AllocVec( sizeof( struct Blockset ), MEMF_ANY|MEMF_CLEAR ) ) )
		{
			FreeBLKSChunk( cnk );
			cnk = NULL;
		}
	}
	return( cnk );
}

/**************************************************************/
VOID FreeBLKSChunk( struct Chunk *cnk )
{
	struct Blockset *bs;
	
	RethinkSWaveBackgrounds();
	RethinkTHAPBackgrounds();

	if( bs = (struct Blockset *)cnk->ch_Data )
	{
		if( bs->bs_Images )
			FreeVec( bs->bs_Images );
		FreeVec( cnk->ch_Data );
	}
	RemoveChunkFromParentFile( cnk );
	FreeVec( cnk );
}

/**************************************************************/

static BOOL MakeBLKSInfoString( struct Chunk *cnk, UBYTE *buf )
{
	UBYTE workbuf[256];
	struct Blockset *bs;

	bs = cnk->ch_Data;

	if( cnk->ch_ParentFile )
		sprintf( workbuf,"Blockset '%s' %d blks %d colours (from %s)",
		bs->bs_Name,
		bs->bs_NumOfBlocks,
		1<<bs->bs_Depth,
		cnk->ch_ParentFile->ft_File );
	else
		sprintf( workbuf,"Blockset '%s' %d blks %d colours",
		bs->bs_Name,
		bs->bs_NumOfBlocks,
		1<<bs->bs_Depth );

	Mystrncpy( buf, workbuf, CHUNKINFOSTRINGSIZE-1 );

	return( TRUE );
}

/**************************************************************/
BOOL BLKSSaver( struct Chunk *cnk, struct IFFHandle *iff )
{
	BOOL success = FALSE;
	ULONG datasize;
	struct Blockset *bs;
	struct BLKSHeader bh;

	D(bug( "BLKSSaver()\n" ) );

	if( cnk->ch_TypeID != ID_BLKS ) return( FALSE ); 				/* sanity check */

	if( bs = cnk->ch_Data )
	{
		datasize = BLKBYTEW * BLKH * bs->bs_Depth * bs->bs_NumOfBlocks;

		if( !(PushChunk( iff, ID_CONK, ID_BLKS, sizeof( struct BLKSHeader ) + datasize ) ) )
		{
			Mystrncpy( bh.blks_Name, bs->bs_Name, BLOCKSETNAMESIZE-1 );
			bh.blks_NumOfBlocks = bs->bs_NumOfBlocks;
			bh.blks_Depth = bs->bs_Depth;
			bh.blks_LayoutWidth = bs->bs_LayoutWidth;
			bh.blks_Flags = bs->bs_Flags;
			bh.blks_PixW = BLKW;
			bh.blks_PixH = BLKH;
			bh.blks_BitMapHeight = bs->bs_BitMapHeight;
			bh.blks_Reserved[0] = 0;
			bh.blks_Reserved[1] = 0;
			bh.blks_Reserved[2] = 0;

			if( WriteChunkBytes( iff, &bh, sizeof( struct BLKSHeader ) ) ==
				sizeof( struct BLKSHeader ) )
			{
				D(bug( "datasize: %ld\n", (LONG)datasize ) );
				if( WriteChunkBytes( iff, bs->bs_Images, datasize ) == datasize )
					success = TRUE;
			}
			PopChunk( iff );
		}
	}

	D(bug( "success: %ld\n", (LONG)success ) );

	return( success );
}

/**************************************************************/

struct Chunk *BLKSLoader( struct IFFHandle *iff, ULONG size,
	struct Filetracker *ft )
{
	BOOL abort = FALSE;
	ULONG datasize;
	struct Chunk *cnk;
	struct Blockset *bs;
	struct BLKSHeader bh;

	if( cnk = CreateBLKSChunk( ft ) )
	{
		bs = cnk->ch_Data;
		if( ReadChunkBytes( iff, &bh, sizeof( struct BLKSHeader ) ) ==
			sizeof( struct BLKSHeader ) )
		{
			Mystrncpy( bs->bs_Name, bh.blks_Name, BLOCKSETNAMESIZE-1 );
			bs->bs_NumOfBlocks = bh.blks_NumOfBlocks;
			bs->bs_Depth = bh.blks_Depth;
			bs->bs_LayoutWidth = bh.blks_LayoutWidth;
			bs->bs_Flags = bh.blks_Flags;
			bs->bs_BitMapHeight = bh.blks_BitMapHeight;

			datasize = BLKBYTEW * BLKH * bs->bs_Depth * bs->bs_NumOfBlocks;
			if( bs->bs_Images = AllocVec( datasize, MEMF_CHIP ) )
			{
				if( ReadChunkBytes( iff, bs->bs_Images, datasize ) != datasize )
					abort = TRUE;
			}
			else
				abort = TRUE;
		}
		else
			abort = TRUE;
	}

	if( abort )
	{
		FreeBLKSChunk( cnk );
		cnk = NULL;
	}

	return( cnk );
}

/**************************************************************/


void UnclippedDrawBlockRP( struct RastPort *rp, WORD x, WORD y,
	struct Blockset *bs, UWORD blk )
{
	static struct BitMap bm;
	UBYTE *p;
	int i;

	assert( rp != NULL );

	if( bs && ( blk < bs->bs_NumOfBlocks ) )
	{
		p = ((UBYTE *)bs->bs_Images) + ( blk * bs->bs_Depth * BLKBYTEW * BLKH );
		bm.BytesPerRow = bs->bs_Depth*BLKBYTEW;
		bm.Rows = BLKH;
		bm.Flags = 0;
		bm.Depth = bs->bs_Depth;
		for( i=0; i<bs->bs_Depth; i++ )
		{
			bm.Planes[i] = p;
			p += BLKBYTEW;
		}
		BltBitMapRastPort( &bm,0,0,rp,x,y,BLKW,BLKH, 0xC0 );
	}
}


/**************************************************************/
struct Chunk *FindBLKSByName( UBYTE *name )
{
	struct Chunk *cnk, *found=NULL;
	struct Blockset *bs;

	for( cnk = (struct Chunk *)chunklist.lh_Head;
		cnk->ch_Node.ln_Succ && !found;
		cnk = (struct Chunk *)cnk->ch_Node.ln_Succ )
	{
		if( cnk->ch_TypeID == ID_BLKS )
		{
			bs = (struct Blockset *)cnk->ch_Data;
			if( !stricmp( bs->bs_Name, name ) )
				found = cnk;
		}
	}

	return found;
}
