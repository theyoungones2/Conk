/*************************************************************************/
//
// HOOPYVISION
// - a fontsensitive, resizable, gadget layout and handling system.
// By Ben Campbell
//
/*************************************************************************/

#include <exec/exec.h>
#include <intuition/intuition.h>
#include <intuition/classes.h>
#include <intuition/classusr.h>
#include <intuition/imageclass.h>
#include <intuition/gadgetclass.h>
#include <intuition/sghooks.h>
#include <libraries/gadtools.h>
#include <utility/hooks.h>
#include <graphics/displayinfo.h>
#include <graphics/gfxbase.h>
#include <proto/exec.h>
#include <proto/dos.h>
#include <proto/diskfont.h>
#include <proto/intuition.h>
#include <proto/gadtools.h>
#include <proto/graphics.h>
#include <proto/utility.h>
#include <clib/alib_protos.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>
#include <ctype.h>

#include "Protos.h"

#include "Hoopy.h"
#define MYDEBUG 1
#include "mydebug.h"

/* require a TextFont struct for fallback to topaz */
extern struct TextFont *topaztextfont;

ULONG __saveds __asm PackedStringHook( register __a0 struct Hook *hookptr,
	register __a2 struct SGWork *work,
	register __a1	ULONG *msg );

ULONG __saveds __asm FixedPointHook( register __a0 struct Hook *hookptr,
	register __a2 struct SGWork *work,
	register __a1	ULONG *msg );

//LONG KPrintF( STRPTR fmt, ... );

BOOL OpenUp( void );
void ShutDown( void );
void main( void );

static UWORD HO_CountSkip( struct HoopyObject *hob );
static BOOL HO_MakeObject( struct HoopyObject *hob, struct HOStuff *hos,
	WORD left, WORD top, WORD width, WORD height );
static void AccomodateLabel( struct NewGadget *ng, struct TextFont *tf );
static BOOL InternalSussWindowSizing( struct HOStuff *hos,
	struct HoopyObject *hob );

static ULONG HO_FindMinHeight( struct HOStuff *hos,
	struct HoopyObject *hob, struct TextFont *tf );
static ULONG HO_FindMinWidth( struct HOStuff *hos,
	struct HoopyObject *hob, struct TextFont *tf );


/************************************************************************/


/******************  HO_CountSkip()  ******************/

static UWORD HO_CountSkip( struct HoopyObject *hob )
{
	UWORD i, skip=1;

	if( hob->ho_Type == HOTYPE_VGROUP ||
		hob->ho_Type == HOTYPE_HGROUP )
	{
		for( i=0; i < hob->ho_Attr0; i++ )
		{
			skip += HO_CountSkip( &hob[skip] );
		}
	}
	return( skip );
}


/******************  HO_FindMinHeight()  ******************/

static ULONG	HO_FindMinHeight( struct HOStuff *hos,
	struct HoopyObject *hob, struct TextFont *tf )
{
	ULONG temp,minh=0;
	UWORD i,j;

	switch( hob->ho_Type )
	{
		case HOTYPE_VGROUP:
			j = 1;
			for( i=0; i < hob->ho_Attr0; i++)
			{
				minh += HO_FindMinHeight( hos, &hob[j], tf );
				j += HO_CountSkip( &hob[j] );
			}
			break;

		case HOTYPE_HGROUP:
			j = 1;
			for( i=0; i < hob->ho_Attr0; i++)
			{
				temp = HO_FindMinHeight( hos, &hob[j], tf );
				if( temp > minh ) minh = temp;
				j += HO_CountSkip( &hob[j] );
			}
			break;

		case HOTYPE_TEXT:
		case HOTYPE_NUMBER:
		case HOTYPE_BUTTON:
		case HOTYPE_CYCLE:
			minh = tf->tf_YSize + 4;
			break;
		case HOTYPE_PALETTE:
			break;
		case HOTYPE_LISTVIEW:
			minh = ( tf->tf_YSize * 3 ) + 4;
			break;
		case HOTYPE_CHECKBOX:
//			minh = CHECKBOX_HEIGHT;
			minh = tf->tf_YSize + 2;
			if( minh < CHECKBOX_HEIGHT ) minh = CHECKBOX_HEIGHT;
			break;
		case HOTYPE_MX:
			minh = tf->tf_YSize + hob->ho_Attr1;
			if( minh < MX_HEIGHT )
				minh = MX_HEIGHT * CountStrings( (STRPTR *)hob->ho_Attr0 );
			else
				minh = ( minh * CountStrings( (STRPTR *)hob->ho_Attr0 ) ) - hob->ho_Attr1;
			break;
		case HOTYPE_INTEGER:
		case HOTYPE_STRING:
		case HOTYPE_FIXEDPOINT:
		case HOTYPE_PACKEDSTRING:
			minh = tf->tf_YSize + 4;
			break;

		case HOTYPE_SPACE:
			break;
		case HOTYPE_HSCROLLER:
		case HOTYPE_HSLIDER:
		case HOTYPE_SCROLLYSPACE:
			minh = hos->hos_ScrollyHeight;
			break;
		case HOTYPE_VSCROLLER:
			minh = hos->hos_ScrollyHeight*3;
			break;
		case HOTYPE_VSLIDER:
			minh = 8;
			break;
		case HOTYPE_IMAGEBUTTON:
			minh = 10;
			break;
		case HOTYPE_HLINE:
			if( hob->ho_Text )
				minh = tf->tf_YSize + 2;
			else
				minh = 2;
			break;
		case HOTYPE_VLINE:
			minh = 0;
			break;
	}
	if( hob->ho_Flags & PLACETEXT_ABOVE )
		minh += ( tf->tf_YSize + 4 );
	if( hob->ho_Flags & PLACETEXT_BELOW )
		minh += ( tf->tf_YSize + 4 );

	if( !( hob->ho_HOFlags & HOFLG_NOBORDER ) )
		minh += ( VSPACE * 2 );

	return( minh );
}

/******************  HO_FindMinWidth()  ******************/

static ULONG	HO_FindMinWidth( struct HOStuff *hos,
	struct HoopyObject *hob, struct TextFont *tf )
{
	ULONG temp,minw=0;
	UWORD i,j;

	switch( hob->ho_Type )
	{
		case HOTYPE_VGROUP:
			j = 1;
			for( i=0; i < hob->ho_Attr0; i++)
			{
				temp = HO_FindMinWidth( hos, &hob[j], tf );
				if( temp > minw ) minw = temp;
				j += HO_CountSkip( &hob[j] );
			}
			break;

		case HOTYPE_HGROUP:
			j = 1;
			for( i=0; i < hob->ho_Attr0; i++)
			{
				minw += HO_FindMinWidth( hos, &hob[j], tf );
				j += HO_CountSkip( &hob[j] );
			}
			break;

		case HOTYPE_TEXT:
//			minw = MyTextLength( tf, (STRPTR)hob->ho_Attr0 ) + 8;
			minw = MyTextLength( tf, "WW" ) + 8;
			break;
		case HOTYPE_BUTTON:
			if( hob->ho_Text )
				minw = MyTextLength( tf, hob->ho_Text ) + 8;
			else
			{
				D(bug("Button text zilch! - ENFORCER time!\n") );
				minw = 16;
			}
			break;
		case HOTYPE_LISTVIEW:
			minw = 64;
			break;
		case HOTYPE_PALETTE:
			break;
		case HOTYPE_CYCLE:
			minw = FindMaxStrLen( (STRPTR *)hob->ho_Attr0, tf ) + 8 + 32;
			break;
		case HOTYPE_MX:
			minw = MX_WIDTH + FindMaxStrLen( (STRPTR *)hob->ho_Attr0, tf ) + 8;
			break;
		case HOTYPE_CHECKBOX:
			minw = CHECKBOX_WIDTH;
			break;
		case HOTYPE_NUMBER:
			minw = MyTextLength( tf, "00" ) + 16;
			break;
		case HOTYPE_INTEGER:
		case HOTYPE_FIXEDPOINT:
//			minw = ( MyTextLength( tf, "0" ) * ( hob->ho_Attr0 + 1 )) + 16;
			minw = (MyTextLength( tf, "0" ) * (hob->ho_Attr0 + 1 ) ) + 16;
			break;
		case HOTYPE_PACKEDSTRING:
		case HOTYPE_STRING:
			if( hob->ho_HOFlags & HOFLG_FREEWIDTH )
				minw = MyTextLength( tf, "WW" ) + 16;
			else
				minw = (MyTextLength( tf, "M" ) * (hob->ho_Attr1 + 1 ) ) + 16;
			break;
		case HOTYPE_SPACE:
			break;
		case HOTYPE_HSCROLLER:
			minw = hos->hos_ScrollyWidth*3;
			break;
		case HOTYPE_HSLIDER:
			minw = 8;
			break;
		case HOTYPE_VSCROLLER:
		case HOTYPE_VSLIDER:
		case HOTYPE_SCROLLYSPACE:
			minw = hos->hos_ScrollyWidth;
			break;

		case HOTYPE_IMAGEBUTTON:
			minw = 24;
			break;
		case HOTYPE_HLINE:
			if( hob->ho_Text )
				minw = MyTextLength( tf, hob->ho_Text ) + 8;
			else
				minw = 0;
			break;
		case HOTYPE_VLINE:
			minw = 4;
			break;
	}

	if( hob->ho_Text )
	{
		if( hob->ho_Flags & PLACETEXT_LEFT )
			minw += ( MyTextLength( tf, hob->ho_Text ) + 16 );
		if( hob->ho_Flags & PLACETEXT_RIGHT )
			minw += ( MyTextLength( tf, hob->ho_Text ) + 16 );
	}

	if( !( hob->ho_HOFlags & HOFLG_NOBORDER ) )
		minw += (HSPACE*2);

	return( minw );
}


/******************  HO_FindOptimalHeight()  ******************/

ULONG	HO_FindOptimalHeight( struct HOStuff *hos,
	struct HoopyObject *hob, struct TextFont *tf )
{
	ULONG temp, opth=0;
	UWORD i,j;

	if( hob->ho_HOFlags & HOFLG_FREEHEIGHT )
		return( ~0 );

	switch( hob->ho_Type )
	{
		case HOTYPE_VGROUP:
			j = 1;
			for( i=0; ( i < hob->ho_Attr0 ) && ( opth != ~0 ); i++)
			{
				temp = HO_FindOptimalHeight( hos, &hob[j], tf );
				if( temp == ~0 )
					opth = ~0;
				else
					opth += temp;
				j += HO_CountSkip( &hob[j] );
			}
			break;

		case HOTYPE_HGROUP:
			j = 1;
			for( i=0; ( i < hob->ho_Attr0 ) && ( opth != ~0 ); i++)
			{
				temp = HO_FindOptimalHeight( hos, &hob[j], tf );
				if( temp == ~0 )
					opth = ~0;
				else
					if( temp > opth ) opth = temp;
				j += HO_CountSkip( &hob[j] );
			}
			break;

		case HOTYPE_TEXT:
		case HOTYPE_BUTTON:
		case HOTYPE_NUMBER:
		case HOTYPE_CYCLE:
			opth = tf->tf_YSize + 4;
			break;
		case HOTYPE_HSCROLLER:
		case HOTYPE_HSLIDER:
		case HOTYPE_SCROLLYSPACE:
			opth = hos->hos_ScrollyHeight;
			break;
		case HOTYPE_VSCROLLER:
		case HOTYPE_VSLIDER:
			opth = ~0;
			break;
		case HOTYPE_SPACE:
			opth = 0;
			break;
		case HOTYPE_LISTVIEW:
			opth = ~0;
			break;
		case HOTYPE_CHECKBOX:
			opth = tf->tf_YSize + 2;
			if( opth < CHECKBOX_HEIGHT ) opth = CHECKBOX_HEIGHT;
			break;
		case HOTYPE_MX:
			opth = tf->tf_YSize + hob->ho_Attr1;
			if( opth < MX_HEIGHT )
				opth = MX_HEIGHT * CountStrings( (STRPTR *)hob->ho_Attr0 );
			else
				opth = ( opth * CountStrings( (STRPTR *)hob->ho_Attr0 ) ) - hob->ho_Attr1;
			break;
		case HOTYPE_INTEGER:
		case HOTYPE_STRING:
		case HOTYPE_FIXEDPOINT:
		case HOTYPE_PACKEDSTRING:
			opth = tf->tf_YSize + 4;
			break;
		case HOTYPE_PALETTE:
			break;
		case HOTYPE_IMAGEBUTTON:
			opth = 10;
			break;
		case HOTYPE_HLINE:
			if( hob->ho_Text )
				opth = tf->tf_YSize + 2;
			else
				opth = 2;
			break;
		case HOTYPE_VLINE:
//			opth = ~0;
			opth = 0;
			break;
	}

	/* modify to allow for label */

	if( (opth != ~0) && (hob->ho_Flags & PLACETEXT_ABOVE) )
		opth += ( tf->tf_YSize + 4 );
	if( (opth != ~0) && (hob->ho_Flags & PLACETEXT_BELOW) )
		opth += ( tf->tf_YSize + 4 );

	if( ( opth != ~0 )	&& !( hob->ho_HOFlags & HOFLG_NOBORDER ) )
		opth += ( VSPACE * 2 );

	return( opth );
}



/******************  HO_FindOptimalWidth()  ******************/

ULONG	HO_FindOptimalWidth( struct HOStuff *hos,
	struct HoopyObject *hob, struct TextFont *tf )
{
	ULONG temp, optw=0;
	UWORD i,j;

	if( hob->ho_HOFlags & HOFLG_FREEWIDTH )
		return( ~0 );

	switch( hob->ho_Type )
	{
		case HOTYPE_VGROUP:
			j = 1;
			for( i=0; ( i < hob->ho_Attr0 ) && ( optw != ~0 ); i++)
			{
				temp = HO_FindOptimalWidth( hos, &hob[j], tf );
				if( temp == ~0 )
					optw = ~0;
				else
					if( temp > optw ) optw = temp;
				j += HO_CountSkip( &hob[j] );
			}
			break;

		case HOTYPE_HGROUP:
			j = 1;
			for( i=0; ( i < hob->ho_Attr0 ) && ( optw != ~0 ); i++)
			{
				temp = HO_FindOptimalWidth( hos, &hob[j], tf );
				if( temp == ~0 )
					optw = ~0;
				else
					optw += temp;
				j += HO_CountSkip( &hob[j] );
			}
			break;

		case HOTYPE_TEXT:
			optw = MyTextLength( tf, (STRPTR)hob->ho_Attr0 ) + 8;
			break;
		case HOTYPE_BUTTON:
			optw = MyTextLength( tf, hob->ho_Text ) + 8;
			break;
		case HOTYPE_CYCLE:
			optw = FindMaxStrLen( (STRPTR *)hob->ho_Attr0, tf ) + 8 + 32;
			break;

		case HOTYPE_SPACE:
			optw = 0;
			break;
		case HOTYPE_LISTVIEW:
		case HOTYPE_NUMBER:
			optw = ~0;
			break;
		case HOTYPE_CHECKBOX:
			optw = CHECKBOX_WIDTH;
			break;
		case HOTYPE_MX:
			optw = MX_WIDTH + FindMaxStrLen( (STRPTR *)hob->ho_Attr0, tf ) + 8;
			break;
		case HOTYPE_INTEGER:
		case HOTYPE_FIXEDPOINT:
			optw = (MyTextLength( tf, "0" ) * (hob->ho_Attr0 + 1 ) ) + 16;
			break;
		case HOTYPE_STRING:
		case HOTYPE_PACKEDSTRING:
			optw = (MyTextLength( tf, "M" ) * (hob->ho_Attr1 + 1 ) ) + 16;
//			optw = ~0;
			break;
		case HOTYPE_PALETTE:
			break;
		case HOTYPE_VSCROLLER:
		case HOTYPE_VSLIDER:
		case HOTYPE_SCROLLYSPACE:
			optw = hos->hos_ScrollyWidth;
			break;
		case HOTYPE_HSCROLLER:
		case HOTYPE_HSLIDER:
			optw = ~0;
			break;
		case HOTYPE_IMAGEBUTTON:
			optw = 24;
			break;
		case HOTYPE_HLINE:
			optw = ~0;
			break;
		case HOTYPE_VLINE:
			optw = 4;
			break;
	}

	if( hob->ho_Text )
	{
		if( (optw != ~0) && (hob->ho_Flags & PLACETEXT_LEFT) )
			optw += ( MyTextLength( tf, hob->ho_Text ) + 16 );
		if( (optw != ~0) && (hob->ho_Flags & PLACETEXT_RIGHT) )
			optw += ( MyTextLength( tf, hob->ho_Text ) + 16 );
	}

	if( ( optw != ~0 ) && !( hob->ho_HOFlags & HOFLG_NOBORDER ) )
		optw += ( HSPACE * 2 );

	return( optw );
}



static struct Gadget *gad;

/******************  HO_MakeLayout()  ******************/

BOOL HO_MakeLayout( struct HoopyObject *hob, struct HOStuff *hos,
	WORD left, WORD top, WORD width, WORD height )
{
	BOOL success = FALSE;

	/* first, kill any old gadgets that may be active */
	HO_KillLayout( hos );

//	ClearWindow( hos->hos_Win );

	/* init gadget stuff */
	gad = CreateContext(&hos->hos_GList);

	if( !( HO_MakeObject( hob, hos, left, top, width, height )  ) )
	{
		FreeGadgets( hos->hos_GList);
		hos->hos_GList = NULL;
	}
	else
	{

		/* attach our groovy new gadgets to the window */
		AddGList( hos->hos_Win, hos->hos_GList, -1, -1, NULL );

		/* activate first string/int gad if any */
//		HO_ActivateNextStringGad( hos->hos_Win, hob );

		GT_RefreshWindow( hos->hos_Win, NULL );

		/* need to redraw the frame coz we're not using GimmeZeroZero */
		RefreshWindowFrame( hos->hos_Win);
		success = TRUE;
	}
	return( success );
}



static void AccomodateLabel( struct NewGadget *ng, struct TextFont *tf )
{
	WORD temp;

	if( ng->ng_GadgetText )
	{

		if( ng->ng_Flags & PLACETEXT_LEFT )
		{
			temp = MyTextLength( tf, ng->ng_GadgetText ) + 16;
			ng->ng_LeftEdge += temp;
			ng->ng_Width -= temp;
		}

		if( ng->ng_Flags & PLACETEXT_RIGHT )
			ng->ng_Width -= ( MyTextLength( tf, ng->ng_GadgetText ) + 16 );

		if( ng->ng_Flags & PLACETEXT_ABOVE )
		{
			ng->ng_TopEdge += tf->tf_YSize + 4;
			ng->ng_Height -= (tf->tf_YSize + 4);
		}

		if( ng->ng_Flags & PLACETEXT_BELOW )
			ng->ng_Height -= (tf->tf_YSize + 4);
	}
}


/******************  HO_MakeObject()  ******************/

static BOOL HO_MakeObject( struct HoopyObject *hob, struct HOStuff *hos,
	WORD left, WORD top, WORD width, WORD height )
{
	UWORD i, j;
	WORD x,y, foo;
	BOOL done,breakflg,success = FALSE;
	LONG	size, temp, wtotal, freespace;
	struct TextFont *tf;
	static struct NewGadget ng;
	char strbuf[128];

	static struct Hook fixedpointhook =
		{ {NULL}, (ULONG (*)())FixedPointHook, NULL, NULL };
	static struct Hook packedstringhook =
		{ {NULL}, (ULONG (*)())PackedStringHook, NULL, NULL };

	tf = hos->hos_tf;

//	printf("HO_MakeObject(), id: %d l: %d t: %d w: %d, h: %d\n",
//		hob->ho_GadgetID,left,top,width,height);

	if( !( hob->ho_HOFlags & HOFLG_NOBORDER ) )
	{
		if( hob->ho_HOFlags & HOFLG_RAISEDBORDER )
			DrawBevelBox( hos->hos_RPort, left, top, width, height,
				GT_VisualInfo, hos->hos_vi, TAG_DONE );
		if( hob->ho_HOFlags & HOFLG_RECESSBORDER )
			DrawBevelBox( hos->hos_RPort, left, top, width, height,
				GT_VisualInfo, hos->hos_vi,
				GTBB_Recessed, TRUE,
				TAG_DONE );

		top += VSPACE;
		height -= ( VSPACE * 2 );
		left += HSPACE;
		width -= ( HSPACE * 2 );
	}

	ng.ng_LeftEdge = left;
	ng.ng_TopEdge = top;
	ng.ng_Width = width;
	ng.ng_Height = height;
	ng.ng_GadgetText = hob->ho_Text;
	ng.ng_TextAttr = &hos->hos_TextAttr;
	ng.ng_GadgetID = hob->ho_GadgetID;
	ng.ng_Flags = hob->ho_Flags;
	ng.ng_VisualInfo = hos->hos_vi;
	ng.ng_UserData = hob;

	switch( hob->ho_Type )
	{
		case HOTYPE_VGROUP:
//			KPrintF("VGroup\n");
			/* 1st pass - tag fixed size items */

			j = 1;
			for( i=0; i < hob->ho_Attr0; i++)
			{
				temp = HO_FindOptimalHeight( hos, &hob[j], tf );

				if( temp != ~0 )
				{
					/* fixed size object - tag it and store its size */
					hob[j].ho_HOFlags |= HOFLG_SIZESUSSED;
					hob[j].ho_Size = temp;
				}
				else
				{
					/* size not yet decided - clear the sizesussed flag */
					hob[j].ho_HOFlags &= ~HOFLG_SIZESUSSED;
				}
				j += HO_CountSkip( &hob[j] );								/* next object... */
			}

			/* gotta keep going through until all items have been fixed in place... */

			done = FALSE;
			while( !done )
			{
				/* nth pass - calculate the unallocated space and weight total */
				freespace = height;
				wtotal = 0;
				j = 1;
				for( i=0; i < hob->ho_Attr0; i++)
				{
					if( hob[j].ho_HOFlags & HOFLG_SIZESUSSED )
						freespace -= hob[j].ho_Size;
					else
						wtotal += hob[j].ho_Weight;
					j += HO_CountSkip( &hob[j] );								/* next object... */
				}

				/* (n+1)th pass - try and suss sizes of remaining objects */
				j = 1;
				breakflg = FALSE;
				for( i=0; (i < hob->ho_Attr0) && (!breakflg); i++)
				{
					if( !(hob[j].ho_HOFlags & HOFLG_SIZESUSSED) )
					{
						/* 'tis an unfixed object... so calc what its size should be... */
						size = ( hob[j].ho_Weight * freespace ) / wtotal;
						hob[j].ho_Size = size;

						/* check size is big enuff */
						temp = HO_FindMinHeight( hos, &hob[j], tf );
						if( size < temp )
						{
							/* too small! - fix at minimum size and go back to try again */
							hob[j].ho_Size = temp;
							hob[j].ho_HOFlags |= HOFLG_SIZESUSSED;
							breakflg = TRUE;
						}
					}
					j += HO_CountSkip( &hob[j] );								/* next object... */
				}
				/* if we went through 'em all without breaking then we're done! */
				if( !breakflg ) done = TRUE;
			}

			/* Last pass - make objects */
			y=0;
			j = 1;
			for( i=0; i < hob->ho_Attr0; i++)
			{
				temp = hob[j].ho_Size;
				success = HO_MakeObject( &hob[j], hos, left, top+y, width, temp );
				y += temp;
				j += HO_CountSkip( &hob[j] );								/* next object... */
			}
			break;


		case HOTYPE_HGROUP:
//			KPrintF("HGroup\n");
			/* 1st pass - tag fixed size items */

			j = 1;
			for( i=0; i < hob->ho_Attr0; i++)
			{
				temp = HO_FindOptimalWidth( hos, &hob[j], tf );
				if( temp != ~0 )
				{
					/* fixed size object - tag it and store its size */
					hob[j].ho_HOFlags |= HOFLG_SIZESUSSED;
					hob[j].ho_Size = temp;
				}
				else
				{
					/* size not yet decided - clear the sizesussed flag */
					hob[j].ho_HOFlags &= ~HOFLG_SIZESUSSED;
				}
				j += HO_CountSkip( &hob[j] );								/* next object... */
			}

			/* gotta keep going through until all items have been fixed in place... */

			done = FALSE;
			while( !done )
			{
				/* nth pass - calculate the unallocated space and weight total */
				freespace = width;
				wtotal = 0;
				j = 1;
				for( i=0; i < hob->ho_Attr0; i++)
				{
					if( hob[j].ho_HOFlags & HOFLG_SIZESUSSED )
						freespace -= hob[j].ho_Size;
					else
						wtotal += hob[j].ho_Weight;
					j += HO_CountSkip( &hob[j] );								/* next object... */
				}

//				printf( "  freespace: %ld, wtotal: %ld\n", freespace, wtotal );

				/* (n+1)th pass - try and suss sizes of remaining objects */
				j = 1;
				breakflg = FALSE;
				for( i=0; (i < hob->ho_Attr0) && (!breakflg); i++)
				{
					if( !(hob[j].ho_HOFlags & HOFLG_SIZESUSSED) )
					{
						/* 'tis an unfixed object... so calc what its size should be... */
						size = ( hob[j].ho_Weight * freespace ) / wtotal;
						hob[j].ho_Size = size;

						/* check size is big enuff */
						temp = HO_FindMinWidth( hos, &hob[j], tf );
						if( size < temp )
						{
							/* too small! - fix at minimum size and go back to try again */
							hob[j].ho_Size = temp;
							hob[j].ho_HOFlags |= HOFLG_SIZESUSSED;
							breakflg = TRUE;
						}
					}
					j += HO_CountSkip( &hob[j] );								/* next object... */
				}
				/* if we went through 'em all without breaking then we're done! */
				if( !breakflg ) done = TRUE;
			}

			/* Last pass - make objects */
			x=0;
			j = 1;
			for( i=0; i < hob->ho_Attr0; i++)
			{
				temp = hob[j].ho_Size;
				success = HO_MakeObject( &hob[j], hos, left + x, top, temp, height );
				x += temp;
				j += HO_CountSkip( &hob[j] );								/* next object... */
			}
			break;


		case HOTYPE_NUMBER:
//			KPrintF("Number\n");

			AccomodateLabel( &ng, tf );
			if( gad = CreateGadget( NUMBER_KIND, gad, &ng,
				GTNM_Number, hob->ho_Attr0,
				GTNM_Border, TRUE, TAG_END ) )
			{
				hob->ho_Gadget = gad;
				success = TRUE;
			}
			break;
		case HOTYPE_TEXT:
//			KPrintF("Text\n");

			AccomodateLabel( &ng, tf );
			if( gad = CreateGadget( TEXT_KIND, gad, &ng,
				GTTX_Text, hob->ho_Attr0,
				GTTX_Border, TRUE, TAG_END ) )
			{
				hob->ho_Gadget = gad;
				success = TRUE;
			}
			break;
		case HOTYPE_BUTTON:
//			KPrintF("Button\n");
			if( !( hob->ho_HOFlags & HOFLG_FREEWIDTH ) )
				ng.ng_Width = MyTextLength( tf, hob->ho_Text ) + 8;
			if( !( hob->ho_HOFlags & HOFLG_FREEHEIGHT ) )
				ng.ng_Height = tf->tf_YSize + 4;
			AccomodateLabel( &ng, tf );
			if( gad = CreateGadget( BUTTON_KIND, gad, &ng,
				GA_Disabled, hob->ho_HOFlags & HOFLG_DISABLED, TAG_END ) )
			{
				hob->ho_Gadget = gad;
				success = TRUE;
			}
			break;
		case HOTYPE_LISTVIEW:
//			KPrintF("ListView\n");
			AccomodateLabel( &ng, tf );
			if( gad = CreateGadget( LISTVIEW_KIND, gad, &ng,
				GTLV_Labels, hob->ho_Attr0,
				GTLV_ShowSelected, NULL,	//hob->ho_Attr1,
				GTLV_Selected, hob->ho_Value,
				GTLV_ReadOnly, hob->ho_Attr1,
				GA_Disabled, hob->ho_HOFlags & HOFLG_DISABLED,
				TAG_END ) )
			{
				hob->ho_Gadget = gad;
				success = TRUE;
			}
			break;
		case HOTYPE_PALETTE:
//			AccomodateLabel( &ng, tf );
//			if( gad = CreateGadget( PALETTE_KIND, gad, &ng,
//				GTPA_Depth, 3,
//				TAG_END ) )
//				success = TRUE;
			break;
		case HOTYPE_CYCLE:
//			KPrintF("Cycle\n");

			AccomodateLabel( &ng, tf );
			if( gad = CreateGadget( CYCLE_KIND, gad, &ng,
				GTCY_Labels, hob->ho_Attr0,
				GTCY_Active, hob->ho_Value,
				GA_Disabled, hob->ho_HOFlags & HOFLG_DISABLED,
				TAG_END ) )
			{
				hob->ho_Gadget = gad;
				success = TRUE;
			}
			break;
		case HOTYPE_CHECKBOX:
//			KPrintF("CheckBox\n");
			AccomodateLabel( &ng, tf );
			if( gad = CreateGadget( CHECKBOX_KIND, gad, &ng,
				GTCB_Checked, hob->ho_Value,
				GA_Disabled, hob->ho_HOFlags & HOFLG_DISABLED,
				TAG_END ) )
			{
				hob->ho_Gadget = gad;
				success = TRUE;
			}
			break;
		case HOTYPE_MX:
//			KPrintF("MX\n");
			ng.ng_Width = MX_WIDTH;
			ng.ng_Height = MX_HEIGHT;
			ng.ng_GadgetText = NULL;
			ng.ng_Flags |= PLACETEXT_RIGHT;

			temp = hob->ho_Attr1;
			if( ( temp + tf->tf_YSize ) < MX_HEIGHT )
				temp = MX_HEIGHT - tf->tf_YSize;

			if( gad = CreateGadget( MX_KIND, gad, &ng,
				GTMX_Labels, hob->ho_Attr0,
				GTMX_Spacing, temp,			//hob->ho_Attr1,
				GTMX_Active, hob->ho_Value,
				GA_Disabled, hob->ho_HOFlags & HOFLG_DISABLED,
				TAG_END ) )
			{
				hob->ho_Gadget = gad;
				success = TRUE;
			}
			break;
		case HOTYPE_STRING:
			AccomodateLabel( &ng, tf );
			if( gad = CreateGadget( STRING_KIND, gad, &ng,
				GTST_String, hob->ho_Attr0,
				GTST_MaxChars, hob->ho_Attr1,
				GA_TabCycle, TRUE,
				GA_Disabled, hob->ho_HOFlags & HOFLG_DISABLED,
				TAG_END ) )
			{
				hob->ho_Gadget = gad;
				success = TRUE;
			}
			break;
		case HOTYPE_PACKEDSTRING:
			AccomodateLabel( &ng, tf );
			if( gad = CreateGadget( STRING_KIND, gad, &ng,
				GTST_String, hob->ho_Attr0,
				GTST_MaxChars, hob->ho_Attr1,
				GTST_EditHook, &packedstringhook,
				GA_TabCycle, TRUE,
				GA_Disabled, hob->ho_HOFlags & HOFLG_DISABLED,
				TAG_END ) )
			{
				hob->ho_Gadget = gad;
				success = TRUE;
			}
			break;
		case HOTYPE_FIXEDPOINT:
			AccomodateLabel( &ng, tf );
			FixedPointToString( hob->ho_Value, strbuf );
			if( gad = CreateGadget( STRING_KIND, gad, &ng,
				GTST_String, strbuf,
				GTST_MaxChars, hob->ho_Attr0,
				GTST_EditHook, &fixedpointhook,
				GA_TabCycle, TRUE,
				GA_Disabled, hob->ho_HOFlags & HOFLG_DISABLED,
				TAG_END ) )
			{
				hob->ho_Gadget = gad;
				success = TRUE;
			}
			break;
		case HOTYPE_INTEGER:
			AccomodateLabel( &ng, tf );
			if( gad = CreateGadget( INTEGER_KIND, gad, &ng,
				GTIN_Number, hob->ho_Value,
				GTIN_MaxChars, hob->ho_Attr0,
				GA_TabCycle, TRUE,
				GA_Disabled, hob->ho_HOFlags & HOFLG_DISABLED,
				TAG_END ) )
			{
				hob->ho_Gadget = gad;
				success = TRUE;
			}
			break;
		case HOTYPE_SCROLLYSPACE:
//			KPrintF("Scrolly");
		case HOTYPE_SPACE:
//			KPrintF("Space\n");
			hob->ho_Attr0 = ((ULONG)left)<<16 | (ULONG)top;
			hob->ho_Attr1 = ((ULONG)width)<<16 | (ULONG)height;
			success = TRUE;
			break;

		case HOTYPE_HSCROLLER:
			AccomodateLabel( &ng, tf );

			if( gad = CreateGadget( SCROLLER_KIND, gad, &ng,
				GA_Disabled, hob->ho_HOFlags & HOFLG_DISABLED,
				GTSC_Visible, hob->ho_Attr0,
				GTSC_Total, hob->ho_Attr1,
				GTSC_Top, hob->ho_Value,
				GTSC_Arrows,hos->hos_ScrollyWidth,
				GA_RelVerify,TRUE,
				PGA_FREEDOM, LORIENT_HORIZ,
				TAG_END ) )
			{
				hob->ho_Gadget = gad;
				success = TRUE;
			}
			break;
		case HOTYPE_VSCROLLER:
			AccomodateLabel( &ng, tf );
			if( gad = CreateGadget( SCROLLER_KIND, gad, &ng,
				GA_Disabled, hob->ho_HOFlags & HOFLG_DISABLED,
				GTSC_Visible, hob->ho_Attr0,
				GTSC_Total, hob->ho_Attr1,
				GTSC_Top, hob->ho_Value,
				GTSC_Arrows,hos->hos_ScrollyHeight,
				GA_RelVerify,TRUE,
				PGA_FREEDOM, LORIENT_VERT,
				TAG_END ) )
			{
				hob->ho_Gadget = gad;
				success = TRUE;
			}
			break;

		case HOTYPE_HSLIDER:
			AccomodateLabel( &ng, tf );
			ng.ng_LeftEdge += 16;
			if( gad = CreateGadget( SLIDER_KIND, gad, &ng,
				GA_Disabled, hob->ho_HOFlags & HOFLG_DISABLED,
				GTSL_Min, hob->ho_Attr0,
				GTSL_Max, hob->ho_Attr1,
				GTSL_Level, hob->ho_Value,
				GTSL_LevelPlace, PLACETEXT_LEFT,
				GTSL_LevelFormat, "%ld",
				GTSL_MaxLevelLen, 2,
				PGA_FREEDOM, LORIENT_HORIZ,
				TAG_END ) )
			{
				hob->ho_Gadget = gad;
				success = TRUE;
			}
			break;

		case HOTYPE_VSLIDER:
			AccomodateLabel( &ng, tf );
			if( gad = CreateGadget( SLIDER_KIND, gad, &ng,
				GA_Disabled, hob->ho_HOFlags & HOFLG_DISABLED,
				GTSL_Min, hob->ho_Attr0,
				GTSL_Max, hob->ho_Attr1,
				GTSL_Level, hob->ho_Value,
				PGA_FREEDOM, LORIENT_VERT,
				TAG_END ) )
			{
				hob->ho_Gadget = gad;
				success = TRUE;
			}
			break;

		case HOTYPE_IMAGEBUTTON:
			if( gad = CreateGadget( GENERIC_KIND, gad, &ng,
				TAG_END ) )
			{
				hob->ho_Gadget = gad;
				success = TRUE;
				gad->GadgetType |=		GTYP_BOOLGADGET;
				gad->Flags |=					GFLG_GADGHIMAGE | GFLG_GADGIMAGE;
				gad->Activation =			GACT_RELVERIFY;
				gad->GadgetRender =		(APTR)hob->ho_Attr0;
				gad->SelectRender =		(APTR)hob->ho_Attr1;
				gad->MutualExclude =	NULL;
				gad->SpecialInfo =		NULL;
			}
			break;
		case HOTYPE_HLINE:
//			KPrintF("HLine\n");
			if( width > 8 )
			{

				if( hob->ho_Text )
				{
					foo = MyTextLength( tf, hob->ho_Text );

					left += width/2 - foo/2;
					top += ( tf->tf_Baseline );

					SetAPen( hos->hos_RPort, hos->hos_dri->dri_Pens[ HIGHLIGHTTEXTPEN ] );
					Move( hos->hos_RPort, left, top );
					Text( hos->hos_RPort, hob->ho_Text, strlen( hob->ho_Text ) );
				}
				else
				{
					top += ( height - 2 ) / 2;

					DrawBevelBox( hos->hos_RPort, left, top, width, 2,
						GTBB_Recessed, TRUE,
						GT_VisualInfo, hos->hos_vi, TAG_DONE );
					success = TRUE;
				}
			}
			break;
		case HOTYPE_VLINE:
//			KPrintF("VLine\n");
			if( height > 0 )
			{
//				SetAPen( hos->hos_RPort, 1 );
//				Move( hos->hos_RPort, left, top );
//				Draw( hos->hos_RPort, left, top + height );
				left += ( width - 4 ) / 2;
				DrawBevelBox( hos->hos_RPort, left, top, 4, height,
					GTBB_Recessed, TRUE,
					GT_VisualInfo, hos->hos_vi, TAG_DONE );
				success = TRUE;
			}
			break;
	}

	return( success );
}


/******************  HO_GetHOStuff()  ******************/

struct HOStuff *HO_GetHOStuff( struct Window *win, struct TextFont *tf )
{
	struct HOStuff *hos;
	BOOL error = FALSE;

	if( hos = AllocVec( sizeof( struct HOStuff ), MEMF_ANY|MEMF_CLEAR ) )
	{
		if( hos->hos_vi = GetVisualInfo( win->WScreen, TAG_DONE ) )
		{
			if( hos->hos_dri = GetScreenDrawInfo( win->WScreen ) )
			{
				hos->hos_Win = win;
				hos->hos_RPort = win->RPort;
				hos->hos_GList	= NULL;
				hos->hos_tf = tf;
				hos->hos_TextAttr.ta_Name = tf->tf_Message.mn_Node.ln_Name;
				hos->hos_TextAttr.ta_YSize = tf->tf_YSize;
				hos->hos_TextAttr.ta_Style = tf->tf_Style;
				hos->hos_TextAttr.ta_Flags = tf->tf_Flags;

				if( win->WScreen->Width >= 600 )
				{
					hos->hos_BigHoriz = TRUE;
					hos->hos_ScrollyWidth = 16;
				}
				else
				{
					hos->hos_BigHoriz = FALSE;
					hos->hos_ScrollyWidth = 14;
				}

				if( win->WScreen->Height >= 380 )
				{
					hos->hos_BigVert = TRUE;
					hos->hos_ScrollyHeight = 12;
				}
				else
				{
					hos->hos_BigVert = FALSE;
					hos->hos_ScrollyHeight = 10;
				}
			}
			else
				error = TRUE;
		}
		else
			error = TRUE;

		if( error )
		{
			if( hos->hos_vi ) FreeVisualInfo( hos->hos_vi );
			if( hos->hos_dri ) FreeScreenDrawInfo( win->WScreen, hos->hos_dri );
			FreeVec( hos );
			hos = NULL;
		}
	}

	return( hos );
}

/******************  HO_FreeHOStuff()  ******************/

void HO_FreeHOStuff( struct HOStuff *hos )
{
	if( hos )
	{
		if( hos->hos_vi ) FreeVisualInfo( hos->hos_vi );
		FreeVec( hos );
	}
}


/******************  HO_KillLayout()  ******************/
void HO_KillLayout( struct HOStuff *hos )
{
	if( hos->hos_GList )
	{
		RemoveGList( hos->hos_Win, hos->hos_GList, -1 );
		FreeGadgets( hos->hos_GList );
		hos->hos_GList = NULL;
	}
}


/******************  HO_DisableObject()  ******************/
//
// Disables/Enables an object and all its children. Refreshing
// is automagic.
//

void HO_DisableObject( struct HoopyObject *hob, struct HOStuff *hos, BOOL state )
{
	ULONG optw=0;
	UWORD i,j;
//	LONG c;

//	printf("HO_DisableObject():\n");

	if( hos->hos_GList )
	{
		if( state )
			hob->ho_HOFlags |= HOFLG_DISABLED;
		else
			hob->ho_HOFlags &= ~HOFLG_DISABLED;

		switch( hob->ho_Type )
		{
			case HOTYPE_VGROUP:
//				printf( "  VGroup\n" );
				j = 1;
				for( i=0; ( i < hob->ho_Attr0 ) && ( optw != ~0 ); i++)
				{
					HO_DisableObject( &hob[j], hos, state );
					j += HO_CountSkip( &hob[j] );
				}
				break;

			case HOTYPE_HGROUP:
//				printf( "  HGroup\n" );
				j = 1;
				for( i=0; ( i < hob->ho_Attr0 ) && ( optw != ~0 ); i++)
				{
					HO_DisableObject( &hob[j], hos, state );
					j += HO_CountSkip( &hob[j] );
				}
				break;

			/* should probably check for v39 here, but Grant sez we don't have to. */
			case HOTYPE_MX:
			case HOTYPE_LISTVIEW:
			case HOTYPE_BUTTON:
			case HOTYPE_CYCLE:
			case HOTYPE_CHECKBOX:
				if( hob->ho_Gadget )
				{
					GT_SetGadgetAttrs( hob->ho_Gadget, hos->hos_Win, NULL,
						GA_Disabled, state );
				}
				break;
			case HOTYPE_FIXEDPOINT:
			case HOTYPE_PACKEDSTRING:
			case HOTYPE_INTEGER:
					if( state )
						OffGadget( hob->ho_Gadget, hos->hos_Win, NULL );
					else
						OnGadget( hob->ho_Gadget, hos->hos_Win, NULL );
				break;
			case HOTYPE_STRING:
				if( hob->ho_Gadget )
				{
					if( state )
						OffGadget( hob->ho_Gadget, hos->hos_Win, NULL );
					else
						OnGadget( hob->ho_Gadget, hos->hos_Win, NULL );
				}
				break;
			case HOTYPE_SPACE:
			case HOTYPE_TEXT:
			case HOTYPE_NUMBER:
			case HOTYPE_HSCROLLER:
			case HOTYPE_HSLIDER:
			case HOTYPE_PALETTE:
			case HOTYPE_IMAGEBUTTON:
			case HOTYPE_HLINE:
			case HOTYPE_VLINE:
				break;
		}
	}
//	printf("Done.\n");
}

/******************  HO_RefreshObject()  ******************/
//


void HO_RefreshObject( struct HoopyObject *hob, struct HOStuff *hos )
{
	ULONG optw=0;
	UWORD i,j;
	char strbuf[128];

	if( hos->hos_GList )
	{
		switch( hob->ho_Type )
		{
			case HOTYPE_VGROUP:
				j = 1;
				for( i=0; ( i < hob->ho_Attr0 ) && ( optw != ~0 ); i++)
				{
					HO_RefreshObject( &hob[j], hos );
					j += HO_CountSkip( &hob[j] );
				}
				break;

			case HOTYPE_HGROUP:
				j = 1;
				for( i=0; ( i < hob->ho_Attr0 ) && ( optw != ~0 ); i++)
				{
					HO_RefreshObject( &hob[j], hos );
					j += HO_CountSkip( &hob[j] );
				}
				break;


			case HOTYPE_TEXT:
				if( hob->ho_Gadget )
				{
					GT_SetGadgetAttrs( hob->ho_Gadget, hos->hos_Win, NULL,
						GA_Disabled, hob->ho_HOFlags & HOFLG_DISABLED,
						GTTX_Text, hob->ho_Attr0, TAG_DONE );
				}
				break;

			case HOTYPE_NUMBER:
				if( hob->ho_Gadget )
				{
					GT_SetGadgetAttrs( hob->ho_Gadget, hos->hos_Win, NULL,
						GA_Disabled, hob->ho_HOFlags & HOFLG_DISABLED,
						GTNM_Number, hob->ho_Attr0, TAG_DONE );
				}
				break;
			case HOTYPE_LISTVIEW:
				if( hob->ho_Gadget )
				{
					GT_SetGadgetAttrs( hob->ho_Gadget, hos->hos_Win, NULL,
						GTLV_Labels, hob->ho_Attr0,
						GTLV_ShowSelected, NULL,	//hob->ho_Attr1,
						GTLV_Selected, hob->ho_Value,
						GTLV_ReadOnly, hob->ho_Attr1,
						GA_Disabled, hob->ho_HOFlags & HOFLG_DISABLED,
						TAG_DONE );
				}
				break;
			case HOTYPE_HSCROLLER:
			case HOTYPE_VSCROLLER:
				if( hob->ho_Gadget )
				{
					GT_SetGadgetAttrs( hob->ho_Gadget, hos->hos_Win, NULL,
						GA_Disabled, hob->ho_HOFlags & HOFLG_DISABLED,
						GTSC_Visible, hob->ho_Attr0,
						GTSC_Total, hob->ho_Attr1,
						GTSC_Top, hob->ho_Value,
						TAG_END );
				}
				break;
			case HOTYPE_HSLIDER:
			case HOTYPE_VSLIDER:
				if( hob->ho_Gadget )
				{
					GT_SetGadgetAttrs( hob->ho_Gadget, hos->hos_Win, NULL,
						GA_Disabled, hob->ho_HOFlags & HOFLG_DISABLED,
						GTSL_Min, hob->ho_Attr0,
						GTSL_Max, hob->ho_Attr1,
						GTSL_Level, hob->ho_Value,
						TAG_END );
				}
				break;
			case HOTYPE_INTEGER:
				if( hob->ho_Gadget )
				{
					GT_SetGadgetAttrs( hob->ho_Gadget, hos->hos_Win, NULL,
						GA_Disabled, hob->ho_HOFlags & HOFLG_DISABLED,
						GTIN_Number, hob->ho_Value,
						TAG_END );
				}
				break;
			case HOTYPE_FIXEDPOINT:
				if( hob->ho_Gadget )
				{
					FixedPointToString( hob->ho_Value, strbuf );
					GT_SetGadgetAttrs( hob->ho_Gadget, hos->hos_Win, NULL,
						GTST_String, strbuf,
						GA_Disabled, hob->ho_HOFlags & HOFLG_DISABLED,
						TAG_END );
				}
				break;
			case HOTYPE_STRING:
			case HOTYPE_PACKEDSTRING:
				if( hob->ho_Gadget )
				{
					GT_SetGadgetAttrs( hob->ho_Gadget, hos->hos_Win, NULL,
						GTST_String, hob->ho_Attr0,
						GA_Disabled, hob->ho_HOFlags & HOFLG_DISABLED,
						TAG_END );
				}
				break;
			case HOTYPE_CYCLE:
				if( hob->ho_Gadget )
				{
					GT_SetGadgetAttrs( hob->ho_Gadget, hos->hos_Win, NULL,
						GTCY_Active, hob->ho_Value,
						GA_Disabled, hob->ho_HOFlags & HOFLG_DISABLED,
						TAG_END );
				}
				break;
			case HOTYPE_CHECKBOX:
				if( hob->ho_Gadget )
				{
					GT_SetGadgetAttrs( hob->ho_Gadget, hos->hos_Win, NULL,
						GTCB_Checked, hob->ho_Value,
						GA_Disabled, hob->ho_HOFlags & HOFLG_DISABLED,
						TAG_END );
				}
				break;

			case HOTYPE_MX:
			case HOTYPE_BUTTON:
			case HOTYPE_PALETTE:
			case HOTYPE_IMAGEBUTTON:
				if( hob->ho_Gadget )
				{
					GT_SetGadgetAttrs( hob->ho_Gadget, hos->hos_Win, NULL,
						GA_Disabled, hob->ho_HOFlags & HOFLG_DISABLED,
						TAG_END );
				}
				break;
			case HOTYPE_SPACE:
			case HOTYPE_HLINE:
			case HOTYPE_VLINE:
				break;
		}
	}
}


/******************  HO_ReadGadgetStates()  ******************/
//
// Read out the values of any gadgets that may be out of sync with
// the intuimessages we've recieved. Values stored back in corresponding
// HoopyObject struct.
//
// note: should check for v39+ and use GT_GetGadgetAttrs() sometime...
// (but had problem when linking)
//

void HO_ReadGadgetStates( struct HOStuff *hos, struct HoopyObject *hob )
{
	BOOL done = FALSE;
	char *str;
	UWORD	i=0;
	struct StringInfo *si;

//	D(bug("HO_ReadGadgetStates()\n") );

	while( !done )
	{
//		D(bug("%d: ",i) );

		switch( hob->ho_Type )
		{
			case HOTYPE_END:
//				D(bug("  END\n") );
				done = TRUE;				/* end of layout */
				break;

			/* These ones need to be read... */

			case HOTYPE_CHECKBOX:
//				D(bug("  CheckBox\n") );
				if( hob->ho_Gadget )
				{
					hob->ho_Value =
						hob->ho_Gadget->Flags&GFLG_SELECTED ? TRUE:FALSE;
				}
				break;
			case HOTYPE_INTEGER:
//				D(bug("  Integer\n") );
				if( hob->ho_Gadget )
				{
					hob->ho_Value = ( (struct StringInfo *)hob->ho_Gadget->SpecialInfo)->
						LongInt;
				}
				break;
			case HOTYPE_FIXEDPOINT:
//				D(bug("  FixedPoint\n") );
				if( hob->ho_Gadget )
				{
					if( si = hob->ho_Gadget->SpecialInfo )
					{
						if( str = si->Buffer )
							StrToFixedPoint( &hob->ho_Value, str );
					}
					else
						D(bug("fixpoint  >StringInfo NULL!  gad: %d, '%s'\n", hob->ho_GadgetID, hob->ho_Text ) );
				}
				break;
			case HOTYPE_STRING:
			case HOTYPE_PACKEDSTRING:
//				D(bug("  string/packedstring\n") );
				if( hob->ho_Gadget )
				{
					if( hob->ho_Attr0 )
					{
						if( si = hob->ho_Gadget->SpecialInfo )
						{
							if( str = si->Buffer )
								Mystrncpy( (STRPTR)hob->ho_Attr0, str,hob->ho_Attr1 );
							else
								D(bug("  >StringInfo->Buffer NULL!\n") );
						}
						else
							D(bug("  >StringInfo NULL!\n") );
					}
				}
				break;

				/* ...these ones are always kept up to date via intuimessages... */

			case HOTYPE_LISTVIEW:
			case HOTYPE_MX:
			case HOTYPE_PALETTE:
			case HOTYPE_HSCROLLER:
			case HOTYPE_HSLIDER:
//				D(bug("  Misc1\n") );
				break;
			case HOTYPE_CYCLE:
//				D(bug("  Cycle\n") );
				break;

				/* ...and these ones don't have any editable data associated with them. */

			case HOTYPE_BUTTON:
//				D(bug("  Button\n") );
				break;
			case HOTYPE_SPACE:
//				D(bug("  Space\n") );
				break;
			case HOTYPE_VGROUP:
//				D(bug("  VGroup: %d\n",hob->ho_Attr0) );
				break;
			case HOTYPE_HGROUP:
//				D(bug("  HGroup: %d\n",hob->ho_Attr0 ));
				break;
			case HOTYPE_NUMBER:
//				D(bug("  Number\n") );
				break;
			case HOTYPE_TEXT:
//				D(bug("  Text\n") );
				break;
			case HOTYPE_IMAGEBUTTON:
			case HOTYPE_HLINE:
			case HOTYPE_VLINE:
//				D(bug("  Misc2\n") );
				break;
		}
		hob++;
		i++;
	}
}


/******************  HO_CheckIMsg()  ******************/
//
// Checks an IntuiMessage to see if a gadget value has been changed.
// If so, it changes the corresponding HoopyObject to reflect any
// updated stuff.
//
// The IntuiMessage is not altered, replied to, chopped, boiled, fried,
// baked, exploded or harmed in any way.
//
// Note: the IntuiMessage should have been first filtered by GadTools.
//

void HO_CheckIMsg( struct IntuiMessage *imsg )
{
	struct Gadget *gad;
	struct HoopyObject *hob;	//, *otherhob;

	if( imsg )
	{
		switch( imsg->Class )
		{
			case IDCMP_GADGETUP:
				if( gad = (struct Gadget *)imsg->IAddress )
				{
					if( hob = gad->UserData )
					{
						switch( hob->ho_Type )
						{
							case HOTYPE_VSCROLLER:
							case HOTYPE_HSCROLLER:
							case HOTYPE_VSLIDER:
							case HOTYPE_HSLIDER:
							case HOTYPE_LISTVIEW:
							case HOTYPE_CYCLE:
								hob->ho_Value = (LONG)imsg->Code;
								break;
							case HOTYPE_STRING:
							case HOTYPE_INTEGER:
							case HOTYPE_FIXEDPOINT:
							case HOTYPE_PACKEDSTRING:
								HO_ActivateNextStringGad( imsg->IDCMPWindow, hob );
/*							
								otherhob = hob + 1;
								while( ( otherhob->ho_Type != HOTYPE_END ) &&
									( otherhob->ho_Type != HOTYPE_STRING ) &&
									( otherhob->ho_Type != HOTYPE_PACKEDSTRING ) &&
									( otherhob->ho_Type != HOTYPE_FIXEDPOINT ) &&
									( otherhob->ho_Type != HOTYPE_INTEGER ) )
								{
									otherhob++;
								}
								if( ( otherhob->ho_Type == HOTYPE_STRING ) ||
									( otherhob->ho_Type == HOTYPE_PACKEDSTRING ) ||
									( otherhob->ho_Type == HOTYPE_FIXEDPOINT ) ||
									( otherhob->ho_Type == HOTYPE_INTEGER ) )
								{
									if( otherhob->ho_Gadget &&
										!(otherhob->ho_HOFlags & HOFLG_DISABLED ) )
										ActivateGadget( otherhob->ho_Gadget, imsg->IDCMPWindow, NULL );
								}
*/
								break;
						}
					}
				}
				break;

			case IDCMP_GADGETDOWN:
				if( gad = (struct Gadget *)imsg->IAddress )
				{
					if( hob = gad->UserData )
					{
						switch( hob->ho_Type )
						{
							case HOTYPE_MX:
								hob->ho_Value = (LONG)imsg->Code;
								break;
						}
					}
				}
				break;
		}
	}
}


void HO_ActivateNextStringGad( struct Window *win, struct HoopyObject *hob )
{
	struct HoopyObject *otherhob;

	if( !win )
		return;

	otherhob = hob + 1;
	while( ( otherhob->ho_Type != HOTYPE_END ) &&
		( otherhob->ho_Type != HOTYPE_STRING ) &&
		( otherhob->ho_Type != HOTYPE_PACKEDSTRING ) &&
		( otherhob->ho_Type != HOTYPE_FIXEDPOINT ) &&
		( otherhob->ho_Type != HOTYPE_INTEGER ) )
	{
		otherhob++;
	}
	if( ( otherhob->ho_Type == HOTYPE_STRING ) ||
		( otherhob->ho_Type == HOTYPE_PACKEDSTRING ) ||
		( otherhob->ho_Type == HOTYPE_FIXEDPOINT ) ||
		( otherhob->ho_Type == HOTYPE_INTEGER ) )
	{
		if( otherhob->ho_Gadget &&
			!(otherhob->ho_HOFlags & HOFLG_DISABLED ) )
			ActivateGadget( otherhob->ho_Gadget, win, NULL );
	}
}


BOOL HO_SussWindowSizing( struct HOStuff *hos, struct HoopyObject *hob )
{
	BOOL bigenuff;
	struct TextFont *tf;

	if( !(bigenuff = InternalSussWindowSizing( hos, hob )) )
	{
		/* fall back to topaz... */
		tf = topaztextfont;
		hos->hos_tf = tf;
		hos->hos_TextAttr.ta_Name = tf->tf_Message.mn_Node.ln_Name;
		hos->hos_TextAttr.ta_YSize = tf->tf_YSize;
		hos->hos_TextAttr.ta_Style = tf->tf_Style;
		hos->hos_TextAttr.ta_Flags = tf->tf_Flags;

		SetFont( hos->hos_Win->RPort, topaztextfont );

		/* ...and try again */
		bigenuff = InternalSussWindowSizing( hos, hob );
	}
	return( bigenuff );
}



static BOOL InternalSussWindowSizing( struct HOStuff *hos,
	struct HoopyObject *hob )
{
	struct Window *win;
	ULONG minw,minh;
	WORD left,top,width,height,deltaw,deltah;
	BOOL bigenuff;

	bigenuff = FALSE;

	win = hos->hos_Win;
	assert( win != NULL );

	left = win->LeftEdge;
	top = win->TopEdge;
	width = win->Width;
	height = win->Height;

	minw = HO_FindMinWidth( hos, hob, hos->hos_tf ) +
		win->BorderLeft + win->BorderRight;
	minh = HO_FindMinHeight( hos, hob, hos->hos_tf ) +
		win->BorderTop + win->BorderBottom;

	/* window already big enough? */
	if( ( width >= minw ) && ( height >= minh ) )
		bigenuff = TRUE;
	else
	{
		if( (minw <= win->WScreen->Width) && (minh <= win->WScreen->Height) )
		{
			bigenuff = TRUE;
			deltaw = 0;
			deltah = 0;
			if( width < minw )
				width = minw;
			if( height < minh )
				height = minh;

			if( (left + width) > win->WScreen->Width )
			{
				left = win->WScreen->Width - width;
			}
			if( (top + height) > win->WScreen->Height )
			{
				top = win->WScreen->Height - height;
			}

			ChangeWindowBox( win, left, top, width, height );
			Delay(10);
		}
	}

	if( bigenuff == TRUE );
		WindowLimits( win, minw, minh, ~0, ~0);

	return( bigenuff );
}





UWORD HO_DoReq( struct HoopyObject *hob, struct Window *win )
{
	struct HOStuff *hos;
	struct	IntuiMessage		*imsg, *gtimsg;
	ULONG		signalset, mainsignal;
	UWORD	ret = 0;

	ULONG class;
	UWORD code;
//	UWORD qualifier;
	APTR iaddress;
//	WORD mousex, mousey;
	ULONG seconds, micros;

	if( hos = HO_GetHOStuff( win, win->RPort->Font ) )
	{
		HO_SussWindowSizing( hos, hob );
		ClearWindow( win );
		HO_MakeLayout( hob, hos, win->BorderLeft, win->BorderTop,
			win->Width - win->BorderLeft - win->BorderRight,
			win->Height - win->BorderTop - win->BorderBottom );

		mainsignal = 1L << win->UserPort->mp_SigBit;

		while ( ret < 0x8000 )
		{
			signalset = Wait( mainsignal );

			if( signalset & mainsignal )
			{
				while( imsg = (struct IntuiMessage *)GetMsg( win->UserPort ) )
				{
					if( imsg->IDCMPWindow == win )
					{

						if( gtimsg = GT_FilterIMsg( imsg ) )
						{
							HO_CheckIMsg( gtimsg );

							class = gtimsg->Class;
							code = gtimsg->Code;
					//		qualifier = gtimsg->Qualifier;
							iaddress = gtimsg->IAddress;
					//		mousex = gtimsg->MouseX;
					//		mousey = gtimsg->MouseY;
							seconds = gtimsg->Seconds;
							micros = gtimsg->Micros;

							GT_PostFilterIMsg( gtimsg );

							switch( class )
							{
								case IDCMP_CLOSEWINDOW:
									ret = 0xFFFF;
									ReplyMsg( (struct Message *)imsg );
									break;
								case IDCMP_NEWSIZE:
									ClearWindow( win );
									HO_ReadGadgetStates( hos, hob );
									HO_MakeLayout( hob, hos,
										win->BorderLeft, win->BorderTop,
										win->Width - win->BorderLeft - win->BorderRight,
										win->Height - win->BorderTop - win->BorderBottom );
									ReplyMsg( (struct Message *)imsg );
									break;
								case IDCMP_GADGETUP:
									ret = ((struct Gadget *)iaddress)->GadgetID;
									ReplyMsg( (struct Message *)imsg );
									break;
								case IDCMP_REFRESHWINDOW:
									GT_BeginRefresh(win);
									GT_EndRefresh(win,TRUE);
									ReplyMsg( (struct Message *)imsg );
									break;
								default:
									ReplyMsg( (struct Message *)imsg );
									break;
							}
						}
						else
							ReplyMsg( (struct Message *)imsg );
					}
					else
						ReplyMsg( (struct Message *)imsg );
				}
			}
		}
		HO_ReadGadgetStates( hos, hob );
		HO_KillLayout( hos );
		HO_FreeHOStuff( hos );
	}
	return( ret );
}




ULONG __saveds __asm PackedStringHook( register __a0 struct Hook *hookptr,
	register __a2 struct SGWork *work,
	register __a1	ULONG *msg )
{
	ULONG ret = ~0L;

	if( *msg == SGH_KEY )
	{
		if( (work->EditOp == EO_REPLACECHAR) ||
			(work->EditOp == EO_INSERTCHAR) )
		{
			if( MakePackable( work->WorkBuffer ) )
			{
				work->Actions |= SGA_BEEP;
				work->Actions &= ~SGA_USE;
			}
		}
	}
	else
	{
		ret = 0;
	}
	return( ret );
}



ULONG __saveds __asm FixedPointHook( register __a0 struct Hook *hookptr,
	register __a2 struct SGWork *work,
	register __a1	ULONG *msg )
{
	ULONG ret = ~0L;

	if( *msg == SGH_KEY )
	{
		if( (work->EditOp == EO_REPLACECHAR) ||
			(work->EditOp == EO_INSERTCHAR) )
		{
			if( isdigit( work->Code ) ||
				( work->Code == '+' && work->BufferPos == 1 ) ||
				( work->Code == '-' && work->BufferPos == 1 ) ||
				work->Code == ':' ||
				work->Code == '.' )
			{
				if( !StrToFixedPoint( &work->LongInt, work->WorkBuffer ) )
				{
					work->Actions |= SGA_BEEP;
					work->Actions &= ~SGA_USE;
				}
			}
			else
			{
				work->Actions |= SGA_BEEP;
				work->Actions &= ~SGA_USE;
			}
		}
		else if( work->EditOp == EO_ENTER )
		{
			if( StrToFixedPoint( &work->LongInt, work->WorkBuffer ) )
			{
				FixedPointToString( work->LongInt, work->WorkBuffer );
				work->NumChars = strlen( work->WorkBuffer );
			}
			else
				work->Actions |= SGA_BEEP;
		}
	}
	else
	{
		ret = 0;
	}
	return( ret );
}


BOOL StrToFixedPoint( LONG *fixnum, char *str )
{
	BOOL allright = TRUE;
	LONG val = 0;
	LONG fract = 0;
	char c;
//	BOOL abort = FALSE;
	int sign = 1, tens = 1;

	if( *str == '-' )
	{
		sign = -1;
		str++;
	}
	else if( *str == '+' )
		str++;

	while( isdigit( c = *str++ ) )
		val = ( val * 10 ) + ( c - '0' );

	switch( c )
	{
		case '\0':
			val *= FIXMULT;
			break;
		case ':':
			while( isdigit( c = *str++ ) )
				fract = ( fract * 10 ) + ( c - '0' );
			if( c=='\0' && ( fract < FIXMULT ) )
				val = val*FIXMULT + fract;
			else
				allright = FALSE;
			break;
		case '.':
			while( isdigit( c = *str++ ) )
			{
				fract = ( fract * 10 ) + ( c - '0' );
				tens *= 10;
			}
			if( c == '\0' )
				val = (val * FIXMULT) + ( ( fract * FIXMULT ) / tens );
			else
				allright = FALSE;
			break;
		default:
			allright = FALSE;
			break;
	}

	*fixnum = val*sign;
	return( allright );
}



void OBSFixedPointToString( LONG fixnum, char *buf )
{
	if( fixnum >= 0 )
		if( fixnum & ( FIXMULT - 1 ) )
			sprintf( buf, "%d:%d", fixnum / FIXMULT, fixnum & ( FIXMULT - 1 ) );
		else
			sprintf( buf, "%d", fixnum / FIXMULT );
	else
		if( (-fixnum) & ( FIXMULT - 1 ) )
			sprintf( buf, "-%d:%d", (-fixnum) / FIXMULT, (-fixnum) & ( FIXMULT - 1 ) );
		else
			sprintf( buf, "-%d", (-fixnum) / FIXMULT );
}



void FixedPointToString( LONG fixnum, char *buf )
{
	long whole,fract;

	if( fixnum >= 0 )
	{
		whole = fixnum / FIXMULT;
		fract = fixnum & (FIXMULT-1);
	}
	else
	{
		*buf++ = '-';
		whole = (-fixnum) / FIXMULT;
		fract = (-fixnum) & (FIXMULT-1);
	}

	if( fract )
		sprintf( buf, "%d:%d", whole, fract );
	else
		sprintf( buf, "%d", whole );
}
