
#define FILES_C

#include <stdio.h>
#include <string.h>
#include <stdarg.h>

#include <dos/dos.h>
#include <dos/dostags.h>
#include <exec/exec.h>
#include <intuition/intuition.h>
#include <graphics/gfxbase.h>
#include <libraries/gadtools.h>
#include <libraries/diskfont.h>
#include <libraries/asl.h>
#include <libraries/iffparse.h>

#include <proto/exec.h>
#include <proto/dos.h>
#include <proto/intuition.h>
#include <proto/graphics.h>
#include <proto/diskfont.h>
#include <proto/asl.h>
#include <clib/alib_protos.h>
#include <proto/gadtools.h>
#include <proto/iffparse.h>

#include <global.h>

void LoadFile( STRPTR drawer, STRPTR file )
{
	char namebuf[512];
	BPTR fh;
	struct IFFHandle *iff;
	LONG ifferror = 0;
	UWORD i;
	struct ContextNode *cn;
	struct Filetracker *ft;
	struct Chunk *chunk;
	ULONG idarray[] = { ID_MAP, ID_BLKS, ID_BOBS, ID_ANIM, ID_PROG, ID_CMAP,
		ID_DISP, ID_PLYR, ID_DFRM, ID_WEAP, ID_TWAV, ID_SFX, ID_THAP,
		ID_ZCFG, ID_SWAV, ID_SPRT, ID_GAME, ID_EOTR, 0L };

	D(bug("LoadFile(), %s %s\n", drawer, file ) );

	if( ft = AllocVec( sizeof( struct Filetracker ), MEMF_ANY|MEMF_CLEAR ) )
	{
		Mystrncpy( ft->ft_Drawer, drawer, 256-1 );
		Mystrncpy( ft->ft_File, file, 256-1 );

		Mystrncpy( namebuf, drawer, 512-1 );
		if( AddPart( namebuf, file, 512) )
		{
			if( fh = Open( namebuf, MODE_OLDFILE ) )
			{
				if( iff = AllocIFF() )
				{
					iff->iff_Stream = fh;
					InitIFFasDOS( iff );
					OpenIFF( iff, IFFF_READ );

					for( i=0; idarray[i] && !ifferror; i++ )
					{
						ifferror = StopChunk( iff, ID_CONK, idarray[i] );
					}
					if( !ifferror )
					{
						do
						{
							ifferror = ParseIFF( iff, IFFPARSE_SCAN );
							if( !ifferror )
							{
								if( cn = CurrentChunk( iff ) )
								{

									/* call specific chunkloader */
									switch( cn->cn_ID )
									{
										case ID_PROG:
											chunk = PROGLoader( iff, cn->cn_Size, ft );
											break;
										case ID_DFRM:
											chunk = DFRMLoader( iff, cn->cn_Size, ft );
											break;
										case ID_CMAP:
											chunk = CMAPLoader( iff, cn->cn_Size, ft );
											break;
										case ID_WEAP:
											chunk = WEAPLoader( iff, cn->cn_Size, ft );
											break;
										case ID_TWAV:
											chunk = TWAVLoader( iff, cn->cn_Size, ft );
											break;
										case ID_MAP:
											chunk = MAPLoader( iff, cn->cn_Size, ft );
											break;
										case ID_BLKS:
											chunk = BLKSLoader( iff, cn->cn_Size, ft );
											break;
										case ID_SFX:
											chunk = SFXLoader( iff, cn->cn_Size, ft );
											break;
										case ID_THAP:
											chunk = THAPLoader( iff, cn->cn_Size, ft );
											break;
										case ID_BOBS:
											chunk = BOBSLoader( iff, cn->cn_Size, ft );
											break;
										case ID_ANIM:
											chunk = ANIMLoader( iff, cn->cn_Size, ft );
											break;
										case ID_ZCFG:
											chunk = ZCFGLoader( iff, cn->cn_Size, ft );
											break;
										case ID_SWAV:
											chunk = SWAVLoader( iff, cn->cn_Size, ft );
											break;
										case ID_SPRT:
											chunk = SPRTLoader( iff, cn->cn_Size, ft );
											break;
										case ID_GAME:
											chunk = GAMELoader( iff, cn->cn_Size, ft );
											break;
										case ID_DISP:
											chunk = DISPLoader( iff, cn->cn_Size, ft );
											break;
										case ID_EOTR:
											chunk = EOTRLoader( iff, cn->cn_Size, ft );
											break;
										default:
											chunk = GenericChunkLoader( iff, cn->cn_Size, ft, cn->cn_ID );
									}

									if( chunk )
									{
										/* Chunk loaded ok, add it to the chunklist */
										AddTail( &chunklist, (struct Node *)chunk );
									}
								}
							}
						} while( !ifferror );
					}
					CloseIFF( iff );
					FreeIFF( iff );
				}
				Close( fh );
			}
		}

		if( ft->ft_ChunkCount )
		{
			/* yep, we loaded some chunks, so remember this file */
			ft->ft_OriginalChunkCount = ft->ft_ChunkCount;
			AddTail( &filelist, (struct Node *)ft );
		}
		else
		{
			/* no chunks loaded */
			FreeVec( ft );
		}
	}
}



BOOL SaveFile( STRPTR drawer, STRPTR file, struct Chunk **cnkarray, UWORD count )
{
	struct IFFHandle *iff;
	struct Chunk *cnk;
	BOOL allok;
	struct Hook myhook = { {NULL}, (ULONG (*)())MyBufferedDOSHook, NULL, NULL };
	struct Filetracker *ft;
	char namebuf[512];

	allok = TRUE;

	Mystrncpy( namebuf, drawer, 512-1 );
	if( AddPart( namebuf, file, 512) )
	{
		if( ft = AllocVec( sizeof( struct Filetracker ), MEMF_ANY|MEMF_CLEAR ) )
		{
			Mystrncpy( ft->ft_Drawer, drawer, 256-1 );
			Mystrncpy( ft->ft_File, file, 256-1 );

			if( iff = AllocIFF() )
			{
				if( iff->iff_Stream = Open( namebuf, MODE_NEWFILE ) )
				{
//					InitIFFasDOS( iff );
					InitIFF( iff, IFFF_FSEEK, &myhook );
					if( !OpenIFF( iff, IFFF_WRITE ) )
					{
						if( !PushChunk( iff, ID_CONK, ID_FORM, IFFSIZE_UNKNOWN ) )
						{
							while( count-- && allok )
							{
								cnk = *cnkarray++;

								/* remove from old file, if needed */
								if( cnk->ch_ParentFile )
									RemoveChunkFromParentFile( cnk );

								/* add to new file */
								if( ft )
									ft->ft_ChunkCount++;
								cnk->ch_ParentFile = ft;

								allok = (*cnk->ch_Saver)( cnk, iff );		/* ignoring errors... */
								if( allok )
									cnk->ch_Modified = FALSE;

							}
							PopChunk( iff );
						}
						CloseIFF( iff );
					}
					Close( iff->iff_Stream );
				}
				FreeIFF( iff );
			}

			if( ft->ft_ChunkCount )
			{
				ft->ft_OriginalChunkCount = ft->ft_ChunkCount;
				AddTail( &filelist, (struct Node *)ft );
			}
			else
				FreeVec( ft );

		}
	}
	return( allok );
}




LONG __saveds __asm MyBufferedDOSHook( register __a0 struct Hook *hook,
	register __a2 struct IFFHandle *iff,
	register __a1 struct IFFStreamCmd *actionpkt )
{
	register BPTR fh;
	register LONG nbytes, error;
	register UBYTE *buf;

	fh = (BPTR)iff->iff_Stream;
	nbytes = actionpkt->sc_NBytes;
	buf = (UBYTE *)actionpkt->sc_Buf;

	switch( actionpkt->sc_Command )
	{
		case IFFCMD_READ:
			error = !( FRead( fh, buf, nbytes, 1 ) == 1 );
			break;
		case IFFCMD_WRITE:
			error = !( FWrite( fh, buf, nbytes, 1 ) == 1 );
			break;
		case IFFCMD_SEEK:
			Flush( fh );
			error = !(Seek( fh, nbytes, OFFSET_CURRENT ) == 0 );
			break;
		case IFFCMD_INIT:
		case IFFCMD_CLEANUP:
			error = 0;
			break;
	}
	return( error );
}



WORD IDFile( STRPTR filename )
{
	BPTR fh;
	WORD type;
	ULONG buf[3];

	type = FT_UNKNOWN;
	if( fh = Open( filename, MODE_OLDFILE ) )
	{
		if( FRead( fh, &buf, 4, 3 ) )
		{
			if( buf[0] == ID_FORM )
			{
				switch( buf[2] )
				{
					case ID_CONK:
						type = FT_CONK;
						break;
					case ID_ILBM:
						type = FT_ILBM;
						break;
					case ID_ANIM:
						type = FT_ANIM;
						break;
				}
			}
		}

		Close( fh );
	}
	return( type );
}



BOOL LoadPalette( STRPTR filename, struct Palette *palette )
{
	struct IFFHandle	*iff;
	BOOL							success = FALSE;
	int								size;

	if( iff = AllocIFF() )
	{
		if( iff->iff_Stream = Open( filename,MODE_OLDFILE ) )
		{
			InitIFFasDOS( iff );

			if( !OpenIFF( iff, IFFF_READ ) )
			{
				if( !StopChunk( iff, ID_ILBM, ID_CMAP ) &&
					!StopChunk( iff, ID_CONK, ID_CMAP ) )
				{
					/* Parse the file... */

					if( !ParseIFF( iff, IFFPARSE_SCAN ) )
					{
						struct ContextNode *Chunk = CurrentChunk( iff );

						size = Chunk -> cn_Size;
						palette->pl_Count = size/3;

						if(ReadChunkBytes( iff, palette->pl_Colours, size ) == size)
							success = TRUE;
					}
				}
				CloseIFF( iff );
			}
			Close( iff->iff_Stream );
		}
		FreeIFF( iff );
	}
	return success;
}
