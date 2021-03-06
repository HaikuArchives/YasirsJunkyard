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
//-------------------------------------
//-------------------------------------
#include "../BMenuField.h"
//-----------------------------------------------------------------------------

dle::BMenuField::BMenuField( BMenu *menu, uint32 flags ) :
	::BMenuField( BRect(0,0,0,0), NULL, NULL, menu, false, (uint32)B_FOLLOW_NONE, flags|B_FRAME_EVENTS ),
	Object( this )
{
	SetDivider( 0.0f );
}

dle::BMenuField::~BMenuField()
{
}

void dle::BMenuField::FrameResized( float new_width, float new_height )
{
	ReLayout();
}

// The BMenuField resizes iteself, so the initial size does not work :(
// If there just were a way to get the largest possible size of the BMenuField...
dle::MinMax2 dle::BMenuField::GetMinMaxSize()
{
	float width;
	float height;
	GetPreferredSize( &width, &height );
//	printf( "BMenuField:GetMinMaxSize() %p: %f %f\n", this, width, height );
//	ASSERT( width == 0 );
	return MinMax2( width+1,width+1, height+1,height+1 );
}

void dle::BMenuField::SetSize( const BRect &size )
{
	Object::SetSize( size );
}

//-----------------------------------------------------------------------------

void dle::BMenuField::MouseDown( BPoint where )
{
	if( SendMouseEventToParent() )
		Parent()->MouseDown( ConvertToParent(where) );
	else
		::BMenuField::MouseDown( where );
}

void dle::BMenuField::MouseUp( BPoint where )
{
//	msg->PrintToStream();
	::BMenuField::MouseUp( where );
}

//-----------------------------------------------------------------------------
