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
#include "../Line.h"
//-----------------------------------------------------------------------------

dle::HLine::HLine( style border ) :
	BView( BRect(0,0,0,0), NULL, B_FOLLOW_NONE, B_WILL_DRAW ),
	Object( this )
{
	fBorderStyle = border;

	fHighColor = tint_color( ui_color(B_PANEL_BACKGROUND_COLOR), B_LIGHTEN_MAX_TINT );
	fLowColor = tint_color( ui_color(B_PANEL_BACKGROUND_COLOR), B_DARKEN_3_TINT );
}

dle::MinMax2 dle::HLine::GetMinMaxSize()
{
	return MinMax2( 0,kMaxSize, 2,2 );
}

void dle::HLine::Draw( BRect updateRect )
{
	BRect bounds = Bounds();

	BeginLineArray( 2 );
	AddLine( BPoint(bounds.left,bounds.top), BPoint(bounds.right,bounds.top), fHighColor );
	AddLine( BPoint(bounds.left,bounds.bottom), BPoint(bounds.right,bounds.bottom), fLowColor );
	EndLineArray();
}

//-----------------------------------------------------------------------------

dle::VLine::VLine( style border ) :
	BView( BRect(0,0,0,0), NULL, B_FOLLOW_NONE, B_WILL_DRAW ),
	Object( this )
{
	fBorderStyle = border;

	fHighColor = tint_color( ui_color(B_PANEL_BACKGROUND_COLOR), B_LIGHTEN_MAX_TINT );
	fLowColor = tint_color( ui_color(B_PANEL_BACKGROUND_COLOR), B_DARKEN_3_TINT );
}

dle::MinMax2 dle::VLine::GetMinMaxSize()
{
	return MinMax2( 2,2, 0,kMaxSize );
}

void dle::VLine::Draw( BRect updateRect )
{
	BRect bounds = Bounds();

	BeginLineArray( 2 );
	AddLine( BPoint(bounds.left,bounds.top), BPoint(bounds.left,bounds.bottom), fHighColor );
	AddLine( BPoint(bounds.right,bounds.top), BPoint(bounds.right,bounds.bottom), fLowColor );
	EndLineArray();
}

//-----------------------------------------------------------------------------
