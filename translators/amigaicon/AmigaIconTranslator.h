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
#ifndef DAMN_AMIGAICONTRANSLATOR_H
#define DAMN_AMIGAICONTRANSLATOR_H
//-----------------------------------------------------------------------------
//-------------------------------------
#include <translation/TranslatorAddOn.h>
#include <translation/TranslatorFormats.h>
//-------------------------------------
//namespace damn { class Settings; };
#include "misc/Settings.h"
//-----------------------------------------------------------------------------

// VER:
//#define AMIGAICON_VER_MAJOR		0
//#define AMIGAICON_VER_MINOR		6
//#define AMIGAICON_VER_REVISION	0

#define AMIGAICON_TYPE 'AInf'
#define AMIGAICON_MIME "image/x-amigaicon"

#define AMIGAICON_CFGFILE "AmigaIconTranslator_Settings"

//-----------------------------------------------------------------------------

extern char translatorName[];
extern char translatorInfo[];
extern int32 translatorVersion;

extern translation_format inputFormats[];
extern translation_format outputFormats[];

//-----------------------------------------------------------------------------

extern bool fVerbose;

extern damn::Settings gSettings;
#define DEFAULT_WHICHICON 0
#define DEFAULT_PALETTE 1

struct InfoHeader
{
	bool	image1;
	bool	image2;

	bool	tool;
	int		type;
};

//-----------------------------------------------------------------------------

// AmigaIconTranslator.h
void RegisterMime();

// AmigaIconRead.cpp
status_t ReadAmigaIconHeader( InfoHeader *header, BPositionIO *stream );
status_t AmigaIcon2Bitmap( const InfoHeader &infoheader, BPositionIO *instream, BPositionIO *outstream, BMessage *msg );

//-----------------------------------------------------------------------------
#endif


