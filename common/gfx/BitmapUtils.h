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
#ifndef DAMN_BITMAPUTILS_H
#define DAMN_BITMAPUTILS_H
//-----------------------------------------------------------------------------    
//-------------------------------------
//#include <interface/InterfaceDefs.h>
#include <interface/GraphicsDefs.h>
//-------------------------------------
//-----------------------------------------------------------------------------    

struct bitmap_rgb
{
	bitmap_rgb() {};
	bitmap_rgb( const rgb_color &col ) : blue(col.blue), green(col.green), red(col.red), alpha(col.alpha) {};

//	rgb_color operator=() { rgb_color col; col.red=red; col.green=green; col.blue=blue; col.alpha=alpha; return col; };

	uint8 blue;
	uint8 green;
	uint8 red;
	uint8 alpha;
};

#define BBITMAP32_BITS(bitmap,xpos,ypos) \
	(((uint32*)(((uint8*)((bitmap)->Bits())) + ((ypos)*(srcbitmap)->BytesPerRow()))) + (xpos))

//------------------------------------------------------------------------------
#endif
