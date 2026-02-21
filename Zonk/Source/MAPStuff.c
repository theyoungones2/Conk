/********************************************************/
//
// MAPStuff.c
//
/********************************************************/

#define MAPSTUFF_C

#include <stdio.h>
#include <strings.h>
#include <assert.h>

#include <exec/exec.h>
#include <dos/dos.h>
#include <graphics/gfx.h>
#include <graphics/scale.h>
#include <libraries/iffparse.h>
#include <proto/exec.h>
#include <proto/dos.h>
#include <proto/graphics.h>
#include <proto/iffparse.h>

#include "global.h"


/* map format on disk - MAPHeader struct followed by mapdata */

struct MAPHeader
{
	UBYTE Name[ MAPNAMESIZE ];		/* _NOT_ packed! */
	UWORD Width;									/* width & height in blocks */
	UWORD Height;
};


static BOOL MakeMAPInfoString( struct Chunk *cnk, UBYTE *buf );

/**************************************************************/

struct Chunk *CreateMAPChunk( struct Filetracker *ft )
{
	struct Chunk *cnk;
	if( cnk = AllocVec( sizeof( struct Chunk ), MEMF_ANY|MEMF_CLEAR ) )
	{
		if( ft )
			ft->ft_ChunkCount++;
		cnk->ch_TypeID = ID_MAP;
		cnk->ch_Edit = NULL;
		cnk->ch_Free = FreeMAPChunk;
		cnk->ch_MakeInfoString = MakeMAPInfoString;
		cnk->ch_Saver = MAPSaver;
		cnk->ch_ParentFile = ft;				/* can be NULL */
		cnk->ch_Modified = FALSE;

		if( !( cnk->ch_Data = AllocVec( sizeof( struct Map ), MEMF_ANY|MEMF_CLEAR ) ) )
		{
			FreeMAPChunk( cnk );
			cnk = NULL;
		}
	}
	return( cnk );
}

/**************************************************************/
VOID FreeMAPChunk( struct Chunk *cnk )
{
	struct Map *map;

	RethinkSWaveBackgrounds();
	RethinkTHAPBackgrounds();

	if( map = (struct Map *)cnk->ch_Data )
	{
		if( map->map_Data )
			FreeVec( map->map_Data );
		FreeVec( cnk->ch_Data );
	}
	RemoveChunkFromParentFile( cnk );
	FreeVec( cnk );

}

/**************************************************************/

static BOOL MakeMAPInfoString( struct Chunk *cnk, UBYTE *buf )
{
	UBYTE workbuf[256];
	struct Map *map;

	map = cnk->ch_Data;

	if( cnk->ch_ParentFile )
		sprintf( workbuf,"Map '%s' %dx%d (from %s)",
		map->map_Name,
		map->map_Width,
		map->map_Height,
		cnk->ch_ParentFile->ft_File );
	else
		sprintf( workbuf,"Map '%s' %dx%d",
		map->map_Name,
		map->map_Width,
		map->map_Height );

	Mystrncpy( buf, workbuf, CHUNKINFOSTRINGSIZE-1 );

	return( TRUE );
}

/**************************************************************/
BOOL MAPSaver( struct Chunk *cnk, struct IFFHandle *iff )
{
	BOOL success = FALSE;
	ULONG datasize;
	struct Map *map;
	struct MAPHeader mh;

	assert( cnk->ch_TypeID == ID_MAP );
	D(bug( "MAPSaver()\n" ) );

	if( map = (struct Map *)cnk->ch_Data )
	{
		datasize = map->map_Width * map->map_Height * sizeof( UWORD );

		if( !(PushChunk( iff, ID_CONK, ID_MAP, sizeof( struct MAPHeader ) + datasize ) ) )
		{
			Mystrncpy( mh.Name, map->map_Name, MAPNAMESIZE-1 );
			mh.Width = map->map_Width;
			mh.Height = map->map_Height;

			if( WriteChunkBytes( iff, &mh, sizeof( struct MAPHeader ) ) ==
				sizeof( struct MAPHeader ) )
			{
				if( WriteChunkBytes( iff, map->map_Data, datasize ) == datasize )
					success = TRUE;
			}
			PopChunk( iff );
		}
	}

	return( success );
}

/**************************************************************/

struct Chunk *MAPLoader( struct IFFHandle *iff, ULONG size,
	struct Filetracker *ft )
{
	BOOL abort = FALSE;
	ULONG datasize;
	struct Chunk *cnk;
	struct Map *map;
	struct MAPHeader mh;

	if( cnk = CreateMAPChunk( ft ) )
	{
		map = (struct Map *)cnk->ch_Data;
		if( ReadChunkBytes( iff, &mh, sizeof( struct MAPHeader ) ) ==
			sizeof( struct MAPHeader ) )
		{
			strcpy( map->map_Name, mh.Name );
			map->map_Width = mh.Width;
			map->map_Height = mh.Height;
			datasize = map->map_Width * map->map_Height * sizeof( UWORD );
			if( map->map_Data = AllocVec( datasize, MEMF_ANY ) )
			{
				if( ReadChunkBytes( iff, map->map_Data, datasize ) != datasize )
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
		FreeMAPChunk( cnk );
		cnk = NULL;
	}

	return( cnk );
}


/**************************************************************/

VOID DrawMapBM( struct DrawMapBMArgs *dmargs )
{
	struct BitMap blkbm;
	UWORD i,pixx,pixy,scaleblkw,scaleblkh,blk;
	WORD mapx,mapy;
	UWORD	*mapline;
	UBYTE *p;
	struct BitScaleArgs bsa;

	bsa.bsa_SrcX = 0;
	bsa.bsa_SrcY = 0;
	bsa.bsa_SrcWidth = BLKW;
	bsa.bsa_SrcHeight = BLKH;
	bsa.bsa_XSrcFactor = BLKW;
	bsa.bsa_YSrcFactor = BLKH;
//	bsa.bsa_DestX = 0;
//	bsa.bsa_DestY = 0;
	bsa.bsa_DestWidth = BLKW * dmargs->ZoomFactor;
	bsa.bsa_DestHeight = BLKH * dmargs->ZoomFactor;
	bsa.bsa_XDestFactor = BLKW * dmargs->ZoomFactor;
	bsa.bsa_YDestFactor = BLKH * dmargs->ZoomFactor;
	bsa.bsa_SrcBitMap = &blkbm;
	bsa.bsa_DestBitMap = dmargs->bm;
	bsa.bsa_Flags = 0;

	blkbm.BytesPerRow = BLKBYTEW * dmargs->Blockset->bs_Depth;
	blkbm.Rows = BLKH;
	blkbm.Flags = 0;
	blkbm.Depth = dmargs->Blockset->bs_Depth;

	scaleblkw = BLKW * dmargs->ZoomFactor;
	scaleblkh = BLKH * dmargs->ZoomFactor;

	pixy = 0;
	mapy = dmargs->MapY;
	while( pixy < dmargs->ViewHeight &&
		( pixy + bsa.bsa_DestHeight ) < dmargs->BMHeight )
	{


		pixx = 0;
		mapx = dmargs->MapX;
		mapline = dmargs->Map->map_Data + mapx + ( mapy * dmargs->Map->map_Width );

		while( pixx < dmargs->ViewWidth &&
			( pixx + bsa.bsa_DestWidth ) < dmargs->BMWidth )
		{

			bsa.bsa_DestX = pixx;
			bsa.bsa_DestY = pixy;

			if( mapx >= 0 && mapx < dmargs->Map->map_Width &&
				mapy >= 0 && mapy < dmargs->Map->map_Height )
			{
				blk = *mapline;
			}
			else
				blk = 2;
			mapline++;

			p = (UBYTE *)dmargs->Blockset->bs_Images;
			p += ( BLKBYTEW * (LONG)dmargs->Blockset->bs_Depth * (LONG)blk * BLKH );

			for( i=0; i < blkbm.Depth; i++ )
			{
				blkbm.Planes[i] = p;
				p += 2;
			}

			if( dmargs->ZoomFactor == 1 )
				BltBitMap( &blkbm,0,0,dmargs->bm,pixx,pixy,BLKW,BLKH,0xC0,0xFF,NULL );
			else
				BitMapScale( &bsa );

			pixx += scaleblkw;
			mapx++;
			blk++;
		}
		pixy += scaleblkh;
		mapy++;
	}
}


/**************************************************************/
void ScrollMapBM( struct DrawMapBMArgs *dmargs, WORD dx, WORD dy, UWORD qual )
{

	if( qual & IEQUALIFIER_CONTROL )
	{
		if( dx < 0 )
			dmargs->MapX = 0;
		else if( dx > 0 )
			dmargs->MapX = dmargs->Map->map_Width - ( dmargs->ViewWidth / BLKW );

		if( dy < 0 )
			dmargs->MapY = 0;
		else if( dy > 0 )
			dmargs->MapY = dmargs->Map->map_Height - ( dmargs->ViewHeight / BLKH );

		DrawMapBM( dmargs );
	}
	else
	{
		if( (qual & IEQUALIFIER_RALT) || (qual & IEQUALIFIER_LALT) )
		{
			if( dx > 0 )
				dmargs->MapX += 4;
			else if( dx < 0 )
				dmargs->MapX -= 4;

			if( dy > 0 )
				dmargs->MapY += 4;
			else if( dy < 0 )
				dmargs->MapY -= 4;
		}
		else if( (qual & IEQUALIFIER_RSHIFT) || (qual & IEQUALIFIER_LSHIFT) )
		{
			if( dx > 0 )
				dmargs->MapX += ( ( dmargs->ViewWidth / BLKW ) - 1 );
			else if( dx < 0 )
				dmargs->MapX -= ( ( dmargs->ViewWidth / BLKW ) - 1 );

			if( dy > 0 )
				dmargs->MapY += ( ( dmargs->ViewHeight / BLKH ) - 1 );
			else if( dy < 0 )
				dmargs->MapY -= ( ( dmargs->ViewHeight / BLKH ) - 1 );
		}
		else
		{
			if( dx > 0 )
				dmargs->MapX += 1;
			else if( dx < 0 )
				dmargs->MapX -= 1;

			if( dy > 0 )
				dmargs->MapY += 1;
			else if( dy < 0 )
				dmargs->MapY -= 1;
		}

		DrawMapBM( dmargs );
	}
}

/**************************************************************/
struct Chunk *FindMAPByName( UBYTE *name )
{
	struct Chunk *cnk, *found=NULL;
	struct Map *map;

	for( cnk = (struct Chunk *)chunklist.lh_Head;
		cnk->ch_Node.ln_Succ && !found;
		cnk = (struct Chunk *)cnk->ch_Node.ln_Succ )
	{
		if( cnk->ch_TypeID == ID_MAP )
		{
			map = (struct Map *)cnk->ch_Data;
			if( !stricmp( map->map_Name, name ) )
				found = cnk;
		}
	}

	return found;
}
