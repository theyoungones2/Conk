/****************************************************************************/
//
// BONK: SettingsMenu.c
//
/****************************************************************************/



#include <stdio.h>
#include <string.h>
#include <dos/dos.h>
//#include <dos/dostags.h>
#include <exec/exec.h>
#include <intuition/intuition.h>
#include <graphics/gfxbase.h>
//#include <rexx/rxslib.h>
#include <libraries/gadtools.h>
//#include <libraries/diskfont.h>
#include <libraries/asl.h>
//#include "dh1:programming/stuff/include/libraries/asl.h"

//#include <libraries/iffparse.h>

#include <proto/exec.h>
#include <proto/dos.h>
#include <proto/intuition.h>
#include <proto/graphics.h>
//#include <proto/diskfont.h>
#include <proto/asl.h>
#include <clib/alib_protos.h>
#include <proto/gadtools.h>
//#include <proto/iffparse.h>


#include <global.h>
#include <ExternVars.h>




/*************  DoSettingsMenu ***********/
//
//	Handle SettingsMenu selections.
//

void DoSettingsMenu(struct SuperWindow *sw,UWORD id)
{
	UWORD itemnum,subnum;
	struct MenuItem *mi;
	// struct List *winlist;

	itemnum = ITEMNUM(id);
	subnum = SUBNUM(id);

	mi = ItemAddress( sw->sw_MenuStrip, id );

	switch(itemnum)
	{
//		case MNSET_WINDOWIDS:
//			glob.cg_ShowWindowID = (mi->Flags & CHECKED) ? TRUE : FALSE;
			/* change this sometime - just need to remake menus and window titles */
//			if( winlist = ListAndHideWindows() )
//				ReshowWindowList(winlist);
//			break;
		case MNSET_MAKEBACKUPS:
			glob.cg_MakeBackups = (mi->Flags & CHECKED) ? TRUE : FALSE;
			RefreshAllMenus();
			break;
		case MNSET_SETSETTINGS:
			OpenNewScreenConfigWindow();
			break;
		case MNSET_SAVESETTINGS:
			SaveSettings( "Bonk.cfg" );
			break;
		case MNSET_SAVESETTINGSAS:
			SaveSettingsAs(sw->sw_Window);
			break;
		case MNSET_LOADSETTINGS:
			LoadSettingsAs(sw->sw_Window);
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

//	nm[MNSET_WINDOWIDS_IDX].nm_Flags = glob.cg_ShowWindowID ?
//		CHECKIT|MENUTOGGLE|CHECKED : CHECKIT|MENUTOGGLE;
	nm[MNSET_MAKEBACKUPS_IDX].nm_Flags = glob.cg_MakeBackups ?
		CHECKIT|MENUTOGGLE|CHECKED : CHECKIT|MENUTOGGLE;
}



/************* SaveSettingsAs() *************/
//
// Brings up a file requester to get a filename,
// and saves out the current configuration.
//

void SaveSettingsAs( struct Window *win )
{
	char name[256];
	struct List *rlist;
	BOOL pleb;

	/* block input to stop the luser doing anything mental */
	if(rlist = BlockAllWindows())
	{
		/* bring up the file requester */
		pleb = AslRequestTags( filereq,
			ASLFR_Window, win,
			ASLFR_DoSaveMode, TRUE,
			ASLFR_TitleText, "Save Settings As",
			TAG_END );
		FreeBlockedWindows(rlist);
		if(pleb)
		{
			/* glue together the full pathname */
			if( AddPart( Mystrncpy(name, filereq->fr_Drawer, 256-1),
				filereq->fr_File,
				256 ) )
			{
				/* try and save it */
				SaveSettings(name);
			}
//			else
//			{
				/* no good - name too long */
//				printf("Invalid name\n");
//			}
		}
	}
}



/************* SaveSettings() *************/
//
// Saves the current config out under the given name.
//

void SaveSettings(char *name)
{
	BPTR fh;

	if( fh = Open(name, MODE_NEWFILE) )
	{
		/* write out an ID string */
		Write(fh, "CKCONFIG" ,8);
		Write(fh, &glob, sizeof(glob) );
		Close(fh);
	}
}



/************* LoadSettingsAs() *************/
//
// Brings up a file requester to load in a config
// file. Returns TRUE if all is cool and froody,
// else FALSE.
//

BOOL LoadSettingsAs( struct Window *win )
{
	char name[256];
	BOOL pleb, ret=FALSE;
	struct List *rlist;

	if(rlist = BlockAllWindows())
	{
		/* bring up the file requester */
		pleb = AslRequestTags( filereq,
			ASLFR_Window, win,
			ASLFR_DoSaveMode, FALSE,
			ASLFR_TitleText, "Load Settings",
			TAG_END );
		FreeBlockedWindows(rlist);
		if(pleb)
		{
			/* glue together the full pathname */
			if( AddPart( Mystrncpy(name, filereq->fr_Drawer, 256-1),
				filereq->fr_File,
				256 ) )
			{
				/* try and load it in */
				ret = LoadSettings(name);
			}
//			else
//			{
				/* no good - name too long */
//				printf("Invalid name\n");
//			}
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
	struct Config cg;
	BPTR		fh;
	UBYTE		header[8];
	BOOL		result = FALSE;

	if( fh = Open( name, MODE_OLDFILE ) )
	{
		Read(fh, header, 8);
		if( strncmp( header, "CKCONFIG", 8) == 0 )
		{
			/* header string OK - read in the config */
			if( Read( fh, &cg, sizeof(cg) ) == sizeof(cg) )
			{
				/* use the new config */
				InstallNewGlobalConfig(&cg);
				result = TRUE;
			}
//			else
//				printf("Error reading config!\n");
		}
//		else
//			printf("Not a config file!\n");
		Close(fh);
	}
//	else
//		printf("Couldn't open config file!\n");
	return(result);
}
