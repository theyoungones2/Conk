/********************************************************/
//
// DFRMStuff.c
//
/********************************************************/

#define DFRMSTUFF_C

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



struct DiskFormatDude {
	UBYTE dfd_Program[ PROGNAMEPACKSIZE ];
	LONG	dfd_XPos;
	LONG	dfd_YPos;
	UWORD	dfd_Delay;
};

static VOID SortFormation( struct Formation *fm );
static BOOL MakeDFRMInfoString( struct Chunk *cnk, UBYTE *buf );


/**************************************************************/
/* new dude formation chunk */

struct Chunk *CreateDFRMChunk( struct Filetracker *ft )
{
	struct Chunk *cnk;

	if( cnk = AllocVec( sizeof( struct Chunk ), MEMF_ANY|MEMF_CLEAR ) )
	{
		if( ft )
			ft->ft_ChunkCount++;
		cnk->ch_TypeID = ID_DFRM;
		cnk->ch_Edit = OpenNewFormationWindow;
		cnk->ch_Free = FreeDFRMChunk;
		cnk->ch_MakeInfoString = MakeDFRMInfoString;
		cnk->ch_Saver = DFRMSaver;
		cnk->ch_ParentFile = ft;
		cnk->ch_Modified = FALSE;
		NewList( &cnk->ch_DataList );

	}
	return( cnk );
}

/**************************************************************/

static BOOL MakeDFRMInfoString( struct Chunk *cnk, UBYTE *buf )
{
	UBYTE workbuf[256];

	if( cnk->ch_ParentFile )
		sprintf( workbuf,"%d Formations (from %s)", CountNodes( &cnk->ch_DataList ),
			cnk->ch_ParentFile->ft_File );
	else
		sprintf( workbuf,"%d Formations", CountNodes( &cnk->ch_DataList ) );

	Mystrncpy( buf, workbuf, CHUNKINFOSTRINGSIZE-1 );

	return( TRUE );
}



/**************************************************************/

VOID FreeDFRMChunk( struct Chunk *cnk )
{
	struct Formation *fm;

	if( cnk->ch_TypeID == ID_DFRM )
	{
		while( !IsListEmpty( &cnk->ch_DataList ) )
		{
			fm = (struct Formation *)cnk->ch_DataList.lh_Head;
			Remove( &fm->fm_Node );
			FreeFormation( fm );
		}
		RemoveChunkFromParentFile( cnk );
		FreeVec( cnk );
	}
}


/**************************************************************/
//
//

BOOL DFRMSaver( struct Chunk *cnk, struct IFFHandle *iff )
{
	BOOL success = FALSE, abort = FALSE;
	ULONG count;
	struct Formation *fm;
	struct Dude *dude;
	struct DiskFormatDude dfd;
	UBYTE fmpackname[ FORMATIONNAMEPACKSIZE ];

	D(bug( "DFRMSaver()\n" ) );

	assert( cnk->ch_TypeID == ID_DFRM );

	if( !(PushChunk( iff, ID_CONK, ID_DFRM, IFFSIZE_UNKNOWN ) ) )
	{
		/* write number of formations */
		count = CountNodes( &cnk->ch_DataList );
		if( WriteChunkBytes( iff, &count, sizeof( ULONG ) ) != sizeof( ULONG ) )
			abort = TRUE;

		for( fm = (struct Formation *)cnk->ch_DataList.lh_Head;
			fm->fm_Node.ln_Succ && !abort;
			fm = (struct Formation *)fm->fm_Node.ln_Succ )
		{
			SortFormation( fm );
			PackASCII( fm->fm_Name, fmpackname, FORMATIONNAMESIZE-1 );
			count = CountNodes( (struct List *)&fm->fm_DudeList );
			/* write formation name and num of dudes */
			if( ( WriteChunkBytes( iff, fmpackname, FORMATIONNAMEPACKSIZE ) !=
				FORMATIONNAMEPACKSIZE ) ||
				( WriteChunkBytes( iff, &count, sizeof( ULONG ) ) != sizeof( ULONG ) ) )
			{
				abort = TRUE;
			}

			for( dude = (struct Dude *)fm->fm_DudeList.mlh_Head;
				dude->du_Node.mln_Succ && !abort;
				dude = (struct Dude *)dude->du_Node.mln_Succ )
			{
				PackASCII( dude->du_Program, dfd.dfd_Program, PROGNAMESIZE-1 );
				dfd.dfd_XPos = dude->du_XPos;
				dfd.dfd_YPos = dude->du_YPos;
				dfd.dfd_Delay = dude->du_Delay;
				if( WriteChunkBytes( iff, &dfd, sizeof( struct DiskFormatDude ) ) !=
					sizeof( struct DiskFormatDude ) )
				{
					abort = TRUE;
				}
			}
		}
		PopChunk( iff );
	}

	success = !abort;

	return( success );
}



struct Chunk *DFRMLoader( struct IFFHandle *iff, ULONG size,
	struct Filetracker *ft )
{
	BOOL abort = FALSE;
	struct Chunk *cnk;
	ULONG fmcount;
	struct Formation *fm;

	if( cnk = CreateDFRMChunk( ft ) )
	{
		if( ReadChunkBytes( iff, &fmcount, 4) != 4 )
			abort = TRUE;
		while( fmcount-- && !abort )
		{
			if( fm = LoadFormation( iff ) )
				AddTail( &cnk->ch_DataList, &fm->fm_Node );
			else
				abort = TRUE;
		}
	}

	if( abort )
	{
		FreeDFRMChunk( cnk );
		cnk = NULL;
	}

	return( cnk );
}

/**************************************************************/

struct Formation *CreateFormation( UBYTE *name )
{
	struct Formation *fm;

	if( fm = AllocVec( sizeof( struct Formation ), MEMF_ANY|MEMF_CLEAR ) )
	{
		Mystrncpy( fm->fm_Name, name, FORMATIONNAMESIZE-1 );
		fm->fm_Node.ln_Name = fm->fm_Name;
		NewList( (struct List *)&fm->fm_DudeList );
	}
	return( fm );
}


/**************************************************************/

VOID FreeFormation( struct Formation *fm )
{
	struct Dude *dude;

	while( !IsListEmpty( (struct List *)&fm->fm_DudeList ) )
	{
		dude = (struct Dude *)fm->fm_DudeList.mlh_Head;
		Remove( (struct Node *)&dude->du_Node );
		FreeVec( dude );
	}
	FreeVec( fm );
}

/**************************************************************/

struct Formation *LoadFormation( struct IFFHandle *iff )
{
	BOOL abort=FALSE;
	struct Formation *fm = NULL;
	UBYTE namebuf[ FORMATIONNAMESIZE ];
	UBYTE packnamebuf[ FORMATIONNAMEPACKSIZE ];
	ULONG dudecount;
	struct Dude *dude;
	struct DiskFormatDude dfd;

	if( ReadChunkBytes( iff, packnamebuf, FORMATIONNAMEPACKSIZE) ==
		FORMATIONNAMEPACKSIZE )
	{
		UnpackASCII( packnamebuf, namebuf, FORMATIONNAMESIZE-1 );
		if( fm = CreateFormation( namebuf ) )
		{
			if( ReadChunkBytes( iff, &dudecount, 4 ) == 4 )
			{
				while( dudecount-- && !abort )
				{
					if( ReadChunkBytes( iff, &dfd,
						sizeof( struct DiskFormatDude ) ) == sizeof( struct DiskFormatDude ) )
					{
						if( dude = AllocVec( sizeof( struct Dude ), MEMF_ANY|MEMF_CLEAR ) )
						{
							dude->du_Tagged = NULL;
							UnpackASCII( dfd.dfd_Program, dude->du_Program, PROGNAMESIZE-1 );
							dude->du_XPos = dfd.dfd_XPos;
							dude->du_YPos = dfd.dfd_YPos;
							dude->du_Delay = dfd.dfd_Delay;
							AddTail( (struct List *)&fm->fm_DudeList,
								(struct Node *)&dude->du_Node );
						}
						else
							abort = TRUE;
					}
					else
						abort = TRUE;
				}
			}
		}
	}
	if( abort )
	{
		FreeFormation( fm );
		fm = NULL;
	}
	return( fm );
}

/**************************************************************/
// Sort all dudes in a formation according to du_Delay

static VOID SortFormation( struct Formation *fm )
{
	struct	Dude *dude, *selecteddude;
	struct	MinList templist;

	NewList( (struct List *)&templist );
	while( !IsListEmpty( (struct List *)&fm->fm_DudeList ) )
	{
		/* find dude with lowest delay */
		selecteddude = ( struct Dude *)fm->fm_DudeList.mlh_Head;
		for( dude = ( struct Dude *)fm->fm_DudeList.mlh_Head;
			dude->du_Node.mln_Succ;
			dude = ( struct Dude *)dude->du_Node.mln_Succ )
		{
			if( dude->du_Delay < selecteddude->du_Delay )
				selecteddude = dude;
		}
		Remove( (struct Node *)selecteddude );
		AddTail( (struct List *)&templist, (struct Node *)selecteddude );
	}

	/* copy back to main list */

	while( !IsListEmpty( (struct List *)&templist ) )
	{
		dude = (struct Dude *)templist.mlh_Head;
		Remove( (struct Node *) dude );
		AddTail( (struct List *)&fm->fm_DudeList, (struct Node *) dude );
	}
}

