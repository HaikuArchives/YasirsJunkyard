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
#include "gfx/TranslatorUtils.h"
#include "misc/InstallTranslator.h"
#include "misc/Settings.h"
#include "misc/StorageUtils.h"
#include "PalmAlbumTranslator.h"
//-----------------------------------------------------------------------------
/*
TODO: read/write prefs
read version from resource
install mimetype
*/

void RegisterMime();

//-----------------------------------------------------------------------------

char translatorName[] = "Palm AlbumToGo Translator";
char translatorInfo[] = "Reads and writes images in the AlbumToGo file format.";
int32 translatorVersion= VERSION_MAJOR*100 + VERSION_MINOR*10 + VERSION_REVISION; // VER:

translation_format inputFormats[] = 
{
	{ PALMALBUM_TYPE,		B_TRANSLATOR_BITMAP,	0.3f, 0.8f, PALMALBUM_MIME,			"Palm AlbumToGo" },
	{ B_TRANSLATOR_BITMAP,	B_TRANSLATOR_BITMAP,	0.4f, 0.8f, "image/x-be-bitmap",	"Be Bitmap format (PalmAlbum)" },
	{ 0,					0,						0.0f, 0.0f, "",						"" }
};

translation_format outputFormats[] = 
{
	{ PALMALBUM_TYPE,		B_TRANSLATOR_BITMAP,	0.3f, 0.8f, PALMALBUM_MIME,			"Palm AlbumToGo" },
	{ B_TRANSLATOR_BITMAP,	B_TRANSLATOR_BITMAP, 	0.4f, 0.8f, "image/x-be-bitmap",	"Be Bitmap format (PalmAlbum)" },
	{ 0,					0,						0.0f, 0.0f, "",						"" }
};

//-----------------------------------------------------------------------------

bool fVerbose = getenv("PALMALBUM_TRANSLATOR_DEBUG")!=NULL;

damn::Settings gSettings( PALMALBUM_CFGFILE, true );

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
	if( (outType!=0) && (outType!=B_TRANSLATOR_BITMAP) && (outType!=PALMALBUM_TYPE) )
	{
		if(fVerbose) printf( "%s: unknown outtype: %.4s\n", translatorName, (char*)&outType );
		return B_NO_TRANSLATOR;
	}

	if( IsPalmAlbum(inSource) == B_OK )
	{
		if(fVerbose) printf( "%s: PalmAlbum file found\n", translatorName );

		outInfo->type = inputFormats[0].type;
		outInfo->group = inputFormats[0].group;
		outInfo->quality = inputFormats[0].quality;
		outInfo->capability = inputFormats[0].capability;
		strcpy( outInfo->name, inputFormats[0].name );
		strcpy( outInfo->MIME, inputFormats[0].MIME );

		return B_OK;
	}

	inSource->Seek( 0, SEEK_SET );

	if( damn::IsTranslatorBitmap(inSource) == B_OK )
	{
		if(fVerbose) printf( "%s: TranslatorBitmap file found\n", translatorName );

		outInfo->type = inputFormats[1].type;
		outInfo->group = inputFormats[1].group;
		outInfo->quality = inputFormats[1].quality;
		outInfo->capability = inputFormats[1].capability;
		strcpy( outInfo->name, inputFormats[1].name );
		strcpy( outInfo->MIME, inputFormats[1].MIME );

		return B_OK;
	}

	return B_NO_TRANSLATOR;
}

//-----------------------------------------------------------------------------

status_t Translate(
			BPositionIO		*inSource,
	const	translator_info	*inInfo,
			BMessage		*/*ioExtension*/,
			uint32			outType,
			BPositionIO		*outDestination )
{
	if(fVerbose) printf( "%s:Translate() from:%.4s to:%.4s\n", translatorName, (char*)&inInfo->type, (char*)&outType );

	if( outType == inInfo->type )
	{
		// Damn R4.5 cmpiler/linker, if I use damn::Copy() I get linker errors!!!! CRAP!
		return damn_Copy( inSource, outDestination );
	}
	else if( (outType==B_TRANSLATOR_BITMAP) && (inInfo->type==PALMALBUM_TYPE) )
	{
		return PalmAlbum2Bitmap( inSource, outDestination );
	}
	else if( (outType==PALMALBUM_TYPE) && (inInfo->type==B_TRANSLATOR_BITMAP) )
	{
		return Bitmap2PalmAlbum( inSource, outDestination );
	}

	return B_NO_TRANSLATOR;
}

//-----------------------------------------------------------------------------

class ConfigView : public damn::RootView
{
public:
					ConfigView();
					~ConfigView();

			void	MessageReceived( BMessage *msg );
			void	AttachedToWindow();

private:
	damn::BButton	*fRegMimeButton;
//	damn::BCheckBox	*fConvertCB;
};

ConfigView::ConfigView() :
	damn::RootView( BRect(0,0,100-1,100-1), B_FOLLOW_ALL, 0 )
{
	damn::BStringView *sv;
	damn::Space *sp;

	damn::AutoScrollView *asv = new damn::AutoScrollView;

		damn::VSplit *vs = new damn::VSplit( 1 );
	
			sv = new damn::BStringView( "heading", translatorName );
			sv->SetFont( be_bold_font );
			vs->AddObject( sv, 1.0f );
	
			BString version; version<<"Palm AlbumToGo image translator v"<<(int32)VERSION_MAJOR<<"."<<(int32)VERSION_MINOR<<"."<<(int32)VERSION_REVISION<<"  "<<__DATE__;
			sv = new damn::BStringView( "version", version.String() );
			vs->AddObject( sv, 1.0f );
	
			sv = new damn::BStringView( "copyright", B_UTF8_COPYRIGHT "'2000 by Jesper Hansen (jesper@funcom.com)" );
			vs->AddObject( sv, 1.0f );
	
			sp = new damn::Space();
			vs->AddObject( sp, 1.0f );
	
//			fConvertCB = new damn::BCheckBox( "bw", "Read BW/Gray images as truecolor", new BMessage('rd32') );
//			fConvertCB->SetValue( gSettings.GetBool("converto32bit",default_converto32bit) );
//			vs->AddObject( fConvertCB, 1.0f );
	
			sp = new damn::Space();
			vs->AddObject( sp, 5.0f );
	
			fRegMimeButton = new damn::BButton( "regmime", "Register mimetype", new BMessage('regm') );
			vs->AddObject( fRegMimeButton, 0.5f );
	
			sp = new damn::Space();
			vs->AddObject( sp, 5.0f );
	
			damn::HSplit *hs2 = new damn::HSplit( 0 );
				sp = new damn::HSpace();
				hs2->AddObject( sp, 1.0f );
	//			sv = new damn::BStringView( "gpl", "Released under the GPL" );
	//			hs2->AddObject( sv, 1.0f );
//				fGPLButton = new damn::BButton( "showgpl", "Released under the GPL" B_UTF8_ELLIPSIS, new BMessage('sgpl') );
//				hs2->AddObject( fGPLButton, 0.5f );
				sp = new damn::HSpace( 8,8 );
				hs2->AddObject( sp, 1.0f );
			vs->AddObject( hs2, 1.0f );

		asv->AddObject( vs );
	
	AddObject( asv );
}

ConfigView::~ConfigView()
{
}

void ConfigView::MessageReceived( BMessage *msg )
{
	switch( msg->what )
	{
		case 'regm':
			RegisterMime();
			break;
	}
}

void ConfigView::AttachedToWindow()
{
//	SetViewColor( Parent()->ViewColor() );
	damn::RootView::FrameResized( Bounds().Width()+1, Bounds().Height()+1 );

	fRegMimeButton->SetTarget( this );
//	fConvertCB->SetTarget( this );
}

status_t MakeConfig( BMessage */*ioExtension*/, BView **outView, BRect *outExtent )
{
	ConfigView *root = new ConfigView();

	*outView = root;
	*outExtent = root->Bounds();

	return B_OK;
}

//-----------------------------------------------------------------------------

void RegisterMime()
{
	BMimeType mime( PALMALBUM_MIME );
	BMessage fileext;
//	mime.GetFileExtensions( &fileext );
//	fileext.AddString( "extensions", "hsi" );
//	fileext.AddString( "extensions", "raw" );
//	mime.SetFileExtensions( &fileext );
#if B_BEOS_VERSION >=0x0460
	mime.SetSnifferRule( "0.8 [60](\"PAI8ATGC\")" );
#endif
	mime.SetShortDescription( "Palm AlbumToGo" );
	mime.SetLongDescription( "Palm AlbumToGo" );
	mime.Install();
}

void Install()
{
	app_info appinfo;
	status_t status = be_app->GetAppInfo( &appinfo );
	if( status != B_OK ) exit( 1 );
	status = damn::InstallTranslator( translatorName, appinfo.ref );
	if( status != B_OK )
	{
		(new BAlert( translatorName, "An error occured while installing the translator", "Crap!"))->Go();
		exit( 1 );
	}
}

//-----------------------------------------------------------------------------

int main( int argc, char **argv )
{
	BApplication app( "application/x-vnd.DamnRednex-PalmAlbumTranslatorAddOn" );

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

