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
//#include <string.h>
//-------------------------------------
#include <app/Application.h>
#include <interface/Bitmap.h>
#include <storage/File.h>
#include <translation/TranslatorRoster.h>
#include <translation/BitmapStream.h>
//-------------------------------------
#include "gfx/ColorQuant.h"
//-----------------------------------------------------------------------------

int FindBest( const std::vector<rgb_color> &palette, const rgb_color &color )
{
	bitmap_rgb bitrgb = color;

	int bestindex = 0;
	int bestdist = INT_MAX;
	for( uint i=0; i<palette.size(); i++ )
	{
		int dist = bitrgb.RGBDist( palette[i] );
		if( dist < bestdist )
		{
			bestdist = dist;
			bestindex = i;
		}
	}

	return bestindex;
}

int main( int argc, char **argv )
{
	status_t status;
	BApplication app( "application/x-vnd.DamnRednex-ColorQuantTest" );
	
	assert( argc == 5 );
	int dstcolors = atoi( argv[1] );
	int dstcolorbits = atoi( argv[2] );
	const char *srcfilename = argv[3];
	const char *dstfilename = argv[4];

//--

	printf( "Reading %s...\n", srcfilename );

	BFile srcfile( srcfilename, B_READ_ONLY );
	assert( srcfile.InitCheck() >= B_OK );

	BBitmapStream output;
	status = BTranslatorRoster::Default()->Translate( &srcfile, NULL, NULL, &output, B_TRANSLATOR_BITMAP );
	assert( status >= B_OK );

	BBitmap *bitmap;
	output.DetachBitmap( &bitmap );

	assert( bitmap->ColorSpace()==B_RGB32 ||  bitmap->ColorSpace()==B_RGBA32 );

//--

	printf( "Quantisizing...\n" );

	damn::ColorQuant quant( dstcolors, dstcolorbits );
	
	quant.AddColors( bitmap );
	
	std::vector<rgb_color> palette = quant.GetColors();

	printf( "Colors: %ld\n", palette.size() );
	for( uint i=0; i<palette.size(); i++ )
	{
		const rgb_color &rgb = palette[i];
		printf( "%3d: %3d %3d %3d\n", i, rgb.red,rgb.green,rgb.blue );
	}
	
//--

	int width = bitmap->Bounds().IntegerWidth()+1;
	int height = bitmap->Bounds().IntegerHeight()+1;
	for( int iy=0; iy<height; iy++ )
	{
		bitmap_rgb *rgbline = (bitmap_rgb*)BBITMAP32_BITS( bitmap, 0, iy );
		for( int ix=0; ix<width; ix++ )
		{
			const bitmap_rgb &rgb = rgbline[ix];
			int index = FindBest( palette, rgb );
			rgb_color bestrgb = palette[index];
			rgbline[ix] = bestrgb;
		}
	}

//--

	printf( "Writing %s...\n", dstfilename );
	
	BFile dstfile( dstfilename, B_WRITE_ONLY|B_CREATE_FILE|B_ERASE_FILE );
	assert( dstfile.InitCheck() >= B_OK );

	BBitmapStream input( bitmap );
	status = BTranslatorRoster::Default()->Translate( &input, NULL, NULL, &dstfile, 'PNG ' );
	assert( status >= B_OK );
	
//--

	return 0;
}



