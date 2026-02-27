
/****  Main.c Protos  ****/

VOID	__regargs __chkabort(void);
BOOL 	InstallNewGlobalConfig(struct Config *cg);

/****  misc.c protos  ****/

BOOL MakeBackupFile( STRPTR filename );
UWORD NodeNumber(struct List *list, struct Node *node);
struct Node *FindNthNode(struct List *list, UWORD num);
void ClearWindow( struct Window *win );
void	AdjustWindowFont(struct SuperWindow *sw,UWORD width,UWORD height);
char *MyStrDup(char *source);
void HideAllWindows(void);
void ReshowHiddenWindows(void);
/* struct List *ListAndHideProjWindows(struct Project *pj); */
struct Screen *GetDefaultScreen(void);
//struct List *BlockAllWindows(void);
//void FreeBlockedWindows(struct List *list);
void SetWait(struct Window *win);
void ClrWait(struct Window *win);
void SetCorrectPointer(struct ProjectWindow *pw);
void SetShanghaiState(struct Screen *scr, BOOL Shanghai);
BOOL GetPubScreenName(struct Screen *scr, char *name);
long DoDOS(char *cmd);
void WaitForWindowMove(void);
BOOL SussOutQuiting(BOOL force);
UWORD GetPotValue(UWORD numlevels,UWORD currentlevel);
UWORD GetSliderLevel(UWORD numlevels,UWORD pot);
struct Gadget *GetGadgetFromID(struct Gadget *gad,UWORD id);
struct BitMap *CreateBitMap(UWORD width, UWORD height, UWORD depth, BOOL interleaved);
void DestroyBitMap(struct BitMap *bm, UWORD width, UWORD height, UWORD depth);
void ClearBitMap(struct BitMap *bm);
void ResetUndoBuffer(struct Project *proj);
void DoUndo(struct Project *proj);
void UpdateUndo(struct Project *pj, WORD x, WORD y);
void UpdateUndoBrush(struct Brush *br,struct Project *pj, WORD x, WORD y);
void DrawIndentBox(struct Window *win,UWORD xmin, UWORD ymin, UWORD xmax, UWORD ymax);
void SetProcessWindowPtr(void);
void __asm MyDeleteMsgPort(register __a0 struct MsgPort *port);
void CloseWindowSafely( struct Window * );
void	StripIntuiMessages( struct MsgPort *, struct Window * );
BOOL GetFile(struct Window *win, char *title, char *initaildrawer, char *initialfile,
						 BOOL dosavemode, BOOL domultiselect);
char *MakeFullPathName(char *drawer, char *file);
WORD MyTextLength( struct TextFont *tf, char *str );
void MySetPointer(struct Window *win, UWORD *sprite,UWORD width, UWORD height,
									WORD xoffset, WORD yoffset);
void MyClearPointer(struct Window *win);
struct Node *MyFindName(struct List *list, STRPTR name, struct Node *startnode);
void RethinkPaletteBox(struct PaletteBox *pb,
											UWORD boxleft, UWORD boxtop, UWORD boxwidth, UWORD boxheight);
void BevelColour(struct PaletteBox *pb, UWORD col);
void RestoreColour(struct PaletteBox *pb, UWORD col);
char *Mystrncpy(char *to, char *from, int length);
UWORD FindMaxStrLen( STRPTR *str, struct TextFont *tf );
UWORD CountStrings( STRPTR *str );
void ActivateWindowUnderMouse(void);
void SafeActivateWindowUnderMouse(void);
UWORD Digits(UWORD num);
UWORD HO_SimpleDoReq( struct HoopyObject *hob, char *title, struct Window *parentwin);

/****  SuperWindows.c protos  ****/

void RememberWindow(struct SuperWindow *sw);
void InstallUniqueWindowID(struct SuperWindow *sw);
struct SuperWindow *FindSuperWindowByName(char *name);
struct Window *OpenKludgeWindow(void);
void CloseKludgeWindow(struct Window *win);
struct SuperWindow *GetOpenSuperWindow(void);
int CountSuperWindows(void);
void RefreshAllMenus( void );
char *BuildWindowTitle(struct SuperWindow *sw);
void CloseAllSuperWindows(void);
APTR BlockWindow( struct Window *win );
VOID UnblockWindow( APTR blockwinhandle );
BOOL BlockAllSuperWindows( VOID );
VOID UnblockAllSuperWindows( VOID );
BOOL BlockProjectWindows( struct Project *pj );
VOID UnblockProjectWindows( struct Project *pj );
void SetSuperWindowScreenTitle(struct SuperWindow *sw);
void SetSuperWindowsScreenTitle(void);


/****  Projects.c Protos  ****/

struct File *AllocNewFile(void);
void FreeFile(struct File *fl);
void SetFileName(struct File *fl, char *name, char *drawer);
void LinkProjectToFile(struct File *fl, struct Project *pj);
struct Project *OpenMinProject(struct WindowDefault *wf, struct File *fl);
struct Project *OpenNewProject(struct WindowDefault *wf, struct File *fl);
struct Palette *CloseProject(struct Project *pj);
struct Palette *FreeProject(struct Project *pj);
void UnlinkProjectFromFile(struct Project *pj);
struct Project *GetPrevProjectInFile(struct Project *pj);
void UnlinkProjectWindow(struct ProjectWindow *pw);
struct ProjectWindow *GetPrevProjectWindow(struct ProjectWindow *pw);
int CountProjectWindows(struct Project *proj);
BOOL SetupProjBitMaps(struct Project *pj);
void DeallocProjBitMaps(struct Project *pj);
BOOL SetUpDefaultBitMaps(struct Project *pj);
BOOL SetUpAlmostDefaultBitMaps(struct Project *pj, UWORD colours);
void RefreshProjectName(struct Project *pj );
void RefreshAllProjectTitles(void);
void UseProjSettings(struct Project *pj);
void SussOutModificationStuff(struct Project *pj);
struct Project *GetEmptyProject(struct File *fl);
BOOL SetChunkName(struct Project *pj);


/****  ProjectWindows.c Protos  ****/
BOOL OpenNewProjWindow(struct WindowDefault *wf, struct Project *pj);
void ScrollUp(struct ProjectWindow *pw,UWORD ydelta, WORD mx, WORD my);		/* Delta's always absolute */
void ScrollDown(struct ProjectWindow *pw,UWORD ydelta, WORD mx, WORD my);
void ScrollLeft(struct ProjectWindow *pw,UWORD xdelta, WORD mx, WORD my);
void ScrollRight(struct ProjectWindow *pw,UWORD xdelta, WORD mx, WORD my);
void GetTrueWindowSize(struct ProjectWindow *pw);
void RedisplayProjWindow(struct ProjectWindow *pw, BOOL fullyredisplay);
void GetBitMapOffsets(WORD mx, WORD my, WORD *bmx, WORD *bmy, struct ProjectWindow *pw);
void GridLockOffsets(WORD *bmx, WORD *bmy, struct BrushInfo *bi, WORD width, WORD height);
void GetBrushGridLockOffsets(WORD *minx, WORD *miny, WORD *maxx, WORD *maxy);
void SetMagnifyBoxSize(struct ProjectWindow *pw);
void SetupEditMenu(struct Project *pj, struct NewMenu *nm);
BOOL NewEditType(struct Project *pj, UWORD newtype, UWORD newdepth);
void OutlineBitMapRegions(struct ProjectWindow *pw);


/****  AboutWindow.c Protos  ****/
BOOL OpenNewAboutWindow(struct WindowDefault *wf);

/****  RevealWindow.c Protos  ****/
BOOL OpenNewRevealWindow(struct WindowDefault *wf, UWORD type);
void RV_RemakeAll(void);

/****  ScreenConfigWindow.c Protos  ****/
BOOL OpenNewScreenConfigWindow(struct WindowDefault *wf);

/****  ToolsWindow.c Protos  ****/
BOOL OpenNewToolsWindow(struct WindowDefault *wf);
void SetToolsWindowGadget(UWORD toolid,BOOL state);
void SetNewTool(UWORD toolid,BOOL state);

/****  PaletteWindow.c Protos  ****/
BOOL OpenNewPaletteWindow(struct WindowDefault *wf);
void SetPalettePen(UWORD col, UWORD pen);
void RethinkPaletteWindows(UWORD oldlmbcol);
void RedisplayAllPaletteWindows( void );

/****  EditPaletteWindow.c Protos  ****/
BOOL OpenNewEditPaletteWindow(struct WindowDefault *wf, struct Project *pj, UWORD col);
BOOL RemakeScreenIfNeeded(struct Palette *pal);

/****  SeqWindow.c Protos  ****/
BOOL OpenNewSeqWindow(struct WindowDefault *wf, struct Project *pj);
void FreeSequence(struct Project *pj);
BOOL WriteANIMChunk(struct IFFHandle *handle, struct Sequence *sq, char *bobbankname);
BOOL LoadAnim(struct Project *pj, struct IFFHandle *handle, char *name);
struct Frame *DeleteFrame(struct Anim *an, struct Frame *fr);
void DeleteBobFromAllAnims(struct Sequence *sq, struct MyBob *bob);
void MakeSureSeqWindowsDontViewBob(struct Sequence *sq, struct MyBob *bob);

/****  RangeWindow.c ****/
BOOL OpenNewRangeWindow(struct WindowDefault *wf, struct Project *pj );
UWORD GetColourCellInRange(struct Range *rn, UWORD col);

/****  Requesters.c Protos   ****/
int	CheckPalettes(struct Project *pj);
int	ModifiedProjectRequester(struct Project *pj);
int	ModifiedSequenceRequester(struct Project *pj);
struct Window *OpenKludgeWindow(void);
void CloseKludgeWindow(struct Window *win);
int	QuitProgramRequester(void);
WORD __stdargs GroovyReq(STRPTR title,STRPTR text,STRPTR gadgets,...);
int ScreenLockedReq(LONG locks);
void MissingILBMChunkReq(char *name,char *chunk);
WORD InvalidNameRequester(void);
UWORD DeleteBobRequester( struct Project *pj, struct Window *parentwin );

/**** ProjectMenu.c	****/
UWORD DoProjectMenu(struct SuperWindow *sw,UWORD id);
BOOL PM_SaveFileAs(struct ProjectWindow *pw);

/****  WindowMenu.c  ****/
UWORD DoWindowMenu(struct SuperWindow *sw,UWORD id);
void SetupWindowMenu(struct SuperWindow *sw, struct NewMenu *nm);

/****  SettingsMenu.c ****/
void DoSettingsMenu(struct SuperWindow *sw,UWORD id);
void SetupSettingsMenu(struct NewMenu *nm);
void SaveSettingsAs( struct SuperWindow *sw);
void SaveSettings(char *name);
BOOL LoadSettingsAs(struct SuperWindow *sw);
BOOL LoadSettings(char *name);
void RefreshSettingsMenu(void);
BOOL LoadGCFGChunk(struct IFFHandle *handle);
BOOL WriteGCFGChunk( struct IFFHandle *handle, struct Config *cg);

/**** UserMenu.c ****/
void LinkInUserMenu(struct NewMenu *nm);

/**** EditMenu.c ****/
void DoEditMenu(struct ProjectWindow *pw,UWORD id);
void RefreshEditMenu(struct Project *pj);

/**** ModeMenu.c ****/
void SetupModeMenu(struct SuperWindow *sw, struct NewMenu *nm);
void DoModeMenu(struct SuperWindow *sw,UWORD id);
void NewDrawMode(UWORD drmd);
void RefreshModeMenu(void);

/**** BrushMenu.c ****/
void DoBrushMenu(struct SuperWindow *sw,UWORD id);

/**** UserMenu.c ****/
void DoUserMenu(struct SuperWindow *sw,UWORD id);

/**** PubScreenRequester.c ****/
int PubScreenRequester(struct Window *parent, char *namebuffer);

/**** GetProjectRequester.c ****/
struct Project *GetProjectRequester(struct Window *parent, struct Project *missproject, char *title);

/**** GetBobRequester.c ****/
struct MyBob *GetBobRequester(struct ProjectWindow *pw, char *title);

/**** PlayAnimRequester.c ****/
void PlayAnimRequester(struct Sequence *sq,struct Anim *an);

/**** IFF.c ****/
BOOL OpenFile(STRPTR file, STRPTR drawer, struct Project *originalproject);
struct MyILBMStruct *LoadnScanILBM(struct IFFHandle *handle);
void FreeLoadnScanILBM(struct MyILBMStruct *mi);
BOOL LoadCrapChunk(struct IFFHandle *handle, struct File *fl, ULONG chunkid, ULONG size);
BOOL WriteCrapChunks(struct File *fl, struct IFFHandle *handle);
BOOL LoadILBM(struct IFFHandle *handle, char *file, char *drawer, struct Project *originalproject, struct File *fl);
void FreeStoredProps(struct Project *proj);
BOOL SaveILBM(struct Project *pj, struct IFFHandle *handle);
BOOL WriteCMAPChunk( struct IFFHandle *handle, struct Palette *pal);
BOOL LoadCMAPIntoPalette(struct Palette *pal, struct IFFHandle *handle);
BOOL LoadCMAP(struct Project *pj, struct IFFHandle *handle, BOOL usepalette);
BOOL LoadBlocks(struct Project *pj, struct IFFHandle *handle, char *name, BOOL usepalette);
BOOL SaveBlocks(char *name, struct Project *pj, struct IFFHandle *handle);
BOOL LoadAnimBrushAs(struct Window *win, struct AnimBrush *ab);
BOOL LoadAnimBrush(struct AnimBrush *ab, char *file, char *drawer);

/**** Screens.c ****/
LONG		GetScreenLocks(struct Screen *scr);
BOOL		SetupScreen(struct ScreenConfig *scrconfig);
BOOL		OpenNewScreen(struct ScreenConfig *scrconfig);
BOOL		BorrowScreen(struct ScreenConfig *scrconfig);
BOOL		CloseOurScreen(struct ScreenConfig *scrconfig);
struct PubScreenNode *GetPubScreen(char *name);
BOOL		UseNewScreenSettings(struct ScreenConfig *scrconfig);
BOOL		RemakeScreen(void);
void SetupDefaultScreenConfig(struct ScreenConfig *scrconfig);

/**** ColourStuff.c ****/
void InstallPalette(struct Screen *scr, struct Palette *pal);
UWORD FindnBestPen( struct Palette *pal, UWORD numpens, UBYTE r, UBYTE g, UBYTE b );
void SussDrawInfoPens( UWORD *pens, struct Palette *pal, UWORD numcols );
void CreateDefaultPalette(void);
BOOL SussOutCMap(UWORD numofcolours,BOOL cmapok,UBYTE *colours,
								struct Palette *pal,struct Screen *scr);
//void UsePalette(struct Palette *pal);
void InstallNewGlobPalette(struct Palette *pal);
BOOL PaletteSame(struct Palette *pal1, struct Palette *pal2);

/**** Tools.c  ****/
UWORD DominantColour( UWORD buttonstate );
void DrawSelector(struct ProjectWindow *pw, WORD x0, WORD y0);
void EraseSelector(struct ProjectWindow *pw);
void Tool_Down(struct ProjectWindow *pw, WORD x, WORD y, WORD button);
void Tool_Up(struct ProjectWindow *pw, WORD x, WORD y, WORD button);
void Tool_Move(struct ProjectWindow *pw, WORD x, WORD y);
void Tool_Cancel(struct ProjectWindow *pw, BOOL refresh);
void NewTool(struct ProjectWindow *pw,UWORD tool);
BOOL SussOutAltPaging(struct Project *pj);

/**** PlotRoutines.c ****/
void PlotPixel(struct ProjectWindow *pw,WORD x,WORD y);
BOOL SmartPlotPixel(struct ProjectWindow *pw,WORD x,WORD y);
BOOL SmartPlotBrush(struct ProjectWindow *pw, WORD x, WORD y, struct Rectangle *rec, UWORD col, UWORD mode);
void __asm ExtraSmartPlotPixel(register __a0 struct Project *pj,
															 register __d0 WORD x,
															 register __d1 WORD y,
															 register __a1 UWORD col);
void ExtraSmartPlotBrush(struct Project *pj, WORD x, WORD y, UWORD col, UWORD mode);
void QuickPlotPixel(struct ProjectWindow *pw,struct Window *win,WORD x, WORD y);
BOOL QuickSmartPlotBrush(struct ProjectWindow *pw, WORD x, WORD y,
												 struct Rectangle *rec, BOOL premagnified, UWORD col, UWORD mode);
void DoExtraSmartColLine( struct ProjectWindow *pw, struct Rectangle *rec,int x1, int y1, int x2, int y2, UWORD col, UWORD mode );
void DoSmartColLine ( struct ProjectWindow *pw, struct Rectangle *rec, int x1, int y1, int x2, int y2, UWORD col, UWORD mode);
void DoInvHorizLine( struct ProjectWindow *pw, int x1, int x2, int y2 );
void DoInvVertLine( struct ProjectWindow *pw, int x1, int y1, int y2 );
void DoColHorizLine( struct ProjectWindow *pw, int x1, int x2, int y2, UWORD col );
void DoColVertLine( struct ProjectWindow *pw, int x1, int y1, int y2, UWORD col );
void DoInvCrossHairs(struct ProjectWindow *pw, int x, int y);
void MyBltMaskBitMap(struct BitMap *srcbitmap,
								WORD srcx, WORD srcy,
								struct BitMap *destbitmap,
								WORD destx, WORD desty,
								WORD sizex, WORD sizey,
								UWORD minterm,
								PLANEPTR bltmask);
void DoInvBox(struct ProjectWindow *pw, WORD  boxx,			WORD  boxy,
																				WORD boxwidth, WORD boxheight);
void DoFloodFill(struct Project *pj, struct Rectangle *rec, WORD x, WORD y, UWORD col );
void DoSmartColOBox(struct ProjectWindow *pw, struct Rectangle *rec,
										UWORD minx, UWORD miny, UWORD maxx, UWORD maxy,
										UWORD col, UWORD mode);
void DoExtraSmartColOBox(struct ProjectWindow *pw, struct Rectangle *rec,
												 UWORD minx, UWORD miny, UWORD maxx, UWORD maxy,
												 UWORD col, UWORD mode);
void DoSmartColSBox(struct ProjectWindow *pw, struct Rectangle *rec,
										UWORD minx, UWORD miny, UWORD maxx, UWORD maxy,
										UWORD col);
void DoExtraSmartColSBox(struct ProjectWindow *pw, struct Rectangle *rec,
										WORD minx, WORD miny, WORD maxx, WORD maxy,
										UWORD col);
UWORD SmoothPoint(struct Project *pj, WORD x, WORD y);
void RefreshWindowSection(struct ProjectWindow *pw,	WORD minx, WORD miny, WORD width, WORD height);
void ShadePoint(struct Project *pj, WORD x, WORD y, BOOL shadeup);
void SmartOEllipse( struct ProjectWindow *pw, int xo, int yo, int a, int b, struct Rectangle *rec, UWORD col, UWORD mode);
void ExtraSmartOEllipse( struct Project *pj, int xo, int yo, int a, int b, struct Rectangle *rec, UWORD col, UWORD mode);
void SmartSEllipse( struct ProjectWindow *pw, int xo, int yo, int a, int b, struct Rectangle *rec, UWORD col, UWORD mode);
void ExtraSmartSEllipse( struct Project *pj, int xo, int yo, int a, int b, struct Rectangle *rec, UWORD col, UWORD mode);

/**** Brush.c  ****/
BOOL GetCustomBrushImage(struct Brush *br, struct BitMap *srcbm,
												 WORD x, WORD y, WORD width, WORD height);
void FreeBrushBitMaps(struct Brush *br);
BOOL DrawCustomBrush(struct BrushInfo *bi,struct ProjectWindow *pw, WORD brushleft, WORD brushtop,
										 struct Rectangle *rec, BOOL premagnified);
BOOL WipeWithCustomBrush(struct BrushInfo *bi,struct ProjectWindow *pw,
												 WORD brushleft, WORD brushtop,
												 struct Rectangle *rec, BOOL premagnified, UWORD col);
BOOL ReplaceWithCustomBrush(struct BrushInfo *bi,struct ProjectWindow *pw, WORD brushleft, WORD brushtop,
										 struct Rectangle *rec, BOOL premagnified, UWORD col);
BOOL SolidWithCustomBrush(struct BrushInfo *bi,struct ProjectWindow *pw, WORD brushleft, WORD brushtop,
										 struct Rectangle *rec, UWORD col);
BOOL SetupDefaultBrushes(void);
BOOL MagnifyBrushIntoWorkSpace(struct BrushInfo *bi,UWORD pixelsize,UWORD col, UWORD mode);
void BltBrushToMainBitMap(struct Project *pj, struct BrushInfo *bi, WORD brushleft, WORD brushtop, UWORD col, UWORD mode);
void OutlineBrush(struct BrushInfo *bi, UWORD col,
									 struct BitMap *dstbm, struct BitMap *dstmaskbm );
void OutlineCurrentBrush(UWORD col);
BOOL RestoreBrush(struct Brush *br);
BOOL FlipBrushVertically(struct BrushInfo *bi);
void FlipBitMapVertically(struct BitMap *srcbm, UWORD width, UWORD height, UWORD depth);
BOOL FlipBrushHorizontally(struct BrushInfo *bi);
void FlipBitMapHorizontally(struct BitMap *srcbm, struct BitMap *dstbm,
														UWORD width, UWORD height, UWORD depth);
BOOL RotateBrush90(struct BrushInfo *bi);
void RotateBitMap90(struct BitMap *srcbm, struct BitMap *dstbm,
									UWORD srcwidth, UWORD srcheight, UWORD depth);
void NastyMakeColourMask(struct BitMap *srcbitmap,struct BitMap *destbitmap,
												 UWORD width, UWORD height, UWORD col);
void RotateBrushHandle(struct Brush *br);
void HalveBrush(struct BrushInfo *bi, struct Palette *pal);
BOOL RotateBrush(struct BrushInfo *bi, double theta);

/**** AnimBrush.c ****/
BOOL GUIGetAnimBrushFrames(struct AnimBrush *ab, struct ProjectWindow *pw,
													 WORD minx, WORD miny, WORD width, WORD height, UWORD button);
void FreeAnimBrush(struct AnimBrush *ab);
void FreeABrushBitMaps(struct AnimBrush *ab, struct ABrush *abr);
void InitAnimBrush(struct AnimBrush *ab);
BOOL GetAnimBrushFrames(struct AnimBrush *ab, struct Project *pj,
												UWORD numofframes, WORD left, WORD top, WORD width, WORD height, UWORD button);
void SetAnimBrushFrame(struct AnimBrush *ab, struct ABrush *abr);
void GotoNextAnimBrushFrame(struct AnimBrush *ab);
struct ABrush *AllocABrush(UWORD width, UWORD height, UWORD depth);


/**** RastPorts.c ****/
struct RastPort *AllocBitMapRPort( int width, int height, int depth );
void FreeBitMapRPort( struct RastPort *rp, int width, int height, int depth  );

/**** SubTask.c ****/
void WaitForSubTaskDone(struct MsgPort *mp);
void SubTaskStart(void);
//void SubTaskHandler(void);
void SortOutProjMenuVerify(struct ProjectWindow *pw,struct IntuiMessage *imsg);
BOOL FilterSubTaskMessages(struct IntuiMessage *msg);
void ReplySubTaskMsg(struct Message *msg);

/**** Bobs.c ****/
BOOL TestRow(struct BitMap *bm,UWORD x, UWORD y, UWORD width);
BOOL TestColumn(struct BitMap *bm, UWORD x, UWORD y, UWORD height);
BOOL TestColumnWord(struct BitMap *bm, UWORD x, UWORD y, UWORD height);
BOOL InitBobMode(struct Project *pj, UWORD newdepth);
struct MyBob *AllocBob(struct Project *pj, struct List *boblist);
struct MyBob *AllocNextBob(struct Project *pj, struct List *boblist);
void AppendBobToList(struct List *boblist, struct MyBob *bob);
void FreeBobBitMaps(struct MyBob *bob);
void EditBob(struct Project *pj,struct MyBob *bob);
void GotoPreviousBob(struct Project *pj);
void GotoNextBob(struct Project *pj);
void GotoNewBob(struct Project *pj, struct MyBob *bob);
BOOL StoreBobImage(struct Project *pj, struct MyBob *bob);
BOOL StoreBobMask(struct Project *pj, struct MyBob *bob);
BOOL StoreBob(struct Project *pj, struct MyBob *bob);
void TurnOnBobMaskEditing(struct Project *pj);
void TurnOnBobImageEditing(struct Project *pj);
BOOL SaveBobs(char *name, struct Project *pj, struct IFFHandle *handle);
BOOL LoadBobs(struct Project *pj, struct IFFHandle *handle, char *name);
void UpdateBobNameGadgets(struct Project *pj, struct MyBob *bob);
struct MyBob *FindBobByNumber(struct List *boblist,UWORD number);
BOOL GUIDeleteBobs( struct Project *pj, struct Window *parentwin );
BOOL DeleteBobs(struct Project *pj, UWORD first, UWORD last);
BOOL GUIInsertBobs(struct Project *pj, struct Window *parentwin);
BOOL InsertBobs(struct Project *pj, UWORD numofframes, UWORD beforeframe);
BOOL GUICopyBobs(struct Project *pj, struct Window *parentwin);
BOOL CopyCurrentBobToRange(struct Project *pj, UWORD first, UWORD last);
BOOL CopyRangeBeforeBob(struct Project *pj, UWORD first, UWORD last, UWORD before);
void DeleteMask(struct Project *pj, struct MyBob *bob);
void RemakeMask(struct Project *pj, struct MyBob *bob);
BOOL BobCollisionZoneOK(struct Project *pj);
void CreateDefaultHitZone(struct MyBob *bob);
void SetBobName(struct Project *pj, struct MyBob *bob, char *name);

/**** Sprites.c ****/
BOOL InitSpriteMode(struct Project *pj, UWORD newdepth);
void GotoPreviousSprite(struct Project *pj);
void GotoNextSprite(struct Project *pj);
void GotoNewSprite(struct Project *pj, struct MyBob *bob);
BOOL StoreSprite(struct Project *pj, struct MyBob *bob);
BOOL SaveSprites(char *name, struct Project *pj, struct IFFHandle *handle);
BOOL LoadSprites(struct Project *pj, struct IFFHandle *handle, char *name);

/* Shapes.c */
BOOL LoadShapesFile(char *drawer, char *file);

/* HoopyVision.c */
ULONG HO_FindMinHeight( struct HOStuff *hos, struct HoopyObject *hob, struct TextFont *tf );
ULONG HO_FindMinWidth( struct HOStuff *hos,	struct HoopyObject *hob, struct TextFont *tf );
ULONG	HO_FindMinWidth( struct HOStuff *hos,
	struct HoopyObject *hob, struct TextFont *tf );
ULONG	HO_FindOptimalHeight( struct HOStuff *hos,
	struct HoopyObject *hob, struct TextFont *tf );
BOOL HO_MakeLayout( struct HoopyObject *hob, struct HOStuff *hos,
	WORD left, WORD top, WORD width, WORD height );
struct HOStuff *HO_GetHOStuff( struct Window *win, struct TextFont *tf );
void HO_FreeHOStuff( struct HOStuff *hos );
void HO_KillLayout( struct HOStuff *hos );
void HO_ReadGadgetStates( struct HOStuff *hos, struct HoopyObject *hob );
void HO_CheckIMsg( struct IntuiMessage *imsg );
void HO_DisableObject( struct HoopyObject *hob, struct HOStuff *hos, BOOL state );
void HO_RefreshObject( struct HoopyObject *hob, struct HOStuff *hos );
BOOL HO_SussWindowSizing( struct HOStuff *hos, struct HoopyObject *hob );
UWORD HO_DoReq( struct HoopyObject *hob, struct Window *win );
void FixedPointToString( LONG fixnum, char *buf );
BOOL StrToFixedPoint( LONG *fixnum, char *str );
void HO_ActivateNextStringGad( struct Window *win, struct HoopyObject *hob );

/**** MiscAsm.a ****/
void __asm MapImages(register __a0 struct MyIMStruct *ms);
void __asm MakeToolsGadgets(register __a0 struct ToolsWindowGadList *tgl);
UBYTE __asm UncompILBM(register __a0 struct BitMapHeader *bmh,
											 register __a1 struct ContextNode *body,
											 register __d0 struct BitMap *bm,
											 register __d1 struct BitMap *mskbm,
											 register __d2 struct IFFHandle *iff);

UWORD __asm GetPixelColour(register __a0 struct BitMap *bm,
													register __d0 UWORD XOffset,
													register __d1 UWORD YOffset);

void __asm PutPixelColour(register __a0 struct BitMap *bm,
													register __d0 UWORD XOffset,
													register __d1 UWORD YOffset,
													register __d2 UWORD Colour);

void __asm MagnifyBitMap(register __a0 struct Magnify *mg);

void __asm QuickMagnifyBitMap(register __a0 struct QuickMagnify *qm);

void __asm MyBltBitMapRastPort(	register __a0 struct BitMap *srcBitMap,
																register __d0 UWORD xSrc,
																register __d1 UWORD ySrc,
																register __a1 struct RastPort *destRP,
																register __d2 UWORD xDest,
																register __d3 UWORD yDest,
																register __d4 UWORD xSize,
																register __d5 UWORD ySize,
																register __d6 UWORD minterm);

void __asm MyBltBitMap(	register __a0 struct BitMap *srcBitMap,
												register __d0 UWORD xSrc,
												register __d1 UWORD ySrc,
												register __a1 struct BitMap *destBitMap,
												register __d2 UWORD xDest,
												register __d3 UWORD yDest,
												register __d4 UWORD xSize,
												register __d5 UWORD ySize,
												register __d6 UWORD minterm,
												register __d7 UWORD mask,
												register __a2 APTR	tempA);

UWORD __asm RecFloodFill(register __a0 struct Project *pj,
												register __a1 struct Rectangle *rec,
												register __d0 WORD X,
												register __d1 WORD Y,
												register __d2 UWORD col,
												register __d3 UWORD backcol);

void __asm SubTaskStart(void);

VOID __asm UnpackASCII( register __a0 UBYTE *source,
												register __a1 UBYTE *dest,
												register __d0 UWORD srcsize );

VOID __asm PackASCII( register __a0 UBYTE *source,
											register __a1 UBYTE *dest,
											register __d0 UWORD numchars );

BOOL __asm MakePackable( register __a0 UBYTE *str );
BOOL __asm IsPackable( register __d0 UBYTE c );

/* void __asm MyBltMaskBitMap(	register __a0 struct BitMap *srcBitMap,
														register __d0	WORD xSrc,
														register __d1 WORD ySrc,
														register __a1 struct BitMap *destBitMap,
														register __d2 WORD xDest,
														register __d3 WORD yDest,
														register __d4 WORD xSize,
														register __d5 WORD ySize,
														register __d6 UWORD minterm,
														register __a2 PLANEPTR bltMask);
 */
