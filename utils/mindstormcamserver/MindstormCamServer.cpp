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
#include <zlib.h>
//-------------------------------------
#include <app/Application.h>
#include <interface/Bitmap.h>
#include <interface/View.h>
#include <interface/Window.h>
#include <storage/File.h>
#include <storage/Mime.h>
#include <translation/BitmapStream.h>
#include <translation/TranslatorRoster.h>
//-------------------------------------
#include "gfx/BitmapScale.h"
#include "httpserver/HttpServer.h"
#include "libcpiacam/CPiACam.h"
#include "mindstorm/RcxComm.h"
#include "text/Bitmap2Text.h"
//-----------------------------------------------------------------------------

#define SAVEFRAMES

#ifdef SAVEFRAMES
int saveinterval = 60;
const char *savepath = "/boot/home/TechCam";
//const char *linkfile = "/boot/home/public_html/webcam.jpg";
#endif

//-------------------------------------

#define HTTPPORT 80

//-------------------------------------

#define SERIAL_PORT "/dev/ports/serial2"

//-------------------------------------

class CamServer;

//-----------------------------------------------------------------------------

class BitmapView : public BView
{
public:
	BitmapView( BRect rect, uint32 resizeMask );

	void DrawNewBitmap( const BBitmap *bitmap );
	void Draw( BRect rect );

private:
	const BBitmap *fBitmap;
};

//-------------------------------------

class VideoWindow : public BWindow
{
public:
	VideoWindow( BRect rect, const char *title );
	
	bool QuitRequested();
	
	void DrawBitmap( const BBitmap *bitmap );
//	void Redraw();

private:
	BitmapView	*fBitmapView;
};

//-------------------------------------

class CamApp : public BApplication
{
public:
	CamApp();
	~CamApp();

	void ReadyToRun();
	const BBitmap *GetBitmap() const { return fBitmap; }

	float GetRcxBatteryLevel();
	void SendRcxMessage( uint8 msg ) { fRCX->SendMessage(msg); }

private:
	static long _WorkThread( void *data );
	void WorkThread();
	
	CPiACam			*fCam;
	
	thread_id		fWorkThread;
	volatile bool	fWorkQuit;

	VideoWindow 	*fVideoWindow;
	BBitmap			*fBitmap;

	CamServer		*fServer;

	damn::RcxComm	*fRCX;
	float			fRCXBatteryLevel;
	bigtime_t		fLastRCXBatteryLevel;
};

//-------------------------------------

class CamServer : public damn::HttpServer
{
public:
	CamServer();
	~CamServer();
	
private:
	bool ConnectionRequested( const BNetAddress &address );
	void RequestReceived( Connection *connection );
};

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

BitmapView::BitmapView( BRect rect, uint32 resizemask ) :
	BView( rect, "Bitmap view", resizemask, B_WILL_DRAW|B_FULL_UPDATE_ON_RESIZE )
{
	SetViewColor( B_TRANSPARENT_COLOR );
	fBitmap = NULL;
}

void BitmapView::DrawNewBitmap( const BBitmap *bitmap )
{
	fBitmap = bitmap;

	if( fBitmap )
		Invalidate();
}

void BitmapView::Draw( BRect rect )
{
	if( fBitmap )
		DrawBitmap( fBitmap, fBitmap->Bounds(), Frame() );
	else
		FillRect( rect );
}

//-----------------------------------------------------------------------------

VideoWindow::VideoWindow( BRect rect, const char *title ) :
	BWindow( rect, title, B_TITLED_WINDOW_LOOK, B_NORMAL_WINDOW_FEEL, 0 )
{
	rect.OffsetTo( 0, 0 );
	fBitmapView = new BitmapView( rect, B_FOLLOW_ALL );
	AddChild( fBitmapView );

	Show();
}

bool VideoWindow::QuitRequested()
{
	be_app->PostMessage( B_QUIT_REQUESTED );
	return true;
}

void VideoWindow::DrawBitmap( const BBitmap *bitmap )
{
	fBitmapView->DrawNewBitmap( bitmap );
}

//-----------------------------------------------------------------------------

CamApp::CamApp() :
	BApplication( "application/x-vnd.DamnRednex-MindstormCamServer" )
{
	fCam = NULL;
	fWorkQuit = false;
	fWorkThread = -1;

	fVideoWindow = NULL;
	fBitmap = NULL;
	fServer = NULL;
	
	fRCXBatteryLevel = 0.0f;
	fLastRCXBatteryLevel = 0;
	
	
	fRCX = NULL;
}

CamApp::~CamApp()
{
	if( fWorkThread >= 0 )
	{
		fWorkQuit = true;
		printf( "Waiting for worker thread to quit...\n" );
		status_t retval;
		wait_for_thread( fWorkThread, &retval );
	}

	if( fVideoWindow->Lock() )
		delete fVideoWindow;

	delete fServer;

	delete fRCX;

	delete fBitmap;
	
	if( fCam && fCam->Lock() )
		delete fCam;
}

void CamApp::ReadyToRun()
{
	fVideoWindow = new VideoWindow( BRect(100,100,100+352-1,100+288-1), "Video" );
	fBitmap = new BBitmap( BRect(0,0,352-1,288-1), B_BITMAP_ACCEPTS_VIEWS, B_RGB32 );
	BView *view = new BView( fBitmap->Bounds(), "", B_FOLLOW_NONE, B_WILL_DRAW );
	BFont font;
	font.SetFamilyAndStyle( "ProFontISOLatin1", "Regular" );
	font.SetSize( 9 );
	view->SetFont( &font );
	fBitmap->AddChild( view );
	

	fCam = new CPiACam( "/dev/video/cpia/ppc2_0" );
	if( fCam==NULL || (fCam->InitCheck()<B_NO_ERROR) )
	{
		fprintf( stderr, "Could not open camera device\n" );
		PostMessage( B_QUIT_REQUESTED );
		return;
	}	

	fCam->Lock();
	printf( "CPiA PPC2; Firmware version:%d.%d VideoCompressor version:%d.%d\n", 
		fCam->GetFirmwareVersion(), fCam->GetFirmwareRevision(),
		fCam->GetVideoCompressorVersion(), fCam->GetVideoCompressorRevision() );
	fCam->EnableAutoExposure( true );
	fCam->SetFrameRate( CPiACam::FRATE_25, CPiACam::FDIV_1 );

#if 0
	fCam->EnableCompression( CPiACam::COMPRESSION_DISABLED, CPiACam::DECIMATION_OFF );
#else
	fCam->EnableCompression( CPiACam::COMPRESSION_AUTO, CPiACam::DECIMATION_ON );
	fCam->SetCompressionTarget( CPiACam::COMPTARGET_QUALITY, 10, 1.0f );
//	fCam->SetYUVTreshold( float ythreshold, float uvthreshold );
#endif

	fCam->Unlock();


	fRCX = new damn::RcxComm( SERIAL_PORT );
	if( fRCX->InitCheck() != B_OK )
	{
		fprintf( stderr, "Could not open serial port\n" );
		PostMessage( B_QUIT_REQUESTED );
		return;
	}
	int32 rcxbat = fRCX->GetBatteryVoltage();
	printf( "RCX Batery level: %ld.%03ld\n", rcxbat/1000, rcxbat%1000 );

	fWorkQuit = false;
	fWorkThread = spawn_thread( _WorkThread, "Video Worker", B_NORMAL_PRIORITY, this );
	resume_thread( fWorkThread );

	fServer = new CamServer();
	fServer->Go( true );
}

long CamApp::_WorkThread( void *data )
{
	CamApp *app = (CamApp*)data;
	app->WorkThread();
	return 0;
}

void CamApp::WorkThread()
{
//	thread_id thisthread = find_thread( NULL );

#ifdef SAVEFRAMES
	bigtime_t lastsave = system_time() - saveinterval*1000000;
#endif

	bool delta = false;

	while( !fWorkQuit )
	{
//		if( Lock() )
		{
			fCam->Lock();
			fCam->CaptureFrame( fBitmap, delta );
			fCam->Unlock();
			if( fBitmap->Lock() )
			{
#if 1
				BView *view = fBitmap->ChildAt( 0 );
				time_t t=time(NULL);

				struct tm *t2 = localtime( &t );
				char string[32];
				strftime( string, sizeof(string), "%a %d %b %H:%M:%S", t2 );

				int strwidth = (int)view->StringWidth( string );
				font_height fontheight;
				view->GetFontHeight( &fontheight );
				int strheight = (int)(fontheight.ascent + fontheight.descent);
				
				view->SetHighColor( 0,0,0 );
//				BRect b = view->Bounds();
				view->FillRect( BRect(351-strwidth,287-strheight,351,287) );
			
//				view->SetDrawingMode( B_OP_ALPHA );
				view->SetHighColor( 255,255,255 );
				view->SetLowColor( 0,0,0 );
				fBitmap->ChildAt(0)->DrawString( string, BPoint(351-strwidth,287-strheight+fontheight.ascent) );

				view->Sync();
#endif
				fBitmap->Unlock();
			}
			delta = true;

			if( fVideoWindow->Lock() )
			{
				fVideoWindow->DrawBitmap( fBitmap );
				fVideoWindow->Unlock();
			}

#ifdef SAVEFRAMES
			bigtime_t now = system_time();
			if( now > lastsave+saveinterval*1000000 )
			{
				time_t timet;
				time( &timet );
				struct tm timetm;
				localtime_r( &timet, &timetm );
			
				char filename[1024];
				sprintf( filename, "%s/cam_%02d%02d%02d_%02d%02d%02d.jpg", savepath,
					timetm.tm_year%100, timetm.tm_mon+1, timetm.tm_mday,
					timetm.tm_hour, timetm.tm_min, timetm.tm_sec );
				printf( "<%s>\n", filename );
	
				BTranslatorRoster *roster = BTranslatorRoster::Default();
				BBitmapStream stream( fBitmap );
				BFile file( filename, B_CREATE_FILE | B_WRITE_ONLY );
				roster->Translate( &stream, NULL, NULL, &file, 'JPEG' );

				// we already know the mime type, but this is easier :)
				update_mime_info( filename, false, false, false );

				BBitmap *bitmap2;
				stream.DetachBitmap( &bitmap2 ); // get out bitmap back

//				unlink( linkfile );
//				symlink( filename, linkfile );

				lastsave += saveinterval*1000000;
			}
#endif
		}
	}
	fWorkQuit = false;
}

float CamApp::GetRcxBatteryLevel()
{
	bigtime_t now = system_time();
	if( now > fLastRCXBatteryLevel+60*1000000 )
	{
		fLastRCXBatteryLevel = now;
		int32 rcxbat = fRCX->GetBatteryVoltage();
		fRCXBatteryLevel = float(rcxbat) / 1000.0f;
	}
	return fRCXBatteryLevel;
}		

//-----------------------------------------------------------------------------

CamServer::CamServer() :
	damn::HttpServer( BNetAddress((const char*)NULL,HTTPPORT) )
{
}

CamServer::~CamServer()
{
}

bool CamServer::ConnectionRequested( const BNetAddress &address )
{
	char clientname[256]; // FIXME: is there a max?
	uint16 clientport;
	address.GetAddr( clientname, &clientport );
	fprintf( stderr, "Connection requested from: %s:%d\n", clientname, (int)clientport );

	return true;
}

void CamServer::RequestReceived( Connection *connection )
{
	if( connection->GetCommand() == "GET" )
	{
		int text_width, text_height, text_compresslevel=0;
		char text_mode[32] = "";
		
		if( connection->GetArg().FindFirst("?pan=") >= 0 )
		{
			CamApp *app = (CamApp*)be_app;
			if( app->Lock() )
			{
				if( connection->GetArg().FindFirst("?pan=left") >= 0 )
//					app->SendRcxMessage( 'L' );
					app->SendRcxMessage( 'U' );
				else if( connection->GetArg().FindFirst("?pan=right") >= 0 )
//					app->SendRcxMessage( 'R' );
					app->SendRcxMessage( 'D' );
				else if( connection->GetArg().FindFirst("?pan=up") >= 0 )
//					app->SendRcxMessage( 'U' );
					app->SendRcxMessage( 'L' );
				else if( connection->GetArg().FindFirst("?pan=down") >= 0 )
//					app->SendRcxMessage( 'D' );
					app->SendRcxMessage( 'R' );
				app->Unlock();
			}
					
			snooze( 1000000 ); // FIXME: wait for RCX reply instead...
			connection->SendRedirection( "./", false );
		}
//		if( connection->GetArg()=="/" || connection->GetArg()=="/index.html" )
//		else if( connection->GetArg() == "/text.html" )
		else if( sscanf(connection->GetArg().String(), "/%8[^_]_%dx%d_%d.html",text_mode,&text_width,&text_height,&text_compresslevel) >= 3 )
		{
			if( text_width<4 || text_width>352 || text_height<4 || text_height>240 )
			{
				text_width = 60;
				text_height = 30;
			}
			
			int refresh_time = (text_width*text_height)/1000;
			if( refresh_time < 1 )
				refresh_time = 1;

			if( text_width==40 && text_height==20 )
				refresh_time = 0;
			
			if( text_compresslevel<0 || text_compresslevel>9 )
				text_compresslevel = 0;
			
			bool color = true;
			if( strcmp(text_mode,"bwtext") == 0 )
				color = false;
	

			CamApp *app = (CamApp*)be_app;
			if( app->Lock() )
			{
				BBitmap bitmap( BRect(0,0,text_width-1,text_height-1), B_RGB32 );
				damn::Scale( app->GetBitmap(), &bitmap, damn::filter_lanczos3 );
				app->Unlock();
				std::vector<std::vector<damn::colchar_t> > text = damn::Bitmap2Text( &bitmap );
	
				BString html;
				html << "<html>\n";
				html << "<head>\n";
				html << "<meta http-equiv=\"refresh\" content=\"" << refresh_time << "\">\n";
				html << "<meta http-equiv=\"pragma\" content=\"no-cache\">\n";
				html << "<meta http-equiv=\"cache-control\" content=\"no-cache\">\n";
				html << "<title>WebCam</title>\n";
				html << "</head>\n";
				html << "<body bgcolor=#000000>\n";
				html << "<center>\n";

				html << "<pre><font size=-2 color=#ffffff>";
				uint32 lastcol = 0xffffffff;
				for( int iy=0; iy<text_height; iy++ )
				{
					for( int ix=0; ix<text_width; ix++ )
					{
						damn::colchar_t cc = text[iy][ix];
						if( color )
						{
							cc.rgbcol.red = (int)(pow(float(cc.rgbcol.red)/256.0f,0.75f)*256.0f);
							cc.rgbcol.green = (int)(pow(float(cc.rgbcol.green)/256.0f,0.75f)*256.0f);
							cc.rgbcol.blue = (int)(pow(float(cc.rgbcol.blue)/256.0f,0.75f)*256.0f);
							cc.rgbcol.red &= 0xf0; cc.rgbcol.red |= cc.rgbcol.red>>4;
							cc.rgbcol.green &= 0xf0; cc.rgbcol.green |= cc.rgbcol.green>>4;
							cc.rgbcol.blue &= 0xf0; cc.rgbcol.blue |= cc.rgbcol.blue>>4;
							uint32 col = cc.rgbcol.red<<16 | cc.rgbcol.green<<8 | cc.rgbcol.blue;
							if( col != lastcol )
							{
								if( lastcol != 0xffffffff )
									html << "</font>";
								char hex[7];
								sprintf( hex, "%02X%02X%02X", cc.rgbcol.red, cc.rgbcol.green, cc.rgbcol.blue );
								html << "<font color=#" << hex << ">";
								lastcol = col;
							}
						}
						html << cc.c;
					}
					html << "\n";
				}
				if( color )
					html << "</font>";
				html << "</font></pre>";

				html << "</center>\n";
				html << "</body>\n";
				html << "</html>\n";

				if( text_compresslevel )
				{
#if 1
					uLongf compressed_len = 8+(html.Length()*101/100+12)+4;
					uint8 *compressed = new uint8[compressed_len];
					if( compress2((Bytef*)compressed+8,&compressed_len, (Bytef*)html.String(), html.Length(), text_compresslevel) == Z_OK )
					{
						compressed_len -= 4;

						compressed[0] = 0x1f; // magic
						compressed[1] = 0x8b; // magic
						compressed[2] = 8; // deflate
						compressed[3] = 0x00; // text file
						compressed[4] = compressed[5] = compressed[6] = compressed[7] = 0; // modify time
						compressed[8] = 0x00; // xflags
						compressed[9] = 0xff; // oscode
						
						uLong crc = crc32( 0L, Z_NULL, 0 );
						crc = crc32( crc, (Bytef*)html.String(), html.Length() );

						compressed[8+compressed_len+0] = (crc>>0)&0xff;
						compressed[8+compressed_len+1] = (crc>>8)&0xff;
						compressed[8+compressed_len+2] = (crc>>16)&0xff;
						compressed[8+compressed_len+3] = (crc>>24)&0xff;
						compressed[8+compressed_len+4] = (html.Length()>>0)&0xff;
						compressed[8+compressed_len+5] = (html.Length()>>8)&0xff;
						compressed[8+compressed_len+6] = (html.Length()>>16)&0xff;
						compressed[8+compressed_len+7] = (html.Length()>>24)&0xff;
					
						BString xheader = "Content-encoding: gzip\n";
						connection->SendData( compressed, 8+compressed_len+8, "text/html", &xheader );
						printf( "Sending compressed %ld -> %ld\n", html.Length(), 8+compressed_len+8 );
#else
#endif
					}
					else
						connection->SendData( html.String(), html.Length(), "text/html" );
					delete compressed;
				}
				else
				{
					connection->SendData( html.String(), html.Length(), "text/html" );
				}
			}

		}
		else if( connection->GetArg()=="/" || connection->GetArg().FindFirst(".html") >= 0 )
		{
			float battery = 0.0f;
			CamApp *app = (CamApp*)be_app;
			if( app->Lock() )
			{
				battery = app->GetRcxBatteryLevel();
				app->Unlock();
			}
		
			BString html;
			html << "<html>\n";
			html << "<head>\n";
			html << "<meta http-equiv=\"refresh\" content=\"10\">\n";
			html << "<meta http-equiv=\"pragma\" content=\"no-cache\">\n";
			html << "<meta http-equiv=\"cache-control\" content=\"no-cache\">\n";
			html << "<title>WebCam</title>\n";
			html << "</head>\n";
			html << "<body>\n";
			
			html << "<hr>\n";
			
			html << "<center>\n";
			html << "<img src=webcam_" << system_time() << ".jpg>\n";
			html << "</center>\n";

			html << "<hr>\n";
			
			html << "<center width=5%>\n";
			html << "<table cols=3 rows=3 border=0 cellspacing=0 cellpadding=0>\n";
			html <<	"<tr>\n";
			html <<	"<td></td>\n";
			html <<	"<td align=center valign=center><form><input type=\"submit\" name=\"pan\" value=\"up\"></form></td>\n";
			html <<	"<td></td>\n";
			html <<	"</tr>\n";
			html <<	"<tr valign=center>\n";
			html <<	"<td align=center valign=center><form><input type=\"submit\" name=\"pan\" value=\"left\"></form></td>\n";
			html <<	"<td align=center valign=center>pan</td>\n";
			html <<	"<td align=center valign=center><form><input type=\"submit\" name=\"pan\" value=\"right\"></form></td>\n";
			html <<	"</tr>\n";
			html <<	"<tr>\n";
			html <<	"<td></td>\n";
			html <<	"<td align=center valign=center><form><input type=\"submit\" name=\"pan\" value=\"down\"></form></td>\n";
			html <<	"<td></td>\n";
			html <<	"</tr>\n";
			html << "</table>\n";
			html << "</center>\n";

			html << "<hr>\n";
		
			html << "<b>Note: the camera-controll unit is currently out of order...</b>";

			html << "<hr>\n";

			html << "RCX Battery: " << battery << "v<br>\n";
			html << "NQC <a href=camctrl.nqc>Source</a> for the RCX.\n<br>";
			html << "Text version: <a href=text_60x30.html>60x30</a> <a href=text_78x39.html>78x39</a> <a href=text_120x60.html>120x60</a> <a href=text_176x88.html>176x88</a> <a href=text_352x176.html>352x176</a>\n<br>";
			html << "Compressed Text version: <a href=text_60x30_5.html>60x30</a> <a href=text_78x39_5.html>78x39</a> <a href=text_120x60_5.html>120x60</a> <a href=text_176x88_5.html>176x88</a> <a href=text_352x176_5.html>352x176</a>\n<br>";
			html << "BW Text version: <a href=bwtext_60x30.html>60x30</a> <a href=bwtext_78x39.html>78x39</a> <a href=bwtext_120x60.html>120x60</a> <a href=bwtext_176x88.html>176x88</a> <a href=bwtext_352x176.html>352x176</a>\n<br>";
			html << "Compressed BW Text version: <a href=bwtext_60x30_5.html>60x30</a> <a href=bwtext_78x39_5.html>78x39</a> <a href=bwtext_120x60_5.html>120x60</a> <a href=bwtext_176x88_5.html>176x88</a> <a href=bwtext_352x176_5.html>352x176</a>\n";

			html << "<hr>\n";

			html << "The sources for this: <a href=http://sourceforge.net/project/?group_id=554>BeCPiA WebCam driver</a> <a href=http://sourceforge.net/project/?group_id=724>Junkyard</a>.\n";

			html << "<hr>\n";

			html << "</body>\n";
			html << "</html>\n";
			connection->SendData( html.String(), html.Length(), "text/html" );
		}
		else if( connection->GetArg().FindFirst(".jpg") >= 0 )
		{
			CamApp *app = (CamApp*)be_app;
			if( app->Lock() )
			{
				BBitmap *bitmap = new BBitmap( app->GetBitmap() );
				app->Unlock();
	
				BTranslatorRoster *roster = BTranslatorRoster::Default();
				BBitmapStream stream( bitmap );
				BMallocIO bitmapfile;
				roster->Translate( &stream, NULL, NULL, &bitmapfile, 'JPEG' );
	
				connection->SendData( bitmapfile.Buffer(), bitmapfile.BufferLength(), "image/jpeg" );
			}
		}
		else if( connection->GetArg().FindFirst(".nqc") >= 0 )
		{
			BFile file( "camctrl.nqc", B_READ_ONLY );
			if( file.InitCheck() >= B_OK )
			{
				off_t filesize = 0;
				file.GetSize( &filesize );
				
				char *filedata = new char[filesize];
				memset( filedata, 0, filesize );
				file.Read( filedata, filesize );
				
				connection->SendData( filedata, filesize, "text/plain" );
				
				delete filedata;
			}
		}
		else
			connection->SendError( HTTP_ERR_NOT_FOUND, "no such file" );
	}
	else
		connection->SendError( HTTP_ERR_BAD_REQUEST, "Unknown command" );
}

//-----------------------------------------------------------------------------

int main()
{
	CamApp app;

	app.Run();

	return 0;
}

//-----------------------------------------------------------------------------


