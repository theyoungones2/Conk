/****************************************************************************/
//
// BONK: Tools.c
//
/****************************************************************************/



#include <stdio.h>
#include <devices/inputevent.h>
#include <graphics/gfx.h>
#include <exec/exec.h>

#include <proto/graphics.h>
#include <proto/exec.h>

#include <global.h>
#include <ExternVars.h>


#define LMB 1
#define MMB 2
#define RMB 4

extern UWORD currenttool;
static UWORD toolstate=0;

/************  ResetUndoBuffer()  ************/
//
//
//

void ResetUndoBuffer( struct ProjectWindow *pw, struct UndoBuffer *ub )
{
	ub->ub_ProjectWindow = pw;
	if( ub->ub_ArraySize > INITUNDOBUFSIZE )
	{
		FreeVec( ub->ub_Array );
		ub->ub_Array = NULL;
	}

	if( !ub->ub_Array )
	{
		if( ub->ub_Array = AllocVec( INITUNDOBUFSIZE * sizeof( struct UndoItem ),
			MEMF_ANY ) )
			ub->ub_ArraySize = INITUNDOBUFSIZE;
//			printf( "ResetUndoBuffer() new bufsize = %d\n", ub->ub_ArraySize );
	}
	ub->ub_Index = 0;
}



/************  RememberBlk()  ************/
//
// Add the given block and position info onto the
// given UndoBuffer.
//

void RememberBlk( struct UndoBuffer *ub, UWORD x, UWORD y, UWORD blk )
{
	struct UndoItem *tmp;
	ULONG i;

	if( ub->ub_Array )
	{
		if( ub->ub_Index == ub->ub_ArraySize )
		{
			/* overflow - gotta allocate a bigger array */

			if( tmp = AllocVec( ( ub->ub_ArraySize + UNDOBUFSTEP ) *
				sizeof( struct UndoItem ), MEMF_ANY ) )
			{
				/* setup new array and kill old one */
				CopyMem( ub->ub_Array, tmp,
					ub->ub_ArraySize * sizeof( struct UndoItem ) );
				FreeVec( ub->ub_Array );
				ub->ub_Array = tmp;
				ub->ub_ArraySize += UNDOBUFSTEP;
//				printf("New undo array, size: %d\n",ub->ub_ArraySize);
			}
		}

		/* add block into array (if enough room) */
		if( ub->ub_Index != ub->ub_ArraySize )
		{
			i = ub->ub_Index++;
			ub->ub_Array[i].ui_Block = blk;
			ub->ub_Array[i].ui_XPos = x;
			ub->ub_Array[i].ui_YPos = y;
//			printf("RememberBlk() index = %d\n",i);
		}
	}
}

/************  Undo()  ************/
//
//

void Undo( struct UndoBuffer *ub )
{
	ULONG i=ub->ub_Index;
	struct UndoItem *ui = ub->ub_Array;

	if( ui )
	{
		while( i-- > 0 )
		{
			PlotBlock( ub->ub_ProjectWindow, ui[i].ui_XPos, ui[i].ui_YPos, ui[i].ui_Block );
		}
		ub->ub_Index = 0;
	}
}


/************  FreeUndoBuffer  ************/
//
//
//

void FreeUndoBuffer( struct UndoBuffer *ub )
{
	if( ub->ub_Array )
	{
		FreeVec( ub->ub_Array );
		ub->ub_Array = 0;
	}
}

/************  DominantBlock()  ************/
//
// Works out which mouse button is dominant
// and returns its associated block number.
// buttonstate should be non-null, ie only
// call here if you know there is a button
// being pressed.

UWORD DominantBlock( UWORD buttonstate )
{
	if( buttonstate & LMB )
		return( lmbblk );
	else
	{
		if( buttonstate & RMB )
			return( rmbblk );
		else
		{
			if( buttonstate & MMB )
				return( mmbblk );
		}
	}
	return(0);		/* nasty, shouldn't get to here */
}



/************  PlotBlock()  ************/
//
// Plot a block on the map, and update all relevent windows.
//

void PlotBlock(struct ProjectWindow *pw, WORD mx, WORD my, UWORD blk)
{
	struct Project *proj;
	struct Window *win;
	WORD x,y;
	struct PreviewWindow *vw;

	/* see which project we're attached to */
	proj = pw->pw_Project;

	if( ( mx >= 0 ) && ( mx < proj->pj_MapW ) &&
		( my >= 0 ) && ( my < proj->pj_MapH ) )
	{

		/* Are we erasing? And with the background locked? And with a */
		/* valid background map? */
		if( blk == rmbblk && proj->pj_BackLockFlag && proj->pj_BackMap )
		{
			/* yep - plot the corresponding block from the background map */
			/* instead of erasing. */
			blk = *(proj->pj_BackMap + (my * proj->pj_MapW) + mx);
		}

		/* plot block onto the map */
		*(proj->pj_Map + (my * proj->pj_MapW) + mx) = blk;

		/* set modified-project flag */
		if( !proj->pj_ModifiedFlag )
		{
			proj->pj_ModifiedFlag = TRUE;
			RefreshProjectName(proj);
		}

		/* go through the ProjectWindows list and plot into them */
		for(pw = proj->pj_ProjectWindows; pw; pw = pw->pw_NextProjWindow)
		{
			/* make sure window isn't hidden*/
			if(win = pw->pw_sw.sw_Window)
			{
				/* find window-relative position */
				x = mx - pw->pw_MapX;
				y = my - pw->pw_MapY;
				/* plot if within window */
				if( InWindow(pw->pw_sw.sw_Window,x,y) )
					DrawBlock(win, x, y, blk);
			}
		}

		/* go through the PreviewWindows list and plot into them */
		for(vw = proj->pj_PreviewWindows; vw; vw = vw->vw_NextPreviewWindow)
		{
			/* make sure window isn't hidden*/
			if(win = vw->vw_sw.sw_Window)
			{
				/* find window-relative position */
				x = mx - vw->vw_WinX;
				y = my - vw->vw_WinY;
				WinPlot(win, x, y, FindBlockColour( globlocks, blk ) );
			}
		}
	}
}


/************  RPlotBlock()  ************/
//
// Plot a block on the map, and update all relevent windows.
// Block is also remembered in the undobuffer.
// ! This routine should be optimised !

void RPlotBlock(struct ProjectWindow *pw, WORD mx, WORD my, UWORD blk)
{
	RememberBlk( &undobuf, mx, my, PeekBlock( pw->pw_Project, mx, my ) );
	PlotBlock( pw, mx, my, blk);
}

/***************  PeekBlock()  ***************/
//
// Returns the value of the block at the given map coords.
//

UWORD PeekBlock(struct Project *proj, WORD mx, WORD my )
{
	/* read block from map */
	return( *(proj->pj_Map + (my * proj->pj_MapW) + mx) );
}



/***************  DoFBox()  ***************/
//
// Plot a filled box of blocks onto the map.
// mx0,my0,mx1,my1 coords of rectangle, blk=block to use.
//

void DoFBox(struct ProjectWindow *pw, WORD mx0, WORD my0,
	WORD mx1, WORD my1, UWORD blk )
{
	UWORD x,y;
	struct Project *proj;

	proj = pw->pw_Project;

	/* make sure we're on legal ground */
	if( ( mx0 >=0 ) && ( mx0 < proj->pj_MapW ) &&
		( mx1 >=0 ) && ( mx1 < proj->pj_MapW ) &&
		( my0 >=0 ) && ( my0 < proj->pj_MapH ) &&
		( my1 >=0 ) && ( my1 < proj->pj_MapH ) )
	{

		/* want mx0,my0 at top left corner */
		if( mx0 > mx1)
		{
			x=mx1;
			mx1=mx0;
			mx0=x;
		}
		if( my0 > my1)
		{
			y=my1;
			my1=my0;
			my0=y;
		}

		/* draw it on! */
		for(y = my0; y <= my1; y++ )
		{
			for(x = mx0; x <= mx1; x++ )
			{
				RPlotBlock(pw, x, y, blk);
			}
		}
	}
//	else
//		printf("DoFBox() Fuckup!\n");
}


/***************  DoBox()  ***************/
//
// Plot an outline box of blocks onto the map.
// mx0,my0,mx1,my1 coords of rectangle, blk=block to use.
//

void DoBox(struct ProjectWindow *pw, WORD mx0, WORD my0,
	WORD mx1, WORD my1, UWORD blk )
{
	UWORD x,y;
	struct Project *proj;

	proj = pw->pw_Project;

	/* make sure we're on legal ground */
	if( ( mx0 >=0 ) && ( mx0 < proj->pj_MapW ) &&
		( mx1 >=0 ) && ( mx1 < proj->pj_MapW ) &&
		( my0 >=0 ) && ( my0 < proj->pj_MapH ) &&
		( my1 >=0 ) && ( my1 < proj->pj_MapH ) )
	{

		/* want mx0,my0 at top left corner */
		if( mx0 > mx1)
		{
			x=mx1;
			mx1=mx0;
			mx0=x;
		}
		if( my0 > my1)
		{
			y=my1;
			my1=my0;
			my0=y;
		}

		/* draw it on! */
		for(x = mx0; x <= mx1; x++ )
		{
			RPlotBlock(pw, x, my0, blk);
			RPlotBlock(pw, x, my1, blk);
		}

		for(y = my0+1; y < my1; y++ )
		{
			RPlotBlock(pw, mx0, y, blk);
			RPlotBlock(pw, mx1, y, blk);
		}
	}
//	else
//		printf("DoBox() Fuckup!\n");
}


void RecFloodFill( struct ProjectWindow *pw, WORD x, WORD y, UWORD blk,
	UWORD backblk )
{
	WORD ledge, redge;
	struct Project *proj;

	proj = pw->pw_Project;

	if( x>=0 && y>=0 && x<proj->pj_MapW && y<proj->pj_MapH )
	{
		if(PeekBlock( pw->pw_Project,x, y ) == backblk )
		{
			for( ledge = x;
				ledge > 0 && PeekBlock( pw->pw_Project,ledge-1,y) == backblk;
				ledge-- );
			for( redge = x;
				redge < proj->pj_MapW && PeekBlock( pw->pw_Project,redge+1,y) == backblk;
				redge++ );
			for( x=ledge; x<=redge; x++)
				RPlotBlock( pw,x, y, blk );
			for( x=ledge; x<=redge; x++)
			{
				RecFloodFill( pw, x, y-1, blk, backblk );
				RecFloodFill( pw, x, y+1, blk, backblk );
			}
		}
	}
}



void DoFloodFill(struct ProjectWindow *pw, WORD x, WORD y, UWORD blk )
{
	UWORD backblk;

	backblk = PeekBlock( pw->pw_Project, x, y);
	if( blk != backblk )
		RecFloodFill( pw, x, y, blk, backblk);
}



/*
   Written by Petri Kuittinen, last modifications 25th June 1993
*/
void DoBlkLine ( struct ProjectWindow *pw, int x1, int y1, int x2, int y2, UWORD blk )
{
  /* d = delta, x and y are current coordinates, ax and ay are the
     absolute offset between x and y coordinates, sx and sy are signs
     of the offsets */
  register int d, x, y, ax, ay, sx, sy; 

  sx = 1; ax = (x2-x1)<<1;
  if (ax<0) {ax = -ax; sx = -sx;}
  sy = 1; ay = (y2-y1)<<1;
  if (ay<0) {ay = -ay; sy = -sy;}
  x = x1; y = y1;

  if (ax>ay)
    {           
      /* x offset higher */
      d = ay-(ax>>1);
      for (;;)
        {
//          (*put_pixel) (x, y);
					RPlotBlock( pw, x, y, blk);

          if (x==x2) return;
          if (d>=0) {y += sy; d -= ax;}
          x += sx; d += ay;
        }
    }
    else
      {                 
        /* y offset higher */
        d = ax-(ay>>1);
        for (;;)
          {
//            (*put_pixel) (x, y);
						RPlotBlock( pw, x, y, blk);
            if (y==y2) return;
            if (d>=0) {x += sx; d -= ay;}
            y += sy; d += ax;
          }
      }
}




/*
   Written by Petri Kuittinen
*/
void DoInvBlkLine ( struct ProjectWindow *pw, int x1, int y1, int x2, int y2 )
{
  /* d = delta, x and y are current coordinates, ax and ay are the
     absolute offset between x and y coordinates, sx and sy are signs
     of the offsets */
  register int d, x, y, ax, ay, sx, sy; 

  sx = 1; ax = (x2-x1)<<1;
  if (ax<0) {ax = -ax; sx = -sx;}
  sy = 1; ay = (y2-y1)<<1;
  if (ay<0) {ay = -ay; sy = -sy;}
  x = x1; y = y1;

  if (ax>ay)
    {           
      /* x offset higher */
      d = ay-(ax>>1);
      for (;;)
        {
//          (*put_pixel) (x, y);
//					PlotBlock( pw, x, y, blk);
					BlkRectFill( pw->pw_sw.sw_Window, x, y, x, y );

          if (x==x2) return;
          if (d>=0) {y += sy; d -= ax;}
          x += sx; d += ay;
        }
    }
    else
      {                 
        /* y offset higher */
        d = ax-(ay>>1);
        for (;;)
          {
//            (*put_pixel) (x, y);
//						PlotBlock( pw, x, y, blk);
						BlkRectFill( pw->pw_sw.sw_Window, x, y, x, y );
            if (y==y2) return;
            if (d>=0) {x += sx; d -= ay;}
            y += sy; d += ax;
          }
      }
}


/***************  DrawSelector()  ***************/
//
// Draw selector (cursor thing) at given position in window.
// Selector drawn depends on currenttool and toolstate (mouse buttons).
// ie, could be single block, line, rectangle etc... depends on tool.
//
// x0,y0 block coords, relative to window topleft.

void DrawSelector(struct ProjectWindow *pw, WORD x0, WORD y0)
{
	struct Window *win;
	WORD blockswide, blockshigh;
	WORD x1,y1;

	win = pw->pw_sw.sw_Window;
	/* suss out window dimensions */
	blockswide = ( win->Width - win->BorderLeft - win->BorderRight ) / BLKW;
	blockshigh = ( win->Height - win->BorderTop - win->BorderBottom ) / BLKH;

	/* draw all selectors in COMPLEMENT mode */
	SetDrMd(win->RPort, COMPLEMENT);

	pw->pw_SelType = currenttool;
	pw->pw_SelToolState = toolstate;
	pw->pw_SelState = TRUE;
	pw->pw_SelX0 = x0;
	pw->pw_SelY0 = y0;

	switch( currenttool )
	{
		case TL_DRAW:
		case TL_FILL:
		case TL_PICK:
			BlkRectFill( pw->pw_sw.sw_Window, x0, y0, x0, y0 );
			break;
		case TL_PASTE:
			x1 = x0 + currentbrush->br_Width - 1;
			y1 = y0 + currentbrush->br_Height - 1;
			if( x1<0 ) x1=0;
			if( y1<0 ) y1=0;
			if( x1>blockswide ) x1 = blockswide;
			if( y1>blockshigh ) y1 = blockshigh;
			pw->pw_SelX1 = x1;
			pw->pw_SelY1 = y1;
			BlkRectFill( pw->pw_sw.sw_Window, x0, y0, x1, y1 );
			break;
		case TL_BOX:
		case TL_FBOX:
		case TL_LINE:
		case TL_CUT:
			x1 = pw->pw_AnchorX - pw->pw_MapX;
			y1 = pw->pw_AnchorY - pw->pw_MapY;
			if( x1<0 ) x1=0;
			if( y1<0 ) y1=0;
			if( x1>blockswide ) x1 = blockswide;
			if( y1>blockshigh ) y1 = blockshigh;
			pw->pw_SelX1 = x1;
			pw->pw_SelY1 = y1;
			switch( currenttool )
			{
				case TL_BOX:
					if( toolstate )
						BlkRectOutline( pw->pw_sw.sw_Window, x0, y0, x1, y1 );
					else
						BlkRectFill( pw->pw_sw.sw_Window, x0, y0, x0, y0 );
					break;
				case TL_FBOX:
				case TL_CUT:
					if( toolstate )
						BlkRectFill( pw->pw_sw.sw_Window, x0, y0, x1, y1 );
					else
						BlkRectFill( pw->pw_sw.sw_Window, x0, y0, x0, y0 );
					break;
				case TL_LINE:
					if( toolstate )
						DoInvBlkLine( pw, x0, y0, x1, y1 );
					else
						BlkRectFill( pw->pw_sw.sw_Window, x0, y0, x0, y0 );
					break;
			}
			break;
	}
}

/***************  EraseSelector  ***************/
//
// Erase the previously drawn selector (if any).
//

void EraseSelector(struct ProjectWindow *pw)
{
	struct Window *win;
//	WORD x0,y0,x1,y1;

	if(pw->pw_SelState)
	{
		win = pw->pw_sw.sw_Window;
//		x0 = pw->pw_SelX0;
//		y0 = pw->pw_SelY0;

		SetDrMd(win->RPort, COMPLEMENT);

		switch( pw->pw_SelType )
		{
			case TL_DRAW:
			case TL_FILL:
			case TL_PICK:
				BlkRectFill(pw->pw_sw.sw_Window, pw->pw_SelX0, pw->pw_SelY0,
					pw->pw_SelX0,pw->pw_SelY0 );
				break;
			case TL_PASTE:
				BlkRectFill(pw->pw_sw.sw_Window, pw->pw_SelX0, pw->pw_SelY0,
					pw->pw_SelX1,pw->pw_SelY1 );
				break;
			case TL_BOX:
				if(pw->pw_SelToolState)
					BlkRectOutline(pw->pw_sw.sw_Window, pw->pw_SelX0, pw->pw_SelY0,
						pw->pw_SelX1,pw->pw_SelY1 );
				else
					BlkRectFill(pw->pw_sw.sw_Window, pw->pw_SelX0, pw->pw_SelY0,
						pw->pw_SelX0,pw->pw_SelY0 );
				break;
			case TL_FBOX:
			case TL_CUT:
				if(pw->pw_SelToolState)
					BlkRectFill(pw->pw_sw.sw_Window, pw->pw_SelX0, pw->pw_SelY0,
						pw->pw_SelX1,pw->pw_SelY1 );
				else
					BlkRectFill(pw->pw_sw.sw_Window, pw->pw_SelX0, pw->pw_SelY0,
						pw->pw_SelX0,pw->pw_SelY0 );
				break;
			case TL_LINE:
				if(pw->pw_SelToolState)
					DoInvBlkLine(pw, pw->pw_SelX0, pw->pw_SelY0,
						pw->pw_SelX1,pw->pw_SelY1 );
				else
					BlkRectFill(pw->pw_sw.sw_Window, pw->pw_SelX0, pw->pw_SelY0,
						pw->pw_SelX0,pw->pw_SelY0 );
				break;
		}
		pw->pw_SelState = FALSE;
	}
}



/***************  ResetSelector()  ***************/

void ResetSelector( struct ProjectWindow *pw )
{
	pw->pw_SelState = FALSE;
	pw->pw_PrevX = -1;
	pw->pw_PrevY = -1;
}



/***************  Tool_Down()  ***************/
//
// Handles button-down events in projectwindows.
// x,y = block coord at which event occured,
// button = IECODE_xBUTTON code from IDCMP.
//

void Tool_Down(struct ProjectWindow *pw, WORD x, WORD y, WORD button)
{
//	printf("Tool_Down(), button:%d, x:%d, y:%d\n",button,x,y);
	UWORD oldtoolstate = toolstate;

	EraseSelector(pw);

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

//	printf("currenttool: %d\n",currenttool);

	/* take action depending on current tool */

	switch( currenttool )
	{
		case TL_DRAW:
			pw->pw_PrevX = x;
			pw->pw_PrevY = y;
			ResetUndoBuffer( pw, &undobuf );
			switch( button )
			{
				case IECODE_LBUTTON:
					RPlotBlock( pw, x + pw->pw_MapX, y + pw->pw_MapY, lmbblk );
					break;
				case IECODE_MBUTTON:
					RPlotBlock( pw, x + pw->pw_MapX, y + pw->pw_MapY, mmbblk );
//					printf( "Block0: %d\nBlock1: %d\n", FindBlockColour(globlocks,0),
//					FindBlockColour(globlocks,1) );
					break;
				case IECODE_RBUTTON:
					RPlotBlock( pw, x + pw->pw_MapX, y + pw->pw_MapY, rmbblk );
					break;
			}
			break;
		case TL_BOX:
		case TL_FBOX:
		case TL_LINE:
		case TL_CUT:
			if( !oldtoolstate )
			{
				ResetUndoBuffer( pw, &undobuf );
				pw->pw_AnchorX = x + pw->pw_MapX;
				pw->pw_AnchorY = y + pw->pw_MapY;
			}
			else
				toolstate = 0;
			break;
		case TL_PICK:
			switch( button )
			{
				case IECODE_LBUTTON:
					lmbblk = PeekBlock( pw->pw_Project, x + pw->pw_MapX, y + pw->pw_MapY );
					break;
				case IECODE_MBUTTON:
					mmbblk = PeekBlock( pw->pw_Project, x + pw->pw_MapX, y + pw->pw_MapY );
//					mmbblk = PeekBlock( pw->pw_Project, x, y );
					break;
				case IECODE_RBUTTON:
					rmbblk = PeekBlock( pw->pw_Project, x + pw->pw_MapX, y + pw->pw_MapY );
//					rmbblk = PeekBlock( pw->pw_Project, x, y );
					break;
			}
			SetTool( TL_DRAW );
			break;
		case TL_FILL:
			ResetUndoBuffer( pw, &undobuf );
			DoFloodFill( pw, pw->pw_MapX+x,pw->pw_MapY+y, DominantBlock(toolstate) );
			break;
		case TL_PASTE:
			ResetUndoBuffer( pw, &undobuf );
			if( toolstate & RMB )
				DoFBox(pw, x + pw->pw_MapX, y + pw->pw_MapY,
					x + pw->pw_MapX + currentbrush->br_Width - 1,
					y + pw->pw_MapY + currentbrush->br_Height - 1,
					rmbblk );
			else
				PasteBrush( pw, pw->pw_MapX+x, pw->pw_MapY+y, currentbrush );
			break;
	}
}



/***************  Tool_Up()  ***************/

void Tool_Up(struct ProjectWindow *pw, WORD x, WORD y, WORD button)
{
	//printf("Tool_Up(), button:%d, x:%d, y:%d\n",button,x,y);

	UWORD oldtoolstate = toolstate;
	struct Brush *tempbrush;

	switch( button )
	{
		case IECODE_LBUTTON:
			toolstate &= ~LMB;
			break;
		case IECODE_MBUTTON:
			toolstate &= ~MMB;
			break;
		case IECODE_RBUTTON:
			toolstate &= ~RMB;
			break;
	}

	if( oldtoolstate )
	{
		switch( currenttool)
		{
			case TL_BOX:
				EraseSelector(pw);
				DoBox(pw, pw->pw_AnchorX, pw->pw_AnchorY,
					x + pw->pw_MapX, y + pw->pw_MapY, DominantBlock(oldtoolstate) );
				toolstate=0;
				break;
			case TL_FBOX:
				EraseSelector(pw);
				DoFBox(pw, pw->pw_AnchorX, pw->pw_AnchorY,
					x + pw->pw_MapX, y + pw->pw_MapY, DominantBlock(oldtoolstate) );
				toolstate=0;
				break;
			case TL_LINE:
				EraseSelector(pw);
				DoBlkLine(pw, x + pw->pw_MapX, y + pw->pw_MapY,
					pw->pw_AnchorX, pw->pw_AnchorY,
					DominantBlock(oldtoolstate) );
				toolstate=0;
				break;
			case TL_CUT:
				EraseSelector(pw);
				//FreeBrush( currentbrush );
				tempbrush = GrabBrush( pw->pw_Project, pw->pw_AnchorX, pw->pw_AnchorY,
					x + pw->pw_MapX, y + pw->pw_MapY );
				if( tempbrush )
				{
//					Insert( &brushes, &tempbrush->br_Node, &currentbrush->br_Node );
					AddTail( &brushes, (struct Node *)tempbrush );
					FreeOldBrushes();
					currentbrush = tempbrush;
					RefreshAllBrushWindows();
				}
				if( button == IECODE_RBUTTON )
				{
					DoFBox( pw, pw->pw_AnchorX, pw->pw_AnchorY,
					x + pw->pw_MapX, y + pw->pw_MapY, rmbblk );
				}
				toolstate=0;
				SetTool( TL_PASTE );
				break;
		}
	}
}



/***************  Tool_Move()  ***************/

void Tool_Move(struct ProjectWindow *pw, WORD x, WORD y)
{
	if( x != pw->pw_PrevX || y != pw->pw_PrevY  )
	{
		switch( currenttool )
		{
			case TL_DRAW:
				if( InWindow(pw->pw_sw.sw_Window,x,y) )
				{
					EraseSelector(pw);
					if(toolstate && ( x != pw->pw_PrevX || y != pw->pw_PrevY ) )
						RPlotBlock( pw, x + pw->pw_MapX, y + pw->pw_MapY,
							DominantBlock(toolstate) );
					DrawSelector(pw, x, y);
				}
				break;
			case TL_PASTE:
				if( InWindow(pw->pw_sw.sw_Window,x,y) )
				{
					EraseSelector(pw);
					if(toolstate && ( x != pw->pw_PrevX || y != pw->pw_PrevY ) )
					{
						if( toolstate & RMB )
							DoFBox(pw, x + pw->pw_MapX, y + pw->pw_MapY,
								x + pw->pw_MapX + currentbrush->br_Width - 1,
								y + pw->pw_MapY + currentbrush->br_Height - 1,
								rmbblk );
						else
							PasteBrush( pw, x + pw->pw_MapX, y + pw->pw_MapY, currentbrush );
					}
					DrawSelector(pw, x, y);
				}
				break;
			case TL_BOX:
			case TL_FBOX:
			case TL_FILL:
			case TL_PICK:
			case TL_LINE:
			case TL_CUT:
				if( InWindow( pw->pw_sw.sw_Window,x,y) )
				{
					EraseSelector(pw);
					DrawSelector(pw, x, y);
				}
		}
		pw->pw_PrevX = x;
		pw->pw_PrevY = y;
	}
}

/***************  Tool_Cancel()  ***************/

void Tool_Cancel(struct ProjectWindow *pw, BOOL refresh)
{
}



