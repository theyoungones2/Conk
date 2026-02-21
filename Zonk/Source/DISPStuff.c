/********************************************************/
//
// DISPStuff.c
//
/********************************************************/

#define DISPSTUFF_C

#include <stdio.h>
#include <stdlib.h>
#include <strings.h>

#include <exec/exec.h>
#include <dos/dos.h>
#include <libraries/iffparse.h>
#include <proto/exec.h>
#include <proto/dos.h>
#include <proto/iffparse.h>
//#include <proto/graphics.h>

#include "global.h"

static BOOL MakeDISPInfoString( struct Chunk *cnk, UBYTE *buf );

/**************************************************************/

struct Chunk *CreateDISPChunk( struct Filetracker *ft )
{
	struct Chunk *cnk;
	struct ZonkDisp *zdip;

	if( cnk = AllocVec( sizeof( struct Chunk ), MEMF_ANY|MEMF_CLEAR ) )
	{
		if( ft )
			ft->ft_ChunkCount++;
		cnk->ch_TypeID = ID_DISP;
		cnk->ch_Edit = OpenNewDISPWindow;
		cnk->ch_Free = FreeDISPChunk;
		cnk->ch_MakeInfoString = MakeDISPInfoString;
		cnk->ch_Saver = DISPSaver;
		cnk->ch_ParentFile = ft;				/* can be NULL */
		cnk->ch_Modified = FALSE;

		if( ( cnk->ch_Data = AllocVec( sizeof( struct ZonkDisp ), MEMF_ANY|MEMF_CLEAR ) ) )
		{
			/* Set up some default values */
			zdip = (struct ZonkDisp *)cnk->ch_Data;
			zdip->zdp_NumOfPlayerStructs = 1;
			zdip->zdp_NumOfBulletStructs = 10;
			zdip->zdp_NumOfBDStructs = 20;
		}
		else
		{
			FreeDISPChunk( cnk );
			cnk = NULL;
		}
	}
	return( cnk );
}

/**************************************************************/
VOID FreeDISPChunk( struct Chunk *cnk )
{
	if( cnk->ch_Data )
		FreeVec( cnk->ch_Data );

	RemoveChunkFromParentFile( cnk );
	FreeVec( cnk );
}

/**************************************************************/

static BOOL MakeDISPInfoString( struct Chunk *cnk, UBYTE *buf )
{
	UBYTE workbuf[256];
	if( cnk->ch_ParentFile )
		sprintf( workbuf,"Display (from %s)", cnk->ch_ParentFile->ft_File );
	else
		sprintf( workbuf,"Display" );

	Mystrncpy( buf, workbuf, CHUNKINFOSTRINGSIZE-1 );

	return( TRUE );
}



/**************************************************************/
BOOL DISPSaver( struct Chunk *cnk, struct IFFHandle *iff )
{
	BOOL success = FALSE;
	ULONG palettesize;
	struct Display dh;
	struct ZonkDisp *zdip;

	D(bug( "DISPSaver()\n" ) );

	assert( cnk != NULL );
	assert( cnk->ch_TypeID == ID_DISP );

	if( zdip = cnk->ch_Data )
	{
		palettesize = 3 * zdip->zdp_Palette.pl_Count;

		if( !(PushChunk( iff, ID_CONK, ID_DISP,
			palettesize + sizeof( struct Display ) ) ) )
		{
			/* translate ZonkDisp into Display */

			dh.dp_Flags = zdip->zdp_Flags;
			dh.dp_FrameRate = zdip->zdp_FrameRate;

			CopyMem( zdip->zdp_Map, dh.dp_Map, 16 );
			CopyMem( zdip->zdp_BlockSet, dh.dp_BlockSet, 16 );
			PackASCII( zdip->zdp_BackDrop, dh.dp_BackDrop, GENERICNAMESIZE-1 );
			PackASCII( zdip->zdp_InitProgram, dh.dp_InitProgram,
				PROGNAMESIZE-1 );

			dh.dp_LevelWidth = zdip->zdp_LevelWidth;
			dh.dp_LevelHeight = zdip->zdp_LevelHeight;
			dh.dp_LevelDepth = zdip->zdp_LevelDepth;
			dh.dp_DisplayID = zdip->zdp_DisplayID;

			dh.dp_NumOfPlayerStructs = zdip->zdp_NumOfPlayerStructs;
			dh.dp_NumOfBulletStructs = zdip->zdp_NumOfBulletStructs;
			dh.dp_NumOfBDStructs = zdip->zdp_NumOfBDStructs;
			dh.dp_SpriteColourBank = zdip->zdp_SpriteColourBank;
			dh.dp_SpriteBurstMode = zdip->zdp_SpriteBurstMode;

//			dh.dp_GameHeaderHeight = zdip->zdp_GameHeaderHeight;
//			dh.dp_StatHeaderHeight = zdip->zdp_StatHeaderHeight;
			/* need a better formula */
			dh.dp_GameHeaderHeight = 2 + (zdip->zdp_Palette.pl_Count/32);
			dh.dp_StatHeaderHeight = 3;
			PackASCII( zdip->zdp_StatBackDropPicture,
				dh.dp_StatBackDropPicture,	GENERICNAMESIZE-1 );
			dh.dp_StatDisplayID = zdip->zdp_StatDisplayID;
			PackASCII( zdip->zdp_StatUpdateProg,
				dh.dp_StatUpdateProg,	PROGNAMESIZE-1 );
			PackASCII( zdip->zdp_StatShieldBarImage,
				dh.dp_StatShieldBarImage,	GENERICNAMESIZE-1 );

			dh.dp_DefaultBorderLeft = zdip->zdp_DefaultBorderLeft;
			dh.dp_DefaultBorderLeftType = zdip->zdp_DefaultBorderLeftType;
			dh.dp_DefaultBorderRight = zdip->zdp_DefaultBorderRight;
			dh.dp_DefaultBorderRightType = zdip->zdp_DefaultBorderRightType;
			dh.dp_DefaultBorderTop = zdip->zdp_DefaultBorderTop;
			dh.dp_DefaultBorderTopType = zdip->zdp_DefaultBorderTopType;
			dh.dp_DefaultBorderBottom = zdip->zdp_DefaultBorderBottom;
			dh.dp_DefaultBorderBottomType = zdip->zdp_DefaultBorderBottomType;
			dh.dp_DefaultBorderFlags = zdip->zdp_DefaultBorderFlags;

			if( zdip->zdp_DefaultBorderFlags & 1 )
				dh.dp_DefaultBorderLeftType = 0-dh.dp_DefaultBorderLeftType;
			if( zdip->zdp_DefaultBorderFlags & 2 )
				dh.dp_DefaultBorderRightType = 0-dh.dp_DefaultBorderRightType;
			if( zdip->zdp_DefaultBorderFlags & 4 )
				dh.dp_DefaultBorderTopType = 0-dh.dp_DefaultBorderTopType;
			if( zdip->zdp_DefaultBorderFlags & 8 )
				dh.dp_DefaultBorderBottomType = 0-dh.dp_DefaultBorderBottomType;

//			dh.dp_NumOfPlayers = zdip->zdp_NumOfPlayers;
			dh.dp_NumOfPlayers = zdip->zdp_NumOfPlayerStructs;

			PackASCII( zdip->zdp_PlayerInitProgram1, dh.dp_PlayerInitProgram1,
				PROGNAMESIZE-1 );
			PackASCII( zdip->zdp_PlayerInitProgram2, dh.dp_PlayerInitProgram2,
				PROGNAMESIZE-1 );
			PackASCII( zdip->zdp_PlayerInitProgram3, dh.dp_PlayerInitProgram3,
				PROGNAMESIZE-1 );
			PackASCII( zdip->zdp_PlayerInitProgram4, dh.dp_PlayerInitProgram4,
				PROGNAMESIZE-1 );

			dh.dp_NumOfColours = zdip->zdp_Palette.pl_Count;

			/* write header */
			if( WriteChunkBytes( iff, &dh, sizeof( struct Display ) ) ==
				sizeof( struct Display ) )
			{
				/* Write palette */
				if( WriteChunkBytes( iff, &zdip->zdp_Palette.pl_Colours,
					palettesize ) == palettesize )
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

struct Chunk *DISPLoader( struct IFFHandle *iff, ULONG size,
	struct Filetracker *ft )
{
	BOOL abort = FALSE;
	ULONG datasize;
	struct Chunk *cnk;

	struct ZonkDisp *zdip;
	struct Display dh;

	if( cnk = CreateDISPChunk( ft ) )
	{
		zdip = (struct ZonkDisp *)cnk->ch_Data;

		if( ReadChunkBytes( iff, &dh, sizeof( struct Display ) ) ==
			sizeof( struct Display ) )
		{
			/* translate */

			zdip->zdp_Flags = dh.dp_Flags;
			zdip->zdp_FrameRate = dh.dp_FrameRate;

			CopyMem( dh.dp_Map, zdip->zdp_Map, 16 );
			CopyMem( dh.dp_BlockSet, zdip->zdp_BlockSet, 16 );
			UnpackASCII( dh.dp_BackDrop, zdip->zdp_BackDrop,
				GENERICNAMESIZE-1 );
			UnpackASCII( dh.dp_InitProgram, zdip->zdp_InitProgram,
				PROGNAMESIZE-1 );

			zdip->zdp_LevelWidth = dh.dp_LevelWidth;
			zdip->zdp_LevelHeight = dh.dp_LevelHeight;
			zdip->zdp_LevelDepth = dh.dp_LevelDepth;
			zdip->zdp_DisplayID = dh.dp_DisplayID;

			zdip->zdp_NumOfPlayerStructs = dh.dp_NumOfPlayerStructs;
			zdip->zdp_NumOfBulletStructs = dh.dp_NumOfBulletStructs;
			zdip->zdp_NumOfBDStructs = dh.dp_NumOfBDStructs;
			zdip->zdp_SpriteColourBank = dh.dp_SpriteColourBank;
			zdip->zdp_SpriteBurstMode = dh.dp_SpriteBurstMode;

//			zdip->zdp_GameHeaderHeight = dh.dp_GameHeaderHeight;
//			zdip->zdp_StatHeaderHeight = dh.dp_StatHeaderHeight;
			/* work stuff out here please... */
			zdip->zdp_GameHeaderHeight = 3;
			zdip->zdp_StatHeaderHeight = 3;
			UnpackASCII( dh.dp_StatBackDropPicture,
				zdip->zdp_StatBackDropPicture, GENERICNAMESIZE-1 );
			zdip->zdp_StatDisplayID = dh.dp_StatDisplayID;
			UnpackASCII( dh.dp_StatUpdateProg,
				zdip->zdp_StatUpdateProg, PROGNAMESIZE-1 );
			UnpackASCII( dh.dp_StatShieldBarImage,
				zdip->zdp_StatShieldBarImage, GENERICNAMESIZE-1 );

			zdip->zdp_DefaultBorderLeft = dh.dp_DefaultBorderLeft;
			zdip->zdp_DefaultBorderLeftType = dh.dp_DefaultBorderLeftType;
			zdip->zdp_DefaultBorderRight = dh.dp_DefaultBorderRight;
			zdip->zdp_DefaultBorderRightType = dh.dp_DefaultBorderRightType;
			zdip->zdp_DefaultBorderTop = dh.dp_DefaultBorderTop;
			zdip->zdp_DefaultBorderTopType = dh.dp_DefaultBorderTopType;
			zdip->zdp_DefaultBorderBottom = dh.dp_DefaultBorderBottom;
			zdip->zdp_DefaultBorderBottomType = dh.dp_DefaultBorderBottomType;
			zdip->zdp_DefaultBorderFlags = 0;

			if( zdip->zdp_DefaultBorderLeftType < 0 )
				zdip->zdp_DefaultBorderFlags |= 1;
			if( zdip->zdp_DefaultBorderRightType < 0 )
				zdip->zdp_DefaultBorderFlags |= 2;
			if( zdip->zdp_DefaultBorderTopType < 0 )
				zdip->zdp_DefaultBorderFlags |= 4;
			if( zdip->zdp_DefaultBorderBottomType < 0 )
				zdip->zdp_DefaultBorderFlags |= 8;

			zdip->zdp_NumOfPlayers = dh.dp_NumOfPlayers;

			UnpackASCII( dh.dp_PlayerInitProgram1,
				zdip->zdp_PlayerInitProgram1, PROGNAMESIZE-1 );
			UnpackASCII( dh.dp_PlayerInitProgram2,
				zdip->zdp_PlayerInitProgram2, PROGNAMESIZE-1 );
			UnpackASCII( dh.dp_PlayerInitProgram3,
				zdip->zdp_PlayerInitProgram3, PROGNAMESIZE-1 );
			UnpackASCII( dh.dp_PlayerInitProgram4,
				zdip->zdp_PlayerInitProgram4, PROGNAMESIZE-1 );

			zdip->zdp_Palette.pl_Count = dh.dp_NumOfColours;

			datasize = dh.dp_NumOfColours*3;
			if( ReadChunkBytes( iff, zdip->zdp_Palette.pl_Colours,
				datasize ) != datasize )
			{
				abort = TRUE;
			}
		}
	}

	if( abort )
	{
		FreeDISPChunk( cnk );
		cnk = NULL;
	}

	return( cnk );
}

