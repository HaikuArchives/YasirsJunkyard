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
#include <stdio.h>
//-------------------------------------
#include <interface/ScrollBar.h>
#include <interface/Window.h>
//-------------------------------------
#include "../RootView.h"
//-----------------------------------------------------------------------------

dle::RootView::RootView( BRect frame, uint32 resizeMask, uint32 flags ) :
	BView( frame, "rootview", resizeMask, flags )
{
	fRoot = NULL;
}

dle::RootView::~RootView()
{
}
		
void dle::RootView::FrameResized( float /*new_width*/, float /*new_height*/ )
{
	ResizeChild();
}

void dle::RootView::ReLayout()
{
	ResizeChild();
}

void dle::RootView::ResizeChild()
{
//	DisableUpdates();
	Window()->BeginViewTransaction();
#if 0
	dle::MinMax2 mm = fRoot->GetMinMaxSize();
	BRect aligned = AlignRect( Bounds(), mm, CENTER );
	fRoot->SetSize( aligned );
//	if( !fRoot->GetView()->Window() ) // FIXME: revert!
//		AddChild( fRoot->GetView() );
#else
	fRoot->GetView()->SetViewColor( this->ViewColor() );
	fRoot->SetSize( Bounds() );
#endif
	Window()->EndViewTransaction();
//	EnableUpdates();
}

void dle::RootView::AddObject( Object *object )
{
	assert( object != NULL );
	assert( fRoot == NULL );
	
	fRoot = object;

//	fRoot->GetView()->SetViewColor( this->ViewColor() );
	AddChild( fRoot->GetView() );

#if 0
	dle::MinMax2 mm = fRoot->GetMinMaxSize();
//	SetSizeLimits( mm.hmin-1, mm.hmax-1, mm.vmin-1, mm.vmax-1 );
	
	float view_width = Bounds().Width()+1;
	float view_height = Bounds().Height()+1;
	if( view_width>mm.hmax ) view_width = mm.hmax;
	if( view_width<mm.hmin ) view_width = mm.hmin;
	if( view_height>mm.vmax ) view_height = mm.vmax;
	if( view_height<mm.vmin ) view_height = mm.vmin;

	ResizeTo( view_width-1, view_height-1 );
	fRoot->SetSize( Bounds() );
#endif
//	printf( "Unaligned:" ); BRect(0,0,view_width-1,view_height-1).PrintToStream();
//	BRect aligned = AlignRect( BRect(0,0,view_width-1,view_height-1), mm, CENTER );
//	printf( "Aligned:" ); aligned.PrintToStream();
//	ResizeTo( aligned.Width()+1, aligned.Height()+1 );
//	fRoot->SetSize( Bounds() );
}

//-----------------------------------------------------------------------------
