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
#include <math.h>
//-------------------------------------
#include "BitmapRot.h" 
//-----------------------------------------------------------------------------

//void RotatorRotate128( uint8 *dst, uint8 *src, float x, float y, float centerx, float centery, float scale, float ang, int width, int height );
//void RotatorRotate64 ( uint8 *dst, uint8 *src, float x, float y, float centerx, float centery, float scale, float ang, int width, int height );

#define PREV 16
#define NEW  16

void RotatorRotate256(
	uint8 *dst, uint8 *src,
	float centerx, float centery,
	float scale, float ang,
	int width, int height )
{
	float fdx  = cos(ang)*scale; int dx = int( fdx*65536.0f /*+ 0.5f*/ );
	float fdy  = sin(ang)*scale; int dy = int( fdy*65536.0f /*+ 0.5f*/ );
	float fddx = cos(ang+PI/2)*scale; int ddx = int( fddx*65536.0f /*+ 0.5f*/ );
	float fddy = sin(ang+PI/2)*scale; int ddy = int( fddy*65536.0f /*+ 0.5f*/ );
	
	float fi = centerx - fdx*centerx - fddx*centery; int i = int( (fi+0.5f)*65536.0f );
	float fj = centery - fdy*centerx - fddy*centery; int j = int( (fj+0.5f)*65536.0f );
	
//#define GETVAL(x,y) ((i*(ddx+1))+(dx*(+1)x))/65536,((j*ddy)+(dy*y))/65536
	
//	printf( "%dx%d: %dx%d\n", 0, 0, GETVAL(0,0) );
//	printf( "%dx%d: %dx%d\n", 127, 127, GETVAL(127,128) );
//	printf( "%dx%d: %dx%d\n", 128, 128, GETVAL(128,128) );
//	printf( "%dx%d: %dx%d\n", 255, 255, GETVAL(255,255) );

	for( int iy=0; iy<height; iy++ )
	{
		int ii = i;
		int jj = j;
//		float fii = fi;
//		float fjj = fj;
		for( int ix=0; ix<width; ix++ )
		{
//			if( (ix==0 || ix==127 || ix==128 || ix==255) && iy==0 )
//			if( (ix==0 || ix==255) && (iy==0 || iy==255) )
//				printf( "%dx%d - %dx%d - %fx%f\n", ix, iy, ii>>16, jj>>16, fii, fjj );
				
#if( PREV == NEW )
			*dst++=(*dst+src[((ii&0xff0000)>>16)+((jj&0xff0000)>>8)]+1)/2; 
//			*dst++=(*dst+src[int(fii+0.5f)*256+int(fjj+0.5f)]+1)/2; 
#else
			*dst=(
				(*dst)*PREV+
				(src[((ii&0xff0000)>>16)+((jj&0xff0000)>>8)]+1)*NEW
				)/(PREV+NEW);
			dst++;
#endif
			ii+=dx;
			jj+=dy;
//			fii+=fdx;
//			fjj+=fdy;
		}
		i+=ddx;
		j+=ddy;
//		fi+=fddx;
//		fj+=fddy;
	}
} 

#if 0
void RotatorRotate128( 
	uchar *dst, uchar *src, 
	float x, float y, 
	float centerx, float centery, 
	float scale, float ang, 
	int width, int height )
{ 
	int	ddx,ddy,d2x,d2y; 
	int	i,j; 
 
	int	xx,yy; 
	int	ii,jj; 
 
	float vddx=(cos(ang)*scale);
	float vddy=(sin(ang)*scale);
	ddx = vddx*256*256;
	ddy = vddy*256*256;
 
	float vd2x=(cos(ang+3.1415f/2.0f)*scale);
	float vd2y=(sin(ang+3.1415f/2.0f)*scale);
	d2x = vd2x*256*256;
	d2y = vd2y*256*256;

	i=(x-vddx*centerx-vd2x*centery)*256*256;
	j=(y-vddy*centerx-vd2y*centery)*256*256;
 
	for(yy=0; yy<height; yy++) 
	{ 
		ii=i; 
		jj=j; 
		for(xx=0; xx<width/2; xx++) 
		{ 
//			dst[0]=(dst[0]+src[((ii&0x7f0000)>>16)+((jj&0x7f0000)>>9)]+1)/2; 
			*dst++=(*dst+src[((ii>>16)&0x7f)+((jj>>9)&0x3f80)]+1)/2; 
			ii += ddx; 
			jj += ddy; 

//			dst[1]=(dst[1]+src[((ii&0x7f0000)>>16)+((jj&0x7f0000)>>9)]+1)/2; 
			*dst++=(*dst+src[((ii>>16)&0x7f)+((jj>>9)&0x3f80)]+1)/2; 
			ii += ddx; 
			jj += ddy; 

//			dst += 2; 
		} 
		i+=d2x; 
		j+=d2y; 
	} 
} 

void RotatorRotate64( uchar *dst, uchar *src, int x, int y, int centerx, int centery, int scale, int ang, int width, int height )
{ 
	int	ddx,ddy,d2x,d2y; 
	int	i,j; 
 
	int	xx,yy; 
	int	ii,jj; 
 
	ang=ang&1023; 
 
	ddx=(sintabx[ang+256]*scale)/256; 
	ddy=(sintabx[ang]*scale)/256; 
 
	d2x=(sintaby[ang+256]*scale)/256; 
	d2y=(sintaby[ang]*scale)/256; 
 

//	x=128*128*128; 
//	y=128*128*128; 
	i=x-ddx*centerx-d2x*centery; 
	j=y-ddy*centerx-d2y*centery; 
 
	//printf("i: %x j: %x\n",i,j); 
 
	for(yy=0; yy<height; yy++) 
	{ 
		ii=i; 
		jj=j; 
		for(xx=0; xx<width; xx++) 
		{ 
//			*dst=src[((ii&0x3f0000)>>16)+((jj&0x3f0000)>>10)]; 
#if(PREV==NEW)
			*dst=(*dst+src[((ii&0x3f0000)>>16)+((jj&0x3f0000)>>10)]+1)/2; 
#else
			*dst=(
				(*dst)*PREV+
				(src[((ii&0x3f0000)>>16)+((jj&0x3f0000)>>10)]+1)*NEW
				)/(PREV+NEW);
#endif
			dst++; 
			ii+=ddx; 
			jj+=ddy; 
		} 
		i+=d2x; 
		j+=d2y; 
	} 
} 
 
#endif