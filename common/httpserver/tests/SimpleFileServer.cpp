// Copyright 1999 Jesper Hansen. Free for use under the Gnu Public License
//-----------------------------------------------------------------------------
#include <assert.h>
#include <stdio.h>
#include <deque>

#include <app/Application.h>
#include <net/NetDebug.h>
#include <net/NetEndpoint.h>
#include <storage/Path.h>
#include <storage/Directory.h>
#include <storage/Entry.h>
#include <storage/File.h>
#include <support/String.h>

#include "httpserver/HttpServer.h"
//-----------------------------------------------------------------------------

#define HTTPD_PORT1 80

//#define HTTPD_ROOT "/boot/home/public_html"
#define HTTPD_ROOT "."
#define HTTPD_INDEXFILE "index.html"

//-----------------------------------------------------------------------------

class TestServer : public damn::HttpServer
{
public:
	TestServer( const BNetAddress &addr );
	~TestServer();
	
private:
	status_t SendFile( Connection *connection, BFile &file );

	bool ConnectionRequested( const BNetAddress &address );
	void RequestReceived( Connection *connection );

	BDirectory fServerRoot;
};

//-------------------------------------

TestServer::TestServer( const BNetAddress &addr ) :
	damn::HttpServer( addr )
{
	fServerRoot.SetTo( HTTPD_ROOT );
	assert( fServerRoot.InitCheck() == B_NO_ERROR );
}

TestServer::~TestServer()
{
}

bool TestServer::ConnectionRequested( const BNetAddress &address )
{
	char clientname[256]; // FIXME: is there a max?
	uint16 clientport;
	address.GetAddr( clientname, &clientport );
	fprintf( stderr, "Connection requested from: %s:%d\n", clientname, (int)clientport );

	return true;
}

void TestServer::RequestReceived( Connection *connection )
{
	if( connection->GetCommand() == "GET" )
	{
		// Make sure that nobody tries to mess with us, and gets a file outside the HTTPD_ROOT directory.
		if( connection->GetArg().Length()==0 || connection->GetArg()[0]!='/' || connection->GetArg().FindFirst("/..")>=0 )
		{
			connection->SendError( HTTP_ERR_BAD_REQUEST, "Illegal file/path" );
		}
		else if( connection->GetArg() == "/stat.html" )
		{
			BString s = GetStat();
			connection->SendData( s.String(), s.Length(), "text/html" );
		}
		else
		{
			BFile file;

			BEntry entry;
			assert( fServerRoot.InitCheck() == B_NO_ERROR );
			if( connection->GetArg()=="/" )
				file.SetTo( &fServerRoot, HTTPD_INDEXFILE, B_READ_ONLY );
			else if( fServerRoot.FindEntry(connection->GetArg().String()+1,&entry,true)==B_NO_ERROR )
			{
				if( entry.IsFile() )
					file.SetTo( &entry, B_READ_ONLY );
				else if( entry.IsDirectory() )
				{
					BPath path( &fServerRoot, connection->GetArg().String()+1 );
					path.Append( HTTPD_INDEXFILE );
					printf( "%s\n", path.Path() );
					file.SetTo( path.Path(), B_READ_ONLY );
				}
			}
				
			if( !file.IsReadable() || SendFile(connection, file)!=B_NO_ERROR )
				connection->SendError( HTTP_ERR_NOT_FOUND, "Not Found" );
		}
	}
	else
		connection->SendError( HTTP_ERR_BAD_REQUEST, "Unknown command" );
}

status_t TestServer::SendFile( Connection *connection, BFile &file )
{
	status_t status;
	
	off_t size;
	status = file.GetSize( &size );
	if( status != B_NO_ERROR ) return status;
	
	void *buffer = malloc( size );
	if( !buffer ) return B_NO_MEMORY;

	ssize_t readsize = file.Read( buffer, size );
	if( readsize != size )
	{
		free( buffer );
		return B_ERROR;
	}

	char mimetype[256];
	ssize_t atrsize = file.ReadAttr( "BEOS:TYPE", B_MIME_TYPE, 0, mimetype, sizeof(mimetype) );
	
	connection->SendData( buffer, size, atrsize>0&&atrsize<255?mimetype:NULL );
	
	free( buffer );
	
	return B_NO_ERROR;
}

//-----------------------------------------------------------------------------

int main()
{
	BApplication app( "application/app.vnd-DamnRednex.DamnFileServer" );

	status_t status;
	
//	BNetDebug::Enable( true );

	BNetAddress addr( (const char*)NULL, HTTPD_PORT1 );
	TestServer *server = new TestServer( addr );
	thread_id servertid =  server->Go( true );

	wait_for_thread( servertid, &status );

	return 0;
}

//-----------------------------------------------------------------------------
