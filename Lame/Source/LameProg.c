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
#include <game.h>

/****************   CookProgs()   ************************/
//
//

UWORD CookProgs( struct LameReq *lr )
{
	struct FileNode *fn;
	struct ChunkNode *cn;
	struct ProgFileHeader *pfh;
	struct ProgNode *pn;
	UBYTE	*p;
	ULONG	numofprogs,prognum;
	UWORD err = LAMERR_ALLOK;
	UBYTE	namebuffer[PROGFULLNAMESIZE];

	D(bug("---  Cook PROG's  --- Pass 1 ---"));

	/* Pass 1 */
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
				if( cn->cn_TypeID == ID_PROG )
				{
					p = cn->cn_Data;
					numofprogs = *(ULONG *)p;
					p += 4;
					D2(bug(" NumOfProgs: %ld\n",numofprogs));
					for(prognum = 0; prognum < numofprogs; prognum++)
					{
						pfh = (struct ProgFileHeader *)p;
						p += sizeof(struct ProgFileHeader);
						if(pn = MYALLOCVEC( sizeof(struct ProgNode), MEMF_ANY|MEMF_CLEAR,"Program Node") )
						{
							UnpackASCII(pfh->pfh_Name, namebuffer, PROGFULLNAMESIZE-1);
							D2(bug("Prog: %s Memory: %ld\n", namebuffer, pfh->pfh_Memory));
							if( pn->pn_Prog = MYALLOCVEC( pfh->pfh_Memory, MEMF_ANY|MEMF_CLEAR,"ProgNode->pn_Prog") )
							{
								pn->pn_Node.ln_Name = pfh->pfh_Name;
								pn->pn_SrcData = p;
								pn->pn_ProgSize = pfh->pfh_Memory;
								AddTail(&proglist,&pn->pn_Node);
							}
							else
							{
								MYFREEVEC(pn);
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
						p += pfh->pfh_ProgFileSize;
					}
					cn->cn_Cooked = TRUE;
				}
			}
		}
	}
	if(!err)
	{
		if( !IsListEmpty(&proglist) )
		{
			D(bug(" --- Pass 2 ---\n"));
			for(pn = (struct ProgNode *)proglist.lh_Head;
				 (pn->pn_Node.ln_Succ) && (err < LAME_FAILAT);
					pn = (struct ProgNode *)pn->pn_Node.ln_Succ )
			{
				/* Unpack the Program Name. This is then passed into the Template solver */
				UnpackASCII(pn->pn_Node.ln_Name, namebuffer, PROGFULLNAMESIZE-1);
				D2(bug("Prog: %s\n", namebuffer));

				/* Set up Solve Template Registers */
				stdestptr = pn->pn_Prog;
				stsrcptr = pn->pn_SrcData;
				err = LAMERR_ALLOK;
				while(err == LAMERR_ALLOK)
					err = SolveTemplate(lr, namebuffer);
				if(err == LAMERR_ENDOFPROG) err = LAMERR_ALLOK;
			}
		}
	}
	return(err);
}


/****************   FreeCookedProgs()   ************************/

void FreeCookedProgs( void )
{
	struct ProgNode *pn;
	while( !IsListEmpty( &proglist ) )
	{
		pn = (struct ProgNode *)proglist.lh_Head;
		if(pn->pn_Prog)
			MYFREEVEC(pn->pn_Prog);
		Remove( &pn->pn_Node );
		MYFREEVEC( pn );
	}
}

/****************   SolveTemplate()   ************************/

UWORD SolveTemplate(struct LameReq *lr, char *progname)
{
	UBYTE								*temptr;
	UWORD								 operator;
	BOOL								 done = FALSE;

	struct AnimNode			*an;
	struct BobNode			*bn;
	struct SpriteNode		*sn;
	struct ILBMNode			*ilbmnode;
	struct ProgNode			*pn;
	struct PlayerNode		*plyrnode;
	struct SFXNode			*sfx;
	struct PathNode			*pathnode;
	struct WeaponNode		*wn;
	struct FileNode			*fn;
	struct DiskEOTRHeader *deotr;
	UWORD								 plyrnum, strlen;
	UWORD								 err = LAMERR_ALLOK;
	UBYTE								 namebuffer[BOBFULLNAMESIZE];

	if( operator = *(UWORD *)stsrcptr )
	{
		D2(bug(" OPERATOR: %ld '%s'\n",operator, lr->lr_OperatorNames[operator]));
		*(ULONG *)stdestptr = lr->lr_Operators[operator];
		stdestptr += 4;
		stsrcptr += 2;
		temptr = lr->lr_Templates[operator];
		while( (!done) && (err < LAME_FAILAT) )
		{
			if(*temptr != TEMARG_END)
			{
				switch(*temptr)
				{
					case TEMARG_END:
						D2(bug("  END\n"));
						break;
					case TEMARG_BYTE:
						D2(bug("  BYTE: %ld\n",*stsrcptr));
						*stdestptr++ = *stsrcptr++;
						break;
					case TEMARG_WORD:
						D2(bug("  WORD: %ld\n",*(UWORD *)stsrcptr));
						*(UWORD *)stdestptr = *(UWORD *)stsrcptr;
						stdestptr += 2;
						stsrcptr += 2;
						break;
					case TEMARG_LONG:
						D2(bug("  LONG: %ld\n",*(ULONG *)stsrcptr));
						*(ULONG *)stdestptr = *(ULONG *)stsrcptr;
						stdestptr += 4;
						stsrcptr += 4;
						break;
					case TEMARG_STRING:
						D2(bug("  STRING\n"));
						strlen = 0;
						while(*stsrcptr != 0)						/* Copy all the chars till we get a NULL */
						{
							*stdestptr++ = *stsrcptr++;
							strlen += 1;
						}
						*stdestptr++ = *stsrcptr++;			/* Copy The NULL */
						strlen += 1;
						if(strlen & 1)
							*stdestptr++ = *stsrcptr++;			/* Copy The Extra char if it's an odd length */
						break;
					case TEMARG_IMAGEREF:
						D2(bug("  IMAGEREF: "));
						if(!NullName(stsrcptr))
						{
							D2(UnpackASCII(stsrcptr, namebuffer, BOBFULLNAMESIZE-1));
							D2(bug("%s...", namebuffer));
							if( bn = (struct BobNode *)FindCompressedName(&boblist,stsrcptr,NULL) )
							{
								D2(bug("Resolved\n"));
								*(ULONG *)stdestptr = (ULONG)&bn->bn_ImageHeader;
								stdestptr += 4;
							}
							else
							{
								if( sn = (struct SpriteNode *)FindCompressedName(&spritelist,stsrcptr,NULL) )
								{
									D2(bug("Resolved\n"));
									*(ULONG *)stdestptr = (ULONG)&sn->sn_ImageHeader;
									stdestptr += 4;
								}
								else
								{
									if( ilbmnode = (struct ILBMNode *)FindCompressedName(&ilbmlist,stsrcptr,NULL) )
									{
										D2(bug("Resolved\n"));
										*(ULONG *)stdestptr = (ULONG)&ilbmnode->ilbm_PaulsPictureImage;
										stdestptr += 4;
									}
									else
									{
										D2(bug("unresolved\n"));
										err = LAMERR_IMAGENOTFOUND;
										UnpackASCII(stsrcptr, namebuffer, BOBFULLNAMESIZE-1);
										sprintf(errorstring, "Error Code: %ld\nAction: '%s'\nDescription: Image '%s' Not Found\nIn Action List '%s'\n", err, lr->lr_OperatorNames[operator], namebuffer, progname);
										DisplayError(errorstring);
									}
								}
							}
						}
						else
						{
							*(ULONG *)stdestptr = NULL;
							stdestptr += 4;
							D2(bug("NULL\n"));
						}
						stsrcptr += BOBFILENAMESIZE;
						break;
					case TEMARG_ANIMREF:
						D2(bug("  ANIMREF: "));
						if(!NullName(stsrcptr))
						{
							D2(UnpackASCII(stsrcptr, namebuffer, ANIMFULLNAMESIZE-1));
							D2(bug("%s...",namebuffer));
							if( an = (struct AnimNode *)FindCompressedName(&animlist,stsrcptr,NULL) )
							{
								D2(bug("Resolved\n"));
								*((ULONG *)stdestptr) = (ULONG)an->an_AnimHeader;
								*((ULONG *)stdestptr) += sizeof(struct AnimFileHeader);
								stdestptr += 4;
							}
							else
							{
								D2(bug("Unresolved\n"));
								err = LAMERR_ANIMNOTFOUND;
								UnpackASCII(stsrcptr, namebuffer, ANIMFULLNAMESIZE-1);
								sprintf(errorstring, "Error Code: %ld\nAction: '%s'\nDescription: Anim '%s' Not Found\nIn Action List '%s'\n", err, lr->lr_OperatorNames[operator], namebuffer, progname);
								DisplayError(errorstring);
							}
						}
						else
						{
							*(ULONG *)stdestptr = NULL;
							stdestptr += 4;
							D2(bug("NULL\n"));
						}
						stsrcptr += ANIMFILENAMESIZE;
						break;
					case TEMARG_DUDEREF:
						break;
					case TEMARG_PROGREF:
						D2(bug("  PROGREF: "));
						if(!NullName(stsrcptr))
						{
							D2(UnpackASCII(stsrcptr, namebuffer, PROGFULLNAMESIZE-1));
							D2(bug("%s\n",namebuffer));
							if( pn = (struct ProgNode *)FindCompressedName(&proglist,stsrcptr,NULL) )
							{
								*(ULONG *)stdestptr = (ULONG)pn->pn_Prog;
								stdestptr += 4;
							}
							else
							{
								err = LAMERR_PROGNOTFOUND;
								UnpackASCII(stsrcptr, namebuffer, PROGFULLNAMESIZE-1);
								sprintf(errorstring, "Error Code: %ld\nAction: '%s'\nDescription: Action List '%s' Not Found\nIn Action List '%s'\n", err, lr->lr_OperatorNames[operator], namebuffer, progname);
								DisplayError(errorstring);
							}
						}
						else
						{
							*(ULONG *)stdestptr = NULL;
							stdestptr += 4;
							D2(bug("NULL\n"));
						}
						stsrcptr += PROGFILENAMESIZE;
						break;
					case TEMARG_SFXREF:
						D2(bug("  SFXREF: "));
						if(!NullName(stsrcptr))
						{
							D2(UnpackASCII(stsrcptr, namebuffer, SFXFULLNAMESIZE-1));
							D2(bug("%s...", namebuffer));
							if( sfx = (struct SFXNode *)FindCompressedName(&sfxlist,stsrcptr,NULL) )
							{
								D2(bug("Resolved\n"));
								*(ULONG *)stdestptr = (ULONG)sfx->sfx_SpamParam;
								stdestptr += 4;
							}
							else
							{
								D2(bug("unresolved\n"));
								err = LAMERR_SFX_NOT_FOUND;
								UnpackASCII(stsrcptr, namebuffer, SFXFULLNAMESIZE-1);
								sprintf(errorstring, "Error Code: %ld\nAction: '%s'\nDescription: Sound Effect '%s' Not Found\nIn Action List '%s'\n", err, lr->lr_OperatorNames[operator], namebuffer, progname);
								DisplayError(errorstring);
							}
						}
						else
						{
							*(ULONG *)stdestptr = NULL;
							stdestptr += 4;
							D2(bug("NULL\n"));
						}
						stsrcptr += SFXFILENAMESIZE;
						break;
			 		case TEMARG_OPERATOR:
						//D2(bug("  OPERATOR: %ld  '%s'\n",*(UWORD *)stsrcptr, lr->lr_OperatorNames[*(UWORD *)stsrcptr]));
						D2(bug("  OPERATOR: %ld \n",*(UWORD *)stsrcptr));
						err = SolveTemplate(lr, progname);
						break;
					case TEMARG_PLYRREF:
						D2(bug("  PLAYRREF\n"));
						plyrnum = *(UWORD *)stsrcptr;
						stsrcptr += 2;
						if( !IsListEmpty(&playerlist) )
						{
							for(plyrnode = (struct PlayerNode *)playerlist.lh_Head;
									(plyrnum != 0) && (plyrnode->pn_Node.ln_Succ);
									plyrnode = (struct PlayerNode *)plyrnode->pn_Node.ln_Succ)
								plyrnum--;
							if(plyrnum != 0)
							{
								err = LAMERR_PLYR_NUMBER_TOO_HIGH;
								sprintf(errorstring, "Error Code: %ld\nAction List: '%s' Action: '%s'\nDescription: Player Number Too High\n", err, namebuffer, lr->lr_OperatorNames[operator]);
								DisplayError(errorstring);
							}
							else
								*(ULONG *)stdestptr = (ULONG)&plyrnode->pn_Bob;
							stdestptr += 4;
						}
						else
						{
							err = LAMERR_PLYR_NUMBER_TOO_HIGH;
							sprintf(errorstring, "Error Code: %ld\nAction List: '%s' Action: '%s'\nDescription: Player Number Too High\n", err, namebuffer, lr->lr_OperatorNames[operator]);
							DisplayError(errorstring);
						}
						break;
					case TEMARG_WEAPREF:
						D2(bug("  WEAPREF: "));
						if(!NullName(stsrcptr))
						{
							D2(UnpackASCII(stsrcptr, namebuffer, WEAPFULLNAMESIZE-1));
							D2(bug("%s...", namebuffer));
							if( wn = (struct WeaponNode *)FindCompressedName(&weaponlist,stsrcptr,NULL) )
							{
								D2(bug("Resolved\n"));
								*(ULONG *)stdestptr = (ULONG)&wn->wn_PaulsWeapon;
								stdestptr += 4;
							}
							else
							{
								D2(bug("unresolved\n"));
								err = LAMERR_WEAPON_NOT_FOUND;
								UnpackASCII(stsrcptr, namebuffer, WEAPFULLNAMESIZE-1);
								sprintf(errorstring, "Error Code: %ld\nAction: '%s'\nDescription: Weapon '%s' Not Found\nIn Action List '%s'\n", err, lr->lr_OperatorNames[operator], namebuffer, progname);
								DisplayError(errorstring);
							}
						}
						else
						{
							*(ULONG *)stdestptr = NULL;
							stdestptr += 4;
							D2(bug("NULL\n"));
						}
						stsrcptr += WEAPFILENAMESIZE;
						break;
					case TEMARG_PATHREF:
						D2(bug("  PATHREF: "));
						if(!NullName(stsrcptr))
						{
							D2(UnpackASCII(stsrcptr, namebuffer, PROGFULLNAMESIZE-1));
							D2(bug("%s\n",namebuffer));
							if( pathnode = (struct PathNode *)FindCompressedName(&pathlist,stsrcptr,NULL) )
							{
								*(ULONG *)stdestptr = (ULONG)pathnode->pn_PathHeader;
								stdestptr += 4;
							}
							else
							{
								err = LAMERR_PATH_NOT_FOUND;
								UnpackASCII(stsrcptr, namebuffer, PATHFULLNAMESIZE-1);
								sprintf(errorstring, "Error Code: %ld\nActionList: '%s' Action: '%s'\nDescription: Path '%s' Not Found\n", err, progname, lr->lr_OperatorNames[operator], namebuffer);
								DisplayError(errorstring);
							}
						}
						else
						{
							*(ULONG *)stdestptr = NULL;
							stdestptr += 4;
							D2(bug("NULL\n"));
						}
						stsrcptr += PROGFILENAMESIZE;
						break;
					case TEMARG_RAWFILEREF:
						D2(bug("  RAWFILEREF: "));
						if(!NullName(stsrcptr))
						{
							D2(UnpackASCII(stsrcptr, namebuffer, BOBFULLNAMESIZE-1));
							D2(bug("%s\n",namebuffer));
							if( fn = FindRawFile(&filelist, stsrcptr, NULL) )
							{
								*(ULONG *)stdestptr = (ULONG)fn->fn_Data;
								stdestptr += 4;
							}
							else
							{
								err = LAMERR_FILE_NOT_FOUND;
								UnpackASCII(stsrcptr, namebuffer, BOBFULLNAMESIZE-1);
								sprintf(errorstring, "Error Code: %ld\nAction: '%s'\nDescription: File '%s' Not Found\nIn Action List '%s'\n", err, lr->lr_OperatorNames[operator], namebuffer, progname);
								DisplayError(errorstring);
							}
						}
						else
						{
							*(ULONG *)stdestptr = NULL;
							stdestptr += 4;
							D2(bug("NULL\n"));
						}
						stsrcptr += BOBFILENAMESIZE;
						break;
					case TEMARG_LEVELREF:
						D2(bug("  LEVELREF: "));
						D2(UnpackASCII(stsrcptr, namebuffer, LEVELFULLNAMESIZE-1));
						D2(bug("%s\n", namebuffer));
						*(ULONG *)stdestptr = *(ULONG *)stsrcptr;
						stdestptr += 4; stsrcptr += 4;
						*(ULONG *)stdestptr = *(ULONG *)stsrcptr;
						stdestptr += 4; stsrcptr += 4;
						*(UWORD *)stdestptr = *(UWORD *)stsrcptr;
						stdestptr += 2; stsrcptr += 2;
						break;
					case TEMARG_EOTRREF:
						D2(bug("  EOTRREF: "));
						if(!NullName(stsrcptr))
						{
							D2(UnpackASCII(stsrcptr, namebuffer, EOTRFULLNAMESIZE-1));
							D2(bug("%s\n",namebuffer));
							if( deotr = (struct DiskEOTRHeader *)FindCompressedName( &eotrlist, stsrcptr, NULL ) )
							{
								*(ULONG *)stdestptr = (ULONG)deotr;
								stdestptr += 4;
							}
							else
							{
								err = LAMERR_EOTRNOTFOUND;
								UnpackASCII(stsrcptr, namebuffer, EOTRFULLNAMESIZE-1);
								sprintf(errorstring, "Error Code: %ld\nAction: '%s'\nDescription: EdgeOfTheRoad '%s' Not Found\nIn Action List '%s'\n", err, lr->lr_OperatorNames[operator], namebuffer, progname);
								DisplayError(errorstring);
							}
						}
						else
						{
							*(ULONG *)stdestptr = NULL;
							stdestptr += 4;
							D2(bug("NULL\n"));
						}
						stsrcptr += PROGFILENAMESIZE;
						break;
					default:
						err = LAMERR_BADTEMPLATE;
						sprintf(errorstring, "Error Code: %ld\nActionList: '%s' Action: '%s'\nDescription: Unrecognised Template Argument: %ld\n\n", err, progname, lr->lr_OperatorNames[operator], (ULONG)*temptr, progname);
						DisplayError(errorstring);
						break;
				}
				temptr++;
			}
			else
				done = TRUE;
		}
	}
	else
	{
		D2(bug(" Operator == 0\n"));
		err = LAMERR_ENDOFPROG;
		*(ULONG *)stdestptr = NULL;
	}
	return(err);
}

/****************   CheckUniqueProgNames()   ************/
//
//  Goes through all the progs and makes sure
// that all the names are unique.

void CheckUniqueProgNames(void)
{
	struct ProgNode *pn;
	UBYTE	namebuffer[PROGFULLNAMESIZE];
	UWORD err = LAMERR_ALLOK;

	for(pn = (struct ProgNode *)proglist.lh_Head;
			pn->pn_Node.ln_Succ && pn->pn_Node.ln_Succ->ln_Succ;
			pn = (struct ProgNode *)pn->pn_Node.ln_Succ)
	{
		if(!NullName(pn->pn_Node.ln_Name))
		{
			if(FindCompressedName(&proglist, pn->pn_Node.ln_Name, pn->pn_Node.ln_Succ))
			{
				err = LAMERR_NAME_NOT_UNIQUE;
				UnpackASCII(pn->pn_Node.ln_Name, namebuffer, PROGFULLNAMESIZE-1);
				sprintf(errorstring, "Warning %ld: Action List Name '%s' Not Unique\n", err, namebuffer);
				DisplayWarning(errorstring);
			}
		}
		else
		{
			err = LAMERR_EMPTY_NAME;
			sprintf(errorstring, "Warning %ld: Action List Has No Name\n", err);
			DisplayWarning(errorstring);
		}
	}
}
