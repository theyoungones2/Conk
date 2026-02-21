/****************************************************************************/
//
// BONK: Requesters.c
//
/****************************************************************************/

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
//#include <proto/graphics.h>
//#include <proto/diskfont.h>
//#include <proto/asl.h>
//#include <clib/alib_protos.h>
//#include <proto/gadtools.h>

#include <global.h>
#include <ExternVars.h>

/***************** ModifiedProjectRequester() **************/
//
//  Ask for the users response about closing a modified
// project. (NOTE. it is assumed that the project has been
// modified.)
//  Will return the response directly from the requester,
// ie:  Save = 1 ;  Abandon = 2 ;  Cancel = 0

int	ModifiedProjectRequester(struct Project *proj)
{
//	printf("ModifiedProjectRequester()\n");
	return( GroovyReq( PROGNAME, "Project '%s'\nhas been modified",
		"Save|Abandon|Cancel", proj->pj_Node.ln_Name ) );
}


/***************** QuitProgramRequester **************/
//
//  Ask for the users response about Quiting the program.

int	QuitProgramRequester()
{
	return(GroovyReq( PROGNAME, "Do You Want to Quit ?",
		"Yes, quit|No don't") );
}

/***************** ScreenLockedReq **************/
//
//  Print up that we can't close the screen cos there
//  are x number of visitors on it.
//
int ScreenLockedReq(LONG locks)
{
		return(GroovyReq( PROGNAME,
			"Can't Close Screen\n%ld Visitor(s) On Screen",
			"Retry|Abort",
			locks) );
}


/***************** GroovyReq **************/
//
//  Open an 'EasyRequest'er on any window that we can,
//  and return the result.
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
