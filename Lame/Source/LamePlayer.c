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


/*******************  PreCookPlayers()   ****************************/

UWORD PreCookPlayers( struct LameReq *lr )
{
	struct ChunkNode *cn;
	struct PlayerNode *playernode;
	struct PaulsBob *prevbob = NULL;
	struct Display *disp;
	UWORD err = LAMERR_ALLOK;
	UBYTE *p;
	UWORD	i;

	if( !( cn = FindChunkByType( ID_DISP ) ) )
	{
		err = LAMERR_NODISPLAY;
		sprintf(errorstring, "Error Code: %ld\nDescription: No Display Chunk Found\n", err);
		DisplayError(errorstring);
		return(err);
	}

	D(bug("--- Pre Cook PLYR's ---\n"));
	disp = (struct Display *)cn->cn_Data;

	D2(bug(" NumOfPlayers %lu\n",disp->dp_NumOfPlayers));
	if(disp->dp_NumOfPlayers > 4)
	{
		err = LAMERR_TOO_MANY_PLAYERS;
		sprintf(errorstring, "Error Code: %ld\nDescription: Too Many Players\nMaximum Number Of Players Is 4\n", err);
		DisplayError(errorstring);
		return(err);
	}

		/* Make sure we allocate enough structures */
	if(disp->dp_NumOfPlayerStructs < disp->dp_NumOfPlayers)
		disp->dp_NumOfPlayerStructs = disp->dp_NumOfPlayers;

	p = disp->dp_PlayerInitProgram1;
	for( i=0; (i < disp->dp_NumOfPlayerStructs) && (err < LAME_FAILAT); i++ )
	{
		if( !(playernode = MYALLOCVEC( sizeof( struct PlayerNode ), MEMF_ANY|MEMF_CLEAR, "Player Node" ) ) )
		{
			err = LAMERR_MEM;
			sprintf(errorstring, "Error Code: %ld\nDescription: Not Enough Memory\n", err);
			DisplayError(errorstring);
			return(err);
		}

		if(prevbob)
			prevbob->bb_PaulsObject.ob_Next = (struct PaulsObject *)&playernode->pn_Bob;
		playernode->pn_Bob.bb_PaulsObject.ob_Previous = &prevbob->bb_PaulsObject;
		playernode->pn_Bob.bb_PaulsObject.ob_Next = NULL;
		playernode->pn_Bob.bb_PaulsObject.ob_ScreenOffset1 = -1;
		playernode->pn_Bob.bb_PaulsObject.ob_ScreenOffset2 = -1;

		if(i < disp->dp_NumOfPlayers)
		{
			playernode->pn_Bob.bb_Data1 = (ULONG)p;
			p += PROGFILENAMESIZE;
		}
		else
			playernode->pn_Bob.bb_Data1 = 0;

		prevbob = &playernode->pn_Bob;
		/* add the playernode to the playerlist */
		AddTail( &playerlist, &playernode->pn_Node );
	}
	/* Not fully cooked yet so I'll wait, until post cook players */

	return(err);
}

/*******************  PostCookPlayers()   ****************************/

UWORD PostCookPlayers( struct LameReq *lr )
{
	struct PlayerNode *playernode;
	struct ProgNode		*pn;
	UWORD err = LAMERR_ALLOK, plyrnum = 1;
	UBYTE	namebuffer[PROGFULLNAMESIZE];

	D(bug("--- Post Cook PLYR's ---\n"));

	for(playernode = (struct PlayerNode *)playerlist.lh_Head;
			(playernode->pn_Node.ln_Succ) && (err < LAME_FAILAT);
			playernode = (struct PlayerNode *)playernode->pn_Node.ln_Succ)
	{
		if(playernode->pn_Bob.bb_Data1)
		{
			if(!NullName((UBYTE *)playernode->pn_Bob.bb_Data1))
			{
				D2(UnpackASCII((UBYTE *)playernode->pn_Bob.bb_Data1, namebuffer, PROGFULLNAMESIZE-1));
				D2(bug(" InitProgram: '%s'\n",namebuffer));
				if( pn = (struct ProgNode *)FindCompressedName(&proglist, (char *)playernode->pn_Bob.bb_Data1, NULL) )
					playernode->pn_Bob.bb_Data1 = (ULONG)pn->pn_Prog;
				else
				{
					err = LAMERR_PROGNOTFOUND;
					UnpackASCII((UBYTE *)playernode->pn_Bob.bb_Data1, namebuffer, PROGFULLNAMESIZE-1);
					sprintf(errorstring, "Error Code: %ld\nAction List: %s\nDescription: Action List Not Found\nFor Player Number: %ld\n", err, namebuffer, plyrnum);
					DisplayError(errorstring);
				}
			}
			else
			{
				playernode->pn_Bob.bb_Data1 = NULL;
				D2(bug(" InitProgram: NULL"));
			}
		}
		plyrnum++;
	}
	return(err);
}

/****************   FreeCookedPlayers()   ************************/

void FreeCookedPlayers( void )
{
	struct PlayerNode *playernode;
	while( !IsListEmpty( &playerlist ) )
	{
		playernode = (struct PlayerNode *)playerlist.lh_Head;
		Remove( &playernode->pn_Node );
		MYFREEVEC( playernode );
	}
}
