#include <stdio.h>
//#include <stdarg.h>
//#include <string.h>
#include <math.h>
//#include <dos/dos.h>
//#include <dos/dostags.h>
#include <exec/exec.h>
//#include <intuition/intuition.h>
//#include <graphics/gfxbase.h>
#include <graphics/gfx.h>
//#include <rexx/rxslib.h>
//#include <libraries/gadtools.h>
//#include <libraries/diskfont.h>
//#include <libraries/asl.h>

#include <proto/exec.h>
//#include <proto/dos.h>
#include <proto/intuition.h>
#include <proto/graphics.h>
//#include <proto/diskfont.h>
//#include <proto/asl.h>
//#include <clib/alib_protos.h>
//#include <proto/gadtools.h>

#include <graphics/gfx.h>
#include <global.h>
#include <ExternVars.h>

static void InitBrush1Bitmaps(void);
static void InitBrush2Bitmaps(void);
static void InitBrush3Bitmaps(void);
static void InitBrush4Bitmaps(void);
static void InitBrush5Bitmaps(void);
static void InitBrush6Bitmaps(void);
static void InitBrush7Bitmaps(void);
static void InitBrush8Bitmaps(void);

static void BltColourBrush(struct Project *pj, struct BrushInfo *bi,
														WORD brushxoffset, WORD brushyoffset,
														WORD x, WORD y, WORD width, WORD height, UWORD col);

BOOL GetCustomBrushImage(struct Brush *br, struct BitMap *srcbm,
												 WORD x, WORD y, WORD width, WORD height)
{
	struct BrushInfo *ib,*db;

	if( (!br) || (!srcbm) || (width <= 0) || (height <= 0) || (x < 0) || (y < 0) )
	{
		DisplayBeep(mainscreen);
		return(FALSE);
	}
	ib = &br->br_InitialBrush;
	db = &br->br_DrawBrush;

	FreeBrushBitMaps(br);

	if( !(ib->bi_Image = CreateBitMap(width, height, srcbm->Depth, FALSE)) )
	{
		DisplayBeep(mainscreen);
		return(FALSE);
	}

	if( !(ib->bi_Mask = CreateBitMap(width, height, 1, FALSE)) )
	{
		DestroyBitMap(ib->bi_Image,width,height,srcbm->Depth);
		ib->bi_Image = NULL;
		DisplayBeep(mainscreen);
		return(FALSE);
	}

	if( !(db->bi_Image = CreateBitMap(width, height, srcbm->Depth, FALSE)) )
	{
		DestroyBitMap(ib->bi_Image,width,height,srcbm->Depth);
		DestroyBitMap(ib->bi_Mask,width,height,1);
		ib->bi_Image = NULL;
		ib->bi_Mask = NULL;
		DisplayBeep(mainscreen);
		return(FALSE);
	}
	if( !(db->bi_Mask = CreateBitMap(width, height, 1, FALSE)) )
	{
		DestroyBitMap(ib->bi_Image,width,height,srcbm->Depth);
		DestroyBitMap(ib->bi_Mask,width,height,1);
		DestroyBitMap(db->bi_Image,width,height,srcbm->Depth);
		ib->bi_Image = NULL;
		ib->bi_Mask = NULL;
		db->bi_Image = NULL;
		DisplayBeep(mainscreen);
		return(FALSE);
	}

	br->br_ID = BR_CUSTOM;
	ib->bi_Width = width;
	ib->bi_Height = height;
	ib->bi_Depth = srcbm->Depth;
	ib->bi_XHandle = width>>1;
	ib->bi_YHandle = height>>1;
	/*My*/BltBitMap(srcbm,x,y,ib->bi_Image,0,0,width,height,0xC0,0xFF,NULL);
	db->bi_Width = width;
	db->bi_Height = height;
	db->bi_Depth = srcbm->Depth;
	db->bi_XHandle = width>>1;
	db->bi_YHandle = height>>1;
	/*My*/BltBitMap(srcbm,x,y,db->bi_Image,0,0,width,height,0xC0,0xFF,NULL);

	NastyMakeColourMask(ib->bi_Image, ib->bi_Mask, width, height, rmbcol);

	BltBitMap(ib->bi_Mask,0,0,db->bi_Mask,0,0,width,height,0xC0,0xFF,NULL);

	/* Reverse Mask */
	//BltBitMap(br->br_MaskBitMap,0,0,br->br_MaskBitMap,0,0,width,height,
	//					0x30,1,NULL);
	
	return(TRUE);
}

void FreeBrushBitMaps(struct Brush *br)
{
	struct BrushInfo *ib,*db;

	if(br)
	{
		ib = &br->br_InitialBrush;
		db = &br->br_DrawBrush;

		if( ib->bi_Image )
		{
			DestroyBitMap(ib->bi_Image,ib->bi_Width,ib->bi_Height,ib->bi_Depth);
			ib->bi_Image = NULL;
		}
		if( ib->bi_Mask )
		{
			DestroyBitMap(ib->bi_Mask,ib->bi_Width,ib->bi_Height,1);
			ib->bi_Mask = NULL;
		}
		if( db->bi_Image )
		{
			DestroyBitMap(db->bi_Image,db->bi_Width,db->bi_Height,db->bi_Depth);
			db->bi_Image = NULL;
		}
		if( db->bi_Mask )
		{
			DestroyBitMap(db->bi_Mask,db->bi_Width,db->bi_Height,1);
			db->bi_Mask = NULL;
		}
	}
}

BOOL DrawCustomBrush(struct BrushInfo *bi,struct ProjectWindow *pw, WORD brushleft, WORD brushtop,
										 struct Rectangle *rec, BOOL premagnified)
{
	WORD brushright,brushbottom;
	WORD winleft,winright,wintop,winbottom;
	WORD visablebrushleft,visablebrushright,visablebrushtop,visablebrushbottom;
	WORD winrelleft,winrelwidth,winreltop,winrelheight;
	struct Window *win;
	struct QuickMagnify qm;

	if( (!bi) || (!pw) || (!pw->pw_sw.sw_Window) )
		return(FALSE);

	brushright = brushleft + bi->bi_Width - 1;
	brushbottom = brushtop + bi->bi_Height - 1;

	winleft = pw->pw_XOffset;
	winright = winleft + pw->pw_MagWidth - 1;
	if( winright > (pw->pw_Project->pj_Width-1) )
		winright = pw->pw_Project->pj_Width - 1;

	wintop = pw->pw_YOffset;
	winbottom = wintop + pw->pw_MagHeight - 1;
	if( winbottom > (pw->pw_Project->pj_Height-1) )
		winbottom = pw->pw_Project->pj_Height - 1;

	if( (brushleft > winright) || (brushright < winleft) ||
			(brushtop > winbottom) || (brushbottom < wintop) )
		return(FALSE);

	/* Ok, so if we get here then the brush is definitely on screen somewhere */

	visablebrushleft = brushleft;
	visablebrushright = brushright;
	visablebrushtop = brushtop;
	visablebrushbottom = brushbottom;

	if(brushleft < winleft) visablebrushleft = winleft;
	if(brushright > winright) visablebrushright = winright;
	if(brushtop < wintop) visablebrushtop = wintop;
	if(brushbottom > winbottom) visablebrushbottom = winbottom;

	winrelleft = (visablebrushleft - pw->pw_XOffset) * pw->pw_PixelSize;
	winrelwidth = (visablebrushright - visablebrushleft + 1) * pw->pw_PixelSize;
	winreltop = (visablebrushtop - pw->pw_YOffset) * pw->pw_PixelSize;
	winrelheight = (visablebrushbottom - visablebrushtop + 1) * pw->pw_PixelSize;

	win = pw->pw_sw.sw_Window;

	if(premagnified)
	{
		BltMaskBitMapRastPort(workbm, (visablebrushleft - brushleft)*pw->pw_PixelSize, (visablebrushtop - brushtop)*pw->pw_PixelSize, win->RPort,
													win->BorderLeft + winrelleft, win->BorderTop + winreltop,
													winrelwidth, winrelheight, 0xE0 , workmaskbm->Planes[0]);
	}
	else
	{
		if(pw->pw_PixelSize != 1)
		{
			qm.qm_SrcBitMap = bi->bi_Image;
			qm.qm_SrcXOffset = visablebrushleft - brushleft;
			qm.qm_SrcYOffset = visablebrushtop - brushtop;
			qm.qm_SrcWidth = (visablebrushright - visablebrushleft + 1);
			qm.qm_SrcHeight = (visablebrushbottom - visablebrushtop + 1);
			qm.qm_DestBitMap = workbm;
			qm.qm_DestXOffset = 0;
			qm.qm_DestYOffset = 0;
			qm.qm_DestPixelSize = pw->pw_PixelSize;
			QuickMagnifyBitMap(&qm);
			qm.qm_SrcBitMap = bi->bi_Mask;
			qm.qm_SrcXOffset = visablebrushleft - brushleft;
			qm.qm_SrcYOffset = visablebrushtop - brushtop;
			qm.qm_SrcWidth = (visablebrushright - visablebrushleft + 1);
			qm.qm_SrcHeight = (visablebrushbottom - visablebrushtop + 1);
			qm.qm_DestBitMap = workmaskbm;
			qm.qm_DestXOffset = 0;
			qm.qm_DestYOffset = 0;
			qm.qm_DestPixelSize = pw->pw_PixelSize;
			QuickMagnifyBitMap(&qm);
			BltMaskBitMapRastPort(workbm, 0, 0, win->RPort,
														win->BorderLeft + winrelleft, win->BorderTop + winreltop,
														winrelwidth, winrelheight, 0xE0 , workmaskbm->Planes[0]);
		}
		else
		{
			BltMaskBitMapRastPort(bi->bi_Image, (visablebrushleft - brushleft), (visablebrushtop - brushtop), win->RPort,
														win->BorderLeft + winrelleft, win->BorderTop + winreltop,
														winrelwidth, winrelheight, 0xE0 , bi->bi_Mask->Planes[0]);
		}
	}
	if(rec)
	{
		rec->MinX = visablebrushleft;
		rec->MinY = visablebrushtop;
		rec->MaxX = visablebrushright;
		rec->MaxY = visablebrushbottom;
	}

	return(TRUE);
}


BOOL WipeWithCustomBrush(struct BrushInfo *bi,struct ProjectWindow *pw,
												 WORD brushleft, WORD brushtop,
												 struct Rectangle *rec, BOOL premagnified, UWORD col)
{
	WORD brushright,brushbottom;
	WORD winleft,winright,wintop,winbottom;
	WORD visablebrushleft,visablebrushright,visablebrushtop,visablebrushbottom;
	WORD winrelleft,winrelwidth,winreltop,winrelheight;
	PLANEPTR oldplane1;
	UBYTE olddepth;
	UWORD plane;

	struct Window *win;
	struct QuickMagnify qm;

	if( (!bi) || (!pw) || (!pw->pw_sw.sw_Window) )
		return(FALSE);

	//ClearBitMap(workmaskbm);

	brushright = brushleft + bi->bi_Width - 1;
	brushbottom = brushtop + bi->bi_Height - 1;

	winleft = pw->pw_XOffset;
	winright = winleft + pw->pw_MagWidth - 1;
	if( winright > (pw->pw_Project->pj_Width - 1) )
		winright = pw->pw_Project->pj_Width - 1;

	wintop = pw->pw_YOffset;
	winbottom = wintop + pw->pw_MagHeight - 1;
	if( winbottom > (pw->pw_Project->pj_Height - 1) )
		winbottom = pw->pw_Project->pj_Height - 1;

	if( (brushleft > winright) || (brushright < winleft) ||
			(brushtop > winbottom) || (brushbottom < wintop) )
		return(FALSE);

	/* Ok, so if we get here then the brush is definitely on screen somewhere */

	visablebrushleft = brushleft;
	visablebrushright = brushright;
	visablebrushtop = brushtop;
	visablebrushbottom = brushbottom;

	if(brushleft < winleft) visablebrushleft = winleft;
	if(brushright > winright) visablebrushright = winright;
	if(brushtop < wintop) visablebrushtop = wintop;
	if(brushbottom > winbottom) visablebrushbottom = winbottom;

	winrelleft = (visablebrushleft - pw->pw_XOffset) * pw->pw_PixelSize;
	winrelwidth = (visablebrushright - visablebrushleft + 1) * pw->pw_PixelSize;
	winreltop = (visablebrushtop - pw->pw_YOffset) * pw->pw_PixelSize;
	winrelheight = (visablebrushbottom - visablebrushtop + 1) * pw->pw_PixelSize;

	win = pw->pw_sw.sw_Window;

	if(premagnified)
	{
		BltMaskBitMapRastPort(workbm, (visablebrushleft - brushleft)*pw->pw_PixelSize, (visablebrushtop - brushtop)*pw->pw_PixelSize, win->RPort,
											win->BorderLeft + winrelleft, win->BorderTop + winreltop,
											winrelwidth, winrelheight, 0xE0, workmaskbm->Planes[0]);
	}
	else
	{
		if(pw->pw_PixelSize != 1)
		{
			qm.qm_SrcBitMap = bi->bi_Mask;
			qm.qm_SrcXOffset = visablebrushleft - brushleft;
			qm.qm_SrcYOffset = visablebrushtop - brushtop;
			qm.qm_SrcWidth = (visablebrushright - visablebrushleft + 1);
			qm.qm_SrcHeight = (visablebrushbottom - visablebrushtop + 1);
			qm.qm_DestBitMap = workmaskbm;
			qm.qm_DestXOffset = 0;
			qm.qm_DestYOffset = 0;
			qm.qm_DestPixelSize = pw->pw_PixelSize;
			QuickMagnifyBitMap(&qm);

			olddepth = workbm->Depth;
			workbm->Depth = 1;
			oldplane1 = workbm->Planes[0];

			for(plane = 0; plane < workbmdepth; plane++)
			{
				workbm->Planes[0] = workbm->Planes[plane];

				if(col & (1<<plane))
					BltBitMap(workmaskbm,0,0,workbm,0,0,winrelwidth,winrelheight,0xC0,0xFF,NULL);
				else
					BltBitMap(workmaskbm,0,0,workbm,0,0,winrelwidth,winrelheight,0x00,0xFF,NULL);
			}
			workbm->Planes[0] = oldplane1;
			workbm->Depth = olddepth;

			BltMaskBitMapRastPort(workbm, 0, 0, win->RPort,
														win->BorderLeft + winrelleft, win->BorderTop + winreltop,
														winrelwidth, winrelheight, 0xE0 , workmaskbm->Planes[0]);
		}
		else
		{
			olddepth = workbm->Depth;
			workbm->Depth = 1;
			oldplane1 = workbm->Planes[0];

			BltBitMap(bi->bi_Mask, (visablebrushleft - brushleft), (visablebrushtop - brushtop),
								workmaskbm, 0, 0,
								winrelwidth, winrelheight,
								0xC0,0xFF,NULL);

			for(plane = 0; plane < workbmdepth; plane++)
			{
				workbm->Planes[0] = workbm->Planes[plane];

				if(col & (1<<plane))
					BltBitMap(workmaskbm,0,0,workbm,0,0,winrelwidth,winrelheight,0xC0,0xFF,NULL);
				else
					BltBitMap(workmaskbm,0,0,workbm,0,0,winrelwidth,winrelheight,0x00,0xFF,NULL);
			}
			workbm->Planes[0] = oldplane1;
			workbm->Depth = olddepth;
			BltMaskBitMapRastPort(workbm, 0, 0, win->RPort,
														win->BorderLeft + winrelleft, win->BorderTop + winreltop,
														winrelwidth, winrelheight, 0xE0 , workmaskbm->Planes[0]);
		}
	}
	if(rec)
	{
		rec->MinX = visablebrushleft;
		rec->MinY = visablebrushtop;
		rec->MaxX = visablebrushright;
		rec->MaxY = visablebrushbottom;
	}

	return(TRUE);
}

BOOL ReplaceWithCustomBrush(struct BrushInfo *bi,struct ProjectWindow *pw, WORD brushleft, WORD brushtop,
										 struct Rectangle *rec, BOOL premagnified, UWORD col)
{
	WORD brushright,brushbottom;
	WORD winleft,winright,wintop,winbottom;
	WORD visablebrushleft,visablebrushright,visablebrushtop,visablebrushbottom;
	WORD winrelleft,winrelwidth,winreltop,winrelheight;
	struct Window *win;
	struct QuickMagnify qm;

	if( (!bi) || (!pw) || (!pw->pw_sw.sw_Window) )
		return(FALSE);

	brushright = brushleft + bi->bi_Width - 1;
	brushbottom = brushtop + bi->bi_Height - 1;

	winleft = pw->pw_XOffset;
	winright = winleft + pw->pw_MagWidth - 1;
	if( winright > (pw->pw_Project->pj_Width-1) )
		winright = pw->pw_Project->pj_Width - 1;

	wintop = pw->pw_YOffset;
	winbottom = wintop + pw->pw_MagHeight - 1;
	if( winbottom > (pw->pw_Project->pj_Height-1) )
		winbottom = pw->pw_Project->pj_Height - 1;

	if( (brushleft > winright) || (brushright < winleft) ||
			(brushtop > winbottom) || (brushbottom < wintop) )
		return(FALSE);

	/* Ok, so if we get here then the brush is definitely on screen somewhere */

	visablebrushleft = brushleft;
	visablebrushright = brushright;
	visablebrushtop = brushtop;
	visablebrushbottom = brushbottom;

	if(brushleft < winleft) visablebrushleft = winleft;
	if(brushright > winright) visablebrushright = winright;
	if(brushtop < wintop) visablebrushtop = wintop;
	if(brushbottom > winbottom) visablebrushbottom = winbottom;

	winrelleft = (visablebrushleft - pw->pw_XOffset) * pw->pw_PixelSize;
	winrelwidth = (visablebrushright - visablebrushleft + 1) * pw->pw_PixelSize;
	winreltop = (visablebrushtop - pw->pw_YOffset) * pw->pw_PixelSize;
	winrelheight = (visablebrushbottom - visablebrushtop + 1) * pw->pw_PixelSize;

	win = pw->pw_sw.sw_Window;

	if(premagnified)
	{
		BltBitMapRastPort(workbm, (visablebrushleft - brushleft)*pw->pw_PixelSize, (visablebrushtop - brushtop)*pw->pw_PixelSize, win->RPort,
											win->BorderLeft + winrelleft, win->BorderTop + winreltop,
											winrelwidth, winrelheight, 0xC0 );
	}
	else
	{
		if(pw->pw_PixelSize != 1)
		{
			qm.qm_SrcBitMap = bi->bi_Image;
			qm.qm_SrcXOffset = visablebrushleft - brushleft;
			qm.qm_SrcYOffset = visablebrushtop - brushtop;
			qm.qm_SrcWidth = (visablebrushright - visablebrushleft + 1);
			qm.qm_SrcHeight = (visablebrushbottom - visablebrushtop + 1);
			qm.qm_DestBitMap = workbm;
			qm.qm_DestXOffset = 0;
			qm.qm_DestYOffset = 0;
			qm.qm_DestPixelSize = pw->pw_PixelSize;
			QuickMagnifyBitMap(&qm);
			BltBitMapRastPort(workbm, 0, 0, win->RPort,
												win->BorderLeft + winrelleft, win->BorderTop + winreltop,
												winrelwidth, winrelheight, 0xC0 );
		}
		else
		{
			BltBitMapRastPort(bi->bi_Image, (visablebrushleft - brushleft), (visablebrushtop - brushtop), win->RPort,
												win->BorderLeft + winrelleft, win->BorderTop + winreltop,
												winrelwidth, winrelheight, 0xC0 );
		}
	}

	if(rec)
	{
		rec->MinX = visablebrushleft;
		rec->MinY = visablebrushtop;
		rec->MaxX = visablebrushright;
		rec->MaxY = visablebrushbottom;
	}

	return(TRUE);
}

BOOL SolidWithCustomBrush(struct BrushInfo *bi,struct ProjectWindow *pw, WORD brushleft, WORD brushtop,
										 struct Rectangle *rec, UWORD col)
{
	WORD brushright,brushbottom;
	WORD winleft,winright,wintop,winbottom;
	WORD visablebrushleft,visablebrushright,visablebrushtop,visablebrushbottom;
	WORD winrelleft,winrelwidth,winreltop,winrelheight;
	struct Window *win;

	if( (!bi) || (!pw) || (!pw->pw_sw.sw_Window) )
		return(FALSE);

	brushright = brushleft + bi->bi_Width - 1;
	brushbottom = brushtop + bi->bi_Height - 1;

	winleft = pw->pw_XOffset;
	winright = winleft + pw->pw_MagWidth - 1;
	if( winright > (pw->pw_Project->pj_Width-1) )
		winright = pw->pw_Project->pj_Width - 1;

	wintop = pw->pw_YOffset;
	winbottom = wintop + pw->pw_MagHeight - 1;
	if( winbottom > (pw->pw_Project->pj_Height-1) )
		winbottom = pw->pw_Project->pj_Height - 1;

	if( (brushleft > winright) || (brushright < winleft) ||
			(brushtop > winbottom) || (brushbottom < wintop) )
		return(FALSE);

	/* Ok, so if we get here then the brush is definitely on screen somewhere */

	visablebrushleft = brushleft;
	visablebrushright = brushright;
	visablebrushtop = brushtop;
	visablebrushbottom = brushbottom;

	if(brushleft < winleft) visablebrushleft = winleft;
	if(brushright > winright) visablebrushright = winright;
	if(brushtop < wintop) visablebrushtop = wintop;
	if(brushbottom > winbottom) visablebrushbottom = winbottom;

	winrelleft = (visablebrushleft - pw->pw_XOffset) * pw->pw_PixelSize;
	winrelwidth = (visablebrushright - visablebrushleft + 1) * pw->pw_PixelSize;
	winreltop = (visablebrushtop - pw->pw_YOffset) * pw->pw_PixelSize;
	winrelheight = (visablebrushbottom - visablebrushtop + 1) * pw->pw_PixelSize;

	win = pw->pw_sw.sw_Window;

	SetAPen(win->RPort,col);
	RectFill(win->RPort,win->BorderLeft+winrelleft,  win->BorderTop+winreltop,
		win->BorderLeft+winrelleft+winrelwidth-1,  win->BorderTop+winreltop+winrelheight-1);

	if(rec)
	{
		rec->MinX = visablebrushleft;
		rec->MinY = visablebrushtop;
		rec->MaxX = visablebrushright;
		rec->MaxY = visablebrushbottom;
	}

	return(TRUE);
}


BOOL SetupDefaultBrushes(void)
{
	BOOL ret = FALSE;

	brushes[BR_DOT].br_ID													= BR_DOT;
	brushes[BR_DOT].br_InitialBrush.bi_Width			= 1;
	brushes[BR_DOT].br_InitialBrush.bi_Height			= 1;
	brushes[BR_DOT].br_InitialBrush.bi_Depth			= 0;
	brushes[BR_DOT].br_InitialBrush.bi_XHandle		= 0;
	brushes[BR_DOT].br_InitialBrush.bi_YHandle		= 0;
	brushes[BR_DOT].br_InitialBrush.bi_Image			= NULL;
	brushes[BR_DOT].br_InitialBrush.bi_Mask				= NULL;
	brushes[BR_DOT].br_DrawBrush.bi_Width					= 1;
	brushes[BR_DOT].br_DrawBrush.bi_Height				= 1;
	brushes[BR_DOT].br_DrawBrush.bi_Depth					= 0;
	brushes[BR_DOT].br_DrawBrush.bi_XHandle				= 0;
	brushes[BR_DOT].br_DrawBrush.bi_YHandle				= 0;
	brushes[BR_DOT].br_DrawBrush.bi_Image					= NULL;
	brushes[BR_DOT].br_DrawBrush.bi_Mask					= NULL;

	brushes[BR_BRUSH1].br_ID													= BR_BRUSH1;
	brushes[BR_BRUSH1].br_InitialBrush.bi_Width				= 2;
	brushes[BR_BRUSH1].br_InitialBrush.bi_Height			= 2;
	brushes[BR_BRUSH1].br_InitialBrush.bi_Depth				= 1;
	brushes[BR_BRUSH1].br_InitialBrush.bi_XHandle			= 0;
	brushes[BR_BRUSH1].br_InitialBrush.bi_YHandle			= 0;
	brushes[BR_BRUSH1].br_InitialBrush.bi_Image				= NULL;
	brushes[BR_BRUSH1].br_InitialBrush.bi_Mask				= NULL;
	brushes[BR_BRUSH1].br_DrawBrush.bi_Width					= 2;
	brushes[BR_BRUSH1].br_DrawBrush.bi_Height					= 2;
	brushes[BR_BRUSH1].br_DrawBrush.bi_Depth					= 1;
	brushes[BR_BRUSH1].br_DrawBrush.bi_XHandle				= 0;
	brushes[BR_BRUSH1].br_DrawBrush.bi_YHandle				= 0;
	brushes[BR_BRUSH1].br_DrawBrush.bi_Image					= NULL;
	if( !(brushes[BR_BRUSH1].br_DrawBrush.bi_Mask = CreateBitMap(2, 2, 1, FALSE)) ) goto Abort;
	InitBrush1Bitmaps();

	brushes[BR_BRUSH2].br_ID													= BR_BRUSH2;
	brushes[BR_BRUSH2].br_InitialBrush.bi_Width				= 3;
	brushes[BR_BRUSH2].br_InitialBrush.bi_Height			= 3;
	brushes[BR_BRUSH2].br_InitialBrush.bi_Depth				= 1;
	brushes[BR_BRUSH2].br_InitialBrush.bi_XHandle			= 1;
	brushes[BR_BRUSH2].br_InitialBrush.bi_YHandle			= 1;
	brushes[BR_BRUSH2].br_InitialBrush.bi_Image				= NULL;
	brushes[BR_BRUSH2].br_InitialBrush.bi_Mask				= NULL;
	brushes[BR_BRUSH2].br_DrawBrush.bi_Width					= 3;
	brushes[BR_BRUSH2].br_DrawBrush.bi_Height					= 3;
	brushes[BR_BRUSH2].br_DrawBrush.bi_Depth					= 1;
	brushes[BR_BRUSH2].br_DrawBrush.bi_XHandle				= 1;
	brushes[BR_BRUSH2].br_DrawBrush.bi_YHandle				= 1;
	brushes[BR_BRUSH2].br_DrawBrush.bi_Image					= NULL;
	if( !(brushes[BR_BRUSH2].br_DrawBrush.bi_Mask = CreateBitMap(3, 3, 1, FALSE)) ) goto Abort;
	InitBrush2Bitmaps();

	brushes[BR_BRUSH3].br_ID													= BR_BRUSH3;
	brushes[BR_BRUSH3].br_InitialBrush.bi_Width				= 4;
	brushes[BR_BRUSH3].br_InitialBrush.bi_Height			= 4;
	brushes[BR_BRUSH3].br_InitialBrush.bi_Depth				= 1;
	brushes[BR_BRUSH3].br_InitialBrush.bi_XHandle			= 1;
	brushes[BR_BRUSH3].br_InitialBrush.bi_YHandle			= 1;
	brushes[BR_BRUSH3].br_InitialBrush.bi_Image				= NULL;
	brushes[BR_BRUSH3].br_InitialBrush.bi_Mask				= NULL;
	brushes[BR_BRUSH3].br_DrawBrush.bi_Width					= 4;
	brushes[BR_BRUSH3].br_DrawBrush.bi_Height					= 4;
	brushes[BR_BRUSH3].br_DrawBrush.bi_Depth					= 1;
	brushes[BR_BRUSH3].br_DrawBrush.bi_XHandle				= 1;
	brushes[BR_BRUSH3].br_DrawBrush.bi_YHandle				= 1;
	brushes[BR_BRUSH3].br_DrawBrush.bi_Image					= NULL;
	if( !(brushes[BR_BRUSH3].br_DrawBrush.bi_Mask = CreateBitMap(4, 4, 1, FALSE)) ) goto Abort;
	InitBrush3Bitmaps();

	brushes[BR_BRUSH4].br_ID													= BR_BRUSH4;
	brushes[BR_BRUSH4].br_InitialBrush.bi_Width				= 5;
	brushes[BR_BRUSH4].br_InitialBrush.bi_Height			= 5;
	brushes[BR_BRUSH4].br_InitialBrush.bi_Depth				= 1;
	brushes[BR_BRUSH4].br_InitialBrush.bi_XHandle			= 2;
	brushes[BR_BRUSH4].br_InitialBrush.bi_YHandle			= 2;
	brushes[BR_BRUSH4].br_InitialBrush.bi_Image				= NULL;
	brushes[BR_BRUSH4].br_InitialBrush.bi_Mask				= NULL;
	brushes[BR_BRUSH4].br_DrawBrush.bi_Width					= 5;
	brushes[BR_BRUSH4].br_DrawBrush.bi_Height					= 5;
	brushes[BR_BRUSH4].br_DrawBrush.bi_Depth					= 1;
	brushes[BR_BRUSH4].br_DrawBrush.bi_XHandle				= 2;
	brushes[BR_BRUSH4].br_DrawBrush.bi_YHandle				= 2;
	brushes[BR_BRUSH4].br_DrawBrush.bi_Image					= NULL;
	if( !(brushes[BR_BRUSH4].br_DrawBrush.bi_Mask = CreateBitMap(5, 5, 1, FALSE)) ) goto Abort;
	InitBrush4Bitmaps();

	brushes[BR_BRUSH5].br_ID													= BR_BRUSH5;
	brushes[BR_BRUSH5].br_InitialBrush.bi_Width				= 6;
	brushes[BR_BRUSH5].br_InitialBrush.bi_Height			= 6;
	brushes[BR_BRUSH5].br_InitialBrush.bi_Depth				= 1;
	brushes[BR_BRUSH5].br_InitialBrush.bi_XHandle			= 3;
	brushes[BR_BRUSH5].br_InitialBrush.bi_YHandle			= 3;
	brushes[BR_BRUSH5].br_InitialBrush.bi_Image				= NULL;
	brushes[BR_BRUSH5].br_InitialBrush.bi_Mask				= NULL;
	brushes[BR_BRUSH5].br_DrawBrush.bi_Width					= 6;
	brushes[BR_BRUSH5].br_DrawBrush.bi_Height					= 6;
	brushes[BR_BRUSH5].br_DrawBrush.bi_Depth					= 1;
	brushes[BR_BRUSH5].br_DrawBrush.bi_XHandle				= 3;
	brushes[BR_BRUSH5].br_DrawBrush.bi_YHandle				= 3;
	brushes[BR_BRUSH5].br_DrawBrush.bi_Image					= NULL;
	if( !(brushes[BR_BRUSH5].br_DrawBrush.bi_Mask = CreateBitMap(6, 6, 1, FALSE)) ) goto Abort;
	InitBrush5Bitmaps();

	brushes[BR_BRUSH6].br_ID													= BR_BRUSH6;
	brushes[BR_BRUSH6].br_InitialBrush.bi_Width				= 7;
	brushes[BR_BRUSH6].br_InitialBrush.bi_Height			= 7;
	brushes[BR_BRUSH6].br_InitialBrush.bi_Depth				= 1;
	brushes[BR_BRUSH6].br_InitialBrush.bi_XHandle			= 3;
	brushes[BR_BRUSH6].br_InitialBrush.bi_YHandle			= 3;
	brushes[BR_BRUSH6].br_InitialBrush.bi_Image				= NULL;
	brushes[BR_BRUSH6].br_InitialBrush.bi_Mask				= NULL;
	brushes[BR_BRUSH6].br_DrawBrush.bi_Width					= 7;
	brushes[BR_BRUSH6].br_DrawBrush.bi_Height					= 7;
	brushes[BR_BRUSH6].br_DrawBrush.bi_Depth					= 1;
	brushes[BR_BRUSH6].br_DrawBrush.bi_XHandle				= 3;
	brushes[BR_BRUSH6].br_DrawBrush.bi_YHandle				= 3;
	brushes[BR_BRUSH6].br_DrawBrush.bi_Image					= NULL;
	brushes[BR_BRUSH6].br_DrawBrush.bi_Mask						= NULL;
	if( !(brushes[BR_BRUSH6].br_DrawBrush.bi_Mask = CreateBitMap(7, 7, 1, FALSE)) ) goto Abort;
	InitBrush6Bitmaps();

	brushes[BR_BRUSH7].br_ID													= BR_BRUSH7;
	brushes[BR_BRUSH7].br_InitialBrush.bi_Width				= 10;
	brushes[BR_BRUSH7].br_InitialBrush.bi_Height			= 10;
	brushes[BR_BRUSH7].br_InitialBrush.bi_Depth				= 1;
	brushes[BR_BRUSH7].br_InitialBrush.bi_XHandle			= 5;
	brushes[BR_BRUSH7].br_InitialBrush.bi_YHandle			= 5;
	brushes[BR_BRUSH7].br_InitialBrush.bi_Image				= NULL;
	brushes[BR_BRUSH7].br_InitialBrush.bi_Mask				= NULL;
	brushes[BR_BRUSH7].br_DrawBrush.bi_Width					= 10;
	brushes[BR_BRUSH7].br_DrawBrush.bi_Height					= 10;
	brushes[BR_BRUSH7].br_DrawBrush.bi_Depth					= 1;
	brushes[BR_BRUSH7].br_DrawBrush.bi_XHandle				= 5;
	brushes[BR_BRUSH7].br_DrawBrush.bi_YHandle				= 5;
	brushes[BR_BRUSH7].br_DrawBrush.bi_Image					= NULL;
	if( !(brushes[BR_BRUSH7].br_DrawBrush.bi_Mask = CreateBitMap(10, 10, 1, FALSE)) ) goto Abort;
	InitBrush7Bitmaps();

	brushes[BR_BRUSH8].br_ID													= BR_BRUSH8;
	brushes[BR_BRUSH8].br_InitialBrush.bi_Width				= 16;
	brushes[BR_BRUSH8].br_InitialBrush.bi_Height			= 16;
	brushes[BR_BRUSH8].br_InitialBrush.bi_Depth				= 1;
	brushes[BR_BRUSH8].br_InitialBrush.bi_XHandle			= 8;
	brushes[BR_BRUSH8].br_InitialBrush.bi_YHandle			= 8;
	brushes[BR_BRUSH8].br_InitialBrush.bi_Image				= NULL;
	brushes[BR_BRUSH8].br_InitialBrush.bi_Mask				= NULL;
	brushes[BR_BRUSH8].br_DrawBrush.bi_Width					= 16;
	brushes[BR_BRUSH8].br_DrawBrush.bi_Height					= 16;
	brushes[BR_BRUSH8].br_DrawBrush.bi_Depth					= 1;
	brushes[BR_BRUSH8].br_DrawBrush.bi_XHandle				= 8;
	brushes[BR_BRUSH8].br_DrawBrush.bi_YHandle				= 8;
	brushes[BR_BRUSH8].br_DrawBrush.bi_Image					= NULL;
	if( !(brushes[BR_BRUSH8].br_DrawBrush.bi_Mask = CreateBitMap(16, 16, 1, FALSE)) ) goto Abort;
	InitBrush8Bitmaps();

	brushes[BR_CUSTOM].br_ID													= BR_CUSTOM;
	brushes[BR_CUSTOM].br_InitialBrush.bi_Width				= 1;
	brushes[BR_CUSTOM].br_InitialBrush.bi_Height			= 1;
	brushes[BR_CUSTOM].br_InitialBrush.bi_Depth				= 1;
	brushes[BR_CUSTOM].br_InitialBrush.bi_XHandle			= 8;
	brushes[BR_CUSTOM].br_InitialBrush.bi_YHandle			= 8;
	brushes[BR_CUSTOM].br_InitialBrush.bi_Image				= NULL;
	brushes[BR_CUSTOM].br_InitialBrush.bi_Mask				= NULL;
	brushes[BR_CUSTOM].br_DrawBrush.bi_Width					= 1;
	brushes[BR_CUSTOM].br_DrawBrush.bi_Height					= 1;
	brushes[BR_CUSTOM].br_DrawBrush.bi_Depth					= 1;
	brushes[BR_CUSTOM].br_DrawBrush.bi_XHandle				= 0;
	brushes[BR_CUSTOM].br_DrawBrush.bi_YHandle				= 0;
	brushes[BR_CUSTOM].br_DrawBrush.bi_Image					= NULL;
	brushes[BR_CUSTOM].br_DrawBrush.bi_Mask						= NULL;

	brushes[BR_ANIMBRUSH].br_ID												= BR_ANIMBRUSH;
	brushes[BR_ANIMBRUSH].br_InitialBrush.bi_Width		= 1;
	brushes[BR_ANIMBRUSH].br_InitialBrush.bi_Height		= 1;
	brushes[BR_ANIMBRUSH].br_InitialBrush.bi_Depth		= 1;
	brushes[BR_ANIMBRUSH].br_InitialBrush.bi_XHandle	= 8;
	brushes[BR_ANIMBRUSH].br_InitialBrush.bi_YHandle	= 8;
	brushes[BR_ANIMBRUSH].br_InitialBrush.bi_Image		= NULL;
	brushes[BR_ANIMBRUSH].br_InitialBrush.bi_Mask			= NULL;
	brushes[BR_ANIMBRUSH].br_DrawBrush.bi_Width				= 1;
	brushes[BR_ANIMBRUSH].br_DrawBrush.bi_Height			= 1;
	brushes[BR_ANIMBRUSH].br_DrawBrush.bi_Depth				= 1;
	brushes[BR_ANIMBRUSH].br_DrawBrush.bi_XHandle			= 0;
	brushes[BR_ANIMBRUSH].br_DrawBrush.bi_YHandle			= 0;
	brushes[BR_ANIMBRUSH].br_DrawBrush.bi_Image				= NULL;
	brushes[BR_ANIMBRUSH].br_DrawBrush.bi_Mask				= NULL;

	ret = TRUE;
Abort:
	return(ret);
}

static void InitBrush1Bitmaps(void)
{
	UWORD *ptr, wordsperrow;
	struct BitMap *bm;
	UWORD	src[] = {0xC000,0xC000};

	if(bm = brushes[BR_BRUSH1].br_DrawBrush.bi_Mask)
	{
		wordsperrow = bm->BytesPerRow/2;
		ptr = (UWORD *)bm->Planes[0];
		*ptr = src[0];	ptr += wordsperrow;
		*ptr = src[1];
	}
}

static void InitBrush2Bitmaps(void)
{
	UWORD *ptr, wordsperrow;
	struct BitMap *bm;
	UWORD	src[] = {0x4000,0xE000,0x4000};

	if(bm = brushes[BR_BRUSH2].br_DrawBrush.bi_Mask)
	{
		wordsperrow = bm->BytesPerRow/2;
		ptr = (UWORD *)bm->Planes[0];
		*ptr = src[0];	ptr += wordsperrow;
		*ptr = src[1];	ptr += wordsperrow;
		*ptr = src[2];
	}
}

static void InitBrush3Bitmaps(void)
{
	UWORD *ptr, wordsperrow;
	struct BitMap *bm;
	UWORD	src[] = {0x6000,0xF000,0xF000,0x6000};

	if(bm = brushes[BR_BRUSH3].br_DrawBrush.bi_Mask)
	{
		wordsperrow = bm->BytesPerRow/2;
		ptr = (UWORD *)bm->Planes[0];
		*ptr = src[0];	ptr += wordsperrow;
		*ptr = src[1];	ptr += wordsperrow;
		*ptr = src[2];	ptr += wordsperrow;
		*ptr = src[3];
	}
}

static void InitBrush4Bitmaps(void)
{
	UWORD *ptr, wordsperrow;
	struct BitMap *bm;
	UWORD	src[] = {0x7000,0xF800,0xF800,0xF800,0x7000};

	if(bm = brushes[BR_BRUSH4].br_DrawBrush.bi_Mask)
	{
		wordsperrow = bm->BytesPerRow/2;
		ptr = (UWORD *)bm->Planes[0];
		*ptr = src[0];	ptr += wordsperrow;
		*ptr = src[1];	ptr += wordsperrow;
		*ptr = src[2];	ptr += wordsperrow;
		*ptr = src[3];	ptr += wordsperrow;
		*ptr = src[4];
	}
}

static void InitBrush5Bitmaps(void)
{
	UWORD *ptr, wordsperrow;
	struct BitMap *bm;
	UWORD	src[] = {0x3000,0x7800,0xFC00,0xFC00,0x7800,0x3000};

	if(bm = brushes[BR_BRUSH5].br_DrawBrush.bi_Mask)
	{
		wordsperrow = bm->BytesPerRow/2;
		ptr = (UWORD *)bm->Planes[0];
		*ptr = src[0];	ptr += wordsperrow;
		*ptr = src[1];	ptr += wordsperrow;
		*ptr = src[2];	ptr += wordsperrow;
		*ptr = src[3];	ptr += wordsperrow;
		*ptr = src[4];	ptr += wordsperrow;
		*ptr = src[5];
	}
}

static void InitBrush6Bitmaps(void)
{
	UWORD *ptr, wordsperrow;
	struct BitMap *bm;
	UWORD	src[] = {0x3800,0x7C00,0xFE00,0xFE00,0xFE00,0x7C00,0x3800};

	if(bm = brushes[BR_BRUSH6].br_DrawBrush.bi_Mask)
	{
		wordsperrow = bm->BytesPerRow/2;
		ptr = (UWORD *)bm->Planes[0];
		*ptr = src[0];	ptr += wordsperrow;
		*ptr = src[1];	ptr += wordsperrow;
		*ptr = src[2];	ptr += wordsperrow;
		*ptr = src[3];	ptr += wordsperrow;
		*ptr = src[4];	ptr += wordsperrow;
		*ptr = src[5];	ptr += wordsperrow;
		*ptr = src[6];
	}
}

static void InitBrush7Bitmaps(void)
{
	UWORD *ptr, wordsperrow;
	struct BitMap *bm;
	UWORD	src[] = {0x0C00,0x3F00,0x7F80,0x7F80,0xFFC0,0xFFC0,0x7F80,0x7F80,0x3F00,0x0C00};

	if(bm = brushes[BR_BRUSH7].br_DrawBrush.bi_Mask)
	{
		wordsperrow = bm->BytesPerRow/2;
		ptr = (UWORD *)bm->Planes[0];
		*ptr = src[0];	ptr += wordsperrow;
		*ptr = src[1];	ptr += wordsperrow;
		*ptr = src[2];	ptr += wordsperrow;
		*ptr = src[3];	ptr += wordsperrow;
		*ptr = src[4];	ptr += wordsperrow;
		*ptr = src[5];	ptr += wordsperrow;
		*ptr = src[6];	ptr += wordsperrow;
		*ptr = src[7];	ptr += wordsperrow;
		*ptr = src[8];	ptr += wordsperrow;
		*ptr = src[9];
	}
}

static void InitBrush8Bitmaps(void)
{
	UWORD *ptr, wordsperrow;
	struct BitMap *bm;
	UWORD	src[] = {0x07E0,0x1FF8,0x3FFC,0x7FFE,0x7FFE,0xFFFF,0xFFFF,0xFFFF,0xFFFF,0xFFFF,0xFFFF,0x7FFE,0x7FFE,0x3FFC,0x1FF8,0x07E0};

	if(bm = brushes[BR_BRUSH8].br_DrawBrush.bi_Mask)
	{
		wordsperrow = bm->BytesPerRow/2;
		ptr = (UWORD *)bm->Planes[0];
		*ptr = src[0];	ptr += wordsperrow;
		*ptr = src[1];	ptr += wordsperrow;
		*ptr = src[2];	ptr += wordsperrow;
		*ptr = src[3];	ptr += wordsperrow;
		*ptr = src[4];	ptr += wordsperrow;
		*ptr = src[5];	ptr += wordsperrow;
		*ptr = src[6];	ptr += wordsperrow;
		*ptr = src[7];	ptr += wordsperrow;
		*ptr = src[8];	ptr += wordsperrow;
		*ptr = src[9];	ptr += wordsperrow;
		*ptr = src[10];	ptr += wordsperrow;
		*ptr = src[11];	ptr += wordsperrow;
		*ptr = src[12];	ptr += wordsperrow;
		*ptr = src[13];	ptr += wordsperrow;
		*ptr = src[14];	ptr += wordsperrow;
		*ptr = src[15];
	}
}

/****************   MagnifyBrushIntoWorkSpace()   ***********************/
//
//  Given a Bitmap brush (ie Custom brush), magnify it into the work buffers.
// If the brush is to big to fit, then it returns an error.
// Returns:	TRUE  - It's in the work bitmap
//					FALSE - Won't fit in the work bitmap.

BOOL MagnifyBrushIntoWorkSpace(struct BrushInfo *bi, UWORD pixelsize, UWORD col, UWORD mode)
{
	struct QuickMagnify qm;
	PLANEPTR oldplane1;
	UBYTE olddepth;
	UWORD plane;

	if(!bi)
		return(FALSE);

	if( ((bi->bi_Width * pixelsize) <  workbmwidth) &&
			((bi->bi_Height * pixelsize) <  workbmheight) )
	{
		switch(mode)
		{
			case DRMD_MATTE:
				qm.qm_SrcBitMap = bi->bi_Image;
				qm.qm_SrcXOffset = 0;
				qm.qm_SrcYOffset = 0;
				qm.qm_SrcWidth = bi->bi_Width;
				qm.qm_SrcHeight = bi->bi_Height;
				qm.qm_DestBitMap = workbm;
				qm.qm_DestXOffset = 0;
				qm.qm_DestYOffset = 0;
				qm.qm_DestPixelSize = pixelsize;
				QuickMagnifyBitMap(&qm);
				qm.qm_SrcBitMap = bi->bi_Mask;
				qm.qm_SrcXOffset = 0;
				qm.qm_SrcYOffset = 0;
				qm.qm_SrcWidth = bi->bi_Width;
				qm.qm_SrcHeight = bi->bi_Height;
				qm.qm_DestBitMap = workmaskbm;
				qm.qm_DestXOffset = 0;
				qm.qm_DestYOffset = 0;
				qm.qm_DestPixelSize = pixelsize;
				QuickMagnifyBitMap(&qm);
				return(TRUE);
				break;
			case DRMD_COLOUR:
				qm.qm_SrcBitMap = bi->bi_Mask;
				qm.qm_SrcXOffset = 0;
				qm.qm_SrcYOffset = 0;
				qm.qm_SrcWidth = bi->bi_Width;
				qm.qm_SrcHeight = bi->bi_Height;
				qm.qm_DestBitMap = workmaskbm;
				qm.qm_DestXOffset = 0;
				qm.qm_DestYOffset = 0;
				qm.qm_DestPixelSize = pixelsize;
				QuickMagnifyBitMap(&qm);

				WaitBlit();
				olddepth = workbm->Depth;
				workbm->Depth = 1;
				oldplane1 = workbm->Planes[0];
				for(plane = 0; plane < workbmdepth; plane++)
				{
					workbm->Planes[0] = workbm->Planes[plane];
	
					if(col & (1<<plane))
						BltBitMap(workmaskbm,0,0,workbm,0,0,bi->bi_Width*pixelsize,bi->bi_Height*pixelsize,0xE0,0xFF,NULL);
					else
						BltBitMap(workmaskbm,0,0,workbm,0,0,bi->bi_Width*pixelsize,bi->bi_Height*pixelsize,0x00,0xFF,NULL);
					WaitBlit();
				}
				workbm->Planes[0] = oldplane1;
				workbm->Depth = olddepth;
				return(TRUE);
				break;
		}
	}
	else
		return(FALSE);
}

/*********** BltBrushToMainBitMap() *********************/
//
// POTENTIAL BUG: If the brush is bigger then the work bitmap, then
// it will most probably crash.

void BltBrushToMainBitMap(struct Project *pj, struct BrushInfo *bi,
													WORD brushleft, WORD brushtop, UWORD col, UWORD mode)
{
	WORD brushright,brushbottom;
	WORD visablebrushleft,visablebrushright,visablebrushtop,visablebrushbottom;
	WORD	x,y;
	WORD  fullbrushwidth, fullbrushheight, brushxoffset,brushyoffset;
	WORD	orgfullbrushwidth, orgbrushxoffset,	orgvisablebrushleft;

	//ClearBitMap(workmaskbm);

	if( (!bi) || (!pj) )
		return;

	brushright = brushleft + bi->bi_Width - 1;
	brushbottom = brushtop + bi->bi_Height - 1;

	if( (brushleft >= pj->pj_Width) || (brushright < 0) ||
			(brushtop >= pj->pj_Height) || (brushbottom < 0) )
		return;

	/* Ok, so if we get here then the brush is definitely on screen somewhere */

	visablebrushleft = brushleft;
	visablebrushright = brushright;
	visablebrushtop = brushtop;
	visablebrushbottom = brushbottom;

	if(brushleft < 0) visablebrushleft = 0;
	if(brushright > (pj->pj_Width-1)) visablebrushright = (pj->pj_Width-1);
	if(brushtop < 0) visablebrushtop = 0;
	if(brushbottom > (pj->pj_Height-1)) visablebrushbottom = (pj->pj_Height-1);

	switch(mode)
	{
		case DRMD_MATTE:
			BltMaskBitMapRastPort(bi->bi_Image, (visablebrushleft - brushleft), (visablebrushtop - brushtop),
														pj->pj_MainRPort, visablebrushleft, visablebrushtop,
														visablebrushright-visablebrushleft+1, visablebrushbottom-visablebrushtop+1, 0xE0 , bi->bi_Mask->Planes[0]);
			break;
		case DRMD_CYCLE:
		case DRMD_COLOUR:
			fullbrushwidth = visablebrushright-visablebrushleft+1;
			fullbrushheight = visablebrushbottom-visablebrushtop+1;
			brushxoffset = visablebrushleft - brushleft;
			brushyoffset = visablebrushtop - brushtop;
			if( (fullbrushwidth <= workbmwidth) && (fullbrushheight <= workbmheight) )
			{
				BltColourBrush(pj,bi,brushxoffset,brushyoffset,
						visablebrushleft, visablebrushtop,
						fullbrushwidth, fullbrushheight,col);
			}
			else
			{
				if( (fullbrushwidth > workbmwidth) && (fullbrushheight > workbmheight) )
				{
					/* Horiz & Vert Split */

					orgfullbrushwidth = fullbrushwidth;
					orgbrushxoffset = brushxoffset;
					orgvisablebrushleft = visablebrushleft;
					while(fullbrushheight > 0)
					{
						if(fullbrushheight > workbmheight)
							y = workbmheight;
						else
							y = fullbrushheight;

						fullbrushwidth = orgfullbrushwidth;
						brushxoffset = orgbrushxoffset;
						visablebrushleft = orgvisablebrushleft;
						while(fullbrushwidth > 0)
						{
							if(fullbrushwidth > workbmwidth)
							{
								BltColourBrush(pj,bi,brushxoffset,brushyoffset,
									visablebrushleft, visablebrushtop,
									workbmwidth, y,col);
							}
							else
							{
								BltColourBrush(pj,bi,brushxoffset,brushyoffset,
									visablebrushleft, visablebrushtop,
									fullbrushwidth, y, col);
							}
							brushxoffset += workbmwidth;
							fullbrushwidth -= workbmwidth;
							visablebrushleft += workbmwidth;
						}
						brushyoffset += workbmheight;
						fullbrushheight -= workbmheight;
						visablebrushtop += workbmheight;		
					}
				}
				else
				{
					/* Horiz Split */
					if(fullbrushwidth > workbmwidth)
					{
						while(fullbrushwidth > 0)
						{
							if(fullbrushwidth > workbmwidth)
							{
								BltColourBrush(pj,bi,brushxoffset,brushyoffset,
									visablebrushleft, visablebrushtop,
									workbmwidth, fullbrushheight,col);
							}
							else
							{
								BltColourBrush(pj,bi,brushxoffset,brushyoffset,
									visablebrushleft, visablebrushtop,
									fullbrushwidth, fullbrushheight,col);
							}
							brushxoffset += workbmwidth;
							fullbrushwidth -= workbmwidth;
							visablebrushleft += workbmwidth;
						}
					}
					else
					{
						/* Vert Split */
						while(fullbrushheight > 0)
						{
							if(fullbrushheight > workbmheight)
							{
								BltColourBrush(pj,bi,brushxoffset,brushyoffset,
									visablebrushleft, visablebrushtop,
									fullbrushwidth, workbmheight, col);
							}
							else
							{
								BltColourBrush(pj,bi,brushxoffset,brushyoffset,
									visablebrushleft, visablebrushtop,
									fullbrushwidth, fullbrushheight, col);
							}
							brushyoffset += workbmheight;
							fullbrushheight -= workbmheight;
							visablebrushtop += workbmheight;		
						}
					}
				}
			}
			break;
		case DRMD_REPLACE:
			BltBitMapRastPort(bi->bi_Image, (visablebrushleft - brushleft), (visablebrushtop - brushtop),
												pj->pj_MainRPort, visablebrushleft, visablebrushtop,
												visablebrushright-visablebrushleft+1, visablebrushbottom-visablebrushtop+1, 0xC0 );
			break;
		case DRMD_SOLID:
			SetAPen(pj->pj_MainRPort,col);
			RectFill(pj->pj_MainRPort,visablebrushleft, visablebrushtop,
																visablebrushright, visablebrushbottom);
			break;
		case DRMD_SMOOTH:
			for(y = (visablebrushtop - brushtop); y <= (visablebrushbottom-brushtop); y++)
			{
				for(x = (visablebrushleft - brushleft); x <= (visablebrushright-brushleft); x++)
				{
					if( GetPixelColour(bi->bi_Mask,x,y) )
					{
						SmoothPoint(pj,brushleft+x,brushtop+y);
					}
				}
			}
			break;
		case DRMD_SHADE:
			if(col == lmbcol)
				col = FALSE;
			else
				col = TRUE;
			for(y = (visablebrushtop - brushtop); y <= (visablebrushbottom-brushtop); y++)
			{
				for(x = (visablebrushleft - brushleft); x <= (visablebrushright-brushleft); x++)
				{
					if( GetPixelColour(bi->bi_Mask,x,y) )
					{
						ShadePoint(pj,brushleft+x,brushtop+y,col);
					}
				}
			}
			break;
	}
}


static void BltColourBrush(struct Project *pj, struct BrushInfo *bi,
														WORD brushxoffset, WORD brushyoffset,
														WORD x, WORD y, WORD width, WORD height, UWORD col)
{
	PLANEPTR oldplane1;
	UBYTE olddepth;
	UWORD plane;

	if( (!bi) || (!pj) )
		return;

	olddepth = workbm->Depth;
	workbm->Depth = 1;
	oldplane1 = workbm->Planes[0];

	BltBitMap(bi->bi_Mask, brushxoffset, brushyoffset,
						workmaskbm, 0, 0, width, height, 0xC0,0xFF,NULL);

	for(plane = 0; plane < workbmdepth; plane++)
	{
		workbm->Planes[0] = workbm->Planes[plane];

		if(col & (1<<plane))
			BltBitMap(workmaskbm,0,0,workbm,0,0,width,height,0xC0,0xFF,NULL);
		else
			BltBitMap(workmaskbm,0,0,workbm,0,0,width,height,0x00,0xFF,NULL);
		WaitBlit();
	}
	workbm->Planes[0] = oldplane1;
	workbm->Depth = olddepth;
	BltMaskBitMapRastPort(workbm, 0, 0,
												pj->pj_MainRPort, x, y,
												width, height, 0xE0 , workmaskbm->Planes[0]);
}


/**********************   OutlineBrush()   *******************************/
//
//  Outline the given brush. (Can really only be a custom brush).

void OutlineBrush(struct BrushInfo *bi, UWORD col,
									 struct BitMap *dstbm, struct BitMap *dstmaskbm )
{
	UWORD x,y;
	struct BitMap *srcbm,*srcmaskbm;
	UWORD width, height;
	UWORD pix,prevpix;

	if( (!bi) || (!dstbm) )
		return;

	srcbm = bi->bi_Image;
	srcmaskbm = bi->bi_Mask;
	width = bi->bi_Width;
	height = bi->bi_Height;

	WaitBlit();
	if( (!srcbm) || (!srcmaskbm) )
		return;

	BltBitMap(srcbm, 0, 0, dstbm, 1, 1, width, height, 0xC0, 0xFF, NULL);
	BltBitMap(srcmaskbm, 0, 0, dstmaskbm, 1, 1, width, height, 0xC0, 0xFF, NULL);
	WaitBlit();

	/* Do horizontal scan */
	for(y = 1; y <= height; y++)
	{
		prevpix = 0;
		for(x = 1; x <= width; x++)
		{
			if( pix = GetPixelColour(srcmaskbm,x-1,y-1) )
			{
				if(!prevpix)
				{
					PutPixelColour(dstbm    ,x-1,y,col);
					PutPixelColour(dstmaskbm,x-1,y,1);
				}
				if(x == width)
				{
					PutPixelColour(dstbm    ,x+1,y,col);
					PutPixelColour(dstmaskbm,x+1,y,1);
				}
			}
			else
			{
				if(prevpix)
				{
					PutPixelColour(dstbm    ,x,y,col);
					PutPixelColour(dstmaskbm,x,y,1);
				}
			}
			prevpix = pix;
		}
	}

	/* Do Vertical scan */
	for(x = 1; x <= width; x++)
	{
		prevpix = 0;
		for(y = 1; y <= height; y++)
		{
			if( pix = GetPixelColour(srcmaskbm,x-1,y-1) )
			{
				if(!prevpix)
				{
					PutPixelColour(dstbm    ,x,y-1,col);
					PutPixelColour(dstmaskbm,x,y-1,1);
				}
				if(y == height)
				{
					PutPixelColour(dstbm    ,x,y+1,col);
					PutPixelColour(dstmaskbm,x,y+1,1);
				}
			}
			else
			{
				if(prevpix)
				{
					PutPixelColour(dstbm    ,x,y,col);
					PutPixelColour(dstmaskbm,x,y,1);
				}
			}
			prevpix = pix;
		}
	}
}

void OutlineCurrentBrush(UWORD col)
{
	struct Brush *br;
	struct BrushInfo *bi;
	struct BitMap *dstbm,*dstmaskbm;
	UWORD newwidth,newheight,newdepth;

	br = currentbrush;
	bi = &br->br_DrawBrush;

	WaitForSubTaskDone(mainmsgport);

	if(br->br_ID == BR_CUSTOM)
	{
		newwidth = (bi->bi_Width)+2;
		newheight = (bi->bi_Height)+2;
		if( !(newdepth = bi->bi_Depth) )
			newdepth = mainscreen->RastPort.BitMap->Depth;
	
		if(dstbm = CreateBitMap(newwidth,newheight,newdepth,FALSE))
		{
			if(dstmaskbm = CreateBitMap(newwidth,newheight,1,FALSE))
			{
				OutlineBrush(bi, col, dstbm, dstmaskbm);
				DestroyBitMap(bi->bi_Image,bi->bi_Width,bi->bi_Height,bi->bi_Depth);
				DestroyBitMap(bi->bi_Mask,bi->bi_Width,bi->bi_Height,1);
				bi->bi_Image = dstbm;
				bi->bi_Mask = dstmaskbm;
				bi->bi_Width = newwidth;
				bi->bi_Height = newheight;
				bi->bi_Depth = newdepth;
				bi->bi_XHandle += 1;
				bi->bi_YHandle += 1;
				if(lastprojectwindow)
				{
					EraseSelector(lastprojectwindow);
					DrawSelector(lastprojectwindow,lastprojectwindow->pw_SelX0,lastprojectwindow->pw_SelY0);
				}
			}
			else
			{
				printf("Create Mask BitMap failed\n");
				DestroyBitMap(dstbm,newwidth,newheight,bi->bi_Depth);
			}
		}
		else
			printf("Create Main BitMap failed\n");
	}
	else
		GroovyReq("Error","Can Only Outline\nCustom Brush","Continue");
}

/************************   RestoreBrush()   *****************************/
//

BOOL RestoreBrush(struct Brush *br)
{
	struct BrushInfo *ib,*db;
	struct BitMap *newimage,*newmask;

	if(!br)
		return(FALSE);

	ib = &br->br_InitialBrush;
	db = &br->br_DrawBrush;

	if(br->br_ID == BR_CUSTOM)
	{
		if( (!ib->bi_Image) || (!ib->bi_Mask) )
			return(TRUE);
			
		if( (ib->bi_Width != db->bi_Width) || (ib->bi_Height != db->bi_Height) || (ib->bi_Depth != db->bi_Depth) )
		{
			if( !(newimage = CreateBitMap(ib->bi_Width, ib->bi_Height, ib->bi_Depth, FALSE)) )
			{
				printf("Create Image BitMap Failed\n");
				return(FALSE);
			}
			if( !(newmask = CreateBitMap(ib->bi_Width, ib->bi_Height, 1, FALSE)) )
			{
				printf("Create Mask BitMap Failed\n");
				DestroyBitMap(newimage,ib->bi_Width, ib->bi_Height, ib->bi_Depth);
				return(FALSE);
			}
	
			if(db->bi_Image) DestroyBitMap(db->bi_Image, db->bi_Width, db->bi_Height, db->bi_Depth);
			if(db->bi_Mask) DestroyBitMap(db->bi_Mask, db->bi_Width, db->bi_Height, 1);
			db->bi_Image = newimage;
			db->bi_Mask = newmask;
			db->bi_Width = ib->bi_Width;
			db->bi_Height = ib->bi_Height;
			db->bi_Depth = ib->bi_Depth;
			db->bi_XHandle = ib->bi_XHandle;
			db->bi_YHandle = ib->bi_YHandle;
		}
		/* By here we should have the two bitmaps exactly the same dimensions */
	
		BltBitMap(ib->bi_Image,0,0,db->bi_Image,0,0,ib->bi_Width,ib->bi_Height,0xC0,0xFF,NULL);
		BltBitMap(ib->bi_Mask,0,0,db->bi_Mask,0,0,ib->bi_Width,ib->bi_Height,0xC0,0xFF,NULL);
		if(lastprojectwindow)
		{
			EraseSelector(lastprojectwindow);
			DrawSelector(lastprojectwindow,lastprojectwindow->pw_SelX0,lastprojectwindow->pw_SelY0);
		}
	}
	else
		GroovyReq("Error","Can Only Restore\nCustom Brush","Continue");

	return(TRUE);
}

/******************************   FLipBrushVertically()   ***********************/
//

BOOL FlipBrushVertically(struct BrushInfo *bi)
{
	if( (!bi) || (!bi->bi_Image) )
		return(TRUE);

	FlipBitMapVertically(bi->bi_Image,bi->bi_Width, bi->bi_Height,bi->bi_Depth);
	FlipBitMapVertically(bi->bi_Mask,bi->bi_Width, bi->bi_Height,1);

	if(lastprojectwindow)
	{
		EraseSelector(lastprojectwindow);
		DrawSelector(lastprojectwindow,lastprojectwindow->pw_SelX0,lastprojectwindow->pw_SelY0);
	}	
}

/****************************   FlipBitMapVertically()   **************************/
//

void FlipBitMapVertically(struct BitMap *srcbm, UWORD width, UWORD height, UWORD depth)
{
	UWORD	wordwidth,plane,line,word,*srcptr,*dstptr,wordsperrow,tmpword,halfheight;

	wordwidth = (width+15)>>4;
	wordsperrow = srcbm->BytesPerRow/2;
	halfheight = height/2;

	if(!srcbm)
		return;

	WaitBlit();

	for(plane = 0; plane < depth; plane++)
	{
		srcptr = (UWORD *)srcbm->Planes[plane];
		dstptr = srcptr + wordsperrow*(height-1);
		for(line = 0; line < halfheight; line++)
		{
			for(word = 0; word < wordwidth; word++)
			{
				tmpword = dstptr[word];
				dstptr[word] = srcptr[word];
				srcptr[word] = tmpword;
			}
			srcptr += wordsperrow;
			dstptr -= wordsperrow;
		}
	}
}

/********************************   FlipBrushHorizontally()   *******************/
//

BOOL FlipBrushHorizontally(struct BrushInfo *bi)
{
	struct BitMap *tmpimage,*tmpmask;

	if( (!bi) || (!bi->bi_Image) )
		return(TRUE);

	if( !(tmpimage = CreateBitMap(bi->bi_Width, bi->bi_Height, bi->bi_Depth, FALSE)) )
		return(FALSE);
	if( !(tmpmask = CreateBitMap(bi->bi_Width, bi->bi_Height, 1, FALSE)) )
	{
		DestroyBitMap(tmpimage,bi->bi_Width, bi->bi_Height,bi->bi_Depth);
		return(FALSE);
	}
	FlipBitMapHorizontally(bi->bi_Image,tmpimage,bi->bi_Width, bi->bi_Height,bi->bi_Depth);
	FlipBitMapHorizontally(bi->bi_Mask,tmpmask,bi->bi_Width, bi->bi_Height,1);

	DestroyBitMap(tmpimage,bi->bi_Width, bi->bi_Height,bi->bi_Depth);
	DestroyBitMap(tmpmask,bi->bi_Width, bi->bi_Height,1);
	if(lastprojectwindow)
	{
		EraseSelector(lastprojectwindow);
		DrawSelector(lastprojectwindow,lastprojectwindow->pw_SelX0,lastprojectwindow->pw_SelY0);
	}
}

/*******************************   FlipBitMapHorizontally()   *********************/
//

void FlipBitMapHorizontally(struct BitMap *srcbm, struct BitMap *tmpbm,
														UWORD width, UWORD height, UWORD depth)
{
	static UBYTE flippednumbers[] = {
				0x00,0x80,0x40,0xC0,0x20,0xA0,0x60,0xE0,0x10,0x90,0x50,0xD0,0x30,0xB0,0x70,0xF0,
				0x08,0x88,0x48,0xC8,0x28,0xA8,0x68,0xE8,0x18,0x98,0x58,0xD8,0x38,0xB8,0x78,0xF8,
				0x04,0x84,0x44,0xC4,0x24,0xA4,0x64,0xE4,0x14,0x94,0x54,0xD4,0x34,0xB4,0x74,0xF4,
				0x0C,0x8C,0x4C,0xCC,0x2C,0xAC,0x6C,0xEC,0x1C,0x9C,0x5C,0xDC,0x3C,0xBC,0x7C,0xFC,
				0x02,0x82,0x42,0xC2,0x22,0xA2,0x62,0xE2,0x12,0x92,0x52,0xD2,0x32,0xB2,0x72,0xF2,
				0x0A,0x8A,0x4A,0xCA,0x2A,0xAA,0x6A,0xEA,0x1A,0x9A,0x5A,0xDA,0x3A,0xBA,0x7A,0xFA,
				0x06,0x86,0x46,0xC6,0x26,0xA6,0x66,0xE6,0x16,0x96,0x56,0xD6,0x36,0xB6,0x76,0xF6,
				0x0E,0x8E,0x4E,0xCE,0x2E,0xAE,0x6E,0xEE,0x1E,0x9E,0x5E,0xDE,0x3E,0xBE,0x7E,0xFE,
				0x01,0x81,0x41,0xC1,0x21,0xA1,0x61,0xE1,0x11,0x91,0x51,0xD1,0x31,0xB1,0x71,0xF1,
				0x09,0x89,0x49,0xC9,0x29,0xA9,0x69,0xE9,0x19,0x99,0x59,0xD9,0x39,0xB9,0x79,0xF9,
				0x05,0x85,0x45,0xC5,0x25,0xA5,0x65,0xE5,0x15,0x95,0x55,0xD5,0x35,0xB5,0x75,0xF5,
				0x0D,0x8D,0x4D,0xCD,0x2D,0xAD,0x6D,0xED,0x1D,0x9D,0x5D,0xDD,0x3D,0xBD,0x7D,0xFD,
				0x03,0x83,0x43,0xC3,0x23,0xA3,0x63,0xE3,0x13,0x93,0x53,0xD3,0x33,0xB3,0x73,0xF3,
				0x0B,0x8B,0x4B,0xCB,0x2B,0xAB,0x6B,0xEB,0x1B,0x9B,0x5B,0xDB,0x3B,0xBB,0x7B,0xFB,
				0x07,0x87,0x47,0xC7,0x27,0xA7,0x67,0xE7,0x17,0x97,0x57,0xD7,0x37,0xB7,0x77,0xF7,
				0x0F,0x8F,0x4F,0xCF,0x2F,0xAF,0x6F,0xEF,0x1F,0x9F,0x5F,0xDF,0x3F,0xBF,0x7F,0xFF };
	UWORD	bytewidth,byteright,plane,line,byte;
	UBYTE	*srcptr,*dstptr;

	if(!srcbm)
		return;

	bytewidth = ((width+15)>>3)&0xFFFE;
	byteright = bytewidth-1;

	WaitBlit();

	for(plane = 0; plane < depth; plane++)
	{
		srcptr = srcbm->Planes[plane];
		dstptr = tmpbm->Planes[plane];
		for(line = 0; line < height; line++)
		{
			for(byte = 0; byte < bytewidth; byte++)
				dstptr[byteright-byte] = flippednumbers[ srcptr[byte] ];
			srcptr += srcbm->BytesPerRow;
			dstptr += tmpbm->BytesPerRow;
		}
	}
	byte = (bytewidth*8)-width;
	BltBitMap(tmpbm,byte,0,srcbm,0,0,width,height,0xC0,0xFF,NULL);
}

/*****************************   RotateBrush()   ******************************/
//

BOOL RotateBrush90(struct BrushInfo *bi)
{
	struct BitMap *dstimage,*dstmask;
	UWORD dstwidth,dstheight;

	if( (!bi) || (!bi->bi_Image) )
		return(TRUE);

	dstwidth = bi->bi_Height;
	dstheight = bi->bi_Width;

	if( !(dstimage = CreateBitMap(dstwidth, dstheight, bi->bi_Depth, FALSE)) )
		return(FALSE);
	if( !(dstmask = CreateBitMap(dstwidth, dstheight, 1, FALSE)) )
	{
		DestroyBitMap(dstimage,dstwidth, dstheight,bi->bi_Depth);
		return(FALSE);
	}
	RotateBitMap90(bi->bi_Image,dstimage,bi->bi_Width, bi->bi_Height,bi->bi_Depth);
	RotateBitMap90(bi->bi_Mask,dstmask,bi->bi_Width, bi->bi_Height,1);

	DestroyBitMap(bi->bi_Image,bi->bi_Width, bi->bi_Height,bi->bi_Depth);
	DestroyBitMap(bi->bi_Mask,bi->bi_Width, bi->bi_Height,1);
	bi->bi_Image = dstimage;
	bi->bi_Mask = dstmask;
	bi->bi_Width = dstwidth;
	bi->bi_Height = dstheight;
	if(lastprojectwindow)
	{
		EraseSelector(lastprojectwindow);
		DrawSelector(lastprojectwindow,lastprojectwindow->pw_SelX0,lastprojectwindow->pw_SelY0);
	}
}

void RotateBitMap90(struct BitMap *srcbm, struct BitMap *dstbm,
									UWORD srcwidth, UWORD srcheight, UWORD depth)
{
	UWORD	x,y;

	if(!srcbm)
		return;

	WaitBlit();

	for(y = 0; y < srcheight; y++)
	{
		for(x = 0; x < srcwidth; x++)
			PutPixelColour(dstbm,srcheight-y-1,x,GetPixelColour(srcbm,x,y));
	}
}

/**************************   NastyMakeColourMask()   *********************/
//
//  Make a mask in the destbitmap (must be one bitplane), of the source bitmap
// less colour 'col'. ie, if colour 'col' appears in the srcbitmap then the
// mask will be zero there, and one elsewhere.
// If colour plane we use, d = (a | ~b), else d = (a | b)

void NastyMakeColourMask(struct BitMap *srcbitmap,struct BitMap *destbitmap,
												 UWORD width, UWORD height, UWORD col)
{
	UWORD wordwidth,depth,plane;
	UWORD a,b,d,x,y;
	extern struct Custom __far custom;

	if(!srcbitmap || !destbitmap)
		return;

	wordwidth = (width+15)/16;
	depth = srcbitmap->Depth;

	for(plane = 0; plane < depth; plane++)
	{
		for(y = 0; y < height; y++)
		{
			for( x = 0; x<wordwidth; x++)
			{
				a = *(UWORD *)(destbitmap->Planes[0] + (destbitmap->BytesPerRow * y)  + (x*2));
				b = *(UWORD *)(srcbitmap->Planes[plane] + (srcbitmap->BytesPerRow * y)  + (x*2));
				d = (col & (1<<plane)) ? a|(~b) : a|b;
				*(UWORD *)(destbitmap->Planes[0] + (destbitmap->BytesPerRow * y) + (x*2)) = d;
			}
		}

	}
}

void RotateBrushHandle(struct Brush *br)
{
	struct BrushInfo *bi;

	bi = &br->br_DrawBrush;

	/* If in center goto bottom left */
	if( (bi->bi_XHandle == (bi->bi_Width/2)) && (bi->bi_YHandle == (bi->bi_Height/2)) )
	{
		bi->bi_XHandle = 0;
		bi->bi_YHandle = bi->bi_Height-1;
		return;
	}
	if( (bi->bi_XHandle == 0) && (bi->bi_YHandle == bi->bi_Height-1) )
	{
		bi->bi_XHandle = 0;
		bi->bi_YHandle = 0;
		return;
	}
	if( (bi->bi_XHandle == 0) && (bi->bi_YHandle == 0) )
	{
		bi->bi_XHandle = bi->bi_Width-1;
		bi->bi_YHandle = 0;
		return;
	}
	if( (bi->bi_XHandle == bi->bi_Width-1) && (bi->bi_YHandle == 0) )
	{
		bi->bi_XHandle = bi->bi_Width-1;
		bi->bi_YHandle = bi->bi_Height-1;
		return;
	}

	/* If we get here then the brush is either at the bottom right or not set
		 so just make it goto the centre */
	bi->bi_XHandle = bi->bi_Width/2;
	bi->bi_YHandle = bi->bi_Height/2;
}

void HalveBrush(struct BrushInfo *bi, struct Palette *pal)
{
	struct BitMap *dstimage,*dstmask;
	UWORD dstwidth,dstheight;
	UWORD srcright, srcbottom;
	UWORD x,y,col1,col2,col3,col4,newcol,r,g,b;

	if(!bi)
		return;

	if(!bi->bi_Image)
		return;

	srcright = bi->bi_Width-1;
	srcbottom = bi->bi_Height-1;
	dstwidth = bi->bi_Width/2;
	dstheight = bi->bi_Height/2;

	if( !(dstimage = CreateBitMap(dstwidth, dstheight, bi->bi_Depth, FALSE)) )
		return;
	if( !(dstmask = CreateBitMap(dstwidth, dstheight, 1, FALSE)) )
	{
		DestroyBitMap(dstimage,dstwidth, dstheight,bi->bi_Depth);
		return;
	}
	if(BlockAllSuperWindows())
	{
		for(y = 0; y < dstheight; y++)
		{
			for(x = 0; x < dstwidth; x++)
			{
				col1 = GetPixelColour(bi->bi_Image, x*2, y*2);
				col2 = GetPixelColour(bi->bi_Image, min(srcright,(x*2)+1), y*2);
				col3 = GetPixelColour(bi->bi_Image, x*2, min(srcbottom,(y*2)+1));
				col4 = GetPixelColour(bi->bi_Image, min(srcright,(x*2)+1), min(srcbottom,(y*2)+1));
				r = (pal->pl_Colours[col1].r + pal->pl_Colours[col2].r +
						 pal->pl_Colours[col3].r + pal->pl_Colours[col4].r)/4;
				g = (pal->pl_Colours[col1].g + pal->pl_Colours[col2].g +
						 pal->pl_Colours[col3].g + pal->pl_Colours[col4].g)/4;
				b = (pal->pl_Colours[col1].b + pal->pl_Colours[col2].b +
						 pal->pl_Colours[col3].b + pal->pl_Colours[col4].b)/4;
				newcol = FindnBestPen(pal, pal->pl_Count, r,g,b);
				PutPixelColour(dstimage,x,y,newcol);
			}
		}
		DestroyBitMap(bi->bi_Image,bi->bi_Width, bi->bi_Height,bi->bi_Depth);
		DestroyBitMap(bi->bi_Mask,bi->bi_Width, bi->bi_Height,1);
		bi->bi_Image = dstimage;
		bi->bi_Mask = dstmask;
		bi->bi_Width = dstwidth;
		bi->bi_Height = dstheight;
		bi->bi_XHandle /= 2;
		bi->bi_YHandle /= 2;
		NastyMakeColourMask(bi->bi_Image, bi->bi_Mask, bi->bi_Width, bi->bi_Height, rmbcol);
		UnblockAllSuperWindows();
	}
	if(lastprojectwindow)
	{
		EraseSelector(lastprojectwindow);
		DrawSelector(lastprojectwindow,lastprojectwindow->pw_SelX0,lastprojectwindow->pw_SelY0);
	}
}

/***************************   RotateBrush()   ****************************/
//
//  Rotate the Brush by the angle theta (radians).
//  Rotates about the bottom left of the brush.
//  Positive theta rotates unti-clockwise, ie, from x to y if viewed from a
// cartesian graph.

#define RotateX(x,y,theta) (((x)*cos(theta)) - ((y)*sin(theta)))
#define RotateY(x,y,theta) (((x)*sin(theta)) + ((y)*cos(theta)))

BOOL RotateBrush(struct BrushInfo *bi, double theta)
{
	BOOL ret = FALSE;
	struct BitMap *dstimage = NULL,*dstmask = NULL;
	double x1,y1,x2,y2,x3,y3;
	double minx,miny,maxx,maxy;
	double width,height;
	UWORD col,tmp;

	if(bi && bi->bi_Image)
	{
		/* Work out the size of the new bitmap, this is done
			 by rotating the corners of the old one and working out the
			 new min & max values */
		x1 = RotateX(0.0,(double)bi->bi_Height,theta);
		y1 = RotateY(0.0,(double)bi->bi_Height,theta);
		x2 = RotateX((double)bi->bi_Width,(double)bi->bi_Height,theta);
		y2 = RotateY((double)bi->bi_Width,(double)bi->bi_Height,theta);
		x3 = RotateX((double)bi->bi_Width,0.0,theta);
		y3 = RotateY((double)bi->bi_Width,0.0,theta);
		minx = 0;
		if(x1 < minx) minx = x1;
		if(x2 < minx) minx = x2;
		if(x3 < minx) minx = x3;
		miny = 0;
		if(y1 < miny) miny = y1;
		if(y2 < miny) miny = y2;
		if(y3 < miny) miny = y3;
		maxx = 0;
		if(x1 > maxx) maxx = x1;
		if(x2 > maxx) maxx = x2;
		if(x3 > maxx) maxx = x3;
		maxy = 0;
		if(y1 > maxy) maxy = y1;
		if(y2 > maxy) maxy = y2;
		if(y3 > maxy) maxy = y3;
		width = maxx-minx+1;
		tmp = (UWORD)width+1.0;
		width = (double)tmp;
		height = maxy-miny+1;
		tmp = (UWORD)height+1.0;
		height = (double)tmp;

		if(dstimage = CreateBitMap((UWORD)width, (UWORD)height, bi->bi_Depth, FALSE))
		{
			if(dstmask = CreateBitMap((UWORD)width, (UWORD)height, 1, FALSE))
			{
				/* make theta negative 'cos we'll now work backwards */
				theta = -theta;

				for(y1=0.0; y1<height; y1+=1.0)
				{
					for(x1=0.0; x1<width; x1+=1.0)
					{
						x3 = x1+0.5+minx;
						y3 = y1+0.5+miny;
						x2 = RotateX(x3, y3, theta);
						y2 = RotateY(x3, y3, theta);

						if(x2<0.0 || y2<0.0 || x2>=(double)bi->bi_Width || y2>=(double)bi->bi_Height)
							PutPixelColour(dstimage, (UWORD)x1, (UWORD)(height-1.0-y1), 0);
						else
						{
							col = GetPixelColour(bi->bi_Image, (UWORD)x2, (UWORD)((bi->bi_Height)-y2));
							PutPixelColour(dstimage, (UWORD)x1, (UWORD)((height-1.0)-y1), col);
						}
					}
				}
				ret = TRUE;
				if(ret)
				{
					DestroyBitMap(bi->bi_Image,bi->bi_Width, bi->bi_Height,bi->bi_Depth);
					DestroyBitMap(bi->bi_Mask,bi->bi_Width, bi->bi_Height,1);
					bi->bi_Image = dstimage;
					bi->bi_Mask = dstmask;
					bi->bi_Width = (UWORD)width;
					bi->bi_Height = (UWORD)height;
					NastyMakeColourMask(bi->bi_Image, bi->bi_Mask, bi->bi_Width, bi->bi_Height, rmbcol);
				}
			}
		}
	}
	if(!ret)
	{
		if(dstimage) DestroyBitMap(dstimage,(UWORD)width, (UWORD)height, bi->bi_Depth);
		if(dstmask) DestroyBitMap(dstmask, (UWORD)width, (UWORD)height, 1);
		GroovyReq("Rotate Brush","Not Enough Memory","Continue");
	}
	return(ret);
}
