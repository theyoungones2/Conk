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

//#include <proto/exec.h>
//#include <proto/dos.h>
#include <proto/intuition.h>
//#include <proto/graphics.h>
//#include <proto/diskfont.h>
//#include <proto/asl.h>
//#include <clib/alib_protos.h>
//#include <proto/gadtools.h>

#include <global.h>
#include <ExternVars.h>


/* local protos */

static UWORD WM_SubMenuOpen(struct SuperWindow *sw, UWORD subnum);
static void WM_MakeDefault(struct SuperWindow *sw);
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
			OpenNewRevealWindow(&glob.cg_WindowDefaults[SWTY_REVEAL],RWNS_WINDOWS);
			return(0);
			break;
		case MNWIN_MAKEDEFAULT:
			WM_MakeDefault(sw);
			return(0);
			break;
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
		case MNWIN_VERTICAL:
			if(sw->sw_Type == SWTY_TOOLS)
			{
				((struct ToolsWindow *)sw)->tw_Vertical = mi->Flags & CHECKED;
				//(*sw->sw_HideWindow)(sw);
				//(*sw->sw_ShowWindow)(sw);
			}
			return(3);
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
			if( (sw->sw_Type == SWTY_PROJECT) && (proj = ((struct ProjectWindow *)sw)->pw_Project) )
				OpenNewProjWindow(&glob.cg_WindowDefaults[SWTY_PROJECT], proj);
			else
				OpenNewProject(&glob.cg_WindowDefaults[SWTY_PROJECT],NULL);
			break;
		case MNWINOPEN_ABOUT:
			OpenNewAboutWindow(&glob.cg_WindowDefaults[SWTY_ABOUT]);
			break;
		case MNWINOPEN_REVEAL:
			OpenNewRevealWindow(&glob.cg_WindowDefaults[SWTY_REVEAL],RWNS_WINDOWS);
			break;
		case MNWINOPEN_SCRCON:
			OpenNewScreenConfigWindow(&glob.cg_WindowDefaults[SWTY_SCREENCONFIG]);
			break;
		case MNWINOPEN_TOOLS:
			OpenNewToolsWindow(&glob.cg_WindowDefaults[SWTY_TOOLS]);
			break;
		case MNWINOPEN_USERCON:
			break;
		case MNWINOPEN_PALETTE:
			OpenNewPaletteWindow(&glob.cg_WindowDefaults[SWTY_PALETTE]);
			break;
		case MNWINOPEN_EDPALETTE:
			if(sw->sw_Type == SWTY_PROJECT)
				OpenNewEditPaletteWindow( &glob.cg_WindowDefaults[SWTY_EDITPALETTE],((struct ProjectWindow *)sw)->pw_Project, lmbcol);
			break;
		case MNWINOPEN_SEQUENCER:
			switch(sw->sw_Type)
			{
				case SWTY_PROJECT:
					OpenNewSeqWindow( &glob.cg_WindowDefaults[SWTY_SEQUENCE],((struct ProjectWindow *)sw)->pw_Project);
					break;
				case SWTY_SEQUENCE:
					OpenNewSeqWindow( &glob.cg_WindowDefaults[SWTY_SEQUENCE],((struct SeqWindow *)sw)->asw_Sequence->sq_Project);
					break;
			}
			break;
		case MNWINOPEN_RANGE:
			switch(sw->sw_Type)
			{
				case	SWTY_PROJECT:
					OpenNewRangeWindow(&glob.cg_WindowDefaults[SWTY_RANGE],((struct ProjectWindow *)sw)->pw_Project);
					break;
				case SWTY_SEQUENCE:
					OpenNewRangeWindow(&glob.cg_WindowDefaults[SWTY_RANGE],((struct SeqWindow *)sw)->asw_Sequence->sq_Project);
					break;
			}
	}
	return(0);
}

static void WM_MakeDefault(struct SuperWindow *sw)
{
	struct WindowDefault	*wf;
	struct Window					*win;

	if(win = sw->sw_Window)	/* just in case... */
	{
		wf = &glob.cg_WindowDefaults[sw->sw_Type];
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
	struct Project *pj;
	struct Palette *pal = NULL;

	switch(sw->sw_Type)
	{
		case SWTY_PROJECT:
			pj = ( (struct ProjectWindow *) sw)->pw_Project;
			if (pj->pj_ModifiedFlag)
			{
				if (ModifiedProjectRequester(pj))
					pal = (*sw->sw_BuggerOff)(sw);
			}
			else
				pal = (*sw->sw_BuggerOff)(sw);
			break;
		default:
			(*sw->sw_BuggerOff)(sw);
			break;
	}
	if(pal)
		InstallNewGlobPalette(pal);

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
	if(sw->sw_Type == SWTY_TOOLS)
	{
		nm[MNWIN_VERTICAL_IDX].nm_Flags = ((struct ToolsWindow *)sw)->tw_Vertical ?
			CHECKIT|MENUTOGGLE | CHECKED : CHECKIT|MENUTOGGLE;
	}
}

