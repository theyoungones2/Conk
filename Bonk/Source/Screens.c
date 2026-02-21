/****************************************************************************/
//
// BONK: Screens.c
//
/****************************************************************************/


#include <stdio.h>
//#include <stdarg.h>
#include <string.h>
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
//#include <proto/dos.h>
#include <proto/intuition.h>
#include <proto/graphics.h>
#include <proto/diskfont.h>
//#include <proto/asl.h>
//#include <clib/alib_protos.h>
//#include <proto/gadtools.h>

#include <intuition/intuitionbase.h>
#include <externvars.h>
#include <Global.h>

/****************** SetupScreen() ****************************/
BOOL SetupScreen(struct ScreenConfig *scrconfig) 
{
	/*	Have to copy out the config font crap because the name is stored in the
			config itself and not a pointer to the name */

	/* Set up ScreenFont */
	screentextattr.ta_Name = scrconfig->sg_SFNameBuf;
	screentextattr.ta_YSize = scrconfig->sg_SFYSize;
	screentextattr.ta_Style = scrconfig->sg_SFStyle;
	screentextattr.ta_Flags = scrconfig->sg_SFFlags;
	if(!(screentextfont = OpenDiskFont(&screentextattr)) )
	{
		screentextattr.ta_Name = "topaz.font";
		screentextattr.ta_YSize = 8;
		screentextattr.ta_Style = 0;
		screentextattr.ta_Flags = 0;
		if(!(screentextfont = OpenDiskFont(&screentextattr)) ) return(FALSE);
	}

	/* Set up WindowFont */
	windowtextattr.ta_Name = scrconfig->sg_WFNameBuf;
	windowtextattr.ta_YSize = scrconfig->sg_WFYSize;
	windowtextattr.ta_Style = scrconfig->sg_WFStyle;
	windowtextattr.ta_Flags = scrconfig->sg_WFFlags;
	if(!(windowtextfont = OpenDiskFont(&windowtextattr)) )
	{
		windowtextattr.ta_Name = "topaz.font";
		windowtextattr.ta_YSize = 8;
		windowtextattr.ta_Style = 0;
		windowtextattr.ta_Flags = 0;
		if(!(windowtextfont = OpenDiskFont(&windowtextattr)) ) return(FALSE);
	}

	if(scrconfig->sg_OwnScreen)
	{
		if(!OpenNewScreen(scrconfig))
			/*  If OpenNewScreen() Fails, then we're basically fucked. OpenNewScreen()
			 * does everything humanly possible to get a screen to work on, including
			 * borrowing workbench, so if it fails then we can just pack our bags and
			 * bugger off */
			return(FALSE);
	}
	else
	{
		if( !BorrowScreen(scrconfig) )
			return(FALSE);
	}
	return(TRUE);
}

/**********************  Open New Screen  *********************************/

BOOL OpenNewScreen(struct ScreenConfig *scrconfig)
{
	static UWORD pens[NUMDRIPENS+1];	//			= { 0,1,1,2,1,0xFFFF };
	static char	screenname[ 80 ];
	BOOL	usingdefaultsettings = FALSE, done = FALSE;
	UWORD result,	pubscreennumber		= 1;
	ULONG	screenerrorcode		= NULL;

	SussDrawInfoPens( pens, globpalette, 1 << scrconfig->sg_DisplayDepth );

	strcpy(screenname,SCREENNAME);

	while(!done)
	{
		screenerrorcode = NULL;

		if( mainscreen = OpenScreenTags(NULL,
										SA_Type,				PUBLICSCREEN,
										SA_DetailPen,		pens[DETAILPEN],
										SA_BlockPen,		pens[BLOCKPEN],
										SA_PubName,			&screenname,
										SA_Pens,				(ULONG)pens,
										SA_Width,				scrconfig->sg_DisplayWidth,
										SA_Height,			scrconfig->sg_DisplayHeight,
										SA_Depth,				scrconfig->sg_DisplayDepth,
										SA_DisplayID,		scrconfig->sg_DisplayID,
										SA_Font,				&screentextattr,
										SA_AutoScroll,	scrconfig->sg_AutoScroll,
										SA_Interleaved,	(IntuitionBase->LibNode.lib_Version >= 39) ?
																			TRUE : FALSE,
										SA_ErrorCode,		&screenerrorcode,
										TAG_DONE,				NULL) )
		{
			done = TRUE;
		}
		else
		{
			switch(screenerrorcode)
			{
				case OSERR_PUBNOTUNIQUE:
					sprintf( screenname, "%s.%d", SCREENNAME,pubscreennumber );
					pubscreennumber++;
					if(pubscreennumber == 10)
					{
						mainscreen = NULL;
						GroovyReq( PROGNAME,"Can't Open Screen\nUsing Workbench","Continue");
						strcpy(scrconfig->sg_BorrowedName, "Workbench");
		/*
						scrconfig->sg_DisplayID = 0;
						scrconfig->sg_DisplayWidth = 320;
						scrconfig->sg_DisplayHeight = 256;
						scrconfig->sg_DisplayDepth = 2;
						scrconfig->sg_OverscanType = 0;
						scrconfig->sg_AutoScroll = TRUE;
						scrconfig->sg_Shanghai = FALSE;
		 */
						scrconfig->sg_OwnScreen = FALSE;
						BorrowScreen(scrconfig);
						done = TRUE;
					}
					break;
				case OSERR_NOMONITOR:
				case OSERR_NOCHIPS:
				case OSERR_UNKNOWNMODE:
				case OSERR_TOODEEP:
				case OSERR_ATTACHFAIL:
					mainscreen = NULL;
					if(!usingdefaultsettings)
					{
						GroovyReq( PROGNAME, "Can't Open Screen\nUsing Default Settings","Continue");
						scrconfig->sg_BorrowedName[0] = '\0';
						scrconfig->sg_DisplayID = 0;
						scrconfig->sg_DisplayWidth = 320;
						scrconfig->sg_DisplayHeight = 256;
						scrconfig->sg_DisplayDepth = 2;
						scrconfig->sg_OverscanType = 0;
						scrconfig->sg_OwnScreen = TRUE;
						scrconfig->sg_AutoScroll = TRUE;
						scrconfig->sg_Shanghai = FALSE;
						usingdefaultsettings = TRUE;
					}
					else
					{
						GroovyReq( PROGNAME, "Can't Open Screen\nUsing Workbench","Continue");
						strcpy(scrconfig->sg_BorrowedName, "Workbench");
		/*
						scrconfig->sg_DisplayID = 0;
						scrconfig->sg_DisplayWidth = 320;
						scrconfig->sg_DisplayHeight = 256;
						scrconfig->sg_DisplayDepth = 2;
						scrconfig->sg_OverscanType = 0;
						scrconfig->sg_AutoScroll = TRUE;
						scrconfig->sg_Shanghai = FALSE;
		 */
						scrconfig->sg_OwnScreen = FALSE;
						BorrowScreen(scrconfig);
						done = TRUE;
					}
					break;
				case OSERR_NOMEM:
				case OSERR_NOCHIPMEM:
					mainscreen = NULL;
					result = GroovyReq( PROGNAME, "Not Enough Memory\nUse Default Settings\nor Workbench","Default|Workbench");
					if(result == 1)
					{
						scrconfig->sg_BorrowedName[0] = '\0';
						scrconfig->sg_DisplayID = 0;
						scrconfig->sg_DisplayWidth = 320;
						scrconfig->sg_DisplayHeight = 256;
						scrconfig->sg_DisplayDepth = 2;
						scrconfig->sg_OverscanType = 0;
						scrconfig->sg_OwnScreen = TRUE;
						scrconfig->sg_AutoScroll = TRUE;
						scrconfig->sg_Shanghai = FALSE;
						usingdefaultsettings = TRUE;
					}
					else
					{
						strcpy(scrconfig->sg_BorrowedName, "Workbench");
		/*
						scrconfig->sg_DisplayID = 0;
						scrconfig->sg_DisplayWidth = 320;
						scrconfig->sg_DisplayHeight = 256;
						scrconfig->sg_DisplayDepth = 2;
						scrconfig->sg_OverscanType = 0;
						scrconfig->sg_AutoScroll = TRUE;
						scrconfig->sg_Shanghai = FALSE;
		 */
						scrconfig->sg_OwnScreen = FALSE;
						BorrowScreen(scrconfig);
						done = TRUE;
					}
					break;
			}
		}
	}

	if(scrconfig->sg_OwnScreen)
	{
		/* Make the screen open to the public */
		PubScreenStatus(mainscreen,NULL);
		ownscreen = TRUE;
	
		SetShanghaiState(mainscreen,scrconfig->sg_Shanghai);
	
		InstallPalette(mainscreen, globpalette);
		return(TRUE);
	}
}


/**********************  BorrowScreen  *********************************/

BOOL BorrowScreen(struct ScreenConfig *scrconfig)
{
	struct PubScreenNode		*screennode;
	UWORD result;
	BOOL done = FALSE, ret = FALSE;

	ownscreen = FALSE;

	while(!done)
	{
		if( (screennode = GetPubScreen(scrconfig->sg_BorrowedName)) )
		{
			ownscreen = FALSE;
			mainscreen = screennode->psn_Screen;
			SetShanghaiState(mainscreen,scrconfig->sg_Shanghai);
			ScreenToFront(mainscreen);
			done = TRUE;
			ret = TRUE;
		}
		else
		{
			if( stricmp("Workbench",scrconfig->sg_BorrowedName) ==  0)
			{
				result = GroovyReq( PROGNAME, "Can't Find Workbench\nOpen On New Screen?","Yes|Abort");
				if( !result )
				{
					mainscreen = NULL;
					done = TRUE;
				}
				else
				{
					scrconfig->sg_BorrowedName[0] = '\0';
					scrconfig->sg_DisplayID = 0;
					scrconfig->sg_DisplayWidth = 320;
					scrconfig->sg_DisplayHeight = 256;
					scrconfig->sg_DisplayDepth = 2;
					scrconfig->sg_OverscanType = 0;
					scrconfig->sg_OwnScreen = TRUE;
					scrconfig->sg_AutoScroll = TRUE;
					scrconfig->sg_Shanghai = FALSE;
					ret = OpenNewScreen(scrconfig);
					done = TRUE;
				}
			}
			else
			{
				result = GroovyReq( PROGNAME, "Can't Find Screen %s\nOpen On New Screen or Workbench?","Screen|Workbench|Abort",scrconfig->sg_BorrowedName );
				switch(result)
				{
					case 0:
						mainscreen = NULL;
						done = TRUE;
						ret = FALSE;
						break;
					case 1:
						scrconfig->sg_BorrowedName[0] = '\0';
						scrconfig->sg_DisplayID = 0;
						scrconfig->sg_DisplayWidth = 320;
						scrconfig->sg_DisplayHeight = 256;
						scrconfig->sg_DisplayDepth = 2;
						scrconfig->sg_OverscanType = 0;
						scrconfig->sg_OwnScreen = TRUE;
						scrconfig->sg_AutoScroll = TRUE;
						scrconfig->sg_Shanghai = FALSE;
						ret = OpenNewScreen(scrconfig);
						done = TRUE;
						break;
					case 2:
						strcpy(scrconfig->sg_BorrowedName, "Workbench");
		/*
						scrconfig->sg_DisplayID = 0;
						scrconfig->sg_DisplayWidth = 320;
						scrconfig->sg_DisplayHeight = 256;
						scrconfig->sg_DisplayDepth = 2;
						scrconfig->sg_OverscanType = 0;
						scrconfig->sg_AutoScroll = TRUE;
						scrconfig->sg_Shanghai = FALSE;
		 */
						scrconfig->sg_OwnScreen = FALSE;
						break;
				}
			}
		}
	}
	return(ret);
}

/**********************  CloseOurScreen  **********************************/



/* see CloseOurScreen(), below. */

BOOL OldCloseOurScreen(struct ScreenConfig *scrconfig)
{
	LONG locks = 0;
	int response = 0;

	SetShanghaiState(mainscreen,FALSE);

	if(ownscreen)
	{
		do
		{
			response = 0;
			if ( locks = GetScreenLocks(mainscreen) )
				response = ScreenLockedReq(locks);
		} while ( response == 1 );

		if (!locks)
		{
			if (mainscreen) CloseScreen(mainscreen);
			mainscreen = NULL;
			return(TRUE);
		}
		else
		{
			SetShanghaiState(mainscreen,scrconfig->sg_Shanghai);
			return(FALSE);
		}
	}
	else
		return(TRUE);
}



BOOL CloseOurScreen(struct ScreenConfig *scrconfig)
{
	BOOL ret;
	if(ret = OldCloseOurScreen(scrconfig) )
	{
			if(screentextfont) CloseFont(screentextfont);
			if(windowtextfont) CloseFont(windowtextfont);
			screentextfont = NULL;
			windowtextfont = NULL;
	}
	return(ret);
}



/****************************  Get Screen Locks *********************************/

LONG GetScreenLocks(struct Screen *scr)
{
struct List						*pubscreenlist;
struct PubScreenNode	*screennode;
WORD locks						= 0;

	if (scr)
	{
			/* Lock the list of public screens. */

		pubscreenlist = LockPubScreenList();

			/* Scan the list and try to find our private node. */

		for (	screennode = (struct PubScreenNode *)pubscreenlist->lh_Head ;
					( (screennode->psn_Node.ln_Succ) && (screennode->psn_Screen != scr) );
					screennode = (struct PubScreenNode *)screennode->psn_Node.ln_Succ) {}

		if ( screennode )
		{
				/* Okay, we know who and where we are, check the number of visitor windows
				 * currently open on our screen. */
			locks = screennode->psn_VisitorCount;
			UnlockPubScreenList();
		}
	}
	return ( (LONG)locks );
}

/**********************   GetPubScreen *************************************/
struct PubScreenNode *GetPubScreen(char *name)
{
struct List						*pubscreenlist;
struct PubScreenNode	*screennode;

	pubscreenlist = LockPubScreenList();
	screennode = (struct PubScreenNode *)FindName(pubscreenlist,name);
	UnlockPubScreenList();
	return(screennode);
}

/**************  UseNewScreenSettings()  **************/
//
// Reconfigures the global screen/font settings using the given
// ScreenConfig struct (which may then be freed).
//

BOOL UseNewScreenSettings(struct ScreenConfig *scrconfig)
{
	struct List *hidelist;
	BOOL ret = TRUE;

	/* hide and remember all the visible windows */
	if (hidelist = ListAndHideWindows() )
	{
		/* try and kill off the old screen */
		if ( CloseOurScreen(&glob.cg_ScrConfig) )
		{
			/* copy the screen config struct out to the global config */
			/* I _love_ ANSI C! */
			glob.cg_ScrConfig = *scrconfig;

			/* open the new screen with the new settings */
			if( !(ret = SetupScreen(&glob.cg_ScrConfig)) )
			{
				mainscreen = NULL;
			}
		}
		else
			GroovyReq( PROGNAME, "Couldn't Close Screen","Abort");

		/* bring back all those windows */
		ReshowWindowList(hidelist);

		/* update the revealwindows */
		RV_RemakeAll();
	}
	return(ret);
}

BOOL RemakeScreen(void)
{
	struct ScreenConfig sc;

	sc = glob.cg_ScrConfig;
	return( UseNewScreenSettings(&sc) );
}
