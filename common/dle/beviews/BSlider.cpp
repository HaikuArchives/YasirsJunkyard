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
//-------------------------------------
//-------------------------------------
#include "../BSlider.h"
//-----------------------------------------------------------------------------
// Bug?: BSlider::GetPreferredSize() returns the current width as the prefered width!!!!

dle::BSlider::BSlider( const char *label, BMessage *message, int32 minvalue, int maxvalue, thumb_style thumbtype, uint32 flags ) :
	::BSlider( BRect(0,0,0,0), NULL, label, message, minvalue, maxvalue, thumbtype, B_FOLLOW_NONE, flags ),
	Object( this )
{
}

dle::BSlider::~BSlider()
{
}

dle::MinMax2 dle::BSlider::GetMinMaxSize()
{
	assert( Window() != NULL );
	
	float width;
	float height;
	GetPreferredSize( &width, &height );
//	return MinMax2( width+1,kMaxSize, height+1,height+1 );
//	printf( "BSlider:GetMinMaxSize() %p: %f %f\n", this, width, height );
	return MinMax2( 32,kMaxSize, height+1,height+1 );
}

void dle::BSlider::SetSize( const BRect &size )
{
	Object::SetSize( size );
}

//-----------------------------------------------------------------------------

void dle::BSlider::MouseDown( BPoint where )
{
	if( SendMouseEventToParent() )
		Parent()->MouseDown( ConvertToParent(where) );
	else
		::BSlider::MouseDown( where );
}

void dle::BSlider::MouseUp( BPoint where )
{
//	msg->PrintToStream();
	::BSlider::MouseUp( where );
}

//-----------------------------------------------------------------------------
