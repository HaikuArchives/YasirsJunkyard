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
#include <alloca.h>
#include <stdio.h>
//-------------------------------------
#include <support/Debug.h>
//-------------------------------------
#include "../Split.h"
#include "../Internal.h"
//-----------------------------------------------------------------------------

/** \class dle::HSplit
 * \ingroup DLE
 * A horizontal splitter.
 *
 */

//-----------------------------------------------------------------------------

dle::HSplit::HSplit() :
	BView( BRect(0,0,0,0), "group", B_FOLLOW_NONE, B_WILL_DRAW ),
	Group( this )
{
	InnerSpacing( false );
}

dle::HSplit::~HSplit()
{
}

void dle::HSplit::AttachedToWindow()
{
	SetViewColor( GetColor() );
}

dle::MinMax2 dle::HSplit::GetMinMaxSize()
{
	ASSERT_WITH_MESSAGE( fChilds.CountItems(), "A HSplit container must not be empty" );

	float minheight = 0;
	float maxheight = FLT_MAX;
	float *minwidth = (float*)alloca( fChilds.CountItems()*sizeof(float) );
	float *maxwidth = (float*)alloca( fChilds.CountItems()*sizeof(float) );
	
	for( int i=0; i<fChilds.CountItems(); i++ )
	{
		childinfo *ci = fChilds.ItemAt( i );
		MinMax2 childminmax = ci->object->GetMinMaxSize();
		ASSERT( childminmax.vert.min <= childminmax.vert.max );
		ADJUSTRECT( ci, childminmax );

		ASSERT( childminmax.vert.min <= childminmax.vert.max );

		if( childminmax.vert.min>minheight ) minheight = childminmax.vert.min;
		if( childminmax.vert.max<maxheight ) maxheight = childminmax.vert.max;
		minwidth[i] = childminmax.horz.min;
		maxwidth[i] = childminmax.horz.max;
	}
	if( maxheight < minheight )
		maxheight = minheight;
//	ASSERT( maxheight>=minheight );

	MinMax1	widthmm = SpreadCalcSize( minwidth, maxwidth, fChilds.CountItems() );

	return MinMax2( widthmm.min,widthmm.max, minheight,maxheight );
}

void dle::HSplit::SetSize( const BRect &size )
{
//	printf( "dle::HSplit::SetSize:\n" );
//	size.PrintToStream();

	Group::SetSize( size );

	float *width = (float*)alloca( fChilds.CountItems() * sizeof(float) );
	float *minwidth = (float*)alloca( fChilds.CountItems() * sizeof(float) );
	float *maxwidth = (float*)alloca( fChilds.CountItems() * sizeof(float) );
	float *weight = (float*)alloca( fChilds.CountItems() * sizeof(float) );

	for( int i=0; i<fChilds.CountItems(); i++ )
	{
		childinfo *ci = fChilds.ItemAt( i );
		MinMax2 childminmax = ci->object->GetMinMaxSize();
		ADJUSTRECT( ci, childminmax );

		minwidth[i] = childminmax.horz.min;
		maxwidth[i] = childminmax.horz.max;
		weight[i] = ci->weight;
	}

	Spread( minwidth, maxwidth, width, weight, fChilds.CountItems(), size.Width()+1 );

	BRect srect;
	srect.top = 0;
	srect.bottom = (size.bottom-size.top);
	srect.left = 0;
	srect.right = 0;
	for( int i=0; i<fChilds.CountItems(); i++ )
	{
		childinfo *ci = fChilds.ItemAt( i );
		MinMax2 childminmax = ci->object->GetMinMaxSize();
		ADJUSTRECT( ci, childminmax );

		srect.right = srect.left + width[i]-1;

		BRect sarect = AlignRect( srect, childminmax, ci->align );
		DEADJUSTRECT( ci, sarect );
		ci->object->SetSize( sarect );

		srect.left = srect.right+1;
	}
}

//-----------------------------------------------------------------------------

/** \class dle::VSplit
 * \ingroup DLE
 * A vertical splitter.
 *
 */

//-----------------------------------------------------------------------------

dle::VSplit::VSplit() :
	BView( BRect(0,0,0,0), "group", B_FOLLOW_NONE, B_WILL_DRAW ),
	Group( this )
{
	InnerSpacing( false );
}

dle::VSplit::~VSplit()
{
}

void dle::VSplit::AttachedToWindow()
{
	SetViewColor( GetColor() );
}

dle::MinMax2 dle::VSplit::GetMinMaxSize()
{
	ASSERT_WITH_MESSAGE( fChilds.CountItems(), "A VSplit container must not be empty" );

	float minwidth = 0;
	float maxwidth = kMaxSize;
	float *minheight = (float*)alloca( fChilds.CountItems() * sizeof(float) );
	float *maxheight = (float*)alloca( fChilds.CountItems() * sizeof(float) );

	for( int i=0; i<fChilds.CountItems(); i++ )
	{
		childinfo *ci = fChilds.ItemAt( i );
		MinMax2 childminmax = ci->object->GetMinMaxSize();
		ASSERT( childminmax.horz.min <= childminmax.horz.max );
		ADJUSTRECT( ci, childminmax );
		
		ASSERT( childminmax.horz.min <= childminmax.horz.max );

		if( childminmax.horz.min>minwidth ) minwidth = childminmax.horz.min;
		if( childminmax.horz.max<maxwidth ) maxwidth = childminmax.horz.max;
		minheight[i] = childminmax.vert.min;
		maxheight[i] = childminmax.vert.max;
	}
	if( maxwidth < minwidth )
		maxwidth = minwidth;

	MinMax1	heightmm = SpreadCalcSize( minheight, maxheight, fChilds.CountItems() );

	return MinMax2( minwidth,maxwidth, heightmm.min,heightmm.max );
}

void dle::VSplit::SetSize( const BRect &size )
{
//	printf( "dle::VSplit::SetSize:\n" );
//	size.PrintToStream();
	
//	DEBUGGER( "Pling!" );

	Group::SetSize( size );

	float *height = (float*)alloca( fChilds.CountItems() * sizeof(float) );
	float *minheight = (float*)alloca( fChilds.CountItems() * sizeof(float) );
	float *maxheight = (float*)alloca( fChilds.CountItems() * sizeof(float) );
	float *weight = (float*)alloca( fChilds.CountItems() * sizeof(float) );

	for( int i=0; i<fChilds.CountItems(); i++ )
	{
		childinfo *ci = fChilds.ItemAt( i );
		MinMax2 childminmax = ci->object->GetMinMaxSize();
		ADJUSTRECT( ci, childminmax );

		minheight[i] = childminmax.vert.min;
		maxheight[i] = childminmax.vert.max;
		weight[i] = ci->weight;
	}

	Spread( minheight, maxheight, height, weight, fChilds.CountItems(), size.Height()+1 );

	BRect srect;
	srect.top = 0;
	srect.bottom = 0;
	srect.left = 0;
	srect.right = (size.right-size.left);
	for( int i=0; i<fChilds.CountItems(); i++ )
	{
		childinfo *ci = fChilds.ItemAt( i );
		MinMax2 childminmax = ci->object->GetMinMaxSize();
		ADJUSTRECT( ci, childminmax );

		srect.bottom = srect.top + height[i]-1;

		BRect sarect = AlignRect( srect, childminmax, ci->align );
		DEADJUSTRECT( ci, sarect );
		ci->object->SetSize( sarect );

		srect.top = srect.bottom+1;
	}
}

//-----------------------------------------------------------------------------

