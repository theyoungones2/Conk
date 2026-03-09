/***************************************/
//PROTOS
/****************************************/

#include	<devices/inputevent.h>

LONG KPrintF( STRPTR fmt, ... );

/* Ponk Protos */
UWORD __asm stricmp( register __a0 char *str1, register __a1 char *str2);
void __asm strcpy( register __a0 char *dest, register __a1 char *source);
ULONG __asm GetTextLength( register __a0 char *string);
VOID __asm UnpackASCII( register __a0 UBYTE *source, register __a1 UBYTE *dest, register __d0 UWORD srcsize );
VOID __asm PackASCII( register __a0 UBYTE *source, register __a1 UBYTE *dest, register __d0 UWORD numchars );
void __asm ActionStart( register __a0 struct LameReq *lr);
void __asm WaitVBlank(void);
void __asm WaitPastVBlank(void);
void __asm KillSprites(void);
void __asm AnimCopperIntHandler(void);
UWORD __asm JoystickDecode( register __d0 UWORD port);
void __asm GC_InitGroovyConStruct(register __a0 struct GroovyCon *gc);
void __asm PrintInsertVolumeText(register __a0 struct GroovyConsoleDevice *, register __a1 char *fmtstring, register __a2 char *volumename);

/* ModulePlayer Protos */
void __asm InitCIAInterrupt(void);
void __asm StopCIAInterrupt(void);
void __asm InitModulePlayer( register __a0 void *modulebase, register __a1 void *ModuleEndAddress );
void __asm StopModulePlayer(void);
BOOL __asm StartPlayerSubTask(void);
void __asm EndPlayerSubTask(void);
void __asm PlaySample( register __a0 struct SpamParam *spp,
	register __a1 UBYTE *sampledata, register __d0 UWORD chan );

/* Global Lame Functions */
BOOL __asm InitLame(void);
void __asm KillLame(void);
UWORD __asm LoadFiles( register __a0 struct LameReq *lr, register __a1 struct LevelNode *ln);
void __asm FreeFiles( void );
void __asm FlushFiles( void );

/* Private Lame Functions  */
UWORD CookShieldBarImage(struct LameReq *lr);
void FreeCookedShieldBarImage(void);
BOOL SussOutVolumes(char *fullpathname);
BOOL InsertVolumeRequester(char *volumename);
void FreeExtraCopperWaits(void);
UWORD AllocInterruptCopperWait(struct LameReq *lr);
void ExamineFiles( void );
void CookChunks( struct LameReq *lr );
void SetupReturns( struct LameReq *lr );
UWORD LoadRawFile(BPTR fh, struct FileNode *fn);
UWORD GenericReadChunk( BPTR fh, ULONG len, struct FileNode *fn, ULONG memtype);
UWORD ReadTHAPChunk( BPTR fh, ULONG len, struct FileNode *fn );
UWORD LoadAnimFile(BPTR fh, char *name);
void *ReadAnimFileChunk(BPTR fh, ULONG len, char *name);
void FreeAnimation(struct AnimInfoNode *ain);
UWORD CookAnimations( struct LameReq *lr );
void FreeCookedAnimations(void);
UWORD CookEvents( struct LameReq *lr );
void FreeCookedEvents(void);
struct EventNode * __asm DoEvents(register __a0 struct List *list,
																	register __a1 struct AnimInfoNode *ain,
																	register __a2 struct EventNode *ev,
																	register __d0 UWORD frame);
void __asm SetBitplanePointers(register __a0 struct AnimInfoNode *ain,
															 register __a1 UBYTE *bitmap);

UWORD DisplayAnimation(struct LevelNode *ln);
void CopyBitMap(ULONG *srcptr, ULONG *destptr, ULONG bmbytesize);

APTR MyDebugAllocVec( ULONG bytesize, ULONG requirements, char *name );
void MyDebugFreeVec( APTR memoryblock );
void DisplayMemList(void);
APTR MyAllocVec( ULONG bytesize, ULONG requirements );
void MyFreeVec( APTR memoryblock );
void FreeForgotenMemory(void);
struct ChunkNode *FindChunkByName( ULONG typeid, char *name );
struct ChunkNode *FindChunkByType( ULONG typeid );
struct FileNode *FindRawFile(struct List *list, char *name, struct FileNode *startfn);
struct FileNode *FindFileByName(char *name);
UWORD CookBobs( struct LameReq *lr );
void FreeCookedBobs( void );
UWORD CookSprites( struct LameReq *lr );
void FreeCookedSprites( void );
void CheckUniqueImageNames(void);
UWORD CookAnims( struct LameReq *lr );
void FreeCookedAnims( void );
void CheckUniqueAnimNames(void);
UWORD CookProgs( struct LameReq *lr );
void FreeCookedProgs( void );
void CheckUniqueProgNames(void);
UWORD PreCookPlayers( struct LameReq *lr );
UWORD PostCookPlayers( struct LameReq *lr );
void FreeCookedPlayers( void );
UWORD PreCookWeapons( struct LameReq *lr );
UWORD PostCookWeapons( struct LameReq *lr );
void FreeCookedWeapons( void );
void CheckUniqueWeaponNames(void);
UWORD CookDudeFormations( struct LameReq *lr );
void FreeCookedDudeFormations( void );
void CheckUniqueFormationNames(void);
UWORD CookTimedWaves( struct LameReq *lr );
void FreeCookedTimedWaves( void );
UWORD CookScrollWaves( struct LameReq *lr );
void FreeCookedScrollWaves( void );
UWORD CookSFX( struct LameReq *lr );
void FreeCookedSFX( void );
void CheckUniqueSFXNames(void);
UWORD AllocNewSliceNode(	UWORD bitmapwidth, UWORD bitmapheight, struct Display *disp,
													struct PaulsPictureImage *ppi);
void FreeSliceNode(struct SliceNode *sn);
UWORD AllocScroll(struct GameSlice *slice, struct MAPHeader *mapheader,
								 struct BLKSHeader *blksheader, UWORD bufferwidth, UWORD bufferheight);
void FreeScroll(struct GameSlice *slice);

/* from LameEOTR.c */
void 	FreeCookedEdgeOfTheRoad( void );
UWORD CookEdgeOfTheRoad( struct LameReq *lr );

/* from other files... */

UWORD CookDisplay( struct LameReq *lr );
void FreeCookedDisplay( void );
void CheckUniqueDisplayChunk(void);
UWORD PreCookStatBox( struct LameReq *lr );
UWORD CookStatBox( struct LameReq *lr );
void FreeCookedStatBox( void );
UWORD CookCopperWaits(  struct LameReq *lr );
UWORD	AllocInterlaceData(struct GameSlice *slice);
void FreeInterlaceData(struct GameSlice *slice);
UWORD AllocBitMaps(struct SliceNode *sn, ULONG bmsize, struct PaulsPictureImage *ppi);
void DeallocBitMaps(struct SliceNode *sn);
UWORD AllocCopperHeader(struct SliceHeader *sh, UWORD depth, ULONG displayid);
void FreeCopperHeader(struct SliceHeader *sh);
struct CopperWaitNode *AllocCopperWait(UWORD line, UWORD commandinst, UWORD commandvalue);
void FreeCopperWait(struct CopperWaitNode *cwn);
UWORD AllocCopperList(struct GameSlice *slice, struct Display *dp,
											struct DisplayNode *dn);
void DeallocCopperList( struct DisplayNode *dn);
UWORD SolveTemplate(struct LameReq *lr, char *progname);
void TerminateFile(struct FileNode *fn);
void FreeChunk(struct ChunkNode *cn);
UWORD UnpackILBM(struct BitMapHeader *bmhd, UBYTE *body, UBYTE *destimage, UWORD allignedbytewidth);
UWORD CookILBMs( struct LameReq *lr );
void FreeCookedILBMs( void );
void PrintPonkWarnings(void);
void DumpBadDudes(struct List *list);
char *MyStrDup(char *source);
STRPTR MyFilePart( STRPTR );
STRPTR MyPathPart( STRPTR );
void CopyFilePart( STRPTR name, UBYTE *buf );
void CopyPathPart( STRPTR name, UBYTE *buf );

struct Node *FindStringName(struct List *list, char *name, struct Node *startnode);
struct Node *FindCompressedName(struct List *list, char *name, struct Node *startnode);
BOOL CompressedStringSame(UWORD *string1, UWORD *string2);

STRPTR __asm *LoadLevelConfig( register __a0 STRPTR configfile );
void __asm FreeLevelConfig( register __a0 STRPTR *array );

void DisplayError(char *string);
void DisplayWarning(char *string);


/**************    EXTERNAL VARS   ***********************/
extern struct MinList			 memlist;
extern struct List				 filelist;		/* File List or something like that */
extern struct List				 boblist;
extern struct List				 spritelist;
extern struct List				 ilbmlist;
extern struct List				 animlist;
extern struct List				 proglist;
extern struct List				 formationlist;
extern struct List				 pathlist;
extern struct List				 playerlist;
extern struct List				 timedwavelist;
extern struct List				 scrollwavelist;
extern struct List				 sfxlist;
extern struct List				 weaponlist;
extern struct List				 eotrlist;
extern struct List				 bulletlist;
extern struct List				 baddudelist;
extern struct DisplayNode	 displaynode;
extern struct List				 slicelist;
extern struct List				 eventlist;
extern struct MinList			 copperwaitlist;
//extern struct GameSlice *firstslice;
extern struct StatSlice		*statslice;
extern UWORD							*shieldbarimages;
extern UWORD							 extraallocatedplayers;
extern UWORD							 extraallocatedplayerbullets;
extern UWORD							 extraallocateddudes;

extern BOOL								 agacomputer;
extern BOOL								 agaonlygame;
extern struct PaulsScrollWave	*firstleftswav,*firstrightswav;
extern WORD									displaytop, displayheight;

extern UBYTE *stdestptr, *stsrcptr;
extern struct LameReq *stlr;						/* SolveTemplate arguments */

extern char		errorstring[512];					/* General buffer to for error details */
extern struct GfxBase					*GfxBase;

extern UBYTE	NextLevelBuffer[10];
extern UWORD	ALittleCopperList[];
extern struct Interrupt CopperInterruptStruct;
extern UBYTE	ModulePlayerEnable;
extern void	 *SongDataPtr;
extern ULONG	CIAABase;
extern UBYTE	RawKeyArray[IECODE_KEY_CODE_LAST];
extern struct TextFont *TopazTextFont;
extern UWORD DummyCopper[];
extern struct Task *OurTaskBase;

/* Ponk Warnings */
extern ULONG	recursiveactionlistaddress;
