#include <stdio.h>
//#include <stdarg.h>
//#include <string.h>
//#include <dos/dos.h>
//#include <dos/dostags.h>
#include <exec/exec.h>
#include <intuition/intuition.h>
//#include <graphics/gfxbase.h>
//#include <rexx/rxslib.h>
//#include <libraries/gadtools.h>
//#include <libraries/diskfont.h>
//#include <libraries/asl.h>

#include <proto/exec.h>
#include <proto/dos.h>
#include <proto/intuition.h>
#include <proto/graphics.h>
//#include <proto/diskfont.h>
//#include <proto/asl.h>
//#include <clib/alib_protos.h>
//#include <proto/gadtools.h>
#include <datatypes/pictureclass.h>
#include <proto/iffparse.h>

#include <Global.h>
#include <ExternVars.h>
#include <HoopyVision.h>

struct File *AllocNewFile(void)
{
	struct File *fl;

	if( !(fl = AllocVec(sizeof(struct File), MEMF_CLEAR|MEMF_ANY)) )
		return(NULL);
	fl->fl_Name = MyStrDup("Unitiled");
	fl->fl_Drawer = NULL;
	NewList(&fl->fl_CrapChunks);
	fl->fl_FirstProject = NULL;
	return(fl);
}

void FreeFile(struct File *fl)
{
	struct CrapChunk *cc;
	struct Palette *pal;

	if(fl)
	{
		if(fl->fl_Name) FreeVec(fl->fl_Name);
		if(fl->fl_Drawer) FreeVec(fl->fl_Drawer);
		while(!IsListEmpty(&fl->fl_CrapChunks))
		{
			cc = (struct CrapChunk *)fl->fl_CrapChunks.lh_Head;
			Remove(&cc->cc_Node);
			if(cc->cc_ChunkData) FreeVec(cc->cc_ChunkData);
			FreeVec(cc);
		}
		while(fl->fl_FirstProject)
		{
			pal = CloseProject(fl->fl_FirstProject);
			if(pal)
				InstallNewGlobPalette(pal);
		}
		FreeVec(fl);
	}
}

void SetFileName(struct File *fl, char *name, char *drawer)
{
	if(fl)
	{
		if(fl->fl_Name) FreeVec(fl->fl_Name);
		if(fl->fl_Drawer) FreeVec(fl->fl_Drawer);
		fl->fl_Name = NULL;
		fl->fl_Drawer = NULL;
		if(name) fl->fl_Name = MyStrDup(name);
		if(drawer) fl->fl_Drawer = MyStrDup(drawer);
	}
}

/********************   LinkProjectToFile()   **********************/
//
//  Link the given project to the end of the files' project list.

void LinkProjectToFile(struct File *fl, struct Project *pj)
{
	struct Project *tmppj;

	if(fl && pj)
	{
		pj->pj_File = fl;
		if(fl->fl_FirstProject)
		{
			for(tmppj = fl->fl_FirstProject; tmppj->pj_NextProjectInFile; tmppj = tmppj->pj_NextProjectInFile);
			tmppj->pj_NextProjectInFile = pj;							//Add ourselves at the tail of the
			pj->pj_NextProjectInFile = NULL;							//file project list.
		}
		else
		{
			pj->pj_NextProjectInFile = NULL;							//Add ourselves at the tail of the
			fl->fl_FirstProject = pj;											//file project list.
		}
	}
}

/**************** OpenMinProject() ****************/
//
//  Creates a new, untitled Project and attaches a default window to it.
//  This doesn't allocate any bitmaps. BitMaps must be allocated by some
// other routine before this project is referenced by the main event
// handler.
// Inputs:	Window Default Structure to use
//					File Structure to attach it to. (Can be NULL)
// Returns:	A pointer to the new project structure.
//					NULL = Error.
//

struct Project *OpenMinProject(struct WindowDefault *wf, struct File *fl)
{
	struct Project *pj;
	struct Palette *oldpal;

	if( !(pj = AllocVec(sizeof(struct Project), MEMF_CLEAR|MEMF_ANY)) )
		return(NULL);

	pj->pj_ProjectWindows = NULL;
	pj->pj_ModifiedFlag = FALSE;
	pj->pj_MainBitMap = NULL;
	pj->pj_UndoBitMap = NULL;
	pj->pj_MaskBitMap = NULL;
	LinkProjectToFile(fl,pj);
	memcpy( &pj->pj_Palette, globpalette, sizeof(struct Palette) );
	oldpal = globpalette;
	globpalette = &pj->pj_Palette;
	pj->pj_IsFile = FALSE;
	pj->pj_Disabled = FALSE;
	pj->pj_Edable = TRUE;
	pj->pj_UndoLeft = 0;
	pj->pj_UndoRight = 1;
	pj->pj_UndoTop = 0;
	pj->pj_UndoBottom = 1;
	pj->pj_EditType = wf->wf_Flags & WF_PROJECTTYPEMASK;
	pj->pj_CutBobBoxWidth = 16;
	pj->pj_CutBobBoxHeight = 16;
	pj->pj_CurrentRange = pj->pj_Ranges;
	memset(&pj->pj_Ranges[0],0xFF,512);
	memset(&pj->pj_Ranges[1],0xFF,512);
	memset(&pj->pj_Ranges[2],0xFF,512);
	memset(&pj->pj_Ranges[3],0xFF,512);
	memset(&pj->pj_Ranges[4],0xFF,512);
	memset(&pj->pj_Ranges[5],0xFF,512);
	memset(&pj->pj_Ranges[6],0xFF,512);
	memset(&pj->pj_Ranges[7],0xFF,512);

	NewList(&pj->pj_BobList);
	if( (pj->pj_EditType == EDTY_BOBS) || (pj->pj_EditType == EDTY_SPRITES) )
	{
		AllocNextBob(pj, &pj->pj_BobList);
		pj->pj_CurrentBob = (struct MyBob *)pj->pj_BobList.lh_Head;
	}
	else
		pj->pj_CurrentBob = NULL;

	strcpy(pj->pj_BankName,"-<UNTITLED>-");

	AddHead(&projects,&pj->pj_Node);

	if( !OpenNewProjWindow(wf, pj) )
	{
		globpalette = oldpal;
		Remove(&pj->pj_Node);
		FreeVec(pj);
		return(NULL);
	}
	else
		RefreshAllProjectTitles();

	return(pj);
}

/**************** OpenNewProject() ****************/
//
//  This is just an extension on OpenMinProject(), but allocates
// a couple of bitmaps the size of the screen.
//
// Inputs:	Window Default Structure to use
//					File Structure to attach it to. (Can be NULL)
// Returns: Pointer to new project, NULL = Error.
//
struct Project *OpenNewProject(struct WindowDefault *wf, struct File *fl)
{
	struct Project *pj;

	if( !(pj = OpenMinProject(wf,fl)) )
		return(NULL);

	if( !SetUpDefaultBitMaps(pj) )
	{
		CloseProject(pj);
		return(NULL);
	}
	if(pj->pj_ProjectWindows)
		RedisplayProjWindow(pj->pj_ProjectWindows, TRUE);
	RedisplayAllPaletteWindows();
	return(pj);
}

/**************** CloseProject() ****************/
//
// Closes a project by going through and killing off all the
// superwindows attached to it. Note that the project is actually
// killed by FreeProject(), which is called automagically by the
// superwindow shutdown.
//
// Returns: A pointer to a new palette to use.
//					NULL = No need to change current palette.
//
struct Palette *CloseProject(struct Project *pj)
{
	struct ProjectWindow *pw;
	struct Palette *pal;

	for (pw = pj->pj_ProjectWindows; pw; pw = pj->pj_ProjectWindows)
	{
		pal = (*pw->pw_sw.sw_BuggerOff)(&pw->pw_sw);
	}
	return(pal);
}

/**************** FreeProject() ****************/
//
// Frees up any resources used by a project and removes it
// from the projects list.
// Returns: NULL        == No need to change palette;
//					(Palette *) == Palette struct to use;

struct Palette *FreeProject(struct Project *pj)
{
	struct Project *pj2,*useproj=NULL;
	BOOL done=FALSE;
	struct Palette *pal = NULL;
	struct MyBob *bob;
	struct File *fl;

	/* Find the next palette to use after this project is free'd */

	if(globpalette == &pj->pj_Palette)
	{
		for(pj2 = (struct Project *)projects.lh_Head;
				(pj2->pj_Node.ln_Succ) && (!done);
				pj2 = (struct Project *)pj2->pj_Node.ln_Succ)
		{
			if(pj2 != pj)
			{
				useproj = pj2;
				if(useproj->pj_Depth == pj->pj_Depth)
					done = TRUE;
			}
		}
		if(useproj)
			pal = &useproj->pj_Palette;
		else
			pal = &kludgepalette;
	}

	/* free data here... */

	while(pj->pj_EditPaletteWindows)
		(*pj->pj_EditPaletteWindows->epw_sw.sw_BuggerOff)(&pj->pj_EditPaletteWindows->epw_sw);

	FreeSequence(pj);
	while( !IsListEmpty(&pj->pj_BobList) )
	{
		bob = (struct MyBob *)pj->pj_BobList.lh_Head;
		Remove(&bob->bb_Node);
		FreeBobBitMaps(bob);
		FreeVec(bob);
	}
	DeallocProjBitMaps(pj);

	if(fl = pj->pj_File)
	{
		UnlinkProjectFromFile(pj);
		if(!fl->fl_FirstProject)
			FreeFile(fl);
	}
	Remove(&pj->pj_Node);
	FreeVec(pj);
	return(pal);
}


/**************** UnlinkProjectFromFile() ****************/
//
// Detaches a project from its parent file.
// (note that we assume that the project _is_ actually
// attached to a file in the first place).

void UnlinkProjectFromFile(struct Project *pj)
{
	struct Project *prevpj,*nextpj;
	struct File *fl;

	/*	unlink the project from the file project list */

	fl = pj->pj_File;

	nextpj = pj->pj_NextProjectInFile;
	prevpj = GetPrevProjectInFile(pj);

	if(prevpj == NULL)
		fl->fl_FirstProject = nextpj;
	else
		prevpj->pj_NextProjectInFile = nextpj;

	pj->pj_File = NULL;
	pj->pj_NextProjectInFile = NULL;
}


/***************** GetPrevProjectInFile() **************/
//
//  Find the prevous project connected to this file.
// If there is no previous project, then NULL is returned.

struct Project *GetPrevProjectInFile(struct Project *pj)
{
	struct Project *prevpj;
	struct File *fl;

	fl = pj->pj_File;
	prevpj = fl->fl_FirstProject;

	if (prevpj == pj)
		prevpj = NULL;
	else
	{
		while (prevpj->pj_NextProjectInFile != pj)
			prevpj = prevpj->pj_NextProjectInFile;
	}
	return(prevpj);
}

/**************** UnlinkProjectWindow() ****************/
//
// Detaches a superwindow from its parent project.
// (note that we assume that the window _is_ actually
// attached to a project in the first place).

void UnlinkProjectWindow(struct ProjectWindow *pw)
{
	struct ProjectWindow *prevwindow,*nextwindow;
	struct Project *pj;

	/*	unlink the superwindow from the projects window list */

	pj = pw->pw_Project;

	nextwindow = pw->pw_NextProjWindow;
	prevwindow = GetPrevProjectWindow(pw);

	if (prevwindow == NULL)
		pj->pj_ProjectWindows = nextwindow;
	else
		prevwindow->pw_NextProjWindow = nextwindow;

	pw->pw_Project = NULL;
	pw->pw_NextProjWindow = NULL;
}


/***************** GetPrevProjectWindow() **************/
//
//  Find the prevous window connected to this project.
// If there is no previous window, then NULL is returned.

struct ProjectWindow *GetPrevProjectWindow(struct ProjectWindow *pw)
{
	struct ProjectWindow *prevwindow;
	struct Project *proj;

	proj = pw->pw_Project;
	prevwindow = proj->pj_ProjectWindows;

	if (prevwindow == pw)
		prevwindow = NULL;
	else
	{
		while (prevwindow->pw_NextProjWindow != pw)
			prevwindow = prevwindow->pw_NextProjWindow;
	}
	return(prevwindow);
}

/***************** Count Project Windows **************/
//
// Return the number of Open Windows for this Project.
// Hidden windows are counted as closed.

int CountProjectWindows(struct Project *proj)
{
	int count = 0;
	struct ProjectWindow *pw;

	for (pw = proj->pj_ProjectWindows; pw; pw = pw->pw_NextProjWindow)
		if(pw->pw_sw.sw_Window) count++;
	return(count);
}

/***************** Dealloc ProjBitMaps ***************/
//
// Deallocate the Project BitMaps.
//
void DeallocProjBitMaps(struct Project *pj)
{
	if(pj->pj_MainRPort)
	{
		FreeBitMapRPort( pj->pj_MainRPort, pj->pj_Width, pj->pj_Height, pj->pj_Depth);
		pj->pj_MainRPort = NULL;
		pj->pj_MainBitMap = NULL;
	}
	if(pj->pj_UndoBitMap)
	{
		DestroyBitMap(pj->pj_UndoBitMap, pj->pj_Width, pj->pj_Height, pj->pj_Depth);
		pj->pj_UndoBitMap = NULL;
	}
	if(pj->pj_MaskBitMap)
	{
		DestroyBitMap(pj->pj_MaskBitMap, pj->pj_Width, pj->pj_Height, 1);
		pj->pj_MaskBitMap = NULL;
	}

}


/***************** SetUpDefaultBitMaps *****************/
//
//  Allocates a main & undo bitmap for the project. The size
// of these is the same as the current screen size.
//
BOOL SetUpDefaultBitMaps(struct Project *pj)
{
	DeallocProjBitMaps(pj);

	pj->pj_Width = mainscreen->Width;
	pj->pj_Height = mainscreen->Height;
	pj->pj_Depth = mainscreen->RastPort.BitMap->Depth;

	if(pj->pj_EditType == EDTY_BOBS && !glob.cg_SaveChunky )
	{
		if(pj->pj_Width > BOB_MAXWIDTH)
			pj->pj_Width = BOB_MAXWIDTH;
		if(pj->pj_Height > (BOB_MAXHEIGHT/pj->pj_Depth))
			pj->pj_Height = BOB_MAXHEIGHT/pj->pj_Depth;
	}
	pj->pj_Palette.pl_Count = 1<<pj->pj_Depth;

	if( pj->pj_MainRPort = AllocBitMapRPort(pj->pj_Width,pj->pj_Height,pj->pj_Depth) )
	{
		pj->pj_MainBitMap = pj->pj_MainRPort->BitMap;

		if( pj->pj_UndoBitMap = CreateBitMap(pj->pj_Width,pj->pj_Height,pj->pj_Depth,TRUE) )
		{
			if(pj->pj_EditType == EDTY_BOBS)
			{
				if( pj->pj_MaskBitMap = CreateBitMap(pj->pj_Width,pj->pj_Height,1,TRUE) )
					return(TRUE);
				else
					DeallocProjBitMaps(pj);
			}
			else
				return(TRUE);
		}
		else
			DeallocProjBitMaps(pj);
	}
	else
	{
		pj->pj_MainRPort = NULL;
		pj->pj_MainBitMap = NULL;
	}

	return(FALSE);
}

/***************** SetUpAlmostDefaultBitMaps *****************/
//
//  Allocates a main & undo bitmap for the project. The size
// of these is the same as the current screen size.
//
BOOL SetUpAlmostDefaultBitMaps(struct Project *pj, UWORD colours)
{
	DeallocProjBitMaps(pj);

	pj->pj_Width = mainscreen->Width;
	pj->pj_Height = mainscreen->Height;
	switch(colours)
	{
		case 2:
			pj->pj_Depth = 1;
			break;
		case 4:
			pj->pj_Depth = 2;
			break;
		case 8:
			pj->pj_Depth = 3;
			break;
		case 16:
			pj->pj_Depth = 4;
			break;
		case 32:
			pj->pj_Depth = 5;
			break;
		case 64:
			pj->pj_Depth = 6;
			break;
		case 128:
			pj->pj_Depth = 7;
			break;
		case 256:
			pj->pj_Depth = 8;
			break;
	}
	if(pj->pj_EditType == EDTY_BOBS && !glob.cg_SaveChunky )
	{
		if(pj->pj_Width > BOB_MAXWIDTH)
			pj->pj_Width = BOB_MAXWIDTH;
		if(pj->pj_Height > (BOB_MAXHEIGHT/pj->pj_Depth))
			pj->pj_Height = BOB_MAXHEIGHT/pj->pj_Depth;
	}
	pj->pj_Palette.pl_Count = colours;

	if( pj->pj_MainRPort = AllocBitMapRPort(pj->pj_Width,pj->pj_Height,pj->pj_Depth) )
	{
		pj->pj_MainBitMap = pj->pj_MainRPort->BitMap;

		if( pj->pj_UndoBitMap = CreateBitMap(pj->pj_Width,pj->pj_Height,pj->pj_Depth,TRUE) )
		{
			if(pj->pj_EditType == EDTY_BOBS)
			{
				if( pj->pj_MaskBitMap = CreateBitMap(pj->pj_Width,pj->pj_Height,1,TRUE) )
					return(TRUE);
				else
					DeallocProjBitMaps(pj);
			}
			else
				return(TRUE);
		}
		else
			DeallocProjBitMaps(pj);
	}
	else
	{
		pj->pj_MainRPort = NULL;
		pj->pj_MainBitMap = NULL;
	}

	return(FALSE);
}

/***************  RefreshAllProjectTitles() *************/
//
// Go though every ProjectWindow and update the window title
//

void RefreshAllProjectTitles(void)
{
	struct Project *pj;

	if(!IsListEmpty(&projects))
	{
		for(pj = (struct Project *)projects.lh_Head;
				pj->pj_Node.ln_Succ;
				pj = (struct Project *)pj->pj_Node.ln_Succ)
		{
			RefreshProjectName(pj);
		}
	}
}

/***************  RefreshProjectName()  ***************/
//
// Go through all the ProjectWindows attached to the specified
// Project and update their titles.
//

void RefreshProjectName(struct Project *pj )
{
	struct ProjectWindow *pw;
	char *oldtitle;

	for( pw = pj->pj_ProjectWindows; pw; pw = pw->pw_NextProjWindow)
	{
		/* update title in SuperWindow struct */
		pw->pw_sw.sw_Title = pj->pj_File ? pj->pj_File->fl_Name : "Untitled";
		/* is the window actually open (ie not hidden)? */
		if( pw->pw_sw.sw_Window )
		{
			/* free old title string */
			oldtitle = pw->pw_sw.sw_Window->Title;
			/* create and set up new title */
			SetWindowTitles( pw->pw_sw.sw_Window,
				pw->pw_sw.sw_FlgTitle ? BuildWindowTitle(&pw->pw_sw) : NULL,
				(UBYTE *)~0 );
			if(oldtitle)
				FreeVec(oldtitle);
		}
	}
}

/*********************   UseProjSettings()   *******************/
//
//  Setup the screen so that the project is displayed in the right
// palette, at the right depth, using the right screen mode (depending
// on the KeepScreenMode flag).
//

void UseProjSettings(struct Project *pj)
{
	struct Rectangle rect;
	struct ProjectWindow *pw;

	if( (ownscreen) && (pj) )
	{
		globpalette = &pj->pj_Palette;
		glob.cg_ScrConfig.sg_DisplayDepth = pj->pj_Depth;
		if(!glob.cg_KeepScreenMode)
		{
			glob.cg_ScrConfig.sg_DisplayID = pj->pj_DisplayID;

			if( QueryOverscan( pj->pj_DisplayID, &rect, glob.cg_ScrConfig.sg_OverscanType) )
			{
				glob.cg_ScrConfig.sg_DisplayWidth = rect.MaxX - rect.MinX + 1;
				glob.cg_ScrConfig.sg_DisplayHeight = rect.MaxY - rect.MinY + 1;
			}
		}
		if( !RemakeScreenIfNeeded(globpalette) )
		{
			InstallPalette(mainscreen,globpalette);
			RefreshAllProjectTitles();
			for(pw = pj->pj_ProjectWindows; pw; pw = pw->pw_NextProjWindow)
			{
				if(pw->pw_sw.sw_Window)
					RedisplayProjWindow(pw,TRUE);
			}
		}
	}
	RedisplayAllPaletteWindows();
}

void SussOutModificationStuff(struct Project *pj)
{
	if(!pj->pj_ModifiedFlag)
	{
		pj->pj_ModifiedFlag = TRUE;
		RefreshProjectName(pj);
	}
	if( (!pj->pj_CurrentBobModified) && ((pj->pj_EditType == EDTY_BOBS) || (pj->pj_EditType == EDTY_SPRITES)) )
	{
		pj->pj_CurrentBobModified = TRUE;
		if(!pj->pj_CurrentBob->bb_Node.ln_Succ->ln_Succ)
		{
			AllocNextBob(pj, &pj->pj_BobList);
		}
	}
}

/*****************    GetEmptyProject()    *******************/
//
//  Scans through the existing project bank searching for an empty
// project, if it finds one it returns it, but if it can't it will
// try to open a minimum project and return the result.

struct Project *GetEmptyProject(struct File *fl)
{
	struct Project *pj = NULL, *theproj = NULL;

	for(pj = (struct Project *)projects.lh_Head;
		 (pj->pj_Node.ln_Succ) && (!theproj);
			pj = (struct Project *)pj->pj_Node.ln_Succ)
	{
		if( (!pj->pj_IsFile) && (!pj->pj_ModifiedFlag) )
		{
			theproj = pj;
			if(theproj->pj_File)
				printf("Error - GetEmptyProject() - Project Has a File\n");
			LinkProjectToFile(fl,theproj);
		}
	}
	if(!theproj)
	{
		if( !(theproj = OpenMinProject(&glob.cg_WindowDefaults[SWTY_PROJECT],fl)) )
			GroovyReq("Open New Project","Can't Open New Project","Continue");
	}
	return(theproj);
}

/****************   SetChunkName()  ***************************/


BOOL SetChunkName(struct Project *pj)
{
	BOOL success = FALSE;
	char buffermem[16];
	static struct HoopyObject hob[] =
	{
		{ HOTYPE_VGROUP, 100, HOFLG_FREEWIDTH|HOFLG_FREEHEIGHT,3,0,0,0, NULL, 0,0,0,0,0 },
			{ HOTYPE_STRING, 100, HOFLG_NOBORDER, 0,16,0,0, NULL,0,0,0,0,0 },
			{ HOTYPE_SPACE, 10, HOFLG_NOBORDER|HOFLG_FREEWIDTH|HOFLG_FREEHEIGHT, 0,0,0,0, NULL,0,0,0,0,0 },
			{ HOTYPE_HGROUP, 100, HOFLG_NOBORDER|HOFLG_FREEWIDTH,3,0,0,0, NULL, 0,0,0,0,0 },
				{ HOTYPE_BUTTON, 100, 0, 0,0,0,0, "  OK  ",0,0x8000,0,0,0 },
				{ HOTYPE_SPACE, 100, HOFLG_NOBORDER|HOFLG_FREEWIDTH, 0,0,0,0, NULL,0,0,0,0,0 },
				{ HOTYPE_BUTTON, 100, 0, 0,0,0,0, "Cancel",0,0xFFFF,0,0,0 },
		{HOTYPE_END}
	};

	hob[1].ho_Attr0 = (ULONG)buffermem;

	if(pj && BlockAllSuperWindows())
	{
		strcpy(buffermem, pj->pj_BankName);

		if(	HO_SimpleDoReq( hob, "Name Chunk", NULL) != 0xFFFF)
		{
			strcpy(pj->pj_BankName, buffermem);
			MakePackable(pj->pj_BankName);
			RefreshProjectName(pj);
			success = TRUE;
		}
		UnblockAllSuperWindows();
	}
	return(success);
}
