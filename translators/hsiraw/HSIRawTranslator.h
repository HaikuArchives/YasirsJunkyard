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
#ifndef DAMN_HSIRAWTRANSLATOR_H
#define DAMN_HSIRAWTRANSLATOR_H
//-----------------------------------------------------------------------------
//-------------------------------------
#include <translation/TranslatorAddOn.h>
#include <translation/TranslatorFormats.h>
//-------------------------------------
#include "Version.h"
//-----------------------------------------------------------------------------

#define HSIRAW_TYPE 'HRaw'
#define HSIRAW_MIME "image/x-hsiraw"

#define HSIRAW_CFGFILE "HsiRawTranslator_Settings"

//-----------------------------------------------------------------------------

//extern char translatorName[];
//extern char translatorInfo[];
//extern int32 translatorVersion;

extern translation_format inputFormats[];
extern translation_format outputFormats[];

//-----------------------------------------------------------------------------

extern bool fVerbose;

extern const uint8 bw_palette[2][3];
extern const uint8 gray_palette[256][3];
extern const uint8 beos_palette[256][3];

struct RawHeader
{
	int	width;
	int	height;
	int	colors;
};

//-----------------------------------------------------------------------------

// HSIRead
status_t ReadRawHeader( RawHeader *header, BPositionIO *stream );
TranslatorBitmap ConvertToBEndian( const TranslatorBitmap &native );
status_t HSIBW2BitmapGray( const RawHeader &rawheader, BPositionIO *instream, BPositionIO *outstream );
status_t HSIGray2BitmapGray( const RawHeader &rawheader, BPositionIO *instream, BPositionIO *outstream );
status_t HSIBe2Bitmap8bit( const RawHeader &rawheader, BPositionIO *instream, BPositionIO *outstream );
status_t HSIPal2Bitmap32bit( const RawHeader &rawheader, uint8 *palette, BPositionIO *instream, BPositionIO *outstream );
status_t HSI24bit2Bitmap32bit( const RawHeader &rawheader, BPositionIO *instream, BPositionIO *outstream );

// HSIWrite
status_t ReadBitmapHeader( TranslatorBitmap *header, BPositionIO *stream );
status_t Bitmap32bit2HSI24bit( const TranslatorBitmap &bitmapheader, BPositionIO *instream, BPositionIO *outstream );
status_t Bitmap8bit2HSI8bit( const TranslatorBitmap &bitmapheader, const uint8 palette[][3], int palettesize, BPositionIO *instream, BPositionIO *outstream );

//-----------------------------------------------------------------------------
#endif
