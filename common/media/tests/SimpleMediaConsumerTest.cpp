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
//#include <assert.h>
#include <stdio.h>
#include <errno.h>
//-------------------------------------
#include <app/Application.h>
//#include <interface/Bitmap.h>
//#include <interface/Screen.h>
#include <interface/Window.h>
#include <interface/View.h>
//#include <storage/File.h>
//-------------------------------------
#include "media/SimpleVideoConsumer.h"
//-----------------------------------------------------------------------------

class BitmapView : public BView
{
public:
	BitmapView( BRect rect, const char *name, BBitmap *bitmap, uint32 resizemask, uint32 flags );

	void Draw( BRect updaterect );

private:
	BBitmap	*fBitmap;
};

BitmapView::BitmapView( BRect rect, const char *name, BBitmap *bitmap, uint32 resizemask, uint32 flags ) :
	BView( rect, name, resizemask, flags|B_WILL_DRAW|B_FULL_UPDATE_ON_RESIZE )
{
	SetViewColor( B_TRANSPARENT_COLOR );

	fBitmap = bitmap;
}

void BitmapView::Draw( BRect updaterect )
{
	if( fBitmap )
		DrawBitmap( fBitmap );//, destbitmap.Bounds(), Bounds() );
}

//-----------------------------------------------------------------------------

class VideoWindow : public BWindow, public damn::VideoCapture
{
public:
	VideoWindow( BRect rect );
	~VideoWindow();

	bool QuitRequested();
	void MessageReceived( BMessage *msg );

	void NewBitmap( const BBitmap *bitmap );

private:
	BView	*fRootView;
};

VideoWindow::VideoWindow( BRect rect ) :
	BWindow( rect, "Video", B_TITLED_WINDOW_LOOK, B_NORMAL_WINDOW_FEEL, B_ASYNCHRONOUS_CONTROLS )
{
	if( damn::VideoCapture::InitCheck() < B_OK )
		printf( "VideoCapture init error\n" );
		
	fRootView = new BView( Bounds(), "RootView", B_FOLLOW_ALL, B_WILL_DRAW );
	fRootView->SetViewColor( ui_color(B_PANEL_BACKGROUND_COLOR) );
	AddChild( fRootView );

	Show();
}

VideoWindow::~VideoWindow()
{
}

bool VideoWindow::QuitRequested()
{
	be_app->PostMessage( B_QUIT_REQUESTED );
	return true;
}

void VideoWindow::MessageReceived( BMessage *msg )
{
	switch( msg->what )
	{
		default:
			msg->PrintToStream();
			BWindow::MessageReceived( msg );
	}
}

void VideoWindow::NewBitmap( const BBitmap *bitmap )
{
	Lock();
	fRootView->DrawBitmap( bitmap );
	Unlock();
}


//-----------------------------------------------------------------------------

int main( int argc, char **argv )
{
	BApplication app( "application/x-vnd.DamnRednex-SimpleMediaConsumerTest" );

//		void SetCallback( void (*callback)(const BBitmap *bitmap) ) { fCallback=callback; }

//	BBitmap screenshot( BRect(0,0,BITMAPWIDTH-1,BITMAPHEIGHT-1), B_RGB32 );
//	BRect screenshotbounds = screenshot.Bounds();
//	BScreen().ReadBitmap( &screenshot, false, &screenshotbounds );

	VideoWindow *win = new VideoWindow( BRect(100,100,100+352-1,100+288-1) );
//	win->AddChild( new BitmapScaleView(win->Bounds(), "ScaleView", &screenshot, B_FOLLOW_ALL, 0) );
	
	app.Run();

	return 0;
};

//-----------------------------------------------------------------------------
