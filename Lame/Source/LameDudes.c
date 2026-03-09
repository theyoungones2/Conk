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


/******************   ReadTHAPChunk()   **********************/
UWORD ReadTHAPChunk( BPTR fh, ULONG len, struct FileNode *fn )
{
	UWORD err = LAMERR_ALLOK;
	struct ChunkNode *cn;
	struct PathNode *pn = NULL, *prevpn = NULL;
	struct PathHeader tmpheader;
	ULONG	numofpaths,pathnum,sourcesize,compiledsize;
	BOOL firstpath = TRUE;

	D(bug(" Reading THAP chunk..."));

	if( cn = MYALLOCVEC( sizeof( struct ChunkNode ), MEMF_ANY|MEMF_CLEAR, "Thap ChunkNode" ) )
	{
		if( Read( fh, &numofpaths, 4 ) == 4 )
		{
			D(bug("Num Of Paths: %ld\n",numofpaths));
			for(pathnum = 0; (pathnum < numofpaths) && (err < LAME_FAILAT); pathnum++)
			{
				D(bug("Path Num: %ld\n",pathnum));

				if(pn = MYALLOCVEC(sizeof(struct PathNode),MEMF_ANY|MEMF_CLEAR,"PathNode"))
				{
					AddTail(&pathlist,&pn->pn_Node);
					if(prevpn) prevpn->pn_NextPathNodeInChunk = pn;
					if(firstpath)
					{
						cn->cn_FirstItem = pn;
						firstpath = FALSE;
					}
					if( Read(fh, &tmpheader, sizeof(struct PathHeader)) == sizeof(struct PathHeader) )
					{
						if( Read( fh, &sourcesize, 4 ) == 4 )
						{
							D(bug("Source Size: %ld\n",sourcesize));
							if(sourcesize != 0)
								Seek( fh, sourcesize, OFFSET_CURRENT );

							if( Read( fh, &compiledsize, 4 ) == 4 )
							{
								D(bug("Compiled Size: %ld\n",compiledsize));
								if( pn->pn_PathHeader = MYALLOCVEC(compiledsize+sizeof(struct PathHeader), MEMF_ANY|MEMF_CLEAR,"PathHeader") )
								{
									CopyMem(&tmpheader, pn->pn_PathHeader, sizeof(struct PathHeader));
									pn->pn_Node.ln_Name = pn->pn_PathHeader->ph_PackedName;
									if( Read( fh, pn->pn_PathHeader->ph_Data, compiledsize ) == compiledsize )
									{
										prevpn = pn;
									}
									else
									{
										/* Read Path Header */
										err = LAMERR_READ;
										sprintf(errorstring, "Error Code: %ld\nFile: %s\nDescription: Read Error\n", err, fn->fn_Node.ln_Name);
										DisplayError(errorstring);
									}
								}
								else
								{
									/* Path Header */
									err = LAMERR_MEM;
									sprintf(errorstring, "Error Code: %ld\nDescription: Not Enough Memory\n", err);
									DisplayError(errorstring);
								}
							}
							else
							{
								/* Read compiled size */
								err = LAMERR_READ;
								sprintf(errorstring, "Error Code: %ld\nFile: %s\nDescription: Read Error\n", err, fn->fn_Node.ln_Name);
								DisplayError(errorstring);
							}
						}
						else
						{
							/* Read source size */
							err = LAMERR_READ;
							sprintf(errorstring, "Error Code: %ld\nFile: %s\nDescription: Read Error\n", err, fn->fn_Node.ln_Name);
							DisplayError(errorstring);
						}
					}
					else
					{
						/* Read Path Header */
						err = LAMERR_READ;
						sprintf(errorstring, "Error Code: %ld\nFile: %s\nDescription: Read Error\n", err, fn->fn_Node.ln_Name);
						DisplayError(errorstring);
					}
				}
				else
				{
					/* Path Node */
					err = LAMERR_MEM;
					sprintf(errorstring, "Error Code: %ld\nDescription: Not Enough Memory\n", err);
					DisplayError(errorstring);
				}
			}	/* For Next */
		}
		else
		{
			/* Read Num of Paths */
			err = LAMERR_READ;
			sprintf(errorstring, "Error Code: %ld\nFile: %s\nDescription: Read Error\n", err, fn->fn_Node.ln_Name);
			DisplayError(errorstring);
		}

		/* skip pad byte if odd length */
		if( len & 1 ) Seek( fh, 1, OFFSET_CURRENT );

		cn->cn_TypeID = ID_THAP;
		cn->cn_Node.ln_Name = NULL;		// No name string
		cn->cn_Cooked = FALSE;
		cn->cn_Freeable = FALSE;
		AddTail( &fn->fn_ChunkList, (struct Node *)cn );
		D(bug("OK\n"));
	}
	else
	{
		/* Chunk Node */
		err = LAMERR_MEM;
		sprintf(errorstring, "Error Code: %ld\nDescription: Not Enough Memory\n", err);
		DisplayError(errorstring);
	}

	if(err)
	{
		D(bug("Failed.\n"));
	}
	return(err);
}


/*******************  CookDudeFormations()   ****************************/

UWORD CookDudeFormations( struct LameReq *lr )
{
	struct FileNode									*fn = NULL;
	struct ChunkNode								*cn = NULL;
	struct FormationNode						*fnode = NULL;
	struct ProgNode									*pn = NULL;
	UBYTE														*p = NULL;
	struct PaulsFormationParam			*paramptr = NULL;
	struct PaulsFormation						*prevpf = NULL;
	/* struct PaulsFormation						*pf = NULL; */
	UBYTE														*formationname = NULL;
	struct DiskFormationDude				*dfd = NULL;
	UWORD														 i = 0, dudenum = 0;
	ULONG														 numofformations = 0, numofdudes = 0;
	UWORD														 err = LAMERR_ALLOK;
	UBYTE														 prognamebuffer[PROGFULLNAMESIZE];
	UBYTE														 formationnamebuffer[FORMATIONFULLNAMESIZE];

	if( !IsListEmpty(&formationlist) )
		prevpf = &((struct FormationNode *)formationlist.lh_TailPred)->fn_PaulsFormation;

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
				if( cn->cn_TypeID == ID_DFRM )
				{
					/* Found a DFRM chunk */
					D(bug("--- Cook DFRM's ---\n"));

					p = cn->cn_Data;

					numofformations = *(ULONG *)p;  p += 4;

					D2(bug(" NumOfFormations %lu\n",numofformations));

					for( i=0; (i < numofformations) && (err < LAME_FAILAT); i++ )
					{
						formationname = p; p += FORMATIONFILENAMESIZE;

						numofdudes = *(ULONG *)p; p += 4;

						if( fnode = MYALLOCVEC( sizeof(struct FormationNode), MEMF_ANY|MEMF_CLEAR,"FormationNode") )
						{
							fnode->fn_Node.ln_Name = formationname;
							AddTail( &formationlist, &fnode->fn_Node );
							if(prevpf)	prevpf->pf_Next = &fnode->fn_PaulsFormation;
							D2(UnpackASCII(fnode->fn_Node.ln_Name, formationnamebuffer, FORMATIONFULLNAMESIZE-1));
							D2(bug("FormationName: %s, Address: $%lx\n", formationnamebuffer, &fnode->fn_PaulsFormation));
							fnode->fn_PaulsFormation.pf_Next = NULL;
							fnode->fn_PaulsFormation.pf_Previous = prevpf;
							fnode->fn_PaulsFormation.pf_Dudes = numofdudes;
							fnode->fn_PaulsFormation.pf_Flags = 0;
	
							/* Allocate Dude Parameter list */
							if( paramptr = MYALLOCVEC( sizeof(struct PaulsFormationParam)*numofdudes, MEMF_ANY|MEMF_CLEAR,"PaulsFormationParam") )
							{
								fnode->fn_Params = paramptr;
								fnode->fn_PaulsFormation.pf_Params = paramptr;
								D2(bug(" Params: %lx\n", paramptr));
								for(dudenum = 0; (dudenum < numofdudes) && (err < LAME_FAILAT); dudenum++)
								{
									D2(bug(" Formation: %lu ", i));
									D2(bug(" Dude: %lu ", dudenum));
									dfd = (struct DiskFormationDude *)p; p += sizeof(struct DiskFormationDude);
									paramptr->pp_Delay = dfd->dfd_Delay;
									paramptr->pp_XPos = dfd->dfd_XPos<<5;
									paramptr->pp_YPos = dfd->dfd_YPos<<5;
									D2(bug(" Delay: %lu", paramptr->pp_Delay));
									D2(bug(" XPos: %ld", paramptr->pp_XPos));
									D2(bug(" YPos: %ld\n", paramptr->pp_YPos));
									if(!NullName(dfd->dfd_Program))
									{
										if( pn = (struct ProgNode *)FindCompressedName(&proglist, dfd->dfd_Program, NULL) )
											paramptr->pp_Prog = (ULONG *)pn->pn_Prog;
										else
										{
											err = LAMERR_PROGNOTFOUND;
											UnpackASCII(dfd->dfd_Program, prognamebuffer, PROGFULLNAMESIZE-1);
											UnpackASCII(formationname, formationnamebuffer, FORMATIONFULLNAMESIZE-1);
											sprintf(errorstring, "Error Code: %ld\nAction List: %s\nDescription: Action List Not Found, for Formation '%s'\n", err, prognamebuffer, formationnamebuffer);
											DisplayError(errorstring);
										}
									}
									else
										paramptr->pp_Prog = NULL;

									paramptr++;
								}
							}
							else
							{
								err = LAMERR_MEM;
								sprintf(errorstring, "Error Code: %ld\nDescription: Not Enough Memory\n", err);
								DisplayError(errorstring);
							}
						}
						else
						{
							err = LAMERR_MEM;
							sprintf(errorstring, "Error Code: %ld\nDescription: Not Enough Memory\n", err);
							DisplayError(errorstring);
							return(err);
						}
						prevpf = &fnode->fn_PaulsFormation;
					}
					cn->cn_Cooked = TRUE;
				}
			}
		}
	}
	return(err);
}


/****************   FreeCookedDudeFormations()   ************************/

void 	FreeCookedDudeFormations( void )
{
	struct FormationNode *fnode;
	while( !IsListEmpty( &formationlist ) )
	{
		fnode = (struct FormationNode *)formationlist.lh_Head;
		if(fnode->fn_Params)
			MYFREEVEC(fnode->fn_Params);
		Remove( &fnode->fn_Node );
		MYFREEVEC( fnode );
	}
}

/****************   CheckUniqueFormationNames()   ************/
//
//  Goes through all the formations and makes sure
// that all the names are unique.

void CheckUniqueFormationNames(void)
{
	struct FormationNode *fn;
	UBYTE	namebuffer[FORMATIONFULLNAMESIZE];
	UWORD err = LAMERR_ALLOK;

	for(fn = (struct FormationNode *)formationlist.lh_Head;
			fn->fn_Node.ln_Succ && fn->fn_Node.ln_Succ->ln_Succ;
			fn = (struct FormationNode *)fn->fn_Node.ln_Succ)
	{
		if(!NullName(fn->fn_Node.ln_Name))
		{
			if(FindCompressedName(&formationlist, fn->fn_Node.ln_Name, fn->fn_Node.ln_Succ))
			{
				err = LAMERR_NAME_NOT_UNIQUE;
				UnpackASCII(fn->fn_Node.ln_Name, namebuffer, FORMATIONFULLNAMESIZE-1);
				sprintf(errorstring, "Warning %ld: Formation Name '%s' Not Unique\n", err, namebuffer);
				DisplayWarning(errorstring);
			}
		}
		else
		{
			err = LAMERR_EMPTY_NAME;
			sprintf(errorstring, "Warning %ld: Formation Has No Name\n", err);
			DisplayWarning(errorstring);
		}
	}
}

/*******************  CookTimedWaves()   ****************************/

UWORD CookTimedWaves( struct LameReq *lr )
{
	struct FileNode									*fn = NULL;
	struct ChunkNode								*cn = NULL;
	struct DiskTimedWave						*dtw = NULL;
	struct TimedWaveNode						*twn = NULL;
	struct FormationNode						*fnode = NULL;
	struct ProgNode									*pn = NULL;
	struct PaulsTimedWave						*ptw = NULL;
	UBYTE														*p = NULL;
	UWORD														 i = 0;
	UWORD														 err = LAMERR_ALLOK;
	UWORD														 numofwaves = 0;
	UBYTE														 formationnamebuffer[PROGFULLNAMESIZE];

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
				if( cn->cn_TypeID == ID_TWAV )
				{
					/* Found a TWAV chunk */
					D(bug("--- Cook TWAV's ---\n"));

					p = cn->cn_Data;

					numofwaves = *(ULONG *)p;  p += 4;

					D(bug(" NumOfWaves %lu\n", numofwaves));

					if( twn = MYALLOCVEC( sizeof( struct TimedWaveNode ), MEMF_ANY|MEMF_CLEAR,"TimedWaveNode") )
					{
						if( twn->twn_PaulsTimedWave = MYALLOCVEC( sizeof(struct PaulsTimedWave)*numofwaves, MEMF_ANY|MEMF_CLEAR,"PaulsTimedWave") )
						{
							AddTail( &timedwavelist, &twn->twn_Node );
							ptw = twn->twn_PaulsTimedWave;
							dtw = (struct DiskTimedWave *)p;
							twn->twn_NumberOfTimedWaves = numofwaves;
							for( i=0; (i < numofwaves) && (err < LAME_FAILAT); i++ )
							{
								D(bug(" Wave Num: %lu ",i));
								twn->twn_Node.ln_Name = NULL;
								if(!NullName(dtw->dtw_PackedEvent ))
								{
									if( dtw->dtw_Type == TWTYPE_FORMATION )
									{
										if(fnode = (struct FormationNode *)FindCompressedName(&formationlist, dtw->dtw_PackedEvent, NULL) )
											ptw->ptw_Event = &fnode->fn_PaulsFormation;
										else
										{
											err = LAMERR_FORMATION_NOT_FOUND;
											UnpackASCII(dtw->dtw_PackedEvent, formationnamebuffer, FORMATIONFULLNAMESIZE-1);
											sprintf(errorstring, "Error Code: %ld\nFormation: %s\nDescription: Formation Not Found for Timed Event %ld\n", err, formationnamebuffer, i);
											DisplayError(errorstring);
										}
									}
									else	// TWTYPE_INITDUDE or TWTYPE_ACTIONLIST
									{
										if( pn = (struct ProgNode *)FindCompressedName(&proglist, dtw->dtw_PackedEvent, NULL) )
											ptw->ptw_Event = pn->pn_Prog;
										else
										{
											err = LAMERR_PROGNOTFOUND;
											UnpackASCII(dtw->dtw_PackedEvent, formationnamebuffer, PROGFULLNAMESIZE-1);
											sprintf(errorstring, "Error Code: %ld\nActionList: %s\nDescription: ActionList Not Found for Timed Event %ld\n", err, formationnamebuffer, i);
											DisplayError(errorstring);
										}
									}
								}
								else
									ptw->ptw_Event = NULL;
		
								ptw->ptw_Time = (UWORD)dtw->dtw_Time;
								ptw->ptw_XPos = dtw->dtw_X;
								ptw->ptw_YPos = dtw->dtw_Y;
								ptw->ptw_Type = (UBYTE)dtw->dtw_Type;
								ptw->ptw_Flags = (UBYTE)dtw->dtw_Flags;

								/* add the FormationNode to the formationlist */
								dtw++;
								ptw++;
							}
						}
						else
						{
							MYFREEVEC(&twn->twn_Node);
							err = LAMERR_MEM;
							sprintf(errorstring, "Error Code: %ld\nDescription: Not Enough Memory\n", err);
							DisplayError(errorstring);
						}
					}
					else
					{
						err = LAMERR_MEM;
						sprintf(errorstring, "Error Code: %ld\nDescription: Not Enough Memory\n", err);
						DisplayError(errorstring);
					}
					cn->cn_Cooked = TRUE;
				}
			}
		}
	}
	return(err);
}


/****************   FreeCookedTimedWaves()   ************************/

void 	FreeCookedTimedWaves( void )
{
	struct TimedWaveNode *twn;
	while( !IsListEmpty( &timedwavelist ) )
	{
		twn = (struct TimedWaveNode *)timedwavelist.lh_Head;
		if(twn->twn_PaulsTimedWave)
			MYFREEVEC(twn->twn_PaulsTimedWave);
		Remove( &twn->twn_Node );
		MYFREEVEC( twn );
	}
}

/*******************  CookScrollWaves()   ****************************/

UWORD CookScrollWaves( struct LameReq *lr )
{
	struct FileNode									*fn = NULL;
	struct ChunkNode								*cn = NULL;
	struct DFSWAVHeader							*dswh = NULL;
	struct DFScrollWave							*dsw = NULL,*dswbase = NULL;
	struct FormationNode						*fnode = NULL;
	struct ProgNode									*pnode = NULL;
	struct PaulsScrollWave					*psw = NULL, *prevpsw = NULL, *nextpsw=NULL, *rightsentinal, *leftsentinal;
	struct ScrollWaveNode						*swn = NULL;
	UBYTE														*p = NULL;
	UWORD														 i = 0;
	UWORD														 err = LAMERR_ALLOK;
	UBYTE														 namebuffer[PROGFULLNAMESIZE];

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
				if( cn->cn_TypeID == ID_SWAV )
				{
					/* Found an SWAV chunk */
					D(bug("--- Cook SWAV's ---\n"));

					p = cn->cn_Data;

					dswh = (struct DFSWAVHeader *)p;
					p += sizeof(struct DFSWAVHeader);

					D(bug(" NumOfScrollWaves %lu\n", dswh->dswh_NumOfWaves));

					if( swn = MYALLOCVEC( sizeof( struct ScrollWaveNode ), MEMF_ANY|MEMF_CLEAR,"ScrollWaveNode") )
					{
						/* Have to allocate a couple of extra ScrollWave structures for the right & left
							sentinal nodes. The RightSentinal is the very first node and the LeftSentinal
							is the very last one. */
						if( swn->swn_PaulsScrollWave = MYALLOCVEC( sizeof(struct PaulsScrollWave)*(dswh->dswh_NumOfWaves+2), MEMF_ANY|MEMF_CLEAR,"PaulsScrollWave") )
						{
							AddTail( &scrollwavelist, &swn->swn_Node );
							D2(bug("swn->swn_PaulsScrollWave: $%lx\n",swn->swn_PaulsScrollWave));
							psw = swn->swn_PaulsScrollWave;
							dswbase = (struct DFScrollWave *)p;
							dsw = dswbase;
							swn->swn_NumberOfScrollWaves = dswh->dswh_NumOfWaves;
							swn->swn_Node.ln_Name = NULL;
							/* Fill In the PrevRight Sentinal Node */
							rightsentinal = psw;
							psw->psw_Type = PTWF_SCROLL;
							psw->psw_ActionFormation = NULL;
							psw->psw_XPos = 0;
							psw->psw_YPos = 0;
							psw->psw_Flags = 0;
							psw->psw_AppearCount = 0;
							psw->psw_LeftTrigger = 0;
							psw->psw_RightTrigger = 0x80000000;
							psw++;
							for( i=0; (i < dswh->dswh_NumOfWaves) && (err < LAME_FAILAT); i++ )
							{
								D2(bug(" Wave Num: %lu ",i));

								psw->psw_Type = PTWF_SCROLL;
								if(!NullName(dsw->dsw_Name))
								{
									if(dsw->dsw_Flags & 1)
									{
										/* Formation */
										if(fnode = (struct FormationNode *)FindCompressedName(&formationlist, dsw->dsw_Name, NULL) )
											psw->psw_ActionFormation = &fnode->fn_PaulsFormation;
										else
										{
											err = LAMERR_FORMATION_NOT_FOUND;
											UnpackASCII(dsw->dsw_Name, namebuffer, FORMATIONFULLNAMESIZE-1);
											sprintf(errorstring, "Error Code: %ld\nFormation: %s\nDescription: Formation Not Found for\nScroll Triggered Attack Wave #%ld\n", err, namebuffer, i);
											DisplayError(errorstring);
										}
									}
									else
									{
										/* Action List */
										if(pnode = (struct ProgNode *)FindCompressedName(&proglist, dsw->dsw_Name, NULL) )
											psw->psw_ActionFormation = pnode->pn_Prog;
										else
										{
											err = LAMERR_PROGNOTFOUND;
											UnpackASCII(dsw->dsw_Name, namebuffer, PROGFULLNAMESIZE-1);
											sprintf(errorstring, "Error Code: %ld\nAction List: %s\nDescription: Action List Not Found for\nScroll Triggered Attack Wave #%ld\n", err, namebuffer, i);
											DisplayError(errorstring);
										}
									}
								}
								else
								{
									psw->psw_ActionFormation = NULL;
									sprintf(errorstring, "Warning - No Action List or Formation Given\nFor Scroll Triggered Attack Wave #%ld\n", i);
									DisplayError(errorstring);
								}
								psw->psw_XPos = dsw->dsw_XPos<<5;
								psw->psw_YPos = dsw->dsw_YPos<<5;
								psw->psw_Flags = dsw->dsw_Flags;
								psw->psw_AppearCount = dsw->dsw_AppearCount;
								psw->psw_LeftTrigger = (dsw->dsw_LfTrigger == ~0) ? ~0 : dsw->dsw_LfTrigger<<5;
								psw->psw_RightTrigger = (dsw->dsw_RtTrigger == ~0) ? ~0 : dsw->dsw_RtTrigger<<5;
								dsw++;
								psw++;
							}
							/* Fill In the NextLeft Sentinal Node */
							leftsentinal = psw;
							psw->psw_Type = PTWF_SCROLL;
							psw->psw_ActionFormation = NULL;
							psw->psw_XPos = 0;
							psw->psw_YPos = 0;
							psw->psw_Flags = 0;
							psw->psw_AppearCount = 0;
							psw->psw_LeftTrigger = 0x7FFFFFFF;
							psw->psw_RightTrigger = 0;
							psw++;

							/* Do Left Links */
							D2(bug("- Left Links -\n"));
							if(dswh->dswh_LfHead != ~0)
							{
								prevpsw = NULL;
								/* Add one to LfHead to Skip the Right Sentinal */
								psw = &swn->swn_PaulsScrollWave[dswh->dswh_LfHead+1];
								firstleftswav = psw;
								dsw = &dswbase[dswh->dswh_LfHead];
								while(dsw)
								{
									D2(bug("psw->psw_XPos: $%lx, psw->psw_YPos: $%lx\n",
										psw->psw_XPos, psw->psw_YPos));
									nextpsw = (dsw->dsw_LfSucc != ~0) ? &swn->swn_PaulsScrollWave[dsw->dsw_LfSucc+1] : leftsentinal;
									psw->psw_NextLeft = nextpsw;
									psw->psw_PrevLeft = prevpsw;
									prevpsw = psw;
									psw = nextpsw;
									dsw = (dsw->dsw_LfSucc != ~0) ? &dswbase[dsw->dsw_LfSucc] : NULL;
								}
								leftsentinal->psw_PrevLeft = prevpsw;
							}
							else
								firstleftswav = NULL;

							/* Do Right Links */
							D2(bug("- Right Links -\n"));
							if(dswh->dswh_RtHead != ~0)
							{
								prevpsw = rightsentinal;
								/* Add one to RtHead to Skip the Right Sentinal */
								psw = &swn->swn_PaulsScrollWave[dswh->dswh_RtHead+1];
								rightsentinal->psw_NextRight = psw;
								firstrightswav = psw;
								dsw = &dswbase[dswh->dswh_RtHead];
								while(dsw)
								{
									D2(bug("psw->psw_XPos: $%lx, psw->psw_YPos: $%lx\n",
										psw->psw_XPos, psw->psw_YPos));
									nextpsw = (dsw->dsw_RtSucc != ~0) ? &swn->swn_PaulsScrollWave[dsw->dsw_RtSucc+1] : NULL;
									psw->psw_NextRight = nextpsw;
									psw->psw_PrevRight = prevpsw;
									prevpsw = psw;
									psw = nextpsw;
									dsw = (dsw->dsw_RtSucc != ~0) ? &dswbase[dsw->dsw_RtSucc] : NULL;
								}
							}
							else
								firstrightswav = NULL;
						}
						else
						{
							MYFREEVEC(&swn->swn_Node);
							err = LAMERR_MEM;
							sprintf(errorstring, "Error Code: %ld\nDescription: Not Enough Memory\n", err);
							DisplayError(errorstring);
						}
					}
					else
					{
						err = LAMERR_MEM;
						sprintf(errorstring, "Error Code: %ld\nDescription: Not Enough Memory\n", err);
						DisplayError(errorstring);
					}
					cn->cn_Cooked = TRUE;
				}
			}
		}
	}
	return(err);
}


/****************   FreeCookedScrollWaves()   ************************/

void FreeCookedScrollWaves( void )
{
	struct ScrollWaveNode *swn;
	while( !IsListEmpty( &scrollwavelist ) )
	{
		swn = (struct ScrollWaveNode *)scrollwavelist.lh_Head;
		if(swn->swn_PaulsScrollWave)
			MYFREEVEC(swn->swn_PaulsScrollWave);
		Remove( &swn->swn_Node );
		MYFREEVEC( swn );
	}
}
