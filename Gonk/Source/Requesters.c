/*
		Stuff for handling most requesters
 */

#include <stdio.h>
#include <stdarg.h>
//#include <string.h>
//#include <dos/dos.h>
//#include <dos/dostags.h>
//#include <exec/exec.h>
#include <intuition/intuition.h>
//#include <graphics/gfxbase.h>
//#include <rexx/rxslib.h>
//#include <libraries/gadtools.h>
//#include <libraries/diskfont.h>
//#include <libraries/asl.h>

//#include <proto/exec.h>
//#include <proto/dos.h>
#include <proto/intuition.h>
#include <proto/graphics.h>
//#include <proto/diskfont.h>
//#include <proto/asl.h>
//#include <clib/alib_protos.h>
#include <proto/gadtools.h>

#include <global.h>
#include <ExternVars.h>
#include <HoopyVision.h>

/***************** ModifiedProjectRequester() **************/
//
//  Ask for the users response about closing a modified
// project. (NOTE. it is assumed that the project has been
// modified.)
//  Will return the response directly from the requester,
// ie:  Save = 1 ;  Abandon = 2 ;  Cancel = 0

int	ModifiedProjectRequester(struct Project *pj)
{
	int response		= 0;

	response = GroovyReq(Txt_ConstructionKit,Txt_ModifiedProject,Txt_SaveAbandonCancel,pj->pj_File ? pj->pj_File->fl_Name : "Untitled");

	if (response == 1)
	{
		PM_SaveFileAs(pj->pj_ProjectWindows);
	}
	return(response);
}

/***************** ModifiedSequenceRequester() **************/
//
//  Ask for the users response about closing a modified
// project. (NOTE. it is assumed that the project has been
// modified.)
//  Will return the response directly from the requester,
// ie:  Save = 1 ;  Abandon = 2 ;  Cancel = 0

int	ModifiedSequenceRequester(struct Project *pj)
{
	int response		= 0;

	response = GroovyReq("Quit Project","Project '%s'\nHas A Modified\nAnim Sequence","Save|Abandon|Cancel",pj->pj_File ? pj->pj_File->fl_Name : "Untitled");

	if (response == 1)
	{
		PM_SaveFileAs(pj->pj_ProjectWindows);
	}
	return(response);
}


/***************** CheckPalettes() **************/
//
//  Will check that the projects palette is the same as the
// currently visible palette. If it the same, then that's fine. If
// the palettes are different, then a requester appears asking if you
// want to save the project with 1) original palette
//															 2) current palette but keep the old one
//															 3) current palette and replace the old one.
//															 4) Cancel
//  Will return the response directly from the requester,
// ie:  original = 1 ;  current&keep = 2 ; current&replace = 3 ; Cancel = 0
// Will also return 1 if the palettes are the same.

int	CheckPalettes(struct Project *pj)
{
	int response		= 1;
	UWORD col,numofcols;
	BOOL same = TRUE;

	if(globpalette != &pj->pj_Palette)
	{
		numofcols = 1<<pj->pj_Depth;
		for(col = 0; (col < numofcols) && (same == TRUE); col++)
		{
			if( (globpalette->pl_Colours[col].r != pj->pj_Palette.pl_Colours[col].r) ||
					(globpalette->pl_Colours[col].g != pj->pj_Palette.pl_Colours[col].g) ||
					(globpalette->pl_Colours[col].b != pj->pj_Palette.pl_Colours[col].b) )
				same = FALSE;
		}
		if(!same)
		{
			response = GroovyReq("Palette Check","Project '%s'\nHas A Different Palette To\nThat Currently Viewed\nSave Which Palette:\n1) Original\n2) Current & Keep Original\n3) Current & Replace Original","1|2|3|Cancel",pj->pj_File ? pj->pj_File->fl_Name : "Untitled");
		}
	}
	return(response);
}

/***************** QuitProgramRequester **************/
//
//  Ask for the users response about Quiting the program.

int	QuitProgramRequester()
{
	return(GroovyReq(Txt_ConstructionKit,Txt_DoYouWantToQuit,Txt_QuitDontQuit) );
}

/***************** ScreenLockedReq **************/
//
//  Print up that we can't close the screen cos there
//  are x number of visitors on it.
//
int ScreenLockedReq(LONG locks)
{
		return(GroovyReq(Txt_EditorError,Txt_CantCloseScreenVisitorsOnScreen,Txt_RetryAbort,locks) );
}

/***************** InvalidNameRequester() **********************/
//
//  If a file name is invalid (as given by MakeFullPathName(), then
// bring up this requester. This is a very small routine, but I imagine
// that I'll use it quite a bit, so just incase I want to change the
// wording later I'll leave it in a routine.

WORD InvalidNameRequester(void)
{
	return( GroovyReq("File Error","Invalid Name","Continue") );
}

/***************** GroovyReq **************/
//
//  Open an 'EasyRequest'er on any window that we can,
//  and return the reult.
//
WORD __stdargs GroovyReq(STRPTR title,STRPTR text,STRPTR gadgets,...)
{
	struct EasyStruct	easy;
	struct SuperWindow *sw;
	struct Window *win;
	WORD				response = FALSE;
	va_list			varargs;

	if (sw = GetOpenSuperWindow())
		win=sw->sw_Window;
	else
		win = OpenKludgeWindow();

	easy.es_StructSize	= sizeof(struct EasyStruct);
	easy.es_Flags		= NULL;
	easy.es_Title		= title;
	easy.es_TextFormat	= text;
	easy.es_GadgetFormat	= gadgets;

	va_start(varargs,gadgets);
	response = EasyRequestArgs(win,&easy,NULL,varargs);
	va_end(varargs);

	if( (!sw) && (win) ) CloseKludgeWindow(win);

	return(response);
}



/***************** OpenKludgeWindow **************/
//
// Open up a very small,borderless,backdrop window.
// This is used if there are no other windows on screen.

struct Window *OpenKludgeWindow()
{
	return(OpenWindowTags(NULL,
											WA_Left,				0,
											WA_Top,					50,
											WA_Width,				2,
											WA_Height,			2,
											WA_Flags,				WFLG_BORDERLESS|WFLG_BACKDROP,
											WA_CustomScreen,MAINSCREEN,
											WA_AutoAdjust,	TRUE,
											TAG_DONE,				NULL) );
}

/***************** CloseKludgeWindow **************/
//
// I think you can work out what it does all by yourself.

void CloseKludgeWindow(struct Window *win)
{
	CloseWindow(win);
}

void MissingILBMChunkReq(char *name,char *chunk)
{
	GroovyReq(Txt_EditorError,Txt_FileMissingChunk,Txt_Abort,name,chunk);
}
