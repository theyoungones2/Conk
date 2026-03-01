/****************************************************************************/
//
// BONK: Files.c
//
/****************************************************************************/


#include <string.h>
#include <dos/dos.h>
//#include <dos/dostags.h>
#include <exec/exec.h>
#include <intuition/intuition.h>
//#include <graphics/gfxbase.h>
//#include <rexx/rxslib.h>
//#include <libraries/gadtools.h>
//#include <libraries/diskfont.h>
#include <libraries/asl.h>
#include <libraries/iffparse.h>

#include <proto/exec.h>
#include <proto/dos.h>
#include <proto/intuition.h>
//#include <proto/graphics.h>
//#include <proto/diskfont.h>
#include <proto/asl.h>
//#include <clib/alib_protos.h>
//#include <proto/gadtools.h>
#include <proto/iffparse.h>


#include <Global.h>
#include <ilbm.h>
#include <ExternVars.h>


/* BLKSHeader struct - saved at beginning of BLKS chunk */
/* for internal blockset struct, see Global.h */

struct BLKSHeader
{
	UBYTE	blks_Name[BLOCKSETNAMESIZE];
	UWORD	blks_NumOfBlocks;
	UWORD blks_Depth;
	UWORD	blks_LayoutWidth;	// how many blocks across in the Blocks window
													// (20 for lores)
	/* future expansion stuff */
	UWORD	blks_Flags;				// set to 0!
	UWORD	blks_PixW;				// width in pixels (currently 16)
	UWORD	blks_PixH;				// height (currently 16)
	UWORD blks_BitMapHeight;// How high the bitmap is to be made.
	UWORD	blks_Reserved[3];	// set to 0!
};

static struct BlockSet *ImportILBM( struct IFFHandle *iff, struct BitMapHeader *bmhd, UBYTE *body );
BOOL unpackrow(BYTE **pSource, BYTE **pDest, WORD srcBytes0, WORD dstBytes0);

/*
BYTE gbbctrlbyte;
UBYTE *gbbptr;
UBYTE gbbcompression;
UBYTE gbbcount;
*/

/*****************  WriteMAPChunk()  *****************/
//
// Write a project as an iff MAP chunk using a standard IFFHandle.
//
// Returns success.
//

BOOL WriteMAPChunk( struct IFFHandle *handle, struct Project *proj )
{
	ULONG size;
	BOOL success = FALSE;
	// char name[MAPNAMESIZE];

	size = proj->pj_MapW * proj->pj_MapH * sizeof( UWORD );

	if( !PushChunk( handle, 0, ID_MAP, IFFSIZE_UNKNOWN ) )
	{
//		Mystrncpy( name, proj->pj_Node.ln_Name, MAPNAMESIZE-1 );
		if(	WriteChunkBytes( handle, proj->pj_MapName, MAPNAMESIZE ) == MAPNAMESIZE &&
			WriteChunkBytes( handle, &proj->pj_MapW, 4 ) == 4 &&
			WriteChunkBytes( handle, proj->pj_Map, size ) == size )
			success = TRUE;
		if( PopChunk( handle ) ) success = FALSE;
	}
	return( success );
}


/*****************  WriteCMAPChunk()  *****************/
//
// Write a Palette struct out to an IFF file as a standard ILBM
// CMAP chunk.
//
// Returns success.

BOOL WriteCMAPChunk( struct IFFHandle *handle, struct Palette *pal )
{
	ULONG size;
	BOOL success = FALSE;

	/* figure out the amount of data we want to write */
	size = pal->pl_Count * 3;

	if( !PushChunk( handle, 0, ID_CMAP, IFFSIZE_UNKNOWN ) )
	{
		/* copy out the RGB triplets */
		if( WriteChunkBytes( handle, pal->pl_Colours, size ) == size )
			success = TRUE;
		/* tell IFFParse to write it */
		if( PopChunk( handle ) ) success = FALSE;
	}
	return( success );
}

/*****************  WriteBLKSChunk()  *****************/
//
// Write a blockset to an iff file as a BLKS chunk.
//
// Returns success.

BOOL WriteBLKSChunk( struct IFFHandle *handle, struct BlockSet *bs )
{
	BOOL success = FALSE;
	ULONG datsize;
	struct BLKSHeader bh;

	if( !PushChunk( handle, 0, ID_BLKS, IFFSIZE_UNKNOWN ) )
	{
		/* set up the header info */
		Mystrncpy( bh.blks_Name, bs->bs_Name, BLOCKSETNAMESIZE-1 );
		bh.blks_NumOfBlocks = bs->bs_NumOfBlocks;
		bh.blks_Depth = bs->bs_Depth;
		bh.blks_LayoutWidth = bs->bs_LayoutWidth;
		/* these last ones are for future expansion */
		bh.blks_Flags = 0;
		bh.blks_PixW = BLKW;
		bh.blks_PixH = BLKH;
		bh.blks_BitMapHeight = bs->bs_BitMapHeight;
		bh.blks_Reserved[0] = 0;
		bh.blks_Reserved[1] = 0;
		bh.blks_Reserved[2] = 0;

		/* figure out how much image data there is */
		datsize = bs->bs_NumOfBlocks * bs->bs_BlkSize;

		/* write the header followed by the image data */
		if( WriteChunkBytes( handle, &bh,
			sizeof(struct BLKSHeader) ) == sizeof( struct BLKSHeader ) &&
			WriteChunkBytes( handle, bs->bs_Data, datsize ) == datsize )
			success = TRUE;
		if( PopChunk( handle ) ) success = FALSE;
	}
	return( success );
}



/**************  SaveProject  **************/
//
// Save a project under the given filename. The global palette and/or
// global BlockSet may also be saved with the mapdata, if paletteflag
// and/or blocksetflag are set.
// Will overwrite any existing file, and will not bring up any error
// requesters.
//
// Returns TRUE if operation is successful.

BOOL SaveProject( struct Project *proj, char *name, BOOL paletteflag,
	BOOL blocksetflag )
{
	struct IFFHandle *handle;
	BOOL abort = FALSE;
	BOOL success = FALSE;

	if( glob.cg_MakeBackups )
	{
		if(	!MakeBackupFile( name ) )
			return FALSE;
	}

	if( handle = AllocIFF() )
	{
		/* open the DOS file */
		if( handle->iff_Stream = Open( name, MODE_NEWFILE ) )
		{
			InitIFFasDOS( handle );
			if( !OpenIFF( handle, IFFF_WRITE ) )
			{
				if( !PushChunk( handle, ID_CONK, ID_FORM, IFFSIZE_UNKNOWN ) )
				{

					/* save out the map data */
					if( !WriteMAPChunk( handle, proj ) )
						abort = TRUE;

					/* optionally save out the palette */
					if( paletteflag && !abort )
						if(!WriteCMAPChunk( handle, globpalette ) )
							abort = TRUE;

					/* optionally save out the blockset */
					if( blocksetflag && !abort )
						if(!WriteBLKSChunk( handle, globlocks ) )
							abort = TRUE;

					if( PopChunk( handle ) )
						abort = TRUE;

					if( !abort ) success = TRUE;
				}
				CloseIFF( handle );
			}
			Close( handle->iff_Stream );
		}
		FreeIFF( handle );
	}
	return( success );
}




/******************  GUISaveProject()  ******************/
//
// A GUI wrapper for SaveProject().
// Save a project under its current name, bringing up a requester
// to let the user save the Blockset and Palette too...
// Will not save untitled projects.
//
// Returns success.

BOOL GUISaveProject( struct Project *proj )
{
	BOOL success = FALSE;
	char name[256];
	struct List *wlist = NULL;
	static struct ReqCrap rc[] =
		{
			{ "Save Additional Data:", HOOPYREQTYPE_TEXT, NULL, 0, NULL },
			{ "Blockset?", HOOPYREQTYPE_CHECK, NULL, 0, NULL },
			{ "Palette?", HOOPYREQTYPE_CHECK, NULL, 0, NULL },
		};

//	wlist = BlockAllWindows();

	if( proj && !proj->pj_UntitledFlag )				/* sanity check */
	{
		rc[1].rc_Value = proj->pj_SaveBlockSetFlag;
		rc[2].rc_Value = proj->pj_SavePaletteFlag;

		if( DoHoopyReq( "Save Map", rc, 3 ) )
		{
			if( AddPart( Mystrncpy(name, proj->pj_Drawer, 256-1),
				proj->pj_Node.ln_Name, 256-1 ) )
			{
				wlist = BlockAllWindows();
				if( SaveProject( proj, name, rc[2].rc_Value, rc[1].rc_Value ) )
				{
//					printf("SAVED\n");
					success = TRUE;
				}
				if( wlist ) FreeBlockedWindows( wlist );

			}
			if( !success )
			{
				if(wlist = BlockAllWindows())
				{
					GroovyReq( PROGNAME, "Couldn't save file:\n%s", "OK", name );
					FreeBlockedWindows( wlist );
				}
			}
		}
	}
//	if( wlist ) FreeBlockedWindows( wlist );

	if( success )
	{
		proj->pj_SaveBlockSetFlag = rc[1].rc_Value;
		proj->pj_SavePaletteFlag = rc[2].rc_Value;
		proj->pj_ModifiedFlag = FALSE;
		RefreshProjectName( proj );
	}

	return( success );
}


/******************  GUISaveProjectAs()  ******************/
//
// A further GUI wrapper for GUISaveProject(), bringing up
// a file requester to specify a new name for the project first.
//
// Returns success.


BOOL GUISaveProjectAs( struct Project *proj, struct Window *win )
{
	char name[256], *oldname, *olddrawer;
	BOOL olduntitledflag, abortflag = FALSE, success = FALSE;
	struct List *wlist = NULL;
	BPTR fh;

	wlist = BlockAllWindows();
//	if( wlist = BlockAllWindows() )
//	{
		/* bring up the file requester */
		abortflag = !AslRequestTags( filereq,
			ASLFR_Window, win,
			ASLFR_TitleText, "Save Project",
			ASLFR_DoSaveMode, TRUE,
			TAG_END );
		if( wlist ) FreeBlockedWindows(wlist);
//	}
	if( !abortflag )
	{
		/* put the full filename together */
		if( AddPart( Mystrncpy(name, filereq->fr_Drawer, 256-1),
			filereq->fr_File, 256 ) )
		{
			/* check to see if the file already exists */
			if( fh = Open( name, MODE_OLDFILE ) )
			{
				Close(fh);
				wlist = BlockAllWindows();
				abortflag = !GroovyReq( PROGNAME, "File already exists",
					"Replace|Cancel" );
				if( wlist ) FreeBlockedWindows( wlist );
//				else
//					abortflag = TRUE;
			}

			if( !abortflag )
			{
				/* save old name, just in case... */
				olddrawer = proj->pj_Drawer;
				oldname = proj->pj_Node.ln_Name;
				olduntitledflag = proj->pj_UntitledFlag;

				/* copy new names into project struct */
				proj->pj_Drawer = MyStrDup( filereq->fr_Drawer );
				proj->pj_Node.ln_Name = MyStrDup( filereq->fr_File );
				proj->pj_UntitledFlag = FALSE;

				if( GUISaveProject( proj ) )
				{
					/* saved ok - discard old name */
					if( olddrawer ) FreeVec( olddrawer );
					if( oldname ) FreeVec( oldname );
					success = TRUE;
				}
				else
				{
					/* fucked up - free new name, and restore old one... */
					if( proj->pj_Drawer ) FreeVec( proj->pj_Drawer );
					if( proj->pj_Node.ln_Name ) FreeVec( proj->pj_Node.ln_Name );
					proj->pj_Drawer = olddrawer;
					proj->pj_Node.ln_Name = oldname;
					proj->pj_UntitledFlag = olduntitledflag;
				}
			}
		}
	}
//	if( wlist ) FreeBlockedWindows( wlist );
	return(success);
}




static BOOL ReadMAPProp( struct StoredProperty *mapprop, struct Project *proj )
{
	ULONG mapsize;
	BOOL success = FALSE;
	UWORD *p;

	p = mapprop->sp_Data;
	/* mapsize = propsize - namestring - widthword - heightword */
	mapsize = mapprop->sp_Size - 16 - ( sizeof( UWORD ) * 2 );
	if( proj->pj_Map = AllocVec( mapsize, MEMF_ANY ) )
	{
		CopyMem( p, proj->pj_MapName, MAPNAMESIZE );
		p = p + 8;		//skip the name
		proj->pj_MapW = *p++;
		proj->pj_MapH = *p++;
		CopyMem( p, proj->pj_Map, mapsize );
		success = TRUE;
	}
	return( success );
}


struct BlockSet *ReadBLKSProp( struct StoredProperty *blksprop )
{
	struct BlockSet *bs;
	struct BLKSHeader *bh;
	ULONG datsize;

	bh = (struct BLKSHeader *)blksprop->sp_Data;
	datsize = blksprop->sp_Size - sizeof( struct BLKSHeader );

	if( bs = AllocVec( sizeof( struct BlockSet ), MEMF_ANY|MEMF_CLEAR ) )
	{
		if( bs->bs_Data = AllocVec( datsize, MEMF_CHIP ) )
		{
			Mystrncpy( bs->bs_Name, bh->blks_Name, BLOCKSETNAMESIZE-1);
			CopyMem( (UBYTE *)blksprop->sp_Data + sizeof(struct BLKSHeader),
				bs->bs_Data, datsize );
			bs->bs_NumOfBlocks = bh->blks_NumOfBlocks;
			bs->bs_Depth = bh->blks_Depth;
			bs->bs_LayoutWidth = bh->blks_LayoutWidth;
			bs->bs_Pad = 0;
			bs->bs_BytesPerRow = (BLKWORDW*2) * bs->bs_Depth;
			bs->bs_BlkSize = bs->bs_BytesPerRow * BLKH;
			bs->bs_BitMapHeight = bh->blks_BitMapHeight;
		}
		else
		{
			/* bail out */
			FreeVec( bs );
			bs = NULL;
		}
	}
	return( bs );
}



static struct Palette *ReadCMAPProp( struct StoredProperty *cmapprop )
{
	struct Palette *pal;

	if( pal = AllocVec( sizeof( struct Palette ), MEMF_ANY ) )
	{
		pal->pl_Count = cmapprop->sp_Size/3;
		if( pal->pl_Colours = AllocVec( cmapprop->sp_Size, MEMF_ANY ) )
			CopyMem( cmapprop->sp_Data, pal->pl_Colours, cmapprop->sp_Size );
		else
		{
			FreeVec( pal );
			pal = NULL;
		}
	}
	return( pal );
}


/****************  OpenProject()  ****************/
//
//
//


struct Project *OpenProject( char *drawer, char *file )
{
	BPTR fh;
	struct BlockSet *bs=NULL;
	struct Palette *pal;
	struct Project *proj = NULL;
	struct IFFHandle *iff;
	struct StoredProperty *mapprop, *blksprop, *cmapprop;
	struct List	*wlist;
	LONG ifferr = 0;
	BOOL projfuckup = FALSE;
//	BOOL cancelflag = FALSE;
	BOOL newpalette = FALSE, newblocks = FALSE;
	char name[256];
//	UWORD	i;
	static struct ReqCrap rc[] =
		{
			{ "Load Additional Data:", HOOPYREQTYPE_TEXT, NULL, 0, NULL },
			{ "Blocks?", HOOPYREQTYPE_CHECK, NULL, TRUE, NULL },
			{ "Palette?", HOOPYREQTYPE_CHECK, NULL, TRUE, NULL },
		};

	if( !(wlist = BlockAllWindows() ) ) return( NULL );

	/* put the full filename together */
	if( AddPart( Mystrncpy(name, drawer, 256-1), file, 256-1 ) )
	{
		/* open the file */
		if( fh = Open( name, MODE_OLDFILE ) )
		{
			if( iff = AllocIFF() )
			{
				/* set up out DOS file as the input stream */
				iff->iff_Stream = fh;
				InitIFFasDOS( iff );
				if( !( ifferr = OpenIFF( iff, IFFF_READ ) ) )
				{
					/* we want to store BLKS, CMAP and MAP chunks. */
					/* StopOnExit needed because otherwise the propchunks */
					/* will go out of context before we get to read them. */
					if( !( ifferr = PropChunk( iff, ID_CONK, ID_BLKS ) ) &&
						!( ifferr = PropChunk( iff, ID_CONK, ID_MAP ) ) &&
						!( ifferr = PropChunk( iff, ID_CONK, ID_CMAP ) ) &&
						!( ifferr = StopOnExit( iff, ID_CONK, ID_FORM ) ) )
					{
						/* do the deed */
						ifferr = ParseIFF( iff, IFFPARSE_SCAN );
						blksprop = FindProp( iff, ID_CONK, ID_BLKS );
						cmapprop = FindProp( iff, ID_CONK, ID_CMAP );
						mapprop = FindProp( iff, ID_CONK, ID_MAP );

						if( mapprop )
						{
							if( blksprop )
							{
								rc[1].rc_Type = HOOPYREQTYPE_CHECK;
								rc[1].rc_Value = TRUE;
							}
							else
							{
								rc[1].rc_Type = HOOPYREQTYPE_NULL;
								rc[1].rc_Value = FALSE;
							}

							if( cmapprop )
							{
								rc[2].rc_Type = HOOPYREQTYPE_CHECK;
								rc[2].rc_Value = TRUE;
							}
							else
							{
								rc[2].rc_Type = HOOPYREQTYPE_NULL;
								rc[2].rc_Value = FALSE;
							}

							if( blksprop || cmapprop )
							{
								if( DoHoopyReq( "Load Map", rc, 3 ) )
								{
									if( cmapprop && rc[2].rc_Value )
									{
//										printf("Using CMAP.\n");
										if( pal = ReadCMAPProp( cmapprop ) )
										{
											FreePalette( globpalette );
											globpalette = pal;
											newpalette = TRUE;
//											RemakeScreen();
										}
									}

									if( blksprop && rc[1].rc_Value )
									{
//										printf("Using BLKS.\n");
										if( bs = ReadBLKSProp( blksprop ) )
										{
											FreeBlockSet( globlocks );
											globlocks = bs;
//											RedrawAllProjects();
//											RedrawAllBlockWindows();
											newblocks = TRUE;
										}
									}



								}
								else
									projfuckup = TRUE;
							}

							if( !projfuckup )
							{
								if( proj = AllocProject() )
								{
									InitProject( proj );
									/* read the mapdata and attach to the project struct */
									if( !ReadMAPProp( mapprop, proj ) )
										projfuckup = TRUE;
								}
							}
						}
					}
					CloseIFF( iff );
				}
				FreeIFF( iff );

				/* check code returned by ParseIFF() */
				if( ifferr == IFFERR_NOTIFF )
				{
					/* not an IFF file - seek back to the beginning of the file */
					/* and give option to read it in as raw data */
					Seek( fh, 0, OFFSET_BEGINNING );
					if( proj = AllocProject() )
					{
						InitProject( proj );
						/* read the raw mapdata and attach to the project struct */
						if( !ReadRawMap( fh, proj ) )
							projfuckup = TRUE;
					}
				}
			}
			Close( fh );
		}
	}

	FreeBlockedWindows( wlist );

	if( newblocks || newpalette )
	{
		/* suss out new preview colours if there are open previewwindows...*/
		if( FindSuperWindowByType( SWTY_PREVIEW ) )
			CalcBlockPreviewColours( globlocks );
		else
			FreeBlockPreviewColours( globlocks );
	}

	if( newpalette )
	{
		if( newblocks )
			glob.cg_ScrConfig.sg_DisplayDepth = globlocks->bs_Depth;

		RemakeScreen();
	}
	if( newblocks && !newpalette )
	{
		RedrawAllProjects();
		RedrawAllBlockWindows();
	}

	if( proj && !projfuckup )
	{
		/* setup project params */
		proj->pj_Node.ln_Name = MyStrDup( file );
		proj->pj_Drawer	= MyStrDup( drawer );
		proj->pj_ModifiedFlag = FALSE;
		proj->pj_UntitledFlag = FALSE;
//		proj->pj_ProjectWindows = NULL;
//		proj->pj_PreviewWindows = NULL;
//		proj->pj_BlkAnimWindows = NULL;
//		NewList( &proj->pj_BlkAnims );
		/* clear bookmarks */
//		for( i=0; i<10; i++ )
//		{
//			proj->pj_BookMarkX[i] = 0xFFFF;
//			proj->pj_BookMarkY[i] = 0xFFFF;
//		}

		proj->pj_SavePaletteFlag = rc[2].rc_Value;
		proj->pj_SaveBlockSetFlag = rc[1].rc_Value;

		/* make sure we allocated names ok */
		if( !proj->pj_Node.ln_Name || !proj->pj_Drawer )
			projfuckup = TRUE;
		else
		{
			/* open a default window on this project */
			if( !OpenNewProjWindow(proj) )
				projfuckup = TRUE;
			else
			{
				/* link the project into the project list */
				AddHead(&projects,&proj->pj_Node);
			}
		}
	}

	/* discard the project if there was a problem */
	if( proj && projfuckup )
	{
		FreeProject( proj );
		proj = NULL;
	}

return( proj );
}



/****************  ReadRawMap()  ****************/
//

BOOL ReadRawMap( BPTR fh, struct Project *proj )
{
	BOOL success = FALSE;
	ULONG filesize, blksize, mapsize;
	UWORD *dest;
	UBYTE *src;

	static UBYTE *typelabels[] = {
		(UBYTE *)"Byte",
		(UBYTE *)"Word",
		NULL };

	static struct ReqCrap rc[] =
	{
		{ "Load Raw Map Data:", HOOPYREQTYPE_TEXT, NULL, 0, NULL },
		{ "Type:", HOOPYREQTYPE_CYCLE, typelabels, 1, NULL },
		{ "Width:", HOOPYREQTYPE_INT, NULL, 128, NULL },
		{ "Height:", HOOPYREQTYPE_INT, NULL, 16, NULL },
		{ "Headersize:", HOOPYREQTYPE_INT, NULL, 0, NULL },
	};

	/* find total filesize */
	Seek( fh, 0, OFFSET_END );
	filesize = Seek( fh, 0, OFFSET_CURRENT );
//	Seek( fh, 0, OFFSET_BEGINNING );

	if( DoHoopyReq( "Import Raw", rc, 5 ) )
	{
		/* make sure headersize is less than total filesize */
		if( rc[4].rc_Value < filesize )
		{
			if( rc[1].rc_Value == 0)
					/* working with bytesized blocks */
					blksize = sizeof( UBYTE );
			else
					/* words */
					blksize = sizeof( UWORD );

			/* calc mapsize: num of blocks (be they bytes or words...) */
			mapsize = rc[2].rc_Value * rc[3].rc_Value;

			/* seek past header */
			Seek( fh, rc[4].rc_Value, OFFSET_BEGINNING );
			proj->pj_MapW = rc[2].rc_Value;
			proj->pj_MapH = rc[3].rc_Value;
			if( proj->pj_Map =
				AllocVec( proj->pj_MapW * proj->pj_MapH * sizeof(UWORD),
					MEMF_ANY | MEMF_CLEAR ) )
			{
				if( blksize == sizeof( UWORD ) )
				{
					/* word sized blocks */
					Read( fh, proj->pj_Map, mapsize*blksize );
					success = TRUE;
				}
				else
				{
					/* bytesized blocks */
					Read( fh, proj->pj_Map, mapsize*blksize );

					/* unpack map - expand bytes into words */
					src = (UBYTE *)proj->pj_Map + mapsize;
					dest = proj->pj_Map + mapsize;
					while( mapsize )
					{
						*--dest = (*--src & 0x00FF );
						--mapsize;
					}
					success = TRUE;
				}
			}
		}
	}
	return( success );
}


/****************  LoadBlockSet()  ****************/
//

struct BlockSet *LoadBlockSet( char *name )
{
	struct BlockSet *bs = NULL;
	BOOL		newpalette = FALSE;
	BPTR fh;
	struct IFFHandle *iff;
	LONG ifferr = 0;
	struct StoredProperty *blksprop, *cmapprop, *bmhdprop, *bodyprop;
	struct Palette *pal;
	struct List *wlist = NULL;

	static UBYTE *depthlabels[] = {
		(UBYTE *)"1 (2 Colours)",
		(UBYTE *)"2 (4 Colours)",
		(UBYTE *)"3 (8 Colours)",
		(UBYTE *)"4 (16 Colours)",
		(UBYTE *)"5 (32 Colours)",
		(UBYTE *)"6 (64 Colours)",
		(UBYTE *)"7 (128 Colours)",
		(UBYTE *)"8 (256 Colours)",
		NULL };

	static UBYTE *leavelabels[] = {
		(UBYTE *)"Interleaved",
		(UBYTE *)"Non-Interleaved",
		NULL };

	struct ReqCrap				rc[] =
		{
			{ "How many bitplanes?", HOOPYREQTYPE_TEXT, NULL, 0, NULL },
			{ "", HOOPYREQTYPE_CYCLE, depthlabels, 0, NULL },
			{ "", HOOPYREQTYPE_CYCLE, leavelabels, 0, NULL },
		};

	wlist = BlockAllWindows();

	/* open the file */
	if( fh = Open( name, MODE_OLDFILE ) )
	{
		if( iff = AllocIFF() )
		{
			/* set up out DOS file as the input stream */
			iff->iff_Stream = fh;
			InitIFFasDOS( iff );
			if( !( ifferr = OpenIFF( iff, IFFF_READ ) ) )
			{
				/* we want to store BLKS and CMAP chunks from FORM CONKs */
				/* and BMHD CMAP & BODY chunks from FORM ILBMs */
				/* StopOnExit needed because otherwise the propchunks */
				/* will go out of context before we get to read them. */
				if( !( ifferr = PropChunk( iff, ID_CONK, ID_BLKS ) ) &&
					!( ifferr = PropChunk( iff, ID_CONK, ID_CMAP ) ) &&
					!( ifferr = StopOnExit( iff, ID_CONK, ID_FORM ) ) &&
					!( ifferr = PropChunk( iff, ID_ILBM, ID_BMHD ) ) &&
					!( ifferr = PropChunk( iff, ID_ILBM, ID_CMAP ) ) &&
					!( ifferr = PropChunk( iff, ID_ILBM, ID_BODY ) ) &&
					!( ifferr = StopOnExit( iff, ID_ILBM, ID_FORM ) ) )
				{
					/* do the deed */
					ifferr = ParseIFF( iff, IFFPARSE_SCAN );
					blksprop = FindProp( iff, ID_CONK, ID_BLKS );
					bmhdprop = FindProp( iff, ID_ILBM, ID_BMHD );
					bodyprop = FindProp( iff, ID_ILBM, ID_BODY );
					if( blksprop )
					{
						/* 'tis a FORM CONK thing */
						cmapprop = FindProp( iff, ID_CONK, ID_CMAP );
						bs = ReadBLKSProp( blksprop );
					}
					else
					{
						if( bmhdprop && bodyprop )
						{
							/* 'Tis a FORM ILBM thing */
							cmapprop = FindProp( iff, ID_ILBM, ID_CMAP );
							bs = ImportILBM( iff, (struct BitMapHeader *)bmhdprop->sp_Data,
								(UBYTE *)bodyprop->sp_Data );
						}
					}
					if( bs && cmapprop )
					{
						if( pal = ReadCMAPProp( cmapprop ) )
						{
							if( GroovyReq("Load Blocks",
								"Do you want to load\nthe palette as well?",
								"Yep|Nope" ) )
							{
								FreePalette( globpalette );
								globpalette = pal;
//								RemakeScreen();
								newpalette = TRUE;
							}
						}
					}
				}
				CloseIFF( iff );
			}
			FreeIFF( iff );

			if( ifferr == IFFERR_NOTIFF )
			{
				/* not an IFF file - seek back to the beginning of the file */
				/* and read it in as raw data */
				Seek( fh, 0, OFFSET_BEGINNING );
				rc[1].rc_Value = glob.cg_ScrConfig.sg_DisplayDepth - 1;
				if( DoHoopyReq( "Load Blocks", rc, 3 ) )
				{
					bs = ReadRawBlockSet( fh, rc[1].rc_Value + 1, !rc[2].rc_Value );
				}
			}
		}
		Close( fh );
		if(bs)
		{
//			printf("%d Blocks loaded.\n", bs->bs_NumOfBlocks);
			/* suss out new preview colours if there are open previewwindows...*/
			if( FindSuperWindowByType( SWTY_PREVIEW ) )
				CalcBlockPreviewColours( bs );
		}
//		else
//			printf("Block load crapout...\n");
	}
	if( wlist ) FreeBlockedWindows( wlist );

	if( newpalette ) RemakeScreen();

	return( bs );
}


/****************  ReadRawBlockSet()  ****************/
//

struct BlockSet *ReadRawBlockSet( BPTR fh, UBYTE depth, BOOL interleaved )
{
	struct BlockSet *bs = NULL;
//	UBYTE *rawdat;
	ULONG	len;
	UWORD buf[ 8 ][ BLKH ];

	UWORD *dest, blk, plane, line;

	/* find file length */
	Seek( fh, 0, OFFSET_END );
	len = Seek( fh, 0, OFFSET_BEGINNING );

	/* alloc space to Read() it in */
	if( dest = AllocVec( len, MEMF_CHIP ) )
	{
		/* alloc structure */
		if( bs = AllocVec( sizeof( struct BlockSet ), MEMF_ANY | MEMF_CLEAR ) )
		{
			/* suss blockset values */
			bs->bs_Depth = depth;
			bs->bs_BytesPerRow = BLKWORDW * sizeof( UWORD ) * depth;
			bs->bs_BlkSize = BLKH * BLKWORDW * sizeof( UWORD ) * depth;
			bs->bs_NumOfBlocks = len / bs->bs_BlkSize;
			bs->bs_LayoutWidth = 20;			// assume blocks originally from lores screen
			bs->bs_BitMapHeight =
				( ( bs->bs_NumOfBlocks / bs->bs_LayoutWidth ) + 1 ) *	BLKH;
			bs->bs_Data = (UBYTE *)dest;
			/* suss out new preview colours if there are open previewwindows...*/
			if( FindSuperWindowByType( SWTY_PREVIEW ) )
				CalcBlockPreviewColours( bs );

			/* read in blockdata */
			if( interleaved )
			{
//				printf("interleaved blockdata\n");
				Read(fh, dest, len);
			}
			else
			{
				/* read in non-interleaved data - we've got to interleave it on */
				/* the fly... heavy heavy heavy... */

//				printf("non-interleaved blockdata\n");

				for( blk = 0; blk < bs->bs_NumOfBlocks; blk++ )
				{

					/* read in a non-interleaved block image */
//					printf( "blk: %d, read: %d\n",blk, Read( fh, buf, bs->bs_BlkSize ));
					for( line = 0; line < BLKH; line++ )
					{
						for( plane=0; plane < bs->bs_Depth; plane++ )
						{
//							printf("blk: %d, line: %d, plane: %d\n",blk,line,plane);
							*dest = buf[ plane ][ line ];
							++dest;
						}
					}
				}
			}
		}
		else
		{
			FreeVec( dest );
		}
	}
	return(bs);
}



static struct BlockSet *ImportILBM( struct IFFHandle *iff, struct BitMapHeader *bmhd, UBYTE *body )
{
	struct BlockSet *bs;
	UWORD blockshigh, blockswide, blky, blkx, plane, line, byteswide;
	UBYTE	*p, *q, *rowbase, *rowbuf;

/*
	printf( "------------------------\n" );
	printf( "Import ILBM:\n" );
	printf( " Width:       %d\n", bmhd->bmh_Width );
	printf( " Height:      %d\n", bmhd->bmh_Height );
	printf( " Depth:       %d\n", bmhd->bmh_Depth );
	printf( " Compression: %d\n", bmhd->bmh_Compression );
	printf( " Masking:     %d\n", bmhd->bmh_Masking );
	printf( " Flags:       %d\n", bmhd->bmh_Flags );
*/

	if( bmhd->bmh_Depth <= 8 )
	{
		blockswide = bmhd->bmh_Width / BLKW;
		blockshigh = bmhd->bmh_Height / BLKH;
		byteswide = bmhd->bmh_Width / 8;

		if(rowbuf = AllocVec( byteswide, MEMF_ANY ) )
		{
			if(bs = AllocVec( sizeof( struct BlockSet ), MEMF_ANY|MEMF_CLEAR ) )
			{
				Mystrncpy( bs->bs_Name, "Wibble" , BLOCKSETNAMESIZE-1 );
				bs->bs_NumOfBlocks = blockswide * blockshigh;
				bs->bs_LayoutWidth = blockswide;
				bs->bs_Depth = bmhd->bmh_Depth;
				bs->bs_Pad = 0;
				bs->bs_BytesPerRow = 2 * bs->bs_Depth;
				bs->bs_BlkSize = bs->bs_Depth * 2 * BLKH;
				bs->bs_BitMapHeight = bmhd->bmh_Height;
				/* grab some memory for block images */
				if( bs->bs_Data = AllocVec( bs->bs_BlkSize * bs->bs_NumOfBlocks, MEMF_CHIP|MEMF_CLEAR ) )
				{
					/* point to first row of blocks in blockset */
					rowbase = bs->bs_Data;
					for( blky=0; blky<blockshigh; blky++ )
					{
						for( line=0; line<16; line++ )
						{
							for( plane=0; plane<bs->bs_Depth; plane++ )
							{
								p = rowbase;
								p += ( line * 2 * bs->bs_Depth);
								p += ( plane * 2 );

								if( bmhd->bmh_Compression == cmpNone )
								{
									CopyMem( body, rowbuf, byteswide );
									body += byteswide;
								}
								else
								{
									q=rowbuf;
									unpackrow( &body, &q, 10000, byteswide );
								}

								q=rowbuf;
								for( blkx=0; blkx<blockswide; blkx++ )
								{
										p[0] = *q++;
										p[1] = *q++;
										p += bs->bs_BlkSize;
								}
							}
							/* skip line if mask plane present */
							if( bmhd->bmh_Masking == mskHasMask )
							{
								if( bmhd->bmh_Compression == cmpNone )
								{
									CopyMem( body, rowbuf, byteswide );
									body += byteswide;
								}
								else
								{
									q=rowbuf;
									unpackrow( &body, &q, 10000, byteswide );
								}
							}
						}
						/* point to next row of blocks in blockset */
						rowbase += blockswide * bs->bs_BlkSize;
					}
				}
				else
				{
					FreeVec(bs);
					bs = NULL;
				}
			}
			FreeVec( rowbuf );
		}
	}
	else
		GroovyReq( "Import ILBM", "File has too many bitplanes!", "Bugger");
	return( bs );
}


/*----------------------------------------------------------------------*
 * unpacker.c Convert data from "cmpByteRun1" run compression. 11/15/85
 *
 * Based on code by Jerry Morrison and Steve Shaw, Electronic Arts.
 * This software is in the public domain.
 *
 *	control bytes:
 *	 [0..127]   : followed by n+1 bytes of data.
 *	 [-1..-127] : followed by byte to be repeated (-n)+1 times.
 *	 -128       : NOOP.
 *
 * This version for the Commodore-Amiga computer.
 *----------------------------------------------------------------------*/

/*----------- UnPackRow ------------------------------------------------*/

#define UGetByte()	(*source++)
#define UPutByte(c)	(*dest++ = (c))

/* Given POINTERS to POINTER variables, unpacks one row, updating the source
 * and destination pointers until it produces dstBytes bytes.
 */

BOOL unpackrow(BYTE **pSource, BYTE **pDest, WORD srcBytes0, WORD dstBytes0)
    {
    register BYTE *source = *pSource;
    register BYTE *dest   = *pDest;
    register WORD n;
    register WORD srcBytes = srcBytes0;
    register WORD dstBytes = dstBytes0;
    BOOL error = TRUE;	/* assume error until we make it through the loop */
    WORD minus128 = -128;  /* get the compiler to generate a CMP.W */
    register BYTE c;

    while( dstBytes > 0 )  {
	if ( (srcBytes -= 1) < 0 )  goto ErrorExit;
    	n = UGetByte();

    	if (n >= 0) {
	    n += 1;
	    if ( (srcBytes -= n) < 0 )  goto ErrorExit;
	    if ( (dstBytes -= n) < 0 )  goto ErrorExit;
	    do {  UPutByte(UGetByte());  } while (--n > 0);
	    }

    	else if (n != minus128) {
	    n = -n + 1;
	    if ( (srcBytes -= 1) < 0 )  goto ErrorExit;
	    if ( (dstBytes -= n) < 0 )  goto ErrorExit;
	    c = UGetByte();
	    do {  UPutByte(c);  } while (--n > 0);
	    }
	}
    error = FALSE;	/* success! */

  ErrorExit:
    *pSource = source;  *pDest = dest;

    return(error);
    }

