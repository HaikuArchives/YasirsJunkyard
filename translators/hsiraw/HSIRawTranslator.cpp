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
//#include "dle/DamnLayoutEngine.h"
#include "dle/AutoScrollView.h"
#include "dle/BButton.h"
#include "dle/BCheckBox.h"
#include "dle/BStringView.h"
#include "dle/RootView.h"
#include "dle/Space.h"
#include "dle/Split.h"

#include "misc/InstallTranslator.h"
#include "misc/Settings.h"
//-------------------------------------
#include "HSIRawTranslator.h"
//-----------------------------------------------------------------------------
/*
TODO: read/write prefs
read version from resource
install mimetype
*/

void RegisterMime();

//-----------------------------------------------------------------------------

char translatorName[] = "HSIRaw Translator";
char translatorInfo[] = "Reads and writes images in the SIRaw file format.";
int32 translatorVersion= VERSION_MAJOR*100 + VERSION_MINOR*10 + VERSION_REVISION; // VER:

translation_format inputFormats[] = 
{
	{ HSIRAW_TYPE,			B_TRANSLATOR_BITMAP,	0.3f, 0.8f, HSIRAW_MIME,			"HSI Raw" },
	{ B_TRANSLATOR_BITMAP,	B_TRANSLATOR_BITMAP,	0.4f, 0.8f, "image/x-be-bitmap",	"Be Bitmap format (HSIRawTranslator)" },
	{ 0,					0,						0.0f, 0.0f, "",						"" }
};

translation_format outputFormats[] = 
{
	{ HSIRAW_TYPE,			B_TRANSLATOR_BITMAP,	0.3f, 0.8f, HSIRAW_MIME,			"HSI Raw" },
	{ B_TRANSLATOR_BITMAP,	B_TRANSLATOR_BITMAP, 	0.4f, 0.8f, "image/x-be-bitmap",	"Be Bitmap format (HSIRawTranslator)" },
	{ 0,					0,						0.0f, 0.0f, "",						"" }
};

//-----------------------------------------------------------------------------

bool fVerbose = getenv("HSIRAW_TRANSLATOR_DEBUG")!=NULL;

damn::Settings gSettings( HSIRAW_CFGFILE, true );
static bool default_converto32bit = true;

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
	if( (outType!=0) && (outType!=B_TRANSLATOR_BITMAP) && (outType!=HSIRAW_TYPE) )
	{
		if(fVerbose) printf( "%s: unknown outtype: %.4s\n", translatorName, (char*)&outType );
		return B_NO_TRANSLATOR;
	}

	RawHeader rawheader;
	if( ReadRawHeader(&rawheader, inSource) == B_NO_ERROR )
	{
		if(fVerbose) printf( "%s: HsiRaw, size:%dx%d colors:%d\n", translatorName, rawheader.width, rawheader.height, rawheader.colors );

		outInfo->type = inputFormats[0].type;
		outInfo->group = inputFormats[0].group;
		outInfo->quality = inputFormats[0].quality;
		outInfo->capability = inputFormats[0].capability;
		strcpy( outInfo->name, inputFormats[0].name );
		strcpy( outInfo->MIME, inputFormats[0].MIME );

		return B_NO_ERROR;
	}

	inSource->Seek( 0, SEEK_SET );
	TranslatorBitmap bitmapheader;
	if( ReadBitmapHeader(&bitmapheader, inSource) == B_NO_ERROR )
	{
		if(fVerbose) printf( "%s: TranslatorBitmap, size:%ldx%ld colorspace:%d\n", translatorName, bitmapheader.bounds.IntegerWidth()+1, bitmapheader.bounds.IntegerHeight()+1, bitmapheader.colors );

		outInfo->type = inputFormats[1].type;
		outInfo->group = inputFormats[1].group;
		outInfo->quality = inputFormats[1].quality;
		outInfo->capability = inputFormats[1].capability;
		strcpy( outInfo->name, inputFormats[1].name );
		strcpy( outInfo->MIME, inputFormats[1].MIME );

		return B_NO_ERROR;
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
	else if( (outType==B_TRANSLATOR_BITMAP) && (inInfo->type==HSIRAW_TYPE) )
	{
		RawHeader rawheader;
		if( ReadRawHeader(&rawheader, inSource) < B_OK )
		{
			if(fVerbose) printf("%s:Translate(): could not read hsi header\n", translatorName);
			return B_IO_ERROR;
		}
		
		if( rawheader.colors != 0 )
		{
			// palette based
			uint8 rawpalette[256*3];

			// read palette to dertemine the output type
			if( inSource->Read(rawpalette, rawheader.colors*3) != rawheader.colors*3 )
			{
				if(fVerbose) printf("%s:Translate(): could not read hsi palette\n", translatorName);
				return B_IO_ERROR;
			}
			if( rawheader.colors==2 && memcmp(rawpalette,bw_palette,2*3)==0 )
			{
				if( gSettings.GetBool("converto32bit",default_converto32bit) )
					return HSIPal2Bitmap32bit( rawheader, rawpalette, inSource, outDestination );
				else
					return HSIBW2BitmapGray( rawheader, inSource, outDestination );
			}
			else if( rawheader.colors==256 && memcmp(rawpalette,gray_palette,256*3)==0 )
			{
				if( gSettings.GetBool("converto32bit",default_converto32bit) )
					return HSIPal2Bitmap32bit( rawheader, rawpalette, inSource, outDestination );
				else
					return HSIGray2BitmapGray( rawheader, inSource, outDestination );
			}
			else if( rawheader.colors==256 && memcmp(rawpalette,beos_palette,256*3)==0 )
			{
				return HSIBe2Bitmap8bit( rawheader, inSource, outDestination );
			}
			else
			{
				return HSIPal2Bitmap32bit( rawheader, rawpalette, inSource, outDestination );
			}
		}
		else
		{
			return HSI24bit2Bitmap32bit( rawheader, inSource, outDestination );
		}
	}
	else if( (outType==HSIRAW_TYPE) && (inInfo->type==B_TRANSLATOR_BITMAP) )
	{
		TranslatorBitmap bitmapheader;
		if( ReadBitmapHeader(&bitmapheader, inSource) < B_OK )
		{
			if(fVerbose) printf("%s:Translate(): could not read translator header\n", translatorName);
			return B_IO_ERROR;
		}
		
		if( (bitmapheader.colors==B_RGB32) || (bitmapheader.colors==B_RGBA32) )
		{
			return Bitmap32bit2HSI24bit( bitmapheader, inSource, outDestination );
		}
		else if( bitmapheader.colors == B_COLOR_8_BIT )
		{
			return Bitmap8bit2HSI8bit( bitmapheader, beos_palette, 256, inSource, outDestination );
		}
		else if( bitmapheader.colors == B_GRAY8 )
		{
			return Bitmap8bit2HSI8bit( bitmapheader, gray_palette, 256, inSource, outDestination );
		}
		else
		{
			if(fVerbose) printf("%s:Translate(): unsupported colorspace: %d\n", translatorName, (int)bitmapheader.colors );
			return B_NO_TRANSLATOR;
		}
	}

	return B_NO_TRANSLATOR;
}

//-----------------------------------------------------------------------------

class ConfigView : public dle::RootView
{
public:
					ConfigView();
					~ConfigView();

			void	MessageReceived( BMessage *msg );
			void	AttachedToWindow();

private:
	dle::BButton	*fRegMimeButton;
	dle::BCheckBox	*fConvertCB;
};

ConfigView::ConfigView() :
	dle::RootView( BRect(0,0,100-1,100-1), B_FOLLOW_ALL, 0 )
{
	dle::BStringView *sv;
	dle::Space *sp;

	dle::AutoScrollView *asv = new dle::AutoScrollView;
	{
		dle::VSplit *vs = new dle::VSplit();
		{
			sv = new dle::BStringView( translatorName );
			sv->SetFont( be_bold_font );
			vs->AddObject( sv, 1.0f );
	
			BString version; version<<"v"<<(int32)VERSION_MAJOR<<"."<<(int32)VERSION_MINOR<<"."<<(int32)VERSION_REVISION<<"  "<<__DATE__;
			sv = new dle::BStringView( version.String() );
			vs->AddObject( sv, 1.0f );
	
			dle::VSplit *vs2 = new dle::VSplit();
			vs2->SetInner( 0 );
			{
				sv = new dle::BStringView( B_UTF8_COPYRIGHT "1999-2000 by Jesper Hansen" );
				vs2->AddObject( sv, 1.0f );

				sv = new dle::BStringView( "(jesper@funcom.com)" );
				vs2->AddObject( sv, 1.0f );
			}
			vs->AddObject( vs2, 1.0f );
	
			sp = new dle::Space();
			vs->AddObject( sp, 1.0f );
	
			fConvertCB = new dle::BCheckBox( "Read BW/Gray images as truecolor", new BMessage('rd32') );
			fConvertCB->SetValue( gSettings.GetBool("converto32bit",default_converto32bit) );
			vs->AddObject( fConvertCB, 1.0f );
	
			sp = new dle::Space();
			vs->AddObject( sp, 5.0f );
	
			fRegMimeButton = new dle::BButton( "Register mimetype", new BMessage('regm') );
			vs->AddObject( fRegMimeButton, 0.5f );
	
			sp = new dle::Space();
			vs->AddObject( sp, 5.0f );
	
			dle::HSplit *hs2 = new dle::HSplit();
				sp = new dle::HSpace();
				hs2->AddObject( sp, 1.0f );
	//			sv = new dle::BStringView( "gpl", "Released under the GPL" );
	//			hs2->AddObject( sv, 1.0f );
//				fGPLButton = new dle::BButton( "showgpl", "Released under the GPL" B_UTF8_ELLIPSIS, new BMessage('sgpl') );
//				hs2->AddObject( fGPLButton, 0.5f );
				sp = new dle::HSpace( 8,8 );
				hs2->AddObject( sp, 1.0f );
			vs->AddObject( hs2, 1.0f );
		}
		asv->AddObject( vs );
	}	
	AddObject( asv );
}

ConfigView::~ConfigView()
{
}

void ConfigView::MessageReceived( BMessage *msg )
{
	switch( msg->what )
	{
#if 0
		case 'sgpl':
			if( fGPLWindow->Lock() )
			{
				fGPLWindow->Activate();
				fGPLWindow->Unlock();
			}
			else
				fGPLWindow = ShowGPL();
			break;
#endif
		case 'regm':
			RegisterMime();
			break;
			
		case 'rd32':
			gSettings.SetBool( "converto32bit", fConvertCB->Value() );
			gSettings.Write();
			break;
	}
}

void ConfigView::AttachedToWindow()
{
//	SetViewColor( Parent()->ViewColor() );
	dle::RootView::FrameResized( Bounds().Width()+1, Bounds().Height()+1 );

	fRegMimeButton->SetTarget( this );
	fConvertCB->SetTarget( this );
}

status_t MakeConfig( BMessage */*ioExtension*/, BView **outView, BRect *outExtent )
{
	ConfigView *root = new ConfigView();

	*outView = root;
	*outExtent = root->Bounds();

	return B_NO_ERROR;
}

//-----------------------------------------------------------------------------

void RegisterMime()
{
	BMimeType mime( HSIRAW_MIME );
	BMessage fileext;
//	mime.GetFileExtensions( &fileext );
	fileext.AddString( "extensions", "hsi" );
	fileext.AddString( "extensions", "raw" );
	mime.SetFileExtensions( &fileext );
	mime.SetShortDescription( "HSI Raw" );
	mime.SetLongDescription( "HSI Raw" );
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
	BApplication app( "application/x-vnd.DamnRednex-HsiRawTranslatorAddOn" );

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

