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
#include <interface/Bitmap.h>
//-------------------------------------
#include "ColorQuant.h"
//-----------------------------------------------------------------------------    

damn::ColorQuant::ColorQuant( int maxcolors, int colorbits )
{
	assert( maxcolors>=1 && maxcolors<=256 );
	assert( colorbits>=1 && colorbits<=8 );
	
	fMaxColors = maxcolors;
	fColorBits = colorbits;

	fTree = NULL;
	fLeafCount = 0;
	for( int i=0; i<=colorbits; i++ )
		fReducibleNodes[i] = NULL;
}

damn::ColorQuant::~ColorQuant()
{
	if( fTree )
		DeleteTree( &fTree );
}

void damn::ColorQuant::DeleteTree( Node **node )
{
	for( int i=0; i<8; i++ )
	{
		if( (*node)->child[i] )
			DeleteTree( &((*node)->child[i]) );
	}
	free( *node );
}

//-----------------------------------------------------------------------------    

void damn::ColorQuant::AddColor( uint8 r, uint8 g, uint8 b )
{
	_AddColor( &fTree, r, g, b, 0 );
	while( fLeafCount > fMaxColors )
		ReduceTree();
}

void damn::ColorQuant::AddColors( const BBitmap *bitmap )
{
	assert( bitmap != NULL );
	assert( bitmap->ColorSpace()==B_RGB32 ||  bitmap->ColorSpace()==B_RGBA32 );

	int width = bitmap->Bounds().IntegerWidth()+1;
	int height = bitmap->Bounds().IntegerHeight()+1;
	for( int iy=0; iy<height; iy++ )
	{
		bitmap_rgb *rgbline = (bitmap_rgb*)BBITMAP32_BITS( bitmap, 0, iy );
		for( int ix=0; ix<width; ix++ )
		{
			const bitmap_rgb &rgb = rgbline[ix];
//			_AddColor( &fTree, rgb.red, rgb.green, rgb.blue, fColorBits, 0, 1, &fLeafCount, fReducibleNodes );
			AddColor( rgb.red, rgb.green, rgb.blue );
		}
//		printf( "%d %d\n", iy, height );
	}
//	while( fLeafCount > fMaxColors )
//	{
//		printf( ">%d %d\n", fLeafCount, fMaxColors );
//		ReduceTree();
//	}
}

void damn::ColorQuant::_AddColor( Node **node, uint8 r, uint8 g, uint8 b, int level )
{
	if( !*node )
		*node = CreateNode( level );

	if( (*node)->leaf )
	{
		(*node)->count++;
		(*node)->red += r;
		(*node)->green += g;
		(*node)->blue += b;
	}
	else
	{
		int index =
			(((r>>(7-level)) & 1) << 2) |
			(((g>>(7-level)) & 1) << 1) |
			(((b>>(7-level)) & 1) << 0);
		_AddColor( &((*node)->child[index]), r, g, b, level+1 );
	}
}

//-----------------------------------------------------------------------------    

std::vector<rgb_color> damn::ColorQuant::GetColors() const
{
	std::vector<rgb_color> pal;
	_GetColors( fTree, &pal );
	return pal;
}

void damn::ColorQuant::_GetColors( Node *node, std::vector<rgb_color> *palette ) const
{
	if( node->leaf )
	{
		rgb_color c;
		c.red = (node->red) / (node->count);
		c.green = (node->green) / (node->count);
		c.blue = (node->blue) / (node->count);
		c.alpha = 255;
		palette->push_back( c );
	}
	else
	{
		for( int i=0; i<8; i++ )
		{
			if( node->child[i]  )
				_GetColors( node->child[i], palette );
		}
	}
}

//-----------------------------------------------------------------------------    

void damn::ColorQuant::ReduceTree()
{
	int i;
	for( i=fColorBits-1; i>0 && fReducibleNodes[i]==NULL; i-- );
	Node* node = fReducibleNodes[i];

	uint r = 0;
	uint g = 0;
	uint b = 0;
	uint count = 0;

	uint minusage = 0xffffffff;
	int child1 = -1;
	for( int i=0; i<8; i++ )
	{
		if( node->child[i] )
		{
			if( node->count	< minusage )
			{
				child1 = i;
				minusage = node->count;
			}
		}
	}
	if( child1 >= 0 )
	{
		r += node->child[child1]->red;
		g += node->child[child1]->green;
		b += node->child[child1]->blue;
		count += node->child[child1]->count;
		free( node->child[child1] );
		node->child[child1] = NULL;
	}
	
	minusage = 0xffffffff;
	int child2 = -1;
	for( int i=0; i<8; i++ )
	{
		if( node->child[i] )
		{
			if( node->count	< minusage )
			{
				child2 = i;
				minusage = node->count;
			}
		}
	}
	if( child2 >= 0 )
	{
		r += node->child[child2]->red;
		g += node->child[child2]->green;
		b += node->child[child2]->blue;
		count += node->child[child2]->count;
		free( node->child[child2] );
		node->child[child2] = NULL;
	}

	uint childs = 0;
	for( int i=0; i<8; i++ )
	{
		if( node->child[i] )
			childs++;
	}
	
	if( childs )
	{
		fLeafCount -= 2;
		for( uint i=0; i<count; i++ )
			AddColor( r, g, b );
	}
	else
	{
		node->leaf = true;
		fLeafCount -= (((child1>=0)+(child2>=0))-1);
		node->red	+= r;
		node->green	+= g;
		node->blue	+= b;
		node->count	+= count;
		fReducibleNodes[i] = node->next;
	}
}

damn::ColorQuant::Node *damn::ColorQuant::CreateNode( int level )
{
	Node *node;

	if( (node=(Node*)malloc(sizeof(Node))) == NULL )
		return NULL;
	memset( node, 0, sizeof(Node) );

	node->leaf = (level==fColorBits);
	if( node->leaf )
		fLeafCount++;
	else
	{
		node->next = fReducibleNodes[level];
		fReducibleNodes[level] = node;
	}
	return node;
}

//-----------------------------------------------------------------------------    

