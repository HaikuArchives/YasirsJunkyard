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
//-------------------------------------
#include <app/Application.h>
#include <app/Message.h>
#include <interface/StringView.h>
#include <interface/View.h>
//-------------------------------------
#include "ProgressBar.h"
#include "MIWin.h"
//-----------------------------------------------------------------------------

MIWin::MIWin() :
	BWindow( BRect(100,100,100+WIN_WIDTH-1,100+WIN_HEIGHT-1), "MakeIcon", B_MODAL_WINDOW_LOOK, B_NORMAL_WINDOW_FEEL, B_NOT_V_RESIZABLE|B_AVOID_FOCUS )
{
	BView *root = new BView( Bounds(), "Root", B_FOLLOW_ALL, 0 );
	root->SetViewColor( ui_color(B_PANEL_BACKGROUND_COLOR) );
	AddChild( root );

	fFilename = new BStringView( BRect(10,10,WIN_WIDTH-10-1,20-1), "Filename", "", B_FOLLOW_LEFT_RIGHT );
	root->AddChild( fFilename );
//	fFilename->ResizeToPrefered();
	float width, height;
	fFilename->GetPreferredSize( &width, &height );
	fFilename->ResizeTo( WIN_WIDTH-20-1, height-1 );

	fProgressBar = new RProgressBar( BRect(10,10+height+5,WIN_WIDTH-10-1,10+height+5+10-1), "Progress", B_FOLLOW_LEFT_RIGHT );
	root->AddChild( fProgressBar );
	
	ResizeTo( WIN_WIDTH-1, 10+height+5+10+10-1 );
	
//	new BMessageRunner( this, new BMessage('tick'), 1000000/20 );

	Show();
}

MIWin::~MIWin()
{
}

void MIWin::SetProgress( const char *filename, float progress )
{
	fFilename->SetText( filename );
	fProgressBar->SetProgress( progress );
	Flush();
}

void MIWin::MessageReceived( BMessage *message )
{
	switch( message->what )
	{
		case 'tick':
		{
//			float v = float(rand())/float(RAND_MAX);
//			fProgressBar->SetProgress( v );
			break;
		}
	}
}

bool MIWin::QuitRequested()
{
	be_app->PostMessage( B_QUIT_REQUESTED );
	return true;
}

//-----------------------------------------------------------------------------

