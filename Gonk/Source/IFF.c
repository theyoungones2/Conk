/*****************************************************************/
// Various routines to handle IFF Stuff.
//

#include <stdio.h>
//#include <stdarg.h>
//#include <string.h>
//#include <dos/dos.h>
//#include <dos/dostags.h>
//#include <exec/exec.h>
//#include <intuition/intuition.h>
//#include <graphics/gfxbase.h>
//#include <rexx/rxslib.h>
#include <libraries/gadtools.h>
//#include <libraries/diskfont.h>
//#include <libraries/asl.h>
#include <libraries/iffparse.h>

#include <proto/exec.h>
#include <proto/dos.h>
#include <proto/intuition.h>
#include <proto/graphics.h>
//#include <proto/diskfont.h>
#include <proto/asl.h>
//#include <clib/alib_protos.h>
//#include <proto/gadtools.h>
#include <proto/iffparse.h>

#include <global.h>
#include <ExternVars.h>
#include <ilbm.h>
#include <packer.h>
#include <HoopyVision.h>

struct BLKSHeader
{
	UBYTE	blks_Name[16];		/* Name of Block Bank (Uncompressed as yet) */
	UWORD	blks_NumOfBlocks;
	UWORD blks_Depth;
	UWORD	blks_LayoutWidth;	/* how many blocks across in the Blocks window */
													/* (20 for lores) */
	/* future expansion stuff */
	UWORD	blks_Flags;				/* See Defines Below */
	UWORD	blks_PixW;				/* width in pixels (currently 16) */
	UWORD	blks_PixH;				/* height (currently 16) */
	UWORD blks_BitMapHeight;/* How high the bitmap is to be made. */
	UWORD	blks_Reserved[3];	/* set to 0! */
};
#define BLKSB_CHUNKY 0
#define BLKSF_CHUNKY 1<<BLKSB_CHUNKY

static BOOL ScanIFF(struct IFFHandle *handle, struct List *chunklist);
static void PrintChunkList(struct List *chunklist);
static BOOL LoadCorrectChunks(struct IFFHandle *handle, STRPTR file, STRPTR drawer,
															struct List *chunklist, struct Project *originalproject);
static void SortOutBitMapSize(struct MyILBMStruct *mi);
static void SortOutScreenSettings(struct MyILBMStruct *mi,
																	struct Project *proj,
																	ULONG modeid);
static ULONG ConvertCAMG(struct MyILBMStruct *mi);
static BOOL WriteBMHDChunk( struct IFFHandle *handle, struct Project *pj);
static BOOL WriteCAMGChunk( struct IFFHandle *handle, struct Project *pj);
static BOOL WriteBODYChunk( struct IFFHandle *handle, struct Project *pj);
static BOOL WriteBLKSChunk(struct IFFHandle *handle, struct Project *pj, char *name);
static void PasteBlockToBitMap(UWORD x,UWORD y, UWORD depth,
															 UWORD *block, struct BitMap *bm);
static BOOL LoadTheAnimBrush( struct AnimBrush *ab, struct IFFHandle *handle);

/**************************************************************/
//								GENERAL IFF LOADING STUFF										//
/**************************************************************/

/***************  OpenFile()  **********************/
//
// Returns: TRUE - The File Loaded Okay;
//					FALSE - The File Failed To Load Correctly.

BOOL OpenFile(STRPTR file, STRPTR drawer, struct Project *originalproject)
{
	char *name = NULL;
	struct IFFHandle *handle = NULL;
	BOOL success = FALSE;
	struct List chunklist;
	struct ChunkNode *cn;
	BPTR doshandle;
	ULONG buffer;

	NewList(&chunklist);

	WaitForSubTaskDone(mainmsgport);

	/* String the path and file name together */
	if( name = MakeFullPathName(drawer, file) )
	{
		if(doshandle = Open(name, MODE_OLDFILE))
		{
			Read(doshandle,&buffer,4);
			Seek(doshandle,0,OFFSET_BEGINNING);
			if(buffer != ID_FORM)
			{
				if(GroovyReq("Open File","File Not IFF\nLoad as Blitz Shapes File?","Yes|Cancel") == 1)
				{
					Close(doshandle);
					doshandle = NULL;
					success = LoadShapesFile(drawer,file);
				}
			}
			else
			{
				/* File is IFF.... Well most probably is */
				if( handle = AllocIFF() )
				{
					/* open the DOS file */
					handle->iff_Stream = doshandle;
					InitIFFasDOS( handle );
					if( !OpenIFF( handle, IFFF_READ ) )
					{
						/* Scan the file to get a list of all the chunks in it */
						if( !ScanIFF(handle,&chunklist) )
							GroovyReq("File Error","File Type Not Recognised","Bummer");
						else
						{
							//PrintChunkList(&chunklist);
	
							/* Restart the file again, cos after scanning it we are at
								the end of the file, so we have to go back to the start again */
							CloseIFF( handle );
							Close( handle->iff_Stream );
							doshandle = NULL;
							FreeIFF( handle );
							handle = NULL;
							//Seek(handle->iff_Stream, 0, OFFSET_BEGINNING);
	
							handle = AllocIFF();
							handle->iff_Stream = Open( name, MODE_OLDFILE );
	
							InitIFFasDOS( handle );
							if( !OpenIFF( handle, IFFF_READ ) )
							{
								/* Load in all that we want */
								if(!(success = LoadCorrectChunks(handle, file, drawer, &chunklist, originalproject)))
									GroovyReq("Open Project","Open Project Failed","Bugger");
							}
						}
						CloseIFF( handle );
					}
					FreeIFF( handle );
				}
			}
			if(doshandle)
				Close( doshandle );
		}
		FreeVec(name);
	}
	else
		InvalidNameRequester();

	while(!IsListEmpty(&chunklist))
	{
		cn = (struct ChunkNode *)chunklist.lh_Head;
		Remove(&cn->cn_Node);
		FreeVec(cn);
	}
	return(success);
}

/*********************   ScanIFF()   **************************/
//
//  Scan through the IFF and record each new Chunks' ID and Type.
// Returns: TRUE - No Errors,
//					FALSE - Error for one reason or another.

static BOOL ScanIFF(struct IFFHandle *handle, struct List *chunklist)
{
	LONG parseerror = 0;
	BOOL abort = FALSE;
	struct ChunkNode *cn;
	struct ContextNode *text;

	NewList(chunklist);

	while( (!abort) && (parseerror != IFFERR_EOF) )
	{
		parseerror = ParseIFF(handle, IFFPARSE_RAWSTEP);

		switch(parseerror)
		{
			case 0:
				if( text = CurrentChunk(handle) )
				{
					if( cn = AllocVec( sizeof(struct ChunkNode), MEMF_ANY|MEMF_CLEAR) )
					{
						cn->cn_ID = text->cn_ID;
						cn->cn_Type = text->cn_Type;
						cn->cn_Size = text->cn_Size;
						AddTail(chunklist, &cn->cn_Node);
					}
					else
						abort = TRUE;
				}
				else
					abort = TRUE;
				break;
			case IFFERR_EOF:
			case IFFERR_EOC:
				break;
			default:
				abort = TRUE;
				break;
		}
	}
	if(abort)
	{
		while(!IsListEmpty(chunklist))
		{
			cn = (struct ChunkNode *)chunklist->lh_Head;
			Remove(&cn->cn_Node);
			FreeVec(cn);
		}
	}
	return((BOOL)!abort);
}

/*******************   LoadCorrectChunks()   ***********************/
//
//  Load the required chunks out from a file. If there is a selection of
// chunks to choose from then bring up a requester asking which chunks to
// load.

static BOOL LoadCorrectChunks(struct IFFHandle *handle, STRPTR file, STRPTR drawer,
															struct List *chunklist, struct Project *originalproject)
{
	char *name = NULL;
	static struct Palette currentpalette;
	struct Project *pj = NULL, *pj2 = NULL, *nextpj = NULL;
	UWORD chunkcount = 0;
	struct HoopyObject *hob=NULL, *currenthob=NULL,*storagehob=NULL;
	struct ChunkNode *cn = NULL;
	BOOL abort = FALSE;
	LONG parseerror = 0;
	struct File *fl = NULL;
	//BOOL done = FALSE;
	struct Palette *pal;

	currentpalette.pl_Count = 0;

	/* If No Chunks then exit */

	if( IsListEmpty(chunklist) )
		return(FALSE);

	if( !(name = MakeFullPathName(drawer, file)) )
		return(FALSE);

	if( !(fl = AllocNewFile()) )
	{
		FreeVec(name);
		return(FALSE);
	}
	SetFileName(fl, file, drawer);

	/* Go through all the chunks */
	for(cn = (struct ChunkNode *)chunklist->lh_Head;
			cn->cn_Node.ln_Succ;
			cn = (struct ChunkNode *)cn->cn_Node.ln_Succ)
	{
			/* If the file is ILBM, then load it in as is */
		if( (cn->cn_ID == ID_FORM) && (cn->cn_Type == ID_ILBM) )
		{
			abort = !LoadILBM(handle, file, drawer, originalproject, fl);
			goto TheExit;
		}

			/* If the file contains a chunk we can do something with
				then add one to the number of usable chunks */
		if(cn->cn_Type == ID_CONK)
		{
			if( (cn->cn_ID == ID_BOBS) ||
					(cn->cn_ID == ID_SPRT) ||
					(cn->cn_ID == ID_CMAP) ||
					(cn->cn_ID == ID_BLKS) ||
					(cn->cn_ID == ID_GCFG) ||
					(cn->cn_ID == ID_ANIM) )
				chunkcount++;
		}
	}

		/* If no usable chunks then exit */
	if(chunkcount == 0)
		return(FALSE);

	/* Bring up a hoppyreq asking which chunks we want to load */
	if( hob = AllocVec(sizeof(struct HoopyObject)*(2+chunkcount+1+6),MEMF_ANY|MEMF_CLEAR))
	{
		currenthob = hob;

		currenthob->ho_Type = HOTYPE_VGROUP;
		currenthob->ho_Weight = 100;
		currenthob->ho_HOFlags = HOFLG_FREEWIDTH|HOFLG_FREEHEIGHT;
		currenthob->ho_Attr0 = 1+chunkcount+1+2;	/* title+chunks+storage+space+ok/cancel */
		currenthob++;

		currenthob->ho_Type = HOTYPE_TEXT;
		currenthob->ho_Weight = 100;
		currenthob->ho_HOFlags = HOFLG_NOBORDER;
		currenthob->ho_Attr0 = (ULONG)"Load Which Chunks?";
		currenthob++;

		for(cn = (struct ChunkNode *)chunklist->lh_Head;
				cn->cn_Node.ln_Succ;
				cn = (struct ChunkNode *)cn->cn_Node.ln_Succ)
		{
			if( cn->cn_Type == ID_CONK )
			{
				switch(cn->cn_ID)
				{
					case ID_BOBS:
						currenthob->ho_Type = HOTYPE_CHECKBOX;
						currenthob->ho_Weight = 100;
						currenthob->ho_HOFlags = 0;
						currenthob->ho_Attr0 = 0;
						currenthob->ho_Text = "Bobs";
						currenthob->ho_Value = TRUE;
						currenthob->ho_Flags = PLACETEXT_RIGHT;
						currenthob++;
						break;
					case ID_SPRT:
						currenthob->ho_Type = HOTYPE_CHECKBOX;
						currenthob->ho_Weight = 100;
						currenthob->ho_HOFlags = 0;
						currenthob->ho_Attr0 = 0;
						currenthob->ho_Text = "Sprites";
						currenthob->ho_Value = TRUE;
						currenthob->ho_Flags = PLACETEXT_RIGHT;
						currenthob++;
						break;
					case ID_CMAP:
						currenthob->ho_Type = HOTYPE_CHECKBOX;
						currenthob->ho_Weight = 100;
						currenthob->ho_HOFlags = 0;
						currenthob->ho_Attr0 = 0;
						currenthob->ho_Text = "Palette";
						currenthob->ho_Value = TRUE;
						currenthob->ho_Flags = PLACETEXT_RIGHT;
						currenthob++;
						break;
					case ID_BLKS:
						currenthob->ho_Type = HOTYPE_CHECKBOX;
						currenthob->ho_Weight = 100;
						currenthob->ho_HOFlags = 0;
						currenthob->ho_Attr0 = 0;
						currenthob->ho_Text = "Blocks";
						currenthob->ho_Value = TRUE;
						currenthob->ho_Flags = PLACETEXT_RIGHT;
						currenthob++;
						break;
					case ID_GCFG:
						currenthob->ho_Type = HOTYPE_CHECKBOX;
						currenthob->ho_Weight = 100;
						currenthob->ho_HOFlags = 0;
						currenthob->ho_Attr0 = 0;
						currenthob->ho_Text = "Config";
						currenthob->ho_Value = TRUE;
						currenthob->ho_Flags = PLACETEXT_RIGHT;
						currenthob++;
						break;
					case ID_ANIM:
						currenthob->ho_Type = HOTYPE_CHECKBOX;
						currenthob->ho_Weight = 100;
						currenthob->ho_HOFlags = 0;
						currenthob->ho_Attr0 = 0;
						currenthob->ho_Text = "Anim";
						currenthob->ho_Value = TRUE;
						currenthob->ho_Flags = PLACETEXT_RIGHT;
						currenthob++;
						break;
				}
			}
		}
		storagehob = currenthob;
		currenthob->ho_Type = HOTYPE_CHECKBOX;
		currenthob->ho_Weight = 100;
		currenthob->ho_HOFlags = 0;
		currenthob->ho_Attr0 = 0;
		currenthob->ho_Text = "Storage";
		currenthob->ho_Value = TRUE;
		currenthob->ho_Flags = PLACETEXT_RIGHT;
		currenthob++;

		currenthob->ho_Type = HOTYPE_SPACE;
		currenthob->ho_Weight = 10;
		currenthob->ho_HOFlags = HOFLG_NOBORDER|HOFLG_FREEWIDTH|HOFLG_FREEHEIGHT;
		currenthob->ho_Attr0 = 0;
		currenthob->ho_Text = NULL;
		currenthob->ho_Value = 0;
		currenthob++;

		currenthob->ho_Type = HOTYPE_HGROUP;
		currenthob->ho_Weight = 100;
		currenthob->ho_HOFlags = HOFLG_NOBORDER|HOFLG_FREEWIDTH;
		currenthob->ho_Attr0 = 3;
		currenthob->ho_Text = NULL;
		currenthob->ho_Value = 0;
		currenthob++;

		currenthob->ho_Type = HOTYPE_BUTTON;
		currenthob->ho_Weight = 100;
		currenthob->ho_HOFlags = 0;
		currenthob->ho_Attr0 = 0;
		currenthob->ho_Text = "OK";
		currenthob->ho_Value = 0;
		currenthob->ho_GadgetID = 0x8000;
		currenthob++;

		currenthob->ho_Type = HOTYPE_SPACE;
		currenthob->ho_Weight = 100;
		currenthob->ho_HOFlags = HOFLG_NOBORDER|HOFLG_FREEWIDTH;
		currenthob->ho_Attr0 = 0;
		currenthob->ho_Text = NULL;
		currenthob->ho_Value = 0;
		currenthob++;

		currenthob->ho_Type = HOTYPE_BUTTON;
		currenthob->ho_Weight = 100;
		currenthob->ho_HOFlags = 0;
		currenthob->ho_Attr0 = 0;
		currenthob->ho_Text = "Cancel";
		currenthob->ho_Value = 0;
		currenthob->ho_GadgetID = 0xFFFF;
		currenthob++;

		currenthob->ho_Type = HOTYPE_END;
		currenthob->ho_Weight = 0;
		currenthob->ho_HOFlags = 0;
		currenthob->ho_Attr0 = 0;
		currenthob->ho_Text = NULL;
		currenthob->ho_Value = 0;
		currenthob->ho_GadgetID = 0;
		currenthob++;

		if(HO_SimpleDoReq(hob, "Open Project", NULL) != 0xFFFF)
		{
			cn = (struct ChunkNode *)chunklist->lh_Head;
			currenthob = hob;
			currenthob += 2;

			while( (!abort) && (parseerror != IFFERR_EOF) )
			{
				parseerror = ParseIFF(handle, IFFPARSE_RAWSTEP);

				switch(parseerror)
				{
					case 0:
						if(cn->cn_Type == ID_CONK)
						{
							if(	(cn->cn_ID == ID_BOBS) ||
									(cn->cn_ID == ID_SPRT) ||
									(cn->cn_ID == ID_CMAP) ||
									(cn->cn_ID == ID_BLKS) ||
									(cn->cn_ID == ID_GCFG) ||
									(cn->cn_ID == ID_ANIM) )
							{
								if(currenthob->ho_Value)
								{
									switch(cn->cn_ID)
									{
										case ID_CMAP:
											abort = !LoadCMAPIntoPalette(&currentpalette,handle);
											break;
										case ID_BOBS:
											if( (pj = GetEmptyProject(fl)) && (BlockProjectWindows(pj)) )
											{
												if(currentpalette.pl_Count != 0)
													memcpy( &pj->pj_Palette,&currentpalette,sizeof(struct Palette) );
												abort = !LoadBobs(pj,handle,name);
												if(!abort) pj->pj_IsFile = TRUE;
												RefreshProjectName(pj);
											}
											break;
										case ID_SPRT:
											if( (pj = GetEmptyProject(fl)) && (BlockProjectWindows(pj)) )
											{
												if(currentpalette.pl_Count != 0)
													memcpy( &pj->pj_Palette, &currentpalette, sizeof(struct Palette) );
												abort = !LoadSprites(pj,handle,name);
												if(!abort) pj->pj_IsFile = TRUE;
												RefreshProjectName(pj);
											}
											break;
										case ID_BLKS:
											if( (pj = GetEmptyProject(fl)) && (BlockProjectWindows(pj)) )
											{
												if(currentpalette.pl_Count != 0)
													memcpy( &pj->pj_Palette, &currentpalette, sizeof(struct Palette) );
												abort = !LoadBlocks(pj, handle, name, FALSE);
												if(!abort) pj->pj_IsFile = TRUE;
												RefreshProjectName(pj);
											}
											break;
										case ID_GCFG:
											abort = !LoadGCFGChunk(handle);
											break;
										case ID_ANIM:
											if(pj)
												abort = !LoadAnim(pj,handle,name);
											break;
									}
								}
								else
								{
									/* The user didn't want to load it so if he had storage
										turned on then just store the chunk	*/
									if(storagehob->ho_Value)
										abort = !LoadCrapChunk(handle, fl, cn->cn_ID, cn->cn_Size);
								}
								currenthob++;
							}
							else
							{
								if( (cn->cn_ID != ID_FORM) && (storagehob->ho_Value) )
								{
									abort = !LoadCrapChunk(handle, fl, cn->cn_ID, cn->cn_Size);
								}
							}
						}
						cn = (struct ChunkNode *)cn->cn_Node.ln_Succ;
						break;
					case IFFERR_EOF:
					case IFFERR_EOC:
						break;
					default:
						abort = TRUE;
						break;
				}
			}

			/* If the user selected to load a CMAP only, then we just
			 * want to load the CMAP into the original project. */
			if( (currentpalette.pl_Count != 0) && (originalproject) && (!abort) && (!pj) )
			{
				memcpy( &originalproject->pj_Palette.pl_Colours, &currentpalette.pl_Colours, currentpalette.pl_Count*3 );
				originalproject->pj_Palette.pl_Count = 1<<originalproject->pj_Depth;
				pj = originalproject;
			}

			/* Check for any projects that I may have opened with no bitmap. If
			 * I did open any, then just shut them down */
			pj2 = (struct Project *)projects.lh_Head;
			while(pj2->pj_Node.ln_Succ)
			{
				nextpj = (struct Project *)pj2->pj_Node.ln_Succ;
				if(!pj2->pj_MainBitMap)
				{
					pal = CloseProject(pj2);
					if(pal)
						InstallNewGlobPalette(pal);
					if(pj2 == pj) pj = NULL;
				}
				pj2 = nextpj;
			}

			if( (!abort) && (pj) )
				UseProjSettings(pj);
		}
		FreeVec(hob);
	}
	else
		abort = TRUE;

TheExit:
	if(name) FreeVec(name);
	if(fl && !fl->fl_FirstProject)
		FreeFile(fl);
	return((BOOL)!abort);
}

static void PrintChunkList(struct List *chunklist)
{
	struct ChunkNode *cn;
	char idbuf[5],typebuf[5];

	for(cn = (struct ChunkNode *)chunklist->lh_Head;
			cn->cn_Node.ln_Succ;
			cn = (struct ChunkNode *)cn->cn_Node.ln_Succ)
	{
		printf("ID:%s    Type:%s\n",IDtoStr(cn->cn_ID,idbuf),IDtoStr(cn->cn_Type,typebuf) );
	}
}

BOOL LoadCrapChunk(struct IFFHandle *handle, struct File *fl, ULONG chunkid, ULONG size)
{
	BOOL ret = FALSE;
	struct CrapChunk *cc;

	if( cc = AllocVec(sizeof(struct CrapChunk),MEMF_ANY|MEMF_CLEAR) )
	{
		AddTail(&fl->fl_CrapChunks, &cc->cc_Node);
		cc->cc_ChunkID = chunkid;
		if(cc->cc_ChunkData = AllocVec(size,MEMF_ANY|MEMF_CLEAR))
		{
			cc->cc_ChunkSize = size;
			if(ReadChunkBytes(handle,cc->cc_ChunkData,size) == size)
				ret = TRUE;
		}
	}
	return(ret);
}

BOOL WriteCrapChunks(struct File *fl, struct IFFHandle *handle)
{
	BOOL abort = FALSE;
	struct CrapChunk *cc;
	
	for(cc = (struct CrapChunk *)fl->fl_CrapChunks.lh_Head;
			(cc->cc_Node.ln_Succ) && (!abort);
			cc = (struct CrapChunk *)cc->cc_Node.ln_Succ)
	{
		if(cc->cc_ChunkData)
		{
			if( !PushChunk( handle, 0, cc->cc_ChunkID, cc->cc_ChunkSize ) )
			{
				if( WriteChunkBytes(handle, cc->cc_ChunkData, cc->cc_ChunkSize) != cc->cc_ChunkSize)
					abort = TRUE;
			}
		}
		if(!abort)
		{
			if( PopChunk(handle) )
				abort = TRUE;
		}
	}
	return((BOOL)!abort);
}

BOOL LoadCMAPIntoPalette(struct Palette *pal, struct IFFHandle *handle)
{
	struct	ContextNode		*text;
	LONG									 size;
	UBYTE									*colourlist		=		NULL;
	BOOL									 ret					=		FALSE;

	memcpy(pal, globpalette, sizeof(struct Palette));

	if( text = CurrentChunk(handle) )
	{
		size = text->cn_Size;
		if(colourlist = AllocVec(size,MEMF_ANY))
		{
			if(ReadChunkBytes(handle,colourlist,size) == size)
			{
				if(SussOutCMap(size/3,TRUE,colourlist,pal,NULL))
					ret = TRUE;
			}
			FreeVec(colourlist);
		}
	}
	return(ret);
}

BOOL LoadCMAP(struct Project *pj, struct IFFHandle *handle, BOOL UsePalette)
{
	struct ContextNode *text;
	LONG size;
	UBYTE *colourlist=NULL;
	BOOL ret = FALSE;
	UWORD depth = 4;

	if( text = CurrentChunk(handle) )
	{
		size = text->cn_Size;
		if(colourlist = AllocVec(size,MEMF_ANY))
		{
			if(ReadChunkBytes(handle,colourlist,size) == size)
			{
				if(SussOutCMap(size/3,TRUE,colourlist,&pj->pj_Palette,NULL))
				{
					if(UsePalette)
					{
						if(!pj->pj_MainBitMap)
						{
							switch(size/3)
							{
								case 2:
									depth = 1;
									break;
								case 4:
									depth = 2;
									break;
								case 8:
									depth = 3;
									break;
								case 16:
									depth = 4;
									break;
								case 32:
									depth = 5;
									break;
								case 64:
									depth = 6;
									break;
								case 128:
									depth = 7;
									break;
								case 256:
									depth = 8;
									break;
							}
							pj->pj_Depth = depth;
						}
						UseProjSettings(pj);
					}
					ret = TRUE;
				}
			}
			FreeVec(colourlist);
		}
	}
	return(ret);
}

/**************************************************************/
//										ILBM STUFF															//
/**************************************************************/

/***************************   LoadILBM()   **************************/
//
//  Load An ILBM into the given Project.
// Returns: TRUE - Loaded Fine,
//					FALSE - Failed To Load.

BOOL LoadILBM(struct IFFHandle *handle, char *file, char *drawer,
							struct Project *originalproject, struct File *fl)
{
	struct Project *pj = NULL;
	struct MyILBMStruct *mi;
	struct BitMapHeader *bmhd;
	struct BitMap *ubm=NULL,*mskbm=NULL;
	struct RastPort *mrp=NULL;
	LONG	numofcolours;
	ULONG	modeid				=	0;
	static struct HoopyObject hob[] =
	{
		{ HOTYPE_VGROUP, 100, HOFLG_FREEWIDTH|HOFLG_FREEHEIGHT,5,0,0,0, NULL, 0,0,0,0,0 },
			{ HOTYPE_TEXT, 100, HOFLG_NOBORDER, (ULONG)"Load Which Chunks?",0,0,0, NULL,0,0,0,0,0 },
			{ HOTYPE_CHECKBOX, 100, 0, 0,0,0,0, "Palette",TRUE,0,PLACETEXT_RIGHT,0,0 },
			{ HOTYPE_CHECKBOX, 100, 0, 0,0,0,0, "Image",TRUE,0,PLACETEXT_RIGHT,0,0 },
			{ HOTYPE_SPACE, 10, HOFLG_NOBORDER|HOFLG_FREEWIDTH|HOFLG_FREEHEIGHT, 0,0,0,0, NULL,0,0,0,0,0 },
			{ HOTYPE_HGROUP, 100, HOFLG_NOBORDER|HOFLG_FREEWIDTH,3,0,0,0, NULL, 0,0,0,0,0 },
				{ HOTYPE_BUTTON, 100, 0, 0,0,0,0, "OK",0,0x8000,0,0,0 },
				{ HOTYPE_SPACE, 100, HOFLG_NOBORDER|HOFLG_FREEWIDTH, 0,0,0,0, NULL,0,0,0,0,0 },
				{ HOTYPE_BUTTON, 100, 0, 0,0,0,0, "Cancel",0,0xFFFF,0,0,0 },
		{HOTYPE_END}
	};

	if( !(mi=LoadnScanILBM(handle)) )
		return(FALSE);

	switch(mi->mis_Error)
	{
		case MYIFFERR_MEMORY:
			GroovyReq(Txt_EditorError,Txt_NotEnoughMemory,Txt_Abort);
			goto ErrorReturn;
			break;
		case MYIFFERR_NOTILBMFILE:
			GroovyReq(Txt_EditorError,Txt_FileNotILBM,Txt_Abort);
			goto ErrorReturn;
			break;
	}
		/* Check that we have all the crutial chunks, if we don't then
			 we can just stop right here and now */

	if(!mi->mis_BMHD)
	{
		MissingILBMChunkReq(file,"BMHD");
		goto ErrorReturn;
	}

	if(!mi->mis_CMAP)
	{
		MissingILBMChunkReq(file,"CMAP");
		goto ErrorReturn;
	}

	if(!mi->mis_BODY)
	{
		MissingILBMChunkReq(file,"BODY");
		goto ErrorReturn;
	}

	bmhd = mi->mis_BMHD->sp_Data;

	//printf("Page Width: %d  Page Height: %d  Depth: %d\n",
	//				bmhd->bmh_PageWidth, bmhd->bmh_PageHeight, bmhd->bmh_Depth);
	//printf("Width: %d  Height: %d  Left: %d  Top: %d\n",
	//			bmhd->bmh_Width,bmhd->bmh_Height,bmhd->bmh_Left,bmhd->bmh_Top);
	
	//printf("X Aspect: %d  Y Aspect: %d\n",
	//				bmhd->bmh_XAspect, bmhd->bmh_YAspect);
	
	//printf("Transparent Colour: %d\n",bmhd->bmh_Transparent);
	
	//printf("Mask: %d\n",bmhd->bmh_Masking);

	/* Right Check to see what bits the person wants to load in */

	if( HO_SimpleDoReq(hob, "Load ILBM", NULL) == 0xFFFF )
		goto ErrorReturn;

	if(hob[3].ho_Value)
	{
		/* If we get here, then we have all the relevant chunks of data
			that we need, and the user wants to load the image, so there's
			really only one more thing that can go wrong and that's if it
			can't allocate the bitmaps. Firstly, dealloc the old bitmaps
			then alloc the New Bitmaps */

		if(bmhd->bmh_Depth > 8)
		{
			GroovyReq("Load ILBM","Depth too High\nMax is 8 BitPlanes","Continue");
			goto ErrorReturn;
		}

		if( !(pj = GetEmptyProject(fl)) )
		{
			GroovyReq("Load ILBM","Can't Open New Project","Abort");
			goto ErrorReturn;
		}

		if(!BlockProjectWindows(pj))
			goto ErrorReturn;

		DeallocProjBitMaps(pj);

		SortOutBitMapSize(mi);

		if( !(mrp = AllocBitMapRPort(mi->mis_BitMapWidth,mi->mis_BitMapHeight,bmhd->bmh_Depth)) )
		{
			DeallocProjBitMaps(pj);
			SetUpDefaultBitMaps(pj);
			GroovyReq("Load ILBM",Txt_NotEnoughMemory,Txt_Abort);
			goto ErrorReturn;
		}

		modeid = ConvertCAMG(mi);

		/* If proj is not ham, then we need an undo bitmap to do any editing */
		if( !(modeid & HAM) )
		{
			if( !(ubm = CreateBitMap(mi->mis_BitMapWidth,mi->mis_BitMapHeight,bmhd->bmh_Depth,TRUE)) )
			{
				pj->pj_Edable = FALSE;
				GroovyReq(Txt_EditorError,"Can't allocate BitMap\nFile not Editable.","Continue");
			}
		}

		if(bmhd->bmh_Masking == mskHasMask)
			mskbm = CreateBitMap(mi->mis_BitMapWidth,mi->mis_BitMapHeight,1,FALSE);

		if( ((bmhd->bmh_Masking == mskHasMask) && (!mskbm)) )
		{
			DeallocProjBitMaps(pj);
			SetUpDefaultBitMaps(pj);
			GroovyReq(Txt_EditorError,Txt_NotEnoughMemory,Txt_Abort);
			goto ErrorReturn;
		}

		pj->pj_Width=mi->mis_BitMapWidth;
		pj->pj_Height=mi->mis_BitMapHeight;
		pj->pj_Depth=bmhd->bmh_Depth;
	
		pj->pj_MainRPort=mrp;
		pj->pj_MainBitMap=mrp->BitMap;
		pj->pj_UndoBitMap=ubm;
		pj->pj_MaskBitMap=mskbm;
		NewEditType(pj,EDTY_FRAME,pj->pj_Depth);

		UncompILBM(bmhd,mi->mis_BODY,pj->pj_MainBitMap,mskbm,mi->mis_Handle);
		if(ubm)
		{
			BltBitMap(pj->pj_MainBitMap,0,0,
								pj->pj_UndoBitMap,0,0,pj->pj_Width,pj->pj_Height,
								0xC0,0xFF,NULL);
		}

		if(hob[2].ho_Value)
		{	
			/* Do CMAP Stuff */
			numofcolours = mi->mis_CMAP->sp_Size / 3;
			SussOutCMap(numofcolours,(bmhd->bmh_Flags & BMHDF_CMAPOK),mi->mis_CMAP->sp_Data,
									&pj->pj_Palette,NULL);
		}
		pj->pj_Palette.pl_Count = 1<<pj->pj_Depth;

		pj->pj_IsFile = TRUE;
		RefreshProjectName(pj);
	}
	else
	{
		if( (hob[2].ho_Value) && (originalproject) )
		{	
			/* Do CMAP Stuff */
			numofcolours = mi->mis_CMAP->sp_Size / 3;
			SussOutCMap(numofcolours,(bmhd->bmh_Flags & BMHDF_CMAPOK),mi->mis_CMAP->sp_Data,
									&originalproject->pj_Palette, NULL);
			originalproject->pj_Palette.pl_Count = 1<<originalproject->pj_Depth;
			pj = originalproject;
		}
		pj->pj_Palette.pl_Count = 1<<pj->pj_Depth;
	}

	SortOutScreenSettings(mi,pj,modeid);

	FreeLoadnScanILBM(mi);

	if(pj) UseProjSettings(pj);

	return(TRUE);

ErrorReturn:
	FreeLoadnScanILBM(mi);
	return(FALSE);
}

/*************   LoadnScanILBM   ******************/
// Given an iffhandle it will scan it. All
// data is stored in my own little struct (MyILBMStruct)
// This includes the iff handle, error codes,...

struct MyILBMStruct *LoadnScanILBM(struct IFFHandle *handle)
{
	struct MyILBMStruct	*mi						= NULL;
	struct StoredProperty *storedprop	= NULL;
	struct CollectionItem *collectitem = NULL;
	LONG error;

	if(!(mi=AllocVec(sizeof(struct MyILBMStruct),MEMF_ANY|MEMF_CLEAR)) )
		return(NULL);

	mi->mis_Handle=handle;

	/* These are all the chunks that we can do something with */
	PropChunk(mi->mis_Handle, ID_ILBM, ID_BMHD);
	PropChunk(mi->mis_Handle, ID_ILBM, ID_CMAP);
	PropChunk(mi->mis_Handle, ID_ILBM, ID_CAMG);
	PropChunk(mi->mis_Handle, ID_ILBM, ID_GRAB);
	CollectionChunk(mi->mis_Handle, ID_ILBM, ID_CRNG);
	CollectionChunk(mi->mis_Handle, ID_ILBM, ID_DRNG);
	StopChunk(mi->mis_Handle, ID_ILBM, ID_BODY);

	error = ParseIFF(mi->mis_Handle, IFFPARSE_SCAN);
	if(error)
	{
		mi->mis_Error = MYIFFERR_NOTILBMFILE;
		return(mi);
	}
	mi->mis_BODY = CurrentChunk(mi->mis_Handle);
	if( storedprop = FindProp(mi->mis_Handle, ID_ILBM, ID_BMHD) )	mi->mis_BMHD = storedprop;
	if( storedprop = FindProp(mi->mis_Handle, ID_ILBM, ID_CMAP) )	mi->mis_CMAP = storedprop;
	if( storedprop = FindProp(mi->mis_Handle, ID_ILBM, ID_CAMG) )	mi->mis_CAMG = storedprop;
	if( storedprop = FindProp(mi->mis_Handle, ID_ILBM,ID_GRAB) )	mi->mis_GRAB = storedprop;
	if( collectitem = FindCollection(mi->mis_Handle, ID_ILBM, ID_CRNG) )	mi->mis_CRNG = collectitem;
	if( collectitem = FindCollection(mi->mis_Handle, ID_ILBM, ID_DRNG) )	mi->mis_DRNG = collectitem;

	return(mi);
}

void FreeLoadnScanILBM(struct MyILBMStruct *mi)
{
	FreeVec(mi);
}

static void SortOutBitMapSize(struct MyILBMStruct *mi)
{
	struct BitMapHeader *bmhd;

	bmhd = mi->mis_BMHD->sp_Data;

	if(bmhd->bmh_PageWidth >= bmhd->bmh_Width)
		mi->mis_BitMapWidth = bmhd->bmh_PageWidth;
	else
		mi->mis_BitMapWidth = bmhd->bmh_Width;

	if(bmhd->bmh_PageHeight >= bmhd->bmh_Height)
		mi->mis_BitMapHeight = bmhd->bmh_PageHeight;
	else
		mi->mis_BitMapHeight = bmhd->bmh_Height;
}


static void SortOutScreenSettings(struct MyILBMStruct *mi,
																	struct Project *pj,
																	ULONG modeid)
{
	pj->pj_DisplayID = modeid;
	if(modeid & HAM)
		pj->pj_Edable = FALSE;
	UseProjSettings(pj);
}

/*******************    ConvertCAMG   **********************/
//
//  Returns ModeID displayable on this computer. Sorts out crap like
// mode not available and such like.
// NOTE:
// 1) mi->mis_BMHD structure must be initialized prior to this call.
// 2) This routine returns the straight Display ID as it sees it. So
//    it is valid to return stuff like Extra_HalfBrite and HAM modes.

static ULONG ConvertCAMG(struct MyILBMStruct *mi)
{
	UWORD  wide,high,deep;
	ULONG modeid = 0L;
	struct BitMapHeader		*bmhd;
	BOOL	gotcamg = FALSE;
	BOOL	modeavail = TRUE;
	char	*reqgadgets;
	UWORD	reqret = NULL;

	bmhd = (struct BitMapHeader *)mi->mis_BMHD->sp_Data;

	wide = bmhd->bmh_PageWidth;
	high = bmhd->bmh_PageHeight;
	deep = bmhd->bmh_Depth;

	/* Grab CAMG's idea of the viewmodes. */

	if( modeid = (* (ULONG *)mi->mis_CAMG->sp_Data) )
	{
		gotcamg = TRUE;

		/* knock bad bits out of old-style 16-bit viewmode CAMGs */

		if((!(modeid & MONITOR_ID_MASK))||
			((modeid & EXTENDED_MODE)&&(!(modeid & 0xFFFF0000))))
				modeid &= (~(EXTENDED_MODE|SPRITES|GENLOCK_AUDIO|GENLOCK_VIDEO|VP_HIDE));

		//D(bug("Filter1: CAMG now $%08lx\n",modeid));

		/* check for bogus CAMG like DPaintII brushes
		 * with junk in upper word and extended bit
		 * not set in lower word.
		 */

		if((modeid & 0xFFFF0000)&&(!(modeid & 0x00001000)))
		{
			modeid = 0L;
			gotcamg=FALSE;
		}

		if( ModeNotAvailable(modeid) )
			modeavail = FALSE;

		//D(bug("Filter2: CAMG is %s\n", sp ? "OK" : "NOT OK"));
	}

	if( (!gotcamg) || (!modeavail) )
	{
		//printf("Making Custom DisplayID.\n");
		/* No CAMG (or bad CAMG) present; use computed modes. */

		modeid = modeid & (EXTRA_HALFBRITE|HAM);
		if (wide >= 640)
			modeid |= HIRES;
		if (high >= 400)
			modeid |= LACE;

		/*  If we have 6 bit planes and no clues as to weather it's ham or not then
		 * we should ask the user. */
		if( (deep == 6) && ( (modeid & (EXTRA_HALFBRITE|HAM)) == 0) )
		{
			if( GfxBase->ChipRevBits0 & (GFXF_AA_ALICE|GFXF_AA_LISA) )
				reqgadgets = "HAM|6BP|EHB";
			else
				reqgadgets = "HAM|EHB";
			reqret = GroovyReq("ConStruction Kit","File Missing CAMG Chunk\nLoad As",reqgadgets);
			switch(reqret)
			{
				case 0:
					modeid |= EXTRA_HALFBRITE;
					break;
				case 1:
					modeid |= HAM;
					break;
				case 2:
					/* This is standard 6BP screen */
					break;
			}
		}
	}

	return(modeid);
}


/**************  SaveILBM  **************/
//
// Returns TRUE if operation is successful.
//

BOOL SaveILBM(struct Project *pj, struct IFFHandle *handle)
{
	BOOL success = FALSE;
	int palettetype;
	UWORD col;

	if( !PushChunk( handle, ID_ILBM, ID_FORM, IFFSIZE_UNKNOWN ) )
	{
		palettetype = CheckPalettes(pj);
		if(palettetype)
		{
			if( WriteBMHDChunk( handle, pj ) )
			{
				switch(palettetype)
				{
					case 1:
						success = WriteCMAPChunk( handle, &pj->pj_Palette);
						break;
					case 2:
						success = WriteCMAPChunk( handle, globpalette);
						break;
					case 3:
						success = WriteCMAPChunk( handle, globpalette);
						for(col = 0; col < 256; col++)
						{
							pj->pj_Palette.pl_Colours[col].r = globpalette->pl_Colours[col].r;
							pj->pj_Palette.pl_Colours[col].g = globpalette->pl_Colours[col].g;
							pj->pj_Palette.pl_Colours[col].b = globpalette->pl_Colours[col].b;
						}
						break;
				}
				if( success && WriteCAMGChunk( handle, pj ) )
				{
					if( WriteBODYChunk( handle, pj) )
					{
						success = TRUE;
					}
				}
			}
		}
		PopChunk( handle );
	}
	return( success );
}

static BOOL WriteBMHDChunk( struct IFFHandle *handle, struct Project *pj)
{
	struct BitMapHeader bmh;
	BOOL success = FALSE;
	ULONG size;

	size = sizeof(bmh);

	if( !PushChunk( handle, 0, ID_BMHD, size ) )
	{
		bmh.bmh_Width = pj->pj_Width;
		bmh.bmh_Height = pj->pj_Height;
		bmh.bmh_Left = 0;
		bmh.bmh_Top = 0;
		bmh.bmh_Depth = pj->pj_Depth;
		bmh.bmh_Masking = 0;
		bmh.bmh_Compression = cmpByteRun1;
		bmh.bmh_Flags = BMHDB_CMAPOK;
		bmh.bmh_Transparent = 0;
		bmh.bmh_XAspect = 1;
		bmh.bmh_YAspect = 1;
		bmh.bmh_PageWidth = pj->pj_Width;
		bmh.bmh_PageHeight = pj->pj_Height;

		if( WriteChunkBytes( handle, &bmh, size ) == size )
			success = TRUE;
		/* tell IFFParse to clean up the ends */
		if( PopChunk( handle ) ) success = FALSE;
	}
	return( success );
}

BOOL WriteCMAPChunk( struct IFFHandle *handle, struct Palette *pal)
{
	ULONG size;
	BOOL success = FALSE;

	/* figure out the amount of data we want to write */
	size = pal->pl_Count * 3;

	if( !PushChunk( handle, ID_CONK, ID_CMAP, size ) )
	{
		/* copy out the RGB triplets */
		if( WriteChunkBytes( handle, pal->pl_Colours, size ) == size )
			success = TRUE;
		/* tell IFFParse to tie up the ends */
		if( PopChunk( handle ) ) success = FALSE;
	}
	return( success );
}

static BOOL WriteCAMGChunk( struct IFFHandle *handle, struct Project *pj)
{
	ULONG size;
	BOOL success = FALSE;

	/* figure out the amount of data we want to write */
	size = 4;

	if( !PushChunk( handle, 0, ID_CAMG, size ) )
	{
		if( WriteChunkBytes( handle, &glob.cg_ScrConfig.sg_DisplayID, size ) == size )
			success = TRUE;
		/* tell IFFParse to tie up the ends */
		if( PopChunk( handle ) ) success = FALSE;
	}
	return( success );
}

static BOOL WriteBODYChunk( struct IFFHandle *handle, struct Project *pj)
{
	BOOL					 abort = FALSE;
	struct BitMap	*bm;
	UWORD					 plane,line;
	UBYTE					*destbuffer,*destptr,*srcptr;
	ULONG					 destbuffersize, packedsize, rowsize;

	bm = pj->pj_MainBitMap;
	rowsize = ((pj->pj_Width+15)>>3)&0xFFFFFFFE;

	destbuffersize = MaxPackedSize(rowsize);
	if(destbuffer = AllocVec(destbuffersize,MEMF_ANY|MEMF_CLEAR) )
	{
		if( !PushChunk( handle, 0, ID_BODY, IFFSIZE_UNKNOWN ) )
		{
			for(line = 0; ((line<pj->pj_Height) && (!abort)); line++)
			{
				for(plane = 0; ( (plane<pj->pj_Depth) && (!abort)); plane++)
				{
					srcptr = bm->Planes[plane]+(bm->BytesPerRow*line);
					destptr = destbuffer;
					packedsize = PackRow(&srcptr, &destptr, rowsize);
					if( WriteChunkBytes( handle, destbuffer, packedsize ) != packedsize)
						abort = TRUE;
				}
			}
			if( PopChunk( handle ) )
				abort = TRUE;
		}
		FreeVec(destbuffer);
	}
	else
		abort = TRUE;

	return((BOOL)!abort);
}

/**********************************************************************/
//															BLOCKS																//
/**********************************************************************/

BOOL LoadBlocks(struct Project *pj, struct IFFHandle *handle, char *name, BOOL usepalette)
{
	UWORD bmwidth,bmheight,bmdepth,blkswide,blkshigh,x,y,x2,y2,count;
	UWORD	block[16*8];
	UBYTE	block2[2*16*8];
	BOOL abort = FALSE;
	LONG blksize;
	struct ProjectWindow *pw;
	struct Palette *pal;
	struct BLKSHeader blks;
	struct BitMap blockbm;

	if(ReadChunkBytes(handle,&blks,sizeof(blks)) != sizeof(blks) )
		return(FALSE);

	MakePackable(blks.blks_Name);
	Mystrncpy(pj->pj_BankName, blks.blks_Name, 16);

	blockbm.BytesPerRow = 2*blks.blks_Depth;
	blockbm.Rows = 16;
	blockbm.Flags = 0;
	blockbm.Depth = blks.blks_Depth;
	blockbm.Planes[0] = (UBYTE *)&block[0];
	blockbm.Planes[1] = (UBYTE *)&block[1];
	blockbm.Planes[2] = (UBYTE *)&block[2];
	blockbm.Planes[3] = (UBYTE *)&block[3];
	blockbm.Planes[4] = (UBYTE *)&block[4];
	blockbm.Planes[5] = (UBYTE *)&block[5];
	blockbm.Planes[6] = (UBYTE *)&block[6];
	blockbm.Planes[7] = (UBYTE *)&block[7];

	//printf("Load Blocks: NumOfBlocks = %d\n",blks.blks_NumOfBlocks);

	blkswide = blks.blks_LayoutWidth;
	bmwidth = blkswide*blks.blks_PixW;
	blkshigh = (blks.blks_NumOfBlocks/blkswide);
	bmheight = blks.blks_BitMapHeight;		//blkshigh*blks.blks_PixH;
	bmdepth = blks.blks_Depth;
	if(blks.blks_Flags & BLKSF_CHUNKY)
		blksize = blks.blks_PixW * blks.blks_PixH;
	else
		blksize = (blks.blks_PixW / 8) * blks.blks_PixH * blks.blks_Depth;

	DeallocProjBitMaps(pj);

	if( (pj->pj_MainRPort = AllocBitMapRPort(bmwidth,bmheight,bmdepth)) &&
			(pj->pj_UndoBitMap = CreateBitMap(bmwidth,bmheight,bmdepth,TRUE)) )
	{
		pj->pj_MainBitMap = pj->pj_MainRPort->BitMap;
		pj->pj_Width = bmwidth;
		pj->pj_Height = bmheight;
		pj->pj_Depth = bmdepth;
		NewEditType(pj,EDTY_BLOCKS,pj->pj_Depth);

		for(y = 0; ((y<blkshigh) && (!abort)); y++)
		{
			for(x = 0; ((x<blkswide) && (!abort)); x++)
			{
				if(blks.blks_Flags & BLKSF_CHUNKY)
				{
					if( ReadChunkBytes(handle, block2, blksize) == blksize)
					{
						count = 0;
						for(y2 = 0; y2 < 16; y2++)
						{
							for(x2 = 0; x2 < 16; x2++)
							{
								PutPixelColour(&blockbm, x2, y2, block2[count++]);
							}
						}
					}
					else
						abort = TRUE;
				}
				else
				{
					if( ReadChunkBytes(handle,block,blksize) != blksize)
						abort = TRUE;
				}
				if(!abort)
					PasteBlockToBitMap(x*16, y*16, bmdepth, block, pj->pj_MainBitMap);
			}
		}
		BltBitMap(pj->pj_MainBitMap,0,0,
							pj->pj_UndoBitMap,0,0,bmwidth,bmheight,0xC0,0xFF,NULL);

		if(usepalette)
		{
			for(pw = pj->pj_ProjectWindows;	pw;	pw = pw->pw_NextProjWindow)
				RedisplayProjWindow(pw,TRUE);
			if(lastprojectwindow)
				DrawSelector(lastprojectwindow, lastprojectwindow->pw_SelX0, lastprojectwindow->pw_SelY0);
		}
	}
	else
	{
		DeallocProjBitMaps(pj);
		GroovyReq("Gonk","Not Enough Memory\nTerminating Project","Continue");
		if( pal = CloseProject(pj) )
			InstallNewGlobPalette(pal);
	}
	return((BOOL)!abort);
}

static void PasteBlockToBitMap(UWORD x,UWORD y, UWORD depth,
															 UWORD *block, struct BitMap *bm)
{
	ULONG bmoffset;
	UWORD line,plane;

	bmoffset = (bm->BytesPerRow*y)+(x/8);

	for(line = 0; line<16; line++)
	{
		for(plane = 0; plane<depth; plane++)
		{
			*(UWORD *)(bm->Planes[plane]+bmoffset) = *block++;
		}
		bmoffset += bm->BytesPerRow;
	}
}


BOOL SaveBlocks(char *name, struct Project *pj, struct IFFHandle *handle)
{
	BOOL success = FALSE;
	int palettetype;
	UWORD col;
	static struct HoopyObject hob[] =
	{
		{ HOTYPE_VGROUP, 100, HOFLG_FREEWIDTH|HOFLG_FREEHEIGHT,6,0,0,0, NULL, 0,0,0,0,0 },
			{ HOTYPE_TEXT, 100, HOFLG_NOBORDER, (ULONG)"Save Chunks?",0,0,0, NULL,0,0,0,0,0 },
			{ HOTYPE_CHECKBOX, 100, 0, 0,0,0,0, "Config",FALSE,0,PLACETEXT_RIGHT,0,0 },
			{ HOTYPE_CHECKBOX, 100, 0, 0,0,0,0, "Palette",TRUE,0,PLACETEXT_RIGHT,0,0 },
			{ HOTYPE_CHECKBOX, 100, 0, 0,0,0,0, "Blocks",TRUE,0,PLACETEXT_RIGHT,0,0 },
			{ HOTYPE_SPACE, 10, HOFLG_NOBORDER|HOFLG_FREEWIDTH|HOFLG_FREEHEIGHT, 0,0,0,0, NULL,0,0,0,0,0 },
			{ HOTYPE_HGROUP, 100, HOFLG_NOBORDER|HOFLG_FREEWIDTH,3,0,0,0, NULL, 0,0,0,0,0 },
				{ HOTYPE_BUTTON, 100, 0, 0,0,0,0, "OK",0,0x8000,0,0,0 },
				{ HOTYPE_SPACE, 100, HOFLG_NOBORDER|HOFLG_FREEWIDTH, 0,0,0,0, NULL,0,0,0,0,0 },
				{ HOTYPE_BUTTON, 100, 0, 0,0,0,0, "Cancel",0,0xFFFF,0,0,0 },
		{HOTYPE_END}
	};

	if(stricmp(pj->pj_BankName,"-<UNTITLED>-") == 0)
		SetChunkName(pj);

	hob[2].ho_Value = 0;		/* Config */
	hob[3].ho_Value = 1;		/* Palette */
	hob[4].ho_Value = 1;		/* Blocks */
	if(	HO_SimpleDoReq( hob, "Save Blocks", NULL) != 0xFFFF)
	{
		palettetype = CheckPalettes(pj);
		if(palettetype)
		{
			success = TRUE;
			if(hob[2].ho_Value)										/* Config */
				success = WriteGCFGChunk( handle, &glob);
			if(success && hob[3].ho_Value)					/* Palette */
			{
				switch(palettetype)
				{
					case 1:
						success = WriteCMAPChunk( handle, &pj->pj_Palette);
						break;
					case 2:
						success = WriteCMAPChunk( handle, globpalette);
						break;
					case 3:
						success = WriteCMAPChunk( handle, globpalette);
						for(col = 0; col < 256; col++)
						{
							pj->pj_Palette.pl_Colours[col].r = globpalette->pl_Colours[col].r;
							pj->pj_Palette.pl_Colours[col].g = globpalette->pl_Colours[col].g;
							pj->pj_Palette.pl_Colours[col].b = globpalette->pl_Colours[col].b;
						}
						break;
				}
			}
			if(success && hob[4].ho_Value)
				success = WriteBLKSChunk(handle, pj, name);
		}
	}
	return( success );
}

static BOOL WriteBLKSChunk(struct IFFHandle *handle, struct Project *pj, char *name)
{
	UWORD blockwidth,blockheight,height,plane,x,arrayoffset,blocky,blockx,pos;
	UWORD	block[16*8];
	UBYTE	block2[2*16*8];
	struct BitMap *bm;
	BOOL abort = FALSE;
	struct BLKSHeader blks;

	blockwidth = pj->pj_Width>>4;
	//blockheight = pj->pj_Height>>4;
	bm = pj->pj_MainBitMap;

			/* Calc Bottom Pos */
	for(pos = pj->pj_Height-1;
		 (pos >= 1) && (!TestRow(bm,0,pos,pj->pj_Width));
			pos--) {}
	blockheight = (pos+15)/16;

	if( !PushChunk( handle, 0, ID_BLKS, IFFSIZE_UNKNOWN ) )
	{
		Mystrncpy(blks.blks_Name, pj->pj_BankName, 16);
		blks.blks_NumOfBlocks = blockwidth*blockheight;
		blks.blks_Depth = pj->pj_Depth;
		blks.blks_LayoutWidth = blockwidth;
		blks.blks_Flags = glob.cg_SaveChunky ? BLKSF_CHUNKY : 0;
		blks.blks_PixW = 16;
		blks.blks_PixH = 16;
		blks.blks_BitMapHeight = pj->pj_Height;
		blks.blks_Reserved[0] = 0;
		blks.blks_Reserved[1] = 0;
		blks.blks_Reserved[2] = 0;

		if( WriteChunkBytes(handle,&blks,sizeof(blks)) != sizeof(blks))
			abort = TRUE;

		for(blocky = 0; ((blocky<blockheight) && (!abort)); blocky++)
		{
			for(blockx = 0; ((blockx<blockwidth) && (!abort)); blockx++)
			{
				if(glob.cg_SaveChunky)
				{
					arrayoffset = 0;
					for(height = 0; ((height<16) && (!abort)); height++)
					{
						for(x = 0; ((x<16) && (!abort)); x++)
						{
							block2[arrayoffset++] = GetPixelColour(bm, (blockx*16)+x, (blocky*16)+height);
						}
					}
					if(WriteChunkBytes(handle, block2, 2*16*8) != 2*16*8)
						abort = TRUE;
				}
				else
				{
					arrayoffset = 0;
					for(height = 0; ((height<16) && (!abort)); height++)
					{
						for(plane = 0; ((plane<pj->pj_Depth) && (!abort)); plane++)
						{
							block[arrayoffset++] = *(UWORD *)(bm->Planes[plane]+((height+(blocky*16))*bm->BytesPerRow)+blockx*2);
						}
					}
					if(WriteChunkBytes(handle,block,arrayoffset*2) != arrayoffset*2)
						abort = TRUE;
				}
			}
		}
		if(!abort)
		{
			if( PopChunk(handle) )
				abort = TRUE;
		}
	}
	return((BOOL)!abort);
}

/**********************************************************************/
//														ANIM BRUSH															//
/**********************************************************************/

BOOL LoadAnimBrushAs(struct Window *win, struct AnimBrush *ab)
{
	BOOL ret = FALSE;

	if( GetFile(win, "Load Anim Brush", NULL, NULL, FALSE, FALSE) )
	{
		ret = LoadAnimBrush(ab, filereq->fr_File, filereq->fr_Drawer);
	}
	return(ret);
}

BOOL LoadAnimBrush(struct AnimBrush *ab, char *file, char *drawer)
{
	char *name = NULL;
	struct IFFHandle *handle;
	BOOL success = FALSE;

	WaitForSubTaskDone(mainmsgport);

	/* String the path and file name together */
	if( name = MakeFullPathName(drawer, file) )
	{
		if( handle = AllocIFF() )
		{
			/* open the DOS file */
			if( handle->iff_Stream = Open( name, MODE_OLDFILE ) )
			{
				InitIFFasDOS( handle );
				if( !OpenIFF( handle, IFFF_READ ) )
				{
					success = LoadTheAnimBrush( ab, handle);
					CloseIFF( handle );
				}
				Close( handle->iff_Stream );
			}
			FreeIFF( handle );
		}
		FreeVec(name);
	}
	else
		InvalidNameRequester();

	return(success);
}

static BOOL LoadTheAnimBrush( struct AnimBrush *ab, struct IFFHandle *handle)
{
	struct StoredProperty		*sp;
	struct List *abrushlist;
	struct ABrush *abr, *prevabr;
	//BOOL   loadcmap = FALSE;
	UWORD	 framecount;
	UWORD  width,height,depth,numofframes;
	PLANEPTR destptr;
	struct BitMap *bm;
	UWORD	column,row,plane,bytewidth;
	UBYTE		*dltaptrs[8],*dltaptr;
	UBYTE		value,count;
	UWORD   bytesperrow;

	struct BitMapHeader			*bmhd = NULL;
	UBYTE										*cmap = NULL;
	struct DPAnimChunk			*dpan = NULL;
	struct AnimHdr					*anhd = NULL;
	UBYTE										*dlta = NULL;
	struct ContextNode			*body = NULL;

	abrushlist = &ab->ab_ABrushList;

	if(!IsListEmpty(abrushlist))
		FreeAnimBrush(ab);

	abr = (struct ABrush *)abrushlist->lh_TailPred;

	/* These are all the chunks that we can do something with */
	PropChunk(handle, ID_ILBM, ID_BMHD);
	PropChunk(handle, ID_ILBM, ID_CMAP);
	PropChunk(handle, ID_ILBM, ID_DPAN);
	PropChunk(handle, ID_ILBM, ID_ANHD);
	PropChunk(handle, ID_ILBM, ID_DLTA);
	StopChunk(handle, ID_ILBM, ID_BODY);
	StopOnExit(handle, ID_ILBM, ID_FORM);

	if( ParseIFF(handle, IFFPARSE_SCAN) )
	{
		GroovyReq("Import Anim Brush","Parsing Error\nFile not Anim Brush","Continue");
		return(FALSE);
	}

	body = CurrentChunk(handle);
	if(sp = FindProp(handle, ID_ILBM, ID_BMHD)) bmhd = (struct BitMapHeader *)sp->sp_Data;
	if(sp = FindProp(handle, ID_ILBM, ID_CMAP)) cmap = (UBYTE *)sp->sp_Data;
	if(sp = FindProp(handle, ID_ILBM, ID_DPAN)) dpan = (struct DPAnimChunk *)sp->sp_Data;

	if( !bmhd || !dpan || !body )
	{
		GroovyReq("Import Anim Brush","Error Loading\nFile Not Anim Brush","Continue");
		return(FALSE);
	}
/*
	if(cmap)
		loadcmap = GroovyReq("Import Anim Brush","Load Palette?","Yes|No");

	if(loadcmap)
		SussOutCMap(1<<bmhd->bmh_Depth,FALSE,cmap,&pj->pj_Palette,NULL);
 */

	width = bmhd->bmh_Width;
	height = bmhd->bmh_Height;
	depth = bmhd->bmh_Depth;
	numofframes = dpan->dp_NumOfFrames;

	ab->ab_Width = width;
	ab->ab_Height = height;
	ab->ab_Depth = depth;
	if( !(abr = AllocABrush(width, height, depth)) )
	{
		GroovyReq("Import Anim Brush","Not Enough Chip Memory","Continue");
		return(FALSE);
	}
	AddHead(&ab->ab_ABrushList, &abr->ab_Node);
	prevabr = abr;

	UncompILBM(bmhd, body, abr->ab_Image, NULL, handle);
	NastyMakeColourMask(abr->ab_Image, abr->ab_Mask, width, height, 0);

	/* Get out of the first ILBM context */
	ParseIFF(handle, IFFPARSE_RAWSTEP);
	ParseIFF(handle, IFFPARSE_RAWSTEP);


	for(framecount = 0;
			framecount < (numofframes-1);
			framecount++)
	{
		if( !(abr = AllocABrush(width, height, depth)) )
		{
			GroovyReq("Import Anim Brush","Not Enough Chip Memory","Continue");
			return(FALSE);
		}
		AddTail(&ab->ab_ABrushList, &abr->ab_Node);
		BltBitMap(prevabr->ab_Image, 0, 0,
							abr->ab_Image, 0, 0, width, height, 0xC0, 0xFF, NULL);

		if( ParseIFF(handle, IFFPARSE_SCAN) != IFFERR_EOC)
		{
			GroovyReq("Import Anim Brush","Parsing Error\nFile not Anim Brush","Continue");
			return(FALSE);
		}

		/* Get The ANHD chunk */
		if(sp = FindProp(handle, ID_ILBM, ID_ANHD))
			anhd = sp->sp_Data;
		else
		{
			GroovyReq("Import Anim Brush","Loading Error\nFile Missing ANHD chunk","Continue");
			return(FALSE);
		}

		/* Get The DLTA chunk */
		if(sp = FindProp(handle, ID_ILBM, ID_DLTA))
			dlta = sp->sp_Data;
		else
		{
			GroovyReq("Import Anim Brush","Loading Error\nFile Missing DLTA chunk","Continue");
			return(FALSE);
		}

		if( (anhd->operation != 5) || (anhd->interleave != 1) )
		{
			GroovyReq("Import Anim Brush","Loading Error\nFile Not Anim Brush","Continue");
			return(FALSE);
		}

		/* Do The EOR Stuff */
		bm = abr->ab_Image;
		bytesperrow = bm->BytesPerRow;
		bytewidth = (((width+15)/8) & 0xFFFE);
		memcpy(dltaptrs,dlta,depth*4);
		for(plane = 0; plane < depth; plane++)
		{
			if(dltaptrs[plane])
				dltaptrs[plane] += (ULONG)dlta;
		}
		for(plane = 0; plane < depth; plane++)
		{
			dltaptr = dltaptrs[plane];
			if(dltaptr)
			{
				for(column = 0; column < bytewidth; column++)
				{
					destptr = bm->Planes[plane]+column;
					value = *dltaptr++;
					if(value)
					{
						for(row = 0; row < height; )
						{
							value = *dltaptr++;
							if(value == 0)				/* Run */
							{
								count = *dltaptr++;
								value = *dltaptr++;
								for(;count != 0; count--)
								{
									*destptr ^= value;
									destptr += bytesperrow;
									row++;
								}
							}
							else
							{
								if(value & 0x80)		/* Dump */
								{
									for(value &= 0x7F; value != 0; value--)
									{
										*destptr ^= *dltaptr++;
										destptr += bytesperrow;
										row++;
									}
								}
								else								/* Skip */
								{
									destptr += ((UWORD)value)*bytesperrow;
									row += value;
								}
							}
						}
					}
				}
			}
		}
		NastyMakeColourMask(abr->ab_Image, abr->ab_Mask, width, height, 0);
		prevabr = abr;
	}
/*
	if(loadcmap)
		UseProjSettings(pj);
 */
	SetAnimBrushFrame(ab, (struct ABrush *)ab->ab_ABrushList.lh_Head);
	NewTool(lastprojectwindow, TL_POINT);
	SetToolsWindowGadget(TL_POINT,TRUE);
	currentbrush = &brushes[BR_ANIMBRUSH];
	SetToolsWindowGadget(TLB_ANIMBRUSH, TRUE);
	if(drawmode != DRMD_MATTE)
		NewDrawMode(DRMD_MATTE);
	if(lastprojectwindow)
		DrawSelector(lastprojectwindow, lastprojectwindow->pw_SelX0, lastprojectwindow->pw_SelY0);
	return(TRUE);
}
