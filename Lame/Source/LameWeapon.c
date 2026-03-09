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


/*******************  PreCookWeapons()   ****************************/

UWORD PreCookWeapons( struct LameReq *lr )
{
	struct FileNode *fn;
	struct ChunkNode *cn;
	UWORD err = LAMERR_ALLOK;
	UBYTE *p;
	UWORD	i;
	struct PaulsWeapon *pw;
	struct WeaponDiskDef *wdd;
	struct WeaponNode *wn;
	ULONG numofweapons;

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
				if( cn->cn_TypeID == ID_WEAP )
				{
					/* Found a WEAP chunk */
					D(bug("--- Pre Cook WEAP's ---\n"));

					p = (UBYTE *)cn->cn_Data;

					numofweapons = (UWORD)(*(ULONG *)p); p += 4;
					D(bug(" NumOfWeapons: %lu\n", numofweapons));

					wdd = (struct WeaponDiskDef *)p;

					for( i=0; (i < numofweapons) && (err < LAME_FAILAT); i++ )
					{
						if(wn = MYALLOCVEC( sizeof(struct WeaponNode), MEMF_ANY|MEMF_CLEAR,"Weapon Node") )
						{
							wn->wn_Node.ln_Name = wdd->wdd_Name;
							AddTail(&weaponlist, &wn->wn_Node);
							pw = &wn->wn_PaulsWeapon;
							pw->wp_PowerUp = wdd->wdd_PowerUp;
							pw->wp_PowerDown = wdd->wdd_PowerDown;
							pw->wp_FireActionList = wdd->wdd_FireProgram;
							pw->wp_Bullets = 0;
							pw->wp_MaxBullets = wdd->wdd_MaxBullets;
							pw->wp_BulletDelay = 0;
							pw->wp_MaxBulletDelay = wdd->wdd_FireDelay;
							pw->wp_Flags = wdd->wdd_Flags;
							pw->wp_Reserved = 0;
							wdd++;
						}
						else
						{
							err = LAMERR_MEM;
							sprintf(errorstring, "Error Code: %ld\nDescription: Not Enough Memory\n", err);
							DisplayError(errorstring);
							return( err );
						}
					}
				}
			}
		}
	}
	return(err);
}

UWORD PostCookWeapons( struct LameReq *lr )
{
	struct FileNode *fn;
	struct ChunkNode *cn;
	struct PaulsWeapon *pw;
	UBYTE	namebuffer1[WEAPFULLNAMESIZE];
	UBYTE	namebuffer2[WEAPFULLNAMESIZE];
	struct ProgNode *pn;
	UWORD err = LAMERR_ALLOK;
	struct WeaponNode *wn,*wn2;

	D(bug("--- Post Cook WEAP's ---\n"));	
	for(wn = (struct WeaponNode *)weaponlist.lh_Head;
			wn->wn_Node.ln_Succ;
			wn = (struct WeaponNode *)wn->wn_Node.ln_Succ)
	{
		pw = &wn->wn_PaulsWeapon;
		if(!NullName((char *)pw->wp_FireActionList))
		{
			if( pn = (struct ProgNode *)FindCompressedName(&proglist, pw->wp_FireActionList, NULL) )
				pw->wp_FireActionList = pn->pn_Prog;
			else
			{
				err = LAMERR_PROGNOTFOUND;
				UnpackASCII(pw->wp_FireActionList, namebuffer1, PROGFULLNAMESIZE-1);
				UnpackASCII(wn->wn_Node.ln_Name, namebuffer2, WEAPFULLNAMESIZE-1);
				sprintf(errorstring, "Error Code: %ld\nAction List: %s\nDescription: Action List Not Found\nFor Weapon '%s'\n", err, namebuffer1, namebuffer2);
				DisplayError(errorstring);
			}
		}
		else
			pw->wp_FireActionList = NULL;

		if(!NullName((char *)pw->wp_PowerUp))
		{
			if( wn2 = (struct WeaponNode *)FindCompressedName(&weaponlist, pw->wp_PowerUp, NULL) )
				pw->wp_PowerUp = &wn2->wn_PaulsWeapon;
			else
			{
				err = LAMERR_WEAPON_NOT_FOUND;
				UnpackASCII(pw->wp_PowerUp, namebuffer1, WEAPFULLNAMESIZE-1);
				UnpackASCII(wn->wn_Node.ln_Name, namebuffer2, WEAPFULLNAMESIZE-1);
				sprintf(errorstring, "Error Code: %ld\nDescription: Power Up '%s' Not Found\nFor Weapon '%s'\n", err, namebuffer1, namebuffer2);
				DisplayError(errorstring);
			}
		}
		else
			pw->wp_PowerUp = NULL;

		if(!NullName((char *)pw->wp_PowerDown))
		{
			if( wn2 = (struct WeaponNode *)FindCompressedName(&weaponlist, pw->wp_PowerDown, NULL) )
				pw->wp_PowerDown = &wn2->wn_PaulsWeapon;
			else
			{
				err = LAMERR_WEAPON_NOT_FOUND;
				UnpackASCII(pw->wp_PowerDown, namebuffer1, WEAPFULLNAMESIZE-1);
				UnpackASCII(wn->wn_Node.ln_Name, namebuffer2, WEAPFULLNAMESIZE-1);
				sprintf(errorstring, "Error Code: %ld\nDescription: Power Down '%s' Not Found\nFor Weapon '%s'\n", err, namebuffer1, namebuffer2);
				DisplayError(errorstring);
			}
		}
		else
			pw->wp_PowerDown = NULL;
	}

	if(err < LAME_FAILAT)
	{
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
					if( cn->cn_TypeID == ID_WEAP )
						cn->cn_Cooked = TRUE;
				}
			}
		}
	}
	return(err);
}

/****************   FreeCookedWeapons()   ************************/

void 	FreeCookedWeapons( void )
{
	struct WeaponNode *wn;

	while(!IsListEmpty(&weaponlist))
	{
		wn = (struct WeaponNode *)weaponlist.lh_Head;
		Remove(&wn->wn_Node);
		MYFREEVEC(wn);
	}
}

/****************   CheckUniqueWeaponNames()   ************/
//
//  Goes through all the weapons and makes sure
// that all the names are unique.

void CheckUniqueWeaponNames(void)
{
	struct WeaponNode *wn;
	UBYTE	namebuffer[ANIMFULLNAMESIZE];
	UWORD err = LAMERR_ALLOK;

	for(wn = (struct WeaponNode *)weaponlist.lh_Head;
			wn->wn_Node.ln_Succ && wn->wn_Node.ln_Succ->ln_Succ;
			wn = (struct WeaponNode *)wn->wn_Node.ln_Succ)
	{
		if(!NullName(wn->wn_Node.ln_Name))
		{
			if(FindCompressedName(&weaponlist, wn->wn_Node.ln_Name, wn->wn_Node.ln_Succ))
			{
				err = LAMERR_NAME_NOT_UNIQUE;
				UnpackASCII(wn->wn_Node.ln_Name, namebuffer, WEAPFULLNAMESIZE-1);
				sprintf(errorstring, "Warning %ld: Weapon Name '%s' Not Unique\n", err, namebuffer);
				DisplayWarning(errorstring);
			}
		}
		else
		{
			err = LAMERR_EMPTY_NAME;
			sprintf(errorstring, "Warning %ld: Weapon Has No Name\n", err);
			DisplayWarning(errorstring);
		}
	}
}
