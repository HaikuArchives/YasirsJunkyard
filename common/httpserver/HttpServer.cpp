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
#include <assert.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <vector>
//-------------------------------------
#include <kernel/OS.h>
#include <net/NetDebug.h>
#include <support/List.h>
//-------------------------------------
#include "HttpServer.h"
//-----------------------------------------------------------------------------

/** \class damn::HttpServer
 * A baseclass for a simple http server.
 *
 * 
 * Example micro server:
 * <pre>
 * class MicroServer : public damn::HttpServer
 * {
 * public:
 * 	MicroServer( const BNetAddress &addr ) : damn::HttpServer( addr )
 * 	{
 * 	}
 * 
 * private:
 * 	void RequestReceived( Connection *connection )
 * 	{
 * 		if( connection->GetCommand() == "GET" )
 * 		{
 * 			static const char s[] = "<html><body>Test!</body></html>";
 * 			connection->SendData( s, strlen(s), "text/html" );
 * 		}
 * 		else
 * 			connection->SendError( HTTP_ERR_BAD_REQUEST, "Unknown command" );
 * 	}
 * };
 * </pre>
 */

//-----------------------------------------------------------------------------

/** Construct a server object.
 * This does not start the server, but only inttializes the socket.
 * If the initialization fails, Go() will fail.<br>
 * To start the server call Go().
 */
damn::HttpServer::HttpServer( const BNetAddress &addr ) :
	fConnectionsLock( "ConnectionListLock" )
{
	fThreadId = -1;

	fInitStatus = fEndpoint.Bind( addr );
	if( fInitStatus != B_NO_ERROR ) return;
	
	fInitStatus = fEndpoint.Listen( 10 );
	if( fInitStatus != B_NO_ERROR ) return;

	fSelfDestruct = false;
}

/** Destroys the server object.
 * All clients connected to the server will get disconnected.
 */
damn::HttpServer::~HttpServer()
{
	// stop the thread:
	if( fThreadId >= 0 )
	{
		fSelfDestruct = false;
		fQuitThread = true;
		fEndpoint.Close();
		fprintf( stderr, "Waiting for server thread to quit...\n" );
		status_t retval;
		wait_for_thread( fThreadId, &retval );
	}

	// kill all the connections:
	while( 1 )
	{
		fConnectionsLock.Lock();
		if( fConnections.CountItems() == 0 ) break;
		fprintf( stderr, "There is %lu connections alive...\n", fConnections.CountItems() );
		
	again:
		for( int i=0; i<fConnections.CountItems(); i++ )
		{
			HttpServer::Connection *connection = fConnections.ItemAt( i );
			delete connection;
			goto again;
		}

		fConnectionsLock.Unlock();
		snooze( 1000000/10 );
	}
}

status_t damn::HttpServer::Go( bool spawnthread )
{
	if( fInitStatus != B_NO_ERROR ) return fInitStatus;
	
	if( spawnthread )
	{
		fQuitThread = false;
		fSelfDestruct = true;
		fThreadId = spawn_thread( _AcceptLoop, "accept loop", B_NORMAL_PRIORITY, this );
		fInitStatus = resume_thread( fThreadId );
		if( fInitStatus != B_NO_ERROR )
		{
			kill_thread( fThreadId );
			return fInitStatus;
		}
		return fThreadId;
	}
	else
	{
		AcceptLoop();
	}

	return B_NO_ERROR;
}
	

void damn::HttpServer::Quit()
{
	fQuitThread = true;
}

long damn::HttpServer::_AcceptLoop( void *_this )
{
	HttpServer *server = (HttpServer*)_this;
	server->AcceptLoop();
	if( server->fSelfDestruct )
		delete _this;
	return 0;
}

void damn::HttpServer::AcceptLoop()
{
	while( !fQuitThread )
	{
		// Wait for connection
		BNetDebug::Print( "waiting for connection..." );
		BNetEndpoint *client = fEndpoint.Accept();
		BNetDebug::Print( "connected..." );
		if( client == NULL )
		{
			fprintf( stderr, "Accept returned a NULL pointer!!?!\n" );
			break;
		}

		bool result = fConnectionsLock.Lock();
		assert( result );
		if( ConnectionRequested(client->RemoteAddr()) )
		{
			Connection *connection = new Connection( this, client );
			if( connection->Go() < B_NO_ERROR )
			{
				fprintf( stderr, "Could not create connection\n" );
				delete connection;
			}
			fConnections.AddItem( connection );
		}
		else
			delete client;
		fConnectionsLock.Unlock();
	}
}

bool damn::HttpServer::ConnectionRequested( const BNetAddress &/*address*/ )
{
	return true;
}

BString damn::HttpServer::GetStat() const
{
	BString string;
	
	bool result = fConnectionsLock.Lock();
	assert( result );
	
	string << "<html><head>\n";
	string << "<meta http-equiv=\"refresh\" content=\"5\">\n";
	string << "<meta http-equiv=\"pragma\" content=\"no-cache\">\n";
	string << "<meta http-equiv=\"cache-control\" content=\"no-cache\">\n";
	string << "<title>DamnHttpServer stats</title></head>\n";
	string << "<body bgcolor=\"#c0c0c0\" text=\"#000000\">\n";

	string << "<table border=1>\n";
	for( int i=0; i<fConnections.CountItems(); i++ )
	{
		const Connection *connection = fConnections.ItemAt( i );
		// index | client:port | file | ref | agent
		string << "<tr>";

		string << "<td>" << connection->GetClientHostName() << ":" << (int32)connection->GetClientPort() <<"<br>";
		string << connection->GetPath() << "</td>";

		string << "<td>";
		ssize_t pos = connection->fCurrentPos;
		ssize_t tot = connection->fTotSize;
		float pct = (float(pos)/float(tot))*100.0f;
		if( tot>0 )	string << pos/1024 << "K/" << tot/1024 << "K&nbsp;(" << pct << "%)<br>";
		else		string << pos/1024 << "K/?<br>";
		bigtime_t nowtime = system_time();
		float kps = (float(pos)/1024.0f) / ((float(nowtime)-float(connection->fStartTime))/1000000.0f);
		string << kps << "K/s";
		string << "</td>";

		string << "<td>";
//		for( int j=0; j<connection->GetParameterCnt(); j++ )
//		{
//			const Connection::Parameter &param = connection->GetParameter( j );
//			string << param.fName << "=" << param.fValue << "<br>";
//		}
		for( std::map<BString,BString>::const_iterator iparam=connection->GetParameters().begin(); iparam!=connection->GetParameters().end(); iparam++ )
			string << iparam->first << "=" << iparam->second << "<br>";

		string << "</td>";
		string << "</tr>";
	}
	string << "</table>\n";

	string << "</body>\n";

	fConnectionsLock.Unlock();
	
	return string;
}

//------------------------------------------------------------------------------

damn::HttpServer::Connection::Connection( HttpServer *server, BNetEndpoint *endpoint ) :
	fServer( server ),
	fEndpoint( endpoint )
{
	fEndpoint.SetTimeout( 60 * 1000000 );
	fInitStatus = B_NO_ERROR;
	fSelfDestruct = false;

	fContentLength = 0;
	fContent = NULL;
	fDecodedContent = NULL;

	in_addr addr;
	if( endpoint->RemoteAddr().GetAddr(addr,&fClientPort) == B_NO_ERROR )
	{
		struct hostent *hent = gethostbyaddr( (char*)&addr.s_addr, sizeof(struct in_addr), AF_INET );
		if( hent!=NULL && hent->h_name!=NULL )
			sprintf( fClientHostName, "%s", hent->h_name );
		else
		{
			uint32 naddr = ntohl(addr.s_addr);
			sprintf( fClientHostName, "%ld.%ld.%ld.%ld", (naddr>>24)&0xff, (naddr>>16)&0xff, (naddr>>8)&0xff, (naddr>>0)&0xff );
		}
	}
	else
	{
		strcpy( fClientHostName, "" );
		fClientPort = 0;
	}
}

damn::HttpServer::Connection::~Connection()
{
	fSelfDestruct = false;

	fEndpoint.Close();
	if( fThreadId >= 0 )
	{
		fQuitThread = true;
		fprintf( stderr, "Waiting for connection thread to quit...\n" );
		status_t retval;
		wait_for_thread( fThreadId, &retval );
	}

//	for( int i=fParameters.CountItems()-1; i>=0; i-- )
//		delete fParameters.ItemAtFast(i);

	free( fContent );
	delete fDecodedContent;
	
	bool result;
	result = fServer->fConnectionsLock.Lock();
	if( result == false )
	{
		fprintf( stderr, "Could not lock connection list: server:%p threadid:%ld lockingthreadid:%ld semid:%ld lockcnt:%ld lockreqcnt:%ld\n",
			fServer, find_thread(NULL), fServer->fConnectionsLock.LockingThread(), fServer->fConnectionsLock.Sem(), fServer->fConnectionsLock.CountLocks(),  fServer->fConnectionsLock.CountLockRequests() );
	}
	assert( result );
	result = fServer->fConnections.RemoveItem( this );
	assert( result );
	fServer->fConnectionsLock.Unlock();
}

thread_id damn::HttpServer::Connection::Go()
{
	if( fInitStatus != B_NO_ERROR ) return fInitStatus;
	
	fQuitThread = false;
	fSelfDestruct = true;
	fThreadId = spawn_thread( _Loop, "client loop", B_NORMAL_PRIORITY, this );
	fInitStatus = resume_thread( fThreadId );
	if( fInitStatus != B_NO_ERROR )
	{
		kill_thread( fThreadId );
		return fInitStatus;
	}
	return fThreadId;
}

long damn::HttpServer::Connection::_Loop( void *_this )
{
	HttpServer::Connection *_connection = (HttpServer::Connection*)_this;
	_connection->Loop();
	_connection->fThreadId = -1;
	if( _connection->fSelfDestruct )	delete _connection;
	return 0;
}

void damn::HttpServer::Connection::Loop()
{
	try
	{
		fKeepAlive = false;
		do
		{
			fRequest = "<requesting>";
//			for( int i=fParameters.CountItems()-1; i>=0; i-- )
//				delete fParameters.ItemAtFast(i);
//			fParameters.MakeEmpty();
			fParameters.empty();

			fContentType = "";
			fContentLength = 0;
			fContent = NULL;

			delete fDecodedContent;
			fDecodedContent = NULL;

			fStartTime = 0;
			fCurrentPos = 0;
			fTotSize = 0;

			fRequest = fEndpoint.ReceiveString();
			if( fRequest == "" )
				continue;
//-- decode --
			fMajorVersion = -1;
			fMinorVersion = -1;

			int parampos = fRequest.FindFirst( ' ' );
			if( parampos <= 0 )
			{
				fCommand = fRequest;
			}
			else
			{
				fRequest.CopyInto( fCommand, 0, parampos );
		
				int verpos = fRequest.FindLast( " HTTP/" );
				if( verpos<=parampos || sscanf(fRequest.String()+verpos+6, "%d.%d", &fMajorVersion, &fMinorVersion)!=2 )
				{
					fMajorVersion = -1;
					fMinorVersion = -1;
					fRequest.CopyInto( fArg, parampos+1, fRequest.Length()-parampos );
				}
				else
				{
					fRequest.CopyInto( fArg, parampos+1, verpos-parampos-1 );
				}
			}

			// Split fArg into fPath and a list of CGI parms
			// (split, then deescape)
			int cgipos = fArg.FindFirst( '?' );
			if( cgipos >= 0 )
			{
				// process cgi parms
				fPath = DeescapeString( fArg.String(), cgipos );
				while( cgipos >= 0 )
				{
					BString cgiparam;

					int begcgi = cgipos;
					cgipos = fArg.FindFirst( '&', begcgi+1 );
					if( cgipos >= 0 )
						cgiparam = DeescapeString( fArg.String()+begcgi+1, cgipos-(begcgi+1) );
					else
						cgiparam = DeescapeString( fArg.String()+begcgi+1 );

					printf( "CGI: [%s]\n", cgiparam.String() );
				}
			}
			else
			{
				fPath = DeescapeString( fArg );
			}
//?userfile1=test123&pan=up HTTP/1.1  			
			

			BString param;
			while( (param=fEndpoint.ReceiveString()) != "" )
			{
//				printf( "Opt: \"%s\"\n", param.String() );
				int split = param.FindFirst( ':' );
				if( split > 0 )
				{
#if 0
					Parameter *p = new Parameter;
					param.CopyInto( p->fName, 0, split );
					param.CopyInto( p->fValue, split+1, param.Length()-split );
					p->fName.RemoveFirst( " " );
					p->fName.RemoveLast( " " );
					p->fValue.RemoveFirst( " " );
					p->fValue.RemoveLast( " " );
//					printf( "Opt: [%s=%s]\n", p->fParam.String(), p->fValue.String() );
					fParameters.AddItem( p );
					if( p->fName=="Connection" && p->fValue=="Keep-Alive" )
						fKeepAlive = true;
#else
					BString key,val;
					param.CopyInto( key, 0, split );
					param.CopyInto( val, split+1, param.Length()-split );
					key.ToLower();
					key.RemoveFirst( " " );
					key.RemoveLast( " " );
					val.RemoveFirst( " " );
					val.RemoveLast( " " );
//					printf( "Opt: [%s=%s]\n", key.String(), val.String() );
					fParameters[key] = val;
#endif
				}
				else
					fprintf( stderr, "Invalid parameter: [%s]\n", param.String() );
			}
			
			std::map<BString,BString>::iterator iparm;
			if( (iparm=fParameters.find("connection"))!=fParameters.end() && iparm->second=="Keep-Alive" )
				fKeepAlive = true;

			if( (iparm=fParameters.find("content-length"))!=fParameters.end() )
			{
				size_t length = atoi( iparm->second.String() );
				if( length>0 && length<16*1024*1024 ) // FIXME: this should be configurable!
				{
					fContent = malloc( length );
					if( fContent != NULL )
					{
						fContentLength = length;
						fEndpoint.ReceiveData( fContent, fContentLength );
					}
				}
				if( (iparm=fParameters.find("content-type"))!=fParameters.end() )
					fContentType = iparm->second;
			}
			
			
			

			rename_thread( find_thread(NULL), fPath.String() );

			if( fArg == "/server-stat.html" )
			{
				BString string = fServer->GetStat();
				SendData( string.String(), string.Length(), "text/html" );
			}
			else
			{
				fServer->RequestReceived( this );
			}
	
		}
		while( fKeepAlive && !fQuitThread );
	}
	catch( status_t status )
	{
		fprintf( stderr, "HTTPServer::Connection::Loop(): cought status_t exception: %s\n", strerror(status) );
		return;
	}
	catch( ... )
	{
		fprintf( stderr, "HTTPServer::Connection::Loop(): cought unknown exception\n" );
		return;
	}
}

// FIXME: this function is way to hardcoded, loosen it up a bit...
const damn::HttpServer::Connection::UrlEncodedForm *damn::HttpServer::Connection::GetContent()
{
	if( fDecodedContent )
		return fDecodedContent;

	if( fContentLength==0 || fContent==NULL )
		return NULL;

	std::map<BString,BString>::iterator iparam;
	if( (iparam=fParameters.find("content-type"))==fParameters.end() )
		return NULL;

	if( iparam->second.IFindFirst("multipart/form-data;boundary=")!=0 )
		return NULL;

	const char *boundary = iparam->second.String() + iparam->second.FindFirst( "=" ) + 1;
//	printf( "BOUND: <<%s>>\n", boundary );
	size_t boundarylen = strlen( boundary );
	
	std::vector<const char*> beginmarks;
	std::vector<const char*> endmarks;
	
	for( uint i=0; i<fContentLength-boundarylen; i++ )
	{
		const char *content = (char*)fContent+i;
		if( *content==boundary[0] && memcmp(content,boundary,boundarylen)==0 )
		{
			// check if this is a endmark:
			if( fContentLength-i>=(boundarylen+2) && content[boundarylen]=='-' && content[boundarylen+1]=='-' )
				endmarks.push_back( content );
			else
				beginmarks.push_back( content );
		}
	}

//	printf( "BEGIN:\n" );
//	for( uint i=0; i<beginmarks.size(); i++ )
//		printf( "%d:%ld:<<<<%.*s>>>>\n", i, beginmarks[i]-(char*)fContent, (int)boundarylen, beginmarks[i] );
//	printf( "END:\n" );
//	for( uint i=0; i<endmarks.size(); i++ )
//		printf( "%d:%ld:<<<<%.*s>>>>\n", i, endmarks[i]-(char*)fContent, (int)boundarylen+2, endmarks[i] );

	if( beginmarks.size()!=1 || endmarks.size()!=1 )
		return NULL;
		
	

#if 0

	char boundary[1024];
	if( sscanf(iparm->second.String(), "form-data; name=\"%1000s\"; filename=\"%1000s\"", name, filename );
	printf( ">> <%s> <%s>\n", name, filename );


content-type=multipart/form-data;boundary=---------------------------7d08236ea4

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

	

	if( (iparm=fParameters.find("content-disposition"))==fParameters.end() )
		return NULL;
	
	char name[1024];
	char filename[1024];
	sscanf( iparm->second.String(), "form-data; name=\"%1000s\"; filename=\"%1000s\"", name, filename );
	printf( ">> <%s> <%s>\n", name, filename );
#endif
	return NULL;
}

void damn::HttpServer::Connection::SendData( const void *data, ssize_t size, const char *mimetype, const BString *extraheader )
{
	if( size<0 ) fKeepAlive = false;

	fTotSize = size;
	fStartTime = system_time();

	if( fMajorVersion>=0 && fMinorVersion>=0 )
	{
		BString header;
		header << "HTTP/1.0 200 OK\n";
		if( mimetype ) header << "Content-Type: " << mimetype << "\n";
		if( size >= 0 ) header << "Content-Length: " << size << "\n";
		header << "Connection: " << ((fKeepAlive)?"Keep-Alive":"Close") << "\n";
		if( extraheader )
			header << *extraheader;
		header << "\n";
		fEndpoint.SendString( header );
	}

	fEndpoint.SendData( data, size );
}

void damn::HttpServer::Connection::SendDataHeader( ssize_t size, const char *mimetype )
{
	if( size<0 ) fKeepAlive = false;

	fTotSize = size;
	fStartTime = system_time();

	if( fMajorVersion>=0 && fMinorVersion>=0 )
	{
		BString header;
		header << "HTTP/1.0 200 OK\n";
		if( mimetype ) header << "Content-Type: " << mimetype << "\n";
		if( size >= 0 ) header << "Content-Length: " << size << "\n";
		header << "Connection: " << ((fKeepAlive)?"Keep-Alive":"Close") << "\n";
		header << "\n";
		fEndpoint.SendString( header );
	}
}

void damn::HttpServer::Connection::SendDataBlock( const void *data, size_t size )
{
	fEndpoint.SendData( data, size );
	fCurrentPos += size;
}

void damn::HttpServer::Connection::SendError( int errorno, const char *string )
{
	const char *errstring = string?string:"ERROR";
	
	BString content;
	content << "<html>\n";
	content << "<head><title>" << (int32)errorno << " " << errstring << "</title></head>\n";
	content << "<body><h2>" << (int32)errorno << " " << errstring << "</h2></body>\n";
	content << "</html>\n";
	
	if( fMajorVersion>=0 && fMinorVersion>=0 )
	{
		BString header;
		header << "HTTP/1.0 " << (int32)errorno << " " << errstring << "\n";
		header << "Content-Type: text/html\n";
		header << "Content-Length: " << content.Length() << "\n";
		header << "Connection: " << ((fKeepAlive)?"Keep-Alive":"Close") << "\n";
		header << "\n";
		fEndpoint.SendData( header.String(), header.Length() );
	}

	fEndpoint.SendData( content.String(), content.Length() );
}

void damn::HttpServer::Connection::SendRedirection( const char *newurl, bool permanent )
{
	int errorno = permanent ? HTTP_ERR_MOVED_PERMANENTLY : HTTP_ERR_MOVED_TEMPORARILY;

	BString content;
	content << "<html>\n";
	content << "<head><title>" << (int32)errorno << " Moved</title></head>\n";
	content << "<body><h2>";
	content << (int32)errorno << " Moved<br>";
	content << "You can find it <a href=\"" << newurl << "\">here</a>.";
	content << "</h2></body>\n";
	content << "</html>\n";
	
	if( fMajorVersion>=0 && fMinorVersion>=0 )
	{
		BString header;
		header << "HTTP/1.0 " << (int32)errorno << " " << "Moved\n";
		header << "Content-Type: text/html\n";
		header << "Location: " << newurl << "\n";
		header << "Content-Length: " << content.Length() << "\n";
		header << "Connection: " << ((fKeepAlive)?"Keep-Alive":"Close") << "\n";
		header << "\n";
		fEndpoint.SendData( header.String(), header.Length() );
	}

	fEndpoint.SendData( content.String(), content.Length() );
}

//------------------------------------------------------------------------------

BString damn::HttpServer::Connection::EscapeString( const BString &original )
{
	return original; // FIXME:
}

BString damn::HttpServer::Connection::DeescapeString( const BString &original )
{
	return DeescapeString( original.String(), original.Length() );
}

BString damn::HttpServer::Connection::DeescapeString( const char *original, int length )
{
	assert( original != NULL );

	if( length == -1 ) length = strlen( original );

	BString deescaped;
	for( int i=0; i<length; i++ )
	{
		int c = (uchar)original[i];
		if( c == '%' )
		{
			int c1 = length-i>1 ? (uchar)original[i+1] : -1;
			int c2 = length-i>2 ? (uchar)original[i+2] : -1;
			
			if( c1 == '%' )
			{
				deescaped += c;
				i++;
			}
			else if( isxdigit(c1) && isxdigit(c2) )
			{
				uchar cx = (c1<='9'?c1-'0':toupper(c1)-'A'+10)<<4 | (c2<='9'?c2-'0':toupper(c2)-'A'+10);
				if( cx > 0 )
				{
					deescaped += cx;
					i += 2;
				}
				else
					deescaped += c;
			}
			else
				deescaped += c;
		}
		else
			deescaped += c;
	}
	
	return deescaped;
}

//------------------------------------------------------------------------------
