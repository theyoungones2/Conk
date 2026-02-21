#define ZONKMISC_C
#include <exec/exec.h>
#include <intuition/intuition.h>
#include <intuition/classes.h>
#include <intuition/classusr.h>
#include <intuition/imageclass.h>
#include <intuition/gadgetclass.h>
#include <libraries/gadtools.h>
#include <graphics/displayinfo.h>
#include <graphics/gfxbase.h>
#include <proto/exec.h>
#include <proto/dos.h>
#include <proto/diskfont.h>
#include <proto/intuition.h>
#include <proto/gadtools.h>
#include <proto/graphics.h>
#include <proto/utility.h>
#include <proto/layers.h>

#include <clib/alib_protos.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>

#include "Global.h"


/*******************  ClearWindow()  *******************/
//
// Clears the interior of a window without zapping crap
// out of the borders.
//
// Probably.
//

void ClearWindow( struct Window *win )
{
	SetAPen( win->RPort, 0 );
	RectFill(win->RPort,
		win->BorderLeft,
		win->BorderTop,
		win->Width - win->BorderRight - 1,
		win->Height - win->BorderBottom - 1 );
}


/******************  MyTextLength()  ******************/
//
// Like the system TextLength() function, just doesn't
// need a Rastport to figure it out (so you can suss out
// stuff _before_ you open the window).
//
// inputs:
// tf:  TextFont struct of the font being used
// str: null-terminated string to be measured

WORD MyTextLength( struct TextFont *tf, char *str )
{
	WORD width = 0;

	assert( tf != NULL );
	assert( str != NULL );

	if( tf->tf_Flags & FPF_PROPORTIONAL )
	{
		/* proportional font */
		for( ; *str != '\0'; str++ )
		{
			width += ((WORD *)tf->tf_CharKern)[ *str - tf->tf_LoChar ];
			width += ((WORD *)tf->tf_CharSpace)[ *str - tf->tf_LoChar ];
		}
	}
	else
	{
		/* fixed-space font */
		width = tf->tf_XSize * strlen( str );
//		printf("%s\n", str );
	}
	return( width );
}


/*********************  FindMaxStrLen()  *********************/
//
// Find the TextLength (in pixels) of the longest string in
// an NULL terminated STRPTR array, using the given font.

UWORD FindMaxStrLen( STRPTR *str, struct TextFont *tf )
{
	UWORD i, len, maxlen = 0;

	for( i = 0; str[i]; i++ )
	{
			len = MyTextLength( tf, str[i] );
			if( len > maxlen ) maxlen = len;
	}
	return( maxlen );
}

/*********************  CountStrings()  *********************/
//
// Count the number of strings in a NULL terminated STRPTR array.
//

UWORD CountStrings( STRPTR *str )
{
	UWORD i=0;

	for( i = 0; str[i]; i++ );
	return( i );
}


/*******************************************************/
// ComposeScreenModeString()
//
// Fill a string in with a screenmode name and number of colours.
// Note: no length checking done, so make sure str buffer is a reasonable
// size.
//

void ComposeScreenModeString( STRPTR str, ULONG displayid, UWORD depth )
{
	DisplayInfoHandle handle;
	struct NameInfo query;

	if( handle = FindDisplayInfo( displayid ) )
	{
		if( GetDisplayInfoData( handle, (UBYTE *)&query, sizeof( struct NameInfo ),
			DTAG_NAME,NULL ))
		{
			sprintf( str,"%s, %ld colours",query.Name, (ULONG)1l<<depth );
		}
	}
}



/*********************  Mystrncpy()  ***********************/
//  This is just a slight varience from the original strncpy()
// in that this puts a null terminator on the string, after
// the last character, so that Mystrncpy(to,from,15) would copy
// the first 15 characters then put in a terminator.

char *Mystrncpy(char *to, char *from, int length)
{
	register char *dest = to;
	register char *src = from;

	while( length-- && *src )
		*dest++ = *src++;
	*dest = '\0';

	return( to );
}




/*********************  MyStrDup()  ***********************/
// Makes a copy of a string in AllocVec()ed memory.
// (Free string with FreeVec() )

char *MyStrDup(char *source)
{
	char *dest;

	if( dest = AllocVec( strlen( source ) + 1 , MEMF_ANY) )
		strcpy(dest,source);
	return(dest);
}


/*********************  CountNodes()  ***********************/
//
// Returns the number of nodes attached to a List.

ULONG CountNodes( struct List *l )
{
	ULONG c;
	struct Node *noddy;

	c=0;
	for( noddy = l->lh_Head; noddy->ln_Succ; noddy = noddy->ln_Succ )
		c++;
	return( c);
}

/*******************  FindNthNode()  *******************/
//
// Returns a ptr to the Nth node in the specified list.
// The counting begins at zero (0 = the node after the head node).
// NULL is returned if the node doesn't exist.
//

struct Node *FindNthNode(struct List *list, UWORD num)
{
	struct Node *node;

	if( IsListEmpty(list) ) return(NULL);
	node = list->lh_Head;
	while( num )
	{
		node = node->ln_Succ;
		if(!node->ln_Succ) return(NULL);
		--num;
	}
	return(node);
}


/*******************  BubbleUp()  *******************/

BOOL BubbleUp( struct List *glub, struct Node *noddy )
{
	BOOL doneit = FALSE;
	struct Node *pred;

	pred = noddy->ln_Pred;
	if( pred->ln_Pred )
	{
		Remove( noddy );
		Insert( glub, noddy, pred->ln_Pred );
		doneit = TRUE;
	}
	return(doneit);
}

/*******************  BubbleDown()  *******************/

BOOL BubbleDown( struct List *glub, struct Node *noddy )
{
	BOOL doneit = FALSE;
	struct Node *succ;

	succ = noddy->ln_Succ;
	if( succ->ln_Succ )
	{
		Remove( noddy );
		Insert( glub, noddy, succ );
		doneit = TRUE;
	}
	return(doneit);
}

/*******************  MyStrToUpper()  *******************/

char *MyStrToUpper( char *str )
{
	register char *p = str;
	while( *p )
	{
		*p = toupper( *p );
		p++;
	}
	return( str );
}

/**************************************************************/
// Remove()s and FreeVec()s all nodes in the given list.
//

VOID FreeNodes( struct List *l )
{
	struct Node *noddy;

	while( !IsListEmpty( l ) )
	{
		noddy = l->lh_Head;
		Remove( noddy );
		FreeVec( noddy );
	}
}


/***************** Create BitMap **********************/
//  Given the right details it will create, initialise and
// allocate a bitmap structure. (and BitMap)
// Return: NULL = Not enuf memory.

struct BitMap *CreateBitMap(UWORD width, UWORD height, UWORD depth, BOOL interleaved)
{
	struct BitMap *bm;
	PLANEPTR allocated = (PLANEPTR) 1;
	UWORD i;
	ULONG flags;

	flags = interleaved ? BMF_INTERLEAVED|BMF_CLEAR : BMF_CLEAR;

	if(GfxBase->LibNode.lib_Version >= 39)
	{
		bm = AllocBitMap(width,height,depth,flags,NULL);
	}
	else
	{
		if(bm = AllocVec(sizeof(struct BitMap), MEMF_ANY|MEMF_CLEAR))
		{
			InitBitMap(bm,depth,width,height);
			for(i=0; ((i < depth) && (allocated)); i++)
				allocated = (bm->Planes[i] = AllocRaster(width,height));
	
			if (allocated == NULL)
			{
				DestroyBitMap(bm,width,height,depth);
				bm = NULL;
			}
		}
	}
	return(bm);
}

/************************** Destroy BitMap *******************/
// Counterpart to CreateBitMap
//
void DestroyBitMap(struct BitMap *bm, UWORD width, UWORD height, UWORD depth)
{
	int i;

	if(bm)
	{
		if(GfxBase->LibNode.lib_Version >= 39)
		{
			FreeBitMap(bm);
		}
		else
		{
			for(i=0; (i<depth); i++)
			{
				if(bm->Planes[i])
					FreeRaster(bm->Planes[i], width, height);
			}
			FreeVec(bm);
		}
	}
}

/*******************  WritePixelClipped()  *******************/

VOID WritePixelClipped( struct RastPort *rp, LONG x, LONG y,
	struct Rectangle *cliprect )
{
	if( x >= cliprect->MinX && x <= cliprect->MaxX &&
		y >= cliprect->MinY && y <= cliprect->MaxY )
	{
		WritePixel( rp, x, y );
	}
}


/*******************  RectFillClipped()  *******************/
VOID RectFillClipped( struct RastPort *rp, LONG minx, LONG miny,
	LONG maxx, LONG maxy,	struct Rectangle *cliprect )
{
	if( minx < cliprect->MinX ) minx = cliprect->MinX;
	if( maxx > cliprect->MaxX ) maxx = cliprect->MaxX;
	if( miny < cliprect->MinY ) miny = cliprect->MinY;
	if( maxy > cliprect->MaxY ) maxy = cliprect->MaxY;

	if( minx < maxx && miny < maxy )
		RectFill( rp, minx, miny, maxx, maxy );
}

#define TOPMASK 1
#define BOTTOMMASK 2
#define LEFTMASK 4
#define RIGHTMASK 8

UWORD CompOutCode( LONG x, LONG y, struct Rectangle *cliprect )
{
	UWORD outcode = 0;

	if( y > cliprect->MaxY )
		outcode = TOPMASK;
	else
		if( y < cliprect->MinY ) outcode = BOTTOMMASK;
	if( x > cliprect->MaxX )
		outcode |= RIGHTMASK;
	else
		if( x < cliprect->MinX ) outcode |= LEFTMASK;
	return( outcode );
}


/*******************  DrawLineClipped()  *******************/
VOID DrawLineClipped( struct RastPort *rp, LONG x0, LONG y0,
	LONG x1, LONG y1,	struct Rectangle *cliprect )
{
	BOOL done = FALSE, accept = FALSE;
	UWORD	outcode0, outcode1, outcodeout;
	LONG x,y;

	outcode0 = CompOutCode( x0, y0, cliprect );
	outcode1 = CompOutCode( x1, y1, cliprect );

	do
	{
		if( !outcode0 && !outcode1 )
		{
			accept = TRUE;
			done = TRUE;
		}
		else
		{
			if( outcode0 & outcode1 )
				done = TRUE;
			else
			{
				if( outcode0 )
					outcodeout = outcode0;
				else
					outcodeout = outcode1;

				if( outcodeout & TOPMASK )
				{
					x = x0 + ( x1 - x0 ) * ( cliprect->MaxY - y0 ) / ( y1 - y0 );
					y = cliprect->MaxY;
				}
				else
				{
					if( outcodeout & BOTTOMMASK )
					{
						x = x0 + ( x1 - x0 ) * ( cliprect->MinY - y0 ) / ( y1 - y0 );
						y = cliprect->MinY;
					}
					else
					{
						if( outcodeout & RIGHTMASK )
						{
							y = y0 + ( y1 - y0 ) * ( cliprect->MaxX - x0 ) / ( x1 - x0 );
							x = cliprect->MaxX;
						}
						else
						{
							if( outcodeout & LEFTMASK )
							{
								y = y0 + ( y1 - y0 ) * ( cliprect->MinX - x0 ) / ( x1 - x0 );
								x = cliprect->MinX;
							}
						}
					}
				}

				if( outcodeout == outcode0 )
				{
					x0 = x;
					y0 = y;
					outcode0 = CompOutCode( x0, y0, cliprect );
				}
				else
				{
					x1 = x;
					y1 = y;
					outcode1 = CompOutCode( x1, y1, cliprect );
				}
			}
		}
	} while( !done );

	if( accept )
	{
		Move( rp, x0, y0 );
		Draw( rp, x1, y1 );
	}
}


/*******************  FindNodeNum()  *******************/
//
// Figures out the position of a node in a list.
// returns position (counting from 0) or ~0 if off list.
//

UWORD FindNodeNum( struct List *list, struct Node *noddy )
{
	WORD num,i;
	struct Node *bigears;

	num = ~0;
	i=0;
	for( bigears = list->lh_Head;
		bigears->ln_Succ && num == ~0;
		bigears = bigears->ln_Succ )
	{
		if( bigears == noddy )
			num = i;
		i++;
	}
	return( (UWORD)num );
}


/*******************  MakeBackupFile()  *******************/
//
// Tries to make a backup file (.bak) by renaming the given filename.
// If the file specified doesn't exist, the old backup (if any) is
// left untouched.
//
// Returns success - may fail if Rename() fails.

BOOL MakeBackupFile( STRPTR filename )
{
	BOOL allok;
	UBYTE bakname[256];
	BPTR lock;

	allok = TRUE;

	/* create backup name */
	Mystrncpy( bakname, filename, 256-4-1 );
	strcat( bakname,".bak" );

	if( lock = Lock( filename, ACCESS_READ ) )
	{
		/* file exists */
		UnLock( lock );
		DeleteFile( bakname );
		if( !Rename( filename, bakname ) )
			allok = FALSE;			/* fail if we couldn't make the backup */
	}
	/* if file doesn't exist just keep the old backup and exit */

	return( allok );
}



/*******************  UnclipWindow()  *******************/

VOID UnclipWindow( struct Window *win )
{
	struct Region *oldregion;

	if( oldregion = InstallClipRegion( win->WLayer, NULL ) )
		DisposeRegion( oldregion );
}

/*******************  ClipWindow()  *******************/

struct Region *ClipWindow( struct Window *win, LONG minx, LONG miny,
	LONG maxx, LONG maxy )
{
	struct Region *newregion;
	struct Rectangle myrectangle;

	myrectangle.MinX = minx;
	myrectangle.MinY = miny;
	myrectangle.MaxX = maxx;
	myrectangle.MaxY = maxy;

	if( newregion = NewRegion() )
	{
		if( !OrRectRegion( newregion, &myrectangle ) )
		{
			DisposeRegion( newregion );
			 newregion = NULL;
		}
	}
	return( InstallClipRegion( win->WLayer, newregion ) );
}

/*******************  AppendToDataList()  *******************/
// Create a DataNode struct for each node in srclist, and add to
// the tail of dnlist.
// dnlist must be an initialised List (may have any number of nodes).
// The created DataNodes have ln_Name copied from the source node, and
// dn_Data pointing to the source node itself.
//
// Returns success. If failure, _some_ nodes might have been added.

BOOL AppendToDataList( struct List *dnlist, struct List *srclist )
{
	struct Node *noddy;
	struct DataNode *dn;
	BOOL allok;

	allok = TRUE;

	for( noddy = srclist->lh_Head;
		noddy->ln_Succ && allok;
		noddy = noddy->ln_Succ )
	{
		if( dn = AllocVec( sizeof( struct DataNode ), MEMF_ANY|MEMF_CLEAR ) )
		{
			dn->dn_Data = (APTR)noddy;
			dn->dn_Node.ln_Name = noddy->ln_Name;
			AddTail( dnlist, &dn->dn_Node );
		}
		else
			allok = FALSE;
	}
	return( allok );
}


/*******************  AlphaAdd()  *******************/
void AlphaAdd( struct List *list, struct Node *noddy )
{
	struct Node *n;

	n = list->lh_Head;
	while( n->ln_Succ && ( strcmpi( n->ln_Name, noddy->ln_Name ) < 0 ) )
		n = n->ln_Succ;
	Insert( list, noddy, n->ln_Pred );
}

/*******************  DumpList()  *******************/
void DumpList( struct List *l )
{
	int i=0;
	struct Node *n;

	for( n = l->lh_Head; n->ln_Succ; n=n->ln_Succ )
	{
		printf("%d: %s\n",i++,n->ln_Name );
	}
}



