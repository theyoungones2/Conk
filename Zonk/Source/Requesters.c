/********************************************************/
//
// Requesters.c
//
/********************************************************/

#define REQUESTERS_C

#include <stdio.h>
#include <string.h>
#include <stdarg.h>

#include <dos/dos.h>
#include <dos/dostags.h>
#include <exec/exec.h>
#include <intuition/intuition.h>
//#include <graphics/gfxbase.h>
#include <libraries/gadtools.h>
#include <libraries/diskfont.h>
#include <libraries/asl.h>
#include <libraries/iffparse.h>

#include <proto/exec.h>
#include <proto/dos.h>
#include <proto/intuition.h>
#include <proto/graphics.h>
#include <proto/diskfont.h>
#include <proto/asl.h>
#include <clib/alib_protos.h>
#include <proto/gadtools.h>
//#include <proto/iffparse.h>

#include "hoopy.h"
#include "global.h"


static BOOL InternalMyStringRequest( struct Window *parentwin, char *winname, char *gadname,
	UBYTE *buffer, UWORD maxchars, UWORD type );

/*******************************************************************/
BOOL MyStringRequest( struct Window *parentwin, char *winname, char *gadname,
	UBYTE *buffer, UWORD maxchars )
{
	return InternalMyStringRequest( parentwin, winname, gadname, buffer, maxchars, 0 );
}


/*******************************************************************/
BOOL MyPackedStringRequest( struct Window *parentwin, char *winname, char *gadname,
	UBYTE *buffer, UWORD maxchars )
{
	return InternalMyStringRequest( parentwin, winname, gadname, buffer, maxchars, 1 );
}


/*******************************************************************/

/* type: 0=standard string, 1=packed string */

static BOOL InternalMyStringRequest( struct Window *parentwin, char *winname, char *gadname,
	UBYTE *buffer, UWORD maxchars, UWORD type )
{
	struct Window *win;
	char tempbuf[256];
	BOOL ret = FALSE;

	static struct HoopyObject gadlayout[] =
	{
		{ HOTYPE_VGROUP, 100, HOFLG_RAISEDBORDER|HOFLG_FREEWIDTH|HOFLG_FREEHEIGHT,3,0,0,0, NULL, 0,0,0,0,0 },
			{ HOTYPE_STRING, 100, HOFLG_NOBORDER, 0,0,0,0, NULL,0,0,0,0,0 },
			{ HOTYPE_SPACE, 100, HOFLG_NOBORDER|HOFLG_FREEWIDTH|HOFLG_FREEHEIGHT, 0,0,0,0, NULL,0,0,0,0,0 },
			{ HOTYPE_HGROUP, 100, HOFLG_NOBORDER|HOFLG_FREEWIDTH,3,0,0,0, NULL, 0,0,0,0,0 },
				{ HOTYPE_BUTTON, 100, HOFLG_NOBORDER, 0,0,0,0, "OK",0,0x8000,0,0,0 },
				{ HOTYPE_SPACE, 100, HOFLG_NOBORDER|HOFLG_FREEWIDTH, 0,0,0,0, NULL,0,0,0,0,0 },
				{ HOTYPE_BUTTON, 100, HOFLG_NOBORDER, 0,0,0,0, "Cancel",0,0xFFFF,0,0,0 },
		{HOTYPE_END}
	};

	if( type == 0 )
		gadlayout[1].ho_Type = HOTYPE_STRING;
	else
		gadlayout[1].ho_Type = HOTYPE_PACKEDSTRING;

	Mystrncpy( tempbuf,buffer,maxchars );

	if( win = OpenWindowTags( NULL,
		WA_Left, parentwin->LeftEdge + parentwin->BorderLeft,
		WA_Top, parentwin->TopEdge + parentwin->BorderTop,
		WA_Width, 300,
		WA_Height, 60,
		WA_MaxWidth, ~0,
		WA_MaxHeight, ~0,
		WA_PubScreen, parentwin->WScreen,
		WA_PubScreenFallBack, TRUE,
		WA_AutoAdjust,	TRUE,
		WA_Title,				winname,
		WA_CloseGadget,	TRUE,
		WA_DragBar,			TRUE,
		WA_SizeGadget,	TRUE,
		WA_DepthGadget,	TRUE,
		WA_SizeBRight,	TRUE,
		WA_NewLookMenus,TRUE,
		WA_Activate,		TRUE,
		WA_IDCMP,				IDCMP_CLOSEWINDOW|IDCMP_NEWSIZE|IDCMP_REFRESHWINDOW|
			STRINGIDCMP|TEXTIDCMP|BUTTONIDCMP,
		TAG_DONE,				NULL) )
	{
		gadlayout[1].ho_Attr0 = (ULONG)tempbuf;
		gadlayout[1].ho_Attr1 = maxchars;
		gadlayout[1].ho_Text = gadname;
		gadlayout[1].ho_Flags = PLACETEXT_LEFT;

		if( HO_DoReq( gadlayout, win ) != 0xFFFF )
		{
			Mystrncpy( buffer,tempbuf,maxchars );
			ret = TRUE;
		}
		CloseWindow( win );
	}
	return( ret );
}

/*******************************************************************/

BOOL MyIntRequest( struct Window *parentwin, char *winname, char *gadname,
	LONG *value )
{
	struct Window *win;
	BOOL ret;

	static struct HoopyObject gadlayout[] =
	{
		{ HOTYPE_VGROUP, 100, HOFLG_RAISEDBORDER|HOFLG_FREEWIDTH|HOFLG_FREEHEIGHT,3,0,0,0, NULL, 0,0,0,0,0 },
			{ HOTYPE_INTEGER, 100, HOFLG_NOBORDER, 0,0,0,0, NULL,0,0,0,0,0 },
			{ HOTYPE_SPACE, 100, HOFLG_NOBORDER|HOFLG_FREEWIDTH|HOFLG_FREEHEIGHT, 0,0,0,0, NULL,0,0,0,0,0 },
			{ HOTYPE_HGROUP, 100, HOFLG_NOBORDER|HOFLG_FREEWIDTH,3,0,0,0, NULL, 0,0,0,0,0 },
				{ HOTYPE_BUTTON, 100, HOFLG_NOBORDER, 0,0,0,0, "OK",0,0x8000,0,0,0 },
				{ HOTYPE_SPACE, 100, HOFLG_NOBORDER|HOFLG_FREEWIDTH, 0,0,0,0, NULL,0,0,0,0,0 },
				{ HOTYPE_BUTTON, 100, HOFLG_NOBORDER, 0,0,0,0, "Cancel",0,0xFFFF,0,0,0 },
		{HOTYPE_END}
	};

	ret = FALSE;
	if( win = OpenWindowTags( NULL,
		WA_Left, parentwin->LeftEdge + parentwin->BorderLeft,
		WA_Top, parentwin->TopEdge + parentwin->BorderTop,
		WA_Width, 300,
		WA_Height, 60,
		WA_MaxWidth, ~0,
		WA_MaxHeight, ~0,
		WA_PubScreen, parentwin->WScreen,
		WA_PubScreenFallBack, TRUE,
		WA_AutoAdjust,	TRUE,
		WA_Title,				winname,
		WA_CloseGadget,	TRUE,
		WA_DragBar,			TRUE,
		WA_SizeGadget,	TRUE,
		WA_DepthGadget,	TRUE,
		WA_SizeBRight,	TRUE,
		WA_NewLookMenus,TRUE,
		WA_Activate,		TRUE,
		WA_IDCMP,				IDCMP_CLOSEWINDOW|IDCMP_NEWSIZE|IDCMP_REFRESHWINDOW|
			STRINGIDCMP|TEXTIDCMP|BUTTONIDCMP,
		TAG_DONE,				NULL) )
	{
		gadlayout[1].ho_Attr0 = 10;
		gadlayout[1].ho_Value = *value;
		gadlayout[1].ho_Text = gadname;
		gadlayout[1].ho_Flags = PLACETEXT_LEFT;

		if( HO_DoReq( gadlayout, win ) != 0xFFFF )
		{
			*value = gadlayout[1].ho_Value;
			ret = TRUE;
		}
		CloseWindow( win );
	}
	return( ret );
}


/*******************************************************************/

UWORD MyOperatorRequest( struct Window *parentwin, UWORD operatortype )
{
	struct Window *win;

	struct DataNode *ops, *noddy;
	struct List oplist;
	UWORD i,j, op = 0xFFFF;
	static struct HoopyObject gadlayout[] =
	{
		{ HOTYPE_VGROUP, 100, HOFLG_RAISEDBORDER|HOFLG_FREEWIDTH|HOFLG_FREEHEIGHT,2,0,0,0, NULL, 0,0,0,0,0 },
			{ HOTYPE_LISTVIEW, 100, HOFLG_NOBORDER|HOFLG_FREEWIDTH|HOFLG_FREEHEIGHT, 0,0,0,0, NULL,0,0,0,0,0 },
			{ HOTYPE_HGROUP, 100, HOFLG_NOBORDER|HOFLG_FREEWIDTH,3,0,0,0, NULL, 0,0,0,0,0 },
				{ HOTYPE_BUTTON, 100, HOFLG_NOBORDER, 0,0,0,0, "OK",0,0x8000,0,0,0 },
				{ HOTYPE_SPACE, 100, HOFLG_NOBORDER|HOFLG_FREEWIDTH, 0,0,0,0, NULL,0,0,0,0,0 },
				{ HOTYPE_BUTTON, 100, HOFLG_NOBORDER, 0,0,0,0, "Cancel",0,0xFFFF,0,0,0 },
		{HOTYPE_END}
	};

	NewList( &oplist );

	if( ops = AllocVec( sizeof( struct DataNode ) * numofoperators,
		MEMF_ANY|MEMF_CLEAR ) )
	{
		j = 0;
		for( i = 0; i < numofoperators; i++ )
		{
			if( operators[i].od_Type == operatortype )
			{
				ops[ j ].dn_Node.ln_Name = operators[i].od_Name;
				ops[ j ].dn_Data = (APTR)i;
				AddTail( &oplist, &ops[j++].dn_Node );
			}
		}

		gadlayout[1].ho_Attr0 = (ULONG)&oplist;

		if( win = OpenWindowTags( NULL,
			WA_Left, parentwin->LeftEdge + parentwin->BorderLeft,
			WA_Top, parentwin->TopEdge + parentwin->BorderTop,
			WA_Width, 160,
			WA_Height, 200,
			WA_MaxWidth, ~0,
			WA_MaxHeight, ~0,
			WA_PubScreen, parentwin->WScreen,
			WA_PubScreenFallBack, TRUE,
			WA_AutoAdjust,	TRUE,
			WA_Title,				operatortype == OPTYPE_ACTION ? "Select action" : "Select channelroutine",
			WA_CloseGadget,	TRUE,
			WA_DragBar,			TRUE,
			WA_SizeGadget,	TRUE,
			WA_DepthGadget,	TRUE,
			WA_SizeBRight,	TRUE,
			WA_NewLookMenus,TRUE,
			WA_Activate,		TRUE,
			WA_IDCMP,				IDCMP_CLOSEWINDOW|IDCMP_NEWSIZE|IDCMP_REFRESHWINDOW|
				LISTVIEWIDCMP|BUTTONIDCMP,
			TAG_DONE,				NULL) )
		{
			if( HO_DoReq( gadlayout, win ) != 0xFFFF )
			{
				noddy = (struct DataNode *)FindNthNode( &oplist, gadlayout[1].ho_Value );
				op = (UWORD)noddy->dn_Data;
			}
			CloseWindow( win );
		}
		FreeVec( ops );
	}
	return( op );
}


/*******************************************************************/

struct Formation *MyFormationRequest( struct Window *win, struct Chunk *cnk )
{
	struct DataNode *dn;
	struct List fmlist;
	struct Formation *fm = NULL;
	UWORD num;

	NewList( &fmlist );

	if( cnk )
	{
		if( cnk->ch_TypeID == ID_DFRM )
		{
			for( fm = (struct Formation *)cnk->ch_DataList.lh_Head;
				fm->fm_Node.ln_Succ;
				fm = (struct Formation *)fm->fm_Node.ln_Succ )
			{
				if( dn = AllocVec( sizeof( struct DataNode ), MEMF_ANY|MEMF_CLEAR ) )
				{
					dn->dn_Data = fm;
					dn->dn_Node.ln_Name = fm->fm_Name;
					AddTail( &fmlist, &dn->dn_Node );
				}
			}
			num = MyListViewRequest( win, "Select Formation", "Formations:", &fmlist );

			if( num != 0xFFFF )
				dn = (struct DataNode *)FindNthNode( &fmlist, num );
			else
				dn = NULL;

			if( dn )
				fm = (struct Formation *)dn->dn_Data;
			else
				fm = NULL;

			while( !IsListEmpty( &fmlist ) )
			{
				dn = (struct DataNode *)fmlist.lh_Head;
				Remove( &dn->dn_Node );
				FreeVec( dn );
			}
		}
	}

	return( fm );
}



/*******************************************************************/
ULONG RequestChunktype( struct Window *parentwin, UBYTE *wintitle )
{
	struct DataNode datnodes[10] = {
		{ 0,0,0,0,"Game Config",(APTR)ID_GAME },
		{ 0,0,0,0,"Display",(APTR)ID_DISP },
		{ 0,0,0,0,"ActionLists",(APTR)ID_PROG },
		{ 0,0,0,0,"Formations",(APTR)ID_DFRM },
		{ 0,0,0,0,"Weapons",(APTR)ID_WEAP },
		{ 0,0,0,0,"Timed Attackwaves",(APTR)ID_TWAV },
		{ 0,0,0,0,"Sound Effects",(APTR)ID_SFX },
		{ 0,0,0,0,"Paths",(APTR)ID_THAP },
		{ 0,0,0,0,"ScrollWave",(APTR)ID_SWAV },
		{ 0,0,0,0,"EdgeOfTheRoad",(APTR)ID_EOTR },
	};

	struct DataNode *dn;
	struct List l;
	UWORD i, num;
	ULONG id = 0;

	NewList( &l );
	for( i=0; i<10; i++ )
		AddTail( &l, &datnodes[i].dn_Node );

	num = MyListViewRequest( parentwin, wintitle, "Chunks:", &l );

	if( num < 0x8000 )
	{
		if( dn = (struct DataNode *)FindNthNode( &l, num ) )
			id = (ULONG)dn->dn_Data;
	}
	else
		id = 0;

	return( id );
}






/*******************************************************************/
//
// A requester which lets the luser pick an item from a Listview.
// Returns the number of the node selected, or 0xFFFF if cancelled
// or otherwise fucked up.
//

UWORD MyListViewRequest( struct Window *parentwin, UBYTE *wintitle,
	UBYTE *gadtitle, struct List *labels )
{
	struct Window *win;
	UWORD num;

	static struct HoopyObject gadlayout[] =
	{
		{ HOTYPE_VGROUP, 100, HOFLG_RAISEDBORDER|HOFLG_FREEWIDTH|HOFLG_FREEHEIGHT,2,0,0,0, NULL, 0,0,0,0,0 },
			{ HOTYPE_LISTVIEW, 100, HOFLG_NOBORDER|HOFLG_FREEWIDTH|HOFLG_FREEHEIGHT, 0,0,0,0, NULL,0,0,0,0,0 },
			{ HOTYPE_HGROUP, 100, HOFLG_NOBORDER|HOFLG_FREEWIDTH,3,0,0,0, NULL, 0,0,0,0,0 },
				{ HOTYPE_BUTTON, 100, HOFLG_NOBORDER, 0,0,0,0, "OK",0,0x8000,0,0,0 },
				{ HOTYPE_SPACE, 100, HOFLG_NOBORDER|HOFLG_FREEWIDTH, 0,0,0,0, NULL,0,0,0,0,0 },
				{ HOTYPE_BUTTON, 100, HOFLG_NOBORDER, 0,0,0,0, "Cancel",0,0xFFFF,0,0,0 },
		{HOTYPE_END}
	};

	gadlayout[1].ho_Value = 0xFFFF;
	gadlayout[1].ho_Text = gadtitle;
	if( gadtitle ) gadlayout[1].ho_Flags |= PLACETEXT_ABOVE;

	if( win = OpenWindowTags( NULL,
		WA_Left, parentwin->LeftEdge + parentwin->BorderLeft,
		WA_Top, parentwin->TopEdge + parentwin->BorderTop,
		WA_Width, 160,
		WA_Height, 200,
		WA_MaxWidth, ~0,
		WA_MaxHeight, ~0,
		WA_PubScreen, parentwin->WScreen,
		WA_PubScreenFallBack, TRUE,
		WA_AutoAdjust,	TRUE,
		WA_Title,				wintitle,
		WA_CloseGadget,	TRUE,
		WA_DragBar,			TRUE,
		WA_SizeGadget,	TRUE,
		WA_DepthGadget,	TRUE,
		WA_SizeBRight,	TRUE,
		WA_NewLookMenus,TRUE,
		WA_Activate,		TRUE,
		WA_IDCMP,				IDCMP_CLOSEWINDOW|IDCMP_NEWSIZE|IDCMP_REFRESHWINDOW|
			LISTVIEWIDCMP|BUTTONIDCMP,
		TAG_DONE,				NULL) )
	{
		if( !zonkfig.scfg.scfg_DefaultFont && mainwinfont )
			SetFont( win->RPort, mainwinfont );

		gadlayout[1].ho_Attr0 = (ULONG)labels;
		num = HO_DoReq( gadlayout, win );
		if( num != 0xFFFF )
			num = gadlayout[1].ho_Value;
		CloseWindow( win );
	}
	return( num );
}


/*******************************************************************/
struct Program *RequestProgram( struct Window *parentwin, struct Chunk *cnk,
	UBYTE *wintitle, UBYTE *gadtitle )
{
	struct Program *prog = NULL;
	struct List proglist;
	struct DataNode *dn;
	BOOL allok = TRUE;
	UWORD num;

	NewList( &proglist );

	if( !cnk )
	{
		/* list all chunks */
		for( cnk = (struct Chunk *)&chunklist.lh_Head;
			cnk->ch_Node.ln_Succ && allok;
			cnk = (struct Chunk *)cnk->ch_Node.ln_Succ )
		{
			if( cnk->ch_TypeID == ID_PROG )
			{
				allok = AppendToProgramList( cnk, &proglist );
			}
		}
	}
	else
	{
		if( cnk->ch_TypeID == ID_PROG )
		{
			/* just list the specified chunk */
			allok = AppendToProgramList( cnk, &proglist );
		}
	}

	if( allok )
	{
		num = MyListViewRequest( parentwin, wintitle, gadtitle, &proglist );
		if( num < 0x8000 )
		{
			if( dn = (struct DataNode *)FindNthNode( &proglist,num ) )
			{
				prog = (struct Program *)dn->dn_Data;
			}
		}
	}

	FreeNodes( &proglist );

	return( prog );
}

/*******************************************************************/
//
//	parentwin = 
//  wintitle = title of requester window
//	gadtitle = title of listview gadget
//	typeid =	chunktype to look for. If NULL, function tries to find any
//						of the types in typeidarray (not yet implemented - bummer. )
//	typeidarray = NULL-terminated array of typeids to look for.

struct Chunk *RequestChunk( struct Window *parentwin, UBYTE *wintitle,
	UBYTE *gadtitle, ULONG typeid, ULONG *typeidarray )
{
	struct List cnklist;
	struct Chunk *cnk;
	BOOL match, abort = FALSE;
	UBYTE workbuf[ CHUNKINFOSTRINGSIZE ];
	struct DataNode *dn;
	UWORD num;

	NewList( &cnklist );

	for( cnk = (struct Chunk *)chunklist.lh_Head;
		cnk->ch_Node.ln_Succ && !abort;
		cnk = (struct Chunk *)cnk->ch_Node.ln_Succ )
	{
		match = FALSE;

		if( typeid )
		{
			if( cnk->ch_TypeID == typeid )
				match = TRUE;
		}
		else
		{
			if(	!typeidarray )
				match = TRUE;
			/* else match from array... */
		}

		if( match )
		{
			if( dn = AllocVec( sizeof( struct DataNode ),MEMF_ANY|MEMF_CLEAR ) )
			{
				if( cnk->ch_MakeInfoString && (*cnk->ch_MakeInfoString)( cnk, workbuf ) )
				{
					if( dn->dn_Node.ln_Name = MyStrDup( workbuf ) )
					{
						dn->dn_Data = cnk;
						AddTail( &cnklist, &dn->dn_Node );
//						printf("cnk: %s\n",workbuf );
					}
					else
					{
						FreeVec( dn );
						dn = NULL;
						abort = TRUE;
					}
				}
			}
			else
				abort = TRUE;

		}
	}

	/* do it */

	cnk = NULL;
	if( !abort )
	{
		num = MyListViewRequest( parentwin, wintitle, gadtitle, &cnklist );
		if( num < 0x8000 )
		{
			if( dn = (struct DataNode *)FindNthNode( &cnklist,num ) )
			{
				cnk = (struct Chunk *)dn->dn_Data;
			}
		}
	}

	while( !IsListEmpty( &cnklist ) )
	{
		dn = (struct DataNode *)cnklist.lh_Head;
		Remove( &dn->dn_Node );
		if( dn->dn_Node.ln_Name )
			FreeVec( dn->dn_Node.ln_Name );
		FreeVec( dn );
	}

	return( cnk );
}


/********************************************************************/
// PlebReq()
//

WORD __stdargs PlebReq( struct Window *win, STRPTR title, STRPTR text,
	STRPTR gadgets,...)
{
	struct EasyStruct	easy;
	WORD				response = FALSE;
	va_list			varargs;

	easy.es_StructSize	= sizeof(struct EasyStruct);
	easy.es_Flags		= NULL;
	easy.es_Title		= title;
	easy.es_TextFormat	= text;
	easy.es_GadgetFormat	= gadgets;

	va_start( varargs, gadgets );
	response = EasyRequestArgs( win, &easy, NULL, varargs );
	va_end( varargs );

	return(response);
}

