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
#ifndef DAMN_COLORQUANT_H
#define DAMN_COLORQUANT_H
//-----------------------------------------------------------------------------    
#include <vector>
//-------------------------------------
#include <interface/GraphicsDefs.h>
#include <support/SupportDefs.h>
class BBitmap;
//-------------------------------------
#include "gfx/BitmapUtils.h"
//-----------------------------------------------------------------------------    

// Replace with something smarter...

namespace damn
{
	class ColorQuant
	{
	public:
		ColorQuant( int maxcolors, int colorbits );
		virtual ~ColorQuant();
	
		void AddColor( uint8 r, uint8 g, uint8 b );
		void AddColor( const rgb_color &color ) { AddColor(color.red,color.green,color.blue); }
		void AddColor( const bitmap_rgb &color ) { AddColor(color.red,color.green,color.blue); }
		void AddColors( const BBitmap *bitmap );
	
		std::vector<rgb_color> GetColors() const;
		
	private:
		struct Node
		{
			bool	leaf;

			uint	count;
			uint	red;
			uint	green;
			uint	blue;

			Node	*child[8];
			Node	*next;
		};

		void DeleteTree( Node **node );

		void _AddColor( Node **node, uint8 r, uint8 g, uint8 b, int level );
		void _GetColors( Node *node, std::vector<rgb_color> *palette ) const;

		void ReduceTree();
		Node *CreateNode( int level );

		int		fMaxColors;
		int		fColorBits;

		Node	*fTree;
		int		fLeafCount;
		Node	*fReducibleNodes[9];
	};
}

//-----------------------------------------------------------------------------    
#endif
