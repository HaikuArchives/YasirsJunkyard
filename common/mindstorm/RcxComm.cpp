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
#include <deque>
#include <stdio.h>
//-------------------------------------
//-------------------------------------
#include "RcxComm.h"
//-----------------------------------------------------------------------------

//#define VERBOSE
//#define SENDRECV_VERBOSE

#define RCX_OP_PING				0x10
#define RCX_OP_BATTERYVOLTAGE	0x30
#define RCX_OP_MESSAGE			0xf7

//-----------------------------------------------------------------------------

damn::RcxComm::RcxComm( const char *serialdevice )
{
	fInitStatus = fSerPort.Open( serialdevice );
	if( fInitStatus >= B_OK )
	{
		fSerPort.SetDataRate( B_2400_BPS );
		fSerPort.SetDataBits( B_DATA_BITS_8 );
		fSerPort.SetParityMode( B_ODD_PARITY );
		fSerPort.SetStopBits( B_STOP_BITS_1 );
		fSerPort.SetFlowControl( B_NOFLOW_CONTROL );
		fSerPort.SetBlocking( true );
	}
	
	fLastSendCommand = 0;
}

damn::RcxComm::~RcxComm()
{
}

status_t damn::RcxComm::InitCheck() const
{
	return fInitStatus<B_ERROR?fInitStatus:B_OK;
}
	
//-----------------------------------------------------------------------------

ssize_t damn::RcxComm::Read( void *buf, size_t count )
{
	ssize_t readlen = fSerPort.Read( buf, count );
#ifdef SENDRECV_VERBOSE
	fprintf( stderr, "R>" );
	for( int i=0; i<readlen; i++ )
	{
		if( i!=0 )
			fprintf( stderr, " " );
		fprintf( stderr, "%02X", ((uint8*)buf)[i] );
	}
	fprintf( stderr, "\n" );
#endif
	return readlen;
}

ssize_t damn::RcxComm::Write( const void *buf, size_t count )
{
#ifdef SENDRECV_VERBOSE
	fprintf( stderr, "W>" );
	for( uint i=0; i<count; i++ )
	{
		if( i!=0 )
			fprintf( stderr, " " );
		fprintf( stderr, "%02X", ((uint8*)buf)[i] );
	}
	fprintf( stderr, "\n" );
#endif
	return fSerPort.Write( buf, count );
}

//-----------------------------------------------------------------------------

status_t damn::RcxComm::RecvBuffer( void *data, size_t datalength )
{
	int retrycnt = 3;
	bigtime_t timeout = 1000000 / (2400/(10+10));
	std::deque<uint8> recvbuffer;
	while( true )
	{
retry:
		if( retrycnt-- <= 0 )
			break;

		fSerPort.SetTimeout( timeout );
		timeout *= 2;

		while( 1 )
		{
			uint8 datablock[8];
			size_t blocklength = Read( datablock, sizeof(datablock) );
			if( blocklength <= 0 ) break;
			recvbuffer.insert( recvbuffer.end(), datablock, datablock+blocklength );
		}

		fSerPort.SetTimeout( B_INFINITE_TIMEOUT );
retryheader:		
		// find header:
		while( recvbuffer.size() >= 3 )
		{
			if( recvbuffer[0]==0x55 && recvbuffer[1]==0xff && recvbuffer[2]==0x00 )
				break;
			recvbuffer.pop_front();
		}

		if( recvbuffer.size()<3+2+2 )
		{
#ifdef VERBOSE
			fprintf( stderr, "RcxComm::RecvBuffer(): got weird bufferlength, retrying...\n" );
#endif
			goto retry; // we need more data!
		}
			
		// decode data:
		uint decodelen = 0;
		uint8 checksum = 0;
		uint decodelen_valid = -1;
		uint decodelen_valid_checksum;
		
#ifdef VERBOSE
		fprintf( stderr, "D>" );
		for( int i=0; i<recvbuffer.size(); i++ )
		{
			if( i!=0 )
				fprintf( stderr, " " );
			fprintf( stderr, "%02X", recvbuffer[i] );
		}
		fprintf( stderr, "\n" );
#endif

		for( uint i=3; i<recvbuffer.size()-3; i+=2 )
		{
//			fprintf( stderr, ">>%d\n", i );
			decodelen = (i-3)/2;

			if( recvbuffer.size()-i>=5 && recvbuffer[i+2]==0x55 && recvbuffer[i+3]==0xff && recvbuffer[i+4]==0x00 )
				break;
				
			if( i==3 && recvbuffer[i]==fLastSendCommand )
			{
				for( int i=0; i<3+2+2; i++ )
					recvbuffer.pop_front();
#ifdef VERBOSE
				fprintf( stderr, "RcxComm::RecvBuffer(): weird IR reflection, retrying...\n" );
#endif
				goto retryheader;
			}
		
			if( recvbuffer[i] != (recvbuffer[i+1]^0xff) )
			{
				if( decodelen_valid >= 0 )
				{
#ifdef VERBOSE
					fprintf( stderr, "RcxComm::RecvBuffer(): found early checksum, done...\n" );
#endif
					decodelen = decodelen_valid;
					checksum = decodelen_valid_checksum;
					break;
				}
				else
				{
					recvbuffer.pop_front();
#ifdef VERBOSE
					fprintf( stderr, "RcxComm::RecvBuffer(): got bogus data, retrying...\n" );
#endif
					goto retry;
				}
			}

			if( recvbuffer.size()-i>=2 && recvbuffer[i]==checksum )
			{
				decodelen_valid = decodelen-1;
				decodelen_valid_checksum = checksum;
			}

			if( decodelen < datalength )
			{
				((uint8*)data)[decodelen] = recvbuffer[i];
//				fprintf( stderr, "%d:%d\n", decodelen, recvbuffer[i] );
			}
			checksum += recvbuffer[i];
		}
		decodelen++;
		// check checksum:
		fprintf( stderr, "%d [%02X %02X] [%02X]\n", decodelen, recvbuffer[3+decodelen*2], recvbuffer[3+decodelen*2+1]^0xff, checksum );
		if( recvbuffer[3+decodelen*2]!=(recvbuffer[3+decodelen*2+1]^0xff) || checksum!=recvbuffer[3+decodelen*2] )
		{
			// not enough data, or a bad checksum, there is no way to know :(
#ifdef VERBOSE
			fprintf( stderr, "RcxComm::RecvBuffer(): illegal checksum, retrying...\n" );
#endif
			goto retry;
		}
		
		return decodelen<datalength ? decodelen : datalength;
	}
	
	return B_ERROR;
}

status_t damn::RcxComm::SendBuffer( const void *data, size_t datalength )
{
	uint8 header[3] = { 0x55, 0xff, 0x00 };
	Write(header, 3 );

	uint8 checksum = 0;
	uint8 val[2];
		
	for( uint i=0; i<datalength; i++ )
	{
		val[0] = ((uint8*)data)[i];
		if( i == 0 )
		{
			if( val[0] == fLastSendCommand )
				val[0] ^= 0x08;
			fLastSendCommand = val[0];
		}
		val[1] = val[0] ^ 0xff;
		Write( val, 2 );
		checksum += val[0];
	}
	val[0] = checksum;
	val[1] = val[0] ^ 0xff;
	Write( val, 2 );
	
	return B_OK;
}

status_t damn::RcxComm::SendCommand( const void *senddata, size_t senddatalength, void *recvdata, size_t recvdatalen )
{
	uint8 lastcommand = fLastSendCommand;

	int retrycnt = 3;
	while( retrycnt-- > 0 )
	{
		fSerPort.ClearInput();
		fLastSendCommand = lastcommand;
		SendBuffer( senddata, senddatalength );

		int replylen = RecvBuffer( recvdata, recvdatalen );
		if( replylen > 0 )
			return replylen;
	}

	fLastSendCommand = lastcommand;
	return B_ERROR;
}

status_t damn::RcxComm::Sync()
{
// if synced return B_OK;

	uint8 command[1] = { RCX_OP_PING };
	uint8 reply[8];
	
	status_t status = SendCommand( command,sizeof(command),reply,sizeof(reply) );
	if( status < B_OK )
	{
		status_t status = SendCommand( command,sizeof(command),reply,sizeof(reply) );
		if( status < B_OK )
			return status;
	}
	
//	synched!
	return B_OK;
}

//-----------------------------------------------------------------------------

status_t damn::RcxComm::GetBatteryVoltage()
{
	status_t status = Sync();
	if( status < B_OK ) return status;
	
	uint8 command[1] = { RCX_OP_BATTERYVOLTAGE };
	uint8 reply[8];
	
	status = SendCommand( command,sizeof(command),reply,sizeof(reply) );
	if( status != 3 )
		return B_ERROR;

	return reply[1] + (reply[2]<<8);
}

status_t damn::RcxComm::SendMessage( uint8 msg )
{
	status_t status = Sync();
	if( status < B_OK ) return status;
	
	fSerPort.ClearInput();

	uint8 command[2] = { RCX_OP_MESSAGE, msg };
	SendBuffer( command, sizeof(command) );

	return B_OK;
}
