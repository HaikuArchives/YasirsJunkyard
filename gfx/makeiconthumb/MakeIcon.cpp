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
//#include <stdio.h>
//-------------------------------------
#include <interface/Bitmap.h>
#include <interface/View.h>
#include <storage/NodeInfo.h>
#include <translation/BitmapStream.h>
#include <translation/TranslatorRoster.h>
//-------------------------------------
#include "gfx/BitmapScale.h"
#include "gfx/BitmapConvert.h"
#include "gfx/BitmapUtils.h"
//-------------------------------------
#include "MakeIcon.h"
//-----------------------------------------------------------------------------

static BBitmap *MakeIcon( BBitmap *srcbitmap, int icon_width, int icon_height );

//-----------------------------------------------------------------------------

status_t MakeIcon( const entry_ref &ref )
{
	BFile file( &ref, B_READ_WRITE );

	if( !file.IsFile() )
	{
		return B_ERROR;
	}
	if( !file.IsReadable() )
	{
		return B_ERROR;
	}

	return MakeIcon( &file );
}

status_t MakeIcon( BEntry *entry )
{
	BFile file( entry, B_READ_WRITE );

	if( !file.IsFile() )
	{
		return B_ERROR;
	}
	if( !file.IsReadable() )
	{
		return B_ERROR;
	}

	return MakeIcon( &file );
}

status_t MakeIcon( BFile *file )
{
	BBitmap *bitmap = ReadBitmap( file );
	if( !bitmap )
		return B_ERROR;

	BNodeInfo nodeinfo( file );
	status_t status = MakeIcon( bitmap, &nodeinfo );

	delete bitmap;
	return status;
}

status_t MakeIcon( BBitmap *bitmap, BNodeInfo *nodeinfo )
{
	bool own_bitmap32;
	BBitmap *bitmap_32;

	if( bitmap->ColorSpace() != B_RGB_32_BIT )
	{
		bitmap_32 = ConvertTo32bit( bitmap );
		if( !bitmap_32 ) return B_ERROR;
		own_bitmap32 = true;
	}
	else
	{
		bitmap_32 = bitmap;
		own_bitmap32 = false;
	}

	BBitmap *bitmap_icon32 = MakeIcon( bitmap_32, 32, 32 );
	BBitmap *bitmap_icon16 = MakeIcon( bitmap_32, 16, 16 );

	if( own_bitmap32 ) delete bitmap_32;

	nodeinfo->SetIcon( bitmap_icon32, B_LARGE_ICON );
	nodeinfo->SetIcon( bitmap_icon16, B_MINI_ICON );
	
	delete bitmap_icon16;
	delete bitmap_icon32;

	return B_NO_ERROR;
}

//-----------------------------------------------------------------------------

BBitmap *ReadBitmap( BFile *file )
{
	BTranslatorRoster *roster = BTranslatorRoster::Default();

	BNodeInfo nodeinfo( file );

	char mimetype[B_MIME_TYPE_LENGTH];
	if( nodeinfo.GetType(mimetype) >= B_OK )
	{
		BBitmapStream output;
		if( roster->Translate(file, NULL, NULL, &output, B_TRANSLATOR_BITMAP, 0, mimetype) >= B_OK )
		{
			BBitmap *bitmap;
			if( output.DetachBitmap(&bitmap)<B_NO_ERROR || bitmap==NULL )
			{
				return NULL;
			}
			return bitmap;
		}
	}

	// try and guess the filetype
	translator_info datainfo;

	if( roster->Identify(file,NULL,&datainfo) < B_OK )
	{
		return NULL;
	}
	nodeinfo.SetType( datainfo.MIME );

	BBitmapStream output;
//	if( roster->Translate(&file, &datainfo, NULL, &output, B_TRANSLATOR_BITMAP) < B_OK )
	if( roster->Translate(file, NULL, NULL, &output, B_TRANSLATOR_BITMAP) < B_OK )
	{
		return NULL;
	}

	BBitmap *bitmap;
	if( output.DetachBitmap(&bitmap)<B_NO_ERROR || bitmap==NULL )
	{
		return NULL;
	}
	
	return bitmap;

	return NULL;
}

static BBitmap *MakeIcon( BBitmap *srcbitmap, int icon_width, int icon_height )
{
	float width = srcbitmap->Bounds().Width() + 1.0;
	float height = srcbitmap->Bounds().Height() + 1.0;
	float scalewidth = float(icon_width)/width;
	float scaleheight = float(icon_height)/height;
	float scale = scalewidth<scaleheight ? scalewidth : scaleheight;
	int dstwidth = (int)floor(width*scale);
	int dstheight = (int)floor(height*scale);

	BBitmap *scaled = new BBitmap( BRect(0,0, dstwidth-1, dstheight-1), srcbitmap->ColorSpace() );
	damn::Scale( srcbitmap, scaled, damn::filter_lanczos3 );

	BBitmap *scaled_8bit = new BBitmap( BRect(0,0,icon_width-1,icon_height-1), B_COLOR_8_BIT, true );
	BView *view = new BView( BRect(0,0,icon_width-1,icon_height-1), "", 0, 0 );
	scaled_8bit->AddChild( view );

//	filter_filter,
//	filter_box,
//	filter_triangle,
//	filter_bell,
//	filter_bspline,
//	filter_lanczos3,
//	filter_mitchell


	uint8	*dst_bits = (uint8*)scaled_8bit->Bits();
	int		dst_stride = scaled_8bit->BytesPerRow();

	// make icon transparent
	for( int iy=0; iy<icon_height; iy++ )
	{
		for( int ix=0; ix<icon_width; ix++ )
		{
			dst_bits[ix+iy*dst_stride] = 0xff;
		}
	}
	
	BBitmap *scaled_8bit_2 = ConvertTo8bit( scaled );

	scaled_8bit->Lock();
//	view->DrawBitmap( scaled, BPoint((icon_width-dstwidth)/2,(icon_height-dstheight)/2) );
	view->DrawBitmap( scaled_8bit_2, BPoint((icon_width-dstwidth)/2,(icon_height-dstheight)/2) );
	scaled_8bit->Unlock();

	delete scaled_8bit_2;
	delete scaled;

	return scaled_8bit;
}
	
