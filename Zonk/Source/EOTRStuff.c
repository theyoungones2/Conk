/********************************************************/
//
// EOTRStuff.c
//
/********************************************************/

#define EOTRSTUFF_C

#include <stdio.h>
#include <strings.h>

#include <exec/exec.h>
#include <dos/dos.h>
#include <libraries/iffparse.h>
#include <proto/exec.h>
#include <proto/dos.h>
#include <proto/iffparse.h>
//#include <proto/graphics.h>

#include "global.h"

static BOOL MakeEOTRInfoString( struct Chunk *cnk, UBYTE *buf );


/**************************************************************/

struct Chunk *CreateEOTRChunk( struct Filetracker *ft )
{
	struct Chunk *cnk;
	struct EdgeOfTheRoad *eotr;

	if( cnk = AllocVec( sizeof( struct Chunk ), MEMF_ANY|MEMF_CLEAR ) )
	{
		if( ft )
			ft->ft_ChunkCount++;
		cnk->ch_TypeID = ID_EOTR;
		cnk->ch_Edit = OpenNewEOTRWindow;
		cnk->ch_Free = FreeEOTRChunk;
		cnk->ch_MakeInfoString = MakeEOTRInfoString;
		cnk->ch_Saver = EOTRSaver;
		cnk->ch_ParentFile = ft;				/* can be NULL */
		cnk->ch_Modified = FALSE;

		if( ( cnk->ch_Data = AllocVec( sizeof( struct EdgeOfTheRoad ), MEMF_ANY|MEMF_CLEAR ) ) )
		{
			/* Set up some default values */
			eotr = (struct EdgeOfTheRoad *)cnk->ch_Data;
			Mystrncpy( eotr->Name, defname, EOTRNAMESIZE-1 );
			eotr->NumOfBlocks = 0;
		}
		else
		{
			FreeEOTRChunk( cnk );
			cnk = NULL;
		}
	}
	return( cnk );
}

/**************************************************************/
VOID FreeEOTRChunk( struct Chunk *cnk )
{
	if( cnk->ch_Data )
		FreeVec( cnk->ch_Data );

	RemoveChunkFromParentFile( cnk );
	FreeVec( cnk );
}

/**************************************************************/

static BOOL MakeEOTRInfoString( struct Chunk *cnk, UBYTE *buf )
{
	UBYTE workbuf[256];
	struct EdgeOfTheRoad *eotr;

	eotr = (struct EdgeOfTheRoad *)cnk->ch_Data;

	if( cnk->ch_ParentFile )
		sprintf( workbuf,"EdgeOfTheRoad '%s' (from %s)", eotr->Name,
			cnk->ch_ParentFile->ft_File );
	else
		sprintf( workbuf,"EdgeOfTheRoad '%s'", eotr->Name );

	Mystrncpy( buf, workbuf, CHUNKINFOSTRINGSIZE-1 );

	return( TRUE );
}



/**************************************************************/
BOOL EOTRSaver( struct Chunk *cnk, struct IFFHandle *iff )
{
	BOOL success = FALSE;
	struct EdgeOfTheRoad *eotr;
	struct DiskEOTRHeader gerald;

	D(bug( "EOTRSaver()\n" ) );

	assert( cnk != NULL );
	assert( cnk->ch_TypeID == ID_EOTR );

	if( eotr = (struct EdgeOfTheRoad *)cnk->ch_Data )
	{
		if( !(PushChunk( iff, ID_CONK, ID_EOTR,
			sizeof( struct DiskEOTRHeader ) + eotr->NumOfBlocks ) ) )
		{

			/* form the disk format header */
			PackASCII( eotr->Name, gerald.Name, EOTRNAMESIZE-1 );
			Mystrncpy( gerald.BlocksetName, eotr->BlocksetName,
				BLOCKSETNAMESIZE-1 );
			gerald.NumOfBlocks = eotr->NumOfBlocks;
			gerald.Reserved = 0;

			if( WriteChunkBytes( iff, &gerald, sizeof( struct DiskEOTRHeader ) ) ==
				sizeof( struct DiskEOTRHeader ) )
			{
				/* now write the block values */

				if( WriteChunkBytes( iff, eotr->Data, eotr->NumOfBlocks ) ==
					eotr->NumOfBlocks )
				{
					success = TRUE;
				}
			}
			PopChunk( iff );
		}
	}

	return( success );
}

/**************************************************************/

struct Chunk *EOTRLoader( struct IFFHandle *iff, ULONG size,
	struct Filetracker *ft )
{
	struct Chunk *cnk;
	BOOL allok = FALSE;
	struct EdgeOfTheRoad *eotr;
	struct DiskEOTRHeader gerald;

	if( cnk = CreateEOTRChunk( ft ) )
	{
		eotr = (struct EdgeOfTheRoad *)cnk->ch_Data;
		if( ReadChunkBytes( iff, &gerald, sizeof( gerald ) ) ==
			sizeof( gerald ) )
		{
			UnpackASCII( gerald.Name, eotr->Name, EOTRNAMESIZE-1 );
			Mystrncpy( eotr->BlocksetName, gerald.BlocksetName,
				BLOCKSETNAMESIZE-1 );
			eotr->NumOfBlocks = gerald.NumOfBlocks;

			if( ReadChunkBytes( iff, eotr->Data, eotr->NumOfBlocks ) ==
				eotr->NumOfBlocks )
			{
				allok = TRUE;
			}
		}
	}

	if( !allok )
	{
		if( cnk )
			FreeEOTRChunk( cnk );
		cnk = NULL;
	}

	return( cnk );
}

