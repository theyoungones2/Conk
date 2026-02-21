/********************************************************/
//
// OpReq.c
//
/********************************************************/

#define OPREQ.C

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

static void MakeList( struct List *list, struct Noddy *n, UWORD optype,
	UWORD classmask );
static UWORD EventWibble( struct Window *fenster, struct Noddy *noddybank,
	UWORD operatortype );


struct Noddy
{
	struct Node	Node;
	UWORD				Op;
};

#define GID_CLASS 2
#define GID_LIST 4
#define GID_OK 6
#define GID_CANCEL 8


#define ACLASSCOUNT 7
#define CCLASSCOUNT 1

struct Node aclassnames[ ACLASSCOUNT ] =
	{	{ NULL, NULL, 0, 0, "All" },
	{ NULL, NULL, 0, 0, "Objects" },
	{ NULL, NULL, 0, 0, "Variables" },
	{ NULL, NULL, 0, 0, "Movement" },
	{ NULL, NULL, 0, 0, "Weapons" },
	{ NULL, NULL, 0, 0, "Images" },
	{ NULL, NULL, 0, 0, "ActionLists" } };

struct Node cclassnames[ CCLASSCOUNT ] =
	{ { NULL, NULL, 0, 0, "All" } };


static struct HoopyObject gadlayout[] =
{
	{ HOTYPE_VGROUP, 100, HOFLG_RAISEDBORDER|HOFLG_FREEWIDTH|HOFLG_FREEHEIGHT,2,0,0,0, NULL, 0,0,0,0,0 },
		{ HOTYPE_HGROUP, 100, HOFLG_NOBORDER|HOFLG_FREEWIDTH,3,0,0,0, NULL, 0,0,0,0,0 },
			{ HOTYPE_LISTVIEW, 50,HOFLG_FREEWIDTH|HOFLG_FREEHEIGHT, 0,0,0,0, "Class",0,GID_CLASS,PLACETEXT_ABOVE,0,0 },
			{ HOTYPE_VLINE, 100, HOFLG_FREEHEIGHT, 0,0,0,0, NULL,0,0,0,0,0 },
			{ HOTYPE_LISTVIEW, 100, HOFLG_FREEWIDTH|HOFLG_FREEHEIGHT, 0,0,0,0, NULL,0,GID_LIST,PLACETEXT_ABOVE,0,0 },
		{ HOTYPE_HGROUP, 100, HOFLG_NOBORDER|HOFLG_FREEWIDTH,3,0,0,0, NULL, 0,0,0,0,0 },
			{ HOTYPE_BUTTON, 100, HOFLG_NOBORDER, 0,0,0,0, "OK",0,GID_OK,0,0,0 },
			{ HOTYPE_SPACE, 100, HOFLG_NOBORDER|HOFLG_FREEWIDTH, 0,0,0,0, NULL,0,0,0,0,0 },
			{ HOTYPE_BUTTON, 100, HOFLG_NOBORDER, 0,0,0,0, "Cancel",0,GID_CANCEL,0,0,0 },
	{HOTYPE_END}
};


/*******************************************************************/

UWORD OpReq( struct Window *parentwin, UWORD operatortype )
{
	struct Window *fenster;
	UWORD op;
	struct Noddy *noddybank;

	op = 0xFFFF;
	if( noddybank = AllocVec( sizeof( struct Noddy ) * numofoperators, MEMF_ANY ) )
	{
		if( fenster = OpenWindowTags( NULL,
			WA_Left, parentwin->LeftEdge + parentwin->BorderLeft,
			WA_Top, parentwin->TopEdge + parentwin->BorderTop,
			WA_Width, zonkfig.OpReqW,
			WA_Height, zonkfig.OpReqH,
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
				LISTVIEWIDCMP|BUTTONIDCMP|CYCLEIDCMP,
			TAG_DONE,				NULL) )
		{
			op = EventWibble( fenster, noddybank, operatortype );

			zonkfig.OpReqW = fenster->Width;
			zonkfig.OpReqH = fenster->Height;
			CloseWindow( fenster );
		}
		FreeVec( noddybank );
	}

	return( op );
}




static void MakeList( struct List *list, struct Noddy *n, UWORD optype,
	UWORD classnum )
{
	int i;

/* index by class num */

/*
ACTION CLASSES:
BIT	CLASS
1		Objects
2		Variables
3		Movement
4		Weapons
5		Images
6		ActionLists
9		Basic
16	Unclassified
*/

	UWORD aclassmask[] = { 0xFFFF, 0x0001, 0x0002, 0x0004, 0x0008, 0x0010, 0x0020 };
	UWORD cclassmask[] = { 0xFFFF };
	UWORD mask;

	NewList( list );

	for( i=1; i<numofoperators; i++ )
	{
		if( operators[i].od_Type == OPTYPE_CHANNL )
			mask = cclassmask[ classnum ];
		else
			mask = aclassmask[ classnum ];

		if( operators[i].od_Type == optype &&
			( operators[i].od_Class &	mask ) &&
			( operators[i].od_Name[0] != '*') )
		{
			n->Node.ln_Name = operators[i].od_Name;
			n->Op = i;
			AlphaAdd( list, &n->Node );
			n++;
		}
	}
}


static UWORD EventWibble( struct Window *fenster, struct Noddy *noddybank,
	UWORD operatortype )
{
	UWORD op;
	struct List oplist;
	struct List classlist;
	struct Noddy *n;
	BOOL done;
	int i;
	static UWORD adefaults[ ACLASSCOUNT ] = {0,0,0,0,0,0,0};
	static UWORD cdefaults[ CCLASSCOUNT ] = {0};
	static UWORD adefclass = 0,cdefclass = 0;
	UWORD classnum;

	struct HOStuff *hos;
	struct	IntuiMessage		*imsg, *gtimsg;
	ULONG		signalset, mainsignal;
	UWORD picknum;
	UWORD prevpicknum = 0xFFFF;

	ULONG class;
	UWORD code;
//	UWORD qualifier;
	APTR iaddress;
//	WORD mousex, mousey;
	ULONG seconds, micros;
	ULONG startseconds=0, startmicros=0;

	op = 0xFFFF;
	done = FALSE;

	/* make up the class list */
	NewList( &classlist );

	if( operatortype == OPTYPE_ACTION )
	{
		classnum = adefclass;
		for( i=0; i<ACLASSCOUNT; i++  )
			AddTail( &classlist, &aclassnames[i] );
	}
	else	/* OPTYPE_CHANNL */
	{
		classnum = cdefclass;
		for( i=0; i<CCLASSCOUNT; i++  )
			AddTail( &classlist, &cclassnames[i] );
	}

//	DumpList( &classlist );

	gadlayout[ GID_CLASS ].ho_Attr0 = (LONG)&classlist;
	gadlayout[ GID_CLASS ].ho_Value = (LONG)classnum;

	MakeList( &oplist, noddybank, operatortype, classnum );

	gadlayout[ GID_LIST ].ho_Attr0 = (LONG)&oplist;
	picknum = (operatortype==OPTYPE_ACTION) ?
		adefaults[ classnum ] : cdefaults[ classnum ];
	gadlayout[ GID_LIST ].ho_Value = (LONG)picknum;

	gadlayout[ GID_LIST ].ho_Text = (operatortype==OPTYPE_ACTION) ?
		"Action" : "ChRoutine";

	if( hos = HO_GetHOStuff( fenster, fenster->RPort->Font ) )
	{
		HO_SussWindowSizing( hos, gadlayout );
		ClearWindow( fenster );
		HO_MakeLayout( gadlayout, hos, fenster->BorderLeft, fenster->BorderTop,
			fenster->Width - fenster->BorderLeft - fenster->BorderRight,
			fenster->Height - fenster->BorderTop - fenster->BorderBottom );

		mainsignal = 1L << fenster->UserPort->mp_SigBit;

		while( !done )
		{
			signalset = Wait( mainsignal );

			if( signalset & mainsignal )
			{
				while( imsg = (struct IntuiMessage *)GetMsg( fenster->UserPort ) )
				{
					if( imsg->IDCMPWindow == fenster )
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
									picknum = 0xFFFF;
									done = TRUE;
									ReplyMsg( (struct Message *)imsg );
									break;
								case IDCMP_NEWSIZE:
									ClearWindow( fenster );
									HO_ReadGadgetStates( hos, gadlayout );
									HO_MakeLayout( gadlayout, hos,
										fenster->BorderLeft, fenster->BorderTop,
										fenster->Width - fenster->BorderLeft - fenster->BorderRight,
										fenster->Height - fenster->BorderTop - fenster->BorderBottom );
									ReplyMsg( (struct Message *)imsg );
									break;
								case IDCMP_GADGETUP:
									ReplyMsg( (struct Message *)imsg );
									switch( ((struct Gadget *)iaddress)->GadgetID )
									{
										case GID_CANCEL:
											picknum = 0xFFFF;
											done = TRUE;
											break;
										case GID_OK:
											done = TRUE;
											break;
										case GID_LIST:
											picknum = code;
											if(operatortype==OPTYPE_ACTION)
												adefaults[ classnum ] = picknum;
											else
												cdefaults[ classnum ] = picknum;

											if( ( picknum == prevpicknum )&&
												DoubleClick( startseconds, startmicros,
												seconds, micros) )
											{
												done = TRUE;
											}
											else
											{
												startseconds = seconds;
												startmicros = micros;
												prevpicknum = picknum;
											}
											break;
										case GID_CLASS:
											classnum = code;
											gadlayout[ GID_LIST ].ho_Attr0 = (LONG)~0;
											HO_RefreshObject( &gadlayout[ GID_LIST ], hos );
											MakeList( &oplist, noddybank, operatortype, classnum );
											gadlayout[ GID_LIST ].ho_Attr0 = (LONG)&oplist;
											picknum = (operatortype==OPTYPE_ACTION) ?
												adefaults[ classnum ] : cdefaults[ classnum ];
											gadlayout[ GID_LIST ].ho_Value = (LONG)picknum;
											HO_RefreshObject( &gadlayout[ GID_LIST ], hos );
											break;
									}
									break;
								case IDCMP_REFRESHWINDOW:
									GT_BeginRefresh(fenster);
									GT_EndRefresh(fenster,TRUE);
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
		HO_ReadGadgetStates( hos, gadlayout );
		HO_KillLayout( hos );
		HO_FreeHOStuff( hos );
	}

	if( operatortype == OPTYPE_ACTION )
		adefclass = classnum;
	else
		adefclass = classnum;

	if( picknum != 0xFFFF )
	{
		if( n = (struct Noddy *)(FindNthNode( &oplist,
			picknum ) ) )
		{
			op = n->Op;
		}
	}

	return( op );
}
