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
#include <string.h>
//-------------------------------------
#include <interface/Region.h>
//-------------------------------------
#include "../ProgressBar.h"
//-----------------------------------------------------------------------------

rgb_color makergb( uint8 r, uint8 g, uint8 b, uint8 a=255 ) { rgb_color col={r,g,b,a}; return col; }

//-----------------------------------------------------------------------------

dle::ProgressBar::ProgressBar() :
	BView( BRect(0,0,0,0), NULL, B_FOLLOW_NONE, B_WILL_DRAW|B_FULL_UPDATE_ON_RESIZE ),
	Object( this )
{
	fBackCol		= ui_color( B_PANEL_BACKGROUND_COLOR );
	fLightCol		= tint_color( fBackCol, B_LIGHTEN_MAX_TINT );
	fDarkCol		= tint_color( fBackCol, B_DARKEN_2_TINT );
	fBlackCol		= makergb(0,0,0);
	fProgressCol	= tint_color( fBackCol, B_DARKEN_4_TINT );
	fProgressDoneCol= makergb(152,152,255);
	
	fProgress = 0.0f;

	SetViewColor( B_TRANSPARENT_COLOR );
}

//-----------------------------------------------------------------------------

void dle::ProgressBar::SetProgress( float level )
{
	if( level<0.0f ) level=0.0f;
	if( level>1.0f ) level=1.0f;

	BRect bounds = Bounds();

	float barwidth = bounds.right-bounds.left-2;
	int oldfill = (int)floor( barwidth*fProgress+0.49f );
	int newfill = (int)floor( barwidth*level+0.49f );
	
	fProgress = level;

	if( oldfill == newfill ) return;
	
	ClipToBar( true );
	if( newfill > oldfill )
	{
		SetHighColor( fProgressDoneCol );
		FillRect( BRect(2+oldfill-1,2,2+newfill-1,bounds.bottom-1) );
	}
	else
	{
		SetHighColor( fProgressCol );
		FillRect( BRect(2+newfill-1,2,2+oldfill-1,bounds.bottom-1) );
	}
	ClipToBar( false );
}

float dle::ProgressBar::GetProgress() const
{
	return fProgress;
}

//-----------------------------------------------------------------------------

dle::MinMax2 dle::ProgressBar::GetMinMaxSize()
{
	return MinMax2( 0,kMaxSize, 0,kMaxSize );
}

void dle::ProgressBar::SetSize( const BRect &size )
{
//	printf( "dle::DebugView::SetSize:\n" );
//	size.PrintToStream();
//	Parent()->Bounds().PrintToStream();

	Object::SetSize( size );
}

void dle::ProgressBar::Draw( BRect updateRect )
{
	BRect bounds = Bounds();

	// draw frame
	ClipToBar( false );
	BeginLineArray( 6 );
	AddLine( bounds.LeftTop(), bounds.RightTop()+BPoint(-1,0), fDarkCol );	// top
	AddLine( bounds.RightTop(), bounds.RightBottom(), fLightCol );	// right
	AddLine( bounds.RightBottom()+BPoint(-1,0), bounds.LeftBottom()+BPoint(-1,0), fLightCol );	// bottom
	AddLine( bounds.LeftBottom(), bounds.LeftTop()+BPoint(0,-1), fDarkCol );	// left
	AddLine( bounds.LeftTop()+BPoint(1,1), bounds.RightTop()+BPoint(-1,1), fBlackCol );	// inner top
	AddLine( bounds.LeftTop()+BPoint(1,2), bounds.LeftBottom()+BPoint(1,-1), fBlackCol );	// inner left
	EndLineArray();

	// draw bar
	ClipToBar( true );
	float barwidth = bounds.right-bounds.left-2;
	int fill = (int)floor( barwidth*fProgress+0.49f );
	SetHighColor( fProgressDoneCol );
	FillRect( BRect(2,2,2+fill-1,bounds.bottom-1) );
	SetHighColor( fProgressCol );
	FillRect( BRect(2+fill,2,bounds.right-1,bounds.bottom-1) );
}

void dle::ProgressBar::ClipToBar( bool restrict )
{
	if( !Window() ) return;
	
	if( restrict )
	{
		BRect bounds = Bounds();
		BRegion progressregion;
		progressregion.Set( BRect(bounds.left+2,bounds.top+1,bounds.right-1,bounds.bottom-1) );
		ConstrainClippingRegion( &progressregion );
	}
	else
	{
		ConstrainClippingRegion( NULL );
	}
}


//-----------------------------------------------------------------------------

