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
#include <stdlib.h>
//-------------------------------------
#include <drivers/ISA.h>
#include <drivers/Drivers.h>
#include <drivers/KernelExport.h>
#include <drivers/driver_settings.h>
#include <kernel/OS.h>
//-------------------------------------
#include "ssc_driver.h"
#include "ssc_private.h"
#include "ssc_boot.h"
#include "utils.h"
//-----------------------------------------------------------------------------

#define range(v,l,h) (max(min((v),(h)),(l)))
static bool validateheader( const void *header );

static const char *ssc_name[] =
{
	"misc/ssc/flash/all",	// 512K
	"misc/ssc/flash/boot",	// 64K
	"misc/ssc/flash/0",		// 64K
	"misc/ssc/flash/1",		// 128K
	"misc/ssc/flash/2",		// 128K
	"misc/ssc/flash/3",		// 128K
	"misc/ssc/flash/23",	// 256K
	"misc/ssc/ram/0",
	"misc/ssc/ram/1",
	"misc/ssc/ram/2",
	"misc/ssc/ram/3",
	NULL
};

ssc_info ssc_dev_info[] =
{
	{true,	SSC_CMD_READ_BANK_0,		512*1024,	0x1f,	SSC_CMD_WRITE_BANK_0,		512*1024},	// flash/all
	{true,	SSC_CMD_READ_BANK_0,		64*1024,	0x01,	SSC_CMD_WRITE_BANK_0,		64*1024},	// flash/boot
	{true,	-1,							64*1024,	0x02,	SSC_CMD_WRITE_BANK_1,		64*1024},	// flash/0
	{true,	-1,							128*1024,	0x04,	SSC_CMD_WRITE_BANK_2,		128*1024},	// flash/1
	{true,	-1,							128*1024,	0x08,	SSC_CMD_WRITE_BANK_3,		128*1024},	// flash/2
	{true,	-1,							128*1024,	0x10,	SSC_CMD_WRITE_BANK_4,		128*1024},	// flash/3
	{true,	-1,							256*1024,	0x18,	SSC_CMD_WRITE_BANK_3,		256*1024},	// flash/23
	
	{false,	SSC_CMD_READ_RAM_BANK_0,	8*1024,		0,	SSC_CMD_WRITE_RAM_BANK_0,	8*1024},	// ram/0
	{false,	SSC_CMD_READ_RAM_BANK_1,	8*1024,		0,	SSC_CMD_WRITE_RAM_BANK_1,	8*1024},	// ram/1
	{false,	SSC_CMD_READ_RAM_BANK_2,	8*1024,		0,	SSC_CMD_WRITE_RAM_BANK_2,	8*1024},	// ram/2
	{false,	SSC_CMD_READ_RAM_BANK_3,	8*1024,		0,	SSC_CMD_WRITE_RAM_BANK_3,	8*1024},	// ram/3
};

static status_t sscdev_open( const char *name, uint32 flags, void **cookie );
static status_t sscdev_close( void *cookie );
static status_t sscdev_free( void *cookie );
static status_t sscdev_control( void *cookie, uint32 op, void *arg, size_t len );
static status_t sscdev_read( void *cookie, off_t position, void *buf, size_t *num_bytes );
static status_t sscdev_write( void *cookie, off_t position, const void *buffer, size_t *num_bytes );

device_hooks ssc_hooks = 
{
	sscdev_open, 			/* -> open entry point */
	sscdev_close, 			/* -> close entry point */
	sscdev_free,			/* -> free cookie */
	sscdev_control, 		/* -> control entry point */
	sscdev_read,			/* -> read entry point */
	sscdev_write			/* -> write entry point */
};

struct isa_module_info *isa = NULL;

static uint16 ioport = 0x378; // read from config file

uint32 in_use;

//-----------------------------------------------------------------------------

status_t init_hardware()
{
	return B_OK;
}


status_t init_driver()
{
	status_t status;
	int card;
	ssc_cookie ssc;
	void *settingsfile;

	dprintf( "ssc>init_driver()\n" );

	settingsfile = load_driver_settings( "ssc" );
	if( settingsfile )
	{
		const char *setting = get_driver_parameter( settingsfile, "ioport", NULL, NULL );
		uint16 settings_ioport = setting ? strtol(setting,NULL,0) : 0;
		
		if( settings_ioport )
			ioport = settings_ioport;
		unload_driver_settings( settingsfile );
	}

//	dprintf( "ssc>ioport:0x%04x\n", ioport );

	in_use = 0;

	status = get_module( B_ISA_MODULE_NAME, (module_info **)&isa );
	if( status < B_OK )
	{
		dprintf( "ssc>could not find iso module()\n" );
		return B_ERROR;
	}

	ssc.par_port = ioport;
	ssc.par_port_data = ssc.par_port + 0;
	ssc.par_port_status = ssc.par_port + 1;
	ssc.par_port_ctrl = ssc.par_port + 2;
	
	ssc_reset( &ssc );
	card = ssc_getcardtype( &ssc );
	if( card < B_OK )
	{
		put_module( B_ISA_MODULE_NAME );
		return B_ERROR;
	}
	
	return B_OK;
}

void uninit_driver()
{
	dprintf( "ssc>uninit_driver()\n" );

	put_module( B_ISA_MODULE_NAME );
}

//-----------------------------------------------------------------------------
	
const char **publish_devices()
{
	return ssc_name;
}

device_hooks *find_device( const char* name )
{
	return &ssc_hooks;
}

//-----------------------------------------------------------------------------

const ssc_info *find_device_info( const char* name )
{
	int i;
	for( i=0; ssc_name[i]; i++ )
	{
		if( strcmp(name,ssc_name[i]) == 0 )
			return &ssc_dev_info[i];
	}
	return NULL;
}

static status_t sscdev_open( const char *name, uint32 flags, void **cookie )
{
	ssc_cookie *ssc;
	int card;

	if( atomic_or(&in_use,1) )
	{
		dprintf( "ssc>driver is already in use\n" );
		return B_BUSY;
	}

	ssc = (ssc_cookie*)malloc( sizeof(ssc_cookie) );
	if( ssc == NULL )
	{
		atomic_and( &in_use, 0 );
		return B_ERROR;
	}
	
	ssc->par_port = ioport;
	ssc->par_port_data = ssc->par_port + 0;
	ssc->par_port_status = ssc->par_port + 1;
	ssc->par_port_ctrl = ssc->par_port + 2;

	ssc_reset( ssc );
	card = ssc_getcardtype( ssc );
	if( card < B_OK )
	{
		free( ssc );
		atomic_and( &in_use, 0 );
		return B_ERROR;
	}

	ssc->failed = false;
	ssc->cmd = find_device_info( name );
	ssc->rw_mode = 0;
	ssc->pos = 0;

	*cookie = ssc;
	
	return B_OK;
}

static status_t sscdev_close( void *cookie )
{
	ssc_cookie *ssc = (ssc_cookie*)cookie;
	
	if( ssc->cmd && ssc->rw_mode==2 && ssc->cmd->clearmask && !ssc->failed )
	{
		int i;
		for( i=0; i<2; i++ )
		{
			if( validateheader(ssc->card_header[i]) )
			{
				ssc_boot_addentry( ssc, ssc->cmd->clearmask, ssc->card_header[i] );
				break;
			}
		}
	}

//	dumphex( ssc->card_header[0], sizeof(ssc->card_header[0]) );
//	dumphex( ssc->card_header[1], sizeof(ssc->card_header[1]) );

	atomic_and( &in_use, 0 );

	return B_OK;
}

static status_t sscdev_free( void *cookie )
{
	if( cookie )
		free( cookie );

	return B_OK;
}

static status_t sscdev_control( void *cookie, uint32 op, void *arg, size_t len )
{
	switch( op )
	{
	}

	return B_BAD_VALUE;
}

static status_t sscdev_read( void *cookie, off_t position, void *buffer, size_t *num_bytes )
{
	status_t status;
	ssc_cookie *ssc = (ssc_cookie*)cookie;
	int readreqbytes = *num_bytes;
	int readbytes;
	int i;
	int data;

	*num_bytes = 0;

	if( ssc->cmd )
	{
		if( ssc->rw_mode == 0 )
		{
			if( ssc->cmd->readcmd != -1 )
			{
				dprintf( "SETMODE: %02X\n", ssc->cmd->readcmd );
				status = ssc_setmode( ssc, ssc->cmd->readcmd );
				if( status < B_OK )
				{
					dprintf( "ssc>read: could not send read command 0x%02X\n", ssc->cmd->readcmd );
					ssc->failed = true;
					return B_IO_ERROR;
				}
				ssc->rw_mode = 1;
			}
			else
			{
				dprintf( "ssc>read: this device does not support reading\n" );
				ssc->failed = true;
				return B_IO_ERROR;
			}
		}
		else if( ssc->rw_mode != 1 )
		{
			dprintf( "ssc>read: the device is not configured for reading...\n" );
			ssc->failed = true;
			return B_IO_ERROR;
		}
		
		if( position != ssc->pos )
		{
			dprintf( "ssc>read: seeking is not supported...\n" );
			ssc->failed = true;
			return B_IO_ERROR;
		}
		
		readbytes = min( readreqbytes, ssc->cmd->readlen-position );
		if( readbytes < 0 )
		{
			dprintf( "ssc>read: reading beyond device devsize:%d readpos:%Ld readsize:%d...\n", ssc->cmd->readlen, position, readreqbytes );
			ssc->failed = true;
			return B_IO_ERROR;
		}
		
		for( i=0; i<readbytes && (data=ssc_readbyte(ssc))>=B_OK; i++ )
			((uint8*)buffer)[i] = data;
			
		ssc->pos += i;

		if( i != readbytes )
		{
			dprintf("ssc>read: only read %d of %d bytes\n", i, readbytes );
			ssc->failed = true;
		}
		
		*num_bytes = i;

		return B_OK;
	}
	else
	{
		return B_IO_ERROR;
	}
}

#define PTRADD(p,a) (((uint8*)p)+(a))

int memcpywindow(
	void *dst, int dst_winstart, int dst_winsize,
	const void *src, int src_start, int src_size )
{
	if( src_start >= dst_winstart+dst_winsize ) return 0;
	if( src_start+src_size <= dst_winstart ) return 0;
	
	if( src_start < dst_winstart )
	{
		int prelen = dst_winstart-src_start;
		src = PTRADD( src, prelen );
		src_start += prelen;
		src_size -= prelen;
	}
	
	if( src_start+src_size > dst_winstart+dst_winsize )
	{
		int postlen = (src_start+src_size)-(dst_winstart+dst_winsize);
		src_size -= postlen;
	}
	
	if( src_size<=0 || src_size>dst_winsize )
	{
		dprintf( "**********memcpywindow failed: %d\n", src_size );
		return 0;
	}
	
	memcpy( PTRADD(dst,src_start-dst_winstart), src, src_size );
	return src_size;
}

static status_t sscdev_write( void *cookie, off_t position, const void *buffer, size_t *num_bytes )
{
	status_t status;
	ssc_cookie *ssc = (ssc_cookie*)cookie;
	int writereqbytes = *num_bytes;
	int writebytes;
	int i;

	*num_bytes = 0;

	if( ssc->cmd )
	{
		if( ssc->rw_mode == 0 )
		{
			if( ssc->cmd->writecmd != -1 )
			{
				if( ssc->cmd->flash )
				{
					int cardtype;
					ssc_reset( ssc );
					cardtype = ssc_getcardtype( ssc );
					if( cardtype < B_OK )
					{
						dprintf( "ssc>write: could not determine card type\n" );
						ssc->failed = true;
						return B_IO_ERROR;
					}
					if( cardtype!=CARD_SSTDFLASH && cardtype!=CARD_AMDFLASH ) // NOTE: there might exist other types of cards...
					{
						dprintf( "ssc>write: unknown card type: %02X\n", cardtype );
						ssc->failed = true;
						return B_ERROR;
					}

					if( ssc->cmd->clearmask  )
					{
						status = ssc_clearflash( ssc, ssc->cmd->clearmask );
						if( status < B_OK )
						{
							dprintf( "ssc>write: could not clear flash 0x%02X\n", ssc->cmd->clearmask );
							ssc->failed = true;
							return B_IO_ERROR;
						}
					}
				}
				status = ssc_setmode( ssc, ssc->cmd->writecmd );
				if( status < B_OK )
				{
					dprintf( "ssc>write: could not send write command 0x%02X\n", ssc->cmd->writecmd );
					ssc->failed = true;
					return B_IO_ERROR;
				}
				ssc->rw_mode = 2;
			}
			else
			{
				dprintf( "ssc>write: this device does not support writing\n" );
				ssc->failed = true;
				return B_IO_ERROR;
			}
			
			memset( ssc->card_header, 0, sizeof(ssc->card_header) );

		}
		else if( ssc->rw_mode != 2 )
		{
			dprintf( "ssc>write: the device is not configured for writing...\n" );
			ssc->failed = true;
			return B_IO_ERROR;
		}
		
		if( position != ssc->pos )
		{
			dprintf( "ssc>write: seeking is not supported...\n" );
			ssc->failed = true;
			return B_IO_ERROR;
		}
		
		writebytes = min( writereqbytes, ssc->cmd->writelen-position );
		if( writebytes <= 0 )
		{
			dprintf( "ssc>write: writing beyond device devsize:%d writepos:%Ld writesize:%d...\n", ssc->cmd->writelen, position, writereqbytes );
			ssc->failed = true;
			return B_IO_ERROR;
		}
		
		for( i=0; i<writebytes && (ssc_writebyte(ssc,((uint8*)buffer)[i]))>=B_OK; i++ )
			;

		if( i==0 && writebytes>0 )
		{
			dprintf("ssc>write: write failed\n" );
			ssc->failed = true;
			return B_IO_ERROR;
		}
			
		// update headers:
		memcpywindow( ssc->card_header[0], 0, sizeof(ssc->card_header[0]), buffer, position, writebytes );
		memcpywindow( ssc->card_header[1], 0x100, sizeof(ssc->card_header[0]), buffer, position, writebytes );

		ssc->pos += i;

		if( i != writebytes )
		{
			dprintf("ssc>write: only wrote %d of %d bytes\n", i, writebytes );
			ssc->failed = true;
		}
		
		*num_bytes = i;
		
		return B_OK;
	}
	else
	{
		return B_IO_ERROR;
	}
}

//-----------------------------------------------------------------------------

static const uint8 header_magic[16*3] =
{
	0xCE,0xED,0x66,0x66,0xCC,0x0D,0x00,0x0B,0x03,0x73,0x00,0x83,0x00,0x0C,0x00,0x0D,
	0x00,0x08,0x11,0x1F,0x88,0x89,0x00,0x0E,0xDC,0xCC,0x6E,0xE6,0xDD,0xDD,0xD9,0x99,
	0xBB,0xBB,0x67,0x63,0x6E,0x0E,0xEC,0xCC,0xDD,0xDC,0x99,0x9F,0xBB,0xB9,0x33,0x3E
};

static bool validateheader( const void *header )
{
	uint8 headerchecksum;
	int i;

	if( memcmp((uint8*)(header)+0x04,header_magic,sizeof(header_magic)) != 0 )
		return false;
	
	headerchecksum = 0;
	for( i=0x34; i<0x4d; i++ )
		headerchecksum += ((uint8*)(header))[i];
	headerchecksum = 0xe7-headerchecksum;
	
	if( headerchecksum != ((uint8*)(header))[0x4d] )
		return false;
	
	return true;
}


//-----------------------------------------------------------------------------
