/*
 * Copyright (c) 1999, Jesper Hansen. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither name of the company nor the names of its contributors may
 *    be used to endorse or promote products derived from this software
 *    without specific prior written permission. 
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE REGENTS OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

//-----------------------------------------------------------------------------
#include <alloca.h>
#include <math.h>
#include <string.h>
//-------------------------------------
#include <support/Debug.h>
//-------------------------------------
#include "../Core.h"
#include "../Internal.h"
//-----------------------------------------------------------------------------

/** 
 * \ingroup DLE
 * DLE utility function.
 */
dle::MinMax1 dle::SpreadCalcSize( const float *min, const float *max, int count )
{
	ASSERT( min != NULL );
	ASSERT( max != NULL );
	ASSERT( count >= 0 );

	dle::MinMax1 size( 0, 0 );

	for( int i=0; i<count; i++ )
	{
		size.min += ceil( min[i] );
		size.max  += ceil( max[i] );
	}

	return size;
}

//-----------------------------------------------------------------------------

void DumpSpreadValues( const float *min, const float *max, float *dstsize, const float *weight, int count, float totsize )
{
    fprintf( stderr, "Count: %d TotalSize: %f\n", count, totsize );
    for( int i=0; i<count; i++ )
	fprintf( stderr, "%d: %f %f %f %f\n", i, min[i], max[i], dstsize[i], weight?weight[i]:1.0f );
}

void __AssertSpreadValues( const char *expr, const char *desc, const char *file, int line,
			   const float *min, const float *max, float *dstsize, const float *weight, int count, float totsize )
{
    fprintf( stderr, "dle::Spread assertion: %s:%d: %s (%s)\n", file, line, expr, desc );
    DumpSpreadValues( min, max, dstsize, weight, count, totsize );
//    abort();
	DEBUGGER( desc );
}

#define AssertSpreadValues( expr, desc, min, max, dstsize, weight, count, totsize ) \
((void)((expr) ? 0 : (__AssertSpreadValues(#expr, desc, __FILE__, __LINE__, min, max, dstsize, weight, count, totsize),0)))


/** DLE utility function.
 * \ingroup DLE
 *
 */
void dle::Spread( const float *min, const float *max, float *dstsize, const float *weight, int count, float totsize )
{
	ASSERT( dstsize != NULL );
	ASSERT( min != NULL );
	ASSERT( max != NULL );
	ASSERT( count >= 0 );
	ASSERT_WITH_MESSAGE( totsize>=0, "huh" );
	ASSERT( floor(totsize) == totsize );

	float minsize = 0;
	float maxsize = 0;
	float totweight = 0;

	for( int i=0; i<count; i++ )
	{
//		printf( "dle::Spread(): %d: min:%f max:%f\n", i, min[i], max[i] );
		AssertSpreadValues( min[i]<=max[i], "max should be larger than min", min, max, dstsize, weight, count, totsize );

		ASSERT( floor(min[i]) == min[i] );
		ASSERT( floor(max[i]) == max[i] );

		minsize += min[i];
		maxsize += max[i];
		totweight += weight ? weight[i] : 1;
	}
	
//	printf(" dle::Spread(): minsize:%f maxsize:%f totsize:%f\n", minsize, maxsize, totsize );
	ASSERT_WITH_MESSAGE( maxsize>=totsize, "The container is too big" );
	AssertSpreadValues( minsize<=totsize, "Not enough room in the container", min, max, dstsize, weight, count, totsize );
	ASSERT_WITH_MESSAGE( totweight>0, "The summed weight if the container elements must be positive" );

	uint8 *done = (uint8*)alloca(count);
	int   donecnt = 0;
	memset( done, 0, count );
	float totextra = totsize-minsize;
	while( 1 )
	{
		float extra = totextra;
		float extraunused = 0;

		int i;
		for( i=0; i<count; i++ )
		{
			if( !done[i] )
			{
				float relweight = weight ? weight[i]/totweight : 1.0f/float(count-donecnt);
				float gotunused = extra*relweight + extraunused;
//				printf( "%d: relweight:%f gotunused:%f\n", i, relweight, gotunused );
				extraunused = gotunused - floor(gotunused+0.5);
				gotunused = floor(gotunused+0.5);
		
				dstsize[i] = min[i] + gotunused;
				if( dstsize[i] > max[i] )
				{
					dstsize[i] = max[i];
					done[i] = 1;
					donecnt++;
					totweight -= weight ? weight[i] : 0.0f;
					totextra -= dstsize[i]-min[i];
					break;
				}
//				printf( "%d: %f\n", i, dstsize[i] );
			}
		}
		if( i==count )
			break;
	}
}

//-----------------------------------------------------------------------------

/** Fit/align a BRect into a MinMax2.
 * If the \a fitrect is smaller than the minimum size specifies by \a minmax
 *  it will assert.<br>
 * If it's larger that the max size specified by \a minmax the retuned BRect
 *  will get the max allowed size (as specified by \a minmax) and it's then
 *  aligned according to \align.
 *
 * \ingroup DLE
 * \internal
 */
BRect dle::AlignRect( const BRect &fitrect, const MinMax2 &minmax, align_t align )
{
	ASSERT( minmax.horz.min >= 0 );
	ASSERT( minmax.vert.min >= 0 );
	ASSERT( minmax.horz.max >= minmax.horz.min );
	ASSERT( minmax.vert.max >= minmax.vert.min );

	ASSERT_WITH_MESSAGE( fitrect.Width()+1>=minmax.horz.min, "internal layout error: The alingment rect is not wide enough to fit the object" );
	ASSERT_WITH_MESSAGE( fitrect.Height()+1>=minmax.vert.min, "internal layout error: The alingment rect is not high enough to fit the object" );

//	assert( fitrect.Width()+1 <= minmax.hmax );
//	assert( fitrect.Height()+1 <= minmax.vmax );

	ASSERT_WITH_MESSAGE( (align&HMASK)==LEFT || (align&HMASK)==HCENTER || (align&HMASK)==RIGHT, "internal layout error: unknown horizontal align value" );
	ASSERT_WITH_MESSAGE( (align&VMASK)==TOP || (align&VMASK)==VCENTER || (align&VMASK)==BOTTOM, "internal layout error: unknown vertical align value" );


	BRect rect;

	if( fitrect.Width()+1 > minmax.horz.max )
	{
		if( (align&HMASK)==LEFT )			rect.left = 0;
		else if( (align&HMASK)==HCENTER )	rect.left = floor((fitrect.Width()+1-minmax.horz.max)/2);
		else if( (align&HMASK)==RIGHT )		rect.left = fitrect.Width()+1-minmax.horz.max;
		rect.left += fitrect.left;
		rect.right = rect.left+minmax.horz.max-1;
	}
	else
	{
		rect.left = fitrect.left;
		rect.right = fitrect.right;
	}

	if( fitrect.Height()+1 > minmax.vert.max )
	{
		if( (align&VMASK)==TOP )			rect.top = 0;
		else if( (align&VMASK)==VCENTER )	rect.top = floor((fitrect.Height()+1-minmax.vert.max)/2);
		else if( (align&VMASK)==BOTTOM )	rect.top = fitrect.Height()+1-minmax.vert.max;
		rect.top += fitrect.top;
		rect.bottom = rect.top+minmax.vert.max-1;
	}
	else
	{
		rect.top = fitrect.top;
		rect.bottom = fitrect.bottom;
	}
	
//	rect.PrintToStream();
	return rect;
}

//-----------------------------------------------------------------------------

