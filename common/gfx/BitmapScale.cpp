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
//-------------------------------------
#include <interface/Bitmap.h>
#include <support/SupportDefs.h>
//-------------------------------------
#include "BitmapScale.h"
//-----------------------------------------------------------------------------

struct contrib
{
	int		sourcepos;
	float	floatweight;
	int32	weight;
};

struct contriblist
{
//	contriblist(int cnt) { contribcnt=0; contribs=new contrib[cnt]; }
	~contriblist() { delete[] contribs; }
	int		contribcnt;
	contrib	*contribs;
};

typedef float (scale_filterfunc)(float);

static float Filter_Box( float t );
static float Filter_Triangle( float t );
static float Filter_Bell( float t );
static float Filter_BSpline( float t );
static float Filter_Catrom( float t );
static float Filter_Gaussian( float t );
static float Filter_Sinc( float t );
static float Filter_Bessel( float t );
static float Filter_Mitchell( float t );
static float Filter_Hanning( float t );
static float Filter_Hamming( float t );
static float Filter_Blackman( float t );
static float Filter_Kaiser( float t );
static float Filter_Normal( float t );
static float Filter_Filter( float t );
static float Filter_Lanczos3( float t );

int clamp( int v, int l, int h ) { return v<l?l:v>h?h:v; }

//-----------------------------------------------------------------------------

static contriblist *CalcFilterWeight( float scale, float filterwidth, int srcsize, int dstsize, scale_filterfunc *filterfunc )
{
	contriblist *contriblists = new contriblist[dstsize];

	float size;
	float fscale;
	if( scale < 1.0f )
	{
		size = filterwidth / scale;
		fscale = scale;
	}
	else
	{
		size = filterwidth;
		fscale = 1.0f;
	}
	
	for( int i=0; i<dstsize; i++ )
	{
		contriblists[i].contribcnt = 0;
		contriblists[i].contribs = new contrib[ (int)(size*2+1) ];
		float center = (float)i / scale;
		float start = ceil( center - size );
		float end = floor( center + size );
		float totweight = 0.0f;
//		for( int j=(int)start; j<=end; j++ )
		int j=(int)start;
		do
		{
			int sourcepos;
			if( j < 0 )					sourcepos = -j;
			else if( j >= srcsize )		sourcepos = (srcsize-j)+srcsize-1;
			else						sourcepos = j;
			int newcontrib = contriblists[i].contribcnt++;
			contriblists[i].contribs[newcontrib].sourcepos = sourcepos;

			float weight = filterfunc( (center-(float)j)*fscale ) * fscale;
			totweight += weight;
			contriblists[i].contribs[newcontrib].floatweight = weight;

			j++;
		}
		while( j<=end );

		totweight = 1.0f/totweight;
		for( int j=0; j<contriblists[i].contribcnt; j++ )
		{
			contriblists[i].contribs[j].weight = (int)(contriblists[i].contribs[j].floatweight * totweight * 65530);
		}
		uint val=0;
		for( int j=0; j<contriblists[i].contribcnt; j++ )
		{
			val += contriblists[i].contribs[j].weight;
//			printf( "%d ", contriblists[i].contribs[j].weight );
		}
//		printf( "%X\n", val );
			
	}
	
	return contriblists;
}

// -----------------------------------------------------------------------------

/** Compies \a srcbitmap into \a dstbitmap while scaling/filtering it.
 *
 * \param srcbitmap Source bitmap, must be in B_RGB32 or B_RGBA32 colorspace.
 * \param dstbitmap Destination bitmap, must be in B_RGB32 or B_RGBA32 colorspace.
 * \param filtertype 
 * \param filterwidth Width of filter, set to 0.0f for default.
 *
 * \bug Scaling up with the point filter does not work.
 */
void damn::Scale( const BBitmap *srcbitmap, BBitmap *dstbitmap, damn::bitmapscale_filtertype filtertype, float filterwidth )
{
	assert( dstbitmap != NULL );
	assert( srcbitmap != NULL );

	assert( (srcbitmap->ColorSpace()==B_RGB32) || (srcbitmap->ColorSpace()==B_RGBA32) );
	assert( (dstbitmap->ColorSpace()==B_RGB32) || (dstbitmap->ColorSpace()==B_RGBA32) );

	scale_filterfunc *filterfunc;
	float default_filterwidth;
	
	switch( filtertype )
	{
		case filter_point:		filterfunc=Filter_Box;		default_filterwidth=0.0f;	break;
		case filter_box:		filterfunc=Filter_Box;		default_filterwidth=0.5f;	break;
		case filter_triangle:	filterfunc=Filter_Triangle;	default_filterwidth=1.0f;	break;
		case filter_bell:		filterfunc=Filter_Bell;		default_filterwidth=1.5f;	break;
		case filter_bspline:	filterfunc=Filter_BSpline;	default_filterwidth=2.0f;	break;
		case filter_catrom:		filterfunc=Filter_Catrom;	default_filterwidth=2.0f;	break;
		case filter_gaussian:	filterfunc=Filter_Gaussian;	default_filterwidth=1.25f;	break;
		case filter_sinc:		filterfunc=Filter_Sinc;		default_filterwidth=4.0f;	break;
		case filter_bessel:		filterfunc=Filter_Bessel;	default_filterwidth=3.2383f;break;
		case filter_mitchell:	filterfunc=Filter_Mitchell;	default_filterwidth=2.0f;	break;
		case filter_hanning:	filterfunc=Filter_Hanning;	default_filterwidth=1.0f;	break;
		case filter_hamming:	filterfunc=Filter_Hamming;	default_filterwidth=1.0f;	break;
		case filter_blackman:	filterfunc=Filter_Blackman;	default_filterwidth=1.0f;	break;
		case filter_kaiser:		filterfunc=Filter_Kaiser;	default_filterwidth=1.0f;	break;
		case filter_normal:		filterfunc=Filter_Normal;	default_filterwidth=1.25f;	break;
		case filter_filter:		filterfunc=Filter_Filter;	default_filterwidth=1.0f;	break;
		case filter_lanczos3:	filterfunc=Filter_Lanczos3;	default_filterwidth=3.0f;	break;

		default:				filterfunc=Filter_Filter;	default_filterwidth=1.0f;	break;
	}

	if( filterwidth == 0.0f ) filterwidth = default_filterwidth;

	int srcbitmap_width  = srcbitmap->Bounds().IntegerWidth()+1;
	int srcbitmap_height = srcbitmap->Bounds().IntegerHeight()+1;
	int dstbitmap_width  = dstbitmap->Bounds().IntegerWidth()+1;
	int dstbitmap_height = dstbitmap->Bounds().IntegerHeight()+1;
	int tmpbitmap_width  = dstbitmap_width;
	int tmpbitmap_height = srcbitmap_height;

	uint32 *srcbitmap_bits = (uint32*)srcbitmap->Bits();
	uint32 *dstbitmap_bits = (uint32*)dstbitmap->Bits();
	uint32 *tmpbitmap_bits = new uint32[tmpbitmap_width*tmpbitmap_height];

	int srcbitmap_bpr = srcbitmap->BytesPerRow() / 4;
	int dstbitmap_bpr = dstbitmap->BytesPerRow() / 4;
	int tmpbitmap_bpr = tmpbitmap_width;

	float xscale = float(dstbitmap_width) / float(srcbitmap_width);
	float yscale = float(dstbitmap_height) / float(srcbitmap_height);

//--
	contriblist *contriblists = CalcFilterWeight( xscale, filterwidth, srcbitmap_width, dstbitmap_width, filterfunc );

	for( int iy=0; iy<tmpbitmap_height; iy++ )
	{
		uint32 *src_bits = srcbitmap_bits + iy*srcbitmap_bpr;
		uint32 *dst_bits = tmpbitmap_bits + iy*tmpbitmap_bpr;
		for( int ix=0; ix<tmpbitmap_width; ix++ )
		{
			int32 rweight;
			int32 gweight;
			int32 bweight;
			int32 aweight;
			rweight = gweight = bweight = aweight = 0;

			for( int ix2=0; ix2<contriblists[ix].contribcnt; ix2++ )
			{
				const contrib &scontrib = contriblists[ix].contribs[ix2];
				uint32 color = src_bits[ scontrib.sourcepos ];
				int32 weight = scontrib.weight;

				rweight += ((color    )&0xff) * weight;
				gweight += ((color>> 8)&0xff) * weight;
				bweight += ((color>>16)&0xff) * weight;
				aweight += ((color>>24)&0xff) * weight;
			}

			rweight = clamp(rweight>>16,0,255);
			gweight = clamp(gweight>>16,0,255);
			bweight = clamp(bweight>>16,0,255);
			aweight = clamp(aweight>>16,0,255);
			dst_bits[ ix ] = (rweight) | (gweight<<8) | (bweight<<16) | (aweight<<24) ;
		}
	}

	delete[] contriblists;
//--

	contriblists = CalcFilterWeight( yscale, filterwidth, srcbitmap_height, dstbitmap_height, filterfunc );

	// help cache coherency:
	uint32 *bitmaprow = new uint32[ tmpbitmap_height ];
	for( int ix=0; ix<dstbitmap_width; ix++ )
	{
		for( int iy=0; iy<tmpbitmap_height; iy++ )
			bitmaprow[iy] = tmpbitmap_bits[ix+iy*tmpbitmap_bpr];
			
		for( int iy=0; iy<dstbitmap_height; iy++ )
		{
			int32 rweight;
			int32 gweight;
			int32 bweight;
			int32 aweight;
			rweight = gweight = bweight = aweight = 0;
			
			for( int iy2=0; iy2<contriblists[iy].contribcnt; iy2++ )
			{
				const contrib &scontrib = contriblists[iy].contribs[iy2];
				uint32 color = bitmaprow[ scontrib.sourcepos ];
				int32 weight = scontrib.weight;

				rweight += ((color    )&0xff) * weight;
				gweight += ((color>> 8)&0xff) * weight;
				bweight += ((color>>16)&0xff) * weight;
				aweight += ((color>>24)&0xff) * weight;
			}
			rweight = clamp(rweight>>16,0,255);
			gweight = clamp(gweight>>16,0,255);
			bweight = clamp(bweight>>16,0,255);
			aweight = clamp(aweight>>16,0,255);
			dstbitmap_bits[ ix + iy*dstbitmap_bpr ] = (rweight) | (gweight<<8) | (bweight<<16) | (aweight<<24) ;
	}
	}

	delete[] bitmaprow;
	delete[] contriblists;
	delete[] tmpbitmap_bits;
}

//-----------------------------------------------------------------------------
// box, pulse, Fourier window, 1st order (constant) b-spline

/** \enum damn::bitmapscale_filtertype::filter_point
 * Point filter
 *
 * \f$f(x) = \left\{ \begin{array}{l}
 *		0,	x < -0.5 \\
 *		1,	-0.5 \leq x \leq 0.5 \\
 *		0,	x > 0.5
 * \end{array} \right.\f$
 */

/** \enum damn::bitmapscale_filtertype::filter_box
 * Box, Pulse, Fourier window, 1st order (constant) b-spline
 *
 * \f$f(x) = \left\{ \begin{array}{l}
 *		0,	x < -0.5 \\
 *		1,	-0.5 \leq x \leq 0.5 \\
 *		0,	x > 0.5
 * \end{array} \right.\f$
 */
static float Filter_Box( float t )
{
    if( t < -0.5f ) return 0.0f;
    if( t < 0.5f ) return 1.0f;
    return 0.0f;
}

//-----------------------------------------------------------------------------
// triangle, Bartlett window, 2nd order (linear) b-spline

/** \enum damn::bitmapscale_filtertype::filter_triangle
 * Triangle, Bartlett window, 2nd order (linear) b-spline
 *
 * \f$f(x) = \left\{ \begin{array}{l}
 *		0,			x < -1 \\
 *		1 - | x |,	-1 \leq x \leq 1 \\
 *		0,			x > 1
 * \end{array} \right.\f$
 */
static float Filter_Triangle( float t )
{
	if( t < 0.0f ) t = -t;
	if( t < 1.0f ) return 1.0f - t;
	return 0.0f;
}

//-----------------------------------------------------------------------------
// 3rd order (quadratic) b-spline

static float Filter_Bell( float t )
{
	if( t < 0.0f ) t = -t;
	if( t < 0.5f ) return 0.75f-(t*t);
	if( t < 1.5f ) 
	{
		t = t-1.5f;
		return 0.5f * (t*t);
	}
	return 0.0f;
}

//-----------------------------------------------------------------------------

// 4th order (cubic) b-spline
static float Filter_BSpline( float t )
{
	float tt;

	if( t < 0.0f ) t = -t;
	if( t < 1.0f ) 
	{
		tt = t*t;
		return (0.5f * tt * t) - tt + (2.0f / 3.0f);
	}
	else if( t < 2.0f )
	{
		t = 2.0f-t;
		return (1.0f / 6.0f) * (t * t * t);
	}
	return 0.0f;
}

//-----------------------------------------------------------------------------

// Catmull-Rom spline, Overhauser spline
static float Filter_Catrom( float t )
{
	if( t < 0.0f ) t = -t;
	if( t < 1.0f ) return 0.5f*(2.0f+t*t*(-5.0f+t*3.0f));
	if( t < 2.0f ) return 0.5f*(4.0f+t*(-8.0f+t*(5.0f-t)));
    return 0.0f;
}

//-----------------------------------------------------------------------------
// Gaussian (infinite)

/** \enum damn::bitmapscale_filtertype::filter_gaussian
 * Gaussian (infinite)
 *
 * \f$ f(x) = e ^ { -2 x ^ 2 } \times \sqrt { \frac 2 \pi } \f$
 */
static float Filter_Gaussian( float t )
{
	return exp(-2.0f*t*t) * sqrt(2.0f/PI);
}

//-----------------------------------------------------------------------------
// Sinc, perfect lowpass filter (infinite)

static float Filter_Sinc( float t )
{
	if( t == 0.0f ) return 1.0f;
	t *= PI;
	return sin(t) / t;
}

//-----------------------------------------------------------------------------
// Bessel (for circularly symm. 2-d filt, inf), See Pratt "Digital Image Processing" p. 97

static float Filter_Bessel( float t )
{
	if( t == 0.0f ) return PI/4.0f;
	return j1(t*PI) / (t*2.0f); // j1 is order 1 bessel function from the stdlib.
}

//-----------------------------------------------------------------------------

static const float mitchell_b = 1.0f / 3.0f;
static const float mitchell_c = 1.0f / 3.0f;

static float Filter_Mitchell( float t )
{
	float tt;

	tt = t * t;
	if( t < 0.0f ) t = -t;
	if( t < 1.0f ) 
	{
		t = ((12.0f - 9.0f*mitchell_b - 6.0f*mitchell_c) * (t * tt)) +
			((-18.0f + 12.0f*mitchell_b + 6.0f*mitchell_c) * tt) + (6.0f - 2.0f*mitchell_b);
		return t/6.0f;
	} 
	else if( t < 2.0f ) 
	{
		t = ((-1.0f*mitchell_b - 6.0f*mitchell_c) * (t * tt)) + ((6.0f*mitchell_b + 30.0f*mitchell_c) * tt) +
			((-12.0f*mitchell_b - 48.0f*mitchell_c) * t) + (8.0f*mitchell_b + 24.0f*mitchell_c);
		return t/6.0f;
	}
	return 0.0f;
}

//-----------------------------------------------------------------------------
// Hanning window

/** \enum damn::bitmapscale_filtertype::filter_hanning
 * Hanning window
 *
 * \f$ f(x) = \frac 1 2 + \frac 1 2 \cos( \pi x ) \f$
 */
static float Filter_Hanning( float t )
{
	return 0.5f + 0.5f*cos(t*PI);
}

//-----------------------------------------------------------------------------
// Hamming window

/** \enum damn::bitmapscale_filtertype::filter_hamming
 * Hamming window
 *
 * \f$ f(x) = \frac{27}{50} + \frac{23}{50} \cos( \pi x ) \f$
 */
static float Filter_Hamming( float t )
{
	return 0.54f + 0.46f*cos(t*PI);
}

//-----------------------------------------------------------------------------
// Blackman window

/** \enum damn::bitmapscale_filtertype::filter_blackman
 * Blackman window
 *
 * \f$ f(x) = \frac{21}{50} + \frac{25}{50} \cos( \pi x ) + \frac{4}{50} \cos( 2 \pi x ) \f$
 */
static float Filter_Blackman( float t )
{
	return 0.42f + 0.50f*cos(t*PI) + 0.08f*cos(t*2.0f*PI);
}

//-----------------------------------------------------------------------------

#define EPSILON 1e-7

static float bessel_i0( float t )
{
	float tt4 = t*t/4.0f;
	float t2 = tt4;
	
	float sum = 1.0f;
	for( int i=2; t2>EPSILON; i++ )
	{
		sum += t2;
		t2 *= tt4 / float(i*i);
    }

    return sum;
}

static float kaiser_a = 6.5f;
static float kaiser_i0a = 1.0f / bessel_i0(kaiser_a);

// Kaiser window
static float Filter_Kaiser( float t )
{
    return bessel_i0( kaiser_a * sqrt(1.0f-t*t) ) * kaiser_i0a;
}

//-----------------------------------------------------------------------------
// Normal(x) = Gaussian(x/2)/2

/** \enum damn::bitmapscale_filtertype::filter_normal
 * normal(x) = gaussian(x/2)/2
 *
 * \f$ f(x) = \frac{\mbox{$ e ^ {\mbox{$ - \frac{x}{2} x $}} $}}{\mbox{$ \sqrt { 2 \pi } $}} \f$
 */
static float Filter_Normal( float t )
{
	return exp( -t * t/2.0f ) / sqrt(2.0f*PI);
}

//-----------------------------------------------------------------------------
// Filter

/** \enum damn::bitmapscale_filtertype::filter_filter
 * Filter
 *
 * \f$f(x) = \left\{ \begin{array}{l}
 *		0,								x < -1 \\
 *		f(x) = 2|x|^3 - 3|x|^2 + 1,		-1 \leq x \leq 1 \\
 *		0,								x > 1
 * \end{array} \right.\f$
 */
static float Filter_Filter( float t )
{
	if( t < 0.0f ) t = -t;
	if( t < 1.0f ) return (2.0f * t - 3.0f) * t * t + 1.0f;
	return 0.0f;
}

//-----------------------------------------------------------------------------

static float Filter_Lanczos3( float t )
{
#if 0
	if( t < 0.0f ) t = -t;
	if( t < 3.0 ) return sinc(t) * sinc(t/3.0f);
	return 0.0f;
#else
	if( t == 0.0f ) return 1.0f * 1.0f;
	if( t < 3.0f )
	{
		t *= PI;
//		return sin(t)/t * sin(t/3.0f)/(t/3.0f);
		return 3.0f*sin(t)*sin(t*(1.0f/3.0f)) / (t*t);
	}
	return 0.0f;
#endif
}

//-----------------------------------------------------------------------------


