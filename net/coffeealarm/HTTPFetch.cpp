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
#include <string.h>
//-------------------------------------
#include <net/NetEndpoint.h>
#include <support/String.h>
//-------------------------------------
#include "NetStringEndpoint.h"
#include "HTTPFetch.h"
//-----------------------------------------------------------------------------

class URL // FIXME: find another name, and move to common
{
public:
	URL( const BString &url );
	
	const BString &Host() const { return fHost; }
	int Port() const { return fPort; }
	const BString &Request() const { return fRequest; }

private:
	BString fProtocol;
	BString fUsername;
	BString fPassword;
	BString fHost;
	uint16	fPort;
	BString fRequest;
};

URL::URL( const BString &url )
{
	fHost = url;
	int requestindex = fHost.FindFirst( '/' );
	if( requestindex >= 0 )
	{
		fRequest.SetTo( fHost.String()+requestindex );
		fHost.Truncate( requestindex );
	}
	
	fPort = 80; // hack!
	
	printf( "URL:Host:\"%s\"\n", fHost.String() );
	printf( "URL:Port:%d\n", fPort );
	printf( "URL:Req: \"%s\"\n", fRequest.String() );
}

//-----------------------------------------------------------------------------

status_t HTTPFetch( const char *url, BDataIO *io )
{
	status_t status;
	
	URL purl( url );
	
	BNetAddress hostaddr( purl.Host().String(), purl.Port() );
	
	BNetEndpoint *host = new BNetEndpoint;
	damn::NetStringEndpoint strhost( host );

	status = host->Connect( hostaddr );
	if( status < B_OK )
	{
		fprintf( stderr, "%s: could not connect: %s\n", url, strerror(status) );
		return status;
	}
	
	try
	{
		BString req; req << "GET " << purl.Request()  << " HTTP/1.0\n\n";
		strhost.SendString( req );

		// get header
		while( true )
		{
			BString string = strhost.ReceiveString();
			if( string.Length() == 0 )
				break;
			printf( "HEADER:\"%s\"\n", string.String() );
		}

		// get data
		char buffer[4096];
		while( true )
		{
			size_t recvlen = strhost.ReceiveData( buffer, sizeof(buffer), false );
			if( recvlen > 0 )
			{
				size_t writelen = io->Write( buffer, recvlen );
				if( writelen != recvlen )
				{
					fprintf( stderr, "%s: could not write data\n", url );
					return B_ERROR;
				}
			}
			else
				break;
		}
	}
	catch( ... )
	{
		fprintf( stderr, "%s: error while sending/reciving data\n", url );
		return B_ERROR;
	}
	
	return B_OK;
}

//-----------------------------------------------------------------------------
