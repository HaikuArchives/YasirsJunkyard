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
#include "ssc_private.h"
//-----------------------------------------------------------------------------

static int ssc_readnib( ssc_cookie *ssc, bigtime_t timeout );

static status_t ssc_togglestrobe( ssc_cookie *ssc, bigtime_t delay );
static status_t ssc_setstrobe( ssc_cookie *ssc, bigtime_t delay, bool set );

#define SSC_TIMEOUT_WRITE (1000000)
#define SSC_TIMEOUT_READ (1000000)

//-----------------------------------------------------------------------------

int ssc_getcardtype( ssc_cookie *ssc )
{
	status_t status;
	int val;

	status = ssc_setmode( ssc, SSC_CMD_CARD_TYPE );
	if( status < B_OK )
	{
		dprintf( "ssc>ssc_getcardtype failed...\n" );
		return status;
	}

	val = ssc_readbyte( ssc );

//	dprintf( "ssc>ssc_getcardtype: %02X\n", val );

	return val;
}

status_t ssc_clearflash( ssc_cookie *ssc, uint32 bankmask )
{
	status_t status;
	int clearcmd = -1;
	int i;

	if( bankmask == 0x1f ) clearcmd = SSC_CMD_CLEAR_BANK_ALL;
	else if( bankmask == 0x01 ) clearcmd = SSC_CMD_CLEAR_BANK_0;
	else if( bankmask == 0x02 ) clearcmd = SSC_CMD_CLEAR_BANK_1;
	else if( bankmask == 0x04 ) clearcmd = SSC_CMD_CLEAR_BANK_2;
	else if( bankmask == 0x08 ) clearcmd = SSC_CMD_CLEAR_BANK_3;
	else if( bankmask == 0x10 ) clearcmd = SSC_CMD_CLEAR_BANK_4;
	else if( bankmask == 0x18 ) clearcmd = SSC_CMD_CLEAR_BANK_3_4;
	else
	{
		dprintf( "ssc>ssc_clearflash(): illegal gank mask: 0x%02lx\n", bankmask );
		return B_ERROR;
	}

	status = ssc_setmode( ssc, clearcmd );
	if( status < B_OK )
	{
		dprintf( "ssc>ssc_clearflash(): could not send command: 0x%02x\n", clearcmd );
		return status;
	}

	for( i=10000000/SSC_TIMEOUT_READ; i>0; i-- )
	{
		int val = ssc_readbyte( ssc );

		if( val == 0xaa )
			break;

		if( val != B_TIMED_OUT )
		{
			dprintf( "SSC>ssc_clearflash(): could not read status byte\n" );
			return status;
		}
	}

	return B_OK;
}

//-----------------------------------------------------------------------------

status_t ssc_setmode( ssc_cookie *ssc, int mode )
{
	status_t status;

	status = ssc_writebyte( ssc, 0x55 );
	if( status < B_OK ) return status;
	
	status = ssc_writebyte( ssc, mode );
	return status;
}

//-----------------------------------------------------------------------------

int ssc_read( ssc_cookie *ssc, void *buffer, int num_bytes )
{
	int i;
	for( i=0; i<num_bytes; i++ )
	{
		int v = ssc_readbyte( ssc );
		if( v < B_OK )
			return i;
		((uint8*)buffer)[i] = v;
	}
	return i;
}

int ssc_write( ssc_cookie *ssc, const void *buffer, int num_bytes )
{
	int i;
	for( i=0; i<num_bytes; i++ )
	{
		if( ssc_writebyte(ssc, ((uint8*)buffer)[i]) < B_OK )
			return i;
	}
	return i;
}

//-----------------------------------------------------------------------------

void ssc_reset( ssc_cookie *ssc )
{
	outp( ssc->par_port_ctrl, 0xce );	// 11111110
	snooze( 2500 );						// I don't have docs, so I hope this is enough.
	outp( ssc->par_port_ctrl, 0xcc );	// 11111100
	snooze( 2500 );
}

int ssc_readbyte( ssc_cookie *ssc )
{
	int datal, datah;
	
	datal = ssc_readnib( ssc, SSC_TIMEOUT_READ );
	if( datal < B_OK ) return datal;

	datah = ssc_readnib( ssc, SSC_TIMEOUT_READ );
	if( datah < B_OK ) return datah;

	return datah<<4 | datal;
}

static int ssc_readnib( ssc_cookie *ssc, bigtime_t timeout )
{
	status_t status;
	int data;
	
	status = ssc_setstrobe( ssc, timeout, true );
	if( status < B_OK ) return status;
	data = inp( ssc->par_port_status );
	status = ssc_setstrobe( ssc, timeout, false );
	if( status < B_OK ) return status;

	return ((data>>4)&0xf)^0x8;
}

status_t ssc_writebyte( ssc_cookie *ssc, int val )
{
	status_t status;
	
	outp( ssc->par_port_data, val );

	status = ssc_togglestrobe( ssc, SSC_TIMEOUT_WRITE );
	if( status < B_OK )
		dprintf( "ssc>ssc_writebyte: failed...\n" );

	return status;
}

//-----------------------------------------------------------------------------

static status_t ssc_togglestrobe( ssc_cookie *ssc, bigtime_t delay )
{
	status_t status;

	status = ssc_setstrobe( ssc, delay, true );
	if( status < B_OK ) return status;

	status = ssc_setstrobe( ssc, delay, false );
	return status;
}

static status_t ssc_setstrobe( ssc_cookie *ssc, bigtime_t delay, bool set )
{
	bigtime_t starttime = system_time();

	outp( ssc->par_port_ctrl, 0xcc | (set?0x01:0x00) ); // 1111110x

	while( 1 )
	{
		bigtime_t elapsedtime = system_time()-starttime;
		int data = inp( ssc->par_port_status );
//		dprintf( "%02X\n", data );
	
		if( (data&0x08) == (set?0x00:0x08) )
			return B_OK;

		if( elapsedtime > delay )
		{
			dprintf( "ssc>ssc_setstrobe(): timeout...\n" );
			return B_TIMED_OUT;
		}
	}
}

//-----------------------------------------------------------------------------

