/********************************************************/
//
// TWAVStuff.c
//
/********************************************************/

#define TWAVSTUFF_C

#include <stdio.h>
#include <strings.h>
#include <assert.h>

#include <exec/exec.h>
#include <dos/dos.h>
#include <libraries/iffparse.h>
#include <proto/exec.h>
#include <proto/dos.h>
#include <proto/iffparse.h>

#include "global.h"


/* TWAV chunk structure: */
/* First longword is number of attackwaves in chunk, */
/* then comes array of DiskFormatTimedWave structs. */

struct DiskFormatTimedWave
{
	UBYTE PackedEvent[ GENERICNAMEPACKSIZE ];
	UWORD				Time;
	LONG				X;
	LONG				Y;
	UBYTE				Type;
	UBYTE				Flags;
};



static BOOL MakeTWAVInfoString( struct Chunk *cnk, UBYTE *buf );

/**************************************************************/

struct Chunk *CreateTWAVChunk( struct Filetracker *ft )
{
	struct Chunk *cnk;

	if( cnk = AllocVec( sizeof( struct Chunk ), MEMF_ANY|MEMF_CLEAR ) )
	{
		if( ft )
			ft->ft_ChunkCount++;
		cnk->ch_TypeID = ID_TWAV;
		cnk->ch_Edit = OpenNewTimedWaveWindow;
		cnk->ch_Free = FreeTWAVChunk;
		cnk->ch_MakeInfoString = MakeTWAVInfoString;
		cnk->ch_Saver = TWAVSaver;
		cnk->ch_ParentFile = ft;
		cnk->ch_Modified = FALSE;
		/* ch_DataList is used as a queue of TimedWaves */
		NewList( &cnk->ch_DataList );
	}
	return( cnk );
}

/**************************************************************/

static BOOL MakeTWAVInfoString( struct Chunk *cnk, UBYTE *buf )
{
	UBYTE workbuf[256];

	if( cnk->ch_ParentFile )
		sprintf( workbuf,"Timed Attackwaves (from %s)",
			cnk->ch_ParentFile->ft_File );
	else
		sprintf( workbuf,"Timed Attackwaves" );

	Mystrncpy( buf, workbuf, CHUNKINFOSTRINGSIZE-1 );

	return( TRUE );
}

/**************************************************************/

VOID FreeTWAVChunk( struct Chunk *cnk )
{
	struct Node *foo;

	if( cnk->ch_TypeID == ID_TWAV )
	{
		while( !IsListEmpty( &cnk->ch_DataList ) )
		{
			foo = cnk->ch_DataList.lh_Head;
			Remove( foo );
			FreeVec( foo );
		}

		RemoveChunkFromParentFile( cnk );
		FreeVec( cnk );
	}
}




/**************************************************************/

BOOL TWAVSaver( struct Chunk *cnk, struct IFFHandle *iff )
{
	ULONG count;
	struct TimedWave *twave;
	struct DiskFormatTimedWave dftw;
	BOOL abort = FALSE;

	assert( cnk != NULL );
	assert( cnk->ch_TypeID == ID_TWAV );
	D(bug( "TWAVSaver()\n") );

	if( !(PushChunk( iff, ID_CONK, ID_TWAV, IFFSIZE_UNKNOWN ) ) )
	{
		/* write number of waves */
		count = CountNodes( &cnk->ch_DataList );
		if( WriteChunkBytes( iff, &count, sizeof( ULONG ) ) != sizeof( ULONG ) )
			abort = TRUE;

		for( twave = (struct TimedWave *)cnk->ch_DataList.lh_Head;
			twave->Noddy.ln_Succ && !abort;
			twave = (struct TimedWave *)twave->Noddy.ln_Succ )
		{
			PackASCII( twave->Event, dftw.PackedEvent, GENERICNAMESIZE-1 );
			dftw.Time = twave->Time;
			dftw.X = twave->X;
			dftw.Y = twave->Y;
			dftw.Type = twave->Type;
			dftw.Flags = twave->Flags;
			if( WriteChunkBytes( iff, &dftw, sizeof( struct DiskFormatTimedWave ) ) !=
				sizeof( struct DiskFormatTimedWave ) )
			{
				abort = TRUE;
			}
		}
		PopChunk( iff );
	}

	if( abort )
		return( FALSE );
	else
		return( TRUE );
}


/**************************************************************/

struct Chunk *TWAVLoader( struct IFFHandle *iff, ULONG size,
	struct Filetracker *ft )
{
	BOOL abort = FALSE;
	struct Chunk *cnk;
	ULONG count;
	struct DiskFormatTimedWave dftw;
	struct TimedWave *twave;

	if( cnk = CreateTWAVChunk( ft ) )
	{
		if( ReadChunkBytes( iff, &count, 4) != 4 )
			abort = TRUE;

		while( count-- && !abort )
		{
			if( ReadChunkBytes( iff, &dftw, sizeof( struct DiskFormatTimedWave) ) ==
				sizeof( struct DiskFormatTimedWave) )
			{
				if( twave = AllocVec( sizeof( struct TimedWave ), MEMF_ANY|MEMF_CLEAR ) )
				{
					UnpackASCII( dftw.PackedEvent, twave->Event,
						GENERICNAMESIZE-1 );
					twave->Time = dftw.Time;
					twave->X = dftw.X;
					twave->Y = dftw.Y;
					twave->Type = dftw.Type;
					twave->Flags = dftw.Flags;
					EnqueueTimedWave( &cnk->ch_DataList, twave );
				}
				else
					abort = TRUE;
			}
			else
				abort = TRUE;
		}
	}

	if( abort )
	{
		FreeTWAVChunk( cnk );
		cnk = NULL;
	}

	return( cnk );
}


/**************************************************************/
//
// Enqueue a TimedWave according to its Time field.
//

VOID EnqueueTimedWave( struct List *twavelist,
	struct TimedWave *twave )
{
	struct TimedWave *prevtwave;

	prevtwave = (struct TimedWave *)twavelist->lh_Head;
	while( prevtwave->Noddy.ln_Succ &&
		prevtwave->Time < twave->Time )
	{
		prevtwave = (struct TimedWave *)prevtwave->Noddy.ln_Succ;
	}
	Insert( twavelist, &twave->Noddy, prevtwave->Noddy.ln_Pred );
}
