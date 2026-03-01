/****************************************************************************/
//
// BONK: MapMenu.c
//
/****************************************************************************/


#include <stdio.h>
#include <string.h>
//#include <dos/dos.h>
//#include <dos/dostags.h>
#include <exec/exec.h>
#include <intuition/intuition.h>
//#include <intuition/gadgetclass.h>
#include <graphics/gfxbase.h>
//#include <rexx/rxslib.h>
#include <libraries/gadtools.h>
//#include <libraries/diskfont.h>
//#include <libraries/asl.h>
//#include <libraries/iffparse.h>

#include <proto/exec.h>
//#include <proto/dos.h>
#include <proto/intuition.h>
#include <proto/graphics.h>
//#include <proto/diskfont.h>
//#include <proto/asl.h>
#include <clib/alib_protos.h>
#include <proto/gadtools.h>
//#include <proto/iffparse.h>

#include <global.h>
#include <ExternVars.h>


/* local protos */

/*************  DoMapMenu ***********/
//
//	Sort out everything to do with the Map Menu.
//
// RETURN:	0 = Carry on - all ok.
//					1 = Stop processing menus in this window.
//					2 = Refresh SuperWindow when done (hide then show).
//					3 = Refresh SuperWindow _and_ stop processing menus.

UWORD DoMapMenu(struct SuperWindow *sw,UWORD id)
{
	UWORD itemnum,subnum;
	struct MenuItem *mi;
	struct ProjectWindow *pw, *otherpw;
	// UBYTE mapnamebuf[ MAPNAMESIZE ];

	if( sw->sw_Type == SWTY_PROJECT )
	{
		pw = (struct ProjectWindow *)sw;

		itemnum = ITEMNUM(id);
		subnum = SUBNUM(id);

		mi = ItemAddress( sw->sw_MenuStrip, id );

		switch(itemnum)
		{
			case MNMAP_FIXBG:
				if( mi->Flags & CHECKED )
					LockBackground( pw->pw_Project );
				else
					UnlockBackground( pw->pw_Project );
//				return(2);
				/* refresh all the menus for this project... */
				for( otherpw = pw->pw_Project->pj_ProjectWindows;
					otherpw;
					otherpw = otherpw->pw_NextProjWindow )
				{
					if( otherpw->pw_sw.sw_Window )
						( *otherpw->pw_sw.sw_RefreshMenus)( &otherpw->pw_sw );
				}
				return(0);
				break;
			case MNMAP_RESIZE:
				GUIResizeProject( pw->pw_Project );
				return(0);
				break;
			case MNMAP_RENAME:
				RenameMap( pw->pw_Project );
				return( 0 );
				break;
			case MNMAP_SETBM:
				SetBookMark( pw,subnum );
				return(0);
				break;
			case MNMAP_GOTOBM:
				GotoBookMark( pw,subnum );
				return(0);
				break;
		}
	}
	return(0);
}


void SetupMapMenu(struct ProjectWindow *pw, struct NewMenu *nm)
{
	nm[MNMAP_FIXBG_IDX].nm_Flags = pw->pw_Project->pj_BackLockFlag ?
		CHECKIT|MENUTOGGLE | CHECKED : CHECKIT|MENUTOGGLE;
}

