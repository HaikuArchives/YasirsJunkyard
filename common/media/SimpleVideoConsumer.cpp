/*
 * Copyright (c) 2000, Jesper Hansen. All rights reserved.
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
#include <app/Message.h>
#include <app/Messenger.h>
#include <interface/Bitmap.h>
#include <interface/TabView.h>
#include <interface/View.h>
#include <interface/Window.h>
#include <kernel/scheduler.h>
#include <media/Buffer.h>
#include <media/BufferGroup.h>
#include <media/MediaRoster.h>
#include <media/MediaTheme.h>
#include <media/TimeSource.h>
//-------------------------------------
#include "SimpleVideoConsumer.h"
//-----------------------------------------------------------------------------

damn::SimpleVideoConsumer::SimpleVideoConsumer( const char *name, BMediaAddOn *addon, const uint32 internal_id ) :
	BMediaNode( name ),
	BBufferConsumer( B_MEDIA_RAW_VIDEO )
{
	fAddOn = addon;
	fInternalID = internal_id;
	fConnected = false;

	fBuffers = NULL;
	for(int i=0; i<3; i++ )
	{
		fBitmap[i] = NULL;
		fMyBufferList[i] = NULL;
	}

	fCallback = NULL;
}

damn::SimpleVideoConsumer::~SimpleVideoConsumer()
{
}

//-----------------------------------------------------------------------------

void damn::SimpleVideoConsumer::NodeRegistered()
{
	media_format format;
	format.type = B_MEDIA_RAW_VIDEO;
	media_raw_video_format vid_format;
	vid_format.field_rate				= media_raw_video_format::wildcard.field_rate;
	vid_format.interlace				= 1;
	vid_format.first_active				= media_raw_video_format::wildcard.first_active;
	vid_format.last_active				= media_raw_video_format::wildcard.last_active;
	vid_format.orientation				= B_VIDEO_TOP_LEFT_RIGHT;
	vid_format.pixel_width_aspect		= media_raw_video_format::wildcard.pixel_width_aspect;
	vid_format.pixel_height_aspect		= media_raw_video_format::wildcard.pixel_height_aspect;
	vid_format.display.format			= B_RGB32;
	vid_format.display.line_width		= media_raw_video_format::wildcard.display.line_width;
	vid_format.display.line_count		= media_raw_video_format::wildcard.display.line_count;
	vid_format.display.bytes_per_row	= media_raw_video_format::wildcard.display.bytes_per_row;
	vid_format.display.pixel_offset		= 0;
	vid_format.display.line_offset		= 0;
	vid_format.display.flags			= 0;
	format.u.raw_video = vid_format; 

	fInputNode.destination.port = ControlPort();
	fInputNode.destination.id = 0;
	fInputNode.source = media_source::null;
	fInputNode.format = format;

	Run();
}

BMediaAddOn *damn::SimpleVideoConsumer::AddOn( long *cookie ) const
{
	// do the right thing if we're ever used with an add-on
	*cookie = fInternalID;
	return fAddOn;
}

void damn::SimpleVideoConsumer::Start( bigtime_t performance_time )
{
	BMediaEventLooper::Start(performance_time);
}

void damn::SimpleVideoConsumer::Stop( bigtime_t performance_time, bool immediate )
{
	BMediaEventLooper::Stop( performance_time, immediate );
}

void damn::SimpleVideoConsumer::Seek( bigtime_t media_time, bigtime_t performance_time )
{
	BMediaEventLooper::Seek(media_time, performance_time);
}

void damn::SimpleVideoConsumer::TimeWarp( bigtime_t at_real_time, bigtime_t to_performance_time )
{
	BMediaEventLooper::TimeWarp(at_real_time, to_performance_time);
}

status_t damn::SimpleVideoConsumer::RequestCompleted( const media_request_info &info )
{
	return B_OK;
}

status_t damn::SimpleVideoConsumer::HandleMessage( int32 message, const void *data, size_t size )
{
	return B_OK;
}

status_t damn::SimpleVideoConsumer::DeleteHook( BMediaNode *node )
{
	return BMediaEventLooper::DeleteHook(node);
}

//-----------------------------------------------------------------------------

#define JITTER 20000

void damn::SimpleVideoConsumer::HandleEvent( const media_timed_event *event, bigtime_t lateness, bool realTimeEvent )
{
	BBuffer *buffer;
	
	switch (event->type)
	{
		case BTimedEventQueue::B_START:
			break;

		case BTimedEventQueue::B_STOP:
			EventQueue()->FlushEvents(event->event_time, BTimedEventQueue::B_ALWAYS, true, BTimedEventQueue::B_HANDLE_BUFFER);
			break;

		case BTimedEventQueue::B_HANDLE_BUFFER:
			buffer = (BBuffer *) event->pointer;
			if( RunState()==B_STARTED && fConnected )
			{
				// see if this is one of our buffers
				uint32 index = 0;
				bool ourbuffers = true;
				while(index < 3)
				{
					if( buffer == fMyBufferList[index] )
						break;
					else
						index++;
				}
						
				if (index == 3)
				{
					// no, buffers belong to consumer
					ourbuffers = false;
					index = 0;
				}
				
//				printf( "Got bitmap %ld\n", index );

				if( (RunMode() == B_OFFLINE) ||
					 ((TimeSource()->Now() > (buffer->Header()->start_time - JITTER)) &&
					  (TimeSource()->Now() < (buffer->Header()->start_time + JITTER))) )
				{
					if( fCallback )
					{
						if ( !ourbuffers )
							memcpy( fBitmap[index]->Bits(), buffer->Data(), fBitmap[index]->BitsLength() );
						
						if( !fCallback(fBitmap[index], fCallbackUserData) )
							fCallback = NULL;
					}
				}
				buffer->Recycle();
			}
			else
				buffer->Recycle();
			break;

		default:
			break;
	}			
}

status_t damn::SimpleVideoConsumer::AcceptFormat( const media_destination &dest, media_format *format )
{
	if( dest != fInputNode.destination )
		return B_MEDIA_BAD_DESTINATION;	
	
	if( format->type == B_MEDIA_NO_TYPE )
		format->type = B_MEDIA_RAW_VIDEO;
	
	if( format->type != B_MEDIA_RAW_VIDEO )
		return B_MEDIA_BAD_FORMAT;

	if( format->u.raw_video.display.format != B_RGB32 &&
		format->u.raw_video.display.format != B_RGB16 &&
		format->u.raw_video.display.format != B_RGB15 &&			
		format->u.raw_video.display.format != B_GRAY8 &&			
		format->u.raw_video.display.format != media_raw_video_format::wildcard.display.format )
	{
		return B_MEDIA_BAD_FORMAT;
	}
		
	if (format->u.raw_video.display.format == media_raw_video_format::wildcard.display.format)
		format->u.raw_video.display.format = B_RGB16;

//	char format_string[256];		
//	string_for_format(*format, format_string, 256);
//	FUNCTION("VideoConsumer::AcceptFormat: %s\n", format_string);

	return B_OK;
}

status_t damn::SimpleVideoConsumer::GetNextInput( int32 *cookie, media_input *out_input )
{
	// custom build a destination for this connection
	// put connection number in id
	if( *cookie < 1 )
	{
		fInputNode.node = Node();
		fInputNode.destination.id = *cookie;
		sprintf( fInputNode.name, "Video Consumer" );
		*out_input = fInputNode;
		(*cookie)++;
		return B_OK;
	}
	else
	{
		return B_MEDIA_BAD_DESTINATION;
	}
}

void damn::SimpleVideoConsumer::DisposeInputCookie( int32 /*cookie*/ )
{
}

void damn::SimpleVideoConsumer::BufferReceived( BBuffer *buffer )
{
	if( RunState() == B_STOPPED )
	{
		buffer->Recycle();
		return;
	}

	media_timed_event event(
		buffer->Header()->start_time,
		BTimedEventQueue::B_HANDLE_BUFFER,
		buffer, BTimedEventQueue::B_RECYCLE_BUFFER );
	EventQueue()->AddEvent(event);
}

void damn::SimpleVideoConsumer::ProducerDataStatus( const media_destination &for_whom, int32 status, bigtime_t at_media_time )
{
	if( for_whom != fInputNode.destination )
		return;
}

status_t damn::SimpleVideoConsumer::GetLatencyFor( const media_destination &for_whom, bigtime_t *out_latency, media_node_id *out_timesource )
{
	if( for_whom != fInputNode.destination )
		return B_MEDIA_BAD_DESTINATION;
	
	*out_latency = 10000; // I have no idear...
	*out_timesource = TimeSource()->ID();
	return B_OK;
}

status_t damn::SimpleVideoConsumer::Connected( const media_source &producer, const media_destination &where, const media_format &with_format, media_input *out_input )
{
	fInputNode.source = producer;
	fInputNode.format = with_format;
	fInputNode.node = Node();
	sprintf( fInputNode.name, "Video Consumer" );
	*out_input = fInputNode;

	uint32 user_data = 0;
	int32 change_tag = 1;	
	if( CreateBuffers(with_format) == B_OK )
		BBufferConsumer::SetOutputBuffersFor( producer, fDestination, fBuffers, (void *)&user_data, &change_tag, true );
	else
		return B_ERROR;

//	mFtpBitmap = new BBitmap(BRect(0, 0, 320-1, 240-1), B_RGB32, false, false);
	fConnected = true;

	return B_OK;
}

void damn::SimpleVideoConsumer::Disconnected( const media_source &producer, const media_destination &where )
{
	if( where==fInputNode.destination && producer==fInputNode.source )
	{
		// disconnect the connection
		fInputNode.source = media_source::null;
//		delete mFtpBitmap;
		fConnected = false;
	}
}

status_t damn::SimpleVideoConsumer::FormatChanged(
	const media_source &producer,
	const media_destination &consumer, 
	int32 from_change_count,
	const media_format &format)
{
	if( consumer != fInputNode.destination )
		return B_MEDIA_BAD_DESTINATION;

	if( producer != fInputNode.source )
		return B_MEDIA_BAD_SOURCE;

	fInputNode.format = format;
	
	return CreateBuffers( format );
}

//---------------------------------------------------------------

status_t damn::SimpleVideoConsumer::CreateBuffers( const media_format &with_format )
{
	// delete any old buffers
	DeleteBuffers();	

	status_t status = B_OK;

	// create a buffer group
	uint32 mXSize = with_format.u.raw_video.display.line_width;
	uint32 mYSize = with_format.u.raw_video.display.line_count;	
//	uint32 mRowBytes = with_format.u.raw_video.display.bytes_per_row;
	color_space colorspace = with_format.u.raw_video.display.format;

	fBuffers = new BBufferGroup();
	status = fBuffers->InitCheck();
	if( status != B_OK )
		return status;

	// and attach the  bitmaps to the buffer group
	for( uint32 i=0; i<3; i++ )
	{
		fBitmap[i] = new BBitmap( BRect(0,0,(mXSize-1),(mYSize - 1)), colorspace, false, true );
		if( fBitmap[i]->IsValid() )
		{
			buffer_clone_info info;
			info.area = area_for( fBitmap[i]->Bits() );
			info.offset = 0;
			info.size = (size_t)fBitmap[i]->BitsLength();
			info.flags = i;
			info.buffer = 0;

			status = fBuffers->AddBuffer( info );
			if( status != B_OK )
				return status;
		}
		else 
			return B_ERROR;
	}
	
	BBuffer **buflist = new BBuffer*[3];
	for( int i=0; i<3; i++ )
		buflist[i] = NULL;
	
	status = fBuffers->GetBufferList( 3, buflist );
	if( status == B_OK )
	{
		for( int i=0; i<3; i++ )
		{
			if( buflist[i] != NULL )
				fMyBufferList[i] = buflist[i];
			else
				return B_ERROR;
		}
	}
		
	return status;
}


void damn::SimpleVideoConsumer::DeleteBuffers()
{
//	status_t status;

	if( fBuffers )
	{
		delete fBuffers;
		fBuffers = NULL;
		
		for( uint32 i=0; i<3; i++ )
		{
			if( fBitmap[i]->IsValid() )
			{
				delete fBitmap[i];
				fBitmap[i] = NULL;
			}
		}
	}
}

//---------------------------------------------------------------
//---------------------------------------------------------------
//---------------------------------------------------------------

damn::VideoCapture::VideoCapture()
{
	media_node fTimeSourceNode;
	port_id fPort;

	status_t status;
	
	fCallback = NULL;
	fVideoControlWindow = NULL;

	// find the media roster
	fMediaRoster = BMediaRoster::Roster( &status );
	if( status != B_OK )
	{
		fprintf( stderr, "can't find the media roster: %s\n", strerror(status) );
		fInitStatus = status;
		return;
	}	

	// find the time source
	status = fMediaRoster->GetTimeSource( &fTimeSourceNode );
	if( status != B_OK )
	{
		fprintf( stderr, "can't get a time source: %s\n", strerror(status) );
		fInitStatus = status;
		return;
	}

	// find a video producer node
	status = fMediaRoster->GetVideoInput( &fProducerNode );
	if( status != B_OK )
	{
		fprintf( stderr, "can't find a video input: %s\n", strerror(status) );
		fInitStatus = status;
		return;
	}

	// create the video consumer node
	fConsumer = new damn::SimpleVideoConsumer( "SimpleVideoConsumer", NULL, 0 );
	if( !fConsumer )
	{
		fprintf( stderr, "can't create the consumer\n" );
		fInitStatus =  B_ERROR;
	}
	
	// register the node
	status = fMediaRoster->RegisterNode( fConsumer );
	if( status != B_OK )
	{
		fprintf( stderr, "can't register the consumer: %s\n", strerror(status) );
		fInitStatus = status;
		return;
	}
	fPort = fConsumer->ControlPort();
	
	// find free producer output
	int32 cnt = 0;
	status = fMediaRoster->GetFreeOutputsFor( fProducerNode, &fProducerOut, 1,  &cnt, B_MEDIA_RAW_VIDEO );
	if( status!=B_OK || cnt<1 )
	{
		if( status==B_OK ) status = B_RESOURCE_UNAVAILABLE;
		fprintf( stderr, "can't find an available video stream: %s\n", strerror(status) );
		fInitStatus = status;
		return;
	}

	// find free consumer input
	cnt = 0;
	status = fMediaRoster->GetFreeInputsFor( fConsumer->Node(), &fConsumerIn, 1, &cnt, B_MEDIA_RAW_VIDEO );
	if( status!=B_OK || cnt<1 )
	{
		if( status==B_OK ) status = B_RESOURCE_UNAVAILABLE;
		fprintf( stderr, "can't find an available connection to the consumer: %s\n", strerror(status) );
		fInitStatus = status;
		return;
	}

	// Connect The Nodes
	media_format format;
	format.type = B_MEDIA_RAW_VIDEO;
	media_raw_video_format vid_format;
	vid_format.field_rate				= media_raw_video_format::wildcard.field_rate;
	vid_format.interlace				= 1;
	vid_format.first_active				= media_raw_video_format::wildcard.first_active;
	vid_format.last_active				= media_raw_video_format::wildcard.last_active;
	vid_format.orientation				= B_VIDEO_TOP_LEFT_RIGHT;
	vid_format.pixel_width_aspect		= media_raw_video_format::wildcard.pixel_width_aspect;
	vid_format.pixel_height_aspect		= media_raw_video_format::wildcard.pixel_height_aspect;
	vid_format.display.format			= B_RGB32;
	vid_format.display.line_width		= media_raw_video_format::wildcard.display.line_width;
	vid_format.display.line_count		= media_raw_video_format::wildcard.display.line_count;
	vid_format.display.bytes_per_row	= media_raw_video_format::wildcard.display.bytes_per_row;
	vid_format.display.pixel_offset		= 0;
	vid_format.display.line_offset		= 0;
	vid_format.display.flags			= 0;
	format.u.raw_video = vid_format; 
	
	// connect producer to consumer
	status = fMediaRoster->Connect( fProducerOut.source, fConsumerIn.destination, &format, &fProducerOut, &fConsumerIn );
	if( status != B_OK )
	{
		fprintf( stderr, "can't connect the video source to the consumer: %s\n", strerror(status) );
		fInitStatus = status;
		return;
	}
	
	// set time sources
	status = fMediaRoster->SetTimeSourceFor( fProducerNode.node, fTimeSourceNode.node );
	if( status != B_OK )
	{
		fprintf( stderr, "can't set the timesource for the consumer: %s\n", strerror(status) );
		fInitStatus = status;
		return;
	}
	
	status = fMediaRoster->SetTimeSourceFor( fConsumer->ID(), fTimeSourceNode.node );
	if( status != B_OK )
	{
		fprintf( stderr, "can't set the timesource for the consumer: %s\n", strerror(status) );
		fInitStatus = status;
		return;
	}
	
	// figure out what recording delay to use
	bigtime_t latency = 0;
	status = fMediaRoster->GetLatencyFor( fProducerNode, &latency );
	status = fMediaRoster->SetProducerRunModeDelay( fProducerNode, latency );

	// start the nodes
	bigtime_t initLatency = 0;
	status = fMediaRoster->GetInitialLatencyFor( fProducerNode, &initLatency );
	if( status < B_OK )
	{
		fprintf( stderr, "error getting initial latency for fCaptureNode: %s\n", strerror(status) );
	}
	initLatency += estimate_max_scheduling_latency();
	
	BTimeSource *timeSource = fMediaRoster->MakeTimeSourceFor(fProducerNode);
	bool running = timeSource->IsRunning();
	
	// workaround for people without sound cards
	// because the system time source won't be running
	bigtime_t real = BTimeSource::RealTime();
	if( !running )
	{
		status = fMediaRoster->StartTimeSource( fTimeSourceNode, real );
		if( status != B_OK )
		{
			timeSource->Release();
			fprintf( stderr, "cannot start time source: %s\n", strerror(status) );
			fInitStatus = status;
			return;
		}

		status = fMediaRoster->SeekTimeSource( fTimeSourceNode, 0, real );
		if( status != B_OK )
		{
			timeSource->Release();
			fprintf( stderr, "cannot seek time source: %s\n", strerror(status) );
			fInitStatus = status;
			return;
		}
	}

	bigtime_t perf = timeSource->PerformanceTimeFor( real + latency + initLatency );
	timeSource->Release();
	
	// start the nodes
	status = fMediaRoster->StartNode( fProducerNode, perf );
	if( status != B_OK )
	{
		fprintf( stderr, "can't start the video source: %s\n", strerror(status) );
		fInitStatus = status;
		return;
	}

	status = fMediaRoster->StartNode( fConsumer->Node(), perf );
	if( status != B_OK )
	{
		fprintf( stderr, "can't start the consumer: %s\n", strerror(status) );
		fInitStatus = status;
		return;
	}

	fConsumer->SetCallback( _NewBitmap, this );
	
	fInitStatus = B_OK;
}

damn::VideoCapture::~VideoCapture()
{
	if( !fMediaRoster )
		return;
	
	if( fConsumer )
	{
		fMediaRoster->StopNode( fConsumer->Node(), 0, true );
	
		fMediaRoster->Disconnect( fProducerOut.node.node, fProducerOut.source, fConsumerIn.node.node, fConsumerIn.destination );

		if( fProducerNode != media_node::null )
		{
			fMediaRoster->ReleaseNode( fProducerNode );
			fProducerNode = media_node::null;
		}
		fMediaRoster->ReleaseNode( fConsumer->Node() );
	}
}

bool damn::VideoCapture::_NewBitmap( const BBitmap *bitmap, void *userdata )
{
	damn::VideoCapture *_this = (damn::VideoCapture*)userdata;
	_this->NewBitmap( bitmap );
	return true;
}

status_t damn::VideoCapture::InitCheck() const
{
	return fInitStatus;
}

		
void damn::VideoCapture::NewBitmap( const BBitmap *bitmap )
{
	if( fCallback )
	{
		if( !fCallback(bitmap, fCallbackUserData) )
			fCallback = NULL;
	}
}

//---------------------------------------------------------------

#define VIDEO_SIZE_X 320
#define VIDEO_SIZE_Y 240

#define WINDOW_SIZE_X (VIDEO_SIZE_X + 80)
#define WINDOW_SIZE_Y (VIDEO_SIZE_Y + 230)

#define WINDOW_OFFSET_X 28
#define WINDOW_OFFSET_Y 28

class ControlWindow : public BWindow {

public:
		ControlWindow(const BRect & frame, BView * controls, media_node node);
		void MessageReceived(BMessage * message);
		bool QuitRequested();

//		void Hide() { Show(); }

private:
		BView *				fView;
		media_node			fNode;
};

ControlWindow::ControlWindow(
	const BRect & frame,
	BView * controls,
	media_node node) :
	BWindow(frame, "Video Preferences", B_TITLED_WINDOW, B_ASYNCHRONOUS_CONTROLS)
{
	fView = controls;
	fNode = node;
		
	AddChild(fView);
}

//---------------------------------------------------------------

void
ControlWindow::MessageReceived(BMessage * message) 
{
	BParameterWeb * web = NULL;
//	BView * panel = NULL;
	status_t err;
	
	switch (message->what)
	{
		case B_MEDIA_WEB_CHANGED:
		{
			// If this is a tab view, find out which tab 
			// is selected
			BTabView *tabView = dynamic_cast<BTabView*>(fView);
			int32 tabNum = -1;
			if (tabView)
				tabNum = tabView->Selection();

			RemoveChild(fView);
			delete fView;
			
			err = BMediaRoster::Roster()->GetParameterWebFor(fNode, &web);
			
			if ((err >= B_OK) &&
				(web != NULL))
			{
				fView = BMediaTheme::ViewFor(web);
				AddChild(fView);

				// Another tab view?  Restore previous selection
				if (tabNum > 0)
				{
					BTabView *newTabView = dynamic_cast<BTabView*>(fView);	
					if (newTabView)
						newTabView->Select(tabNum);
				}
			}
			break;
		}
		default:
			BWindow::MessageReceived(message);
	}
}

//---------------------------------------------------------------

bool 
ControlWindow::QuitRequested()
{
//	Hide();
//	be_app->PostMessage(msg_control_win);
	return true;
}

void damn::VideoCapture::ShowParameterWeb()
{
	if( fVideoControlWindow )
	{
		fVideoControlWindow->Activate();
		return;
	}

	BParameterWeb *web = NULL;
	BView *view = NULL;
	media_node node = fProducerNode;
	status_t err = fMediaRoster->GetParameterWebFor( node, &web );
	if( (err>=B_OK) && (web!=NULL) )
	{
		view = BMediaTheme::ViewFor( web );
		fVideoControlWindow = new ControlWindow(
			BRect(2*WINDOW_OFFSET_X + WINDOW_SIZE_X, WINDOW_OFFSET_Y,
				2*WINDOW_OFFSET_X + WINDOW_SIZE_X + view->Bounds().right, WINDOW_OFFSET_Y + view->Bounds().bottom),
			view, node);
		fMediaRoster->StartWatching( BMessenger(NULL, fVideoControlWindow), node, B_MEDIA_WEB_CHANGED );
		fVideoControlWindow->Show();
	}
}

