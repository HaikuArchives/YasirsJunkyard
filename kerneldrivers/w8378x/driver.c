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
#include "w8378x_driver.h"
//-----------------------------------------------------------------------------

static status_t w8378x_open( const char *name, uint32 flags, void **cookie );
static status_t w8378x_close( void *cookie );
static status_t w8378x_free( void *cookie );
static status_t w8378x_control( void *cookie, uint32 op, void *arg, size_t len );
static status_t w8378x_read( void *cookie, off_t position, void *buf, size_t *num_bytes );
static status_t w8378x_write( void *cookie, off_t position, const void *buffer, size_t *num_bytes );

static const char *w8378x_name[] = {
	"misc/w8378x",
	NULL
};

device_hooks w8378x_hooks = 
{
	w8378x_open, 			/* -> open entry point */
	w8378x_close, 			/* -> close entry point */
	w8378x_free,			/* -> free cookie */
	w8378x_control, 		/* -> control entry point */
	w8378x_read,			/* -> read entry point */
	w8378x_write			/* -> write entry point */
};

typedef struct
{
	int dummy;
} wb_cookie;

struct isa_module_info *isa = NULL;
int w8378x_type;

//static long in_use;
spinlock port_lock;

#define inp(_port) ((uint8)(isa->read_io_8(_port)))
#define outp(_port,_data) (isa->write_io_8(_port, _data))

//-----------------------------------------------------------------------------

int w8378x_port = 0x290;
#define W8378x_ADDR_REG_OFFSET 5
#define W8378x_DATA_REG_OFFSET 6

#define W8378x_REG_TEMP				0x27
#define W8378x_REG_TEMP_OVER		0x39
#define W8378x_REG_TEMP_HYST		0x3A
#define W8378x_REG_TEMP_CONFIG		0x52
#define W8378x_REG_SCFG2			0x59

#define W8378x_REG_TEMP2_MSB		0x0150
#define W8378x_REG_TEMP2_LSB		0x0151
#define W8378x_REG_TEMP2_CONFIG		0x0152
#define W8378x_REG_TEMP2_HYST		0x0153
#define W8378x_REG_TEMP2_OVER		0x0155

#define W8378x_REG_TEMP3_MSB		0x0250
#define W8378x_REG_TEMP3_LSB		0x0251
#define W8378x_REG_TEMP3_CONFIG		0x0252
#define W8378x_REG_TEMP3_HYST		0x0253
#define W8378x_REG_TEMP3_OVER		0x0255

#define W8378x_REG_FAN1				0x0028
#define W8378x_REG_FAN2				0x0029
#define W8378x_REG_FAN3				0x002a

#define W8378x_REG_VID_FANDIV		0x47
#define W8378x_REG_PIN				0x4B
#define W8378x_REG_VBAT				0x5d


#define W8378x_REG_CONFIG		0x40
#define W8378x_REG_BANK			0x4E
#define W8378x_REG_CHIPMAN		0x4F
#define W8378x_REG_WCHIPID		0x58

#define W8378x_TYPEID_W83781D	0x10
#define W8378x_TYPEID_W83782D	0x30

uint16 w8378x_readvalue( uint16 reg )
{
	bool readword;
	uint16 retval;
	
	cpu_status cpustatus = disable_interrupts();
	acquire_spinlock( &port_lock );
	
	// register 50,53 and 55 in bank 1 and 2 should be read as 16 bit:
	readword = (((reg&0xff00)==0x0100)||((reg&0xff00)==0x0200)) &&
		(((reg&0x00ff)==0x0050)||((reg&0x00ff)==0x0053)||((reg&0x00ff)==0x0055));

	// set bank
	if( reg&0xff00 )
	{
		outp( w8378x_port+W8378x_ADDR_REG_OFFSET, W8378x_REG_BANK );
		outp( w8378x_port+W8378x_DATA_REG_OFFSET, reg >> 8);
	}

	// read msb
	outp( w8378x_port+W8378x_ADDR_REG_OFFSET, reg&0x00ff );
	retval = inp( w8378x_port+W8378x_DATA_REG_OFFSET );

	if( readword )
	{
		// read lsb
		outp( w8378x_port+W8378x_ADDR_REG_OFFSET, (reg&0x00ff)+1 );
		retval <<= 8;
		retval |= inp( w8378x_port+W8378x_DATA_REG_OFFSET );
	}

	// reset bank
	if( reg&0xff00 )
	{
		outp( w8378x_port+W8378x_ADDR_REG_OFFSET, W8378x_REG_BANK );
		outp( w8378x_port+W8378x_DATA_REG_OFFSET, 0);
	}

	release_spinlock( &port_lock );
	restore_interrupts( cpustatus );

	return retval;
}

void w8378x_writevalue( uint16 reg, uint16 value )
{
	bool writeword;

	cpu_status cpustatus = disable_interrupts();
	acquire_spinlock( &port_lock );

	// register 50,53 and 55 in bank 1 and 2 should be written as 16 bit:
	writeword = (((reg&0xff00)==0x0100)||((reg&0xff00)==0x0200)) &&
		(((reg&0x00ff)==0x0050)||((reg&0x00ff)==0x0053)||((reg&0x00ff)==0x0055));

	// set bank
	if( reg&0xff00 )
	{
		outp( w8378x_port+W8378x_ADDR_REG_OFFSET, W8378x_REG_BANK );
		outp( w8378x_port+W8378x_DATA_REG_OFFSET, reg >> 8);
	}

	outp( w8378x_port+W8378x_ADDR_REG_OFFSET, reg&0x00ff );
	if( writeword )
	{
		// write lsb
		outp( w8378x_port+W8378x_DATA_REG_OFFSET, value>>8 );
		outp( w8378x_port+W8378x_ADDR_REG_OFFSET, (reg&0x00ff)+1 );
	}

	// write msb
	outp( w8378x_port+W8378x_DATA_REG_OFFSET, value&0x00ff );

	// reset bank
	if( reg&0xff00 )
	{
		outp( w8378x_port+W8378x_ADDR_REG_OFFSET, W8378x_REG_BANK );
		outp( w8378x_port+W8378x_DATA_REG_OFFSET, 0);
	}

	release_spinlock( &port_lock );
	restore_interrupts( cpustatus );
}

//-----------------------------------------------------------------------------

status_t init_hardware()
{
	return B_NO_ERROR;
}


status_t init_driver()
{
	status_t status;
	int i, j;

	kprintf( "w8378x>init_driver()\n" );

//	in_use = 0;
	port_lock = 0;

	status = get_module( B_ISA_MODULE_NAME, (module_info **)&isa );
	if( status < B_NO_ERROR )
		goto error0;
	
	// hardware detection (not found in the manual, but the Linux driver):
	i = inp( w8378x_port+1 );
	if( inp(w8378x_port+2)!=i || inp(w8378x_port+3)!=i || inp(w8378x_port+7)!=i )
		goto error1;

	i = inp( w8378x_port+5) & 0x7f;
	outp( w8378x_port+5, (~i)&0x7f );
	if( (inp(w8378x_port+5)&0x7f) != ((~i)&0x7f) )
	{
		outp( w8378x_port+5, i );
		goto error1;
	}

	if( w8378x_readvalue(W8378x_REG_CONFIG) & 0x80 )
		goto error1;
	i = w8378x_readvalue( W8378x_REG_BANK );
	j = w8378x_readvalue( W8378x_REG_CHIPMAN );
	if( !(i&0x07) && ((!(i&0x80) && (j!=0xa3)) || ((i&0x80) && (j!=0x5c))) )
		goto error1;

	// reset it to bank 0
	w8378x_writevalue( W8378x_REG_BANK,w8378x_readvalue(W8378x_REG_BANK)&0xf8 );

	// detecting winbond version
	i = w8378x_readvalue( W8378x_REG_WCHIPID);
	if( i==0x10 || i==0x11 )
	{
		// my W83781D manual says that the chipid is 0x10, but the
		// W83781D on my P2B-DS returns 0x11??!?
		w8378x_type = W8378x_TYPEID_W83781D;
		kprintf( "w8378x>init_driver(): found W83781D\n" );
	}
	else if( i==0x30 )
	{
		w8378x_type = W8378x_TYPEID_W83782D;
		kprintf( "w8378x>init_driver(): found W83782D\n" );
	}
	else
	{
		kprintf( "w8378x>init_driver(): found unknown winbond chip, id:%02X\n", i );
		goto error1;
	}
	return B_NO_ERROR;

error1:
	put_module( B_ISA_MODULE_NAME );
error0:
	kprintf( "w8378x>init_driver(): could not find hardware\n" );
	return B_ERROR;
}

void uninit_driver()
{
	kprintf( "w83782d>uninit_driver()\n" );

	put_module( B_ISA_MODULE_NAME );
}

//-----------------------------------------------------------------------------
	
const char **publish_devices()
{
	return w8378x_name;
}

device_hooks *find_device( const char* name )
{
	return &w8378x_hooks;
}

//-----------------------------------------------------------------------------

static status_t w8378x_open( const char *name, uint32 flags, void **cookie )
{
	wb_cookie *wbcookie;

//	if( atomic_or(&in_use,1) )
//		return B_BUSY;

	wbcookie = (wb_cookie*)malloc( sizeof(wb_cookie) );
	if( wbcookie == NULL )
	{
//		atomic_and( &in_use, 0 );
		return B_ERROR;
	}

	*cookie = wbcookie;
	
	return B_NO_ERROR;
}

static status_t w8378x_close( void *cookie )
{
//	atomic_and( &in_use, 0 );

	return B_NO_ERROR;
}

static status_t w8378x_free( void *cookie )
{
	if( cookie )
		free( cookie );

	return B_NO_ERROR;
}

static status_t w8378x_control( void *cookie, uint32 op, void *arg, size_t len )
{
	switch( op )
	{
		case W8378x_READ_TEMP1:
			*((int*)arg) = ((int)(int8)w8378x_readvalue(W8378x_REG_TEMP))*256;
			return B_NO_ERROR;

		case W8378x_READ_TEMP2:
			*((int*)arg) = ((int)(int16)w8378x_readvalue(W8378x_REG_TEMP2_MSB));
			return B_NO_ERROR;

		case W8378x_READ_TEMP3:
			*((int*)arg) = ((int)(int16)w8378x_readvalue(W8378x_REG_TEMP3_MSB));
				return B_NO_ERROR;
			return B_NO_ERROR;

		case W8378x_READ_FAN1:
		{
			int fandiv, rpm;
			fandiv = (w8378x_readvalue(W8378x_REG_VID_FANDIV)>>4)&0x03;
			if( w8378x_type==W8378x_TYPEID_W83782D )
				fandiv |= ((w8378x_readvalue(W8378x_REG_VBAT)>>5)&1)<<2;
			fandiv = 1<<fandiv;
			rpm = w8378x_readvalue( W8378x_REG_FAN1 );
			if( rpm==0 || rpm==255 )
				return B_BAD_VALUE;
			rpm = 1350000/(rpm*fandiv);
			*((int*)arg) = rpm;
			return B_NO_ERROR;
		}

		case W8378x_READ_FAN2:
		{
			int fandiv, rpm;
			fandiv = (w8378x_readvalue(W8378x_REG_VID_FANDIV)>>6)&0x03;
			if( w8378x_type==W8378x_TYPEID_W83782D )
				fandiv |= ((w8378x_readvalue(W8378x_REG_VBAT)>>6)&1)<<2;
			fandiv = 1<<fandiv;
			rpm = w8378x_readvalue( W8378x_REG_FAN2 );
			if( rpm==0 || rpm==255 )
				return B_BAD_VALUE;
			rpm = 1350000/(rpm*fandiv);
			*((int*)arg) = rpm;
			return B_NO_ERROR;
		}

		case W8378x_READ_FAN3:
		{
			int fandiv, rpm;
			fandiv = (w8378x_readvalue(W8378x_REG_PIN)>>6)&0x03;
			if( w8378x_type==W8378x_TYPEID_W83782D )
				fandiv |= ((w8378x_readvalue(W8378x_REG_VBAT)>>7)&1)<<2;
			fandiv = 1<<fandiv;
			rpm = w8378x_readvalue( W8378x_REG_FAN3 );
			if( rpm==0 || rpm==255 )
				return B_BAD_VALUE;
			rpm = 1350000/(rpm*fandiv);
			*((int*)arg) = rpm;
			return B_NO_ERROR;
		}
	}

	return B_BAD_VALUE;
}

static status_t w8378x_read( void *cookie, off_t position, void *buf, size_t *num_bytes )
{
	*num_bytes = 0;
	return B_IO_ERROR;
}

static status_t w8378x_write( void *cookie, off_t position, const void *buffer, size_t *num_bytes )
{
	*num_bytes = 0;
	return B_IO_ERROR;
}

//-----------------------------------------------------------------------------

