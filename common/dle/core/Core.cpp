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
#include <app/Message.h>
#include <interface/View.h>
#include <interface/Window.h>
#include <support/Debug.h>
//-------------------------------------
#include "../Core.h"
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

void dle::MinMax2::PrintToStream() const
{
	printf( "MinMax2(horz:%.1f-%.1f vert:%.1f-%.1f)\n", horz.min, horz.max, vert.min, vert.max );
}

//-----------------------------------------------------------------------------

/** Constructs a DLE object.
 *
 * \param view The view to help layout (must not be NULL).
 */
dle::Object::Object( BView *view )
{
	ASSERT_WITH_MESSAGE( view!=NULL, "The view can not be NULL" );
	fView = view;
}

dle::Object::~Object()
{
}

void dle::Object::SetSize( const BRect &size )
{
	const BRect &bounds = fView->Frame();
	if( bounds != size )
	{
		fView->MoveTo( size.left, size.top );
		if( bounds.Width()!=size.Width() || bounds.Height()!=size.Height() )
			fView->ResizeTo( size.Width(), size.Height() );
	}
}

BView *dle::Object::GetView() const
{
	return fView;
}

//dle::Object *dle::Object::GetParent() const
//{
//	return dynamic_cast<Object*>( fView->Parent() );
//}

void dle::Object::ReLayout()
{
	ReLayoutParent();
}

const dle::Settings *dle::Object::GetParentSettings() const
{
	BView *view = fView;
	while( view )
	{
		Settings *parentsettings = dynamic_cast<Settings*>( view->Parent() );
		if( parentsettings )
			return parentsettings;

		view = view->Parent(); // Skip non DLE views
	}

	Settings *root = dynamic_cast<Settings*>( fView->Window() );
	if( root )
		return root;

	return NULL;
}

void dle::Object::ReLayoutParent()
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

bool dle::Object::SendMouseEventToParent() const
{
	BView *view = GetView();
	if( view == NULL )
		return false;

	BView *parent = view->Parent();
	if( parent == NULL )
		return false;

	BWindow *window = view->Window();
	if( window == NULL )
		return false;

	BMessage *msg = window->CurrentMessage();
	if( msg == NULL )
		return false;
	
	int32 modifiers;
	if( msg->FindInt32("modifiers",&modifiers) < B_OK )
		return false;
	
	if( !(modifiers&B_SHIFT_KEY) )
		return false;
		
	return true;
}

//-----------------------------------------------------------------------------

//dle::Grid::Grid

//-----------------------------------------------------------------------------

//dle::Table::Table

//-----------------------------------------------------------------------------

/** \class dle::Space
 * \ingroup DLE
 * Helper object that is used to put space between other Object's.
 *
 */

//-----------------------------------------------------------------------------

dle::Root::Root()
{
}

dle::Root::~Root()
{
}

//-----------------------------------------------------------------------------

