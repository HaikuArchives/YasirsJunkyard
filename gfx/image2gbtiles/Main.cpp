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
#include <getopt.h>
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
#include "gfx/BitmapScale.h"
#include "gfx/BitmapUtils.h"
//-----------------------------------------------------------------------------

#define SPLITID( _id ) char(((_id)>>24)&0xff),char(((_id)>>16)&0xff),char(((_id)>>8)&0xff),char(((_id)>>0)&0xff)

void Usage( const char *argv0 )
{
	fprintf( stderr, "Usage: %s [OPTION]... <srcimage> <dstimage>\n", argv0 );
	fprintf( stderr, "\n" );
	fprintf( stderr, "  -h, --help                   display this help and exit\n" );
	fprintf( stderr, "  -p, --palette=col0,col1,col2 sets the grayscale levels\n" );
	fprintf( stderr, "  -v, --verbose                explain what is being done\n" );
	fprintf( stderr, "\n" );
}

int main( int argc, char **argv )
{
	status_t status;
	BApplication app( "application/x-vnd.DamnRednex-Image2GBTiles" );
	
	static struct option long_options[] =
	{
		{"help",		0, 0, 'h'},
        	{"palette",		1, 0, 'p'},
	        {"verbose",		0, 0, 'v'},
	        {NULL,			0, 0, 0}
	};

	bool verbose = false;
	int palette[3] = { 0x40, 0x80, 0xc0 };

	int option;
	int option_index = 0;
	while( (option=getopt_long(argc, argv, "hp:v", long_options, &option_index)) != EOF )
	{
		switch( option )
		{
			case 'h':
				Usage( argv[0] );
				exit( 0 );

			case 'p':
				if( sscanf(optarg,"%d,%d,%d", &palette[0], &palette[1], &palette[2]) != 3 )
				{
					fprintf( stderr, "You must specify 3 colors\n" );
					Usage( argv[0] );
					exit( 1 );
				}
				break;

			case 'v':
				verbose = true;
				break;

			default:
				Usage( argv[0] );
				exit( 1 );  
		}
	}
	
	if( argc-optind != 1 )
	{
		fprintf( stderr, "Too many/few arguments\n" );
		Usage( argv[0] );
		exit( 1 );
	}

	const char *srcfilename = argv[optind++];
//	const char *destfilename = argv[optind++];

//--

	if( verbose ) printf( "Reading %s...\n", srcfilename );

	BFile srcfile( srcfilename, B_READ_ONLY );
	if( srcfile.InitCheck() != B_NO_ERROR )
	{
		fprintf( stderr, "Could not open file %s: %s\n", srcfilename, strerror(srcfile.InitCheck()) );
		exit( 1 );
	}

	// Find the type of the imput file
	translator_info srcinfo;
	status = BTranslatorRoster::Default()->Identify( &srcfile, NULL, &srcinfo );
	if( status != B_NO_ERROR )
	{
		fprintf( stderr, "Could not read image %s: %s\n", srcfilename, strerror(status) );
		exit( 1 );
	}
	
	srcfile.Seek( 0, SEEK_SET );
	
	BBitmapStream output;
	status = BTranslatorRoster::Default()->Translate( &srcfile, &srcinfo, NULL, &output, B_TRANSLATOR_BITMAP );
	if( status != B_NO_ERROR )
	{
		fprintf( stderr, "Could not read image %s: %s\n", srcfilename, strerror(status) );
		exit( 1 );
	}

	BBitmap *srcbitmap;
	output.DetachBitmap( &srcbitmap );

//--

	int srcwidth = srcbitmap->Bounds().IntegerWidth()+1;
	int srcheight = srcbitmap->Bounds().IntegerHeight()+1;

	if( verbose ) printf( "source image: size:%dx%d: colorspace:0x%08X type:%s [%c%c%c%c]\n",
		srcwidth, srcheight, srcbitmap->ColorSpace(), srcinfo.MIME, SPLITID(srcinfo.type) );
		
	if( (srcbitmap->ColorSpace()!=B_RGB32) && (srcbitmap->ColorSpace()!=B_RGBA32) )
	{
		fprintf( stderr, "The image has an unknown colorspace...\n" );
		exit( 1 );
	}
	
//--
	int tileindex = 0;
	for( int iy=0; iy<srcheight; iy+=8 )
	{
		for( int ix=0; ix<srcwidth; ix+=8 )
		{
			uint8 tile[8][8];
			memset(	tile, 0, sizeof(tile) );
			for( int tiy=0; tiy<8; tiy++ )
			{
				bitmap_rgb *bits = (bitmap_rgb*)BBITMAP32_BITS( srcbitmap, ix, iy+tiy );
				for( int tix=0; tix<8; tix++ )
				{
					// FIXME: the border check sould be done outsize...
					if( ix+tix<srcwidth && iy+tiy<srcheight )
					{
						bitmap_rgb color = bits[tix];
						uint8 gray = (color.red*299 + color.green*587 + color.blue*114) / 1000;
						if( gray < palette[0] ) 		gray=0;
						else if( gray < palette[1] )	gray=1;
						else if( gray < palette[2] )	gray=2;
						else 							gray=3;
						tile[tiy][tix] = gray;
					}
				}
			}
			
			fprintf( stdout, "\tdw\t" );
			for( int tiy=0; tiy<8; tiy++ )
			{
				fprintf( stdout, "`" );
				for( int tix=0; tix<8; tix++ )
				{
					fprintf( stdout, "%d", tile[tiy][tix] );
				}
				if( tiy != 7 )
					fprintf( stdout, "," );
			}
			fprintf( stdout, " ; %d (%d,%d)\n", tileindex, ix/8, iy/8 );
			tileindex++;
		}
	}

//--

	return 0;
}



