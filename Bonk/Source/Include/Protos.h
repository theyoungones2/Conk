/****************************************************************************/
//
// BONK: Protos.h
//
/****************************************************************************/



#include <dos/dos.h>

/****  Main.c Protos  ****/

VOID		HandleIDCMPStuff(void);
void		__regargs __chkabort(void);
BOOL		OpenStuff(void);
void		CloseDown(void);
void		CloseWindowSafely( struct Window * );
void		StripIntuiMessages( struct MsgPort *, struct Window * );
BOOL		SetupScreen(struct ScreenConfig *scrconfig);
BOOL		OpenNewScreen(struct ScreenConfig *scrconfig);
BOOL		BorrowScreen(struct ScreenConfig *scrconfig);
BOOL		CloseOurScreen(struct ScreenConfig *scrconfig);
struct PubScreenNode *GetPubScreen(char *name);
void		SetProcessWindowPtr(void);
void InstallNewGlobalConfig(struct Config *cg);

/****  misc.c protos  ****/

struct Node *FindNthNode(struct List *list, UWORD num);
void ClearWindow( struct Window *win );
void	AdjustWindowFont(struct SuperWindow *sw,UWORD width,UWORD height);
char *MyStrDup(char *source);
struct List *ListAndHideWindows(void);
void ReshowWindowList(struct List *winlist);
struct Screen *GetDefaultScreen(void);
struct List *BlockAllWindows(void);
// struct List *NewBlockAllWindows(void);
void FreeBlockedWindows(struct List *list);
void SetWait(struct Window *win);
void ClrWait(struct Window *win);
void SetShanghaiState(struct Screen *scr, BOOL Shanghai);
BOOL GetPubScreenName(struct Screen *scr, char *name);
long DoDOS(char *cmd);
void WaitForWindowMove(void);
BOOL SussOutQuiting(BOOL force);
void MyRectFill(struct RastPort *rp, WORD x1, WORD y1, WORD x2, WORD y2);
void WinPlot( struct Window *win, WORD x, WORD y, UBYTE col );
WORD MyTextLength( struct TextFont *tf, char *str );
UWORD HandleGlobalRawKey( struct SuperWindow *sw, UWORD rawkey, UWORD qual );
struct BitMap *CreateBitMap(UWORD width, UWORD height, UWORD depth,
	BOOL interleaved);
void DestroyBitMap(struct BitMap *bm, UWORD width, UWORD height, UWORD depth);
char *Mystrncpy(char *to, char *from, int length);
BOOL MakeBackupFile( STRPTR filename );
char *MyStrToUpper( char *str );

/****  SuperWindows.c protos  ****/

void RememberWindow(struct SuperWindow *sw);
void InstallUniqueWindowID(struct SuperWindow *sw);
struct SuperWindow *FindSuperWindowByName(char *name);
struct Window *OpenKludgeWindow(void);
void CloseKludgeWindow(struct Window *win);
struct SuperWindow *GetOpenSuperWindow(void);
int CountSuperWindows(void);
char *BuildWindowTitle(struct SuperWindow *sw);
void RefreshAllMenus( void );
struct SuperWindow *FindSuperWindowByType( UWORD type );

/****  Projects.c Protos  ****/

void InitProject( struct Project *proj );
struct Project *OpenNewProject( UWORD width, UWORD height, STRPTR name );
struct Project *GUIOpenNewProject( void );
struct Project *OldOpenProject( char *drawer, char *file );
void CloseProject(struct Project *proj);
void RemoveProject(struct Project *proj);
struct Project *AllocProject( void );
void FreeProject(struct Project *proj);
void UnlinkProjectWindow(struct ProjectWindow *pw);
struct ProjectWindow *GetPrevProjectWindow(struct ProjectWindow *pw);
int	ModifiedProjectRequester(struct Project *proj);
int CountProjectWindows(struct Project *proj);
void RefreshProjectName(struct Project *proj );
void MaybeCloseProjectWindow( struct ProjectWindow *pw );
BOOL ResizeProject( struct Project *proj, UWORD neww, UWORD newh );
void GUIResizeProject( struct Project *proj );
void RedrawAllProjects( void );
BOOL LockBackground( struct Project *proj );
void UnlockBackground( struct Project *proj );
void SetBookMark( struct ProjectWindow *pw, UWORD num );
void GotoBookMark( struct ProjectWindow *pw, UWORD num );
void RenameMap( struct Project *proj );

/****  ProjectWindows.c Protos  ****/
BOOL OpenNewProjWindow(struct Project *proj);
void DrawMap(struct ProjectWindow *pw);
void UpdatePWScrollers(struct ProjectWindow *pw);
void JumpToNextProjectWindow( struct SuperWindow *thissw );

/****  AboutWindow.c Protos  ****/
BOOL OpenNewAboutWindow(void);

/****  RevealWindow.c Protos  ****/
BOOL OpenNewRevealWindow(UWORD type);
void RV_RemakeAll(void);

/****  ScreenConfigWindow.c Protos  ****/
int OpenNewScreenConfigWindow(void);


/****  Requesters.c Protos   ****/
int	ModifiedProjectRequester(struct Project *proj);
struct Window *OpenKludgeWindow(void);
void CloseKludgeWindow(struct Window *win);
int	QuitProgramRequester(void);
WORD __stdargs GroovyReq(STRPTR title,STRPTR text,STRPTR gadgets,...);
//int GroovyReq(struct EasyStruct *req);
int ScreenLockedReq(LONG locks);

/**** ProjectMenu.c	****/
UWORD DoProjectMenu(struct SuperWindow *sw,UWORD id);
/* shift these to Projects.c or something... */
void PM_Save( struct Project *proj, struct Window *win );
void PM_SaveAs( struct Project *proj, struct Window *win );

/****  WindowMenu.c  ****/
UWORD DoWindowMenu(struct SuperWindow *sw,UWORD id);
void SetupWindowMenu(struct SuperWindow *sw, struct NewMenu *nm);
void MakeDefault(struct SuperWindow *sw);

/****  SettingsMenu.c ****/
void DoSettingsMenu(struct SuperWindow *sw,UWORD id);
void SetupSettingsMenu(struct NewMenu *nm);
void SaveSettingsAs( struct Window *win );
void SaveSettings(char *name);
BOOL LoadSettingsAs(struct Window *win);
BOOL LoadSettings(char *name);

/**** UserMenu.c ****/
void LinkInUserMenu(struct NewMenu *nm);


/**** PubScreenRequester ****/
int PubScreenRequester(struct Window *parent, char *namebuffer);
void DoUserMenu(struct SuperWindow *sw,UWORD id);

/****  Blocks.c  ****/
struct BlockSet *LoadBlockSet( char *name );
struct BlockSet *LoadRawBlockSet( char *name, UBYTE depth );
struct BlockSet *CreateDummyBlockSet(void);
void FreeBlockSet( struct BlockSet *bs );
void KludgeLoadBlocks(void);
void DrawBlock( struct Window *win, LONG xpos, LONG ypos, UWORD blk );
void BltBlockWindow( struct Window *win, LONG xpos, LONG ypos, UWORD blk );
UBYTE *LoadFile( char *name, ULONG memtype );
void FreeMap(struct Project *proj);
UBYTE FindBlockColour( struct BlockSet *bs, UWORD blk );
void CalcBlockPreviewColours( struct BlockSet *bs );
void FreeBlockPreviewColours( struct BlockSet *bs );

BOOL InWindow(struct Window *win, WORD x, WORD y);
void BlkRectFill(struct Window *win, UWORD x0, UWORD y0,
	UWORD x1, UWORD y1);
void BlkRectOutline(struct Window *win, UWORD x0, UWORD y0,
	UWORD x1, UWORD y1);
WORD ClipToWindowBlkX( struct Window *win, WORD blkx );
WORD ClipToWindowBlkY( struct Window *win, WORD blky );

/**** Tools.c  ****/


void ResetUndoBuffer( struct ProjectWindow *pw, struct UndoBuffer *ub );
void RememberBlk( struct UndoBuffer *ub, UWORD x, UWORD y, UWORD blk );
void Undo( struct UndoBuffer *ub );
void FreeUndoBuffer( struct UndoBuffer *ub );
UWORD DominantBlock( UWORD buttonstate );
void RPlotBlock(struct ProjectWindow *pw, WORD mx, WORD my, UWORD blk);
void PlotBlock(struct ProjectWindow *pw, WORD mx, WORD my, UWORD blk);
UWORD PeekBlock(struct Project *proj, WORD x, WORD y );
void DoFBox(struct ProjectWindow *pw, WORD mx0, WORD my0,
	WORD mx1, WORD my1, UWORD blk );
void DrawSelector(struct ProjectWindow *pw, WORD x, WORD y);
void EraseSelector(struct ProjectWindow *pw);
void ResetSelector( struct ProjectWindow *pw );
void Tool_Down(struct ProjectWindow *pw, WORD x, WORD y, WORD button);
void Tool_Up(struct ProjectWindow *pw, WORD x, WORD y, WORD button);
void Tool_Move(struct ProjectWindow *pw, WORD x, WORD y);
void Tool_Cancel(struct ProjectWindow *pw, BOOL refresh);

/****  ColourStuff.c  ****/
void FreePalette( struct Palette *pal );
struct Palette *CreateDefaultPalette(void);
struct Palette *LoadPalette(char *name);
void InstallPalette(struct Screen *scr, struct Palette *pal);
//UWORD FindnBestPen( struct Palette *pal, UWORD numpens, UBYTE r, UBYTE g, UBYTE b );
void SussDrawInfoPens( UWORD *pens, struct Palette *pal, UWORD numcols );


/****  ToolsWindow.c Protos  ****/
BOOL OpenNewToolsWindow(void);
void SetTool( UWORD tool );
UWORD ReadTool( void );
void SetToolsWindowGadget(UWORD toolid,BOOL state);
struct Gadget *GetGadgetFromID(struct Gadget *gad,UWORD id);

/**** BlockRoutines.a Protos  ****/

//BOOL __asm BuildBlockHistogram( register __a0 struct BlockSet *bs,
//	register __a1 UWORD *array, register __d0 UWORD blk );
UBYTE	__asm BlkPickPixel( register __a0 struct BlockSet *bs,
	register __d0 UWORD x, register __d1 UWORD y, register __d2 UWORD blk );
UBYTE	__asm BMPlot( register __a0 struct BitMap *bm,
	register __d0 UWORD x, register __d1 UWORD y, register __d2 UWORD colour );
UWORD __asm FindnBestPen( register __a0 struct Palette *pal,
	register __d0 UWORD numpens,
	register __d1 UBYTE r, register __d2 UBYTE g, register __d3 UBYTE b );
UBYTE __asm CalcBlockColour( register __a0 struct BlockSet *bs,
	register __a1 struct Palette *pal,
	register __d0 UWORD blk );


/**** BlockWindow.c Protos  ****/
BOOL OpenNewBlockWindow( void );
void BlockWindowToFront( void );
void RedrawAllBlockWindows( void );

/****  MapSizeReq.c  ****/
BOOL MapSizeReq( char *titlestring, LONG *data );

/****  EditMenu.c ? ****/
UWORD DoEditMenu(struct SuperWindow *sw,UWORD id);

/****  HoopyReq.c  ****/
BOOL	DoHoopyReq( char *titlestring, struct ReqCrap *rc, UWORD count );

/****  Brushes.c  ****/
struct Brush *AllocBrush( UWORD width, UWORD height );
void FreeBrush( struct Brush *br );
struct Brush *GrabBrush( struct Project *proj, WORD x0, WORD y0,
	WORD x1, WORD y1 );
void PasteBrush( struct ProjectWindow *pw, WORD x0, WORD y0, struct Brush *br );
struct Brush *GrabBWBrush( struct BlockWindow *bw, WORD x0, WORD y0,
	WORD x1, WORD y1);
void FreeAllBrushes( void );
void FreeOldBrushes( void );

/**** PreviewWindow.c Protos  ****/
BOOL OpenNewPreviewWindow( struct Project *proj );
void DrawMapPreview( struct PreviewWindow *vw );

/**** BlkAnimWindow.c Protos  ****/
BOOL OpenNewBlkAnimWindow( struct Project *proj );

/****  BlkAnimStuff.c Protos  ****/
struct BlkAnim *CreateNewBlkAnim( WORD length );

/**** BrushWindow.c Protos  ****/
BOOL OpenNewBrushWindow(void);
void RefreshAllBrushWindows( void );

/****  Files.c  ****/
BOOL SaveProject( struct Project *proj, char *name, BOOL paletteflag,
	BOOL blocksetflag );
BOOL GUISaveProject( struct Project *proj );
BOOL GUISaveProjectAs( struct Project *proj, struct Window *win );
struct Project *OpenProject( char *drawer, char *file );
BOOL ReadRawMap( BPTR fh, struct Project *proj );
struct BlockSet *ReadRawBlockSet( BPTR fh, UBYTE depth, BOOL interleaved );

/****  MapMenu.c  ****/
UWORD DoMapMenu(struct SuperWindow *sw,UWORD id);
void SetupMapMenu(struct ProjectWindow *pw, struct NewMenu *nm);

/**** Screens.c ****/
LONG		GetScreenLocks(struct Screen *scr);
BOOL		SetupScreen(struct ScreenConfig *scrconfig);
BOOL		OpenNewScreen(struct ScreenConfig *scrconfig);
BOOL		BorrowScreen(struct ScreenConfig *scrconfig);
BOOL		CloseOurScreen(struct ScreenConfig *scrconfig);
struct PubScreenNode *GetPubScreen(char *name);
BOOL		UseNewScreenSettings(struct ScreenConfig *scrconfig);
BOOL		RemakeScreen(void);
