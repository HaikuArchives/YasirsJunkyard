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
//-------------------------------------
#include "YUV2RGB_1632.h"
//-----------------------------------------------------------------------------

YUV2RGB::YUV2RGB( color_space colorspace )
{
	switch( colorspace )
	{
		case B_RGB32:	// B[7:0]  G[7:0]  R[7:0]  -[7:0]
		case B_RGBA32:	// B[7:0]  G[7:0]  R[7:0]  A[7:0]
			Init( 32, 0x00ff0000, 0x0000ff00, 0x000000ff );
			break;

		case B_RGB24:	// B[7:0]  G[7:0]  R[7:0]
			assert( 0 );
			break;
			
		case B_RGB16:	// G[2:0],B[4:0]  R[4:0],G[5:3]
			Init( 16, 0xf800, 0x07e0, 0x1f );
			break;

		case B_RGB15:	// G[2:0],B[4:0]  -[0],R[4:0],G[4:3]
		case B_RGBA15:	// G[2:0],B[4:0]  A[0],R[4:0],G[4:3] 
			Init( 16, 0x7c00, 0x03e0, 0x1f );
			break;
		
		default:
			assert( 0 );
			break;
	}
}

YUV2RGB::YUV2RGB( int bits, uint32 redmask, uint32 greenmask, uint32 bluemask )
{
	Init( bits, redmask, greenmask, bluemask );
}

//-----------------------------------------------------------------------------

// FIXME: optimize!
int YUV2RGB::CountSetBits( uint32 v )
{
	if( v == 0 ) return 0;
	if( v & 1 ) return 1 + CountSetBits( v >> 1 );
	return CountSetBits( v >> 1 );
}

int YUV2RGB::CountClrBitsLow( uint32 v )
{
	if( v == 0 ) return sizeof(uint32)*8;
	if( v & 1 ) return 0;
	return 1 + CountClrBitsLow( v >> 1 );
}


void YUV2RGB::Init( int bits, uint32 redmask, uint32 greenmask, uint32 bluemask )
{
	fBits = bits;

	for( int i=0; i<256; i++ )
	{
		float u = i-128.0f;
		float v = i-128.0f;
		fVRedScale[i]	= int(  1.4013f * v );
		fUGreenScale[i]	= int( -0.3444f * u );
		fVGreenScale[i]	= int( -0.7136f * v );
		fUBlueScale[i]	= int(  1.7734f * u );
	}
	
	int redbits = CountSetBits( redmask );
	int greenbits = CountSetBits( greenmask );
	int bluebits = CountSetBits( bluemask );
	
	int redshift = CountClrBitsLow( redmask );
	int greenshift = CountClrBitsLow( greenmask );
	int blueshift = CountClrBitsLow( bluemask );

	for( int i=0; i<256; i++ )
	{
		fRedClampBegin[i+256]	= (i>>(8-redbits)) << redshift;
		fGreenClampBegin[i+256]	= (i>>(8-greenbits)) << greenshift;
		fBlueClampBegin[i+256]	= (i>>(8-bluebits)) << blueshift;

//		printf( "%3d: %04X %04X %04X\n", i, 
//			fRedClampBegin[i+256],
//			fGreenClampBegin[i+256],
//			fBlueClampBegin[i+256] );

		if( bits==15 || bits==16 )
		{
			fRedClampBegin[i+256]	|= fRedClampBegin[i+256] << 16;
			fGreenClampBegin[i+256]	|= fGreenClampBegin[i+256] << 16;
			fBlueClampBegin[i+256]	|= fBlueClampBegin[i+256] << 16;
		}
	}

	for( int i=0; i<256; i++ )
	{
		fRedClampBegin[i]		= fRedClampBegin[256];
		fRedClampBegin[i+512]	= fRedClampBegin[511];
		fGreenClampBegin[i]		= fGreenClampBegin[256];
		fGreenClampBegin[i+512]	= fGreenClampBegin[511];
		fBlueClampBegin[i]		= fBlueClampBegin[256];
		fBlueClampBegin[i+512]	= fBlueClampBegin[511];
	}

	fRedClamp	= fRedClampBegin + 256;
	fGreenClamp	= fGreenClampBegin + 256;
	fBlueClamp	= fBlueClampBegin + 256;
}

//-----------------------------------------------------------------------------

void YUV2RGB::Convert( int width, int height, uint8 *sy, uint8 *su, uint8 *sv, int sustride, int svstride, int systride, void *drgb, int drgbstride, bool scale )
{
	if( !scale )
	{
		if( fBits==16 )			Convert16( width,height, sy,su,sv, sustride,svstride,systride, drgb,drgbstride );
		else if( fBits==32 )	Convert32( width,height, sy,su,sv, sustride,svstride,systride, drgb,drgbstride );
		else assert( 0 );
	}
	else
	{
		if( fBits==32 )	Convert32Double( width,height, sy,su,sv, sustride,svstride,systride, drgb,drgbstride );
		else assert( 0 );
	}
}

//-----------------------------------------------------------------------------

void YUV2RGB::Convert16(
	int width, int height,
	uint8 *sy,
	uint8 *su,
	uint8 *sv,
	int systride,
	int sustride,
	int svstride,
	void *drgb,
	int drgbstride )
{
	drgbstride = drgbstride/2;

	uint8 *sy_0 = sy;
	uint8 *sy_1 = sy+systride;
	
	uint16 *drgb_0 = (uint16*)drgb;
	uint16 *drgb_1 = (uint16*)drgb+drgbstride;

	int ewidth = width&(~1);
	int eheight = height&(~1);
	for( int iy=0; iy<eheight; iy+=2 )
	{
		for( int ix=0; ix<ewidth; ix+=2 )
		{
			int u = *su++;
			int v = *sv++;

			int r = fVRedScale[v];
			int g = fUGreenScale[u] + fVGreenScale[v];
			int b = fUBlueScale[u];
			//--------------------
			int y = *sy_0++;
			*drgb_0++ = fRedClamp[y+r] | fGreenClamp[y+g] | fBlueClamp[y+b];
			//--------------------
			y = *sy_0++;
			*drgb_0++ = fRedClamp[y+r] | fGreenClamp[y+g] | fBlueClamp[y+b];
			//--------------------
			y = *sy_1++;
			*drgb_1++ = fRedClamp[y+r] | fGreenClamp[y+g] | fBlueClamp[y+b];
			//--------------------
			y = *sy_1++;
			*drgb_1++ = fRedClamp[y+r] | fGreenClamp[y+g] | fBlueClamp[y+b];
		}
		su += (sustride-(ewidth/2));
		sv += (svstride-(ewidth/2));
		sy_0 += (systride-ewidth)+systride;
		sy_1 += (systride-ewidth)+systride;
		
		drgb_0 += (drgbstride-ewidth)+drgbstride;
		drgb_1 += (drgbstride-ewidth)+drgbstride;
	}
}

void YUV2RGB::Convert32(
	int width, int height,
	uint8 *sy,
	uint8 *su,
	uint8 *sv,
	int systride,
	int sustride,
	int svstride,
	void *drgb,
	int drgbstride )
{
	drgbstride = drgbstride/4;

	uint8 *sy_0 = sy;
	uint8 *sy_1 = sy+systride;
	
	uint32 *drgb_0 = (uint32*)drgb;
	uint32 *drgb_1 = (uint32*)drgb+drgbstride;

	int ewidth = width&(~1);
	int eheight = height&(~1);
	for( int iy=0; iy<eheight; iy+=2 )
	{
		for( int ix=0; ix<ewidth; ix+=2 )
		{
			int u = *su++;
			int v = *sv++;

			int r = fVRedScale[v];
			int g = fUGreenScale[u] + fVGreenScale[v];
			int b = fUBlueScale[u];
			//--------------------
			int y = *sy_0++;
			*drgb_0++ = fRedClamp[y+r] | fGreenClamp[y+g] | fBlueClamp[y+b];
			//--------------------
			y = *sy_0++;
			*drgb_0++ = fRedClamp[y+r] | fGreenClamp[y+g] | fBlueClamp[y+b];
			//--------------------
			y = *sy_1++;
			*drgb_1++ = fRedClamp[y+r] | fGreenClamp[y+g] | fBlueClamp[y+b];
			//--------------------
			y = *sy_1++;
			*drgb_1++ = fRedClamp[y+r] | fGreenClamp[y+g] | fBlueClamp[y+b];
		}
		su += (sustride-(ewidth/2));
		sv += (svstride-(ewidth/2));
		sy_0 += (systride-ewidth)+systride;
		sy_1 += (systride-ewidth)+systride;
		
		drgb_0 += (drgbstride-ewidth)+drgbstride;
		drgb_1 += (drgbstride-ewidth)+drgbstride;
	}
}

void YUV2RGB::Convert32Double(
	int width, int height,
	uint8 *sy,
	uint8 *su,
	uint8 *sv,
	int systride,
	int sustride,
	int svstride,
	void *drgb,
	int drgbstride )
{
	drgbstride = drgbstride/4;
	int drgbstride2 = drgbstride*2;

	uint8 *sy_0 = sy;
	uint8 *sy_1 = sy+systride;
	
	uint32 *drgb_0 = (uint32*)drgb;
	uint32 *drgb_1 = (uint32*)drgb+drgbstride;

	int ewidth = width&(~1);
	int eheight = height&(~1);
	for( int iy=0; iy<eheight; iy+=2 )
	{
		for( int ix=0; ix<ewidth; ix+=2 )
		{
			int u = *su++;
			int v = *sv++;

			int r = fVRedScale[v];
			int g = fUGreenScale[u] + fVGreenScale[v];
			int b = fUBlueScale[u];
			//--------------------
			int y = *sy_0++;
			uint32 col = fRedClamp[y+r] | fGreenClamp[y+g] | fBlueClamp[y+b];
			*(drgb_0+drgbstride2) = col;
			*drgb_0++ = col;
			*(drgb_0+drgbstride2) = col;
			*drgb_0++ = col;
			//--------------------
			y = *sy_0++;
			col = fRedClamp[y+r] | fGreenClamp[y+g] | fBlueClamp[y+b];
			*(drgb_0+drgbstride2) = col;
			*drgb_0++ = col;
			*(drgb_0+drgbstride2) = col;
			*drgb_0++ = col;

			*(drgb_0+drgbstride2) = col;
			*drgb_0++ = col;
			//--------------------
			y = *sy_1++;
			col = fRedClamp[y+r] | fGreenClamp[y+g] | fBlueClamp[y+b];
			*(drgb_1+drgbstride2) = col;
			*drgb_1++ = col;
			*(drgb_1+drgbstride2) = col;
			*drgb_1++ = col;
			//--------------------
			y = *sy_1++;
			col = fRedClamp[y+r] | fGreenClamp[y+g] | fBlueClamp[y+b];
			*(drgb_1+drgbstride2) = col;
			*drgb_1++ = col;
			*(drgb_1+drgbstride2) = col;
			*drgb_1++ = col;

			*(drgb_1+drgbstride2) = col;
			*drgb_1++ = col;
		}
		su += (sustride-(ewidth/2));
		sv += (svstride-(ewidth/2));
		sy_0 += (systride-ewidth)+systride;
		sy_1 += (systride-ewidth)+systride;
		
//		drgb_0 += (drgbstride-ewidth*2)+drgbstride*3;
//		drgb_1 += (drgbstride-ewidth*2)+drgbstride*3;
		drgb_0 += drgbstride*3;
		drgb_1 += drgbstride*3;
	}
}


