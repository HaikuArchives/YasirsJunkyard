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
#include <stdlib.h>
#include <string.h>
//-------------------------------------
//-------------------------------------
#include "LoadView.h"
//-----------------------------------------------------------------------------

LoadView::LoadView( BRect rect, uint32 resizingmode ) :
	BView( rect, "LoadView",  resizingmode, B_WILL_DRAW|B_FULL_UPDATE_ON_RESIZE|B_FRAME_EVENTS )
{
	fRefValue = 0.0f;
	fHistAlloc = 0;
	fHist = NULL;

	fHistAlloc = 1;
	fHist = (float*)malloc( fHistAlloc*sizeof(float) );
	memset( fHist, 0, fHistAlloc*sizeof(float) );
	
	FrameResized( rect.Width(), rect.Height() );
	
//	SetViewColor( B_TRANSPARENT_COLOR );
}

LoadView::~LoadView()
{
	free( fHist );
}

void LoadView::FrameResized( float newwidth, float newheight )
{
	if( newwidth+1 > fHistAlloc )
	{
		int inewwidth = (int)newwidth+1;
		fHist = (float*)realloc( fHist, inewwidth*sizeof(float) );
		assert( fHist != NULL );
		memset( fHist+fHistAlloc, 0, (inewwidth-fHistAlloc)*sizeof(float) );
		fHistAlloc = inewwidth;
	}
}

void LoadView::Draw( BRect updaterect )
{
//	int left = (int)Bounds().left;
	int right = (int)Bounds().right;
	int top = (int)Bounds().top;
	int bottom = (int)Bounds().bottom;
	int iref = int(((bottom+1)-top)*(1.0f-fRefValue)+0.45f);

	rgb_color white; white.red=white.green=white.blue=255; white.alpha=255;
	rgb_color black; black.red=black.green=black.blue=0;   white.alpha=255;
	rgb_color refcol; refcol.red=255; refcol.green=0; refcol.blue=0;   refcol.alpha=255;

	BeginLineArray( (updaterect.right-updaterect.left+1)*3 );
	for( int x=(int)updaterect.left; x<=(int)updaterect.right; x++ )
	{
		float load = 0.0f;

//		if( right-x < fHistAlloc )
			load = fHist[right-x];

		float fill = ((bottom+1)-top)*load+0.45f;

		int itop = top;
		int ibottom = bottom;
		int ifill = (int)fill;

		if( itop<=ibottom-ifill )
			AddLine( BPoint(x,itop), BPoint(x,ibottom-ifill), white );
		if( ifill != 0 )
			AddLine( BPoint(x,ibottom), BPoint(x,ibottom-ifill+1), black );

		if( fRefValue>0.0f && fRefValue<1.0f )
			AddLine( BPoint(x,iref), BPoint(x,iref), refcol );
	}
	EndLineArray();
}

void LoadView::SetRefValue( float ref )
{
	fRefValue = ref;
	if( Window() )
		Draw( Bounds() );
}

void LoadView::AddSlice( float load )
{
	memmove( fHist+1, fHist, (fHistAlloc-1)*sizeof(float) );
	fHist[0] = load;

	BRect src = Bounds().OffsetByCopy( 0, 0 );
	BRect dst = src.OffsetByCopy( -1, 0 );
	CopyBits( src, dst );
	if( Window() )
		Draw( BRect(Bounds().right,Bounds().top,Bounds().right,Bounds().bottom) );
}

//-----------------------------------------------------------------------------
