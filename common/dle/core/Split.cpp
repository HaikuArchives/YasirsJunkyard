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
#include "../DamnLayoutEngine.h"
//-----------------------------------------------------------------------------

template<class T> T min( T a, T b ) { return a<b?a:b; }
template<class T> T max( T a, T b ) { return a>b?a:b; }

//-----------------------------------------------------------------------------

/** \class damn::Split
 * \ingroup DLE
 * A collection of Object objects.
 *
 */

//-----------------------------------------------------------------------------

damn::Split::Split( float spacing, align_t align ) :
	BView(BRect(0,0,0,0), "split", B_FOLLOW_NONE, 0),
	Group( this )
{
	assert( spacing >= 0 );
//	assert( (align&(LEFTRIGHTMASK|TOPBOTTOMMASH)==0)  && ((align&LEFTRIGHTMASK)<=RIGHT) && ((align&TOPBOTTOMMASH)<=BOTTOM) );

	SetSpacing( spacing );

	fDefaultAlign = align;
}

damn::Split::~Split()
{
}

void damn::Split::AttachedToWindow()
{
	if( Parent() )
		SetViewColor( Parent()->ViewColor() );
}

void damn::Split::AddObject( Object *child, align_t align, float weight )
{
	assert( child != NULL );
	assert( weight >= 0.0f );
	
	childinfo *c = new childinfo;
	c->object = child;
	c->weight = weight;
	c->align = (align_t)(
		((align&HMASK)==HDEFAULT) ? (fDefaultAlign&HMASK) : (align&HMASK) | 
		((align&VMASK)==VDEFAULT) ? (fDefaultAlign&VMASK) : (align&VMASK));
	
	fChilds.AddItem( c );
	fView->AddChild( child->GetView() );
}

//-----------------------------------------------------------------------------

/** \class damn::HSplit
 * \ingroup DLE
 * A horizontal splitter.
 *
 */

//-----------------------------------------------------------------------------

damn::HSplit::HSplit( float spacing, align_t align ) :
	Split( spacing, align )
{
}

damn::HSplit::~HSplit()
{
}

damn::MinMax2 damn::HSplit::GetMinMaxSize()
{
	float minheight = 0;
	float maxheight = 0;
	float *minwidth = (float*)alloca( fChilds.CountItems()*sizeof(float) );
	float *maxwidth = (float*)alloca( fChilds.CountItems()*sizeof(float) );

	for( int i=0; i<fChilds.CountItems(); i++ )
	{
		childinfo *ci = fChilds.ItemAt( i );
		MinMax2 childminmax = ci->object->GetMinMaxSize();

		if( childminmax.vmin>minheight ) minheight = childminmax.vmin;
		if( childminmax.vmax>maxheight ) maxheight = childminmax.vmax;
		minwidth[i] = childminmax.hmin;
		maxwidth[i] = childminmax.hmax;
	}
	assert( maxheight>=minheight );

	MinMax1	widthmm = SpreadCalcSize( minwidth, maxwidth, fChilds.CountItems() );

	float hspacing = fHSpacingLeft + max(fHSpacingMid*(fChilds.CountItems()-1),0.0f) + fHSpacingRight;
	widthmm.min += hspacing;
	widthmm.max += hspacing;

	return MinMax2( widthmm.min,widthmm.max, minheight+fVSpacingTop+fVSpacingBottom,maxheight+fVSpacingTop+fVSpacingBottom );
}

void damn::HSplit::SetSize( const BRect &size )
{
//	printf( "damn::HSplit::SetSize:\n" );
//	size.PrintToStream();

	Split::SetSize( size );

	float *width = (float*)alloca( fChilds.CountItems() * sizeof(float) );
	float *minwidth = (float*)alloca( fChilds.CountItems() * sizeof(float) );
	float *maxwidth = (float*)alloca( fChilds.CountItems() * sizeof(float) );
	float *weight = (float*)alloca( fChilds.CountItems() * sizeof(float) );

	for( int i=0; i<fChilds.CountItems(); i++ )
	{
		childinfo *ci = fChilds.ItemAt( i );
		MinMax2 childminmax = ci->object->GetMinMaxSize();

		minwidth[i] = childminmax.hmin;
		maxwidth[i] = childminmax.hmax;
		weight[i] = ci->weight;
	}

	float hspacing = fHSpacingLeft + max(fHSpacingMid*(fChilds.CountItems()-1),0.0f) + fHSpacingRight;

	Spread( minwidth, maxwidth, width, weight, fChilds.CountItems(), size.Width()+1-hspacing );

	BRect srect;
	srect.top = fVSpacingTop;
	srect.bottom = (size.bottom-size.top)-fVSpacingBottom;
	srect.left = size.left + fHSpacingLeft;
	srect.right = 0;
	for( int i=0; i<fChilds.CountItems(); i++ )
	{
		childinfo *ci = fChilds.ItemAt( i );
		MinMax2 childminmax = ci->object->GetMinMaxSize();

		srect.right = srect.left + width[i]-1;
		ci->object->SetSize( AlignRect(srect,childminmax,ci->align) );

		srect.left = srect.right+1 + fHSpacingMid;
	}
}

//-----------------------------------------------------------------------------

/** \class damn::VSplit
 * \ingroup DLE
 * A vertical splitter.
 *
 */

//-----------------------------------------------------------------------------

damn::VSplit::VSplit( float spacing, align_t align ) :
	Split( spacing, align )
{
}

damn::VSplit::~VSplit()
{
}

damn::MinMax2 damn::VSplit::GetMinMaxSize()
{
	float minwidth = 0;
	float maxwidth = 0;
	float *minheight = (float*)alloca( fChilds.CountItems() * sizeof(float) );
	float *maxheight = (float*)alloca( fChilds.CountItems() * sizeof(float) );

	for( int i=0; i<fChilds.CountItems(); i++ )
	{
		childinfo *ci = fChilds.ItemAt( i );
		MinMax2 childminmax = ci->object->GetMinMaxSize();

		if( childminmax.hmin>minwidth ) minwidth = childminmax.hmin;
		if( childminmax.hmax>maxwidth ) maxwidth = childminmax.hmax;
		minheight[i] = childminmax.vmin;
		maxheight[i] = childminmax.vmax;
	}
	assert( maxwidth>=minwidth );

	MinMax1	heightmm = SpreadCalcSize( minheight, maxheight, fChilds.CountItems() );

	float vspacing = fVSpacingTop + max(fVSpacingMid*(fChilds.CountItems()-1),0.0f) + fVSpacingBottom;
	heightmm.min += vspacing;
	heightmm.max += vspacing;

	return MinMax2( minwidth+fHSpacingLeft+fHSpacingRight,maxwidth+fHSpacingLeft+fHSpacingRight, heightmm.min,heightmm.max );
}

void damn::VSplit::SetSize( const BRect &size )
{
//	printf( "damn::VSplit::SetSize:\n" );
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

		minheight[i] = childminmax.vmin;
		maxheight[i] = childminmax.vmax;
		weight[i] = ci->weight;
	}

	float vspacing = fVSpacingTop + max(fVSpacingMid*(fChilds.CountItems()-1),0.0f) + fVSpacingBottom;

	Spread( minheight, maxheight, height, weight, fChilds.CountItems(), size.Height()+1-vspacing );

	BRect srect;
	srect.top = fVSpacingTop;
	srect.bottom = 0;
	srect.left = fHSpacingLeft;
	srect.right = (size.right-size.left)-fHSpacingRight;
//	srect.top = size.top + fVSpacingTop;
//	srect.bottom = 0;
//	srect.left = fHSpacingLeft;
//	srect.right = (size.right-size.left)-fHSpacingRight;
	for( int i=0; i<fChilds.CountItems(); i++ )
	{
		childinfo *ci = fChilds.ItemAt( i );
		MinMax2 childminmax = ci->object->GetMinMaxSize();

		srect.bottom = srect.top + height[i]-1;
		ci->object->SetSize( AlignRect(srect,childminmax,ci->align) );

		srect.top = srect.bottom+1 + fVSpacingMid;
	}
}

//-----------------------------------------------------------------------------

