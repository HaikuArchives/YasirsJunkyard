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
#include <stdio.h>
#include <sys/types.h>
//-------------------------------------
#include <net/NetDebug.h>
//-------------------------------------
#include "NetStringEndpoint.h"
//-----------------------------------------------------------------------------

#if 0
****
-----------------------------7d0f033ea4
Content-Disposition: form-data; name="userfile1"; filename="C:\Autoexec.bak C:\Autoexec.bat"
Content-Type: application/octet-stream


-----------------------------7d0f033ea4--
****


****
userfile1=c%3A%5Cautoexec.bat&pan=up
****


****
-----------------------------7d017c8ea4
Content-Disposition: form-data; name="userfile1"; filename="C:\autoexec.bat"
Content-Type: application/octet-stream

mode con codepage prepare=((850) C:\WINDOWS\COMMAND\ega.cpi)
mode con codepage select=850
keyb no,,C:\WINDOWS\COMMAND\keyboard.sys
rem C:\PROGRA~1\NUMEGA\SOFTIC~1\WINICE.EXE
PATH=%PATH%;C:\MSSQL7\BINN

-----------------------------7d017c8ea4--
****






#endif

damn::NetStringEndpoint::NetStringEndpoint( BNetEndpoint *endpoint )
{
	fEndpoint = endpoint;
	fTimeout = B_INFINITE_TIMEOUT;
}

damn::NetStringEndpoint::~NetStringEndpoint()
{
	delete fEndpoint;
}

void damn::NetStringEndpoint::ReceiveData( void *data, size_t datalength )
{
	size_t recvlength = 0;

	fEndpoint->SetTimeout( fTimeout );

	// First, get the data from the queue:
	if( fData.size() )
	{
		size_t datasize = fData.size();
		if( datasize > datalength )
			datasize = datalength;
		printf( "Data from queue:%ld\n", datasize );
		for( size_t i=0; i<datasize; i++ )
		{
			((uint8*)data)[recvlength++] = fData.front();
			fData.pop_front();
		}
	}
	
	while( datalength != recvlength )
	{
		// recevive network data
		printf( "Data from socket:%ld\n", datalength-recvlength );
		status_t status = fEndpoint->Receive( (uint8*)data+recvlength, datalength-recvlength );
		if( status<B_NO_ERROR )
		{	
			status=fEndpoint->Error();
			throw status;
		}
		if( status == 0 )
			throw (status_t)B_TIMED_OUT; //timeout
		recvlength += status;
	}
}

BString damn::NetStringEndpoint::ReceiveString()
{
	status_t status;
	
    BString string;

	fEndpoint->SetTimeout( fTimeout );

    while( 1 )
    {
		while( fData.size() > 0 )
		{
			uint8 c = fData.front(); fData.pop_front();
			if( c == '\n' )
				return string;
			else if( c != '\r' )
				string += c;
		}
	
		// recevive network data
		uint8 ldata[1024];
		status = fEndpoint->Receive( ldata, sizeof(ldata) );
		if( status<B_NO_ERROR )
		{	
			status=fEndpoint->Error();
			throw status;
		}
		if( status == 0 ) throw (status_t)B_TIMED_OUT; //timeout
		fData.insert( fData.end(), ldata, ldata+status );
	}
}

void damn::NetStringEndpoint::SendData( const void *data, size_t datalength )
{
	if( (size_t)fEndpoint->Send(data,datalength) != datalength )
		throw (status_t)fEndpoint->Error();
}

	
void damn::NetStringEndpoint::SendString( const char *string, size_t stringlen )
{
	if( stringlen==(size_t)-1 ) stringlen=(size_t)strlen( string );
	if( (size_t)fEndpoint->Send(string,stringlen) != stringlen )
		throw (status_t)fEndpoint->Error();
}


//-----------------------------------------------------------------------------



