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
#include <interface/GraphicsDefs.h>
//-------------------------------------
//-----------------------------------------------------------------------------

class YUV2RGB
{
public:
	YUV2RGB( color_space colorspace );
	YUV2RGB( int bits, uint32 redmask, uint32 greenmask, uint32 bluemask );

	void Convert( int width, int height, uint8 *sy, uint8 *su, uint8 *sv, int sustride, int svstride, int systride, void *drgb, int drgbstride, bool scale );
	void Convert16( int width, int height, uint8 *sy, uint8 *su, uint8 *sv, int sustride, int svstride, int systride, void *drgb, int drgbstride );
	void Convert32( int width, int height, uint8 *sy, uint8 *su, uint8 *sv, int sustride, int svstride, int systride, void *drgb, int drgbstride );
	void Convert32Double( int width, int height, uint8 *sy, uint8 *su, uint8 *sv, int sustride, int svstride, int systride, void *drgb, int drgbstride );

private:
	void Init( int bits, uint32 redmask, uint32 greenmask, uint32 bluemask );

	int CountSetBits( uint32 v );
	int CountClrBitsLow( uint32 v );
	
	int		fBits;

	int		fVRedScale[256];
	int		fUGreenScale[256];
	int		fVGreenScale[256];
	int		fUBlueScale[256];

	uint32	*fRedClamp,		fRedClampBegin[256*3];
	uint32	*fGreenClamp,	fGreenClampBegin[256*3];
	uint32	*fBlueClamp,	fBlueClampBegin[256*3];
};

//-----------------------------------------------------------------------------
