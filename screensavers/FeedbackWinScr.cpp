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
//-------------------------------------
#include <app/Application.h>
//-------------------------------------
#include "FeedbackWinScr.h"
//-----------------------------------------------------------------------------

FeedbackWinScr::FeedbackWinScr( status_t *error ) :
	BWindowScreen( "Feedback", B_32_BIT_640x480, error, false ),
	fFeedback( B_RGB32 )
{
	printf( "FeedbackWinScr::FeedbackWinScr()\n" );

	fThread = B_ERROR;
}

FeedbackWinScr::~FeedbackWinScr()
{
	printf( "FeedbackWinScr::~FeedbackWinScr()\n" );

	Hide();
}

bool FeedbackWinScr::QuitRequested()
{
//	be_app->PostMessage( B_QUIT_REQUESTED );
	return true;
}

void FeedbackWinScr::ScreenConnected( bool active )
{
	printf( "FeedbackWinScr::ScreenConnected: %d\n", active );

	BWindowScreen::ScreenConnected( active );
	
	if( active )
	{
		assert( fThread == B_ERROR );
		fQuitThread = false;
		fThread = spawn_thread( _DrawThread, "DrawThread", B_NORMAL_PRIORITY, this );
		resume_thread( fThread );
	}
	else
	{
		assert( fThread != B_ERROR );
		fQuitThread = true;
		status_t status;
		wait_for_thread( fThread, &status );
		fThread = B_ERROR;
	}
}

int32 FeedbackWinScr::_DrawThread( void *data )
{
	((FeedbackWinScr*)data)->DrawThread();
	return 0;
}

void FeedbackWinScr::DrawThread()
{
	graphics_card_info *cinfo = CardInfo();
// int16	version;
// int16	id;
// void		*frame_buffer;
// char		rgba_order[4];
// int16	flags;
// int16	bits_per_pixel;
// int16	bytes_per_row;
// int16	width;
// int16	height;
	
	frame_buffer_info *fbinfo = FrameBufferInfo();
// int16	bits_per_pixel;
// int16	bytes_per_row;
// int16	width;
// int16	height;
// int16	display_width;
// int16	display_height;
// int16	display_x;
// int16	display_y;

	memset( cinfo->frame_buffer, 0, cinfo->bytes_per_row*cinfo->height );

	while( !fQuitThread )
	{
		int width = 256;
		if( width > fbinfo->display_width/2 ) width = fbinfo->display_width/2;

		int height = 256;
		if( height > fbinfo->display_height/2 ) height = fbinfo->display_height/2;
		
//		height -= 4;

		fFeedback.UpdateFrame();
//		uint8 *bits = (uint8*)cinfo->frame_buffer + ((fbinfo->display_width-width*2)/2)*(fbinfo->bits_per_pixel/8);
		uint8 *bits = (uint8*)cinfo->frame_buffer;
		fFeedback.GetFrame( bits, width, height, fbinfo->bytes_per_row, true );
	}
}

//-----------------------------------------------------------------------------
