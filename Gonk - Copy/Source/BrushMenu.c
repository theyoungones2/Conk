#include <stdio.h>
//#include <stdarg.h>
#include <string.h>
//#include <dos/dos.h>
//#include <dos/dostags.h>
//#include <exec/exec.h>
//#include <intuition/intuition.h>
//#include <graphics/gfxbase.h>
//#include <rexx/rxslib.h>
#include <libraries/gadtools.h>
#include <libraries/iffparse.h>
//#include <libraries/diskfont.h>
//#include <libraries/asl.h>

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

#define MN_BRUSH					5

static BOOL LoadBrushAs(struct SuperWindow *sw);
static BOOL PasteBrush(struct SuperWindow *sw);
static BOOL LoadBrushPart2(struct IFFHandle *handle);
static BOOL ReadILBMIntoBrush(struct Brush *br,struct BitMapHeader *bmhd, UBYTE *cmap,
															struct ContextNode *body, struct IFFHandle *handle);
BOOL SaveILBMBrushAs(struct SuperWindow *sw);
BOOL SaveILBMBrush(struct Brush *br, char *name);
BOOL CopyILBMBrush(struct Brush *br);
BOOL SaveBrushPart2(struct Brush *br, struct IFFHandle *handle);
static BOOL WriteBrushBMHDChunk( struct IFFHandle *handle, struct Brush *br);
static BOOL WriteBrushBODYChunk( struct IFFHandle *handle, struct Brush *br);
static void HandleHandleSubMenu(struct SuperWindow *sw, UWORD subnum);
static void DB_SubMenuFlip(struct SuperWindow *sw, UWORD subnum);
static void HandleAnimBrushSubMenu(struct SuperWindow *sw, UWORD subnum);
static void DoAnimBrushSettings(struct SuperWindow *sw);
static void HandleSizeSubMenu(struct SuperWindow *sw, UWORD subnum);

/*************  DoBrushMenu ***********/
//
//	Handle BrushMenu selections.
//

void DoBrushMenu(struct SuperWindow *sw,UWORD id)
{
	UWORD itemnum,subnum;
	struct MenuItem *mi;

	itemnum = ITEMNUM(id);
	subnum = SUBNUM(id);

	mi = ItemAddress( sw->sw_MenuStrip, id );

	WaitForSubTaskDone(mainmsgport);

	switch(itemnum)
	{
		case MNBRH_ANIMBRUSH:
			HandleAnimBrushSubMenu(sw, subnum);
			break;
		case MNBRH_LOAD:
			LoadBrushAs(sw);
			break;
		case MNBRH_SAVE:
			SaveILBMBrushAs(sw);
			break;
		case MNBRH_COPY:
			CopyILBMBrush(&brushes[BR_CUSTOM]);
			break;
		case MNBRH_PASTE:
			PasteBrush(sw);
			break;
		case MNBRH_RESTORE:
			RestoreBrush(currentbrush);
			break;
		case MNBRH_SIZE:
			HandleSizeSubMenu(sw,subnum);
			break;
		case MNBRH_FLIP:
			DB_SubMenuFlip(sw,subnum);
			break;
		case MNBRH_OUTLINE:
			OutlineCurrentBrush(lmbcol);
			break;
		case MNBRH_HANDLE:
			HandleHandleSubMenu(sw, subnum);
			break;
		case MNBRH_FREE:
			WaitForSubTaskDone(mainmsgport);
			if(currentbrush->br_ID == BR_CUSTOM)
			{
				currentbrush = &brushes[BR_DOT];
				SetToolsWindowGadget(TLB_DOT, TRUE);
			}
			FreeBrushBitMaps(&brushes[BR_CUSTOM]);
			break;
	}
}

static BOOL LoadBrushAs(struct SuperWindow *sw)
{
	BOOL ret = FALSE;
	char *name;
	struct IFFHandle *handle;

	if( GetFile(sw->sw_Window, "Load Brush", NULL, NULL, FALSE, FALSE) )
	{
		if( name = MakeFullPathName(filereq->fr_Drawer, filereq->fr_File) )
		{
			if( BlockAllSuperWindows() )
			{
				if( handle = AllocIFF() )
				{
					/* open the DOS file */
					if( handle->iff_Stream = Open( name, MODE_OLDFILE ) )
					{
						InitIFFasDOS( handle );
						ret = LoadBrushPart2(handle);
						Close( handle->iff_Stream );
					}
					else
						GroovyReq("Load Brush","Can't Open File","Continue");
					FreeIFF( handle );
				}
				else
					GroovyReq("Load Brush","No Enough Memory","Continue");
				UnblockAllSuperWindows();
			}
			else
				GroovyReq("Load Brush","No Enough Memory","Continue");
			FreeVec(name);
		}
		else
			GroovyReq("Load Brush","File Name Error","Continue");
	}
	return(ret);	
}

static BOOL PasteBrush(struct SuperWindow *sw)
{
	BOOL ret = FALSE;
	struct IFFHandle *handle;

	if( BlockAllSuperWindows() )
	{
		if( handle = AllocIFF() )
		{
			/* open the DOS file */
			if( handle->iff_Stream = (ULONG) OpenClipboard( 0 ) )
			{
				InitIFFasClip( handle );
				ret = LoadBrushPart2(handle);
				CloseClipboard( (struct ClipboardHandle *)handle->iff_Stream );
			}
			else
				GroovyReq("Paste Brush","Can't Open Clipboard","Continue");
			FreeIFF( handle );
		}
		else
			GroovyReq("Paste Brush","No Enough Memory","Continue");
		UnblockAllSuperWindows();
	}
	else
		GroovyReq("Paste Brush","No Enough Memory","Continue");

	return(ret);	
}

static BOOL LoadBrushPart2(struct IFFHandle *handle)
{
	struct StoredProperty *storedprop	= NULL;
	struct BitMapHeader *bmhd = NULL;
	UBYTE								*cmap = NULL;
	struct ContextNode	*body = NULL;
	BOOL ret = FALSE;

	if( !OpenIFF( handle, IFFF_READ ) )
	{
		PropChunk(handle, ID_ILBM, ID_BMHD);
		PropChunk(handle, ID_ILBM, ID_CMAP);
		StopChunk(handle, ID_ILBM, ID_BODY);
		if( !ParseIFF(handle, IFFPARSE_SCAN) )
		{
			if( storedprop = FindProp(handle, ID_ILBM, ID_BMHD) )	bmhd = storedprop->sp_Data;
			if( storedprop = FindProp(handle, ID_ILBM, ID_CMAP) )	cmap = storedprop->sp_Data;
			body = CurrentChunk(handle);

			if( bmhd && cmap && body && (bmhd->bmh_Depth <= 8) )
			{
				FreeBrushBitMaps(&brushes[BR_CUSTOM]);
				if(ReadILBMIntoBrush(&brushes[BR_CUSTOM],bmhd,cmap,body,handle))
				{
					NewTool(lastprojectwindow,TL_POINT);
					SetToolsWindowGadget(TL_POINT,TRUE);
					currentbrush = &brushes[BR_CUSTOM];
					SetToolsWindowGadget(TLB_CUSTOM,TRUE);
					if(drawmode != DRMD_MATTE)
						NewDrawMode(DRMD_MATTE);
					ret = TRUE;
				}
			}
			else
				GroovyReq("Load Brush","Depth too High\nMax is 8 BitPlanes","Continue");
		}
		else
			GroovyReq("Load Brush","File missing CMAP Chunk","Continue");
		CloseIFF( handle );
	}
	else
		GroovyReq("Load Brush","Can't OpenIFF","Continue");
	return(ret);
}

static BOOL ReadILBMIntoBrush(struct Brush *br,struct BitMapHeader *bmhd, UBYTE *cmap,
																struct ContextNode *body, struct IFFHandle *handle)
{
	struct BrushInfo *ib, *db;
	BOOL ret = FALSE;

	ib = &br->br_InitialBrush;
	db = &br->br_DrawBrush;

	FreeBrushBitMaps(br);

	if(ib->bi_Image = CreateBitMap(bmhd->bmh_Width, bmhd->bmh_Height, bmhd->bmh_Depth, FALSE))
	{
		if(ib->bi_Mask = CreateBitMap(bmhd->bmh_Width, bmhd->bmh_Height, 1, FALSE))
		{
			if(db->bi_Image = CreateBitMap(bmhd->bmh_Width, bmhd->bmh_Height, bmhd->bmh_Depth, FALSE))
			{
				if(db->bi_Mask = CreateBitMap(bmhd->bmh_Width, bmhd->bmh_Height, 1, FALSE))
				{
					UncompILBM(bmhd,body,ib->bi_Image,ib->bi_Mask,handle);
					ret = TRUE;
				}
			}
		}
	}
	if(ret)
	{
		br->br_ID = BR_CUSTOM;
		ib->bi_Width = bmhd->bmh_Width;
		ib->bi_Height = bmhd->bmh_Height;
		ib->bi_Depth = bmhd->bmh_Depth;
		ib->bi_XHandle = ib->bi_Width>>1;
		ib->bi_YHandle = ib->bi_Height>>1;
		db->bi_Width = bmhd->bmh_Width;
		db->bi_Height = bmhd->bmh_Height;
		db->bi_Depth = bmhd->bmh_Depth;
		db->bi_XHandle = db->bi_Width>>1;
		db->bi_YHandle = db->bi_Height>>1;
	}
	else
	{
		if(!ib->bi_Image) DestroyBitMap(ib->bi_Image,bmhd->bmh_Width,bmhd->bmh_Height,bmhd->bmh_Depth);
		if(!ib->bi_Mask) DestroyBitMap(ib->bi_Mask,bmhd->bmh_Width,bmhd->bmh_Height,1);
		if(!db->bi_Image) DestroyBitMap(db->bi_Image,bmhd->bmh_Width,bmhd->bmh_Height,bmhd->bmh_Depth);
		if(!db->bi_Mask) DestroyBitMap(db->bi_Mask,bmhd->bmh_Width,bmhd->bmh_Height,1);
		ib->bi_Image = NULL;
		ib->bi_Mask = NULL;
		db->bi_Image = NULL;
		db->bi_Mask = NULL;
	}
	NastyMakeColourMask(ib->bi_Image, ib->bi_Mask, ib->bi_Width, ib->bi_Height, rmbcol);

	BltBitMap(ib->bi_Image,0,0,db->bi_Image,0,0,ib->bi_Width,ib->bi_Height,0xC0,0xFF,NULL);
	BltBitMap(ib->bi_Mask,0,0,db->bi_Mask,0,0,ib->bi_Width,ib->bi_Height,0xC0,0xFF,NULL);

	return(TRUE);
}


/******************   SaveILBMBrushAs()   *********************/
//
//  Same as any other saveas thingy. Just Calls up Save ILBM.

BOOL SaveILBMBrushAs(struct SuperWindow *sw)
{
	BOOL ret = FALSE;
	char *name;

	if( GetFile(sw->sw_Window, "Save Brush As", NULL, NULL, TRUE, FALSE) )
	{
		/* glue together the full pathname */
		if( name = MakeFullPathName(filereq->fr_Drawer,filereq->fr_File) )
		{
			/* try and save it */
			ret = SaveILBMBrush(&brushes[BR_CUSTOM], name);
		}
		else
		{
			InvalidNameRequester();
		}
	}
	return(ret);
}

/**************  SaveILBM  **************/
//
// Returns TRUE if operation is successful.
//

BOOL SaveILBMBrush(struct Brush *br, char *name)
{
	struct IFFHandle *handle;
	BOOL success = FALSE;

	if( handle = AllocIFF() )
	{
		if( (!glob.cg_MakeBackups) ||
				(glob.cg_MakeBackups && MakeBackupFile(name)) )
		{
			/* open the DOS file */
			if( handle->iff_Stream = Open( name, MODE_NEWFILE ) )
			{
				InitIFFasDOS( handle );
				success = SaveBrushPart2(br, handle);
				Close( handle->iff_Stream );
			}
		}
		FreeIFF( handle );
	}
	return( success );
}

/**************  CopyILBMBrush  **************/
//
// Returns TRUE if operation is successful.
//

BOOL CopyILBMBrush(struct Brush *br)
{
	struct IFFHandle *handle;
	BOOL success = FALSE;

	if( handle = AllocIFF() )
	{
		/* open the DOS file */
		if( handle->iff_Stream = (ULONG)OpenClipboard( 0 ) )
		{
			InitIFFasClip( handle );
			success = SaveBrushPart2(br, handle);
			CloseClipboard( (struct ClipboardHandle *)handle->iff_Stream );
		}
		FreeIFF( handle );
	}
	return( success );
}


BOOL SaveBrushPart2(struct Brush *br, struct IFFHandle *handle)
{
	BOOL success = FALSE;

	if( !OpenIFF( handle, IFFF_WRITE ) )
	{
		if( !PushChunk( handle, ID_ILBM, ID_FORM, IFFSIZE_UNKNOWN ) )
		{
			if( WriteBrushBMHDChunk( handle, br ) )
			{
				if( WriteCMAPChunk( handle, globpalette ) )
				{	
					if( WriteBrushBODYChunk( handle, br) )
					{
						success = TRUE;
					}
				}
			}
			PopChunk( handle );
		}
		CloseIFF( handle );
	}
	return(success);
}

static BOOL WriteBrushBMHDChunk( struct IFFHandle *handle, struct Brush *br)
{
	struct BitMapHeader bmh;
	BOOL success = FALSE;
	ULONG size;
	struct BrushInfo *db;

	db = &br->br_DrawBrush;
	size = sizeof(bmh);

	if( !PushChunk( handle, 0, ID_BMHD, size ) )
	{
		bmh.bmh_Width = db->bi_Width;
		bmh.bmh_Height = db->bi_Height;
		bmh.bmh_Left = 0;
		bmh.bmh_Top = 0;
		bmh.bmh_Depth = db->bi_Depth;
		bmh.bmh_Masking = 0;
		bmh.bmh_Compression = cmpByteRun1;
		bmh.bmh_Flags = BMHDB_CMAPOK;
		bmh.bmh_Transparent = 0;
		bmh.bmh_XAspect = 1;
		bmh.bmh_YAspect = 1;
		bmh.bmh_PageWidth = db->bi_Width;
		bmh.bmh_PageHeight = db->bi_Height;

		if( WriteChunkBytes( handle, &bmh, size ) == size )
			success = TRUE;
		/* tell IFFParse to clean up the ends */
		if( PopChunk( handle ) ) success = FALSE;
	}
	return( success );
}

static BOOL WriteBrushBODYChunk( struct IFFHandle *handle, struct Brush *br)
{
	BOOL					 abort = FALSE;
	struct BitMap	*bm;
	UWORD					 plane,line;
	UBYTE					*destbuffer,*destptr,*srcptr;
	ULONG					 destbuffersize, packedsize, rowsize;
	struct BrushInfo *db;

	db = &br->br_DrawBrush;
	bm = db->bi_Image;
	rowsize = ((db->bi_Width+15)>>3)&0xFFFFFFFE;

	destbuffersize = MaxPackedSize(rowsize);
	if(destbuffer = AllocVec(destbuffersize,MEMF_ANY|MEMF_CLEAR) )
	{
		if( !PushChunk( handle, 0, ID_BODY, IFFSIZE_UNKNOWN ) )
		{
			for(line = 0; ((line<db->bi_Height) && (!abort)); line++)
			{
				for(plane = 0; ( (plane<db->bi_Depth) && (!abort)); plane++)
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



static void HandleHandleSubMenu(struct SuperWindow *sw, UWORD subnum)
{
	struct ProjectWindow *pw = NULL;

	if(sw->sw_Type == SWTY_PROJECT)
		pw = (struct ProjectWindow *)sw;
	else
	{
		if(lastprojectwindow)
			pw = lastprojectwindow;
	}
	switch(subnum)
	{
		case MNBRHHANDLE_ROTATE:
			WaitForSubTaskDone(mainmsgport);
			if(pw) EraseSelector(pw);
			RotateBrushHandle(currentbrush);
			if(pw) DrawSelector(pw, pw->pw_SelX0, pw->pw_SelY0);
			break;
		case MNBRHHANDLE_PLACE:
			if(pw)
			{
				NewTool(pw, TL_BRUSHHANDLE);
				SetToolsWindowGadget(TL_BRUSHHANDLE, TRUE);
				DrawSelector(pw, pw->pw_SelX0, pw->pw_SelY0);
			}
			break;
	}
}

static void HandleAnimBrushSubMenu(struct SuperWindow *sw, UWORD subnum)
{
	struct ProjectWindow *pw = NULL;

	if( sw->sw_Type == SWTY_PROJECT)
		pw = (struct ProjectWindow *)sw;
	else
		pw = lastprojectwindow;

	switch(subnum)
	{
		case	MNBRHANIM_LOAD:
			LoadAnimBrushAs(sw->sw_Window, &animbrush);
			break;
		case	MNBRHANIM_SAVEAS:
			break;
		case	MNBRHANIM_PICKUP:
			if(pw)
			{
				NewTool(pw, TL_GETANIMBRUSH);
				SetToolsWindowGadget(TL_GETANIMBRUSH, TRUE);
				DrawSelector(pw, pw->pw_SelX0, pw->pw_SelY0);
			}
			break;
		case	MNBRHANIM_SETTINGS:
			DoAnimBrushSettings(sw);
			break;
		case	MNBRHANIM_USE:
			if(brushes[BR_ANIMBRUSH].br_DrawBrush.bi_Image)
			{
				EraseSelector(pw);
				currentbrush = &brushes[BR_ANIMBRUSH];
				SetToolsWindowGadget(TLB_ANIMBRUSH, TRUE);
				DrawSelector(pw, pw->pw_SelX0, pw->pw_SelY0);
			}
			break;
		case	MNBRHANIM_FREE:
			if(currentbrush->br_ID == BR_ANIMBRUSH)
			{
				currentbrush = &brushes[BR_DOT];
				SetToolsWindowGadget(TLB_DOT, TRUE);
			}
			FreeAnimBrush(&animbrush);
			break;
	}
}


static void DB_SubMenuFlip(struct SuperWindow *sw, UWORD subnum)
{
	switch(subnum)
	{
		case MNBRHFLIP_HORIZONTAL:
			FlipBrushHorizontally(&currentbrush->br_DrawBrush);
			break;
		case MNBRHFLIP_VERICAL:
			FlipBrushVertically(&currentbrush->br_DrawBrush);
			break;
		case MNBRHFLIP_ROTATE90:
			RotateBrush90(&currentbrush->br_DrawBrush);
			break;
	}
}

static void DoAnimBrushSettings(struct SuperWindow *sw)
{
	UBYTE		numofcellstext[20];
	UWORD numofcells;
	UWORD currentcell;
	struct ABrush *abrush;
	struct Window *parentwin = NULL;

	static struct HoopyObject gadlayout[] =
	{
		{ HOTYPE_VGROUP, 100, HOFLG_FREEWIDTH|HOFLG_FREEHEIGHT,5,0,0,0, NULL, 0,0,0,0,0 },
			{ HOTYPE_TEXT, 100, HOFLG_NOBORDER, 0,0,0,0, NULL,0,0,0,0,0 },
			{ HOTYPE_INTEGER, 100, 0, 4,0,0,0, "Current: (1=Start)",0,0,PLACETEXT_LEFT,0,0 },
			{ HOTYPE_CYCLE, 100, 0, 0,0,0,0, "Direction:",0,0,0,0,0 },
			{ HOTYPE_SPACE, 10, HOFLG_NOBORDER|HOFLG_FREEWIDTH|HOFLG_FREEHEIGHT, 0,0,0,0, NULL,0,0,0,0,0 },
			{ HOTYPE_HGROUP, 100, HOFLG_NOBORDER|HOFLG_FREEWIDTH,3,0,0,0, NULL, 0,0,0,0,0 },
				{ HOTYPE_BUTTON, 100, 0, 0,0,0,0, "OK",0,0x8000,0,0,0 },
				{ HOTYPE_SPACE, 100, HOFLG_NOBORDER|HOFLG_FREEWIDTH, 0,0,0,0, NULL,0,0,0,0,0 },
				{ HOTYPE_BUTTON, 100, 0, 0,0,0,0, "Cancel",0,0xFFFF,0,0,0 },
		{HOTYPE_END}
	};
	char		*labels[] = {"Forward", "Reverse", "Ping Pong", NULL};

	if( !sw || !(parentwin = sw->sw_Window) || (IsListEmpty(&animbrush.ab_ABrushList)) )
		return;

	if(BlockAllSuperWindows())
	{
		numofcells = NodeNumber(&animbrush.ab_ABrushList, animbrush.ab_ABrushList.lh_TailPred);
		sprintf(numofcellstext,"Num Of Cells: %ld",numofcells);
		gadlayout[1].ho_Attr0 = (ULONG)numofcellstext;
	
		currentcell = NodeNumber(&animbrush.ab_ABrushList, (struct Node *)animbrush.ab_CurrentABrush);
		gadlayout[2].ho_Value = currentcell;
	
		gadlayout[3].ho_Attr0 = (ULONG)labels;
	
		if(HO_SimpleDoReq( gadlayout, "Anim Brush Settings", parentwin) != 0xFFFF )
		{
			if(gadlayout[2].ho_Value != currentcell)
			{
				if(gadlayout[2].ho_Value > numofcells)
				{
					DisplayBeep(mainscreen);
					gadlayout[2].ho_Value = numofcells;
				}
				if(gadlayout[2].ho_Value < 1)
				{
					DisplayBeep(mainscreen);
					gadlayout[2].ho_Value = 1;
				}
				if(abrush = (struct ABrush *)FindNthNode(&animbrush.ab_ABrushList, gadlayout[2].ho_Value-1))
					SetAnimBrushFrame(&animbrush, abrush);
			}
			switch(gadlayout[3].ho_Value)
			{
				case 0:		/* Forward */
					animbrush.ab_Direction = ABDIR_FORWARD;
					break;
				case 1:		/* Reverse */
					animbrush.ab_Direction = ABDIR_REVERSE;
					break;
				case 2:		/* Ping Pong */
					animbrush.ab_Direction = ABDIR_PINGPONG;
					animbrush.ab_PingPongDirection = ABDIR_FORWARD;
					break;
			}
		}
		UnblockAllSuperWindows();
	}
}	

static void HandleSizeSubMenu(struct SuperWindow *sw, UWORD subnum)
{
	if(currentbrush == &brushes[BR_CUSTOM])
	{
		switch(subnum)
		{
			case MNBRHSIZE_STRETCH:
				break;
			case MNBRHSIZE_HALVE:
				HalveBrush(&currentbrush->br_DrawBrush, globpalette);
				break;
			case MNBRHSIZE_DOUBLE:
				break;
			case MNBRHSIZE_DOUBLEHORIZ:
				break;
			case MNBRHSIZE_DOUBLEVERT:
				break;
		}
	}
	else
		GroovyReq("Size Brush","Can Only Transform\nCustom Brush","Okay");
}
