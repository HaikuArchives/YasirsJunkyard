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
#include <string.h>
//-------------------------------------
#include <support/Debug.h>
//-------------------------------------
#include "../DamnLayoutEngine.h"
//-----------------------------------------------------------------------------

damn::MinMax1 damn::SpreadCalcSize( const float *min, const float *max, int count )
{
	assert( min != NULL );
	assert( max != NULL );
	assert( count >= 0 );

	damn::MinMax1 size( 0, 0 );

	for( int i=0; i<count; i++ )
	{
		size.min += ceil( min[i] );
		size.max  += ceil( max[i] );
	}

	return size;
}

//-----------------------------------------------------------------------------

void damn::Spread( float *min, float *max, float *dstsize, float *weight, int count, float totsize )
{
	assert( dstsize != NULL );
	assert( min != NULL );
	assert( max != NULL );
	assert( count >= 0 );
	assert( totsize >= 0 );
	assert( floor(totsize) == totsize );

	float minsize = 0;
	float maxsize = 0;
	float totweight = 0;

	for( int i=0; i<count; i++ )
	{
//		printf( "damn::Spread(): %d: min:%f max:%f\n", i, min[i], max[i] );
		assert( min[i]<=max[i] );
		assert( floor(min[i]) == min[i] );
		assert( floor(max[i]) == max[i] );

		minsize += min[i];
		maxsize += max[i];
		totweight += weight ? weight[i] : 1;
	}
	
//	printf(" damn::Spread(): minsize:%f maxsize:%f totsize:%f\n", minsize, maxsize, totsize );
	ASSERT_WITH_MESSAGE( maxsize>=totsize, "Not enough room in the container" );
	ASSERT_WITH_MESSAGE( minsize<=totsize, "The container is too big" );
	assert( totweight>0 );

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

BRect damn::AlignRect( const BRect &fitrect, const MinMax2 &minmax, align_t align )
{
	assert( minmax.hmin >= 0 );
	assert( minmax.vmin >= 0 );
	assert( minmax.hmax >= minmax.hmin );
	assert( minmax.vmax >= minmax.vmin );
	assert( fitrect.Width()+1 >= minmax.hmin );
//	assert( fitrect.Width()+1 <= minmax.hmax );
	assert( fitrect.Height()+1 >= minmax.vmin );
//	assert( fitrect.Height()+1 <= minmax.vmax );
	assert( (align&HMASK)==LEFT || (align&HMASK)==HCENTER || (align&HMASK)==RIGHT );
	assert( (align&VMASK)==TOP || (align&VMASK)==VCENTER || (align&VMASK)==BOTTOM );


	BRect rect;

	if( fitrect.Width()+1 > minmax.hmax )
	{
		if( (align&HMASK)==LEFT )			rect.left = 0;
		else if( (align&HMASK)==HCENTER )	rect.left = floor((fitrect.Width()+1-minmax.hmax)/2);
		else if( (align&HMASK)==RIGHT )		rect.left = fitrect.Width()+1-minmax.hmax;
		rect.left += fitrect.left;
		rect.right = rect.left+minmax.hmax-1;
	}
	else
	{
		rect.left = fitrect.left;
		rect.right = fitrect.right;
	}

	if( fitrect.Height()+1 > minmax.vmax )
	{
		if( (align&VMASK)==TOP )			rect.top = 0;
		else if( (align&VMASK)==VCENTER )	rect.top = floor((fitrect.Height()+1-minmax.vmax)/2);
		else if( (align&VMASK)==BOTTOM )	rect.top = fitrect.Height()+1-minmax.vmax;
		rect.top += fitrect.top;
		rect.bottom = rect.top+minmax.vmax-1;
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

