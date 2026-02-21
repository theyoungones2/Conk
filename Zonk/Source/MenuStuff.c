/********************************************************/
//
// MenuStuff.c
//
/********************************************************/

#define MENUSTUFF_C

#include <stdio.h>
//#include <string.h>
//#include <stdarg.h>

//#include <dos/dos.h>
//#include <dos/dostags.h>
#include <exec/exec.h>
#include <intuition/intuition.h>
//#include <graphics/gfxbase.h>
#include <libraries/gadtools.h>
//#include <libraries/diskfont.h>
//#include <libraries/asl.h>
//#include <libraries/iffparse.h>

#include <proto/exec.h>
//#include <proto/dos.h>
#include <proto/intuition.h>
//#include <proto/graphics.h>
//#include <proto/diskfont.h>
//#include <proto/asl.h>
//#include <clib/alib_protos.h>
#include <proto/gadtools.h>
//#include <proto/iffparse.h>

//#include "hoopy.h"
#include "Global.h"
#include "MenuID.h"


struct NewMenu projectmenu[] = {
		{ NM_TITLE,	"Project",								0,	0,	0,	(APTR)PROJMENU },
		{  NM_ITEM,	"New Chunk...",					 "N",	0,	0,	(APTR)PROJMENU_NEW },
		{  NM_ITEM,	"Load Chunk(s)...",			 "O",	0,	0,	(APTR)PROJMENU_LOAD },
		{	 NM_ITEM,	NM_BARLABEL,							0,	0,	0,	(APTR)PROJMENU_DUMMY },
		{  NM_ITEM,	"About...",								0,	0,	0,	(APTR)PROJMENU_ABOUT },
		{  NM_ITEM,	"Quit Program...",			 "Q",	0,	0,	(APTR)PROJMENU_QUIT },
		{ NM_END, NULL, 0,	0,	0,	0,},
	};


struct NewMenu windowmenu[] = {
		{ NM_TITLE,	"Window",								0,	0,	0,	(APTR)WINMENU },
		{  NM_ITEM,	"Open",									0,	0,	0,	(APTR)WINMENU_OPEN },
		{  	NM_SUB,	"Chunk...",							0,	0,	0,	(APTR)WINMENU_OPENCHUNK },
		{  	NM_SUB,	"Screen Settings...",		0,	0,	0,	(APTR)WINMENU_OPENSCRSET },
		{  	NM_SUB,	"About...",							0,	0,	0,	(APTR)WINMENU_OPENABOUT },
		{  NM_ITEM,	"Close",								0,	0,	0,	(APTR)WINMENU_CLOSE },
		{ NM_END, NULL, 0,	0,	0,	0,},
	};


struct NewMenu settingsmenu[] = {
		{ NM_TITLE,	"Settings",							0,	0,	0,	(APTR)SETMENU },
		{  NM_ITEM,	"Save Icons?",					0,	CHECKIT,	0,	(APTR)SETMENU_SAVEICONS },
		{  NM_ITEM,	"make Backups?",				0,	CHECKIT,	0,	(APTR)SETMENU_MAKEBACKUPS },
		{	 NM_ITEM,	NM_BARLABEL,						0,	0,	0,	(APTR)SETMENU_DUMMY },
		{  NM_ITEM,	"Screen Settings...",		0,	0,	0,	(APTR)SETMENU_SCRSET },
		{	 NM_ITEM,	NM_BARLABEL,						0,	0,	0,	(APTR)SETMENU_DUMMY },
		{  NM_ITEM,	"Load Settings...",			0,	0,	0,	(APTR)SETMENU_LOAD },
		{  NM_ITEM,	"Save Settings",				0,	0,	0,	(APTR)SETMENU_SAVE },
		{  NM_ITEM,	"Save Settings As...",	0,	0,	0,	(APTR)SETMENU_SAVEAS },
		{ NM_END, NULL, 0,	0,	0,	0,},
	};


/********************************************************/
// HandleProjectMenu()
//
// Handle MENUPICKs occuring on the global project menu.
// menuid is our custom identifier from GTMENUITEM_USERDATA.
// Upper word of menuid will be PROJMENU (see MenuID.h)

BOOL HandleProjectMenu( struct SuperWindow *sw, ULONG menuid )
{
	BOOL stop;
	struct SuperWindow *swone,*swtoo;

	stop = FALSE;
	switch( menuid )
	{
		case PROJMENU_NEW:
			NewChunk( sw );
			break;
		case PROJMENU_LOAD:
			LoadChunks( sw );
			break;
		case PROJMENU_ABOUT:
			if( BlockAllSuperWindows() )
			{
				PlebReq( sw->sw_Window, "ZONK",
					"Version 0.90 (Beta)\nBy Ben Campbell\n23/5/96 Release\n\n©1996 42° South", "OK" );
			}
			UnblockAllSuperWindows();
			break;
		case PROJMENU_QUIT:
			swone = (struct SuperWindow *)superwindows.lh_Head;
			while( swone->sw_Node.ln_Succ )
			{
				swtoo = (struct SuperWindow *)swone->sw_Node.ln_Succ;
				(*swone->sw_BuggerOff)(swone);
				swone = swtoo;
			}
			stop = TRUE;
			break;
	}

	return( stop );
}

/********************************************************/
// HandleWindowMenu()
//
// Handle MENUPICKs occuring on the global window menu.
// menuid is our custom identifier from GTMENUITEM_USERDATA.
// Upper word of menuid will be WINMENU (see MenuID.h)

BOOL HandleWindowMenu( struct SuperWindow *sw, ULONG menuid )
{
	BOOL stop;

	stop = FALSE;
	switch( menuid )
	{
		case WINMENU_OPENCHUNK:
			OpenNewChunkWindow();
			break;
		case WINMENU_OPENSCRSET:
			OpenNewScreenConfigWindow();
			break;
		case WINMENU_CLOSE:
			(*sw->sw_BuggerOff)(sw);
			stop = TRUE;
			break;
	}
	return( stop );
}

/********************************************************/
// HandleSettingsMenu()
//
// Handle MENUPICKs occuring on the global settings menu.
// menuid is our custom identifier from GTMENUITEM_USERDATA.
// Upper word of menuid will be WINMENU (see MenuID.h)

BOOL HandleSettingsMenu( struct SuperWindow *sw, ULONG menuid )
{
	struct MenuItem *item;
	BOOL stop;
	struct ZonkConfig zcfg;

	stop = FALSE;
	switch( menuid )
	{
		case SETMENU_SAVEICONS:
			if( item = FindMenuItem( sw->sw_SettingsMenu, SETMENU_SAVEICONS ) )
			{
				zonkfig.SaveIcons = ( item->Flags & CHECKED ) ? TRUE:FALSE;
				SetGlobalOption( SETMENU_SAVEICONS, zonkfig.SaveIcons );
			}
			break;
		case SETMENU_MAKEBACKUPS:
			if( item = FindMenuItem( sw->sw_SettingsMenu, SETMENU_MAKEBACKUPS ) )
			{
				zonkfig.MakeBackups = ( item->Flags & CHECKED ) ? TRUE:FALSE;
				SetGlobalOption( SETMENU_MAKEBACKUPS, zonkfig.MakeBackups );
			}
			break;
		case SETMENU_SCRSET:
			OpenNewScreenConfigWindow();
			break;
		case SETMENU_SAVE:
			SaveZonkConfig( "PROGDIR:Zonk.cfg", &zonkfig );
			break;
		case SETMENU_SAVEAS:
			GUISaveZonkConfigAs( &zonkfig );
			break;
		case SETMENU_LOAD:
			if( GUILoadZonkConfig( &zcfg ) )
				InstallZonkConfig( &zcfg );
			stop = TRUE;
			break;
	}
	return( stop );
}


/********************************************************/

VOID SetGlobalOption( ULONG menuid, BOOL state )
{
	struct SuperWindow *superdave;
	struct MenuItem *item;

	for( superdave = (struct SuperWindow *)superwindows.lh_Head;
		superdave->sw_Node.ln_Succ;
		superdave = (struct SuperWindow *)superdave->sw_Node.ln_Succ )
	{
		if( superdave->sw_SettingsMenu )
		{
			if( item = FindMenuItem( superdave->sw_SettingsMenu, menuid ) )
			{
				if( state )
					item->Flags |= CHECKED;
				else
					item->Flags &= ~CHECKED;
			}
		}
	}
}


/********************************************************/
//GenericMakeMenus()
//
// Add global menus & custom menu(s) (optional) to a superwindow.
// Pair with GenericZapMenus()
//
//

BOOL GenericMakeMenus( struct SuperWindow *sw, struct NewMenu *custommenu )
{
	BOOL success = FALSE;

	if( zonkfig.SaveIcons )
		settingsmenu[1].nm_Flags |= CHECKED;
	else
		settingsmenu[1].nm_Flags &= ~CHECKED;

	if( zonkfig.MakeBackups )
		settingsmenu[2].nm_Flags |= CHECKED;
	else
		settingsmenu[2].nm_Flags &= ~CHECKED;

	sw->sw_ProjectMenu = CreateMenus( projectmenu, TAG_DONE );
	if( custommenu )
		sw->sw_CustomMenu = CreateMenus( custommenu, TAG_DONE );
	sw->sw_WindowMenu = CreateMenus( windowmenu, TAG_DONE );
	sw->sw_SettingsMenu = CreateMenus( settingsmenu, TAG_DONE );

	if( sw->sw_ProjectMenu && sw->sw_WindowMenu && sw->sw_SettingsMenu )
	{
		sw->sw_ProjectMenu->NextMenu = sw->sw_WindowMenu;
		sw->sw_WindowMenu->NextMenu = sw->sw_SettingsMenu;
		if( sw->sw_CustomMenu )
			sw->sw_SettingsMenu->NextMenu = sw->sw_CustomMenu;

		if( LayoutMenus( sw->sw_ProjectMenu, sw->sw_VisualInfo, TAG_DONE ) )
		{
			SetMenuStrip( sw->sw_Window, sw->sw_ProjectMenu );
			/* call refreshmenus() here to set up checkmarks, disabling etc... */
			/* ... or maybe not... */

//			printf("GenericMakeMenus()\n");
//			printf("makebackups: %ld\n",zonkfig.MakeBackups);

//			SetGlobalOption( SETMENU_SAVEICONS, zonkfig.SaveIcons );
//			SetGlobalOption( SETMENU_MAKEBACKUPS, zonkfig.MakeBackups );

			success = TRUE;
		}
	}

	if( !success )
	{
		if( sw->sw_ProjectMenu ) FreeMenus( sw->sw_ProjectMenu );
		if( sw->sw_WindowMenu ) FreeMenus( sw->sw_WindowMenu );
		if( sw->sw_SettingsMenu ) FreeMenus( sw->sw_SettingsMenu );
		if( sw->sw_CustomMenu ) FreeMenus( sw->sw_CustomMenu );
		sw->sw_ProjectMenu = NULL;
		sw->sw_WindowMenu = NULL;
		sw->sw_SettingsMenu = NULL;
		sw->sw_CustomMenu = NULL;
	}
	return( success );
}


/********************************************************/
// GenericZapMenus()
//
// Remove and free menus from a superwindow.
//

VOID GenericZapMenus( struct SuperWindow *sw )
{
	if( sw->sw_Window )
	{
		ClearMenuStrip( sw->sw_Window );

		if( sw->sw_ProjectMenu ) FreeMenus( sw->sw_ProjectMenu );
		if( sw->sw_WindowMenu ) FreeMenus( sw->sw_WindowMenu );
		if( sw->sw_SettingsMenu ) FreeMenus( sw->sw_SettingsMenu );
		if( sw->sw_CustomMenu ) FreeMenus( sw->sw_CustomMenu );
		sw->sw_ProjectMenu = NULL;
		sw->sw_WindowMenu = NULL;
		sw->sw_SettingsMenu = NULL;
		sw->sw_CustomMenu = NULL;
	}
}

/********************************************************/
//

struct MenuItem *FindMenuItem( struct Menu *startmenu, ULONG menuid )
{
	struct Menu *menu;
	struct MenuItem *item, *subitem, *matchitem;

	menu = startmenu;
	matchitem = NULL;

	while( menu && !matchitem )
	{
		item = menu->FirstItem;
		while( item && !matchitem )
		{
			if( (ULONG)GTMENUITEM_USERDATA( item ) == menuid )
				matchitem = item;
			subitem = item->SubItem;
			while( subitem && !matchitem )
			{
				if( (ULONG)GTMENUITEM_USERDATA( subitem ) == menuid )
					matchitem = subitem;
				subitem = subitem->NextItem;
			}
			item = item->NextItem;
		}
		menu = menu->NextMenu;
	}
	return( matchitem );
}



/********************************************************/
// A MENUPICK handler
// customhandler may be NULL if no custom menu (CUSTMENU) processing
// is to be done.

VOID GenericHandleMenuPick( struct SuperWindow *sw, UWORD menunum,
	BOOL (*customhandler)( struct SuperWindow *sw, ULONG id ) )
{
	BOOL stop;
	struct MenuItem *item;
	ULONG id;
	UWORD menuid;

	stop = FALSE;
	while( menunum != MENUNULL && !stop )
	{
		item = ItemAddress( sw->sw_ProjectMenu, menunum );
		id = ( ULONG )GTMENUITEM_USERDATA( item );
		menuid = (UWORD)( id >> 16 );

		switch( menuid )
		{
			case PROJMENU:
				stop = HandleProjectMenu( sw, id );
				break;
			case WINMENU:
				stop = HandleWindowMenu( sw, id );
				break;
			case SETMENU:
				stop = HandleSettingsMenu( sw, id );
				break;
			case CUSTMENU:
				if( customhandler )
				{
					stop = customhandler( sw, id );
				}
				break;
		}
		menunum = item->NextSelect;
	}
}

