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

#define MN_EDIT						1

static void HandleColourSubMenu(struct ProjectWindow *pw,UWORD subnum);
static void HandleBobsSubMenu(struct ProjectWindow *pw, UWORD subnum);
static void HandleSpritesSubMenu(struct ProjectWindow *pw, UWORD subnum);
void GUIRemapProject(struct ProjectWindow *pw);
BOOL RemapProject(struct Project *pj, struct Palette *destpl, UWORD newdepth);
static BOOL RemapBobsSameDepth(UBYTE *remaptable, struct Project *pj);
static BOOL RemapBobsDifferentDepth(UBYTE *remaptable, struct Project *pj, UWORD newdepth);
static BOOL RemapNonBobProject(UBYTE *remaptable, struct Project *pj, UWORD newdepth);
void RemapBitMap(UBYTE *remaptable, struct BitMap *srcbitmap, struct BitMap *destbitmap, UWORD width, UWORD height);
static BOOL LoadPaletteAs(struct ProjectWindow *pw);
static BOOL SavePaletteAs(struct ProjectWindow *pw);


void SetupEditMenu(struct Project *pj, struct NewMenu *nm)
{
	/* suss out the state of the clicky checkmark things */

	if( (pj == NULL) && (lastprojectwindow) )
		pj = lastprojectwindow->pw_Project;

	nm[MNEDT_BLOCKS_IDX].nm_Flags = CHECKIT|MENUTOGGLE;
	nm[MNEDT_BOBS_IDX].nm_Flags = CHECKIT|MENUTOGGLE;
	nm[MNEDT_SPRITES_IDX].nm_Flags = CHECKIT|MENUTOGGLE;
	nm[MNEDT_FRAME_IDX].nm_Flags = CHECKIT|MENUTOGGLE;
	nm[MNEDT_BLOCKS2_IDX].nm_Flags = NM_ITEMDISABLED;
	nm[MNEDT_BOBS2_IDX].nm_Flags = NM_ITEMDISABLED;
	nm[MNEDT_SPRITES2_IDX].nm_Flags = NM_ITEMDISABLED;
	nm[MNEDT_FRAME2_IDX].nm_Flags = NM_ITEMDISABLED;
	nm[MNEDT_NAMECHUNK_IDX].nm_Flags = NM_ITEMDISABLED;
	if(pj)
	{
		switch(pj->pj_EditType)
		{
			case EDTY_BLOCKS:
				nm[MNEDT_BLOCKS_IDX].nm_Flags = CHECKIT|MENUTOGGLE|CHECKED;
				nm[MNEDT_BLOCKS2_IDX].nm_Flags = 0;
				nm[MNEDT_NAMECHUNK_IDX].nm_Flags = 0;
				break;
			case EDTY_BOBS:
				nm[MNEDT_BOBS_IDX].nm_Flags = CHECKIT|MENUTOGGLE|CHECKED;
				nm[MNEDT_BOBS2_IDX].nm_Flags = 0;
				nm[MNEDTBOBS_EDITMASK_IDX].nm_Flags = pj->pj_EditBobMask ?
					CHECKIT|MENUTOGGLE|CHECKED : CHECKIT|MENUTOGGLE;
				nm[MNEDT_NAMECHUNK_IDX].nm_Flags = 0;
				break;
			case EDTY_SPRITES:
				nm[MNEDT_SPRITES_IDX].nm_Flags = CHECKIT|MENUTOGGLE|CHECKED;
				nm[MNEDT_SPRITES2_IDX].nm_Flags = 0;
				nm[MNEDT_NAMECHUNK_IDX].nm_Flags = 0;
				break;
			case EDTY_FRAME:
				nm[MNEDT_FRAME_IDX].nm_Flags = CHECKIT|MENUTOGGLE|CHECKED;
				nm[MNEDT_FRAME2_IDX].nm_Flags = 0;
				break;
		}
	}
}

/***********************   RefreshEditMenu()    *********************/
//
//  Update the Edit Menu of the given project. If no project then
// just try for the previous selected one.

void RefreshEditMenu(struct Project *pj)
{
	struct ProjectWindow *pw;
	struct Menu *mn;
	struct MenuItem *mi;
	struct MenuItem *items[MNEDT_NUMOFITEMS];
	UWORD count;

	if(pj == NULL)
	{
		if(lastprojectwindow)
			pj = lastprojectwindow->pw_Project;
		else
			return;
	}

	/* Change the Menus for each window in the project */
	for(pw = pj->pj_ProjectWindows; pw; pw = pw->pw_NextProjWindow)
	{
		if( (pw->pw_sw.sw_Window) && (pw->pw_sw.sw_MenuStrip) && 
				(mn = pw->pw_sw.sw_Menus[MN_EDIT]) )
		{
			count = 0;
			for(mi = mn->FirstItem; mi; mi = mi->NextItem)
			{
				items[count] = mi;
				switch(count)
				{
					case MNEDT_BLOCKS:
					case MNEDT_BOBS:
					case MNEDT_SPRITES:
					case MNEDT_FRAME:
						mi->Flags &= ~CHECKED;
						break;
					case MNEDT_BLOCKS2:
					case MNEDT_BOBS2:
					case MNEDT_FRAME2:
					case MNEDT_NAMECHUNK:
						mi->Flags &= ~ITEMENABLED;
						break;
				}
				count++;
			}
			switch(pj->pj_EditType)
			{
				case EDTY_BLOCKS:
					items[MNEDT_BLOCKS]->Flags |= CHECKED;
					items[MNEDT_BLOCKS2]->Flags |= ITEMENABLED;
					items[MNEDT_NAMECHUNK]->Flags |= ITEMENABLED;
					break;
				case EDTY_BOBS:
					items[MNEDT_BOBS]->Flags |= CHECKED;
					items[MNEDT_BOBS2]->Flags |= ITEMENABLED;
					items[MNEDT_NAMECHUNK]->Flags |= ITEMENABLED;
					count = 0;
					for(mi = items[MNEDT_BOBS2]->SubItem; mi; mi = mi->NextItem)
					{
						if(count == MNEDTBOBS_EDITMASK)
						{
							if(pj->pj_EditBobMask)
								mi->Flags |= CHECKED;
							else
								mi->Flags &= ~CHECKED;
						}
					}
					break;
				case EDTY_SPRITES:
					items[MNEDT_SPRITES]->Flags |= CHECKED;
					items[MNEDT_NAMECHUNK]->Flags |= ITEMENABLED;
					items[MNEDT_SPRITES2]->Flags |= ITEMENABLED;
					break;
				case EDTY_FRAME:
					items[MNEDT_FRAME]->Flags |= CHECKED;
					items[MNEDT_FRAME2]->Flags |= ITEMENABLED;
					break;
			}
		}
	}
}

void DoEditMenu(struct ProjectWindow *pw,UWORD id)
{
	UWORD itemnum,subnum;
	struct MenuItem *mi;

	itemnum = ITEMNUM(id);
	subnum = SUBNUM(id);

	mi = ItemAddress( pw->pw_sw.sw_MenuStrip, id );

	WaitForSubTaskDone(mainmsgport);

	switch(itemnum)
	{
		case MNEDT_BLOCKS:
			NewEditType(pw->pw_Project,EDTY_BLOCKS,pw->pw_Project->pj_Depth);
			break;
		case MNEDT_BOBS:
			NewEditType(pw->pw_Project,EDTY_BOBS,pw->pw_Project->pj_Depth);
			break;
		case MNEDT_SPRITES:
			NewEditType(pw->pw_Project,EDTY_SPRITES,pw->pw_Project->pj_Depth);
			break;
		case MNEDT_FRAME:
			NewEditType(pw->pw_Project,EDTY_FRAME,pw->pw_Project->pj_Depth);
			break;
		case MNEDT_BOBS2:
			HandleBobsSubMenu(pw,subnum);
			break;
		case MNEDT_SPRITES2:
			HandleSpritesSubMenu(pw,subnum);
			break;
		case MNEDT_COLOUR:
			HandleColourSubMenu(pw, subnum);
			break;
		case MNEDT_UNDO:
			DoUndo(pw->pw_Project);
			break;
		case MNEDT_NAMECHUNK:
			SetChunkName(pw->pw_Project);
			break;
	}
}

static void HandleBobsSubMenu(struct ProjectWindow *pw, UWORD subnum)
{
	struct MyBob *bob;

	switch(subnum)
	{
		case MNEDTBOBS_PREVIOUS:
			WaitForSubTaskDone(mainmsgport);
			GotoPreviousBob(pw->pw_Project);
			break;
		case MNEDTBOBS_NEXT:
			WaitForSubTaskDone(mainmsgport);
			GotoNextBob(pw->pw_Project);
			break;
		case MNEDTBOBS_GOTO:
			WaitForSubTaskDone(mainmsgport);
			if(BlockAllSuperWindows())
			{
				bob = GetBobRequester(pw, "Goto Bob");
				UnblockAllSuperWindows();
			}
			if(bob)
			{
				GotoNewBob(pw->pw_Project, bob);
				RefreshProjectName(pw->pw_Project);
			}
			break;
		case MNEDTBOBS_PLAY:
			break;
		case MNEDTBOBS_PINGPONG:
			break;
		case MNEDTBOBS_INSERT:
			if(pw->pw_sw.sw_Window)
				GUIInsertBobs(pw->pw_Project, pw->pw_sw.sw_Window);
			break;
		case MNEDTBOBS_DELETE:
			if(pw->pw_sw.sw_Window)
				GUIDeleteBobs(pw->pw_Project, pw->pw_sw.sw_Window);
			break;
		case MNEDTBOBS_COPY:
			if(pw->pw_sw.sw_Window)
				GUICopyBobs(pw->pw_Project, pw->pw_sw.sw_Window);
			break;
		case MNEDTBOBS_EDITMASK:
			if(pw->pw_Project->pj_EditBobMask)
			{
				WaitForSubTaskDone(mainmsgport);
				TurnOnBobImageEditing(pw->pw_Project);
			}
			else
				TurnOnBobMaskEditing(pw->pw_Project);
			RefreshEditMenu(pw->pw_Project);
			break;
		case MNEDTBOBS_DELETEMASK:
			DeleteMask(pw->pw_Project, pw->pw_Project->pj_CurrentBob);
			break;
		case MNEDTBOBS_REMAKEMASK:
			RemakeMask(pw->pw_Project, pw->pw_Project->pj_CurrentBob);
			break;
		case MNEDTBOBS_EDITZONE:
			WaitForSubTaskDone(mainmsgport);
			NewTool( pw, TL_HITZONE);
			SetToolsWindowGadget(TL_HITZONE, TRUE);
			DrawSelector(pw, pw->pw_SelX0, pw->pw_SelY0);
			break;
		case MNEDTBOBS_DELETEZONE:
			if(bob = pw->pw_Project->pj_CurrentBob)
				bob->bb_Flags &= ~MBF_COLLISIONZONESET;
			RefreshProjectName(pw->pw_Project);
			break;
		case MNEDTBOBS_REMAKEZONE:
			if(bob = pw->pw_Project->pj_CurrentBob)
			{
				bob->bb_ColLeft = bob->bb_Left;
				bob->bb_ColRight = bob->bb_Left + bob->bb_Width - 1;
				bob->bb_ColTop = bob->bb_Top;
				bob->bb_ColBottom = bob->bb_Top + bob->bb_Height - 1;
				bob->bb_Flags |= MBF_COLLISIONZONESET;
			}
			RefreshProjectName(pw->pw_Project);
			break;
	}
}

static void HandleSpritesSubMenu(struct ProjectWindow *pw, UWORD subnum)
{
	struct MyBob *bob = NULL;

	switch(subnum)
	{
		case MNEDTSPRITES_PREVIOUS:
			WaitForSubTaskDone(mainmsgport);
			GotoPreviousSprite(pw->pw_Project);
			break;
		case MNEDTSPRITES_NEXT:
			WaitForSubTaskDone(mainmsgport);
			GotoNextSprite(pw->pw_Project);
			break;
		case MNEDTSPRITES_GOTO:
			WaitForSubTaskDone(mainmsgport);
			if(BlockAllSuperWindows())
			{
				bob = GetBobRequester(pw, "Goto Sprite");
				UnblockAllSuperWindows();
			}
			if(bob)
			{
				GotoNewSprite(pw->pw_Project, bob);
				RefreshProjectName(pw->pw_Project);
			}
			break;
		case MNEDTSPRITES_PLAY:
			break;
		case MNEDTSPRITES_PINGPONG:
			break;
		case MNEDTSPRITES_INSERT:
			if(pw->pw_sw.sw_Window)
				GUIInsertBobs(pw->pw_Project, pw->pw_sw.sw_Window);
			break;
		case MNEDTSPRITES_DELETE:
			if(pw->pw_sw.sw_Window)
				GUIDeleteBobs(pw->pw_Project, pw->pw_sw.sw_Window);
			break;
		case MNEDTSPRITES_COPY:
			if(pw->pw_sw.sw_Window)
				GUICopyBobs(pw->pw_Project, pw->pw_sw.sw_Window);
			break;
		case MNEDTSPRITES_EDITZONE:
			WaitForSubTaskDone(mainmsgport);
			NewTool( pw, TL_HITZONE);
			SetToolsWindowGadget(TL_HITZONE, TRUE);
			DrawSelector(pw, pw->pw_SelX0, pw->pw_SelY0);
			break;
		case MNEDTSPRITES_DELETEZONE:
			if(bob = pw->pw_Project->pj_CurrentBob)
				bob->bb_Flags &= ~MBF_COLLISIONZONESET;
			RefreshProjectName(pw->pw_Project);
			break;
		case MNEDTSPRITES_REMAKEZONE:
			if(bob = pw->pw_Project->pj_CurrentBob)
			{
				bob->bb_ColLeft = bob->bb_Left;
				bob->bb_ColRight = bob->bb_Left + bob->bb_Width - 1;
				bob->bb_ColTop = bob->bb_Top;
				bob->bb_ColBottom = bob->bb_Top + bob->bb_Height - 1;
				bob->bb_Flags |= MBF_COLLISIONZONESET;
			}
			RefreshProjectName(pw->pw_Project);
			break;
	}
}

BOOL NewEditType(struct Project *pj, UWORD newtype, UWORD newdepth)
{
	struct MyBob *bob;
	BOOL ret = TRUE;
	struct ProjectWindow *pw;
	UWORD height;

	if(pj->pj_EditType != newtype)
	{
		/* Remove The Old Edit Type */
		switch(pj->pj_EditType)
		{
			case EDTY_BLOCKS:
				break;
			case EDTY_BOBS:
				if( newtype != EDTY_SPRITES )
				{
					while( !IsListEmpty(&pj->pj_BobList) )
					{
						bob = (struct MyBob *)pj->pj_BobList.lh_Head;
						Remove(&bob->bb_Node);
						FreeBobBitMaps(bob);
						FreeVec(bob);
					}
				}
				break;
			case EDTY_SPRITES:
				if(newtype != EDTY_BOBS)
				{
					while( !IsListEmpty(&pj->pj_BobList) )
					{
						bob = (struct MyBob *)pj->pj_BobList.lh_Head;
						Remove(&bob->bb_Node);
						FreeBobBitMaps(bob);
						FreeVec(bob);
					}
				}
				break;
			case EDTY_FRAME:
				break;
		}
		/* Enable The New Edit Type */
		switch(newtype)
		{
			case EDTY_BLOCKS:
				pj->pj_EditType = EDTY_BLOCKS;
				for(pw = pj->pj_ProjectWindows; pw; pw = pw->pw_NextProjWindow)
					RedisplayProjWindow(pw,TRUE);
				break;
			case EDTY_BOBS:
				ret = InitBobMode(pj,newdepth);
				break;
			case EDTY_SPRITES:
				ret = InitSpriteMode(pj,newdepth);
				break;
			case EDTY_FRAME:
				pj->pj_EditType = EDTY_FRAME;
				for(pw = pj->pj_ProjectWindows; pw; pw = pw->pw_NextProjWindow)
					RedisplayProjWindow(pw,TRUE);
				break;
		}
	}
	else
	{
		if(newdepth != pj->pj_Depth)
		{
			DeallocProjBitMaps(pj);
			pj->pj_Depth = newdepth;
			if(pj->pj_EditType == EDTY_BOBS)
			{
				height = mainscreen->Height;
				if(height > (BOB_MAXHEIGHT/pj->pj_Depth))
					height = BOB_MAXHEIGHT/pj->pj_Depth;
				if( (pj->pj_MainRPort = AllocBitMapRPort(pj->pj_Width, height, pj->pj_Depth)) &&
						(pj->pj_UndoBitMap = CreateBitMap(pj->pj_Width, height, pj->pj_Depth, TRUE)) &&
						(pj->pj_MaskBitMap = CreateBitMap(pj->pj_Width, height, 1, TRUE)) )
				{
					pj->pj_MainBitMap = pj->pj_MainRPort->BitMap;
				}
				else
					ret = FALSE;
			}
			else
			{
				if( (pj->pj_MainRPort = AllocBitMapRPort(pj->pj_Width, pj->pj_Height, pj->pj_Depth)) &&
						(pj->pj_UndoBitMap = CreateBitMap(pj->pj_Width, pj->pj_Height, pj->pj_Depth, TRUE)) )
				{
					pj->pj_MainBitMap = pj->pj_MainRPort->BitMap;
				}
				else
					ret = FALSE;
			}
		}
	}
	RefreshEditMenu(pj);
	RefreshProjectName(pj);
	return(ret);
}


/*************  HandleColourSubMenu ***********/
//
//	Handle ColourMenu selections.
//

static void HandleColourSubMenu(struct ProjectWindow *pw,UWORD subnum)
{
	WaitForSubTaskDone(mainmsgport);

	switch(subnum)
	{
		case MNEDTCOL_USEPALETTE:
			if(pw->pw_sw.sw_Type == SWTY_PROJECT)
				UseProjSettings(pw->pw_Project);
			break;
		case MNEDTCOL_REMAP:
			if(pw->pw_sw.sw_Type == SWTY_PROJECT)
				GUIRemapProject(pw);
			break;
		case MNEDTCOL_LOADPALETTE:
			if(pw->pw_sw.sw_Type == SWTY_PROJECT)
				LoadPaletteAs(pw);
			break;
		case MNEDTCOL_SAVEPALETTE:
			if(pw->pw_sw.sw_Type == SWTY_PROJECT)
				SavePaletteAs(pw);
			break;
	}
}

static BOOL LoadPaletteAs(struct ProjectWindow *pw)
{
	BOOL ret = FALSE;
	char *name;
	struct IFFHandle *handle;
	struct Project *pj;

	pj = pw->pw_Project;

	if( GetFile(pw->pw_sw.sw_Window, "Load Palette", NULL, NULL, FALSE, FALSE) )
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
						if( !OpenIFF( handle, IFFF_READ ) )
						{
							StopChunk(handle, ID_ILBM, ID_CMAP);
							StopChunk(handle, ID_CONK, ID_CMAP);
							if( !ParseIFF(handle, IFFPARSE_SCAN) )
							{
								if( LoadCMAPIntoPalette(&pj->pj_Palette, handle) )
								{
									pj->pj_Palette.pl_Count = 1<<pj->pj_Depth;
									UseProjSettings(pj);
									ret = TRUE;
								}
							}
							else
								GroovyReq("Load Palette","File missing CMAP Chunk","Continue");
							CloseIFF( handle );
						}
						else
							GroovyReq("Load Palette","Can't OpenIFF","Continue");
						Close( handle->iff_Stream );
					}
					else
						GroovyReq("Load Palette","Can't Open File","Continue");
					FreeIFF( handle );
				}
				else
					GroovyReq("Load Palette","No Enough Memory","Continue");
				UnblockAllSuperWindows();
			}
			else
				GroovyReq("Load Palette","No Enough Memory","Continue");
			FreeVec(name);
		}
		else
			GroovyReq("Load Palette","File Name Error","Continue");
	}
	return(ret);	
}

static BOOL SavePaletteAs(struct ProjectWindow *pw)
{
	BOOL ret = FALSE;
	char *name;
	struct IFFHandle *handle;
	struct Project *pj;

	pj = pw->pw_Project;

	if( GetFile(pw->pw_sw.sw_Window, "Save Palette", NULL, NULL, TRUE, FALSE) )
	{
		if( name = MakeFullPathName(filereq->fr_Drawer, filereq->fr_File) )
		{
			if( BlockAllSuperWindows() )
			{
				if( handle = AllocIFF() )
				{
					/* open the DOS file */
					if( handle->iff_Stream = Open( name, MODE_NEWFILE ) )
					{
						InitIFFasDOS( handle );
						if( !OpenIFF( handle, IFFF_WRITE ) )
						{
							if( !PushChunk( handle, ID_ILBM, ID_FORM, IFFSIZE_UNKNOWN ) )
							{
								ret = WriteCMAPChunk( handle, &pj->pj_Palette);
								PopChunk( handle );
							}
							else
								GroovyReq("Save Palette","IFF Error","Continue");
							CloseIFF( handle );
						}
						else
							GroovyReq("Save Palette","Can't OpenIFF","Continue");
						Close( handle->iff_Stream );
					}
					else
						GroovyReq("Save Palette","Can't Open File","Continue");
					FreeIFF( handle );
				}
				else
					GroovyReq("Save Palette","No Enough Memory","Continue");
				UnblockAllSuperWindows();
			}
			else
				GroovyReq("Save Palette","No Enough Memory","Continue");
			FreeVec(name);
		}
		else
			GroovyReq("Save Palette","File Name Error","Continue");
	}
	return(ret);	
}

void GUIRemapProject(struct ProjectWindow *pw)
{
	struct Project *topj;
	struct Project *frompj;

	frompj = pw->pw_Project;

	if( BlockAllSuperWindows() )
	{
		if( topj = GetProjectRequester(pw->pw_sw.sw_Window, frompj, "Remap To...") )
		{
			if(!PaletteSame(&frompj->pj_Palette, &topj->pj_Palette))
			{
				if( (frompj->pj_EditType == EDTY_BOBS) || (frompj->pj_EditType == EDTY_SPRITES) )
				{
					if( (frompj->pj_CurrentBob) && (frompj->pj_CurrentBobModified) )
						StoreBob(frompj, frompj->pj_CurrentBob);

					if(!IsListEmpty(&frompj->pj_BobList))
					{
						BlockAllSuperWindows();
						RemapProject(frompj, &topj->pj_Palette, topj->pj_Depth);
					}
					else
						GroovyReq("Remap Project","Empty Bob List","Abort");
				}
				else
					RemapProject(frompj, &topj->pj_Palette, topj->pj_Depth);
			}
			else
				GroovyReq("Remap Project","Palettes Identical On Both Projects","Cancel");
		}
		UnblockAllSuperWindows();
	}
}

BOOL RemapProject(struct Project *pj, struct Palette *destpl, UWORD newdepth)
{
	struct Palette *srcpl;
	UBYTE remaptable[256];
	UWORD count;
	BOOL ret = TRUE;

	srcpl = &pj->pj_Palette;

	/* Make a remap table. This table acts as a colour translation array. ie, we get the colour
		of the source pixel, look up that position in the array, and pull out the new colour
		that it should be. */
	for(count = 0; count < srcpl->pl_Count; count++)
	{
		remaptable[count] = FindnBestPen(destpl, destpl->pl_Count, srcpl->pl_Colours[count].r, srcpl->pl_Colours[count].g, srcpl->pl_Colours[count].b);
	}

	if( (pj->pj_EditType == EDTY_BOBS) || (pj->pj_EditType == EDTY_SPRITES) )
	{
		if(newdepth == pj->pj_Depth)
			ret = RemapBobsSameDepth(remaptable, pj);
		else
			ret = RemapBobsDifferentDepth(remaptable, pj, newdepth);

		EditBob(pj, pj->pj_CurrentBob);
	}
	else
		ret = RemapNonBobProject(remaptable, pj, newdepth);

	memcpy(&pj->pj_Palette, destpl, sizeof(struct Palette));
	RemakeScreenIfNeeded(globpalette);

	BltBitMap(pj->pj_MainBitMap, 0, 0, pj->pj_UndoBitMap, 0, 0,
						pj->pj_Width, pj->pj_Height, 0xC0, 0xFF, 0);
	pj->pj_UndoTop = 0;
	pj->pj_UndoLeft = 0;
	pj->pj_UndoBottom = 1;
	pj->pj_UndoRight = 1;
	WaitBlit();
	return(ret);
}

static BOOL RemapBobsSameDepth(UBYTE *remaptable, struct Project *pj)
{
	struct MyBob *bob;

	/* We have to go through all the bobs and remap them */
	for(bob = (struct MyBob *)pj->pj_BobList.lh_Head;
			bob->bb_Node.ln_Succ;
			bob = (struct MyBob *)bob->bb_Node.ln_Succ)
	{
		if( bob->bb_Image )
			RemapBitMap(remaptable, bob->bb_Image, bob->bb_Image, bob->bb_Width, bob->bb_Height);
	}
	return(TRUE);
}

static BOOL RemapBobsDifferentDepth(UBYTE *remaptable, struct Project *pj, UWORD newdepth)
{
	struct BitMap **bmarraybase = NULL;
	struct RastPort *destrp = NULL;
	struct BitMap *destbitmap = NULL, *undobitmap;
	struct MyBob *bob;
	UWORD numofbobs, bobnum;
	BOOL ret = TRUE;

	if(destrp = AllocBitMapRPort( pj->pj_Width, pj->pj_Height, newdepth) )
	{
		destbitmap = destrp->BitMap;
		if( undobitmap = CreateBitMap( pj->pj_Width, pj->pj_Height, newdepth, TRUE) )
		{
			numofbobs = ((struct MyBob *)pj->pj_BobList.lh_TailPred)->bb_Number;
			if( bmarraybase = AllocVec(4*numofbobs, MEMF_ANY|MEMF_CLEAR) )
			{
				bobnum = 0;
				for(bob = (struct MyBob *)pj->pj_BobList.lh_Head;
						(bob->bb_Node.ln_Succ) && (ret);
						bob = (struct MyBob *)bob->bb_Node.ln_Succ)
				{
					if(bob->bb_Image)
					{
						if( !(bmarraybase[bobnum] = CreateBitMap(bob->bb_Width, bob->bb_Height, newdepth, TRUE)) )
							ret = FALSE;
					}
					bobnum++;
				}
				if(ret)
				{
					/* Okay, we now have an array of pointers to bitmaps, one for each bob. So
						all we should have to do is remap into each of these. */

					/* But First kill the old project main bitmaps and use the new ones */
					FreeBitMapRPort(pj->pj_MainRPort, pj->pj_Width, pj->pj_Height, pj->pj_Depth);
					if(pj->pj_UndoBitMap) DestroyBitMap(pj->pj_UndoBitMap, pj->pj_Width, pj->pj_Height, pj->pj_Depth);
					pj->pj_MainRPort = destrp;
					pj->pj_MainBitMap = destbitmap;
					pj->pj_UndoBitMap = undobitmap;
					pj->pj_Depth = newdepth;
					if(pj->pj_Depth == mainscreen->RastPort.BitMap->Depth)
						pj->pj_Disabled = FALSE;

					bobnum = 0;
					for(bob = (struct MyBob *)pj->pj_BobList.lh_Head;
							bob->bb_Node.ln_Succ;
							bob = (struct MyBob *)bob->bb_Node.ln_Succ)
					{
						if(bob->bb_Image)
						{
							RemapBitMap(remaptable, bob->bb_Image, bmarraybase[bobnum], bob->bb_Width, bob->bb_Height);
							DestroyBitMap(bob->bb_Image, bob->bb_Width, bob->bb_Height, bob->bb_Depth);
							bob->bb_Image = bmarraybase[bobnum];
							bob->bb_Depth = newdepth;
						}
						bobnum++;
					}
				}
				else
				{
					/* Clean up any memory allocation */
		
					bobnum = 0;
					for(bob = (struct MyBob *)pj->pj_BobList.lh_Head;
							bob->bb_Node.ln_Succ;
							bob = (struct MyBob *)bob->bb_Node.ln_Succ)
					{
						if(bmarraybase[bobnum])
						{
							DestroyBitMap(bmarraybase[bobnum], bob->bb_Width, bob->bb_Height, newdepth);
							bmarraybase[bobnum] = NULL;
						}
						bobnum++; 
					}
					FreeBitMapRPort(destrp, pj->pj_Width, pj->pj_Height, newdepth);
					DestroyBitMap(undobitmap, pj->pj_Width, pj->pj_Height, newdepth);
					GroovyReq("Remap Error","Not Enough Chip Memory","Okay");
				}
				FreeVec(bmarraybase);
				bmarraybase = NULL;
			}
			else
			{
				GroovyReq("Remap Error","Not Enough Memory","Okay");
				ret = FALSE;
			}
		}
		else
		{
			FreeBitMapRPort(destrp, pj->pj_Width, pj->pj_Height, newdepth);
			GroovyReq("Remap Error","Not Enough Chip Memory","Okay");
			ret = FALSE;
		}
	}
	else
	{
		GroovyReq("Remap Error","Not Enough Chip Memory","Okay");
		ret = FALSE;
	}
	return(ret);
}

static BOOL RemapNonBobProject(UBYTE *remaptable, struct Project *pj, UWORD newdepth)
{
	struct BitMap *srcbitmap, *destbitmap;
	struct RastPort *destrp;
	struct ProjectWindow *pw;
	BOOL ret = TRUE;

	/* If the new depth is the same as the old depth, then we can just remap in place,
		else we have to alloc a new bitmap of the right depth */
	srcbitmap = pj->pj_MainBitMap;
	if(newdepth == pj->pj_Depth)
		destbitmap = pj->pj_MainBitMap;
	else
	{
		if(destrp = AllocBitMapRPort( pj->pj_Width, pj->pj_Height, newdepth) )
			destbitmap = destrp->BitMap;
		else
			destbitmap = NULL;
	}
	if(destbitmap)
	{
		RemapBitMap(remaptable, srcbitmap, destbitmap, pj->pj_Width, pj->pj_Height);

		if(destbitmap != srcbitmap)
		{
			FreeBitMapRPort(pj->pj_MainRPort, pj->pj_Width, pj->pj_Height, pj->pj_Depth);
			pj->pj_MainRPort = destrp;
			pj->pj_MainBitMap = destbitmap;
			pj->pj_Depth = newdepth;
			if(pj->pj_UndoBitMap)
				DestroyBitMap(pj->pj_UndoBitMap, pj->pj_Width, pj->pj_Height, pj->pj_Depth);
			if(pj->pj_UndoBitMap = CreateBitMap( pj->pj_Width, pj->pj_Height, newdepth, TRUE) )
			{
				if(pj->pj_Depth == mainscreen->RastPort.BitMap->Depth)
					pj->pj_Disabled = FALSE;
			}
			else
			{
				GroovyReq("Remap Error","Not Enough Memory For Undo BitMap\nProject Disabled","Okay");
				pj->pj_Disabled = TRUE;
			}
		}

		for(pw = pj->pj_ProjectWindows; pw; pw = pw->pw_NextProjWindow)
		{
			if(pw->pw_sw.sw_Window)
				RedisplayProjWindow(pw,TRUE);
		}
	}
	else
	{
		GroovyReq("Remap Error","Not Enough Chip Memory\nFor Operation","Okay");
		ret = FALSE;
	}
	return(ret);
}

void RemapBitMap(UBYTE *remaptable, struct BitMap *srcbitmap, struct BitMap *destbitmap,
								 UWORD width, UWORD height)
{
	UWORD x, y, col;

	for(y = 0; y < height; y++)
	{
		for(x = 0; x < width; x++)
		{
			col = GetPixelColour(srcbitmap, x, y);
			PutPixelColour(destbitmap, x, y, remaptable[col]);
		}
	}
}
