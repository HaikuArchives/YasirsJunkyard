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
#ifndef DAMN_HTTPSERVER_H
#define DAMN_HTTPSERVER_H
//-----------------------------------------------------------------------------
//-------------------------------------
#include <net/netdb.h>
#include <net/NetEndpoint.h>
#include <support/Locker.h>
#include <support/String.h>
class BNetAddress;
//-------------------------------------
#include "misc/List.h"
#include "NetStringEndpoint.h"
//-----------------------------------------------------------------------------

enum HTTPErrors
{
	HTTP_ERR_OK =					200,	// 200 OK
	HTTP_ERR_MOVED_PERMANENTLY =	301,	// 301 Moved Permanently
	HTTP_ERR_MOVED_TEMPORARILY =	302,	// 302 Moved Temporarily
	HTTP_ERR_BAD_REQUEST =			400,	// 400 Bad Request
	HTTP_ERR_NOT_FOUND =			404,	// 404 Not Found
	HTTP_ERR_INTERNAL_SERVER_ERROR= 500,	// 500 Internal Server Error
	HTTP_ERR_NOT_IMPLEMENTED =		501		// 501 Not Implemented
};

//-----------------------------------------------------------------------------

namespace damn
{
	class HttpServer
	{
		class Connection;
		friend Connection;

	public:
							HttpServer( const BNetAddress &addr );
		virtual				~HttpServer();

				thread_id	Go( bool spawnthread=false );
	
	protected:
				void		Quit();
	
		static	long		_AcceptLoop( void *_this );
				void		AcceptLoop();
	
		virtual	bool		ConnectionRequested( const BNetAddress &address );
		virtual void		RequestReceived( Connection *connection ) =0;
	
				BString		GetStat() const;
	private:
	
		status_t				fInitStatus;
	
		BNetEndpoint			fEndpoint;
		
		volatile bool			fSelfDestruct;
		volatile bool			fQuitThread;
		thread_id				fThreadId;
	
		mutable BLocker			fConnectionsLock;
		damn::List<Connection>	fConnections;
	};
	
//-------------------------------------
	
	class HttpServer::Connection
	{
		friend HttpServer;
	public:
		struct Parameter { BString fName; BString fValue; };
	
							Connection( HttpServer *server, BNetEndpoint *endpoint );
	
		const	BString		&GetRequest() const { return fRequest; }
		const	BString		&GetCommand() const { return fCommand; }
		const	BString		&GetArg() const { return fArg; }
		const	BString		&GetPath() const { return fPath; }
		
				int			GetVersion() const { return fMajorVersion*65536 + fMinorVersion; }
	
				int			GetParameterCnt() const { return fParameters.CountItems(); }
		const	Parameter	&GetParameter( int index ) const { return *fParameters.ItemAt(index); }
	
		const	char		*GetClientHostName() const { return fClientHostName; }
				uint16		GetClientPort() const { return fClientPort; }
	
	
				void		SendData( const void *data, ssize_t size, const char *mimetype=NULL, const BString *extraheader=NULL );
	
				void		SendDataHeader( ssize_t size, const char *mimetype=NULL );
				void		SendDataBlock( const void *data, size_t size );
	
				void		SendError( int errorno, const char *string=NULL );

				void		SendRedirection( const char *newurl, bool permanent=false );

		static BString		EscapeString( const BString &original );
		static BString		DeescapeString( const BString &original );
		static BString		DeescapeString( const char *original, int length=-1 );
	
	private:
							~Connection();
	
				status_t	Go();
	
		static	long		_Loop( void *_this );
				void		Loop();
				
		status_t				fInitStatus;
	
		HttpServer				*fServer;
		NetStringEndpoint		fEndpoint;
	
		char 					fClientHostName[MAXHOSTNAMELEN];
		uint16					fClientPort;
	
		volatile bool			fSelfDestruct;
		volatile bool			fQuitThread;
		thread_id				fThreadId;
		
		bool					fKeepAlive;
	
		// command
		BString					fRequest;
		BString					fCommand;
		BString					fArg;
		BString					fPath;
		int						fMajorVersion;
		int						fMinorVersion;
		damn::List<Parameter>	fParameters;
		
		// SendData stats:
		bigtime_t				fStartTime;
		ssize_t					fCurrentPos;
		ssize_t					fTotSize;
	};
}

//-----------------------------------------------------------------------------
#endif

