/*
 * Source generated with ARexxBox 1.12 (May 18 1993)
 * which is Copyright (c) 1992,1993 Michael Balzer
 */

#include <exec/types.h>
#include <exec/memory.h>
#include <dos/dos.h>
#include <rexx/storage.h>
#include <rexx/rxslib.h>

#ifdef __GNUC__
/* GCC needs all struct defs */
#include <dos/exall.h>
#include <graphics/graphint.h>
#include <intuition/classes.h>
#include <devices/keymap.h>
#include <exec/semaphores.h>
#endif

#include <clib/alib_protos.h>
#include <clib/exec_protos.h>
#include <clib/dos_protos.h>
#include <clib/rexxsyslib_protos.h>

#ifndef __NO_PRAGMAS

#ifdef AZTEC_C
#include <pragmas/exec_lib.h>
#include <pragmas/dos_lib.h>
#include <pragmas/rexxsyslib_lib.h>
#endif

#ifdef LATTICE
#include <pragmas/exec_pragmas.h>
#include <pragmas/dos_pragmas.h>
#include <pragmas/rexxsyslib_pragmas.h>
#endif

#endif /* __NO_PRAGMAS */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#ifdef LATTICE
#undef toupper
#define inline __inline
#endif

#ifdef __GNUC__
#undef toupper
static inline char toupper( char c )
{
	return( islower(c) ? c - 'a' + 'A' : c );
}
#endif

#ifdef AZTEC_C
#define inline
#endif

#include "RexxMain.h"


//extern struct ExecBase *SysBase;
//extern struct DosLibrary *DOSBase;
//extern struct RxsLib *RexxSysBase;


/* $ARB: I 727904511 */


/* $ARB: B 1 CLEAR */
#include <global.h>
#include <protos.h>
#include <externvars.h>
#include <intuition/intuition.h>
#include <intuition/intuitionbase.h>
#include <proto/intuition.h>

void rx_clear( struct RexxHost *host, struct rxd_clear **rxd, long action, struct RexxMsg *rexxmsg )
{
	struct rxd_clear *rd = *rxd;

	switch( action )
	{
		case RXIF_INIT:
			*rxd = AllocVec( sizeof *rd, MEMF_CLEAR );
			if( rd = *rxd )
			{
				/* set your DEFAULTS here */
			}
			break;
			
		case RXIF_ACTION:
			/* Insert your CODE here */
			rd->rc = 0;
			break;
		
		case RXIF_FREE:
			/* FREE your local data here */
			FreeVec( rd );
			break;
	}
	return;
}
/* $ARB: E 1 CLEAR */

/* $ARB: B 2 CLOSE */
void rx_close( struct RexxHost *host, struct rxd_close **rxd, long action, struct RexxMsg *rexxmsg )
{
	struct rxd_close *rd = *rxd;
	struct Project *proj;
	struct SuperWindow *sw;

	switch( action )
	{
		case RXIF_INIT:
			*rxd = AllocVec( sizeof *rd, MEMF_CLEAR );
			if( rd = *rxd )
			{
				/* set your DEFAULTS here */
			}
			break;
			
		case RXIF_ACTION:
			/* Insert your CODE here */

			if( sw = (struct SuperWindow *)FindName(&superwindows,currentrexxwindow) )
			{
				switch(sw->sw_Type)
				{
					case SWTY_PROJECT:
						proj = ( (struct ProjectWindow *) sw)->pw_Project;
						if( (proj->pj_ModifiedFlag) && (!rd->arg.force) )
						{
							if (ModifiedProjectRequester(proj))
								(*sw->sw_BuggerOff)(sw);
						}
						else
							(*sw->sw_BuggerOff)(sw);
						break;
					default:
						(*sw->sw_BuggerOff)(sw);
						break;
				}
				SussOutQuiting(rd->arg.force);
				rd->rc = 0;
			}
			else
			{
				rd->rc = -5;
				rd->rc2 = (LONG)Txt_SelectedWindowDoesntExist;
			}

			break;
		
		case RXIF_FREE:
			/* FREE your local data here */
			FreeVec( rd );
			break;
	}
	return;
}
/* $ARB: E 2 CLOSE */

/* $ARB: B 3 NEW */
void rx_new( struct RexxHost *host, struct rxd_new **rxd, long action, struct RexxMsg *rexxmsg )
{
	struct rxd_new *rd = *rxd;
	struct Project *proj;

	switch( action )
	{
		case RXIF_INIT:
			*rxd = AllocVec( sizeof *rd, MEMF_CLEAR );
			break;
			
		case RXIF_ACTION:
			/* Insert your CODE here */
			proj = OpenNewProject(&glob.cg_WindowDefaults[SWTY_PROJECT],NULL);
			if(proj)
			{
				rd->rc = 0;
				rd->res.windowid = proj->pj_ProjectWindows->pw_sw.sw_idstring;
			}
			else
			{
				rd->rc = -5;
				rd->rc2 = (LONG)Txt_CantOpenProject;
				rd->res.windowid = NULL;
			}
			break;
		
		case RXIF_FREE:
			/* FREE your local data here */
			FreeVec( rd );
			break;
	}
	return;
}
/* $ARB: E 3 NEW */

/* $ARB: B 4 OPEN */
void rx_open( struct RexxHost *host, struct rxd_open **rxd, long action, struct RexxMsg *rexxmsg )
{
	struct rxd_open *rd = *rxd;

	switch( action )
	{
		case RXIF_INIT:
			*rxd = AllocVec( sizeof *rd, MEMF_CLEAR );
			if( rd = *rxd )
			{
				/* set your DEFAULTS here */
			}
			break;
			
		case RXIF_ACTION:
			/* Insert your CODE here */
			rd->rc = 0;
			break;
		
		case RXIF_FREE:
			/* FREE your local data here */
			FreeVec( rd );
			break;
	}
	return;
}
/* $ARB: E 4 OPEN */

/* $ARB: B 6 QUIT */
void rx_quit( struct RexxHost *host, struct rxd_quit **rxd, long action, struct RexxMsg *rexxmsg )
{
	struct rxd_quit *rd = *rxd;
	struct Project *proj;
	struct SuperWindow *sw;
	int abort = FALSE;

	switch( action )
	{
		case RXIF_INIT:
			*rxd = AllocVec( sizeof *rd, MEMF_CLEAR );
			if( rd = *rxd )
			{
				/* set your DEFAULTS here */
			}
			break;
			
		case RXIF_ACTION:
			/* Insert your CODE here */

			while(!IsListEmpty(&projects) && !abort)
			{
				proj = (struct Project *)projects.lh_Head;
				if ( (proj->pj_ModifiedFlag) && (!rd->arg.force) )
				{
					if (ModifiedProjectRequester(proj))
						CloseProject(proj);
					else
						abort = TRUE;
				}
				else
					CloseProject(proj);
			}
			while(!IsListEmpty(&superwindows) && !abort)
			{
				sw = (struct SuperWindow *)superwindows.lh_Head;
				(*sw->sw_BuggerOff)(sw);
			}

			SussOutQuiting(rd->arg.force);
			rd->rc = 0;
			break;
		
		case RXIF_FREE:
			/* FREE your local data here */
			FreeVec( rd );
			break;
	}
	return;
}
/* $ARB: E 6 QUIT */

/* $ARB: B 7 SAVE */
void rx_save( struct RexxHost *host, struct rxd_save **rxd, long action, struct RexxMsg *rexxmsg )
{
	struct rxd_save *rd = *rxd;

	switch( action )
	{
		case RXIF_INIT:
			*rxd = AllocVec( sizeof *rd, MEMF_CLEAR );
			break;
			
		case RXIF_ACTION:
			/* Insert your CODE here */
			rd->rc = 0;
			break;
		
		case RXIF_FREE:
			/* FREE your local data here */
			FreeVec( rd );
			break;
	}
	return;
}
/* $ARB: E 7 SAVE */

/* $ARB: B 8 SAVEAS */
void rx_saveas( struct RexxHost *host, struct rxd_saveas **rxd, long action, struct RexxMsg *rexxmsg )
{
	struct rxd_saveas *rd = *rxd;

	switch( action )
	{
		case RXIF_INIT:
			*rxd = AllocVec( sizeof *rd, MEMF_CLEAR );
			if( rd = *rxd )
			{
				/* set your DEFAULTS here */
			}
			break;
			
		case RXIF_ACTION:
			/* Insert your CODE here */
			rd->rc = 0;
			break;
		
		case RXIF_FREE:
			/* FREE your local data here */
			FreeVec( rd );
			break;
	}
	return;
}
/* $ARB: E 8 SAVEAS */

/* $ARB: B 9 ACTIVATEWINDOW */
void rx_activatewindow( struct RexxHost *host, struct rxd_activatewindow **rxd, long action, struct RexxMsg *rexxmsg )
{
	struct rxd_activatewindow *rd = *rxd;
	struct SuperWindow *sw;

	switch( action )
	{
		case RXIF_INIT:
			*rxd = AllocVec( sizeof *rd, MEMF_CLEAR );
			break;
			
		case RXIF_ACTION:
			/* Insert your CODE here */
			if( sw = (struct SuperWindow *)FindName(&superwindows,currentrexxwindow) )
			{
				if(sw->sw_Window)
				{
					ActivateWindow(sw->sw_Window);
					rd->rc = 0;
				}
				else
				{
					rd->rc = -5;
					rd->rc2 = (LONG)Txt_CantActivateAHiddenWindow;
				}
			}
			else
			{
				rd->rc = -5;
				rd->rc2 = (LONG)Txt_SelectedWindowDoesntExist;
			}
			break;
		
		case RXIF_FREE:
			/* FREE your local data here */
			FreeVec( rd );
			break;
	}
	return;
}
/* $ARB: E 9 ACTIVATEWINDOW */

/* $ARB: B 10 CHANGEWINDOW */
void rx_changewindow( struct RexxHost *host, struct rxd_changewindow **rxd, long action, struct RexxMsg *rexxmsg )
{
	struct rxd_changewindow *rd = *rxd;
	struct SuperWindow *sw;
	UWORD	width,height;

	switch( action )
	{
		case RXIF_INIT:
			*rxd = AllocVec( sizeof *rd, MEMF_CLEAR );
			if( rd = *rxd )
			{
				/* set your DEFAULTS here */
			}
			break;
			
		case RXIF_ACTION:
			/* Insert your CODE here */
			if( sw = (struct SuperWindow *)FindName(&superwindows,currentrexxwindow) )
			{
				width = *rd->arg.width;
				height = *rd->arg.height;

				if(width < sw->sw_MinWidth)	width = sw->sw_MinWidth;
				if(width > sw->sw_MaxWidth)	width = sw->sw_MaxWidth;

				if(height < sw->sw_MinHeight)	height = sw->sw_MinHeight;
				if(height > sw->sw_MaxHeight)	height = sw->sw_MaxHeight;

				if(sw->sw_Window)
				{
					ChangeWindowBox( sw->sw_Window, *rd->arg.leftedge,*rd->arg.topedge,width,height);
					WaitForWindowMove();
				}
				else
				{
					sw->sw_Left = (WORD)*rd->arg.leftedge;
					sw->sw_Top = (WORD)*rd->arg.topedge;
					sw->sw_Width = (WORD)width;
					sw->sw_Height = (WORD)height;
				}
				rd->rc = 0;
			}
			else
			{
				rd->rc = -5;
				rd->rc2 = (LONG)Txt_SelectedWindowDoesntExist;
			}
			break;
		
		case RXIF_FREE:
			/* FREE your local data here */
			FreeVec( rd );
			break;
	}
	return;
}
/* $ARB: E 10 CHANGEWINDOW */

/* $ARB: B 11 MOVEWINDOW */
void rx_movewindow( struct RexxHost *host, struct rxd_movewindow **rxd, long action, struct RexxMsg *rexxmsg )
{
	struct rxd_movewindow *rd = *rxd;
	struct SuperWindow *sw;

	switch( action )
	{
		case RXIF_INIT:
			*rxd = AllocVec( sizeof *rd, MEMF_CLEAR );
			if( rd = *rxd )
			{
				/* set your DEFAULTS here */
			}
			break;
			
		case RXIF_ACTION:
			/* Insert your CODE here */
			if( sw = (struct SuperWindow *)FindName(&superwindows,currentrexxwindow) )
			{
				if(sw->sw_Window)
				{
					ChangeWindowBox( sw->sw_Window, *rd->arg.leftedge,*rd->arg.topedge,
					sw->sw_Window->Width, sw->sw_Window->Height );
					WaitForWindowMove();
				}
				else
				{
					sw->sw_Left = (WORD)*rd->arg.leftedge;
					sw->sw_Top = (WORD)*rd->arg.topedge;
				}
				rd->rc = 0;
			}
			else
			{
				rd->rc = -5;
				rd->rc2 = (LONG)Txt_SelectedWindowDoesntExist;
			}
			break;
		
		case RXIF_FREE:
			/* FREE your local data here */
			FreeVec( rd );
			break;
	}
	return;
}
/* $ARB: E 11 MOVEWINDOW */

/* $ARB: B 12 SIZEWINDOW */
void rx_sizewindow( struct RexxHost *host, struct rxd_sizewindow **rxd, long action, struct RexxMsg *rexxmsg )
{
	struct rxd_sizewindow *rd = *rxd;
	struct SuperWindow *sw;
	WORD width,height;

	switch( action )
	{
		case RXIF_INIT:
			*rxd = AllocVec( sizeof *rd, MEMF_CLEAR );
			if( rd = *rxd )
			{
				/* set your DEFAULTS here */
			}
			break;
			
		case RXIF_ACTION:
			/* Insert your CODE here */
			if( sw = (struct SuperWindow *)FindName(&superwindows,currentrexxwindow) )
			{
				width = *rd->arg.width;
				height = *rd->arg.height;

				if(width < sw->sw_MinWidth)	width = sw->sw_MinWidth;
				if(width > sw->sw_MaxWidth)	width = sw->sw_MaxWidth;

				if(height < sw->sw_MinHeight)	height = sw->sw_MinHeight;
				if(height > sw->sw_MaxHeight)	height = sw->sw_MaxHeight;

				if(sw->sw_Window)
				{
					ChangeWindowBox( sw->sw_Window,
					 sw->sw_Window->LeftEdge,sw->sw_Window->TopEdge, width, height );
					WaitForWindowMove();
				}
				else
				{
					sw->sw_Width = (WORD)width;
					sw->sw_Height = (WORD)height;
				}
				rd->rc = 0;
			}
			else
			{
				rd->rc = -5;
				rd->rc2 = (LONG)Txt_SelectedWindowDoesntExist;
			}
			break;
		
		case RXIF_FREE:
			/* FREE your local data here */
			FreeVec( rd );
			break;
	}
	return;
}
/* $ARB: E 12 SIZEWINDOW */

/* $ARB: B 13 UNZOOMWINDOW */
void rx_unzoomwindow( struct RexxHost *host, struct rxd_unzoomwindow **rxd, long action, struct RexxMsg *rexxmsg )
{
	struct rxd_unzoomwindow *rd = *rxd;

	switch( action )
	{
		case RXIF_INIT:
			*rxd = AllocVec( sizeof *rd, MEMF_CLEAR );
			break;
			
		case RXIF_ACTION:
			/* Insert your CODE here */
			rd->rc = 0;
			break;
		
		case RXIF_FREE:
			/* FREE your local data here */
			FreeVec( rd );
			break;
	}
	return;
}
/* $ARB: E 13 UNZOOMWINDOW */

/* $ARB: B 14 WINDOWTOBACK */
void rx_windowtoback( struct RexxHost *host, struct rxd_windowtoback **rxd, long action, struct RexxMsg *rexxmsg )
{
	struct rxd_windowtoback *rd = *rxd;
	struct SuperWindow *sw;

	switch( action )
	{
		case RXIF_INIT:
			*rxd = AllocVec( sizeof *rd, MEMF_CLEAR );
			break;
			
		case RXIF_ACTION:
			/* Insert your CODE here */
			if( sw = (struct SuperWindow *)FindName(&superwindows,currentrexxwindow) )
			{
				if(sw->sw_Window)
					WindowToBack(sw->sw_Window);
				rd->rc = 0;
			}
			else
			{
				rd->rc = -5;
				rd->rc2 = (LONG)Txt_SelectedWindowDoesntExist;
			}
			break;
		
		case RXIF_FREE:
			/* FREE your local data here */
			FreeVec( rd );
			break;
	}
	return;
}
/* $ARB: E 14 WINDOWTOBACK */

/* $ARB: B 15 WINDOWTOFRONT */
void rx_windowtofront( struct RexxHost *host, struct rxd_windowtofront **rxd, long action, struct RexxMsg *rexxmsg )
{
	struct rxd_windowtofront *rd = *rxd;
	struct SuperWindow *sw;

	switch( action )
	{
		case RXIF_INIT:
			*rxd = AllocVec( sizeof *rd, MEMF_CLEAR );
			break;
			
		case RXIF_ACTION:
			/* Insert your CODE here */
			if( sw = (struct SuperWindow *)FindName(&superwindows,currentrexxwindow) )
			{
				if(sw->sw_Window)
					WindowToFront(sw->sw_Window);
				rd->rc = 0;
			}
			else
			{
				rd->rc = -5;
				rd->rc2 = (LONG)Txt_SelectedWindowDoesntExist;
			}
			break;
		
		case RXIF_FREE:
			/* FREE your local data here */
			FreeVec( rd );
			break;
	}
	return;
}
/* $ARB: E 15 WINDOWTOFRONT */

/* $ARB: B 16 ZOOMWINDOW */
void rx_zoomwindow( struct RexxHost *host, struct rxd_zoomwindow **rxd, long action, struct RexxMsg *rexxmsg )
{
	struct rxd_zoomwindow *rd = *rxd;

	switch( action )
	{
		case RXIF_INIT:
			*rxd = AllocVec( sizeof *rd, MEMF_CLEAR );
			break;
			
		case RXIF_ACTION:
			/* Insert your CODE here */
			rd->rc = 0;
			break;
		
		case RXIF_FREE:
			/* FREE your local data here */
			FreeVec( rd );
			break;
	}
	return;
}
/* $ARB: E 16 ZOOMWINDOW */

/* $ARB: B 17 SELECTWINDOW */
void rx_selectwindow( struct RexxHost *host, struct rxd_selectwindow **rxd, long action, struct RexxMsg *rexxmsg )
{
	struct rxd_selectwindow *rd = *rxd;

	switch( action )
	{
		case RXIF_INIT:
			*rxd = AllocVec( sizeof *rd, MEMF_CLEAR );
			if( rd = *rxd )
			{
				/* set your DEFAULTS here */
			}
			break;
			
		case RXIF_ACTION:
			/* Insert your CODE here */
			if(FindName(&superwindows,rd->arg.windowid))
			{
				strcpy(currentrexxwindow,rd->arg.windowid);
				rd->rc = 0;
			}
			else
			{
				rd->rc = -5;
				rd->rc2 = (LONG)Txt_WindowDoesntExist;
			}
			break;
		
		case RXIF_FREE:
			/* FREE your local data here */
			FreeVec( rd );
			break;
	}
	return;
}
/* $ARB: E 17 SELECTWINDOW */

/* $ARB: B 18 FINDWINDOW */
void rx_findwindow( struct RexxHost *host, struct rxd_findwindow **rxd, long action, struct RexxMsg *rexxmsg )
{
	struct rxd_findwindow *rd = *rxd;

	switch( action )
	{
		case RXIF_INIT:
			*rxd = AllocVec( sizeof *rd, MEMF_CLEAR );
			if( rd = *rxd )
			{
				/* set your DEFAULTS here */
			}
			break;
			
		case RXIF_ACTION:
			/* Insert your CODE here */
			rd->rc = 0;
			break;
		
		case RXIF_FREE:
			/* FREE your local data here */
			FreeVec( rd );
			break;
	}
	return;
}
/* $ARB: E 18 FINDWINDOW */

/* $ARB: B 19 ACTIVEWINDOW */
void rx_activewindow( struct RexxHost *host, struct rxd_activewindow **rxd, long action, struct RexxMsg *rexxmsg )
{
	struct rxd_activewindow *rd = *rxd;
	struct Window *win;
	struct SuperWindow *sw;
	ULONG lock;

	switch( action )
	{
		case RXIF_INIT:
			*rxd = AllocVec( sizeof *rd, MEMF_CLEAR );
			break;
			
		case RXIF_ACTION:
			/* Insert your CODE here */
			lock = LockIBase(0);
			win = NULL;	/* IntuitionBase->ActiveWindow; */
			UnlockIBase(lock);

			sw = (struct SuperWindow *)win->UserData;
			rd->res.windowid = sw->sw_Node.ln_Name;

			rd->rc = 0;
			break;
		
		case RXIF_FREE:
			/* FREE your local data here */
			FreeVec( rd );
			break;
	}
	return;
}
/* $ARB: E 19 ACTIVEWINDOW */


#ifndef RX_ALIAS_C
char *ExpandRXCommand( struct RexxHost *host, char *command )
{
	/* Insert your ALIAS-HANDLER here */
	return( NULL );
}
#endif

