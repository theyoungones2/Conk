
#include <stdio.h>
#include <string.h>
#include <exec/exec.h>
#include <intuition/intuition.h>

#include <proto/exec.h>
#include <proto/intuition.h>
#include <proto/graphics.h>
#include <clib/alib_protos.h>


#include <global.h>
#include <ExternVars.h>


/************ OpenNewAboutWindow ************/
//

BOOL OpenNewAboutWindow( void )
{
	struct List *l;

	if( l = BlockAllWindows() )
	{
		GroovyReq( PROGNAME, "Version 0.90 (Beta)\nBy Ben Campbell\n23/5/96 Release\n\n©1996 42° South", "Groovy." );
	
		FreeBlockedWindows( l );
	}
	return TRUE;
}
