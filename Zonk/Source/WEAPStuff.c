/********************************************************/
//
// WEAPStuff.c
//
/********************************************************/

#define WEAPSTUFF_C

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

/* First longword of WEAP chunk gives number of weapons saved. */
/* WeaponDiskDef structures follow. */

struct WeaponDiskDef
{
	UBYTE	wdd_Name[ WEAPONNAMEPACKSIZE ];
	UBYTE wdd_FireProgram[ PROGNAMEPACKSIZE ];
	UWORD wdd_FireDelay;
	UWORD wdd_MaxBullets;
	UBYTE	wdd_PowerUp[ WEAPONNAMEPACKSIZE ];
	UBYTE	wdd_PowerDown[ WEAPONNAMEPACKSIZE ];
	UWORD wdd_Flags;
	UWORD	wdd_Reserved;
};

static BOOL MakeWEAPInfoString( struct Chunk *cnk, UBYTE *buf );

/**************************************************************/

struct Chunk *CreateWEAPChunk( struct Filetracker *ft )
{
	struct Chunk *cnk;

	if( cnk = AllocVec( sizeof( struct Chunk ), MEMF_ANY|MEMF_CLEAR ) )
	{
		if( ft )
			ft->ft_ChunkCount++;
		cnk->ch_TypeID = ID_WEAP;
		cnk->ch_Edit = OpenNewWeaponWindow;
		cnk->ch_Free = FreeWEAPChunk;
		cnk->ch_MakeInfoString = MakeWEAPInfoString;
		cnk->ch_Saver = WEAPSaver;
		cnk->ch_ParentFile = ft;
		cnk->ch_Modified = FALSE;
		cnk->ch_Data = NULL;
		cnk->ch_Size = 0;

		/* list of struct WeaponDefs */
		NewList( &cnk->ch_DataList );
	}
	return( cnk );
}

/**************************************************************/

static BOOL MakeWEAPInfoString( struct Chunk *cnk, UBYTE *buf )
{
	UBYTE workbuf[256];

	if( cnk->ch_ParentFile )
		sprintf( workbuf,"%ld WeaponDefs (from %s)",
			(LONG)CountNodes( &cnk->ch_DataList ), cnk->ch_ParentFile->ft_File );
	else
		sprintf( workbuf,"%ld WeaponDefs", (LONG)CountNodes( &cnk->ch_DataList ) );

	Mystrncpy( buf, workbuf, CHUNKINFOSTRINGSIZE-1 );

	return( TRUE );
}

/**************************************************************/

VOID FreeWEAPChunk( struct Chunk *cnk )
{
	if( cnk->ch_TypeID == ID_WEAP )
	{
		FreeNodes( &cnk->ch_DataList );
		RemoveChunkFromParentFile( cnk );
		FreeVec( cnk );
	}
}

/**************************************************************/

BOOL WEAPSaver( struct Chunk *cnk, struct IFFHandle *iff )
{
	struct WeaponDef *wd;
	struct WeaponDiskDef diskdef;
	BOOL allcoolandgroovy;
	ULONG weapcount;

	assert( cnk != NULL );
	assert( cnk->ch_TypeID == ID_WEAP );
	D(bug( "WEAPSaver()\n") );

	allcoolandgroovy = TRUE;
	weapcount = CountNodes( &cnk->ch_DataList );

	if( !(PushChunk( iff, ID_CONK, ID_WEAP,
		4 + (sizeof( struct WeaponDiskDef ) * weapcount ) ) ) )
	{
		/* write number of weapons */
		if( WriteChunkBytes( iff, &weapcount, sizeof( ULONG ) ) != sizeof( ULONG ) )
			allcoolandgroovy = FALSE;

		for( wd = (struct WeaponDef *)cnk->ch_DataList.lh_Head;
			wd->wd_Node.ln_Succ && allcoolandgroovy;
			wd = (struct WeaponDef *)wd->wd_Node.ln_Succ )
		{
			PackASCII( wd->wd_FireProgram, diskdef.wdd_FireProgram, PROGNAMESIZE - 1 );
			PackASCII( wd->wd_Name, diskdef.wdd_Name, WEAPONNAMESIZE - 1 );
			PackASCII( wd->wd_PowerUp, diskdef.wdd_PowerUp, WEAPONNAMESIZE - 1 );
			PackASCII( wd->wd_PowerDown, diskdef.wdd_PowerDown, WEAPONNAMESIZE - 1 );
			diskdef.wdd_FireDelay = wd->wd_FireDelay;
			diskdef.wdd_MaxBullets = wd->wd_MaxBullets;
			diskdef.wdd_Flags = wd->wd_Flags;
			diskdef.wdd_Reserved = 0;

			if( WriteChunkBytes( iff, &diskdef, sizeof( struct WeaponDiskDef ) ) !=
				sizeof( struct WeaponDiskDef ) )
			{
				allcoolandgroovy = FALSE;
			}
		}
		PopChunk( iff );
	}

	return( allcoolandgroovy );
}


/**************************************************************/

struct Chunk *WEAPLoader( struct IFFHandle *iff, ULONG size,
	struct Filetracker *ft )
{
	BOOL abort = FALSE;
	struct Chunk *cnk;
	ULONG count;
	struct WeaponDiskDef wdd;
	struct WeaponDef *wd;

	if( cnk = CreateWEAPChunk( ft ) )
	{
		if( ReadChunkBytes( iff, &count, 4) != 4 )
			abort = TRUE;

		while( count-- && !abort )
		{
			if( wd = AllocVec( sizeof( struct WeaponDef ), MEMF_ANY ) )
			{
				if( ReadChunkBytes( iff, &wdd, sizeof( struct WeaponDiskDef ) )
					== sizeof( struct WeaponDiskDef ) )
				{
					UnpackASCII( wdd.wdd_FireProgram, wd->wd_FireProgram, PROGNAMESIZE-1 );
					UnpackASCII( wdd.wdd_Name, wd->wd_Name, WEAPONNAMESIZE-1 );
					UnpackASCII( wdd.wdd_PowerUp, wd->wd_PowerUp, WEAPONNAMESIZE-1 );
					UnpackASCII( wdd.wdd_PowerDown, wd->wd_PowerDown, WEAPONNAMESIZE-1 );
					wd->wd_FireDelay = wdd.wdd_FireDelay;
					wd->wd_MaxBullets = wdd.wdd_MaxBullets;
					wd->wd_Node.ln_Name = wd->wd_Name;
					wd->wd_Flags = wdd.wdd_Flags;
					AddTail( &cnk->ch_DataList, (struct Node *)wd );
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
		FreeWEAPChunk( cnk );
		cnk = NULL;
	}

	return( cnk );
}


/**************************************************************/

struct WeaponDef *RequestWeapon( struct Window *parentwin, 	STRPTR wintitle,
	struct Chunk *cnk )
{
	UWORD num;
	struct WeaponDef *wd;

	wd = NULL;
	if( cnk && ( cnk->ch_TypeID == ID_WEAP ) )
	{
		num = MyListViewRequest( parentwin, wintitle, "Weapons:", &cnk->ch_DataList );
		if( num != 0xFFFF )
		{
			wd = (struct WeaponDef *)FindNthNode( &cnk->ch_DataList, num );
		}
	}
	return( wd );
}
