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
#include <string.h>
//-------------------------------------
#include <support/ByteOrder.h>
#include <support/DataIO.h>
//-------------------------------------
#include "misc/AutoPtr.h"
#include "HSIRawTranslator.h"
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// HSIHEADER:
// 0000 Magic			(0x6d 0x68 0x77 0x61 0x6e 0x68)
// 0006 Version			(0x0004)
// 0008 Width
// 000A Height
// 000C PaletteSize		(2-256 for palettebased images, 0 or -24 for truecolor)
// 000E HorDPI
// 0010 VerDPI
// 0012 Gamma			(0 for unknown)
// 0014 Unused
// 0020
//-----------------------------------------------------------------------------

status_t ReadRawHeader( RawHeader *header, BPositionIO *stream )
{
	uint8 rawheader[32];

	if( stream->Read(rawheader, 32) != 32 )
	{
		if(fVerbose) printf( "%s:ReadRawHeader(): could not read header\n", translatorName );
		return B_IO_ERROR;
	}

	if( memcmp(rawheader, "\x6d\x68\x77\x61\x6e\x68", 6) != 0 )
	{
		if(fVerbose) printf( "%s:ReadRawHeader(): wrong magic\n", translatorName );
		return B_ERROR;
	}
	
	int version = B_BENDIAN_TO_HOST_INT16( *((int16*)(rawheader+0x06)) );

	if( version!=4 )
	{
		if(fVerbose) printf( "%s:ReadRawHeader(): Unknown version (%d)\n", translatorName, version );
		return B_ERROR;
	}

	header->width = B_BENDIAN_TO_HOST_INT16( *((uint16*)(rawheader+0x08)) );
	header->height = B_BENDIAN_TO_HOST_INT16( *((uint16*)(rawheader+0x0a)) );

	header->colors = B_BENDIAN_TO_HOST_INT16( *((int16*)(rawheader+0x0c)) );
	if( header->colors==-24 ) header->colors=0;
	if( header->colors<0 || header->colors>256 )
	{
		if(fVerbose) printf( "%s:ReadRawHeader(): illegal palette size (%d)\n", translatorName, header->colors );
		return B_ERROR;
	}

	return B_NO_ERROR;
}

//-----------------------------------------------------------------------------

TranslatorBitmap ConvertToBEndian( const TranslatorBitmap &native )
{
	TranslatorBitmap bendian;
	bendian.magic			= B_HOST_TO_BENDIAN_INT32( native.magic );
	bendian.bounds.left		= B_HOST_TO_BENDIAN_FLOAT( native.bounds.left );
	bendian.bounds.top		= B_HOST_TO_BENDIAN_FLOAT( native.bounds.top );
	bendian.bounds.right	= B_HOST_TO_BENDIAN_FLOAT( native.bounds.right );
	bendian.bounds.bottom	= B_HOST_TO_BENDIAN_FLOAT( native.bounds.bottom );
	bendian.rowBytes		= B_HOST_TO_BENDIAN_INT32( native.rowBytes );
	bendian.colors			= (color_space)B_HOST_TO_BENDIAN_INT32( (uint32)native.colors );
	bendian.dataSize		= B_HOST_TO_BENDIAN_INT32( native.dataSize );
	return bendian;
}

//-----------------------------------------------------------------------------

status_t WriteBitmapHeader( const RawHeader &rawheader, BPositionIO *stream, color_space colorspace, int bpp )
{
	TranslatorBitmap bmheader;
	bmheader.magic			= B_TRANSLATOR_BITMAP;
	bmheader.bounds.left	= 0;
	bmheader.bounds.top		= 0;
	bmheader.bounds.right	= rawheader.width-1;
	bmheader.bounds.bottom	= rawheader.height-1;
	bmheader.rowBytes		= (bmheader.bounds.IntegerWidth()+1)*bpp;
	bmheader.colors			= colorspace;
	bmheader.dataSize		= bmheader.rowBytes*((int)bmheader.bounds.bottom+1);
	TranslatorBitmap big_bmheader = ConvertToBEndian( bmheader );

	if( stream->Write(&big_bmheader, sizeof(big_bmheader)) != sizeof(big_bmheader) )
	{
		if(fVerbose) printf("%s:WriteBitmapHeader(): could not write bitmap header\n", translatorName);
		return B_IO_ERROR;
	}
	if( stream->SetSize(bmheader.dataSize+sizeof(bmheader)) < B_OK )
	{
		if(fVerbose) printf("%s:WriteBitmapHeader(): could not set size of stream\n", translatorName);
		return B_IO_ERROR;
	}
	
	return B_NO_ERROR;
}

//-----------------------------------------------------------------------------

status_t HSIBW2BitmapGray( const RawHeader &rawheader, BPositionIO *instream, BPositionIO *outstream )
{
	if(fVerbose) printf("%s:HSIBW2BitmapGray()\n", translatorName);
	
	status_t status = WriteBitmapHeader( rawheader, outstream, B_GRAYSCALE_8_BIT, 1 );
	if( status!=B_NO_ERROR ) return status;

	damn::AutoArray<uint8> bitsin( rawheader.width );
	damn::AutoArray<uint8> bitsout( rawheader.width );
	for( int iy=0; iy<rawheader.height; iy++ )
	{
		if( instream->Read(bitsin, rawheader.width) != rawheader.width )
		{
			if(fVerbose) printf("%s:HSIBW2BitmapGray(): coult not read bits\n", translatorName);
			return B_IO_ERROR;
		}
		for( int ix=0; ix<rawheader.width; ix++ )
		{
				bitsout[ix] = bitsin[ix]?0xff:0x00;
		}
		if( outstream->Write(bitsout, rawheader.width) != rawheader.width )
		{
			if(fVerbose) printf("%s:HSIBW2BitmapGray(): coult not write bits\n", translatorName);
			return B_IO_ERROR;
		}
	}
	return B_NO_ERROR;
}

//-----------------------------------------------------------------------------

status_t HSIGray2BitmapGray( const RawHeader &rawheader, BPositionIO *instream, BPositionIO *outstream )
{
	if(fVerbose) printf("%s:HSIGray2BitmapGray()\n", translatorName);

	status_t status = WriteBitmapHeader( rawheader, outstream, B_GRAYSCALE_8_BIT, 1 );
	if( status!=B_NO_ERROR ) return status;

	damn::AutoArray<uint8> bits( rawheader.width );
	for( int iy=0; iy<rawheader.height; iy++ )
	{
		if( instream->Read(bits, rawheader.width) != rawheader.width )
		{
			if(fVerbose) printf("%s:HSIGray2BitmapGray(): coult not read bits\n", translatorName);
			return B_IO_ERROR;
		}
		if( outstream->Write(bits, rawheader.width) != rawheader.width )
		{
			if(fVerbose) printf("%s:HSIGray2BitmapGray(): coult not write bits\n", translatorName);
			return B_IO_ERROR;
		}
	}
	return B_NO_ERROR;
}

//-----------------------------------------------------------------------------

status_t HSIBe2Bitmap8bit( const RawHeader &rawheader, BPositionIO *instream, BPositionIO *outstream )
{
	if(fVerbose) printf("%s:HSIBe2Bitmap8bit()\n", translatorName);

	status_t status = WriteBitmapHeader( rawheader, outstream, B_COLOR_8_BIT, 1 );
	if( status!=B_NO_ERROR ) return status;

	damn::AutoArray<uint8> bits( rawheader.width );
	for( int iy=0; iy<rawheader.height; iy++ )
	{
		if( instream->Read(bits, rawheader.width) != rawheader.width )
		{
			if(fVerbose) printf("%s:HSIBe2Bitmap8bit(): coult not read bits\n", translatorName);
			return B_IO_ERROR;
		}
		if( outstream->Write(bits, rawheader.width) != rawheader.width )
		{
			if(fVerbose) printf("%s:HSIBe2Bitmap8bit(): coult not write bits\n", translatorName);
			return B_IO_ERROR;
		}
	}
	return B_NO_ERROR;
}

//-----------------------------------------------------------------------------

status_t HSIPal2Bitmap32bit( const RawHeader &rawheader, uint8 *palette, BPositionIO *instream, BPositionIO *outstream )
{
	if(fVerbose) printf("%s:HSIPal2Bitmap32bit()\n", translatorName);

	status_t status = WriteBitmapHeader( rawheader, outstream, B_RGB_32_BIT, 4 );
	if( status!=B_NO_ERROR ) return status;

	uint32 palette32[256];
	int pi;
	for( pi=0; pi<rawheader.colors; pi++ )
		palette32[pi] = B_HOST_TO_BENDIAN_INT32( palette[pi*3+2]<<24 | palette[pi*3+1]<<16 | palette[pi*3+0]<<8 );
	for( ; pi<256; pi++ )
		palette32[pi] = B_HOST_TO_BENDIAN_INT32( 0x00000000 );

	damn::AutoArray<uint8> bitsin( rawheader.width );
	damn::AutoArray<uint32> bitsout( rawheader.width );
	for( int iy=0; iy<rawheader.height; iy++ )
	{
		if( instream->Read(bitsin, rawheader.width) != rawheader.width )
		{
			if(fVerbose) printf("%s:HSIPal2Bitmap32bit(): coult not read bits\n", translatorName);
			return B_IO_ERROR;
		}
		for( int ix=0; ix<rawheader.width; ix++ )
		{
			bitsout[ix] = palette32[bitsin[ix]];
		}
		if( outstream->Write(bitsout, rawheader.width*4) != rawheader.width*4 )
		{
			if(fVerbose) printf("%s:HSIPal2Bitmap32bit(): coult not write bits\n", translatorName);
			return B_IO_ERROR;
		}
	}
	return B_NO_ERROR;
}

status_t HSI24bit2Bitmap32bit( const RawHeader &rawheader, BPositionIO *instream, BPositionIO *outstream )
{
	if(fVerbose) printf("%s:HSI24bit2Bitmap32bit()\n", translatorName);

	status_t status = WriteBitmapHeader( rawheader, outstream, B_RGB_32_BIT, 4 );
	if( status!=B_NO_ERROR ) return status;

	damn::AutoArray<uint8> bitsin( rawheader.width*3 );
	damn::AutoArray<uint32> bitsout( rawheader.width );
	for( int iy=0; iy<rawheader.height; iy++ )
	{
		if( instream->Read(bitsin, rawheader.width*3) != rawheader.width*3 )
		{
			if(fVerbose) printf("%s:HSI24bit2Bitmap32bit(): coult not read bits\n", translatorName);
			return B_IO_ERROR;
		}
		for( int ix=0; ix<rawheader.width; ix++ )
		{
			bitsout[ix] = B_HOST_TO_BENDIAN_INT32( bitsin[ix*3+2]<<24 | bitsin[ix*3+1]<<16 | bitsin[ix*3+0]<<8 );
		}
		if( outstream->Write(bitsout, rawheader.width*4) != rawheader.width*4 )
		{
			if(fVerbose) printf("%s:HSI24bit2Bitmap32bit(): coult not write bits\n", translatorName);
			return B_IO_ERROR;
		}
	}
	return B_NO_ERROR;
}

//-----------------------------------------------------------------------------

