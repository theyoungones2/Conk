
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
#include <math.h>

#include <global.h>
#include <ExternVars.h>

#define LMB 1
#define MMB 2
#define RMB 4

void RestoreDamageRegion(struct ProjectWindow *pw)
{
	struct Window *win;
	struct QuickMagnify qm;
	UWORD	col, winright, winbottom;

	win = pw->pw_sw.sw_Window;

	if( (pw->pw_DamageX0 == pw->pw_DamageX1) &&
			(pw->pw_DamageY0 == pw->pw_DamageY1) )
	{
		SetDrMd(win->RPort, JAM1);
		col = GetPixelColour(pw->pw_Project->pj_MainBitMap,
												 pw->pw_DamageX0, pw->pw_DamageY0);
		SetAPen(win->RPort, col);
		SmartPlotPixel(pw, pw->pw_DamageX0, pw->pw_DamageY0);
	}
	else
	{
		if(pw->pw_DamageX0 < pw->pw_XOffset) pw->pw_DamageX0 = pw->pw_XOffset;
		if(pw->pw_DamageY0 < pw->pw_YOffset) pw->pw_DamageY0 = pw->pw_YOffset;

		winright = pw->pw_XOffset+pw->pw_MagWidth-1;
		if(winright > pw->pw_Project->pj_Width - 1) winright = pw->pw_Project->pj_Width - 1;
		if(pw->pw_DamageX1 > winright) pw->pw_DamageX1 = winright;

		winbottom = pw->pw_YOffset+pw->pw_MagHeight-1;
		if(winbottom > pw->pw_Project->pj_Height - 1) winbottom = pw->pw_Project->pj_Height - 1;
		if(pw->pw_DamageY1 > winbottom) pw->pw_DamageY1 = winbottom;

		qm.qm_SrcBitMap = pw->pw_Project->pj_MainBitMap;
		qm.qm_SrcXOffset = pw->pw_DamageX0;
		qm.qm_SrcYOffset = pw->pw_DamageY0;
		qm.qm_SrcWidth = (pw->pw_DamageX1 - pw->pw_DamageX0 + 1);
		qm.qm_SrcHeight = (pw->pw_DamageY1 - pw->pw_DamageY0 + 1);
		qm.qm_DestBitMap = workbm;
		qm.qm_DestXOffset = 0;
		qm.qm_DestYOffset = 0;
		qm.qm_DestPixelSize = pw->pw_PixelSize;
		QuickMagnifyBitMap(&qm);
		/*My*/BltBitMapRastPort(workbm, 0, 0, win->RPort,
					win->BorderLeft + (pw->pw_DamageX0 - pw->pw_XOffset) * pw->pw_PixelSize,
					win->BorderTop  + (pw->pw_DamageY0 - pw->pw_YOffset) * pw->pw_PixelSize,
					(pw->pw_DamageX1 - pw->pw_DamageX0 + 1) * pw->pw_PixelSize,
					(pw->pw_DamageY1 - pw->pw_DamageY0 + 1) * pw->pw_PixelSize, 0xC0);
	}
}

/************  DominantColour()  ************/
//
// Works out which mouse button is dominant
// and returns its associated block number.
// buttonstate should be non-null, ie only
// call here if you know there is a button
// being pressed.

UWORD DominantColour( UWORD buttonstate )
{
	if( buttonstate & LMB )
		return( lmbcol );
	else
	{
		if( buttonstate & RMB )
			return( rmbcol );
		else
		{
			if( buttonstate & MMB )
				return( mmbcol );
		}
	}
	return(0);		/* nasty, shouldn't get to here */
}

/***************  DrawSelector()  ***************/
//
// Draw selector (cursor thing) at given position in window.
// Selector drawn depends on currenttool and toolstate (mouse buttons).
// ie, could be single pixel, line, rectangle etc... depends on tool.
//
// x0,y0 map coords, relative to window topleft.

void DrawSelector(struct ProjectWindow *pw, WORD x0, WORD y0)
{
	struct Window *win;
	struct Project *pj;
	WORD  minx,miny,maxx,maxy,tmp;
	struct Rectangle rec;
	UWORD mode;
	struct MyBob		*bob;

	pj = pw->pw_Project;

	if( (x0 < pj->pj_Width) && (y0 < pj->pj_Height) )
	{
		SetSignal(0L,SIGF_BREAK);
		win = pw->pw_sw.sw_Window;
		/* suss out window dimensions */
		
		pw->pw_SelType = currenttool;
		pw->pw_SelToolState = toolstate;
		pw->pw_SelState = TRUE;
		pw->pw_SelX0 = x0;
		pw->pw_SelY0 = y0;

		if( toolstate & (RMB|MMB) )
		{
			switch(drawmode)
			{
				case DRMD_REPLACE:
					mode = DRMD_SOLID;
					break;
				default:
					mode = DRMD_COLOUR;
					break;
			}
		}
		else
		{
			switch(drawmode)
			{
				case DRMD_SMOOTH:
					mode = DRMD_MATTE;
					break;
				case DRMD_SHADE:
					mode = DRMD_MATTE;
					break;
				default:
		 			mode = drawmode;
					break;
			}
		}

		switch( currenttool )
		{
			case TL_POINT:
				GridLockOffsets(&x0,&y0,&currentbrush->br_DrawBrush, pj->pj_Width, pj->pj_Height);
				pw->pw_SelX0 = x0;
				pw->pw_SelY0 = y0;
			case TL_PLOT:
				if( SmartPlotBrush(pw, x0, y0, &rec,lmbcol,mode) )
				{
					pw->pw_DamageX0 = rec.MinX;
					pw->pw_DamageY0 = rec.MinY;
					pw->pw_DamageX1 = rec.MaxX;
					pw->pw_DamageY1 = rec.MaxY;
				}
				else
					pw->pw_SelState = FALSE;
				break;
			case TL_MAGNIFY:
				if(pw->pw_Magnified)
					pw->pw_SelState = FALSE;
				else
				{
					pw->pw_DamageX0 = x0 - (pw->pw_MagnifyBoxWidth / 2);
					pw->pw_DamageY0 = y0 - (pw->pw_MagnifyBoxHeight / 2);
					if( (pw->pw_DamageX0 + pw->pw_MagnifyBoxWidth) > pj->pj_Width)
						pw->pw_DamageX0 = pj->pj_Width - pw->pw_MagnifyBoxWidth;
					if( (pw->pw_DamageY0 + pw->pw_MagnifyBoxHeight) > pj->pj_Height)
						pw->pw_DamageY0 = pj->pj_Height - pw->pw_MagnifyBoxHeight;
					if(pw->pw_DamageX0 < 0)
						pw->pw_DamageX0 = 0;
					if(pw->pw_DamageY0 < 0)
						pw->pw_DamageY0 = 0;
					pw->pw_DamageX1 = pw->pw_MagnifyBoxWidth;
					pw->pw_DamageY1 = pw->pw_MagnifyBoxHeight;
					DoInvBox(pw,pw->pw_DamageX0,pw->pw_DamageY0,pw->pw_DamageX1, pw->pw_DamageY1);
				}
				break;
			case TL_LINE:
				GridLockOffsets(&x0,&y0,&currentbrush->br_DrawBrush, pj->pj_Width, pj->pj_Height);
				pw->pw_SelX0 = x0;
				pw->pw_SelY0 = y0;
				if(toolstate)
				{
					DoSmartColLine(pw,&rec,pw->pw_SelX1,pw->pw_SelY1,pw->pw_SelX0,pw->pw_SelY0,DominantColour(toolstate),mode);
					pw->pw_DamageX0 = rec.MinX;
					pw->pw_DamageY0 = rec.MinY;
					pw->pw_DamageX1 = rec.MaxX;
					pw->pw_DamageY1 = rec.MaxY;
				}
				else
				{
					if( SmartPlotBrush(pw, pw->pw_SelX0, pw->pw_SelY0, &rec,lmbcol,mode) )
					{
						pw->pw_DamageX0 = rec.MinX;
						pw->pw_DamageY0 = rec.MinY;
						pw->pw_DamageX1 = rec.MaxX;
						pw->pw_DamageY1 = rec.MaxY;
					}
					else
						pw->pw_SelState = FALSE;
				}
				break;
			case TL_GETBRUSH:
				pw->pw_SelX0 = x0;
				pw->pw_SelY0 = y0;
				if(toolstate)
				{
					minx = pw->pw_SelX0;
					miny = pw->pw_SelY0;
					maxx = pw->pw_SelX1;
					maxy = pw->pw_SelY1;
					if(minx > maxx)
					{
						tmp = minx;
						minx = maxx;
						maxx = tmp;
					}
					if(miny > maxy)
					{
						tmp = miny;
						miny = maxy;
						maxy = tmp;
					}
					GetBrushGridLockOffsets(&minx, &miny, &maxx, &maxy);
					if( gridlockstate && glob.cg_ExcludeBrush )
					{
						if(minx != maxx)
							maxx -= 1;
						if(miny != maxy)
							maxy -= 1;
					}
					if(maxx > (pj->pj_Width-1))
						maxx = pj->pj_Width-1;
					if(maxy > (pj->pj_Height-1))
						maxx = pj->pj_Height-1;
					pw->pw_DamageX0 = minx;
					pw->pw_DamageY0 = miny;
					pw->pw_DamageX1 = maxx;
					pw->pw_DamageY1 = maxy;
					DoInvHorizLine(pw,minx,maxx-1,miny);
					DoInvVertLine (pw,maxx,miny,maxy-1);
					DoInvHorizLine(pw,maxx,minx+1,maxy);
					DoInvVertLine (pw,minx,maxy,miny+1);
				}
				else
				{
					pw->pw_DamageX0 = pw->pw_SelX0;
					pw->pw_DamageY0 = pw->pw_SelY0;
					GridLockOffsets(&pw->pw_DamageX0, &pw->pw_DamageY0, NULL, pj->pj_Width, pj->pj_Height);
					DoInvCrossHairs(pw, pw->pw_DamageX0, pw->pw_DamageY0);
				}
				break;
			case TL_GETANIMBRUSH:
				GridLockOffsets(&x0,&y0,NULL, pj->pj_Width, pj->pj_Height);
				pw->pw_SelX0 = x0;
				pw->pw_SelY0 = y0;
				if(toolstate)
				{
					minx = pw->pw_SelX0;
					miny = pw->pw_SelY0;
					maxx = pw->pw_SelX1;
					maxy = pw->pw_SelY1;
					if(minx > maxx)
					{
						tmp = minx;
						minx = maxx;
						maxx = tmp;
					}
					if(miny > maxy)
					{
						tmp = miny;
						miny = maxy;
						maxy = tmp;
					}
					if( gridlockstate && glob.cg_ExcludeBrush )
					{
						if(minx != maxx)
							maxx -= 1;
						if(miny != maxy)
							maxy -= 1;
					}
					pw->pw_DamageX0 = minx;
					pw->pw_DamageY0 = miny;
					pw->pw_DamageX1 = maxx;
					pw->pw_DamageY1 = maxy;
					DoInvHorizLine(pw,minx,maxx-1,miny);
					DoInvVertLine (pw,maxx,miny,maxy-1);
					DoInvHorizLine(pw,maxx,minx+1,maxy);
					DoInvVertLine (pw,minx,maxy,miny+1);
				}
				else
				{
					DoInvCrossHairs(pw, pw->pw_SelX0, pw->pw_SelY0);
				}
				break;
			case TL_BRUSHHANDLE:
				if(toolstate)
				{
					if( SmartPlotBrush(pw, pw->pw_SelX1, pw->pw_SelY1, &rec,lmbcol,mode) )
					{
						pw->pw_DamageX0 = rec.MinX;
						pw->pw_DamageY0 = rec.MinY;
						pw->pw_DamageX1 = rec.MaxX;
						pw->pw_DamageY1 = rec.MaxY;
						DoInvCrossHairs(pw, pw->pw_SelX0, pw->pw_SelY0);
					}
					else
						pw->pw_SelState = FALSE;
				}
				else
				{
					if( SmartPlotBrush(pw, x0, y0, &rec,lmbcol,mode) )
					{
						pw->pw_DamageX0 = rec.MinX;
						pw->pw_DamageY0 = rec.MinY;
						pw->pw_DamageX1 = rec.MaxX;
						pw->pw_DamageY1 = rec.MaxY;
					}
					else
						pw->pw_SelState = FALSE;				
				}
				break;
			case TL_OBOX:
				GridLockOffsets(&x0,&y0,&currentbrush->br_DrawBrush, pj->pj_Width, pj->pj_Height);
				pw->pw_SelX0 = x0;
				pw->pw_SelY0 = y0;
				if(toolstate)
				{
					minx = pw->pw_SelX0;
					miny = pw->pw_SelY0;
					maxx = pw->pw_SelX1;
					maxy = pw->pw_SelY1;
					if(minx > maxx)
					{
						tmp = minx;
						minx = maxx;
						maxx = tmp;
					}
					if(miny > maxy)
					{
						tmp = miny;
						miny = maxy;
						maxy = tmp;
					}
					DoSmartColOBox(pw,&rec,minx,miny,maxx,maxy,DominantColour(toolstate),mode);
					pw->pw_DamageX0 = rec.MinX;
					pw->pw_DamageY0 = rec.MinY;
					pw->pw_DamageX1 = rec.MaxX;
					pw->pw_DamageY1 = rec.MaxY;
				}
				else
				{
					if( SmartPlotBrush(pw, pw->pw_SelX0, pw->pw_SelY0, &rec,lmbcol,DRMD_MATTE) )
					{
						pw->pw_DamageX0 = rec.MinX;
						pw->pw_DamageY0 = rec.MinY;
						pw->pw_DamageX1 = rec.MaxX;
						pw->pw_DamageY1 = rec.MaxY;
					}
					else
						pw->pw_SelState = FALSE;
				}
				break;
			case TL_SBOX:
				GridLockOffsets(&x0,&y0,NULL, pj->pj_Width, pj->pj_Height);
				pw->pw_SelX0 = x0;
				pw->pw_SelY0 = y0;
				if(toolstate)
				{
					minx = pw->pw_SelX0;
					miny = pw->pw_SelY0;
					maxx = pw->pw_SelX1;
					maxy = pw->pw_SelY1;
					if(minx > maxx)
					{
						tmp = minx;
						minx = maxx;
						maxx = tmp;
					}
					if(miny > maxy)
					{
						tmp = miny;
						miny = maxy;
						maxy = tmp;
					}
					DoSmartColSBox(pw,&rec,minx,miny,maxx,maxy,DominantColour(toolstate));
					pw->pw_DamageX0 = rec.MinX;
					pw->pw_DamageY0 = rec.MinY;
					pw->pw_DamageX1 = rec.MaxX;
					pw->pw_DamageY1 = rec.MaxY;
				}
				else
				{
					SetAPen(win->RPort,lmbcol);
					SmartPlotPixel(pw,pw->pw_SelX0,pw->pw_SelY0);
					pw->pw_DamageX0 = pw->pw_SelX0;
					pw->pw_DamageY0 = pw->pw_SelY0;
					pw->pw_DamageX1 = pw->pw_SelX0;
					pw->pw_DamageY1 = pw->pw_SelY0;
				}
				break;
			case TL_OELLIPSE:
				GridLockOffsets(&x0,&y0,NULL, pj->pj_Width, pj->pj_Height);
				pw->pw_SelX0 = x0;
				pw->pw_SelY0 = y0;
				if(toolstate)
				{
					minx = pw->pw_SelX0;
					miny = pw->pw_SelY0;
					maxx = pw->pw_SelX1;
					maxy = pw->pw_SelY1;
					if(minx > maxx)
					{
						tmp = minx;
						minx = maxx;
						maxx = tmp;
					}
					if(miny > maxy)
					{
						tmp = miny;
						miny = maxy;
						maxy = tmp;
					}
					SmartOEllipse(pw, pw->pw_SelX1, pw->pw_SelY1, maxx-minx, maxy-miny, &rec, DominantColour(toolstate), mode);
					pw->pw_DamageX0 = rec.MinX;
					pw->pw_DamageX1 = rec.MaxX;
					pw->pw_DamageY0 = rec.MinY;
					pw->pw_DamageY1 = rec.MaxY;
				}
				else
				{
					DoInvCrossHairs(pw, pw->pw_SelX0, pw->pw_SelY0);
				}
				break;
			case TL_SELLIPSE:
				GridLockOffsets(&x0,&y0,NULL, pj->pj_Width, pj->pj_Height);
				pw->pw_SelX0 = x0;
				pw->pw_SelY0 = y0;
				if(toolstate)
				{
					minx = pw->pw_SelX0;
					miny = pw->pw_SelY0;
					maxx = pw->pw_SelX1;
					maxy = pw->pw_SelY1;
					if(minx > maxx)
					{
						tmp = minx;
						minx = maxx;
						maxx = tmp;
					}
					if(miny > maxy)
					{
						tmp = miny;
						miny = maxy;
						maxy = tmp;
					}
					SmartSEllipse(pw, pw->pw_SelX1, pw->pw_SelY1, maxx-minx, maxy-miny, &rec, DominantColour(toolstate), mode);
					pw->pw_DamageX0 = rec.MinX;
					pw->pw_DamageX1 = rec.MaxX;
					pw->pw_DamageY0 = rec.MinY;
					pw->pw_DamageY1 = rec.MaxY;
				}
				else
				{
					DoInvCrossHairs(pw, pw->pw_SelX0, pw->pw_SelY0);
				}
				break;

			case TL_OCIRCLE:
				GridLockOffsets(&x0,&y0,NULL, pj->pj_Width, pj->pj_Height);
				pw->pw_SelX0 = x0;
				pw->pw_SelY0 = y0;
				if(toolstate)
				{
					minx = pw->pw_SelX0;
					miny = pw->pw_SelY0;
					maxx = pw->pw_SelX1;
					maxy = pw->pw_SelY1;
					if(minx > maxx)
					{
						tmp = minx;
						minx = maxx;
						maxx = tmp;
					}
					if(miny > maxy)
					{
						tmp = miny;
						miny = maxy;
						maxy = tmp;
					}
					tmp = max( (maxx-minx), (maxy-miny) );
					SmartOEllipse(pw, pw->pw_SelX1, pw->pw_SelY1, tmp, tmp, &rec, DominantColour(toolstate), mode);
					pw->pw_DamageX0 = rec.MinX;
					pw->pw_DamageX1 = rec.MaxX;
					pw->pw_DamageY0 = rec.MinY;
					pw->pw_DamageY1 = rec.MaxY;
				}
				else
				{
					DoInvCrossHairs(pw, pw->pw_SelX0, pw->pw_SelY0);
				}
				break;
			case TL_SCIRCLE:
				GridLockOffsets(&x0,&y0,NULL, pj->pj_Width, pj->pj_Height);
				pw->pw_SelX0 = x0;
				pw->pw_SelY0 = y0;
				if(toolstate)
				{
					minx = pw->pw_SelX0;
					miny = pw->pw_SelY0;
					maxx = pw->pw_SelX1;
					maxy = pw->pw_SelY1;
					if(minx > maxx)
					{
						tmp = minx;
						minx = maxx;
						maxx = tmp;
					}
					if(miny > maxy)
					{
						tmp = miny;
						miny = maxy;
						maxy = tmp;
					}
					tmp = max( (maxx-minx), (maxy-miny) );
					SmartSEllipse(pw, pw->pw_SelX1, pw->pw_SelY1, tmp, tmp, &rec, DominantColour(toolstate), mode);
					pw->pw_DamageX0 = rec.MinX;
					pw->pw_DamageX1 = rec.MaxX;
					pw->pw_DamageY0 = rec.MinY;
					pw->pw_DamageY1 = rec.MaxY;
				}
				else
				{
					DoInvCrossHairs(pw, pw->pw_SelX0, pw->pw_SelY0);
				}
				break;
			case TL_HITZONE:
				pw->pw_SelX0 = x0;
				pw->pw_SelY0 = y0;
				if(BobCollisionZoneOK(pj))
				{
					bob = pj->pj_CurrentBob;
					pw->pw_DamageX0 = bob->bb_ColLeft;
					pw->pw_DamageY0 = bob->bb_ColTop;
					pw->pw_DamageX1 = bob->bb_ColRight - bob->bb_ColLeft + 1;
					pw->pw_DamageY1 = bob->bb_ColBottom - bob->bb_ColTop + 1;
					DoInvBox(pw, pw->pw_DamageX0, pw->pw_DamageY0, pw->pw_DamageX1, pw->pw_DamageY1);
				}
				else
					pw->pw_SelState = FALSE;
				break;
		}
	}
}

/***************  EraseSelector  ***************/
//
// Erase the previously drawn selector (if any).
//

void EraseSelector(struct ProjectWindow *pw)
{
	struct Window *win;
	// WORD x0,y0; /* ,x1,y1; */
	//UWORD col;
	//WORD boxx,boxy;
	WORD  minx,miny,maxx,maxy;
	//struct QuickMagnify qm;

	if(pw->pw_SelState)
	{
		win = pw->pw_sw.sw_Window;
		//x0 = pw->pw_SelX0;
		//y0 = pw->pw_SelY0;

		//printf("Erase Selector\n");

		switch( pw->pw_SelType )
		{
			case TL_POINT:
			case TL_PLOT:
				RestoreDamageRegion(pw);
				break;
			case TL_MAGNIFY:
				DoInvBox(pw,pw->pw_DamageX0,pw->pw_DamageY0,pw->pw_DamageX1, pw->pw_DamageY1);
				break;
			case TL_LINE:
				RestoreDamageRegion(pw);
				break;
			case TL_GETBRUSH:
				if(pw->pw_SelToolState)
				{
					minx = pw->pw_DamageX0;
					miny = pw->pw_DamageY0;
					maxx = pw->pw_DamageX1;
					maxy = pw->pw_DamageY1;
					DoInvHorizLine(pw,minx,maxx-1,miny);
					DoInvVertLine (pw,maxx,miny,maxy-1);
					DoInvHorizLine(pw,maxx,minx+1,maxy);
					DoInvVertLine (pw,minx,maxy,miny+1);
				}
				else
				{
					DoInvCrossHairs(pw, pw->pw_DamageX0, pw->pw_DamageY0);
				}
				break;
			case TL_GETANIMBRUSH:
				if(pw->pw_SelToolState)
				{
					minx = pw->pw_DamageX0;
					miny = pw->pw_DamageY0;
					maxx = pw->pw_DamageX1;
					maxy = pw->pw_DamageY1;
					DoInvHorizLine(pw,minx,maxx-1,miny);
					DoInvVertLine (pw,maxx,miny,maxy-1);
					DoInvHorizLine(pw,maxx,minx+1,maxy);
					DoInvVertLine (pw,minx,maxy,miny+1);
				}
				else
				{
					DoInvCrossHairs(pw, pw->pw_SelX0, pw->pw_SelY0);
				}
				break;
			case TL_BRUSHHANDLE:
				if(toolstate)
				{
					DoInvCrossHairs(pw, pw->pw_SelX0, pw->pw_SelY0);
					RestoreDamageRegion(pw);
				}
				else
				{
					RestoreDamageRegion(pw);
				}
				break;
			case TL_OBOX:
				RestoreDamageRegion(pw);
				break;
			case TL_SBOX:
				RestoreDamageRegion(pw);
				break;
			case TL_OELLIPSE:
				if(pw->pw_SelToolState)
					RestoreDamageRegion(pw);
				else
					DoInvCrossHairs(pw, pw->pw_SelX0, pw->pw_SelY0);
				break;
			case TL_SELLIPSE:
				if(pw->pw_SelToolState)
					RestoreDamageRegion(pw);
				else
					DoInvCrossHairs(pw, pw->pw_SelX0, pw->pw_SelY0);
				break;
			case TL_OCIRCLE:
				if(pw->pw_SelToolState)
					RestoreDamageRegion(pw);
				else
					DoInvCrossHairs(pw, pw->pw_SelX0, pw->pw_SelY0);
				break;
			case TL_SCIRCLE:
				if(pw->pw_SelToolState)
					RestoreDamageRegion(pw);
				else
					DoInvCrossHairs(pw, pw->pw_SelX0, pw->pw_SelY0);
				break;
			case TL_HITZONE:
				if(BobCollisionZoneOK(pw->pw_Project))
				{
					DoInvBox(pw, pw->pw_DamageX0, pw->pw_DamageY0, pw->pw_DamageX1, pw->pw_DamageY1);
				}
				break;
		}
		pw->pw_SelState = FALSE;
	}
}



/***************  Tool_Down()  ***************/
//
// Handles button-down events in projectwindows.
// x,y = pixel coord at which event occured,
// button = IECODE_xBUTTON code from IDCMP.
//

void Tool_Down(struct ProjectWindow *pw, WORD x, WORD y, WORD button)
{
	struct Project *pj;
	UWORD oldtoolstate = toolstate;
	//struct Rectangle rec;
	UWORD col,mode;
	struct MyBob *bob;

	EraseSelector(pw);

	pw->pw_SelX0 = x;
	pw->pw_SelY0 = y;

	pj = pw->pw_Project;

	/* modify bits in toolstate */
	switch( button )
	{
		case IECODE_LBUTTON:
			toolstate |= LMB;
			break;
		case IECODE_MBUTTON:
			toolstate |= MMB;
			break;
		case IECODE_RBUTTON:
			toolstate |= RMB;
			break;
	}

	if( toolstate & (RMB|MMB) )
	{
		switch(drawmode)
		{
			case DRMD_REPLACE:
				mode = DRMD_SOLID;
				break;
			case DRMD_SHADE:
				mode = drawmode;
				break;
			default:
				mode = DRMD_COLOUR;
				break;
		}
	}
	else
 		mode = drawmode;

	switch( button )
	{
		case IECODE_LBUTTON:
			col = lmbcol;
			if( (mode == DRMD_CYCLE) && (pj->pj_CurrentRange->rn_Cell[pj->pj_CycleCellNumber] != 0xFFFF) )
				col = pj->pj_CurrentRange->rn_Cell[pj->pj_CycleCellNumber];
			break;
		case IECODE_MBUTTON:
			col = mmbcol;
			break;
		case IECODE_RBUTTON:
			col = rmbcol;
			break;
	}

	/* take action depending on current tool */

	if( (x < pj->pj_Width) && (y < pj->pj_Height) )
	{
		switch( currenttool )
		{
			case TL_POINT:
				GridLockOffsets(&x,&y,&currentbrush->br_DrawBrush, pj->pj_Width, pj->pj_Height);
			case TL_PLOT:
				pw->pw_PrevX = x;
				pw->pw_PrevY = y;
				ResetUndoBuffer(pj);
				pj->pj_UndoTop = y - currentbrush->br_DrawBrush.bi_YHandle;
				pj->pj_UndoBottom = pj->pj_UndoTop + (currentbrush->br_DrawBrush.bi_Height - 1);
				pj->pj_UndoLeft = x - currentbrush->br_DrawBrush.bi_XHandle;
				pj->pj_UndoRight = pj->pj_UndoLeft + (currentbrush->br_DrawBrush.bi_Width - 1);

				ExtraSmartPlotBrush( pw->pw_Project, x, y, col,mode);
				/* set modified-project flag */
				SussOutModificationStuff(pj);
				if(currentbrush->br_ID == BR_ANIMBRUSH) GotoNextAnimBrushFrame(&animbrush);
				SussOutAltPaging(pj);
				break;
			case TL_LINE:
				GridLockOffsets(&x,&y,&currentbrush->br_DrawBrush, pj->pj_Width, pj->pj_Height);
				pw->pw_SelX1 = x;
				pw->pw_SelY1 = y;
				pw->pw_PrevX = x;
				pw->pw_PrevY = y;
				break;
			case TL_FILL:
				pw->pw_PrevX = x;
				pw->pw_PrevY = y;
				break;
			case TL_GETBRUSH:
				/* GridLockOffsets(&x,&y,NULL, pj->pj_Width, pj->pj_Height); */
				pw->pw_SelX1 = x;
				pw->pw_SelY1 = y;
				pw->pw_PrevX = x;
				pw->pw_PrevY = y;
				break;
			case TL_GETANIMBRUSH:
				GridLockOffsets(&x,&y,NULL, pj->pj_Width, pj->pj_Height);
				pw->pw_SelX1 = x;
				pw->pw_SelY1 = y;
				pw->pw_PrevX = x;
				pw->pw_PrevY = y;
				break;
			case TL_BRUSHHANDLE:
				pw->pw_SelX1 = x;
				pw->pw_SelY1 = y;
				pw->pw_PrevX = x;
				pw->pw_PrevY = y;
				break;
			case TL_OBOX:
				GridLockOffsets(&x,&y,&currentbrush->br_DrawBrush, pj->pj_Width, pj->pj_Height);
				pw->pw_SelX1 = x;
				pw->pw_SelY1 = y;
				pw->pw_PrevX = x;
				pw->pw_PrevY = y;
				break;
			case TL_SBOX:
				GridLockOffsets(&x,&y,NULL, pj->pj_Width, pj->pj_Height);
				pw->pw_SelX1 = x;
				pw->pw_SelY1 = y;
				pw->pw_PrevX = x;
				pw->pw_PrevY = y;
				break;
			case TL_OELLIPSE:
				GridLockOffsets(&x,&y,&currentbrush->br_DrawBrush, pj->pj_Width, pj->pj_Height);
				pw->pw_SelX1 = x;
				pw->pw_SelY1 = y;
				pw->pw_PrevX = x;
				pw->pw_PrevY = y;
				break;
			case TL_SELLIPSE:
				GridLockOffsets(&x,&y,&currentbrush->br_DrawBrush, pj->pj_Width, pj->pj_Height);
				pw->pw_SelX1 = x;
				pw->pw_SelY1 = y;
				pw->pw_PrevX = x;
				pw->pw_PrevY = y;
				break;
			case TL_OCIRCLE:
				GridLockOffsets(&x,&y,&currentbrush->br_DrawBrush, pj->pj_Width, pj->pj_Height);
				pw->pw_SelX1 = x;
				pw->pw_SelY1 = y;
				pw->pw_PrevX = x;
				pw->pw_PrevY = y;
				break;
			case TL_SCIRCLE:
				GridLockOffsets(&x,&y,&currentbrush->br_DrawBrush, pj->pj_Width, pj->pj_Height);
				pw->pw_SelX1 = x;
				pw->pw_SelY1 = y;
				pw->pw_PrevX = x;
				pw->pw_PrevY = y;
				break;
			case TL_PICKCOLOUR:
				pw->pw_PrevX = x;
				pw->pw_PrevY = y;
				break;
			case TL_HITZONE:
				pw->pw_PrevX = x;
				pw->pw_PrevY = y;
				if(BobCollisionZoneOK(pj))
				{
					bob = pj->pj_CurrentBob;
					/* We use pw->pw_SelX1 here to give the Number of the drag Bars: 1 = Left,
						2 = Right, 4 = Top, 8 = Bottom. All bits are mutually inclusive. */
					pw->pw_SelX1 = 0;
					if( (y >= bob->bb_ColTop) && (y <= bob->bb_ColBottom) )
					{
						if(x == bob->bb_ColLeft)
							pw->pw_SelX1 |= 1;
						else
						{
							/* Don't let the statements drop through, cos if the left & right are in the
								same place then we only want to pick up one of them */
							if(x == bob->bb_ColRight)
								pw->pw_SelX1 |= 2;
						}
					}
					if( (x >= bob->bb_ColLeft) && (x <= bob->bb_ColRight) )
					{
						if(y == bob->bb_ColTop)
							pw->pw_SelX1 |= 4;
						else
						{
							/* Don't let the statements drop through, 'cos if the top & bottom are in the
								same place then we only want to pick up one of them */
							if(y == bob->bb_ColBottom)
								pw->pw_SelX1 |= 8;
						}
					}
					if(pw->pw_SelX1 == 0)
						toolstate = oldtoolstate;
				}
				else
					toolstate = oldtoolstate;
				DrawSelector(pw,pw->pw_SelX0,pw->pw_SelY0);
				SussOutModificationStuff(pj);
				break;
		}
	}
}


/***************  Tool_Up()  ***************/

void Tool_Up(struct ProjectWindow *pw, WORD x, WORD y, WORD button)
{
	UWORD newtoolstate = toolstate;
	UWORD	col;
	struct Project *pj;
	WORD minx,miny,maxx,maxy,tmp;
	struct Rectangle rec;
	UWORD mode;

	col = DominantColour(toolstate);
	pj = pw->pw_Project;

	switch( button )
	{
		case IECODE_LBUTTON:
			newtoolstate &= ~LMB;
			break;
		case IECODE_MBUTTON:
			newtoolstate &= ~MMB;
			break;
		case IECODE_RBUTTON:
			newtoolstate &= ~RMB;
			break;
	}

	if( toolstate & (RMB|MMB) )
	{
		switch(drawmode)
		{
			case DRMD_REPLACE:
				mode = DRMD_SOLID;
				break;
			case DRMD_SHADE:
				mode = drawmode;
				break;
			default:
				mode = DRMD_COLOUR;
				break;
		}
	}
	else
 		mode = drawmode;

	if( (mode == DRMD_CYCLE) && (pj->pj_CurrentRange->rn_Cell[pj->pj_CycleCellNumber] != 0xFFFF) )
		col = pj->pj_CurrentRange->rn_Cell[pj->pj_CycleCellNumber];

	if( toolstate && 
		(x < pj->pj_Width) && (y < pj->pj_Height) )
	{
		switch( currenttool)
		{
			case TL_POINT:
				GridLockOffsets(&x,&y,&currentbrush->br_DrawBrush, pj->pj_Width, pj->pj_Height);
			case TL_PLOT:
				EraseSelector(pw);
				toolstate = newtoolstate;
				DrawSelector(pw, pw->pw_SelX0, pw->pw_SelY0);
				break;
			case TL_MAGNIFY:
				if(!pw->pw_Magnified)
				{
					pw->pw_XOffset = pw->pw_DamageX0;
					pw->pw_YOffset = pw->pw_DamageY0;
					EraseSelector(pw);
					pw->pw_PixelSize = pw->pw_MagnifyLevel;
					pw->pw_Magnified = TRUE;
					GetTrueWindowSize(pw);
					RedisplayProjWindow(pw,TRUE);
					toolstate = newtoolstate;
					NewTool(pw,prevtool);
					SetToolsWindowGadget(prevtool,TRUE);
				}
				break;
			case TL_LINE:
				SetWait(pw->pw_sw.sw_Window);
				ResetUndoBuffer(pj);
				DoExtraSmartColLine(pw, &rec, pw->pw_SelX1, pw->pw_SelY1, pw->pw_SelX0, pw->pw_SelY0, col, mode);
				pj->pj_UndoLeft = rec.MinX;
				pj->pj_UndoTop = rec.MinY;
				pj->pj_UndoRight = rec.MaxX;
				pj->pj_UndoBottom = rec.MaxY;
				toolstate = newtoolstate;
				if(currentbrush->br_ID == BR_ANIMBRUSH) GotoNextAnimBrushFrame(&animbrush);
				DrawSelector(pw, pw->pw_SelX0, pw->pw_SelY0);
				ClrWait(pw->pw_sw.sw_Window);
				SussOutModificationStuff(pj);
				break;
			case TL_FILL:
				SetWait(pw->pw_sw.sw_Window);
				ResetUndoBuffer(pj);
				DoFloodFill(pj, &rec, pw->pw_SelX0, pw->pw_SelY0, col );
				pj->pj_UndoLeft = rec.MinX;
				pj->pj_UndoTop = rec.MinY;
				pj->pj_UndoRight = rec.MaxX;
				pj->pj_UndoBottom = rec.MaxY;
				SussOutModificationStuff(pj);
				ClrWait(pw->pw_sw.sw_Window);
				break;
			case TL_GETBRUSH:
				minx = pw->pw_SelX1;
				miny = pw->pw_SelY1;
				maxx = pw->pw_SelX0;
				maxy = pw->pw_SelY0;
				if(minx > maxx)
				{
					tmp = minx;
					minx = maxx;
					maxx = tmp;
				}
				if(miny > maxy)
				{
					tmp = miny;
					miny = maxy;
					maxy = tmp;
				}
				GetBrushGridLockOffsets(&minx, &miny, &maxx, &maxy);
				if( gridlockstate && glob.cg_ExcludeBrush )
				{
					if(minx != maxx)
						maxx -= 1;
					if(miny != maxy)
						maxy -= 1;
				}
				if(maxx > (pj->pj_Width-1))
					maxx = pj->pj_Width-1;
				if(maxy > (pj->pj_Height-1))
					maxx = pj->pj_Height-1;
				EraseSelector(pw);
				toolstate = newtoolstate;
				FreeBrushBitMaps(&brushes[BR_CUSTOM]);
				if( GetCustomBrushImage(&brushes[BR_CUSTOM], pj->pj_MainBitMap,
																minx, miny, maxx-minx+1, maxy-miny+1) )
				{
					NewTool(pw,TL_POINT);
					SetToolsWindowGadget(TL_POINT,TRUE);
					currentbrush = &brushes[BR_CUSTOM];
					SetToolsWindowGadget(TLB_CUSTOM,TRUE);
					if( (button == IECODE_MBUTTON) ||
							(button == IECODE_RBUTTON) )
					{
						ResetUndoBuffer(pj);
						if( button == IECODE_MBUTTON)
							DoExtraSmartColSBox(pw,&rec,minx,miny,maxx,maxy,mmbcol);
						else
							DoExtraSmartColSBox(pw,&rec,minx,miny,maxx,maxy,rmbcol);
						pj->pj_UndoLeft = rec.MinX;
						pj->pj_UndoTop = rec.MinY;
						pj->pj_UndoRight = rec.MaxX;
						pj->pj_UndoBottom = rec.MaxY;
						SussOutModificationStuff(pj);
					}
					if(drawmode != DRMD_MATTE)
						NewDrawMode(DRMD_MATTE);
					DrawSelector(pw, pw->pw_SelX0, pw->pw_SelY0);
				}
				else
					GroovyReq("Get Brush","Not Enough Memory\nFor Brush","Continue");
				break;
			case TL_GETANIMBRUSH:
				GridLockOffsets(&x,&y,NULL, pj->pj_Width, pj->pj_Height);
				minx = pw->pw_SelX1;
				miny = pw->pw_SelY1;
				maxx = pw->pw_SelX0;
				maxy = pw->pw_SelY0;
				if(minx > maxx)
				{
					tmp = minx;
					minx = maxx;
					maxx = tmp;
				}
				if(miny > maxy)
				{
					tmp = miny;
					miny = maxy;
					maxy = tmp;
				}
				if( gridlockstate && glob.cg_ExcludeBrush )
				{
					if(minx != maxx)
						maxx -= 1;
					if(miny != maxy)
						maxy -= 1;
				}
				EraseSelector(pw);
				toolstate = newtoolstate;
				if( GUIGetAnimBrushFrames(&animbrush, pw, minx, miny, maxx-minx+1, maxy-miny+1, button) )
				{
					NewTool(pw,TL_POINT);
					SetToolsWindowGadget(TL_POINT,TRUE);
					currentbrush = &brushes[BR_ANIMBRUSH];
					SetToolsWindowGadget(TLB_ANIMBRUSH, TRUE);
					SussOutModificationStuff(pj);
					if(drawmode != DRMD_MATTE)
						NewDrawMode(DRMD_MATTE);
					DrawSelector(pw, pw->pw_SelX0, pw->pw_SelY0);
				}
				else
				{
					if( (currentbrush->br_ID == BR_ANIMBRUSH) && (IsListEmpty(&animbrush.ab_ABrushList)) )
						SetToolsWindowGadget(TLB_DOT, TRUE);
				}
				break;
			case TL_BRUSHHANDLE:
				EraseSelector(pw);
				toolstate = newtoolstate;
				currentbrush->br_DrawBrush.bi_XHandle += (pw->pw_SelX0 - pw->pw_SelX1);
				currentbrush->br_DrawBrush.bi_YHandle += (pw->pw_SelY0 - pw->pw_SelY1);
				NewTool(pw,prevtool);
				SetToolsWindowGadget(prevtool,TRUE);
				DrawSelector(pw, pw->pw_SelX0, pw->pw_SelY0);
				break;
			case TL_OBOX:
				SetWait(pw->pw_sw.sw_Window);
				minx = pw->pw_SelX1;
				miny = pw->pw_SelY1;
				maxx = pw->pw_SelX0;
				maxy = pw->pw_SelY0;
				if(minx > maxx)
				{
					tmp = minx;
					minx = maxx;
					maxx = tmp;
				}
				if(miny > maxy)
				{
					tmp = miny;
					miny = maxy;
					maxy = tmp;
				}
				ResetUndoBuffer(pj);
				DoExtraSmartColOBox(pw, &rec, minx, miny, maxx, maxy, col, mode);
				pj->pj_UndoLeft = rec.MinX;
				pj->pj_UndoTop = rec.MinY;
				pj->pj_UndoRight = rec.MaxX;
				pj->pj_UndoBottom = rec.MaxY;
				toolstate = newtoolstate;
				if(currentbrush->br_ID == BR_ANIMBRUSH) GotoNextAnimBrushFrame(&animbrush);
				DrawSelector(pw, pw->pw_SelX0, pw->pw_SelY0);
				/* set modified-project flag */
				SussOutModificationStuff(pj);
				ClrWait(pw->pw_sw.sw_Window);
				break;
			case TL_SBOX:
				GridLockOffsets(&x,&y,NULL, pj->pj_Width, pj->pj_Height);
				minx = pw->pw_SelX1;
				miny = pw->pw_SelY1;
				maxx = pw->pw_SelX0;
				maxy = pw->pw_SelY0;
				if(minx > maxx)
				{
					tmp = minx;
					minx = maxx;
					maxx = tmp;
				}
				if(miny > maxy)
				{
					tmp = miny;
					miny = maxy;
					maxy = tmp;
				}
				ResetUndoBuffer(pj);
				DoExtraSmartColSBox(pw,&rec,minx,miny,maxx,maxy,col);
				pj->pj_UndoLeft = rec.MinX;
				pj->pj_UndoTop = rec.MinY;
				pj->pj_UndoRight = rec.MaxX;
				pj->pj_UndoBottom = rec.MaxY;
				toolstate = newtoolstate;
				DrawSelector(pw, pw->pw_SelX0, pw->pw_SelY0);
				/* set modified-project flag */
				SussOutModificationStuff(pj);
				break;
			case TL_OELLIPSE:
				SetWait(pw->pw_sw.sw_Window);
				GridLockOffsets(&x,&y,NULL, pj->pj_Width, pj->pj_Height);
				minx = pw->pw_SelX1;
				miny = pw->pw_SelY1;
				maxx = pw->pw_SelX0;
				maxy = pw->pw_SelY0;
				if(minx > maxx)
				{
					tmp = minx;
					minx = maxx;
					maxx = tmp;
				}
				if(miny > maxy)
				{
					tmp = miny;
					miny = maxy;
					maxy = tmp;
				}
				ResetUndoBuffer(pj);
				ExtraSmartOEllipse(pj, pw->pw_SelX1, pw->pw_SelY1, maxx-minx, maxy-miny, &rec, col, mode);
				pj->pj_UndoLeft = rec.MinX;
				pj->pj_UndoTop = rec.MinY;
				pj->pj_UndoRight = rec.MaxX;
				pj->pj_UndoBottom = rec.MaxY;
				if(currentbrush->br_ID == BR_ANIMBRUSH) GotoNextAnimBrushFrame(&animbrush);
				SussOutModificationStuff(pj);
				ClrWait(pw->pw_sw.sw_Window);
				break;
			case TL_SELLIPSE:
				SetWait(pw->pw_sw.sw_Window);
				GridLockOffsets(&x,&y,NULL, pj->pj_Width, pj->pj_Height);
				minx = pw->pw_SelX1;
				miny = pw->pw_SelY1;
				maxx = pw->pw_SelX0;
				maxy = pw->pw_SelY0;
				if(minx > maxx)
				{
					tmp = minx;
					minx = maxx;
					maxx = tmp;
				}
				if(miny > maxy)
				{
					tmp = miny;
					miny = maxy;
					maxy = tmp;
				}
				ResetUndoBuffer(pj);
				ExtraSmartSEllipse(pj, pw->pw_SelX1, pw->pw_SelY1, maxx-minx, maxy-miny, &rec, col, mode);
				pj->pj_UndoLeft = rec.MinX;
				pj->pj_UndoTop = rec.MinY;
				pj->pj_UndoRight = rec.MaxX;
				pj->pj_UndoBottom = rec.MaxY;
				SussOutModificationStuff(pj);
				ClrWait(pw->pw_sw.sw_Window);
				break;

			case TL_OCIRCLE:
				SetWait(pw->pw_sw.sw_Window);
				GridLockOffsets(&x,&y,NULL, pj->pj_Width, pj->pj_Height);
				minx = pw->pw_SelX1;
				miny = pw->pw_SelY1;
				maxx = pw->pw_SelX0;
				maxy = pw->pw_SelY0;
				if(minx > maxx)
				{
					tmp = minx;
					minx = maxx;
					maxx = tmp;
				}
				if(miny > maxy)
				{
					tmp = miny;
					miny = maxy;
					maxy = tmp;
				}
				ResetUndoBuffer(pj);
				tmp = max( (maxx-minx), (maxy-miny) );
				ExtraSmartOEllipse(pj, pw->pw_SelX1, pw->pw_SelY1, tmp, tmp, &rec, col, mode);
				pj->pj_UndoLeft = rec.MinX;
				pj->pj_UndoTop = rec.MinY;
				pj->pj_UndoRight = rec.MaxX;
				pj->pj_UndoBottom = rec.MaxY;
				if(currentbrush->br_ID == BR_ANIMBRUSH) GotoNextAnimBrushFrame(&animbrush);
				SussOutModificationStuff(pj);
				ClrWait(pw->pw_sw.sw_Window);
				break;
			case TL_SCIRCLE:
				SetWait(pw->pw_sw.sw_Window);
				GridLockOffsets(&x,&y,NULL, pj->pj_Width, pj->pj_Height);
				minx = pw->pw_SelX1;
				miny = pw->pw_SelY1;
				maxx = pw->pw_SelX0;
				maxy = pw->pw_SelY0;
				if(minx > maxx)
				{
					tmp = minx;
					minx = maxx;
					maxx = tmp;
				}
				if(miny > maxy)
				{
					tmp = miny;
					miny = maxy;
					maxy = tmp;
				}
				ResetUndoBuffer(pj);
				tmp = max( (maxx-minx), (maxy-miny) );
				ExtraSmartSEllipse(pj, pw->pw_SelX1, pw->pw_SelY1, tmp, tmp, &rec, col, mode);
				pj->pj_UndoLeft = rec.MinX;
				pj->pj_UndoTop = rec.MinY;
				pj->pj_UndoRight = rec.MaxX;
				pj->pj_UndoBottom = rec.MaxY;
				SussOutModificationStuff(pj);
				ClrWait(pw->pw_sw.sw_Window);
				break;
			case TL_PICKCOLOUR:
				col = GetPixelColour(pj->pj_MainBitMap, pw->pw_SelX0, pw->pw_SelY0);
				switch(button)
				{
					case IECODE_LBUTTON:
						SetPalettePen(col,1);
						break;
					case IECODE_MBUTTON:
						SetPalettePen(col,2);
						break;
					case IECODE_RBUTTON:
						SetPalettePen(col,3);
						break;
				}
				NewTool(pw,prevtool);
				SetToolsWindowGadget(prevtool,TRUE);
				break;
		}
		pj->pj_CycleCellNumber = 0;
	}
	toolstate = newtoolstate;
}



/***************  Tool_Move()  ***************/

void Tool_Move(struct ProjectWindow *pw, WORD x, WORD y)
{
	struct Project *pj;
	UWORD mode,col = 0;
	struct MyBob *bob;

	pj = pw->pw_Project;

	if( (x < pj->pj_Width) && (y < pj->pj_Height) )
	{
		if( x != pw->pw_PrevX || y != pw->pw_PrevY  )
		{
			if( toolstate & (RMB|MMB) )
			{
				switch(drawmode)
				{
					case DRMD_REPLACE:
						mode = DRMD_SOLID;
						break;
					case DRMD_SHADE:
						mode = drawmode;
						break;
					default:
						mode = DRMD_COLOUR;
						break;
				}
			}
			else
		 		mode = drawmode;

			if(toolstate)
			{
				col = DominantColour(toolstate);
 				if( (mode == DRMD_CYCLE) && (pj->pj_CurrentRange->rn_Cell[pj->pj_CycleCellNumber] != 0xFFFF) )
					col = pj->pj_CurrentRange->rn_Cell[pj->pj_CycleCellNumber];
			}

			switch( currenttool )
			{
				case TL_POINT:
					GridLockOffsets(&x,&y,&currentbrush->br_DrawBrush, pj->pj_Width, pj->pj_Height);
					EraseSelector(pw);
					if(toolstate)
					{
						ExtraSmartPlotBrush( pw->pw_Project, x , y , col, mode);
						UpdateUndoBrush(currentbrush, pj, x, y);
						if(currentbrush->br_ID == BR_ANIMBRUSH) GotoNextAnimBrushFrame(&animbrush);
						SussOutAltPaging(pj);
					}
					else
						DrawSelector(pw, x, y);
					break;
				case TL_PLOT:
					EraseSelector(pw);
					if(toolstate)
					{
						DoExtraSmartColLine(pw, NULL, pw->pw_PrevX, pw->pw_PrevY, x, y, col, mode);
						UpdateUndoBrush(currentbrush, pj, x, y);
						if(currentbrush->br_ID == BR_ANIMBRUSH) GotoNextAnimBrushFrame(&animbrush);
						SussOutAltPaging(pj);
					}
					else
						DrawSelector(pw, x, y);
					break;
				case TL_MAGNIFY:
					EraseSelector(pw);
					DrawSelector(pw,x,y);
					break;
				case TL_LINE:
					GridLockOffsets(&x,&y,&currentbrush->br_DrawBrush, pj->pj_Width, pj->pj_Height);
					EraseSelector(pw);
					DrawSelector(pw,x,y);
					break;					
				case TL_FILL:
					EraseSelector(pw);
					DrawSelector(pw,x,y);
					break;					
				case TL_GETBRUSH:
					/* GridLockOffsets(&x,&y,NULL, pj->pj_Width, pj->pj_Height); */
					EraseSelector(pw);
					DrawSelector(pw, x, y);
					break;
				case TL_GETANIMBRUSH:
					GridLockOffsets(&x,&y,NULL, pj->pj_Width, pj->pj_Height);
					EraseSelector(pw);
					DrawSelector(pw,x,y);
					break;
				case TL_BRUSHHANDLE:
					GridLockOffsets(&x,&y,&currentbrush->br_DrawBrush, pj->pj_Width, pj->pj_Height);
					EraseSelector(pw);
					DrawSelector(pw,x,y);
					break;
				case TL_OBOX:
					GridLockOffsets(&x,&y,&currentbrush->br_DrawBrush, pj->pj_Width, pj->pj_Height);
					EraseSelector(pw);
					DrawSelector(pw,x,y);
					break;
				case TL_SBOX:
					GridLockOffsets(&x,&y,NULL, pj->pj_Width, pj->pj_Height);
					EraseSelector(pw);
					DrawSelector(pw,x,y);
					break;
				case TL_OELLIPSE:
					GridLockOffsets(&x,&y,&currentbrush->br_DrawBrush, pj->pj_Width, pj->pj_Height);
					EraseSelector(pw);
					DrawSelector(pw,x,y);
					break;
				case TL_SELLIPSE:
					GridLockOffsets(&x,&y,NULL, pj->pj_Width, pj->pj_Height);
					EraseSelector(pw);
					DrawSelector(pw,x,y);
					break;
				case TL_OCIRCLE:
					GridLockOffsets(&x,&y,&currentbrush->br_DrawBrush, pj->pj_Width, pj->pj_Height);
					EraseSelector(pw);
					DrawSelector(pw,x,y);
					break;
				case TL_SCIRCLE:
					GridLockOffsets(&x,&y,NULL, pj->pj_Width, pj->pj_Height);
					EraseSelector(pw);
					DrawSelector(pw,x,y);
					break;
				case TL_PICKCOLOUR:
					EraseSelector(pw);
					DrawSelector(pw,x,y);
					break;
				case TL_HITZONE:
					EraseSelector(pw);
					if(toolstate)
					{
						if(BobCollisionZoneOK(pj))
						{
							bob = pj->pj_CurrentBob;
							if( (pw->pw_SelX1 & 1) && (x <= bob->bb_ColRight) )	bob->bb_ColLeft = x;
							if( (pw->pw_SelX1 & 2) && (x >= bob->bb_ColLeft) )	bob->bb_ColRight = x;
							if( (pw->pw_SelX1 & 4) && (y <= bob->bb_ColBottom) )	bob->bb_ColTop = y;
							if( (pw->pw_SelX1 & 8) && (y >= bob->bb_ColTop) )	bob->bb_ColBottom = y;
						}
					}
					DrawSelector(pw,x,y);
					break;
			}
			pw->pw_PrevX = x;
			pw->pw_PrevY = y;

			/* Do stuff for updating DRMD_CYCLE cell number */
			if( (toolstate & LMB) && (drawmode == DRMD_CYCLE) )
			{
				/* Non wraping cycle */
				if( pj->pj_CycleCellNumber < 255 )
					pj->pj_CycleCellNumber += 1;
				if( (pj->pj_CurrentRange->rn_Cell[pj->pj_CycleCellNumber] == 0xFFFF) &&
						(pj->pj_CycleCellNumber != 0) )
				pj->pj_CycleCellNumber = 0;

				/*	Use 'pj->pj_CycleCellNumber -= 1;' for non cycling range */
			}
		}
	}
}

/***************  Tool_Cancel()  ***************/

void Tool_Cancel(struct ProjectWindow *pw, BOOL refresh)
{
}

/***************  New Tool() *******************/
void NewTool(struct ProjectWindow *pw,UWORD tool)
{
	if(pw)
		EraseSelector(pw);
	currenttool = tool;
	//DrawSelector(pw,pw->pw_SelX0, pw->pw_SelY0);
}

BOOL SussOutAltPaging(struct Project *pj)
{
	BOOL ret = FALSE;

	if( altdown && ((pj->pj_EditType == EDTY_BOBS) || (pj->pj_EditType == EDTY_SPRITES)) )
	{
		SussOutModificationStuff(pj);
		switch(pj->pj_EditType)
		{
			case EDTY_BOBS:
				GotoNextBob(pj);
				break;
			case EDTY_SPRITES:
				GotoNextSprite(pj);
				break;
		}
		ret = TRUE;
	}
	return(ret);
}
