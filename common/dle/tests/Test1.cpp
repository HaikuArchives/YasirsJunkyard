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
#include "dle/DamnLayoutEngine.h"
//-----------------------------------------------------------------------------

void DumpViewTree( BView *view )
{
	
}

//-----------------------------------------------------------------------------

class TestWindow : public damn::Window
{
public:
	TestWindow() : damn::Window( BRect(100,100,199,199), "DLE Test1", B_TITLED_WINDOW_LOOK, B_NORMAL_WINDOW_FEEL, 0 )
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
	BApplication app( "application/x-vnd.DamnRednex-DLETest1" );
	
//	damn::VSplit *vs;
//	damn::HSplit *hs;
//	damn::DebugView *dv;
//	damn::BButton *bb;
	
	TestWindow *win = new TestWindow;
#if 1

#if 0
		vs = new damn::VSplit();
			hs = new damn::HSplit();
				dv = new damn::DebugView( "v1", 255,0,0 );
				hs->AddObject( dv );
				dv = new damn::DebugView( "v2", 0,255,0 );
				hs->AddObject( dv );
			vs->AddObject( hs );
			dv = new damn::DebugView( "v3", 0,0,255 );
			vs->AddObject( dv );
		win->AddObject( vs );
#elif 0
		hs = new damn::HSplit( 1 );
			dv = new damn::DebugView( "v1", 255,0,0 );
			dv->SetMinMaxSize( damn::MinMax2( 1,200, 1,200) );
			hs->AddObject( dv, 1.0f );

//			dv = new damn::DebugView( "v2", 0,255,0 );
////			dv->SetMinMaxSize( damn::MinMax2( 1,1000, 1,1000) );
//			dv->SetMinMaxSize( damn::MinMax2( 10,50, 10,50) );
//			hs->AddObject( dv, damn::BOTTOM, 2.0f );

			bb = new damn::BButton( "b1", "Hitme", new BMessage('hit!') );
			hs->AddObject( bb );//, damn::BOTTOM, 2.0f );

////			dv->SetMinMaxSize( damn::MinMax2( 1,1000, 1,1000) );
//			dv->SetMinMaxSize( damn::MinMax2( 10,50, 10,50) );
//			hs->AddObject( dv, damn::BOTTOM, 2.0f );
			
			dv = new damn::DebugView( "v3", 255,0,0 );
			dv->SetMinMaxSize( damn::MinMax2( 1,200, 1,200) );
			hs->AddObject( dv, 1.0f );
		win->AddObject( hs );
#elif 0
		hs = new damn::HSplit( 0 );
		for( int i=0; i<10; i++ )
		{
			vs = new damn::VSplit( 1 );
			for( int j=0; j<10; j++ )
			{
				dv = new damn::DebugView( "v1", 255,0,0 );
				dv->SetMinMaxSize( damn::MinMax2( 1,200, 1,200) );
				vs->AddObject( dv, 1.0f );
			}
			hs->AddObject( vs, 1.0f );
		}
		win->AddObject( hs );
#elif 0
		damn::VSplit *vs = new damn::VSplit( 0 );
		for( int i=0; i<8; i++ )
		{
			damn::HSplit *hs = new damn::HSplit( 0 );
			for( int j=0; j<8; j++ )
			{
				damn::VSplit *vs = new damn::VSplit( 1 );
				for( int k=0; k<8; k++ )
				{
					dv = new damn::DebugView( "v", i*255/7,j*255/7,k*255/7 );
					dv->SetMinMaxSize( damn::MinMax2( 1,200, 1,200) );
					vs->AddObject( dv, 1.0f );
				}
				hs->AddObject( vs, 1.0f );
			}
			vs->AddObject( hs, 1.0f );
		}
		win->AddObject( vs );
#else
		win->Lock();
		damn::AutoScrollView *asv = new damn::AutoScrollView;
		damn::VSplit *vs = new damn::VSplit( 0 );
		for( int i=0; i<4; i++ )
		{
			damn::HSplit *hs = new damn::HSplit( 0 );
			for( int j=0; j<8; j++ )
			{
				damn::VSplit *vs = new damn::VSplit( 1 );
				for( int k=0; k<4; k++ )
				{
					damn::BButton *bb = new damn::BButton( "but", "HitMe!", new BMessage('hit!') );
					vs->AddObject( bb, 1.0f );
				}
				hs->AddObject( vs, 1.0f );
			}
			vs->AddObject( hs, 1.0f );
		}
		asv->AddObject( vs );
		win->AddObject( asv );
		win->Unlock();
#endif
#endif
	
//	damn::MinMax2 mm = vs->GetMinMaxSize();
//	printf( "minmax: x:%f,%f y:%f,%f\n", mm.hmin,mm.hmax, mm.vmin,mm.vmax );
//	vs->SetSize( BRect(0,0,8-1,12-1) );
	
	// v1=0,0,7,5
	// v1=0,6,7,11

//	sleep( 1000 );

	app.Run();	
	return 0;
}

//-----------------------------------------------------------------------------




