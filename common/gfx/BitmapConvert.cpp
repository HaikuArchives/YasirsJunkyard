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
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
//-------------------------------------
#include <interface/Bitmap.h>
#include <interface/Screen.h>
#include <interface/View.h>
//-------------------------------------
#include "BitmapUtils.h"
#include "BitmapConvert.h"
//-----------------------------------------------------------------------------    

template<class T> T clamp( T v, T a, T b ) { return v<a?a:v>b?b:v; }

//-----------------------------------------------------------------------------    

BBitmap *ConvertTo32bit( BBitmap *bitmap )
{
//	printf( "CS: %d\n", bitmap->ColorSpace() );
	if( (bitmap->ColorSpace()==B_RGB32) || (bitmap->ColorSpace()==B_RGBA32) )
		return new BBitmap( bitmap );
		
	BBitmap *bitmap_32bit = new BBitmap( bitmap->Bounds(), B_RGBA32, true );

	BView *view = new BView( bitmap->Bounds(), "", 0, 0 );
	bitmap_32bit->AddChild( view );
	
	bitmap_32bit->Lock();
	view->DrawBitmap( bitmap );
	bitmap_32bit->Unlock();
	
	return bitmap_32bit;
}

//-----------------------------------------------------------------------------    

BBitmap *ConvertTo8bit( BBitmap *bitmap )
{
	if( bitmap->ColorSpace() == B_COLOR_8_BIT )
		return new BBitmap( bitmap );

	if( (bitmap->ColorSpace()!=B_RGB32) && (bitmap->ColorSpace()!=B_RGBA32) )
		return NULL;

	BBitmap *bitmap_8bit = new BBitmap( bitmap->Bounds(), B_COLOR_8_BIT, false );
	
	int width = (int)bitmap->Bounds().Width()+1;
	int height = (int)bitmap->Bounds().Height()+1;
	bool srchasalpha = bitmap->ColorSpace()==B_RGBA32;

	BScreen screen;
	
	int *forward = new int[3*width*3];
	memset( forward, 0, 3*width*3*sizeof(int) );
	
	bitmap_rgb *srcbits = (bitmap_rgb*)bitmap->Bits();
	int srcstride = bitmap->BytesPerRow()/sizeof(bitmap_rgb);

	uint8 *dstbits = (uint8*)bitmap_8bit->Bits();
	int dststride = bitmap_8bit->BytesPerRow()/sizeof(uint8);

	for( int iy=0; iy<height; iy++ )
	{
		for( int ix=0; ix<width; ix++ )
		{
			bitmap_rgb rgb = srcbits[ix+iy*srcstride];

			int red = clamp( rgb.red+forward[0+ix*3], 0, 255 );
			int green = clamp( rgb.green+forward[1+ix*3], 0, 255 );
			int blue = clamp( rgb.blue+forward[2+ix*3], 0, 255 );

#if 0
			uint8 close_col = screen.IndexForColor( red, green, blue );
#else
			uint8 close_col = 0;
			uint32 close_err = 0x7fffffff;
			for( int i=0; i<256; i++ )
			{
				rgb_color col = screen.ColorForIndex( i );
				uint32 err = (col.red-red)*(col.red-red)*299 +
					(col.green-green)*(col.green-green)*587 +
					(col.blue-blue)*(col.blue-blue)*114;
				if( err<close_err )
				{
					close_col = i;
					close_err = err;
				}
			}
#endif
			rgb_color close_rgb = screen.ColorForIndex( close_col );
#if 1
			if(  srchasalpha && rgb.alpha<192 )
				close_col = 255;
#endif
			dstbits[ix+iy*dststride] = close_col;
			
			int err[3] = {red-close_rgb.red,green-close_rgb.green,blue-close_rgb.blue};
			for( int i=0; i<3; i++ )
			{
#if 0
				int h = err[i]>>1;
				int e1 = (7*h)>>3;
				int e2 = h-e1;
				h = err[i]-h;
				int e3 = (5*h)>>3;
				int e4 = h-e3;
				
				if( ix<width-1 )
				{
					forward[i+(ix+1)*3]+=e1;
					forward[i+(ix+1)*3+3*width]+=e2;
				}
				forward[i+ix*3+3*width]+=e3; 
				if( ix>0 ) forward[i+(ix-1)*3+3*width]+=e4;
#else
				int e1 = (err[i]*8+21)/42;
				int e2 = (err[i]*4+21)/42;

				int e3 = (err[i]*2+21)/42;
				int e4 = (err[i]*4+21)/42;
				int e5 = (err[i]*8+21)/42;
				int e6 = (err[i]*4+21)/42;
				int e7 = (err[i]*2+21)/42;

				int e8 = (err[i]*1+21)/42;
				int e9 = (err[i]*2+21)/42;
				int e10 = (err[i]*4+21)/42;
				int e11 = (err[i]*2+21)/42;
				int e12 = (err[i]*1+21)/42;
				
				if( ix>1 )
				{
					forward[i+(ix-2)*3+3*width]+=e3;
					forward[i+(ix-2)*3+2*3*width]+=e8;
				}
				if( ix>0 )
				{
					forward[i+(ix-1)*3+3*width]+=e4;
					forward[i+(ix-1)*3+2*3*width]+=e9;
				}
				forward[i+(ix)*3+3*width]+=e5;
				forward[i+(ix)*3+2*3*width]+=e10;
				if( ix<width-1 )
				{
					forward[i+(ix+1)*3]+=e1;
					forward[i+(ix+1)*3+3*width]+=e6;
					forward[i+(ix+1)*3+2*3*width]+=e11;
				}
				if( ix<width-2 )
				{
					forward[i+(ix+1)*3]+=e2;
					forward[i+(ix+1)*3+3*width]+=e7;
					forward[i+(ix+1)*3+2*3*width]+=e12;
				}
				
#endif
			}
			
//			uint8 col = screen.IndexForColor( rgb.red, rgb.green, rgb.blue );
//			dstbits[ix+iy*dststride] = col;
		}
		
		memmove( forward, forward+3*width, 3*width*2*sizeof(int) );
		memset( forward+3*width*2, 0, width*3*sizeof(int) );
	}

	delete forward;
	return bitmap_8bit;
}

//-----------------------------------------------------------------------------    

