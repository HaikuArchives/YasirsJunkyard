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
#include <zlib.h>
//-------------------------------------
#include <storage/Mime.h>
//-------------------------------------
#include "misc/AutoPtr.h"
#include "ZStorage.h"
//-----------------------------------------------------------------------------

status_t damn::ZCompress( BDataIO *out, BDataIO *in, bool gzip, int compresslevel )
{
	if( gzip )
	{
		uint8 header[10] =
		{
			0x1f,	// magic
			0x8b,	// magic
			8,		// deflate
			0x00,	// text file
			0,0,0,0,// modify time
			0x00,	// xflags
			0xff 	// oscode
		};
		if( out->Write(header,sizeof(header)) != sizeof(header) )
			return B_ERROR;
	}

	z_stream zstream;
	zstream.zalloc = NULL;
	zstream.zfree = NULL;
	zstream.opaque = NULL;

//    int err = deflateInit( &zstream, compresslevel );
	int err = deflateInit2( &zstream, compresslevel, Z_DEFLATED, gzip?-MAX_WBITS:MAX_WBITS, 8, Z_DEFAULT_STRATEGY );
	if( err != Z_OK )
		return B_ERROR;
		
	int buffersize = 64*1024;
	damn::AutoArray<uint8> inbuffer( buffersize );
	damn::AutoArray<uint8> outbuffer( buffersize );
	
	uLong crc = crc32( 0L, Z_NULL, 0 );
	uLong srclen = 0;
	
	zstream.next_in = inbuffer;
	zstream.avail_in = 0;
	zstream.next_out = outbuffer;
	zstream.avail_out = buffersize;
	
	uint8 tail[4];
	int tail_use = 0;
	
	int flushmode = Z_NO_FLUSH;
	while( 1 )
	{
		if( zstream.avail_in == 0 )
		{
			ssize_t readlen = in->Read( inbuffer, buffersize );
			if( readlen < 0 )
			{
				deflateEnd( &zstream );
				return B_ERROR;
			}
			if( readlen == 0 )
			{
				flushmode = Z_FINISH;
			}
			else
			{
				if( gzip )
				{
					crc = crc32( crc, inbuffer, readlen );
					srclen += readlen;
				}
	
				zstream.next_in = inbuffer;
				zstream.avail_in = readlen;
			}
		}

		err = deflate( &zstream, flushmode );
//		fprintf( stderr, "COMPRESS: %d %d->%d\n", err, zstream.total_in, zstream.total_out );
		if( err!=Z_OK && err!=Z_STREAM_END )
		{
			deflateEnd( &zstream );
			return B_ERROR;
		}

		if( (flushmode==Z_NO_FLUSH && zstream.avail_out==0) || (flushmode==Z_FINISH && zstream.avail_out!=buffersize) )
		{
			void *buffer = outbuffer;
			size_t bufferlen = buffersize-zstream.avail_out;
#if 0
			if( gzip )
			{
				if( zstream.total_out<=buffersize )
				{
					// remove the zlib header (we have already addet a gzip header)
					buffer = ((uint8*)buffer) + 2;
					bufferlen -= 2;
				}
				if( flushmode==Z_FINISH && err==Z_STREAM_END )
				{
					// remove the last 4 bytes (adler checksum)
					bufferlen -= 4;
				}
			}
#endif

//			printf( "WRITE: %d\n", bufferlen );
			ssize_t writelen = out->Write( buffer, bufferlen );
			if( writelen != bufferlen )
			{
				deflateEnd( &zstream );
				return B_ERROR;
			}
			
			zstream.next_out = outbuffer;
			zstream.avail_out = buffersize;
		}

		if( err == Z_STREAM_END )
			break;
	}

	deflateEnd( &zstream );

	if( gzip )
	{
		uint8 tail[8] =
		{
			(crc>>0)&0xff, (crc>>8)&0xff, (crc>>16)&0xff, (crc>>24)&0xff,
			(srclen>>0)&0xff, (srclen>>8)&0xff, (srclen>>16)&0xff, (srclen>>24)&0xff
		};
		if( out->Write(tail,sizeof(tail)) != sizeof(tail) )
			return B_ERROR;
	}

	return B_OK;
}


#if 0




					uLongf compressed_len = 8+(html.Length()*101/100+12)+4;
					uint8 *compressed = new uint8[compressed_len];
					if( compress2((Bytef*)compressed+8,&compressed_len, (Bytef*)html.String(), html.Length(), text_compresslevel) == Z_OK )
					{
						compressed_len -= 4;

						compressed[0] = 0x1f; // magic
						compressed[1] = 0x8b; // magic
						compressed[2] = 8; // deflate
						compressed[3] = 0x00; // text file
						compressed[4] = compressed[5] = compressed[6] = compressed[7] = 0; // modify time
						compressed[8] = 0x00; // xflags
						compressed[9] = 0xff; // oscode
						
						uLong crc = crc32( 0L, Z_NULL, 0 );
						crc = crc32( crc, (Bytef*)html.String(), html.Length() );

						compressed[8+compressed_len+0] = (crc>>0)&0xff;
						compressed[8+compressed_len+1] = (crc>>8)&0xff;
						compressed[8+compressed_len+2] = (crc>>16)&0xff;
						compressed[8+compressed_len+3] = (crc>>24)&0xff;
						compressed[8+compressed_len+4] = (html.Length()>>0)&0xff;
						compressed[8+compressed_len+5] = (html.Length()>>8)&0xff;
						compressed[8+compressed_len+6] = (html.Length()>>16)&0xff;
						compressed[8+compressed_len+7] = (html.Length()>>24)&0xff;
					
						BString xheader = "Content-encoding: gzip\n";
						connection->SendData( compressed, 8+compressed_len+8, "text/html", &xheader );
						printf( "Sending compressed %ld -> %ld\n", html.Length(), 8+compressed_len+8 );
					}
					else
						connection->SendData( html.String(), html.Length(), "text/html" );
					delete compressed;
				}
				else
				{
					connection->SendData( html.String(), html.Length(), "text/html" );
				}
			}





}
#endif

//-----------------------------------------------------------------------------
