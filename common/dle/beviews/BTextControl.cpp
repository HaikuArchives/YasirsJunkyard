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
#include "../BTextControl.h"
//-----------------------------------------------------------------------------

dle::BTextControl::BTextControl( const char *initial_text, BMessage *message, uint32 flags ) :
	::BTextControl( BRect(0,0,0,0), NULL, NULL, initial_text, message, B_FOLLOW_NONE, flags ),
	Object( this )
{
	SetDivider( -3 ); // BeOS adds three pixels of nothin in front of a textcontrol... bad OS!
}

dle::BTextControl::~BTextControl()
{
}

dle::MinMax2 dle::BTextControl::GetMinMaxSize()
{
//	float stringwidth = StringWidth( Text() );
//	font_height fontheight;
//	GetFontHeight( &fontheight );
//	float height = 1 + ceil(fontheight.ascent+fontheight.descent+fontheight.leading) + 1;

//	return MinMax2( 1+stringwidth+1,1+stringwidth+1, height,height );

	float width;
	float height;
	GetPreferredSize( &width, &height );
//	return MinMax2( width+1,kMaxSize, height+1,height+1 );
	return MinMax2( 100,kMaxSize, height+1,height+1 );
}

void dle::BTextControl::SetSize( const BRect &size )
{
	Object::SetSize( size );
}

//-----------------------------------------------------------------------------

