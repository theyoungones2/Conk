#include <stdio.h>
//#include <stdarg.h>
#include <string.h>
//#include <dos/dos.h>
//#include <dos/dostags.h>
//#include <exec/exec.h>
//#include <intuition/intuition.h>
//#include <graphics/gfxbase.h>
//#include <rexx/rxslib.h>
#include <libraries/gadtools.h>
//#include <libraries/diskfont.h>
//#include <libraries/asl.h>

#include <proto/exec.h>
#include <proto/dos.h>
#include <proto/intuition.h>
//#include <proto/graphics.h>
//#include <proto/diskfont.h>
#include <proto/asl.h>
//#include <clib/alib_protos.h>
//#include <proto/gadtools.h>

#include <global.h>
#include <ExternVars.h>

#define MN_MODE							4

/*
		{ NM_TITLE,	"Mode",									0,	0,	0,	0,},
		{  NM_ITEM, "Matte",								0,	CHECKIT|MENUTOGGLE,	0,	0,},
		{  NM_ITEM, "Colour",								0,	CHECKIT|MENUTOGGLE,	0,	0,},
		{  NM_ITEM, "Replace",							0,	CHECKIT|MENUTOGGLE,	0,	0,},
		{  NM_ITEM, "Shade",								0,	CHECKIT|MENUTOGGLE,	0,	0,},
		{  NM_ITEM, "Smooth",								0,	CHECKIT|MENUTOGGLE,	0,	0,},
*/

void SetupModeMenu(struct SuperWindow *sw, struct NewMenu *nm)
{
	/* suss out the state of the clicky checkmark things */

	nm[MNMDE_MATTE_IDX].nm_Flags = CHECKIT|MENUTOGGLE;
	nm[MNMDE_COLOUR_IDX].nm_Flags = CHECKIT|MENUTOGGLE;
	nm[MNMDE_REPLACE_IDX].nm_Flags = CHECKIT|MENUTOGGLE;
	nm[MNMDE_SHADE_IDX].nm_Flags = CHECKIT|MENUTOGGLE;
	nm[MNMDE_SMOOTH_IDX].nm_Flags = CHECKIT|MENUTOGGLE;
	nm[MNMDE_CYCLE_IDX].nm_Flags = CHECKIT|MENUTOGGLE;
	nm[MNMDE_DUMMY1_IDX].nm_Flags = CHECKIT|MENUTOGGLE;
	nm[MNMDE_DUMMY2_IDX].nm_Flags = CHECKIT|MENUTOGGLE;
	nm[MNMDE_DUMMY3_IDX].nm_Flags = CHECKIT|MENUTOGGLE;
	switch(drawmode)
	{
		case DRMD_MATTE:
			nm[MNMDE_MATTE_IDX].nm_Flags = CHECKIT|MENUTOGGLE|CHECKED;
			break;
		case DRMD_COLOUR:
			nm[MNMDE_COLOUR_IDX].nm_Flags = CHECKIT|MENUTOGGLE|CHECKED;
			break;
		case DRMD_REPLACE:
			nm[MNMDE_REPLACE_IDX].nm_Flags = CHECKIT|MENUTOGGLE|CHECKED;
			break;
		case DRMD_SHADE:
			nm[MNMDE_SHADE_IDX].nm_Flags = CHECKIT|MENUTOGGLE|CHECKED;
			break;
		case DRMD_SMOOTH:
			nm[MNMDE_SMOOTH_IDX].nm_Flags = CHECKIT|MENUTOGGLE|CHECKED;
			break;
		case DRMD_CYCLE:
			nm[MNMDE_CYCLE_IDX].nm_Flags = CHECKIT|MENUTOGGLE|CHECKED;
			break;
	}
}

void DoModeMenu(struct SuperWindow *sw,UWORD id)
{
	UWORD itemnum,subnum;
	struct MenuItem *mi;

	itemnum = ITEMNUM(id);
	subnum = SUBNUM(id);

	mi = ItemAddress( sw->sw_MenuStrip, id );

	WaitForSubTaskDone(mainmsgport);

	switch(itemnum)
	{
		case MNMDE_MATTE:
			NewDrawMode(DRMD_MATTE);
			break;
		case MNMDE_COLOUR:
			NewDrawMode(DRMD_COLOUR);
			break;
		case MNMDE_REPLACE:
			NewDrawMode(DRMD_REPLACE);
			break;
		case MNMDE_SHADE:
			NewDrawMode(DRMD_SHADE);
			break;
		case MNMDE_SMOOTH:
			NewDrawMode(DRMD_SMOOTH);
			break;
		case MNMDE_CYCLE:
			NewDrawMode(DRMD_CYCLE);
			break;
	}
}

void NewDrawMode(UWORD drmd)
{
	drawmode = drmd;
	//RefreshAllMenus();
	RefreshModeMenu();
	SetSuperWindowsScreenTitle();
}

void RefreshModeMenu(void)
{
	struct SuperWindow *sw;
	struct Menu *mn;
	struct MenuItem *mi;
	UWORD count;

	for(sw = (struct SuperWindow *)superwindows.lh_Head;
			sw->sw_Node.ln_Succ;
			sw = (struct SuperWindow *)sw->sw_Node.ln_Succ)
	{
		count = 0;
		if(mn = sw->sw_Menus[MN_MODE])
		{
			for(mi = mn->FirstItem; mi; mi = mi->NextItem)
			{
				if(count == drawmode)
					mi->Flags |= CHECKED;
				else
					mi->Flags &= ~CHECKED;
				count++;
			}
		}
	}
}
