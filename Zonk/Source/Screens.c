/********************************************************/
//
// Screens.c
//
// Handles our public screen, "ZONK" (or "ZONK.n").
//
/********************************************************/


#define SCREENS_C

#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <math.h>

//#include <dos/dos.h>
//#include <dos/dostags.h>
#include <exec/exec.h>
#include <intuition/intuition.h>
//#include <graphics/gfxbase.h>
//#include <rexx/rxslib.h>
#include <libraries/gadtools.h>
#include <libraries/diskfont.h>
//#include <libraries/asl.h>
//#include "dh1:programming/stuff/include/libraries/asl.h"
//#include <libraries/iffparse.h>

#include <proto/exec.h>
//#include <proto/dos.h>
#include <proto/intuition.h>
#include <proto/graphics.h>
#include <proto/diskfont.h>
//#include <proto/asl.h>
#include <clib/alib_protos.h>
//#include <proto/gadtools.h>
//#include <proto/iffparse.h>

//#include <rexx/rexxmain.h>
//#include <rexx/RexxMain_rxcl.c>

#include "global.h"


//static struct Screen *gfxscreen = NULL;
static UBYTE gfxscreenname[MAXPUBSCREENNAME+1];
static struct Screen *gfxscreen = NULL;

/* Global vars */

struct TextFont *gfxscrfont = NULL;
struct TextFont *gfxwinfont = NULL;
struct TextFont *mainwinfont = NULL;

/* local protos */
static UBYTE *OpenGFXScreen( struct ScreenConfig *scfg );
static BOOL CloseGFXScreen( void );

void InstallPalette(struct Screen *scr, struct Palette *pal);
void SussDrawInfoPens( UWORD *pens, struct Palette *pal, UWORD numcols );
UWORD FindnBestPen( struct Palette *pal, UWORD numpens, UBYTE r, UBYTE g, UBYTE b );


static UBYTE *OpenGFXScreen( struct ScreenConfig *scfg )
{
	ULONG errorcode;
	static UWORD pens[NUMDRIPENS];

//	printf("OpenGFXScreen()...");

	if( scfg->scfg_GFXScrType != 1 )
		return( NULL );

	if( gfxscreen )
	{
		if( !CloseGFXScreen() )
			return( NULL );
	}

	SussDrawInfoPens( pens, &scfg->scfg_Palette,
		min( 1<<scfg->scfg_Depth, scfg->scfg_Palette.pl_Count ) );

	strcpy( gfxscreenname, "ZONK" );

	gfxscreen = OpenScreenTags( NULL,
										SA_Type,				PUBLICSCREEN,
										SA_PubName,			gfxscreenname,
										SA_Pens,				(ULONG)pens,
										SA_Width,				scfg->scfg_Width,
										SA_Height,			scfg->scfg_Height,
										SA_Depth,				scfg->scfg_Depth,
										SA_DisplayID,		scfg->scfg_DisplayID,
										SA_Font,				&scfg->scfg_GFXScrFont,
										SA_AutoScroll,	scfg->scfg_AutoScroll,
										SA_Interleaved,	TRUE,
										SA_ErrorCode,		&errorcode,
										TAG_DONE, NULL );

	if( gfxscreen )
	{
		PubScreenStatus( gfxscreen, 0 );
		InstallPalette( gfxscreen, &scfg->scfg_Palette );

	}
	else
	{
		switch( errorcode )
		{
			case OSERR_NOMONITOR:			/* named monitor spec not available	*/
				PlebReq( NULL, PROGNAME, "Monitor not available", "OK" );
				break;
			case OSERR_NOCHIPS:	   		/* you need newer custom chips		*/
				PlebReq( NULL, PROGNAME, "You need newer custom chips", "OK" );
				break;
			case OSERR_NOMEM:	   			/* couldn't get normal memory		*/
				PlebReq( NULL, PROGNAME, "Not enough memory", "OK" );
				break;
			case OSERR_NOCHIPMEM:			/* couldn't get chipmem			*/
				PlebReq( NULL, PROGNAME, "Not enough chip memory", "OK" );
				break;
			case OSERR_PUBNOTUNIQUE:	/* public screen name already used	*/
				PlebReq( NULL, PROGNAME, "Pubscreen name already used", "OK" );
				break;
			case OSERR_UNKNOWNMODE:		/* don't recognize mode asked for	*/
				PlebReq( NULL, PROGNAME, "Unknown screenmode", "OK" );
				break;
			case OSERR_TOODEEP:				/* Screen deeper than HW supports	*/
				PlebReq( NULL, PROGNAME, "Your hardware doesn't support that many colours", "OK" );
				break;
			case OSERR_ATTACHFAIL:		/* Failed to attach screens		*/
				break;
			case OSERR_NOTAVAILABLE:	/* Mode not available for other reason	*/
				PlebReq( NULL, PROGNAME, "Screenmode not available", "OK" );
				break;
		}
		return( NULL );
	}

//	printf("Done.\n");

	return( gfxscreenname );
}


static BOOL CloseGFXScreen( void )
{
	BOOL success = FALSE, abort = FALSE;

//	printf("CloseGFXScreen()\n");

	while( gfxscreen && !abort )
	{
		if( CloseScreen( gfxscreen ) )
		{
			gfxscreen = NULL;
			success = TRUE;
//			printf("-success\n");
		}
		else
		{
//			printf("-fail. Retry...\n");
			abort = !(BOOL)PlebReq( NULL, PROGNAME, "Can't close screen",
				"Retry|Cancel" );
		}
	}

	return( success );
}



BOOL InstallScreenConfig( struct ScreenConfig *scfg )
{
//	printf("installScreenConfig()\n");
	if( !UninstallScreenConfig() )
		return( FALSE );

	zonkfig.scfg = *scfg;

	/* open any fonts we need */

	/* install namestrings in textattr structs */

	zonkfig.scfg.scfg_GFXScrFont.ta_Name = zonkfig.scfg.scfg_GFXScrFontName;
	zonkfig.scfg.scfg_GFXWinFont.ta_Name = zonkfig.scfg.scfg_GFXWinFontName;
	zonkfig.scfg.scfg_MainWinFont.ta_Name = zonkfig.scfg.scfg_MainWinFontName;

	switch( zonkfig.scfg.scfg_GFXScrType )
	{
		/* 0 = no gfx screen */
		/* 1 = create new pubscreen */
		/* 2 = use existing pubscreen */
		case 1:
			gfxscrfont = OpenDiskFont( &zonkfig.scfg.scfg_GFXScrFont );
			/* fall through */
		case 2:
			gfxwinfont = OpenDiskFont( &zonkfig.scfg.scfg_GFXWinFont );
			break;
	}

	if( !zonkfig.scfg.scfg_DefaultFont )
		mainwinfont = OpenDiskFont( &zonkfig.scfg.scfg_MainWinFont );

	if( zonkfig.scfg.scfg_GFXScrType == 1 )
		OpenGFXScreen( &zonkfig.scfg );

	return( TRUE );

}


BOOL OldInstallScreenConfig( struct ScreenConfig *scfg )
{
//	printf("installScreenConfig()\n");
	if( !UninstallScreenConfig() )
		return( FALSE );

	/* open any fonts we need */

	/* install namestrings in textattr structs */

	scfg->scfg_GFXScrFont.ta_Name = scfg->scfg_GFXScrFontName;
	scfg->scfg_GFXWinFont.ta_Name = scfg->scfg_GFXWinFontName;
	scfg->scfg_MainWinFont.ta_Name = scfg->scfg_MainWinFontName;

	switch( scfg->scfg_GFXScrType )
	{
		/* 0 = no gfx screen */
		/* 1 = create new pubscreen */
		/* 2 = use existing pubscreen */
		case 1:
			gfxscrfont = OpenDiskFont( &scfg->scfg_GFXScrFont );
			/* fall through */
		case 2:
			gfxwinfont = OpenDiskFont( &scfg->scfg_GFXWinFont );
			break;
	}

	if( !scfg->scfg_DefaultFont )
		mainwinfont = OpenDiskFont( &scfg->scfg_MainWinFont );

	if( scfg->scfg_GFXScrType == 1 )
		OpenGFXScreen( scfg );

	zonkfig.scfg = *scfg;

	return( TRUE );

}



BOOL UninstallScreenConfig( void )
{
//	printf("UninstallScreenConfig()\n");
	if( gfxscreen )
	{
		if( !CloseGFXScreen() )
			return( FALSE );
	}
//	printf("-kill fonts\n");

	if( gfxscrfont ) CloseFont( gfxscrfont );
	if( gfxwinfont ) CloseFont( gfxwinfont );
	if( mainwinfont ) CloseFont( mainwinfont );
	gfxscrfont = NULL;
	gfxwinfont = NULL;
	mainwinfont = NULL;

	return(TRUE);
}




struct Screen *LockGFXScreen( void )
{
	struct Screen *scr;

	switch( zonkfig.scfg.scfg_GFXScrType )
	{
		case 2:					/* someone elses pubscreen */
			scr = LockPubScreen( zonkfig.scfg.scfg_GFXPubScrName );
			break;
		case 1:					/* our own pubscreen */
			scr = LockPubScreen( gfxscreenname );
			break;
		case 0:					/* no gfx screen */
		default:
			scr = NULL;
			break;
	}
	return( scr );
}


void UnlockGFXScreen( struct Screen *scr )
{
	switch( zonkfig.scfg.scfg_GFXScrType )
	{
		case 2:					/* someone elses pubscreen */
		case 1:					/* our own pubscreen */
			UnlockPubScreen( NULL, scr );
			break;
		case 0:					/* no gfx screen */
		default:
			break;
	}
}





struct Screen *LockMainScreen( void )
{
	struct Screen *scr;

	switch( zonkfig.scfg.scfg_MainScrType )
	{
		case 2:				/* someone elses public screen */
			scr = LockPubScreen( zonkfig.scfg.scfg_MainPubScrName );
			break;
		case 1:				/* gfx screen */
			scr = LockGFXScreen();
			break;
		case 0:				/* default publicscreen */
		default:
			scr = NULL;
			break;
	}
	return( scr );
}


void UnlockMainScreen( struct Screen *scr )
{
	switch( zonkfig.scfg.scfg_MainScrType )
	{
		case 2:				/* someone elses public screen */
			UnlockPubScreen( NULL, scr );
			break;
		case 1:				/* gfx screen */
			UnlockGFXScreen( scr );
			break;
		case 0:				/* default publicscreen */
		default:
			break;
	}
}



/*************  InstallPalette()  *************/
//
// Installs the given palette into a screen.
//

void InstallPalette(struct Screen *scr, struct Palette *pal)
{
	ULONG			gun;
	UWORD			*colourtable4;
	ULONG			*colourtable32;
	UWORD			count,tableoffset;

	if( GfxBase->LibNode.lib_Version >= 39 )
	{
		if( colourtable32 = AllocVec(2+2+((pal->pl_Count)*(4*3))+4,MEMF_ANY|MEMF_CLEAR) )
		{
			/* Copy the 8 bit colour table to the 32 bit table. */
			tableoffset=1;
			for(count=0; count<(pal->pl_Count); count++)
			{
				colourtable32[tableoffset++] = pal->pl_Colours[count].r;
				colourtable32[tableoffset++] = pal->pl_Colours[count].g;
				colourtable32[tableoffset++] = pal->pl_Colours[count].b;
			}
	
			/* Now scale to 32 bits */
			tableoffset = 1;
			count=pal->pl_Count;
			while(count--)
			{
				gun = colourtable32[tableoffset];
				colourtable32[tableoffset++] |= ((gun << 24) | (gun << 16) | (gun << 8));
				gun = colourtable32[tableoffset];
				colourtable32[tableoffset++] |= ((gun << 24) | (gun << 16) | (gun << 8));
				gun = colourtable32[tableoffset];
				colourtable32[tableoffset++] |= ((gun << 24) | (gun << 16) | (gun << 8));
			}
			colourtable32[0] = ((pal->pl_Count) << 16)+0;
			colourtable32[1+((pal->pl_Count)*3)]=0;
			LoadRGB32(&scr->ViewPort,colourtable32);
			FreeVec(colourtable32);
		}
	}
	else
	{
		/* If we get here, then we're not running V39 or greater, bummer, so we use
				the standard SetRGB4. */

		if( colourtable4 = AllocVec((pal->pl_Count)*2,MEMF_ANY|MEMF_CLEAR) )
		{
			for(count=0; count<(pal->pl_Count); count++)
				colourtable4[count] = ((pal->pl_Colours[count].r & 0xF0)<<4) |
															(pal->pl_Colours[count].g & 0xF0) |
															(pal->pl_Colours[count].b>>4);
			LoadRGB4(&scr->ViewPort,colourtable4,pal->pl_Count);
			FreeVec(colourtable4);
		}
	}
}

/*******************  FindnBestPen()  *******************/
//
// Go through a Palette looking for the closest matching
// entry to the given (24 bit) rgb value. Returns palette
// index. Can also potentially return 0xFFFF if stuff is
// seriously out to lunch.
//
// inputs:
//  pal = palette to pick from
//  numpens = how many pens available
//  r,g,b = desired colour
//
// Known bugs: matching method arrived at via complete
//             guesswork - crap results highly probable.

UWORD FindnBestPen( struct Palette *pal, UWORD numpens, UBYTE r, UBYTE g, UBYTE b )
{
	WORD i;
	UWORD bestpen = 0xFFFF, minpens;
	ULONG delta, bestdelta = 0xFFFFFFFF;

	minpens = (numpens < pal->pl_Count) ? numpens : pal->pl_Count;

	for( i = 0; i < minpens; i++)
	{
		delta = ( (r - pal->pl_Colours[i].r) * (r-pal->pl_Colours[i].r) ) +
						( (g - pal->pl_Colours[i].g) * (g-pal->pl_Colours[i].g) ) +
						( (b - pal->pl_Colours[i].b) * (b-pal->pl_Colours[i].b) );
		if( delta < bestdelta )
		{
			bestpen = i;
			bestdelta = delta;
		}
	}
	return(bestpen);
}


/********************  SussDrawInfoPens()  ********************/
//
// Sets up a drawinfo pen array by selecting colours in the
// given palette. Tries to keep things looking relatively sane,
// bas-relief and readable, but this really depends on the colours
// available. Uses different 'ideals' for dark, medium and bright
// background colours.
//
// params:
//  pens    = ptr to uninitialized pen array (+ room for 0xFFFF terminator)
//  pal     = palette to choose the colours from
//  numcols = number of colours to consider (ie num of colours on screen)

void SussDrawInfoPens( UWORD *pens, struct Palette *pal, UWORD numcols )
{
	//UBYTE *p;
	ULONG brightness;

	if(numcols == 2)
	{
		pens[SHINEPEN]  = FindnBestPen( pal, numcols, 0xFF, 0xFF, 0xFF);
		pens[SHADOWPEN] = FindnBestPen( pal, numcols, 0x00, 0x00, 0x00);
		pens[DETAILPEN] = pens[SHADOWPEN];
		pens[BLOCKPEN] = pens[SHINEPEN];
		pens[TEXTPEN] = pens[SHINEPEN];
		pens[FILLPEN] = pens[SHINEPEN];
		pens[FILLTEXTPEN] = pens[SHADOWPEN];
		pens[BACKGROUNDPEN] = 0xFFFF;
		return;
	}

	brightness = (pal->pl_Colours[0].r * pal->pl_Colours[0].r)+
							 (pal->pl_Colours[0].g * pal->pl_Colours[0].g)+
							 (pal->pl_Colours[0].b * pal->pl_Colours[0].b);

	if (brightness < 12288)
	{
		/* dark background */
		/* so avoid using dark colours */

		/* optimise some of this shit sometime... */
		pens[DETAILPEN] = FindnBestPen( pal, numcols, 255, 255, 255 ); //128
		pens[BLOCKPEN] = FindnBestPen( pal, numcols, 128, 128, 128 );
		pens[TEXTPEN] = FindnBestPen( pal, numcols, 255, 255, 255 ); //128
		pens[SHINEPEN]  = FindnBestPen( pal, numcols, 0xFF, 0xFF, 0xFF);
		pens[SHADOWPEN] = FindnBestPen( pal, numcols, 0x44, 0x44, 0x44);
		pens[FILLPEN] = FindnBestPen( pal, numcols, 102, 136, 187);
		pens[FILLTEXTPEN] = FindnBestPen( pal, numcols, 255, 255, 255); //128
		pens[BACKGROUNDPEN] = 0xFFFF;
	}
	else
	{
		if(brightness > 110592)
		{
			/* bright background */
			/* - avoid really bright colours */

			pens[DETAILPEN] = FindnBestPen( pal, numcols, 128, 128, 128 );
			pens[BLOCKPEN] = FindnBestPen( pal, numcols, 0, 0, 0 );
			pens[TEXTPEN] = FindnBestPen( pal, numcols, 0, 0, 0 );
			pens[SHINEPEN] = FindnBestPen( pal, numcols, 192, 192, 192);
			pens[SHADOWPEN] = FindnBestPen( pal, numcols, 128, 128, 128);
			pens[FILLPEN] = FindnBestPen( pal, numcols, 102, 136, 187);
			pens[FILLTEXTPEN] = FindnBestPen( pal, numcols, 0, 0, 0);
			pens[BACKGROUNDPEN] = 0xFFFF;
		}
		else
		{
			/* normal (workbenchesque) pen selections */
			/* aim for standard workbench default palette (almost) */

			pens[DETAILPEN] = FindnBestPen( pal, numcols, 128, 128, 128 );
			pens[BLOCKPEN] = FindnBestPen( pal, numcols, 0, 0, 0 );
			pens[TEXTPEN] = FindnBestPen( pal, numcols, 0, 0, 0 );
			pens[SHINEPEN] = FindnBestPen( pal, numcols, 255, 255, 255);
			pens[SHADOWPEN] = FindnBestPen( pal, numcols, 0, 0, 0);
			pens[FILLPEN] = FindnBestPen( pal, numcols, 102, 136, 187);
			pens[FILLTEXTPEN] = FindnBestPen( pal, numcols, 0, 0, 0);
			pens[BACKGROUNDPEN] = 0xFFFF;
		}
	}
}
