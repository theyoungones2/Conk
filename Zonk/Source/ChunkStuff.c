/********************************************************/
//
// ChunkStuff.c
//
/********************************************************/

#define CHUNKSTUFF_C

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

struct List chunklist;
struct List filelist;

static BOOL MakeCMAPInfoString( struct Chunk *cnk, UBYTE *buf );
static BOOL ListCMAP( struct Chunk *cnk );


/**************************************************************/
VOID InitChunkStorage( VOID )
{
	NewList( &chunklist );
	NewList( &filelist );
}


/**************************************************************/
//
//
//

struct Chunk *CreateGenericChunk( struct Filetracker *ft, ULONG typeid )
{
	struct Chunk *cnk;
	if( cnk = AllocVec( sizeof( struct Chunk ), MEMF_ANY|MEMF_CLEAR ) )
	{
		if( ft )
			ft->ft_ChunkCount++;
		cnk->ch_TypeID = typeid;
		cnk->ch_Edit = NULL;
		cnk->ch_Free = FreeGenericChunk;
		cnk->ch_MakeInfoString = NULL;
		cnk->ch_Saver = GenericChunkSaver;
		cnk->ch_ParentFile = ft;				/* can be NULL */
		cnk->ch_Modified = FALSE;

		cnk->ch_Data = NULL;
		cnk->ch_Size = 0;

	}
	return( cnk );
}

/**************************************************************/

VOID FreeGenericChunk( struct Chunk *cnk )
{
	if( cnk->ch_Data )
		FreeVec( cnk->ch_Data );
	RemoveChunkFromParentFile( cnk );
	FreeVec( cnk );
}

/**************************************************************/
//

struct Chunk *GenericChunkLoader( struct IFFHandle *iff, ULONG size,
	struct Filetracker *ft, ULONG typeid )
{
	BOOL abort = FALSE;
	struct Chunk *cnk;

	if( cnk = CreateGenericChunk( ft, typeid ) )
	{
		if( cnk->ch_Data = AllocVec( size, MEMF_ANY ) )
		{
			cnk->ch_Size = size;
			if( ReadChunkBytes( iff, cnk->ch_Data, size) != size )
				abort = TRUE;
		}
		else
			abort = TRUE;

		if( abort )
		{
			FreeGenericChunk( cnk );
			cnk = NULL;
		}
	}

	return( cnk );
}




/**************************************************************/

BOOL GenericChunkSaver( struct Chunk *cnk, struct IFFHandle *iff )
{
	BOOL success = FALSE;

	if( !(PushChunk( iff, ID_CONK, cnk->ch_TypeID, cnk->ch_Size ) ) )
	{
		if( WriteChunkBytes( iff, cnk->ch_Data, cnk->ch_Size ) == cnk->ch_Size )
			success = TRUE;
		PopChunk( iff );
	}
	return( success );
}




/**************************************************************/
//
// FreeAllChunks()
// Just what the name says. Ignores chunk locking.
//

VOID FreeAllChunks( VOID )
{
	struct Chunk *cnk;
	struct Filetracker *ft;

	while( !IsListEmpty( &chunklist ) )
	{
		cnk = (struct Chunk *)chunklist.lh_Head;
		Remove( (struct Node *)cnk );
//		FreeChunk( cnk );
		(*cnk->ch_Free)(cnk);
	}

	while( !IsListEmpty( &filelist ) )
	{
		ft = (struct Filetracker *)filelist.lh_Head;
		Remove( (struct Node *)ft );
		FreeVec( ft );
	}
}


/**************************************************************/


VOID RemoveChunkFromParentFile( struct Chunk *cnk )
{
	struct Filetracker *ft;

	if( ft = cnk->ch_ParentFile )
	{
		if( ft->ft_ChunkCount ) ft->ft_ChunkCount--;
		if( !ft->ft_ChunkCount )
		{
			Remove( &ft->ft_Node );
			FreeVec( ft );
		}
	}
}

/**************************************************************/
//
//  LockChunk() - stops a chunk from being freed via normal channels.
//  Must be paired with UnlockChunk(). Calls can be nested.
//  See also FreeAllChunks() - it ignores locking.

void LockChunk( struct Chunk *cnk )
{
	assert( cnk != NULL );
	cnk->ch_LockCount++;
	return;
}


/**************************************************************/
//
//

VOID UnlockChunk( struct Chunk *cnk )
{
	assert( cnk != NULL );
	assert( cnk->ch_LockCount > 0 );

	if( cnk )
	{
		if( cnk->ch_LockCount > 0 )
			--cnk->ch_LockCount;
	}
}


/**************************************************************/

struct Chunk *CreateCMAPChunk( struct Filetracker *ft )
{
	struct Chunk *cnk;
	if( cnk = AllocVec( sizeof( struct Chunk ), MEMF_ANY|MEMF_CLEAR ) )
	{
		if( ft )
			ft->ft_ChunkCount++;
		cnk->ch_TypeID = ID_CMAP;
		cnk->ch_Edit = ListCMAP;
		cnk->ch_Free = FreeCMAPChunk;
		cnk->ch_MakeInfoString = MakeCMAPInfoString;
		cnk->ch_Saver = CMAPSaver;
		cnk->ch_ParentFile = ft;				/* can be NULL */
		cnk->ch_Modified = FALSE;

		cnk->ch_Data = AllocVec( sizeof( struct Palette ), MEMF_ANY|MEMF_CLEAR );
		if( cnk->ch_Data )
			( ( struct Palette *)cnk->ch_Data )->pl_Count = 0;
		else
		{
			FreeCMAPChunk( cnk );
			cnk = NULL;
		}
	}
	return( cnk );
}

/**************************************************************/
VOID FreeCMAPChunk( struct Chunk *cnk )
{
	FreeGenericChunk( cnk );
}

/**************************************************************/

static BOOL MakeCMAPInfoString( struct Chunk *cnk, UBYTE *buf )
{
	UBYTE workbuf[256];
	UWORD count;

	count = ((struct Palette *)cnk->ch_Data)->pl_Count;

	if( cnk->ch_ParentFile )
		sprintf( workbuf,"Palette, %d colours (from %s)", count,
			cnk->ch_ParentFile->ft_File );
	else
		sprintf( workbuf,"Palette, %d colours", count );

	Mystrncpy( buf, workbuf, CHUNKINFOSTRINGSIZE-1 );

	return( TRUE );
}

/**************************************************************/
BOOL CMAPSaver( struct Chunk *cnk, struct IFFHandle *iff )
{
	BOOL success = FALSE;
	ULONG size;
	struct Palette *pal;

	if( cnk->ch_TypeID != ID_CMAP ) return( FALSE ); 				/* sanity check */

	pal = (struct Palette *)cnk->ch_Data;
	size = pal->pl_Count * 3;

	if( !(PushChunk( iff, ID_CONK, ID_CMAP, pal->pl_Count*3 ) ) )
	{
		if( WriteChunkBytes( iff, &pal->pl_Colours, size ) == size )
			success = TRUE;
		PopChunk( iff );
	}

	return( success );
}

/**************************************************************/

struct Chunk *CMAPLoader( struct IFFHandle *iff, ULONG size,
	struct Filetracker *ft )
{
	BOOL abort = FALSE;
	struct Chunk *cnk;
	struct Palette *pal;

	if( cnk = CreateCMAPChunk( ft ) )
	{
		pal = (struct Palette *)cnk->ch_Data;
		if( ( pal->pl_Count = size/3 ) <= 256 )
		{
			if( ReadChunkBytes( iff, &pal->pl_Colours, size) != size )
				abort = TRUE;
		}
		else
			abort = TRUE;
	}

	if( abort )
	{
		FreeCMAPChunk( cnk );
		cnk = NULL;
	}

	return( cnk );
}



/**************************************************************/

static BOOL ListCMAP( struct Chunk *cnk )
{
	// struct Palette *pal;
	// UWORD i;
/*
	if( (cnk->ch_TypeID == ID_CMAP) && (pal = cnk->ch_Data ) )
	{
		for( i = 0; i < pal->pl_Count; i++ )
			printf("%d.b,%d.b,%d.b\n",pal->pl_Colours[i].r, pal->pl_Colours[i].g,
				pal->pl_Colours[i].b );
	}
*/
	return( TRUE );
}

/**************************************************************/
// FindChunkType()
//
// Find a chunk of the given type.
// Inputs:
//	cnk = Chunk to start searching at (NULL means start of chunklist)
//	typeid = ID to look for
// Returns ptr to matching Chunk or NULL.
//

struct Chunk *FindChunkType( struct Chunk *cnk, ULONG typeid )
{
	struct Chunk *found = NULL;

	if( !cnk )
		cnk = (struct Chunk *)chunklist.lh_Head;

	while( cnk->ch_Node.ln_Succ && !found )
	{
		if( cnk->ch_TypeID == typeid )
			found = cnk;
		cnk = (struct Chunk *)cnk->ch_Node.ln_Succ;
	}
	return( found );
}


/**************************************************************/
// Generic mechanism to indicate a chunk has been modified.
// _All_ active superwindows have their titles refreshed.

void ModifyChunk(struct Chunk *cnk )
{
	if( !cnk->ch_Modified )
	{
		cnk->ch_Modified = TRUE;
		RefreshSuperWindowTitles();
		RefreshChunkListings();
	}
}


