/****************************************************************************/
//
// BONK: SuperWindows.c
//
/****************************************************************************/


#include <stdio.h>
#include <string.h>
#include <dos/dos.h>
//#include <dos/dostags.h>
#include <exec/exec.h>
#include <intuition/intuition.h>
//#include <intuition/gadgetclass.h>
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
#include <clib/alib_protos.h>
//#include <proto/gadtools.h>
//#include <proto/iffparse.h>

#include <global.h>
#include <ExternVars.h>

/*
#include <stdio.h>
#include <string.h>
#include <exec/exec.h>
#include <intuition/intuition.h>
#include <dos/dos.h>

#include <clib/alib_protos.h>
#include <clib/exec_protos.h>
#include <clib/intuition_protos.h>
#include <clib/dos_protos.h>
*/

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

	while ((num < 1000) && (done == FALSE))
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
// Create a title string for the given superwindow.
//

char *BuildWindowTitle( struct SuperWindow *sw )
{
	UWORD len = 1;
	char *str, *p;

	len += strlen(sw->sw_Title);

	if( sw->sw_Type == SWTY_PROJECT)
	{
		/* maybe allow space for the modified sign ('+') */
		if( ((struct ProjectWindow *)sw)->pw_Project->pj_ModifiedFlag ) len++;
		/* maybe allow space for the locked background flag (' B') */
		if( ((struct ProjectWindow *)sw)->pw_Project->pj_BackLockFlag ) len += 2;
		/* allow space for map name ( and brackets, spaces )*/
		len += MAPNAMESIZE + 4;
	}

	if( sw->sw_Type == SWTY_BLOCK )
		len += BLOCKSETNAMESIZE + 4;

	if( str = AllocVec( len, MEMF_ANY|MEMF_CLEAR ) )
	{
		p = str;

		p = strcat(p, sw->sw_Title );

		if( sw->sw_Type == SWTY_PROJECT)
		{
			if( ((struct ProjectWindow *)sw)->pw_Project->pj_ModifiedFlag )
				p = strcat( p, "+" );
			if( ((struct ProjectWindow *)sw)->pw_Project->pj_BackLockFlag )
				p = strcat( p, " B" );

			p=strcat( p, " [" );
			p=strcat( p, ((struct ProjectWindow *)sw)->pw_Project->pj_MapName );
			p=strcat( p, "]" );
		}

		if( sw->sw_Type == SWTY_BLOCK )
		{
			p=strcat( p, " [" );
			p=strcat( p, globlocks->bs_Name );
			p=strcat( p, "]" );
		}

	}
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


/*************  FindSuperWindowByType()  *************/
//

struct SuperWindow *FindSuperWindowByType( UWORD type )
{
	struct SuperWindow *sw, *foundsw = NULL;

	for( sw = (struct SuperWindow *)superwindows.lh_Head;
		sw->sw_Node.ln_Succ && !foundsw;
		sw = (struct SuperWindow *)sw->sw_Node.ln_Succ )
	{
		if( sw->sw_Type == type ) foundsw = sw;
	}
	return( foundsw );
}



