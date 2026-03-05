/****************************************************************************/
//
// BONK: ExternalVars.h
//
/****************************************************************************/




/* GLOBAL VARIABLES */

// extern struct IntuitionBase		*IntuitionBase;
// extern struct GfxBase					*GfxBase;
// extern struct DosLibrary			*DOSBase;
// extern struct Library					*GadToolsBase;
// extern struct RxsLib          *RexxSysBase;
// extern struct Library					*DiskfontBase;
// extern struct Library					*AslBase;
// extern struct Library					*IFFParseBase;

extern struct Process					*taskbase;
extern struct MsgPort					*mainmsgport;
extern struct RexxHost					*myrexxhost;

extern BOOL											ownscreen;
extern struct Screen						*mainscreen;
extern struct TextAttr					screentextattr;
extern struct TextFont					*screentextfont;
extern struct TextAttr					windowtextattr;
extern struct TextFont					*windowtextfont;
extern struct TextAttr					topaztextattr;
extern struct TextFont					*topaztextfont;
extern struct FileRequester			*filereq;

extern struct ScreenModeRequester *screenmodereq;
extern struct FontRequester 		*fontreq;

extern struct List							superwindows;
extern struct List							projects;

extern APTR											oldtaskwindowptr;
//extern BOOL											showwindowid;

/* Global Screen config Stuff */
/*
extern ULONG								scr_DisplayID;
extern ULONG								scr_DisplayWidth;
extern ULONG								scr_DisplayHeight;
extern UWORD								scr_DisplayDepth;
extern UWORD								scr_OverscanType;
extern BOOL									scr_AutoScroll;
extern char									*scr_Name;
extern BOOL									scr_Shanghai;
*/

extern char									OldDefaultPubScreenName[MAXPUBSCREENNAME];
extern UWORD								OldPubScreenModes;

extern char									currentrexxwindow[6];

//extern struct WindowDefault windefs[SWTY_NUM];

extern struct Config glob;
extern struct BlockSet *globlocks;
extern UBYTE globlockpreviewcolours[256];
extern struct Palette *globpalette;
extern struct UndoBuffer undobuf;
extern struct ProjectWindow *prevprojwin;
extern UWORD lmbblk, mmbblk, rmbblk;

extern struct Brush						*currentbrush;
extern struct List						brushes;
extern struct BitMap					*scratchbm;
