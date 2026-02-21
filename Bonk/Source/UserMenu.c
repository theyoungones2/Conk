/****************************************************************************/
//
// BONK: UserMenu.c
//
/****************************************************************************/


#include <stdio.h>
#include <string.h>
//#include <dos/dos.h>
#include <dos/dostags.h>
#include <exec/exec.h>
#include <intuition/intuition.h>
//#include <intuition/gadgetclass.h>
//#include <graphics/gfxbase.h>
//#include <rexx/rxslib.h>
#include <libraries/gadtools.h>
//#include <libraries/diskfont.h>
//#include <libraries/asl.h>
//#include <libraries/iffparse.h>

#include <proto/exec.h>
#include <proto/dos.h>
#include <proto/intuition.h>
#include <proto/graphics.h>
//#include <proto/diskfont.h>
//#include <proto/asl.h>
#include <clib/alib_protos.h>
#include <proto/gadtools.h>
//#include <proto/iffparse.h>

#include <global.h>
#include <ExternVars.h>


/*
#include <exec/exec.h>
#include <intuition/intuition.h>
#include <intuition/intuitionbase.h>
#include <dos/dostags.h>
#include <libraries/gadtools.h>

#include <stdio.h>
#include <strings.h>

#include <clib/alib_protos.h>
#include <clib/exec_protos.h>
#include <clib/graphics_protos.h>
#include <clib/intuition_protos.h>
#include <clib/dos_protos.h>
#include <clib/gadtools_protos.h>
*/

/***************  LinkInUserMenu *************/
//
//

void LinkInUserMenu(struct NewMenu *nm)
{
	int count;

//	strcpy(glob.cg_UserMenu[0].um_Name,"NewShell");
//	strcpy(glob.cg_UserMenu[0].um_Command,"NewShell");
//	strcpy(glob.cg_UserMenu[1].um_Name,"WBTris");
//	strcpy(glob.cg_UserMenu[1].um_Command,"dh1:Games/wbtris");
//	strcpy(glob.cg_UserMenu[2].um_Name,"Conkit");
//	strcpy(glob.cg_UserMenu[2].um_Command,"ck:conkit");

	if(glob.cg_UserMenu[0].um_Name[0])
	{
		nm->nm_Type = NM_TITLE;
		nm->nm_Label = "User";
		nm->nm_CommKey = 0;
		nm->nm_Flags = 0;
		nm->nm_MutualExclude = 0;
		nm->nm_UserData = 0;
		nm++;

		for(count = 0; glob.cg_UserMenu[count].um_Name[0]; count++)
		{
			nm->nm_Type = NM_ITEM;
			nm->nm_Label = glob.cg_UserMenu[count].um_Name;
			nm->nm_CommKey = 0;
			nm->nm_Flags = 0;
			nm->nm_MutualExclude  = 0;
			nm->nm_UserData = 0;
			nm++;
		}
	}
	nm->nm_Type = NM_END;
	nm->nm_Label = NULL;
	nm->nm_CommKey = 0;
	nm->nm_Flags = 0;
	nm->nm_MutualExclude = 0;
	nm->nm_UserData = 0;
}

/*************  DoUserMenu ***********/
//
//	Handle UserMenu selections.
//

void DoUserMenu(struct SuperWindow *sw,UWORD id)
{
	UWORD itemnum;

	itemnum = ITEMNUM(id);

	DoDOS(glob.cg_UserMenu[itemnum].um_Command);
}