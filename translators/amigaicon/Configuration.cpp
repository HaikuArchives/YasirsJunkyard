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
#include <interface/MenuBar.h>
#include <interface/MenuField.h>
#include <interface/MenuItem.h>
#include <interface/PopUpMenu.h>
#include <storage/Mime.h>
#include <support/String.h>
#include <translation/TranslatorAddOn.h>
//-------------------------------------
#include "dle/DamnLayoutEngine.h"
#include "misc/Settings.h"
#include "AmigaIconTranslator.h"
//-----------------------------------------------------------------------------

class ConfigView : public damn::RootView
{
public:
					ConfigView();
					~ConfigView();

			void	MessageReceived( BMessage *msg );
			void	AttachedToWindow();

private:
	damn::BButton		*fRegMimeButton;
	damn::BMenuField	*fIconMF;
	damn::BMenuField	*fPaletteMF;
};

//-----------------------------------------------------------------------------

ConfigView::ConfigView() :
	damn::RootView( BRect(0,0,100-1,100-1), B_FOLLOW_ALL, 0 )
{
	damn::BStringView *sv;
	damn::Space *sp;
	damn::HSplit *hs;
	BPopUpMenu *popmenu;

	damn::AutoScrollView *asv = new damn::AutoScrollView;

		damn::VSplit *vs = new damn::VSplit( 1 );
	
			sv = new damn::BStringView( "heading", translatorName );
			sv->SetFont( be_bold_font );
			vs->AddObject( sv, 1.0f );
	
			BString version; version<<"Amiga icon translator v"<<(int32)VERSION_MAJOR<<"."<<(int32)VERSION_MINOR<<"."<<(int32)VERSION_REVISION<<"  "<<__DATE__;
			sv = new damn::BStringView( "version", version.String() );
			vs->AddObject( sv, 1.0f );
	
			sv = new damn::BStringView( "copyright", B_UTF8_COPYRIGHT "'99 by Jesper Hansen (jesper@funcom.com)" );
			vs->AddObject( sv, 1.0f );
	
			sp = new damn::Space();
			vs->AddObject( sp, 1.0f );
	
	//		fReadSelCB = new damn::BCheckBox( "readsel", "Read selected icon", new BMessage('rdsl') );
	//		fReadSelCB->SetValue( gSettings.GetBool("readselected",default_readselected) );
	//		vs->AddObject( fReadSelCB, 1.0f );
	
			hs = new damn::HSplit( 0 );
				sp = new damn::HSpace();
				hs->AddObject( sp, 1.0f );
				sv = new damn::BStringView( "icon", "Icon:" );
				hs->AddObject( sv, 1.0f );
				popmenu = new BPopUpMenu( "" );
				popmenu->AddItem( new BMenuItem("Unselected", new BMessage('rius')) );
				popmenu->AddItem( new BMenuItem("Selected", new BMessage('risl')) );
				popmenu->AddItem( new BMenuItem("Both", new BMessage('ribt')) );
				fIconMF = new damn::BMenuField( "icon", popmenu );
	//			fIconMF->SetLabel( fIconMF->Menu()->ItemAt( 1 )->Label() );
				fIconMF->Menu()->Superitem()->SetLabel( fIconMF->Menu()->ItemAt( gSettings.GetInt32("whichicon",DEFAULT_WHICHICON) )->Label() );
				hs->AddObject( fIconMF, 1.0f );
				sp = new damn::HSpace();
				hs->AddObject( sp, 1.0f );
			vs->AddObject( hs, 1.0f );
	
			hs = new damn::HSplit( 0 );
				sp = new damn::HSpace();
				hs->AddObject( sp, 1.0f );
				sv = new damn::BStringView( "palette", "Palette:" );
				hs->AddObject( sv, 1.0f );
				popmenu = new BPopUpMenu( "" );
				popmenu->AddItem( new BMenuItem("AmigaOS 1.x", new BMessage('pa1x')) );
				popmenu->AddItem( new BMenuItem("AmigaOS 2.x", new BMessage('pa2x')) );
				popmenu->AddItem( new BMenuItem("MagicWB", new BMessage('pamw')) );
				fPaletteMF = new damn::BMenuField( "palette", popmenu );
	//			fPaletteMF->SetLabel( fPaletteMF->Menu()->ItemAt( 1 )->Label() );
				fPaletteMF->Menu()->Superitem()->SetLabel( fPaletteMF->Menu()->ItemAt( gSettings.GetInt32("palette",DEFAULT_PALETTE) )->Label() );
				hs->AddObject( fPaletteMF, 1.0f );
				sp = new damn::HSpace();
				hs->AddObject( sp, 1.0f );
			vs->AddObject( hs, 1.0f );
	
			sp = new damn::Space();
			vs->AddObject( sp, 5.0f );
	
			fRegMimeButton = new damn::BButton( "regmime", "Register mimetype", new BMessage('regm') );
			vs->AddObject( fRegMimeButton, 0.5f );
	
			sp = new damn::Space();
			vs->AddObject( sp, 5.0f );
	
			hs = new damn::HSplit( 0 );
				sp = new damn::HSpace();
				hs->AddObject( sp, 1.0f );
	//			sv = new damn::BStringView( "gpl", "Released under the GPL" );
	//			hs2->AddObject( sv, 1.0f );
//				fGPLButton = new damn::BButton( "showgpl", "Released under the GPL" B_UTF8_ELLIPSIS, new BMessage('sgpl') );
//				hs->AddObject( fGPLButton, 0.5f );
				sp = new damn::HSpace( 8,8 );
				hs->AddObject( sp, 1.0f );
			vs->AddObject( hs, 1.0f );

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
	
		case 'rius': gSettings.SetInt32( "whichicon", 0 ); gSettings.Write(); break;
		case 'risl': gSettings.SetInt32( "whichicon", 1 ); gSettings.Write(); break;
		case 'ribt': gSettings.SetInt32( "whichicon", 2 ); gSettings.Write(); break;
		
		case 'pa1x': gSettings.SetInt32( "palette", 0 ); gSettings.Write(); break;
		case 'pa2x': gSettings.SetInt32( "palette", 1 ); gSettings.Write(); break;
		case 'pamw': gSettings.SetInt32( "palette", 2 ); gSettings.Write(); break;
	}
}

void ConfigView::AttachedToWindow()
{
//	SetViewColor( Parent()->ViewColor() );
	damn::RootView::FrameResized( Bounds().Width()+1, Bounds().Height()+1 );

//	fGPLButton->SetTarget( this );
	fRegMimeButton->SetTarget( this );

	for( int i=fIconMF->Menu()->CountItems()-1; i>=0; i-- )
		fIconMF->Menu()->ItemAt(i)->SetTarget( this );

	for( int i=fIconMF->Menu()->CountItems()-1; i>=0; i-- )
		fPaletteMF->Menu()->ItemAt(i)->SetTarget( this );
}

status_t MakeConfig( BMessage */*ioExtension*/, BView **outView, BRect *outExtent )
{
	ConfigView *root = new ConfigView();

	*outView = root;
	*outExtent = root->Bounds();

	return B_NO_ERROR;
}

//-----------------------------------------------------------------------------

