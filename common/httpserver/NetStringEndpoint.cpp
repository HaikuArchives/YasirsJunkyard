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

damn::NetStringEndpoint::NetStringEndpoint( BNetEndpoint *endpoint )
{
	fEndpoint = endpoint;
	fTimeout = B_INFINITE_TIMEOUT;
}

damn::NetStringEndpoint::~NetStringEndpoint()
{
	delete fEndpoint;
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

void damn::NetStringEndpoint::SendData( const void *data, int32 len )
{
	if( fEndpoint->Send(data,len) != len )
		throw (status_t)fEndpoint->Error();
}

	
void damn::NetStringEndpoint::SendString( const char *string, int32 len )
{
	if( len==-1 ) len=strlen(string);
	if( fEndpoint->Send(string,len) != len )
		throw (status_t)fEndpoint->Error();
}


//-----------------------------------------------------------------------------



