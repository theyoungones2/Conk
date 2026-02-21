
/* contents of the HOStuff struct are Hoopyvision-private */

struct HOStuff
{
	struct Window 		*hos_Win;
	struct RastPort		*hos_RPort;
	APTR							hos_vi;
	struct DrawInfo		*hos_dri;
	struct Gadget			*hos_GList;
	struct TextFont		*hos_tf;
	struct TextAttr		hos_TextAttr;
	BOOL							hos_BigHoriz;		/* Horizontally hires? (approx width > 600) */
	BOOL							hos_BigVert;		/* Vertically hires? (approx height > 380) */
	UWORD							hos_ScrollyWidth;
	UWORD							hos_ScrollyHeight;
	UWORD							Reserved[2];		/* for vspace/hspace? */
};

#define VSPACE 2				/* vertical borderspace around objects */
#define HSPACE 4				/* horizontal bordersapce around objects */

//obsolete
#define SCROLLYWIDTH 18
#define SCROLLYHEIGHT 10

/* For fixed point values */
#define FIXMULT 32
#define FIXMULTFP 32.0

struct HoopyObject
{
	UWORD	ho_Type;				/* object type - see below */
	UWORD	ho_Weight;			/* used with HOFLG_FREEWIDTH & HOFLG_FREEHEIGHT */
	UWORD	ho_HOFlags;			/* see below... */
	ULONG	ho_Attr0;				/* object attributes, used in object creation... */
	ULONG	ho_Attr1;				/* ...4 longs, use depends on object type */
	ULONG	ho_Attr2;
	ULONG	ho_Attr3;
	UBYTE	*ho_Text;				/* ptr to any string the object might need */

	ULONG	ho_Value;				/* current gadget value (eg 0 or 1 for checkbox) */

	UWORD	ho_GadgetID;		/* gadget ID */
	ULONG	ho_Flags;				/* gadget flags ( from GadTools )*/
	struct Gadget *ho_Gadget;	/* filled out by hoopyvision if this is a GT gadget */
	WORD	ho_Size;				/* private */
};

#define HOFLG_FREEWIDTH		0x1				/* this object horizontally stretchable */
#define HOFLG_FREEHEIGHT	0x2				/* this object vertically stretchable */
#define HOFLG_NOBORDER		0x4				/* no borderspace around this object please */
#define HOFLG_RAISEDBORDER	0x8
#define HOFLG_RECESSBORDER	0x10
#define HOFLG_DISABLED			0x20		/* disable this object (if possible ) */

#define HOFLG_SIZESUSSED	0x8000		/* private */

/* HoopyObject Types: */

#define HOTYPE_END			0
#define HOTYPE_VGROUP		1
#define HOTYPE_HGROUP		2
#define HOTYPE_SPACE		3
#define HOTYPE_BUTTON		4
#define HOTYPE_CHECKBOX	5
#define HOTYPE_INTEGER	6
#define HOTYPE_LISTVIEW	7
#define HOTYPE_MX				8
#define HOTYPE_NUMBER		9
#define HOTYPE_CYCLE		10
#define HOTYPE_PALETTE	11
#define HOTYPE_VSCROLLER	12
#define HOTYPE_HSCROLLER	13
#define HOTYPE_VSLIDER		14
#define HOTYPE_HSLIDER	15
#define HOTYPE_STRING		16
#define HOTYPE_TEXT			17

#define HOTYPE_IMAGEBUTTON	18
#define HOTYPE_HLINE				19
#define HOTYPE_VLINE				20
#define HOTYPE_SCROLLYSPACE		21
#define HOTYPE_FIXEDPOINT			22
#define HOTYPE_PACKEDSTRING		23

/*
VGROUP:
	Vertical group of any number (or types) of objects (including other groups).
	The child objects immediately follow their parent group object.
	ho_Attr0: number of objects in this group
	ho_Value: not used.

HGROUP:
	Horizontal group of any number (or types) of objects (including other groups).
	The child objects immediately follow their parent group object.
	ho_Attr0: number of objects in this group
	ho_Value: not used.

SPACE:
	A blank space for use as a placeholder to seperate objects.
	Current position and dimensions of space is put into ho_Attr0 and ho_Attr1
	respectively by HO_MakeLayout():
	High word of ho_Attr0 is Left, low word is Top,
	High word of ho_Attr1 is Width, low word is Height.

CYCLE:
	Standard Gadtools gadget.
	ho_Attr0: points to null terminated array of STRPTRs to cycle through
	ho_Value: currently selected item

CHECKBOX:
	Standard Gadtools gadget.
	ho_Value: TRUE = checked, FALSE = not.

MX:
	Standard Gadtools gadget.
	ho_Attr0: ptr to null terminated array of strings to select from
	ho_Attr1: vertical spacing between items (in pixels)
	ho_Value: currently selected item

STRING:
	Standard Gadtools gadget.
	ho_Attr0: ptr to buffer to store string
	ho_Attr1: length of buffer
	ho_Value: not used - just read from buffer (ho_Attr0).

INTEGER:
	Standard Gadtools gadget.
	ho_Attr0: maxchars in gadget
	ho_Value: Long integer value


PACKEDSTRING:
	Standard Gadtools gadget with edithook to only allow
	packable characters (see Wibble.s).
	ho_Attr0: ptr to buffer to store string
	ho_Attr1: length of buffer
	ho_Value: not used - just read from buffer (ho_Attr0).

FIXEDPOINT:
	For entering fixed point values. Precision set by FIXMULT define.
	Numbers can be entered as nn.nn (like standard floats) or nn:nn
	(direct representation of number).
	ho_Attr0: maxchars in gadget
	ho_Value: 32bit FixedPoint value

BUTTON:
	Standard Gadtools gadget.

LISTVIEW:
	Standard Gadtools gadget.
	ho_Attr0: List
	ho_Attr1: BOOL readonly

TEXT:
	ho_Attr0: contents of text gadget

NUMBER:
	ho_Attr0: number to show

VSCROLLER/HSCROLLER:
	ho_Attr0: visible
	ho_Attr1: total
	ho_Value: top

VSLIDER/HSLIDER:
	ho_Attr0: min value
	ho_Attr1: max value
	ho_Value: current value


FUNCTIONS:

void HO_DisableObject( struct HoopyObject *hob, struct HOStuff *hos, BOOL state );
void HO_RefreshObject( struct HoopyObject *hob, struct HOStuff *hos );


*/

