/*************  Game Configuration File Stuff (19/09/95)  ******************/
/* Used by Lame and Zonk */

#define GAME_H

#define LEVELFILENAMESIZE			10
#define LEVELFULLNAMESIZE			14

struct GameFileHeader
{
	char					gfh_GameName[64];
	char					gfh_Author[64];
	char					gfh_Comment[128];
	UWORD					gfh_ConkVersion;			/* currently 1 */
	UWORD					gfh_Computer;					/* 42 = Amiga */
	UWORD					gfh_NumOfLevels;
	ULONG					gfh_Reserved[4];
	/* LevelFileHeader[gfh_NumOfLevels] */
};

struct LevelFileHeader
{
	char					lfh_LevelName[ LEVELFILENAMESIZE ];		/* packed */
	UWORD					lfh_LevelType;												/* see below */
	UWORD					lfh_NumOfFiles;
	char					lfh_Comment[64];
	char					lfh_Password[10];
	ULONG					lfh_Reserved[4];
	/*
	UWORD					lfh_FileType;				\__ * lfh_NumOfFiles
	char					lfh_FileName[...];	/
	 */
};

/* Level Types */
#define LT_GAME 0
#define LT_TITLE 1
#define LT_ANIM 2

/* File Types */
#define FT_UNKNOWN 0
#define FT_CONK 1
#define FT_ILBM 2
#define FT_MODULE 3
#define FT_ANIM 4




#define EOTRFILENAMESIZE			10
#define EOTRFULLNAMESIZE			14

/* EdgeOfTheRoad Header - disk/Lame/Ponk format */
struct DiskEOTRHeader
{
	struct Node	Noddy;															/* For Lames use */
	UBYTE				Name[ EOTRFILENAMESIZE ];
	UBYTE				BlocksetName[ 16 ];		/* For Zonks use */
	UWORD				NumOfBlocks;
	ULONG				Reserved;
};
/* Block values follow (UBYTE [NumOfBlocks] ). */
