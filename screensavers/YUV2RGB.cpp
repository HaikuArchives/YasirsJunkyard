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
//-------------------------------------
#include "YUV2RGB.h"
//-----------------------------------------------------------------------------

static uint32 YUV_Y_tab[256];
static uint32 YUV_UB_tab[256];
static uint32 YUV_VR_tab[256];
static uint32 YUV_UG_tab[256];
static uint32 YUV_VG_tab[256];
static uint32 RGB_CLIP_15BIT_LR_R[32+32+32];
static uint32 RGB_CLIP_15BIT_LR_G[32+32+32];
static uint32 RGB_CLIP_15BIT_LR_B[32+32+32];

//------------------------------------------------------------------------------

// YUV->RGB
//  R = Y               + 1.40200 * V
//  G = Y - 0.34414 * U - 0.71414 * V
//  B = Y + 1.77200 * U
void Generate_YUV_2_RGB_Tables()
{
	float t_ub = (1.77200/2.0) * (float)(1<<6) + 0.5;
	float t_vr = (1.40200/2.0) * (float)(1<<6) + 0.5;
	float t_ug = (0.34414/2.0) * (float)(1<<6) + 0.5;
	float t_vg = (0.71414/2.0) * (float)(1<<6) + 0.5;
	
	for(int i=0; i<256; i++ )
	{
		float x = (float)(i*2 - 255);
		
		YUV_UB_tab[i] = uint32( (t_ub*x) + (1<<5) );
		YUV_VR_tab[i] = uint32( (t_vr*x) + (1<<5) );
		YUV_UG_tab[i] = uint32( -t_ug*x );
		YUV_VG_tab[i] = uint32( (-t_vg*x) + (1<<5) );
		YUV_Y_tab[i]  = (i<<6) | (i>>2);
	}

	for( int i=0; i<32+32+32; i++ )
	{
		int iv = i-32;
		if( iv<0 ) iv=0;
		if( iv>31 ) iv=31;
				
		RGB_CLIP_15BIT_LR_R[i] = iv<<10;
		RGB_CLIP_15BIT_LR_G[i] = iv<<5;
		RGB_CLIP_15BIT_LR_B[i] = iv<<0;
	}
}

//------------------------------------------------------------------------------

void Convert_YUV411_2_RGB555(
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

	int /*r,g,b,*/col;

	uint8 *sy_0 = sy;
	uint8 *sy_1 = sy+systride;
	
	uint16 *drgb_0 = (uint16*)drgb;
	uint16 *drgb_1 = (uint16*)drgb+drgbstride;

	int ewidth = width&(~1);
	int eheight = height&(~1);
	for( int iy=0; iy<eheight; iy+=2 )
	{
//		printf( "%d:%p\n", iy, sy_0 );
		for( int ix=0; ix<ewidth; ix+=2 )
		{
			int u = *su++;
			int v = *sv++;
			int vr = YUV_VR_tab[v];
			int uvg = YUV_VG_tab[v] + YUV_UG_tab[u];
			int ub = YUV_UB_tab[u];
			//--------------------
			int y = *sy_0++;
			y = YUV_Y_tab[y];
#if 0
			r = (y + vr) >> (6+3);
			if(r<0) r=0; else if(r>31) r=31;
			g = (y + uvg) >> (6+3);
			if(g<0) g=0; else if(g>31) g=31;
			b = (y + ub) >> (6+3);
			if(b<0) b=0; else if(b>31) b=31;
			*drgb_0++ = (r<<10) | (g<<5) | (b);
#else
			col = RGB_CLIP_15BIT_LR_R[((y+vr)>>(6+3)) + 32];
			col |= RGB_CLIP_15BIT_LR_G[((y+uvg)>>(6+3))+32];
			col |= RGB_CLIP_15BIT_LR_B[((y+ub)>>(6+3))+32];
			*drgb_0++ = col;
#endif
			//--------------------
			y = *sy_0++;
			y = YUV_Y_tab[y];
#if 0
			r = (y + vr) >> (6+3);
			if(r<0) r=0; else if(r>31) r=31;
			g = (y + uvg) >> (6+3);
			if(g<0) g=0; else if(g>31) g=31;
			b = (y + ub) >> (6+3);
			if(b<0) b=0; else if(b>31) b=31;
			
			*drgb_0++ = (r<<10) | (g<<5) | (b);
#else
			col = RGB_CLIP_15BIT_LR_R[((y+vr)>>(6+3)) + 32];
			col |= RGB_CLIP_15BIT_LR_G[((y+uvg)>>(6+3))+32];
			col |= RGB_CLIP_15BIT_LR_B[((y+ub)>>(6+3))+32];
			*drgb_0++ = col;
#endif
			//--------------------
			y = *sy_1++;
			y = YUV_Y_tab[y];
#if 0
			r = (y + vr) >> (6+3);
			if(r<0) r=0; else if(r>31) r=31;
			g = (y + uvg) >> (6+3);
			if(g<0) g=0; else if(g>31) g=31;
			b = (y + ub) >> (6+3);
			if(b<0) b=0; else if(b>31) b=31;
			
			*drgb_1++ = (r<<10) | (g<<5) | (b);
#else
			col = RGB_CLIP_15BIT_LR_R[((y+vr)>>(6+3)) + 32];
			col |= RGB_CLIP_15BIT_LR_G[((y+uvg)>>(6+3))+32];
			col |= RGB_CLIP_15BIT_LR_B[((y+ub)>>(6+3))+32];
			*drgb_1++ = col;
#endif
			//--------------------
			y = *sy_1++;
			y = YUV_Y_tab[y];
#if 0
			r = (y + vr) >> (6+3);
			if(r<0) r=0; else if(r>31) r=31;
			g = (y + uvg) >> (6+3);
			if(g<0) g=0; else if(g>31) g=31;
			b = (y + ub) >> (6+3);
			if(b<0) b=0; else if(b>31) b=31;
			
			*drgb_1++ = (r<<10) | (g<<5) | (b);
#else
			col = RGB_CLIP_15BIT_LR_R[((y+vr)>>(6+3)) + 32];
			col |= RGB_CLIP_15BIT_LR_G[((y+uvg)>>(6+3))+32];
			col |= RGB_CLIP_15BIT_LR_B[((y+ub)>>(6+3))+32];
			*drgb_1++ = col;
#endif
		}
		su += (sustride-(ewidth/2));
		sv += (svstride-(ewidth/2));
		sy_0 += (systride-ewidth)+systride;
		sy_1 += (systride-ewidth)+systride;
		
		drgb_0 += (drgbstride-ewidth)+drgbstride;
		drgb_1 += (drgbstride-ewidth)+drgbstride;
	}
}

