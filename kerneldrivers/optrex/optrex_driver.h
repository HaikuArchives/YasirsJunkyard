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
#ifndef DAMN_OPTREXDRIVER_H
#define DAMN_OPTREXDRIVER_H
//-----------------------------------------------------------------------------
//-------------------------------------
#include <drivers/Drivers.h>
//-------------------------------------
//-----------------------------------------------------------------------------

#define OPTREX_WIDTH 16
#define OPTREX_HEIGHT 2

//-----------------------------------------------------------------------------

enum
{
	OPTREX_UPLOADLINE0 = B_DEVICE_OP_CODES_END+1,
	OPTREX_UPLOADLINE1,

	OPTREX_UPLOADALLLINES = OPTREX_UPLOADLINE1+31,

	OPTREX_UPLOADGLYP0,
	OPTREX_UPLOADGLYP1,
	OPTREX_UPLOADGLYP2,
	OPTREX_UPLOADGLYP3,
	OPTREX_UPLOADGLYP4,
	OPTREX_UPLOADGLYP5,
	OPTREX_UPLOADGLYP6,
	OPTREX_UPLOADGLYP7
};

//-----------------------------------------------------------------------------
#endif
