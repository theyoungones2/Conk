/********************************************************/
//
// SWAVStuff.c
//
/********************************************************/

#define SWAVSTUFF_C

#include <stdio.h>
#include <string.h>
//#include <stdarg.h>
//#include <math.h>
#include <assert.h>

#include <dos/dos.h>
#include <dos/dostags.h>
#include <exec/exec.h>
#include <intuition/intuition.h>
#include <intuition/imageclass.h>
#include <graphics/gfxbase.h>
#include <graphics/gfx.h>
#include <graphics/gfxmacros.h>
#include <libraries/gadtools.h>
#include <libraries/diskfont.h>
#include <libraries/asl.h>
//#include <libraries/iffparse.h>

#include <proto/exec.h>
#include <proto/dos.h>
#include <proto/intuition.h>
#include <proto/graphics.h>
#include <proto/diskfont.h>
#include <proto/asl.h>
#include <clib/alib_protos.h>
#include <proto/gadtools.h>
#include <proto/iffparse.h>

#include "hoopy.h"
#include "global.h"

static BOOL MakeSWAVInfoString( struct Chunk *cnk, UBYTE *buf );

static UWORD NumberZScrollWaves( struct Chunk *cnk );
static struct ZScrollWave *FindNextWave( struct Chunk *cnk, UWORD currentpos,
	BOOL leftflag );
static UWORD ShittySortZScrollWaves( struct Chunk *cnk, BOOL leftflag );

/**************************************************************/

struct Chunk *CreateSWAVChunk( struct Filetracker *ft )
{
	struct Chunk *cnk;
	BOOL abort;

	abort = FALSE;
	if( cnk = AllocVec( sizeof( struct Chunk ), MEMF_ANY|MEMF_CLEAR ) )
	{
		if( ft )
			ft->ft_ChunkCount++;
		cnk->ch_TypeID = ID_SWAV;
		cnk->ch_Edit = OpenNewSWaveWindow;
		cnk->ch_Free = FreeSWAVChunk;
		cnk->ch_MakeInfoString = MakeSWAVInfoString;
		cnk->ch_Saver = SWAVSaver;
		cnk->ch_ParentFile = ft;				/* can be NULL */
		cnk->ch_Modified = FALSE;
		NewList( &cnk->ch_DataList );
	}
	return( cnk );
}

/**************************************************************/

static BOOL MakeSWAVInfoString( struct Chunk *cnk, UBYTE *buf )
{
	UBYTE workbuf[256];

	if( cnk->ch_ParentFile )
		sprintf( workbuf,"%d ScrollWaves (from %s)",
			CountNodes( &cnk->ch_DataList),
			cnk->ch_ParentFile->ft_File );
	else
		sprintf( workbuf,"%d Scrollwaves", CountNodes( &cnk->ch_DataList) );

	Mystrncpy( buf, workbuf, CHUNKINFOSTRINGSIZE-1 );

	return( TRUE );
}


/**************************************************************/
// Free a SWAV chunk and all its associated crap.

VOID FreeSWAVChunk( struct Chunk *cnk )
{
	struct ZScrollWave *zsw;

	if( cnk->ch_TypeID == ID_SWAV )
	{
		while( !IsListEmpty( &cnk->ch_DataList ) )
		{
			zsw = (struct ZScrollWave *)cnk->ch_DataList.lh_Head;
			Remove( (struct Node *)&zsw->zsw_Node );
			FreeZScrollWave( zsw );
		}

		RemoveChunkFromParentFile( cnk );
		FreeVec( cnk );
	}
}


/**************************************************************/

struct Chunk *SWAVLoader( struct IFFHandle *iff, ULONG size,
	struct Filetracker *ft )
{
	struct Chunk *cnk;
	struct ZScrollWave *zsw;
	struct DFScrollWave dsf;
	struct DFSWAVHeader swavheader;
	BOOL allok;
	UWORD count;

	allok = TRUE;
	if( cnk = CreateSWAVChunk( ft ) )
	{
		if( ReadChunkBytes( iff, &swavheader, sizeof( struct DFSWAVHeader ) ) ==
			sizeof( struct DFSWAVHeader ) )
		{
			count = swavheader.NumOfWaves;
			D(bug("%ld ScrollWaves\n", count ) );
			while( count-- && allok )
			{
				if( ReadChunkBytes( iff, &dsf, sizeof( struct DFScrollWave ) ) ==
					sizeof( struct DFScrollWave ) )
				{
					if( zsw = AllocZScrollWave() )
					{

						/* Num, LfSucc, RtSucc only used for sorting and saving */
						UnpackASCII( dsf.dsf_Name, zsw->zsw_Name, GENERICNAMESIZE-1 );
						zsw->zsw_Flags = dsf.dsf_Flags;
						zsw->zsw_AppearCount = dsf.dsf_AppearCount;
						zsw->zsw_XPos = dsf.dsf_XPos;
						zsw->zsw_YPos = dsf.dsf_YPos;
						zsw->zsw_LfTrigger = dsf.dsf_LfTrigger;
						zsw->zsw_RtTrigger = dsf.dsf_RtTrigger;
						zsw->zsw_TrigFlags = 3;
						if( zsw->zsw_LfTrigger == 0xFFFF )
						{
							zsw->zsw_TrigFlags &= ~2;
							zsw->zsw_LfTrigger = zsw->zsw_XPos - 32;
						}
						if( zsw->zsw_RtTrigger == 0xFFFF )
						{
							zsw->zsw_TrigFlags &= ~1;
							zsw->zsw_RtTrigger = zsw->zsw_XPos + 32;
						}
						AddTail( &cnk->ch_DataList, (struct Node *)zsw );
					}
					else
						allok = FALSE;
				}
				else
					allok = FALSE;
			}
		}
		else
			allok = FALSE;
	}

	if( !allok )
	{
		FreeSWAVChunk( cnk );
		cnk = NULL;
	}

	return( cnk );
}


/**************************************************************/

BOOL SWAVSaver( struct Chunk *cnk, struct IFFHandle *iff )
{
	struct ZScrollWave *zsw;
	struct DFScrollWave dsf;
	struct DFSWAVHeader swavheader;
	LONG size;
	BOOL allok;

	assert( cnk != NULL );
	assert( cnk->ch_TypeID == ID_SWAV );

	D(bug( "SWAVSaver()\n") );

	swavheader.NumOfWaves = CountNodes( &cnk->ch_DataList );
	swavheader.LfHead = ShittySortZScrollWaves( cnk, TRUE );
	swavheader.RtHead = ShittySortZScrollWaves( cnk, FALSE );
	swavheader.Reserved = 0;

	size = sizeof( struct DFSWAVHeader ) +
		( swavheader.NumOfWaves * sizeof( struct DFScrollWave ) );

	allok = TRUE;
	if( !(PushChunk( iff, ID_CONK, ID_SWAV, size ) ) )
	{
		/* write chunk header */
		if( WriteChunkBytes( iff, &swavheader, sizeof( struct DFSWAVHeader ) ) ==
			sizeof( struct DFSWAVHeader ) )
		{
			for( zsw = (struct ZScrollWave *)cnk->ch_DataList.lh_Head;
				zsw->zsw_Node.mln_Succ && allok;
				zsw = (struct ZScrollWave *)zsw->zsw_Node.mln_Succ )
			{
				dsf.dsf_LfSucc = zsw->zsw_LfSucc;
				dsf.dsf_RtSucc = zsw->zsw_RtSucc;
				PackASCII( zsw->zsw_Name, dsf.dsf_Name, GENERICNAMESIZE-1 );
				dsf.dsf_Flags = zsw->zsw_Flags;
				dsf.dsf_AppearCount = zsw->zsw_AppearCount;
				dsf.dsf_LfTrigger = zsw->zsw_LfTrigger;
				dsf.dsf_RtTrigger = zsw->zsw_RtTrigger;
				if( !(zsw->zsw_TrigFlags & 2 ) )
					dsf.dsf_LfTrigger = 0xFFFF;
				if( !(zsw->zsw_TrigFlags & 1 ) )
					dsf.dsf_RtTrigger = 0xFFFF;
				dsf.dsf_XPos = zsw->zsw_XPos;
				dsf.dsf_YPos = zsw->zsw_YPos;
				if( WriteChunkBytes( iff, &dsf, sizeof( struct DFScrollWave ) ) !=
					sizeof( struct DFScrollWave ) )
				{
					allok = FALSE;
				}
			}
		}
		else
			allok = FALSE;

		PopChunk( iff );
	}

	return( allok );
}


/**************************************************************/

struct ZScrollWave *AllocZScrollWave( VOID )
{
	struct ZScrollWave *zsw;

	zsw = AllocVec( sizeof( struct ZScrollWave ), MEMF_ANY|MEMF_CLEAR );
	return( zsw );
}


/**************************************************************/
VOID FreeZScrollWave( struct ZScrollWave *zsw )
{
	FreeVec( zsw );
}


/**************************************************************/
// NumberZScrollWaves()
//
// Go through a SWAV chunk list of ZScrollWaves and fill in the
// zsw_Num field with the nodes position in the list.
//
// returns the number of nodes in the list.

static UWORD NumberZScrollWaves( struct Chunk *cnk )
{
	struct ZScrollWave *zsw;
	UWORD num;

	assert( cnk != NULL );
	assert( cnk->ch_TypeID == ID_SWAV );

	num = 0;
	for( zsw = (struct ZScrollWave *)cnk->ch_DataList.lh_Head;
		zsw->zsw_Node.mln_Succ;
		zsw = (struct ZScrollWave *)zsw->zsw_Node.mln_Succ )
	{
		zsw->zsw_Num = num++;
	}
	return( num );
}

/**************************************************************/
//FindNextWave()
//
// Pick the wave closest to the current map position.
// Set leftflag TRUE to search lefttriggers, FALSE for right.
// Waves without triggers (zsw_??Trigger == 0xFFFF) and waves
// with zsw_Num == 0xFFFF are excluded from the search.
// Returns NULL if no wave found.

static struct ZScrollWave *FindNextWave( struct Chunk *cnk, UWORD currentpos,
	BOOL leftflag )
{
	struct ZScrollWave *zsw, *closest;
	LONG delta, mindelta, pos;
	UWORD trigger;

	assert( cnk != NULL );
	assert( cnk->ch_TypeID == ID_SWAV );

	closest = NULL;
	mindelta = 0xFFFF;
	pos = (LONG)currentpos;

	for( zsw = (struct ZScrollWave *)cnk->ch_DataList.lh_Head;
		zsw->zsw_Node.mln_Succ;
		zsw = (struct ZScrollWave *)zsw->zsw_Node.mln_Succ )
	{
		if( leftflag )
		{
			trigger = zsw->zsw_LfTrigger;
			if( zsw->zsw_Num != 0xFFFF && (zsw->zsw_TrigFlags & 2) )
			{
				delta = (LONG)trigger - pos;
				if( (delta >= 0) && (delta < mindelta) )
				{
					mindelta = delta;
					closest = zsw;
				}
			}
		}
		else
		{
			trigger = zsw->zsw_RtTrigger;
			if( zsw->zsw_Num != 0xFFFF && (zsw->zsw_TrigFlags & 1) )
			{
				delta = (LONG)trigger - pos;
				if( (delta >= 0) && (delta < mindelta) )
				{
					mindelta = delta;
					closest = zsw;
				}
			}
		}
	}
	return( closest );
}


/**************************************************************/

static UWORD ShittySortZScrollWaves( struct Chunk *cnk, BOOL leftflag )
{
	struct ZScrollWave *zsw, *nextzsw;
	UWORD pos, head, trigger;

	NumberZScrollWaves( cnk );
	pos = 0;
	if( zsw = FindNextWave( cnk, pos, leftflag ) )
	{
		head = zsw->zsw_Num;
		pos = leftflag ? zsw->zsw_LfTrigger : zsw->zsw_RtTrigger;
		zsw->zsw_Num = 0xFFFF;		/* so it won't get picked again */

		while( nextzsw = FindNextWave( cnk, pos, leftflag ) )
		{
			if( leftflag )
			{
				trigger = zsw->zsw_LfTrigger;
				if( zsw->zsw_TrigFlags & 2 )
				{
					zsw->zsw_LfSucc = nextzsw->zsw_Num;
					nextzsw->zsw_Num = 0xFFFF;		/* so it won't get picked again */
					zsw = nextzsw;
					pos = trigger;
				}
			}
			else
			{
				trigger = zsw->zsw_RtTrigger;
				if( zsw->zsw_TrigFlags & 1 )
				{
					zsw->zsw_RtSucc = nextzsw->zsw_Num;
					nextzsw->zsw_Num = 0xFFFF;		/* so it won't get picked again */
					zsw = nextzsw;
					pos = trigger;
				}
			}
		}
		/* terminate list */
		if( leftflag )
			zsw->zsw_LfSucc = 0xFFFF;
		else
			zsw->zsw_RtSucc = 0xFFFF;
	}
	else
		head = 0xFFFF;		/* empty left list */
	return( head );
}

