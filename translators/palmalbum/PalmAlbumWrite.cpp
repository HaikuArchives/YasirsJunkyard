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
#include <stdio.h>
#include <unistd.h>
//-------------------------------------
#include <storage/File.h>
#include <support/ByteOrder.h>
#include <support/TypeConstants.h>
#include <translation/BitmapStream.h>
//-------------------------------------
#include "pi-file.h"

#include "gfx/BitmapScale.h"
#include "gfx/ColorQuant.h"
#include "gfx/TranslatorUtils.h"
#include "misc/AutoPtr.h"
#include "storage/Utils.h"
#include "PalmAlbumTranslator.h"
//-----------------------------------------------------------------------------

#define PA_WIDTH 160	// The AlbumToGo application can only handle 160x160 images :(
#define PA_HEIGHT 160

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

inline int clamp( int value, int min, int max )
{
	if( value < min ) return min;
	if( value > max ) return max;
	return value;
}

//-----------------------------------------------------------------------------

status_t Bitmap2PalmAlbum( BPositionIO *instream, BPositionIO *outstream )
{
//	status_t status;

	/*
	 *  WARNING: This way of getting the streams filename is totaly
	 *  non documented, and it probertly only works on BeFS volumes.
	 */

	char dstname[B_FILE_NAME_LENGTH] = "";

	if( dynamic_cast<BFile*>(outstream) != NULL )
	{
		BFile *outfile = dynamic_cast<BFile*>(outstream);
		int namelen = outfile->ReadAttr( "\x13",B_STRING_TYPE,0,dstname,sizeof(dstname)-1 );
		if( namelen > 0 )
		{
			dstname[namelen] = 0;
			char *ext = strrchr( dstname, '.' );
			if( ext ) *ext = 0;
		}
	}
	
	if( dstname[0] == 0 )
	{
		// make up a bogus name
		sprintf( dstname, "noname_%d", rand() );
	}


	TranslatorBitmap header;
	if( instream->Read(&header, sizeof(header)) != sizeof(header) )
		return B_IO_ERROR;
	header = damn::ConvertToNative( header );

	uint srcwidth = header.bounds.IntegerWidth()+1;
	uint srcheight = header.bounds.IntegerHeight()+1;
	
	if( (header.colors==B_RGB32) || (header.colors==B_RGBA32) )
	{
		damn::AutoArray<uint32> srcbits( header.dataSize/4 );
		if( instream->Read(srcbits,header.dataSize) != ssize_t(header.dataSize) )
		{
			if(fVerbose) printf( "%s:Bitmap2PalmAlbum(): could not read bits\n", translatorName );
			return B_IO_ERROR;
		}

		damn::AutoArray<uint32> dstbits( PA_WIDTH*PA_HEIGHT );

		uint32 fillcolor = 0; // FIXME: should be configureable
		for( uint iy=0; iy<PA_HEIGHT; iy++ )
		{
			for( uint ix=0; ix<PA_WIDTH; ix++ )
			{
				dstbits[ix+iy*PA_WIDTH] = fillcolor;
			}
		}
		
		float scale = min_c( float(PA_WIDTH)/float(srcwidth), float(PA_HEIGHT)/float(srcheight) );
		int scalewidth = int(floor(srcwidth*scale));
		int scaleheight = int(floor(srcheight*scale));
		
		int scalexpos = (PA_WIDTH-scalewidth)/2;
		int scaleypos = (PA_HEIGHT-scaleheight)/2;
		
		damn::Scale(
			srcbits, srcwidth, srcheight, srcwidth,
			dstbits+scalexpos+scaleypos*PA_WIDTH, scalewidth, scaleheight, PA_WIDTH,
			damn::filter_lanczos3 );

		damn::ColorQuant q( 0xf0, 4 );
		for( uint iy=0; iy<PA_HEIGHT; iy++ )
		{
			for( uint ix=0; ix<PA_WIDTH; ix++ )
			{
				uint32 c = dstbits[ix+iy*PA_WIDTH];
				q.AddColor( (c>>16)&0xff, (c>>8)&0xff, (c>>0)&0xff );
			}
		}

		// xxrrggbb
		std::vector<rgb_color> pal = q.GetColors();
		for( uint i=0; i<pal.size(); i++ )
		{
			rgb_color c = pal[i];
			c.red = (c.red&0xf0) | (c.red>>4);
			c.green = (c.green&0xf0) | (c.green>>4);
			c.blue = (c.blue&0xf0) | (c.blue>>4);
//			printf( "%3d: %02X %02X %02X\n", i, c.red, c.green, c.blue );
		}

		// dither
		damn::AutoArray<uint8> ditherbits( PA_WIDTH*PA_HEIGHT );
		damn::AutoArray<int[3]> dithererror( 2*(1+PA_WIDTH+1) );
		memset( dithererror, 0, (2*(PA_WIDTH+2))*sizeof(int)*3 );
		int errorindex = 0;

		for( int iy=0; iy<PA_HEIGHT; iy++ )
		{
			int errorindex2 = errorindex;
			errorindex = errorindex^1;

			memset( dithererror+(errorindex2*(PA_WIDTH+2)), 0, (PA_WIDTH+2)*sizeof(int)*3 );
			
			for( int ix=0; ix<PA_WIDTH; ix++ )
			{
				bitmap_rgb c = ((bitmap_rgb*)dstbits.Get())[ix+iy*PA_WIDTH];
				c.red = clamp( int(c.red)+int(dithererror[errorindex*(PA_WIDTH+2)+1+ix][0]), 0, 255 );
				c.green = clamp( int(c.green)+int(dithererror[errorindex*(PA_WIDTH+2)+1+ix][1]), 0, 255 );
				c.blue = clamp( int(c.blue)+int(dithererror[errorindex*(PA_WIDTH+2)+1+ix][2]), 0, 255 );

				int index = FindBest( pal, c );
				ditherbits[ix+iy*PA_WIDTH] = index;

				rgb_color bestrgb = pal[index];
				int errr = int(c.red) - int(bestrgb.red);
				int errg = int(c.green) - int(bestrgb.green);
				int errb = int(c.blue) - int(bestrgb.blue);
				
				dithererror[errorindex*(PA_WIDTH+2)+ix+2][0]	+= (errr*7)>>4;
				dithererror[errorindex*(PA_WIDTH+2)+ix+2][1]	+= (errg*7)>>4;
				dithererror[errorindex*(PA_WIDTH+2)+ix+2][2]	+= (errb*7)>>4;
				dithererror[errorindex2*(PA_WIDTH+2)+ix][0]		+= (errr*3)>>4;
				dithererror[errorindex2*(PA_WIDTH+2)+ix][1]		+= (errg*3)>>4;
				dithererror[errorindex2*(PA_WIDTH+2)+ix][2]		+= (errb*3)>>4;
				dithererror[errorindex2*(PA_WIDTH+2)+ix+1][0]	+= (errr*5)>>4;
				dithererror[errorindex2*(PA_WIDTH+2)+ix+1][1]	+= (errg*5)>>4;
				dithererror[errorindex2*(PA_WIDTH+2)+ix+1][2]	+= (errb*5)>>4;
				dithererror[errorindex2*(PA_WIDTH+2)+ix+2][0]	+= (errr*1)>>4;
				dithererror[errorindex2*(PA_WIDTH+2)+ix+2][1]	+= (errg*1)>>4;
				dithererror[errorindex2*(PA_WIDTH+2)+ix+2][2]	+= (errb*1)>>4;
				
			}
		}

		const char *tmpfilename = tmpnam(NULL);

		DBInfo dbi;
		dbi.more = 0;
		dbi.flags = dlpDBFlagResource | dlpDBFlagBackup;
		dbi.miscFlags = 0;
		dbi.type = 'PAI8';
		dbi.creator = 'ATGC';
		dbi.version = 1;
		dbi.modnum = 0;
		dbi.createDate = time(NULL);
		dbi.modifyDate = dbi.createDate;
		dbi.backupDate = dbi.createDate;
		dbi.index = 0;
		memset( dbi.name, 0, sizeof(dbi.name) );
		strncpy( dbi.name, dstname, 32 );
		dbi.name[31] = 0;
	
		struct pi_file *pifile = pi_file_create( (char*)tmpfilename, &dbi );
		pi_file_append_resource( pifile, (void*)"\0\0\0\0\0\0\0\0\0\0\0\0", 12, 'Pref', 1100 );
		pi_file_append_resource( pifile, (void*)"\0\0", 2, 'Note', 1200 );

		damn::AutoArray<uint8> bitmapresource( 32 + pal.size()*4 + PA_WIDTH*PA_HEIGHT );
		PAHeader *resheader = (PAHeader*)bitmapresource.Get();
		resheader->_unknown00 = 0;
		resheader->_unknown04 = 0;
		resheader->width = B_HOST_TO_BENDIAN_INT32( PA_WIDTH );
		resheader->height = B_HOST_TO_BENDIAN_INT32( PA_HEIGHT );
		resheader->_unknown10 = 0xffffffff;
		resheader->_unknown14 = 0;
		resheader->width2 = resheader->width;
		resheader->_unknown1C = 8;
		resheader->_unknown1D = 0;
		resheader->_unknown1E = 0;
		resheader->colors = pal.size();

		uint8 *respalette = bitmapresource+32;
		for( uint i=0; i<pal.size(); i++ )
		{
			rgb_color c = pal[i];
			respalette[i*4+0] = i;
			respalette[i*4+1] = c.red;
			respalette[i*4+2] = c.green;
			respalette[i*4+3] = c.blue;
		}
		memcpy( bitmapresource + 32 + pal.size()*4, ditherbits, PA_WIDTH*PA_HEIGHT );
	
		pi_file_append_resource( pifile, bitmapresource, 32 + pal.size()*4 + PA_WIDTH*PA_HEIGHT, 'PBmp', 1000 );
		pi_file_close( pifile );

		BFile tmpfile( tmpfilename, B_READ_ONLY );
		damn::Copy_( outstream, &tmpfile );
		tmpfile.Unset();
		unlink( tmpfilename );

		return B_OK;
	}
	
	// TODO: add 8 bits supprt (should be easy)
	return B_ERROR;
}

