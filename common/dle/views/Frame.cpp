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
#include <support/Debug.h>
//-------------------------------------
#include "../Frame.h"
#include "../Internal.h"
//-----------------------------------------------------------------------------

dle::Frame::Frame( style border=RAISED_LOWERED ) :
	::BView( BRect(0,0,0,0), "Frame", B_FOLLOW_NONE, B_WILL_DRAW|B_FULL_UPDATE_ON_RESIZE ),
	Group( this )
{
//	fNeedInnerSpacing = false;

	fBorderStyle = border;

	fHighColor = tint_color( ui_color(B_PANEL_BACKGROUND_COLOR), B_LIGHTEN_MAX_TINT );
	fLowColor = tint_color( ui_color(B_PANEL_BACKGROUND_COLOR), B_DARKEN_3_TINT );

	fFancyHigh1Color = tint_color( ui_color(B_PANEL_BACKGROUND_COLOR), B_LIGHTEN_MAX_TINT );
	fFancyLow1Color = tint_color( ui_color(B_PANEL_BACKGROUND_COLOR), B_DARKEN_1_TINT );
	fFancyHigh2Color = tint_color( ui_color(B_PANEL_BACKGROUND_COLOR), B_NO_TINT );
	fFancyLow2Color = tint_color( ui_color(B_PANEL_BACKGROUND_COLOR), B_DARKEN_4_TINT );
}

void dle::Frame::AttachedToWindow()
{
	SetViewColor( B_TRANSPARENT_COLOR );
}

dle::MinMax2 dle::Frame::GetMinMaxSize()
{
	ASSERT_WITH_MESSAGE( fChilds.CountItems()<=1, "A Frame can only have one child" );

	float border = GetBorderSize();

	MinMax2 mm( border*2,border*2, border*2,border*2 );
		
	if( fChilds.CountItems() )
	{
		childinfo *ci = fChilds.ItemAt( 0 );
		MinMax2 childminmax = ci->object->GetMinMaxSize();
		FORCESIZE( ci, childminmax );
//		ADJUSTRECT( ci, childminmax );

		mm.horz.min += childminmax.horz.min;
		mm.horz.max += childminmax.horz.max;
		mm.vert.min += childminmax.vert.min;
		mm.vert.max += childminmax.vert.max;
	}
	
	return mm;
}

void dle::Frame::SetSize( const BRect &size )
{
	Group::SetSize( size );

	float border = GetBorderSize();
	
	if( fChilds.CountItems() )
	{
		childinfo *ci = fChilds.ItemAt( 0 );
		MinMax2 childminmax = ci->object->GetMinMaxSize();
		FORCESIZE( ci, childminmax );
//		ADJUSTRECT( ci, childminmax );

		BRect childrect;
		childrect.left = border;
		childrect.right = size.Width() - border;
		childrect.top = border;
		childrect.bottom = size.Height() - border;

//		BRect childrect = AlignRect( childrect, childminmax, ci->align );
//		DEADJUSTRECT( ci, childrect );
		
		ci->object->SetSize( childrect );
	}
}

void dle::Frame::Draw( BRect updateRect )
{
	BRect bounds = Bounds();

	switch( fBorderStyle )
	{
		case LOWERED:
			BeginLineArray( 4 );
			DrawBorder( fLowColor, fHighColor, bounds );
			EndLineArray();			
			break;

		case RAISED:
			BeginLineArray( 4 );
			DrawBorder( fHighColor, fLowColor, bounds );
			EndLineArray();			
			break;

		case FANCY_LOWERED:
			BeginLineArray( 8 );
			DrawBorder( fFancyLow1Color, fFancyHigh1Color, bounds );
			bounds.InsetBy( 1, 1 );
			DrawBorder( fFancyLow2Color, fFancyHigh2Color, bounds );
			EndLineArray();			
			break;

		case FANCY_RAISED:
			BeginLineArray( 8 );
			DrawBorder( fFancyHigh1Color, fFancyLow1Color, bounds );
			bounds.InsetBy( 1, 1 );
			DrawBorder( fFancyHigh2Color, fFancyLow2Color, bounds );
			EndLineArray();			
			break;

		default:
			ASSERT_WITH_MESSAGE( 0, "internal DLE error: unknown border style" );
	}

//	SetHighColor( fColor );
//	FillRect( Bounds() );

//	SetHighColor( 255, 0, 0 );
//	StrokeRect( Bounds() );
}

//-----------------------------------------------------------------------------

void dle::Frame::DrawBorder( const rgb_color &c1, const rgb_color &c2, const BRect &rect )
{
	AddLine( BPoint(rect.left,   rect.bottom), BPoint(rect.left,  rect.top), c1 ); // left
	AddLine( BPoint(rect.left+1, rect.top),    BPoint(rect.right, rect.top), c1 ); // top
	AddLine( BPoint(rect.right,  rect.top+1),  BPoint(rect.right, rect.bottom), c2 ); // right
	AddLine( BPoint(rect.right-1,rect.bottom), BPoint(rect.left+1,rect.bottom), c2 ); // bottom
}

float dle::Frame::GetBorderSize() const
{
	switch( fBorderStyle )
	{
		case LOWERED:
		case RAISED:
			return 1.0f;
	
		case FANCY_LOWERED:
		case FANCY_RAISED:
			return 2.0f;
			
		case LOWERED_RAISED:
		case RAISED_LOWERED:
			return 2.0f;

		default:
			ASSERT_WITH_MESSAGE( 0, "internal DLE error: unknown border style" );
			return 5.0f;
	}
}

