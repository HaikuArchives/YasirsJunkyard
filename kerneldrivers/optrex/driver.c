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
#include <malloc.h>
//-------------------------------------
#include <drivers/ISA.h>
#include <drivers/Drivers.h>
#include <drivers/KernelExport.h>
#include <kernel/OS.h>
//-------------------------------------
#include "optrex_driver.h"
//-----------------------------------------------------------------------------

static status_t optrex_open( const char *name, uint32 flags, void **cookie );
static status_t optrex_close( void *cookie );
static status_t optrex_free( void *cookie );
static status_t optrex_control( void *cookie, uint32 op, void *arg, size_t len );
static status_t optrex_read( void *cookie, off_t position, void *buf, size_t *num_bytes );
static status_t optrex_write( void *cookie, off_t position, const void *buffer, size_t *num_bytes );

static const char *optrex_name[] = {
	"misc/optrex",
	NULL
};

device_hooks optrex_hooks = 
{
	optrex_open, 			/* -> open entry point */
	optrex_close, 			/* -> close entry point */
	optrex_free,			/* -> free cookie */
	optrex_control, 		/* -> control entry point */
	optrex_read,			/* -> read entry point */
	optrex_write			/* -> write entry point */
};

typedef struct
{
	int		device_id;
	int		par_port;
	
	int		cursor_x;
	int		cursor_y;
	
	uint8	vram[OPTREX_HEIGHT][OPTREX_WIDTH];
	uint8	oldvram[OPTREX_HEIGHT][OPTREX_WIDTH];
} optrex_cookie;

struct isa_module_info *isa = NULL;

static long in_use;

#define inp(_port) (isa->read_io_8(_port))
#define outp(_port,_data) (isa->write_io_8(_port, _data))

#define MAKECTRL(_REGSEL,_RDWR,_ENABLE) ( ((_REGSEL)?0x00:0x01) | ((_RDWR)?0x00:0x02) | ((_ENABLE)?0x04:0x00) )

//-----------------------------------------------------------------------------

#define OPTREX_CLEAR			0x01	// 0 0 0 0 0 0 0 1 - clear the display, and move the coursor to pos 0,0
#define OPTREX_HOME				0x02	// 0 0 0 0 0 0 1 ? - move the coursor to pos 0,0
#define OPTREX_EM				0x04	// 0 0 0 0 0 1 . . - entry mode
#define OPTREX_EM_DEC			0x04	// . . . . . . 0 . - entry mode - decrement
#define OPTREX_EM_INC			0x06	// . . . . . . 1 . - entry mode - increment
#define OPTREX_EM_NOSHIFT		0x04	// . . . . . . . 0 - entry mode - dont shift display
#define OPTREX_EM_SHIFT			0x05	// . . . . . . . 1 - entry mode - shift display
#define OPTREX_ONOFF			0x08	// 0 0 0 0 1 . . . - on/off
#define OPTREX_ONOFF_NOFLASH 	0x08	// . . . . . . . 0 - on/off - dont flash character at cur pos
#define OPTREX_ONOFF_FLASH		0x09	// . . . . . . . 1 - on/off - flash character at cur pos
#define OPTREX_ONOFF_CUROFF		0x08	// . . . . . . 0 . - on/off - cursor off
#define OPTREX_ONOFF_CURON		0x0A	// . . . . . . 1 . - on/off - curson on
#define OPTREX_ONOFF_OFF		0x08	// . . . . . 0 . . - on/off - on
#define OPTREX_ONOFF_ON			0x0C	// . . . . . 1 . . - on/off - off
#define OPTREX_SHIFT			0x10	// 0 0 0 1 . . ? ? - shift display
#define OPTREX_SHIFT_SHIFT		0x10	// . . . . 0 . . . - shift display - shift(scroll) display
#define OPTREX_SHIFT_MOVECUR	0x18	// . . . . 1 . . . - shift display - move cursor
#define OPTREX_SHIFT_LEFT		0x10	// . . . . . 0 . . - shift display - left
#define OPTREX_SHIFT_RIGHT		0x14	// . . . . . 1 . . - shift display - right
#define OPTREX_CFG				0x20	// 0 0 1 . . . ? ? - configure interface/display
#define OPTREX_CFG_4BIT			0x20	// . . . 0 . . . . - configure interface/display - 4 bit interface
#define OPTREX_CFG_8BIT			0x30	// . . . 1 . . . . - configure interface/display - 8 bit interface
#define OPTREX_CFG_DUTY8		0x20	// . . . . 0 . . . - configure interface/display - 1/8,1/11 duty
#define OPTREX_CFG_DUTY16		0x28	// . . . . 1 . . . - configure interface/display - 1/16 duty
#define OPTREX_CFG_SMALLFONT	0x20	// . . . . . 0 . . - configure interface/display - 5x7 dot font
#define OPTREX_CFG_BIGFONT		0x24	// . . . . . 1 . . - configure interface/display - 5x10 dot font
#define OPTREX_CGEN				0x40	// 0 1 c c c c c c - set character gen ram address
#define OPTREX_DISPADDR			0x80	// 1 p p p p p p p - set display ram address

#define OPTREX_WAIT			40
#define OPTREX_CLEAR_WAIT	(1640-OPTREX_WAIT)

void optrexhw_write( optrex_cookie *cookie, uint8 value, bool command, bigtime_t delay )
{
	cpu_status cpustatus = disable_interrupts();
	outp( cookie->par_port, value );
	outp( cookie->par_port+2, MAKECTRL(command?0:1,0,1) );
	outp( cookie->par_port+2, MAKECTRL(command?0:1,0,0) );
	restore_interrupts( cpustatus );

	spin( delay );
}
void optrexhw_writecommand( optrex_cookie *cookie, uint8 command )
{
	optrexhw_write( cookie, command, true, OPTREX_WAIT );
}

void optrexhw_writechar( optrex_cookie *cookie, uint8 value )
{
	optrexhw_write( cookie, value, false, OPTREX_WAIT );
}

void oprtexhw_clear( optrex_cookie *cookie )
{
	optrexhw_writecommand( cookie, OPTREX_CLEAR );
	snooze( OPTREX_CLEAR_WAIT );

	memset( cookie->vram, ' ', sizeof(cookie->vram) );
	memset( cookie->oldvram, ' ', sizeof(cookie->oldvram) );
	cookie->cursor_x = 0;
	cookie->cursor_y = 0;
}

void optrexhw_uploadglyp( optrex_cookie *cookie, int index, uint8 data[8] )
{
	int i;
	if( index>=0 || index<8 )
	{
		optrexhw_writecommand( cookie, OPTREX_CGEN+index*8 );
		for( i=0; i<8; i++ )
			optrexhw_writechar( cookie, data[i] );

		// The OPTREX_CGEN command moves the cursor, so we invalidate the cashed position
#if 1
		cookie->cursor_x = -1;
		cookie->cursor_y = -1;
#else
		cookie->cursor_x += 8;
		if( cookie->cursor_x >= 20 )
		{
			cookie->cursor_x = (cookie->cursor_x)%20;
			cookie->cursor_y = (cookie->cursor_y+1)%OPTREX_HEIGHT;
		}
#endif
	}
}

void optrexhw_init( optrex_cookie *cookie )
{
	// init 8bit mode:
	optrexhw_write( cookie, OPTREX_CFG|OPTREX_CFG_8BIT, true, 4100 );
	optrexhw_write( cookie, OPTREX_CFG|OPTREX_CFG_8BIT, true, 100 );
	optrexhw_write( cookie, OPTREX_CFG|OPTREX_CFG_8BIT, true, 4100 );
	optrexhw_write( cookie, OPTREX_CFG|OPTREX_CFG_8BIT|OPTREX_CFG_DUTY16, true, 40 );

	optrexhw_writecommand( cookie, OPTREX_CFG|OPTREX_CFG_8BIT|OPTREX_CFG_DUTY16|OPTREX_CFG_SMALLFONT );
	optrexhw_writecommand( cookie, OPTREX_ONOFF|OPTREX_ONOFF_OFF );
	oprtexhw_clear( cookie );
	optrexhw_writecommand( cookie, OPTREX_EM|OPTREX_EM_INC|OPTREX_EM_NOSHIFT );
	optrexhw_writecommand( cookie, OPTREX_ONOFF|OPTREX_ONOFF_ON|OPTREX_ONOFF_CUROFF|OPTREX_ONOFF_NOFLASH );
}

void optrexhw_update( optrex_cookie *cookie )
{
	int ix, iy, ixstart;

	for( iy=0; iy<OPTREX_HEIGHT; iy++ )
	{
		ix = 0;
		while(true)
		{
			while( ix<OPTREX_WIDTH && cookie->vram[iy][ix]==cookie->oldvram[iy][ix] )
				ix++;
			if( ix==OPTREX_WIDTH )
				break;
			ixstart = ix;
			while( ix<OPTREX_WIDTH && cookie->vram[iy][ix]!=cookie->oldvram[iy][ix] )
				ix++;

			if( ixstart!=cookie->cursor_x || iy!=cookie->cursor_y )
			{
				if( iy==0 ) optrexhw_writecommand( cookie, OPTREX_DISPADDR + ixstart );
				else		optrexhw_writecommand( cookie, OPTREX_DISPADDR+0x40  + ixstart );
			}
				
			for( ; ixstart<ix; ixstart++ )
			{
				optrexhw_writechar( cookie, cookie->oldvram[iy][ixstart]=cookie->vram[iy][ixstart] );
			}

			cookie->cursor_x = ix;
			cookie->cursor_y = iy;
		}
	}
}

//-----------------------------------------------------------------------------

status_t init_hardware()
{
	return B_OK;
}


status_t init_driver()
{
	status_t status;

	kprintf( "optrex>init_driver()\n" );

	in_use = 0;

	status = get_module( B_ISA_MODULE_NAME, (module_info **)&isa );
	if( status < B_NO_ERROR )
	{
		kprintf( "optrex>init_driver(): could not get isa module\n" );
		return status;
	}
	
	return B_OK;
}

void uninit_driver()
{
	if( isa )
		put_module( B_ISA_MODULE_NAME );
}

//-----------------------------------------------------------------------------
	
const char **publish_devices()
{
	return optrex_name;
}

device_hooks *find_device( const char* name )
{
	return &optrex_hooks;
}

//-----------------------------------------------------------------------------

static status_t optrex_open( const char *name, uint32 flags, void **cookie )
{
	optrex_cookie *ocookie;
	int devid;
	
	devid = 1;

	if( atomic_or(&in_use,devid) & devid )
		return B_BUSY;

	ocookie = (optrex_cookie*)malloc( sizeof(optrex_cookie) );
	if( ocookie == NULL )
	{
		atomic_and( &in_use, ~(1<<devid) );
		return B_ERROR;
	}

	ocookie->device_id = devid;
//	ocookie->par_port = 0x378;	// FIXME: read from cfg
	ocookie->par_port = 0x278;	// FIXME: read from cfg
	*cookie = ocookie;
	
	optrexhw_init( ocookie );

	return B_NO_ERROR;
}

static status_t optrex_close( void *cookie )
{
	optrex_cookie *ocookie = (optrex_cookie*)cookie;

	atomic_and( &in_use, ~(1<<ocookie->device_id) );

	return B_OK;
}

static status_t optrex_free( void *cookie )
{
	if( cookie )
		free( cookie );

	return B_NO_ERROR;
}

static status_t optrex_control( void *cookie, uint32 op, void *arg, size_t len )
{
	optrex_cookie *ocookie = (optrex_cookie*)cookie;

	switch( op )
	{
		case OPTREX_UPLOADLINE0:
		case OPTREX_UPLOADLINE1:
//			if( lock_memory(arg, OPTREX_WIDTH, B_READ_DEVICE) == B_NO_ERROR )
			{
				memcpy( &ocookie->vram[op-OPTREX_UPLOADLINE0][0], arg, OPTREX_WIDTH );
				optrexhw_update( ocookie );
//				unlock_memory( arg, OPTREX_WIDTH, B_READ_DEVICE );
				return B_NO_ERROR;
			}
			break;

		case OPTREX_UPLOADALLLINES:
//			if( lock_memory(arg, OPTREX_WIDTH*OPTREX_HEIGHT, B_READ_DEVICE) == B_NO_ERROR )
			{
				memcpy( &ocookie->vram[0][0], arg, OPTREX_WIDTH*OPTREX_HEIGHT );
				optrexhw_update( ocookie );
//				unlock_memory( arg, OPTREX_WIDTH*OPTREX_HEIGHT, B_READ_DEVICE );
				return B_NO_ERROR;
			}
			break;
		
		case OPTREX_UPLOADGLYP0:
		case OPTREX_UPLOADGLYP1:
		case OPTREX_UPLOADGLYP2:
		case OPTREX_UPLOADGLYP3:
		case OPTREX_UPLOADGLYP4:
		case OPTREX_UPLOADGLYP5:
		case OPTREX_UPLOADGLYP6:
		case OPTREX_UPLOADGLYP7:
//			if( lock_memory(arg, 8, B_READ_DEVICE) == B_NO_ERROR )
			{
				optrexhw_uploadglyp( ocookie, op-OPTREX_UPLOADGLYP0, (uint8*)arg );
//				unlock_memory( arg, 8, B_READ_DEVICE );
				return B_NO_ERROR;
			}
			break;
	}

	return B_BAD_VALUE;
}

static status_t optrex_read( void *cookie, off_t position, void *buf, size_t *num_bytes )
{
	*num_bytes = 0;
	return B_IO_ERROR;
}

static status_t optrex_write( void *cookie, off_t position, const void *buffer, size_t *num_bytes )
{
	optrex_cookie *ocookie = (optrex_cookie*)cookie;

	if( *num_bytes > OPTREX_WIDTH*OPTREX_HEIGHT )
		*num_bytes = OPTREX_WIDTH*OPTREX_HEIGHT;

	memcpy( ocookie->vram, buffer, *num_bytes );
	optrexhw_update( ocookie );

	return B_NO_ERROR;
}

//-----------------------------------------------------------------------------

