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
#include <stdio.h>
//-------------------------------------
#include <interface/Bitmap.h>
#include <interface/GraphicsDefs.h>
#include <interface/InterfaceDefs.h>
#include <support/ByteOrder.h>
#include <support/DataIO.h>
//-------------------------------------
#include "WriteGifImage.h"
//-----------------------------------------------------------------------------

damn::WriteGifImage::WriteGifImage( BDataIO *stream )
{
	assert( stream != NULL );

	fStream = stream;
	fWidth = 0;
	fHeight = 0;
}

damn::WriteGifImage::~WriteGifImage()
{
}

//-------------------------------------

status_t damn::WriteGifImage::WriteHeader( uint width, uint height )
{
	assert( width>0 && width<=65535 );
	assert( height>0 && height<=65535 );

	fWidth = width;
	fHeight = height;


	uint16 tmp16;
	uint8 tmp8;

// Header
	fStream->Write( "GIF89a", 6 );

// Logical Screen Descriptor
	tmp16 = B_HOST_TO_LENDIAN_INT16( width );
	fStream->Write( &tmp16, 2 );
	tmp16 = B_HOST_TO_LENDIAN_INT16( height );
	fStream->Write( &tmp16, 2 );
	tmp8 = (1<<7) | // global palette
		(0<<4) |    // 7+1 pixels per component
		(0<<3) |    // colors is not sorted
		(7<<0) ;    // 2^(7+1) global palette size
	fStream->Write( &tmp8, 1 );
	tmp8 = 0; // background palette index
	fStream->Write( &tmp8, 1 );
	tmp8 = 0; // aspect ratio
	fStream->Write( &tmp8, 1 );

// Global Color Table
	for( int i=0; i<256; i++ )
	{
		rgb_color rgbcol = system_colors()->color_list[i];
		uint8 col[3] = { rgbcol.red, rgbcol.green, rgbcol.blue };
		fStream->Write( col, 3 );
	}
	
	// magic loop tag
	fStream->Write( "\x21\xff\x0b", 3 );
	fStream->Write( "NETSCAPE2.0", 8+3 );
	fStream->Write( "\x03\x01\xe8\x03\x00", 5 );

	return B_NO_ERROR;
}

//-------------------------------------

#define FLUSHSTREAM() do{ if(datause>0) {uint8 cnt=datause; fStream->Write(&cnt,1); fStream->Write(data,datause); datause=0;} }while(0)
#define WRITESTREAM(_bits) do{ data[datause++]=(_bits); if(datause==sizeof(data)) FLUSHSTREAM(); }while(0)

#define FLUSHBITS() do{ if(bitcnt!=0){ WRITESTREAM(bits); bits=0; bitcnt=0; }}while(0)
#define WRITE9BITS(_v) do{ int _d=(_v)&0x1ff; switch(bitcnt) { \
	case 0: WRITESTREAM(_d&0xff);                                        bits=(_d>>8)&0x01; bitcnt=1; break;\
	case 1: WRITESTREAM(bits|((_d<<1)&0xfe));                            bits=(_d>>7)&0x03; bitcnt=2; break;\
	case 2: WRITESTREAM(bits|((_d<<2)&0xfc));                            bits=(_d>>6)&0x07; bitcnt=3; break;\
	case 3: WRITESTREAM(bits|((_d<<3)&0xf8));                            bits=(_d>>5)&0x0f; bitcnt=4; break;\
	case 4: WRITESTREAM(bits|((_d<<4)&0xf0));                            bits=(_d>>4)&0x1f; bitcnt=5; break;\
	case 5: WRITESTREAM(bits|((_d<<5)&0xe0));                            bits=(_d>>3)&0x3f; bitcnt=6; break;\
	case 6: WRITESTREAM(bits|((_d<<6)&0xc0));                            bits=(_d>>2)&0x7f; bitcnt=7; break;\
	case 7: WRITESTREAM(bits|((_d<<7)&0x80)); WRITESTREAM((_d>>1)&0xff); bits=0;            bitcnt=0; break;\
	}} while(0)
	
uint min( uint a, uint b ) { return a<b?a:b; }

status_t damn::WriteGifImage::WriteImage( const BBitmap &bitmap )
{
	assert( bitmap.IsValid() );
	assert( bitmap.ColorSpace() == B_CMAP8 );
	WriteImage( 
		bitmap.Bits(), 
		bitmap.Bounds().Width()+1,  bitmap.Bounds().Height()+1, bitmap.BytesPerRow(),
		bitmap.Bounds().left, bitmap.Bounds().top );

	return B_NO_ERROR;
}

status_t damn::WriteGifImage::WriteImage( 
	const void *imagedata, 
	uint width, uint height, uint bytesperrow, 
	uint xpos, uint ypos )
{
	assert( width > 0 );
	assert( height > 0 );
	assert( xpos+width <= fWidth );
	assert( ypos+height <= fHeight );

	uint8 tmp8;
	uint16 tmp16;

	// Image Separator
	tmp8 = 0x2c;
	fStream->Write( &tmp8, 1 );
	// Image Left Position
	tmp16 = B_HOST_TO_LENDIAN_INT16( xpos );
	fStream->Write( &tmp16, 2 );
	// Image Top Position
	tmp16 = B_HOST_TO_LENDIAN_INT16( ypos );
	fStream->Write( &tmp16, 2 );
	// Image Width
	tmp16 = B_HOST_TO_LENDIAN_INT16( width );
	fStream->Write( &tmp16, 2 );
	// Image Height
	tmp16 = B_HOST_TO_LENDIAN_INT16( height );
	fStream->Write( &tmp16, 2 );
	// Packed Fields 
	tmp8 = (0<<7) | // Local Color Table Flag (1bit)
		(0<<6) | // Interlace Flag (1bit)
		(0<<5) | // Sort Flag (1bit)
		(0<<3) | // Reserved (2bits)
		(0<<0) ; // Size of Local Color Table (3bits)
	fStream->Write( &tmp8, 1 );

	uint8 data[255];
	uint datause = 0;

	uint8 bits = 0;
	uint bitcnt = 0;

	tmp8 = 8;
	fStream->Write( &tmp8, 1 );

	for( uint iy=0; iy<height; iy++ )
	{
		for( uint ix=0; ix<width; ix++ )
		{
			if( datause == 0 )
				WRITE9BITS( 0x100 ); // table reset code
			uint c = ((uint8*)imagedata)[ix+iy*bytesperrow];
			WRITE9BITS( c );
		}
	}
	WRITE9BITS( 0x101 );
	FLUSHBITS();
	FLUSHSTREAM();

	tmp8 = 0;
	fStream->Write( &tmp8, 1 );

	return B_NO_ERROR;
}

//-------------------------------------

status_t damn::WriteGifImage::WriteExtensionBlock( uint disposal, bool userinput, int transparencyindex, uint delaytime )
{
	assert( disposal<=3 );
	assert( delaytime<=65535 );
	assert( transparencyindex==-1 || (transparencyindex>=0 && transparencyindex<=255) );
	
	uint8 tmp8;
	uint16 tmp16;

	// Extension Introducer
	tmp8 = 0x21;
	fStream->Write( &tmp8, 1 );
	// Graphic Control Label
	tmp8 = 0xf9;
	fStream->Write( &tmp8, 1 );

	// Block Size
	tmp8 = 4;
	fStream->Write( &tmp8, 1 );
	// <Packed Fields>
	tmp8 = 0<<5 | // Reserved (3 bits)
		disposal<<2 | // Disposal Method (3 bits)
		userinput<<1 | // User Input Flag
		(transparencyindex>=0?1:0)<<0; // Transparent Color Flag
	fStream->Write( &tmp8, 1 );
	// Delay Time
	tmp16 = B_HOST_TO_LENDIAN_INT16( delaytime );
	fStream->Write( &tmp16, 2 );
	// Transparent Color Index
	tmp8 = transparencyindex==-1?0:transparencyindex;
	fStream->Write( &tmp8, 1 );

	// Block Terminator
	tmp8 = 0x00;
	fStream->Write( &tmp8, 1 );
	
	return B_NO_ERROR;
}

//-------------------------------------

status_t damn::WriteGifImage::WriteEOF()
{
	uint8 tmp8;

	tmp8 = ';';
	fStream->Write( &tmp8, 1 );

	return B_NO_ERROR;
}

//------------------------------------------------------------------------------
