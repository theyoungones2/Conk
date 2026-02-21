/********************************************************/
//
// GAMEStuff.c
//
/********************************************************/

#define GAMESTUFF_C

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

static BOOL MakeGAMEInfoString( struct Chunk *cnk, UBYTE *buf );

/**************************************************************/

struct Chunk *CreateGAMEChunk( struct Filetracker *ft )
{
	struct Chunk *cnk;
	struct GameConfig *gc;

	if( cnk = AllocVec( sizeof( struct Chunk ), MEMF_ANY|MEMF_CLEAR ) )
	{
		if( ft )
			ft->ft_ChunkCount++;
		cnk->ch_TypeID = ID_GAME;
		cnk->ch_Edit = OpenNewGAMEWindow;
		cnk->ch_Free = FreeGAMEChunk;
		cnk->ch_MakeInfoString = MakeGAMEInfoString;
		cnk->ch_Saver = GAMESaver;
		cnk->ch_ParentFile = ft;
		cnk->ch_Modified = FALSE;
		cnk->ch_Data = AllocVec( sizeof( struct GameConfig ), MEMF_ANY|MEMF_CLEAR );
		if( gc = (struct GameConfig *)cnk->ch_Data )
		{
			/* init GameConfig */
			strcpy( gc->gc_GameFileHeader.gfh_GameName, "A Groovy Game" );

			NewList( &gc->gc_Levels );
		}
		else
		{
			FreeGAMEChunk( cnk );
			cnk = NULL;
		}

	}
	return( cnk );
}



/**************************************************************/

static BOOL MakeGAMEInfoString( struct Chunk *cnk, UBYTE *buf )
{
	UBYTE workbuf[256];

	if( cnk->ch_ParentFile )
		sprintf( workbuf, "Game Config (from %s)", cnk->ch_ParentFile->ft_File );
	else
		sprintf( workbuf,"Game Config" );

	Mystrncpy( buf, workbuf, CHUNKINFOSTRINGSIZE-1 );

	return( TRUE );
}

/**************************************************************/

VOID FreeGAMEChunk( struct Chunk *cnk )
{
	struct GameConfig *gc;
	struct LevelConfig *lc;

	assert( cnk != NULL );
	assert( cnk->ch_TypeID == ID_GAME );

	/* free stuff here */
	if( gc = (struct GameConfig *)cnk->ch_Data )
	{
		while( !IsListEmpty( &gc->gc_Levels ) )
		{
			lc = (struct LevelConfig *)gc->gc_Levels.lh_Head;
			Remove( &lc->lc_Node );
			/* free  LFile list */
			FreeNodes( &lc->lc_Files );
			FreeVec( lc );
		}

		FreeVec( gc );
	}

	RemoveChunkFromParentFile( cnk );
	FreeVec( cnk );
}


/**************************************************************/

BOOL GAMESaver( struct Chunk *cnk, struct IFFHandle *iff )
{
	UWORD foo;
	BOOL allok;
	struct GameConfig *gc;
	struct LevelConfig *lc;
	struct LFile *lf;

	assert( cnk != NULL );
	assert( cnk->ch_TypeID == ID_GAME );
	D(bug( "GAMESaver()\n" ) );

	allok = TRUE;
	gc = (struct GameConfig *)cnk->ch_Data;

	if( !(PushChunk( iff, ID_CONK, ID_GAME, IFFSIZE_UNKNOWN ) ) )
	{
		/* save out GameFileHeader */
		gc->gc_GameFileHeader.gfh_ConkVersion = 1;	/* Conk - The Real Thing (tm) */
		gc->gc_GameFileHeader.gfh_Computer = 42;		/* Amiga */
		gc->gc_GameFileHeader.gfh_NumOfLevels = CountNodes( &gc->gc_Levels );
		gc->gc_GameFileHeader.gfh_Reserved[ 0 ] = 0;
		gc->gc_GameFileHeader.gfh_Reserved[ 1 ] = 0;
		gc->gc_GameFileHeader.gfh_Reserved[ 2 ] = 0;
		gc->gc_GameFileHeader.gfh_Reserved[ 3 ] = 0;

		if( WriteChunkBytes( iff, &gc->gc_GameFileHeader,
			sizeof( struct GameFileHeader ) ) == sizeof( struct GameFileHeader ) )
		{

			/* step through game levels */
			for( lc = (struct LevelConfig *)gc->gc_Levels.lh_Head;
				lc->lc_Node.ln_Succ && allok;
				lc = (struct LevelConfig *)lc->lc_Node.ln_Succ )
			{
				/* write LevelFileHeader */
				PackASCII( lc->lc_Name, lc->lc_LevelFileHeader.lfh_LevelName,
					LEVELFULLNAMESIZE-1 );
				lc->lc_LevelFileHeader.lfh_NumOfFiles = CountNodes( &lc->lc_Files );
				lc->lc_LevelFileHeader.lfh_Password[0] = '\0';
				lc->lc_LevelFileHeader.lfh_Reserved[0] = 0;
				lc->lc_LevelFileHeader.lfh_Reserved[1] = 0;
				lc->lc_LevelFileHeader.lfh_Reserved[2] = 0;
				lc->lc_LevelFileHeader.lfh_Reserved[3] = 0;

				if( WriteChunkBytes( iff, &lc->lc_LevelFileHeader,
					sizeof( struct LevelFileHeader ) ) == sizeof( struct LevelFileHeader ) )
				{
					/* step through level files */
					for( lf = (struct LFile *)lc->lc_Files.lh_Head;
						lf->lf_Node.ln_Succ && allok;
						lf = (struct LFile *)lf->lf_Node.ln_Succ )
					{
						foo = strlen( lf->lf_Name )+1;
						/* pad name to even length */
						if( foo & 1 )
							foo++;
						/* write type and name fields */
						if( WriteChunkBytes( iff, &lf->lf_Type, 2+foo ) != ( 2 + foo ) )
							allok = FALSE;
					}
				}
				else
					allok = FALSE;
			}
		}
		else
			allok = FALSE;
		PopChunk( iff );
	}
	return( allok );
}


/**************************************************************/

struct Chunk *GAMELoader( struct IFFHandle *iff, ULONG size,
	struct Filetracker *ft )
{
	struct Chunk *cnk;
	struct GameConfig *gc;
	struct LevelConfig *lc;
	struct LFile *lf;
	BOOL allok;
	UBYTE *buf, *p;
	int i, j, foo;

	allok = TRUE;
	if( cnk = CreateGAMEChunk( ft ) )
	{
		gc = (struct GameConfig *)cnk->ch_Data;
		if( buf = AllocVec( size, MEMF_ANY ) )
		{
			p = buf;
			if( ReadChunkBytes( iff, buf, size ) == size )
			{
				/* Set up GameConfig */
				gc->gc_GameFileHeader = *( (struct GameFileHeader *) p );
				p += sizeof( struct GameFileHeader );
				NewList( &gc->gc_Levels );

				/* Read in levels */
				for( i=0; ( i < gc->gc_GameFileHeader.gfh_NumOfLevels ) && allok; i++ )
				{
					if( lc = AllocVec( sizeof( struct LevelConfig ),MEMF_ANY|MEMF_CLEAR ) )
					{
						lc->lc_LevelFileHeader = *((struct LevelFileHeader *)p);
						p += sizeof( struct LevelFileHeader );
						UnpackASCII( lc->lc_LevelFileHeader.lfh_LevelName, lc->lc_Name,
							LEVELFULLNAMESIZE-1 );
						NewList( &lc->lc_Files );
						lc->lc_Node.ln_Name = lc->lc_Name;
						AddTail( &gc->gc_Levels, &lc->lc_Node );

						for( j = 0;
							( j < lc->lc_LevelFileHeader.lfh_NumOfFiles ) && allok;
							j++ )
						{
							if( lf = AllocVec( sizeof( struct LFile ), MEMF_ANY|MEMF_CLEAR ) )
							{
								/* read file type */
								lf->lf_Type = *((UWORD *)p);
								p += 2;

								/* read file name */
								Mystrncpy( lf->lf_Name, p, MAXFILENAMESIZE-1 );
								foo = strlen( p ) + 1;
								if( foo & 1 )												/* pad to even */
									foo++;
								p += foo;

								/* add to files list */
								lf->lf_Node.ln_Name = lf->lf_Name;
								AddTail( &lc->lc_Files, &lf->lf_Node );
							}
							else
								allok = FALSE;
						}
					}
					else
						allok = FALSE;
				}
			}
			else
				allok = FALSE;
			FreeVec( buf );
		}
		else
			allok = FALSE;
	}

	if( !allok )
	{
		FreeGAMEChunk( cnk );
		cnk = NULL;
	}

	return( cnk );
}



