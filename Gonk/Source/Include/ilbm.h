/* 
 * ilbm.h:	Definitions for IFFParse ILBM reader.
 */

#include <libraries/iffparse.h>

/* This macro computes the worst case packed size of a "row" of bytes. */
#define MaxPackedSize(rowSize)  ( (rowSize) + ( ((rowSize)+127) >> 7 ) )

/*  IFF types we may encounter  */
#define	ID_ILBM		MAKE_ID('I','L','B','M')
#define	ID_CONK		MAKE_ID('C','O','N','K')

/* ILBM Chunk ID's we may encounter
 */
#define ID_BODY		MAKE_ID('B','O','D','Y')
#define	ID_BMHD		MAKE_ID('B','M','H','D')
#define	ID_CMAP		MAKE_ID('C','M','A','P')
#define	ID_CRNG		MAKE_ID('C','R','N','G')
#define	ID_DRNG		MAKE_ID('D','R','N','G')
//#define	ID_CCRT		MAKE_ID('C','C','R','T')
#define	ID_GRAB		MAKE_ID('G','R','A','B')
#define	ID_SPRT		MAKE_ID('S','P','R','T')
//#define	ID_DEST		MAKE_ID('D','E','S','T')
#define	ID_CAMG		MAKE_ID('C','A','M','G')

#define	ID_DPAN		MAKE_ID('D','P','A','N')
#define	ID_ANHD		MAKE_ID('A','N','H','D')
#define	ID_DLTA		MAKE_ID('D','L','T','A')

#define	ID_BOBS		MAKE_ID('B','O','B','S')
#define	ID_BLKS		MAKE_ID('B','L','K','S')
#define	ID_ANIM		MAKE_ID('A','N','I','M')
#define	ID_GCFG		MAKE_ID('G','C','F','G')

/*  Masking techniques  */
#define	mskNone			0
#define	mskHasMask		1
#define	mskHasTransparentColor	2
#define	mskLasso		3

/* We plan to define this value, to mean that the BMHD.transparentColor
 * variable contains a count of alpha channel planes which are stored
 * in the BODY AFTER the image planes.  As always, the count of
 * masking/alpha planes is not included in BMHD.nPlanes.
 * If you have any comments or input, contact Chris Ludwig (CATS US).
 */
#define	mskHasAlpha		4

/*  Compression techniques  */
#define	cmpNone			0
#define	cmpByteRun1		1



/* ---------- BitMapHeader ---------------------------------------------*/
/*  Required Bitmap header (BMHD) structure describes an ILBM */
struct BitMapHeader
{
	UWORD	 bmh_Width;			/* Width in pixels, be careful of this being bigger than PageWidth */
	UWORD	 bmh_Height;		/* Height in pixels, be careful of this being bigger than PageHeight */
	WORD	 bmh_Left;			/* Left position */
	WORD	 bmh_Top;				/* Top position */
	UBYTE	 bmh_Depth;			/* Number of planes */
	UBYTE	 bmh_Masking;		/* Masking type */
	UBYTE	 bmh_Compression;	/* Compression type */
	UBYTE	 bmh_Flags;
	UWORD	 bmh_Transparent;	/* Transparent color */
	UBYTE	 bmh_XAspect;		/* Pixel aspect from the displayinfo->resoloution.x, Used */
	UBYTE	 bmh_YAspect;		/* for printing ILBM's correctly. */
	WORD	 bmh_PageWidth;
	WORD	 bmh_PageHeight;
};

/* BMHD flags */

/* Advisory that 8 significant bits-per-gun have been stored in CMAP
 * i.e. that the CMAP is definitely not 4-bit values shifted left.
 * This bit will disable nibble examination by color loading routine.
 */
#define BMHDB_CMAPOK	7
#define BMHDF_CMAPOK	(1 << BMHDB_CMAPOK)


/* ---------- ColorRegister --------------------------------------------*/
/* A CMAP chunk is a packed array of ColorRegisters (3 bytes each). */
struct ColorRegister
{
	UBYTE red, green, blue;   /* MUST be UBYTEs so ">> 4" won't sign extend.*/
};

/* ---------- Point2D --------------------------------------------------*/
/* A Point2D is stored in a GRAB chunk. */
struct Point2D
{
	WORD x, y;      /* coordinates (pixels) */
};

/* ---------- DestMerge ------------------------------------------------*/
/* A DestMerge is stored in a DEST chunk. */
struct DestMerge
{
	UBYTE depth;   /* # bitplanes in the original source */
	UBYTE pad1;      /* UNUSED; for consistency store 0 here */
	UWORD planePick;   /* how to scatter source bitplanes into destination */
	UWORD planeOnOff;   /* default bitplane data for planePick */
	UWORD planeMask;   /* selects which bitplanes to store into */
};

/* ---------- SpritePrecedence -----------------------------------------*/
/* A SpritePrecedence is stored in a SPRT chunk. */
struct SpritePrecedence
{
	UWORD SpritePrecedence;
};

/* ---------- Camg Amiga Viewport Mode Display ID ----------------------*/
/* The CAMG chunk is used to store the Amiga display mode in which
 * an ILBM is meant to be displayed.  This is very important, especially
 * for special display modes such as HAM and HALFBRITE where the
 * pixels are interpreted differently.
 * Under V37 and higher, store a 32-bit Amiga DisplayID (aka. ModeID)
 * in the ULONG ViewModes CAMG variable (from GetVPModeID(viewport)).
 * Pre-V37, instead store the 16-bit viewport->Modes.
 * See the current IFF manual for information on screening for bad CAMG
 * chunks when interpreting a CAMG as a 32-bit DisplayID or 16-bit ViewMode.
 * The chunk's content is declared as a ULONG.
 */
struct CamgChunk
{
	ULONG ViewModes;
} ;

/* ---------- CRange cycling chunk -------------------------------------*/
#define RNG_NORATE  36   /* Dpaint uses this rate to mean non-active */
/* A CRange is store in a CRNG chunk. */
struct CRange
{
	WORD  pad1;              /* reserved for future use; store 0 here */
	WORD  rate;      /* 60/sec=16384, 30/sec=8192, 1/sec=16384/60=273 */
	WORD  active;     /* bit0 set = active, bit 1 set = reverse */
	UBYTE low, high;   /* lower and upper color registers selected */
};

/* ---------- Ccrt (Graphicraft) cycling chunk -------------------------*/
/* A Ccrt is stored in a CCRT chunk. */
struct CcrtChunk
{
	WORD  direction;  /* 0=don't cycle, 1=forward, -1=backwards */
	UBYTE start;      /* range lower */
	UBYTE end;        /* range upper */
	LONG  seconds;    /* seconds between cycling */
	LONG  microseconds; /* msecs between cycling */
	WORD  pad;        /* future exp - store 0 here */
};

/* ---------- DPAN DPaint ANim Chunk -----------------------------------*/
struct DPAnimChunk
{
	UWORD dp_Version;			/* current version=4 (might be out of date, not important anyway) */
	UWORD dp_NumOfFrames;	/* number of frames in the animation.*/
	ULONG dp_Flags;				/* Not used */
};

/* ---------- ANHD ANimation HeaDer Chunk ------------------------------*/
struct AnimHdr
{
	UBYTE operation;
	UBYTE mask;
	UWORD w,h;
	WORD x,y;
	ULONG abstime;
	ULONG reltime;
	UBYTE interleave;
	UBYTE pad0;
	ULONG bits;
	UBYTE pad[16];
};
