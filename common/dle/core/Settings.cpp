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
#include <interface/InterfaceDefs.h>
//-------------------------------------
#include "../Core.h"
#include "../Internal.h"
//-----------------------------------------------------------------------------

dle::DefaultSettings gDefault;

#define FINDPARENT( cond ) \
	const Settings *obj = this; \
	while( true ) \
	{ \
		if( obj->cond ) break; \
		const Settings *parent = obj->GetParentSettings(); \
		if( parent==NULL ) { obj = &gDefault; break; } \
		obj = parent; \
	}

//-----------------------------------------------------------------------------

dle::Settings::Settings()
{
	fHaveColor = false;
	fHaveInnerLeft = false;
	fHaveInnerRight = false;
	fHaveInnerTop = false;
	fHaveInnerBottom = false;

	fHaveForcedMinWidth = false;
	fHaveForcedMaxWidth = false;
	fHaveForcedMinHeight = false;
	fHaveForcedMaxHeight = false;
	fForcedSize.SetTo( 0, 0, 0, 0 );

	fWantInnerSpacing = true;

}

dle::DefaultSettings::DefaultSettings()
{
	fColor = ui_color( B_PANEL_BACKGROUND_COLOR );
	fHaveColor = true;

	fInnerLeft = 4;
	fHaveInnerLeft = true;

	fInnerRight = 4;
	fHaveInnerRight = true;

	fInnerTop = 4;
	fHaveInnerTop = true;

	fInnerBottom = 4;
	fHaveInnerBottom = true;
}

const dle::Settings *dle::DefaultSettings::GetParentSettings() const
{
	assert( 0 );
	return NULL;
}


//-----------------------------------------------------------------------------

void dle::Settings::SetColor( const rgb_color &color )
{
	fHaveColor = true;
	fColor = color;
}

void dle::Settings::UnsetColor()
{
	fHaveColor = false;
}

const rgb_color &dle::Settings::GetColor() const
{
	FINDPARENT( fHaveColor );
	return obj->fColor;
}

//-----------------------------------------------------------------------------

void dle::Settings::SetInner( float inner )
{
	fHaveInnerLeft = fHaveInnerRight = fHaveInnerTop = fHaveInnerBottom = true;
	fInnerLeft = fInnerRight = fInnerTop = fInnerBottom = inner;
}

void dle::Settings::UnsetInner()
{
	fHaveInnerLeft = fHaveInnerRight = fHaveInnerTop = fHaveInnerBottom = false;
}

#define INNER( _inner ) \
void dle::Settings::SetInner ## _inner( float inner ) \
{ \
	fHaveInner ## _inner = true; \
	fInner ## _inner = inner; \
} \
void dle::Settings::UnsetInner ## _inner() \
{ \
	fHaveInner ## _inner = false; \
} \
float dle::Settings::GetInner ## _inner() const \
{ \
	FINDPARENT( fHaveInner ## _inner ); \
	return obj->fInner ## _inner; \
}

INNER( Left )
INNER( Right )
INNER( Top )
INNER( Bottom )

//-----------------------------------------------------------------------------

void dle::Settings::ForceWidth( const MinMax1 &minmax )
{
	ForceMinWidth( minmax.min );
	ForceMaxWidth( minmax.max );
}
void dle::Settings::ForceHeight( const MinMax1 &minmax )
{
	ForceMinHeight( minmax.min );
	ForceMaxHeight( minmax.max );
}

void dle::Settings::ForceSize( const MinMax2 &minmax )
{
	ForceWidth( minmax.horz );
	ForceHeight( minmax.vert );
}

void dle::Settings::ForceWidth( float width )
{
	ForceWidth( MinMax1(width,width) );
}
void dle::Settings::ForceHeight( float height )
{
	ForceHeight( MinMax1(height,height));
}

//--------------------------------------

const dle::MinMax2 &dle::Settings::GetForcedSize() const
{
	return fForcedSize;
}

//--------------------------------------
	
#if 1

float dle::Settings::ForcedClipHorz( float width )
{
	if( fHaveForcedMinWidth && width<fForcedSize.horz.min ) return fForcedSize.horz.min;
	if( fHaveForcedMaxWidth && width>fForcedSize.horz.max ) return fForcedSize.horz.max;
	return width;
}
float dle::Settings::ForcedClipVert( float height )
{
	if( fHaveForcedMinHeight && height<fForcedSize.vert.min)  return fForcedSize.vert.min;
	if( fHaveForcedMaxHeight && height>fForcedSize.vert.max ) return fForcedSize.vert.max;
	return height;
}

dle::MinMax1 dle::Settings::ForcedClipHorz( const MinMax1 &minmax )
{
//	return MinMax1( ForcedClipHorz(minmax.min), ForcedClipHorz(minmax.max) );
	float min = ForcedClipHorz( minmax.min );
	float max = ForcedClipHorz( minmax.max );
	return MinMax1( min, max );
}
dle::MinMax1 dle::Settings::ForcedClipVert( const MinMax1 &minmax )
{
//	return MinMax1( ForcedClipVert(minmax.min), ForcedClipVert(minmax.max) );
	float min = ForcedClipVert( minmax.min );
	float max = ForcedClipVert( minmax.max );
	return MinMax1( min, max );
}

#else

dle::MinMax1 dle::Settings::ForcedClipHorz( const MinMax1 &minmax )
{
	return MinMax1(
		fHaveForcedMinWidth ? fForcedSize.horz.min : minmax.min,
		fHaveForcedMaxWidth ? fForcedSize.horz.max : minmax.max );
}
dle::MinMax1 dle::Settings::ForcedClipVert( const MinMax1 &minmax )
{
	return MinMax1(
		fHaveForcedMinHeight ? fForcedSize.vert.min : minmax.min,
		fHaveForcedMaxHeight ? fForcedSize.vert.max : minmax.max );
}

#endif

dle::MinMax2 dle::Settings::ForcedClip( const MinMax2 &minmax )
{
//	return MinMax2( ForcedClipHorz(minmax.horz), ForcedClipVert(minmax.vert) );
	MinMax1 horz = ForcedClipHorz( minmax.horz );
	MinMax1 vert = ForcedClipVert( minmax.vert );
	return MinMax2( horz, vert );
}

//-----------------------------------------------------------------------------

