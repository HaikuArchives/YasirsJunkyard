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
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
//-------------------------------------
#include <kernel/OS.h>
//-------------------------------------
#include "w8378x/w8378x_driver.h"
//-----------------------------------------------------------------------------

int main()
{
	int hf = open( "/dev/misc/w8378x", O_RDWR );
	printf( "hf = %d\n", hf );

	if( hf < 0 )
	{
		printf( "Failed to open device: %s\n", strerror(errno) );
		abort();
	}

	while( 1 )
	{
		int temp1=0, temp2=0, temp3=0;
		int fan1=0, fan2=0, fan3=0;

		ioctl( hf, W8378x_READ_TEMP1, &temp1 );
		ioctl( hf, W8378x_READ_TEMP2, &temp2 );
		ioctl( hf, W8378x_READ_TEMP3, &temp3 );

		ioctl( hf, W8378x_READ_FAN1, &fan1 );
		ioctl( hf, W8378x_READ_FAN2, &fan2 );
		ioctl( hf, W8378x_READ_FAN3, &fan3 );

		printf( "Temp1:%.1fc  Temp2:%.1fc  Temp3:%.1fc  Fan1:%drpm  Fan2:%drpm  Fan3:%drpm\n",
			float(temp1)/256.0f, float(temp2)/256.0f, float(temp3)/256.0f,
			fan1, fan2, fan3 );
		
		snooze( 2000000 );
	}

	close( hf );
	return 0;
}

