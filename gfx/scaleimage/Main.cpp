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
//-----------------------------------------------------------------------------

#define SPLITID( _id ) char(((_id)>>24)&0xff),char(((_id)>>16)&0xff),char(((_id)>>8)&0xff),char(((_id)>>0)&0xff)

void Usage( const char *argv0 )
{
	fprintf( stderr, "Usage: %s [OPTION]... <srcimage> <dstimage>\n", argv0 );
	fprintf( stderr, "\n" );
	fprintf( stderr, "  -h, --help               display this help and exit\n" );
	fprintf( stderr, "  -x, --xsize=WIDTH        scale image to 'WIDTH'\n" );
	fprintf( stderr, "  -y, --ysize=HEIGHT       scale image to 'HEIGHT'\n" );
	fprintf( stderr, "  -t, --outformat=FORMAT   format of dstimage, must be 4 character type\n" );
	fprintf( stderr, "  -f, --filter=FILTER      select filter type, must be one of:'\n" );
	fprintf( stderr, "                             point, box, triangle, bell, bspline, \n" );
	fprintf( stderr, "                             catrom, gaussian, sinc, bessel,\n" );
	fprintf( stderr, "                             mitchell, hanning, hamming, blackman, \n" );
	fprintf( stderr, "                             kaiser, normal, filter or lanczos3\n" );
	fprintf( stderr, "                             bspline, lanczos3 or mitchell\n" );
	fprintf( stderr, "  -w, --filterwidth=WIDTH  Sets the filterwidth\n" );
	fprintf( stderr, "  -v, --verbose            explain what is being done\n" );
	fprintf( stderr, "\n" );
}

int main( int argc, char **argv )
{
	status_t status;
	BApplication app( "application/x-vnd.DamnRednex-ScaleImage" );
	
	static struct option long_options[] = {
		{"help",		0, 0, 'h'},
        {"xsize",		1, 0, 'x'},
        {"ysize",		1, 0, 'y'},
        {"outformat",	1, 0, 't'},
        {"filter",		1, 0, 'f'},
        {"verbose",		0, 0, 'v'},
        {NULL,			0, 0, 0}};

	int destwidth = -1;
	int destheight = -1;
	bitmapscale_filtertype filter = filter_lanczos3;
	bool verbose = false;
	uint32 desttype = 0;
	float filterwidth = 0.0f;

    int option;
    int option_index = 0;
    while( (option=getopt_long(argc, argv, "hx:y:t:f:w:v", long_options, &option_index)) != EOF )
    {
        switch( option )
        {
            case 'h':
                Usage( argv[0] );
                exit( 0 );

            case 'x':
            	destwidth = atoi( optarg );
                if( destwidth <= 0 )
				{
					fprintf( stderr, "Illegal width: %d\n", destwidth );
					Usage( argv[0] );
					exit( 1 );
				}
                break;

            case 'y':
            	destheight = atoi( optarg );
                if( destheight <= 0 )
				{
					fprintf( stderr, "Illegal height: %d\n", destheight );
					Usage( argv[0] );
					exit( 1 );
				}
				break;

			case 't':
				if( strlen(optarg) != 4 )
				{
					fprintf( stderr, "Illegal type: %s\n", optarg );
					Usage( argv[0] );
					exit( 1 );
				}
				desttype = (optarg[0]<<24) | (optarg[1]<<16) | (optarg[2]<<8) | optarg[3];
				break;

            case 'f':
				if( strcasecmp(optarg,"point") == 0 )			filter = filter_point;
				else if( strcasecmp(optarg,"box") == 0 )		filter = filter_box;
				else if( strcasecmp(optarg,"triangle") == 0 )	filter = filter_triangle;
				else if( strcasecmp(optarg,"bell") == 0 )		filter = filter_bell;
				else if( strcasecmp(optarg,"bspline") == 0 )	filter = filter_bspline;
				else if( strcasecmp(optarg,"catrom") == 0 )		filter = filter_catrom;
				else if( strcasecmp(optarg,"gaussian") == 0 )	filter = filter_gaussian;
				else if( strcasecmp(optarg,"sinc") == 0 )		filter = filter_sinc;
				else if( strcasecmp(optarg,"bessel") == 0 )		filter = filter_bessel;
				else if( strcasecmp(optarg,"mitchell") == 0 )	filter = filter_mitchell;
				else if( strcasecmp(optarg,"hanning") == 0 )	filter = filter_hanning;
				else if( strcasecmp(optarg,"hamming") == 0 )	filter = filter_hamming;
				else if( strcasecmp(optarg,"blackman") == 0 )	filter = filter_blackman;
				else if( strcasecmp(optarg,"kaiser") == 0 )		filter = filter_kaiser;
				else if( strcasecmp(optarg,"normal") == 0 )		filter = filter_normal;
				else if( strcasecmp(optarg,"filter") == 0 )		filter = filter_filter;
				else if( strcasecmp(optarg,"lanczos3") == 0 )	filter = filter_lanczos3;
				else
				{
					fprintf( stderr, "Illegal filter: %s\n", optarg );
					Usage( argv[0] );
	                exit( 1 );
				}
				break;
				
			case 'w':
				filterwidth = atof( optarg );
                if( filterwidth < 0.0f )
				{
					fprintf( stderr, "Illegal filter width: %f\n", filterwidth );
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
	
	if( argc-optind != 2 )
    {
        fprintf( stderr, "Too many/few arguments\n" );
		Usage( argv[0] );
		exit( 1 );
    }                   

	const char *srcfilename = argv[optind++];
	const char *destfilename = argv[optind++];

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
	float srcaspect = float(srcwidth) / float(srcheight);

	if( verbose ) printf( "source image: size:%dx%d: colorspace:0x%08X type:%s [%c%c%c%c]\n",
		srcwidth, srcheight, srcbitmap->ColorSpace(), srcinfo.MIME, SPLITID(srcinfo.type) );
		
	if( (srcbitmap->ColorSpace()!=B_RGB32) && (srcbitmap->ColorSpace()!=B_RGBA32) )
	{
		fprintf( stderr, "The image has an unknown colorspace...\n" );
		exit( 1 );
	}
	
	if( destwidth==-1 && destheight==-1 )
	{
		destwidth = srcwidth;
		destheight = srcheight;
	}
	else if( destwidth == -1 )
	{
		destwidth = int(floor(float(destheight) * srcaspect + 0.5f));
	}
	else if( destheight == -1 )
	{
		destheight = int(floor(float(destwidth) / srcaspect + 0.5f));
	}

	if( verbose ) printf( "destination image: size:%dx%d: colorspace:0x%08X\n",
		destwidth, destheight, srcbitmap->ColorSpace() );

	BBitmap *destbitmap = new BBitmap( BRect(0,0,destwidth-1,destheight-1), srcbitmap->ColorSpace() );
	
	if( verbose ) printf( "Scaling...\n" );
	Scale( srcbitmap, destbitmap, filter, filterwidth );

//--

	if( verbose ) printf( "Writing %s...\n", destfilename );
	
	BFile destfile( destfilename, B_WRITE_ONLY|B_CREATE_FILE|B_ERASE_FILE );
	if( destfile.InitCheck() != B_NO_ERROR )
	{
		fprintf( stderr, "Could not open file %s: %s\n", destfilename, strerror(destfile.InitCheck()) );
		exit( 1 );
	}

	if( desttype == 0 ) desttype = srcinfo.type;

	BBitmapStream input( destbitmap );
	status = BTranslatorRoster::Default()->Translate( &input, NULL, NULL, &destfile, desttype );
	if( status != B_NO_ERROR )
	{
		fprintf( stderr, "Could not write image %s: %s\n", destfilename, strerror(status) );
		exit( 1 );
	}

	
//--

	return 0;
}



