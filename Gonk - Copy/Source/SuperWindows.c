/*
 *		Stuff For the Handling of SuperWindows
 */

#include <stdio.h>
//#include <stdarg.h>
#include <string.h>
//#include <dos/dos.h>
//#include <dos/dostags.h>
//#include <exec/exec.h>
//#include <intuition/intuition.h>
//#include <graphics/gfxbase.h>
//#include <rexx/rxslib.h>
//#include <libraries/gadtools.h>
//#include <libraries/diskfont.h>
//#include <libraries/asl.h>

#include <proto/exec.h>
//#include <proto/dos.h>
#include <proto/intuition.h>
//#include <proto/graphics.h>
//#include <proto/diskfont.h>
//#include <proto/asl.h>
//#include <clib/alib_protos.h>
//#include <proto/gadtools.h>

#include <global.h>
#include <ExternVars.h>

struct BlockingReqInfo
{
	struct Requester	Req;
	ULONG							OldIDCMP;
};

/*************** RememberWindow ***************/
//
// Copy out the parameters of an Intuition Window into
// fields in its SuperWindow struct, so we can redisplay
// the window at a later stage.
//

void RememberWindow(struct SuperWindow *sw)
{
	struct Window *w;

	if (w = sw->sw_Window)		/* make sure intuition window is open */
	{
		/* copy out intution window parameters */
		/* into fields in the SuperWindow struct. */
		sw->sw_Left = w->LeftEdge;
		sw->sw_Top = w->TopEdge;
		sw->sw_Width = w->Width;
		sw->sw_Height = w->Height;
		sw->sw_MinWidth = w->MinWidth;
		sw->sw_MinHeight = w->MinHeight;
		sw->sw_MaxWidth = w->MaxWidth;
		sw->sw_MaxHeight = w->MaxHeight;
//		sw->sw_Flags = w->Flags;
//		sw->sw_Title = w->Title;
	}
}


/*************** InstallUniqueWindowID ***************/
//
// Figures out a unique name for the specified SuperWindow.
// Fills in the sw_idstring and sw_id fields.
//

void InstallUniqueWindowID(struct SuperWindow *sw)
{
UWORD	num = 0;
int done = FALSE;

	SetProcessWindowPtr();

	while( (num < 1000) && (!done) )
	{
		sprintf( sw->sw_idstring, "W%d",++num);
		if (!(FindName(&superwindows,sw->sw_idstring )))
		{
			sw->sw_Node.ln_Name = sw->sw_idstring;
			sw->sw_id = num;
			done = TRUE;
		}
	}
}


/*************** FindSuperWindowByName ***************/
//
// Returns ptr to SuperWindow with matching idstring.
// (or NULL)
//

struct SuperWindow *FindSuperWindowByName(char *name)
{
	return( (struct SuperWindow *)FindName(&superwindows,name));
}


/*************** GetOpenSuperWindow ***************/
//
// Returns the first SuperWindow that has an open window.
// NULL = No Open superwindows.
//

struct SuperWindow *GetOpenSuperWindow()
{
	struct SuperWindow *sw;

	sw = (struct SuperWindow *)superwindows.lh_Head;

	while( sw->sw_Node.ln_Succ )
	{
		/* found one open? */
		if( sw->sw_Window )
			return(sw);
		/* next please... */
		sw = (struct SuperWindow *)sw->sw_Node.ln_Succ;
	}
	/* no visible windows - bummer. */
	return(NULL);
}



/***************** Count Super Windows **************/
//
// Return the number of Open Windows for this Project.
// Hidden windows are counted as closed.

int CountSuperWindows()
{
	int count = 0;
	struct SuperWindow *sw;

	for (sw = (struct SuperWindow *)superwindows.lh_Head;
			 sw->sw_Node.ln_Succ;
			 sw = (struct SuperWindow *)sw->sw_Node.ln_Succ)
		if(sw->sw_Window) count++;
	return(count);
}

/*************  BuildWindowTitle()  *************/
//
//  Create a title string for the given superwindow.
//

char *BuildWindowTitle(struct SuperWindow *sw)
{
	UWORD len = 1;
	char *str, *p;
	struct ProjectWindow *pw;
	char buffer[10];

	if( glob.cg_ShowWindowID ) len += strlen(sw->sw_idstring) + 1;
	len += strlen(sw->sw_Title);
	if( sw->sw_Type == SWTY_PROJECT)
	{
		pw = (struct ProjectWindow *)sw;
		/* maybe allow space for the modified sign ('+') */
		if( ((struct ProjectWindow *)sw)->pw_Project->pj_ModifiedFlag ) len++;
		/* Allow space for the Using Palette flag (' P') */
		len += 2;
		/* Allow Space for Bob Number */
		len += 4;
		/* Allow Space for Mask Flag (' M') */
		len += 2;
		/* Allow Space for Collision Zone Flag (' Z') */
		len += 2;
		/* Allow Space for Sprite Number (' #1') */
		len += 4;
		/* Allow Space for Bank Name (' [Bank_Name------]') */
		len += 2+15+1;
	}

	if( str = AllocVec( len, MEMF_ANY|MEMF_CLEAR ) )
	{
		p = str;

		if( glob.cg_ShowWindowID )
		{
			p = strcat( p, sw->sw_idstring );
			p = strcat( p, " " );
		}

		p = strcat(p, sw->sw_Title );

		if( sw->sw_Type == SWTY_PROJECT)
		{
			if( pw->pw_Project->pj_ModifiedFlag )
				p = strcat( p, "+" );
			if( &pw->pw_Project->pj_Palette == globpalette )
				p = strcat( p, " P" );
			else
				p = strcat( p, "  " );
			if( (pw->pw_Project->pj_EditType == EDTY_BOBS) ||
					(pw->pw_Project->pj_EditType == EDTY_SPRITES) )
			{
				sprintf(buffer," %d",pw->pw_Project->pj_CurrentBob->bb_Number);
				p = strcat( p, buffer);
				if( pw->pw_Project->pj_BobHasMask )
					p = strcat( p, " M" );
				else
					p = strcat( p, "  " );
				if( pw->pw_Project->pj_CurrentBob->bb_Flags & MBF_COLLISIONZONESET )
					p = strcat( p, " Z" );
				else
					p = strcat( p, "  " );
				if( pw->pw_Project->pj_EditType == EDTY_SPRITES )
				{
					sprintf(buffer," #%d",pw->pw_Project->pj_CurrentBob->bb_SpriteNumber);
					p = strcat( p, buffer);
				}
				else
					p = strcat( p, "   " );
			}
			if( (pw->pw_Project->pj_EditType == EDTY_BLOCKS) ||
					(pw->pw_Project->pj_EditType == EDTY_BOBS) ||
					(pw->pw_Project->pj_EditType == EDTY_SPRITES) )
			{
				p = strcat( p, " [");
				p = strcat( p, pw->pw_Project->pj_BankName);
				p = strcat( p, "]");
			}
		}	
	}
	SetSuperWindowScreenTitle(sw);

	return( str );
}

/*************  RefreshAllMenus()  *************/
//

void RefreshAllMenus( void )
{
	struct SuperWindow *sw;

	for( sw = (struct SuperWindow *)superwindows.lh_Head;
		sw->sw_Node.ln_Succ;
		sw = (struct SuperWindow *)sw->sw_Node.ln_Succ )
	{
		if(sw->sw_Window)
			(*sw->sw_RefreshMenus)( sw );
	}
}

/***************** CloseAllSuperWindows() **************/
//
// Return the number of Open Windows for this Project.
// Hidden windows are counted as closed.

void CloseAllSuperWindows(void)
{
	struct SuperWindow *sw;

	while( !IsListEmpty(&superwindows) )
	{
		sw = (struct SuperWindow *)superwindows.lh_Head;
		(*sw->sw_BuggerOff)(sw);
	}
}

APTR BlockWindow( struct Window *win )
{
	struct BlockingReqInfo *bri;
	ULONG idcmp;

	if( bri = AllocVec( sizeof(struct BlockingReqInfo), MEMF_ANY|MEMF_CLEAR ) )
	{
		idcmp = win->IDCMPFlags;
		bri->OldIDCMP = idcmp;
		if( idcmp & ( IDCMP_MENUVERIFY|IDCMP_SIZEVERIFY|IDCMP_REQVERIFY ) )
		{
			idcmp &= ~( IDCMP_MENUVERIFY|IDCMP_SIZEVERIFY|IDCMP_REQVERIFY );
			if( !idcmp ) idcmp = IDCMP_CLOSEWINDOW;
			ModifyIDCMP( win, idcmp );
		}

		InitRequester( &bri->Req );
		if( Request( &bri->Req, win ) )
		{
			SetWait( win );
		}
		else
		{
			if( bri->OldIDCMP != idcmp ) ModifyIDCMP( win, bri->OldIDCMP );
			FreeVec( bri );
			bri = NULL;
		}
	}
	return( (APTR)bri );
}



VOID UnblockWindow( APTR blockwinhandle )
{
	struct BlockingReqInfo *bri;
	struct Requester *req;

	bri = (struct BlockingReqInfo *)blockwinhandle;
	req = &bri->Req;
	ClrWait( req->RWindow );
	EndRequest( req, req->RWindow );
	if( bri->OldIDCMP != req->RWindow->IDCMPFlags )
		ModifyIDCMP( req->RWindow, bri->OldIDCMP );
	FreeVec( bri );
}



BOOL BlockAllSuperWindows( VOID )
{
	struct SuperWindow *sw;
	BOOL success = TRUE;

	for( sw = (struct SuperWindow *)superwindows.lh_Head;
		sw->sw_Node.ln_Succ && success;
		sw = (struct SuperWindow *)sw->sw_Node.ln_Succ )
	{
		if( sw->sw_Window && !sw->sw_BlockingHandle )
		{
			sw->sw_BlockingHandle = BlockWindow( sw->sw_Window );
			if( !sw->sw_BlockingHandle )
				success = FALSE;
		}
	}

	if( !success )
		UnblockAllSuperWindows();

	return( success );
}


VOID UnblockAllSuperWindows( VOID )
{
	struct SuperWindow *sw;

	for( sw = (struct SuperWindow *)superwindows.lh_Head;
		sw->sw_Node.ln_Succ;
		sw = (struct SuperWindow *)sw->sw_Node.ln_Succ )
	{
		if( sw->sw_Window && sw->sw_BlockingHandle )
		{
			UnblockWindow( sw->sw_BlockingHandle );
			sw->sw_BlockingHandle = NULL;
		}
	}
}

BOOL BlockProjectWindows( struct Project *pj )
{
	struct ProjectWindow *pw;
	BOOL success = TRUE;

	for( pw = pj->pj_ProjectWindows; pw && success; pw = pw->pw_NextProjWindow)
	{
		if( pw->pw_sw.sw_Window && !pw->pw_sw.sw_BlockingHandle )
		{
			pw->pw_sw.sw_BlockingHandle = BlockWindow( pw->pw_sw.sw_Window );
			if( !pw->pw_sw.sw_BlockingHandle )
				success = FALSE;
		}
	}

	if( !success )
		UnblockProjectWindows(pj);

	return( success );
}

VOID UnblockProjectWindows( struct Project *pj )
{
	struct ProjectWindow *pw;

	for( pw = pj->pj_ProjectWindows; pw; pw = pw->pw_NextProjWindow)
	{
		if( pw->pw_sw.sw_Window && pw->pw_sw.sw_BlockingHandle )
		{
			UnblockWindow( pw->pw_sw.sw_BlockingHandle );
			pw->pw_sw.sw_BlockingHandle = NULL;
		}
	}
}

void SetSuperWindowScreenTitle(struct SuperWindow *sw)
{
	if(sw->sw_Window)
	{
		strcpy(screentitlebuffer, mainscreen->DefaultTitle);
		switch(drawmode)
		{
			case DRMD_MATTE:
				strcat(screentitlebuffer," MATTE");
				break;
			case DRMD_COLOUR:
				strcat(screentitlebuffer," COLOUR");
				break;
			case DRMD_REPLACE:
				strcat(screentitlebuffer," REPLACE");
				break;
			case DRMD_SHADE:
				strcat(screentitlebuffer," SHADE");
				break;
			case DRMD_SMOOTH:
				strcat(screentitlebuffer," SMOOTH");
				break;
			case DRMD_CYCLE:
				strcat(screentitlebuffer," CYCLE");
				break;
		}
		SetWindowTitles(sw->sw_Window, (UBYTE *)-1, screentitlebuffer);
	}
}

void SetSuperWindowsScreenTitle(void)
{
	struct SuperWindow *sw = NULL;

	strcpy(screentitlebuffer, mainscreen->DefaultTitle);
	switch(drawmode)
	{
		case DRMD_MATTE:
			strcat(screentitlebuffer," MATTE");
			break;
		case DRMD_COLOUR:
			strcat(screentitlebuffer," COLOUR");
			break;
		case DRMD_REPLACE:
			strcat(screentitlebuffer," REPLACE");
			break;
		case DRMD_SHADE:
			strcat(screentitlebuffer," SHADE");
			break;
		case DRMD_SMOOTH:
			strcat(screentitlebuffer," SMOOTH");
			break;
		case DRMD_CYCLE:
			strcat(screentitlebuffer," CYCLE");
			break;
	}
	for( sw = (struct SuperWindow *)superwindows.lh_Head;
		sw->sw_Node.ln_Succ;
		sw = (struct SuperWindow *)sw->sw_Node.ln_Succ )
	{
		if( sw->sw_Window )
			SetWindowTitles(sw->sw_Window, (UBYTE *)-1, screentitlebuffer);
	}
}
