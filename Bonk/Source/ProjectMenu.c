/****************************************************************************/
//
// BONK: ProjectMenu.c
//
/****************************************************************************/


#include <stdio.h>
#include <string.h>
#include <dos/dos.h>
//#include <dos/dostags.h>
#include <exec/exec.h>
#include <intuition/intuition.h>
//#include <graphics/gfxbase.h>
//#include <rexx/rxslib.h>
//#include <libraries/gadtools.h>
//#include <libraries/diskfont.h>
#include <libraries/asl.h>
//#include <libraries/iffparse.h>
//#include "dh1:programming/stuff/include/libraries/asl.h"

#include <proto/exec.h>
#include <proto/dos.h>
#include <proto/intuition.h>
//#include <proto/graphics.h>
//#include <proto/diskfont.h>
#include <proto/asl.h>
#include <clib/alib_protos.h>
//#include <proto/gadtools.h>
//#include <proto/iffparse.h>

#include <global.h>
#include <ExternVars.h>


static void PM_OpenProject( struct Window *win );
//void PM_Save( struct Project *proj, struct Window *win );
//void PM_SaveAs( struct Project *proj, struct Window *win );
static UWORD PM_CloseProject(struct SuperWindow *sw);
static UWORD PM_HideProject(struct SuperWindow *sw);
static UWORD PM_RevealAll(void);
static UWORD PM_CloseItAllDown(void);

/*************  DoProjectMenu ***********/
//
//	Sort out everything to do with the Project Menu. If
// we closed a window then we return FALSE, else TRUE.
//
// 0 = All Ok , 1 = Stop Processing
// ********  FIX THE ABOVE COMMENT SOMETIME.  **********

UWORD DoProjectMenu(struct SuperWindow *sw,UWORD id)
{
	UWORD itemnum, subnum;

	itemnum = ITEMNUM(id);
	subnum = SUBNUM(id);

	switch(itemnum)
	{
		case MNPRJ_NEW:
			GUIOpenNewProject();
			return(0);
			break;
		case MNPRJ_OPEN:
			PM_OpenProject( sw->sw_Window );
			return(0);
			break;
		case MNPRJ_CLOSE:
			return(PM_CloseProject(sw));
			break;
		case MNPRJ_SAVE:
			if( sw->sw_Type == SWTY_PROJECT )
//				SaveProject( ( ( struct ProjectWindow *)sw )->pw_Project,
//					"ram:Temp.map", FALSE, FALSE );
				PM_Save( ( ( struct ProjectWindow *)sw )->pw_Project, sw->sw_Window );
			break;
		case MNPRJ_SAVEAS:
			if( sw->sw_Type == SWTY_PROJECT )
				PM_SaveAs( ( ( struct ProjectWindow *)sw )->pw_Project, sw->sw_Window );
			break;
		case MNPRJ_HIDE:
			return(PM_HideProject(sw));
			break;
		case MNPRJ_REVEAL:
			return(PM_RevealAll());
			break;
		case MNPRJ_ABOUT:
			OpenNewAboutWindow();
			return(0);
			break;
		case MNPRJ_QUIT:
			return(PM_CloseItAllDown());
			break;
	}
}



static void PM_OpenProject( struct Window *win )
{
	BOOL pleb;
	struct List *rlist;
//	char name[256];

//	GUIOpenProject( win );

	if(rlist = BlockAllWindows())
	{
		/* bring up the file requester */
		pleb = AslRequestTags( filereq,
			//ASLFR_Screen, mainscreen,
			ASLFR_Window, win,
			ASLFR_TitleText, "Open Project",
			ASLFR_DoSaveMode, FALSE,
			TAG_END );
		FreeBlockedWindows(rlist);

		if(pleb)
		{
			OpenProject(  filereq->fr_Drawer, filereq->fr_File );
		}
	}
}

void PM_Save( struct Project *proj, struct Window *win )
{
	if( proj->pj_UntitledFlag )
		GUISaveProjectAs( proj, win );
	else
		GUISaveProject( proj );
}


void PM_SaveAs( struct Project *proj, struct Window *win )
{
	GUISaveProjectAs( proj, win );
}


static UWORD PM_CloseProject(struct SuperWindow *sw)
{
	struct Project *proj;
	struct ProjectWindow *pw;
	UWORD retval = 0;

	pw = (struct ProjectWindow *)sw;

	if (proj = pw->pw_Project)
	{
		if (proj->pj_ModifiedFlag)
		{
			switch( ModifiedProjectRequester(proj) )
			{
				case 1:
					PM_Save( proj, pw->pw_sw.sw_Window );
					if( !proj->pj_ModifiedFlag )
					{
						CloseProject(proj);
						retval = 1;
					}
					break;
				case 2:
					CloseProject(proj);
					retval = 1;
					break;
			}
		}
		else
		{
			CloseProject(proj);
			retval = 1;
		}
	}
//	SussOutQuiting(FALSE);

	return(retval);
}


static UWORD PM_HideProject(struct SuperWindow *sw)
{
	struct Project *proj;
	struct ProjectWindow *pw;
	UWORD retval = 0;

	pw = (struct ProjectWindow *)sw;

	/* KLUDGE - to stop the luser hiding the last project */
	/* ...we've got to suss out some iconification sometime... */
	if(proj = pw->pw_Project)
	{
		for( pw=proj->pj_ProjectWindows; pw; pw=pw->pw_NextProjWindow)
		{
			(*pw->pw_sw.sw_HideWindow)(&pw->pw_sw);
			retval = 1;
		}
	}
	RV_RemakeAll();
	return(retval);
}


static UWORD PM_RevealAll(void)
{
	OpenNewRevealWindow(RWNS_PROJECTS);
	return(TRUE);
}



static UWORD PM_CloseItAllDown()
{
	struct Project *proj;
	struct SuperWindow *sw;
	int abort = FALSE;
	UWORD retval = 0;

	while(!IsListEmpty(&projects) && !abort)
	{
		proj = (struct Project *)projects.lh_Head;
		if (proj->pj_ModifiedFlag)
		{
			if (ModifiedProjectRequester(proj))
			{
				CloseProject(proj);
				retval = 1;
			}
			else
				abort = TRUE;
		}
		else
		{
			CloseProject(proj);
			retval = 1;
		}
	}
	while(!IsListEmpty(&superwindows) && !abort)
	{
		sw = (struct SuperWindow *)superwindows.lh_Head;
		(*sw->sw_BuggerOff)(sw);
		retval = 1;
	}

	SussOutQuiting(FALSE);
	return(retval);
}
