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
#include <math.h>
//-------------------------------------
#include <support/Debug.h>
//-------------------------------------
#include "../BBox.h"
#include "../Internal.h"
//-----------------------------------------------------------------------------

dle::BBox::BBox( const char *title, border_style border=B_FANCY_BORDER ):
	::BBox( BRect(0,0,0,0), "bbox", B_FOLLOW_NONE,  B_WILL_DRAW|B_FRAME_EVENTS|B_NAVIGABLE_JUMP, border ),
	Group( this )
{
	if( title )
		SetLabel( title );
}

dle::BBox::~BBox()
{
}

dle::MinMax2 dle::BBox::GetMinMaxSize()
{
	ASSERT_WITH_MESSAGE( fChilds.CountItems()<=1, "A BBox can have maximum one child" );

	BRect frame;
	BPoint label;
	GetFrameSize( &frame, &label );

	MinMax2 mm(
		frame.left+frame.right,
		frame.left+frame.right,
		frame.top+frame.bottom,
		frame.top+frame.bottom );
		
	if( fChilds.CountItems() )
	{
		childinfo *ci = fChilds.ItemAt( 0 );
		MinMax2 childminmax = ci->object->GetMinMaxSize();
		ADJUSTRECT( ci, childminmax );

		mm.horz.min += childminmax.horz.min;
		mm.horz.max += childminmax.horz.max;
		mm.vert.min += childminmax.vert.min;
		mm.vert.max += childminmax.vert.max;
	}
	
	float minwidth = label.x+frame.left+frame.right;
	float minheight = max_c(label.y,frame.top)+frame.bottom;
	
	if( mm.horz.min < minwidth ) mm.horz.min = minwidth;
	if( mm.horz.max < minwidth ) mm.horz.max = minwidth;
	if( mm.vert.min < minheight ) mm.vert.min = minheight;
	if( mm.vert.max < minheight ) mm.vert.max = minheight;

//	printf( "****** BBOX:GET:" ); mm.PrintToStream();

	return mm;
}

void dle::BBox::SetSize( const BRect &size )
{
	Group::SetSize( size );

//	printf( "****** BBOX:SET:" ); size.PrintToStream();
	
	BRect frame;
	BPoint label;
	GetFrameSize( &frame, &label );
	
	if( fChilds.CountItems() )
	{
		childinfo *ci = fChilds.ItemAt( 0 );
		MinMax2 childminmax = ci->object->GetMinMaxSize();
		ADJUSTRECT( ci, childminmax );

		BRect childrect;
		childrect.left = frame.left;
		childrect.right = size.Width() - frame.right;
		childrect.top = frame.top;
		childrect.bottom = size.Height() - frame.bottom;

		BRect achildrect = AlignRect( childrect, childminmax, ci->align );
		DEADJUSTRECT( ci, achildrect );
		
		ci->object->SetSize( achildrect );
	}
}

//-----------------------------------------------------------------------------

void dle::BBox::GetFrameSize( BRect *framesize, BPoint *labelsize )
{
	framesize->left = 2;
	framesize->right = 2;
	framesize->bottom = 2;

//	BFont font;
//	GetFont( &font );
//	font.PrintToStream();

	if( Label() )
	{
		const char *label = Label();

		float strwidth = ceil(StringWidth( label ));

		font_height fontheight;
		GetFontHeight( &fontheight );
		float strheight = ceil(fontheight.ascent + fontheight.descent) - 3.0f; // seems safe with all the system fonts...
		
		framesize->top = strheight;

		labelsize->x = 6+4 + strwidth + 1+6;
		labelsize->y = strheight;
	}
	else
	{
		framesize->top = 2;
		labelsize->x = 0;
		labelsize->y = 0;
	}
	// TODO: add LabelView...
}

//-----------------------------------------------------------------------------


