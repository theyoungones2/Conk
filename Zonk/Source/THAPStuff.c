/********************************************************/
//
// THAPStuff.c
//
/********************************************************/

#define THAPSTUFF_C

#include <stdio.h>
#include <strings.h>
#include <assert.h>
#include <math.h>

#include <exec/exec.h>
#include <dos/dos.h>
#include <libraries/iffparse.h>
#include <proto/exec.h>
#include <proto/dos.h>
#include <proto/iffparse.h>

#include "global.h"

static BOOL MakeTHAPInfoString( struct Chunk *cnk, UBYTE *buf );

static BOOL WritePathSource( struct IFFHandle *iff, struct Path *path );
static struct Path *ReadPath( struct IFFHandle *iff );
static ULONG CalcPathCompiledSize( struct Path *path );
static BOOL WritePathCompiled( struct IFFHandle *iff, struct Path *path );

/* THAP chunk format:
 *
 *	number of paths (LONG)
 *	1st path:
 *		packed name of path (BYTE array, PATHNAMEPACKSIZE length )
 *		xorigin of path (LONG)
 *		yorigin of path (LONG)
 *		path source:
 *			size of sourcedata (LONG) 0 = no source
 *      Segment Count (UWORD)
 *			array of struct PathSegments without MinNodes (includes HEAD segment)
 *		path compiled data:
 *			size of compiled data (LONG)
 *				compiled segment data...
 *	Next path... etc
 */


struct DiskFormatPathHeader
{
		UBYTE PackName[ PATHNAMEPACKSIZE ];
		LONG XOrigin, YOrigin;
};

/**************************************************************/

struct Chunk *CreateTHAPChunk( struct Filetracker *ft )
{
	struct Chunk *cnk;

	if( cnk = AllocVec( sizeof( struct Chunk ), MEMF_ANY|MEMF_CLEAR ) )
	{
		if( ft )
			ft->ft_ChunkCount++;
		cnk->ch_TypeID = ID_THAP;
		cnk->ch_Edit = OpenNewTHAPWindow;
		cnk->ch_Free = FreeTHAPChunk;
		cnk->ch_MakeInfoString = MakeTHAPInfoString;
		cnk->ch_Saver = THAPSaver;
		cnk->ch_ParentFile = ft;
		cnk->ch_Modified = FALSE;
		NewList( &cnk->ch_DataList );			/* Used to list Path structs. */

/*
		if( path = CreatePath() )
		{
			strcpy( path->pth_Name, "TESTPATH" );
			AddTail( &cnk->ch_DataList, (struct Node *)path );
		}
*/
	}
	return( cnk );
}

/**************************************************************/

static BOOL MakeTHAPInfoString( struct Chunk *cnk, UBYTE *buf )
{
	UBYTE workbuf[256];

	if( cnk->ch_ParentFile )
		sprintf( workbuf,"%d Paths (from %s)",
			CountNodes( &cnk->ch_DataList),
			cnk->ch_ParentFile->ft_File );
	else
		sprintf( workbuf,"%d Paths", CountNodes( &cnk->ch_DataList) );

	Mystrncpy( buf, workbuf, CHUNKINFOSTRINGSIZE-1 );

	return( TRUE );
}

/**************************************************************/

VOID FreeTHAPChunk( struct Chunk *cnk )
{
	struct Node *foo;

	if( cnk->ch_TypeID == ID_THAP )
	{

		while( !IsListEmpty( &cnk->ch_DataList ) )
		{
			foo = cnk->ch_DataList.lh_Head;
			Remove( foo );
			FreePath( (struct Path *)foo );
		}

		RemoveChunkFromParentFile( cnk );
		FreeVec( cnk );
	}
}




/**************************************************************/

BOOL THAPSaver( struct Chunk *cnk, struct IFFHandle *iff )
{
	BOOL allok;
	struct Path *path;
	struct DiskFormatPathHeader dfph;
	LONG numpaths;

	allok = TRUE;
	D(bug( "THAPSaver()\n" ) );

	if( !(PushChunk( iff, ID_CONK, ID_THAP, IFFSIZE_UNKNOWN ) ) )
	{
		numpaths = CountNodes( &cnk->ch_DataList );
		if( WriteChunkBytes( iff, &numpaths, 4 ) != 4 )
			allok = FALSE;

		path = (struct Path *)cnk->ch_DataList.lh_Head;
		while( numpaths-- && allok )
		{
			/* setup the path header */
			PackASCII( path->pth_Name, dfph.PackName, PATHNAMESIZE-1 );
			if( !IsListEmpty( &cnk->ch_DataList ) )
			{
				dfph.XOrigin =
					(LONG)((struct PathSeg *)path->pth_Segments.mlh_Head)->psg_XPos;
				dfph.YOrigin =
					(LONG)((struct PathSeg *)path->pth_Segments.mlh_Head)->psg_YPos;
			}
			else
			{
				dfph.XOrigin = 0;
				dfph.YOrigin = 0;
			}

			/* write path header */
			if( WriteChunkBytes( iff, &dfph, sizeof( struct DiskFormatPathHeader ) ) ==
				sizeof( struct DiskFormatPathHeader ) )
			{
				/* write sourcedata */
				if( allok = WritePathSource( iff, path ) )
				{

					/* write compiled data */
					allok = WritePathCompiled( iff, path );
				}
			}
			else
				allok = FALSE;

			/* next path... */
			path = (struct Path *)path->pth_Node.ln_Succ;
		}
		PopChunk( iff );
	}
	else
		allok = FALSE;

	return( allok );
}


/**************************************************************/

struct Chunk *THAPLoader( struct IFFHandle *iff, ULONG size,
	struct Filetracker *ft )
{
	struct Chunk *cnk;
	struct Path *path;
	BOOL allok;
	LONG numpaths;

	allok = TRUE;
	if( cnk = CreateTHAPChunk( ft ) )
	{
		if( ReadChunkBytes( iff, &numpaths, 4 ) == 4 )
		{
			while( numpaths-- && allok )
			{
				if( path = ReadPath( iff ) )
					AddTail( &cnk->ch_DataList, (struct Node *)path );
				else
					allok = FALSE;
			}
		}
		else
			allok = FALSE;
	}

	if( !allok )
	{
		FreeTHAPChunk( cnk );
		cnk = NULL;
	}

	return( cnk );
}


/**************************************************************/
//
// Creates a new path.
// Optionally creates an HEAD node at x,y
//

struct Path *CreatePath( BOOL headnode, WORD x, WORD y )
{
	struct Path *path;
	struct PathSeg *psg;

	if( path = AllocVec( sizeof( struct Path ), MEMF_ANY|MEMF_CLEAR ) )
	{
		path->pth_Node.ln_Name = path->pth_Name;
		NewList( (struct List *)&path->pth_Segments );

		if( headnode )
		{
			/* add head node */
			if( psg = AllocVec( sizeof( struct PathSeg ), MEMF_ANY|MEMF_CLEAR ) )
			{
				psg->psg_Type = PSGTYPE_HEAD;
				psg->psg_XPos = x;
				psg->psg_YPos = y;
				AddTail( (struct List *)&path->pth_Segments, (struct Node *)psg );
			}

			/* add lineseg */
			if( psg = AllocVec( sizeof( struct PathSeg ), MEMF_ANY|MEMF_CLEAR ) )
			{
				psg->psg_Type = PSGTYPE_LINE;
				psg->psg_XPos = x+20;
				psg->psg_YPos = y;
				psg->psg_Speed = 32;
				AddTail( (struct List *)&path->pth_Segments, (struct Node *)psg );
			}
		}
	}
	return( path );
}

/**************************************************************/

VOID FreePath( struct Path *path )
{
	FreeNodes( (struct List *)&path->pth_Segments );
	FreeVec( path );
}


/****************************************************************/
//

static BOOL WritePathSource( struct IFFHandle *iff, struct Path *path )
{
	UWORD segcount;
	LONG sourcesize;
	BOOL allok;
	struct PathSeg *psg;

	allok = TRUE;
	segcount = CountNodes( (struct List *)&path->pth_Segments );

	sourcesize = ( sizeof( struct PathSeg ) - sizeof( struct MinNode ) ) *
		segcount;
	sourcesize += 2;	/* allow for segcount word */

//	printf("segcount: %d, sourcesize: %d\n",segcount,sourcesize );

	if( WriteChunkBytes( iff, &sourcesize, sizeof( LONG ) ) == sizeof( LONG ) )
	{
		if( WriteChunkBytes( iff, &segcount, 2 ) == 2 )
		{
			psg = (struct PathSeg *)path->pth_Segments.mlh_Head;
			while( segcount-- && allok )
			{
				if( WriteChunkBytes( iff, &psg->psg_Type,
					sizeof( struct PathSeg ) - sizeof( struct MinNode ) ) !=
					sizeof( struct PathSeg ) - sizeof( struct MinNode ) )
				{
					allok = FALSE;
				}
				psg = (struct PathSeg *)psg->psg_Node.mln_Succ;
			}
		}
		else
			allok = FALSE;
	}
	else
		allok = FALSE;

	return( allok );
}


/****************************************************************/

static BOOL WritePathCompiled( struct IFFHandle *iff, struct Path *path )
{
	BOOL allok;
	struct PathSeg *psg;
	ULONG compsiz;
	LONG prevx, prevy;
	WORD outbuf[3];
	int i=0;
	double dist,dx,dy;

	allok = TRUE;
	compsiz = CalcPathCompiledSize( path );
	prevx=0;
	prevy=0;

	if( WriteChunkBytes( iff, &compsiz, 4 ) == 4 )
	{
		for( psg = (struct PathSeg *)path->pth_Segments.mlh_Head;
			psg->psg_Node.mln_Succ;
			psg = (struct PathSeg *)psg->psg_Node.mln_Succ )
		{
			switch( psg->psg_Type )
			{
				case PSGTYPE_HEAD:
					prevx = ((LONG)psg->psg_XPos) << 5;
					prevy = ((LONG)psg->psg_YPos) << 5;
					break;
				case PSGTYPE_LINE:
					dx = (double)(((LONG)psg->psg_XPos) << 5 ) - (double)prevx;
					dy = (double)(((LONG)psg->psg_YPos) << 5 ) - (double)prevy;
					dist = sqrt( (dx*dx) + (dy*dy) );

//					printf("%d:",i);
//					printf("dist: %d ",(int)dist );
//					printf("dx: %d, dy: %d\n",(int)dx,(int)dy);


					if( dist != 0.0 )
					{
						outbuf[0] = 0 - (WORD)(( dist / (double)psg->psg_Speed )+0.5);
						outbuf[1] = (WORD)((((double)psg->psg_Speed * dx) / dist)+0.5);
						outbuf[2] = (WORD)((((double)psg->psg_Speed * dy) / dist)+0.5);

						if( WriteChunkBytes( iff, outbuf, 6 ) != 6 )
							allok = FALSE;
//						printf("%d steps, %d,%d\n",outbuf[0],outbuf[1],outbuf[2] );
					}

					prevx = prevx + ( ( 0 - outbuf[0] ) * outbuf[1] );
					prevy = prevy + ( ( 0 - outbuf[0] ) * outbuf[2] );

					break;
				case PSGTYPE_BEZCURVE:
					D(bug( "Fuckup...BezCurve\n" ) );
					break;
			}
			i++;
		}

		/* write end word */
		outbuf[0] = 0;
		if( WriteChunkBytes( iff, &outbuf, 2 ) != 2 )
			allok = FALSE;

//		printf("DONE: %d segments\n",i);
	}
	else
		allok = FALSE;

	return( allok );
}



/****************************************************************/

static struct Path *ReadPath( struct IFFHandle *iff )
{
	struct DiskFormatPathHeader dfph;
	struct Path *path;
	struct PathSeg *psg;
	BOOL allok;
	UBYTE *p;
	UWORD segcount;
	LONG foo;

//	printf("Read Path:");

	allok = TRUE;

	path = CreatePath( FALSE, 0, 0 );
	if( !path )
		return( NULL );

	if( ReadChunkBytes( iff, &dfph, sizeof( struct DiskFormatPathHeader ) ) ==
		sizeof( struct DiskFormatPathHeader ) )
	{
		UnpackASCII( dfph.PackName, path->pth_Name, PATHNAMESIZE-1 );

//		printf( "%s, %ld segments\n", path->pth_Name, (LONG)segcount );

		if( ReadChunkBytes( iff, &foo, 4 ) == 4 )		/* skip sourcesize longword */
		{
			if( ReadChunkBytes( iff, &segcount, 2 ) == 2 )
			{

				while( segcount-- && allok )
				{
//					printf( "   %ld\n",(LONG)segcount );

					if( psg = AllocVec( sizeof( struct PathSeg ), MEMF_ANY|MEMF_CLEAR ) )
					{
						if( ReadChunkBytes( iff, &psg->psg_Type,
							sizeof( struct PathSeg ) - sizeof( struct MinNode ) ) ==
							sizeof( struct PathSeg ) - sizeof( struct MinNode ) )
						{
							AddTail( (struct List *)&path->pth_Segments, (struct Node *)psg );
						}
						else
							allok = FALSE;
					}
					else
						allok = FALSE;
				}
	

				if( allok )
				{
					/* skip compiled data */
					if( ReadChunkBytes( iff, &foo, 4 ) == 4 )
					{
						if( foo > 0 )
						{
							if( p = AllocVec( foo, MEMF_ANY ) )
							{
								if( ReadChunkBytes( iff, p, foo ) != foo )
									allok = FALSE;
								FreeVec( p );
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
		}
		else
			allok = FALSE;
	}
	else
		allok = FALSE;

	if( !allok )
	{
		FreePath( path );
		path = NULL;
	}

//	printf(" DONE\n");
	return( path );
}



static ULONG CalcPathCompiledSize( struct Path *path )
{
	struct PathSeg *psg;
	ULONG size;

	size = 0;
	for( psg = (struct PathSeg *)path->pth_Segments.mlh_Head;
		psg->psg_Node.mln_Succ;
		psg = (struct PathSeg *)psg->psg_Node.mln_Succ )
	{
		switch( psg->psg_Type )
		{
			case PSGTYPE_LINE:
				size += 6;				/* WORD type/framecount, WORD xspd, WORD yspd */
				break;
			case PSGTYPE_BEZCURVE:
				D(bug( "Fuckup...BezCurve\n" ) );
				break;
		}
	}

	size += 2;							/* allow for sentinal word */

	return( size );
}
