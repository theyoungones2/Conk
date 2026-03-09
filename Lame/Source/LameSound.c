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
#include <lame.h>
#include <mycustom.h>
#include <debug.h>
#include <protos.h>


/*****************   CookSFX()   *****************************/
//

UWORD CookSFX( struct LameReq *lr )
{
	struct FileNode		*fn;
	struct ChunkNode	*cn;
	UWORD							 err = LAMERR_ALLOK;
	struct SpamParam	*spam;
	struct SFXNode		*sfx;
	UBYTE							*ptr;
	ULONG							 numofsamples,count;

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
				if( cn->cn_TypeID == ID_SFX )
				{
					/* Found a SFX chunk */

					D(bug("---  Cook SFX --- %s\n",cn->cn_Node.ln_Name));
					ptr = cn->cn_Data;
					numofsamples = *(ULONG *)ptr; ptr += 4;
					for(count = 0; (count < numofsamples) && (err < LAME_FAILAT); count++)
					{
						if(sfx = MYALLOCVEC(sizeof(struct SFXNode), MEMF_ANY|MEMF_CLEAR,"SFXNode"))
						{
							spam = (struct SpamParam *)ptr; ptr += sizeof(struct SpamParam);
							AddTail(&sfxlist, &sfx->sfx_Node);
							sfx->sfx_Node.ln_Name = spam->spam_PackedName;
							sfx->sfx_SpamParam = spam;
							ptr += spam->spam_Length;
						}
						else
						{
							err = LAMERR_MEM;
							sprintf(errorstring, "Error Code: %ld\nDescription: Not Enough Memory\n", err);
							DisplayError(errorstring);
						}
					}
					cn->cn_Cooked = TRUE;
				}
			}
		}
	}
	return(err);
}


/****************   FreeCookedSFX()   ************************/

void 	FreeCookedSFX( void )
{
	struct Node *node;
	while( !IsListEmpty( &sfxlist ) )
	{
		node = sfxlist.lh_Head;
		Remove( node );
		MYFREEVEC( node );
	}
}

/****************   CheckUniqueSFXNames()   ************/
//
//  Goes through all the Sound Effects and makes sure
// that all the names are unique.

void CheckUniqueSFXNames(void)
{
	struct SFXNode *sfx;
	UBYTE	namebuffer[SFXFULLNAMESIZE];
	UWORD err = LAMERR_ALLOK;

	for(sfx = (struct SFXNode *)sfxlist.lh_Head;
			sfx->sfx_Node.ln_Succ && sfx->sfx_Node.ln_Succ->ln_Succ;
			sfx = (struct SFXNode *)sfx->sfx_Node.ln_Succ)
	{
		if(!NullName(sfx->sfx_Node.ln_Name))
		{
			if(FindCompressedName(&sfxlist, sfx->sfx_Node.ln_Name, sfx->sfx_Node.ln_Succ))
			{
				err = LAMERR_NAME_NOT_UNIQUE;
				UnpackASCII(sfx->sfx_Node.ln_Name, namebuffer, SFXFULLNAMESIZE-1);
				sprintf(errorstring, "Warning %ld: Sound Effect Name '%s' Not Unique\n", err, namebuffer);
				DisplayWarning(errorstring);
			}
		}
		else
		{
			err = LAMERR_EMPTY_NAME;
			sprintf(errorstring, "Warning %ld: Sound Effect Has No Name\n", err);
			DisplayWarning(errorstring);
		}
	}
}
