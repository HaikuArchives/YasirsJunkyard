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
//#include <assert.h>
//#include <math.h>
//#include <stdio.h>
//-------------------------------------
#include <interface/Bitmap.h>
#include <support/ByteOrder.h>
//#include <support/SupportDefs.h>
//-------------------------------------
#include "Bitmap2Text.h"
//#include "gfx/BitmapScale.h"
//-----------------------------------------------------------------------------

static const char chars[] = ".:-+=osOS@";

#if 0
std::vector<std::vector<damn::colchar_t> > damn::Bitmap2Text( const BBitmap *bitmap, int width, int height )
{
	BBitmap dstbitmap( BRect(0,0,width-1,height-1), B_RGB32 );
	damn::Scale( bitmap, &dstbitmap, damn::filter_box );

	std::vector<std::vector<damn::colchar_t> > text;
	
	for( int iy=0; iy<height; iy++ )
	{
		text.push_back( std::vector<damn::colchar_t>() );

		uint32 *dstrow = (uint32*)((uint8*)dstbitmap.Bits()+iy*dstbitmap.BytesPerRow());
		for( int ix=0; ix<width; ix++ )
		{
			uint32 col = B_LENDIAN_TO_HOST_INT32( dstrow[ix] );
			int r = (col>>8)&0xff;
			int g = (col>>16)&0xff;
			int b = (col>>24)&0xff;
			
			int gray = (r*299 + g*587 + b*114) / 1000;

			colchar_t cc;
			cc.c = chars[gray*(sizeof(chars))/256];
			cc.rgbcol.red = r;
			cc.rgbcol.green = g;
			cc.rgbcol.blue = b;
			cc.rgbcol.alpha = 255;
			
			text[iy].push_back( cc );
		}
	}
	
	return text;
}
#endif

std::vector<std::vector<damn::colchar_t> > damn::Bitmap2Text( const BBitmap *bitmap )
{
	int width = bitmap->Bounds().IntegerWidth()+1;
	int height = bitmap->Bounds().IntegerHeight()+1;

	std::vector<std::vector<damn::colchar_t> > text;
	
	for( int iy=0; iy<height; iy++ )
	{
		text.push_back( std::vector<damn::colchar_t>() );

		const uint32 *dstrow = (const uint32*)((const uint8*)bitmap->Bits()+iy*bitmap->BytesPerRow());
		for( int ix=0; ix<width; ix++ )
		{
			uint32 col = B_BENDIAN_TO_HOST_INT32( dstrow[ix] );
			int r = (col>>8)&0xff;
			int g = (col>>16)&0xff;
			int b = (col>>24)&0xff;
			
			int gray = (r*299 + g*587 + b*114) / 1000;

			colchar_t cc;
			cc.c = chars[gray*(sizeof(chars)-1)/256];
			cc.rgbcol.red = r;
			cc.rgbcol.green = g;
			cc.rgbcol.blue = b;
			cc.rgbcol.alpha = 255;
			
			text[iy].push_back( cc );
		}
	}
	
	return text;
}

//-----------------------------------------------------------------------------


