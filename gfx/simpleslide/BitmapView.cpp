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
#include <interface/Bitmap.h>
#include <interface/ScrollBar.h>
//-------------------------------------
#include "BitmapView.h"
//-----------------------------------------------------------------------------

BitmapView::BitmapView( BRect rect ) :
	BView( rect, "bitmapview", B_FOLLOW_ALL, B_WILL_DRAW | B_FRAME_EVENTS )
{
	fBitmap = NULL;
	fOwnership = true;
	SetViewColor( 220, 220, 220 );
}


BitmapView::~BitmapView()
{
	if( fOwnership && fBitmap )
		delete fBitmap;
}


void BitmapView::SetOwnership( bool ownership )
{
	fOwnership = ownership;
}


bool BitmapView::GetOwnership()
{
	return fOwnership;
}


void BitmapView::SetBitmap( BBitmap *map )
{
	if( fOwnership && fBitmap )
		delete fBitmap;
	fBitmap = map;
	Invalidate();
	AdjustBars();
}

BBitmap *BitmapView::GetBitmap()
{
	return fBitmap;
}


void BitmapView::Draw( BRect area )
{
	if( fBitmap ) DrawBitmap( fBitmap );
}

void BitmapView::FrameResized( float newWidth, float newHeight )
{
	BView::FrameResized(newWidth, newHeight);
	AdjustBars();
}

void BitmapView::WindowActivated( bool state )
{
	BView::WindowActivated( state );
	AdjustBars();
}


void BitmapView::AdjustBar( BScrollBar *bar, float page, float total )
{
	if( total <= page )
	{
		bar->SetRange( 0.0f, 0.0f );
		bar->SetProportion( 1.0f );
	}
	else
	{
		bar->SetRange( 0.0f, total-page );
		bar->SetProportion( page/total );
	}

	float pgStep = page-4.0;
	if( pgStep<16.0 ) pgStep = 16.0;
	bar->SetSteps( 4.0, pgStep );
}

void BitmapView::AdjustBars()
{
	BRect bitmapbounds( 0, 0, 0, 0 ) ;

	if( fBitmap ) bitmapbounds = fBitmap->Bounds();

	BScrollBar *bar = ScrollBar( B_HORIZONTAL );
	if( bar ) AdjustBar( bar, Bounds().Width()+1, bitmapbounds.Width()+1 );

	bar = ScrollBar( B_VERTICAL );
	if( bar ) AdjustBar( bar, Bounds().Height()+1, bitmapbounds.Height()+1 );
}


//-----------------------------------------------------------------------------
