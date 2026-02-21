/********************************************************/
//
// SFXStuff.c
//
/********************************************************/

#define SFXSTUFF_C

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

/*
struct DiskFormatSFXHeader
{
	UBYTE dfx_PackedName[ SFXNAMEPACKSIZE ];
	BYTE	dfx_Pri;
	UBYTE	dfx_Volume;
	UWORD	dfx_Period;
	ULONG dfx_Length;
	UWORD	dfx_Cycles;
	UWORD	dfx_Reserved[4];
};
*/

static BOOL MakeSFXInfoString( struct Chunk *cnk, UBYTE *buf );

/**************************************************************/

struct Chunk *CreateSFXChunk( struct Filetracker *ft )
{
	struct Chunk *cnk;

	if( cnk = AllocVec( sizeof( struct Chunk ), MEMF_ANY|MEMF_CLEAR ) )
	{
		if( ft )
			ft->ft_ChunkCount++;
		cnk->ch_TypeID = ID_SFX;
		cnk->ch_Edit = OpenNewSFXWindow;
		cnk->ch_Free = FreeSFXChunk;
		cnk->ch_MakeInfoString = MakeSFXInfoString;
		cnk->ch_Saver = SFXSaver;
		cnk->ch_ParentFile = ft;
		cnk->ch_Modified = FALSE;
		NewList( &cnk->ch_DataList );
	}
	return( cnk );
}

/**************************************************************/

static BOOL MakeSFXInfoString( struct Chunk *cnk, UBYTE *buf )
{
	UBYTE workbuf[256];

	if( cnk->ch_ParentFile )
		sprintf( workbuf,"%d Sound Effects (from %s)",
			CountNodes( &cnk->ch_DataList),
			cnk->ch_ParentFile->ft_File );
	else
		sprintf( workbuf,"%d Sound Effects", CountNodes( &cnk->ch_DataList) );

	Mystrncpy( buf, workbuf, CHUNKINFOSTRINGSIZE-1 );

	return( TRUE );
}

/**************************************************************/

VOID FreeSFXChunk( struct Chunk *cnk )
{
	struct Node *foo;

	if( cnk->ch_TypeID == ID_SFX )
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

BOOL SFXSaver( struct Chunk *cnk, struct IFFHandle *iff )
{
	ULONG count, len;
	struct SFXHeader *sfx;
	BOOL abort = FALSE;

	assert( cnk->ch_TypeID == ID_SFX );
	D(bug( "SFXSaver()\n" ) );

	count = CountNodes( &cnk->ch_DataList );

	if( !(PushChunk( iff, ID_CONK, ID_SFX, IFFSIZE_UNKNOWN ) ) )
	{
		/* write number of effects */
		if( WriteChunkBytes( iff, &count, sizeof( ULONG ) ) != sizeof( ULONG ) )
			abort = TRUE;

		for( sfx = (struct SFXHeader *)cnk->ch_DataList.lh_Head;
			sfx->sfx_Node.ln_Succ && !abort;
			sfx = (struct SFXHeader *)sfx->sfx_Node.ln_Succ )
		{
			PackASCII( sfx->sfx_Name, sfx->sfx_Spam.spam_PackedName, SFXNAMESIZE-1 );

			len = sizeof( struct SpamParam ) + sfx->sfx_Spam.spam_Length;
			if( WriteChunkBytes( iff, &sfx->sfx_Spam, len ) != len )
				abort = TRUE;
		}
		PopChunk( iff );
	}
	return( (BOOL)( abort ? FALSE:TRUE ) );
}


/**************************************************************/

struct Chunk *SFXLoader( struct IFFHandle *iff, ULONG size,
	struct Filetracker *ft )
{
	struct Chunk *cnk;
	struct SpamParam spam;
	struct SFXHeader *sfx;
	ULONG count;
	BOOL allok = TRUE;

	D(bug( "SFXLoader()\n") );

	if( cnk = CreateSFXChunk( ft ) )
	{
		if( ReadChunkBytes( iff, &count, 4 ) != 4 )
			allok = FALSE;

		while( count-- && allok )
		{
//			if( ReadChunkBytes( iff, &spam, 28 )==28 )
			if( ReadChunkBytes( iff, &spam, sizeof( struct SpamParam ) ) ==
				sizeof( struct SpamParam ) )
			{
				if( sfx = AllocVec( sizeof( struct SFXHeader ) + spam.spam_Length,
					MEMF_CHIP ) )
				{
					spam.spam_RepeatLength = spam.spam_Length;		/* Delete This when repeats are done properly */
/*
					spam.spam_RepeatStart = 0;
					spam.spam_RepeatLength = 0;
					spam.spam_Repeats = 0;
					spam.spam_Flags = 0;
					spam.spam_Reserved[0] = 0;
					spam.spam_Reserved[1] = 0;
					spam.spam_Reserved[2] = 0;
					spam.spam_Reserved[3] = 0;
*/
					if( ReadChunkBytes( iff, sfx+1, spam.spam_Length ) == spam.spam_Length )
					{
						/* copy the loaded spam struct into the SFXHeader */
						sfx->sfx_Spam = spam;
						/* make an ASCII copy of the name */
						UnpackASCII( sfx->sfx_Spam.spam_PackedName, sfx->sfx_Name,
							SFXNAMESIZE-1 );
						sfx->sfx_Node.ln_Name = sfx->sfx_Name;
						AddTail( &cnk->ch_DataList, &sfx->sfx_Node );
						D(bug("'%s' OK\n", sfx->sfx_Name ) );
					}
					else
					{
						allok = FALSE;
						FreeVec( sfx );
						sfx = NULL;
					}
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

	if( !allok )
	{
		D(bug("Failed\n") );
		if( cnk )
			FreeSFXChunk( cnk );
		cnk = NULL;
	}

	return( cnk );
}

