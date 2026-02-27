/*
    Do all the default Project Menu crap
 */

#include <math.h>
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
#include <libraries/asl.h>
#include <workbench/startup.h>

#include <proto/exec.h>
#include <proto/dos.h>
#include <proto/intuition.h>
#include <proto/graphics.h>
//#include <proto/diskfont.h>
#include <proto/asl.h>
//#include <clib/alib_protos.h>
//#include <proto/gadtools.h>
#include <proto/iffparse.h>

#include <global.h>
#include <ExternVars.h>
#include <ilbm.h>
#include <HoopyVision.h>

static BOOL  PM_OpenProjectAs(struct SuperWindow *sw);
static BOOL  PM_OpenProject(struct SuperWindow *sw,STRPTR file, STRPTR drawer);
static UWORD PM_CloseProject(struct SuperWindow *sw);
static UWORD PM_HideProject(struct SuperWindow *sw);
static UWORD PM_RevealAll(void);
static UWORD PM_CloseItAllDown(void);
static BOOL PM_SaveFile(struct File *fl);
static UWORD PM_ScreenMode(struct SuperWindow *sw);
static BOOL PM_WriteProject(char *name, struct Project *pj, struct IFFHandle *handle);
BOOL PM_PageSize(struct ProjectWindow *pw);

/*************  DoProjectMenu ***********/
//
//	Sort out everything to do with the Project Menu.
//	RETURN:
//		0 = All Ok,
//		1 = Stop Processing Menu events.	\__ Mutually inclusive.
//		2 = Refresh Window.								/

UWORD DoProjectMenu(struct SuperWindow *sw,UWORD id)
{
	UWORD itemnum, subnum;
	struct SeqWindow *asw;
	struct Sequence *sq;
	struct Project *pj;
	struct ProjectWindow *pw;

	itemnum = ITEMNUM(id);
	subnum = SUBNUM(id);

	WaitForSubTaskDone(mainmsgport);

	switch(itemnum)
	{
		case MNPRJ_NEW:
			OpenNewProject(&glob.cg_WindowDefaults[SWTY_PROJECT], NULL);
			return(0);
			break;
		case MNPRJ_OPEN:
			PM_OpenProjectAs(sw);
			return(0);
			break;
		case MNPRJ_SAVE:
			switch(sw->sw_Type)
			{
				case SWTY_PROJECT:
					pw = (struct ProjectWindow *)sw;
					if(pw->pw_Project->pj_File)
						PM_SaveFile(pw->pw_Project->pj_File);
					else
						PM_SaveFileAs(pw);
					break;
				case SWTY_SEQUENCE:
					asw = (struct SeqWindow *)sw;
					if(sq = asw->asw_Sequence)
					{
						if(pj = sq->sq_Project)
						{							
							for(pw = pj->pj_ProjectWindows;
									(pw) && (!pw->pw_sw.sw_Window);
									pw = pw->pw_NextProjWindow);
							if(!pw)
								pw = pj->pj_ProjectWindows;
							if(pw->pw_Project->pj_File)
								PM_SaveFile(pw->pw_Project->pj_File);
							else
								PM_SaveFileAs(pw);
						}
					}
					break;
			}
			return(0);
			break;
		case MNPRJ_SAVEAS:
			switch(sw->sw_Type)
			{
				case SWTY_PROJECT:
					PM_SaveFileAs((struct ProjectWindow *)sw);
					break;
				case SWTY_SEQUENCE:
					asw = (struct SeqWindow *)sw;
					if(sq = asw->asw_Sequence)
					{
						if(pj = sq->sq_Project)
						{							
							for(pw = pj->pj_ProjectWindows;
									(pw) && (!pw->pw_sw.sw_Window);
									pw = pw->pw_NextProjWindow);
							if(!pw)
								pw = pj->pj_ProjectWindows;
							PM_SaveFileAs(pw);
						}
					}
					break;
			}
			return(0);
			break;
		case MNPRJ_HIDE:
			return(PM_HideProject(sw));
			break;
		case MNPRJ_REVEAL:
			return(PM_RevealAll());
			break;
		case MNPRJ_CLOSE:
			return(PM_CloseProject(sw));
			break;
		case MNPRJ_PAGESIZE:
			PM_PageSize((struct ProjectWindow *)sw);
			return(0);
			break;
		case MNPRJ_SCREENMODE:
			return( PM_ScreenMode(sw) );
			break;
		case MNPRJ_ICONIFY:
			return(0);
			break;
		case MNPRJ_ABOUT:
			OpenNewAboutWindow(&glob.cg_WindowDefaults[SWTY_ABOUT]);
			return(0);
			break;
		case MNPRJ_QUIT:
			return(PM_CloseItAllDown());
			break;
	}
}

static BOOL PM_OpenProjectAs(struct SuperWindow *sw)
{
	BOOL ret = FALSE;
	UBYTE lockbuffer[256];
	UWORD count;
	struct WBArg *wbarg;

	if( GetFile(sw->sw_Window, "Open Project", NULL, NULL, FALSE, TRUE) )
	{
		if(filereq->fr_NumArgs > 0)
		{
			wbarg = filereq->fr_ArgList;
			for(count = 0; count < filereq->fr_NumArgs; count++)
			{
				if(NameFromLock(wbarg->wa_Lock, lockbuffer, 256))
				{
					/* try and load it in */
					ret = PM_OpenProject(sw, wbarg->wa_Name, lockbuffer);
				}
				wbarg++;
			}
		}
	}
	return(ret);
}


static BOOL PM_OpenProject(struct SuperWindow *sw,STRPTR file, STRPTR drawer)
{
	struct Project *pj = NULL;

	if( BlockAllSuperWindows() )
	{
		if(sw->sw_Type == SWTY_PROJECT)
			pj = ((struct ProjectWindow *)sw)->pw_Project;

		if( !OpenFile(file,drawer, pj) )
		{
			GroovyReq("Open Project Error","Open Project\n%s Failed","Continue",file);
		}
		UnblockAllSuperWindows();
	}
	return(TRUE);
}

static BOOL PM_SaveFile(struct File *fl)
{
	char *name;
	struct IFFHandle *handle;
	BOOL success = FALSE;
	struct Project *pj;

	if(fl)
	{
		if( name = MakeFullPathName(fl->fl_Drawer, fl->fl_Name) )
		{
			if( BlockAllSuperWindows() )
			{
				if( handle = AllocIFF() )
				{
					if( (!glob.cg_MakeBackups) ||
							(glob.cg_MakeBackups && MakeBackupFile(name)) )
					{
						/* open the DOS file */
						if( handle->iff_Stream = Open( name, MODE_NEWFILE ) )
						{
							InitIFFasDOS( handle );
							if( !OpenIFF( handle, IFFF_WRITE ) )
							{
								pj = fl->fl_FirstProject;
								if(pj->pj_EditType == EDTY_FRAME)
									success = SaveILBM(pj, handle);
								else
								{
									/* Not an ILBM Picture */
									if( !PushChunk( handle, ID_CONK, ID_FORM, IFFSIZE_UNKNOWN ) )
									{
										success = TRUE;
										for(; pj && success; pj = pj->pj_NextProjectInFile)
											success = PM_WriteProject(name, pj, handle);
										if(success)
											success = WriteCrapChunks(fl, handle);
					
										PopChunk( handle );
									}
								}
								CloseIFF( handle );
							}
							else
								printf("OpenIFF() Failed\n");

							Close( handle->iff_Stream );
						}
						else
							printf("Open() Failed\n");
					}
					else
						printf("MakeBackups() Failed\n");

					FreeIFF( handle );
				}
				UnblockAllSuperWindows();
			}
			else
				printf("BlockAllSuperWindows() Failed\n");
			FreeVec(name);
			RefreshAllProjectTitles();
		}
		else
		{
			/* no good - Couldn't make the full path name */
			InvalidNameRequester();
		}
	}
	else
		printf("SaveFile() - No File\n");
	return(success);
}

BOOL PM_SaveFileAs(struct ProjectWindow *pw)
{
	struct File *fl;
	struct Project *pj;
	char *title;
	BOOL success = FALSE;

	if(pw)
	{
		pj = pw->pw_Project;
		fl = pj->pj_File;

		switch(pj->pj_EditType)
		{
			case EDTY_BLOCKS:
				title = "Enter Block File Name";
				break;
			case EDTY_BOBS:
				title = "Enter Bob File Name";
				break;
			case EDTY_SPRITES:
				title = "Enter Sprite File Name";
				break;
			case EDTY_FRAME:
				title = "Enter ILBM File Name";
				break;
		}
	
		if( GetFile(pw->pw_sw.sw_Window, title, fl ? fl->fl_Drawer : NULL, fl ? fl->fl_Name : NULL, TRUE, FALSE) )
		{
			if(!fl) fl = AllocNewFile();
			if(fl)
			{
				SetFileName(fl, filereq->fr_File, filereq->fr_Drawer);
				if(!fl->fl_FirstProject)
					LinkProjectToFile(fl,pj);
			}
			pj->pj_IsFile = TRUE;
			success = PM_SaveFile(fl);
		}
	}
	return(success);
}

static BOOL PM_WriteProject(char *name, struct Project *pj, struct IFFHandle *handle)
{
	BOOL ret = FALSE;

	switch(pj->pj_EditType)
	{
		case EDTY_BLOCKS:
			ret = SaveBlocks(name, pj, handle);
			pj->pj_ModifiedFlag = FALSE;
			break;
		case EDTY_BOBS:
			ret = SaveBobs(name, pj, handle);
			pj->pj_ModifiedFlag = FALSE;
			break;
		case EDTY_SPRITES:
			ret = SaveSprites(name, pj, handle);
			pj->pj_ModifiedFlag = FALSE;
			break;
		case EDTY_FRAME:
			GroovyReq("Error","Still Frame Saving\nNot Implemented Yet","Abort");
			break;
	}
	return(ret);
}

static UWORD PM_CloseProject(struct SuperWindow *sw)
{
	struct Project *pj;
	struct ProjectWindow *pw;
	struct Palette *pal = NULL;
	UWORD retval = 0;

	pw = (struct ProjectWindow *)sw;

	if (pj = pw->pw_Project)
	{
		if (pj->pj_ModifiedFlag)
		{
			if (ModifiedProjectRequester(pj))
			{
				pal = CloseProject(pj);
				retval = 1;
			}
		}
		else
		{
			pal = CloseProject(pj);
			retval = 1;
		}
	}
	if(pal)
		InstallNewGlobPalette(pal);

	SussOutQuiting(FALSE);

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
	OpenNewRevealWindow(&glob.cg_WindowDefaults[SWTY_REVEAL],RWNS_PROJECTS);
	return(TRUE);
}



static UWORD PM_CloseItAllDown()
{
	struct Project *proj;
	struct SuperWindow *sw;
	int abort = FALSE;
	UWORD retval = 0;
	struct Palette *pal;

	while(!IsListEmpty(&projects) && !abort)
	{
		proj = (struct Project *)projects.lh_Head;
		if (proj->pj_ModifiedFlag)
		{
			if (ModifiedProjectRequester(proj))
			{
				pal = CloseProject(proj);
				retval = 1;
			}
			else
				abort = TRUE;
		}
		else
		{
			pal = CloseProject(proj);
			retval = 1;
		}
	}
	while(!IsListEmpty(&superwindows) && !abort)
	{
		sw = (struct SuperWindow *)superwindows.lh_Head;
		(*sw->sw_BuggerOff)(sw);
		retval = 1;
	}

	if( (!IsListEmpty(&superwindows)) && pal)
		InstallNewGlobPalette(pal);

	SussOutQuiting(FALSE);
	return(retval);
}

static UWORD PM_ScreenMode(struct SuperWindow *sw)
{
	struct ScreenConfig *sg;
	BOOL	pleb;

	sg = &glob.cg_ScrConfig;
	if( BlockAllSuperWindows() )
	{
		/* Call up the ASL screenmode requester (wb2.1+ only...) */
		/* gotta suss out an alternative too... */

		pleb = AslRequestTags(screenmodereq,
			ASLSM_Window,								sw->sw_Window,
			ASLSM_InitialDisplayID,			sg->sg_DisplayID,
			ASLSM_InitialDisplayWidth,	sg->sg_DisplayWidth,
			ASLSM_InitialDisplayHeight,	sg->sg_DisplayHeight,
			ASLSM_InitialDisplayDepth,	sg->sg_DisplayDepth,
			ASLSM_InitialOverscanType,	sg->sg_OverscanType,
			ASLSM_InitialAutoScroll,		sg->sg_AutoScroll,
			TAG_END );
		UnblockAllSuperWindows();
		if(pleb)
		{
			/* Copy out all the groovy new userselected screenmode data */
			/* into our local ScreenConfig struct */

			sg->sg_DisplayID = screenmodereq->sm_DisplayID;
			sg->sg_DisplayWidth = screenmodereq->sm_DisplayWidth;
			sg->sg_DisplayHeight = screenmodereq->sm_DisplayHeight;
			sg->sg_DisplayDepth = screenmodereq->sm_DisplayDepth;
			sg->sg_OverscanType = screenmodereq->sm_OverscanType;
			sg->sg_AutoScroll = screenmodereq->sm_AutoScroll;
			RemakeScreen();
			return(1);
		}
	}
	return(0);
}

BOOL PM_PageSize(struct ProjectWindow *pw)
{
	struct Project *pj;
	struct ProjectWindow *pw2;
	struct BitMap *undobm=NULL,*maskbm=NULL;
	struct RastPort *mainrp=NULL;
	UWORD width,height;
	UWORD minwidth,minheight;
	static struct HoopyObject hob[] =
	{
		{ HOTYPE_VGROUP, 100, HOFLG_FREEWIDTH|HOFLG_FREEHEIGHT,4,0,0,0, NULL, 0,0,0,0,0 },
			{ HOTYPE_INTEGER, 100, 0, 5,0,0,0, "Width",FALSE,0,PLACETEXT_LEFT,0,0 },
			{ HOTYPE_INTEGER, 100, 0, 5,0,0,0, "Height",FALSE,0,PLACETEXT_LEFT,0,0 },
			{ HOTYPE_SPACE, 10, HOFLG_NOBORDER|HOFLG_FREEWIDTH|HOFLG_FREEHEIGHT, 0,0,0,0, NULL,0,0,0,0,0 },
			{ HOTYPE_HGROUP, 100, HOFLG_NOBORDER|HOFLG_FREEWIDTH,3,0,0,0, NULL, 0,0,0,0,0 },
				{ HOTYPE_BUTTON, 100, 0, 0,0,0,0, "OK",0,0x8000,0,0,0 },
				{ HOTYPE_SPACE, 100, HOFLG_NOBORDER|HOFLG_FREEWIDTH, 0,0,0,0, NULL,0,0,0,0,0 },
				{ HOTYPE_BUTTON, 100, 0, 0,0,0,0, "Cancel",0,0xFFFF,0,0,0 },
		{HOTYPE_END}
	};


	pj = pw->pw_Project;

	hob[1].ho_Value = pj->pj_Width;
	hob[2].ho_Value = pj->pj_Height;

	WaitForSubTaskDone(mainmsgport);

	if(	HO_SimpleDoReq( hob, "Page Size", NULL) != 0xFFFF)
	{
		width = hob[1].ho_Value;
		height = hob[2].ho_Value;
		minwidth = min(width,pj->pj_Width);
		minheight = min(height,pj->pj_Height);
		if( (width < pj->pj_Width) || (height < pj->pj_Height) )
		{
			if(!GroovyReq("Page Size","This Will Crop The Image\nAre You Sure?","Yes|No"))
				return(FALSE);
		}

		if( (pj->pj_MainRPort) && !(mainrp = AllocBitMapRPort(width,height,pj->pj_Depth)) )
		{
			GroovyReq("Page Size","Not Enough Memory","Continue");
			return(FALSE);
		}
		if( (pj->pj_UndoBitMap) && !(undobm = CreateBitMap(width,height,pj->pj_Depth,TRUE)) )
		{
			FreeBitMapRPort(mainrp,width,height,pj->pj_Depth);
			GroovyReq("Page Size","Not Enough Memory","Continue");
			return(FALSE);
		}
		if( (pj->pj_MaskBitMap) && !(maskbm = CreateBitMap(width,height,1,TRUE)) )
		{
			FreeBitMapRPort(mainrp,width,height,pj->pj_Depth);
			DestroyBitMap(undobm,width,height,pj->pj_Depth);
			GroovyReq("Page Size","Not Enough Memory","Continue");
			return(FALSE);
		}

		if(pj->pj_MainRPort)
		{
			BltBitMap(pj->pj_MainBitMap,0,0,mainrp->BitMap,0,0,minwidth,minheight,0xC0,0xFF,NULL);
			FreeBitMapRPort(pj->pj_MainRPort,pj->pj_Width,pj->pj_Height,pj->pj_Depth);
			pj->pj_MainRPort = mainrp;
			pj->pj_MainBitMap = mainrp->BitMap;
		}
		if(pj->pj_UndoBitMap)
		{
			BltBitMap(pj->pj_UndoBitMap,0,0,undobm,0,0,minwidth,minheight,0xC0,0xFF,NULL);
			DestroyBitMap(pj->pj_UndoBitMap,pj->pj_Width,pj->pj_Height,pj->pj_Depth);
			pj->pj_UndoBitMap = undobm;
		}
		if(pj->pj_MaskBitMap)
		{
			BltBitMap(pj->pj_MaskBitMap,0,0,maskbm,0,0,minwidth,minheight,0xC0,0xFF,NULL);
			DestroyBitMap(pj->pj_MaskBitMap,pj->pj_Width,pj->pj_Height,1);
			pj->pj_MaskBitMap = maskbm;
		}
		pj->pj_Width = width;
		pj->pj_Height = height;
		for(pw2 = pj->pj_ProjectWindows; pw2; pw2 = pw2->pw_NextProjWindow)
			RedisplayProjWindow(pw2,TRUE);
		return(TRUE);
	}
	return(FALSE);
}
