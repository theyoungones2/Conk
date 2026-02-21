/****************************************************************************/
//
// BONK: WindowMenu.c
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

static UWORD WM_SubMenuOpen(struct SuperWindow *sw, UWORD subnum);
static void WM_CloseWindow(struct SuperWindow *sw);

/*************  DoWindowMenu ***********/
//
//	Sort out everything to do with the Window Menu.
//
// RETURN:	0 = Carry on - all ok.
//					1 = Stop processing menus in this window.
//					2 = Refresh SuperWindow when done (hide then show).
//					3 = Refresh SuperWindow _and_ stop processing menus.

UWORD DoWindowMenu(struct SuperWindow *sw,UWORD id)
{
	UWORD itemnum,subnum;
	struct MenuItem *mi;

	itemnum = ITEMNUM(id);
	subnum = SUBNUM(id);

	mi = ItemAddress( sw->sw_MenuStrip, id );

	switch(itemnum)
	{
		case MNWIN_OPEN:
			return( WM_SubMenuOpen( sw, subnum ) );
			break;
		case MNWIN_CLOSE:
			WM_CloseWindow(sw);
			return(1);		/* no more processing please. */
			break;
		case MNWIN_HIDE:
			(*sw->sw_HideWindow)(sw);
			RV_RemakeAll();
			return(0);
			break;
		case MNWIN_REVEAL:
			OpenNewRevealWindow(RWNS_WINDOWS);
			return(0);
			break;
//		case MNWIN_MAKEDEFAULT:
//			MakeDefault(sw);
//			return(0);
//			break;
		case MNWIN_BACKDROP:
			sw->sw_FlgBackDrop = mi->Flags & CHECKED;
			return(2);
			break;
		case MNWIN_BORDERLESS:
			sw->sw_FlgBorderless = mi->Flags & CHECKED;
			return(2);
			break;
		case MNWIN_TITLE:
			sw->sw_FlgTitle = mi->Flags & CHECKED;
			return(2);
			break;
		case MNWIN_BORDERGADGETS:
			sw->sw_FlgBorderGadgets = mi->Flags & CHECKED;
			return(2);
			break;
	}
	return(0);
}


static UWORD WM_SubMenuOpen(struct SuperWindow *sw, UWORD subnum)
{
	struct Project *proj;

	switch(subnum)
	{
		case MNWINOPEN_PROJ:
			switch( sw->sw_Type )
			{
				case SWTY_PROJECT:
					/* load a project in from disk */
					if (proj = ((struct ProjectWindow *)sw)->pw_Project)
						OpenNewProjWindow(proj);
					break;
				case SWTY_PREVIEW:
					/* load a project in from disk */
					if (proj = ((struct PreviewWindow *)sw)->vw_Project)
						OpenNewProjWindow(proj);
					break;
				default:
					/* open a untitled new project */
					GUIOpenNewProject();
					break;
			}
			break;
		case MNWINOPEN_TOOLS:
			OpenNewToolsWindow();
			break;
		case MNWINOPEN_BLOCKS:
			OpenNewBlockWindow();
			break;
		case MNWINOPEN_BRUSHES:
			OpenNewBrushWindow();
			break;
		case MNWINOPEN_ABOUT:
			OpenNewAboutWindow();
			break;
		case MNWINOPEN_REVEAL:
			OpenNewRevealWindow(RWNS_WINDOWS);
			break;
		case MNWINOPEN_SCRCON:
			OpenNewScreenConfigWindow();
			break;
		case MNWINOPEN_PREVIEW:
			switch( sw->sw_Type )
			{
				case SWTY_PROJECT:
					if (proj = ((struct ProjectWindow *)sw)->pw_Project)
						OpenNewPreviewWindow(proj);
					break;
				case SWTY_PREVIEW:
					if (proj = ((struct PreviewWindow *)sw)->vw_Project)
						OpenNewPreviewWindow(proj);
					break;
				default:
					GroovyReq("BONK", "Select a Map window first",
						"OK" );
					break;
			}
			break;
	}
	return(0);
}

void MakeDefault(struct SuperWindow *sw)
{
	struct WindowDefault	*wf;
	struct Window					*win;

	if(win = sw->sw_Window)	/* just in case... */
	{
		wf = &glob.cg_WinDefs[sw->sw_Type];
		wf->wf_Left = win->LeftEdge;
		wf->wf_Top = win->TopEdge;
		wf->wf_Width = win->Width;
		wf->wf_Height = win->Height;
		wf->wf_FlgBackDrop = sw->sw_FlgBackDrop;
		wf->wf_FlgBorderless = sw->sw_FlgBorderless;
		wf->wf_FlgTitle = sw->sw_FlgTitle;
		wf->wf_FlgBorderGadgets = sw->sw_FlgBorderGadgets;
	}
}


static void WM_CloseWindow(struct SuperWindow *sw)
{

	switch(sw->sw_Type)
	{
		case SWTY_PROJECT:
			MaybeCloseProjectWindow( (struct ProjectWindow *)sw );
//			proj = ( (struct ProjectWindow *) sw)->pw_Project;
//			if (proj->pj_ModifiedFlag)
//			{
//				if (ModifiedProjectRequester(proj))
//					KillOffProjWindow(sw);
//					(*sw->sw_BuggerOff)(sw);
//			}
//			else
//			(*sw->sw_BuggerOff)(sw);
//			KillOffProjWindow(sw);
			break;
		default:
			(*sw->sw_BuggerOff)(sw);
			break;
	}
	SussOutQuiting(FALSE);
}


void SetupWindowMenu(struct SuperWindow *sw, struct NewMenu *nm)
{
	nm[MNWIN_BACKDROP_IDX].nm_Flags = sw->sw_FlgBackDrop ?
		CHECKIT|MENUTOGGLE | CHECKED : CHECKIT|MENUTOGGLE;
	nm[MNWIN_BORDERLESS_IDX].nm_Flags = sw->sw_FlgBorderless ?
		CHECKIT|MENUTOGGLE | CHECKED : CHECKIT|MENUTOGGLE;
	nm[MNWIN_TITLE_IDX].nm_Flags = sw->sw_FlgTitle ?
		CHECKIT|MENUTOGGLE | CHECKED : CHECKIT|MENUTOGGLE;
	nm[MNWIN_BORDERGADGETS_IDX].nm_Flags = sw->sw_FlgBorderGadgets ?
		CHECKIT|MENUTOGGLE | CHECKED : CHECKIT|MENUTOGGLE;
}

