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
#include <ctype.h>
#include <memory.h>
#include <stdio.h>
#include <vector>

#ifdef __MWERKS__
#define std
#endif
//-------------------------------------
#include <support/ByteOrder.h>
#include <support/DataIO.h>
//-------------------------------------
#include "misc/AutoPtr.h"
#include "misc/Settings.h"
#include "AmigaIconTranslator.h"
//-----------------------------------------------------------------------------

uint8 AmigaOS1x_Palette[4][3] = 
{
	{170,170,170}, {255,255,255}, {  0,  0,  0}, {238,153,  0},
};

uint8 AmigaOS2x_Palette[8][3] = 
{
	{170,170,170}, {  0,  0,  0}, {255,255,255}, {102,136,187}, {238, 68, 68}, { 85,221, 85}, {  0, 68,221}, {238,153,  0},
};

uint8 MagicWB_Palette[8][3] = 
{
	{149,149,149}, {  0,  0,  0}, {255,255,255}, { 59,103,162}, {123,123,123}, {175,175,175}, {170,144,124}, {255,169,151}
};

//-----------------------------------------------------------------------------

TranslatorBitmap ConvertToBigEndian( const TranslatorBitmap &native )
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

TranslatorBitmap ConvertToHostEndian( const TranslatorBitmap &native )
{
	TranslatorBitmap bendian;
	bendian.magic			= B_BENDIAN_TO_HOST_INT32( native.magic );
	bendian.bounds.left		= B_BENDIAN_TO_HOST_FLOAT( native.bounds.left );
	bendian.bounds.top		= B_BENDIAN_TO_HOST_FLOAT( native.bounds.top );
	bendian.bounds.right	= B_BENDIAN_TO_HOST_FLOAT( native.bounds.right );
	bendian.bounds.bottom	= B_BENDIAN_TO_HOST_FLOAT( native.bounds.bottom );
	bendian.rowBytes		= B_BENDIAN_TO_HOST_INT32( native.rowBytes );
	bendian.colors			= (color_space)B_BENDIAN_TO_HOST_INT32( (uint32)native.colors );
	bendian.dataSize		= B_BENDIAN_TO_HOST_INT32( native.dataSize );
	return bendian;
}

//-----------------------------------------------------------------------------
// AMIGAICON
// 0000 Magic			(e3 10)
// 0002 Version			(01)
// 0004 ...
// 0016 Image1
// 001A Image2
// 0020 ...
// 0030 Type
// 0032 Tool
// 0036 ...

// 0000 Magic
// 0002 Version
// 0004 Gadget:Next
// 0008 Gadget:Left
// 000a Gadget:Top
// 000c Gadget:Width
// 000e Gadget:Height
// 0010 Gadget:Flags
// 0012 Gadget:Activation
// 0014 Gadget:Gadgettype
// 0016 Gadget:Render
// 001A Gadget:Selectrender
// 001E Gadget:Gadgettext
// 0022 Gadget:Mutalexclude
// 0026 Gadget:Special info
// 002A Gadget:Id
// 002C Gadget:Userdata
// 0030 Type
// 0032 Deftool
// 0036 Tooltypes
// 003A X
// 003E Y
// 0042 Drawdata
// 0046 Toolwindow
// 004A Stacksize

// 004E Image:Left
// 0050 Image:Top
// 0052 Image:Width
// 0054 Image:Height
// 0056 Image:Depth
// 0058 Image:Imagedata
// 005C Image:Planepick
// 005D Image:Planeonoff
// 006E Image:Nextimage
// 0062 

// 0086 Image:Left
// 0088 Image:Top
// 008A Image:Width
// 008C Image:Height
// 008E Image:Depth
// 0090 Image:Imagedata
// 0094 Image:Planepick
// 0095 Image:Planeonoff
// 0096 Image:Nextimage
// 009A 
//-----------------------------------------------------------------------------
status_t ReadAmigaIconHeader( InfoHeader *header, BPositionIO *stream )
{
	uint8 rawheader[0x4e];

	if( stream->Read(rawheader, sizeof(rawheader)) != sizeof(rawheader) )
	{
		if(fVerbose) printf( "%s:ReadAmigaIconHeader(): could not read header\n", translatorName );
		return B_IO_ERROR;
	}

	int magic = B_BENDIAN_TO_HOST_INT16( *(uint16*)(rawheader+0x00) );
	if( magic != 0xe310 )
	{
		if(fVerbose) printf( "%s:ReadAmigaIconHeader(): wrong magic (0x%04x)\n", translatorName, magic );
		return B_ERROR;
	}
	
	int version = B_BENDIAN_TO_HOST_INT16( *(uint16*)(rawheader+0x02) );
	if( version != 1 )
	{
		if(fVerbose) printf( "%s:ReadAmigaIconHeader(): Unknown version (%d)\n", translatorName, version );
		return B_ERROR;
	}

	header->type = B_BENDIAN_TO_HOST_INT16( *(uint16*)(rawheader+0x30) );
//	if( header->type<1 || header->type>8 )
//	{
//		if(fVerbose) printf( "%s:ReadAmigaIconHeader(): Unknown type (%d)\n", translatorName, header->type );
//		return B_ERROR;
//	}

	header->image1 = B_BENDIAN_TO_HOST_INT32( *(uint32*)(rawheader+0x16) );
	header->image2 = B_BENDIAN_TO_HOST_INT32( *(uint32*)(rawheader+0x1A) );
	
	if( !header->image1 )
	{
		if(fVerbose) printf( "%s:ReadAmigaIconHeader(): COuld not find primary image\n", translatorName );
		return B_ERROR;
	}

	header->tool = B_BENDIAN_TO_HOST_INT32( *(uint32*)(rawheader+0x32) );

	// if this is a project or folder icon, read draw struct
	if( *(uint32*)(rawheader+0x42) )
	{
		uint8 rawdrawer[0x38];

		if( stream->Read(rawdrawer, sizeof(rawdrawer)) != sizeof(rawdrawer) )
		{
			if(fVerbose) printf( "%s:ReadAmigaIconHeader(): could not read drawer header\n", translatorName );
			return B_IO_ERROR;
		}
	}

	return B_NO_ERROR;
}

//-----------------------------------------------------------------------------

status_t ReadOldIcon( BPositionIO *stream, BMessage *msg )
{
	uint8 rawheader[0x14];

	if( stream->Read(rawheader, sizeof(rawheader)) != sizeof(rawheader) )
	{
		if(fVerbose) printf( "%s:ReadImage(): could not read header\n", translatorName );
		return B_IO_ERROR;
	}

	int width = B_BENDIAN_TO_HOST_INT16( *(uint16*)(rawheader+0x04) );
	int height = B_BENDIAN_TO_HOST_INT16( *(uint16*)(rawheader+0x06) );
	int depth = B_BENDIAN_TO_HOST_INT16( *(uint16*)(rawheader+0x08) );
	int plane_pick = *(uint8*)(rawheader+0x0e);
	int plane_onoff = *(uint8*)(rawheader+0x0f);

	if( !plane_pick )
	{
		if(fVerbose) printf( "%s:ReadImage(): the icon does not have any bitplanes??!?\n", translatorName );
		return B_ERROR;
	}

	int plane_depth;
	for( plane_depth=8; (plane_pick&(1<<(plane_depth-1)))==0; plane_depth-- )
		;

	if( depth != plane_depth )
	{
		if(fVerbose) printf( "%s:ReadImage(): illegal number of planes depth:%d planepick:%d\n", translatorName, depth, plane_depth );
		depth = plane_depth;
	}

	int rwidth = ((width+15)/16)*2;

	damn::AutoArray<uint8> rawrow( rwidth );
	damn::AutoArray<uint8> bitmap8( width*height );
	memset( bitmap8, 0, width*height );
	for( int ib=0; ib<depth; ib++ )
	{
		int oslib = 1<<ib;
		if( plane_pick&oslib )
		{
			// read plane
			
			for( int iy=0; iy<height; iy++ )
			{
				if( stream->Read(rawrow, rwidth) != rwidth )
				{
					if(fVerbose) printf( "%s:ReadImage(): read error reading line %d/%d\n", translatorName, iy, height );
					return B_IO_ERROR;
				}
				
				for( int ix=0; ix<width; ix++ )
				{
					if( rawrow[ix/8]&(0x80>>(ix&0x07)) )
						bitmap8[ix+iy*width] |= oslib;
				}
			}
			
		}
		else
		{
			if( plane_onoff&oslib )
			{
				for( int iy=0; iy<height; iy++ )
				{
					for( int ix=0; ix<width; ix++ )
					{
						bitmap8[ix+iy*width] |= oslib;
					}
				}
			}
		}
	}

	damn::AutoArray<uint8> translatorbitmap( sizeof(TranslatorBitmap) + width*height*4 );
	TranslatorBitmap bmheader;
	bmheader.magic			= B_TRANSLATOR_BITMAP;
	bmheader.bounds.left	= 0;
	bmheader.bounds.top		= 0;
	bmheader.bounds.right	= width-1;
	bmheader.bounds.bottom	= height-1;
	bmheader.rowBytes		= (bmheader.bounds.IntegerWidth()+1)*4;
	bmheader.colors			= B_RGB32;
	bmheader.dataSize		= bmheader.rowBytes*((int)bmheader.bounds.bottom+1);
	TranslatorBitmap big_bmheader = ConvertToBigEndian( bmheader );
	memcpy( translatorbitmap, &big_bmheader, sizeof(TranslatorBitmap) );

	uint32 palette[256];
	int paletteindex = gSettings.GetInt32( "palette", DEFAULT_PALETTE );
	for( int i=0; i<256; i++ )
	{
		uint8 *palent;
		switch( paletteindex )
		{
			case 0: palent = AmigaOS1x_Palette[i&3]; break;
			case 1: palent = AmigaOS2x_Palette[i&7]; break;
			default:palent = MagicWB_Palette[i&7]; break;
		}
		palette[i] = B_HOST_TO_BENDIAN_INT32( palent[0]<<8 | palent[1]<<16 | palent[2]<<24 | 0xff<<0 );
	}

	for( int iy=0; iy<height; iy++ )
	{
		uint32 *row = (uint32*)(translatorbitmap + sizeof(TranslatorBitmap) + iy*width*4);
		for( int ix=0; ix<width; ix++ )
		{
			uint8 cindex = bitmap8[ix+iy*width];
			row[ix] = palette[cindex];
		}
	}

	msg->AddData( "oldicons", B_TRANSLATOR_BITMAP, translatorbitmap, sizeof(TranslatorBitmap) + width*height*4 );
	return B_NO_ERROR;
}

int32 ReadString( BPositionIO *stream, BString *string )
{
	int32 stringlen;
	if( stream->Read(&stringlen, sizeof(stringlen)) != sizeof(stringlen) )
	{
		if(fVerbose) printf( "%s:ReadString(): could not read string length\n", translatorName );
		return B_IO_ERROR;
	}
	stringlen = B_BENDIAN_TO_HOST_INT32( stringlen );
	
	if( stringlen == 0 )
	{
		string->SetTo( "" );
		return 0;
	}
	
	if( stringlen > 1024*1024 )
	{
		if(fVerbose) printf( "%s:ReadString(): ToolType string size >1M\n\n", translatorName );
		return B_ERROR;
	}

	char *stringdata = string->LockBuffer( stringlen+1 );
	if( stream->Read(stringdata, stringlen) != stringlen )
	{
		if(fVerbose) printf( "%s:ReadString(): could not read string\n", translatorName );
		return B_IO_ERROR;
	}
	string->UnlockBuffer( stringlen );
	
	return stringlen;
}

//-----------------------------------------------------------------------------

int Decompress( const void *srcdata, void *dstdata, int dstlen, int bits )
{
	const uint8 *src = (const uint8*)srcdata;
	uint8 *dst = (uint8*)dstdata;
	int dstpos = 0;

	int bitmask = (1<<bits)-1;

	int workbits = 0;
	int currentworkbit = 7;

	int8 val;
	while( (val=*src++) != 0 )
	{
		int runlength = 0;

		val -= 0x20;
		if( val <= 0 )
		{
			val -= 0x31;
			if( val <= 0 )
			{
				runlength = 0x80 + val;
				val = 0;
			}
		}
		workbits |= val;

		while( runlength-- >= 0 )
		{
			while( currentworkbit-bits >= 0 )
			{
				currentworkbit -= bits;
				dst[dstpos++] = (workbits>>currentworkbit)&bitmask;
				if( dstpos == dstlen ) return dstpos;
			}
			workbits <<= 7;
			currentworkbit += 7;
		}
	}
	return dstpos;
}

status_t ReadNewIcons( BPositionIO *instream, BMessage *msg )
{
	int32 tooltypes;
	instream->Read( &tooltypes, 4 );
	tooltypes = B_BENDIAN_TO_HOST_INT32( tooltypes )/4 - 1 ;

	std::vector<BString> newicons[2];
	for( int i=0; i<tooltypes; i++ )
	{
		BString string;
		int32 status = ReadString( instream, &string );
		if( status < 0 ) return status;

		if( string.Length()>=4 && string[0]=='I' && string[1]=='M' && isdigit(string[2]) && string[3]=='=' )
		{
			if( string[2] == '1' )
				newicons[0].push_back( BString(string.String()+4) );
			else if( string[2] == '2' )
				newicons[1].push_back( BString(string.String()+4) );
			else
			{
				if(fVerbose) printf( "%s:ReadString(): unknown NewIcon tooltype: \"%s\"\n", translatorName, string.String() );
			}
		}
	}
	
	for( int i=0; i<2; i++ )
	{
		if( newicons[i].size() == 0 )
			continue;
			
		// decode header <w><h><pl><ph>
		if( newicons[i][0].Length()<4 )
		{
			if(fVerbose) printf( "%s:ReadString(): unknown NewIcon header: \"%s\"\n", translatorName, newicons[i][0].String() );
			continue;
		}
		int transparent = (newicons[i][0][0]-0x21) & 1;
		int width = newicons[i][0][1]-0x21;
		int height = newicons[i][0][2]-0x21;
		int colors = (newicons[i][0][3]-0x21)<<6 | (newicons[i][0][4]-0x21);
		if( width<=0 || width>95 || height<=0 || height>95 )
		{
			if(fVerbose) printf( "%s:ReadString(): illegal NewIcon size: %dx%d\n", translatorName, width, height );
			continue;
		}
		if( colors<=0 || colors>256 )
		{
			if(fVerbose) printf( "%s:ReadString(): illegal NewIcon palettesize: %d\n", translatorName, colors );
			continue;
		}
		
		uint stringindex = 0;

		// decode palette
		uint8 palette24[256*3];
		int rempalbytes = colors*3;
		rempalbytes -= Decompress( newicons[i][0].String()+5, palette24, rempalbytes, 8 );
		while( rempalbytes > 0 )
		{
			stringindex++;
			if( stringindex >= newicons[i].size() )
			{
				if(fVerbose) printf( "%s:ReadString(): not enough NewIcon data\n", translatorName );
				return B_ERROR;
			}
			rempalbytes -= Decompress( newicons[i][stringindex].String(), palette24+colors*3-rempalbytes, rempalbytes, 8 );
		}
		
		int bits = 1;
		while( (colors-1)>>bits ) bits++;

		// decode bitmap
		damn::AutoArray<uint8> bitmap( width*height );
		int rembitmapbytes = width*height;
		while( rembitmapbytes > 0 )
		{
			stringindex++;
			if( stringindex >= newicons[i].size() )
			{
				if(fVerbose) printf( "%s:ReadString(): not enough NewIcon data\n", translatorName );
				return B_ERROR;
			}
			rembitmapbytes -= Decompress( newicons[i][stringindex].String(), bitmap+width*height-rembitmapbytes, rembitmapbytes, bits );
		}

		damn::AutoArray<uint8> translatorbitmap( sizeof(TranslatorBitmap) + width*height*4 );
		TranslatorBitmap bmheader;
		bmheader.magic			= B_TRANSLATOR_BITMAP;
		bmheader.bounds.left	= 0;
		bmheader.bounds.top		= 0;
		bmheader.bounds.right	= width-1;
		bmheader.bounds.bottom	= height-1;
		bmheader.rowBytes		= (bmheader.bounds.IntegerWidth()+1)*4;
		bmheader.colors			= B_RGB32;
		bmheader.dataSize		= bmheader.rowBytes*((int)bmheader.bounds.bottom+1);
		TranslatorBitmap big_bmheader = ConvertToBigEndian( bmheader );
		memcpy( translatorbitmap, &big_bmheader, sizeof(TranslatorBitmap) );

		uint32 palette32[256];
		for( int i=0; i<256; i++ )
		{
			if( i < colors )
				palette32[i] = B_HOST_TO_BENDIAN_INT32( palette24[i*3+0]<<8 | palette24[i*3+1]<<16 | palette24[i*3+2]<<24 | 0xff<<0 );
			else
				palette32[i] = B_HOST_TO_BENDIAN_INT32( 0x00ff00ff );
		}
		if( transparent )
			palette32[0] &= B_HOST_TO_BENDIAN_INT32( 0xffffff00 );

		for( int iy=0; iy<height; iy++ )
		{
			uint32 *row = (uint32*)(translatorbitmap + sizeof(TranslatorBitmap) + iy*width*4);
			for( int ix=0; ix<width; ix++ )
			{
				uint8 cindex = bitmap[ix+iy*width];
				row[ix] = palette32[cindex];
			}
		}
		
		msg->AddData( "newicons", B_TRANSLATOR_BITMAP, translatorbitmap, sizeof(TranslatorBitmap) + width*height*4 );
	}
	
	return B_NO_ERROR;
}
	
status_t ReadOldIcons( const InfoHeader &infoheader, BPositionIO *stream, BMessage *msg )
{
	status_t status = ReadOldIcon( stream, msg );
	if( status != B_NO_ERROR )
	{
		if(fVerbose) printf( "%s:ReadOldIcons(): could not read first icon\n", translatorName );
		return status;
	}

	if( infoheader.image2 )
	{
		status_t status = ReadOldIcon( stream, msg );
		if( status != B_NO_ERROR )
		{
			if(fVerbose) printf( "%s:ReadOldIcons(): could not read second icon\n", translatorName );
			return status;
		}
	}

	return B_NO_ERROR;
}

void ConvertBitmapHeaders( BMessage *msg )
{
	
}

status_t AmigaIcon2Bitmap( const InfoHeader &infoheader, BPositionIO *instream, BPositionIO *outstream, BMessage *msg )
{
	status_t status;

	status = ReadOldIcons( infoheader, instream, msg );
	if( status != B_NO_ERROR ) return status;

	if( infoheader.tool )
	{
		BString deftool;
		status = ReadString( instream, &deftool );
//		if( status < 0 )
//			return status;
	}
	
	status = ReadNewIcons( instream, msg );
//	if( status != B_NO_ERROR ) return status;
	
	const void *data1=NULL, *data2=NULL;
	ssize_t datasize1, datasize2;
	if( msg->HasData("newicons",B_TRANSLATOR_BITMAP) )
	{
		msg->FindData( "newicons", B_TRANSLATOR_BITMAP, 0, &data1, &datasize1 );
		msg->FindData( "newicons", B_TRANSLATOR_BITMAP, 1, &data2, &datasize2 );
	}
	else if( msg->HasData("oldicons",B_TRANSLATOR_BITMAP) )
	{
		msg->FindData( "oldicons", B_TRANSLATOR_BITMAP, 0, &data1, &datasize1 );
		msg->FindData( "oldicons", B_TRANSLATOR_BITMAP, 1, &data2, &datasize2 );
	}
	else
		return B_ERROR;
		
	if( gSettings.GetInt32("whichicon",DEFAULT_WHICHICON)==2 && data2 )
	{
		TranslatorBitmap header1 = ConvertToHostEndian( *(const TranslatorBitmap*)data1 );
		TranslatorBitmap header2 = ConvertToHostEndian( *(const TranslatorBitmap*)data2 );
		if( header1.bounds.right == header2.bounds.right )
		{
			TranslatorBitmap newheader = header1;
			newheader.bounds.bottom += header2.bounds.bottom+1;
			newheader.dataSize += header2.dataSize;
			printf( "%f+%f=%f\n", header1.bounds.bottom, header2.bounds.bottom, newheader.bounds.bottom );

			TranslatorBitmap beheader = ConvertToBigEndian( newheader );
			if( outstream->Write(&beheader, sizeof(TranslatorBitmap)) != sizeof(TranslatorBitmap) )
			{
				if(fVerbose) printf("%s:AmigaIcon2Bitmap(): could not write header\n", translatorName);
				return B_IO_ERROR;
			}    

			if( outstream->Write(((uint8*)data1)+sizeof(TranslatorBitmap), datasize1-sizeof(TranslatorBitmap)) != ssize_t(datasize1-sizeof(TranslatorBitmap)) )
			{
				if(fVerbose) printf("%s:AmigaIcon2Bitmap(): could not write bitmap data\n", translatorName);
				return B_IO_ERROR;
			}

			if( outstream->Write(((uint8*)data2)+sizeof(TranslatorBitmap), datasize2-sizeof(TranslatorBitmap)) != ssize_t(datasize2-sizeof(TranslatorBitmap)) )
			{
				if(fVerbose) printf("%s:AmigaIcon2Bitmap(): could not write bitmap data 2\n", translatorName);
				return B_IO_ERROR;
			}
			
			return B_NO_ERROR;
		}
	}

	const void *data=NULL;
	ssize_t datasize;
	if( gSettings.GetInt32("whichicon",DEFAULT_WHICHICON)==1 && data2 )
	{
		data = data2;
		datasize = datasize2;
	}
	else
	{
		data = data1;
		datasize = datasize1;
	}

//	TranslatorBitmap header = ConvertToBigEndian( *(TranslatorBitmap*)data );
//	if( outstream->Write(&header, sizeof(TranslatorBitmap)) != sizeof(TranslatorBitmap) )
	if( outstream->Write((TranslatorBitmap*)data, sizeof(TranslatorBitmap)) != sizeof(TranslatorBitmap) )
	{
		if(fVerbose) printf("%s:AmigaIcon2Bitmap(): could not write header\n", translatorName);
		return B_IO_ERROR;
	}    

	if( outstream->Write(((uint8*)data)+sizeof(TranslatorBitmap), datasize-sizeof(TranslatorBitmap)) != ssize_t(datasize-sizeof(TranslatorBitmap)) )
	{
		if(fVerbose) printf("%s:AmigaIcon2Bitmap(): could not write bitmap data\n", translatorName);
		return B_IO_ERROR;
	}
	return B_NO_ERROR;
}

//-----------------------------------------------------------------------------
