/***********************************************************/
// MenuID.h
//
// ID numbers for global menus. The menu ID field is stored in the
// UserData field provided by GadTools. The High word of the ID number
// gives the Menu ID, and the Low Word gives the MenuItem ID.
// See also MenuStuff.c

/* Menu ID numbers */

#define PROJMENU	1
#define WINMENU		2
#define SETMENU		3
#define USERMENU 	4
#define CUSTMENU	5

/* MenuItem ID numbers */

enum { PROJMENU_NEW = (PROJMENU<<16) + 1, PROJMENU_LOAD, PROJMENU_ABOUT,
	PROJMENU_QUIT, PROJMENU_DUMMY	};

enum { WINMENU_OPEN = (WINMENU<<16) + 1, WINMENU_OPENCHUNK, WINMENU_OPENSCRSET,
	WINMENU_OPENABOUT, WINMENU_CLOSE, WINMENU_DUMMY	};

enum { SETMENU_SAVEICONS = (SETMENU<<16) + 1, SETMENU_MAKEBACKUPS, SETMENU_SCRSET,
	SETMENU_LOAD, SETMENU_SAVE, SETMENU_SAVEAS, SETMENU_DUMMY	};
