
#define PROGSTUFF_C

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


struct ProgramFileHeader
{
	UBYTE pfh_Name[ PROGNAMEPACKSIZE ];
	ULONG pfh_LameSize;
	ULONG pfh_DiskSize;
};

static struct Program *CookPROG( struct ProgramFileHeader *pfh, UBYTE *dat );
static BOOL MakePROGInfoString( struct Chunk *cnk, UBYTE *buf );


/**************************************************************/
// Create a new PROG chunk ( pair with FreePROGChunk() )

struct Chunk *CreatePROGChunk( struct Filetracker *ft )
{
	struct Chunk *cnk;

	if( cnk = AllocVec( sizeof( struct Chunk ), MEMF_ANY|MEMF_CLEAR ) )
	{
		if( ft )
			ft->ft_ChunkCount++;
		cnk->ch_TypeID = ID_PROG;
		cnk->ch_Edit = OpenNewPROGWindow;
		cnk->ch_Free = FreePROGChunk;
		cnk->ch_MakeInfoString = MakePROGInfoString;
		cnk->ch_Saver = PROGSaver;
		cnk->ch_ParentFile = ft;				/* can be NULL */
		cnk->ch_Modified = FALSE;
		NewList( &cnk->ch_DataList );
	}
	return( cnk );
}

/**************************************************************/

static BOOL MakePROGInfoString( struct Chunk *cnk, UBYTE *buf )
{
	UBYTE workbuf[256];

	if( !IsListEmpty( &cnk->ch_DataList ) )
	{
		if( cnk->ch_ParentFile )
		{
			sprintf( workbuf,"%d ActionLists, '%s'... (from %s)",
				CountNodes( &cnk->ch_DataList ),
				cnk->ch_DataList.lh_Head->ln_Name,
				cnk->ch_ParentFile->ft_File );
		}
		else
		{
			sprintf( workbuf,"%d ActionLists, '%s'...",
				CountNodes( &cnk->ch_DataList ),
				cnk->ch_DataList.lh_Head->ln_Name );
		}
	}
	else
	{
		if( cnk->ch_ParentFile )
		{
			sprintf( workbuf,"%d ActionLists (from %s)", CountNodes( &cnk->ch_DataList ),
				cnk->ch_ParentFile->ft_File );
		}
		else
			sprintf( workbuf,"%d ActionLists", CountNodes( &cnk->ch_DataList ) );
	}
	
	Mystrncpy( buf, workbuf, CHUNKINFOSTRINGSIZE-1 );

	return( TRUE );
}


/**************************************************************/
// Free a PROG chunk and all its associated crap.

VOID FreePROGChunk( struct Chunk *cnk )
{
	struct Program *prog;

	if( cnk->ch_TypeID == ID_PROG )
	{
		while( !IsListEmpty( &cnk->ch_DataList ) )
		{
			prog = (struct Program *)cnk->ch_DataList.lh_Head;
			Remove( &prog->pg_Node );
			FreeProgram( prog );
		}
		RemoveChunkFromParentFile( cnk );
		FreeVec( cnk );
	}
}


/*********************************************************************/
//
// Loader for PROG chunks
//
// PROG chunks use the ch_DataList field to hold a list of Program
// structures. ch_Data and ch_Size are not used.
// Free chunk with FreePROGChunk().

struct Chunk *PROGLoader( struct IFFHandle *iff, ULONG size,
	struct Filetracker *ft )
{
	struct Chunk *cnk;
	ULONG i, numofprogs;
	struct ProgramFileHeader pfh;
	BOOL abort = FALSE;
	UBYTE *progdata;
	struct Program *prog;

	if( cnk = CreatePROGChunk( ft ) )
	{
		if( ReadChunkBytes( iff, &numofprogs, 4 ) != 4 )
			abort = TRUE;

		for( i=0; i<numofprogs && !abort; i++ )
		{
			if( ReadChunkBytes( iff, &pfh, sizeof( struct ProgramFileHeader ) ) ==
				sizeof( struct ProgramFileHeader ) )
			{
				if( progdata = AllocVec( pfh.pfh_DiskSize, MEMF_ANY ) )
				{
					if( ReadChunkBytes( iff, progdata, pfh.pfh_DiskSize ) == pfh.pfh_DiskSize )
					{
						if( prog = CookPROG( &pfh, progdata ) )
						{
							CheckProgram( prog );
							AddTail( &cnk->ch_DataList, &prog->pg_Node );
						}
					}
					FreeVec( progdata );
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
		FreePROGChunk( cnk );
		cnk = NULL;
	}

	return( cnk );
}



/**************************************************************/

BOOL PROGSaver( struct Chunk *cnk, struct IFFHandle *iff )
{
	BOOL allok = TRUE;
	struct Program *prog;
	struct OpNode *on;
	struct ProgramFileHeader pfh;
	ULONG	numofprogs;
	UWORD temp = 0;

	assert( cnk->ch_TypeID == ID_PROG );
//	D(bug( "PROGSaver()\n" ) );

	numofprogs = CountNodes( &cnk->ch_DataList );

	if( !(PushChunk( iff, ID_CONK, ID_PROG, IFFSIZE_UNKNOWN ) ) )
	{
		if( WriteChunkBytes( iff, &numofprogs, sizeof( ULONG ) ) == sizeof( ULONG ) )
		{
			for( prog = (struct Program *)cnk->ch_DataList.lh_Head;
				prog->pg_Node.ln_Succ && allok;
				prog = (struct Program *)prog->pg_Node.ln_Succ )
			{
				PackASCII( prog->pg_Name, pfh.pfh_Name, PROGNAMESIZE-1 );
				pfh.pfh_LameSize = CalcProgramLameSize( prog );
				pfh.pfh_DiskSize = CalcProgramDiskSize( prog );
				if( WriteChunkBytes( iff, &pfh, sizeof( struct ProgramFileHeader ) ) ==
					sizeof( struct ProgramFileHeader ) )
				{
					for( on = (struct OpNode *)prog->pg_OpList.mlh_Head;
						on->on_Node.mln_Succ && allok;
						on = (struct OpNode *)on->on_Node.mln_Succ )
					{
						allok = WriteOperator( iff, on->on_OpData );
					}
					/* end of program -> write the END operator */
					if( WriteChunkBytes( iff, &temp, sizeof( UWORD ) ) != sizeof( UWORD ) )
						allok = FALSE;
				}
				else
					allok = FALSE;
			}
		}
		PopChunk( iff );
	}

	return( allok );
}



/***************************************************************/
//
// Takes an image of a program (same as on disk) and cooks
// it to produce our internal format for editing.
//

static struct Program *CookPROG( struct ProgramFileHeader *pfh, UBYTE *dat )
{
	struct Program *prog;
	struct OpNode *on = NULL;
	BOOL abort = FALSE;

	if( prog = AllocVec( sizeof( struct Program ), MEMF_ANY|MEMF_CLEAR ) )
	{
//		Mystrncpy( prog->pg_Name, pfh->pfh_Name, PROGNAMESIZE-1 );

		UnpackASCII( pfh->pfh_Name, prog->pg_Name, PROGNAMESIZE-1 );

//		D(bug ("CookProg(), program: %s\n",prog->pg_Name ) );

		prog->pg_Node.ln_Name = prog->pg_Name;
		NewList( (struct List *)&prog->pg_OpList );

		/* loop through, cook operators */
		while( *((UWORD *)dat) && !abort  )
		{
			if( on = AllocVec( sizeof( struct OpNode ), MEMF_ANY|MEMF_CLEAR ) )
			{
				if( on->on_OpData = CookOperator( &dat ) )
					AddTail( (struct List *)&prog->pg_OpList, (struct Node *)on );
				else
				{
					FreeVec( on );
					abort = TRUE;
				}
			}
			else
				abort = TRUE;
		}
	}
	else
		abort = TRUE;

	/* handle fuckups */
	if( abort && prog )
	{
		while( !IsListEmpty( (struct List *)&prog->pg_OpList ) )
		{
			on = (struct OpNode *)prog->pg_OpList.mlh_Head;
			Remove( (struct Node *)on );
			FreeVec( on );
		}
		FreeVec( prog );
		prog = NULL;
	}

	return( prog );
}





/**************************************************************/
// Add a new (empty) program to a specified PROG chunk.
//

BOOL AddNewProgram( struct Chunk *cnk, char *name )
{
	BOOL success = FALSE;
	struct Program *prog;

	if( cnk->ch_TypeID == ID_PROG )
	{
		if( prog = AllocVec( sizeof( struct Program ), MEMF_ANY|MEMF_CLEAR ) )
		{
			MakePackable( name );
			Mystrncpy( prog->pg_Name, name, PROGNAMESIZE-1 );
			prog->pg_Node.ln_Name = prog->pg_Name;
			NewList( (struct List *)&prog->pg_OpList );
			AddTail( &cnk->ch_DataList, &prog->pg_Node );
			success = TRUE;
		}
	}
	return( success );
}

/**************************************************************/
//

struct Program *CopyProgram( struct Program *srcprog )
{
	struct Program *destprog;
	struct OpNode *srcopnode, *destopnode;

	BOOL allok;

	allok = TRUE;
	if( destprog = AllocVec( sizeof( struct Program ), MEMF_ANY|MEMF_CLEAR ) )
	{

		CopyMem( srcprog->pg_Name, destprog->pg_Name, PROGNAMESIZE );
		destprog->pg_Locked = FALSE;
		destprog->pg_Node.ln_Name = destprog->pg_Name;

		NewList( (struct List *)&destprog->pg_OpList );

		for( srcopnode = (struct OpNode *)srcprog->pg_OpList.mlh_Head;
			srcopnode->on_Node.mln_Succ && allok;
			srcopnode = (struct OpNode *)srcopnode->on_Node.mln_Succ )
		{
			if( destopnode = AllocVec( sizeof( struct OpNode ), MEMF_ANY ) )
			{
				if( destopnode->on_OpData = CopyOperator( srcopnode->on_OpData ) )
					AddTail( (struct List *)&destprog->pg_OpList,
						(struct Node *)destopnode );
				else
				{
					FreeVec( destopnode );
					allok = FALSE;
				}
			}
			else
				allok = FALSE;
		}

		if( !allok )
			FreeProgram( destprog );
	}
	return( destprog );
}



/**************************************************************/
// Free a program and all its associated opdata.
//

VOID FreeProgram( struct Program *prog )
{
	struct OpNode *on;

	while( !IsListEmpty( (struct List *)&prog->pg_OpList ) )
	{
		on = (struct OpNode *)prog->pg_OpList.mlh_Head;
		Remove( (struct Node *)on );
		FreeOperatorData( on->on_OpData );
		FreeVec( on );
	}
	FreeVec( prog );
}

/**************************************************************/
// Calculate how many bytes the program will be on disk.
// (saved along with the program, so Lame knows how much to load)

ULONG CalcProgramDiskSize( struct Program *prog )
{
	struct OpNode *on;
	ULONG size=0;

	for( on = (struct OpNode *)prog->pg_OpList.mlh_Head;
		on->on_Node.mln_Succ;
		on = (struct OpNode *)on->on_Node.mln_Succ )
	{
		size += CalcOpDiskSize( on->on_OpData );
	}
	size += 2;	// allow for END op
	return( size );
}

/**************************************************************/
// Calculate how many bytes the program will be after cooking by Lame.
// (saved along with the program, so Lame knows how many bytes to allocate)

ULONG CalcProgramLameSize( struct Program *prog )
{
	struct OpNode *on;
	ULONG size=0;

	for( on = (struct OpNode *)prog->pg_OpList.mlh_Head;
		on->on_Node.mln_Succ;
		on = (struct OpNode *)on->on_Node.mln_Succ )
	{
		size += CalcOpLameSize( on->on_OpData );
	}
	size += 4;	// allow for END op
	return( size );
}


/**************************************************************/
//
// Appends entries for all the programs in the specified chunk to
// the given list. Programs are added as DataNodes, with ln_Name
// pointing to the program namestring, and dn_Data pointing to the
// actual program.
// Use FreeNodes() to free the nodes.

BOOL AppendToProgramList( struct Chunk *cnk, struct List *proglist )
{
	struct Program *prog;
	BOOL allok = TRUE;
	struct DataNode *dn;

	if( cnk && (cnk->ch_TypeID == ID_PROG) )
	{
		for( prog = (struct Program *)cnk->ch_DataList.lh_Head;
			prog->pg_Node.ln_Succ && allok;
			prog = (struct Program *)prog->pg_Node.ln_Succ )
		{
			if( dn = AllocVec( sizeof(struct DataNode), MEMF_ANY|MEMF_CLEAR ) )
			{
				dn->dn_Node.ln_Name = prog->pg_Name;
				dn->dn_Data = (APTR)prog;
				AddTail( proglist, &dn->dn_Node );
			}
			else
				allok = FALSE;
		}
	}
	return( allok );
}

/**************************************************************/
// GlobalFindProg()
//
// Search for a program by name in all PROG chunks.
// returns ptr to prog or NULL.

struct Program *GlobalFindProg( STRPTR progname )
{
	struct Chunk *cnk;
	struct Program *match;

	match = NULL;
	for( cnk = (struct Chunk *)chunklist.lh_Head;
		cnk->ch_Node.ln_Succ && !match;
		cnk = (struct Chunk *)cnk->ch_Node.ln_Succ )
	{
		if( cnk->ch_TypeID == ID_PROG )
			match = (struct Program *)FindName( &cnk->ch_DataList, progname );
	}
	return( match );
}


/**************************************************************/
UBYTE *FindItemInProgram( struct Program *prog, UBYTE itemtype )
{
	struct OpNode *on;
	UBYTE *itemptr;

	assert( prog != NULL );

	itemptr = NULL;
	for( on = (struct OpNode *)prog->pg_OpList.mlh_Head;
		on->on_Node.mln_Succ && !itemptr;
		on = (struct OpNode *)on->on_Node.mln_Succ )
	{
		itemptr = FindItemInOperator( on->on_OpData, itemtype );
	}
	return( itemptr );
}


/**************************************************************/
// Returns the first image referenced in a Program, either a bob image
// or the first frame of an anim.
// If a program is referenced, this routine will be recalled
// to search it ( see FindImageFromOperator() ) if enterprogs set.

struct BobInfoNode *FindImageFromProgram( struct Program *prog,
	LONG *offsetbuffer, BOOL enterprogs )
{
	struct OpNode *on;
	struct BobInfoNode *bin;

	assert( prog != NULL );

	bin = NULL;
	for( on = (struct OpNode *)prog->pg_OpList.mlh_Head;
		on->on_Node.mln_Succ && !bin;
		on = (struct OpNode *)on->on_Node.mln_Succ )
	{
		bin = FindImageFromOperator( on->on_OpData, offsetbuffer, enterprogs );
	}
	return( bin );
}


/**************************************************************/

void CheckProgram( struct Program *prog )
{
	struct OpNode *on;
	UWORD op;
	UBYTE *opdata, *p;

	assert( prog != NULL );

	for( on = (struct OpNode *)prog->pg_OpList.mlh_Head;
		on->on_Node.mln_Succ;
		on = (struct OpNode *)on->on_Node.mln_Succ )
	{
		opdata = on->on_OpData;

		op = *((UWORD *)opdata);
		opdata += 2;
		if( op == 26 )
		{
			/* it's SetChannel! */
			opdata += 2;	/* skip channelnum word */

			p = *((UBYTE **)opdata);
			op = *((UWORD *)p);
			if( operators[op].od_Name[0] == '*' )
			{
				/* found an obsolete channelroutine! */
				PlebReq( NULL, "Loader", "Warning, ActionList '%s' contains an obsolete ChannelRoutine",
					"OK", prog->pg_Name );
			}
		}
		else if( operators[op].od_Name[0] == '*' )
		{
			/* found an obsolete operator! */
			PlebReq( NULL, "Loader", "Warning, ActionList '%s' contains an obsolete Action",
				"OK", prog->pg_Name );
		}
	}
}

