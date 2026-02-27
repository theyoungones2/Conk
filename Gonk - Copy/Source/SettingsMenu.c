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
#include <libraries/iffparse.h>

#include <proto/exec.h>
#include <proto/dos.h>
#include <proto/intuition.h>
//#include <proto/graphics.h>
//#include <proto/diskfont.h>
#include <proto/asl.h>
//#include <clib/alib_protos.h>
//#include <proto/gadtools.h>
#include <proto/iffparse.h>

#include <global.h>
#include <ExternVars.h>

#define MN_SETTINGS					3

#define	ID_CONK		MAKE_ID('C','O','N','K')
#define	ID_GCFG		MAKE_ID('G','C','F','G')

/*************  DoSettingsMenu ***********/
//
//	Handle SettingsMenu selections.
//

void DoSettingsMenu(struct SuperWindow *sw,UWORD id)
{
	UWORD itemnum,subnum;
	struct MenuItem *mi;
	//struct List *winlist;
	struct SuperWindow *sw2;
	char *oldtitle;

	itemnum = ITEMNUM(id);
	subnum = SUBNUM(id);

	mi = ItemAddress( sw->sw_MenuStrip, id );

	WaitForSubTaskDone(mainmsgport);

	switch(itemnum)
	{
		case MNSET_SAVECHUNKY:
			/* twiddled a checkmark - gotta remake _all_ the menus... (bummer) */
			glob.cg_SaveChunky = (mi->Flags & CHECKED) ? TRUE : FALSE;
			RefreshSettingsMenu();
			break;
		case MNSET_MAKEBACKUPS:
			/* twiddled a checkmark - gotta remake _all_ the menus... (bummer) */
			glob.cg_MakeBackups = (mi->Flags & CHECKED) ? TRUE : FALSE;
			RefreshSettingsMenu();
			break;
		case MNSET_AUTOACTIVATE:
			/* twiddled a checkmark - gotta remake _all_ the menus... (bummer) */
			glob.cg_AutoActivate = (mi->Flags & CHECKED) ? TRUE : FALSE;
			RefreshSettingsMenu();
			break;
		case MNSET_KEEPSCREENMODE:
			/* twiddled a checkmark - gotta remake _all_ the menus... (bummer) */
			glob.cg_KeepScreenMode = (mi->Flags & CHECKED) ? TRUE : FALSE;
			RefreshSettingsMenu();
			break;
		case MNSET_EXCLUDEBRUSH:
			glob.cg_ExcludeBrush = (mi->Flags & CHECKED) ? TRUE : FALSE;
			RefreshSettingsMenu();
			break;
		case MNSET_WINDOWIDS:
			/* twiddled a checkmark - gotta remake _all_ the menus... (bummer) */
			glob.cg_ShowWindowID = (mi->Flags & CHECKED) ? TRUE : FALSE;
			for(sw2 = (struct SuperWindow *)superwindows.lh_Head;
					sw2->sw_Node.ln_Succ;
					sw2 = (struct SuperWindow *)sw2->sw_Node.ln_Succ)
			{
				oldtitle = sw2->sw_Window->Title;
				SetWindowTitles( sw2->sw_Window,
					sw2->sw_FlgTitle ? BuildWindowTitle(sw2) : NULL,
					(UBYTE *)~0 );
				if(oldtitle)
					FreeVec(oldtitle);
			}
			RefreshSettingsMenu();
			break;
		case MNSET_CREATEICONS:
			glob.cg_CreateIcons = (mi->Flags & CHECKED) ? TRUE : FALSE;
			RefreshSettingsMenu();
			break;
		case MNSET_SCREENSETTINGS:
			OpenNewScreenConfigWindow(&glob.cg_WindowDefaults[SWTY_SCREENCONFIG]);
			break;
		case MNSET_SAVESETTINGS:
			SaveSettings(Txt_DefaultConfig);
			break;
		case MNSET_SAVESETTINGSAS:
			SaveSettingsAs(sw);
			break;
		case MNSET_LOADSETTINGS:
			LoadSettingsAs(sw);
			break;
	}
}



/************* SetupSettingsMenu() *************/
//
// Sets up the settings menu before it is layed out
// and attached to a window
// (ie it alters the NewMenu array before it is passed
// to GadTools).
//

void SetupSettingsMenu(struct NewMenu *nm)
{
	/* suss out the state of the clicky checkmark things */
	nm[MNSET_SAVECHUNKY_IDX].nm_Flags = glob.cg_SaveChunky ?
		CHECKIT|MENUTOGGLE|CHECKED : CHECKIT|MENUTOGGLE;
	nm[MNSET_MAKEBACKUPS_IDX].nm_Flags = glob.cg_MakeBackups ?
		CHECKIT|MENUTOGGLE|CHECKED : CHECKIT|MENUTOGGLE;
	nm[MNSET_AUTOACTIVATE_IDX].nm_Flags = glob.cg_AutoActivate ?
		CHECKIT|MENUTOGGLE|CHECKED : CHECKIT|MENUTOGGLE;
	nm[MNSET_KEEPSCREENMODE_IDX].nm_Flags = glob.cg_KeepScreenMode ?
		CHECKIT|MENUTOGGLE|CHECKED : CHECKIT|MENUTOGGLE;
	nm[MNSET_EXCLUDEBRUSH_IDX].nm_Flags = glob.cg_ExcludeBrush ?
		CHECKIT|MENUTOGGLE|CHECKED : CHECKIT|MENUTOGGLE;
	nm[MNSET_WINDOWIDS_IDX].nm_Flags = glob.cg_ShowWindowID ?
		CHECKIT|MENUTOGGLE|CHECKED : CHECKIT|MENUTOGGLE;
	nm[MNSET_CREATEICONS_IDX].nm_Flags = glob.cg_CreateIcons ?
		CHECKIT|MENUTOGGLE|CHECKED : CHECKIT|MENUTOGGLE;
}

void RefreshSettingsMenu(void)
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
		if(mn = sw->sw_Menus[MN_SETTINGS])
		{
			for(mi = mn->FirstItem; mi; mi = mi->NextItem)
			{
				switch(count)
				{
					case MNSET_SAVECHUNKY:
						if(glob.cg_SaveChunky)
							mi->Flags |= CHECKED;
						else
							mi->Flags &= ~CHECKED;
						break;
					case MNSET_MAKEBACKUPS:
						if(glob.cg_MakeBackups)
							mi->Flags |= CHECKED;
						else
							mi->Flags &= ~CHECKED;
						break;
					case MNSET_AUTOACTIVATE:
						if(glob.cg_AutoActivate)
							mi->Flags |= CHECKED;
						else
							mi->Flags &= ~CHECKED;
						break;
					case MNSET_KEEPSCREENMODE:
						if(glob.cg_KeepScreenMode)
							mi->Flags |= CHECKED;
						else
							mi->Flags &= ~CHECKED;
						break;
					case MNSET_EXCLUDEBRUSH:
						if(glob.cg_ExcludeBrush)
							mi->Flags |= CHECKED;
						else
							mi->Flags &= ~CHECKED;
						break;
					case MNSET_WINDOWIDS:
						if(glob.cg_ShowWindowID)
							mi->Flags |= CHECKED;
						else
							mi->Flags &= ~CHECKED;
						break;
					case MNSET_CREATEICONS:
						if(glob.cg_CreateIcons)
							mi->Flags |= CHECKED;
						else
							mi->Flags &= ~CHECKED;
						break;
				}
				count++;
			}
		}
	}
}


/************* SaveSettingsAs() *************/
//
// Brings up a file requester to get a filename,
// and saves out the current configuration.
//

void SaveSettingsAs(struct SuperWindow *sw )
{
	char *name = NULL;

	if( GetFile(sw->sw_Window, "Save Settings As", NULL, "Gonk.cfg", TRUE, FALSE) )
	{
		/* glue together the full pathname */
		if( name = MakeFullPathName(filereq->fr_Drawer, filereq->fr_File) )
		{
			/* try and save it */
			SaveSettings(name);
			FreeVec(name);
		}
		else
		{
			/* no good - Couldn't make the full path name */
			InvalidNameRequester();
		}
	}
}



/************* SaveSettings() *************/
//
// Saves the current config out under the given name.
//

void SaveSettings(char *name)
{
	struct IFFHandle *handle;

	if( BlockAllSuperWindows() )
	{
		if( handle = AllocIFF() )
		{
			/* open the DOS file */
			if( handle->iff_Stream = Open( name, MODE_NEWFILE ) )
			{
				InitIFFasDOS( handle );
				if( !OpenIFF( handle, IFFF_WRITE ) )
				{
					if( !PushChunk( handle, ID_CONK, ID_FORM, IFFSIZE_UNKNOWN ) )
					{

						/* save out the map data */
						if( WriteGCFGChunk( handle, &glob ) )
						{
						}

						PopChunk( handle );
					}
					CloseIFF( handle );
				}
				Close( handle->iff_Stream );
			}
			FreeIFF( handle );
		}
		UnblockAllSuperWindows();
	}
}

BOOL WriteGCFGChunk( struct IFFHandle *handle, struct Config *cg)
{
	BOOL success = FALSE,abort = FALSE;
	struct SuperWindow *sw;
	struct ProjectWindow *pw;
	struct Project *pj;
	UWORD swcount = 0;
	struct WindowDefault *wfbase, *wf;
	ULONG size;
	UWORD flags = 0;

	for(sw = (struct SuperWindow *)superwindows.lh_Head;
			sw->sw_Node.ln_Succ;
			sw = (struct SuperWindow *)sw->sw_Node.ln_Succ)
	{
		if(sw->sw_Window)
		{
			RememberWindow(sw);
			swcount++;
		}
	}

	cg->cg_NumOfOpenWindows = swcount;

	if( !PushChunk( handle, ID_CONK, ID_GCFG, IFFSIZE_UNKNOWN ) )
	{
		glob.cg_WindowDefaults[SWTY_PROJECT].wf_Flags = EDTY_FRAME;
		glob.cg_WindowDefaults[SWTY_TOOLS].wf_Flags = WFF_TOOLSWINDOWVERTICAL;
		if( WriteChunkBytes( handle, cg, sizeof(struct Config) ) == sizeof(struct Config) )
		{
			size = sizeof(struct WindowDefault)*swcount;
			if( (swcount > 0) && (wfbase = AllocVec( size, MEMF_ANY|MEMF_CLEAR)) )
			{
				wf = wfbase;
				for(sw = (struct SuperWindow *)superwindows.lh_Head;
						sw->sw_Node.ln_Succ;
						sw = (struct SuperWindow *)sw->sw_Node.ln_Succ)
				{
					if( (sw->sw_Window) && (sw->sw_Type != SWTY_PROJECT) )
					{
						wf->wf_Left = sw->sw_Left;
						wf->wf_Top = sw->sw_Top;
						wf->wf_Width = sw->sw_Width;
						wf->wf_Height = sw->sw_Height;
						wf->wf_SWType = sw->sw_Type;
						wf->wf_FlgBackDrop = sw->sw_FlgBackDrop;
						wf->wf_FlgBorderless = sw->sw_FlgBorderless;
						wf->wf_FlgTitle = sw->sw_FlgTitle;
						wf->wf_FlgBorderGadgets = sw->sw_FlgBorderGadgets;
						flags = 0;
						if( (sw->sw_Type == SWTY_TOOLS) && ((struct ToolsWindow *)sw)->tw_Vertical )
							flags |= WFF_TOOLSWINDOWVERTICAL;
						wf->wf_Flags = flags;
						wf++;
					}
				}
				for(pj = (struct Project *)projects.lh_Head;
						pj->pj_Node.ln_Succ;
						pj = (struct Project *)pj->pj_Node.ln_Succ)
				{
					for(pw = pj->pj_ProjectWindows; pw; pw = pw->pw_NextProjWindow)
					{
						sw = &pw->pw_sw;
						if( sw->sw_Window )
						{
							wf->wf_Left = sw->sw_Left;
							wf->wf_Top = sw->sw_Top;
							wf->wf_Width = sw->sw_Width;
							wf->wf_Height = sw->sw_Height;
							wf->wf_SWType = sw->sw_Type;
							wf->wf_FlgBackDrop = sw->sw_FlgBackDrop;
							wf->wf_FlgBorderless = sw->sw_FlgBorderless;
							wf->wf_FlgTitle = sw->sw_FlgTitle;
							wf->wf_FlgBorderGadgets = sw->sw_FlgBorderGadgets;
							flags = pj->pj_EditType;
							if(pw != pj->pj_ProjectWindows) flags |= WFF_ATTACHED;
							wf->wf_Flags = flags;
							wf++;
						}
					}
				}
				if( WriteChunkBytes( handle, wfbase, size ) != size )
					abort = TRUE;
				FreeVec(wfbase);
				wfbase = NULL;
			}
			if(!abort)
			{
				size = 2 + (globpalette->pl_Count*3);
				if( WriteChunkBytes( handle, globpalette, size ) != size )
					abort = TRUE;
			}
		}
		else
			abort = TRUE;

		if(!abort)
		{
			if( PopChunk(handle) )
				 success = FALSE;
			else
				success = TRUE;
		}
		else
			success = FALSE;
	}
	return( success );
}

/************* LoadSettingsAs() *************/
//
// Brings up a file requester to load in a config
// file. Returns TRUE if all is cool and froody,
// else FALSE.
//

BOOL LoadSettingsAs(struct SuperWindow *sw)
{
	char *name = NULL;
	BOOL ret=FALSE;

	if( GetFile(sw->sw_Window, "Load Settings", NULL, "Gonk.cfg", FALSE, FALSE) )
	{
		/* glue together the full pathname */
		if( name = MakeFullPathName(filereq->fr_Drawer, filereq->fr_File) )
		{
			/* try and load it in */
			ret = LoadSettings(name);
			FreeVec(name);
		}
		else
		{
			/* no good - name too long */
			InvalidNameRequester();
		}
	}
	return(ret);
}



/************* LoadSettings() *************/
//
// Loads in and installs the specified config file.
// Returns Success.
//

BOOL LoadSettings(char *name)
{
	struct IFFHandle *handle = NULL;
	BOOL		result = FALSE;
	ULONG parseerr = 0;

	if( BlockAllSuperWindows() )
	{
		if( handle = AllocIFF() )
		{
			/* open the DOS file */
			if( handle->iff_Stream = Open( name, MODE_OLDFILE ) )
			{
				InitIFFasDOS( handle );
				if( !OpenIFF( handle, IFFF_READ ) )
				{
					StopChunk( handle, ID_CONK, ID_GCFG );
					parseerr = ParseIFF(handle, IFFPARSE_SCAN);
					if(parseerr == 0)
					{
						result = LoadGCFGChunk( handle );
					}
					else
						printf("ParseError: %ld\n",parseerr);
					CloseIFF( handle );
				}
				Close( handle->iff_Stream );
			}
			FreeIFF( handle );
		}
		UnblockAllSuperWindows();
	}
	return(result);
}

BOOL LoadGCFGChunk(struct IFFHandle *handle)
{
	struct	ContextNode		*text;
	LONG									 size;
	struct	Config				*cg;
	struct WindowDefault	*wf;
	struct SuperWindow		*sw,*nextsw;
	struct Palette				*pl;
	struct Project				*pj = NULL;
	UWORD									 windowcount;
	BOOL									 ret					=		FALSE;

	if( text = CurrentChunk(handle) )
	{
		size = text->cn_Size;
		if(cg = AllocVec(size,MEMF_ANY))
		{
			if(ReadChunkBytes(handle, cg, size) == size)
			{
				sw = (struct SuperWindow *)superwindows.lh_Head;
				while(sw->sw_Node.ln_Succ)
				{
					nextsw = (struct SuperWindow *)sw->sw_Node.ln_Succ;
					if(sw->sw_Type != SWTY_PROJECT)
						(*sw->sw_BuggerOff)(sw);
					else
					{
						pj = ((struct ProjectWindow *)sw)->pw_Project;
						if( (!pj->pj_IsFile) && (!pj->pj_ModifiedFlag) )
						{
							if( pl = (struct Palette *)(*sw->sw_BuggerOff)(sw) )
								InstallNewGlobPalette(pl);
						}
					}
					sw = nextsw;
				}
				wf = cg->cg_OpenWindowDefinitions;
				wf += cg->cg_NumOfOpenWindows;
				pl = (struct Palette *)wf;
				memcpy(&kludgepalette, pl, 2+(pl->pl_Count*3));
				if( InstallNewGlobalConfig(cg) )
				{
					wf = cg->cg_OpenWindowDefinitions;
					for(windowcount = 0; windowcount < cg->cg_NumOfOpenWindows; windowcount++)
					{
						switch(wf->wf_SWType)
						{
							case SWTY_PROJECT:
								if(wf->wf_Flags & WFF_ATTACHED)
									OpenNewProjWindow(wf, pj);
								else
									pj = OpenNewProject(wf,NULL);
								break;
							case SWTY_ABOUT:
								OpenNewAboutWindow(wf);
								break;
							case SWTY_REVEAL:
								OpenNewRevealWindow(wf, RWNS_WINDOWS);
								break;
							case SWTY_SCREENCONFIG:
								OpenNewScreenConfigWindow(wf);
								break;
							case SWTY_TOOLS:
								OpenNewToolsWindow(wf);
								break;
							case SWTY_USERCONFIG:
								break;
							case SWTY_PALETTE:
								OpenNewPaletteWindow(wf);
								break;
							case SWTY_EDITPALETTE:
								if(lastprojectwindow)
									OpenNewEditPaletteWindow(wf,lastprojectwindow->pw_Project, lmbcol);
								break;
							case SWTY_SEQUENCE:
								if(lastprojectwindow)
									OpenNewSeqWindow(wf,lastprojectwindow->pw_Project);
								break;
							case SWTY_RANGE:
								if(lastprojectwindow)
									OpenNewRangeWindow(wf,lastprojectwindow->pw_Project);
								break;
						}
						wf++;
					}
					if(lastprojectwindow)
						ActivateWindow(lastprojectwindow->pw_sw.sw_Window);
					ret = TRUE;
				}
			}
			FreeVec(cg);
		}
	}
	if(IsListEmpty(&superwindows))
		OpenNewProject(&glob.cg_WindowDefaults[SWTY_PROJECT],NULL);
	return(ret);
}
