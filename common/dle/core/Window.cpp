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
//-------------------------------------
//-------------------------------------
#include "../DamnLayoutEngine.h"
//-----------------------------------------------------------------------------

damn::Window::Window( BRect frame, const char *title, window_look look, window_feel feel, uint32 flags, uint32 workspace ) :
	BWindow( frame, title, look, feel, flags, workspace )
{
	fRoot = NULL;
}

damn::Window::~Window()
{
}

void damn::Window::FrameResized( float /*new_width*/, float /*new_height*/ )
{
	DisableUpdates();
	BeginViewTransaction();

	if( fRoot )
		fRoot->SetSize( Bounds() );

	EndViewTransaction();
	EnableUpdates();
}

void damn::Window::AddObject( Object *object )
{
	assert( object != NULL );
	assert( fRoot == NULL );
	
	fRoot = object;

	AddChild( fRoot->GetView() );

	damn::MinMax2 mm = fRoot->GetMinMaxSize();
//	printf( "%f %f %f %f\n", mm.hmin-1, mm.hmax-1, mm.vmin-1, mm.vmax-1 );
	SetSizeLimits( mm.hmin-1, mm.hmax-1, mm.vmin-1, mm.vmax-1 );
	
	float win_width = Bounds().Width()+1;
	float win_height = Bounds().Height()+1;
	if( win_width>mm.hmax ) win_width = mm.hmax;
	if( win_width<mm.hmin ) win_width = mm.hmin;
	if( win_height>mm.vmax ) win_height = mm.vmax;
	if( win_height<mm.vmin ) win_height = mm.vmin;
	ResizeTo( win_width-1, win_height-1 );

	fRoot->SetSize( Bounds() );
}

//-----------------------------------------------------------------------------
