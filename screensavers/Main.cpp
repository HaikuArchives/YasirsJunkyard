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
//-------------------------------------
//#include <add-ons/tracker/TrackerAddOn.h>
//#include <app/Roster.h>
//#include <storage/Entry.h>
//#include <add-ons/screen_saver/ScreenSaver.h>
#include <app/Application.h>
#include <interface/Bitmap.h>
#include <interface/View.h>
#include <interface/Window.h>
//-------------------------------------
//#include "Saver.h"
//#include "SSApp.h"
#include "Feedback.h"
#include "FeedbackWinScr.h"
//-----------------------------------------------------------------------------

class FeedbackWin : public BWindow
{
public:
	FeedbackWin( BRect rect );
	~FeedbackWin();

	bool QuitRequested();

//	void MessageReceived( BMessage *msg );

private:
	static int32 _DrawThread( void *data );
	void DrawThread();

	BBitmap				*fBitmap;	
	BView				*fView;
	Feedback			fFeedback;

	bool				fQuitThread;
	thread_id			fThread;
};

class FeedbackApp : public BApplication
{
public:
	FeedbackApp();
	~FeedbackApp();

	bool QuitRequested();
	void ReadyToRun();

private:
	BWindow	*fWindow;
};

//-----------------------------------------------------------------------------

FeedbackWin::FeedbackWin( BRect rect ) :
	BWindow( rect, "Feedback", B_TITLED_WINDOW_LOOK, B_NORMAL_WINDOW_FEEL, 0 ),
	fFeedback( B_RGB32 )
{
	fBitmap = new BBitmap( BRect(0,0,256-1,256-1), B_RGB32 );
	fView = new BView( Bounds(), "Root", B_FOLLOW_ALL, B_WILL_DRAW|B_FULL_UPDATE_ON_RESIZE );
	
	AddChild( fView );

	fQuitThread = false;
	fThread = spawn_thread( _DrawThread, "DrawThread", B_NORMAL_PRIORITY, this );
	resume_thread( fThread );

	Show();
}

FeedbackWin::~FeedbackWin()
{
	fQuitThread = true;
	status_t status;
	wait_for_thread( fThread, &status );

	delete fBitmap;

//	Hide();
//	Sync();
}

bool FeedbackWin::QuitRequested()
{
	be_app->PostMessage( B_QUIT_REQUESTED );
	return true;
}


int32 FeedbackWin::_DrawThread( void *data )
{
	((FeedbackWin*)data)->DrawThread();
	return 0;
}

void FeedbackWin::DrawThread()
{
	while( !fQuitThread )
	{
		if( LockWithTimeout(10000) == B_OK )
		{
//			printf( "Draw\n" );
			fFeedback.UpdateFrame();
			fFeedback.GetFrame( fBitmap->Bits(), fBitmap->Bounds().IntegerWidth()+1, fBitmap->Bounds().IntegerHeight()+1, fBitmap->BytesPerRow(), false );
			fView->DrawBitmap( fBitmap, fBitmap->Bounds(), fView->Bounds() );
			Unlock();
		}
	}
}

//-----------------------------------------------------------------------------

FeedbackApp::FeedbackApp() :
	BApplication( "application/x-vnd.DamnRednex-FeedbackSaver" )
{
	fWindow = NULL;
}

FeedbackApp::~FeedbackApp()
{
}

bool FeedbackApp::QuitRequested()
{
	if( fWindow && (fWindow->Lock()) )
	{
		fWindow->Quit();
		fWindow = NULL;
	}

	return true;
}

void FeedbackApp::ReadyToRun()
{
#if 1
	fWindow = new FeedbackWin( BRect(100,100,100+256-1,100+256-1) );
#else
	status_t error;
	fWindow = new FeedbackWinScr( &error );
	if( error != B_OK )
		exit( 1 );
	fWindow->Lock();
	fWindow->Show();
	fWindow->Unlock();
#endif
}

//-----------------------------------------------------------------------------

int main()
{
	FeedbackApp app;
	app.Run();
	return 0;
}

//-----------------------------------------------------------------------------
