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
#include <memory.h>
//-------------------------------------
#include <support/ByteOrder.h>
#include <support/DataIO.h>
//-------------------------------------
#include "misc/AutoPtr.h"
#include "HSIRawTranslator.h"
//-----------------------------------------------------------------------------

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

