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
#ifndef DAMN_PALMALBUMTRANSLATOR_H
#define DAMN_PALMALBUMTRANSLATOR_H
//-----------------------------------------------------------------------------
//-------------------------------------
#include <translation/TranslatorAddOn.h>
#include <translation/TranslatorFormats.h>
//-------------------------------------
//-----------------------------------------------------------------------------

#define PALMALBUM_TYPE 'PATG'
#define PALMALBUM_MIME "image/x-albumtogo"

#define PALMALBUM_CFGFILE "PalmAlbumTranslator_Settings"

//-----------------------------------------------------------------------------

extern translation_format inputFormats[];
extern translation_format outputFormats[];

//-----------------------------------------------------------------------------

extern bool fVerbose;

//struct RawHeader
//{
//	int	width;
//	int	height;
//	int	colors;
//};

//-----------------------------------------------------------------------------

struct PAHeader
{
	uint32	_unknown00;		// dont' know, always 0? maybe x position?
	uint32	_unknown04;		// dont' know, always 0? maybe y position?
	uint32	width;			// width (be)
	uint32	height;			// height (be)
	uint32	_unknown10;		// don't know, always ffffffff?
	uint32	_unknown14;		// don't know, always 0?
	uint32	width2;			// same as width (be)
	uint8	_unknown1C;		// don't know, always 8? maybe palette component bitcount?
	uint8	_unknown1D;		// don't know, always 0?
	uint8	_unknown1E;		// don't know, always 0?
	uint8	colors;			// don't know, always F0? maybe palette size?
};

struct PAColor
{
	uint8	index;
	uint8	red;
	uint8	green;
	uint8	blue;
};

// PARead
status_t IsPalmAlbum( BPositionIO *stream );
status_t PalmAlbum2Bitmap( BPositionIO *instream, BPositionIO *outstream );

// PAWrite
status_t Bitmap2PalmAlbum( BPositionIO *instream, BPositionIO *outstream );

//-----------------------------------------------------------------------------
#endif
