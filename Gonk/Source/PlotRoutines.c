
#include <math.h>
#include <stdio.h>
//#include <stdarg.h>
//#include <string.h>
//#include <dos/dos.h>
//#include <dos/dostags.h>
//#include <exec/exec.h>
//#include <intuition/intuition.h>
//#include <graphics/gfxbase.h>
//#include <rexx/rxslib.h>
//#include <libraries/gadtools.h>
//#include <libraries/diskfont.h>
//#include <libraries/asl.h>

#include <proto/exec.h>
//#include <proto/dos.h>
//#include <proto/intuition.h>
#include <proto/graphics.h>
//#include <proto/diskfont.h>
//#include <proto/asl.h>
//#include <clib/alib_protos.h>
//#include <proto/gadtools.h>

#include <hardware/custom.h>

#include <global.h>
#include <ExternVars.h>

extern __far struct Custom custom;

/***************  PlotPixel   *******************/
//
// This is the bare bones plot pixel routine. It does nothing like
// SetDrMd() or SetAPen. It just plots a pixel with the right
// magnification at the right place.
//
void PlotPixel(struct ProjectWindow *pw,WORD x,WORD y)
{
	if(pw->pw_PixelSize == 1)
		WritePixel(pw->pw_sw.sw_Window->RPort, x, y);
	else
		RectFill(pw->pw_sw.sw_Window->RPort, x, y,
						 x+(pw->pw_PixelSize)-1, y+(pw->pw_PixelSize)-1);
}


/********************  SmartPlotPixel *****************/
//
// This is a nice safe pixel plotting routine. It plots a
// pixel in the current colour to the given project window.
// It can handle:
// 1) Not having a window open.
// 2) The pixel you want to plot being outside the window dimensions.
// Inputs: pw    - Pointer to a project window.
//         x & y - x and y offsets to the projects MainBitMap.

BOOL SmartPlotPixel(struct ProjectWindow *pw,WORD x,WORD y)
{
	struct Window *win;
	WORD winxoffset,winyoffset;
	UWORD winx,winy;

	if(win = pw->pw_sw.sw_Window) 
	{
		if( (x >= 0) && (x < pw->pw_Project->pj_Width) && (y >= 0) && (y < pw->pw_Project->pj_Height) )
		{
			winxoffset = x - pw->pw_XOffset;
			winyoffset = y - pw->pw_YOffset;

			if( (winxoffset >= 0) && (winxoffset < pw->pw_MagWidth) &&
					(winyoffset >= 0) && (winyoffset < pw->pw_MagHeight) )
			{
				winx = (winxoffset * pw->pw_PixelSize) + win->BorderLeft;
				winy = (winyoffset * pw->pw_PixelSize) + win->BorderTop;

				PlotPixel(pw,winx,winy);
				return(TRUE);
			}
		}
		else
			return(TRUE);
	}
	return(FALSE);
}

/*****************   SmartPlotBrush()   *********************/
//
// Will plot the current brush at (x - br_XHandle, y - br_YHandle).

BOOL SmartPlotBrush(struct ProjectWindow *pw, WORD x, WORD y, struct Rectangle *rec,
										UWORD col, UWORD mode)
{
	struct Window *win;
	WORD	brushx, brushy;

	if(win = pw->pw_sw.sw_Window)
	{
		brushx = x - currentbrush->br_DrawBrush.bi_XHandle;
		brushy = y - currentbrush->br_DrawBrush.bi_YHandle;

		switch(currentbrush->br_ID)
		{
			case BR_DOT:
				if(currentbrush->br_DrawBrush.bi_Image)
					return( WipeWithCustomBrush(&currentbrush->br_DrawBrush, pw, brushx, brushy, rec, FALSE, col) );
				else
				{
					SetAPen(win->RPort,col);
					SetDrMd(win->RPort, JAM1);
					if( SmartPlotPixel(pw, brushx, brushy) )
					{
						if(rec)
						{
							rec->MinX = brushx;
							rec->MinY = brushy;
							rec->MaxX = brushx;
							rec->MaxY = brushy;
						}
						return(TRUE);
					}
					else
						return(FALSE);
				}
				break;
			case BR_BRUSH1:
			case BR_BRUSH2:
			case BR_BRUSH3:
			case BR_BRUSH4:
			case BR_BRUSH5:
			case BR_BRUSH6:
			case BR_BRUSH7:
			case BR_BRUSH8:
				return( WipeWithCustomBrush(&currentbrush->br_DrawBrush, pw, brushx, brushy, rec, FALSE, col) );
				break;
			case BR_ANIMBRUSH:
			case BR_CUSTOM:
				switch(mode)
				{
					case DRMD_MATTE:
						return( DrawCustomBrush(&currentbrush->br_DrawBrush, pw, brushx, brushy, rec, FALSE) );
						break;
					case DRMD_CYCLE:
					case DRMD_COLOUR:
						return( WipeWithCustomBrush(&currentbrush->br_DrawBrush, pw, brushx, brushy, rec, FALSE, col) );
						break;
					case DRMD_REPLACE:
						return( ReplaceWithCustomBrush(&currentbrush->br_DrawBrush, pw, brushx, brushy, rec, FALSE, col) );
						break;
					case DRMD_SOLID:
						return( SolidWithCustomBrush(&currentbrush->br_DrawBrush, pw, brushx, brushy, rec, col) );
						break;
				}
				break;
		}
	}
}

/*****************   QuickSmartPlotBrush()   *********************/
//
// Will plot the current brush at (x - br_XHandle, y - br_YHandle).

BOOL QuickSmartPlotBrush(struct ProjectWindow *pw, WORD x, WORD y,
												 struct Rectangle *rec, BOOL premagnified, UWORD col, UWORD mode)
{
	struct Window *win;
	WORD	brushx, brushy;

	if(win = pw->pw_sw.sw_Window)
	{
		brushx = x - currentbrush->br_DrawBrush.bi_XHandle;
		brushy = y - currentbrush->br_DrawBrush.bi_YHandle;

		switch(currentbrush->br_ID)
		{
			case BR_DOT:
				if(currentbrush->br_DrawBrush.bi_Image)
					return( WipeWithCustomBrush(&currentbrush->br_DrawBrush, pw, brushx, brushy, rec, FALSE, col) );
				else
				{
					if( SmartPlotPixel(pw, brushx, brushy) )
					{
						if(rec)
						{
							rec->MinX = brushx;
							rec->MinY = brushy;
							rec->MaxX = brushx;
							rec->MaxY = brushy;
						}
						return(TRUE);
					}
					else
						return(FALSE);
				}
				break;
			case BR_BRUSH1:
			case BR_BRUSH2:
			case BR_BRUSH3:
			case BR_BRUSH4:
			case BR_BRUSH5:
			case BR_BRUSH6:
			case BR_BRUSH7:
			case BR_BRUSH8:
				return( WipeWithCustomBrush(&currentbrush->br_DrawBrush, pw, brushx, brushy, rec, FALSE, col) );
				break;
			case BR_ANIMBRUSH:
			case BR_CUSTOM:
				switch(mode)
				{
					case DRMD_CYCLE:
					case DRMD_COLOUR:
						return( WipeWithCustomBrush(&currentbrush->br_DrawBrush, pw, brushx, brushy, rec, premagnified, col) );
						break;
					default:
						return( DrawCustomBrush(&currentbrush->br_DrawBrush, pw, brushx, brushy, rec, premagnified) );
						break;
				}
				break;
		}
	}
}

/********************** Extra Smart Plot Pixel *************/
//
// This is the Enstein of Safe plotting routines. It goes through
// all the project windows of a certain project, and plots a pixel
// to them all.

void __asm ExtraSmartPlotPixel(register __a0 struct Project *pj,
															 register __d0 WORD x,
															 register __d1 WORD y,
															 register __a1 UWORD col)
{
	struct ProjectWindow *pw;

	if( (x < pj->pj_Width) && (y < pj->pj_Height) )
	{
		PutPixelColour(pj->pj_MainBitMap, x, y, col);

		for(pw=pj->pj_ProjectWindows; pw; pw=pw->pw_NextProjWindow )
		{
			if(pw->pw_sw.sw_Window)
			{
				SetAPen(pw->pw_sw.sw_Window->RPort,col);
				SetDrMd(pw->pw_sw.sw_Window->RPort,JAM1);
				SmartPlotPixel(pw,x,y);
			}
		}
	}
}

/********************** Extra Smart Plot Brush *************/
void ExtraSmartPlotBrush(struct Project *pj, WORD x, WORD y, UWORD col, UWORD mode)
{
	struct ProjectWindow *pw;
	WORD brushx, brushy;
	struct BrushInfo *bi;

	bi = &currentbrush->br_DrawBrush;

	brushx = x - bi->bi_XHandle;
	brushy = y - bi->bi_YHandle;

	if( (brushx < pj->pj_Width) && (brushy < pj->pj_Height) )
	{
		switch(mode)
		{
			case DRMD_SMOOTH:
				switch(currentbrush->br_ID)
				{
					case BR_DOT:
						if(bi->bi_Image)
							BltBrushToMainBitMap(pj, bi, brushx, brushy, col, mode);
						else
							SmoothPoint(pj,brushx,brushy);
						break;
					case BR_BRUSH1:
					case BR_BRUSH2:
					case BR_BRUSH3:
					case BR_BRUSH4:
					case BR_BRUSH5:
					case BR_BRUSH6:
					case BR_BRUSH7:
					case BR_BRUSH8:
						BltBrushToMainBitMap(pj, bi, brushx, brushy, col, mode);
						break;
					case BR_ANIMBRUSH:
					case BR_CUSTOM:
						BltBrushToMainBitMap(pj, bi, brushx, brushy, col, mode);
						break;
				}
				for(pw=pj->pj_ProjectWindows; pw; pw=pw->pw_NextProjWindow )
				{
					if(pw->pw_sw.sw_Window)
						RefreshWindowSection(pw,brushx,brushy,bi->bi_Width, bi->bi_Height);
				}
				break;
			case DRMD_SHADE:
				switch(currentbrush->br_ID)
				{
					case BR_DOT:
						if(bi->bi_Image)
							BltBrushToMainBitMap(pj, bi, brushx, brushy, col, mode);
						else
							if(col == lmbcol)
								ShadePoint(pj,brushx,brushy,FALSE);
							else
								ShadePoint(pj,brushx,brushy,TRUE);
						break;
					case BR_BRUSH1:
					case BR_BRUSH2:
					case BR_BRUSH3:
					case BR_BRUSH4:
					case BR_BRUSH5:
					case BR_BRUSH6:
					case BR_BRUSH7:
					case BR_BRUSH8:
						BltBrushToMainBitMap(pj, bi, brushx, brushy, col, mode);
						break;
					case BR_ANIMBRUSH:
					case BR_CUSTOM:
						BltBrushToMainBitMap(pj, bi, brushx, brushy, col, mode);
						break;
				}
				for(pw=pj->pj_ProjectWindows; pw; pw=pw->pw_NextProjWindow )
				{
					if(pw->pw_sw.sw_Window)
						RefreshWindowSection(pw,brushx,brushy,bi->bi_Width, bi->bi_Height);
				}
				break;
			default:
				switch(currentbrush->br_ID)
				{
					case BR_DOT:
						if(currentbrush->br_DrawBrush.bi_Image)
							BltBrushToMainBitMap(pj, &currentbrush->br_DrawBrush, brushx, brushy, col, DRMD_COLOUR);
						else
							PutPixelColour(pj->pj_MainBitMap,brushx,brushy,col);
						break;
					case BR_BRUSH1:
					case BR_BRUSH2:
					case BR_BRUSH3:
					case BR_BRUSH4:
					case BR_BRUSH5:
					case BR_BRUSH6:
					case BR_BRUSH7:
					case BR_BRUSH8:
						BltBrushToMainBitMap(pj, &currentbrush->br_DrawBrush, brushx, brushy, col, DRMD_COLOUR);
						break;
					case BR_ANIMBRUSH:
					case BR_CUSTOM:
						BltBrushToMainBitMap(pj, &currentbrush->br_DrawBrush, brushx, brushy, col, mode);
						break;
				}
				for(pw=pj->pj_ProjectWindows; pw; pw=pw->pw_NextProjWindow )
				{
					if(pw->pw_sw.sw_Window)
					{
						SetAPen(pw->pw_sw.sw_Window->RPort,col);
						SetDrMd(pw->pw_sw.sw_Window->RPort,JAM1);
						SmartPlotBrush(pw,x,y,NULL,col,mode);
					}
				}
				break;
		}
	}
}

/********************** ExtraSmartPlotBrushToMainBitMap *************/
void ExtraSmartPlotBrushToMainBitMap(struct Project *pj, WORD x, WORD y, UWORD col, UWORD mode)
{
	WORD brushx, brushy;
	struct BrushInfo *bi;

	bi = &currentbrush->br_DrawBrush;

	brushx = x - bi->bi_XHandle;
	brushy = y - bi->bi_YHandle;

	if( (brushx < pj->pj_Width) && (brushy < pj->pj_Height) )
	{
		switch(mode)
		{
			case DRMD_SMOOTH:
				switch(currentbrush->br_ID)
				{
					case BR_DOT:
						if(bi->bi_Image)
							BltBrushToMainBitMap(pj, bi, brushx, brushy, col, mode);
						else
							SmoothPoint(pj,brushx,brushy);
						break;
					case BR_BRUSH1:
					case BR_BRUSH2:
					case BR_BRUSH3:
					case BR_BRUSH4:
					case BR_BRUSH5:
					case BR_BRUSH6:
					case BR_BRUSH7:
					case BR_BRUSH8:
						BltBrushToMainBitMap(pj, bi, brushx, brushy, col, mode);
						break;
					case BR_ANIMBRUSH:
					case BR_CUSTOM:
						BltBrushToMainBitMap(pj, bi, brushx, brushy, col, mode);
						break;
				}
				break;
			case DRMD_SHADE:
				switch(currentbrush->br_ID)
				{
					case BR_DOT:
						if(bi->bi_Image)
							BltBrushToMainBitMap(pj, bi, brushx, brushy, col, mode);
						else
							if(col == lmbcol)
								ShadePoint(pj,brushx,brushy,FALSE);
							else
								ShadePoint(pj,brushx,brushy,TRUE);
						break;
					case BR_BRUSH1:
					case BR_BRUSH2:
					case BR_BRUSH3:
					case BR_BRUSH4:
					case BR_BRUSH5:
					case BR_BRUSH6:
					case BR_BRUSH7:
					case BR_BRUSH8:
						BltBrushToMainBitMap(pj, bi, brushx, brushy, col, mode);
						break;
					case BR_ANIMBRUSH:
					case BR_CUSTOM:
						BltBrushToMainBitMap(pj, bi, brushx, brushy, col, mode);
						break;
				}
				break;
			default:
				switch(currentbrush->br_ID)
				{
					case BR_DOT:
						if(currentbrush->br_DrawBrush.bi_Image)
							BltBrushToMainBitMap(pj, &currentbrush->br_DrawBrush, brushx, brushy, col, DRMD_COLOUR);
						else
							PutPixelColour(pj->pj_MainBitMap,brushx,brushy,col);
						break;
					case BR_BRUSH1:
					case BR_BRUSH2:
					case BR_BRUSH3:
					case BR_BRUSH4:
					case BR_BRUSH5:
					case BR_BRUSH6:
					case BR_BRUSH7:
					case BR_BRUSH8:
						BltBrushToMainBitMap(pj, &currentbrush->br_DrawBrush, brushx, brushy, col, DRMD_COLOUR);
						break;
					case BR_ANIMBRUSH:
					case BR_CUSTOM:
						BltBrushToMainBitMap(pj, &currentbrush->br_DrawBrush, brushx, brushy, col, mode);
						break;
				}
				break;
		}
	}
}

/******************** Quick Plot Pixel ****************/
//
// This is a mildly quick plotting routine. But lacks in
// safety features. Don't think it's actually being used
// anywhere.

void QuickPlotPixel(struct ProjectWindow *pw,struct Window *win,WORD x, WORD y)
{
	WORD winxoffset,winyoffset;
	UWORD winx,winy;
	
	winxoffset = x - pw->pw_XOffset;
	winyoffset = y - pw->pw_YOffset;

	if(pw->pw_PixelSize == 1)
	{
		winx = winxoffset + win->BorderLeft;
		winy = winyoffset + win->BorderTop;
		WritePixel(win->RPort, winx, winy);
	}
	else
	{
		winx = (winxoffset * pw->pw_PixelSize) + win->BorderLeft;
		winy = (winyoffset * pw->pw_PixelSize) + win->BorderTop;
		RectFill(win->RPort, winx, winy,
						 winx+(pw->pw_PixelSize)-1, winy+(pw->pw_PixelSize)-1);
	}

}


/**********************  Do Extra Smart Col Line ***********************/
//
// This will go through and plot a line to all project windows,
// in the given colour.
//  Written by Petri Kuittinen, last modifications 25th June 1993

void DoExtraSmartColLine (struct ProjectWindow *pw, struct Rectangle *rec,
													int x1, int y1, int x2, int y2, UWORD col, UWORD mode )
{
  /* d = delta, x and y are current coordinates, ax and ay are the
     absolute offset between x and y coordinates, sx and sy are signs
     of the offsets */
	int d, x, y, ax, ay, sx, sy;
	struct ProjectWindow *pw2;
	WORD minx,miny,maxx,maxy;

	minx = x1 - currentbrush->br_DrawBrush.bi_XHandle;
	miny = y1 - currentbrush->br_DrawBrush.bi_YHandle;
	maxx = x2 - currentbrush->br_DrawBrush.bi_XHandle;
	maxy = y2 - currentbrush->br_DrawBrush.bi_YHandle;
	if(minx > maxx)
	{
		x = maxx;
		maxx = minx;
		minx = x;
	}
	if(miny > maxy)
	{
		y = maxy;
		maxy = miny;
		miny = y;
	}
	maxx += currentbrush->br_DrawBrush.bi_Width-1;
	maxy += currentbrush->br_DrawBrush.bi_Height-1;

	if(rec)
	{
		rec->MinX = minx;
		rec->MinY = miny;
		rec->MaxX = maxx;
		rec->MaxY = maxy;
	}

  sx = 1;
	ax = (x2-x1)<<1;
  if (ax<0)
	{
		ax = -ax;
		sx = -sx;
	}
  sy = 1;
	ay = (y2-y1)<<1;
  if (ay<0)
	{
		ay = -ay;
		sy = -sy;
	}
  x = x1;
	y = y1;

	if (ax>ay)
	{           
		/* x offset higher */
		d = ay-(ax>>1);
		for (;;)
		{
			ExtraSmartPlotBrushToMainBitMap(pw->pw_Project, x, y, col, mode);

			if (x==x2)
				goto TheEnd;

			if (d>=0)
			{
				y += sy;
				d -= ax;
			}
			x += sx;
			d += ay;
		}
	}
	else
	{                 
		/* y offset higher */
		d = ax-(ay>>1);
		for (;;)
		{
			ExtraSmartPlotBrushToMainBitMap(pw->pw_Project, x, y, col, mode);

			if (y==y2)
				goto TheEnd;

			if (d>=0)
			{
				x += sx;
				d -= ay;
			}
			y += sy;
			d += ax;
		}
	}
TheEnd:
	for(pw2 = pw->pw_Project->pj_ProjectWindows; pw2; pw2 = pw2->pw_NextProjWindow )
	{
		if(pw2->pw_sw.sw_Window)
			RefreshWindowSection(pw2, minx, miny, maxx-minx+1, maxy-miny+1);
	}
}




/******************* Do Smart Col Line *********************/
//
// Same as DoColLine() but, it only draws to the given Project
// window, and  uses the system where possible.
//   Written by Petri Kuittinen

void DoSmartColLine ( struct ProjectWindow *pw, struct Rectangle *rec,
											int x1, int y1, int x2, int y2, UWORD col, UWORD mode)
{
  /* d = delta, x and y are current coordinates, ax and ay are the
     absolute offset between x and y coordinates, sx and sy are signs
     of the offsets */
  register int d, x, y, ax, ay, sx, sy; 
	struct Rectangle localrec;
	BOOL premagnified = FALSE;

	if(SetSignal(0L,SIGF_BREAK) & SIGF_BREAK)
	{
		rec->MinX = x1;
		rec->MinY = y1;
		rec->MaxX = x1;
		rec->MaxY = y1;
		return;
	}

/*
	SetDrMd(win->RPort,COMPLEMENT);
	if( (currentbrush->br_ID == BR_DOT) && (pw->pw_PixelSize == 1) )
	{
		winxoffset = x1 - pw->pw_XOffset;
		winyoffset = y1 - pw->pw_YOffset;
		winx = winxoffset + win->BorderLeft;
		winy = winyoffset + win->BorderTop;
		Move(win->RPort,winx,winy);
		winxoffset = x2 - pw->pw_XOffset;
		winyoffset = y2 - pw->pw_YOffset;
		winx = winxoffset + win->BorderLeft;
		winy = winyoffset + win->BorderTop;
		Draw(win->RPort,winx,winy);
		return;
	}
 */

	SetDrMd(pw->pw_sw.sw_Window->RPort,JAM1);
	SetAPen(pw->pw_sw.sw_Window->RPort,col);

	if( ( (currentbrush->br_ID == BR_CUSTOM) || (currentbrush->br_ID == BR_ANIMBRUSH)) && (pw->pw_PixelSize != 1) )
	{
		/* If we're using a custom brush, then see if we can magnify it into the
			work buffer, for quick rendering */

		premagnified = MagnifyBrushIntoWorkSpace(&currentbrush->br_DrawBrush,pw->pw_PixelSize,col,mode);
	}

	if(rec)
	{
		rec->MinX = 16000;
		rec->MinY = 16000;
		rec->MaxX = 0;
		rec->MaxY = 0;
	}

  sx = 1;
	ax = (x2-x1)<<1;
  if (ax<0)
	{
		ax = -ax;
		sx = -sx;
	}
  sy = 1;
	ay = (y2-y1)<<1;
  if (ay<0)
	{
		ay = -ay;
		sy = -sy;
	}
  x = x1;
	y = y1;


	if (ax>ay)
	{           
		/* x offset higher */
		d = ay-(ax>>1);
		for (;;)
		{
			if( (QuickSmartPlotBrush(pw,x,y,&localrec,premagnified,col,mode)) && (rec) )
			{
				if(localrec.MinX < rec->MinX) rec->MinX = localrec.MinX;
				if(localrec.MinY < rec->MinY) rec->MinY = localrec.MinY;
				if(localrec.MaxX > rec->MaxX) rec->MaxX = localrec.MaxX;
				if(localrec.MaxY > rec->MaxY) rec->MaxY = localrec.MaxY;
			}
			if (x==x2) return;
			if (d>=0) {y += sy; d -= ax;}
			x += sx;
			d += ay;
			if(SetSignal(0L,SIGF_BREAK) & SIGF_BREAK)
				return;
		}
	}
	else
	{                 
		/* y offset higher */
		d = ax-(ay>>1);
		for (;;)
		{
			if( (QuickSmartPlotBrush(pw,x,y,&localrec,premagnified,col,mode)) && (rec) )
			{
				if(localrec.MinX < rec->MinX) rec->MinX = localrec.MinX;
				if(localrec.MinY < rec->MinY) rec->MinY = localrec.MinY;
				if(localrec.MaxX > rec->MaxX) rec->MaxX = localrec.MaxX;
				if(localrec.MaxY > rec->MaxY) rec->MaxY = localrec.MaxY;
			}
			if (y==y2) return;
			if (d>=0) {x += sx; d -= ay;}
			y += sy;
			d += ax;
			if(SetSignal(0L,SIGF_BREAK) & SIGF_BREAK)
				return;
		}
	}
}

/******************  Do Inv Horiz Line ************/
//
// A quick Horizontal line plotting routine. Saves plotting
// every pixel one by one.

void DoInvHorizLine( struct ProjectWindow *pw, int x1, int x2, int y1 )
{
	struct Window *win;
	WORD tmpx;

	if( (y1 < pw->pw_YOffset) || (y1 >= (pw->pw_YOffset+pw->pw_MagHeight)) )
		return;

	win = pw->pw_sw.sw_Window;

	SetDrMd(win->RPort,COMPLEMENT);

	if(x1 > x2)
	{
		tmpx = x1;
		x1 = x2;
		x2 = tmpx;
	}
	if(	(x2 < pw->pw_XOffset) || (x1 >= (pw->pw_XOffset+pw->pw_MagWidth)) )
		return;

	if(x1 < pw->pw_XOffset)
		x1 = pw->pw_XOffset;
	if(x2 > (pw->pw_XOffset+pw->pw_MagWidth-1))
		x2 = pw->pw_XOffset+pw->pw_MagWidth-1;

	x1 = ( (x1 - pw->pw_XOffset) * pw->pw_PixelSize) + win->BorderLeft;
	x2 = ( (x2 - pw->pw_XOffset) * pw->pw_PixelSize) + win->BorderLeft - 1;
	y1 = ( (y1 - pw->pw_YOffset) * pw->pw_PixelSize) + win->BorderTop;
	RectFill(win->RPort, x1, y1, x2+pw->pw_PixelSize, y1+pw->pw_PixelSize-1);
}

/***************** Do Inv Vert Line **************/
//
// Same as DoInvHorizLine() except vertical

void DoInvVertLine( struct ProjectWindow *pw, int x1, int y1, int y2 )
{
	struct Window *win;
	WORD tmpy;

	if( (x1 < pw->pw_XOffset) || (x1 >= (pw->pw_XOffset+pw->pw_MagWidth)) )
		return;

	win = pw->pw_sw.sw_Window;

	SetDrMd(win->RPort,COMPLEMENT);

	if(y1 > y2)
	{
		tmpy = y1;
		y1 = y2;
		y2 = tmpy;
	}

	if(	(y2 < pw->pw_YOffset) || (y1 >= (pw->pw_YOffset+pw->pw_MagHeight)) )
		return;

	if(y1 < pw->pw_YOffset)
		y1 = pw->pw_YOffset;
	if(y2 > (pw->pw_YOffset+pw->pw_MagHeight-1))
		y2 = pw->pw_YOffset+pw->pw_MagHeight-1;

	x1 = ( (x1 - pw->pw_XOffset) * pw->pw_PixelSize) + win->BorderLeft;
	y1 = ( (y1 - pw->pw_YOffset) * pw->pw_PixelSize) + win->BorderTop;
	y2 = ( (y2 - pw->pw_YOffset) * pw->pw_PixelSize) + win->BorderTop - 1;
	RectFill(win->RPort, x1, y1, x1+pw->pw_PixelSize-1, y2+pw->pw_PixelSize);
}

/******************  Do Col Horiz Line ************/
//
// A quick Horizontal line plotting routine. Saves plotting
// every pixel one by one.

void DoColHorizLine( struct ProjectWindow *pw, int x1, int x2, int y1, UWORD col)
{
	struct Window *win;
	WORD tmpx;

	if( (y1 < pw->pw_YOffset) || (y1 >= (pw->pw_YOffset+pw->pw_MagHeight)) )
		return;

	win = pw->pw_sw.sw_Window;

	SetDrMd(win->RPort,JAM1);
	SetAPen(win->RPort,col);

	if(x1 > x2)
	{
		tmpx = x1;
		x1 = x2;
		x2 = tmpx;
	}
	if(	(x2 < pw->pw_XOffset) || (x1 >= (pw->pw_XOffset+pw->pw_MagWidth)) )
		return;

	if(x1 < pw->pw_XOffset)
		x1 = pw->pw_XOffset;
	if(x2 > (pw->pw_XOffset+pw->pw_MagWidth-1))
		x2 = pw->pw_XOffset+pw->pw_MagWidth-1;

	x1 = ( (x1 - pw->pw_XOffset) * pw->pw_PixelSize) + win->BorderLeft;
	x2 = ( (x2 - pw->pw_XOffset) * pw->pw_PixelSize) + win->BorderLeft - 1;
	y1 = ( (y1 - pw->pw_YOffset) * pw->pw_PixelSize) + win->BorderTop;
	RectFill(win->RPort, x1, y1, x2+pw->pw_PixelSize, y1+pw->pw_PixelSize-1);
}

/***************** Do Col Vert Line **************/
//
// Same as DoColHorizLine() except vertical

void DoColVertLine( struct ProjectWindow *pw, int x1, int y1, int y2, UWORD col )
{
	struct Window *win;
	WORD tmpy;

	if( (x1 < pw->pw_XOffset) || (x1 >= (pw->pw_XOffset+pw->pw_MagWidth)) )
		return;

	win = pw->pw_sw.sw_Window;

	SetDrMd(win->RPort,JAM1);
	SetAPen(win->RPort,col);

	if(y1 > y2)
	{
		tmpy = y1;
		y1 = y2;
		y2 = tmpy;
	}

	if(	(y2 < pw->pw_YOffset) || (y1 >= (pw->pw_YOffset+pw->pw_MagHeight)) )
		return;

	if(y1 < pw->pw_YOffset)
		y1 = pw->pw_YOffset;
	if(y2 > (pw->pw_YOffset+pw->pw_MagHeight-1))
		y2 = pw->pw_YOffset+pw->pw_MagHeight-1;

	x1 = ( (x1 - pw->pw_XOffset) * pw->pw_PixelSize) + win->BorderLeft;
	y1 = ( (y1 - pw->pw_YOffset) * pw->pw_PixelSize) + win->BorderTop;
	y2 = ( (y2 - pw->pw_YOffset) * pw->pw_PixelSize) + win->BorderTop - 1;
	RectFill(win->RPort, x1, y1, x1+pw->pw_PixelSize-1, y2+pw->pw_PixelSize);
}

void DoInvCrossHairs(struct ProjectWindow *pw, int x, int y)
{
	DoInvHorizLine(pw,pw->pw_XOffset,pw->pw_XOffset+pw->pw_MagWidth-1,y);
	DoInvVertLine(pw,x,pw->pw_YOffset,pw->pw_YOffset+pw->pw_MagHeight-1);
}

void DoInvBox(struct ProjectWindow *pw, WORD  boxx,			WORD  boxy,
																				WORD boxwidth,	WORD boxheight)
{
	if( (boxwidth > 0) & (boxheight > 0) )
	{
		DoInvHorizLine(pw, boxx, boxx+boxwidth-1,boxy);
		DoInvHorizLine(pw, boxx, boxx+boxwidth-1,boxy+boxheight-1);
		if(boxheight > 1)
		{
			DoInvVertLine(pw, boxx, boxy+1, boxy+boxheight-2);
			DoInvVertLine(pw, boxx+boxwidth-1, boxy+1, boxy+boxheight-2);
		}
	}
}

void DoFloodFill(struct Project *pj, struct Rectangle *rec, WORD x, WORD y, UWORD col )
{
	UWORD backcol;

	backcol = GetPixelColour( pj->pj_MainBitMap, x, y);
	if( col != backcol )
		RecFloodFill( pj, rec, x, y, col, backcol);
	else
	{
		rec->MinX = x;
		rec->MinY = y;
		rec->MaxX = x;
		rec->MaxY = y;
	}
}

void DoSmartColOBox(struct ProjectWindow *pw, struct Rectangle *rec,
										UWORD minx, UWORD miny, UWORD maxx, UWORD maxy,
										UWORD col, UWORD mode)
{
	struct Rectangle tmprec;

	DoSmartColLine(pw,&tmprec,minx,miny,maxx,miny,col,mode);
	if(rec)
	{
		rec->MinX = tmprec.MinX;
		rec->MinY = tmprec.MinY;
		rec->MaxX = tmprec.MaxX;
		rec->MaxY = tmprec.MaxY;
	}
	DoSmartColLine(pw,&tmprec,maxx,miny,maxx,maxy,col,mode);
	if(tmprec.MinX < rec->MinX) rec->MinX = tmprec.MinX;
	if(tmprec.MinY < rec->MinY) rec->MinY = tmprec.MinY;
	if(tmprec.MaxX > rec->MaxX) rec->MaxX = tmprec.MaxX;
	if(tmprec.MaxY > rec->MaxY) rec->MaxY = tmprec.MaxY;
	DoSmartColLine(pw,&tmprec,maxx,maxy,minx,maxy,col,mode);
	if(tmprec.MinX < rec->MinX) rec->MinX = tmprec.MinX;
	if(tmprec.MinY < rec->MinY) rec->MinY = tmprec.MinY;
	if(tmprec.MaxX > rec->MaxX) rec->MaxX = tmprec.MaxX;
	if(tmprec.MaxY > rec->MaxY) rec->MaxY = tmprec.MaxY;
	DoSmartColLine(pw,&tmprec,minx,maxy,minx,miny,col,mode);
	if(tmprec.MinX < rec->MinX) rec->MinX = tmprec.MinX;
	if(tmprec.MinY < rec->MinY) rec->MinY = tmprec.MinY;
	if(tmprec.MaxX > rec->MaxX) rec->MaxX = tmprec.MaxX;
	if(tmprec.MaxY > rec->MaxY) rec->MaxY = tmprec.MaxY;
}

void DoExtraSmartColOBox(struct ProjectWindow *pw, struct Rectangle *rec,
												 UWORD minx, UWORD miny, UWORD maxx, UWORD maxy,
												 UWORD col, UWORD mode)
{
	struct Rectangle tmprec;

	DoExtraSmartColLine(pw,&tmprec,minx,miny,maxx,miny,col,mode);
	if(rec)
	{
		rec->MinX = tmprec.MinX;
		rec->MinY = tmprec.MinY;
		rec->MaxX = tmprec.MaxX;
		rec->MaxY = tmprec.MaxY;
	}
	DoExtraSmartColLine(pw,&tmprec,maxx,miny,maxx,maxy,col,mode);
	if(tmprec.MinX < rec->MinX) rec->MinX = tmprec.MinX;
	if(tmprec.MinY < rec->MinY) rec->MinY = tmprec.MinY;
	if(tmprec.MaxX > rec->MaxX) rec->MaxX = tmprec.MaxX;
	if(tmprec.MaxY > rec->MaxY) rec->MaxY = tmprec.MaxY;
	DoExtraSmartColLine(pw,&tmprec,maxx,maxy,minx,maxy,col,mode);
	if(tmprec.MinX < rec->MinX) rec->MinX = tmprec.MinX;
	if(tmprec.MinY < rec->MinY) rec->MinY = tmprec.MinY;
	if(tmprec.MaxX > rec->MaxX) rec->MaxX = tmprec.MaxX;
	if(tmprec.MaxY > rec->MaxY) rec->MaxY = tmprec.MaxY;
	DoExtraSmartColLine(pw,&tmprec,minx,maxy,minx,miny,col,mode);
	if(tmprec.MinX < rec->MinX) rec->MinX = tmprec.MinX;
	if(tmprec.MinY < rec->MinY) rec->MinY = tmprec.MinY;
	if(tmprec.MaxX > rec->MaxX) rec->MaxX = tmprec.MaxX;
	if(tmprec.MaxY > rec->MaxY) rec->MaxY = tmprec.MaxY;
}

void DoSmartColSBox(struct ProjectWindow *pw, struct Rectangle *rec,
										UWORD minx, UWORD miny, UWORD maxx, UWORD maxy,
										UWORD col)
{
	UWORD winright,winbottom;
	UWORD winrelleft,winreltop,winrelright,winrelbottom;
	struct Window *win;

	//printf("DSCSBStart: MinX: %d  MinY: %d MaxX %d  MaxY: %d\n",minx,miny,maxx,maxy);

	if(rec)
	{
		rec->MinX = minx;
		rec->MinY = miny;
		rec->MaxX = minx;
		rec->MaxY = miny;
	}

	if( !(win = pw->pw_sw.sw_Window) )
		return;

	winright = pw->pw_XOffset+pw->pw_MagWidth-1;
	winbottom = pw->pw_YOffset+pw->pw_MagHeight-1;

	if( (minx>winright) || (miny>winbottom) ||
			(maxx<pw->pw_XOffset) || (maxy<pw->pw_YOffset) )
		return;

	/* It's in the window (somwhere) */

	/* Trim the edges to fit inside the window */

	if(minx < pw->pw_XOffset) minx = pw->pw_XOffset;
	if(miny < pw->pw_YOffset) miny = pw->pw_YOffset;
	if(maxx > winright) maxx = winright;
	if(maxy > winbottom) maxy = winbottom;

	if(rec)
	{
		rec->MinX = minx;
		rec->MinY = miny;
		rec->MaxX = maxx;
		rec->MaxY = maxy;
	}

	/* Work out the window offsets */

	winrelleft = (minx - pw->pw_XOffset) * pw->pw_PixelSize;
	winreltop = (miny - pw->pw_YOffset) * pw->pw_PixelSize;
	winrelright = (maxx - pw->pw_XOffset) * pw->pw_PixelSize;
	winrelbottom = (maxy - pw->pw_YOffset) * pw->pw_PixelSize;

	//printf("PixelSize %d\n",pw->pw_PixelSize);
	//printf("DSCSBEnd: MinX: %d  MinY: %d MaxX %d  MaxY: %d\n",winrelleft,winreltop,winrelright,winrelbottom);

	winrelleft += win->BorderLeft;
	winrelright += win->BorderLeft + (pw->pw_PixelSize-1);
	winreltop += win->BorderTop;
	winrelbottom += win->BorderTop + (pw->pw_PixelSize-1);

	SetDrMd(win->RPort, JAM1);
	SetAPen(win->RPort,col);
	RectFill(win->RPort, winrelleft,  winreltop,
											 winrelright, winrelbottom);
}

void DoExtraSmartColSBox(struct ProjectWindow *pw, struct Rectangle *rec,
										WORD minx, WORD miny, WORD maxx, WORD maxy,
										UWORD col)
{
	struct Project *pj;

	//printf("DESCSB: MinX: %d  MinY: %d\nMaxX %d  MaxY: %d\n",minx,miny,maxx,maxy);

	pj = pw->pw_Project;

	if(minx < 0) minx = 0;
	if(miny < 0) miny = 0;
	if(maxx > pj->pj_Width-1) maxx = pj->pj_Width-1;
	if(maxy > pj->pj_Height-1) maxy = pj->pj_Height-1;

	if(rec)
	{
		rec->MinX = minx;
		rec->MinY = miny;
		rec->MaxX = maxx;
		rec->MaxY = maxy;
	}

	SetDrMd(pj->pj_MainRPort, JAM1);
	SetAPen(pj->pj_MainRPort,col);
	RectFill(pj->pj_MainRPort,minx,miny,maxx,maxy);

	for(pw=pj->pj_ProjectWindows; pw; pw=pw->pw_NextProjWindow )
	{
		if(pw->pw_sw.sw_Window)
			DoSmartColSBox(pw,NULL,minx,miny,maxx,maxy,col);
	}
}

/*******************    SmoothPoint()    *******************/
//
//  Averages out the surrounding points and the origin, and
// tries to find the best pen, to average the origin to.
// Return:		0xFFFF = No New Colour;
//						!0xFFFF = The New Colour;

UWORD SmoothPoint(struct Project *pj, WORD x, WORD y)
{
	UWORD red=0,green=0,blue=0;
	UWORD orgcol,col;
	struct Palette *pal;
	UWORD count=0;

	if( (x<0) || (y<0) || (x>=pj->pj_Width) || (y>=pj->pj_Height) )
		return(0xFFFF);

	pal = &pj->pj_Palette;

	orgcol = GetPixelColour(pj->pj_MainBitMap,x,y);
	red += pal->pl_Colours[orgcol].r;
	green += pal->pl_Colours[orgcol].g;
	blue += pal->pl_Colours[orgcol].b;
	count++;

	if( x > 0)
	{
		col = GetPixelColour(pj->pj_MainBitMap,x-1,y);
		red += pal->pl_Colours[col].r;
		green += pal->pl_Colours[col].g;
		blue += pal->pl_Colours[col].b;
		count++;

		if(y > 0)
		{
			col = GetPixelColour(pj->pj_MainBitMap,x-1,y-1);
			red += pal->pl_Colours[col].r;
			green += pal->pl_Colours[col].g;
			blue += pal->pl_Colours[col].b;
			count++;
		}

		if( y < pj->pj_Height-1)
		{
			col = GetPixelColour(pj->pj_MainBitMap,x-1,y+1);
			red += pal->pl_Colours[col].r;
			green += pal->pl_Colours[col].g;
			blue += pal->pl_Colours[col].b;
			count++;
		}
	}

	if( x < pj->pj_Width-1)
	{
		col = GetPixelColour(pj->pj_MainBitMap,x+1,y);
		red += pal->pl_Colours[col].r;
		green += pal->pl_Colours[col].g;
		blue += pal->pl_Colours[col].b;
		count++;

		if( y > 0)
		{
			col = GetPixelColour(pj->pj_MainBitMap,x+1,y-1);
			red += pal->pl_Colours[col].r;
			green += pal->pl_Colours[col].g;
			blue += pal->pl_Colours[col].b;
			count++;
		}

		if( y < pj->pj_Height-1)
		{
			col = GetPixelColour(pj->pj_MainBitMap,x+1,y+1);
			red += pal->pl_Colours[col].r;
			green += pal->pl_Colours[col].g;
			blue += pal->pl_Colours[col].b;
			count++;
		}
	}

	if(y > 0)
	{
		col = GetPixelColour(pj->pj_MainBitMap,x,y-1);
		red += pal->pl_Colours[col].r;
		green += pal->pl_Colours[col].g;
		blue += pal->pl_Colours[col].b;
		count++;
	}

	if( y < pj->pj_Height-1)
	{
		col = GetPixelColour(pj->pj_MainBitMap,x,y+1);
		red += pal->pl_Colours[col].r;
		green += pal->pl_Colours[col].g;
		blue += pal->pl_Colours[col].b;
		count++;
	}

	red = red/count;
	green = green/count;
	blue = blue/count;

	col = FindnBestPen(pal, pal->pl_Count, red, green, blue);
	if(col != orgcol)
	{
		PutPixelColour(pj->pj_MainBitMap, x, y, col);
		return(col);
	}
	else
		return(0xFFFF);
}

/***************************   ShadePoint()   **********************/

void ShadePoint(struct Project *pj, WORD x, WORD y, BOOL shadeup)
{
	WORD col,cell;

	if( (x<0) || (y<0) || (x>=pj->pj_Width) || (y>=pj->pj_Height) )
		return;

	col = GetPixelColour(pj->pj_MainBitMap,x,y);

	cell = GetColourCellInRange(pj->pj_CurrentRange,col);
	if(cell != -1)
	{
		if(shadeup)
		{
			for(cell++; cell <= 255; cell++)
			{
				col = pj->pj_CurrentRange->rn_Cell[cell];
				if(col != -1)
				{
					PutPixelColour(pj->pj_MainBitMap,x,y,col);
					return;
				}
			}
		}
		else
		{
			for(--cell; cell >= 0; --cell)
			{
				col = pj->pj_CurrentRange->rn_Cell[cell];
				if(col != -1)
				{
					PutPixelColour(pj->pj_MainBitMap,x,y,col);
					return;
				}
			}
		}
	}
}

void RefreshWindowSection(struct ProjectWindow *pw,	WORD minx, WORD miny, WORD width, WORD height)
{
	struct Window *win;
	struct Project *pj;
	WORD maxx,maxy;
	WORD winleft,wintop,winright,winbottom;
	WORD visableleft,visableright,visabletop,visablebottom;
	WORD winrelleft,winrelwidth,winreltop,winrelheight;
	struct QuickMagnify qm;

	pj = pw->pw_Project;
	win = pw->pw_sw.sw_Window;

	if( (width == 1) && (height == 1) )
	{
		if( ( minx > 0) && (miny > 0) && (minx < pj->pj_Width) && (miny < pj->pj_Height) )
		{
			SetAPen( win->RPort, GetPixelColour(pj->pj_MainBitMap,minx,miny) );
			SmartPlotPixel(pw, minx, miny);
		}
	}

	maxx = minx + width - 1;
	maxy = miny + height - 1;

	winleft = pw->pw_XOffset;
	winright = winleft + pw->pw_MagWidth-1;
	if( winright > (pj->pj_Width-1) )
		winright = pj->pj_Width - 1;

	wintop = pw->pw_YOffset;
	winbottom = wintop+ pw->pw_MagHeight-1;
	if( winbottom > (pj->pj_Height-1) )
		winbottom = pj->pj_Height - 1;

	if( (minx > winright) || (maxx < winleft) ||
			(miny > winbottom) || (maxy < wintop) )
		return;

	/* Ok, so if we get here then the area is definitely in the window somewhere */

	visableleft = max(minx,winleft);
	visableright = min(maxx,winright);
	visabletop = max(miny,wintop);
	visablebottom = min(maxy,winbottom);

	winrelleft = (visableleft - pw->pw_XOffset) * pw->pw_PixelSize;
	winrelwidth = (visableright - visableleft + 1) * pw->pw_PixelSize;
	winreltop = (visabletop - pw->pw_YOffset) * pw->pw_PixelSize;
	winrelheight = (visablebottom - visabletop + 1) * pw->pw_PixelSize;

	if(pw->pw_PixelSize != 1)
	{
		qm.qm_SrcBitMap = pj->pj_MainBitMap;
		qm.qm_SrcXOffset = visableleft;
		qm.qm_SrcYOffset = visabletop;
		qm.qm_SrcWidth = (visableright - visableleft + 1);
		qm.qm_SrcHeight = (visablebottom - visabletop + 1);
		qm.qm_DestBitMap = workbm;
		qm.qm_DestXOffset = 0;
		qm.qm_DestYOffset = 0;
		qm.qm_DestPixelSize = pw->pw_PixelSize;
		QuickMagnifyBitMap(&qm);
		BltBitMapRastPort(workbm, 0, 0, win->RPort,
											win->BorderLeft + winrelleft, win->BorderTop + winreltop,
											winrelwidth, winrelheight, 0xC0);
	}
	else
	{
		BltBitMapRastPort(pj->pj_MainBitMap, visableleft, visabletop,
											win->RPort, win->BorderLeft + winrelleft, win->BorderTop + winreltop,
											winrelwidth, winrelheight, 0xC0);
	}
}

void Plot4O( struct ProjectWindow *pw, int xo, int yo, int xoffset, int yoffset,
						UWORD col, UWORD mode )
{
	if(pw->pw_sw.sw_Window)
	{
		SmartPlotBrush( pw, xo + xoffset, yo + yoffset, NULL, col, mode);
		SmartPlotBrush( pw, xo + xoffset, yo - yoffset, NULL, col, mode);
		SmartPlotBrush( pw, xo - xoffset, yo + yoffset, NULL, col, mode);
		SmartPlotBrush( pw, xo - xoffset, yo - yoffset, NULL, col, mode);
	}
}


void SmartOEllipse( struct ProjectWindow *pw, int xo, int yo, int a, int b,
									 struct Rectangle *rec, UWORD col, UWORD mode)
{
	int asquared = a * a;
	int bsquared = b * b;
	int twoasquared = 2 * asquared;
	int twobsquared = 2 * bsquared;
	int x, y, twoxtimesbsquared, twoytimesasquared, error;

	if( (a==0) || (b==0) )
	{
		if(rec)
		{
			rec->MinX = xo;
			rec->MinY = yo;
			rec->MaxX = xo+1;
			rec->MaxY = yo+1;
		}
		return;
	}
	x = 0;
	y = b;
	twoxtimesbsquared = 0;
	twoytimesasquared = y * twoasquared;
	error = -y * asquared;

	if(rec)
	{
		rec->MinX = (xo - a) - currentbrush->br_DrawBrush.bi_XHandle;
		rec->MinY = (yo - b) - currentbrush->br_DrawBrush.bi_YHandle;
		rec->MaxX = (xo + a) + (currentbrush->br_DrawBrush.bi_Width - currentbrush->br_DrawBrush.bi_XHandle);
		rec->MaxY = (yo + b) + (currentbrush->br_DrawBrush.bi_Height - currentbrush->br_DrawBrush.bi_YHandle);
	}

	while( twoxtimesbsquared <= twoytimesasquared )
	{
		Plot4O( pw, xo, yo, x, y, col, mode );
		x += 1;
		twoxtimesbsquared += twobsquared;
		error += twoxtimesbsquared - bsquared;
		if( error >= 0 )
		{
			y -= 1;
			twoytimesasquared -= twoasquared;
			error -= twoytimesasquared;
		}
	}

	x = a;
	y = 0;
	twoxtimesbsquared = x * twobsquared;
	twoytimesasquared = 0;
	error = -x * bsquared;

	while( twoxtimesbsquared > twoytimesasquared )
	{
		Plot4O( pw, xo, yo, x, y, col, mode );
		y += 1;
		twoytimesasquared += twoasquared;
		error += twoytimesasquared - asquared;
		if( error >= 0 )
		{
			x -= 1;
			twoxtimesbsquared -= twobsquared;
			error -= twoxtimesbsquared;
		}
	}
}

void ExtraSmartPlot4O( struct Project *pj, int xo, int yo, int xoffset, int yoffset,
						UWORD col, UWORD mode )
{
	ExtraSmartPlotBrushToMainBitMap( pj, xo + xoffset, yo + yoffset, col, mode);
	ExtraSmartPlotBrushToMainBitMap( pj, xo + xoffset, yo - yoffset, col, mode);
	ExtraSmartPlotBrushToMainBitMap( pj, xo - xoffset, yo + yoffset, col, mode);
	ExtraSmartPlotBrushToMainBitMap( pj, xo - xoffset, yo - yoffset, col, mode);
}

void ExtraSmartOEllipse( struct Project *pj, int xo, int yo, int a, int b,
												struct Rectangle *rec, UWORD col, UWORD mode)
{
	int asquared = a * a;
	int bsquared = b * b;
	int twoasquared = 2 * asquared;
	int twobsquared = 2 * bsquared;
	int x, y, twoxtimesbsquared, twoytimesasquared, error;
	WORD	minx,miny,maxx,maxy;
	struct ProjectWindow *pw;

	if( (a==0) || (b==0) )
	{
		if(rec)
		{
			rec->MinX = xo;
			rec->MinY = yo;
			rec->MaxX = xo+1;
			rec->MaxY = yo+1;
		}
		return;
	}

	x = 0;
	y = b;
	twoxtimesbsquared = 0;
	twoytimesasquared = y * twoasquared;
	error = -y * asquared;

	minx = (xo - a) - currentbrush->br_DrawBrush.bi_XHandle;
	miny = (yo - b) - currentbrush->br_DrawBrush.bi_YHandle;
	maxx = (xo + a) + (currentbrush->br_DrawBrush.bi_Width - currentbrush->br_DrawBrush.bi_XHandle);
	maxy = (yo + b) + (currentbrush->br_DrawBrush.bi_Height - currentbrush->br_DrawBrush.bi_YHandle);
	if(minx < 0) minx = 0;
	if(miny < 0) miny = 0;
	if(maxx > pj->pj_Width - 1) maxx = pj->pj_Width - 1;
	if(maxy > pj->pj_Height - 1) maxy = pj->pj_Height - 1;

	if(rec)
	{
		rec->MinX = minx;
		rec->MinY = miny;
		rec->MaxX = maxx;
		rec->MaxY = maxy;
	}

	while( twoxtimesbsquared <= twoytimesasquared )
	{
		ExtraSmartPlot4O( pj, xo, yo, x, y, col, mode );
		x += 1;
		twoxtimesbsquared += twobsquared;
		error += twoxtimesbsquared - bsquared;
		if( error >= 0 )
		{
			y -= 1;
			twoytimesasquared -= twoasquared;
			error -= twoytimesasquared;
		}
	}

	x = a;
	y = 0;
	twoxtimesbsquared = x * twobsquared;
	twoytimesasquared = 0;
	error = -x * bsquared;

	while( twoxtimesbsquared > twoytimesasquared )
	{
		ExtraSmartPlot4O( pj, xo, yo, x, y, col, mode );
		y += 1;
		twoytimesasquared += twoasquared;
		error += twoytimesasquared - asquared;
		if( error >= 0 )
		{
			x -= 1;
			twoxtimesbsquared -= twobsquared;
			error -= twoxtimesbsquared;
		}
	}
	for(pw = pj->pj_ProjectWindows; pw; pw=pw->pw_NextProjWindow)
	{
		if(pw->pw_sw.sw_Window)
			RefreshWindowSection(pw,minx,miny,maxx-minx+1, maxy-miny+1);
	}
}


/* Solid Stuff */

void Plot4S( struct ProjectWindow *pw, int xo, int yo, int xoffset, int yoffset,
						UWORD col, UWORD mode )
{
	struct Window *win;
	int minx,maxx;

	if(win = pw->pw_sw.sw_Window)
	{
		minx = (xo - xoffset < 0) ? 0 : xo - xoffset;
		maxx = (xo + xoffset >= pw->pw_Project->pj_Width) ? pw->pw_Project->pj_Width-1 : xo + xoffset;
		if(yo - yoffset >= 0)
			DoColHorizLine( pw, minx, maxx, yo - yoffset, col );
		if(yo + yoffset < pw->pw_Project->pj_Height)
			DoColHorizLine( pw, minx, maxx, yo + yoffset, col );
	}
}


void SmartSEllipse( struct ProjectWindow *pw, int xo, int yo, int a, int b,
									 struct Rectangle *rec, UWORD col, UWORD mode)
{
	int asquared = a * a;
	int bsquared = b * b;
	int twoasquared = 2 * asquared;
	int twobsquared = 2 * bsquared;
	int x, y, twoxtimesbsquared, twoytimesasquared, error;

	if( (a==0) || (b==0) )
	{
		if(rec)
		{
			rec->MinX = xo;
			rec->MinY = yo;
			rec->MaxX = xo+1;
			rec->MaxY = yo+1;
		}
		return;
	}

	x = 0;
	y = b;
	twoxtimesbsquared = 0;
	twoytimesasquared = y * twoasquared;
	error = -y * asquared;

	if(rec)
	{
		rec->MinX = xo - a;
		rec->MinY = yo - b;
		rec->MaxX = xo + a;
		rec->MaxY = yo + b;
	}

	while( twoxtimesbsquared <= twoytimesasquared )
	{
		Plot4S( pw, xo, yo, x, y, col, mode );
		x += 1;
		twoxtimesbsquared += twobsquared;
		error += twoxtimesbsquared - bsquared;
		if( error >= 0 )
		{
			y -= 1;
			twoytimesasquared -= twoasquared;
			error -= twoytimesasquared;
		}
	}

	x = a;
	y = 0;
	twoxtimesbsquared = x * twobsquared;
	twoytimesasquared = 0;
	error = -x * bsquared;

	while( twoxtimesbsquared > twoytimesasquared )
	{
		Plot4S( pw, xo, yo, x, y, col, mode );
		y += 1;
		twoytimesasquared += twoasquared;
		error += twoytimesasquared - asquared;
		if( error >= 0 )
		{
			x -= 1;
			twoxtimesbsquared -= twobsquared;
			error -= twoxtimesbsquared;
		}
	}
}

void ExtraSmartPlot4S( struct Project *pj, int xo, int yo, int xoffset, int yoffset,
						UWORD col, UWORD mode )
{
	WORD minx,maxx;

	minx = ((xo - xoffset) > 0) ? (xo - xoffset) : 0;
	maxx = ((xo + xoffset) < (pj->pj_Width-1)) ? (xo + xoffset) : (pj->pj_Width - 1);
	if( ((yo - yoffset) >= 0) && ((yo - yoffset) < pj->pj_Height) )
		RectFill( pj->pj_MainRPort, minx, yo - yoffset, maxx, yo - yoffset);
	if( ((yo + yoffset) >= 0) && ((yo + yoffset) < pj->pj_Height) )
		RectFill( pj->pj_MainRPort, minx, yo + yoffset, maxx, yo + yoffset);
}

void ExtraSmartSEllipse( struct Project *pj, int xo, int yo, int a, int b,
												struct Rectangle *rec, UWORD col, UWORD mode)
{
	int asquared = a * a;
	int bsquared = b * b;
	int twoasquared = 2 * asquared;
	int twobsquared = 2 * bsquared;
	int x, y, twoxtimesbsquared, twoytimesasquared, error;
	WORD	minx,miny,maxx,maxy;
	struct ProjectWindow *pw;

	if( (a==0) || (b==0) )
	{
		if(rec)
		{
			rec->MinX = xo;
			rec->MinY = yo;
			rec->MaxX = xo+1;
			rec->MaxY = yo+1;
		}
		return;
	}

	x = 0;
	y = b;
	twoxtimesbsquared = 0;
	twoytimesasquared = y * twoasquared;
	error = -y * asquared;

	minx = xo - a;
	miny = yo - b;
	maxx = xo + a;
	maxy = yo + b;
	if(minx < 0) minx = 0;
	if(miny < 0) miny = 0;
	if(maxx > pj->pj_Width - 1) maxx = pj->pj_Width - 1;
	if(maxy > pj->pj_Height - 1) maxy = pj->pj_Height - 1;

	if(rec)
	{
		rec->MinX = minx;
		rec->MinY = miny;
		rec->MaxX = maxx;
		rec->MaxY = maxy;
	}

	if(pj->pj_MainRPort)
	{
		SetDrMd(pj->pj_MainRPort,JAM1);
		SetAPen(pj->pj_MainRPort,col);
	}

	while( twoxtimesbsquared <= twoytimesasquared )
	{
		ExtraSmartPlot4S( pj, xo, yo, x, y, col, mode );
		x += 1;
		twoxtimesbsquared += twobsquared;
		error += twoxtimesbsquared - bsquared;
		if( error >= 0 )
		{
			y -= 1;
			twoytimesasquared -= twoasquared;
			error -= twoytimesasquared;
		}
	}

	x = a;
	y = 0;
	twoxtimesbsquared = x * twobsquared;
	twoytimesasquared = 0;
	error = -x * bsquared;

	while( twoxtimesbsquared > twoytimesasquared )
	{
		ExtraSmartPlot4S( pj, xo, yo, x, y, col, mode );
		y += 1;
		twoytimesasquared += twoasquared;
		error += twoytimesasquared - asquared;
		if( error >= 0 )
		{
			x -= 1;
			twoxtimesbsquared -= twobsquared;
			error -= twoxtimesbsquared;
		}
	}
	for(pw = pj->pj_ProjectWindows; pw; pw=pw->pw_NextProjWindow)
	{
		if(pw->pw_sw.sw_Window)
			RefreshWindowSection(pw,minx,miny,maxx-minx+1, maxy-miny+1);
	}
}
