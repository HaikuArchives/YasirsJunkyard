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
#include <memory.h>
#include <stdio.h>
#include <stdlib.h>
//-------------------------------------
//#include <add-ons/tracker/TrackerAddOn.h>
//#include <app/Roster.h>
//#include <storage/Entry.h>
//#include <screen_saver/ScreenSaver.h>
#include <game/DirectWindow.h>
#include <interface/Bitmap.h>
#include <interface/Screen.h>
#include <interface/View.h>
//-------------------------------------
#include "Saver.h"
#include "FeedbackWinScr.h"
#include "Feedback.h"
//-----------------------------------------------------------------------------

BScreenSaver *instantiate_screen_saver( BMessage *msg, image_id id )
{
	return new FeedbackSaver( msg, id );
}

//-----------------------------------------------------------------------------

FeedbackSaver::FeedbackSaver( BMessage *message, image_id id ) :
	BScreenSaver( message, id )
{
#ifdef USE_WINSCR
	fWindow = NULL;
#else
	fFeedback = NULL;
#endif

	fPreviewFeedback = NULL;
	fPreviewBitmap = NULL;
}

FeedbackSaver::~FeedbackSaver()
{
#ifdef USE_WINSCR
	if( fWindow )
	{
		fWindow->Lock();
		fWindow->Hide();
		delete fWindow;
		fWindow = NULL;
	}
#endif
}
	
status_t FeedbackSaver::InitCheck()
{
#ifndef USE_WINSCR
	display_mode *modes = NULL;
	uint32 modecnt = 0;
	BScreen().GetModeList( &modes, &modecnt );
	for( uint i=0; i<modecnt; i++ )
	{
#if 0
		printf( "%d:\n", i );
		printf( "\ttiming\n" );
		printf( "\t\tpixel_clock:%ld\n", modes[i].timing.pixel_clock );
		printf( "\t\th_display:%d\n", modes[i].timing.h_display );
		printf( "\t\th_sync_start:%d\n", modes[i].timing.h_sync_start );
		printf( "\t\th_sync_end:%d\n", modes[i].timing.h_sync_end );
		printf( "\t\th_total:%d\n", modes[i].timing.h_total );
		printf( "\t\tv_display:%d\n", modes[i].timing.v_display );
		printf( "\t\tv_sync_start:%d\n", modes[i].timing.v_sync_start );
		printf( "\t\tv_sync_end:%d\n", modes[i].timing.v_sync_end );
		printf( "\t\tv_total:%d\n", modes[i].timing.v_total );
		printf( "\t\tflags:%ld\n", modes[i].timing.flags );
		printf( "\tspace:%ld\n", modes[i].space );
		printf( "\tvirtual_width:%d\n", modes[i].virtual_width );
		printf( "\tvirtual_height:%d\n", modes[i].virtual_height );
		printf( "\th_display_start:%d\n", modes[i].h_display_start );
		printf( "\tv_display_start:%d\n", modes[i].v_display_start );
#endif		
		if( modes[i].space==B_RGB32 && modes[i].virtual_width==640 && modes[i].virtual_height==480 )
		{
			fDisplayMode = modes[i];
			free( modes );
			return B_OK;
		}
	}
	return B_ERROR;
#else
	return B_OK;
#endif
}

status_t FeedbackSaver::StartSaver( BView *view, bool preview )
{
	printf( "FeedbackSaver::StartSaver: preview:%d\n", preview );

	if( preview )
	{
		fPreviewFeedback = new Feedback( B_RGB32 );
		fPreviewBitmap = new BBitmap( BRect(0,0,256-1,256-1), B_RGB32 );

		return B_OK;
	}
	else
	{
#ifdef USE_WINSCR
		BDirectWindow *dirwin = dynamic_cast<BDirectWindow*>(view->Window());
		if( dirwin )
			dirwin->SetFullScreen( false );

		if( fWindow == NULL )
		{
			status_t error = B_OK;
			fWindow = new FeedbackWinScr( &error );
			printf( "FeedbackSaver::StartSaver: %ld\n", error );
			if( error != B_OK )
			{
				delete fWindow;
				fWindow = NULL;
				return error;
			}
		}
	
		fWindow->Lock();
		fWindow->Show();
		fWindow->Unlock();

		return B_OK;
#else
		if( BScreen().GetMode(&fOldDisplayMode) != B_OK )
			return B_ERROR;

		if( BScreen().SetMode(&fDisplayMode) != B_OK )
			return B_ERROR;

		snooze( 500000 );
		
		printf( "Start...\n" );
		fFeedback = new Feedback( (color_space)fDisplayMode.space );

		return B_OK;
#endif
	}
}

void FeedbackSaver::StopSaver()
{
	printf( "FeedbackSaver::StopSaver\n" );

#ifdef USE_WINSCR
	if( fWindow )
	{
		fWindow->Lock();
		fWindow->Hide();
		fWindow->Quit();
//		delete fWindow;
		fWindow = NULL;
	}
#else
	delete fFeedback;
	fFeedback = NULL;

	BScreen().SetMode( &fOldDisplayMode );
#endif

	if( fPreviewFeedback )
	{
		delete fPreviewFeedback;
		fPreviewFeedback = NULL;
	}
	if( fPreviewBitmap )
	{
		delete fPreviewBitmap;
		fPreviewBitmap = NULL;
	}
}

void FeedbackSaver::Draw( BView *view, int32 frame )
{
	if( fPreviewFeedback && fPreviewBitmap )
	{
		fPreviewFeedback->UpdateFrame();
		fPreviewFeedback->GetFrame( fPreviewBitmap->Bits(), fPreviewBitmap->Bounds().IntegerWidth()+1, fPreviewBitmap->Bounds().IntegerHeight()+1, fPreviewBitmap->BytesPerRow(), false );
		view->DrawBitmap( fPreviewBitmap, fPreviewBitmap->Bounds(), view->Bounds() );
	}
}

void FeedbackSaver::DirectConnected( direct_buffer_info *info )
{
#ifndef USE_WINSCR
	printf( "Connect...\n" );

	fDirectInfo = *info;
//	if( fDirectInfo.buffer_state==B_DIRECT_START || fDirectInfo.buffer_state==B_DIRECT_MODIFY )
	{
		printf( "%p %d\n", fDirectInfo.bits, fDirectInfo.bytes_per_row*(fDirectInfo.window_bounds.bottom-fDirectInfo.window_bounds.top+1) );
		memset( fDirectInfo.bits, 0, fDirectInfo.bytes_per_row*(fDirectInfo.window_bounds.bottom-fDirectInfo.window_bounds.top+1) );
	}
#endif
}

void FeedbackSaver::DirectDraw( int32 frame )
{
	printf( "Draw...\n" );

#ifndef USE_WINSCR
	int winwidth = fDirectInfo.window_bounds.right-fDirectInfo.window_bounds.left+1;
	int winheight = fDirectInfo.window_bounds.bottom-fDirectInfo.window_bounds.top+1;

	int width = 256;
	if( width > winwidth/2 ) width = winwidth/2;
	int height = 256;
	if( height > winheight/2 ) height = winheight/2;
		
	fFeedback->UpdateFrame();
	uint8 *bits = (uint8*)fDirectInfo.bits + ((winwidth-width*2)/2)*(fDirectInfo.bits_per_pixel/8);
	fFeedback->GetFrame( bits, width, height, fDirectInfo.bytes_per_row );
#endif
}

//	void StartConfig( BView *configView );
//	void StopConfig();

//	void SupplyInfo( BMessage *info ) const;

//	void ModulesChanged( const BMessage *info );

//	status_t SaveState( BMessage *into ) const;

//	void SetTickSize( bigtime_t ts );
//	bigtime_t TickSize() const;

//	void SetLoop( int32 on_count, int32 off_count );
//	int32 LoopOnCount() const;
//	int32 LoopOffCount() const;

//-----------------------------------------------------------------------------
