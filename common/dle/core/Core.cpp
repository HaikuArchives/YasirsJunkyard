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

/** \defgroup DLE DamnLayoutEngine
 *  
 */

//-------------------------------------

/** \class damn::Object
 * The baseobject of DLE.
 * 
 *
 * \ingroup DLE
 */

//-------------------------------------

/** \class damn::MinMax1
 * One dimensional range.
 *
 * \ingroup DLE
 */

/** \fn damn::MinMax1::MinMax1( float min_, float max_ ) { min=min_; max=max_; }
 * Constructs the object and sets the min and max members to \a min_ and \a max_.
 *
 */

/** \var damn::MinMax1::min
 *
 * \sa damn::MinMax1::max
 */

/** \var damn::MinMax1::max
 *
 * \sa damn::MinMax1::min
 */

//-------------------------------------

/** \class damn::MinMax2
 * Two dimensional range.
 *
 * \ingroup DLE
 */

//-----------------------------------------------------------------------------

/** Constructs a DLE object.
 *
 * \param view The view to help layout (must not be NULL).
 */
damn::Object::Object( BView *view )
{
	assert( view != NULL );
	fView = view;
}

damn::Object::~Object()
{
}

void damn::Object::SetSize( const BRect &size )
{
	const BRect &bounds = fView->Frame();
	if( bounds != size )
	{
		fView->MoveTo( size.left, size.top );
		if( bounds.Width()!=size.Width() || bounds.Height()!=size.Height() )
			fView->ResizeTo( size.Width(), size.Height() );
	}
}

BView *damn::Object::GetView() const
{
	return fView;
}

damn::Object *damn::Object::GetParent() const
{
	return dynamic_cast<Object*>( fView->Parent() );
}

void damn::Object::ReLayout()
{
	ReLayoutParent();
}

void damn::Object::ReLayoutParent()
{
	BView *view = fView;
	while( view )
	{
		Object *parentobj = dynamic_cast<Object*>( view->Parent() );
		if( parentobj )
		{
			parentobj->ReLayout();
			return;
		}
	
		Root *root = dynamic_cast<Root*>( view->Parent() );
		if( root )
		{
			root->ReLayout();
			return;
		}
		
		view = view->Parent(); // Skip non DLE views
	}

	Root *root = dynamic_cast<Root*>( fView->Window() );
	if( root )
	{
		root->ReLayout();
		return;
	}
	
	fprintf( stderr, "ReLayoutParent: could not find parent\n" );
}


//-----------------------------------------------------------------------------

/** \class damn::Group
 * \ingroup DLE
 * Base for Object layout containers.
 *
 */

//-----------------------------------------------------------------------------

damn::Group::Group( BView *view ) :
	Object( view )
{
}

damn::Group::~Group()
{
}

void damn::Group::SetSpacing( float spacing )
{
	SetHSpacing( spacing, spacing*2, spacing );
	SetVSpacing( spacing, spacing*2, spacing );
}

void damn::Group::SetHSpacing( float spacing )
{
	SetHSpacing( spacing, spacing*2, spacing );
}

void damn::Group::SetHSpacing( float left, float mid, float right )
{
	assert( floor(left) == left ); // no support for unaligned spacing
	assert( floor(mid) == mid ); // no support for unaligned spacing
	assert( floor(right) == right ); // no support for unaligned spacing

	fHSpacingLeft  = left;
	fHSpacingMid   = mid;
	fHSpacingRight = right;
}

void damn::Group::SetVSpacing( float spacing )
{
	SetVSpacing( spacing, spacing*2, spacing );
}

void damn::Group::SetVSpacing( float top, float mid, float bottom )
{
	assert( floor(top) == top ); // no support for unaligned spacing
	assert( floor(mid) == mid ); // no support for unaligned spacing
	assert( floor(bottom) == bottom ); // no support for unaligned spacing

	fVSpacingTop    = top;
	fVSpacingMid    = mid;
	fVSpacingBottom = bottom;
}

//-----------------------------------------------------------------------------

//damn::Grid::Grid

//-----------------------------------------------------------------------------

//damn::Table::Table

//-----------------------------------------------------------------------------

/** \class damn::Space
 * \ingroup DLE
 * Helper object that is used to put space between other Object's.
 *
 */

//-----------------------------------------------------------------------------

damn::Space::Space() :
	BView( BRect(0,0,0,0), "space", B_FOLLOW_NONE, 0 ),
	Object( this )
{
	fMinMax = MinMax2( 0,1000000, 0,1000000 );
}

void damn::Space::SetMinMaxSize( const MinMax2 &mm )
{
	fMinMax = mm;
}

damn::MinMax2 damn::Space::GetMinMaxSize()
{
	return fMinMax;
}

void damn::Space::SetSize( const BRect &size )
{
	Object::SetSize( size );
}

//-----------------------------------------------------------------------------

