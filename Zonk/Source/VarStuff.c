/********************************************************/
//
// VarStuff.c
//
/********************************************************/

#define VARSTUFF.C

#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <ctype.h>

#include <dos/dos.h>
#include <dos/dostags.h>
#include <exec/exec.h>
#include <intuition/intuition.h>
//#include <graphics/gfxbase.h>
#include <libraries/gadtools.h>
#include <libraries/diskfont.h>
#include <libraries/asl.h>
#include <libraries/iffparse.h>

#include <proto/exec.h>
#include <proto/dos.h>
#include <proto/intuition.h>
#include <proto/graphics.h>
#include <proto/diskfont.h>
#include <proto/asl.h>
#include <clib/alib_protos.h>
#include <proto/gadtools.h>
//#include <proto/iffparse.h>

#include "hoopy.h"
#include "global.h"


#define	VARTYPE_ALL			0	//not really a type
#define	VARTYPE_OBJECT	1
#define	VARTYPE_GLOBAL	2
#define	VARTYPE_OBS			3


#define VARCOUNT 26

struct Node varnodes[ VARCOUNT ] = {
	{ NULL, NULL, VARTYPE_OBJECT, 0, "XPos" },
	{ NULL, NULL, VARTYPE_OBJECT, 1, "YPos" },
	{ NULL, NULL, VARTYPE_OBJECT, 2, "XSpd" },
	{ NULL, NULL, VARTYPE_OBJECT, 3, "YSpd" },
	{ NULL, NULL, VARTYPE_OBS, 4, "XMaxSpd" },
	{ NULL, NULL, VARTYPE_OBS, 5, "YMaxSpd" },
	{ NULL, NULL, VARTYPE_OBS, 6, "XAccel" },
	{ NULL, NULL, VARTYPE_OBS, 7, "YAccel" },
	{ NULL, NULL, VARTYPE_OBS, 8, "XDeccel" },
	{ NULL, NULL, VARTYPE_OBS, 9, "YDeccel" },
	{ NULL, NULL, VARTYPE_OBJECT, 10, "Shields" },
	{ NULL, NULL, VARTYPE_OBJECT, 11, "Damage" },
	{ NULL, NULL, VARTYPE_OBJECT, 12, "Data1" },
	{ NULL, NULL, VARTYPE_OBJECT, 13, "Data2" },
	{ NULL, NULL, VARTYPE_OBJECT, 14, "Direction" },
	{ NULL, NULL, VARTYPE_GLOBAL, 15, "Global1" },
	{ NULL, NULL, VARTYPE_GLOBAL, 16, "Global2" },
	{ NULL, NULL, VARTYPE_GLOBAL, 17, "Global3" },
	{ NULL, NULL, VARTYPE_GLOBAL, 18, "Global4" },
	{ NULL, NULL, VARTYPE_GLOBAL, 19, "Global5" },
	{ NULL, NULL, VARTYPE_OBS, 20, "CollisionTypes" },
	{ NULL, NULL, VARTYPE_OBS, 21, "ScrollXPos" },
	{ NULL, NULL, VARTYPE_OBS, 22, "ScrollYPos" },
	{ NULL, NULL, VARTYPE_OBS, 23, "ScrollSpeed" },
	{ NULL, NULL, VARTYPE_OBS, 24, "BaseSpeed" },
	{ NULL, NULL, VARTYPE_OBJECT, 25, "ID" },
};

int FindVariable( STRPTR namefrag )
{
	int i;
	char *p, *q;
	int match = -1;

	for( i=0; i<VARCOUNT && ( match ==- 1 ); i++ )
	{
		p = namefrag;
		q = varnodes[i].ln_Name;

		if( *p && ( varnodes[i].ln_Type != VARTYPE_OBS ) )
		{
			while( *p && *q && ( toupper( *p ) == toupper( *q ) ) )
			{
				p++;
				q++;
			}
			if( !*p )
				match = i;
		}
	}
	return match;
}


// add variables to a list.
// don't free the nodes!
// also...
// don't get them wet,
// don't expose them to sunlight,
// and never ever feed them after midnight.

void ListVars( struct List *list, UBYTE type )
{
	int i;

	for( i=0; i<VARCOUNT; i++ )
	{
		if( ( varnodes[i].ln_Type != VARTYPE_OBS ) &&
			( ( varnodes[i].ln_Type == type ) || ( type == VARTYPE_ALL ) ) )
		{
			AlphaAdd( list, &varnodes[i] );
		}
	}
}



WORD RequestVar( struct Window *win )
{
	UWORD num;
	struct List varlist;
	struct Node *noddy;

	NewList( &varlist );
	ListVars( &varlist, VARTYPE_ALL );
	num = MyListViewRequest( win, "Pick Variable", "Variables:", &varlist );

	if( num == 0xFFFF )
		return -1;
	else
	{
		if( noddy = FindNthNode( &varlist, num ) )
			return (WORD)noddy->ln_Pri;
		else
			return -1;
	}
}
