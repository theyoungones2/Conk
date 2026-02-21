
#define ZCFGSTUFF_C

#include <stdio.h>
//#include <string.h>
#include <assert.h>

#include <dos/dos.h>
#include <dos/dostags.h>
#include <exec/exec.h>
//#include <intuition/intuition.h>
#include <graphics/gfxbase.h>
//#include <libraries/gadtools.h>
#include <libraries/diskfont.h>
#include <libraries/asl.h>
#include <libraries/iffparse.h>

#include <proto/exec.h>
#include <proto/dos.h>
//#include <proto/intuition.h>
//#include <proto/graphics.h>
//#include <proto/diskfont.h>
#include <proto/asl.h>
#include <clib/alib_protos.h>
//#include <proto/gadtools.h>
#include <proto/iffparse.h>

#include <global.h>

static BOOL MakeZCFGInfoString( struct Chunk *cnk, UBYTE *buf );
static BOOL InstallZCFG( struct Chunk *cnk );

/**************************************************************/

struct Chunk *CreateZCFGChunk( struct Filetracker *ft )
{
	struct Chunk *cnk;
	struct ZonkConfig *zcfg;

	if( cnk = AllocVec( sizeof( struct Chunk ), MEMF_ANY|MEMF_CLEAR ) )
	{
		if( zcfg = AllocVec( sizeof( struct ZonkConfig ), MEMF_ANY|MEMF_CLEAR ) )
		{
			if( ft )
				ft->ft_ChunkCount++;
			cnk->ch_TypeID = ID_ZCFG;
			cnk->ch_Edit = InstallZCFG;
			cnk->ch_Free = FreeZCFGChunk;
			cnk->ch_MakeInfoString = MakeZCFGInfoString;
			cnk->ch_Saver = ZCFGSaver;
			cnk->ch_ParentFile = ft;
			cnk->ch_Modified = FALSE;

			cnk->ch_Data = zcfg;
			cnk->ch_Size = 0;
		}
		else
		{
			FreeVec( cnk );
			cnk = NULL;
		}
	}
	return( cnk );
}

/**************************************************************/

static BOOL MakeZCFGInfoString( struct Chunk *cnk, UBYTE *buf )
{
	UBYTE workbuf[256];

	if( cnk->ch_ParentFile )
		sprintf( workbuf,"Zonk Config (from %s)",
			cnk->ch_ParentFile->ft_File );
	else
		sprintf( workbuf,"Zonk Config" );

	Mystrncpy( buf, workbuf, CHUNKINFOSTRINGSIZE-1 );

	return( TRUE );
}

/**************************************************************/

VOID FreeZCFGChunk( struct Chunk *cnk )
{

	if( cnk->ch_TypeID == ID_ZCFG )		/* sanity check */
	{
		/* free the ZonkConfig struct */
		if( cnk->ch_Data )
			FreeVec( cnk->ch_Data );

		RemoveChunkFromParentFile( cnk );
		FreeVec( cnk );
	}
}




/**************************************************************/

BOOL ZCFGSaver( struct Chunk *cnk, struct IFFHandle *iff )
{
	BOOL success;

	assert( cnk != NULL );									/* sanity check */
	assert( cnk->ch_TypeID == ID_ZCFG );

	D(bug( "ZCFGSaver()\n") );

	success = FALSE;															/* optimistic outlook */
	if( !(PushChunk( iff, ID_CONK, ID_ZCFG, sizeof( struct ZonkConfig ) ) ) )
	{
		/* write number of effects */
		if( WriteChunkBytes( iff, cnk->ch_Data, sizeof( struct ZonkConfig ) ) ==
			sizeof( struct ZonkConfig ) )
		{
			success = TRUE;
		}
		PopChunk( iff );
	}
	return( success );
}


/**************************************************************/

struct Chunk *ZCFGLoader( struct IFFHandle *iff, ULONG size,
	struct Filetracker *ft )
{
	struct Chunk *cnk;

	if( cnk = CreateZCFGChunk( ft ) )
	{
		/* read the config into the (already allocated) chunk space */
		if( ReadChunkBytes( iff, cnk->ch_Data, sizeof( struct ZonkConfig ) )
			!= sizeof( struct ZonkConfig ) )
		{
			/* fucked it up */
			FreeZCFGChunk( cnk );
			cnk = NULL;
		}
	}
	return( cnk );
}

/**************************************************************/
//
// Save the given ZonkConfig struct under the given filename.
// Wraps it up as a FORM CONK.
// Returns success.

BOOL SaveZonkConfig( STRPTR filename, struct ZonkConfig *zcfg )
{
	struct IFFHandle *iff;
	BOOL success;

	success = FALSE;

	if( iff = AllocIFF() )
	{
		if( iff->iff_Stream = Open( filename, MODE_NEWFILE ) )
		{
			InitIFFasDOS( iff );
			if( !OpenIFF( iff, IFFF_WRITE ) )
			{
				if( !PushChunk( iff, ID_CONK, ID_FORM, IFFSIZE_UNKNOWN ) )
				{
					if( !(PushChunk( iff, ID_CONK, ID_ZCFG, sizeof( struct ZonkConfig ) ) ) )
					{
						if( WriteChunkBytes( iff, zcfg, sizeof( struct ZonkConfig ) ) ==
							sizeof( struct ZonkConfig ) )
						{
							success = TRUE;
						}
						PopChunk( iff );
					}
					PopChunk( iff );
				}
				CloseIFF( iff );
			}
			Close( iff->iff_Stream );
		}
		FreeIFF( iff );
	}
	return( success );
}



/* kludge - editroutine */

static BOOL InstallZCFG( struct Chunk *cnk )
{
	InstallZonkConfig( (struct ZonkConfig *)cnk->ch_Data );
	return( TRUE );
}



/**************************************************************/
//
// LoadZonkConfig()
// Load a ZCFG chunk from the given file into the given ZonkConfig struct.
// Returns success. If load fails, the contents of the ZonkConfig struct
// are undefined.

BOOL LoadZonkConfig( STRPTR filename, struct ZonkConfig *zcfg )
{
	BPTR fh;
	BOOL success;
	ULONG ifferror;
	struct IFFHandle *iff;

	success = FALSE;				/* optimistic outlook */

	if( fh = Open( filename, MODE_OLDFILE ) )
	{
		if( iff = AllocIFF() )
		{
			iff->iff_Stream = fh;
			InitIFFasDOS( iff );

			if( !(ifferror = OpenIFF( iff, IFFF_READ ) ) )
			{
				if( !(ifferror = StopChunk( iff, ID_CONK, ID_ZCFG ) ) )
				{
					if( !(ifferror = ParseIFF( iff, IFFPARSE_SCAN ) ) )
					{
						if( ReadChunkBytes( iff, zcfg, sizeof( struct ZonkConfig ) )
							== sizeof( struct ZonkConfig ) )
						{
							success = TRUE;
						}
					}
				}
				CloseIFF( iff );
			}
			FreeIFF( iff );
		}
		Close( fh );
	}
	return( success );
}


/**************************************************************/
//
// GUISaveZonkConfigAs()


BOOL GUISaveZonkConfigAs( struct ZonkConfig *zcfg )
{
	UBYTE namebuf[512];
	BOOL success = FALSE;		/* now there's optimism for you */
	struct SuperWindow *sw;
	BPTR lock;
	UWORD foo = 1;

	if( sw = FensterSnarf() )
	{
		if( BlockAllSuperWindows() )
		{
			if( AslRequestTags( filereq,
				ASLFR_Window, sw->sw_Window,
				ASLFR_TitleText, "Save Config",
				ASLFR_RejectIcons, TRUE,
				ASLFR_DoMultiSelect, FALSE,
				ASLFR_DoPatterns, FALSE,
				ASLFR_DoSaveMode, TRUE,
				ASLFR_InitialFile, "Zonk.cfg",
				ASLFR_InitialDrawer, "PROGDIR:",
				TAG_DONE ) )
			{
				Mystrncpy( namebuf, filereq->fr_Drawer, 512-1 );
				if( AddPart( namebuf, filereq->fr_File, 512) )
				{
					if( lock = Lock( namebuf, ACCESS_READ ) )
					{
						/* file exists */
						UnLock( lock );
						foo = PlebReq( sw->sw_Window, "Save Config", "File already exists.\nReplace?", "Replace|Cancel" );
					}

					if( foo )
					{
						if( SaveZonkConfig( namebuf, zcfg ) )
							success = TRUE;
						else
							PlebReq( sw->sw_Window, "Save Error", "Save failed!", "Bugger!" );
					}
				}
			}
		}
		UnblockAllSuperWindows();
	}
	
	return( success );
}

 
BOOL GUILoadZonkConfig( struct ZonkConfig *zcfg )
{
	UBYTE namebuf[512];
	BOOL success = FALSE;		/* now there's optimism for you */
	struct SuperWindow *sw;

	if( sw = FensterSnarf() )
	{
		if( BlockAllSuperWindows() )
		{
			if( AslRequestTags( filereq,
				ASLFR_Window, sw->sw_Window,
				ASLFR_TitleText, "Load Config",
				ASLFR_DoPatterns, TRUE,
				ASLFR_InitialPattern, "~(#?.info|#?.bak)",
				ASLFR_DoMultiSelect, TRUE,
				ASLFR_DoSaveMode, FALSE,
				ASLFR_InitialFile, "Zonk.cfg",
				ASLFR_InitialDrawer, "PROGDIR:",
				TAG_DONE ) )
			{
				Mystrncpy( namebuf, filereq->fr_Drawer, 512-1 );
				if( AddPart( namebuf, filereq->fr_File, 512) )
				{
					if( LoadZonkConfig( namebuf, zcfg ) )
						success = TRUE;
				}
			}
		}
		UnblockAllSuperWindows();
	}
	
	return( success );
}
