/********************************************************/
//
// BOBStuff.c
//
/********************************************************/

#define BOBSTUFF_C

#include <stdio.h>
#include <strings.h>
#include <assert.h>

#include <exec/exec.h>
#include <dos/dos.h>
#include <graphics/gfx.h>
#include <intuition/intuition.h>
#include <libraries/iffparse.h>
#include <proto/exec.h>
#include <proto/dos.h>
#include <proto/graphics.h>
#include <proto/intuition.h>
#include <proto/iffparse.h>

#include "global.h"

static BOOL MakeBOBSInfoString( struct Chunk *cnk, UBYTE *buf );
BOOL AddBobsToList( struct List *boblist, struct Chunk *cnk );
static BOOL MakeANIMInfoString( struct Chunk *cnk, UBYTE *buf );
static BOOL MakeSPRTInfoString( struct Chunk *cnk, UBYTE *buf );

/**************************************************************/

struct Chunk *CreateBOBSChunk( struct Filetracker *ft )
{
	struct Chunk *cnk;
	if( cnk = AllocVec( sizeof( struct Chunk ), MEMF_ANY|MEMF_CLEAR ) )
	{
		if( ft )
			ft->ft_ChunkCount++;
		cnk->ch_TypeID = ID_BOBS;
		cnk->ch_Edit = NULL;
		cnk->ch_Free = FreeBOBSChunk;
		cnk->ch_MakeInfoString = MakeBOBSInfoString;
		cnk->ch_Saver = BOBSSaver;
		cnk->ch_ParentFile = ft;				/* can be NULL */
		cnk->ch_Modified = FALSE;

		/* Chunk data is loaded in from disk and stored verbatim... */
		cnk->ch_Data = NULL;
		cnk->ch_Size = 0;

		/* ...but we also create a list of BobInfoNodes, one for each bob. */
		NewList( &cnk->ch_DataList );

	}
	return( cnk );
}

/**************************************************************/

VOID FreeBOBSChunk( struct Chunk *cnk )
{

	/* update any SWaveWindows that might be using the images we're freeing */
	RethinkSWaveBobImages();

	if( cnk->ch_Data )
		FreeVec( cnk->ch_Data );
	if( !IsListEmpty( &cnk->ch_DataList ) )
		FreeNodes( &cnk->ch_DataList );
	RemoveChunkFromParentFile( cnk );
	FreeVec( cnk );
}

/**************************************************************/
//

struct Chunk *BOBSLoader( struct IFFHandle *iff, ULONG size,
	struct Filetracker *ft )
{
	BOOL abort = FALSE;
	struct Chunk *cnk;
	struct BobInfoNode *bin;
	struct BobBankFileHeader *bankheader;
	struct BobHeader *bobheader;
	UWORD i;
	UBYTE *p;

	if( cnk = CreateBOBSChunk( ft ) )
	{
		if( cnk->ch_Data = AllocVec( size, MEMF_CHIP ) )
		{
			cnk->ch_Size = size;
			if( ReadChunkBytes( iff, cnk->ch_Data, size) == size )
			{
				/* read the chunk in ok, now make up BobInfoNode for each bob... */

				bankheader = (struct BobBankFileHeader *)cnk->ch_Data;
				p = (UBYTE *)cnk->ch_Data;
				p += sizeof( struct BobBankFileHeader );

//				printf("----------Load BobBank----------\n");
//				printf("Name: %s\n",bankheader->bf_BankName );
//				printf("NumberOfBobs: %ld\n",(LONG)bankheader->bf_NumberOfBobs );
//				printf("Depth: %ld\n",(LONG)bankheader->bf_Depth );
//				printf("Flags: %ld\n",(LONG)bankheader->bf_Flags );


				for( i = 0; i < bankheader->bf_NumberOfBobs && !abort; i++ )
				{
					if( bin = AllocVec( sizeof( struct BobInfoNode ), MEMF_ANY|MEMF_CLEAR ) )
					{
						bobheader = (struct BobHeader *)p;
						UnpackASCII( bobheader->bf_Name, bin->bin_Name, BOBNAMESIZE-1 );
						bin->bin_Node.ln_Name = bin->bin_Name;
						bin->bin_BobHeader = bobheader;
						bin->bin_Chunk = cnk;

//						printf("  %d: %s\n",i,bin->bin_Name);

						AddTail( &cnk->ch_DataList, &bin->bin_Node );
						/* skip bobheader */
						p += sizeof( struct BobHeader );
						/* skip image and mask data */
						p += (((bobheader->bf_Width+15)/8) & 0xFFFE) *
							bobheader->bf_Height * bankheader->bf_Depth * 2;
					}
					else
						abort = TRUE;
				}
			}
			else
				abort = TRUE;
		}
		else
			abort = TRUE;

		if( abort )
		{
			FreeBOBSChunk( cnk );
			cnk = NULL;
		}
	}

	return( cnk );
}




/**************************************************************/

BOOL BOBSSaver( struct Chunk *cnk, struct IFFHandle *iff )
{
	BOOL success = FALSE;

	D(bug( "BOBSSaver()\n" ) );

	if( !(PushChunk( iff, ID_CONK, ID_BOBS, cnk->ch_Size ) ) )
	{
		if( WriteChunkBytes( iff, cnk->ch_Data, cnk->ch_Size ) == cnk->ch_Size )
			success = TRUE;
		PopChunk( iff );
	}
	return( success );
}

/**************************************************************/

static BOOL MakeBOBSInfoString( struct Chunk *cnk, UBYTE *buf )
{
	UBYTE workbuf[256];
	UWORD count;
	UBYTE *bankname;

	count = ((struct BobBankFileHeader *)cnk->ch_Data)->bf_NumberOfBobs;
	bankname = ((struct BobBankFileHeader *)cnk->ch_Data)->bf_BankName;

	if( cnk->ch_ParentFile )
		sprintf( workbuf,"BobBank '%s', %d images (from %s)", bankname, count,
			cnk->ch_ParentFile->ft_File );
	else
		sprintf( workbuf,"BobBank '%s', %d images", bankname, count );

	Mystrncpy( buf, workbuf, CHUNKINFOSTRINGSIZE-1 );

	return( TRUE );
}


/**************************************************************/
// Append the bobs in a chunk onto a List.
// Each bob is represented by a DataNode with dn_Data pointing
// to its BobInfoNode and ln_Name pointing to its name.
// Returns success. If failure, some bobs may have been added -
// no freeing is done.

BOOL AddBobsToList( struct List *boblist, struct Chunk *cnk )
{
	struct BobInfoNode *bin;
	struct DataNode *dn;
	BOOL allok;

	allok = TRUE;

	for( bin = (struct BobInfoNode *)cnk->ch_DataList.lh_Head;
		bin->bin_Node.ln_Succ && allok;
		bin = (struct BobInfoNode *)bin->bin_Node.ln_Succ )
	{
		if( dn = AllocVec( sizeof( struct DataNode ), MEMF_ANY|MEMF_CLEAR ) )
		{
			dn->dn_Data = (APTR)bin;
			dn->dn_Node.ln_Name = bin->bin_Name;
			AddTail( boblist, &dn->dn_Node );
		}
		else
			allok = FALSE;
	}
	return( allok );
}

/**************************************************************/

struct BobInfoNode *RequestBob( struct Window *parentwin,	UBYTE *wintitle,
	struct Chunk *cnk )
{
	BOOL allok, done, gfxmode;
	struct List boblist;
	struct DataNode *dn;
	struct BobInfoNode *bin;
/*	struct Screen *scr; */
/*	struct Window *fenster; */
	UWORD num;

	bin = NULL;
	allok = TRUE;
	NewList( &boblist );
	if( cnk )
	{
		/* just list the specified chunk... */
		allok = AppendToDataList( &boblist, &cnk->ch_DataList );
	}
	else
	{
		/* list bobs in all chunks */
		for( cnk = (struct Chunk *)chunklist.lh_Head;
			cnk->ch_Node.ln_Succ && allok;
			cnk = (struct Chunk *)cnk->ch_Node.ln_Succ )
		{
			if( cnk->ch_TypeID == ID_BOBS )
				allok = AppendToDataList( &boblist, &cnk->ch_DataList );
		}
	}

	/* maybe check allok flag here... */


	if( !IsListEmpty( &boblist ) )
	{

		/* do we want to start in GFX mode or text mode? */
		if( zonkfig.scfg.scfg_GFXScrType == 1 )
			gfxmode = TRUE;			/* we've got our own GFX screen - use it */
		else
			gfxmode = FALSE;		/* using some other screen */

		done = FALSE;
		while( !done )
		{
			num = MyListViewRequest( parentwin, wintitle, "Bob list", &boblist );

			if( num != 0xFFFF )
			{
				if( dn = (struct DataNode *)FindNthNode( &boblist, num ) )
					bin = ( struct BobInfoNode *)dn->dn_Data;
			}

			done = TRUE;
		}
	}

	FreeNodes( &boblist );
	return( bin );
}


/**************************************************************/
//
// Search through all the BOBS chunks for a named bob image.
// Returns ptr to bobinfo node or NULL

struct BobInfoNode *FindBob( STRPTR bobname )
{
	struct BobInfoNode *bin;
	struct Chunk *cnk;

	bin = NULL;

	for( cnk = (struct Chunk *)chunklist.lh_Head;
		cnk->ch_Node.ln_Succ && !bin;
		cnk = (struct Chunk *)cnk->ch_Node.ln_Succ )
	{
		if( cnk->ch_TypeID == ID_BOBS )
			bin = (struct BobInfoNode *)FindName( &cnk->ch_DataList, bobname );
	}
	return( bin );
}

/**************************************************************/
//

VOID BltBobRastPort( struct RastPort *rp, struct BobInfoNode *bin,
	WORD xpos, WORD ypos )
{
	struct BobHeader *bh;
	struct BobBankFileHeader *bankheader;
	struct BitMap bm;
	UBYTE *p;
	WORD i;

	bankheader = (struct BobBankFileHeader *)bin->bin_Chunk->ch_Data;
	bh = bin->bin_BobHeader;

	bm.Depth = bankheader->bf_Depth;
	bm.BytesPerRow = ( ( ( bh->bf_Width + 15 ) / 8 ) & ~1 ) * bm.Depth;
	bm.Rows = bh->bf_Height;
	bm.Flags = 0;		//BMF_INTERLEAVED;

	p = (UBYTE *)(bh + 1);
	for( i=0; i < bm.Depth; i++ )
	{
		bm.Planes[i] = p;
		p += ( ( ( bh->bf_Width + 15 ) / 8 ) & ~1 );
	}

//	BltBitMapRastPort( &bm, 0,0, rp, xpos, ypos,
//		bh->bf_Width, bh->bf_Height, 0xC0 );

	BltMaskBitMapRastPort( &bm,0,0,rp,xpos,ypos,bh->bf_Width, bh->bf_Height,
		0xE0, (UBYTE *)(bh + 1) + ( bm.BytesPerRow * bm.Rows ) );
}



/**************************************************************/

struct Chunk *CreateANIMChunk( struct Filetracker *ft )
{
	struct Chunk *cnk;
	if( cnk = AllocVec( sizeof( struct Chunk ), MEMF_ANY|MEMF_CLEAR ) )
	{
		if( ft )
			ft->ft_ChunkCount++;
		cnk->ch_TypeID = ID_ANIM;
		cnk->ch_Edit = NULL;
		cnk->ch_Free = FreeANIMChunk;
		cnk->ch_MakeInfoString = MakeANIMInfoString;
		cnk->ch_Saver = ANIMSaver;
		cnk->ch_ParentFile = ft;				/* can be NULL */
		cnk->ch_Modified = FALSE;

		/* Chunk data is loaded in from disk and stored verbatim... */
		cnk->ch_Data = NULL;
		cnk->ch_Size = 0;

		/* ...but we also create a list of AnimInfoNodes, one for each bobanim. */
		NewList( &cnk->ch_DataList );

	}
	return( cnk );
}

/**************************************************************/

VOID FreeANIMChunk( struct Chunk *cnk )
{
	if( cnk->ch_Data )
		FreeVec( cnk->ch_Data );
	if( !IsListEmpty( &cnk->ch_DataList ) )
		FreeNodes( &cnk->ch_DataList );
	RemoveChunkFromParentFile( cnk );
	FreeVec( cnk );
}

/**************************************************************/
//

struct Chunk *ANIMLoader( struct IFFHandle *iff, ULONG size,
	struct Filetracker *ft )
{
	BOOL abort = FALSE;
	struct Chunk *cnk;
	struct SequenceFileHeader *bankheader;
	struct AnimFileHeader *animheader;
	struct AnimInfoNode *ain;

	UWORD i;
	UBYTE *p;

	if( cnk = CreateANIMChunk( ft ) )
	{
		if( cnk->ch_Data = AllocVec( size, MEMF_ANY ) )
		{
			cnk->ch_Size = size;
			if( ReadChunkBytes( iff, cnk->ch_Data, size) == size )
			{
				/* read the chunk in ok, now make up AnimInfoNode for each bobanim... */
				bankheader = (struct SequenceFileHeader *)cnk->ch_Data;
				p = (UBYTE *)(bankheader + 1);
				for( i = 0; i < bankheader->sfh_NumOfAnims && !abort; i++ )
				{
					animheader = (struct AnimFileHeader *)p;
					if( ain = AllocVec( sizeof( struct AnimInfoNode ),
						MEMF_ANY|MEMF_CLEAR ) )
					{
						UnpackASCII( animheader->afh_AnimName, ain->ain_Name, ANIMNAMESIZE-1 );
						ain->ain_Node.ln_Name = ain->ain_Name;
						ain->ain_Anim = animheader;
						ain->ain_Chunk = cnk;
						AddTail( &cnk->ch_DataList, &ain->ain_Node );
					}
					else
						abort = TRUE;

					/* skip to next anim */
					p += sizeof( struct AnimFileHeader ) +
						(sizeof( struct FrameFileHeader ) * (animheader->afh_NumOfFrames+1) );

				}
			}
			else
				abort = TRUE;
		}
		else
			abort = TRUE;

		if( abort )
		{
			FreeANIMChunk( cnk );
			cnk = NULL;
		}
	}

	return( cnk );
}




/**************************************************************/

BOOL ANIMSaver( struct Chunk *cnk, struct IFFHandle *iff )
{
	BOOL success = FALSE;

	if( !(PushChunk( iff, ID_CONK, ID_ANIM, cnk->ch_Size ) ) )
	{
		if( WriteChunkBytes( iff, cnk->ch_Data, cnk->ch_Size ) == cnk->ch_Size )
			success = TRUE;
		PopChunk( iff );
	}
	return( success );
}

/**************************************************************/

static BOOL MakeANIMInfoString( struct Chunk *cnk, UBYTE *buf )
{
	UBYTE workbuf[256];
	UWORD count;
	UBYTE *bankname;

	count = ((struct SequenceFileHeader *)cnk->ch_Data)->sfh_NumOfAnims;
	bankname = ((struct SequenceFileHeader *)cnk->ch_Data)->sfh_BobBankName;

	if( cnk->ch_ParentFile )
		sprintf( workbuf,"%d BobAnims for '%s'  (from %s)", count, bankname,
			cnk->ch_ParentFile->ft_File );
	else
		sprintf( workbuf,"%d BobAnims for '%s'", count, bankname );

	Mystrncpy( buf, workbuf, CHUNKINFOSTRINGSIZE-1 );

	return( TRUE );
}

/**************************************************************/
//
// Search through all the ANIM chunks for a named anim.
// Returns ptr to animinfo node or NULL

struct AnimInfoNode *FindAnim( STRPTR animname )
{
	struct AnimInfoNode *ain;
	struct Chunk *cnk;

	ain = NULL;
	for( cnk = (struct Chunk *)chunklist.lh_Head;
		cnk->ch_Node.ln_Succ && !ain;
		cnk = (struct Chunk *)cnk->ch_Node.ln_Succ )
	{
		if( cnk->ch_TypeID == ID_ANIM )
			ain = (struct AnimInfoNode *)FindName( &cnk->ch_DataList, animname );
	}
	return( ain );
}

/**************************************************************/
struct BobInfoNode *AnimFirstFrame( struct AnimInfoNode *ain, LONG *offsetbuffer )
{
	struct Chunk *cnk;
	struct SequenceFileHeader *bankheader;
	struct BobInfoNode *bin;

	assert( ain->ain_Chunk != NULL );

	bankheader = (struct SequenceFileHeader *)ain->ain_Chunk->ch_Data;

	bin = NULL;
	for( cnk = (struct Chunk *)chunklist.lh_Head;
		cnk->ch_Node.ln_Succ && !bin;
		cnk = (struct Chunk *) cnk->ch_Node.ln_Succ )
	{
		if( cnk->ch_TypeID == ID_BOBS )
		{

			if( !strcmp( bankheader->sfh_BobBankName,
				((struct BobBankFileHeader *)cnk->ch_Data)->bf_BankName ) )
			{
				bin = (struct BobInfoNode *)FindNthNode( &cnk->ch_DataList,
					(((struct FrameFileHeader *)(ain->ain_Anim + 1 ))->ffh_BobNumber-1 ) );
				offsetbuffer[ 0 ] =
					((struct FrameFileHeader *)(ain->ain_Anim + 1 ))->ffh_XOffset;
				offsetbuffer[ 1 ] = 
					((struct FrameFileHeader *)(ain->ain_Anim + 1 ))->ffh_YOffset;
			}
		}
	}
	return( bin );
}

/**************************************************************/

struct AnimInfoNode *RequestAnim( struct Window *parentwin,	UBYTE *wintitle,
	struct Chunk *cnk )
{
	BOOL allok, done, gfxmode;
	struct List animlist;
	struct DataNode *dn;
	struct AnimInfoNode *ain;
/*	struct Screen *scr; */
/*	struct Window *fenster; */
	UWORD num;

	ain = NULL;
	allok = TRUE;
	NewList( &animlist );
	if( cnk )
	{
		/* just list the specified chunk... */
		allok = AppendToDataList( &animlist, &cnk->ch_DataList );
	}
	else
	{
		/* list anims in all chunks */
		for( cnk = (struct Chunk *)chunklist.lh_Head;
			cnk->ch_Node.ln_Succ && allok;
			cnk = (struct Chunk *)cnk->ch_Node.ln_Succ )
		{
			if( cnk->ch_TypeID == ID_ANIM )
				allok = AppendToDataList( &animlist, &cnk->ch_DataList );
		}
	}

	/* maybe check allok flag here... */

	if( !IsListEmpty( &animlist ) )
	{

		/* do we want to start in GFX mode or text mode? */
		if( zonkfig.scfg.scfg_GFXScrType == 1 )
			gfxmode = TRUE;			/* we've got our own GFX screen - use it */
		else
			gfxmode = FALSE;		/* using some other screen */

		done = FALSE;
		while( !done )
		{
			num = MyListViewRequest( parentwin, wintitle, "Anim list", &animlist );

			if( num != 0xFFFF )
			{
				if( dn = (struct DataNode *)FindNthNode( &animlist, num ) )
					ain = ( struct AnimInfoNode *)dn->dn_Data;
			}

			done = TRUE;
		}
	}

	FreeNodes( &animlist );
	return( ain );
}



/**************************************************************/

struct Chunk *CreateSPRTChunk( struct Filetracker *ft )
{
	struct Chunk *cnk;
	if( cnk = AllocVec( sizeof( struct Chunk ), MEMF_ANY|MEMF_CLEAR ) )
	{
		if( ft )
			ft->ft_ChunkCount++;
		cnk->ch_TypeID = ID_SPRT;
		cnk->ch_Edit = NULL;
		cnk->ch_Free = FreeSPRTChunk;
		cnk->ch_MakeInfoString = MakeSPRTInfoString;
		cnk->ch_Saver = SPRTSaver;
		cnk->ch_ParentFile = ft;				/* can be NULL */
		cnk->ch_Modified = FALSE;

		/* Chunk data is loaded in from disk and stored verbatim */
		cnk->ch_Data = NULL;
		cnk->ch_Size = 0;
	}
	return( cnk );
}

/**************************************************************/

VOID FreeSPRTChunk( struct Chunk *cnk )
{
	if( cnk->ch_Data )
		FreeVec( cnk->ch_Data );
	RemoveChunkFromParentFile( cnk );
	FreeVec( cnk );
}

/**************************************************************/
//

struct Chunk *SPRTLoader( struct IFFHandle *iff, ULONG size,
	struct Filetracker *ft )
{
	BOOL abort = FALSE;
	struct Chunk *cnk;

	if( cnk = CreateSPRTChunk( ft ) )
	{
		if( cnk->ch_Data = AllocVec( size, MEMF_CHIP ) )
		{
			cnk->ch_Size = size;
			if( ReadChunkBytes( iff, cnk->ch_Data, size) != size )
				abort = TRUE;
		}
		else
			abort = TRUE;

		if( abort )
		{
			FreeSPRTChunk( cnk );
			cnk = NULL;
		}
	}

	return( cnk );
}




/**************************************************************/

BOOL SPRTSaver( struct Chunk *cnk, struct IFFHandle *iff )
{
	BOOL success = FALSE;

	D(bug( "SPRTSaver()\n" ) );

	if( !(PushChunk( iff, ID_CONK, ID_SPRT, cnk->ch_Size ) ) )
	{
		if( WriteChunkBytes( iff, cnk->ch_Data, cnk->ch_Size ) == cnk->ch_Size )
			success = TRUE;
		PopChunk( iff );
	}
	return( success );
}

/**************************************************************/

static BOOL MakeSPRTInfoString( struct Chunk *cnk, UBYTE *buf )
{
	UBYTE workbuf[256];

	if( cnk->ch_ParentFile )
		sprintf( workbuf,"SpriteBank (from %s)", cnk->ch_ParentFile->ft_File );
	else
		sprintf( workbuf,"SpriteBank" );

	Mystrncpy( buf, workbuf, CHUNKINFOSTRINGSIZE-1 );

	return( TRUE );
}
