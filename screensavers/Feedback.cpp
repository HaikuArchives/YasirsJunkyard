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
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
//-------------------------------------
#include <kernel/OS.h>
//-------------------------------------
#include "BitmapRot.h"
#include "Feedback.h"
//#include "YUV2RGB.h"
//#include "YUV2RGB_1632.h"
//-----------------------------------------------------------------------------

class Waver
{
public:
	Waver( int sines, float velmin,float velmax, float velvelmin,float velvelmax );
	~Waver();
	
	void Update();
	void GetData( float *res, int len, float min, float max );
	void GetData( int *res, int len, int min, int max );

private:
	int		fSinCnt;
	float	*fSinPos;
	float	*fSinScale;
	float	*fSinVel;
	float	*fSinVelVel;
};

#define FRAND(l,h) ((float(rand())/float(RAND_MAX)) * ((h)-(l)) + (l))
#define CLAMP(v,l,h) ((v)<(l)?(l):(v)>(h)?(h):(v))

Waver::Waver( int sines, float velmin,float velmax, float velvelmin,float velvelmax )
{
	fSinCnt = sines;
	fSinPos = new float[fSinCnt];
	fSinScale = new float[fSinCnt];
	fSinVel = new float[fSinCnt];
	fSinVelVel = new float[fSinCnt];
	
	float scaletot = 1.0f;
	
	for( int i=0; i<fSinCnt; i++ )
	{
		float scalemin = CLAMP( 1.0f/float(fSinCnt) - scaletot/3.0f, 0.0f, scaletot );
		float scalemax = CLAMP( 1.0f/float(fSinCnt) + scaletot/3.0f, 0.0f, scaletot );
		float scale = i==fSinCnt-1 ? scaletot : FRAND( scalemin, scalemax );

		fSinPos[i] = 0.0f;
		fSinVel[i] = FRAND( velmin/scale*(1.0f/fSinCnt), velmax/scale*(1.0f/fSinCnt) );
//		fSinVel[i] = FRAND( velmin, velmax );
		fSinVelVel[i] = FRAND( velvelmin, velvelmax );
		
		fSinScale[i] = scale;
		scaletot -= scale;
		printf( "%d: vel:%f velvel:%f scale:%.3f\n", i, fSinVel[i], fSinVelVel[i], fSinScale[i] );
	}
}

Waver::~Waver()
{
	delete fSinPos;
	delete fSinScale;
	delete fSinVel;
	delete fSinVelVel;
}
	
void Waver::Update()
{
	for( int i=0; i<fSinCnt; i++ )
		fSinPos[i] += fSinVelVel[i];
}

void Waver::GetData( float *res, int len, float min, float max )
{
	float pos[fSinCnt];
	for( int i=0; i<fSinCnt; i++ )
		pos[i] = fSinPos[i];
	
	for( int i=0; i<len; i++ )
	{
		float v = 0.0f;
		for( int j=0; j<fSinCnt; j++ )
		{
			v += sin( pos[j] );
			pos[j] += fSinVel[j];
		}
		v = v/(fSinCnt*2.0f) + 0.5f;
		v = v*(max-min)+min;
		res[i] = v;
	}
}

void Waver::GetData( int *res, int len, int min, int max )
{
	float pos[fSinCnt];
	for( int i=0; i<fSinCnt; i++ )
		pos[i] = fSinPos[i];
	
//	float scale = 1.0f / ((fSinCnt*2.0f)/(max-min));
//	float add = 0.5f*float(max-min) + float(min);
	float scale = 1.0f / ((2.0f)/(max-min));
	float add = 0.5f*float(max-min) + float(min);
	
	for( int i=0; i<len; i++ )
	{
		float v = 0.0f;
		for( int j=0; j<fSinCnt; j++ )
		{
			v += sin( pos[j] ) * fSinScale[j];
			pos[j] += fSinVel[j];
		}
		v = v*scale + add;
		res[i] = int( v + 0.5f );
	}
}

//-----------------------------------------------------------------------------

Feedback::Feedback( color_space colorspace ) :
	fYUV2RGB( colorspace )
{
	fYBits[0] = new uint8[256*256];
	fYBits[1] = new uint8[256*256];
	fUBits[0] = new uint8[128*128];
	fUBits[1] = new uint8[128*128];
	fVBits[0] = new uint8[128*128];
	fVBits[1] = new uint8[128*128];
	
	fFrame = 0;
}

Feedback::~Feedback()
{
	delete fYBits[0];
	delete fYBits[1];
	delete fUBits[0];
	delete fUBits[1];
	delete fVBits[0];
	delete fVBits[1];
}

const float yvel = 3.1415f/10.0f;
const float yvelvel = 3.1415f/100.0f;

const float uvvel = 3.1415f/100.0f;
const float uvvelvel = 3.1415f/200.0f;

Waver yxwaver( 3, -yvel,yvel, -yvelvel,yvelvel );
Waver yywaver( 3, -yvel,yvel, -yvelvel,yvelvel );
Waver uxwaver( 3, -uvvel,uvvel, -uvvelvel,uvvelvel );
Waver uywaver( 3, -uvvel,uvvel, -uvvelvel,uvvelvel );
Waver vxwaver( 3, -uvvel,uvvel, -uvvelvel,uvvelvel );
Waver vywaver( 3, -uvvel,uvvel, -uvvelvel,uvvelvel );

void Feedback::UpdateFrame()
{
	yxwaver.Update();
	yywaver.Update();

	uxwaver.Update();
	uywaver.Update();

	vxwaver.Update();
	vywaver.Update();
}

//-----------------------------------------------------------------------------

void Feedback::Blit(
	uint8 *src,
	int srcx, int srcy,
	uint8 *dst,
	int dstx, int dsty,
	int bltwidth, int bltheight,
	int width, int height,
	bool wrap )
{
	if( wrap )
	{
	}
	else
	{
		assert( srcx>=0 && srcx+bltwidth<=width && srcy>=0 && srcy+bltheight<=height );

		if( dstx < 0 )
		{
			srcx += -dstx;
			bltwidth -= -dstx;
			dstx = 0;
		}
		if( dstx+bltwidth > width )
			bltwidth -= (dstx+bltwidth)-width;
		if( bltwidth <= 0 ) return;
			
		if( dsty < 0 )
		{
			srcy += -dsty;
			bltheight -= -dsty;
			dsty = 0;
		}
		if( dsty+bltheight > height )
			bltheight -= (dsty+bltheight)-height;
		if( bltheight <= 0 ) return;
			
		for( int i=0; i<bltheight; i++ )
		{
			memcpy(
				dst + (dsty+i)*width + dstx,
				src + (srcy+i)*width + srcx,
				bltwidth );
		}
	}
}

void Feedback::Zoomer( uint8 *src, uint8 *dst, int width, int height, int fillsize, int fillrad, int bltsize, bool wrap )
{
	int xdot = (rand()%(fillrad-fillsize)) + (width/2-fillrad/2);
	int ydot = (rand()%(fillrad-fillsize)) + (height/2-fillrad/2);
	for( int iy=0; iy<fillsize; iy++ )
	{
		for( int ix=0; ix<fillsize; ix++ )
		{
			uint8 v = src[(ydot+iy)*width+(xdot+ix)];
			if( !wrap )
			{
				if( v < 64 ) v += 1;
				else if( v > 192 ) v -= 1;
				else v += (rand()%3)-1;
			}
			else
				v += 1;
			src[(ydot+iy)*width+(xdot+ix)] = v;
		}
	}

	for( int iy=0; iy<height; iy+=bltsize )
	{
		for( int ix=0; ix<width; ix+=bltsize )
		{
//			int xdisp = (rand()%5)-2;
//			int ydisp = (rand()%5)-2;
			int xdisp = (ix+bltsize/2)-width/2;
			int ydisp = (iy+bltsize/2)-height/2;
//			float len = sqrt( xdisp*xdisp + ydisp*ydisp );
//			xdisp = xdisp*8/len;
//			ydisp = ydisp*8/len;
//			if( xdisp<-16 || xdisp>16 ) xdisp/=8;
//			if( ydisp<-16 || ydisp>16 ) ydisp/=8;
			float ang = atan2( xdisp, ydisp );
			float dist = sqrt( xdisp*xdisp + ydisp*ydisp );
//			ang += dist/100.0f;
			ang += 10.0f / (dist+1);
			xdisp = (int)sin(ang)*2;
			ydisp = (int)cos(ang)*2;
			
			xdisp += (rand()%3)-1;
			ydisp += (rand()%3)-1;
			
			Blit( src,ix,iy, dst,ix+xdisp,iy+ydisp, bltsize,bltsize, width,height, false );
		}
	}
}

#if 0
void Feedback::GetFrame( void *bits, int width, int height, int bpr, bool scale )
{
	int srcframe = fFrame;
	int dstframe = (fFrame+1)%2;

	Zoomer( fYBits[srcframe], fYBits[dstframe], 256,256, 8,16, 16, false );
	Zoomer( fUBits[srcframe], fUBits[dstframe], 128,128, 4,8, 16, true );
	Zoomer( fVBits[srcframe], fVBits[dstframe], 128,128, 4,8, 16, true );

	fYUV2RGB.Convert( width,height, fYBits[dstframe],fUBits[dstframe],fVBits[dstframe], 256,128,128, bits, bpr, scale );

	fFrame = (fFrame+1)%2;
}
#endif

//-----------------------------------------------------------------------------

#if 1
Waver waver_y_rot_ang( 3, 0.0f,0.0f, -3.1415/60,3.1415/60 );
Waver waver_y_rot_zoom( 3, 0.0f,0.0f, -3.1415/80,3.1415/80 );
Waver waver_y_rot_xpos( 3, 0.0f,0.0f, -3.1415/80,3.1415/80 );
Waver waver_y_rot_ypos( 3, 0.0f,0.0f, -3.1415/80,3.1415/80 );

void Feedback::GetFrame( void *bits, int width, int height, int bpr, bool scale )
{
	static bigtime_t lasttime = 0;
	static int frames = 0;
	bigtime_t nowtime = system_time();
	frames++;
	if( nowtime-lasttime > 1000000 )
	{
		printf( "fps:%f\n", (1000000.0f*frames) / float(nowtime-lasttime) );
		lasttime = nowtime;
		frames = 0;
	}

	int srcframe = fFrame;
	int dstframe = (fFrame+1)%2;
	fFrame = (fFrame+1)%2;

#if 0
	int yx[256];
	int yy[256];
	yxwaver.GetData( yx, 256, 0, 127 );
	yywaver.GetData( yy, 256, 0, 128 );
	for( int iy=0; iy<256; iy++ )
	{
		uint8 *yline = (fYBits[0]+iy*256);
		for( int ix=0; ix<256; ix++ )
		{
			*yline++ = yy[iy]+yx[ix];
		}
	}
#endif
#if 0
	int yx[256];
	int yy[256];
	yxwaver.GetData( yx, 256, -127, 127 );
	yywaver.GetData( yy, 256, -127, 127 );
	for( int iy=0; iy<256; iy++ )
	{
		uint8 *yline = (fYBits[0]+iy*256);
		for( int ix=0; ix<256; ix++ )
		{
			*yline++ = ((yy[iy]*yx[ix])>>8) + 128;
		}
	}
#endif
#if 1
	float y_rot_ang;
	waver_y_rot_ang.Update();
	waver_y_rot_ang.GetData( &y_rot_ang, 1, -3.14f/30.0f, 3.14f/30.0f );

	float y_rot_zoom;
	waver_y_rot_zoom.Update();
	waver_y_rot_zoom.GetData( &y_rot_zoom, 1, 0.8f, 1.0f );

	float y_rot_xpos;
	waver_y_rot_xpos.Update();
	waver_y_rot_xpos.GetData( &y_rot_xpos, 1, 127.5f-32.0f, 127.5f+32.0f );

	float y_rot_ypos;
	waver_y_rot_ypos.Update();
	waver_y_rot_ypos.GetData( &y_rot_ypos, 1, 127.5f-32.0f, 127.5f+32.0f );

	RotatorRotate256(
		fYBits[dstframe], fYBits[srcframe],
		y_rot_xpos, y_rot_ypos,
//		128.0f, 128.0f,
		y_rot_zoom, y_rot_ang,
		256, 256 );

	int yx[256];
	int yy[256];
	yxwaver.GetData( yx, 256, -127, 127 );
	yywaver.GetData( yy, 256, -127, 127 );
	for( int iy=128-32; iy<128+32; iy++ )
	{
		uint8 *yline = (fYBits[dstframe]+iy*256);
		for( int ix=128-32; ix<128+32; ix++ )
		{
			uint8 dstcol = (((yy[iy]*yx[ix])>>8)+128 );
			uint8 srccol = yline[ix];
			yline[ix] = (dstcol+srccol*31+15)/32;
		}
	}
#endif

#if 1
	int ux[128];
	int uy[128];
	uxwaver.GetData( ux, 128, 0, 127 );
	uywaver.GetData( uy, 128, 0, 128 );
	for( int iy=0; iy<128; iy++ )
	{
		uint8 *uline = (fUBits[0]+iy*128);
		for( int ix=0; ix<128; ix++ )
		{
			*uline++ = uy[iy]+ux[ix];
		}
	}
#else
	int ux[128];
	int uy[128];
	uxwaver.GetData( ux, 128, -127, 127 );
	uywaver.GetData( uy, 128, -127, 127 );
	for( int iy=0; iy<128; iy++ )
	{
		uint8 *uline = (fUBits[0]+iy*128);
		for( int ix=0; ix<128; ix++ )
		{
			*uline++ = ((uy[iy]*ux[ix])>>8) + 128;
		}
	}
#endif

#if 1
	int vx[128];
	int vy[128];
	vxwaver.GetData( vx, 128, 0, 127 );
	vywaver.GetData( vy, 128, 0, 128 );
	for( int iy=0; iy<128; iy++ )
	{
		uint8 *vline = (fVBits[0]+iy*128);
		for( int ix=0; ix<128; ix++ )
		{
			*vline++ = vy[iy]+vx[ix];
		}
	}
#else
	int vx[128];
	int vy[128];
	vxwaver.GetData( vx, 128, -127, 127 );
	vywaver.GetData( vy, 128, -127, 127 );
	for( int iy=0; iy<128; iy++ )
	{
		uint8 *vline = (fVBits[0]+iy*128);
		for( int ix=0; ix<128; ix++ )
		{
			*vline++ = ((vy[iy]*vx[ix])>>8) + 128;
		}
	}
#endif

	fYUV2RGB.Convert( width,height, fYBits[dstframe],fUBits[0],fVBits[0], 256,128,128, bits, bpr, scale );
}
#endif

//-----------------------------------------------------------------------------
