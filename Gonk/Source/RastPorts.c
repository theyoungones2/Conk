
#include <exec/memory.h>
#include <exec/types.h>
//#include <functions.h>
#include <graphics/clip.h>
#include <graphics/gfx.h>
#include <graphics/gfxmacros.h>
#include <graphics/layers.h>
#include <graphics/regions.h>
#include <graphics/text.h>
#include <graphics/rastport.h>
#include <intuition/preferences.h>

#include <proto/exec.h>
//#include <proto/dos.h>
//#include <proto/intuition.h>
#include <proto/graphics.h>
#include <proto/diskfont.h>
//#include <proto/asl.h>
//#include <clib/alib_protos.h>
//#include <proto/gadtools.h>
#include <proto/layers.h>
#include <global.h>
#include <externvars.h>

/*	Credits goto Todd_Lewis@unc.edu, though it hardly resembles the routine			*/
/*	he sent me in the first place.																							*/
/*	This module provides two functions for easily creating and deleting					*/
/*	off-screen layer BitMaps (which means they get clipped at the edges) with		*/
/*	corresponding RastPorts.  The RastPorts are not given a font.  These				*/
/*	are useful for temporary/scratch work areas where graphic images can be			*/
/*	built and then blitted onto the screen.																			*/

/*	AllocBitMapRPort() allocates a BitMap structure and associated Rasters of		*/
/*	the size indicated by the parameters.  It also allocates a Layer_Info				*/
/*	structure and a Layer structure which together provide clipping at the			*/
/*	BitMap borders. The function returns a pointer to the RastPort. Each				*/
/*	successful call to AllocBitMapRPort() eventually should be paired with a		*/
/*	call to FreeBitMapRPort() to free up any resources used by									*/
/*	AllocBitMapRPort().																													*/
/*		NOTE: "graphics.library" and "layers.library" must be opened prior to			*/
/*	any call to AllocBitMapRPort().																							*/

struct RastPort *AllocBitMapRPort(int width, int height, int depth )
{
	struct RastPort		*rp;
	struct BitMap			*bm;
	struct Layer_Info *layerinfo;
	struct Layer      *layer;

	if( !(bm = CreateBitMap(width,height,depth,FALSE)) )
	{
		GroovyReq("Error","Can't Allocate BitMap","Abort");
		return(NULL);
	}

	if( !(layerinfo = NewLayerInfo()) )
	{
		GroovyReq("Error","Can't Alloc NewLayerInfo","Abort");
		DestroyBitMap(bm,width,height,depth);
		return(NULL);

	}
	if(!(layer = CreateUpfrontLayer(layerinfo, bm, 0, 0, width-1, height-1, LAYERSIMPLE,
																  NULL)) )
	{
		GroovyReq("Error","Can't Alloc New Layer","Abort");
		DisposeLayerInfo( layerinfo );
		DestroyBitMap(bm,width,height,depth);
		return(NULL);
	}

	rp = layer->rp;

	return(rp);
}


/*	FreeBitMapRPort() returns to the system the memory allocated for the Layer,	*/
/*	Layer_Info, and associated RastPort and BitMap also created by							*/
/*	AllocBitMapRPort().  This function should only be called with a pointer			*/
/*	returned by a previous call to AllocBitMapRPort(). Any other use is an			*/
/*	error and will surely crash the system.																			*/
/*		NOTE: "graphics.library" and "layers.library" must be opened prior to			*/
/*	any call to FreeBitMapRPort().																							*/

/*void FreeBitMapRPort(struct RastPort *rp );																		*/


#define DUMMY 0L

        /* This is the clean-up routine.  The RastPort structure has	*/
        /* direct and indirect pointers to all the structures we			*/
        /* need to deal with.																					*/

void FreeBitMapRPort(struct RastPort *rp, int width, int height, int depth )
{
	struct	BitMap			*bm;
	struct	Layer_Info	*layerinfo;
	struct	Layer				*layer;

	if ( rp == NULL ) return;								/* A little bullet-proofing. */

			/* Find all the structs we need... */

	bm = rp->BitMap;
	layer	= rp->Layer;
	layerinfo = layer->LayerInfo;
	DeleteLayer(DUMMY, layer);
	DisposeLayerInfo(layerinfo);
	WaitTOF();
	WaitTOF();
	DestroyBitMap(bm,width,height,depth);
}
