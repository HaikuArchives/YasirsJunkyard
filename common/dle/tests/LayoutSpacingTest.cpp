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
//-------------------------------------
#include "dle/DebugView.h"
#include "dle/Split.h"
#include "dle/Window.h"
//-----------------------------------------------------------------------------

void DumpViewTree( BView *view )
{
	
}

//-----------------------------------------------------------------------------

class TestWindow : public dle::Window
{
public:
	TestWindow() : dle::Window( BRect(100,100,199,199), "DLE Test1", B_TITLED_WINDOW_LOOK, B_NORMAL_WINDOW_FEEL, 0 )
	{
		Show();
	}

	~TestWindow()
	{
	}
	
	bool QuitRequested()
	{
		be_app->PostMessage( B_QUIT_REQUESTED );
		return true;
	}
private:
};

//-----------------------------------------------------------------------------

int main()
{
	BApplication app( "application/x-vnd.dleRednex-DLESpacingLayoutTest" );
	
	TestWindow *win = new TestWindow;
	win->Lock();

#if 1
	dle::VSplit *vs = new dle::VSplit;
	vs->SetInner( 1 );
	for( int i=0; i<4; i++ )
	{
		dle::HSplit *hs = new dle::HSplit;
		for( int j=0; j<8; j++ )
		{
			dle::VSplit *vs = new dle::VSplit;
			for( int k=0; k<4; k++ )
			{
				dle::DebugView *dv = new dle::DebugView( "", 0, 0, 0 );
				vs->AddObject( dv, 1.0f );
			}
			hs->AddObject( vs, 1.0f );
		}
		vs->AddObject( hs, 1.0f );
	}
	win->AddObject( vs );
#endif

	win->Unlock();
	
	app.Run();	
	return 0;
}

//-----------------------------------------------------------------------------




