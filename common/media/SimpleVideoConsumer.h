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

// Basicly CodyCam's VideoConsumer class (minus the ftp stuff)

//-----------------------------------------------------------------------------
#ifndef DAMN_SIMPLEVIDEOCONSUMER_H
#define DAMN_SIMPLEVIDEOCONSUMER_H
//-----------------------------------------------------------------------------
//-------------------------------------
#include <media/BufferConsumer.h>
#include <media/MediaEventLooper.h>
//-------------------------------------
//-----------------------------------------------------------------------------

namespace damn
{
	class SimpleVideoConsumer :  public BMediaEventLooper, public BBufferConsumer
	{
	public:
		SimpleVideoConsumer( const char *name, BMediaAddOn *addon, const uint32 internal_id );
		~SimpleVideoConsumer();
	
		//	BMediaNode
	public:
		virtual	BMediaAddOn	*AddOn(long *cookie) const;
	protected:
		void Start( bigtime_t performance_time );
		void Stop( bigtime_t performance_time, bool immediate );
		void Seek( bigtime_t media_time, bigtime_t performance_time );
		void TimeWarp( bigtime_t at_real_time, bigtime_t to_performance_time );
		void NodeRegistered();
		status_t RequestCompleted( const media_request_info &info );
		status_t HandleMessage( int32 message, const void *data, size_t size );
		status_t DeleteHook( BMediaNode *node );

		//  BMediaEventLooper
protected:
		void HandleEvent( const media_timed_event *event, bigtime_t lateness, bool realTimeEvent );

		//	BBufferConsumer
public:
		status_t AcceptFormat( const media_destination &dest, media_format *format );
		status_t GetNextInput( int32 *cookie, media_input *out_input );
		void DisposeInputCookie( int32 cookie );
protected:
		void BufferReceived( BBuffer *buffer );
private:
		void ProducerDataStatus( const media_destination &for_whom, int32 status, bigtime_t at_media_time );
		status_t GetLatencyFor( const media_destination &for_whom, bigtime_t *out_latency, media_node_id *out_id );
		status_t Connected( const media_source &producer, const media_destination &where, const media_format &with_format, media_input *out_input );
		void Disconnected( const media_source &producer, const media_destination &where );
		status_t FormatChanged( const media_source &producer, const media_destination &consumer, int32 from_change_count, const media_format &format );
							
		//	implementation
public:
		status_t CreateBuffers( const media_format &with_format );
		void DeleteBuffers();
		
		void SetCallback( bool (*callback)(const BBitmap *bitmap,void *userdata), void *userdata ) { fCallback=callback; fCallbackUserData=userdata; }

private:
		BMediaAddOn			*fAddOn;
		uint32				fInternalID;

		bool				fConnected;

		BBitmap				*fBitmap[3];
		BBufferGroup		*fBuffers;
		BBuffer				*fMyBufferList[3];	

		media_input			fInputNode;

		media_destination	fDestination;

		bool				(*fCallback)(const BBitmap *bitmap,void *userdata);
		void				*fCallbackUserData;
	};
	
	class VideoCapture
	{
	public:
		VideoCapture();
		virtual ~VideoCapture();
		
		status_t InitCheck() const;
		
		virtual void NewBitmap( const BBitmap *bitmap );
//		void SetCallback( void (*callback)(const BBitmap *bitmap,void *userdata), void *userdata ) { fCallback=callback; fCallbackUserData=userdata; }
		void SetCallback( bool (*callback)(const BBitmap *bitmap,void *userdata), void *userdata ) { fConsumer->SetCallback(callback,userdata); }

		void ShowParameterWeb();

	private:
		static bool _NewBitmap( const BBitmap *bitmap, void *userdata );


		status_t			fInitStatus;

		BMediaRoster		*fMediaRoster;
		media_output		fProducerOut;
		media_node			fProducerNode;
		media_input			fConsumerIn;
		SimpleVideoConsumer	*fConsumer;
		
		BWindow				*fVideoControlWindow;

		bool				(*fCallback)(const BBitmap *bitmap,void *userdata);
		void				*fCallbackUserData;
	};
}

//-----------------------------------------------------------------------------
#endif

