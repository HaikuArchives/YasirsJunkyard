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
//-------------------------------------
#include <net/NetDebug.h>
//-------------------------------------
#include "httpserver/HttpServer.h"
//-----------------------------------------------------------------------------

class MicroServer : public damn::HttpServer
{
public:
	MicroServer( const BNetAddress &addr ) : damn::HttpServer( addr )
	{
	}

private:
	void RequestReceived( Connection *connection )
	{
		printf( "===============================================================================\n" );
		printf( "HOST: %s:%d\n", connection->GetClientHostName(), connection->GetClientPort() );
		printf( "REQ:  %s\n", connection->GetRequest().String() );
		printf( "CMD:  %s\n", connection->GetCommand().String() );
		printf( "ARG:  %s\n", connection->GetArg().String() );
		printf( "URI:  %s\n", connection->GetPath().String() );
		printf( "VER:  %d.%d\n", connection->GetVersion()/65536, connection->GetVersion()%65536 );
//		for( int i=0; i<connection->GetParameterCnt(); i++ )
//		{
//			printf( "PARM: %s=%s\n", connection->GetParameter(i).fName.String(), connection->GetParameter(i).fValue.String() );
//		}
		for( std::map<BString,BString>::const_iterator iparam=connection->GetParameters().begin(); iparam!=connection->GetParameters().end(); ++iparam )
			printf( "PARM: %s=%s\n", iparam->first.String(), iparam->second.String() );

		const Connection::UrlEncodedForm *content = connection->GetContent();
		if( content )
		{
		}
	
		if( connection->GetCommand()=="GET" || connection->GetCommand()=="POST" )
		{
//			static const char s[] = "<html><body>Test!</body></html>";
			static const char s[] =
				"<html><body>"

				"<FORM ENCTYPE=\"multipart/form-data\" ACTION=\"_URL_\" METHOD=POST>"
				"File to process: <INPUT NAME=\"userfile1\" TYPE=\"file\">"
				"<INPUT TYPE=\"submit\" VALUE=\"Send File\">"
				"</FORM>"

				"<hr>"

				"<form ENCTYPE=\"application/x-www-form-urlencoded\" METHOD=POST>"
				"File to process: <INPUT NAME=\"userfile1\" TYPE=\"file\">"
				"<input type=\"submit\" name=\"pan\" value=\"up\">"
				"</form>"

				"<hr>"

				"<form ENCTYPE=\"application/x-www-form-urlencoded\" METHOD=GET>"
				"File to process: <INPUT NAME=\"userfile1\" TYPE=\"file\">"
				"<input type=\"submit\" name=\"pan\" value=\"up\">"
				"</form>"

				"</body></html>";

			connection->SendData( s, strlen(s), "text/html" );
		}
		else
			connection->SendError( HTTP_ERR_BAD_REQUEST, "Unknown command" );
	}
};

//-------------------------------------

int main()
{
	BNetDebug::Enable( true );
	
	MicroServer *server = new MicroServer( BNetAddress((const char*)NULL,80) );
	server->Go( false );
	return 0;
}

//-----------------------------------------------------------------------------
