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
//-------------------------------------
#include "../DebugView.h"
//-----------------------------------------------------------------------------

dle::DebugView::DebugView( const char *name, uchar r, uchar g, uchar b, uchar a ) :
	BView( BRect(0,0,0,0), name, B_FOLLOW_NONE, B_WILL_DRAW|B_FULL_UPDATE_ON_RESIZE ),
	Object( this )
{
	fColor.red = r;
	fColor.green = g;
	fColor.blue = b;
	fColor.alpha = a;

//	fNeedInnerSpacing = false;

	fName = name;
}

void dle::DebugView::AttachedToWindow()
{
	SetViewColor( B_TRANSPARENT_COLOR );
}

dle::MinMax2 dle::DebugView::GetMinMaxSize()
{
	return MinMax2( 0,kMaxSize, 0,kMaxSize );
}

void dle::DebugView::SetSize( const BRect &size )
{
//	printf( "dle::DebugView::SetSize:\n" );
//	size.PrintToStream();
//	Parent()->Bounds().PrintToStream();

	Object::SetSize( size );
}

void dle::DebugView::Draw( BRect updateRect )
{
	SetHighColor( fColor );
	FillRect( Bounds() );

	SetHighColor( 255, 0, 0 );
	StrokeRect( Bounds() );

}

//-----------------------------------------------------------------------------

