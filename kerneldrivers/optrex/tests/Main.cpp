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
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
//-------------------------------------
#include <kernel/OS.h>
//-------------------------------------
#include "optrex/optrex_driver.h"
//------------------------------------------------------------------------------

int optrix;

void AnimClock( int glyp, int frame )
{
	static const uint8 clockdata[8][8] = {
		{0x00,0x0e,0x15,0x15,0x11,0x0e,0x00,0x00},
		{0x00,0x0e,0x13,0x15,0x11,0x0e,0x00,0x00},
		{0x00,0x0e,0x11,0x17,0x11,0x0e,0x00,0x00},
		{0x00,0x0e,0x11,0x15,0x13,0x0e,0x00,0x00},
		{0x00,0x0e,0x11,0x15,0x15,0x0e,0x00,0x00},
		{0x00,0x0e,0x11,0x15,0x19,0x0e,0x00,0x00},
		{0x00,0x0e,0x11,0x1d,0x11,0x0e,0x00,0x00},
		{0x00,0x0e,0x19,0x15,0x11,0x0e,0x00,0x00} 
	};

	ioctl( optrix, OPTREX_UPLOADGLYP0+glyp, clockdata[frame%8] );
}

int main()
{
	optrix = open( "/dev/misc/optrex", O_RDWR );
	assert( optrix >= 0 );

	const char scrolltext[] = 
		"Real programmers don't comment their code.  It was hard to write, it should be hard to understand. -- "
		"Real programmers don't draw flowcharts.  Flowcharts are, after all, the illiterate's form of documentation.  Cavemen drew flowcharts; look how much good it did them. -- "
		"Real Programmers don't play tennis, or any other sport that requires you to change clothes.  Mountain climbing is OK, and real programmers wear their climbing boots to work in case a mountain should suddenly spring up in the middle of the machine room. -- ";

	char disp[OPTREX_HEIGHT][OPTREX_WIDTH];
	memset( disp, ' ', sizeof(disp) );

	int frameindex = 0;
	int scrollindex = 0;
	while( 1 )
	{
		disp[0][0] = '[';
		disp[0][1] = 0x03;
		disp[0][2] = 0x02;
		disp[0][3] = 0x01;
		disp[0][4] = 0x00;
		disp[0][5] = ']';
		AnimClock( 0, frameindex );
		AnimClock( 1, frameindex/8 );
		AnimClock( 2, frameindex/(8*8) );
		AnimClock( 3, frameindex/(8*8*8) );
		frameindex++;
		
		memmove( &disp[1][0], &disp[1][1], OPTREX_WIDTH-1 );
		if( scrolltext[scrollindex] == '\0' ) scrollindex = 0;
		disp[1][OPTREX_WIDTH-1] = scrolltext[scrollindex++];

		write( optrix, disp, OPTREX_HEIGHT*OPTREX_WIDTH );

		snooze( 1000000 / 10 );
	}
	
	close( optrix );
	return 0;
}