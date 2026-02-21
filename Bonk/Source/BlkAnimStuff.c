/****************************************************************************/
//
// BONK: BlkAnimStuff.c
//
/****************************************************************************/


#include <stdio.h>
//#include <string.h>
//#include <dos/dos.h>
//#include <dos/dostags.h>
#include <exec/exec.h>
//#include <intuition/intuition.h>
//#include <graphics/gfxbase.h>
//#include <rexx/rxslib.h>
//#include <libraries/gadtools.h>
//#include <libraries/diskfont.h>
//#include <libraries/asl.h>
//#include <libraries/iffparse.h>

#include <proto/exec.h>
//#include <proto/dos.h>
#include <proto/intuition.h>
//#include <proto/graphics.h>
//#include <proto/diskfont.h>
//#include <proto/asl.h>
#include <clib/alib_protos.h>
//#include <proto/gadtools.h>
//#include <proto/iffparse.h>



#include <global.h>
#include <ExternVars.h>





struct BlkAnim *CreateNewBlkAnim( WORD length )
{
	struct BlkAnim *ba;

	if( ba = AllocVec( sizeof( struct BlkAnim ), MEMF_ANY|MEMF_CLEAR ) )
	{
		ba->ba_Length = length;
	}
	printf(" CreateNewBlkAnim: ba=%ld\n", ba );
	return( ba );
}


