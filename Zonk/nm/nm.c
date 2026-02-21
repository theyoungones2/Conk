#include <stdio.h>
#include <strings.h>

#include <dos/dos.h>
#include <proto/dos.h>

BOOL __asm MakePackable( register __a0 UBYTE *str );

VOID __asm PackASCII( register __a0 UBYTE *source, register __a1 UBYTE *dest,
	register __d0 UWORD numchars );

#define NAMESIZE 14
#define NAMEPACKSIZE 10

int main( VOID )
{
	struct RDArgs *rdargs;
	LONG argarray[1];
	UBYTE outbuf[512];
	UBYTE inbuf[512];
	UWORD i;

	if( rdargs = ReadArgs( "NAME/A", argarray, NULL ) )
	{

		strcpy( inbuf, (STRPTR)argarray[0]);
		if( MakePackable( inbuf ) )
			printf("Filtered some crap...\n");
		PackASCII( inbuf, outbuf, NAMESIZE );
		for( i=0; i<NAMEPACKSIZE; i++ )
			printf("%d.b,",outbuf[i] );
		printf("\n");
		FreeArgs( rdargs );
	}
	else
		printf("Invalid parameters!\n");

	return( 0 );
}
