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
#include <interface/ScrollBar.h>
//-------------------------------------
#include "../AutoScrollView.h"
#include "../Internal.h"
//-----------------------------------------------------------------------------

dle::AutoScrollView::AutoScrollView() :
	BView( BRect(0,0,0,0), "AutoScrollView", B_FOLLOW_NONE, B_WILL_DRAW ),
	Object( this )
{
	fObject = NULL;

	fHorScrollBar = NULL;
	fVerScrollBar = NULL;
	fScrollView = NULL;
}

void dle::AutoScrollView::AddObject( Object *object )
{
	fObject = object;
	fView->AddChild( fObject->GetView() );
}

dle::MinMax2 dle::AutoScrollView::GetMinMaxSize()
{
	if( fObject )
	{
		MinMax2 mm = fObject->GetMinMaxSize();
		fObject->SetSize( BRect(0,0,mm.horz.min-1,mm.vert.min-1) );
		mm.horz.max = mm.horz.min;
		mm.vert.max = mm.vert.min;
		mm.horz.min = mm.horz.max<32 ? mm.horz.max : 32;
		mm.vert.min = mm.vert.max<32 ? mm.vert.max : 32;
		return mm;
	}
	else
		return MinMax2( 32,32, 32,32 );
}

void dle::AutoScrollView::SetSize( const BRect &size )
{
	Object::SetSize( size );

	if( !fObject ) return;

	dle::MinMax2 mm = fObject->GetMinMaxSize();
	BRect bounds = Bounds();
	if( mm.horz.min>bounds.Width()+1 || mm.vert.min>bounds.Height()+1 )
	{
		bool needhscrollbar = false;
		bool needvscrollbar = false;

		float width = bounds.Width()+1;
		float height = bounds.Height()+1;

		if( mm.horz.min > width ) { height-=B_H_SCROLL_BAR_HEIGHT+1; needhscrollbar=true; }
		else if( mm.horz.max < width ) width = mm.horz.max;
		if( mm.vert.min > height ) { width-=B_V_SCROLL_BAR_WIDTH+1; needvscrollbar=true; }
		else if( mm.vert.max < height ) height = mm.vert.max;
		if( !needhscrollbar && mm.horz.min>width ) { height-=B_H_SCROLL_BAR_HEIGHT+1; needhscrollbar=true; }

		if( !fScrollView )
		{
			fObject->GetView()->RemoveSelf();
			fScrollView = new BView( BRect(0,0,width-1,height-1), "HelperScrollView", B_FOLLOW_NONE, 0 );
			fScrollView->SetViewColor( this->ViewColor() );
			BView::AddChild( fScrollView );
		}
		else
		{
			fScrollView->ResizeTo( width-1, height-1 );
		}

		if( !needhscrollbar && fHorScrollBar )
		{
			fHorScrollBar->RemoveSelf();
			delete fHorScrollBar;
			fHorScrollBar = NULL;
		}
		else if( needhscrollbar )
		{
			if( !fHorScrollBar )
			{
				fHorScrollBar = new BScrollBar( BRect(0,height,width-1,height+B_H_SCROLL_BAR_HEIGHT), "hscrollbar", fScrollView, 0, mm.horz.min-width, B_HORIZONTAL );
				fScrollView->ScrollTo( 0.0f, fScrollView->Bounds().LeftTop().y );
				AddChild( fHorScrollBar );
			}
			else
				fHorScrollBar->ResizeTo( width-1, B_H_SCROLL_BAR_HEIGHT );

			fHorScrollBar->SetRange( 0, mm.horz.min-width );
			fHorScrollBar->SetProportion( width/mm.horz.min );
		}
		
		if( !needvscrollbar && fVerScrollBar )
		{
			fVerScrollBar->RemoveSelf();
			delete fVerScrollBar;
			fVerScrollBar = NULL;
		}
		else if( needvscrollbar )
		{
			if( !fVerScrollBar )
			{
				fVerScrollBar = new BScrollBar( BRect(width,0,width+B_V_SCROLL_BAR_WIDTH,height-1), "vscrollbar", fScrollView, 0, mm.vert.min-height, B_VERTICAL );
				fScrollView->ScrollTo( fScrollView->Bounds().LeftTop().x, 0.0f );
				fScrollView->ScrollTo( 0, 0 );
				AddChild( fVerScrollBar );
			}
			else
				fVerScrollBar->ResizeTo( B_V_SCROLL_BAR_WIDTH, height-1 );

			fVerScrollBar->SetRange( 0, mm.vert.min-height );
			fVerScrollBar->SetProportion( height/mm.vert.min );
		}

		BPoint rlt = fScrollView->Bounds().LeftTop();
		fObject->SetSize( BRect(0,0,needhscrollbar?mm.horz.min-1:width-1,needvscrollbar?mm.vert.min-1:height-1).OffsetBySelf(needhscrollbar?-rlt.x:0,needvscrollbar?-rlt.y:0) );
		if( !fObject->GetView()->Window() )
			fScrollView->AddChild( fObject->GetView() );
	}
	else
	{
		if( fScrollView )
		{
			fObject->GetView()->RemoveSelf();
			fScrollView->RemoveSelf();
			delete fScrollView;
			fScrollView = NULL;
		}
		if( fHorScrollBar )
		{
			fHorScrollBar->RemoveSelf();
			delete fHorScrollBar;
			fHorScrollBar = NULL;
		}
		if( fVerScrollBar )
		{
			fVerScrollBar->RemoveSelf();
			delete fVerScrollBar;
			fVerScrollBar = NULL;
		}
		
		BRect aligned = AlignRect( Bounds(), mm, CENTER );
		fObject->SetSize( aligned );
		if( !fObject->GetView()->Window() ) // FIXME: revert!
			AddChild( fObject->GetView() );
	}
}

//-----------------------------------------------------------------------------

