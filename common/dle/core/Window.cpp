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
#include <app/Message.h>
#include <app/MessageQueue.h>
//-------------------------------------
#include "../Window.h"
//-----------------------------------------------------------------------------

dle::Window::Window( BRect frame, const char *title, window_look look, window_feel feel, uint32 flags, uint32 workspace ) :
	BWindow( frame, title, look, feel, flags, workspace )
{
	fRoot = NULL;
	fHResizable = !((flags&B_NOT_RESIZABLE) || (flags&B_NOT_H_RESIZABLE));
	fVResizable = !((flags&B_NOT_RESIZABLE) || (flags&B_NOT_V_RESIZABLE));
}

dle::Window::~Window()
{
}

void dle::Window::FrameResized( float new_width, float new_height )
{
//	printf( "WINDOW:FRAMERESIZE:RESIZING TO: %fx%f (bounds)\n", Bounds().Width(), Bounds().Height() );
//	printf( "WINDOW:FRAMERESIZE:RESIZING TO: %fx%f (frame)\n", Frame().Width(), Frame().Height() );
//	printf( "WINDOW:FRAMERESIZE:RESIZING TO: %fx%f (args)\n", new_width, new_height );

	DisableUpdates();
	BeginViewTransaction();

	if( fRoot )
		fRoot->SetSize( Bounds() );

	EndViewTransaction();
	EnableUpdates();
}

void dle::Window::ReLayout()
{
//	ResizeChild();
//	fRoot->SetSize( Bounds() );

	dle::MinMax2 mm = fRoot->GetMinMaxSize();
//	printf( "%f %f %f %f\n", mm.hmin-1, mm.hmax-1, mm.vmin-1, mm.vmax-1 );
	SetSizeLimits( mm.horz.min-1, mm.horz.max-1, mm.vert.min-1, mm.vert.max-1 );

	float width = Bounds().Width()+1;
	float height = Bounds().Height()+1;
	
	if( width<mm.horz.min || width>mm.horz.max || height<mm.vert.min || height>mm.vert.max )
	{
		if( width>mm.horz.max ) width = mm.horz.max;
		if( width<mm.horz.min ) width = mm.horz.min;
		if( height>mm.vert.max ) height = mm.vert.max;
		if( height<mm.vert.min ) height = mm.vert.min;
//		printf( "WINDOW:RELAYOUT:RESIZING TO: %fx%f\n", width-1, height-1 );

#if 0
		printf( "MESSAGE QUE:\n" );
		BMessage *msg;
		for( int32 i=0; (msg=MessageQueue()->FindMessage(i))!=NULL; i++ )
		{
			printf( "MESSAGE #%d\n", i );
			msg->PrintToStream();
		}
#endif

		// Realy nasty: remove all pending resize's:
		BMessage *msg;
		while( (msg=MessageQueue()->FindMessage(B_WINDOW_RESIZED,0)) != NULL )
			MessageQueue()->RemoveMessage( msg );
		

		ResizeTo( width-1, height-1 );
	}
	else
	{
		fRoot->SetSize( Bounds() );
	}
}

#if 0
void dle::Window::ResizeChild()
{
//	DisableUpdates();
//	Window()->BeginViewTransaction();
#if 0
	dle::MinMax2 mm = fRoot->GetMinMaxSize();
	BRect aligned = AlignRect( Bounds(), mm, CENTER );
	fRoot->SetSize( aligned );
//	if( !fRoot->GetView()->Window() ) // FIXME: revert!
//		AddChild( fRoot->GetView() );
#else
//	fRoot->GetView()->SetViewColor( this->ViewColor() );
	fRoot->SetSize( Bounds() );
#endif
//	Window()->EndViewTransaction();
//	EnableUpdates();
}
#endif

void dle::Window::AddObject( Object *object )
{
	assert( object != NULL );
	assert( fRoot == NULL );
	
	fRoot = object;
	
	AddChild( fRoot->GetView() );

	dle::MinMax2 mm = fRoot->GetMinMaxSize();
//	printf( "%f %f %f %f\n", mm.hmin-1, mm.hmax-1, mm.vmin-1, mm.vmax-1 );
	SetSizeLimits( mm.horz.min-1, mm.horz.max-1, mm.vert.min-1, mm.vert.max-1 );
	if( fHResizable )
	{
		if( mm.horz.min==mm.horz.max )
			SetFlags( Flags() | B_NOT_H_RESIZABLE );
		else
			SetFlags( Flags() & ~B_NOT_H_RESIZABLE );
	}
	if( fVResizable )
	{
		if( mm.vert.min==mm.vert.max )
			SetFlags( Flags() | B_NOT_V_RESIZABLE );
		else
			SetFlags( Flags() & ~B_NOT_V_RESIZABLE );
	}
	
	float win_width = Bounds().Width()+1;
	float win_height = Bounds().Height()+1;
	if( win_width>mm.horz.max ) win_width = mm.horz.max;
	if( win_width<mm.horz.min ) win_width = mm.horz.min;
	if( win_height>mm.vert.max ) win_height = mm.vert.max;
	if( win_height<mm.vert.min ) win_height = mm.vert.min;

	if( Bounds().Width()!=win_width-1 || Bounds().Height()!=win_height-1 )
		ResizeTo( win_width-1, win_height-1 );
		
	fRoot->SetSize( Bounds() );
}

bool dle::Window::RemoveObject( Object *object )
{
	if( fRoot != object )
		return false;
	
	RemoveChild( fRoot->GetView() );
	fRoot = NULL;
	return true;
}

//-----------------------------------------------------------------------------
