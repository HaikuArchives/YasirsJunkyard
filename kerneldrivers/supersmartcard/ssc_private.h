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
#ifndef DAMN_SSCDRIVERPRIVATE_H
#define DAMN_SSCDRIVERPRIVATE_H
//-----------------------------------------------------------------------------
//-------------------------------------
#include <drivers/Drivers.h>
#include <drivers/ISA.h>
//-------------------------------------
//-----------------------------------------------------------------------------

typedef struct
{
	bool	flash;
	int		readcmd;
	int		readlen;
	int		clearmask;
	int		writecmd;
	int		writelen;
} ssc_info;

typedef struct
{
	int	par_port;
	int par_port_data;
	int par_port_status;
	int	par_port_ctrl;
	
	const ssc_info	*cmd;
	int 			rw_mode;
	int				pos;
	
	bool			failed;
	
	uint8			card_header[2][0x50];
} ssc_cookie;


extern struct isa_module_info *isa;

#define inp(_port) ((uint8)(isa->read_io_8(_port)))
#define outp(_port,_data) (isa->write_io_8(_port, _data))

status_t ssc_setmode( ssc_cookie *ssc, int mode );
int ssc_readbyte( ssc_cookie *ssc );
status_t ssc_writebyte( ssc_cookie *ssc, int val );

int ssc_read( ssc_cookie *ssc, void *buffer, int num_bytes );
int ssc_write( ssc_cookie *ssc, const void *buffer, int num_bytes );

void ssc_reset( ssc_cookie *ssc );

int ssc_getcardtype( ssc_cookie *ssc );
status_t ssc_clearflash( ssc_cookie *ssc, uint32 bankmask );

//-----------------------------------------------------------------------------

#define SSC_BANK_0_SIZE	(64*1024) // boot
#define SSC_BANK_1_SIZE	(64*1024)
#define SSC_BANK_2_SIZE	(128*1024)
#define SSC_BANK_3_SIZE	(128*1024)
#define SSC_BANK_4_SIZE	(256*1024)

#define SSC_BANK_0_OFF	(0*1024)	// boot
#define SSC_BANK_1_OFF	(64*1024)
#define SSC_BANK_2_OFF	(128*1024)
#define SSC_BANK_3_OFF	(256*1024)
#define SSC_BANK_4_OFF	(384*1024)

// ----------

#define SSC_CMD_CLEAR_BANK_0	0x00	// 00 clear bank 0		(clear,read:1,write:2)
#define SSC_CMD_CLEAR_BANK_1	0x01	// 01 clear bank 1		(clear,read:1,write:2)
#define SSC_CMD_CLEAR_BANK_2	0x02	// 02 clear bank 2		(clear,read:1,nowrite)
#define SSC_CMD_CLEAR_BANK_3	0x03	// 03 clear bank 3		(clear,read:1,nowrite)
#define SSC_CMD_CLEAR_BANK_4	0x04	// 04 clear bank 4		(clear,read:1,nowrite)
#define SSC_CMD_CLEAR_BANK_3_4	0x05	// 05 clear bank 3+4	(clear,read:1,nowrite)
#define SSC_CMD_CLEAR_BANK_ALL	0x06	// 06 clear bank all	(clear,read:1,nowrite)

// 07 ??				(noclear,read:1,nowrite)
// 08 ??				(noclear,read:1,nowrite)
// 09 ??				(noclear,read:1,nowrite)
// 0a ??				(noclear,read:1,write:2)
// 0b ??				(noclear,read:1,write:2)
// 0c ??				(noclear,read:1,write:2)
// 0d ??				(noclear,read:1,write:2)
// 0e ??				(noclear,read:1,write:2)
// 0f ??				(noclear,read:1,write:2)
// 10 ??				(noclear,read:1,write:2)
// 11 ??				(noclear,read:1,write:2)
// 12 ??				(noclear,read:1,write:2)
// 13 ??				(noclear,read:1,write:2)
// 14 ??				(noclear,read:1,write:2)
// 15 ??				(noclear,read:1,write:2)
// 16 ??				(noclear,read:1,write:2)
// 17 ??				(noclear,read:1,write:2)
// 18 ??				(noclear,read:1,write:2)
// 19 ??				(noclear,read:1,write:2)
// 1a ??				(noclear,read:1,write:2)
// 1b ??				(noclear,read:1,write:2)
// 1c ??				(noclear,read:1,write:2)
// 1d ??				(noclear,read:1,write:2)
// 1e ??				(noclear,read:1,write:2)
// 1f ??				(noclear,read:1,write:2)

#define SSC_CMD_WRITE_BANK_1	0x20	// 20 write @ 64k		(noclear,read:256k,write:oo)
#define SSC_CMD_WRITE_BANK_2	0x21	// 21 write @ 128k		(noclear,read:1,write:oo)
#define SSC_CMD_WRITE_BANK_3	0x22	// 22 write @ 256k		(noclear,read:1,write:oo)
#define SSC_CMD_WRITE_BANK_4	0x23	// 23 write @ 384k		(noclear,read:1,write:1008k)

// 24 write @ 64k		(noclear,read:1,write:oo)
// 25 write @ 128k		(noclear,read:1,write:oo)
// 26 write @ 256k		(noclear,read:1,write:oo)
// 27 write @ 384k		(noclear,read:1,write:oo)

// 28 ??				(noclear,noread,nowrite!)

// 29 write @ 128k		(noclear,read:1,write:oo)
// 2a write @ 256k		(noclear,read:1,write:oo)
// 2b write @ 384k		(noclear,read:1,write:oo)

// 2c ??				(noclear,noread,nowrite)
// 2d ??				(noclear,noread,nowrite)

// 2e write @ 256k		(noclear,read:1,write:oo)

// 2f ??				(noclear,noread,nowrite)
// 30 ??				(noclear,read:1!,nowrite)
// 31 ??				(noclear,noread,nowrite)
// 32 ??				(noclear,noread,nowrite)
// 33 ??				(noclear,noread,nowrite)

#define SSC_CMD_WRITE_BANK_0	0x34	// 34 write @ 0k		(noclear!,read:1,write:oo)

// 35 ??				(noclear,noread,nowrite)
// 36 ??				(noclear,noread,nowrite)
// 37 ??				(noclear,noread,nowrite)
// 38 ??				(noclear,noread,nowrite)
// 39 ??				(noclear,noread,nowrite)
// 3a ??				(noclear,noread,nowrite)
// 3b ??				(noclear,noread,nowrite)
// 3c ??				(noclear,noread,nowrite)
// 3d ??				(noclear,noread,nowrite)
// 3e ??				(noclear,noread,nowrite)
// 3f ??				(noclear,noread,nowrite)
// 40 ??				(noclear,noread,nowrite)
// 41 ??				(noclear,noread,nowrite)
// 42 ??				(noclear,noread,nowrite)
// 43 ??				(noclear,noread,nowrite)
// 44 ??				(noclear,noread,nowrite)
// 45 ??				(noclear,noread,nowrite)
// 46 ??				(noclear,noread,nowrite)
// 47 ??				(noclear,noread,nowrite)
// 48 ??				(noclear,noread,nowrite)
// 49 ??				(noclear,noread,nowrite)
// 4a ??				(noclear,noread,nowrite)
// 4b ??				(noclear,noread,nowrite)
// 4c ??				(noclear,noread,nowrite)
// 4d ??				(noclear,noread,nowrite)
// 4e ??				(noclear,noread,nowrite)
// 4f ??				(noclear,noread,nowrite)
// 50 ??				(noclear,noread,nowrite)
// 51 ??				(noclear,noread,nowrite)
// 52 ??				(noclear,noread,nowrite)
// 53 ??				(noclear,noread,nowrite)
// 54 ??				(noclear,noread,nowrite)
// 55 ??				(noclear,noread,nowrite)
// 56 ??				(noclear,noread,nowrite)
// 57 ??				(noclear,noread,nowrite)
// 58 ??				(noclear,noread,nowrite)
// 59 ??				(noclear,noread,nowrite)
// 5a ??				(noclear,noread,nowrite)
// 5b ??				(noclear,noread,nowrite)
// 5c ??				(noclear,noread,nowrite)
// 5d ??				(noclear,noread,nowrite)
// 5e ??				(noclear,noread,nowrite)
// 5f ??				(noclear,noread,nowrite)

#define SSC_CMD_WRITE_RAM_BANK_0	0x60	// 60 write ram 0		(noclear,read:4k,write:8k)
#define SSC_CMD_WRITE_RAM_BANK_1	0x61	// 61 write ram 1		(noclear,read:4k,write:8k)
#define SSC_CMD_WRITE_RAM_BANK_2	0x62	// 62 write ram 2		(noclear,read:4k,write:8k)
#define SSC_CMD_WRITE_RAM_BANK_3	0x63	// 63 write ram 3		(noclear,read:4k,write:8k)

// 64 ??				(noclear,noread,nowrite)
// 65 ??				(noclear,noread,nowrite)
// 66 ??				(noclear,noread,nowrite)
// 67 ??				(noclear,noread,nowrite)
// 68 ??				(noclear,noread,nowrite)
// 69 ??				(noclear,noread,nowrite)
// 6a ??				(noclear,noread,nowrite)
// 6b ??				(noclear,noread,nowrite)
// 6c ??				(noclear,noread,nowrite)
// 6d ??				(noclear,noread,nowrite)
// 6e ??				(noclear,noread,nowrite)
// 6f ??				(noclear,noread,nowrite)
// 70 ??				(noclear,noread,nowrite)
// 71 ??				(noclear,noread,nowrite)
// 72 ??				(noclear,noread,nowrite)
// 73 ??				(noclear,noread,nowrite)
// 74 ??				(noclear,noread,nowrite)
// 75 ??				(noclear,noread,nowrite)
// 76 ??				(noclear,noread,nowrite)
// 77 ??				(noclear,noread,nowrite)
// 78 ??				(noclear,noread,nowrite)
// 79 ??				(noclear,noread,nowrite)
// 7a ??				(noclear,noread,nowrite)
// 7b ??				(noclear,noread,nowrite)
// 7c ??				(noclear,noread,nowrite)
// 7d ??				(noclear,noread,nowrite)
// 7e ??				(noclear,noread,nowrite)
// 7f ??				(noclear,noread,nowrite)
// 80 ??				(noclear,noread,nowrite)
// 81 ??				(noclear,noread,nowrite)
// 82 ??				(noclear,noread,nowrite)
// 83 ??				(noclear,noread,nowrite)
// 84 ??				(noclear,noread,nowrite)
// 85 ??				(noclear,noread,nowrite)
// 86 ??				(noclear,noread,nowrite)
// 87 ??				(noclear,noread,nowrite)

// 88 flash check?		(noclear,read:2,write:4) , reads: Flash=01A4,none=FFFF,Tetris=2F2F,Duck2=1358

#define SSC_CMD_CARD_TYPE	0x89	// 89 card type			(noclear,read:1,write:2)

#define SSC_CMD_READ_BANK_0	0x8A	// 8a read bank 0		(noclear,read:812k,write:oo)

// 8b self check?		(noclear,read:1,write:2) returns 55
// 8c ??				(noclear,noread,nowrite)
// 8d ??				(noclear,noread,nowrite)
// 8e ??				(noclear,noread,nowrite)
// 8f ??				(noclear,noread,nowrite)
// 90 ??				(noclear,noread,nowrite)
// 91 ??				(noclear,noread,nowrite)
// 92 ??				(noclear,noread,nowrite)
// 93 ??				(noclear,noread,nowrite)
// 94 ??				(noclear,noread,nowrite)
// 95 ??				(noclear,noread,nowrite)
// 96 ??				(noclear,noread,nowrite)
// 97 ??				(noclear,noread,nowrite)
// 98 ??				(noclear,noread,nowrite)
// 99 ??				(noclear,noread,nowrite)
// 9a ??				(noclear,noread,nowrite)
// 9b ??				(noclear,noread,nowrite)
// 9c ??				(noclear,noread,nowrite)
// 9d ??				(noclear,noread,nowrite)
// 9e ??				(noclear,noread,nowrite)
// 9f ??				(noclear,noread,nowrite)
// a0 ??				(noclear,noread,nowrite)
// a1 ??				(noclear,noread,nowrite)
// a2 ??				(noclear,noread,nowrite)
// a3 ??				(noclear,noread,nowrite)
// a4 ??				(noclear,noread,nowrite)
// a5 ??				(noclear,noread,nowrite)
// a6 ??				(noclear,noread,nowrite)
// a7 ??				(noclear,noread,nowrite)
// a8 ??				(noclear,noread,nowrite)
// a9 ??				(noclear,noread,nowrite)
// aa ??				(noclear,noread,nowrite)
// ab ??				(noclear,noread,nowrite)
// ac ??				(noclear,noread,nowrite)
// ad ??				(noclear,noread,nowrite)
// ae ??				(noclear,noread,nowrite)
// af ??				(noclear,noread,nowrite)
// b0 ??				(noclear,noread,nowrite)
// b1 ??				(noclear,noread,nowrite)
// b2 ??				(noclear,noread,nowrite)
// b3 ??				(noclear,noread,nowrite)
// b4 ??				(noclear,noread,nowrite)
// b5 ??				(noclear,noread,nowrite)
// b6 ??				(noclear,noread,nowrite)
// b7 ??				(noclear,noread,nowrite)
// b8 ??				(noclear,noread,nowrite)
// b9 ??				(noclear,noread,nowrite)
// ba ??				(noclear,noread,nowrite)
// bb ??				(noclear,noread,nowrite)
// bc ??				(noclear,noread,nowrite)
// bd ??				(noclear,noread,nowrite)
// be ??				(noclear,noread,nowrite)
// bf ??				(noclear,noread,nowrite)
// c0 ??				(noclear,noread,nowrite)
// c1 ??				(noclear,noread,nowrite)
// c2 ??				(noclear,noread,nowrite)
// c3 ??				(noclear,noread,nowrite)
// c4 ??				(noclear,noread,nowrite)
// c5 ??				(noclear,noread,nowrite)
// c6 ??				(noclear,noread,nowrite)
// c7 ??				(noclear,noread,nowrite)
// c8 ??				(noclear,noread,nowrite)
// c9 ??				(noclear,noread,nowrite)
// ca ??				(noclear,noread,nowrite)
// cb ??				(noclear,noread,nowrite)
// cc ??				(noclear,noread,nowrite)
// cd ??				(noclear,noread,nowrite)
// ce ??				(noclear,noread,nowrite)
// cf ??				(noclear,noread,nowrite)
// d0 ??				(noclear,noread,nowrite)
// d1 ??				(noclear,noread,nowrite)
// d2 ??				(noclear,noread,nowrite)
// d3 ??				(noclear,noread,nowrite)
// d4 ??				(noclear,noread,nowrite)
// d5 ??				(noclear,noread,nowrite)
// d6 ??				(noclear,noread,nowrite)
// d7 ??				(noclear,noread,nowrite)
// d8 ??				(noclear,noread,nowrite)
// d9 ??				(noclear,noread,nowrite)
// da ??				(noclear,noread,nowrite)
// db ??				(noclear,noread,nowrite)
// dc ??				(noclear,noread,nowrite)
// dd ??				(noclear,noread,nowrite)
// de ??				(noclear,noread,nowrite)
// df ??				(noclear,noread,nowrite)

#define SSC_CMD_READ_RAM_BANK_0	0xE0	// E0 read ram 0		(noclear,read:8k,write:16k)
#define SSC_CMD_READ_RAM_BANK_1	0xE1	// E0 read ram 1		(noclear,read:8k,write:16k)
#define SSC_CMD_READ_RAM_BANK_2	0xE2	// E0 read ram 2		(noclear,read:8k,write:16k)
#define SSC_CMD_READ_RAM_BANK_3	0xE3	// E0 read ram 3		(noclear,read:8k,write:16k)

// e4 ??				(noclear,noread,nowrite)
// e5 ??				(noclear,noread,nowrite)
// e6 ??				(noclear,noread,nowrite)
// e7 ??				(noclear,noread,nowrite)
// e8 ??				(noclear,noread,nowrite)
// e9 ??				(noclear,noread,nowrite)
// ea ??				(noclear,noread,nowrite)
// eb ??				(noclear,noread,nowrite)
// ec ??				(noclear,noread,nowrite)
// ed ??				(noclear,noread,nowrite)
// ee ??				(noclear,noread,nowrite)
// ef ??				(noclear,noread,nowrite)
// f0 ??				(noclear,noread,nowrite)
// f1 ??				(noclear,noread,nowrite)
// f2 ??				(noclear,noread,nowrite)
// f3 ??				(noclear,noread,nowrite)
// f4 ??				(noclear,noread,nowrite)
// f5 ??				(noclear,noread,nowrite)
// f6 ??				(noclear,noread,nowrite)
// f7 ??				(noclear,noread,nowrite)
// f8 ??				(noclear,noread,nowrite)
// f9 ??				(noclear,noread,nowrite)
// fa ??				(noclear,noread,nowrite)
// fb ??				(noclear,noread,nowrite)
// fc ??				(noclear,noread,nowrite)
// fd ??				(noclear,noread,nowrite)
// fe ??				(noclear,noread,nowrite)
// ff ??				(noclear,noread,nowrite)

#define CARD_NONE			0x00
#define CARD_SSTDFLASH		0x09
#define CARD_32KROM			0x10
#define CARD_64KROM			0x11
#define CARD_128KROM		0x12
#define CARD_256KROM		0x13
#define CARD_512KROM		0x14
#define CARD_AMDFLASH		0xA4

#define SSC_CARDTYPE_NONE		0x00 // no card in reader
#define SSC_CARDTYPE_SSTDFLASH	0x09 // SSTD flash card
#define SSC_CARDTYPE_AMDFLASH	0xA4 // AMD flash card
#define SSC_CARDTYPE_32KROM		0x10 // 32k (256 kbits) ROM
#define SSC_CARDTYPE_64KROM		0x11 // 64k (512 kbits) ROM
#define SSC_CARDTYPE_128KROM	0x12 // 128k (1024 kbits) ROM
#define SSC_CARDTYPE_256KROM	0x13 // 256k (2048 kbits) ROM
#define SSC_CARDTYPE_512KROM	0x14 // 512k (4096 kbits) ROM

//-----------------------------------------------------------------------------
#endif
