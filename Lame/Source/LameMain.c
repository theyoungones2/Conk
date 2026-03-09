/*			LAME
				----
  This is the loader and cooker (processor) for CONK. Here's the procedure
for using Lame.
  1) Call InitLame(). This just initialises all the internal lists
and stuff like that.
	2) Call LoadGameConfig(GameFile). The game file is file with all the..
the list of all files to load, one file per line. This than makes up an array
(levelconfigarray) of pointers to the names of the files to load in. This routine
will return a pointer to the start of the file name array, place this pointer
into lamereq->lr_Files.
 3) Call LoadFiles(lamereq). You must supply your own lamereq structure. The
first part of this structure must be completed by you. This routine will
return an error code. If this code is less than LAME_FAILAT, then don't worry about it.
 4) Pull what ever data you need out of the lamereq structure and go for broke.
 5) Call FreeFiles(). This Deallocates all the run time data, but actually leaves the
files in memory. This is so that when we go to load a file, we can see if it is
resident in memory already, and if so don't load it again. This infinitly speeds up the
reloading of level files. There is a ton of memory handling done though, so that if
the MyAllocVec() routine can't allocated the desired amount of memory, it will dump
all the files that have nothing to do with this level (given by the
FileNode->fn_UsedInLevel flag), and it will also dump loaded chunks that have been cooked
(ChunkNode->cn_Cooked), and can be freed (ChunkNode->cn_Freeable). If a chunk out of
a file  is freed then the files fn_Complete flag is set to False. This is to say that
the file will need to be reloaded again.
 6) For each level just keep jumping through steps 2-5.
 7) Call KillLame(). This kills all the loaded files completely.
 */

#include <stdarg.h>
#include <clib/alib_stdio_protos.h>
#include <exec/exec.h>
#include <dos/dos.h>
#include <graphics/modeid.h>
#include <graphics/display.h>
#include <graphics/gfxbase.h>
#include <hardware/intbits.h>
#include <hardware/custom.h>
#include <proto/exec.h>
#include <proto/dos.h>
#include <lame.h>
#include <mycustom.h>
#include <debug.h>
#include <protos.h>
#include <game.h>

extern struct Custom custom;

/****************************************/
// GLOBALS
/****************************************/

struct MinList	 		 memlist;
struct List					 filelist;
struct List					 levellist;
struct List					 boblist;
struct List					 spritelist;
struct List					 ilbmlist;
struct List					 animlist;
struct List					 proglist;
struct List					 formationlist;
struct List					 pathlist;
struct List					 playerlist;
struct List					 timedwavelist;
struct List					 scrollwavelist;
struct List					 sfxlist;
struct List					 weaponlist;
struct List					 eotrlist;
struct List					 bulletlist;
struct List					 baddudelist;
struct List					 eventlist;
struct MinList			 copperwaitlist;
struct DisplayNode	 displaynode;
struct List					 slicelist;
//struct GameSlice	*firstslice;
struct StatSlice		*statslice;
UWORD								*shieldbarimages;

UWORD								 extraallocatedplayers;
UWORD								 extraallocatedplayerbullets;
UWORD								 extraallocateddudes;
UBYTE *stsrcptr, *stdestptr;

BOOL								 agacomputer = FALSE;
BOOL								 agaonlygame = FALSE;
struct PaulsScrollWave	*firstleftswav,*firstrightswav;
struct GameFileHeader	*gamefileheader = NULL;
WORD									displaytop,displayheight;

char		errorstring[512];					/* General buffer for error details */

/* from Action.s */
extern struct VirtualJoystick VirtualJoysticks[];

//static void DumpLevelNames(void);
static void SortOutModuleContinuation(struct LevelNode *ln);
static BOOL SetupVirtualJoysticks( STRPTR keysfile );

/**************   InitLame()   **************************/
//
//Set up the Lame system.
//Returns success.

BOOL __asm InitLame(void)
{
	NewList( (struct List *)&memlist );
	NewList( &filelist );
	NewList( &levellist );
	NewList( &boblist );
	NewList( &spritelist );
	NewList( &ilbmlist );
	NewList( &animlist );
	NewList( &proglist );
	NewList( &formationlist );
	NewList( &pathlist );
	NewList( &playerlist );
	NewList( &timedwavelist );
	NewList( &scrollwavelist );
	NewList( &sfxlist );
	NewList( &weaponlist );
	NewList( &eotrlist );
	NewList( &bulletlist );
	NewList( &baddudelist );
	NewList( &eventlist );
	NewList( &slicelist );
	NewList( (struct List *)&copperwaitlist );
	shieldbarimages = NULL;

	statslice = NULL;
	firstleftswav = NULL;
	firstrightswav = NULL;
	extraallocatedplayers = 0;
	extraallocatedplayerbullets = 0;
	extraallocateddudes = 0;
	//displaynode.dn_FirstSlice = NULL;
	displaynode.dn_CopperList1 = NULL;
	displaynode.dn_CopperList2 = NULL;
	if((GfxBase->ChipRevBits0 & (GFXF_AA_ALICE | GFXF_AA_LISA)) == (GFXF_AA_ALICE | GFXF_AA_LISA))
	{
		D(bug("AGA Computer\n")); 
		agacomputer = TRUE;
	}
	else
	{
		D(bug("OCS Computer\n"));
		agacomputer = FALSE;
	}
	displaytop = 0x002C;

  if (GfxBase->DisplayFlags & PAL)
		displayheight = 0x0100;
	else
		displayheight = 200;

	return( TRUE );
}



/***************   KillLame()   *************************/
//
// Kill everything that lame has allocated.

void __asm KillLame(void)
{
	D(bug("KillLame()\n"));
	FreeFiles();
	FlushFiles();
	FreeForgotenMemory();
	D(bug("KillLame() Done\n"));
}


/***************   LoadGameConfig()   *************************/

extern BPTR OldDirLock;		/* from Ponk.s */
extern BPTR OurDirLock;		/* from Ponk.s */


UWORD __asm LoadGameConfig( register __a0 STRPTR configfile )
{
	BPTR fh = NULL;
	UWORD err = LAMERR_ALLOK,filenum;
	struct LevelNode *ln;
	UBYTE *p;
	struct FileNameStruct *fns = NULL;
	UWORD levelnum;
	ULONG iffbuf[5]; /* [0]FORM,[1]SIZE,[2]CONK,[3]GAME,[4]SIZE */
	UBYTE buf[64];

	UBYTE pathbuf[128];
	UBYTE filebuf[128];

	/* split config name up into path and file */
	CopyFilePart( configfile, filebuf );
	CopyPathPart( configfile, pathbuf );

	/* cd to correct dir */

	OurDirLock = NULL;
	OldDirLock = NULL;

	if( OurDirLock = Lock( pathbuf, ACCESS_READ ) )
		OldDirLock = CurrentDir( OurDirLock );

	D(bug("LoadGameConfig()..."));
	if( fh=Open( filebuf, MODE_OLDFILE ) )
	{
		if(Read(fh, iffbuf, 5*4) == 5*4)
		{
			if( (iffbuf[0] == ID_FORM) && (iffbuf[2] == ID_CONK) && (iffbuf[3] == ID_GAME) )
			{
				if( gamefileheader = MYALLOCVEC( iffbuf[4], MEMF_ANY, "GameConfigMemory" ) )
				{
					if(Read(fh, gamefileheader, iffbuf[4]) == iffbuf[4])
					{
						p = (UBYTE *)gamefileheader;
						p += sizeof(struct GameFileHeader);
			
						for(levelnum = 0; (levelnum < gamefileheader->gfh_NumOfLevels) && (err < LAME_FAILAT); levelnum++)
						{
							if(ln = MYALLOCVEC(sizeof(struct LevelNode),MEMF_ANY|MEMF_CLEAR, "LevelNode"))
							{
								AddTail(&levellist,&ln->ln_Node);
								ln->ln_LevelFileHeader = (struct LevelFileHeader *)p; p += sizeof(struct LevelFileHeader);
								ln->ln_Node.ln_Name = ln->ln_LevelFileHeader->lfh_LevelName;
								ln->ln_NumOfFiles = ln->ln_LevelFileHeader->lfh_NumOfFiles;
				
								if(fns = MYALLOCVEC(sizeof(struct FileNameStruct)*ln->ln_NumOfFiles, MEMF_ANY|MEMF_CLEAR, "FrameNodeStrutures"))
								{
									ln->ln_FileNames = fns;
									for(filenum = 0; (filenum < ln->ln_NumOfFiles) && (err < LAME_FAILAT); filenum++)
									{
										if(p < ((UBYTE *)gamefileheader)+iffbuf[4])
										{
											fns->fns_FileType = *(UWORD *)p; p += 2;
											fns->fns_Name = p; p += GetTextLength(fns->fns_Name)+1;
											if((ULONG)p & 1) p += 1;
											fns++;
										}
										else
										{
											err = LAMERR_TRUNCATED_CONFIG_FILE;
											sprintf(errorstring, "Error: %ld\nDescription: Game Config File Truncated\n", err);
											DisplayError(errorstring);
										}
									}
								}
							}
							else
							{
								err = LAMERR_MEM;
								sprintf(errorstring, "Error: %ld\nDescription: Can't Open File '%s'\n", err, configfile);
								DisplayError(errorstring);
							}
						}
					}
					else
					{
						err = LAMERR_READ;
						sprintf(errorstring, "Error: %ld\nDescription: Game Config File '%s' Too Small\n", err, configfile);
						DisplayError(errorstring);
					}
				}
				else
				{
					err = LAMERR_MEM;
					sprintf(errorstring, "Error: %ld\nDescription: Can't Open File '%s'\n", err, configfile);
					DisplayError(errorstring);
				}
			}
			else
			{
				err = LAMERR_NOT_GAMECONFIG_FILE;
				sprintf(errorstring, "Error: %ld\nDescription: File '%s' Not Game Config File\n", err, configfile);
				DisplayError(errorstring);
			}
		}
		else
		{
			err = LAMERR_READ;
			sprintf(errorstring, "Error: %ld\nDescription: Game Config File '%s' Too Small\n", err, configfile);
			DisplayError(errorstring);
		}
		Close( fh );
	}
	else
	{
		err = LAMERR_FILE_NOT_FOUND;
		sprintf(errorstring, "Error: %ld\nDescription: Can't Open File '%s'\n", err, configfile);
		DisplayError(errorstring);
	}

	/* now suss out input config file */
	if( err < LAME_FAILAT )
	{
		strcpy( buf, MyFilePart( filebuf ) );

		p = buf;
		/* find end of name (or dot)*/
		while( *p && *p != '.' )
			p++;

		*p++ = '.';
		*p++ = 'k';
		*p++ = 'e';
		*p++ = 'y';
		*p++ = 's';
		*p++ = '\0';

		if( !SetupVirtualJoysticks( buf ) )
		{
			SetupVirtualJoysticks( "Conk.keys" );
		}

	}

	D(bug("Done\n"));

	return( err );
}


static BOOL SetupVirtualJoysticks( STRPTR keysfile )
{
	BPTR fh;
	BOOL success = FALSE;
	struct VirtualJoystick *vj;
	struct DiskFormatVJ dvjbuf[4];
	int i;

	if( fh = Open( keysfile, MODE_OLDFILE ) )
	{
		if( Read( fh, dvjbuf, 4 * sizeof( struct DiskFormatVJ) ) ==
			4 * sizeof( struct DiskFormatVJ ) )
		{
			for( i=0; i<4; i++ )
			{
				vj = &VirtualJoysticks[i];
				vj->VJ_Type = dvjbuf[i].Type;
				vj->VJ_HWPort = dvjbuf[i].HWPort;
				vj->VJ_RawKeys[0] = dvjbuf[i].RawKeys[0];
				vj->VJ_RawKeys[1] = dvjbuf[i].RawKeys[1];
				vj->VJ_RawKeys[2] = dvjbuf[i].RawKeys[2];
				vj->VJ_RawKeys[3] = dvjbuf[i].RawKeys[3];
				vj->VJ_RawKeys[4] = dvjbuf[i].RawKeys[4];
				vj->VJ_RawKeys[5] = dvjbuf[i].RawKeys[5];
				vj->VJ_RawKeys[6] = dvjbuf[i].RawKeys[6];
				vj->VJ_RawKeys[7] = dvjbuf[i].RawKeys[7];
			}
			success = TRUE;
		}
		Close( fh );
	}
	return success;
}

/*********************   FreeLevelConfig()   ***************************/

void __asm FreeGameConfig(void)
{
	struct LevelNode *ln;

	D(bug("FreeGameConfig()..."));
	while(!IsListEmpty(&levellist))
	{
		ln = (struct LevelNode *)levellist.lh_Head;
		Remove(&ln->ln_Node);
		if(ln->ln_FileNames)
		{
			MYFREEVEC(ln->ln_FileNames);
			ln->ln_FileNames = NULL;
		}
		MYFREEVEC(ln);
	}
	if(gamefileheader)
	{
		MYFREEVEC(gamefileheader);
		gamefileheader = NULL;
	}
	D(bug("Done\n"));
}


/*******************   RunGame()   ***********************/
//
//  Run The Game until we have no next level

UWORD __asm RunGame( register __a0 struct LameReq *lr )
{
	struct LevelNode *ln = NULL;
	UWORD err = LAMERR_ALLOK;
	char namebuffer[LEVELFULLNAMESIZE];
/*	struct CopperWaitNode *cwn; */

	NextLevelBuffer[0] = 130;

	if(!IsListEmpty(&levellist))
	{
		ln = (struct LevelNode *)levellist.lh_Head;
		while( (err < LAME_FAILAT) && ln )
		{
			UnpackASCII(ln->ln_Node.ln_Name, namebuffer, LEVELFULLNAMESIZE-1);
			D(bug("LevelName: '%s' Level Type: %ld\n", namebuffer, ln->ln_LevelFileHeader->lfh_LevelType));

			/* If there is currently a module playing, it goes through all the files in the
				 level comming up, and if the the same module is loaded in that level too, it just
				 leaves it running, else it kills the current module from playing. */
			SortOutModuleContinuation(ln);

			/* This is the bit where the magic of subroutines happens. */
			err = LoadFiles(lr, ln);

			if(err < LAME_FAILAT)
			{
				switch(ln->ln_LevelFileHeader->lfh_LevelType)
				{
					case LT_GAME:
						ActionStart(lr);
						break;
					case LT_TITLE:
						break;
					case LT_ANIM:
						DisplayAnimation(ln);
						break;
				}

				if(!NullName(NextLevelBuffer))
				{
					if(ln = (struct LevelNode *)FindCompressedName(&levellist, NextLevelBuffer, NULL))
					{
						NextLevelBuffer[0] = 130;
					}
					else
					{
						err = LAMERR_LEVEL_NOT_FOUND;
						UnpackASCII(NextLevelBuffer, namebuffer, LEVELFULLNAMESIZE-1);
						sprintf(errorstring, "Error: %ld\nDescription: Level '%s' Not Found\n", err, namebuffer);
						DisplayError(errorstring);
					}
				}
				else
					ln = NULL;

				custom.cop1lc = (ULONG)ALittleCopperList;
				FreeFiles();
			}
		}
	}
	else
	{
		err = LAMERR_NO_LEVELS_LOADED;
		sprintf(errorstring, "Error: %ld\nDescription: No Levels Loaded\n", err);
		DisplayError(errorstring);
	}
	D(bug("RunGame() Done\n"));
	return(err);
}

/*
static void DumpLevelNames(void)
{
	char namebuffer[LEVELFULLNAMESIZE];
	struct LevelNode *ln;
	UWORD count = 0;

	for(ln = (struct LevelNode *)levellist.lh_Head;
			ln->ln_Node.ln_Succ;
			ln = (struct LevelNode *)ln->ln_Node.ln_Succ)
	{
		count++;
		UnpackASCII(ln->ln_Node.ln_Name, namebuffer, LEVELFULLNAMESIZE-1);
		printf("Level %ld Name '%s'\n", count, namebuffer);
	}
}
*/

static void SortOutModuleContinuation(struct LevelNode *ln)
{
	struct FileNode *fn;
	struct FileNameStruct *fns;
	UWORD i;
	BOOL foundmodule = FALSE;
	char namebuffer[BOBFILENAMESIZE];

	if(ModulePlayerEnable && SongDataPtr)
	{
		for(fn = (struct FileNode *)filelist.lh_Head;
				fn->fn_Node.ln_Succ && (fn->fn_Data != SongDataPtr);
				fn = (struct FileNode *)fn->fn_Node.ln_Succ);
		if(fn->fn_Data == SongDataPtr)
		{
			fns = ln->ln_FileNames;

			for( i=0; (i<ln->ln_NumOfFiles) && !foundmodule; i++ )
			{
				PackASCII(MyFilePart(fns[i].fns_Name), namebuffer, BOBFULLNAMESIZE-1);
				if(CompressedStringSame((UWORD *)fn->fn_CompressedName, (UWORD *)namebuffer) )
					foundmodule = TRUE;
			}
			if(!foundmodule)
				StopModulePlayer();
		}
		else
		{
			/* Couldn't actually find the Module it's currently playing, so had better give an
			   error or something. */
			StopModulePlayer();
			sprintf(errorstring, "Description: Can't find the file for the currently playing module\n");
			DisplayError(errorstring);
		}
	}
}


/******************   LoadFiles()   **********************/
//
// Load in and Cook all the files for the level given.

UWORD __asm LoadFiles( register __a0 struct LameReq *lr,
											 register __a1 struct LevelNode *ln)
{
	UWORD err, i;
	struct FileNode *fn;
	BPTR fh;
	ULONG buf[3];
	struct ChunkNode *cn;
	struct Display *disp;
	struct FileNameStruct *fns;

	D(bug("LoadFiles()\n"));

	err = LAMERR_ALLOK;
	fn = NULL;
	fh = NULL;
	cn = NULL;
	disp = NULL;
	fns = NULL;

	/* step through the filename list and pull stuff in... */
	if(!lr)
	{
		err = LAMERR_FILE_NOT_FOUND;
		sprintf(errorstring, "Description: NULL LameReq Passed to LoadFiles()\n");
		DisplayError(errorstring);
		goto Abort;
	}
	if(!ln)
	{
		err = LAMERR_FILE_NOT_FOUND;
		sprintf(errorstring, "Description: NULL LevelNode Passed To LoadFiles()\n");
		DisplayError(errorstring);
		goto Abort;
	}
	if( !(fns = ln->ln_FileNames) )
	{
		err = LAMERR_MEM;
		sprintf(errorstring, "Error: %ld\nDescription: Not Enough Memory\n", err);
		DisplayError(errorstring);
	}

	for( i=0; (i<ln->ln_NumOfFiles) && (err<LAME_FAILAT); i++ )
	{
		/* see if the file is already in memory */

		D(bug("------------------------\nLoading file: %s...", fns[i].fns_Name ));

		if(fn = FindFileByName(fns[i].fns_Name) )
		{
			/* If the file is incomplete then free the whole file and load it again. This
				should have been done in FreeFiles() already, but just in case... */
			if(!fn->fn_Complete)
			{
				TerminateFile(fn);
				fn = NULL;
			}
		}
		if(fn)
		{
			/* file is already in memory */
			D(bug(" Already loaded.\n"));
			fn->fn_UsedInLevel = TRUE;
		}
		else
		{
			/* got to load file from disk */
			D(bug(" Loading:\n"));

			if(!SussOutVolumes(fns[i].fns_Name))
			{
				err = LAMERR_DOS_VOLUME_NOT_FOUND;
				sprintf(errorstring, "Error Code: %ld\nFile: %s\nDescription: DOS Volume Not Found\n", err, fns[i].fns_Name);
				DisplayError(errorstring);
				goto Abort;
			}

			fh = Open( fns[i].fns_Name, MODE_OLDFILE );
			if( !fh )
			{
				D(bug(" Open() failed!\n"));
				err = LAMERR_FILE_NOT_FOUND;
				sprintf(errorstring, "Error Code: %ld\nFile: %s\nDescription: File Not Found\n", err, fns[i].fns_Name);
				DisplayError(errorstring);
				goto Abort;
			}

			if( Read( fh, buf, 12 ) != 12 )
			{
				err = LAMERR_READ;
				sprintf(errorstring, "Error Code: %ld\nFile: %s\nDescription: Read Error\n", err, fns[i].fns_Name);
				DisplayError(errorstring);
				goto Abort;
			}

			if( buf[0] == ID_FORM )
			{
				switch( buf[2] )
				{
					case ID_CONK:
						if( !(fn = MYALLOCVEC( sizeof( struct FileNode ), MEMF_ANY|MEMF_CLEAR, "CONK File Node" ) ) )
						{
							err = LAMERR_MEM;
							sprintf(errorstring, "Error Code: %ld\nDescription: Not Enough Memory\n", err);
							DisplayError(errorstring);
							goto Abort;
						}
		
						NewList( &fn->fn_ChunkList );
						fn->fn_Node.ln_Name = fns[i].fns_Name;
						fn->fn_FileType = fns[i].fns_FileType;
						fn->fn_Form = ID_CONK;
						fn->fn_Data = NULL;
						while( Read( fh, buf, 8 ) == 8 )
						{
							switch( buf[0] )
							{
								case ID_BLKS:
									if( (err = GenericReadChunk( fh, buf[1], fn, MEMF_CHIP )) < LAME_FAILAT)
									{
										cn = (struct ChunkNode *)fn->fn_ChunkList.lh_TailPred;
										cn->cn_TypeID = ID_BLKS;
										cn->cn_Node.ln_Name = cn->cn_Data;		// Namestring at beginning of data
										cn->cn_Cooked = FALSE;
										cn->cn_Freeable = FALSE;
									}
									break;
								case ID_MAP:
									if( (err = GenericReadChunk( fh, buf[1], fn, MEMF_ANY )) < LAME_FAILAT)
									{
										cn = (struct ChunkNode *)fn->fn_ChunkList.lh_TailPred;
										cn->cn_TypeID = ID_MAP;
										cn->cn_Node.ln_Name = cn->cn_Data;		// Namestring at beginning of data
										cn->cn_Cooked = FALSE;
										cn->cn_Freeable = FALSE;
									}
									break;
								case ID_EOTR:
									if( (err = GenericReadChunk( fh, buf[1], fn, MEMF_ANY )) < LAME_FAILAT)
									{
										cn = (struct ChunkNode *)fn->fn_ChunkList.lh_TailPred;
										cn->cn_TypeID = ID_EOTR;
										cn->cn_Node.ln_Name =
											((struct DiskEOTRHeader *)cn->cn_Data)->Name;
										cn->cn_Cooked = FALSE;
										cn->cn_Freeable = FALSE;
									}
									break;
								case ID_BOBS:
									if( (err = GenericReadChunk( fh, buf[1], fn, MEMF_CHIP )) < LAME_FAILAT)
									{
										cn = (struct ChunkNode *)fn->fn_ChunkList.lh_TailPred;
										cn->cn_TypeID = ID_BOBS;
										cn->cn_Node.ln_Name = cn->cn_Data;	// name at start of bankheader
										cn->cn_Cooked = FALSE;
										cn->cn_Freeable = FALSE;
									}
									break;
								case ID_SPRT:
									if( (err = GenericReadChunk( fh, buf[1], fn, MEMF_ANY )) < LAME_FAILAT)
									{
										cn = (struct ChunkNode *)fn->fn_ChunkList.lh_TailPred;
										cn->cn_TypeID = ID_SPRT;
										cn->cn_Node.ln_Name = cn->cn_Data;	// name at start of bankheader
										cn->cn_Cooked = FALSE;
										cn->cn_Freeable = TRUE;
									}
									break;
								case ID_ANIM:
									if( (err = GenericReadChunk( fh, buf[1], fn, MEMF_ANY )) < LAME_FAILAT)
									{
										cn = (struct ChunkNode *)fn->fn_ChunkList.lh_TailPred;
										cn->cn_TypeID = ID_ANIM;
										cn->cn_Node.ln_Name = cn->cn_Data;		// Namestring at beginning of data
										cn->cn_Cooked = FALSE;
										cn->cn_Freeable = FALSE;
									}
									break;
								case ID_PROG:
									if( (err = GenericReadChunk( fh, buf[1], fn, MEMF_ANY )) < LAME_FAILAT)
									{
										cn = (struct ChunkNode *)fn->fn_ChunkList.lh_TailPred;
										cn->cn_TypeID = ID_PROG;
										cn->cn_Node.ln_Name	=	NULL;
										cn->cn_Cooked = FALSE;
										cn->cn_Freeable = TRUE;
									}
									break;
								case ID_DISP:
									if( (err = GenericReadChunk( fh, buf[1], fn, MEMF_ANY )) < LAME_FAILAT)
									{
										cn = (struct ChunkNode *)fn->fn_ChunkList.lh_TailPred;
										cn->cn_TypeID = ID_DISP;
										cn->cn_Node.ln_Name = NULL;
										cn->cn_Cooked = FALSE;
										cn->cn_Freeable = FALSE;			// Palette taken from file
										disp = (struct Display *)cn->cn_Data;
										
										if(disp->dp_Flags & DPF_AGAONLY)
										{
											agaonlygame = TRUE;
											D(bug("AGA Only Game\n"));
											if(!agacomputer)
											{
												err = LAMERR_REQUIRES_AGA;
												sprintf(errorstring, "Error Code: %ld\nDescription: Game Requires AGA Computer\n", err);
												DisplayError(errorstring);
											}
										}
										else
										{
											agaonlygame = FALSE;
											D(bug("OCS Game\n"));
										}
									}
									break;
/*
								case ID_STAT:
									if( (err = GenericReadChunk( fh, buf[1], fn, MEMF_ANY )) < LAME_FAILAT)
									{
										cn = (struct ChunkNode *)fn->fn_ChunkList.lh_TailPred;
										cn->cn_TypeID = ID_STAT;
										cn->cn_Node.ln_Name = NULL;
										cn->cn_Cooked = FALSE;
										cn->cn_Freeable = FALSE;			// Palette taken from file
									}
									break;
 */
								case ID_WEAP:
									if( (err = GenericReadChunk( fh, buf[1], fn, MEMF_ANY )) < LAME_FAILAT)
									{
										cn = (struct ChunkNode *)fn->fn_ChunkList.lh_TailPred;
										cn->cn_TypeID = ID_WEAP;
										cn->cn_Node.ln_Name = NULL;
										cn->cn_Cooked = FALSE;
										cn->cn_Freeable = TRUE;
									}
									break;
								case ID_DFRM:
									if( (err = GenericReadChunk( fh, buf[1], fn, MEMF_ANY )) < LAME_FAILAT)
									{
										cn = (struct ChunkNode *)fn->fn_ChunkList.lh_TailPred;
										cn->cn_TypeID = ID_DFRM;
										cn->cn_Node.ln_Name = NULL;
										cn->cn_Cooked = FALSE;
										cn->cn_Freeable = TRUE;
									}
									break;
								case ID_THAP:
									err = ReadTHAPChunk( fh, buf[1], fn );
									break;
								case ID_TWAV:
									if( (err = GenericReadChunk( fh, buf[1], fn, MEMF_ANY )) < LAME_FAILAT)
									{
										cn = (struct ChunkNode *)fn->fn_ChunkList.lh_TailPred;
										cn->cn_TypeID = ID_TWAV;
										cn->cn_Node.ln_Name = NULL;
										cn->cn_Cooked = FALSE;
										cn->cn_Freeable = TRUE;
									}
									break;
								case ID_SWAV:
									if( (err = GenericReadChunk( fh, buf[1], fn, MEMF_ANY )) < LAME_FAILAT)
									{
										cn = (struct ChunkNode *)fn->fn_ChunkList.lh_TailPred;
										cn->cn_TypeID = ID_SWAV;
										cn->cn_Node.ln_Name = NULL;
										cn->cn_Cooked = FALSE;
										cn->cn_Freeable = TRUE;
									}
									break;
								case ID_SFX:
									if( (err = GenericReadChunk( fh, buf[1], fn, MEMF_CHIP )) < LAME_FAILAT)
									{
										cn = (struct ChunkNode *)fn->fn_ChunkList.lh_TailPred;
										cn->cn_TypeID = ID_SFX;
										cn->cn_Node.ln_Name = NULL;
										cn->cn_Cooked = FALSE;
										cn->cn_Freeable = FALSE;
									}
									break;
								case ID_EVNT:
									if( (err = GenericReadChunk( fh, buf[1], fn, MEMF_ANY )) < LAME_FAILAT)
									{
										cn = (struct ChunkNode *)fn->fn_ChunkList.lh_TailPred;
										cn->cn_TypeID = ID_EVNT;
										cn->cn_Node.ln_Name	=	NULL;
										cn->cn_Cooked = FALSE;
										cn->cn_Freeable = TRUE;
									}
									break;
								case ID_CMAP:
								case ID_GCFG:
									if( buf[1] & 1 ) buf[1]++;
									Seek( fh, buf[1], OFFSET_CURRENT );
									break;
								default:
									D(bug( " Unrecognised chunk, ID=$%lx\n", buf[0] ));
									if( buf[1] & 1 ) buf[1]++;
									Seek( fh, buf[1], OFFSET_CURRENT );
									break;
							}
							if( err>=LAME_FAILAT ) goto Abort;
						}
		
						/* file loaded ok - add filenode to list */
						fn->fn_UsedInLevel = TRUE;
						fn->fn_Complete = TRUE;
						AddTail( &filelist, (struct Node *)fn );
						break;

					case ID_ILBM:
						if( !(fn = MYALLOCVEC( sizeof( struct FileNode ), MEMF_ANY|MEMF_CLEAR, "ILBM File Node" ) ) )
						{
							err = LAMERR_MEM;
							sprintf(errorstring, "Error Code: %ld\nDescription: Not Enough Memory\n", err);
							DisplayError(errorstring);
							goto Abort;
						}
		
						NewList( &fn->fn_ChunkList );
						fn->fn_Node.ln_Name = fns[i].fns_Name;
						fn->fn_FileType = fns[i].fns_FileType;
						fn->fn_Form = ID_ILBM;

						while( Read( fh, buf, 8 ) == 8 )
						{
							switch( buf[0] )
							{
								case ID_BMHD:
									if( (err = GenericReadChunk( fh, buf[1], fn, MEMF_ANY )) < LAME_FAILAT)
									{
										cn = (struct ChunkNode *)fn->fn_ChunkList.lh_TailPred;
										cn->cn_TypeID = ID_BMHD;
										cn->cn_Node.ln_Name = NULL;
										cn->cn_Cooked = FALSE;
										cn->cn_Freeable = TRUE;
									}
									break;
								case ID_CMAP:
									if( (err = GenericReadChunk( fh, buf[1], fn, MEMF_ANY )) < LAME_FAILAT)
									{
										cn = (struct ChunkNode *)fn->fn_ChunkList.lh_TailPred;
										cn->cn_TypeID = ID_CMAP;
										cn->cn_Node.ln_Name = NULL;
										cn->cn_Cooked = FALSE;
										cn->cn_Freeable = FALSE;		/* Used for PaulsPictureImage */
									}
									break;
								case ID_BODY:
									if( (err = GenericReadChunk( fh, buf[1], fn, MEMF_ANY )) < LAME_FAILAT)
									{
										cn = (struct ChunkNode *)fn->fn_ChunkList.lh_TailPred;
										cn->cn_TypeID = ID_BODY;
										cn->cn_Node.ln_Name = NULL;
										cn->cn_Cooked = FALSE;
										cn->cn_Freeable = TRUE;
									}
									break;
								default:
									if( buf[1] & 1 ) buf[1]++;
									Seek( fh, buf[1], OFFSET_CURRENT );
									break;
							}
							if( err>=LAME_FAILAT ) goto Abort;
						}
		
						/* file loaded ok - add filenode to list */
						fn->fn_UsedInLevel = TRUE;
						fn->fn_Complete = TRUE;
						AddTail( &filelist, (struct Node *)fn );
						break;

					case ID_ANIM:
						err = LoadAnimFile(fh, fns[i].fns_Name);
						break;

					default:
						err = LAMERR_FUCKUP;
						sprintf(errorstring, "Error Code: %ld\nFile: %s\nDescription: Unknown IFF Form\n", err, fns[i].fns_Name);
						DisplayError(errorstring);
						goto Abort;
						break;
				}
			}
			else
			{
				/* Not IFF File, so just load it into chip memory as is, where is */
				if( !(fn = MYALLOCVEC( sizeof( struct FileNode ), MEMF_ANY|MEMF_CLEAR, "RawFile File Node" ) ) )
				{
					err = LAMERR_MEM;
					sprintf(errorstring, "Error Code: %ld\nDescription: Not Enough Memory\n", err);
					DisplayError(errorstring);
					goto Abort;
				}
				NewList( &fn->fn_ChunkList );
				fn->fn_Node.ln_Name = fn->fn_CompressedName;
				PackASCII(MyFilePart(fns[i].fns_Name), fn->fn_CompressedName, BOBFULLNAMESIZE-1);
				fn->fn_Form = 0;
				fn->fn_FileType = fns[i].fns_FileType;
				fn->fn_UsedInLevel = TRUE;
				fn->fn_Complete = TRUE;
				if( (err = LoadRawFile(fh, fn)) < LAME_FAILAT) {}
				AddTail( &filelist, (struct Node *)fn );
			}
			Close( fh );
			fh=NULL;			// indicate file closed.
		}
	}
	/* cook any data that needs it */
	/* All the chunks have been loaded in, so we can now link up chunks which */
	/* need each other. */
	D(bug( "----- Cook Data ------\n" ));

	if( (err = CookILBMs( lr )) >= LAME_FAILAT )
		goto Abort;
	if( (err = CookSFX( lr )) >= LAME_FAILAT )
		goto Abort;
	CheckUniqueSFXNames();
	if( (err = CookEvents( lr )) >= LAME_FAILAT )
		goto Abort;

	if(ln->ln_LevelFileHeader->lfh_LevelType == LT_GAME)
	{
		if( (err = CookBobs( lr )) >= LAME_FAILAT )
			goto Abort;
		if( (err = CookSprites( lr )) >= LAME_FAILAT )
			goto Abort;
		CheckUniqueImageNames();
		if( (err = CookAnims( lr )) >= LAME_FAILAT )
			goto Abort;
		CheckUniqueAnimNames();
		if( (err = CookEdgeOfTheRoad( lr )) >= LAME_FAILAT )
			goto Abort;
		if( (err = PreCookPlayers( lr )) >= LAME_FAILAT )
			goto Abort;
		if( (err = PreCookWeapons( lr )) >= LAME_FAILAT )
			goto Abort;
		CheckUniqueWeaponNames();
		if( (err = CookProgs( lr )) >= LAME_FAILAT )
			goto Abort;
		CheckUniqueProgNames();
		if( (err = PostCookPlayers( lr )) >= LAME_FAILAT )
			goto Abort;
		if( (err = PostCookWeapons( lr )) >= LAME_FAILAT )
			goto Abort;
		if( (err = CookDudeFormations( lr )) >= LAME_FAILAT )
			goto Abort;
		CheckUniqueFormationNames();
		if( (err = CookTimedWaves( lr )) >= LAME_FAILAT )
			goto Abort;
		if( (err = CookScrollWaves( lr )) >= LAME_FAILAT )
			goto Abort;
		CheckUniqueDisplayChunk();
		if( (err = PreCookStatBox( lr )) >= LAME_FAILAT )
			goto Abort;
		if(cn = FindChunkByType(ID_DISP))
		{
			if(((struct Display *)cn->cn_Data)->dp_Flags & DPF_STATBOXATTOP)
			{
				/* Cook The Stat Box First */
				if( (err = CookStatBox( lr )) >= LAME_FAILAT )
					goto Abort;
				if( (err = CookDisplay( lr )) >= LAME_FAILAT )
					goto Abort;
			}
			else
			{
				/* Cook The GameSlice First */
				if( (err = CookDisplay( lr )) >= LAME_FAILAT )
					goto Abort;
				if( (err = CookStatBox( lr )) >= LAME_FAILAT )
					goto Abort;
			}
		}
		else
		{
			if( (err = CookDisplay( lr )) >= LAME_FAILAT )
				goto Abort;
		}
		if( (err = CookShieldBarImage( lr )) >= LAME_FAILAT )
			goto Abort;
		if( (err = AllocInterruptCopperWait( lr )) >= LAME_FAILAT )
			goto Abort;
		if( (err = CookCopperWaits( lr )) >= LAME_FAILAT )
			goto Abort;
	}	

	if(ln->ln_LevelFileHeader->lfh_LevelType == LT_ANIM)
	{
		if( (err = CookAnimations( lr )) >= LAME_FAILAT )
			goto Abort;
	}

	/* Set up pointers that the game code can read */
	SetupReturns( lr );

Abort:
	if(fh)
	{
		Close(fh);			// if there was an error the file might be open still
		fh = NULL;
	}

#ifdef STOP_PAUL
	if(err < LAME_FAILAT)
	{
		if(lr)
		{
			lr->lr_Error = LAMERR_BUGGER_OFF_BACK_TO_WORKBENCH;
			err = lr->lr_Error;
		}
		D(bug("----------------------\n*** Forced Error ***\n"));
	}
#else
	if(lr)
		lr->lr_Error = err;
#endif

	if( err != LAMERR_ALLOK )
	{
		D(bug("*** Load Error ***   Code: %lu\n",err));
		if(lr)
			lr->lr_FileNum = i;
	}

	D(bug("LoadFiles() Done\n"));
	return( err );
}

/****************   FreeFiles()   ************************/
//
//  This deallcoates all the run time data, but keeps all the
// files in memory. Except for non-complete files, which it
// deletes completely.

void __asm FreeFiles( void )
{
	struct FileNode *fn, *nextfn;

	PrintPonkWarnings();
	/* DumpBadDudes(&baddudelist); */

	D(bug("----------------------\nFreeFiles()\n"));

	FreeCookedAnims();
	FreeCookedSprites();
	FreeCookedBobs();
	FreeCookedILBMs();
	FreeCookedSFX();
	FreeCookedProgs();
	FreeCookedPlayers();
	FreeCookedWeapons();
	FreeCookedEdgeOfTheRoad();
	FreeCookedDudeFormations();
	FreeCookedTimedWaves();
	FreeCookedScrollWaves();
	FreeCookedStatBox();
	FreeCookedDisplay();
	FreeCookedShieldBarImage();
	FreeCookedAnimations();
	FreeCookedEvents();
	FreeExtraCopperWaits();

	NewList( &boblist );
	NewList( &spritelist );
	NewList( &ilbmlist );
	NewList( &animlist );
	NewList( &proglist );
	NewList( &formationlist );
	NewList( &pathlist );
	NewList( &playerlist );
	NewList( &timedwavelist );
	NewList( &scrollwavelist );
	NewList( &sfxlist );
	NewList( &weaponlist );
	NewList( &eotrlist );
/*	numofweapons = 0; */
	NewList( &bulletlist );
	NewList( &baddudelist );
	NewList( &eventlist );
	NewList( &slicelist );
	NewList( (struct List *)&copperwaitlist );
	shieldbarimages = NULL;
	statslice = NULL;
	firstleftswav = NULL;
	firstrightswav = NULL;
	if(extraallocatedplayers)
	{
		sprintf(errorstring, "Warning - Had To Allocate an Extra %ld Player Structures\n", extraallocatedplayers);
		DisplayWarning(errorstring);
		extraallocatedplayers = 0;
	}
	if(extraallocatedplayerbullets)
	{
		sprintf(errorstring, "Warning - Had To Allocate an Extra %ld Player Bullet Structures\n", extraallocatedplayerbullets);
		DisplayWarning(errorstring);
		extraallocatedplayerbullets = 0;
	}
	if(extraallocateddudes)
	{
		sprintf(errorstring, "Warning - Had To Allocate an Extra %ld Dude Structures\n", extraallocateddudes);
		DisplayWarning(errorstring);
		extraallocateddudes = 0;
	}
	displaynode.dn_CopperList1 = NULL;
	displaynode.dn_CopperList2 = NULL;
 
	fn = (struct FileNode *)filelist.lh_Head;
	while(fn->fn_Node.ln_Succ)
	{
		nextfn = (struct FileNode *)fn->fn_Node.ln_Succ;
		/* Clear UsedInLevel flag, so that all files are free to be deallocated. */
		fn->fn_UsedInLevel = FALSE;
		/* If the file is incomplete then kill it completely, this make the loader load it again */
		if(!fn->fn_Complete)
			TerminateFile(fn);
		fn = nextfn;
	}
	D(bug("FreeFiles() Done\n"));
}

/**************   FreeChunk()   **************************/
//
//  Deallocates the file data associated with the given chunk.

void FreeChunk(struct ChunkNode *cn)
{
	struct PathNode *pn,*nextpn;

	switch( cn->cn_TypeID )
	{
		case ID_BLKS:
			MYFREEVEC( cn->cn_Data );
			break;
		case ID_MAP:
			MYFREEVEC( cn->cn_Data );
			break;
		case ID_EOTR:
			MYFREEVEC( cn->cn_Data );
			break;
		case ID_BOBS:
			MYFREEVEC( cn->cn_Data );
			break;
		case ID_SPRT:
			MYFREEVEC( cn->cn_Data );
			break;
		case ID_ANIM:
			MYFREEVEC( cn->cn_Data );
			break;
		case ID_PROG:
			MYFREEVEC( cn->cn_Data );
			break;
		case ID_WEAP:
			MYFREEVEC( cn->cn_Data );
			break;
		case ID_DFRM:
			MYFREEVEC( cn->cn_Data );
			break;
		case ID_TWAV:
			MYFREEVEC( cn->cn_Data );
			break;
		case ID_SWAV:
			MYFREEVEC( cn->cn_Data );
			break;
		case ID_SFX:
			MYFREEVEC( cn->cn_Data );
			break;
		case ID_DISP:
			MYFREEVEC( cn->cn_Data );
			break;
/*
		case ID_STAT:
			MYFREEVEC( cn->cn_Data );
			break;
 */
		case ID_BMHD:
			MYFREEVEC( cn->cn_Data );
			break;
		case ID_CMAP:
			MYFREEVEC( cn->cn_Data );
			break;
		case ID_BODY:
			MYFREEVEC( cn->cn_Data );
			break;
		case ID_EVNT:
			MYFREEVEC( cn->cn_Data );
			break;
		case ID_THAP:
			D(bug("  THAP chunk\n"));
			/* This one is a little different, we go through all the paths attached to this
				chunk and free them individually. */
			pn = cn->cn_FirstItem;
			while(pn)
			{
				nextpn = pn->pn_NextPathNodeInChunk;
				/* Remove this path from the global list of paths */
				Remove(&pn->pn_Node);
				if(pn->pn_PathHeader)
				{
					MYFREEVEC(pn->pn_PathHeader);
					pn->pn_PathHeader = NULL;
				}
				MYFREEVEC(pn);
				pn = nextpn;
			}
			break;
		default:
			D(bug("  Unknown chunk\n"));
			break;
	}
	Remove( (struct Node *)cn );
	MYFREEVEC( cn );
}	

/**************   FlushFiles()   **************************/
//
//  This clears out as much unneeded data as it can. It will
// delete unused files, as well as file chunks that have
// been cooked, and can be freed. If any file has an empty
// chunk list, the file itself is also deleted.

void __asm FlushFiles( void )
{
	struct FileNode *fn, *nextfn;
	struct ChunkNode *cn, *nextcn;

	D(bug("----------------------\nFlushFiles()\n"));

	fn = (struct FileNode *)filelist.lh_Head;
	while( fn->fn_Node.ln_Succ )
	{
		nextfn = (struct FileNode *)fn->fn_Node.ln_Succ;
		if( fn->fn_UsedInLevel )
		{
			cn = (struct ChunkNode *)fn->fn_ChunkList.lh_Head;
			while( cn->cn_Node.ln_Succ )
			{
				nextcn = (struct ChunkNode *)cn->cn_Node.ln_Succ;
				if( (cn->cn_Cooked) && (cn->cn_Freeable) )
				{
					D(bug(" Flushing Chunk From File: %s   Chunk:",fn->fn_Node.ln_Name ));
					FreeChunk(cn);
					fn->fn_Complete = FALSE;
				}
				cn = nextcn;
			}
		}
		else
		{
#if MYDEBUG
			switch(fn->fn_Form)
			{
				case 0:
					bug(" Flushing: 'RawFile'\n");
					break;
				case ID_ANIM:
					bug(" Flushing: 'Animation'\n");
					break;
				default:
					bug(" Flushing: '%s'\n", fn->fn_Node.ln_Name);
					break;
			}

#endif
			cn = (struct ChunkNode *)fn->fn_ChunkList.lh_Head;
			while( cn->cn_Node.ln_Succ )
			{
				nextcn = (struct ChunkNode *)cn->cn_Node.ln_Succ;
				FreeChunk(cn);
				cn = nextcn;
			}
		}

		if( IsListEmpty(&fn->fn_ChunkList) && ((fn->fn_Form == ID_CONK) || (!fn->fn_UsedInLevel)) )
		{
#if MYDEBUG
			switch(fn->fn_Form)
			{
				case 0:
					bug(" Removing: 'RawFile'\n");
					break;
				case ID_ANIM:
					bug(" Removing: 'Animation'\n");
					break;
				default:
					bug(" Removing: '%s'\n", fn->fn_Node.ln_Name);
					break;
			}
#endif
			TerminateFile(fn);
		}
		fn = nextfn;
	}
}


/*******************   TerminateFile()   *****************/
//
//  Delete the given file completely from memory. (Unlike
// FreeFiles() that leaves the file resident.)

void TerminateFile(struct FileNode *fn)
{
	struct ChunkNode *cn;

	if(fn)
	{
		while( !IsListEmpty(&fn->fn_ChunkList) )
		{
			cn = (struct ChunkNode *)fn->fn_ChunkList.lh_Head;
			FreeChunk(cn);
		}
		if(fn->fn_Data)
		{
			switch(fn->fn_Form)
			{
				case 0:
					/* Can't free the module if it's currently playing */
					if((ModulePlayerEnable == 0) || (SongDataPtr != fn->fn_Data))
						MYFREEVEC(fn->fn_Data);
					break;
				case ID_ANIM:
					FreeAnimation(fn->fn_Data);
					break;
			}
			fn->fn_Data = NULL;
		}
		Remove(&fn->fn_Node);
		MYFREEVEC(fn);
	}
}

/**************   SetupReturns()   **************************/
//
// Fill in the pointer fields in the LameReq structure for easy reference
// by the game code.
//

void SetupReturns( struct LameReq *lr )
{
	struct SliceNode *slicenode;

	if( !IsListEmpty( &slicelist ) )
	{
		slicenode = (struct SliceNode *)slicelist.lh_Head;
		lr->lr_FirstSlice = slicenode->sn_SliceData;
	}
	if(displaynode.dn_CopperList2)
		lr->lr_CopperLists = 1;
	else
		lr->lr_CopperLists = 0;
	lr->lr_CopperList1 = displaynode.dn_CopperList1;
	lr->lr_CopperList2 = displaynode.dn_CopperList2;
	lr->lr_TheFlags = 0;
	if(agaonlygame)
		lr->lr_TheFlags |= LRF_AGA;
	if(agacomputer)
		lr->lr_TheFlags |= LRF_AGAPALETTE;
}



