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
#include <stdlib.h>
//-------------------------------------
#include <app/Application.h>
#include <app/Roster.h>
#include <interface/Alert.h>
#include <storage/Mime.h>
#include <support/String.h>
#include <translation/TranslatorAddOn.h>
//-------------------------------------
#include "dle/DamnLayoutEngine.h"
#include "misc/InstallTranslator.h"
#include "misc/Settings.h"
#include "AmigaIconTranslator.h"
//-----------------------------------------------------------------------------

void RegisterMime();

//-----------------------------------------------------------------------------

char translatorName[] = "AmigaIcon Translator";
char translatorInfo[] = "Reads Amiga .info files";
int32 translatorVersion= VERSION_MAJOR*100 + VERSION_MINOR*10 + VERSION_REVISION; // VER:

translation_format inputFormats[] = 
{
	{ AMIGAICON_TYPE,		B_TRANSLATOR_BITMAP,	0.1f, 0.5f, AMIGAICON_MIME,			"Amiga icon" },
	{ 0,					0,						0.0f, 0.0f, "",						"" }
};

translation_format outputFormats[] = 
{
	{ B_TRANSLATOR_BITMAP,	B_TRANSLATOR_BITMAP, 	0.4f, 0.8f, "image/x-be-bitmap",	"Be Bitmap format (AmigaIconTranslator)" },
	{ 0,					0,						0.0f, 0.0f, "",						"" }
};

//-----------------------------------------------------------------------------

bool fVerbose = getenv("AMIGAICON_TRANSLATOR_DEBUG")!=NULL;

damn::Settings gSettings( AMIGAICON_CFGFILE, true );

//-----------------------------------------------------------------------------

status_t Identify(
			BPositionIO			*inSource,
	const	translation_format	*/*inFormat*/,	// may be NULL
			BMessage			*/*ioExtension*/, // may be NULL
			translator_info		*outInfo,
			uint32				outType ) 
{
	if(fVerbose) printf( "%s:Identify()\n", translatorName );

	// check that we can handle the output format requested
	if( (outType!=0) && (outType!=B_TRANSLATOR_BITMAP) )
	{
		if(fVerbose) printf( "%s: unknown outtype: %.4s\n", translatorName, (char*)&outType );
		return B_NO_TRANSLATOR;
	}

	InfoHeader infoheader;
	if( ReadAmigaIconHeader(&infoheader, inSource) == B_NO_ERROR )
	{
		if(fVerbose) printf( "%s: Found .info file\n", translatorName );

		outInfo->type = inputFormats[0].type;
		outInfo->group = inputFormats[0].group;
		outInfo->quality = inputFormats[0].quality;
		outInfo->capability = inputFormats[0].capability;
		strcpy( outInfo->name, inputFormats[0].name );
		strcpy( outInfo->MIME, inputFormats[0].MIME );

		return B_NO_ERROR;
	}

//	inSource->Seek( 0, SEEK_SET );

	return B_NO_TRANSLATOR;
}

//-----------------------------------------------------------------------------

status_t Translate(
			BPositionIO		*inSource,
	const	translator_info	*inInfo,
			BMessage		*ioExtension,
			uint32			outType,
			BPositionIO		*outDestination )
{
	if(fVerbose) printf( "%s:Translate() from:%.4s to:%.4s\n", translatorName, (char*)&inInfo->type, (char*)&outType );

	if( outType == inInfo->type )
	{
		uint8 *buffer = new uint8[64*1024];
		ssize_t readsize;
		while( (readsize=inSource->Read(buffer, 64*1024)) > 0 )
		{
			if( outDestination->Write(buffer,readsize) != readsize )
			{
				if(fVerbose) printf( "%s:Translate(): could not copy data\n", translatorName );
				delete buffer;
				return B_IO_ERROR;
			}
		}
		delete buffer;
		return B_NO_ERROR;
	}
	else if( (outType==B_TRANSLATOR_BITMAP) && (inInfo->type==AMIGAICON_TYPE) )
	{
		InfoHeader infoheader;
		if( ReadAmigaIconHeader(&infoheader, inSource) != B_NO_ERROR )
		{
			if(fVerbose) printf("%s:Translate(): could not read icon header\n", translatorName);
			return B_IO_ERROR;
		}
		
		if( ioExtension )
			return AmigaIcon2Bitmap( infoheader, inSource, outDestination, ioExtension );
		else
		{
			BMessage msg;
			return AmigaIcon2Bitmap( infoheader, inSource, outDestination, &msg );
		}
	}

	return B_NO_TRANSLATOR;
}

//-----------------------------------------------------------------------------

void RegisterMime()
{
	BMimeType mime( AMIGAICON_MIME );
	BMessage fileext;
//	mime.GetFileExtensions( &fileext );
	fileext.AddString( "extensions", "info" );
	mime.SetFileExtensions( &fileext );
#if B_BEOS_VERSION >=0x0460
	mime.SetSnifferRule( "0.5 (\"\xe3\x10\x00\x01\")" );
#endif
	mime.SetShortDescription( "Amiga icon" );
	mime.SetLongDescription( "Amiga icon" );
	mime.Install();
}

void Install()
{
	app_info appinfo;
	status_t status = be_app->GetAppInfo( &appinfo );
	if( status != B_NO_ERROR ) exit( 1 );
	status = damn::InstallTranslator( translatorName, appinfo.ref );
	if( status != B_NO_ERROR )
	{
		(new BAlert( translatorName, "An error occured while installing the translator", "Crap!"))->Go();
		exit( 1 );
	}
}

//-----------------------------------------------------------------------------

int main( int argc, char **argv )
{
	BApplication app( "application/x-vnd.DamnRednex-AmigaIconTranslatorAddOn" );

	if( argc==2 && strcmp(argv[1],"--regmime")==0 )
	{
		RegisterMime();
		return 0;
	}
	else if( argc==2 && strcmp(argv[1],"--install")==0 )
	{
		Install();
		RegisterMime();
		return 0;
	}
	else
	{
		int selection = (new BAlert( translatorName, "This is a Translator add-on\n\nDo you want to install it?", "No", "Yes"))->Go();
		if( selection == 1 )
		{
			Install();
			RegisterMime();
		}
		return 0;
	}
	
	return 1;
}

//-----------------------------------------------------------------------------

