
/* main.c */
struct Library *MyOpenLibrary( STRPTR libname, LONG version );
void CloseWindowSafely(struct Window *win);
void StripIntuiMessages(struct MsgPort *mp, struct Window *win );
VOID MakeUpDefaultZonkConfig( struct ZonkConfig *zcfg );
BOOL InstallZonkConfig( struct ZonkConfig *zcfg );

/* Screens.c */
BOOL InstallScreenConfig( struct ScreenConfig *scfg );
BOOL UninstallScreenConfig( void );
struct Screen *LockGFXScreen( void );
void UnlockGFXScreen( struct Screen *scr );
struct Screen *LockMainScreen( void );
void UnlockMainScreen( struct Screen *scr );

/* SuperWindowStuff.c */
APTR BlockWin( struct Window *win );
VOID UnblockWin( APTR blockwinhandle );
void SetWait(struct Window *win);
void ClrWait(struct Window *win);
BOOL BlockAllSuperWindows( VOID );
VOID UnblockAllSuperWindows( VOID );
void RememberWindow( struct SuperWindow *sw );
void UpdateWinDef( struct SuperWindow *sw );
struct SuperWindow *FindSuperWindowByType( struct SuperWindow *sw, UWORD type );
struct List *ListAndHideWindows( VOID );
VOID ShowHiddenWindows( struct List *winlist );
void RefreshSuperWindowTitles( void );
BOOL IsOKToClose( struct SuperWindow *sw );
struct SuperWindow *FensterSnarf( void );

/* PROGWindow.c */
BOOL OpenNewPROGWindow( struct Chunk *cnk );

/* FormationWindow.c */
BOOL OpenNewFormationWindow( struct Chunk *cnk );

/* ScreenConfigWindow.c */
BOOL OpenNewScreenConfigWindow( void );

/* WeaponWindow.c */
BOOL OpenNewWeaponWindow( struct Chunk *cnk );

/* TimedWaveWindow.c */
BOOL OpenNewTimedWaveWindow( struct Chunk *cnk );

/* SFXWindow.c */
BOOL OpenNewSFXWindow( struct Chunk *cnk );

/* SWAVWindow.c */
BOOL OpenNewSWaveWindow( struct Chunk *cnk );
void RethinkSWaveBobImages( void );
void RethinkSWaveBackgrounds( void );

/* THAPWindow.c */
BOOL OpenNewTHAPWindow( struct Chunk *cnk );
void RethinkTHAPBackgrounds( void );

/* GAMEWindow.c */
BOOL OpenNewGAMEWindow( struct Chunk *cnk );

/* DISPWindow.c */
BOOL OpenNewDISPWindow( struct Chunk *cnk );

/* EOTRWindow.c */
BOOL OpenNewEOTRWindow( struct Chunk *cnk );

/* ChunkWindow.c */
BOOL OpenNewChunkWindow( VOID );
VOID RefreshChunkListings( VOID );
void LoadChunks( struct SuperWindow *sw );
void NewChunk( struct SuperWindow *sw );

/* Files.c */
void LoadFile( STRPTR drawer, STRPTR file );
BOOL SaveFile( STRPTR drawer, STRPTR file, struct Chunk **cnkarray, UWORD count );
LONG __saveds __asm MyBufferedDOSHook( register __a0 struct Hook *hook,
	register __a2 struct IFFHandle *iff,
	register __a1 struct IFFStreamCmd *actionpkt );
WORD IDFile( STRPTR filename );
BOOL LoadPalette( STRPTR filename, struct Palette *palette );

/* ZonkMisc.c */
void ClearWindow( struct Window *win );
WORD MyTextLength( struct TextFont *tf, char *str );
UWORD FindMaxStrLen( STRPTR *str, struct TextFont *tf );
UWORD CountStrings( STRPTR *str );
void ComposeScreenModeString( STRPTR str, ULONG displayid, UWORD depth );
char *Mystrncpy(char *to, char *from, int length);
char *MyStrDup(char *source);
ULONG CountNodes( struct List *l );
struct Node *FindNthNode(struct List *list, UWORD num);
BOOL BubbleUp( struct List *glub, struct Node *noddy );
BOOL BubbleDown( struct List *glub, struct Node *noddy );
char *MyStrToUpper( char *str );
VOID FreeNodes( struct List *l );
struct BitMap *CreateBitMap(UWORD width, UWORD height, UWORD depth,
	BOOL interleaved);
void DestroyBitMap(struct BitMap *bm, UWORD width, UWORD height, UWORD depth);
VOID WritePixelClipped( struct RastPort *rp, LONG x, LONG y,
	struct Rectangle *cliprect );
VOID RectFillClipped( struct RastPort *rp, LONG minx, LONG miny,
	LONG maxx, LONG maxy,	struct Rectangle *cliprect );
VOID DrawLineClipped( struct RastPort *rp, LONG x0, LONG y0,
	LONG x1, LONG y1,	struct Rectangle *cliprect );
UWORD FindNodeNum( struct List *list, struct Node *noddy );
BOOL MakeBackupFile( STRPTR filename );
VOID UnclipWindow( struct Window *win );
struct Region *ClipWindow( struct Window *win, LONG minx, LONG miny,
	LONG maxx, LONG maxy );
BOOL AppendToDataList( struct List *dnlist, struct List *srclist );
void AlphaAdd( struct List *list, struct Node *noddy );
void DumpList( struct List *l );

/* Hoopy.c */

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
void HO_ActivateNextStringGad( struct Window *win, struct HoopyObject *hob );

BOOL StrToFixedPoint( LONG *fixnum, char *str );
void FixedPointToString( LONG fixnum, char *buf );

/* ChunkStuff.c */

VOID InitChunkStorage( VOID );
struct Chunk *CreateGenericChunk( struct Filetracker *ft, ULONG typeid );
VOID FreeGenericChunk( struct Chunk *cnk );
struct Chunk *GenericChunkLoader( struct IFFHandle *iff, ULONG size,
	struct Filetracker *ft, ULONG typeid );
BOOL GenericChunkSaver( struct Chunk *cnk, struct IFFHandle *iff );
VOID FreeAllChunks( VOID );
VOID RemoveChunkFromParentFile( struct Chunk *cnk );
struct Chunk *CreateCMAPChunk( struct Filetracker *ft );
VOID FreeCMAPChunk( struct Chunk *cnk );
BOOL CMAPSaver( struct Chunk *cnk, struct IFFHandle *iff );
struct Chunk *CMAPLoader( struct IFFHandle *iff, ULONG size,
	struct Filetracker *ft );
struct Chunk *FindChunkType( struct Chunk *cnk, ULONG typeid );
void LockChunk( struct Chunk *cnk );
VOID UnlockChunk( struct Chunk *cnk );
void ModifyChunk(struct Chunk *cnk );

/* PROGStuff.c */
struct Chunk *CreatePROGChunk( struct Filetracker *ft );
VOID FreePROGChunk( struct Chunk *cnk );
struct Chunk *PROGLoader( struct IFFHandle *iff, ULONG size,
	struct Filetracker *ft );
BOOL PROGSaver( struct Chunk *cnk, struct IFFHandle *iff );
BOOL AddNewProgram( struct Chunk *cnk, char *name );
VOID FreeProgram( struct Program *prog );
ULONG CalcProgramDiskSize( struct Program *prog );
ULONG CalcProgramLameSize( struct Program *prog );
BOOL AppendToProgramList( struct Chunk *cnk, struct List *proglist );
struct Program *CopyProgram( struct Program *srcprog );
struct Program *GlobalFindProg( STRPTR progname );
UBYTE *FindItemInProgram( struct Program *prog, UBYTE itemtype );
struct BobInfoNode *FindImageFromProgram( struct Program *prog,
	LONG *offsetbuffer, BOOL enterprogs );
void CheckProgram( struct Program *prog );

/* DFRMStuff.c */
struct Chunk *CreateDFRMChunk( struct Filetracker *ft );
VOID FreeDFRMChunk( struct Chunk *cnk );
struct Chunk *DFRMLoader( struct IFFHandle *iff, ULONG size,
	struct Filetracker *ft );
BOOL DFRMSaver( struct Chunk *cnk, struct IFFHandle *iff );
struct Formation *CreateFormation( UBYTE *name );
VOID FreeFormation( struct Formation *fm );
struct Formation *LoadFormation( struct IFFHandle *iff );

/* WEAPStuff.c */
struct Chunk *CreateWEAPChunk( struct Filetracker *ft );
VOID FreeWEAPChunk( struct Chunk *cnk );
struct Chunk *WEAPLoader( struct IFFHandle *iff, ULONG size,
	struct Filetracker *ft );
BOOL WEAPSaver( struct Chunk *cnk, struct IFFHandle *iff );
struct WeaponDef *RequestWeapon( struct Window *parentwin, 	STRPTR wintitle,
	struct Chunk *cnk );

/* TWAVStuff.c */
struct Chunk *CreateTWAVChunk( struct Filetracker *ft );
VOID FreeTWAVChunk( struct Chunk *cnk );
struct Chunk *TWAVLoader( struct IFFHandle *iff, ULONG size,
	struct Filetracker *ft );
BOOL TWAVSaver( struct Chunk *cnk, struct IFFHandle *iff );
VOID EnqueueTimedWave( struct List *twavelist,
	struct TimedWave *twave );

/* SFXStuff.c */
struct Chunk *CreateSFXChunk( struct Filetracker *ft );
VOID FreeSFXChunk( struct Chunk *cnk );
struct Chunk *SFXLoader( struct IFFHandle *iff, ULONG size,
	struct Filetracker *ft );
BOOL SFXSaver( struct Chunk *cnk, struct IFFHandle *iff );

/* GAMEStuff.c */
struct Chunk *CreateGAMEChunk( struct Filetracker *ft );
VOID FreeGAMEChunk( struct Chunk *cnk );
struct Chunk *GAMELoader( struct IFFHandle *iff, ULONG size,
	struct Filetracker *ft );
BOOL GAMESaver( struct Chunk *cnk, struct IFFHandle *iff );

/* THAPStuff.c */
struct Chunk *CreateTHAPChunk( struct Filetracker *ft );
VOID FreeTHAPChunk( struct Chunk *cnk );
struct Chunk *THAPLoader( struct IFFHandle *iff, ULONG size,
	struct Filetracker *ft );
BOOL THAPSaver( struct Chunk *cnk, struct IFFHandle *iff );
struct Path *CreatePath( BOOL headnode, WORD x, WORD y );
VOID FreePath( struct Path *path );

/* MAPStuff */
struct Chunk *CreateMAPChunk( struct Filetracker *ft );
VOID FreeMAPChunk( struct Chunk *cnk );
BOOL MAPSaver( struct Chunk *cnk, struct IFFHandle *iff );
struct Chunk *MAPLoader( struct IFFHandle *iff, ULONG size,
	struct Filetracker *ft );
VOID DrawMapBM( struct DrawMapBMArgs *dmargs );
void ScrollMapBM( struct DrawMapBMArgs *dmargs, WORD dx, WORD dy,
	UWORD qual );
struct Chunk *FindMAPByName( UBYTE *name );

/* BLKSStuff */
struct Chunk *CreateBLKSChunk( struct Filetracker *ft );
VOID FreeBLKSChunk( struct Chunk *cnk );
BOOL BLKSSaver( struct Chunk *cnk, struct IFFHandle *iff );
struct Chunk *BLKSLoader( struct IFFHandle *iff, ULONG size,
	struct Filetracker *ft );
void UnclippedDrawBlockRP( struct RastPort *rp, WORD x, WORD y,
	struct Blockset *bs, UWORD blk );
struct Chunk *FindBLKSByName( UBYTE *name );

/* DISPStuff */
struct Chunk *CreateDISPChunk( struct Filetracker *ft );
VOID FreeDISPChunk( struct Chunk *cnk );
BOOL DISPSaver( struct Chunk *cnk, struct IFFHandle *iff );
struct Chunk *DISPLoader( struct IFFHandle *iff, ULONG size,
	struct Filetracker *ft );

/* EOTRStuff */
struct Chunk *CreateEOTRChunk( struct Filetracker *ft );
VOID FreeEOTRChunk( struct Chunk *cnk );
BOOL EOTRSaver( struct Chunk *cnk, struct IFFHandle *iff );
struct Chunk *EOTRLoader( struct IFFHandle *iff, ULONG size,
	struct Filetracker *ft );

/* BOBSStuff.c */
struct Chunk *CreateBOBSChunk( struct Filetracker *ft );
VOID FreeBOBSChunk( struct Chunk *cnk );
struct Chunk *BOBSLoader( struct IFFHandle *iff, ULONG size,
	struct Filetracker *ft );
BOOL BOBSSaver( struct Chunk *cnk, struct IFFHandle *iff );
struct BobInfoNode *RequestBob( struct Window *parentwin,	UBYTE *wintitle,
	struct Chunk *cnk );
struct BobInfoNode *FindBob( STRPTR bobname );
VOID BltBobRastPort( struct RastPort *rp, struct BobInfoNode *bin,
	WORD xpos, WORD ypos );
struct Chunk *CreateANIMChunk( struct Filetracker *ft );
VOID FreeANIMChunk( struct Chunk *cnk );
struct Chunk *ANIMLoader( struct IFFHandle *iff, ULONG size,
	struct Filetracker *ft );
BOOL ANIMSaver( struct Chunk *cnk, struct IFFHandle *iff );
struct AnimInfoNode *FindAnim( STRPTR animname );
struct BobInfoNode *AnimFirstFrame( struct AnimInfoNode *ain, LONG *offsetbuffer );
struct AnimInfoNode *RequestAnim( struct Window *parentwin,	UBYTE *wintitle,
	struct Chunk *cnk );
struct Chunk *CreateSPRTChunk( struct Filetracker *ft );
VOID FreeSPRTChunk( struct Chunk *cnk );
struct Chunk *SPRTLoader( struct IFFHandle *iff, ULONG size,
	struct Filetracker *ft );
BOOL SPRTSaver( struct Chunk *cnk, struct IFFHandle *iff );


/* SWAVStuff.c */
struct Chunk *CreateSWAVChunk( struct Filetracker *ft );
VOID FreeSWAVChunk( struct Chunk *cnk );
struct Chunk *SWAVLoader( struct IFFHandle *iff, ULONG size,
	struct Filetracker *ft );
BOOL SWAVSaver( struct Chunk *cnk, struct IFFHandle *iff );
struct ZScrollWave *AllocZScrollWave( VOID );
VOID FreeZScrollWave( struct ZScrollWave *zsw );



/* Wibble.s */

VOID __asm UnpackASCII( register __a0 UBYTE *source, register __a1 UBYTE *dest,
	register __d0 UWORD srcsize );
VOID __asm PackASCII( register __a0 UBYTE *source, register __a1 UBYTE *dest,
	register __d0 UWORD numchars );
BOOL __asm IsPackable( register __d0 UBYTE c );
BOOL __asm MakePackable( register __a0 UBYTE *str );

/* operatorstuff.c */

BOOL EditOperator( struct Window *parentwin, UBYTE *opdata );
VOID FreeOperatorData( UBYTE *opdata );
ULONG CalcOpSize( UWORD op );
//BOOL InsertOperator( struct Program *prog, UWORD op, WORD pos );
struct OpNode *AllocOperator( UWORD op );
ULONG CalcOpDiskSize( UBYTE *opdata );
ULONG CalcOpLameSize( UBYTE *opdata );
BOOL WriteOperator( struct IFFHandle *iff, UBYTE *opdata );
UBYTE *CopyOperator( UBYTE *opdatbase );
#ifndef NDEBUG
VOID VerifyOpParamSize( VOID );
#endif
UBYTE *FindItemInOperator( UBYTE *opdata, UBYTE itemtype );
struct BobInfoNode *FindImageFromOperator( UBYTE *opdata, LONG *offsetbuffer,
	BOOL enterprogs );
VOID DescribeOperator( STRPTR str, UBYTE *opdata );
BOOL MakeOperatorList( struct Program *prog, struct List *oplist );
VOID FreeOperatorList( struct List *oplist );
UBYTE *CookOperator( UBYTE **opdata );

/* requesters.c */

BOOL MyStringRequest( struct Window *parentwin, char *winname, char *gadname,
	UBYTE *buffer, UWORD maxchars );
BOOL MyPackedStringRequest( struct Window *parentwin, char *winname, char *gadname,
	UBYTE *buffer, UWORD maxchars );
BOOL MyIntRequest( struct Window *parentwin, char *winname, char *gadname,
	LONG *value );
UWORD MyOperatorRequest( struct Window *parentwin, UWORD operatortype );
UWORD MyListViewRequest( struct Window *parentwin, UBYTE *wintitle,
	UBYTE *gadtitle, struct List *labels );
struct Formation *MyFormationRequest( struct Window *win, struct Chunk *cnk );
ULONG RequestChunktype( struct Window *parentwin, UBYTE *wintitle );
struct Program *RequestProgram( struct Window *parentwin, struct Chunk *cnk,
	UBYTE *wintitle, UBYTE *gadtitle );
struct Chunk *RequestChunk( struct Window *parentwin, UBYTE *wintitle,
	UBYTE *gadtitle, ULONG typeid, ULONG *typeidarray );
WORD __stdargs PlebReq( struct Window *win, STRPTR title, STRPTR text,
	STRPTR gadgets,...);

/* OpReq.c */
UWORD OpReq( struct Window *parentwin, UWORD operatortype );


/* MenuStuff.c */
BOOL HandleProjectMenu( struct SuperWindow *sw, ULONG menuid );
BOOL HandleWindowMenu( struct SuperWindow *sw, ULONG menuid );
BOOL HandleSettingsMenu( struct SuperWindow *sw, ULONG menuid );
BOOL GenericMakeMenus( struct SuperWindow *sw, struct NewMenu *custommenu );
VOID GenericZapMenus( struct SuperWindow *sw );
struct MenuItem *FindMenuItem( struct Menu *startmenu, ULONG menuid );
VOID GenericHandleMenuPick( struct SuperWindow *sw, UWORD menunum,
	BOOL (*customhandler)( struct SuperWindow *sw, ULONG id ) );
VOID SetGlobalOption( ULONG menuid, BOOL state );

/* SFXHandler.c */
//BOOL InitSFXHandler( VOID );
//VOID KillSFXHandler( VOID );
//VOID PlaySound( struct SFXHeader *sfx, UWORD chan );

/* ZCFGStuff.c */
BOOL ZCFGSaver( struct Chunk *cnk, struct IFFHandle *iff );
VOID FreeZCFGChunk( struct Chunk *cnk );
struct Chunk *CreateZCFGChunk( struct Filetracker *ft );
struct Chunk *ZCFGLoader( struct IFFHandle *iff, ULONG size,
	struct Filetracker *ft );
BOOL SaveZonkConfig( STRPTR filename, struct ZonkConfig *zcfg );
BOOL LoadZonkConfig( STRPTR filename, struct ZonkConfig *zcfg );
BOOL GUISaveZonkConfigAs( struct ZonkConfig *zcfg );
BOOL GUILoadZonkConfig( struct ZonkConfig *zcfg );

/* VarStuff.c */
int FindVariable( STRPTR namefrag );
void ListVars( struct List *list, UBYTE type );
WORD RequestVar( struct Window *win );


/* from debug.lib */
LONG KPrintF( STRPTR fmt, ... );




