/****************************************************************************/
//
// BONK: Projects.c
//
/****************************************************************************/


#include <string.h>
#include <dos/dos.h>
//#include <dos/dostags.h>
#include <exec/exec.h>
#include <intuition/intuition.h>
//#include <graphics/gfxbase.h>
//#include <rexx/rxslib.h>
//#include <libraries/gadtools.h>
//#include <libraries/diskfont.h>
//#include <libraries/asl.h>
//#include <libraries/iffparse.h>

#include <proto/exec.h>
#include <proto/dos.h>
#include <proto/intuition.h>
//#include <proto/graphics.h>
//#include <proto/diskfont.h>
//#include <proto/asl.h>
//#include <clib/alib_protos.h>
//#include <proto/gadtools.h>
//#include <proto/iffparse.h>



#include <Global.h>
#include <ExternVars.h>





/**************** InitProject() ****************/

void InitProject( struct Project *proj )
{
	UWORD i;
	proj->pj_Node.ln_Name = NULL;
	proj->pj_Drawer = NULL;
	proj->pj_ProjectWindows = NULL;
	proj->pj_PreviewWindows = NULL;
	proj->pj_BlkAnimWindows = NULL;
	proj->pj_NumOfBlkAnims = 0;
	NewList( &proj->pj_BlkAnims );

	proj->pj_ModifiedFlag = FALSE;
	proj->pj_UntitledFlag = TRUE;
	proj->pj_SavePaletteFlag = TRUE;
	proj->pj_SaveBlockSetFlag = TRUE;

	proj->pj_MapW = 0;
	proj->pj_MapH = 0;
	proj->pj_Map = NULL;
	proj->pj_BackLockFlag = FALSE;
	proj->pj_BackMap = NULL;

	/* clear bookmarks */
	for( i=0; i<10; i++ )
	{
		proj->pj_BookMarkX[i] = 0xFFFF;
		proj->pj_BookMarkY[i] = 0xFFFF;
	}

	/* null the bitmap stuff */
//	proj->pj_PreviewBM = NULL;
//	proj->pj_PreviewBMDepth = 0;
}



/**************** OpenNewProject() ****************/
//
// Creates a new, untitled Project with a clear map of the given size
// and opens a ProjectWindow on it.
//
// Return value: ptr to new Project struct
//  FALSE = couldn't create project
//

struct Project *OpenNewProject(UWORD width, UWORD height, STRPTR name)
{
	struct Project *proj;
	UWORD *p, i;

	if( !(proj = AllocVec(sizeof(struct Project), MEMF_CLEAR|MEMF_ANY)) ) return(NULL);

	InitProject( proj );
	proj->pj_Node.ln_Name = MyStrDup( "Untitled" );
//	proj->pj_Drawer	= NULL;
//	proj->pj_ModifiedFlag = FALSE;
	proj->pj_UntitledFlag = TRUE;			/* no title yet! */
//	proj->pj_ProjectWindows = NULL;
//	proj->pj_PreviewWindows = NULL;
//	proj->pj_BlkAnimWindows = NULL;
//	NewList( &proj->pj_BlkAnims );
//	/* clear bookmarks */
//	for( i=0; i<10; i++ )
//	{
//		proj->pj_BookMarkX[i] = 0xFFFF;
//		proj->pj_BookMarkY[i] = 0xFFFF;
//	}
//	LoadRawMap( proj, "ck:TestMap2", 128, 64 );

	if( p = (UWORD *)AllocVec( width * height * sizeof(UWORD),
		MEMF_ANY | MEMF_CLEAR ) )
	{
		proj->pj_Map = p;
		proj->pj_MapW = width;
		proj->pj_MapH = height;
		Mystrncpy( proj->pj_MapName, name, MAPNAMESIZE-1 );

		AddHead(&projects,&proj->pj_Node);

		if( !OpenNewProjWindow(proj) )
		{
			Remove(&proj->pj_Node);
			FreeVec(proj->pj_Map);
			FreeVec(proj);
			return(NULL);
		}
		return(proj);
	}
	else
	{
		FreeVec(proj);
		return(NULL);
	}
}


/****************  GUIOpenNewProject()  ****************/
//
// Brings up a requester to let the user select a map size,
// then calls OpenNewProject() with that size.
//

struct Project *GUIOpenNewProject( void )
{
	struct Project *proj = NULL;
	UBYTE namebuf[MAPNAMESIZE];

	static struct ReqCrap rc[] =
	{
		{ "Name:", HOOPYREQTYPE_STRING, NULL, MAPNAMESIZE-1, NULL },
		{ "Width:", HOOPYREQTYPE_INT, NULL, 128, NULL },
		{ "Height:", HOOPYREQTYPE_INT, NULL, 16, NULL },
	};

	strcpy( namebuf, "-<UNTITLED>-" );
	rc[0].rc_Data = namebuf;

	if( DoHoopyReq( "New Project", rc, 3 ) )
	{
		MyStrToUpper( namebuf );
		proj = OpenNewProject( rc[1].rc_Value, rc[2].rc_Value, namebuf );
	}

	return(proj);
}





/**************** CloseProject() ****************/
//
// Closes a project by going through and killing off all the
// superwindows attached to it. Note that the project is actually
// killed by RemoveProject(), which is called automagically by the
// superwindow shutdown.
//

void CloseProject(struct Project *proj)
{
	struct ProjectWindow *pw, *nextpw;

//	for (pw = proj->pj_ProjectWindows; pw && pw->pw_NextProjWindow; pw = proj->pj_ProjectWindows)
//		(*pw->pw_sw.sw_BuggerOff)(&pw->pw_sw);

	pw = proj->pj_ProjectWindows;
	do
	{
		nextpw = pw->pw_NextProjWindow;
		(*pw->pw_sw.sw_BuggerOff)(&pw->pw_sw);
		pw = nextpw;
	}
	while( nextpw );
}

/**************** RemoveProject() ****************/
//
// Frees up any resources used by a project and removes it
// from the projects list.
//

void RemoveProject(struct Project *proj)
{

	struct PreviewWindow *vw;
	struct BlkAnimWindow *baw;

	/* kill all the preview windows */
	for( vw = proj->pj_PreviewWindows; vw; vw = vw->vw_NextPreviewWindow )
		(*vw->vw_sw.sw_BuggerOff)(&vw->vw_sw);

	/* kill all BlkAnimWindows */
	for( baw = proj->pj_BlkAnimWindows; baw; baw = baw->baw_NextBlkAnimWindow )
		(*baw->baw_sw.sw_BuggerOff)(&baw->baw_sw);

	/* free data... */
/*
	if( proj->pj_Map )
		FreeMap( proj );
	if( proj->pj_Node.ln_Name )
		FreeVec( proj->pj_Node.ln_Name );
	if( proj->pj_Drawer )
		FreeVec( proj->pj_Drawer );
*/
	Remove(&proj->pj_Node);
//	FreeVec(proj);
	FreeProject( proj );
}


/**************** AllocProject() ****************/

struct Project *AllocProject( void )
{
	return( (struct Project *)AllocVec( sizeof(struct Project),
		MEMF_ANY|MEMF_CLEAR ) );
}


/**************** FreeProject() ****************/
//
// Frees a project structure, and any data attached to it.
//

void FreeProject( struct Project *proj )
{
	struct BlkAnim *ba;

	if( proj )
	{
		if( proj->pj_Map ) FreeVec( proj->pj_Map );
		if( proj->pj_BackMap ) FreeVec( proj->pj_BackMap );
		if( proj->pj_Drawer ) FreeVec( proj->pj_Drawer );
		if( proj->pj_Node.ln_Name ) FreeVec( proj->pj_Node.ln_Name );

		while( !IsListEmpty( &proj->pj_BlkAnims ) )
		{
			ba = (struct BlkAnim *)proj->pj_BlkAnims.lh_Head;
			Remove( (struct Node *)ba );
			FreeVec( ba );
		}

		FreeVec( proj );
	}
}


/**************** UnlinkProjectWindow() ****************/
//
// Detaches a superwindow from its parent project.
// (note that we assume that the window _is_ actually
// attached to a project in the first place).

void UnlinkProjectWindow(struct ProjectWindow *pw)
{
	struct ProjectWindow *prevwindow,*nextwindow;
	struct Project *proj;

	/*	unlink the superwindow from the projects window list */

	proj = pw->pw_Project;

	nextwindow = pw->pw_NextProjWindow;
	prevwindow = GetPrevProjectWindow(pw);

	if (prevwindow == NULL)
		proj->pj_ProjectWindows = nextwindow;
	else
		prevwindow->pw_NextProjWindow = nextwindow;
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



/***************  RefreshProjectName()  ***************/
//
// Go through all the ProjectWindows attached to the specified
// Project and update their titles.
//

void RefreshProjectName(struct Project *proj )
{
	struct ProjectWindow *pw;
	char *oldtitle;

	for( pw = proj->pj_ProjectWindows; pw; pw = pw->pw_NextProjWindow)
	{
		/* update title in SuperWindow struct */
		pw->pw_sw.sw_Title = proj->pj_Node.ln_Name;
		/* is the window actually open (ie not hidden)? */
		if( pw->pw_sw.sw_Window )
		{
			oldtitle = pw->pw_sw.sw_Window->Title;
			/* create and set up new title */
			SetWindowTitles( pw->pw_sw.sw_Window,
				pw->pw_sw.sw_FlgTitle ? BuildWindowTitle(&pw->pw_sw) : NULL,
				(char *)-1 );
			/* free old title string */
			if(oldtitle)
				FreeVec(oldtitle);
		}
	}
}



/***************  ResizeProject()  ***************/
//
// Changes the size of a map. If the new size is smaller, the right
// and/or bottom edges of the map will be clipped. If the new size
// is bigger, the extra space is padded out with block zero.
// If we can't alloc enough space for the new map, the old map will
// be left unchanged and this routine will return FALSE.

BOOL ResizeProject( struct Project *proj, UWORD neww, UWORD newh )
{
	UWORD *newdat, *dest;
	UWORD x, y, i;
	BOOL success = FALSE;

	if( newdat = AllocVec( neww * newh * sizeof(UWORD), MEMF_ANY ) )
	{
		/* is background lock on? if yep, unlock it. */
		if( proj->pj_BackLockFlag )
			UnlockBackground( proj );
		/* free background lock buffer if it exists, 'cos it's size */
		/* is going to be potentionally poked. */
		if( proj->pj_BackMap )
		{
			FreeVec( proj->pj_BackMap );	/* zap. */
			proj->pj_BackMap = NULL;
		}

		dest = newdat;
		for( y=0; y<newh; y++ )
		{
			for( x=0; x<neww; x++ )
			{
				if( x < proj->pj_MapW && y < proj->pj_MapH )
				{
					/* still within bounds of old map */
					*dest++ = *( proj->pj_Map + ( proj->pj_MapW * y) + x );
				}
				else
				{
					/* off old map - just put block zero */
					*dest++ = 0;
				}
			}
		}
		FreeVec( proj->pj_Map );
		proj->pj_Map = newdat;
		proj->pj_MapW = neww;
		proj->pj_MapH = newh;
		for( i=0; i<10; i++ )
		{
			if( proj->pj_BookMarkX[i] >= neww || proj->pj_BookMarkY[i] >= newh )
			{
				proj->pj_BookMarkX[i] = 0;
				proj->pj_BookMarkY[i] = 0;
			}
		}

		success = TRUE;
	}
	return( success );
}


/***************  GUIResizeProject()  ***************/
//
// Graphical frontend for ResizeProject() - brings up a new size
// requester and stuff like that.
//

void GUIResizeProject( struct Project *proj )
{
	static struct ReqCrap rc[] =
	{
		{ "Enter new map dimensions", HOOPYREQTYPE_TEXT, NULL, 0, NULL },
		{ "Width:", HOOPYREQTYPE_INT, NULL, 0, NULL },
		{ "Height:", HOOPYREQTYPE_INT, NULL, 0, NULL },
	};

	rc[1].rc_Value = proj->pj_MapW;
	rc[2].rc_Value = proj->pj_MapH;

	if( DoHoopyReq( "Resize Map", rc, 3 ) )
		ResizeProject( proj, rc[1].rc_Value, rc[2].rc_Value );
}


/***************  RedrawAllProjects() ***************/
//
//
//
//

void RedrawAllProjects( void )
{
	struct Project *proj;
	struct ProjectWindow *pw;
	struct PreviewWindow *vw;

	for(proj = (struct Project *)projects.lh_Head;
		proj->pj_Node.ln_Succ;
		proj = (struct Project *)proj->pj_Node.ln_Succ )
	{
		for( pw = proj->pj_ProjectWindows;
			pw;
			pw = pw->pw_NextProjWindow )
		{
			DrawMap( pw );
			UpdatePWScrollers( pw );
		}

		for( vw = proj->pj_PreviewWindows;
			vw;
			vw = vw->vw_NextPreviewWindow )
		{
			DrawMapPreview( vw );
		}

		/* do BlkAnimWindows here! */

	}
}



/*****************  LockBackground()  *****************/
//
// Makes a copy of the current map buffer, and sets the background
// lock flag in the project struct (When the user erases stuff in
// lock mode, blocks are copied back from coresponding positions
// in this secondary buffer). This routine also updates window
// titles to indicate background lock mode is on.

BOOL LockBackground( struct Project *proj )
{
	BOOL success = FALSE;

	if( !proj->pj_BackMap )
	{
		proj->pj_BackMap =
			AllocVec( proj->pj_MapW * proj->pj_MapH * sizeof( UWORD ), MEMF_ANY );
	}
	/* if pj_BackMap set, assume already enough space */

	if( proj->pj_BackMap )
	{
		/* copy the mapdata to the BackMap buffer */
		CopyMem( proj->pj_Map, proj->pj_BackMap,
			proj->pj_MapW * proj->pj_MapH * sizeof( UWORD ) );
		success = TRUE;
		proj->pj_BackLockFlag = TRUE;
		RefreshProjectName( proj );
	}

	return( success );
}


/*****************  UnlockBackground()  *****************/
//
// Unlocks the background for the given project, and alters
// the indictator in window title appropriately.
// The background buffer area is not freed however, because
// we may want to use it again soon, and it really takes bugger
// all memory anyway (the machine I'm using at the moment has 18
// megs. How 'bout you? ).
//
// Wow, this description is longer than the function itself!
// grooooooovy!

void UnlockBackground( struct Project *proj )
{
	proj->pj_BackLockFlag = FALSE;
	RefreshProjectName( proj );
}


/*****************  SetBookMark()  *****************/

void SetBookMark( struct ProjectWindow *pw, UWORD num )
{
	pw->pw_Project->pj_BookMarkX[num] = pw->pw_MapX;
	pw->pw_Project->pj_BookMarkY[num] = pw->pw_MapY;
}


/*****************  GotoBookMark()  *****************/

void GotoBookMark( struct ProjectWindow *pw, UWORD num )
{
	if( pw->pw_Project->pj_BookMarkX[num] != 0xFFFF &&
		pw->pw_Project->pj_BookMarkY[num] != 0xFFFF )
	{
		pw->pw_MapX = pw->pw_Project->pj_BookMarkX[num];
		pw->pw_MapY = pw->pw_Project->pj_BookMarkY[num];
		DrawMap( pw );
		UpdatePWScrollers( pw );
	}
	else
		DisplayBeep( mainscreen );
}

/********************************/
void RenameMap( struct Project *proj )
{
	static struct ReqCrap req[] = {
		{ "Name:", HOOPYREQTYPE_STRING, NULL, MAPNAMESIZE-1, NULL }
	};
	UBYTE namebuf[ MAPNAMESIZE ];

	Mystrncpy( namebuf, proj->pj_MapName, MAPNAMESIZE-1 );
	req[0].rc_Data = namebuf;

	if( DoHoopyReq( "Rename Map", req, 1 ) )
	{
		MyStrToUpper( namebuf );
		Mystrncpy( proj->pj_MapName, namebuf, MAPNAMESIZE-1 );
		RefreshProjectName( proj );
	}
}
