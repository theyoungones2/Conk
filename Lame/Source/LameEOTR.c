#include <stdarg.h>
#include <clib/alib_stdio_protos.h>
#include <exec/exec.h>
#include <dos/dos.h>
#include <graphics/modeid.h>
#include <graphics/display.h>
#include <graphics/gfxbase.h>
#include <hardware/intbits.h>
#include <proto/exec.h>
#include <proto/dos.h>
#include "lame.h"
#include "mycustom.h"
#include "debug.h"
#include "protos.h"
#include "Game.h"

/*******************  CookEdgeOfTheRoad()   ****************************/

UWORD CookEdgeOfTheRoad( struct LameReq *lr )
{
	struct FileNode *fn;
	struct ChunkNode *cn;
	struct DiskEOTRHeader *de;
	UWORD err = LAMERR_ALLOK;

	for( fn = (struct FileNode *)filelist.lh_Head;
		fn->fn_Node.ln_Succ && err < LAME_FAILAT;
		fn = (struct FileNode *)fn->fn_Node.ln_Succ )
	{
		if( fn->fn_UsedInLevel )
		{
			for( cn = (struct ChunkNode *)fn->fn_ChunkList.lh_Head;
				cn->cn_Node.ln_Succ && err < LAME_FAILAT;
				cn = (struct ChunkNode *)cn->cn_Node.ln_Succ )
			{
				if( cn->cn_TypeID == ID_EOTR )
				{
					/* Found a EOTR chunk */
					D(bug("--- Cook EOTR ---\n"));

					de = (struct DiskEOTRHeader *)cn->cn_Data;
					de->Noddy.ln_Name = de->Name;
					AddTail(&eotrlist, &de->Noddy);
					cn->cn_Cooked = TRUE;
				}
			}
		}
	}
	return(err);
}

/****************   FreeCookedEdgeOfTheRoad()   ************************/
// Doesn't really do anything, cos the list is automagically zapped
// at the end of the level anyway, but thought I'd put it in anyway.

void 	FreeCookedEdgeOfTheRoad( void )
{
	struct DiskEOTRHeader *de;

	while(!IsListEmpty(&eotrlist))
	{
		de = (struct DiskEOTRHeader *)eotrlist.lh_Head;
		Remove( &de->Noddy );
	}
}
