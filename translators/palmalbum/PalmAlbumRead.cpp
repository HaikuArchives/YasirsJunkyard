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
#include <unistd.h>
//-------------------------------------
#include <storage/File.h>
#include <support/ByteOrder.h>
#include <support/DataIO.h>
//-------------------------------------
#include "pi-file.h"

#include "gfx/TranslatorUtils.h"
#include "misc/AutoPtr.h"
#include "storage/Utils.h"
#include "PalmAlbumTranslator.h"
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------

status_t IsPalmAlbum( BPositionIO *stream )
{
	uint8 palmheader[4*16+4];
	if( stream->Read(palmheader, sizeof(palmheader)) != sizeof(palmheader) )
	{
		if(fVerbose) printf( "%s:ReadPalmHeader: could not read header\n", translatorName );
		return B_IO_ERROR;
	}

	if( memcmp(palmheader+3*16+12, "PAI8ATGC", 8) != 0 )
	{
		if(fVerbose) printf( "%s:ReadPalmHeader(): wrong magic\n", translatorName );
		return B_ERROR;
	}
	return B_OK;
}

//-----------------------------------------------------------------------------

status_t PalmAlbum2Bitmap( BPositionIO *instream, BPositionIO *outstream )
{
	const char *tmpfilename = tmpnam(NULL);
	
	BFile tmpfile( tmpfilename, B_READ_WRITE|B_CREATE_FILE|B_ERASE_FILE );
	damn::Copy_( &tmpfile, instream );
	tmpfile.Unset();

	struct pi_file *pilotfile = pi_file_open( const_cast<char*>(tmpfilename) );
	unlink( tmpfilename );
	if( pilotfile == NULL )
	{
		if(fVerbose) printf( "%s:PalmAlbum2Bitmap(): could not open pi file %s\n", translatorName, tmpfilename );
		return B_ERROR;
	}

	struct DBInfo info;
	if( pi_file_get_info(pilotfile, &info ) < 0 )
	{
		if(fVerbose) printf( "%s:PalmAlbum2Bitmap(): could not get info for pi file %s\n", translatorName, tmpfilename );
		return B_ERROR;
	}

	int entcount;
	pi_file_get_entries( pilotfile, &entcount );

	for( int ient=0; ient<entcount; ient++ )
	{
		void *resdata;
		int ressize;
		unsigned long restype;
		int resid;
		if( pi_file_read_resource( pilotfile, ient, &resdata, &ressize, &restype, &resid) < 0 )
		{
			pi_file_close( pilotfile );
			if(fVerbose) printf( "%s:PalmAlbum2Bitmap(): could not read entry %d/%d\n", translatorName, ient, entcount );
			return B_ERROR;
		}
		
		if( restype == 'PBmp' )
		{
			if( ressize < 32 )
			{
				if(fVerbose) printf( "%s:PalmAlbum2Bitmap(): the resource is too small %d\n", translatorName, ressize );
				continue;
			}
			PAHeader header;
			memcpy( &header, resdata, sizeof(header) );
			header._unknown00	= B_BENDIAN_TO_HOST_INT32( header._unknown00 );
			header._unknown04	= B_BENDIAN_TO_HOST_INT32( header._unknown04 );
			header.width		= B_BENDIAN_TO_HOST_INT32( header.width );
			header.height		= B_BENDIAN_TO_HOST_INT32( header.height );
			header._unknown10	= B_BENDIAN_TO_HOST_INT32( header._unknown10 );
			header._unknown14	= B_BENDIAN_TO_HOST_INT32( header._unknown14 );
			header.width2		= B_BENDIAN_TO_HOST_INT32( header.width2 );
			
			if( ressize != int(32 + header.colors*4 + header.width*header.height) )
			{
				pi_file_close( pilotfile );
				if(fVerbose) printf( "%s:PalmAlbum2Bitmap(): the resource has a bogus size %d (%ld)\n", translatorName, ressize, 32 + header.colors*4 + header.width*header.height );
				continue;
			}
			
			PAColor palette[256];
			memcpy( palette, ((uint8*)resdata)+32, header.colors*sizeof(PAColor) );
			
			uint32 native_palette[256];
			memset( native_palette, 0, sizeof(native_palette) );
			for( int i=0; i<header.colors; i++ )
				native_palette[i] = B_HOST_TO_BENDIAN_INT32( palette[i].blue<<24 | palette[i].green<<16 | palette[i].red<<8 | 0xff );

			TranslatorBitmap bmheader;
			bmheader.magic = B_TRANSLATOR_BITMAP;
			bmheader.bounds.left = 0;
			bmheader.bounds.top = 0;
			bmheader.bounds.right = header.width-1;
			bmheader.bounds.bottom = header.height-1;
			bmheader.rowBytes = (bmheader.bounds.IntegerWidth()+1) * 4;
			bmheader.colors = B_RGB32;
			bmheader.dataSize = bmheader.rowBytes*(bmheader.bounds.IntegerHeight()+1);

			TranslatorBitmap big_bmheader = damn::ConvertToBigEndian( bmheader );

			if( outstream->Write(&big_bmheader, sizeof(big_bmheader)) != sizeof(big_bmheader) )
			{
				pi_file_close( pilotfile );
				if(fVerbose) printf("%s:PalmAlbum2Bitmap(): could not write bitmap header\n", translatorName);
				return B_IO_ERROR;
			}
			if( outstream->SetSize(bmheader.dataSize+sizeof(bmheader)) < B_OK )
			{
				pi_file_close( pilotfile );
				if(fVerbose) printf("%s:PalmAlbum2Bitmap(): could not set size of stream\n", translatorName);
				return B_IO_ERROR;
			}

			damn::AutoArray<uint32> bitsout( header.width );
			for( uint iy=0; iy<header.height; iy++ )
			{
				uint8 *bitsin = ((uint8*)resdata) + 32 + header.colors*sizeof(PAColor) + iy*header.width;
				for( uint ix=0; ix<header.width; ix++ )
				{
					bitsout[ix] = native_palette[ bitsin[ix] ];
				}
				if( outstream->Write(bitsout, header.width*4) != ssize_t(header.width*4) )
				{
					pi_file_close( pilotfile );
					if(fVerbose) printf("%s:PalmAlbum2Bitmap(): coult not write bits\n", translatorName);
					return B_IO_ERROR;
				}
			}
			pi_file_close( pilotfile );
			return B_NO_ERROR;
		}
	}

	pi_file_close( pilotfile );
	return B_ERROR;
}

//-----------------------------------------------------------------------------


