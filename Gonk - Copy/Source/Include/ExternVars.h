


/* GLOBAL VARIABLES */

extern struct IntuitionBase		* __near IntuitionBase;
extern struct GfxBase					* __near GfxBase;
extern struct DosLibrary			* __near DOSBase;
extern struct Library					* __near GadToolsBase;
extern struct Library					* __near RexxSysBase;
extern struct Library					* __near DiskfontBase;
extern struct Library					* __near AslBase;
extern struct Library					* __near IFFParseBase;
extern struct Library					* __near LayersBase;
/* extern struct Library					* __near UtilityBase; */

extern struct Process					*taskbase;
extern struct MsgPort					*mainmsgport;
extern struct MsgPort					*procmsgport;
extern struct MsgPort					*subtaskport;
extern struct RexxHost				*myrexxhost;
extern struct Process					*messageproc;
extern struct Process					*subtaskproc;

extern BOOL											 ownscreen;
extern struct Screen						*mainscreen;
extern struct TextAttr					 screentextattr;
extern struct TextFont					*screentextfont;
extern struct TextAttr					 windowtextattr;
extern struct TextFont					*windowtextfont;
extern struct TextAttr					 topaztextattr;
extern struct TextFont					*topaztextfont;
extern struct FileRequester			*filereq;

extern struct ScreenModeRequester *screenmodereq;
extern struct FontRequester 			*fontreq;

extern struct List							superwindows;
extern struct List							projects;

extern APTR											oldtaskwindowptr;

extern char										 OldDefaultPubScreenName[MAXPUBSCREENNAME];
extern UWORD									 OldPubScreenModes;

extern char										 currentrexxwindow[6];

extern struct Config					 glob;
extern struct Palette					*globpalette;
extern UWORD	  __far globpens[NUMDRIPENS+1];

extern struct Brush					 __far brushes[];
extern struct AnimBrush			 __far animbrush;

extern UWORD								 __far currenttool;
extern UWORD								 __far prevtool;
extern struct Brush					* __far currentbrush;
extern BOOL									 __far gridlockstate;
extern UWORD 								 __far toolstate, lmbcol, mmbcol, rmbcol;
extern BOOL									 __far altdown;
extern struct ProjectWindow * __far lastprojectwindow;
extern struct PaletteWindow * __far lastpalettewindow;
extern struct BitMap				* __far workbm;
extern UWORD 								 __far workbmwidth;
extern UWORD 								 __far workbmheight;
extern UWORD								 __far workbmdepth;
extern char										  __far screentitlebuffer[80];

extern struct BitMap				* __far workmaskbm;
extern struct	IntuiMessage		* __far lastsubtaskmessage;
extern UWORD	drawmode;

//extern struct WindowDefault windefs[SWTY_NUM];

extern struct Palette  __far kludgepalette;
extern BOOL		 __far subtaskgoing;

/******************************  TEXT STUFF *****************************************/
/* Library Names */
extern char Txt_IntuitionLibrary[];
extern char Txt_GraphicsLibrary[];
extern char Txt_DosLibrary[];
extern char Txt_GadToolsLibrary[];
extern char Txt_RexxSysLibLibrary[];
extern char Txt_DiskFontLibrary[];
extern char Txt_AslLibrary[];
extern char Txt_IFFParseLibrary[];

/* Rexx Errors */
extern char Txt_SelectedWindowDoesntExist[];
extern char Txt_CantOpenProject[];
extern char Txt_CantActivateAHiddenWindow[];
extern char Txt_WindowDoesntExist[];

/* Window Titles */
extern char Txt_FileRequester[];
extern char Txt_ScreenModeRequester[];
extern char Txt_FontRequester[];
extern char Txt_About[];
extern char Txt_ConstructionKit[];
extern char Txt_EditorError[];
extern char Txt_Untitled[];
extern char Txt_PublicScreenRequester[];

/* File Names */
extern char Txt_DefaultConfig[];
extern char Txt_DefaultRexx[];

/* General Errors */
extern char Txt_Workbench2Required[];
extern char Txt_CantOpenScreen[];
extern char Txt_CantOpenWindow[];
extern char Txt_CantOpenGraphicsLibrary[];
extern char Txt_CantOpenDOSLibrary[];
extern char Txt_CantOpenGadToolsLibrary[];
extern char Txt_CantOpenIFFParseLibrary[];
extern char Txt_CantOpenRexxSysLibrary[];
extern char Txt_NotEnoughMemory[];
extern char Txt_CantOpenRexxPort[];
extern char Txt_CantOpenFont[];
extern char Txt_CantOpenASLLibrary[];
extern char Txt_CantOpenASLRequester[];
extern char Txt_CouldntCloseScreen[];

/* Fonts */
extern char Txt_TopazFont[];

/* Gadget Text */
extern char Txt_TestGad[];
extern char Txt_Abort[];
extern char Txt_Cancel[];
extern char Txt_OK[];

/* Requester Stuff */
extern char Txt_ModifiedProject[];
extern char Txt_SaveAbandonCancel[];
extern char Txt_DoYouWantToQuit[];
extern char Txt_QuitDontQuit[];
extern char Txt_CantCloseScreenVisitorsOnScreen[];
extern char Txt_RetryAbort[];
extern char Txt_FileMissingChunk[];
extern char Txt_CantAllocUndoBuffer[];
extern char Txt_Contine[];
extern char Txt_CantOpenFile[];
extern char Txt_FileNotILBM[];

/* Other Stuff */
extern char Txt_DefaultConsole[];
