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
#include <drivers/KernelExport.h>
//-------------------------------------
#include "utils.h"
//-----------------------------------------------------------------------------

static void formathex( char *string, unsigned int val, int nibbels )
{
	int i;
	for( i=0; i<nibbels; i++ )
		string[i] = "0123456789abcdef"[(val>>((nibbels-(i+1))*4))&0xf];
}

// 00000000001111111111222222222233333333334444444444555555555566666666667777777777
// 01234567890123456789012345678901234567890123456789012345678901234567890123456789
// 00000000  00 00 00 00 00 00 00 00  00 00 00 00 00 00 00 00  xxxxxxxxxxxxxxxx
void dumphex( void *data, int length )
{
	int i, j;
	for( i=0; i<length; i+=16 )
	{
		char line[80+1];
		int sublen = length-i>16 ? 16 : length-i;

		memset( line, ' ', sizeof(line) );

		formathex( line+0, i, 8 );
		for( j=0; j<sublen; j++ )
		{
			unsigned char v = ((unsigned char*)data)[i+j];
			formathex( line+(j<8?10:11)+j*3, v, 2 );
			line[60+j] = v>=0x20 ? v : '.';
		}
		line[76] = '\n';
		line[77] = 0;
		dprintf( line );
	}
}
