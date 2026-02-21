#include <stdio.h>
#include <string.h>

#include <exec/exec.h>
#include <devices/console.h>
#include <dos/dos.h>
#include <dos/dostags.h>
#include <intuition/intuition.h>
//#include <graphics/gfxbase.h>

#include <proto/console.h>
#include <proto/exec.h>
#include <proto/dos.h>
#include <proto/intuition.h>
#include <proto/graphics.h>
#include <clib/alib_protos.h>


/* DEFINES */

#define SCRWIDTH 640
#define SCRHEIGHT 200

#define TYPEBUTTONYPOS (8+8)

#define TYPEBUTTONWIDTH (16*8 + 6)
#define TYPEBUTTONHEIGHT (8+4)

#define DIRBUTTONYPOS (TYPEBUTTONHEIGHT + TYPEBUTTONYPOS + 2)

#define BOXWIDTH 144
#define BOXHEIGHT (DIRBUTTONYPOS + 17*8 + 4 )

#define MISCBUTTONWIDTH (8*8 + 6)
#define MISCBUTTONHEIGHT (8+4)

#define FILEBUTTONCHARS 12
#define FILEBUTTONWIDTH (FILEBUTTONCHARS * 8 + 6)
#define FILEBUTTONHEIGHT 12


#define RP (fenster->RPort)


/* STRUCTS */

#define VJTYPE_VIRTUAL			0	/* no actual joystick, check only keys */
#define VJTYPE_STDJOYSTICK	1	/* standard joystick in port 0 or 1 */
#define VJTYPE_CD32PAD			2	/* CD^32 joypad (not implemented yet) */

struct DiskFormatVJ
{
	UWORD Type;
	UWORD HWPort;
	UBYTE	RawKeys[8];
	UBYTE	Pad[4];
};


struct KeyNode
{
	struct Node				Noddy;
	char							Name[108];
	char							GadLabel[ FILEBUTTONCHARS ];
	struct Gadget			Inspector;
	struct IntuiText	IText;
};


/* VARS */

char filename[ 108 ];

USHORT oldcmap[] = {
	0x066C,	0x0CCF,	0x099E,	0x055B,	0x044B,	0x0FF0,	0x0F00,	0x00F0,
};

USHORT cmap1[] = {
	0x012C,	0x025F,	0x013E,	0x001A,	0x0008,	0x0FD0,	0x0C00,	0x009C
};

USHORT cmap[] = {
	0x44C, 0x0CCF, 0x088D, 0x022A, 0x0008, 0x0FF0, 0x0F00, 0x00F0
};



struct DiskFormatVJ configs[4] = {
	{ VJTYPE_VIRTUAL,0,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0,0,0,0 },
	{ VJTYPE_VIRTUAL,0,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0,0,0,0 },
	{ VJTYPE_VIRTUAL,0,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0,0,0,0 },
	{ VJTYPE_VIRTUAL,0,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0,0,0,0 }
	};

static struct Gadget playergads[10][4];		// 10 gads per player
static struct Gadget quitgad;
static struct Gadget savegad;

static int plrx[4] = { 0,160,320,480 };
static int plry[4] = { 1,1,1,1 };

static struct Window *fenster = NULL;
static struct Screen *thescreen = NULL;

/* PROTOS */

static void DrawPlayerStuff( int player );
static void EventLoop( void );
char *FindRawkeyString( int rawkey );
static void LayoutStuff( int player );
static void ShowKey( int player, int pos, int rawkey );
static void WipeKey( int player, int pos );
static void CycleDevice( int player, UWORD qual );
static struct IntuiText *SussDeviceIText( int player );
static void AddMiscGadgets( void );
static BOOL LoadKeysFile( char *name );
static BOOL SaveKeysFile( char *name );
static void ListKeysFiles( char *dirname, struct List *l );
void FreeKeyNodesList( struct List *l );
void AddKeyNodeGadgets( struct List *l );
char *Mystrncpy(char *to, char *from, int length);
static int PickFile( void );
struct Node *FindNthNode(struct List *list, UWORD num);
void QuickyReq( char *text );


#include "images.c"


/*******************************************************/

void main( int argc, char *argv[] )
{
	static struct TextAttr topazattr = { "topaz.font",8,0,0 };

	static struct NewScreen ns = {
		0,0,SCRWIDTH,SCRHEIGHT,3,
		1,2,			//detailpen,blockpen
		HIRES_KEY,				//viewmodes
		CUSTOMSCREEN,		//screentype
		&topazattr,			// textattr
		"Conkfig",
		NULL,						// gadgets
		NULL };					// bitmap

	static struct NewWindow nw = {
		0,0,SCRWIDTH,0,1,2,	//x,y,w,h,detailpen,blockpen
		IDCMP_RAWKEY|IDCMP_GADGETUP|IDCMP_INTUITICKS,
		WFLG_BORDERLESS|WFLG_BACKDROP,
		NULL,		//firstgadget
		NULL,		// checkmark
		NULL,		// title
		NULL,		//screen
		NULL,		//bitmap
		0,0,0,0,	//minsize/maxsize
		CUSTOMSCREEN };

	struct KeyNode *kn;
	int i;
	struct List keyfilelist;

	for( i=0; i<8; i++ )
	{
		configs[0].RawKeys[i] = 0xFF;
		configs[1].RawKeys[i] = 0xFF;
		configs[2].RawKeys[i] = 0xFF;
		configs[3].RawKeys[i] = 0xFF;
	}
/*Key_O		equ	$18
	Key_P		equ	$19
	Key_Z		equ	$31
	Key_A		equ	$20
	Key_Help	equ	$5F
	Key_CrsrUp	equ	$4C
	Key_CrsrDown	equ	$4D
	Key_CrsrLeft	equ	$4F
	Key_CrsrRight	equ	$4E
	Key_Space	equ	$40
	Key_Ralt equ $65
*/
	configs[0].RawKeys[0] = 0x4C;
	configs[0].RawKeys[1] = 0x4D;
	configs[0].RawKeys[2] = 0x4F;
	configs[0].RawKeys[3] = 0x4E;
	configs[0].RawKeys[4] = 0x65;
	configs[0].Type = VJTYPE_STDJOYSTICK;
	configs[0].HWPort = 1;

	thescreen = OpenScreen( &ns );
	if( thescreen )
	{
		LoadRGB4( &thescreen->ViewPort, cmap, 8 );

		nw.Screen = thescreen;
		nw.TopEdge = thescreen->BarHeight + 2;
		nw.Height = SCRHEIGHT - nw.TopEdge;

		fenster = OpenWindow( &nw );
		if( fenster )
		{
			NewList( &keyfilelist );
			ListKeysFiles( "", &keyfilelist );
			if( !IsListEmpty( &keyfilelist ) )
			{
				AddKeyNodeGadgets( &keyfilelist );
				kn = (struct KeyNode *)keyfilelist.lh_Head;
				RefreshGadgets( &kn->Inspector, fenster, NULL );
				i = PickFile();

				if( i>=0 )
				{
					kn = (struct KeyNode *)FindNthNode( &keyfilelist, i );
					if( kn )
					{
						Mystrncpy( filename, kn->Name, 108-1 );
						filename[ strlen( filename ) - 4 ] = 'k';
						filename[ strlen( filename ) - 3 ] = 'e';
						filename[ strlen( filename ) - 2 ] = 'y';
						filename[ strlen( filename ) - 1 ] = 's';
					}
				}

				kn = (struct KeyNode *)keyfilelist.lh_Head;
				RemoveGList( fenster, &kn->Inspector, -1 );
				FreeKeyNodesList( &keyfilelist );

				if( !LoadKeysFile( filename ) )
					QuickyReq( "'.keys' file not found - using default settings" );

				SetRast( RP, 0 );

				LayoutStuff( 0 );
				LayoutStuff( 1 );
				LayoutStuff( 2 );
				LayoutStuff( 3 );
				DrawPlayerStuff( 0 );
				DrawPlayerStuff( 1 );
				DrawPlayerStuff( 2 );
				DrawPlayerStuff( 3 );
				SetAPen( RP, 1);
//			Move( RP, 8, 8 );
//			Text( RP, filename, strlen(filename) );
				SetWindowTitles( fenster, (UBYTE *)-1, filename );

				AddMiscGadgets();

				RefreshGadgets( &playergads[0][0], fenster, NULL );

				EventLoop();
			}
			else
				QuickyReq( "No '.game' files found.\n" );

			CloseWindow( fenster );
		}

		CloseScreen( thescreen );
	}
}



/*******************************************************/

static void LayoutStuff( int player )
{
	struct Gadget *gad;
	int i,x,y;

	static UWORD topleftpairs[] = { 1,0,
		TYPEBUTTONWIDTH,0,
		TYPEBUTTONWIDTH, TYPEBUTTONHEIGHT };

	static UWORD bottomrightpairs[] = { 0,1,
		0, TYPEBUTTONHEIGHT,
		TYPEBUTTONWIDTH, TYPEBUTTONHEIGHT };

	static struct Border typebuttonborder1 = {
		0,0,
		4,0,	//frontpen,backpen
		JAM1,	//DrawMode
		3,		//Count
		bottomrightpairs,	//XY pairs.
		NULL }; //Next

	static struct Border typebuttonborder = {
		0,0,
		1,0,	//frontpen,backpen
		JAM1,	//DrawMode
		3,		//Count
		topleftpairs,	//XY pairs.
		&typebuttonborder1 }; //Next

	static struct Border typebuttonborderselected1 = {
		0,0,
		1,0,	//frontpen,backpen
		JAM1,	//DrawMode
		3,		//Count
		bottomrightpairs,	//XY pairs.
		NULL }; //Next

	static struct Border typebuttonborderselected = {
		0,0,
		4,0,	//frontpen,backpen
		JAM1,	//DrawMode
		3,		//Count
		topleftpairs,	//XY pairs.
		&typebuttonborderselected1 }; //Next

	x = plrx[ player ];
	y = plry[ player ];

	for( i = 0; i < 8; i++ )
	{
		gad = &playergads[i][player];
		gad->NextGadget = NULL;
		gad->TopEdge = y + i*17 + DIRBUTTONYPOS;
		gad->LeftEdge = x + 4;
		gad->Width = 32;
		gad->Height = 16;
		gad->Flags = GFLG_GADGIMAGE|GFLG_GADGHIMAGE;
		gad->Activation = GACT_RELVERIFY;
		gad->GadgetType = GTYP_BOOLGADGET;
		gad->GadgetText = NULL;
		gad->MutualExclude = 0;
		gad->SpecialInfo = 0;
		gad->GadgetID = ( player * 256 ) + i;
		gad->UserData = NULL;

		switch( i )
		{
			case 0:
				gad->GadgetRender = &imageup1;
				gad->SelectRender = &imageup2;
				break;
			case 1:
				gad->GadgetRender = &imagedown1;
				gad->SelectRender = &imagedown2;
				break;
			case 2:
				gad->GadgetRender = &imageleft1;
				gad->SelectRender = &imageleft2;
				break;
			case 3:
				gad->GadgetRender = &imageright1;
				gad->SelectRender = &imageright2;
				break;
			case 4:
				gad->GadgetRender = &imagefire1;
				gad->SelectRender = &imagefire2;
				break;
			case 5:
				gad->GadgetRender = &imagefire1;
				gad->SelectRender = &imagefire2;
				break;
			case 6:
				gad->GadgetRender = &imagefire1;
				gad->SelectRender = &imagefire2;
				break;
			case 7:
				gad->GadgetRender = &imagefire1;
				gad->SelectRender = &imagefire2;
				break;
		}
		AddGadget( fenster, gad, -1 );
	}

	gad = &playergads[ 8 ][ player ];

	gad->NextGadget = NULL;
	gad->TopEdge = y+TYPEBUTTONYPOS;
	gad->LeftEdge = x + 4;
	gad->Width = TYPEBUTTONWIDTH;
	gad->Height = TYPEBUTTONHEIGHT;
	gad->Flags = GFLG_GADGHIMAGE;
	gad->Activation = GACT_RELVERIFY;
	gad->GadgetType = GTYP_BOOLGADGET;
	gad->GadgetText = SussDeviceIText( player );
	gad->MutualExclude = 0;
	gad->SpecialInfo = 0;
	gad->GadgetID = ( player * 256 ) + 8;
	gad->UserData = NULL;
	gad->GadgetRender = &typebuttonborder;
	gad->SelectRender = &typebuttonborderselected;

	AddGadget( fenster, gad, -1 );
}

/*******************************************************/

static void DrawPlayerStuff( int player )
{
	int i;
	char *str;
	int x,y;

//	printf("DrawPlayerStuff( %d )\n",player);

	x = plrx[ player ];
	y = plry[ player ];

	SetDrMd( RP, JAM1 );

	// shadow
	SetAPen( RP, 3 );
	RectFill( RP, x+8, y+4, x+BOXWIDTH+8-1, y+BOXHEIGHT+4-1 );

	// box interior
	SetAPen( RP, 0 );
	RectFill( RP, x, y, x+BOXWIDTH-1, y+BOXHEIGHT-1 );

	// shine border
	SetAPen( RP, 1 );
	Move( RP, x,y );
	Draw( RP, x,y+BOXHEIGHT-1 );
	Move( RP, x+1,y+BOXHEIGHT-1 );
	Draw( RP, x+1,y );
	Draw( RP, x+BOXWIDTH-1,y );

	// shadow border
	SetAPen( RP, 4 );
	Move( RP, x+BOXWIDTH-1,y );
	Draw( RP, x+BOXWIDTH-1,y+BOXHEIGHT-1 );
	Move( RP, x+BOXWIDTH-1-1,y+1 );
	Draw( RP, x+BOXWIDTH-1-1,y+BOXHEIGHT-1 );
	Draw( RP, x+1,y+BOXHEIGHT-1 );

	switch( player )
	{
		case 0:
			str = "PLAYER ONE";
			break;
 		case 1:
			str = "PLAYER TWO";
			break;
		case 2:
			str = "PLAYER THREE";
			break;
		case 3:
			str = "PLAYER FOUR";
			break;
	}

	SetAPen( RP, 3 );
	Move( RP, x+4+2, y + 2+8+1 );
	Text( RP, str, strlen( str ) );
	SetAPen( RP, 1 );
	Move( RP, x+4, y + 2+8 );
	Text( RP, str, strlen( str ) );

	for( i=0; i<8; i++ )
	{
		ShowKey( player, i, configs[player].RawKeys[i] );
	}
}

/*******************************************************/

static void ShowKey( int player, int pos, int rawkey )
{
	int x,y;
	char *str;

	x = plrx[ player ];
	y = plry[ player ];

	str = FindRawkeyString( rawkey );

	Move( RP, x+4+32+4, DIRBUTTONYPOS + y + 4 + 6 + (pos*17) );
	Text( RP, str, strlen( str ) );
}


/*******************************************************/

static void WipeKey( int player, int pos )
{
	int x,y;

	x = plrx[ player ]+4+32+4;
	y = plry[ player ]+DIRBUTTONYPOS + 4 + (pos*17);

	SetAPen( RP, 0 );
	RectFill( RP, x, y, x+(8*12)-1, y+7 );
}

/*******************************************************/

static void EventLoop( void )
{
	struct	IntuiMessage		*msg;
	ULONG		sigflags;
	BOOL		done = FALSE;
	int mode=0;			//0=waiting, 1=oneshotinput, 2=multiinput
	int	inputplayer=0;	// player waiting for input
	int	inputpos=0;			// keypos within player waiting for input
	int player;

	ULONG class;
	UWORD code;
	UWORD qualifier;
	APTR iaddress;
//	WORD MouseX, MouseY;
//	ULONG Seconds, Micros;
	int gid;
	int flashstate=0;
	int quitcount=0;
	BOOL modflag=FALSE;

	while ( !done )
	{
		sigflags = Wait( 1L << fenster->UserPort->mp_SigBit );

		while(msg = (struct IntuiMessage *)GetMsg( fenster->UserPort ) )
		{
			class = msg->Class;
			code = msg->Code;
			qualifier = msg->Qualifier;
			iaddress = msg->IAddress;

			switch( class )
			{
				case IDCMP_RAWKEY:

					if( !(code & IECODE_UP_PREFIX) &&
						!(qualifier & IEQUALIFIER_REPEAT ) )
					{
						if( code == 0x45 )		// esc -> <none>
							code = 0xFF;

						if( code == 0x5F )		// help -> <none>
							code = 0xFF;

//						printf( "Raw: %d - %s\n", code, FindRawkeyString( code ) );
						if( mode )
						{
							modflag = TRUE;

							configs[ inputplayer ].RawKeys[inputpos] = code;
							WipeKey( inputplayer, inputpos );
							SetAPen( RP, 1 );
							ShowKey( inputplayer, inputpos, code );
							if( mode == 1 )
								mode = 0;
							else
							{
								if( ++inputpos >= 8 )
									mode = 0;
							}
						}
					}
					break;
				case IDCMP_INTUITICKS:
					if( quitcount )
					{
						if( --quitcount == 0 )
						{
							quitgad.GadgetText = &itextquit;
							RefreshGList( &quitgad, fenster, NULL, 1 );
						}
					}

					if( mode )
					{
							if( flashstate )
							{
								SetAPen( RP, 0 );
								ShowKey( inputplayer, inputpos,
									configs[ inputplayer ].RawKeys[inputpos] );
								flashstate = 0;
							}
							else
							{
								SetAPen( RP, 4 );
								ShowKey( inputplayer, inputpos,
									configs[ inputplayer ].RawKeys[inputpos] );
								flashstate = 1;
							}
					}
					break;
				case IDCMP_GADGETUP:
					gid = ((struct Gadget *)iaddress)->GadgetID;

					if( mode )
					{
						SetAPen( RP, 1 );
						ShowKey( inputplayer, inputpos,
							configs[ inputplayer ].RawKeys[inputpos] );
						mode = 0;
					}

					player = gid/256;

					if( player < 4 )
					{
						if( (gid & 255 ) < 8 )
						{
							/* it's one of the 8 key gadgets */
							mode = 2;		// get one key
							inputplayer = player;
							inputpos = gid & 255;
							flashstate = 0;
						}

						if( (gid & 255 ) == 8 )
						{
							/* the device gadget */
							CycleDevice( player, qualifier );
							modflag = TRUE;
						}
					}

					if( player == 42 )
					{
						if( (gid & 255 ) == 0 )
						{
							/* quit gadget */	
							if( quitcount || !modflag )
								done = TRUE;
							else
							{
								quitcount = 12;
								quitgad.GadgetText = &itextsure;
								RefreshGList( &quitgad, fenster, NULL, 1 );
							}
						}

						if( (gid & 255 ) == 1 )
						{
							/* Save Gadget */
							SaveKeysFile( filename );
							done = TRUE;
						}
					}

					break;
			}
			ReplyMsg( (struct Message *)msg );
		}
	}
}

/*******************************************************/

static void CycleDevice( int player, UWORD qual )
{

	switch( configs[player].Type)
	{
		case VJTYPE_STDJOYSTICK:
			if( configs[player].HWPort == 0 )
				configs[player].HWPort = 1;
			else
			{
				configs[player].Type = VJTYPE_VIRTUAL;
				configs[player].HWPort = 0;
			}
			break;
		case VJTYPE_VIRTUAL:
			configs[player].Type = VJTYPE_STDJOYSTICK;
			configs[player].HWPort = 0;
			break;
	}

	playergads[8][player].GadgetText = SussDeviceIText( player );

	RefreshGList( &playergads[8][player], fenster , NULL, 1 );
}

/*******************************************************/

static struct IntuiText *SussDeviceIText( int player )
{
	struct IntuiText *itext;

	static struct IntuiText itextport1 = {
		1,0,		//frontpen, backpen
		JAM2,		//drawmode
		4,2,		//l,r
		NULL,		//font
		"Joystick Port 1 ",
		NULL };		//next

	static struct IntuiText itextport2 = {
		1,0,		//frontpen, backpen
		JAM2,		//drawmode
		4,2,		//l,r
		NULL,		//font
		"Joystick Port 2 ",
		NULL };		//next

	static struct IntuiText itextnojoystick = {
		1,0,		//frontpen, backpen
		JAM2,		//drawmode
		4,2,		//l,r
		NULL,		//font
		"   Keys Only    ",
		NULL };		//next

	switch( configs[player].Type )
	{
		case VJTYPE_STDJOYSTICK:
			if( configs[player].HWPort == 0 )
				itext = &itextport1;
			else
				itext = &itextport2;
			break;
		default:
		case VJTYPE_VIRTUAL:
			itext = &itextnojoystick;
			break;
	}

	return( itext );
}

/*******************************************************/

char *FindRawkeyString( int rawkey )
{
	switch( rawkey )
	{
		case 0x00: return "`";
		case 0x01: return "1";
		case 0x02: return "2";
		case 0x03: return "3";
		case 0x04: return "4";
		case 0x05: return "5";
		case 0x06: return "6";
		case 0x07: return "7";
		case 0x08: return "8";
		case 0x09: return "9";
		case 0x0A: return "0";
		case 0x0B: return "-";
		case 0x0C: return "=";
		case 0x0D: return "\\";

		case 0x0F: return "0 (N)";

		case 0x10: return "Q";
		case 0x11: return "W";
		case 0x12: return "E";
		case 0x13: return "R";
		case 0x14: return "T";
		case 0x15: return "Y";
		case 0x16: return "U";
		case 0x17: return "I";
		case 0x18: return "O";
		case 0x19: return "P";
		case 0x1A: return "[";
		case 0x1B: return "]";

		case 0x1D: return "1 (N)";
		case 0x1E: return "2 (N)";
		case 0x1F: return "3 (N)";

		case 0x20: return "A";
		case 0x21: return "S";
		case 0x22: return "D";
		case 0x23: return "F";
		case 0x24: return "G";
		case 0x25: return "H";
		case 0x26: return "J";
		case 0x27: return "K";
		case 0x28: return "L";
		case 0x29: return ";";
		case 0x2A: return "'";

		case 0x2D: return "4 (N)";
		case 0x2E: return "5 (N)";
		case 0x2F: return "6 (N)";

		case 0x31: return "Z";
		case 0x32: return "X";
		case 0x33: return "C";
		case 0x34: return "V";
		case 0x35: return "B";
		case 0x36: return "N";
		case 0x37: return "M";
		case 0x38: return ",";
		case 0x39: return ".";
		case 0x3A: return "/";

		case 0x3C: return ". (N)";
		case 0x3D: return "7 (N)";
		case 0x3E: return "8 (N)";
		case 0x3F: return "9 (N)";

		case 0x40: return "Space";
		case 0x41: return "BackSpace";
		case 0x42: return "Tab";
		case 0x43: return "Enter (N)";
		case 0x44: return "Enter";
		case 0x45: return "Escape";
		case 0x46: return "Del";

		case 0x4A: return "- (N)";

		case 0x4C: return "CursorUp";
		case 0x4D: return "CursorDown";
		case 0x4E: return "CursorRight";
		case 0x4F: return "CursorLeft";

		case 0x50: return "F1";
		case 0x51: return "F2";
		case 0x52: return "F3";
		case 0x53: return "F4";
		case 0x54: return "F5";
		case 0x55: return "F6";
		case 0x56: return "F7";
		case 0x57: return "F8";
		case 0x58: return "F9";
		case 0x59: return "F10";

		case 0x5A: return "( (N)";
		case 0x5B: return ") (N)";
		case 0x5C: return "/ (N)";
		case 0x5D: return "* (N)";
		case 0x5E: return "+ (N)";
		case 0x5F: return "Help";

		case 0x60: return "LShift";
		case 0x61: return "RShift";
		case 0x62: return "CapsLock";
		case 0x63: return "Ctrl";
		case 0x64: return "LAlt";
		case 0x65: return "RAlt";
		case 0x66: return "LAmiga";
		case 0x67: return "RAmiga";

		case 0xFF: return "<none>";
		default: return "<unknown>";
	}
}


/*******************************************************/


static void AddMiscGadgets( void )
{

	static UWORD topleftpairs[] = { 1,0,
		MISCBUTTONWIDTH,0,
		MISCBUTTONWIDTH, MISCBUTTONHEIGHT };

	static UWORD bottomrightpairs[] = { 0,1,
		0, MISCBUTTONHEIGHT,
		MISCBUTTONWIDTH, MISCBUTTONHEIGHT };

	static struct Border buttonborder1 = {
		0,0,
		4,0,	//frontpen,backpen
		JAM1,	//DrawMode
		3,		//Count
		bottomrightpairs,	//XY pairs.
		NULL }; //Next

	static struct Border buttonborder = {
		0,0,
		1,0,	//frontpen,backpen
		JAM1,	//DrawMode
		3,		//Count
		topleftpairs,	//XY pairs.
		&buttonborder1 }; //Next

	static struct Border buttonborderselected1 = {
		0,0,
		1,0,	//frontpen,backpen
		JAM1,	//DrawMode
		3,		//Count
		bottomrightpairs,	//XY pairs.
		NULL }; //Next

	static struct Border buttonborderselected = {
		0,0,
		4,0,	//frontpen,backpen
		JAM1,	//DrawMode
		3,		//Count
		topleftpairs,	//XY pairs.
		&buttonborderselected1 }; //Next

	static struct IntuiText itextsave = {
		1,0,		//frontpen, backpen
		JAM2,		//drawmode
		4,2,		//l,r
		NULL,		//font
		"  Save  ",
		NULL };		//next

	struct Gadget *gad;

	gad = &quitgad;
	gad->NextGadget = NULL;
	gad->TopEdge = fenster->Height - (MISCBUTTONHEIGHT+1);
	gad->LeftEdge = fenster->Width - (MISCBUTTONWIDTH+4);
	gad->Width = MISCBUTTONWIDTH;
	gad->Height = MISCBUTTONHEIGHT;
	gad->Flags = GFLG_GADGHIMAGE;
	gad->Activation = GACT_RELVERIFY;
	gad->GadgetType = GTYP_BOOLGADGET;
	gad->GadgetText = &itextquit;
	gad->MutualExclude = 0;
	gad->SpecialInfo = 0;
	gad->GadgetID = ( 42 * 256 ) + 0;
	gad->UserData = NULL;
	gad->GadgetRender = &buttonborder;
	gad->SelectRender = &buttonborderselected;
	AddGadget( fenster, gad, -1 );

	gad = &savegad;
	gad->NextGadget = NULL;
	gad->TopEdge = fenster->Height - (MISCBUTTONHEIGHT+1);
	gad->LeftEdge = fenster->Width - ((MISCBUTTONWIDTH+4)*2);
	gad->Width = MISCBUTTONWIDTH;
	gad->Height = MISCBUTTONHEIGHT;
	gad->Flags = GFLG_GADGHIMAGE;
	gad->Activation = GACT_RELVERIFY;
	gad->GadgetType = GTYP_BOOLGADGET;
	gad->GadgetText = &itextsave;
	gad->MutualExclude = 0;
	gad->SpecialInfo = 0;
	gad->GadgetID = ( 42 * 256 ) + 1;
	gad->UserData = NULL;
	gad->GadgetRender = &buttonborder;
	gad->SelectRender = &buttonborderselected;
	AddGadget( fenster, gad, -1 );
}


/*******************************************************/

static BOOL LoadKeysFile( char *name )
{
	BPTR fh;
	BOOL success = FALSE;
	int i;

	if( fh = Open( name, MODE_OLDFILE ) )
	{
		if( Read( fh, configs, sizeof configs ) == sizeof configs )
			success = TRUE;
/*
		else
		{
			for( i=0; i<4; i++ )
			{
				configs[i] = { VJTYPE_VIRTUAL,0,0xFF,0xFF,0xFF,0xFF,
					0xFF,0xFF,0xFF,0xFF,0,0,0,0 };
			}
		}
*/

		Close( fh );
	}
	return( success );
}

/*******************************************************/

static BOOL SaveKeysFile( char *name )
{
	BPTR fh;
	BOOL success = FALSE;

	if( fh = Open( name, MODE_NEWFILE ) )
	{
		if( Write( fh, configs, sizeof configs ) == sizeof configs )
			success = TRUE;
		Close( fh );
	}
	return( success );
}


/*******************************************************/

static void ListKeysFiles( char *dirname, struct List *l )
{
	struct FileInfoBlock *fib;
	BPTR currentdir;
	int len;
	struct KeyNode *kn;

	fib = AllocMem( sizeof( struct FileInfoBlock ), MEMF_ANY|MEMF_CLEAR );
	if( fib )
	{
		currentdir = Lock( dirname, ACCESS_READ );
		if( currentdir )
		{
			if( Examine( currentdir, fib ) )
			{
				while( ExNext( currentdir, fib ) )
				{
//					printf("%s\n",fib->fib_FileName );
					if( fib->fib_DirEntryType < 0 )
					{
						/* 'tis a file */

						len = strlen( fib->fib_FileName );

						if( len > 5 )
						{
							if( fib->fib_FileName[len-5] == '.' &&
								fib->fib_FileName[len-4] == 'g' &&
								fib->fib_FileName[len-3] == 'a' &&
								fib->fib_FileName[len-2] == 'm' &&
								fib->fib_FileName[len-1] == 'e' )
							{
								/* found a '.game' file! */

								if( kn = AllocMem( sizeof( struct KeyNode ),
									MEMF_ANY|MEMF_CLEAR ) )
								{
									Mystrncpy( kn->Name, fib->fib_FileName, 108-1 );
									AddTail( l, (struct Node *)kn );
								}
							}
						}
					}
				}
			}
			else
				printf("Examine() failed\n");
			UnLock( currentdir );
		}
		else
			printf("Lock() failed\n");
		FreeMem( fib, sizeof( struct FileInfoBlock ) );
	}
}


/*******************************************************/

void FreeKeyNodesList( struct List *l )
{
	struct KeyNode *kn;

	while( !IsListEmpty( l ) )
	{
		kn = (struct KeyNode *)l->lh_Head;
		Remove( (struct Node *)kn );
		FreeMem( kn, sizeof( struct KeyNode ) );
	}
}


/*******************************************************/

void AddKeyNodeGadgets( struct List *l )
{

	static UWORD topleftpairs[] = { 1,0,
		FILEBUTTONWIDTH,0,
		FILEBUTTONWIDTH, FILEBUTTONHEIGHT };

	static UWORD bottomrightpairs[] = { 0,1,
		0, FILEBUTTONHEIGHT,
		FILEBUTTONWIDTH, FILEBUTTONHEIGHT };

	static struct Border buttonborder1 = {
		0,0,
		4,0,	//frontpen,backpen
		JAM1,	//DrawMode
		3,		//Count
		bottomrightpairs,	//XY pairs.
		NULL }; //Next

	static struct Border buttonborder = {
		0,0,
		1,0,	//frontpen,backpen
		JAM1,	//DrawMode
		3,		//Count
		topleftpairs,	//XY pairs.
		&buttonborder1 }; //Next

	static struct Border buttonborderselected1 = {
		0,0,
		1,0,	//frontpen,backpen
		JAM1,	//DrawMode
		3,		//Count
		bottomrightpairs,	//XY pairs.
		NULL }; //Next

	static struct Border buttonborderselected = {
		0,0,
		4,0,	//frontpen,backpen
		JAM1,	//DrawMode
		3,		//Count
		topleftpairs,	//XY pairs.
		&buttonborderselected1 }; //Next

	struct KeyNode *kn;
	int x,y,i;
	struct IntuiText *it;
	struct Gadget *gad;

	x=0;
	y=0;
	i=0;

	for( kn = (struct KeyNode *)l->lh_Head;
		kn->Noddy.ln_Succ;
		kn = (struct KeyNode *)kn->Noddy.ln_Succ )
	{
		Mystrncpy( kn->GadLabel, kn->Name, FILEBUTTONCHARS-1 );
		/* suss IText */
		it = &kn->IText;
		it->FrontPen = 1;
		it->BackPen = 0;
		it->DrawMode = JAM2;
		it->LeftEdge = 4;
		it->TopEdge = 2;
		it->ITextFont = NULL;
		it->IText = kn->GadLabel;
		it->NextText = NULL;

		/* sort out the gadget */
		gad = &kn->Inspector;
		gad->NextGadget = NULL;
		gad->LeftEdge = x;
		gad->TopEdge = y;
		gad->Width = FILEBUTTONWIDTH;
		gad->Height = FILEBUTTONHEIGHT;
		gad->Flags = GFLG_GADGHIMAGE;
		gad->Activation = GACT_RELVERIFY;
		gad->GadgetType = GTYP_BOOLGADGET;
		gad->GadgetText = &kn->IText;
		gad->MutualExclude = 0;
		gad->SpecialInfo = 0;
		gad->GadgetID = i++;
		gad->UserData = NULL;
		gad->GadgetRender = &buttonborder;
		gad->SelectRender = &buttonborderselected;
		AddGadget( fenster, gad, -1 );

		y += FILEBUTTONHEIGHT + 2;
		if( ( y + FILEBUTTONHEIGHT ) > fenster->Height/4 )
		{
			y = 0;
			x += FILEBUTTONWIDTH + 4;
		}
	}
}


static int PickFile( void )
{
	struct	IntuiMessage		*msg;
	ULONG		sigflags;
	BOOL		done = FALSE;

	ULONG class;
	UWORD code;
	UWORD qualifier;
	APTR iaddress;

	int gid = -1;

	while ( !done )
	{
		sigflags = Wait( 1L << fenster->UserPort->mp_SigBit );

		while(msg = (struct IntuiMessage *)GetMsg( fenster->UserPort ) )
		{
			class = msg->Class;
			code = msg->Code;
			qualifier = msg->Qualifier;
			iaddress = msg->IAddress;

			switch( class )
			{
				case IDCMP_RAWKEY:

					if( code == 0x45 )		// esc -> <none>
						done = TRUE;
				break;
			case IDCMP_INTUITICKS:
				break;
			case IDCMP_GADGETUP:
				gid = ((struct Gadget *)iaddress)->GadgetID;
				done = TRUE;
				break;
			}
			ReplyMsg( (struct Message *)msg );
		}
	}

	return gid;
}




/*********************  Mystrncpy()  ***********************/
//  This is just a slight varience from the original strncpy()
// in that this puts a null terminator on the string, after
// the last character, so that Mystrncpy(to,from,15) would copy
// the first 15 characters then put in a terminator.

char *Mystrncpy(char *to, char *from, int length)
{
	register char *dest = to;
	register char *src = from;

	while( length-- && *src )
		*dest++ = *src++;
	*dest = '\0';

	return( to );
}


/*******************  FindNthNode()  *******************/
//
// Returns a ptr to the Nth node in the specified list.
// The counting begins at zero (0 = the node after the head node).
// NULL is returned if the node doesn't exist.
//

struct Node *FindNthNode(struct List *list, UWORD num)
{
	struct Node *node;

	if( IsListEmpty(list) ) return(NULL);
	node = list->lh_Head;
	while( num )
	{
		node = node->ln_Succ;
		if(!node->ln_Succ) return(NULL);
		--num;
	}
	return(node);
}

/*******************************************************/


void QuickyReq( char *text )
{
	static UWORD topleftpairs[] = { 1,0,
		MISCBUTTONWIDTH,0,
		MISCBUTTONWIDTH, MISCBUTTONHEIGHT };

	static UWORD bottomrightpairs[] = { 0,1,
		0, MISCBUTTONHEIGHT,
		MISCBUTTONWIDTH, MISCBUTTONHEIGHT };

	static struct Border buttonborder1 = {
		0,0,
		4,0,	//frontpen,backpen
		JAM1,	//DrawMode
		3,		//Count
		bottomrightpairs,	//XY pairs.
		NULL }; //Next

	static struct Border buttonborder = {
		0,0,
		1,0,	//frontpen,backpen
		JAM1,	//DrawMode
		3,		//Count
		topleftpairs,	//XY pairs.
		&buttonborder1 }; //Next

	static struct Border buttonborderselected1 = {
		0,0,
		1,0,	//frontpen,backpen
		JAM1,	//DrawMode
		3,		//Count
		bottomrightpairs,	//XY pairs.
		NULL }; //Next

	static struct Border buttonborderselected = {
		0,0,
		4,0,	//frontpen,backpen
		JAM1,	//DrawMode
		3,		//Count
		topleftpairs,	//XY pairs.
		&buttonborderselected1 }; //Next

	static struct IntuiText itextok = {
		1,0,		//frontpen, backpen
		JAM2,		//drawmode
		4,2,		//l,r
		NULL,		//font
		"   OK   ",
		NULL };		//next

	struct Gadget okgad, *gad;
	char *p;
	int l,y;
	BOOL done=FALSE;
	struct	IntuiMessage		*msg;
	ULONG		sigflags;
	UWORD		class;

	SetRast( RP, 0 );
	SetAPen( RP, 1 );

	p = text;
	l=0;
	y=8;

	while( !done )
	{
		if( *p == '\0' || *p == '\n' )
		{
			Move( RP, 0, y );
			Text( RP, text, l );
			if( *p == '\0' )
				done = TRUE;
			else
			{
				p++;
				text = p;
				l=0;
				y += 10;
			}
		}
		else
		{
			p++;
			l++;
		}
	}

	/* sort out the gadget */
	gad = &okgad;
	gad->NextGadget = NULL;
	gad->LeftEdge = 16;
	gad->TopEdge = y+12;
	gad->Width = MISCBUTTONWIDTH;
	gad->Height = MISCBUTTONHEIGHT;
	gad->Flags = GFLG_GADGHIMAGE;
	gad->Activation = GACT_RELVERIFY;
	gad->GadgetType = GTYP_BOOLGADGET;
	gad->GadgetText = &itextok;
	gad->MutualExclude = 0;
	gad->SpecialInfo = 0;
	gad->GadgetID = 0;
	gad->UserData = NULL;
	gad->GadgetRender = &buttonborder;
	gad->SelectRender = &buttonborderselected;
	AddGadget( fenster, gad, -1 );
	RefreshGadgets( gad, fenster, NULL );

	done = FALSE;
	while ( !done )
	{
		sigflags = Wait( 1L << fenster->UserPort->mp_SigBit );

		while(msg = (struct IntuiMessage *)GetMsg( fenster->UserPort ) )
		{
			class = msg->Class;

			switch( class )
			{
			case IDCMP_RAWKEY:
				break;
			case IDCMP_INTUITICKS:
				break;
			case IDCMP_GADGETUP:
				done = TRUE;
				break;
			}
			ReplyMsg( (struct Message *)msg );
		}
	}

	RemoveGList( fenster, gad, -1 );
}


