/* ATan table generator for Ponk */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define NUMSPERLINE 16

void main( void )
{
	double theta;
	int i,fixed;
	int numcount=0;

	for( i=0; i<256; i++ )
	{
		theta = atan( (double)i / 256.0 );
		fixed = (theta * 256.0 )/( 2.0 * PI );

		printf("%d", fixed );
		if( numcount++ >= NUMSPERLINE-1 )
		{
			printf("\n");
			numcount = 0;
		}
		else
			printf(",");
	}
}
