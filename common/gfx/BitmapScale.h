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
#ifndef DAMN_BITMAPSCALE_H
#define DAMN_BITMAPSCALE_H
//-----------------------------------------------------------------------------
//-------------------------------------
#include <interface/Bitmap.h>
//class BBitmap;
//-------------------------------------
//-----------------------------------------------------------------------------

/** Most of the common stuff in the Junkyard is wrappen in this namespace.
 *
 */
namespace damn
{
	/** Filters that works with damn::Scale()
	 *
	 */
	enum bitmapscale_filtertype
	{
		filter_point,
		filter_box,
		filter_triangle,
		filter_bell,		///< 3rd order (quadratic) b-spline
		filter_bspline,		///< 4th order (cubic) b-spline
		filter_catrom,		///< Catmull-Rom spline, Overhauser spline
		filter_gaussian,
		filter_sinc,		///< Sinc, perfect lowpass filter (infinite)
		filter_bessel,		///< Bessel (for circularly symm. 2-d filt, inf)
		filter_mitchell,	///< Mitchell
		filter_hanning,
		filter_hamming,
		filter_blackman,
		filter_kaiser,		///< Kaiser window
		filter_normal,
		filter_filter,
		filter_lanczos3,	///< Lanczos3
	};

	void Scale( const BBitmap *srcbitmap, BBitmap *dstbitmap, damn::bitmapscale_filtertype filtertype, float filterwidth=0.0f );
	void Scale(
		const uint32 *srcbits, int srcwidth, int srcheight, int srcppr,
		uint32 *dstbits, int dstwidth, int dstheight, int dstppr,
		damn::bitmapscale_filtertype filtertype,
		float filterwidth=0.0f );
}

//-----------------------------------------------------------------------------
#endif

