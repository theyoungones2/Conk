#include <stdarg.h>
#include <clib/alib_stdio_protos.h>
#include <exec/exec.h>
#include <dos/dos.h>
#include <graphics/modeid.h>
#include <graphics/display.h>
#include <graphics/gfxbase.h>
#include <hardware/intbits.h>
#include <hardware/dmabits.h>
#include <proto/exec.h>
#include <proto/dos.h>
#include <proto/graphics.h>
#include <lame.h>
#include <mycustom.h>
#include <debug.h>
#include <protos.h>
#include <RawKeyCodes.h>

#include <hardware/custom.h>

static struct ProgNode *GetActionListName(UBYTE *ptr);
extern struct GroovyCon InsertVolumeGroovyCon;
extern struct GroovyConsoleDevice InsertVolumeGroovyConDevice;
extern UWORD DiskChanged;
extern struct Custom custom;

BOOL FindVolume( char *name );

UWORD LoadRawFile(BPTR fh, struct FileNode *fn)
{
	UWORD err = LAMERR_ALLOK;
	ULONG filesize = 0;

	D(bug(" Reading Raw File..."));

	Seek(fh, 0, OFFSET_END);
	filesize = Seek(fh, 0, OFFSET_BEGINNING);
	if( fn->fn_Data = MYALLOCVEC( filesize, MEMF_CHIP, "LoadRawFile() File Memory" ) )
	{
		if( Read( fh, fn->fn_Data, filesize ) == filesize )
		{
			D(bug("OK\n"));
		}
		else
		{
			err = LAMERR_READ;
			sprintf(errorstring, "Error Code: %ld\nFile: %s\nDescription: Read Error\n", err, fn->fn_Node.ln_Name);
			DisplayError(errorstring);
		}
	}
	else
	{
		err = LAMERR_CHIPMEM;
		sprintf(errorstring, "Error Code: %ld\nDescription: Not Enough Chip Memory\n", err);
		DisplayError(errorstring);
	}

	/* cleanup if there were any errors */
	if( err >= LAME_FAILAT )
	{
		D(bug("Failed.\n"));
		if( fn->fn_Data ) MYFREEVEC(fn->fn_Data);
	}

	return(err);
}

/****************   ReadAnimFileChunk()   ************************/
void *ReadAnimFileChunk(BPTR fh, ULONG len, char *name)
{
	UWORD err = LAMERR_ALLOK;
	void *data = NULL;

	if(data = MYALLOCVEC( len, MEMF_ANY, "LoadAnimFileChunk() Chunk Memory" ))
	{
		if( Read( fh, data, len ) == len )
		{
			/* skip pad byte if odd length */
			if( len & 1 ) Seek( fh, 1, OFFSET_CURRENT );
		}
		else
		{
			err = LAMERR_READ;
			sprintf(errorstring, "Error Code: %ld\nFile: %s\nDescription: Read Error\n", err, name);
			DisplayError(errorstring);
		}
	}
	else
	{
		err = LAMERR_MEM;
		sprintf(errorstring, "Error Code: %ld\nDescription: Not Enough Memory\n", err);
		DisplayError(errorstring);
	}

	/* cleanup if there were any errors */
	if( err >= LAME_FAILAT )
	{
		D(bug("Read AnimFileChunk() Failed.\n"));
		if( data ) MYFREEVEC( data );
	}

	return(data);
}

/****************   GenericReadChunk()   ************************/
UWORD GenericReadChunk( BPTR fh, ULONG len, struct FileNode *fn, ULONG memtype)
{
	UWORD err = LAMERR_ALLOK;
	struct ChunkNode *cn;

	D(bug(" Reading Generic chunk..."));

	if( cn = MYALLOCVEC( sizeof( struct ChunkNode ), MEMF_ANY|MEMF_CLEAR, "ReadGenericChunk() Chunk Node" ) )
	{
		if(cn->cn_Data = MYALLOCVEC( len, memtype, "ReadGenericChunk() Chunk Memory" ))
		{
			if( Read( fh, cn->cn_Data, len ) == len )
			{
				/* skip pad byte if odd length */
				if( len & 1 ) Seek( fh, 1, OFFSET_CURRENT );

				AddTail( &fn->fn_ChunkList, (struct Node *)cn );
				D(bug("OK\n"));
			}
			else
			{
				err = LAMERR_READ;
				sprintf(errorstring, "Error Code: %ld\nFile: %s\nDescription: Read Error\n", err, fn->fn_Node.ln_Name);
				DisplayError(errorstring);
			}
		}
		else
		{
			err = LAMERR_MEM;
			sprintf(errorstring, "Error Code: %ld\nDescription: Not Enough Memory\n", err);
			DisplayError(errorstring);
		}
	}
	else
	{
		err = LAMERR_MEM;
		sprintf(errorstring, "Error Code: %ld\nDescription: Not Enough Memory\n", err);
		DisplayError(errorstring);
	}

	/* cleanup if there were any errors */
	if( err >= LAME_FAILAT )
	{
		D(bug("Failed.\n"));
		if( cn )
		{
			if( cn->cn_Data ) MYFREEVEC( cn->cn_Data );
			MYFREEVEC( cn );
		}
	}

	return(err);
}

#if DEBUGMEM

/**************   MyDebugAllocVec()   **************************/
APTR MyDebugAllocVec( ULONG bytesize, ULONG requirements, char *name )
{
	struct MemNode *mn = NULL;

	if( mn = AllocMem( bytesize+sizeof(struct MemNode), requirements ) )
	{
		mn->mn_Type = requirements;
		mn->mn_MemSize = bytesize;
	}
	else
	{
		FlushFiles();
		if( mn = AllocMem( bytesize+sizeof(struct MemNode), requirements ) )
		{
			mn->mn_Type = requirements;
			mn->mn_MemSize = bytesize;
		}
	}
	if(mn)
	{
		mn->mn_Name = name;
		if(!name)
			printf("AllocVec Given NULL Name\n");
		AddTail((struct List *)&memlist,(struct Node *)&mn->mn_Node);
		return( (APTR)(mn+1) );
	}
	return(NULL);
}

/****************   MyDebugFreeVec()   ************************/
void MyDebugFreeVec( APTR memoryblock )
{
	struct MemNode *mn;

	if(memoryblock != 0)
	{
		mn = memoryblock;
		mn--;
		Remove((struct Node *)&mn->mn_Node);
		FreeMem( mn, mn->mn_MemSize+sizeof(struct MemNode) );
	}
	else
		printf("MyDebugFreeVec(): NULL Memory pointer\n");
}

void DisplayMemList(void)
{
	struct MemNode *mn;

	bug("Memory Dump\n");
	for(mn = (struct MemNode *)memlist.mlh_Head;
			mn->mn_Node.mln_Succ;
			mn = (struct MemNode *)mn->mn_Node.mln_Succ)
	{
		bug("Node Name: %s\n",mn->mn_Name);
	}
}

#else
/**************   MyAllocVec()   **************************/
APTR MyAllocVec( ULONG bytesize, ULONG requirements )
{
	struct MemNode *mn;

	bytesize += sizeof(struct MemNode);
	if( mn = AllocMem( bytesize+sizeof(struct MemNode), requirements ) )
	{
		mn->mn_Name = NULL;
		mn->mn_Type = requirements;
		mn->mn_MemSize = bytesize;
	}
	else
	{
		FlushFiles();
		if( mn = AllocMem( bytesize+sizeof(struct MemNode), requirements ) )
		{
			mn->mn_Name = NULL;
			mn->mn_Type = requirements;
			mn->mn_MemSize = bytesize;
		}
	}
	if(mn)
	{
		AddTail((struct List *)&memlist,(struct Node *)&mn->mn_Node);
		return( (APTR)(mn+1) );
	}
	return(NULL);
}

/****************   MyFreeVec()   ************************/
void MyFreeVec( APTR memoryblock )
{
	struct MemNode *mn;

	if(memoryblock != 0)
	{
		mn = memoryblock;
		--mn;
		Remove((struct Node *)&mn->mn_Node);
		FreeMem( mn, mn->mn_MemSize+sizeof(struct MemNode) );
	}
	else
		printf("MyFreeVec(): NULL Memory pointer\n");
}
#endif


/****************   FreeForgotenMemory()   ************************/

void FreeForgotenMemory(void)
{
	struct MemNode *mn;
	BOOL done = FALSE;

	D(bug("FreeFogottenMemory()\n"));
	while( ((memlist.mlh_TailPred)->mln_Pred) && !done)
	{
		mn = (struct MemNode *)memlist.mlh_Head;
		if(mn->mn_Name)
		{
			printf("Forgotten Memory -  Address: $%lx, Size: %ld, Name: %s\n", mn, mn->mn_MemSize, mn->mn_Name);
			MYFREEVEC(mn+1);
		}
		else
		{
			printf("Forgotten Memory - No Name *** STILL RESIDENT ***\n");
			done = TRUE;
		}
	}
	D(bug("FreeFogottenMemory() Done\n"));
}

/*****************   FindChunkByName()   ***********************/
//
// Search for a loaded chunk with the specified name.
// Returns NULL if no match found.

struct ChunkNode *FindChunkByName( ULONG typeid, char *name )
{
	struct FileNode *fn;
	struct ChunkNode *cn = NULL;

	for(fn = (struct FileNode *)filelist.lh_Head;
		 (fn->fn_Node.ln_Succ) && (!cn);
			fn = (struct FileNode *)fn->fn_Node.ln_Succ )
	{
		if( fn->fn_UsedInLevel )
		{
			cn = (struct ChunkNode *)FindStringName( &fn->fn_ChunkList, name, NULL );
			while( (cn) && (cn->cn_Node.ln_Succ) && (cn->cn_TypeID != typeid) )
			{
				cn = (struct ChunkNode *)cn->cn_Node.ln_Succ;
				cn = (struct ChunkNode *)FindStringName(&fn->fn_ChunkList, name, &cn->cn_Node);
			}
		}
	}
	return( cn );
}

/*****************   FindChunkByType()   ***********************/
//
// Searches through the loaded files for the first chunk of the specified
// type.
// returns the first matching chunk or NULL.

struct ChunkNode *FindChunkByType( ULONG typeid )
{
	struct FileNode *fn;
	struct ChunkNode *cn = NULL, *tempcn;

	for( fn = (struct FileNode *)filelist.lh_Head;
		fn->fn_Node.ln_Succ && !cn;
		fn = (struct FileNode *)fn->fn_Node.ln_Succ )
	{
		if( fn->fn_UsedInLevel )
		{
			for( tempcn = (struct ChunkNode *)fn->fn_ChunkList.lh_Head;
				tempcn->cn_Node.ln_Succ && !cn;
				tempcn = (struct ChunkNode *)tempcn->cn_Node.ln_Succ )
			{
				if( tempcn->cn_TypeID == typeid )
					cn = tempcn;
			}
		}
	}
	return( cn );
}

/****************   FindRawFile()   *************************/
//
// Scan the files until a file with the correct name is found (case-insensitive).
// Returns:  NULL = No matching name found,
//					!NULL = A Raw File with the same name.

struct FileNode *FindRawFile(struct List *list, char *name, struct FileNode *startfn)
{
	struct FileNode *fn;

	if(startfn)
		fn = startfn;
	else
		fn = (struct FileNode *)list->lh_Head;

	for( ; fn->fn_Node.ln_Succ; fn = (struct FileNode *)fn->fn_Node.ln_Succ)
	{
		if( (fn->fn_Form == 0) && (fn->fn_Node.ln_Name) )
		{
			if( CompressedStringSame((UWORD *)name, (UWORD *)fn->fn_Node.ln_Name) )
				return(fn);
		}
	}
	return(NULL);
}
/****************   FindStringName()   *************************/
//
// Scan a list until a node with the correct name is found (case-insensitive).
// Returns:  NULL = No matching name found,
//					!NULL = A node with the same name.

struct Node *FindStringName(struct List *list, char *name, struct Node *startnode)
{
	struct Node *node;

	if(startnode)
		node = startnode;
	else
		node = list->lh_Head;

	if(node)
	{
		for( ; node->ln_Succ; node = node->ln_Succ)
		{
			if(node->ln_Name)
			{
				if( !stricmp(name,node->ln_Name) )
					return(node);
			}
		}
	}
	return(NULL);
}

/****************   FindCompressedName()   *************************/
//
// Scan a list until a node with the correct name is found (case-insensitive).
// Returns:  NULL = No matching name found,
//					!NULL = A node with the same name.

struct Node *FindCompressedName(struct List *list, char *name, struct Node *startnode)
{
	struct Node *node;

	if(startnode)
		node = startnode;
	else
		node = list->lh_Head;

	for( ; node->ln_Succ; node = node->ln_Succ)
	{
		if(node->ln_Name)
		{
			if( CompressedStringSame((UWORD *)name, (UWORD *)node->ln_Name) )
				return(node);
		}
	}
	return(NULL);
}

/****************   FindFileByName()   *************************/
//
// Scan a list until a node with the correct name is found (case-insensitive).
// Returns:  NULL = No matching name found,
//					!NULL = A node with the same name.

struct FileNode *FindFileByName(char *name)
{
	struct FileNode *fn, *retfn=NULL;
	UBYTE namebuffer[BOBFULLNAMESIZE];

	for(fn = (struct FileNode *)filelist.lh_Head;
			fn->fn_Node.ln_Succ && !retfn;
			fn = (struct FileNode *)fn->fn_Node.ln_Succ)
	{
		switch(fn->fn_Form)
		{
			case ID_CONK:
			case ID_ILBM:
			case ID_ANIM:
				if(fn->fn_Node.ln_Name)
				{
					if( !stricmp(name, fn->fn_Node.ln_Name) )
						retfn = fn;
				}
				break;
			case 0:
				if(fn->fn_Node.ln_Name)
				{
					PackASCII(MyFilePart(name), namebuffer, BOBFULLNAMESIZE-1);
					if( CompressedStringSame((UWORD *)fn->fn_Node.ln_Name, (UWORD *)namebuffer) )
						retfn = fn;
				}
				break;
			default:
				D(bug("*** FindFileByName() **** Unknown Form\n"));
				break;
		}
	}
	return(retfn);
}

BOOL CompressedStringSame(UWORD *string1, UWORD *string2)
{
	BOOL ret = FALSE;
	if( (string1[0] == string2[0]) && (string1[1] == string2[1]) && (string1[2] == string2[2]) &&
			(string1[3] == string2[3]) && ((string1[4]&0xFFFC) == (string2[4]&0xFFFC)) )
		ret = TRUE;
	return(ret);
}

/***********************   DisplayError()   ***************************/

void DisplayError(char *string)
{
	printf("***********************************************\n");
	printf("%s", string);
	printf("***********************************************\n");
}

void DisplayWarning(char *string)
{
	printf("%s", string);
}

struct PaulsBob * __asm AllocPlayer(void)
{
	struct PlayerNode *pn;
	struct PaulsObject *prevobject = NULL;

	if(!IsListEmpty(&playerlist))
		prevobject = &((struct PlayerNode *)playerlist.lh_TailPred)->pn_Bob.bb_PaulsObject;

	if( pn = MYALLOCVEC(sizeof(struct PlayerNode), MEMF_ANY|MEMF_CLEAR,"AllocPlayer() PlayerNode") )
	{
		pn->pn_Bob.bb_PaulsObject.ob_ScreenOffset1 = -1;
		pn->pn_Bob.bb_PaulsObject.ob_ScreenOffset2 = -1;
		AddTail( &playerlist, &pn->pn_Node );
		if(prevobject)
			prevobject->ob_Next = &pn->pn_Bob.bb_PaulsObject;
	}
	extraallocatedplayers += 1;
	if(pn)
		return(&pn->pn_Bob);
	else
		return(NULL);
}

struct PaulsBob * __asm AllocPlayerBullet(void)
{
	struct BulletNode *bn;
	struct PaulsObject *prevobject = NULL;

	if(!IsListEmpty(&bulletlist))
		prevobject = &((struct BulletNode *)bulletlist.lh_TailPred)->bn_Bob.bb_PaulsObject;

	if( bn = MYALLOCVEC(sizeof(struct BulletNode), MEMF_ANY|MEMF_CLEAR,"AllocPlayerBullet() BobNode") )
	{
		bn->bn_Bob.bb_PaulsObject.ob_ScreenOffset1 = -1;
		bn->bn_Bob.bb_PaulsObject.ob_ScreenOffset2 = -1;
		AddTail( &bulletlist, &bn->bn_Node );
		if(prevobject)
			prevobject->ob_Next = &bn->bn_Bob.bb_PaulsObject;
	}
	extraallocatedplayerbullets += 1;
	if(bn)
		return(&bn->bn_Bob);
	else
		return(NULL);
}

struct PaulsBob * __asm AllocBadDude(void)
{
	struct BadDudeNode *bdn;
	struct PaulsObject *prevobject = NULL;

	if(!IsListEmpty(&baddudelist))
		prevobject = &((struct BadDudeNode *)baddudelist.lh_TailPred)->bdn_Bob.bb_PaulsObject;

	if( bdn = MYALLOCVEC(sizeof(struct BadDudeNode), MEMF_ANY|MEMF_CLEAR,"AllocBadDude() BobNode") )
	{
		bdn->bdn_Bob.bb_PaulsObject.ob_ScreenOffset1 = -1;
		bdn->bdn_Bob.bb_PaulsObject.ob_ScreenOffset2 = -1;
		AddTail( &baddudelist, &bdn->bdn_Node );
		if(prevobject)
			prevobject->ob_Next = &bdn->bdn_Bob.bb_PaulsObject;
	}
	extraallocateddudes += 1;
	if(bdn)
		return(&bdn->bdn_Bob);
	else
		return(NULL);
}

void PrintPonkWarnings(void)
{
	struct ProgNode *pn;
	UBYTE namebuffer[PROGFULLNAMESIZE];
	UWORD err = LAMERR_ALLOK;
	BOOL done;

	if(recursiveactionlistaddress)
	{
		done = FALSE;
		for(pn = (struct ProgNode *)proglist.lh_Head;
				pn->pn_Node.ln_Succ && !done;
				pn = (struct ProgNode *)pn->pn_Node.ln_Succ)
		{
			if( (recursiveactionlistaddress >= (ULONG)pn->pn_Prog) &&
					(recursiveactionlistaddress < (ULONG)(pn->pn_Prog+pn->pn_ProgSize)) )
			{
				err = LAMERR_RECURSIVE_ACTIONLIST;
				UnpackASCII(pn->pn_Node.ln_Name, namebuffer, PROGFULLNAMESIZE-1);
				sprintf(errorstring, "Warning %ld: ActionList: '%s'\nDescription: Attempted to Execute a Recursive ActionList\n", err, namebuffer);
				DisplayWarning(errorstring);
				done = TRUE;
			}
		}
	}
}
/*
void DumpBadDudes(struct List *list)
{
	struct BadDudeNode *bdn;
	struct ProgNode *pn;
	struct PaulsChannel *pc;
	UWORD i,dude;
	UBYTE namebuffer[PROGFULLNAMESIZE];

	dude = 0;
	for(bdn = (struct BadDudeNode *)list->lh_Head;
			bdn->bdn_Node.ln_Succ;
			bdn = (struct BadDudeNode *)bdn->bdn_Node.ln_Succ)
	{
		printf("Dude: %ld\n",dude);
		if(bdn->bdn_Bob.bb_PaulsObject.ob_State)
		{
			pc = &bdn->bdn_Bob.bb_PaulsObject.ob_Channel1;
			for(i = 0; i <= 4; i++)
			{
				printf(" Channel %ld: ",i);
				if(pc[i].cl_UpdateCode)
				{
					if(pn = GetActionListName(pc[i].cl_SetUpProgram))
					{
						UnpackASCII(pn->pn_Node.ln_Name, namebuffer, PROGFULLNAMESIZE-1);
						printf("%s\n",namebuffer);
					}
					else
						printf("*** Program Not Found ***\n");
				}
				else
					printf("empty\n");
			}
		}
		dude++;
	}
}
*/
static struct ProgNode *GetActionListName(UBYTE *ptr)
{
	struct ProgNode *pn,*retpn = NULL;

	for(pn = (struct ProgNode *)proglist.lh_Head;
			pn->pn_Node.ln_Succ && !retpn;
			pn = (struct ProgNode *)pn->pn_Node.ln_Succ)
	{
		if( (ptr >= pn->pn_Prog) && (ptr < pn->pn_Prog+pn->pn_ProgSize) )
			retpn = pn;
	}
	return(retpn);
}

/*********   MyStrDup   *******************/
char *MyStrDup(char *source)
{
	char *dest;

	if( dest = MYALLOCVEC( GetTextLength( source ) + 1 , MEMF_ANY, "MyStrDup()") )
		strcpy(dest,source);
	return(dest);
}

/**********   SussOutVolumes   ****************/
//
// Makes sure the requested DOS volume/device/assign is present before
// trying to load anything. If not present it brings up a requester.
// Returns: TRUE - All Ok to load.
//					FALSE - Failed.

BOOL SussOutVolumes(char *fullpathname)
{
	static char volumebuffer[80]; /* Static for function sharing */
	BOOL ret = TRUE;
	BOOL volexists;
	UWORD count;
	ULONG oldpri;

	oldpri = SetTaskPri(OurTaskBase,0);

	count = 0;
	while( (fullpathname[count] != ':') && (fullpathname[count] != 0) && (count < 79) )
	{
		volumebuffer[count] = fullpathname[count];
		count++;
	}
	volumebuffer[count] = 0;

	if(fullpathname[count] == ':')
	{
		volexists = FALSE;
		while( !volexists && ret )
		{
			volexists = FindVolume( volumebuffer );
			if( !volexists )
				ret = InsertVolumeRequester(volumebuffer);
		}
	}
	SetTaskPri(OurTaskBase,oldpri);
	return(ret);
}

BOOL FindVolume( char *name )
{
	struct RootNode *rn;
	struct DosInfo *dinfo;
	struct DosList *dlist;
	BOOL ret = FALSE;

	if( DOSBase->dl_lib.lib_Version >= 36 )
	{
		/* 2.0+ version */

		if( dlist =
			LockDosList(LDF_VOLUMES|LDF_ASSIGNS|LDF_DEVICES|LDF_READ) )
		{
			if( dlist = FindDosEntry(dlist, name, LDF_VOLUMES|LDF_ASSIGNS|LDF_DEVICES) )
				ret = TRUE;
			UnLockDosList(LDF_VOLUMES|LDF_ASSIGNS|LDF_DEVICES|LDF_READ);
		}
	}
	else
	{
		/* 1.3 version. ugh. */

		Forbid();

		rn = DOSBase->dl_Root;
		dinfo = (struct DosInfo *)BADDR( rn->rn_Info );
		dlist = (struct DosList *)BADDR(dinfo->di_DevInfo);

		while( dlist && !ret )
		{
			if( dlist->dol_Type == DLT_DEVICE ||
				dlist->dol_Type == DLT_DIRECTORY ||
				dlist->dol_Type == DLT_VOLUME )
			{
				if( !stricmp( name, ((char *)BADDR(dlist->dol_Name))+1 ) )
				{
					ret = TRUE;
					break;
				}
			}
			dlist = BADDR( dlist->dol_Next );
		}
		Permit();
	}
	return ret;
}




/********   InsertVolumeRequester()   ****************/
//
// Bring up a shitty requester asking the user to insert the right volume.
// Returns: TRUE - Try Again
//					FALSE - Abort

#define SLICE_CHAR_HEIGHT 4
#define SLICE_PIXEL_HEIGHT (SLICE_CHAR_HEIGHT<<3)

BOOL InsertVolumeRequester(char *volumename)
{
	static UWORD __chip insertvolumecopper[] = {
			DMACON,		DMAF_SETCLR|DMAF_RASTER,
			BPL1MOD,	0x0000,
			BPL2MOD,	0x0000,
			BPLCON0,	0x1200,
			BPLCON1,	0x0000,
			BPLCON2,	0x0224,
			BPLCON3,	0x0C40,
			BPLCON4,	0x0011,
			DDFSTRT,	0x0038,
			DDFSTOP,	0x00D0,
			DIWSTRT,	0x2C81,
			DIWSTOP,	0x2CC1,
			FMODE,		0x0000,

			COLOR00,	0x0000,
			COLOR01,	0x0FFF,

			BPL1PTH,	0,
			BPL1PTL,	0,

			(SLICE_PIXEL_HEIGHT<<8)+0x2C01,0xFF00,
			DMACON,		DMAF_RASTER,

			0xFFFF,0xFFFE,
			0xFFFF,0xFFFE,
	};
	UBYTE	*screenbuffer,*allignedscreenbuffer;
	BOOL ret = FALSE;

	if(screenbuffer = MYALLOCVEC((SLICE_PIXEL_HEIGHT*40)+8, MEMF_CHIP|MEMF_CLEAR, "InsertVolumeBitMap"))
	{
		allignedscreenbuffer = (UBYTE *)( (((ULONG)screenbuffer) + 7) & 0xFFFFFFF8 );

		GC_InitGroovyConStruct(&InsertVolumeGroovyCon);
		InsertVolumeGroovyCon.gc_GfxMem = allignedscreenbuffer;
		InsertVolumeGroovyCon.gc_BytesPerPlane = 40;
		InsertVolumeGroovyCon.gc_BytesPerRow = 40;
		InsertVolumeGroovyCon.gc_Depth = 1;
		InsertVolumeGroovyCon.gc_ConWidth = 40;
		InsertVolumeGroovyCon.gc_ConHeight = SLICE_CHAR_HEIGHT;
		InsertVolumeGroovyCon.gc_TextFont = TopazTextFont;

		insertvolumecopper[31] = (UWORD)(((ULONG)allignedscreenbuffer)>>16);
		insertvolumecopper[33] = (UWORD)(((ULONG)allignedscreenbuffer)&0xFFFF);

		allignedscreenbuffer[0] = 0xFF;
		allignedscreenbuffer[1] = 0xFF;
		allignedscreenbuffer[2] = 0xFF;
		allignedscreenbuffer[40] = 0xFF;

		custom.cop1lc = (ULONG)insertvolumecopper;

		PrintInsertVolumeText(&InsertVolumeGroovyConDevice, "Please Insert Volume\n%s:\n\nReturn = Retry     Esc = Abort\n", volumename);

		while( !RawKeyArray[KEY_RETURN] && !RawKeyArray[KEY_ESC] && !DiskChanged)
		{
			WaitTOF();
		}
		DiskChanged = 0;
		if(RawKeyArray[KEY_ESC])
			ret = FALSE;
		else
			ret = TRUE;

		custom.dmacon = DMAF_RASTER;
		custom.cop1lc = (ULONG)&DummyCopper;

		MYFREEVEC(screenbuffer);
	}
	return(ret);
}


static int MyStrlen( char *p )
{
	int i=0;

	while( *p++ )
		i++;

	return i;
}

/* Replacements for v36+ Dos FilePart()/PathPath() routines */

STRPTR MyFilePart( STRPTR p )
{
	int i;
	i = MyStrlen( p );
	while( --i >= 0 && p[i] != ':' && p[i] != '/' )
		;
	return &p[i+1];
}


STRPTR MyPathPart( STRPTR p )
{
	int i;
	i = MyStrlen( p );

	while( --i >= 0 )
	{
		if( p[i] == ':' )
			return &p[i+1];
		if( p[i] == '/' )
			return &p[i];
	}

	return p;
}


/* these two routines copy file and path parts into buffers */
/* known bug: "//wibble" will be split into "/" and "wibble" - */
/* the last / is dropped. */

void CopyFilePart( STRPTR name, UBYTE *buf )
{
	strcpy( buf, MyFilePart( name ) );
}

void CopyPathPart( STRPTR name, UBYTE *buf )
{
	UBYTE *p,*end;

	end = MyPathPart( name );
	p = name;

	while( p != end )
	{
		*buf++ = *p++;
	}
	*buf++ = '\0';
}
