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
#include <math.h>
//#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
//-------------------------------------
#include <app/Application.h>
#include <interface/Bitmap.h>
#include <storage/File.h>
#include <translation/TranslatorRoster.h>
#include <translation/BitmapStream.h>
//-------------------------------------
#include "text/Bitmap2Text.h"
#include "gfx/BitmapScale.h"
//-----------------------------------------------------------------------------

BBitmap *ReadBitmap( const char *filename )
{
	BFile file( filename, B_READ_ONLY );
	if( file.InitCheck() != B_NO_ERROR )
	{
		fprintf( stderr, "Could not open file %s: %s\n", filename, strerror(file.InitCheck()) );
		return NULL;
	}

	BBitmapStream output;
	status_t status = BTranslatorRoster::Default()->Translate( &file, NULL, NULL, &output, B_TRANSLATOR_BITMAP );
	if( status != B_NO_ERROR )
	{
		fprintf( stderr, "Could not read image %s: %s\n", filename, strerror(status) );
		return NULL;
	}

	BBitmap *bitmap;
	output.DetachBitmap( &bitmap );
	
	return bitmap;
}

#define RANGE(_v,_l,_h) ((_v)<(_l)?(_l):(_v)>(_h)?(_h):(_v))
#define GAMMA(_col) int(RANGE((pow(float(_col)/255.0f,gamma)+brightness)*contrast,0.0f,1.0f)*255.0f)
#define TRUNCCOL(_col) (((_col)&0xf0) | ((_col)>>4)) // for better compression

int main( int argc, char **argv )
{
//	status_t status;
	BApplication app( "application/x-vnd.DamnRednex-Bitmap2Text" );

	assert( argc == 7 );
	
	const char *bitmapfile = argv[1];
	int width = atoi( argv[2] );
	int height = atoi( argv[3] );
	float brightness = atof( argv[4] );
	float contrast = atof( argv[5] );
	float gamma = atof( argv[6] );

	BBitmap *bitmap = ReadBitmap( bitmapfile );
	assert( bitmap != NULL );

	BBitmap dstbitmap( BRect(0,0,width-1,height-1), B_RGB32 );
	damn::Scale( bitmap, &dstbitmap, damn::filter_mitchell );

//	struct colchar_t { char c; /*int ansicol;*/ rgb_color rgbcol; };
	std::vector<std::vector<damn::colchar_t> > text = damn::Bitmap2Text( &dstbitmap );
	
	printf( "<html><body bgcolor=\"#000000\">\n" );
//	printf( "<table bgcolor=\"#000000\"><tr><td>\n" );
	printf( "<pre><font size=\"-2\">\n" );
	
	rgb_color lastcol = { 255,255,255,255 };
	
	for( int iy=0; iy<height; iy++ )
	{
		bool colinit = false;
		for( int ix=0; ix<width; ix++ )
		{
			damn::colchar_t cc = text[iy][ix];
			cc.rgbcol.red = TRUNCCOL(GAMMA(cc.rgbcol.red));
			cc.rgbcol.green = TRUNCCOL(GAMMA(cc.rgbcol.green));
			cc.rgbcol.blue = TRUNCCOL(GAMMA(cc.rgbcol.blue));
			if( !colinit || memcmp(&lastcol,&cc.rgbcol,sizeof(lastcol))!=0 )
			{
				if( colinit )
					printf( "</font>" );
				else
					colinit=true;
				printf( "<font color=\"#%02X%02X%02X\">", cc.rgbcol.red, cc.rgbcol.green, cc.rgbcol.blue );
				lastcol = cc.rgbcol;
			}
			printf( "%c", cc.c );
		}
		if( colinit )
			printf( "</font>" );
		printf( "\n" );
	}

	printf( "</font></pre>\n" );
//	printf( "</td></tr></table>\n" );
	printf( "</body></html>\n" );


	return 0;
}

