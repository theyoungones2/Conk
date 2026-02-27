 #include <stdio.h>
//#include <stdarg.h>
#include <string.h>
//#include <dos/dos.h>
//#include <dos/dostags.h>
#include <exec/exec.h>
//#include <intuition/intuition.h>
//#include <graphics/gfxbase.h>
//#include <rexx/rxslib.h>
#include <libraries/gadtools.h>
//#include <libraries/diskfont.h>
//#include <libraries/asl.h>

#include <proto/exec.h>
//#include <proto/dos.h>
#include <proto/intuition.h>
#include <proto/graphics.h>
//#include <proto/diskfont.h>
//#include <proto/asl.h>
//#include <clib/alib_protos.h>
#include <proto/gadtools.h>
#include <proto/iffparse.h>

#include <global.h>
#include <ExternVars.h>
#include <HoopyVision.h>

#define	ID_ANIM		MAKE_ID('A','N','I','M')
#define	ID_CONK		MAKE_ID('C','O','N','K')

/* menus */

static struct NewMenu	seqmenu[] =
	{
		{ NM_TITLE,	"Project",							0,	0,	0,	0,},
		{  NM_ITEM,	"New",								 "N",	0,	0,	0,},
		{  NM_ITEM,	"Open...",						 "O",	0,	0,	0,},
		{	 NM_ITEM,	NM_BARLABEL,						0,	0,	0,	0,},
		{  NM_ITEM,	"Save",								 "S",	0,	0,	0,},
		{  NM_ITEM,	"Save As...",					 "A",	0,	0,	0,},
		{	 NM_ITEM,	NM_BARLABEL,						0,	0,	0,	0,},
		{  NM_ITEM,	"Hide",									0,	NM_ITEMDISABLED,	0,	0,},
		{  NM_ITEM,	"Reveal...",						0,	0,	0,	0,},
		{  NM_ITEM,	"Close",								0,	NM_ITEMDISABLED,	0,	0,},
		{	 NM_ITEM,	NM_BARLABEL,						0,	0,	0,	0,},
		{  NM_ITEM,	"Page Size...",					0,	NM_ITEMDISABLED,	0,	0,},
		{  NM_ITEM,	"Screen Mode...",				0,	0,	0,	0,},
		{	 NM_ITEM,	NM_BARLABEL,						0,	0,	0,	0,},
		{  NM_ITEM,	"Iconify...",						0,	0,	0,	0,},
		{  NM_ITEM,	"About...",							0,	0,	0,	0,},
		{  NM_ITEM,	"Quit Program...",		 "Q",	0,	0,	0,},

/* 22 */
		{ NM_TITLE,	"Edit",									0,	NM_MENUDISABLED,	0,	0,},
		{  NM_ITEM,	"Colour",								0,	0,	0,	0,},
		{  	NM_SUB,	"Use Palette",				 "P",	NM_MENUDISABLED,	0,	0,},
		{  	NM_SUB,	"Remap",								0,	NM_MENUDISABLED,	0,	0,},
		{  	NM_SUB,	"Load Palette...",			0,	0,	0,	0,},
		{  	NM_SUB,	"Save Palette...",			0,	0,	0,	0,},
		{  NM_ITEM,	"Undo",								 "Z",	0,	0,	0,},
		{  NM_ITEM,	"Name Chunk...",				0,	0,	0,	0,},
		{	 NM_ITEM,	NM_BARLABEL,						0,	0,	0,	0,},
		{  NM_ITEM,	"Blocks",								0,	0,	0,	0,},
		{  	NM_SUB,	"Dummy",								0,	0,	0,	0,},
		{  	NM_SUB,	"Dummy",								0,	0,	0,	0,},
		{  NM_ITEM,	"Bobs",									0,	0,	0,	0,},
		{  	NM_SUB,	"Previous",						 "1",	0,	0,	0,},
		{  	NM_SUB,	"Next",								 "2",	0,	0,	0,},
		{  	NM_SUB,	"Goto...",						 "3",	0,	0,	0,},
		{  	NM_SUB,	"Play...",						 "4",	0,	0,	0,},
		{  	NM_SUB,	"Ping Pong...",				 "6",	0,	0,	0,},
		{	  NM_SUB,	NM_BARLABEL,						0,	0,	0,	0,},
		{  	NM_SUB,	"Insert...",						0,	0,	0,	0,},
		{  	NM_SUB,	"Delete...",						0,	0,	0,	0,},
		{  	NM_SUB,	"Copy...",							0,	0,	0,	0,},
		{	  NM_SUB,	NM_BARLABEL,						0,	0,	0,	0,},
		{  	NM_SUB,	"Edit Mask?",					 "`",	CHECKIT|MENUTOGGLE,	0,	0,},
		{  	NM_SUB,	"Delete Mask",					0,	0,	0,	0,},
		{  	NM_SUB,	"Remake Mask",					0,	0,	0,	0,},
		{	  NM_SUB,	NM_BARLABEL,						0,	0,	0,	0,},
		{  	NM_SUB,	"Edit Zone",						0,	0,	0,	0,},
		{  	NM_SUB,	"Delete Zone",					0,	0,	0,	0,},
		{  	NM_SUB,	"Remake Zone",					0,	0,	0,	0,},
		{  NM_ITEM,	"Sprites",							0,	0,	0,	0,},
		{  	NM_SUB,	"Previous",						 "1",	0,	0,	0,},
		{  	NM_SUB,	"Next",								 "2",	0,	0,	0,},
		{  	NM_SUB,	"Goto...",						 "3",	0,	0,	0,},
		{  	NM_SUB,	"Play...",						 "4",	0,	0,	0,},
		{  	NM_SUB,	"Ping Pong...",				 "6",	0,	0,	0,},
		{	  NM_SUB,	NM_BARLABEL,						0,	0,	0,	0,},
		{  	NM_SUB,	"Insert...",						0,	0,	0,	0,},
		{  	NM_SUB,	"Delete...",						0,	0,	0,	0,},
		{  	NM_SUB,	"Copy...",							0,	0,	0,	0,},
		{	  NM_SUB,	NM_BARLABEL,						0,	0,	0,	0,},
		{  	NM_SUB,	"Set Number...",				0,	0,	0,	0,},
		{	  NM_SUB,	NM_BARLABEL,						0,	0,	0,	0,},
		{  	NM_SUB,	"Edit Zone",						0,	0,	0,	0,},
		{  	NM_SUB,	"Delete Zone",					0,	0,	0,	0,},
		{  	NM_SUB,	"Remake Zone",					0,	0,	0,	0,},
		{  NM_ITEM,	"ILBM",									0,	0,	0,	0,},
		{  	NM_SUB,	"Dummy",								0,	0,	0,	0,},
		{  	NM_SUB,	"Dummy",								0,	0,	0,	0,},
		{  NM_ITEM,	NM_BARLABEL,						0,	0,	0,	0,},
		{  NM_ITEM,	"Blocks",								0,	CHECKIT|MENUTOGGLE,	0,	0,},
		{  NM_ITEM,	"Bobs",									0,	CHECKIT|MENUTOGGLE,	0,	0,},
		{  NM_ITEM,	"Sprites",							0,	CHECKIT|MENUTOGGLE,	0,	0,},
		{  NM_ITEM,	"ILBM",									0,	CHECKIT|MENUTOGGLE,	0,	0,},

/* 67 */
		{ NM_TITLE,	"Window",								0,	0,	0,	0,},
		{  NM_ITEM,	"Open",									0,	0,	0,	0,},
		{   NM_SUB,	"Project...",					 "W",	0,	0,	0,},
		{   NM_SUB,	"About...",						 	0,	0,	0,	0,},
		{   NM_SUB,	"Reveal...",						0,	0,	0,	0,},
		{   NM_SUB,	"ScreenConfig...",			0,	0,	0,	0,},
		{   NM_SUB,	"Tools...",							0,	0,	0,	0,},
		{   NM_SUB,	"UserConfig...",				0,	0,	0,	0,},
		{   NM_SUB,	"Palette...",						0,	0,	0,	0,},
		{   NM_SUB,	"EditPalette...",				0,	0,	0,	0,},
		{   NM_SUB,	"Sequencer...",					0,	0,	0,	0,},
		{   NM_SUB,	"Range...",							0,	0,	0,	0,},
		{  NM_ITEM,	"Hide",									0,	0,	0,	0,},
		{  NM_ITEM,	"Reveal...",						0,	0,	0,	0,},
		{  NM_ITEM,	"Close",								0,	0,	0,	0,},
		{  NM_ITEM,	"Make Default",					0,	0,	0,	0,},
		{  NM_ITEM,	NM_BARLABEL,						0,	0,	0,	0,},
		{  NM_ITEM,	"Backdrop?",						0,	CHECKIT|MENUTOGGLE,	0,	0,},
		{  NM_ITEM,	"Borderless?",					0,	CHECKIT|MENUTOGGLE,	0,	0,},
		{  NM_ITEM,	"Title?",								0,	CHECKIT|MENUTOGGLE,	0,	0,},
		{  NM_ITEM,	"BorderGadgets?",				0,	CHECKIT|MENUTOGGLE,	0,	0,},
		{  NM_ITEM,	NM_BARLABEL,						0,	0,	0,	0,},
		{  NM_ITEM,	"Vertical?",						0,	CHECKIT|MENUTOGGLE|NM_ITEMDISABLED,	0,	0,},

/* 90 */
		{ NM_TITLE,	"Prefs",								0,	0,	0,	0,},
		{  NM_ITEM, "Save Chunky?",					0,	CHECKIT|MENUTOGGLE,	0,	0,},
		{  NM_ITEM, "Make Backups?",				0,	CHECKIT|MENUTOGGLE,	0,	0,},
		{  NM_ITEM, "Auto Activate?",				0,	CHECKIT|MENUTOGGLE,	0,	0,},
		{  NM_ITEM, "Keep Screen Mode?",		0,	CHECKIT|MENUTOGGLE,	0,	0,},
		{  NM_ITEM, "Exclude Brush?",				0,	CHECKIT|MENUTOGGLE,	0,	0,},
		{  NM_ITEM, "Show Window IDs?",			0,	CHECKIT|MENUTOGGLE,	0,	0,},
		{  NM_ITEM, "Create Icons?",				0,	CHECKIT|MENUTOGGLE,	0,	0,},
		{  NM_ITEM,	NM_BARLABEL,						0,	0,	0,	0,},
		{  NM_ITEM,	"Screen Settings...",		0,	0,	0,	0,},
		{  NM_ITEM,	NM_BARLABEL,						0,	0,	0,	0,},
		{  NM_ITEM,	"Load Settings...",			0,	0,	0,	0,},
		{  NM_ITEM,	"Save Settings",				0,	0,	0,	0,},
		{  NM_ITEM,	"Save Settings As...",	0,	0,	0,	0,},

/* 104 */
		{ NM_TITLE,	"Mode",									0,	0,	0,	0,},
		{  NM_ITEM, "Matte",								0,	CHECKIT|MENUTOGGLE,	0,	0,},
		{  NM_ITEM, "Colour",								0,	CHECKIT|MENUTOGGLE,	0,	0,},
		{  NM_ITEM, "Replace",							0,	CHECKIT|MENUTOGGLE,	0,	0,},
		{  NM_ITEM, "Shade",								0,	CHECKIT|MENUTOGGLE,	0,	0,},
		{  NM_ITEM, "Smooth",								0,	CHECKIT|MENUTOGGLE,	0,	0,},
		{  NM_ITEM, "Cycle",								0,	CHECKIT|MENUTOGGLE,	0,	0,},
		{  NM_ITEM, "Dummy",								0,	CHECKIT|MENUTOGGLE,	0,	0,},
		{  NM_ITEM, "Dummy",								0,	CHECKIT|MENUTOGGLE,	0,	0,},
		{  NM_ITEM, "Dummy",								0,	CHECKIT|MENUTOGGLE,	0,	0,},

/* 114 */
		{ NM_TITLE,	"Brush",								0,	0,	0,	0,},
		{  NM_ITEM, "AnimBrush",						0,	0,	0,	0,},
		{   NM_SUB,	"Load...",							0,	0,	0,	0,},
		{   NM_SUB,	"Save As...",						0,	0,	0,	0,},
		{   NM_SUB,	"Pick Up...",						0,	0,	0,	0,},
		{   NM_SUB,	"Settings...",					0,	0,	0,	0,},
		{   NM_SUB,	"Use",									0,	0,	0,	0,},
		{   NM_SUB,	"Free",									0,	0,	0,	0,},
		{  NM_ITEM, "Load...",							0,	0,	0,	0,},
		{  NM_ITEM, "Save...",							0,	0,	0,	0,},
		{  NM_ITEM, "Copy",								 "C",	0,	0,	0,},
		{  NM_ITEM, "Paste",							 "V",	0,	0,	0,},
		{  NM_ITEM, "Restore",						 "B",	0,	0,	0,},
		{  NM_ITEM, "Size",									0,	0,	0,	0,},
		{   NM_SUB,	"Stretch",							0,	0,	0,	0,},
		{   NM_SUB,	"Halve",								0,	0,	0,	0,},
		{   NM_SUB,	"Double",								0,	0,	0,	0,},
		{   NM_SUB,	"Double Horiz",					0,	0,	0,	0,},
		{   NM_SUB,	"Double Vert",					0,	0,	0,	0,},
		{  NM_ITEM, "Flip",									0,	0,	0,	0,},
		{   NM_SUB,	"Horizontal",						0,	0,	0,	0,},
		{   NM_SUB,	"Vertical",							0,	0,	0,	0,},
		{   NM_SUB,	"Rotate 90°",						0,	0,	0,	0,},
		{  NM_ITEM, "Outline",						 "o",	0,	0,	0,},
		{  NM_ITEM, "Handle",								0,	0,	0,	0,},
		{   NM_SUB,	"Rotate",								0,	0,	0,	0,},
		{   NM_SUB,	"Place",								0,	0,	0,	0,},
		{  NM_ITEM, "Free",									0,	0,	0,	0,},

/* 142 */
		{ NM_END,		NULL,										0,	0,	0,	0,},
		{ NM_END,		NULL,										0,	0,	0,	0,},

		{ NM_END,		NULL,										0,	0,	0,	0,},
		{ NM_END,		NULL,										0,	0,	0,	0,},
		{ NM_END,		NULL,										0,	0,	0,	0,},
		{ NM_END,		NULL,										0,	0,	0,	0,},
		{ NM_END,		NULL,										0,	0,	0,	0,},
		{ NM_END,		NULL,										0,	0,	0,	0,},
		{ NM_END,		NULL,										0,	0,	0,	0,},
		{ NM_END,		NULL,										0,	0,	0,	0,},
		{ NM_END,		NULL,										0,	0,	0,	0,},
		{ NM_END,		NULL,										0,	0,	0,	0,},
		{ NM_END,		NULL,										0,	0,	0,	0,},
		{ NM_END,		NULL,										0,	0,	0,	0,},
		{ NM_END,		NULL,										0,	0,	0,	0,},
		{ NM_END,		NULL,										0,	0,	0,	0,},
		{ NM_END,		NULL,										0,	0,	0,	0,},
		{ NM_END,		NULL,										0,	0,	0,	0,},
		{ NM_END,		NULL,										0,	0,	0,	0,},
		{ NM_END,		NULL,										0,	0,	0,	0,},
		{ NM_END,		NULL,										0,	0,	0,	0,},
		{ NM_END,		NULL,										0,	0,	0,	0,},
	};

#define MN_PROJECT					0
#define MN_EDIT							1
#define MN_WINDOW						2
#define MN_SETTINGS					3
#define MN_MODE							4
#define MN_BRUSH						5
#define MN_USER							6

#define MN_EDIT_IDX					17
#define MN_WINDOW_IDX				71
#define MN_SETTINGS_IDX			94
#define MN_MODE_IDX					108
#define MN_BRUSH_IDX				116
#define MN_USER_IDX					146

#define MYGT_ADDFRAME					0
#define MYGT_ADDRANGE					1
#define MYGT_DELFRAME					2
#define MYGT_DELRANGE					3
#define MYGT_NEWANIM					4
#define MYGT_DELANIM					5
#define MYGT_OFFSET						6
#define MYGT_PLAY							7
#define MYGT_HITZONES					8
#define MYGT_NAMEBOBS					9

#define MYGT_BOBNAME					10
#define MYGT_BOBNUMBERMINUS		11
#define MYGT_BOBNUMBER				12
#define MYGT_BOBNUMBERPLUS		13

#define MYGT_ANIMNAME					14
#define MYGT_ANIMNUMBERMINUS	15
#define MYGT_ANIMNUMBER 			16
#define MYGT_ANIMNUMBERPLUS		17

#define MYGT_FRAMENUMBERMINUS	18
#define MYGT_FRAMENUMBER			19
#define MYGT_FRAMENUMBERPLUS	20

#define MYGT_XPOS							21
#define MYGT_YPOS							22
#define MYGT_BOBNUMBER2				23

#define MYGT_ANIMSPEED				24

#define MYGT_RELATIVEPOSITIONS 25
/* REMEMBER:  Change SEQ_NUMOFGADGETS */
#define MYGT_NEWPOS 26
/* Dummy Gadget for RefreshGadget() routine */

/* labels for the Windows/Projects selector (a cycle gadget) */

struct FrameFileHeader
{
	ULONG	ffh_BobNumber;
	LONG	ffh_XOffset;
	LONG	ffh_YOffset;
};
struct AnimFileHeader
{
	UBYTE afh_AnimName[ANIMFILENAMESIZE];
	UWORD	afh_NumOfFrames;
	UWORD	afh_AnimSpeed;
	/* Array of FrameFileHeader Structs */
};
struct SequenceFileHeader
{
	UBYTE	sfh_BobBankName[16];
	UWORD	sfh_NumOfAnims;
	/* array of AnimFileHeader Structs */
};


/************ Prototypes for functions local to this file ************/

BOOL ShowSeqWindow(struct SuperWindow *sw);
BOOL SetupSeqWindowMenus(struct SuperWindow *sw);
void HideSeqWindow(struct SuperWindow *sw);
APTR KillOffSeqWindow(struct SuperWindow *sw);
void SeqWindowHandler(struct SuperWindow *sw,	struct IntuiMessage *imsg);

static void HandleGadgetUp(struct SuperWindow *sw,
													 struct Gadget *gad,
													 UWORD code);
static void SortOutSeqWindowMenus(struct SuperWindow *sw,
																	struct IntuiMessage *imsg,
																	UWORD code);
static void DoGadgets(struct SeqWindow *asw);
static void DisplayViewBob(struct SeqWindow *asw, struct MyBob *bob);
static void DisplayFrame(struct SeqWindow *asw, struct Frame *fr);
static struct Anim *AddAfterAnim(struct Sequence *sq,struct Anim *prevanim);
static struct Anim *AddTailAnim(struct Sequence *sq);
static struct Anim *NewAnim(struct Sequence *sq);
static struct Frame *AddAfterFrame(struct Anim *an,struct Frame *prevframe,struct MyBob *bob,
				WORD x, WORD y);
static struct Frame *AddTailFrame(struct Anim *an,struct MyBob *bob,
				WORD x, WORD y);
static struct Frame *AllocFrame(struct MyBob *bob, WORD x, WORD y);
static void GotoNewViewBob(struct SeqWindow *asw, struct MyBob *bob);
static void SortOutSeqKeys(struct SeqWindow *asw,UWORD keycode,UWORD qual);
static void GotoNewFrame(struct SeqWindow *asw, struct Frame *fr);
static void GotoNewAnim(struct SeqWindow *asw, struct Anim *an);
static BOOL AllocNewSequence(struct Project *pj, BOOL createanim);
static void RenumberAnims(struct Sequence *sq);
static void RenumberFrames(struct Anim *an);
static struct Frame *GetFrameByNumber(struct Anim *an, UWORD number);
static struct Anim *GetAnimByNumber(struct Sequence *sq, UWORD number);
static void RefreshGadget(struct Sequence *sq, struct SeqWindow *missasw, UWORD gadgetid);
static void SussMouseButtons(struct SeqWindow *asw, WORD mx, WORD my, UWORD code);
static void HandleMouseMove(struct SeqWindow *asw,WORD mx, WORD my);
static struct Frame *AddSingleFrame(struct SeqWindow *asw, struct Anim *an, struct Frame *fr, struct MyBob *bob);
static void GUIAddRange(struct SeqWindow *asw);
static void AddRange(struct SeqWindow *asw,
										 struct MyBob *startbob, struct MyBob *stopbob);
static void GUIDeleteRange(struct SeqWindow *asw);
static void DeleteRange(struct Sequence *sq,struct Anim *an,struct Frame *firstframe, struct Frame *lastframe);
static struct Anim *DeleteAnim(struct SeqWindow *asw);
static void SetOffsets(struct Anim *an);
static void CalcAnimHitZones(struct Project *pj, struct Anim *an);
static void RenameBobs(struct Project *pj, struct Anim *an);

/************ OpenNewSeqWindow ************/
//
// Opens a Anim Sequencer Window.
//

BOOL OpenNewSeqWindow(struct WindowDefault *wf, struct Project *pj)
{
	struct	SeqWindow	*asw;
	struct	Anim *an;

		/* Make Sure we have a Project */
	if( (!pj) || ((pj->pj_EditType != EDTY_BOBS) && (pj->pj_EditType != EDTY_SPRITES)) )
	{
		GroovyReq("Anim Sequencer","Must Be Editing Bobs/Sprites.","D-Oh!");
		return(FALSE);
	}
	if( (pj->pj_CurrentBob) && (pj->pj_CurrentBobModified) )
	{
		if(pj->pj_EditType == EDTY_BOBS)
			StoreBob(pj, pj->pj_CurrentBob);
		else
			StoreSprite(pj, pj->pj_CurrentBob);
	}

		/* If the project doesn't yet have an anim sequence associated with it
			then allocate a new one. */
	if(!pj->pj_Sequence)
	{
		if( !AllocNewSequence(pj,TRUE) )
		{
			DisplayBeep(mainscreen);
			return(FALSE);
		}
	}

	/* grab mem for the SeqWindow struct */
	if ( !( asw = AllocVec(sizeof(struct SeqWindow), MEMF_CLEAR ) ) )
		return(FALSE);

	/* Tie our window into the sequence */
	asw->asw_Sequence = pj->pj_Sequence;
	asw->asw_NextSeqWindow = asw->asw_Sequence->sq_SeqWindows;
	asw->asw_Sequence->sq_SeqWindows = asw;

	/* figure out a unique ID for this window (mainly for AREXX use) */
	InstallUniqueWindowID((struct SuperWindow *)asw);
	asw->asw_sw.sw_Node.ln_Name = (char *)&(asw->asw_sw.sw_idstring);

	/* find the Default settings for this type of SuperWindow */
	//wf = &glob.cg_WindowDefaults[SWTY_SEQUENCE];

	/* set up handler routines and assorted SuperWindow data */
	asw->asw_sw.sw_Handler = SeqWindowHandler;
	asw->asw_sw.sw_HideWindow = HideSeqWindow;
	asw->asw_sw.sw_ShowWindow = ShowSeqWindow;
	asw->asw_sw.sw_BuggerOff = KillOffSeqWindow;
	asw->asw_sw.sw_RefreshMenus = SetupSeqWindowMenus;
	asw->asw_sw.sw_Window = NULL;
	asw->asw_sw.sw_MenuStrip = NULL;
	asw->asw_sw.sw_GadList = NULL;
	asw->asw_sw.sw_Type = SWTY_SEQUENCE;
	asw->asw_sw.sw_VisualInfo = NULL;

	/* set up stuff for opening the actual intuition window */
	asw->asw_sw.sw_Left = wf->wf_Left;
	asw->asw_sw.sw_Top = wf->wf_Top;
	asw->asw_sw.sw_Width = wf->wf_Width;
	asw->asw_sw.sw_Height = wf->wf_Height;
	asw->asw_sw.sw_MinWidth = 320;					/* minwidth set for topaz8 */
	asw->asw_sw.sw_MaxWidth = 0xFFFF;
	asw->asw_sw.sw_MaxHeight = 0xFFFF;
	asw->asw_sw.sw_MenuStrip = NULL;

	asw->asw_sw.sw_FlgBackDrop = wf->wf_FlgBackDrop;
	asw->asw_sw.sw_FlgBorderless = wf->wf_FlgBorderless;
	asw->asw_sw.sw_FlgTitle = wf->wf_FlgTitle;
	asw->asw_sw.sw_FlgBorderGadgets = wf->wf_FlgBorderGadgets;

	/* setup SuperWindow basename */
	asw->asw_sw.sw_Title = "Animation Sequencer";

	if( (IsListEmpty(&pj->pj_BobList)) || (pj->pj_BobList.lh_Head == pj->pj_BobList.lh_TailPred) )
	{
		GroovyReq("Anim Sequencer","Can't Open Sequencer.\nNo Bobs to Sequence.","Continue");
		pj->pj_Sequence->sq_SeqWindows = asw->asw_NextSeqWindow;
		FreeVec(asw);
		return(FALSE);
	}

	if(pj->pj_CurrentBob)
		asw->asw_ViewBob = pj->pj_CurrentBob;
	else
		asw->asw_ViewBob = (struct MyBob *)pj->pj_BobList.lh_Head;

	asw->asw_ButtonDown = FALSE;
	an = (struct Anim *)asw->asw_Sequence->sq_Anims.lh_Head;
	asw->asw_CurrentAnim = an;
	if( !IsListEmpty(&an->an_Frames) )
		asw->asw_CurrentFrame = (struct Frame *)an->an_Frames.lh_Head;
	else
		asw->asw_CurrentFrame = NULL;

	asw->asw_RelativePositions = TRUE;

	/* add window into the superwindows list */
	AddTail(&superwindows, &asw->asw_sw.sw_Node);

	/* open the intuition window (if the window doesn't open, */
	/* it is just left hidden - SEP). */
	ShowSeqWindow(&asw->asw_sw);

	return(TRUE);
}



/************ ShowSeqWindow ************/
//
// Displays a Anim Sequencer Window, and sets up all its gadgets, lists
// etc...
//

BOOL ShowSeqWindow(struct SuperWindow *sw)
{
	struct SeqWindow *asw;
	UWORD minh;
	ULONG flags;

	asw = (struct SeqWindow *)sw;

	if(!sw->sw_Window)				/* make sure it isn't already open */
	{
		/* Set up the Visual Info for GadTools */
		if (!(asw->asw_sw.sw_VisualInfo = GetVisualInfo(MAINSCREEN,TAG_END)))
			return(FALSE);

		minh = MAINSCREEN->Font->ta_YSize+1 +	MAINSCREEN->WBorTop + ((8+4)*9)+3;

		flags = WFLG_ACTIVATE | WFLG_NEWLOOKMENUS;
		if(sw->sw_FlgTitle)
			flags = flags | WFLG_DRAGBAR | WFLG_DEPTHGADGET | WFLG_CLOSEGADGET;
		if(sw->sw_FlgBorderGadgets)
			flags = flags | WFLG_SIZEGADGET;

		if(sw->sw_Width < sw->sw_MinWidth)
			sw->sw_Width = sw->sw_MinWidth;
		if(sw->sw_Height < minh)
			sw->sw_Width = minh;

		sw->sw_Window = OpenWindowTags(NULL,
											WA_Left,				sw->sw_Left,
											WA_Top,					sw->sw_Top,
											WA_Width,				sw->sw_Width,
											WA_Height,			sw->sw_Height,
											WA_MinWidth,		sw->sw_MinWidth,
											WA_MinHeight,		minh,
											WA_MaxWidth,		sw->sw_MaxWidth,
											WA_MaxHeight,		sw->sw_MaxHeight,
											WA_Flags,				flags,
											WA_ReportMouse,	TRUE,
											WA_Backdrop,		sw->sw_FlgBackDrop,
											WA_Borderless,	sw->sw_FlgBorderless,
											WA_CustomScreen,	MAINSCREEN,
											WA_AutoAdjust,	TRUE,
											WA_Title,				sw->sw_FlgTitle ?
												BuildWindowTitle(sw) : NULL,
											TAG_DONE,				NULL);

		if( !(sw->sw_Window)) return(FALSE);

		DoGadgets(asw);

		/* Tell the window which superwindow it is attached to */
		/* (so we'll know which handler routine to call) */
		sw->sw_Window->UserData = (BYTE *)asw;

		SetupSeqWindowMenus(sw);

		/* we want to use our global message port for IDCMP stuff */
		sw->sw_Window->UserPort = mainmsgport;
		ModifyIDCMP(sw->sw_Window, IDCMP_CLOSEWINDOW | IDCMP_MENUPICK | IDCMP_NEWSIZE |
			IDCMP_REFRESHWINDOW | IDCMP_RAWKEY | BUTTONIDCMP | STRINGIDCMP | INTEGERIDCMP |
			IDCMP_MOUSEBUTTONS | IDCMP_CHANGEWINDOW | IDCMP_MOUSEMOVE );

		SetSuperWindowScreenTitle(sw);
	}
	return(TRUE);
}


BOOL SetupSeqWindowMenus(struct SuperWindow *sw)
{
	BOOL success = FALSE;
	struct Menu *mn;
	UWORD menucount;

	if (sw->sw_MenuStrip)						/* Clear the menu strip if it exists */
	{
		ClearMenuStrip(sw->sw_Window);
		FreeMenus(sw->sw_MenuStrip);
		sw->sw_MenuStrip = NULL;
	}

	/* Set up some menus */
	SetupWindowMenu( sw, &seqmenu[MN_WINDOW_IDX] );
	SetupEditMenu( NULL, &seqmenu[MN_EDIT_IDX] );
	SetupSettingsMenu( &seqmenu[MN_SETTINGS_IDX] );
	SetupModeMenu( sw, &seqmenu[MN_MODE_IDX] );
	LinkInUserMenu( &seqmenu[MN_USER_IDX] );

	if( sw->sw_MenuStrip = CreateMenus( seqmenu, TAG_END ) )
	{
		if ( LayoutMenus( sw->sw_MenuStrip, sw->sw_VisualInfo, TAG_END) )
		{
			if ( SetMenuStrip( sw->sw_Window, sw->sw_MenuStrip) )
			{
				mn = sw->sw_MenuStrip;
				for(menucount = 0; menucount < MN_NUMOFMENUS; menucount++)
				{
					sw->sw_Menus[menucount] = mn;
					if(mn) mn = mn->NextMenu;
				}
				success = TRUE;
			}
		}
	}

	/* free stuff if any errors occured */
	if( !success && sw->sw_MenuStrip )
	{
		FreeMenus(sw->sw_MenuStrip);
		sw->sw_MenuStrip = NULL;
	}

	return( success );
}


/******************  HideSeqWindow()  ******************/
//
// Hides the given EditPaletteWindow. Frees gadgets and stuff.
//

void HideSeqWindow(struct SuperWindow *sw)
{
	struct SeqWindow *asw;

	asw = (struct SeqWindow *)sw;

	if (sw->sw_Window)												/* make sure it's actually open */
	{
		if( sw->sw_BlockingHandle )
		{
			UnblockWindow( sw->sw_BlockingHandle );
			sw->sw_BlockingHandle = NULL;
		}
		if (sw->sw_MenuStrip)										/* Clear the menu strip if it exists */
		{
			ClearMenuStrip(sw->sw_Window);
			FreeMenus(sw->sw_MenuStrip);
			sw->sw_MenuStrip = NULL;
		}
		RememberWindow(sw);											/* save old window data */

		/* free up string used for window title */
		if(sw->sw_Window->Title)
			FreeVec(sw->sw_Window->Title);

		/* kill the window */
		CloseWindowSafely(sw->sw_Window);				/* close it */
		sw->sw_Window = NULL;										/* indicate that it's gone */

		if(asw->asw_sw.sw_GadList)
		{
			FreeGadgets(asw->asw_sw.sw_GadList);
			asw->asw_sw.sw_GadList = NULL;
		}
		if (sw->sw_VisualInfo) FreeVisualInfo(sw->sw_VisualInfo);
		sw->sw_VisualInfo = NULL;
		SetProcessWindowPtr();
	}
}


/******************  KillOffSeqWindow()  ******************/
//
// Shuts down a SeqWindow and removes it from the SuperWindows
// list.
//

APTR KillOffSeqWindow(struct SuperWindow *sw)
{
	struct SeqWindow *asw;

	asw = (struct SeqWindow *)sw;

	/* close/free the superwindow */
	asw->asw_Sequence->sq_SeqWindows = asw->asw_NextSeqWindow;
	asw->asw_NextSeqWindow = NULL;
	HideSeqWindow(sw);													/* shut down the Intuition window */
	Remove(&(sw->sw_Node));											/* remove from superwindows list */
	FreeVec(asw);																/* free RevealWindow struct */
	return(NULL);
}

/******************  SeqWindowHandler  ******************/
//
// Processes IDCMP messages coming into any SeqWindows,
// dispatching appropriate bits of code.
//

void SeqWindowHandler(struct SuperWindow *sw,
														 struct IntuiMessage *imsg)
{
	WORD mx,my;
	ULONG	class;
	UWORD	code,qual;
	struct IntuiMessage *gtimsg;
	struct Gadget *gad;

	if( gtimsg = GT_FilterIMsg(imsg) )
	{

		/* read out all we need */
		class = gtimsg->Class;
		mx = gtimsg->MouseX;
		my = gtimsg->MouseY;
		code = gtimsg->Code;
		qual = gtimsg->Qualifier;
		gad = (struct Gadget *)gtimsg->IAddress;

		GT_PostFilterIMsg(gtimsg);

		switch(class)
		{
			case IDCMP_CLOSEWINDOW:
				/* tell this window to bugger off */
				ReplyMsg((struct Message *)imsg);
				KillOffSeqWindow(sw);
				SussOutQuiting(FALSE);
				break;
			case IDCMP_REFRESHWINDOW:
				/* repair GadTool renderings */
				ReplyMsg((struct Message *)imsg);
				GT_BeginRefresh(sw->sw_Window);
				GT_EndRefresh(sw->sw_Window,TRUE);
				break;
			case IDCMP_RAWKEY:
				ReplyMsg((struct Message *)imsg);
				SortOutSeqKeys((struct SeqWindow *)sw,code,qual);
				break;
			case IDCMP_MOUSEMOVE:
				ReplyMsg((struct Message *)imsg);
				if( ((struct SeqWindow *)sw)->asw_ButtonDown )
					HandleMouseMove((struct SeqWindow *)sw,mx,my);
				break;
			case IDCMP_GADGETUP:
				ReplyMsg((struct Message *)imsg);
				HandleGadgetUp(sw,gad,code);
				break;
			case IDCMP_MENUPICK:
				/* menuroutine does the reply() */
				SortOutSeqWindowMenus( sw,imsg,code );
				break;
			case IDCMP_NEWSIZE:
				/* resuss all the gadget placements/font stuff */
				/* according to the new windowsize. */
				DoGadgets((struct SeqWindow *)sw);
				break;
			case IDCMP_MOUSEBUTTONS:
				ReplyMsg((struct Message *)imsg);
				SussMouseButtons((struct SeqWindow *)sw,mx,my,code);
				break;
			default:
				/* catch any superfluous crap going through... */
				ReplyMsg((struct Message *)imsg);
				break;
		}
	}
	else
		ReplyMsg((struct Message *)imsg);
}

/****************  HandleGadgetUp()  ****************/
//
// Process GADGETUP IDCMP events for the RevealWindow.
//

static void HandleGadgetUp(struct SuperWindow *sw, struct Gadget *gad, UWORD code)
{
	struct SeqWindow *asw,*asw2;
	struct Sequence *sq;
	struct Anim *an,*currentan;
	struct Frame *fr,*currentfr;
	struct MyBob *bob;
	struct Window *win;

	asw = (struct SeqWindow *)sw;
	sq = asw->asw_Sequence;
	win = asw->asw_sw.sw_Window;
	an = asw->asw_CurrentAnim;
	fr = asw->asw_CurrentFrame;

	switch(gad->GadgetID)
	{
		case MYGT_ADDFRAME:
			if( (asw->asw_CurrentAnim) && (asw->asw_ViewBob) )
			{
				fr = AddSingleFrame(asw, asw->asw_CurrentAnim, asw->asw_CurrentFrame, asw->asw_ViewBob);
/*
				if(currentfr = asw->asw_CurrentFrame)
					fr = AddAfterFrame(asw->asw_CurrentAnim,currentfr,asw->asw_ViewBob,0,0);
				else
					fr = AddTailFrame(asw->asw_CurrentAnim,asw->asw_ViewBob,0,0);
*/
				if(fr)
				{
					GotoNewFrame(asw,fr);
					RefreshGadget(sq,asw,MYGT_NEWPOS);
				}
			}
			break;
		case MYGT_ADDRANGE:
			GUIAddRange(asw);
			break;
		case MYGT_NEWANIM:
			if( currentan = asw->asw_CurrentAnim)
				an = AddAfterAnim(asw->asw_Sequence,currentan);
			else
				an = AddTailAnim(asw->asw_Sequence);
			if(an)
			{
				GotoNewAnim(asw,an);
				RefreshGadget(sq,asw,MYGT_NEWPOS);
			}
			ActivateGadget(asw->asw_Gadgets[MYGT_ANIMNAME],asw->asw_sw.sw_Window,NULL);
			break;
		case MYGT_ANIMNAME:
			an = asw->asw_CurrentAnim;
			Mystrncpy(an->an_Name,((struct StringInfo *)gad->SpecialInfo)->Buffer, ANIMNAMESIZE);
			if(MakePackable(an->an_Name))
				DisplayBeep(mainscreen);
			RefreshGadget(asw->asw_Sequence, NULL, MYGT_ANIMNAME);
			break;
		case MYGT_XPOS:
			if(fr = asw->asw_CurrentFrame)
			{
				fr->fr_X = ((struct StringInfo *)gad->SpecialInfo)->LongInt;
				DisplayFrame(asw,fr);
				RefreshGadget(asw->asw_Sequence,asw,MYGT_XPOS);
			}
			break;
		case MYGT_YPOS:
			if(fr = asw->asw_CurrentFrame)
			{
				fr->fr_Y = ((struct StringInfo *)gad->SpecialInfo)->LongInt;
				DisplayFrame(asw,fr);
				RefreshGadget(asw->asw_Sequence,asw,MYGT_YPOS);
			}
			break;
		case MYGT_DELFRAME:
			if(currentfr = asw->asw_CurrentFrame)
			{
				asw->asw_CurrentFrame = DeleteFrame(asw->asw_CurrentAnim,asw->asw_CurrentFrame);

				for(asw2 = sq->sq_SeqWindows; asw2; asw2 = asw2->asw_NextSeqWindow)
				{
					if(asw2->asw_CurrentFrame == currentfr)
						asw2->asw_CurrentFrame = asw->asw_CurrentFrame;
				}
				RefreshGadget(sq,NULL,MYGT_NEWPOS); //DisplayFrame(asw,asw->asw_CurrentFrame);
			}
			break;
		case MYGT_DELRANGE:
			GUIDeleteRange(asw);
			break;
		case MYGT_DELANIM:
			DeleteAnim(asw);
			break;
		case MYGT_BOBNAME:
			if( bob = (struct MyBob *)MyFindName(&sq->sq_Project->pj_BobList, ((struct StringInfo *)gad->SpecialInfo)->Buffer, NULL) )
			{
				asw->asw_ViewBob = bob;
				DisplayViewBob(asw,asw->asw_ViewBob);
			}
			else
			{
				DisplayBeep(mainscreen);
				GT_SetGadgetAttrs(asw->asw_Gadgets[MYGT_BOBNAME],win,NULL,
					GTST_String,		asw->asw_ViewBob->bb_Node.ln_Name,
					TAG_END);
			}
			break;
		case MYGT_BOBNUMBERMINUS:
			if( (asw->asw_ViewBob->bb_Node.ln_Pred) &&
					(asw->asw_ViewBob->bb_Node.ln_Pred->ln_Pred) )
				GotoNewViewBob(asw, (struct MyBob *)asw->asw_ViewBob->bb_Node.ln_Pred);
			break;
		case MYGT_BOBNUMBERPLUS:
			if( (asw->asw_ViewBob->bb_Node.ln_Succ) &&
					(asw->asw_ViewBob->bb_Node.ln_Succ->ln_Succ) &&
					(asw->asw_ViewBob->bb_Node.ln_Succ->ln_Succ->ln_Succ) )
				GotoNewViewBob(asw, (struct MyBob *)asw->asw_ViewBob->bb_Node.ln_Succ);
			break;
		case MYGT_BOBNUMBER2:
			bob = FindBobByNumber(&sq->sq_Project->pj_BobList,((struct StringInfo *)gad->SpecialInfo)->LongInt);
			if(bob)
			{
				asw->asw_ViewBob = bob;
				DisplayViewBob(asw,asw->asw_ViewBob);
			}
			else
			{
				DisplayBeep(mainscreen);
				GT_SetGadgetAttrs(asw->asw_Gadgets[MYGT_BOBNUMBER2],win,NULL,
					GTIN_Number,		asw->asw_ViewBob->bb_Number,
					TAG_END);
			}
			break;
		case MYGT_ANIMNUMBERMINUS:
			if(!IsListEmpty(&sq->sq_Anims))
			{
				if(an->an_Node.ln_Pred->ln_Pred)
					GotoNewAnim(asw, (struct Anim *)an->an_Node.ln_Pred);
				//else
				//	GotoNewAnim(asw, (struct Anim *)sq->sq_Anims.lh_TailPred);
			}
			break;
		case MYGT_ANIMNUMBERPLUS:
			if(!IsListEmpty(&sq->sq_Anims))
			{
				if(an->an_Node.ln_Succ->ln_Succ)
					GotoNewAnim(asw, (struct Anim *)an->an_Node.ln_Succ);
				//else
				//	GotoNewAnim(asw, (struct Anim *)sq->sq_Anims.lh_Head);
			}
			break;
		case MYGT_ANIMNUMBER:
			an = GetAnimByNumber(sq,((struct StringInfo *)gad->SpecialInfo)->LongInt);
			if(an)
				GotoNewAnim(asw,an);
			else
			{
				DisplayBeep(mainscreen);
				GT_SetGadgetAttrs(asw->asw_Gadgets[MYGT_ANIMNUMBER],win,NULL,
					GTIN_Number,		asw->asw_CurrentAnim->an_Number,
					TAG_END);
			}
			break;
		case MYGT_FRAMENUMBERMINUS:
			if(!IsListEmpty(&an->an_Frames))
			{
				if(fr->fr_Node.ln_Pred->ln_Pred)
					GotoNewFrame(asw, (struct Frame *)fr->fr_Node.ln_Pred);
				//else
				//	GotoNewFrame(asw, (struct Frame *)an->an_Frames.lh_TailPred);
			}
			break;
		case MYGT_FRAMENUMBERPLUS:
			if(!IsListEmpty(&an->an_Frames))
			{
				if(fr->fr_Node.ln_Succ->ln_Succ)
					GotoNewFrame(asw, (struct Frame *)fr->fr_Node.ln_Succ);
				//else
				//	GotoNewFrame(asw, (struct Frame *)an->an_Frames.lh_Head);
			}
			break;
		case MYGT_FRAMENUMBER:
			an = asw->asw_CurrentAnim;
			fr = GetFrameByNumber(an,((struct StringInfo *)gad->SpecialInfo)->LongInt);
			if(fr)
				GotoNewFrame(asw,fr);
			else
			{
				DisplayBeep(mainscreen);
				GT_SetGadgetAttrs(asw->asw_Gadgets[MYGT_FRAMENUMBER],win,NULL,
					GTIN_Number,		asw->asw_CurrentFrame ? asw->asw_CurrentFrame->fr_Number : 0,
					TAG_END);
			}
			break;
		case MYGT_BOBNUMBER:
			bob = FindBobByNumber(&sq->sq_Project->pj_BobList,((struct StringInfo *)gad->SpecialInfo)->LongInt);
			if(bob)
			{
				if(IsListEmpty(&asw->asw_CurrentAnim->an_Frames))
				{
					if(fr = AddTailFrame(asw->asw_CurrentAnim,bob,0,0))
						GotoNewFrame(asw,fr);
				}
				else
					asw->asw_CurrentFrame->fr_Bob = bob;
				RefreshGadget(sq,NULL,MYGT_NEWPOS);
			}
			else
			{
				DisplayBeep(mainscreen);
				GT_SetGadgetAttrs(asw->asw_Gadgets[MYGT_BOBNUMBER],win,NULL,
					GTIN_Number,		asw->asw_CurrentFrame ? asw->asw_CurrentFrame->fr_Bob->bb_Number : 0,
					TAG_END);
			}
			break;
		case MYGT_ANIMSPEED:
			an = asw->asw_CurrentAnim;
			an->an_Speed = ((struct StringInfo *)gad->SpecialInfo)->LongInt;
			if(an->an_Speed < 1)
			{
				DisplayBeep(mainscreen);
				an->an_Speed = 1;
			}
			RefreshGadget(asw->asw_Sequence, NULL, MYGT_ANIMSPEED);
			break;
		case MYGT_RELATIVEPOSITIONS:
			asw->asw_RelativePositions = (code) ? TRUE : FALSE;
			break;
		case MYGT_OFFSET:
			SetOffsets(asw->asw_CurrentAnim);
			RefreshGadget(sq, NULL, MYGT_NEWPOS);
			break;
		case MYGT_PLAY:
			if(BlockAllSuperWindows())
			{
				PlayAnimRequester(asw->asw_Sequence, asw->asw_CurrentAnim);
				UnblockAllSuperWindows();
			}
			break;
		case MYGT_HITZONES:
			CalcAnimHitZones(sq->sq_Project, asw->asw_CurrentAnim);
			if( (sq) && (sq->sq_Project) )
				RefreshProjectName(sq->sq_Project);
			break;
		case MYGT_NAMEBOBS:
			RenameBobs(sq->sq_Project, asw->asw_CurrentAnim);
			RefreshGadget(sq, NULL, MYGT_BOBNAME);
			UpdateBobNameGadgets(sq->sq_Project, sq->sq_Project->pj_CurrentBob);
			break;
	}
}

/************* SortOutSeqWindowMenus() **************/
//
// Handle MENUPICK IDCMP messages - note the reply() is given
// here rather than in the main loop.
// Why is this so? Well... who can tell?
//

static void SortOutSeqWindowMenus(struct SuperWindow *sw,
																	struct IntuiMessage *imsg,
																	UWORD code)
{
	int i = 0;
	UWORD menunumber[10];
	UWORD menunum,itemnum,subnum;
	UWORD status = 0;

	WaitForSubTaskDone(mainmsgport);

	/* NastyBits alert: */
	/* Read out up to 10 NextSelect thingys. Hey, it works! */
	menunumber[i] = code;
	while ( (i < 10) && ( menunumber[i] != MENUNULL) )
	{
		i++;
		menunumber[i] =
			( ItemAddress( sw->sw_MenuStrip, menunumber[i-1] ) )->NextSelect;
	}
	/* reply pronto */
	ReplyMsg((struct Message *)imsg);


	/* Go through out little array of menunumbers and process them */
	/* 'status' is used so certain events (eg Quit) can terminate processing. */

	for(i=0;
		( i < 10 ) &&
			( menunumber[i] != MENUNULL ) &&
			!(status & 1);
		i++ )
	{
		menunum = MENUNUM(menunumber[i]);
		itemnum = ITEMNUM(menunumber[i]);			// These two lines should be
		subnum = SUBNUM(menunumber[i]);				// killed some time.

		switch(menunum)
		{
			case MN_PROJECT:
				status |= DoProjectMenu(sw,menunumber[i]);
				break;
			case MN_WINDOW:
				status |= DoWindowMenu(sw,menunumber[i]);
				break;
			case MN_SETTINGS:
				DoSettingsMenu(sw,menunumber[i]);
				break;
			case MN_MODE:
				DoModeMenu(sw,menunumber[i]);
				break;
			case MN_BRUSH:
				DoBrushMenu(sw,menunumber[i]);
				break;
			case MN_USER:
				DoUserMenu(sw,menunumber[i]);
				break;
		}
	}
	if (status & 2)
	{
		(*sw->sw_HideWindow)(sw);
		(*sw->sw_ShowWindow)(sw);
	}
}

static void SortOutSeqKeys(struct SeqWindow *asw,UWORD keycode,UWORD qual)
{
	struct Sequence *sq;
	struct Anim *an;
	struct Frame *fr;

	sq = asw->asw_Sequence;
	an = asw->asw_CurrentAnim;
	fr = asw->asw_CurrentFrame;

	switch (keycode)
	{
		case 0x4C:			/* UP (Anim Plus) */
			if(!IsListEmpty(&sq->sq_Anims))
			{
				if(an->an_Node.ln_Succ->ln_Succ)
					GotoNewAnim(asw, (struct Anim *)an->an_Node.ln_Succ);
				else
					GotoNewAnim(asw, (struct Anim *)sq->sq_Anims.lh_Head);
			}
			break;

		case 0x4D:			/* DOWN (Anim Minus) */
			if(!IsListEmpty(&sq->sq_Anims))
			{
				if(an->an_Node.ln_Pred->ln_Pred)
					GotoNewAnim(asw, (struct Anim *)an->an_Node.ln_Pred);
				else
					GotoNewAnim(asw, (struct Anim *)sq->sq_Anims.lh_TailPred);
			}
			break;

		case 0x4F:			/* LEFT (frame minus) */
			if(!IsListEmpty(&an->an_Frames))
			{
				if(fr->fr_Node.ln_Pred->ln_Pred)
					GotoNewFrame(asw, (struct Frame *)fr->fr_Node.ln_Pred);
				else
					GotoNewFrame(asw, (struct Frame *)an->an_Frames.lh_TailPred);
			}
			break;

		case 0x4E:			/* RIGHT (frame plus) */
			if(!IsListEmpty(&an->an_Frames))
			{
				if(fr->fr_Node.ln_Succ->ln_Succ)
					GotoNewFrame(asw, (struct Frame *)fr->fr_Node.ln_Succ);
				else
					GotoNewFrame(asw, (struct Frame *)an->an_Frames.lh_Head);
			}
			break;

		case 0x0B:		/* - Key */
			break;

		case 0x0C:		/* + Key */
			break;

		case 0x01:			/* 1 */
			if( (asw->asw_ViewBob->bb_Node.ln_Pred) &&
					(asw->asw_ViewBob->bb_Node.ln_Pred->ln_Pred) )
				GotoNewViewBob(asw, (struct MyBob *)asw->asw_ViewBob->bb_Node.ln_Pred);
			break;

		case 0x02:			/* 2 */
			if( (asw->asw_ViewBob->bb_Node.ln_Succ) &&
					(asw->asw_ViewBob->bb_Node.ln_Succ->ln_Succ) &&
					(asw->asw_ViewBob->bb_Node.ln_Succ->ln_Succ->ln_Succ) )
				GotoNewViewBob(asw, (struct MyBob *)asw->asw_ViewBob->bb_Node.ln_Succ);
			break;
	}
}


/************** DoGadgets() **************/
//
// Sets up and formats all the gadgets within the SeqWindow.
// Adapts to font and window sizes.
// Call at window opening, and again at NEWSIZE events.

static void DoGadgets(struct SeqWindow *asw)
{
	struct Gadget *gad;
	struct NewGadget ng;
	struct Window *win;
	UWORD buttonh,buttonw;
	UWORD	wintop,winbottom,winleft,winright,wingadgetbottom;
	UWORD minwidth,minheight;
	UWORD numericgadgetw,midx,animnameright,bobnameright;
	UWORD pmwidth;	/* Plus/Minus Width */

	win = asw->asw_sw.sw_Window;

	/* first, kill any old gadgets that may be active */
	if(asw->asw_sw.sw_GadList)
	{
		RemoveGList(win, asw->asw_sw.sw_GadList, -1);
		FreeGadgets(asw->asw_sw.sw_GadList);
		asw->asw_sw.sw_GadList = NULL;
	}

	ClearWindow( win );

	/* use window font */
	SetFont(win->RPort,windowtextfont);
	asw->asw_sw.sw_TextAttr = &windowtextattr;
	asw->asw_sw.sw_TextFont = windowtextfont;

	buttonh = win->RPort->Font->tf_YSize + 4;
	buttonw = TextLength(win->RPort,"WWW WWWWW",9)+4;
	minheight = buttonh*7+4;
	minwidth  = buttonw*4;

	/* fall back to Topaz 8 if the window is too small */
	AdjustWindowFont(&asw->asw_sw,minwidth,minheight);

	/* init gadget stuff */
	gad = CreateContext(&asw->asw_sw.sw_GadList);

	wintop = win->BorderTop;
	winbottom = win->Height - win->BorderBottom-1;
	winleft = win->BorderLeft;
	winright = win->Width - win->BorderRight-1;
	buttonh = win->RPort->Font->tf_YSize + 4;
	//buttonw = TextLength(win->RPort,"WWW WWWWW",9)+4;
	buttonw = ((winright-winleft)/4)-2;
	pmwidth = TextLength(win->RPort,"+",1)+5;
	wingadgetbottom = winbottom - (buttonh*7);
	numericgadgetw = buttonw/2;

	/* Draw all the beveled Boxes */

	midx = (winright-winleft)/2;
	asw->asw_BobBoxLeft = winleft+2;
	asw->asw_BobBoxTop = wintop+2;
	asw->asw_BobBoxWidth = (midx-3) - asw->asw_BobBoxLeft + 1;
	asw->asw_BobBoxHeight = (wingadgetbottom-2) - asw->asw_BobBoxTop + 1;
	asw->asw_AnimBoxLeft = midx+1;
	asw->asw_AnimBoxTop = asw->asw_BobBoxTop;
	asw->asw_AnimBoxWidth = (winright-3) - asw->asw_AnimBoxLeft + 1;
	asw->asw_AnimBoxHeight = asw->asw_BobBoxHeight;

	SetAPen(win->RPort,globpens[SHINEPEN]);						/* Draw Horizontal lines */
	Move(win->RPort,asw->asw_BobBoxLeft-2,asw->asw_BobBoxTop-2);
	Draw(win->RPort,asw->asw_AnimBoxLeft+asw->asw_AnimBoxWidth,asw->asw_AnimBoxTop-2);
	SetAPen(win->RPort,globpens[SHADOWPEN]);
	Move(win->RPort,asw->asw_BobBoxLeft-2,asw->asw_BobBoxTop-1);
	Draw(win->RPort,asw->asw_AnimBoxLeft+asw->asw_AnimBoxWidth+1,asw->asw_AnimBoxTop-1);

	SetAPen(win->RPort,globpens[SHINEPEN]);						/* Draw Horizontal lines */
	Move(win->RPort,asw->asw_BobBoxLeft-2,asw->asw_BobBoxTop+asw->asw_BobBoxHeight);
	Draw(win->RPort,asw->asw_AnimBoxLeft+asw->asw_AnimBoxWidth+1,asw->asw_AnimBoxTop+asw->asw_AnimBoxHeight);
	SetAPen(win->RPort,globpens[SHADOWPEN]);
	Move(win->RPort,asw->asw_BobBoxLeft-2,asw->asw_BobBoxTop+asw->asw_BobBoxHeight+1);
	Draw(win->RPort,asw->asw_AnimBoxLeft+asw->asw_AnimBoxWidth+1,asw->asw_AnimBoxTop+asw->asw_AnimBoxHeight+1);

	SetAPen(win->RPort,globpens[SHINEPEN]);						/* Draw Sectioning lines */
	Move(win->RPort,asw->asw_BobBoxLeft-2,asw->asw_BobBoxTop-1);
	Draw(win->RPort,asw->asw_BobBoxLeft-2,asw->asw_BobBoxTop+asw->asw_BobBoxHeight);
	SetAPen(win->RPort,globpens[SHADOWPEN]);
	Move(win->RPort,asw->asw_BobBoxLeft-1,asw->asw_BobBoxTop-1);
	Draw(win->RPort,asw->asw_BobBoxLeft-1,asw->asw_BobBoxTop+asw->asw_BobBoxHeight);

	SetAPen(win->RPort,globpens[SHINEPEN]);						/* Draw Sectioning lines */
	Move(win->RPort,midx-1,asw->asw_BobBoxTop-1);
	Draw(win->RPort,midx-1,asw->asw_BobBoxTop+asw->asw_BobBoxHeight);
	SetAPen(win->RPort,globpens[SHADOWPEN]);
	Move(win->RPort,midx,asw->asw_BobBoxTop-1);
	Draw(win->RPort,midx,asw->asw_BobBoxTop+asw->asw_BobBoxHeight-1);

	SetAPen(win->RPort,globpens[SHINEPEN]);
	Move(win->RPort,asw->asw_AnimBoxLeft+asw->asw_AnimBoxWidth,asw->asw_BobBoxTop-1);
	Draw(win->RPort,asw->asw_AnimBoxLeft+asw->asw_AnimBoxWidth,asw->asw_BobBoxTop+asw->asw_BobBoxHeight);
	SetAPen(win->RPort,globpens[SHADOWPEN]);
	Move(win->RPort,asw->asw_AnimBoxLeft+asw->asw_AnimBoxWidth+1,asw->asw_BobBoxTop-2);
	Draw(win->RPort,asw->asw_AnimBoxLeft+asw->asw_AnimBoxWidth+1,asw->asw_BobBoxTop+asw->asw_BobBoxHeight);

	asw->asw_BobLeft = asw->asw_BobBoxLeft;										/* Store Offsets for later */
	asw->asw_BobTop = asw->asw_BobBoxTop;											/* reference */
	asw->asw_BobWidth = 2;
	asw->asw_BobHeight = 2;
	asw->asw_FrameLeft = asw->asw_AnimBoxLeft;
	asw->asw_FrameTop = asw->asw_AnimBoxTop;
	asw->asw_FrameWidth = 2;
	asw->asw_FrameHeight = 2;

	bobnameright = midx-numericgadgetw-(pmwidth*2)-1;
	ng.ng_TextAttr = asw->asw_sw.sw_TextAttr;
	ng.ng_VisualInfo = asw->asw_sw.sw_VisualInfo;

	ng.ng_TopEdge = wingadgetbottom+buttonh;
	ng.ng_LeftEdge = bobnameright+1;													/* BobNumberMinus */
	ng.ng_Width = pmwidth;
	ng.ng_Height = buttonh;
	ng.ng_GadgetText = "-";
	ng.ng_GadgetID = MYGT_BOBNUMBERMINUS;
	ng.ng_Flags = 0;
	gad = CreateGadget( BUTTON_KIND, gad, &ng,
					TAG_END );
	asw->asw_Gadgets[MYGT_BOBNUMBERMINUS] = gad;

	ng.ng_LeftEdge = bobnameright+pmwidth+1;									/* BobNumber 2 */
	ng.ng_Width = numericgadgetw;
	ng.ng_GadgetText = "Num";
	ng.ng_GadgetID = MYGT_BOBNUMBER2;
	ng.ng_Flags = PLACETEXT_ABOVE;
	gad = CreateGadget( INTEGER_KIND, gad, &ng,
					GTIN_Number,		asw->asw_ViewBob->bb_Number,
					GTIN_MaxChars,	4,
					TAG_END );
	asw->asw_Gadgets[MYGT_BOBNUMBER2] = gad;
	
	ng.ng_LeftEdge = bobnameright+pmwidth+numericgadgetw+1;		/* BobNumberPlus */
	ng.ng_Width = pmwidth;
	ng.ng_GadgetText = "+";
	ng.ng_GadgetID = MYGT_BOBNUMBERPLUS;
	ng.ng_Flags = 0;
	gad = CreateGadget( BUTTON_KIND, gad, &ng,
					TAG_END );
	asw->asw_Gadgets[MYGT_BOBNUMBERPLUS] = gad;

	ng.ng_LeftEdge = winleft;																	/* Bob Name */
	ng.ng_Width = bobnameright-winleft+1;
	ng.ng_GadgetText = "Bob";
	ng.ng_GadgetID = MYGT_BOBNAME;
	ng.ng_Flags = PLACETEXT_ABOVE;
	gad = CreateGadget( STRING_KIND, gad, &ng,
					GTST_String,		asw->asw_ViewBob->bb_Node.ln_Name,
					GTST_MaxChars,	BOBNAMESIZE-1,
					TAG_END );
	asw->asw_Gadgets[MYGT_BOBNAME] = gad;

	animnameright = winright-numericgadgetw-(pmwidth*2)-1;
	ng.ng_LeftEdge = animnameright+1;													/* AnimNumberMinus */
	ng.ng_Width = pmwidth;
	ng.ng_Height = buttonh;
	ng.ng_GadgetText = "-";
	ng.ng_GadgetID = MYGT_ANIMNUMBERMINUS;
	ng.ng_Flags = 0;
	gad = CreateGadget( BUTTON_KIND, gad, &ng,
					TAG_END );
	asw->asw_Gadgets[MYGT_ANIMNUMBERMINUS] = gad;

	ng.ng_LeftEdge = animnameright+pmwidth+1;									/* Anim Number */
	ng.ng_Width = numericgadgetw;
	ng.ng_GadgetText = "Num";
	ng.ng_GadgetID = MYGT_ANIMNUMBER;
	ng.ng_Flags = PLACETEXT_ABOVE;
	gad = CreateGadget( INTEGER_KIND, gad, &ng,
					GTIN_Number,		asw->asw_CurrentAnim->an_Number,
					GTIN_MaxChars,	4,
					TAG_END );
	asw->asw_Gadgets[MYGT_ANIMNUMBER] = gad;

	ng.ng_LeftEdge = animnameright+pmwidth+numericgadgetw+1;	/* AnimNumberPlus */
	ng.ng_Width = pmwidth;
	ng.ng_GadgetText = "+";
	ng.ng_GadgetID = MYGT_ANIMNUMBERPLUS;
	ng.ng_Flags = 0;
	gad = CreateGadget( BUTTON_KIND, gad, &ng,
					TAG_END );
	asw->asw_Gadgets[MYGT_ANIMNUMBERPLUS] = gad;

	ng.ng_LeftEdge = midx;																		/* Anim Name */
	ng.ng_Width = animnameright-midx+1;
	ng.ng_GadgetText = "Anim";
	ng.ng_GadgetID = MYGT_ANIMNAME;
	ng.ng_Flags = PLACETEXT_ABOVE;
	gad = CreateGadget( STRING_KIND, gad, &ng,
					GTST_String,		asw->asw_CurrentAnim->an_Node.ln_Name,
					GTST_MaxChars,	ANIMNAMESIZE-1,
					TAG_END );
	asw->asw_Gadgets[MYGT_ANIMNAME] = gad;

	ng.ng_LeftEdge = winleft;																	/* ADD FRAME */
	ng.ng_TopEdge = wingadgetbottom+(buttonh*2);
	ng.ng_Width = buttonw;
	ng.ng_Height = buttonh;
	ng.ng_GadgetText = "Add Frame";
	ng.ng_GadgetID = MYGT_ADDFRAME;
	ng.ng_Flags = 0;
	gad = CreateGadget( BUTTON_KIND, gad, &ng, TAG_END );
	asw->asw_Gadgets[MYGT_ADDFRAME] = gad;

	ng.ng_LeftEdge += buttonw;																/* ADD RANGE */
	ng.ng_GadgetText = "Add Range";
	ng.ng_GadgetID = MYGT_ADDRANGE;
	ng.ng_Flags = 0;
	gad = CreateGadget( BUTTON_KIND, gad, &ng, TAG_END );
	asw->asw_Gadgets[MYGT_ADDRANGE] = gad;

	ng.ng_TopEdge += buttonh;																	/* DEL FRAME */
	ng.ng_LeftEdge = winleft;
	ng.ng_GadgetText = "Del Frame";
	ng.ng_GadgetID = MYGT_DELFRAME;
	ng.ng_Flags = 0;
	gad = CreateGadget( BUTTON_KIND, gad, &ng, TAG_END );
	asw->asw_Gadgets[MYGT_DELFRAME] = gad;

	ng.ng_LeftEdge += buttonw;																/* DEL RANGE */
	ng.ng_GadgetText = "Del Range";
	ng.ng_GadgetID = MYGT_DELRANGE;
	ng.ng_Flags = 0;
	gad = CreateGadget( BUTTON_KIND, gad, &ng, TAG_END );
	asw->asw_Gadgets[MYGT_DELRANGE] = gad;

	ng.ng_TopEdge += buttonh;																	/* DEL ANIM */
	ng.ng_LeftEdge = winleft;
	ng.ng_GadgetText = "Del Anim";
	ng.ng_GadgetID = MYGT_DELANIM;
	ng.ng_Flags = 0;
	gad = CreateGadget( BUTTON_KIND, gad, &ng, TAG_END );
	asw->asw_Gadgets[MYGT_DELANIM] = gad;

	ng.ng_LeftEdge += buttonw;																/* NEW ANIM */
	ng.ng_GadgetText = "New Anim";
	ng.ng_GadgetID = MYGT_NEWANIM;
	ng.ng_Flags = 0;
	gad = CreateGadget( BUTTON_KIND, gad, &ng, TAG_END );
	asw->asw_Gadgets[MYGT_NEWANIM] = gad;

	ng.ng_TopEdge += buttonh;																	/* OFFSET */
	ng.ng_LeftEdge = winleft;
	ng.ng_GadgetText = "Offset";
	ng.ng_GadgetID = MYGT_OFFSET;
	ng.ng_Flags = 0;
	gad = CreateGadget( BUTTON_KIND, gad, &ng, TAG_END );
	asw->asw_Gadgets[MYGT_OFFSET] = gad;

	ng.ng_LeftEdge += buttonw;																/* PLAY */
	ng.ng_GadgetText = "Play";
	ng.ng_GadgetID = MYGT_PLAY;
	ng.ng_Flags = 0;
	gad = CreateGadget( BUTTON_KIND, gad, &ng, TAG_END );
	asw->asw_Gadgets[MYGT_PLAY] = gad;

	ng.ng_TopEdge += buttonh;																	/* HITZONES */
	ng.ng_LeftEdge = winleft;
	ng.ng_GadgetText = "Hit Zones";
	ng.ng_GadgetID = MYGT_HITZONES;
	ng.ng_Flags = 0;
	gad = CreateGadget( BUTTON_KIND, gad, &ng, TAG_END );
	asw->asw_Gadgets[MYGT_HITZONES] = gad;

	ng.ng_LeftEdge += buttonw;																/* NAMEBOBS */
	ng.ng_GadgetText = "Name Bobs";
	ng.ng_GadgetID = MYGT_NAMEBOBS;
	ng.ng_Flags = 0;
	gad = CreateGadget( BUTTON_KIND, gad, &ng, TAG_END );
	asw->asw_Gadgets[MYGT_NAMEBOBS] = gad;


	/**** Numeric Gadgets ****/

	ng.ng_TopEdge = wingadgetbottom+(buttonh*3);							/* Frame Number Minus*/
	ng.ng_LeftEdge = midx;
	ng.ng_Width = pmwidth;
	ng.ng_GadgetText = "-";
	ng.ng_GadgetID = MYGT_FRAMENUMBERMINUS;
	ng.ng_Flags = 0;
	gad = CreateGadget( BUTTON_KIND, gad, &ng,
					TAG_END );
	asw->asw_Gadgets[MYGT_FRAMENUMBERMINUS] = gad;

	ng.ng_LeftEdge += pmwidth;
	ng.ng_Width = numericgadgetw;
	ng.ng_GadgetText = "Frame";																/* Frame Number */
	ng.ng_GadgetID = MYGT_FRAMENUMBER;
	ng.ng_Flags = PLACETEXT_ABOVE;
	gad = CreateGadget( INTEGER_KIND, gad, &ng,
					GTIN_Number,		(asw->asw_CurrentFrame) ? asw->asw_CurrentFrame->fr_Number : 1,
					GTIN_MaxChars,	4,
					TAG_END );
	asw->asw_Gadgets[MYGT_FRAMENUMBER] = gad;

	ng.ng_LeftEdge += numericgadgetw;													/* Frame Number Plus */
	ng.ng_Width = pmwidth;
	ng.ng_GadgetText = "+";
	ng.ng_GadgetID = MYGT_FRAMENUMBERPLUS;
	ng.ng_Flags = 0;
	gad = CreateGadget( BUTTON_KIND, gad, &ng,
					TAG_END );
	asw->asw_Gadgets[MYGT_FRAMENUMBERPLUS] = gad;

	ng.ng_LeftEdge = (((winright-numericgadgetw)-(ng.ng_LeftEdge+pmwidth))/2)-(numericgadgetw/2)+(ng.ng_LeftEdge+pmwidth);
	ng.ng_Width = numericgadgetw;															/* Bob Number */
	ng.ng_GadgetText = "Bob";
	ng.ng_GadgetID = MYGT_BOBNUMBER;
	ng.ng_Flags = PLACETEXT_ABOVE;
	gad = CreateGadget( INTEGER_KIND, gad, &ng,
					GTIN_Number,		(asw->asw_CurrentFrame) ? asw->asw_CurrentFrame->fr_Bob->bb_Number : 1,
					GTIN_MaxChars,	4,
					TAG_END );
	asw->asw_Gadgets[MYGT_BOBNUMBER] = gad;

	ng.ng_LeftEdge = winright-numericgadgetw;									/* Speed */
	ng.ng_GadgetText = "Spd";
	ng.ng_GadgetID = MYGT_ANIMSPEED;
	ng.ng_Flags = PLACETEXT_ABOVE;
	gad = CreateGadget( INTEGER_KIND, gad, &ng,
					GTIN_Number,		asw->asw_CurrentAnim->an_Speed,
					GTIN_MaxChars,	6,
					TAG_END );
	asw->asw_Gadgets[MYGT_ANIMSPEED] = gad;

	ng.ng_TopEdge += (buttonh*2);															/* X Pos */
	ng.ng_LeftEdge = midx;
	ng.ng_GadgetText = "X";
	ng.ng_GadgetID = MYGT_XPOS;
	ng.ng_Flags = PLACETEXT_ABOVE;
	gad = CreateGadget( INTEGER_KIND, gad, &ng,
					GTIN_Number,		(asw->asw_CurrentFrame) ? asw->asw_CurrentFrame->fr_X : 0,
					GTIN_MaxChars,	6,
					TAG_END );
	asw->asw_Gadgets[MYGT_XPOS] = gad;

	ng.ng_LeftEdge = (((winright-26)-(midx+numericgadgetw))/2)+(numericgadgetw/2)+midx;
	ng.ng_GadgetText = "Y";																		/* Y Pos */
	ng.ng_GadgetID = MYGT_YPOS;
	ng.ng_Flags = PLACETEXT_ABOVE;
	gad = CreateGadget( INTEGER_KIND, gad, &ng,
					GTIN_Number,		(asw->asw_CurrentFrame) ? asw->asw_CurrentFrame->fr_Y : 0,
					GTIN_MaxChars,	6,
					TAG_END );
	asw->asw_Gadgets[MYGT_YPOS] = gad;

	//ng.ng_TopEdge += (buttonh*2);													/* Relative Positions */
	ng.ng_LeftEdge = winright-26;
	//ng.ng_Width += ng.ng_Width;
	ng.ng_GadgetText = "Rel";
	ng.ng_GadgetID = MYGT_RELATIVEPOSITIONS;
	ng.ng_Flags = PLACETEXT_ABOVE;
	gad = CreateGadget( CHECKBOX_KIND, gad, &ng,
					GTCB_Checked,		asw->asw_RelativePositions,
					TAG_END );
	asw->asw_Gadgets[MYGT_RELATIVEPOSITIONS] = gad;

	/* check for error */
	if (!gad)
	{
		FreeGadgets(asw->asw_sw.sw_GadList);
		return;
	}

	/* attach our groovy new gadgets to the window */
	AddGList(win, asw->asw_sw.sw_GadList, -1, -1, NULL);
	GT_RefreshWindow(win, NULL);

		/* need to redraw the frame coz we're not using GimmeZeroZero */
	RefreshWindowFrame(win);

	DisplayViewBob(asw,asw->asw_ViewBob);
	if(asw->asw_CurrentFrame)
		DisplayFrame(asw,asw->asw_CurrentFrame);
}

static BOOL AllocNewSequence(struct Project *pj, BOOL createanim)
{
	struct Sequence *sq;
	struct Anim *an;

	if(pj->pj_Sequence)
		FreeSequence(pj);

	if( !(sq = AllocVec(sizeof(struct Sequence),MEMF_ANY|MEMF_CLEAR)) )
		return(FALSE);
	else
	{
		pj->pj_Sequence = sq;
		sq->sq_Project = pj;
		sq->sq_SeqWindows = NULL;
		NewList(&sq->sq_Anims);
		sq->sq_ModifiedFlag = FALSE;
		if(createanim)
		{
			if( an = NewAnim(sq) )
			{
				an->an_Number = 1;
				AddHead(&sq->sq_Anims,&an->an_Node);
			}
			else
			{
				FreeVec(sq);
				pj->pj_Sequence = NULL;
				return(FALSE);
			}
		}
	}
}

void FreeSequence(struct Project *pj)
{
	struct Sequence *sq;
	struct SeqWindow *asw, *nextasw;
	struct Anim *an,*nextan;
	struct Frame *fr,*nextfr;

	if( (!pj) || !(sq = pj->pj_Sequence) )
		return;

	asw = sq->sq_SeqWindows;
	while(asw)
	{
		nextasw = asw->asw_NextSeqWindow;
		(*asw->asw_sw.sw_BuggerOff)(&asw->asw_sw);
		asw = nextasw;
	}

	an = (struct Anim *)sq->sq_Anims.lh_Head;
	while(an->an_Node.ln_Succ)
	{
		nextan = (struct Anim *)an->an_Node.ln_Succ;
		Remove(&an->an_Node);

		fr = (struct Frame *)an->an_Frames.lh_Head;
		while(fr->fr_Node.ln_Succ)
		{
			nextfr = (struct Frame *)fr->fr_Node.ln_Succ;
			Remove(&fr->fr_Node);
			FreeVec(fr);
			fr = nextfr;
		}

		FreeVec(an);
		an = nextan;
	}
	FreeVec(sq);
	pj->pj_Sequence = NULL;
}

static void DisplayViewBob(struct SeqWindow *asw, struct MyBob *bob)
{
	struct Window *win;
	UWORD width,height;

	if( !(win = asw->asw_sw.sw_Window) )
		return;

	SetAPen(win->RPort,0);
	RectFill(win->RPort,asw->asw_BobLeft,asw->asw_BobTop,
							asw->asw_BobLeft+asw->asw_BobWidth-1,asw->asw_BobTop+asw->asw_BobHeight-1);

	if( (!bob) || (!bob->bb_Image) )
		return;

	width = bob->bb_Width;
	height = bob->bb_Height;

	if(width > asw->asw_BobBoxWidth) width = asw->asw_BobBoxWidth;
	if(height > asw->asw_BobBoxHeight) height = asw->asw_BobBoxHeight;

	BltBitMapRastPort(bob->bb_Image,0,0,win->RPort,asw->asw_BobBoxLeft,asw->asw_BobBoxTop,
				width,height,0xE0);

	asw->asw_BobLeft = asw->asw_BobBoxLeft;
	asw->asw_BobTop = asw->asw_BobBoxTop;
	asw->asw_BobWidth = width;
	asw->asw_BobHeight = height;

	GT_SetGadgetAttrs(asw->asw_Gadgets[MYGT_BOBNAME],win,NULL,
		GTST_String,		bob->bb_Node.ln_Name,
		TAG_END);

	GT_SetGadgetAttrs(asw->asw_Gadgets[MYGT_BOBNUMBER2],win,NULL,
		GTIN_Number,		bob->bb_Number,
		TAG_END);
}

static void DisplayFrame(struct SeqWindow *asw, struct Frame *fr)
{
	struct Window *win;
	struct MyBob *bob;
	WORD left,right,top,bottom,width,height;
	WORD animboxright,animboxbottom;
	WORD imleft=0,imtop=0;

	if( !(win = asw->asw_sw.sw_Window) )
		return;

	SetAPen(win->RPort,0);
	RectFill(win->RPort,asw->asw_FrameLeft,asw->asw_FrameTop,
						asw->asw_FrameLeft+asw->asw_FrameWidth-1,asw->asw_FrameTop+asw->asw_FrameHeight-1);

	if( (!fr) || !(bob = fr->fr_Bob) || (!bob->bb_Image) )
		return;

	animboxright = asw->asw_AnimBoxWidth-1;
	animboxbottom = asw->asw_AnimBoxHeight-1;
	left = fr->fr_X;
	top = fr->fr_Y;
	right = left+bob->bb_Width-1;
	bottom = top+bob->bb_Height-1;
	if( (left > animboxright) || (top > animboxbottom) ||
			(right < 0) || (bottom < 0) )
		return;

	if(left < 0)
	{
		imleft = -left;
		left = 0;
	}
	if(right > animboxright) right = animboxright;
	if(top < 0)
	{
		imtop = -top;
		top = 0;
	}
	if(bottom > animboxbottom) bottom = animboxbottom;

	width = right-left+1;
	height = bottom-top+1;
	BltBitMapRastPort(bob->bb_Image,imleft,imtop,win->RPort,left+asw->asw_AnimBoxLeft,top+asw->asw_AnimBoxTop,
				width,height,0xE0);
	asw->asw_FrameLeft = left+asw->asw_AnimBoxLeft;
	asw->asw_FrameTop = top+asw->asw_AnimBoxTop;
	asw->asw_FrameWidth = width;
	asw->asw_FrameHeight = height;

	GT_SetGadgetAttrs(asw->asw_Gadgets[MYGT_FRAMENUMBER],win,NULL,
		GTIN_Number,		asw->asw_CurrentFrame->fr_Number,
		TAG_END);
	GT_SetGadgetAttrs(asw->asw_Gadgets[MYGT_XPOS],win,NULL,
		GTIN_Number,		asw->asw_CurrentFrame->fr_X,
		TAG_END);
	GT_SetGadgetAttrs(asw->asw_Gadgets[MYGT_YPOS],win,NULL,
		GTIN_Number,		asw->asw_CurrentFrame->fr_Y,
		TAG_END);
	GT_SetGadgetAttrs(asw->asw_Gadgets[MYGT_BOBNUMBER],win,NULL,
		GTIN_Number,		asw->asw_CurrentFrame->fr_Bob->bb_Number,
		TAG_END);
}


static struct Anim *AddAfterAnim(struct Sequence *sq,struct Anim *prevanim)
{
	struct Anim *an;

	if(an = NewAnim(sq))
	{
		Insert(&sq->sq_Anims,&an->an_Node,&prevanim->an_Node);
		an->an_Number = (((struct Anim *)an->an_Node.ln_Pred)->an_Number) + 1;
		if(an->an_Node.ln_Succ->ln_Succ)
			RenumberAnims(sq);
	}
	return(an);
}
static struct Anim *AddTailAnim(struct Sequence *sq)
{
	struct Anim *an;

	if(an = NewAnim(sq))
	{
		if( IsListEmpty(&sq->sq_Anims) )
		{
			AddTail(&sq->sq_Anims,&an->an_Node);
			an->an_Number = 1;
		}
		else
		{
			AddTail(&sq->sq_Anims,&an->an_Node);
			an->an_Number = (((struct Anim *)an->an_Node.ln_Pred)->an_Number) + 1;
		}
		return(an);
	}
	else
		return(NULL);
}

static struct Anim *NewAnim(struct Sequence *sq)
{
	struct Anim *an;

	if( !(an = AllocVec(sizeof(struct Anim),MEMF_ANY|MEMF_CLEAR)) )
		return(NULL);

	NewList(&an->an_Frames);
	an->an_Node.ln_Name = an->an_Name;
	an->an_Name[0] = 0;
	an->an_Speed = 1;
	return(an);
}

static struct Frame *AddAfterFrame(struct Anim *an,struct Frame *prevframe,struct MyBob *bob,
				WORD x, WORD y)
{
	struct Frame *fr;

	if( fr = AllocFrame(bob,x,y) )
	{
		Insert(&an->an_Frames,&fr->fr_Node,&prevframe->fr_Node);
		fr->fr_Number = (((struct Frame *)fr->fr_Node.ln_Pred)->fr_Number) + 1;
		if(fr->fr_Node.ln_Succ->ln_Succ)
			RenumberFrames(an);
	}
	return(fr);
}

static struct Frame *AddTailFrame(struct Anim *an,struct MyBob *bob,
				WORD x, WORD y)
{
	struct Frame *fr;

	if( fr = AllocFrame(bob,x,y) )
	{
		if( IsListEmpty(&an->an_Frames) )
		{
			AddTail(&an->an_Frames,&fr->fr_Node);
			fr->fr_Number = 1;
		}
		else
		{
			AddTail(&an->an_Frames,&fr->fr_Node);
			fr->fr_Number = (((struct Frame *)fr->fr_Node.ln_Pred)->fr_Number) + 1;
		}
		return(fr);
	}
	else
		return(NULL);
}

static struct Frame *AllocFrame(struct MyBob *bob, WORD x, WORD y)
{
	struct Frame *fr;

	if( fr = AllocVec(sizeof(struct Frame),MEMF_ANY|MEMF_CLEAR) )
	{
		fr->fr_Bob = bob;
		fr->fr_X = x;
		fr->fr_Y = y;
		return(fr);
	}
	else
		return(NULL);
}

static void GotoNewViewBob(struct SeqWindow *asw, struct MyBob *bob)
{
	if(bob)
	{
		asw->asw_ViewBob = bob;
		DisplayViewBob(asw,asw->asw_ViewBob);
	}
}

static void GotoNewFrame(struct SeqWindow *asw, struct Frame *fr)
{
	if(fr)
	{
		asw->asw_CurrentFrame = fr;
		DisplayFrame(asw,asw->asw_CurrentFrame);
	}
}

static void GotoNewAnim(struct SeqWindow *asw, struct Anim *an)
{
	struct Window *win;

	if(an)
	{
		asw->asw_CurrentAnim = an;
		if( !IsListEmpty(&an->an_Frames) )
			asw->asw_CurrentFrame = (struct Frame *)an->an_Frames.lh_Head;
		else
			asw->asw_CurrentFrame = NULL;

		DisplayFrame(asw,asw->asw_CurrentFrame);

		if(win = asw->asw_sw.sw_Window)
		{
			GT_SetGadgetAttrs(asw->asw_Gadgets[MYGT_ANIMNAME],win,NULL,
				GTST_String,		asw->asw_CurrentAnim->an_Node.ln_Name,
				TAG_END);

			GT_SetGadgetAttrs(asw->asw_Gadgets[MYGT_ANIMNUMBER],win,NULL,
				GTIN_Number,		asw->asw_CurrentAnim->an_Number,
				TAG_END);

			GT_SetGadgetAttrs(asw->asw_Gadgets[MYGT_ANIMSPEED],win,NULL,
				GTIN_Number,		asw->asw_CurrentAnim->an_Speed,
				TAG_END);
		}
	}
}

BOOL WriteANIMChunk(struct IFFHandle *handle, struct Sequence *sq, char *bobbankname)
{
	BOOL done = FALSE;
	struct SequenceFileHeader sfh;
	struct AnimFileHeader afh;
	struct FrameFileHeader ffh;
	struct Anim *an;
	struct Frame *fr;
	UWORD framecount;

	if( (!sq) || (IsListEmpty(&sq->sq_Anims)) )
		return(TRUE);

	/* Make sure that there is at least one anim with frames in here */
	done = FALSE;
	for(an = (struct Anim *)sq->sq_Anims.lh_Head;
			(an->an_Node.ln_Succ) && (!done);
			an = (struct Anim *)an->an_Node.ln_Succ)
		done = !IsListEmpty(&an->an_Frames);
	if(!done)	return(TRUE);

	if( PushChunk( handle, ID_CONK, ID_ANIM, IFFSIZE_UNKNOWN ) )
		goto errorreturn;

	Mystrncpy(sfh.sfh_BobBankName, bobbankname, 16);
	sfh.sfh_NumOfAnims = ((struct Anim *)sq->sq_Anims.lh_TailPred)->an_Number;
	if( WriteChunkBytes( handle, &sfh, sizeof(sfh) ) != sizeof(sfh) )
		goto errorreturn;

	for(an = (struct Anim *)sq->sq_Anims.lh_Head;
			an->an_Node.ln_Succ;
			an = (struct Anim *)an->an_Node.ln_Succ)
	{
		if( !IsListEmpty(&an->an_Frames) )
		{
			PackASCII(an->an_Name, afh.afh_AnimName, ANIMNAMESIZE-1);
			afh.afh_NumOfFrames = (((struct Frame *)an->an_Frames.lh_TailPred)->fr_Number) * an->an_Speed;
			afh.afh_AnimSpeed = an->an_Speed;
			if( WriteChunkBytes( handle, &afh, sizeof(struct AnimFileHeader) ) != sizeof(struct AnimFileHeader) )
				goto errorreturn;

			for(fr = (struct Frame *)an->an_Frames.lh_Head;
					fr->fr_Node.ln_Succ;
					fr = (struct Frame *)fr->fr_Node.ln_Succ)
			{
				ffh.ffh_BobNumber = fr->fr_Bob->bb_Number;
				ffh.ffh_XOffset = fr->fr_X;
				ffh.ffh_YOffset = fr->fr_Y;
				for(framecount = 0; framecount < an->an_Speed; framecount++)
				{
					if( WriteChunkBytes( handle, &ffh, sizeof(struct FrameFileHeader) ) != sizeof(struct FrameFileHeader) )
						goto errorreturn;
				}
			}
		}
		else
		{
			PackASCII(an->an_Name, afh.afh_AnimName, ANIMNAMESIZE-1);
			afh.afh_NumOfFrames = 0;
			if( WriteChunkBytes( handle, &afh, sizeof(struct AnimFileHeader) ) != sizeof(struct AnimFileHeader) )
				goto errorreturn;
		}
		ffh.ffh_BobNumber = 0;
		ffh.ffh_XOffset = 0;
		ffh.ffh_YOffset = 0;
		if( WriteChunkBytes( handle, &ffh, sizeof(struct FrameFileHeader) ) != sizeof(struct FrameFileHeader) )
			goto errorreturn;
	}
	PopChunk(handle);
	return(TRUE);

errorreturn:
	return(FALSE);
}

BOOL LoadAnim(struct Project *pj, struct IFFHandle *handle, char *name)
{
	BOOL abort = FALSE;
	struct MyBob *bob;
	struct SequenceFileHeader sfh;
	struct AnimFileHeader afh;
	struct FrameFileHeader ffh;
	UWORD animcount,framecount;
	struct	Sequence *sq;
	struct Anim *an;
	struct Frame *fr;
	UWORD speedcount;

	AllocNewSequence(pj,FALSE);
	sq = pj->pj_Sequence;

	if( ReadChunkBytes(handle,&sfh,sizeof(sfh)) != sizeof(sfh))
		return(FALSE);

	for(animcount = 0;
		 ( (animcount < sfh.sfh_NumOfAnims) && (!abort) );
			animcount++)
	{
		if(ReadChunkBytes(handle,&afh,sizeof(struct AnimFileHeader)) == sizeof(struct AnimFileHeader))
		{
			if( an = AddTailAnim(sq) )
			{
				//Mystrncpy(an->an_Name,afh.afh_AnimName,ANIMFILENAMESIZE);
				UnpackASCII(afh.afh_AnimName, an->an_Name, ANIMNAMESIZE-1);
				an->an_Speed = afh.afh_AnimSpeed;
				if(an->an_Speed < 1) an->an_Speed = 1;

				if(afh.afh_NumOfFrames > 0)
				{
					for(framecount = 0;
						 ( (framecount < (afh.afh_NumOfFrames / an->an_Speed)) && (!abort) );
							framecount++)
					{
						if(ReadChunkBytes(handle,&ffh,sizeof(struct FrameFileHeader)) == sizeof(struct FrameFileHeader))
						{
							if( bob = FindBobByNumber(&pj->pj_BobList,ffh.ffh_BobNumber) )
							{
								if( fr = AddTailFrame(an,bob,ffh.ffh_XOffset,ffh.ffh_YOffset) )
								{
								}
								else
									abort = TRUE;
							}
							else
								abort = TRUE;
						}
						else
							abort = TRUE;
	
						/* Read In padded frames that are used to slow down anims */
						for(speedcount = 1; speedcount < an->an_Speed; speedcount++)
						{
							if( !(ReadChunkBytes(handle,&ffh,sizeof(ffh)) == sizeof(ffh)) )
								abort = TRUE;
						}
					}
				}
				if( !(ReadChunkBytes(handle,&ffh,sizeof(ffh)) == sizeof(ffh)) )
					abort = TRUE;
			}
		}
	}
	return((BOOL)!abort);
}

static void RenumberAnims(struct Sequence *sq)
{
	struct Anim *an;
	UWORD count = 1;

	if(IsListEmpty(&sq->sq_Anims))
		return;

	for(an = (struct Anim *)sq->sq_Anims.lh_Head;
			an->an_Node.ln_Succ;
			an = (struct Anim *)an->an_Node.ln_Succ)
		an->an_Number = count++;
}

static void RenumberFrames(struct Anim *an)
{
	struct Frame *fr;
	UWORD count = 1;

	if(IsListEmpty(&an->an_Frames))
		return;

	for(fr = (struct Frame *)an->an_Frames.lh_Head;
			fr->fr_Node.ln_Succ;
			fr = (struct Frame *)fr->fr_Node.ln_Succ)
		fr->fr_Number = count++;
}

static struct Frame *GetFrameByNumber(struct Anim *an, UWORD number)
{
	struct Frame *fr;

	for(fr = (struct Frame *)an->an_Frames.lh_Head;
			fr->fr_Node.ln_Succ;
			fr = (struct Frame *)fr->fr_Node.ln_Succ)
	{
		if(fr->fr_Number == number)
			return(fr);
	}
	return(NULL);
}

static struct Anim *GetAnimByNumber(struct Sequence *sq, UWORD number)
{
	struct Anim *an;

	for(an = (struct Anim *)sq->sq_Anims.lh_Head;
			an->an_Node.ln_Succ;
			an = (struct Anim *)an->an_Node.ln_Succ)
	{
		if(an->an_Number == number)
			return(an);
	}
	return(NULL);
}

struct Frame *DeleteFrame(struct Anim *an, struct Frame *fr)
{
	struct Frame *fr2 = NULL;
	WORD framenumber;

	if( (!fr) || (!an->an_Frames.lh_Head->ln_Succ) )
		return(NULL);

	framenumber = fr->fr_Number;
	Remove(&fr->fr_Node);
	FreeVec(fr);
	if(!IsListEmpty(&an->an_Frames))
	{
		RenumberFrames(an);
		fr2 = GetFrameByNumber(an,framenumber);
		while( (!fr2) && (framenumber >= 1) )
		{
			framenumber--;
			fr2 = GetFrameByNumber(an,framenumber);
		}
	}
	return(fr2);
}

static void RefreshGadget(struct Sequence *sq, struct SeqWindow *missasw, UWORD gadgetid)
{
	struct SeqWindow *asw;
	struct Window *win;

	if( (!sq) || (!sq->sq_SeqWindows) )
		return;

	asw = sq->sq_SeqWindows;
	while(asw)
	{
		if( (asw != missasw) && (win = asw->asw_sw.sw_Window) )
		{
			switch(gadgetid)
			{
				case MYGT_BOBNAME:
					GT_SetGadgetAttrs(asw->asw_Gadgets[MYGT_BOBNAME],win,NULL,
						GTST_String,		asw->asw_ViewBob->bb_Node.ln_Name,
						TAG_END);
					break;
				case MYGT_ANIMNAME:
					GT_SetGadgetAttrs(asw->asw_Gadgets[MYGT_ANIMNAME],win,NULL,
						GTST_String,		asw->asw_CurrentAnim->an_Node.ln_Name,
						TAG_END);
					break;
				case MYGT_XPOS:
				case MYGT_YPOS:
				case MYGT_NEWPOS:
					DisplayFrame(asw,asw->asw_CurrentFrame);
					break;
				case MYGT_ANIMNUMBER:
					DisplayFrame(asw,asw->asw_CurrentFrame);					
					GT_SetGadgetAttrs(asw->asw_Gadgets[MYGT_ANIMNAME],win,NULL,
						GTST_String,		asw->asw_CurrentAnim->an_Node.ln_Name,
						TAG_END);
					GT_SetGadgetAttrs(asw->asw_Gadgets[MYGT_ANIMNUMBER],win,NULL,
						GTIN_Number,		asw->asw_CurrentAnim->an_Number,
						TAG_END);
					break;
				case MYGT_ANIMSPEED:
					GT_SetGadgetAttrs(asw->asw_Gadgets[MYGT_ANIMSPEED], win, NULL,
						GTIN_Number,		asw->asw_CurrentAnim->an_Speed,
						TAG_END);
					break;
			}
		}
		asw = asw->asw_NextSeqWindow;
	}
}

static void SussMouseButtons(struct SeqWindow *asw, WORD mx, WORD my, UWORD code)
{
	if( (!asw) || (!asw->asw_CurrentFrame) )
		return;

	if(code & IECODE_UP_PREFIX)
	{
		if(asw->asw_ButtonDown)
		{
			asw->asw_ButtonDown = FALSE;
			RefreshGadget(asw->asw_Sequence,asw,MYGT_NEWPOS);
		}
	}
	else
	{
		if( (mx>=asw->asw_FrameLeft) && (mx < asw->asw_FrameLeft+asw->asw_FrameWidth) &&
				(my>=asw->asw_FrameTop) && (my < asw->asw_FrameTop+asw->asw_FrameHeight) )
		{
			asw->asw_ButtonDown = TRUE;
			asw->asw_HandleLeft = mx-asw->asw_FrameLeft;
			asw->asw_HandleTop = my-asw->asw_FrameTop;
		}
		else
		{
			if( (mx>=asw->asw_AnimBoxLeft) && (mx < asw->asw_AnimBoxLeft+asw->asw_AnimBoxWidth) &&
					(my>=asw->asw_AnimBoxTop) && (my < asw->asw_FrameTop+asw->asw_AnimBoxHeight) )
			{
				if(mx < asw->asw_FrameLeft)
					asw->asw_CurrentFrame->fr_X -= 1;
				if(my < asw->asw_FrameTop)
					asw->asw_CurrentFrame->fr_Y -= 1;
				if(mx >= asw->asw_FrameLeft+asw->asw_FrameWidth)
					asw->asw_CurrentFrame->fr_X += 1;
				if(my >= asw->asw_FrameTop+asw->asw_FrameHeight)
					asw->asw_CurrentFrame->fr_Y += 1;
				DisplayFrame(asw,asw->asw_CurrentFrame);				
			}
		}
	}
}

static void HandleMouseMove(struct SeqWindow *asw,WORD mx, WORD my)
{
	if( (asw) && (asw->asw_CurrentFrame) && (asw->asw_ButtonDown) )
	{
		mx -= asw->asw_AnimBoxLeft;
		my -= asw->asw_AnimBoxTop;
		mx -= asw->asw_HandleLeft;
		my -= asw->asw_HandleTop;

		asw->asw_CurrentFrame->fr_X = mx;
		asw->asw_CurrentFrame->fr_Y = my;
		DisplayFrame(asw,asw->asw_CurrentFrame);
	}
}

/***********************   AddSingleFrame()   *************************/
//
//  Create a new frame containing the bob 'bob', and append it after the
// anim frame 'fr'. If 'fr' == NULL then it will add the bob to the head of
// the anim 'an'.
//  You should always display the frame again after calling this routine, as it
// adjusts all the frame offsets if needed.
// Returns a pointer to the new frame.

static struct Frame *AddSingleFrame(struct SeqWindow *asw,
																		struct Anim *an, struct Frame *fr,
																		struct MyBob *bob)
{
	struct Frame *fr2;

	if( (!asw) || (!an) || (!bob) )
		return(NULL);

	if(fr)
		fr2 = AddAfterFrame(an, fr, bob, 0, 0);
	else
		fr2 = AddTailFrame(an, bob, 0, 0);

	if( (fr2) && (asw->asw_RelativePositions) && (!IsListEmpty(&an->an_Frames)) )
		SetOffsets(an);

	return(fr2);
}

static void GUIAddRange(struct SeqWindow *asw)
{
	UWORD first,last,tmp;
	struct MyBob *firstbob,*lastbob;
	struct List *boblist;
	static struct HoopyObject hob[] =
	{
		{ HOTYPE_VGROUP, 100, HOFLG_FREEWIDTH|HOFLG_FREEHEIGHT,4,0,0,0, NULL, 0,0,0,0,0 },
			{ HOTYPE_INTEGER, 100, 0, 5,0,0,0, "First:",FALSE,0,PLACETEXT_LEFT,0,0 },
			{ HOTYPE_INTEGER, 100, 0, 5,0,0,0, "Last:",FALSE,0,PLACETEXT_LEFT,0,0 },
			{ HOTYPE_SPACE, 10, HOFLG_NOBORDER|HOFLG_FREEWIDTH|HOFLG_FREEHEIGHT, 0,0,0,0, NULL,0,0,0,0,0 },
			{ HOTYPE_HGROUP, 100, HOFLG_NOBORDER|HOFLG_FREEWIDTH,3,0,0,0, NULL, 0,0,0,0,0 },
				{ HOTYPE_BUTTON, 100, 0, 0,0,0,0, "OK",0,0x8000,0,0,0 },
				{ HOTYPE_SPACE, 100, HOFLG_NOBORDER|HOFLG_FREEWIDTH, 0,0,0,0, NULL,0,0,0,0,0 },
				{ HOTYPE_BUTTON, 100, 0, 0,0,0,0, "Cancel",0,0xFFFF,0,0,0 },
		{HOTYPE_END}
	};


	hob[1].ho_Value = asw->asw_ViewBob->bb_Number;
	hob[2].ho_Value = asw->asw_ViewBob->bb_Number;

	if(	HO_SimpleDoReq( hob, "Add Range", NULL) != 0xFFFF)
	{
		first = hob[1].ho_Value;
		last = hob[2].ho_Value;
		if(first > last)
		{
			tmp = first;
			first = last;
			last = tmp;
		}
		boblist = &asw->asw_Sequence->sq_Project->pj_BobList;

		if( !(firstbob = FindBobByNumber(boblist,first)) ||
				!(lastbob = FindBobByNumber(boblist,last)) )
		{
			GroovyReq("Add Range","Bob Number %d\nNot Found","Cancel");
			return;
		}
		AddRange(asw,firstbob,lastbob);
	}
}

static void AddRange(struct SeqWindow *asw,
										 struct MyBob *startbob, struct MyBob *stopbob)
{
	struct Sequence *sq;
	struct Anim *an;
	struct Frame *prevframe;
	struct MyBob *bob;
	struct Frame *fr;

	sq = asw->asw_Sequence;
	an = asw->asw_CurrentAnim;
	prevframe = asw->asw_CurrentFrame;

	if(startbob == stopbob)
	{
		if( fr = AddSingleFrame(asw, an, prevframe, startbob) )
		{
			GotoNewFrame(asw,fr);
			RefreshGadget(sq,asw,MYGT_NEWPOS);
		}
		return;
	}

	for(bob  = startbob;
			bob != (struct MyBob *)stopbob->bb_Node.ln_Succ;
			bob  = (struct MyBob *)bob->bb_Node.ln_Succ)
	{
		if( fr = AddSingleFrame(asw, an, prevframe, bob) )
		{
			GotoNewFrame(asw,fr);
			prevframe = fr;
		}
	}
}

static void GUIDeleteRange(struct SeqWindow *asw)
{
	struct List *framelist;
	UWORD first,last,tmp;
	struct Frame *firstframe,*lastframe;
	struct Anim *an;
	static struct HoopyObject hob[] =
	{
		{ HOTYPE_VGROUP, 100, HOFLG_FREEWIDTH|HOFLG_FREEHEIGHT,4,0,0,0, NULL, 0,0,0,0,0 },
			{ HOTYPE_INTEGER, 100, 0, 5,0,0,0, "First:",FALSE,0,PLACETEXT_LEFT,0,0 },
			{ HOTYPE_INTEGER, 100, 0, 5,0,0,0, "Last:",FALSE,0,PLACETEXT_LEFT,0,0 },
			{ HOTYPE_SPACE, 10, HOFLG_NOBORDER|HOFLG_FREEWIDTH|HOFLG_FREEHEIGHT, 0,0,0,0, NULL,0,0,0,0,0 },
			{ HOTYPE_HGROUP, 100, HOFLG_NOBORDER|HOFLG_FREEWIDTH,3,0,0,0, NULL, 0,0,0,0,0 },
				{ HOTYPE_BUTTON, 100, 0, 0,0,0,0, "OK",0,0x8000,0,0,0 },
				{ HOTYPE_SPACE, 100, HOFLG_NOBORDER|HOFLG_FREEWIDTH, 0,0,0,0, NULL,0,0,0,0,0 },
				{ HOTYPE_BUTTON, 100, 0, 0,0,0,0, "Cancel",0,0xFFFF,0,0,0 },
		{HOTYPE_END}
	};


	an = asw->asw_CurrentAnim;
	framelist = &an->an_Frames;

	hob[1].ho_Value = ((struct Frame *)framelist->lh_Head)->fr_Number;
	hob[2].ho_Value = ((struct Frame *)framelist->lh_TailPred)->fr_Number;

	if(	HO_SimpleDoReq( hob, "Delete Range", NULL) != 0xFFFF)
	{
		first = hob[1].ho_Value;
		last = hob[2].ho_Value;
		if(first > last)
		{
			tmp = first;
			first = last;
			last = tmp;
		}
		if( !(firstframe = GetFrameByNumber(an,first)) )
		{
			GroovyReq("Delete Range","Frame Number %d\nNot Found","Cancel",first);
			return;
		}

		if( !(lastframe = GetFrameByNumber(an,last)) )
		{
			GroovyReq("Delete Range","Frame Number %d\nNot Found","Cancel",last);
			return;
		}
		DeleteRange(asw->asw_Sequence,an,firstframe,lastframe);
		RefreshGadget(asw->asw_Sequence,NULL,MYGT_NEWPOS);
	}
}

static void DeleteRange(struct Sequence *sq,struct Anim *an,struct Frame *firstframe, struct Frame *lastframe)
{
	struct Frame *frame,*nextframe,*newframe,*deletedframe,*lfsucc;
	struct SeqWindow *asw;

	frame  = firstframe;
	lfsucc = (struct Frame *)lastframe->fr_Node.ln_Succ;
	while(frame != lfsucc)
	{
		nextframe  = (struct Frame *)frame->fr_Node.ln_Succ;
		if(deletedframe = frame)
		{
			newframe = DeleteFrame(an,frame);
			for(asw = sq->sq_SeqWindows; asw; asw = asw->asw_NextSeqWindow)
			{
				if(asw->asw_CurrentFrame == deletedframe)
					asw->asw_CurrentFrame = newframe;
			}
		}
		frame = nextframe;
	}
}

static struct Anim *DeleteAnim(struct SeqWindow *asw)
{
	struct Sequence *sq;
	struct Anim *an,*an2 = NULL,*deletedanim;
	WORD animnumber;

	sq = asw->asw_Sequence;
	an = asw->asw_CurrentAnim;

	if(!IsListEmpty(&an->an_Frames))
	{
		/* Delete Existing Frames */
		DeleteRange(sq,an,(struct Frame *)an->an_Frames.lh_Head,(struct Frame *)an->an_Frames.lh_TailPred);
	}
	deletedanim = an;
	animnumber = an->an_Number;
	Remove(&an->an_Node);
	FreeVec(an);
	if(!IsListEmpty(&sq->sq_Anims))
	{
		RenumberAnims(sq);
		an2 = GetAnimByNumber(sq,animnumber);
		while( (!an2) && (animnumber >= 1) )
		{
			animnumber--;
			an2 = GetAnimByNumber(sq,animnumber);
		}
	}
	else
		an2 = AddTailAnim(sq);

	for(asw = sq->sq_SeqWindows; asw; asw = asw->asw_NextSeqWindow)
	{
		if(asw->asw_CurrentAnim == deletedanim)
		{
			asw->asw_CurrentAnim = an2;
			if(IsListEmpty(&an2->an_Frames))
				asw->asw_CurrentFrame = NULL;
			else
				asw->asw_CurrentFrame = (struct Frame *)an2->an_Frames.lh_Head;
		}
	}
	RefreshGadget(sq,NULL,MYGT_ANIMNUMBER);
	return(an2);
}

void DeleteBobFromAllAnims(struct Sequence *sq, struct MyBob *bob)
{
	struct SeqWindow *asw;
	struct Anim *an;
	struct Frame *fr, *nextfr, *newfr;

	if(sq)
	{
		for(an = (struct Anim *)sq->sq_Anims.lh_Head;
				an->an_Node.ln_Succ;
				an = (struct Anim *)an->an_Node.ln_Succ)
		{
			fr = (struct Frame *)an->an_Frames.lh_Head;
			while(fr->fr_Node.ln_Succ)
			{
				nextfr = (struct Frame *)fr->fr_Node.ln_Succ;
				if(fr->fr_Bob == bob)
				{
					newfr = DeleteFrame(an, fr);

					for(asw = sq->sq_SeqWindows; asw; asw = asw->asw_NextSeqWindow)
					{
						if(asw->asw_CurrentFrame == fr)
							asw->asw_CurrentFrame = newfr;
					}
					RefreshGadget(sq,NULL,MYGT_NEWPOS);
				}
				fr = nextfr;
			}
		}
	}
}

void MakeSureSeqWindowsDontViewBob(struct Sequence *sq, struct MyBob *bob)
{
	struct Project *pj;
	struct SeqWindow *asw, *nextasw;

	if((sq) && (pj = sq->sq_Project))
	{
		if( (pj->pj_BobList.lh_Head->ln_Succ) && (pj->pj_BobList.lh_Head->ln_Succ->ln_Succ) && (pj->pj_BobList.lh_Head->ln_Succ->ln_Succ->ln_Succ))
		{
			for(asw = sq->sq_SeqWindows; asw; asw = asw->asw_NextSeqWindow)
			{
				if(asw->asw_ViewBob == bob)
				{
					if( (asw->asw_ViewBob->bb_Node.ln_Succ) &&
							(asw->asw_ViewBob->bb_Node.ln_Succ->ln_Succ) &&
							(asw->asw_ViewBob->bb_Node.ln_Succ->ln_Succ->ln_Succ) )
						GotoNewViewBob(asw, (struct MyBob *)asw->asw_ViewBob->bb_Node.ln_Succ);
					else
						GotoNewViewBob(asw, (struct MyBob *)pj->pj_BobList.lh_Head);
				}
			}
		}
		else
		{
			asw = sq->sq_SeqWindows;
			while(asw)
			{
				nextasw = asw->asw_NextSeqWindow;
				(*asw->asw_sw.sw_BuggerOff)(&asw->asw_sw);
				asw = nextasw;
			}
		}
	}
}

/***********************   AddSingleFrame()   *************************/
//
//  Adjust all the frame offsets so that they play back like they were
// created in the first place.
//  Should most probably redisplay the frame after calling this routine.

static void SetOffsets(struct Anim *an)
{
	WORD left,top;
	struct Frame *fr;

	if( !IsListEmpty(&an->an_Frames) )
	{
		left = ((struct Frame *)an->an_Frames.lh_Head)->fr_Bob->bb_Left;
		top = ((struct Frame *)an->an_Frames.lh_Head)->fr_Bob->bb_Top;
		for(fr = (struct Frame *)an->an_Frames.lh_Head;
				fr->fr_Node.ln_Succ;
				fr = (struct Frame *)fr->fr_Node.ln_Succ)
		{
			if(fr->fr_Bob->bb_Left < left)
				left = fr->fr_Bob->bb_Left;
			if(fr->fr_Bob->bb_Top < top)
				top = fr->fr_Bob->bb_Top;
		}
		for(fr = (struct Frame *)an->an_Frames.lh_Head;
				fr->fr_Node.ln_Succ;
				fr = (struct Frame *)fr->fr_Node.ln_Succ)
		{
			fr->fr_X = fr->fr_Bob->bb_Left - left;
			fr->fr_Y = fr->fr_Bob->bb_Top - top;
		}
	}
}

/***********************   CalcAnimHitZones()   *************************/
//
//  Calc new Hit Zones for all the bobs in the current anim. This hit zone
// be the mini max of all the frames in the anim, so that the zone will
// be a constant size for all the bobs. This makes for better background
// collision zone detecting.
// Most probably pay to call up RefreshProjectName() after calling this
// routine to get the project windows displaying the 'Z' title flag.

static void CalcAnimHitZones(struct Project *pj, struct Anim *an)
{
	struct Frame *fr;
	struct MyBob *bob;
	WORD left = 16000, top = 16000, right = -16000, bottom = -16000;
	WORD realleft, realtop, realright, realbottom;

	if(!an || !pj)
		return;

	fr = (struct Frame *)an->an_Frames.lh_Head;
	if( fr->fr_Bob )
	{
		for(; fr->fr_Node.ln_Succ; fr = (struct Frame *)fr->fr_Node.ln_Succ)
		{
			if(bob = fr->fr_Bob)
			{
				if(fr->fr_X < left) left = fr->fr_X;
				if(fr->fr_Y < top) top = fr->fr_Y;
				if(fr->fr_X + (bob->bb_Width-1) > right) right = fr->fr_X + (bob->bb_Width-1);
				if(fr->fr_Y + (bob->bb_Height-1) > bottom) bottom = fr->fr_Y + (bob->bb_Height-1);
			}
		}

		fr = (struct Frame *)an->an_Frames.lh_Head;
		bob = fr->fr_Bob;
		realleft = (left - fr->fr_X) + bob->bb_Left;
		realright = (right - fr->fr_X) + bob->bb_Left;
		realtop = (top - fr->fr_Y) + bob->bb_Top;
		realbottom = (bottom - fr->fr_Y) + bob->bb_Top;
		for(;	fr->fr_Node.ln_Succ; fr = (struct Frame *)fr->fr_Node.ln_Succ)
		{
			if(bob = fr->fr_Bob)
			{
				bob->bb_ColLeft = realleft;
				bob->bb_ColRight = realright;
				bob->bb_ColTop = realtop;
				bob->bb_ColBottom = realbottom;
				bob->bb_Flags |= MBF_COLLISIONZONESET;
			}
		}
	}
	SussOutModificationStuff(pj);
}

static void RenameBobs(struct Project *pj, struct Anim *an)
{
	struct Frame *fr = NULL;
	UWORD num, count;
	struct MyBob **bobptrs = NULL;
	BOOL frameok;
	char bobname[BOBNAMESIZE];

	if(!IsListEmpty(&an->an_Frames))
	{
		fr = (struct Frame *)an->an_Frames.lh_TailPred;
		if(bobptrs = AllocVec(fr->fr_Number*4, MEMF_ANY|MEMF_CLEAR))
		{
			if( (strlen(an->an_Node.ln_Name)+Digits(fr->fr_Number)) < BOBNAMESIZE)
			{
				num = 1;
				for(fr = (struct Frame *)an->an_Frames.lh_Head;
						fr->fr_Node.ln_Succ;
						fr = (struct Frame *)fr->fr_Node.ln_Succ)
				{
					if(fr->fr_Bob)
					{
						frameok = TRUE;
						for(count = 1; count < num; count++)
						{
							if(bobptrs[count-1] == fr->fr_Bob)
								frameok = FALSE;
						}
						if(frameok)
						{
							bobptrs[num-1] = fr->fr_Bob;
							sprintf(bobname, "%s%u", an->an_Node.ln_Name, num);
							SetBobName(pj, fr->fr_Bob, bobname);
						}
					}
					num++;
				}
			}
			else
			{
				DisplayBeep(mainscreen);
			}
			FreeVec(bobptrs);
		}
	}
	else
	{
		DisplayBeep(mainscreen);
	}
}
