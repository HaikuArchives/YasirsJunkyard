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
#include <assert.h>
//-------------------------------------
#include <app/Application.h>
#include <interface/Bitmap.h>
#include <interface/Screen.h>
#include <interface/Window.h>
#include <interface/View.h>
#include <storage/File.h>
//-------------------------------------
#include "gfx/BitmapScale.h"
//-----------------------------------------------------------------------------

#define BITMAPWIDTH 256
#define BITMAPHEIGHT 256

//-----------------------------------------------------------------------------

class BitmapScaleView : public BView
{
public:
	BitmapScaleView( BRect rect, const char *name, BBitmap *bitmap, uint32 resizemask, uint32 flags );

	void Draw( BRect updaterect );

private:
	BBitmap	*fBitmap;
};

BitmapScaleView::BitmapScaleView( BRect rect, const char *name, BBitmap *bitmap, uint32 resizemask, uint32 flags ) :
	BView( rect, name, resizemask, flags|B_WILL_DRAW|B_FULL_UPDATE_ON_RESIZE )
{
	SetViewColor( B_TRANSPARENT_COLOR );

	fBitmap = bitmap;
}

void BitmapScaleView::Draw( BRect updaterect )
{
//	filter_filter filter_box filter_triangle filter_bell filter_bspline filter_lanczos3 filter_mitchell

	BBitmap destbitmap( Bounds(), B_RGB32 );
	Scale( fBitmap, &destbitmap, filter_lanczos3 );
	DrawBitmap( &destbitmap );//, destbitmap.Bounds(), Bounds() );
}

//-----------------------------------------------------------------------------

int main( int argc, char **argv )
{
	BApplication app( "application/x-vnd.DamnRednex-BitmapScaleTest" );

	BBitmap screenshot( BRect(0,0,BITMAPWIDTH-1,BITMAPHEIGHT-1), B_RGB32 );
	BRect screenshotbounds = screenshot.Bounds();
	BScreen().ReadBitmap( &screenshot, false, &screenshotbounds );

	BWindow *win = new BWindow( BRect(100,100,100+BITMAPWIDTH-1,100+BITMAPHEIGHT-1), "BitmapScale test", B_TITLED_WINDOW_LOOK, B_NORMAL_WINDOW_FEEL, 0 );
	win->AddChild( new BitmapScaleView(win->Bounds(), "ScaleView", &screenshot, B_FOLLOW_ALL, 0) );
	win->Show();
	
	app.Run();

	return 0;
};

//-----------------------------------------------------------------------------
