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

int main( int argc, char **argv )
{
//	status_t status;
	BApplication app( "application/x-vnd.DamnRednex-Bitmap2Text" );

	assert( argc == 4 );
	
	const char *bitmapfile = argv[1];
	int width = atoi( argv[2] );
	int height = atoi( argv[3] );

	BBitmap *bitmap = ReadBitmap( bitmapfile );
	assert( bitmap != NULL );

	BBitmap dstbitmap( BRect(0,0,width-1,height-1), B_RGB32 );
	damn::Scale( bitmap, &dstbitmap, damn::filter_mitchell );

//	struct colchar_t { char c; /*int ansicol;*/ rgb_color rgbcol; };
	std::vector<std::vector<damn::colchar_t> > text = damn::Bitmap2Text( &dstbitmap );
	
	printf( "<html>\n" );
	printf( "<table bgcolor=#0000><tr><td>" );
	printf( "<pre><font size=-2>" );
	
	for( int iy=0; iy<height; iy++ )
	{
		for( int ix=0; ix<width; ix++ )
		{
			damn::colchar_t cc = text[iy][ix];
//			printf( "%c", cc.c );
			printf( "<font color=\"#%02X%02X%02X\">%c</font>",
				cc.rgbcol.red, cc.rgbcol.green, cc.rgbcol.blue, cc.c );
		}
		printf( "\n" );
	}

	printf( "</font></pre>" );
	printf( "</td></tr></table>" );
	printf( "</html>\n" );


	return 0;
}

