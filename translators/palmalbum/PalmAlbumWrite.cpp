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
#include <pi-file.h>

#include "gfx/BitmapScale.h"
#include "gfx/ColorQuant.h"
#include "gfx/TranslatorUtils.h"
#include "misc/AutoPtr.h"
#include "misc/StorageUtils.h"
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

	char dstname[B_FILE_NAME_LENGTH];

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
	else
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
		damn::Copy( &tmpfile, outstream );
		tmpfile.Unset();
		unlink( tmpfilename );

		return B_OK;
	}
	
	// TODO: add 8 bits supprt (should be easy)
	return B_ERROR;
}








#if 0
//-----------------------------------------------------------------------------
// TRANSLATOR_BITMAP HEADER:
// 0000 Magic			(B_TRANSLATOR_BITMAP)
// 0004 Rect.Left		(float)
// 0008 Rect.Top		(float)
// 000C Rect.Right		(float)
// 0010 Rect.Bottom		(float)
// 0014 RowBytes
// 0018 ColorSpace		(in color_space format)
// 001C DataSize		(-header)
// 0020
//-----------------------------------------------------------------------------

status_t ReadBitmapHeader( TranslatorBitmap *header, BPositionIO *stream )
{
// FIXME: use ConvertToLEndian() instead of all this:

	if( sizeof(TranslatorBitmap) != 32 )
	{
		if(fVerbose) printf("%s:ReadBitmapHeader(): TranslatorBitmap has changed in size, it should have been 32 bytes long\n",translatorName);
		return B_ERROR;
	}

	uint8 rawheader[sizeof(TranslatorBitmap)];

	if( stream->Read(rawheader, sizeof(rawheader)) != sizeof(rawheader) )
	{
		if(fVerbose) printf("%s:ReadBitmapHeader(): could not read header\n", translatorName);
		return B_IO_ERROR;
	}

	header->magic = B_BENDIAN_TO_HOST_INT32( *((int32*)(rawheader+0x00)) );

	if( header->magic != B_TRANSLATOR_BITMAP )
	{
		if(fVerbose) printf("%s:ReadBitmapHeader(): wrong magic\n", translatorName);
		return B_ERROR;
	}
	
	header->bounds.left = B_BENDIAN_TO_HOST_FLOAT( *((float*)(rawheader+0x04)) );
	header->bounds.top = B_BENDIAN_TO_HOST_FLOAT( *((float*)(rawheader+0x08)) );
	header->bounds.right = B_BENDIAN_TO_HOST_FLOAT( *((float*)(rawheader+0x0c)) );
	header->bounds.bottom = B_BENDIAN_TO_HOST_FLOAT( *((float*)(rawheader+0x10)) );
	header->rowBytes = B_BENDIAN_TO_HOST_INT32( *((int32*)(rawheader+0x14)) );
	header->colors = (color_space)B_BENDIAN_TO_HOST_INT32( *((int32*)(rawheader+0x18)) );
	header->dataSize = B_BENDIAN_TO_HOST_INT32( *((int32*)(rawheader+0x1C)) );

	return B_NO_ERROR;
}

//-----------------------------------------------------------------------------

status_t WriteRawHeader( const RawHeader &header, BPositionIO *stream )
{
	uint8 rawheader[32];
	
	memset( rawheader, 0, 32 );
	// magic
	rawheader[0] = 0x6d;
	rawheader[1] = 0x68;
	rawheader[2] = 0x77;
	rawheader[3] = 0x61;
	rawheader[4] = 0x6e;
	rawheader[5] = 0x68;

	// version
	rawheader[6] = 0x00;
	rawheader[7] = 0x04;

	rawheader[8] = (header.width>>8)&0xff;
	rawheader[9] = header.width&0xff;

	rawheader[10] = (header.height>>8)&0xff;
	rawheader[11] = header.height&0xff;

	rawheader[12] = (header.colors>>8)&0xff;
	rawheader[13] = header.colors&0xff;

	if( stream->Write(&rawheader, 32) != 32 )
	{
		if(fVerbose) printf("%s:WriteRawHeader(): could not write header\n", translatorName);
		return B_IO_ERROR;
	}

	return B_NO_ERROR;
}

//-----------------------------------------------------------------------------

bool ValidateHeader( const TranslatorBitmap &bitmapheader )
{
	int width = bitmapheader.bounds.IntegerWidth()+1;
	if( width<1 || width>65535 )
	{
		if(fVerbose) printf("%s:ValidateHeader(): bitmap too wide (%d)\n", translatorName, width);
		return false;
	}

	int height = bitmapheader.bounds.IntegerHeight()+1;
	if( height<1 || height>65535 )
	{
		if(fVerbose) printf("%s:ValidateHeader(): bitmap too high (%d)\n", translatorName, height);
		return false;
	}
	
	return true;
}

//-----------------------------------------------------------------------------

status_t Bitmap32bit2HSI24bit( const TranslatorBitmap &bitmapheader, BPositionIO *instream, BPositionIO *outstream )
{
	if(fVerbose) printf("%s:Bitmap32bit2HSI24bit()\n", translatorName);

	RawHeader hsiheader;
	status_t status;
	
	if( !ValidateHeader(bitmapheader) )
		return B_ERROR;

	hsiheader.width  = bitmapheader.bounds.IntegerWidth()+1;
	hsiheader.height = bitmapheader.bounds.IntegerHeight()+1;
	hsiheader.colors = 0;
	
	if( (status=WriteRawHeader(hsiheader, outstream)) != B_NO_ERROR )
	{
		if(fVerbose) printf("%s:Bitmap32bit2HSI24bit(): could not write header\n", translatorName);
		return status;
	}

	damn::AutoArray<uint32> bitsin( hsiheader.width );
	damn::AutoArray<uint8> bitsout( hsiheader.width*3 );
	for( int iy=0; iy<hsiheader.height; iy++ )
	{
		if( instream->Read(bitsin, hsiheader.width*4) != hsiheader.width*4 )
		{
			if(fVerbose) printf("%s:Bitmap32bit2HSI24bit(): coult not read bits\n", translatorName);
			return B_IO_ERROR;
		}
		for( int ix=0; ix<hsiheader.width; ix++ )
		{
#if defined(__LITTLE_ENDIAN)
			bitsout[ix*3+0] = (bitsin[ix]>>16)&0xff;
			bitsout[ix*3+1] = (bitsin[ix]>>8)&0xff;
			bitsout[ix*3+2] = (bitsin[ix]>>0)&0xff;
#else
			bitsout[ix*3+0] = (bitsin[ix]>>8)&0xff;
			bitsout[ix*3+1] = (bitsin[ix]>>16)&0xff;
			bitsout[ix*3+2] = (bitsin[ix]>>24)&0xff;
#endif
		}
		if( outstream->Write(bitsout, hsiheader.width*3) != hsiheader.width*3 )
		{
			if(fVerbose) printf("%s:Bitmap32bit2HSI24bit(): coult not write bits\n", translatorName);
			return B_IO_ERROR;
		}
	}
	return B_NO_ERROR;
}

status_t Bitmap8bit2HSI8bit( const TranslatorBitmap &bitmapheader, const uint8 palette[][3], int palettesize, BPositionIO *instream, BPositionIO *outstream )
{
	if(fVerbose) printf("%s:Bitmap8bit2HSI8bit()\n", translatorName);

	RawHeader hsiheader;
	status_t status;
	
	if( !ValidateHeader(bitmapheader) )
		return B_ERROR;

	hsiheader.width  = bitmapheader.bounds.IntegerWidth()+1;
	hsiheader.height = bitmapheader.bounds.IntegerHeight()+1;
	hsiheader.colors = palettesize;
	
	if( (status=WriteRawHeader(hsiheader, outstream)) != B_NO_ERROR )
	{
		if(fVerbose) printf("%s:Bitmap8bit2HSI8bit(): could not write header\n", translatorName);
		return status;
	}

	if( outstream->Write(palette, palettesize*3) != palettesize*3 )
	{
		if(fVerbose) printf("%s:Bitmap8bit2HSI8bit(): could not write palette\n", translatorName);
		return B_IO_ERROR;
	}
	
	damn::AutoArray<uint8> bits( hsiheader.width );
	for( int iy=0; iy<hsiheader.height; iy++ )
	{
		if( instream->Read(bits, hsiheader.width) != hsiheader.width )
		{
			if(fVerbose) printf("%s:Bitmap8bit2HSI8bit(): coult not read bits\n", translatorName);
			return B_IO_ERROR;
		}
		if( outstream->Write(bits, hsiheader.width) != hsiheader.width )
		{
			if(fVerbose) printf("%s:Bitmap8bit2HSI8bit(): coult not write bits\n", translatorName);
			return B_IO_ERROR;
		}
	}
	return B_NO_ERROR;
}

#endif