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
#include <app/Application.h>
#include <interface/Bitmap.h>
#include <interface/Screen.h>
#include <interface/ScrollView.h>
#include <interface/Slider.h>
#include <storage/Directory.h>
#include <storage/File.h>
#include <support/Beep.h>
#include <translation/BitmapStream.h>
#include <translation/TranslatorRoster.h>
//-------------------------------------
#include "gfx/BitmapUtils.h"

#include "CAWin.h"
#include "HTTPFetch.h"
//-----------------------------------------------------------------------------

#define MSG_NEWBITMAP 'newb'
#define MSG_DELTASLIDE 'dlts'

//-----------------------------------------------------------------------------

#define WIN_BORDER_X 8
#define WIN_BORDER_Y 8

#define WIN_BITMAP_X WIN_BORDER_X
#define WIN_BITMAP_Y WIN_BORDER_Y
#define WIN_BITMAP_WIDTH 320
#define WIN_BITMAP_HEIGHT 240

#define WIN_DELTABITMAP_X (WIN_BORDER_X+WIN_BITMAP_WIDTH+WIN_BORDER_X)
#define WIN_DELTABITMAP_Y WIN_BORDER_Y
#define WIN_DELTABITMAP_WIDTH (WIN_BITMAP_WIDTH/2)
#define WIN_DELTABITMAP_HEIGHT (WIN_BITMAP_HEIGHT/2)

#define WIN_DELTAGRAPH_X (WIN_BORDER_X+WIN_BITMAP_WIDTH+WIN_BORDER_X)
#define WIN_DELTAGRAPH_Y (WIN_DELTABITMAP_Y+WIN_DELTABITMAP_HEIGHT+WIN_BORDER_Y)
#define WIN_DELTAGRAPH_WIDTH (WIN_BITMAP_WIDTH/2)
#define WIN_DELTAGRAPH_HEIGHT (WIN_BITMAP_HEIGHT/4-WIN_BORDER_Y/2)

#define WIN_DELTASLIDER_X (WIN_BORDER_X+WIN_BITMAP_WIDTH+WIN_BORDER_X)
#define WIN_DELTASLIDER_Y (WIN_DELTAGRAPH_Y+WIN_DELTAGRAPH_HEIGHT+WIN_BORDER_Y)
#define WIN_DELTASLIDER_WIDTH WIN_DELTAGRAPH_WIDTH
#define WIN_DELTASLIDER_HEIGHT (WIN_BITMAP_HEIGHT/4-WIN_BORDER_Y/2)

#define WIN_WIDTH	(WIN_BORDER_X+ WIN_BITMAP_WIDTH + WIN_BORDER_X + WIN_DELTABITMAP_WIDTH +WIN_BORDER_X)
#define WIN_HEIGHT	(WIN_BORDER_Y+ WIN_BITMAP_HEIGHT +WIN_BORDER_Y)

//-----------------------------------------------------------------------------

CAWin::CAWin() :
	BWindow( BRect(10,40,10+WIN_WIDTH-1,40+WIN_HEIGHT-1), "Coffee Alarm", B_TITLED_WINDOW_LOOK, B_NORMAL_WINDOW_FEEL, B_NOT_ZOOMABLE|B_NOT_RESIZABLE|B_ASYNCHRONOUS_CONTROLS )
{
	fBitmap = NULL;
	fOldBitmap = NULL;
	
	fRefValue = 0.5f;
	
	fRootView = new BView( BRect(0,0,WIN_WIDTH-1,WIN_HEIGHT-1), "Root", B_FOLLOW_ALL, B_WILL_DRAW );
	fRootView->SetViewColor( ui_color(B_PANEL_BACKGROUND_COLOR) );
	AddChild( fRootView );
	
	fDeltaBitmap = new BBitmap( BRect(0,0,WIN_DELTABITMAP_WIDTH-1,WIN_DELTABITMAP_HEIGHT-1), B_CMAP8 );

	fBitmapView = new BitmapView( BRect(WIN_BITMAP_X,WIN_BITMAP_Y,WIN_BORDER_X+WIN_BITMAP_WIDTH-1,WIN_BORDER_Y+WIN_BITMAP_HEIGHT-1) );
	fRootView->AddChild( fBitmapView );

	fDeltaBitmapView = new BitmapView( BRect(WIN_DELTABITMAP_X,WIN_DELTABITMAP_Y,WIN_DELTABITMAP_X+WIN_DELTABITMAP_WIDTH-1,WIN_DELTABITMAP_Y+WIN_DELTABITMAP_HEIGHT-1) );
	fDeltaBitmapView->SetBitmap( fDeltaBitmap );
	fRootView->AddChild( fDeltaBitmapView );
	
	fDeltaLoadView = new LoadView( BRect(WIN_DELTAGRAPH_X,WIN_DELTAGRAPH_Y,WIN_DELTAGRAPH_X+WIN_DELTAGRAPH_WIDTH-1,WIN_DELTAGRAPH_Y+WIN_DELTAGRAPH_HEIGHT-1), B_FOLLOW_ALL );
	fDeltaLoadView->SetRefValue( fRefValue );
	fRootView->AddChild( fDeltaLoadView );

	fDeltaSlider = new BSlider( BRect(WIN_DELTASLIDER_X,WIN_DELTASLIDER_Y,WIN_DELTASLIDER_X+WIN_DELTASLIDER_WIDTH-1,WIN_DELTASLIDER_Y+WIN_DELTASLIDER_HEIGHT-1),
		"DeltaSlider", "Change trigger", new BMessage(MSG_DELTASLIDE),  0,255, B_BLOCK_THUMB, B_FOLLOW_ALL, B_NAVIGABLE|B_WILL_DRAW|B_FRAME_EVENTS/*|B_INPUT_METHOD_AWARE*/ );
	fRootView->AddChild( fDeltaSlider );
	fDeltaSlider->SetModificationMessage( new BMessage(MSG_DELTASLIDE) );
	fDeltaSlider->SetHashMarks( B_HASH_MARKS_BOTH );
	fDeltaSlider->SetHashMarkCount( 9 );
	fDeltaSlider->SetLimitLabels( "0%", "100%" );
	fDeltaSlider->SetValue( int(fRefValue*256) );
	
	fThreadQuitReq = false;
	fThread = spawn_thread( _ImageFetcherThread, "Bitmap fetcher", B_NORMAL_PRIORITY, this );
	resume_thread( fThread );

	Show();
}

CAWin::~CAWin()
{
	delete fBitmap;
	delete fOldBitmap;
	delete fDeltaBitmap;
}

//--------------------------------------------------------------------------------

bool CAWin::QuitRequested()
{
	status_t status;

	printf( "Requesting thread quit...\n" );
	fThreadQuitReq = true;
	wait_for_thread( fThread, &status );
	printf( "Requesting thread quit...ok\n" );

	be_app->PostMessage( B_QUIT_REQUESTED );
	return true;
}

void CAWin::MessageReceived( BMessage *msg )
{
	switch( msg->what )
	{
		case MSG_NEWBITMAP:
			fBitmapView->SetBitmap( fBitmap );
			fBitmapView->Invalidate();
			fDeltaBitmapView->Invalidate();
			break;
			
		case MSG_DELTASLIDE:
		{
			int32 value;
			if( msg->FindInt32("be:value", &value) >= B_OK )
			{
				fRefValue = float(value) / 256.0f;
				fDeltaLoadView->SetRefValue( fRefValue );
			}
			break;
		}

		default:
			msg->PrintToStream();
			BWindow::MessageReceived( msg );
	}
}

//--------------------------------------------------------------------------------

BBitmap *CAWin::FetchBitmap()
{
	BMallocIO data;

	status_t status = HTTPFetch( "fridge.funcom.com/fridgecam/codycam.jpg", &data );
	if( status < B_OK )
		return NULL;
		
	BBitmapStream stream;

	BTranslatorRoster *roster = BTranslatorRoster::Default();
	if( roster->Translate(&data, NULL, NULL, &stream, B_TRANSLATOR_BITMAP) < B_OK )
		return NULL;

	BBitmap *bitmap = NULL;
	stream.DetachBitmap( &bitmap );
	return bitmap;
}

int32 CAWin::_ImageFetcherThread( void *data )
{
	CAWin *win = (CAWin*)data;
	win->ImageFetcherThread();
	return 0;
}

void CAWin::ImageFetcherThread()
{
	printf( "Worker started\n" );
	while( !fThreadQuitReq )
	{
		printf( "Fetching bitmap...\n" );
		BBitmap *bitmap = FetchBitmap();
		if( bitmap )
		{
			printf( "Fetching bitmap...success\n" );
			if( LockWithTimeout(1000000) >= B_OK )
			{
				delete fOldBitmap;
				fOldBitmap = fBitmap;
				fBitmap = bitmap;
				CalcDeltaBitmap();
				PostMessage( MSG_NEWBITMAP );
				Unlock();
			}
		}
		snooze( 5*1000000 );
	}
}

//-----------------------------------------------------------------------------

void CAWin::CalcDeltaBitmap()
{
	memset( fDeltaBitmap->Bits(), 0, fDeltaBitmap->BitsLength() );

	if( fBitmap==NULL || fOldBitmap==NULL )
		return;
	if( fBitmap->Bounds()!=fOldBitmap->Bounds() )
		return;
	if( fBitmap->ColorSpace()!=B_RGB32 && fBitmap->ColorSpace()!=B_RGBA32 )
		return;
	if( fOldBitmap->ColorSpace()!=B_RGB32 && fOldBitmap->ColorSpace()!=B_RGBA32 )
		return;

	int width = fBitmap->Bounds().IntegerWidth()+1;
	int height= fBitmap->Bounds().IntegerHeight()+1;
	
	int globaldiff = 0;

	for( int iy=0; iy<height; iy++ )
	{
		bitmap_rgb *bits = (bitmap_rgb*)(((uint8*)fBitmap->Bits())+iy*fBitmap->BytesPerRow());
		bitmap_rgb *oldbits = (bitmap_rgb*)(((uint8*)fOldBitmap->Bits())+iy*fOldBitmap->BytesPerRow());
		for( int ix=0; ix<width; ix++ )
		{
			bitmap_rgb col = bits[ix];
			bitmap_rgb oldcol = oldbits[ix];
			int diff = (col.red-oldcol.red)*(col.red-oldcol.red)*299 +
				(col.green-oldcol.green)*(col.green-oldcol.green)*587 +
				(col.blue-oldcol.blue)*(col.blue-oldcol.blue)*114;
			diff = int(pow(sqrt(float(diff)/1000.0f)/256.0f,0.4f)*31.0f);
			
			globaldiff += diff;
			
			int dstx = (ix*WIN_DELTABITMAP_WIDTH)/width;
			int dsty = (iy*WIN_DELTABITMAP_HEIGHT)/height;
			uint8 *dstbits = ((uint8*)fDeltaBitmap->Bits())+dsty*fDeltaBitmap->BytesPerRow()+dstx;
			if( diff > *dstbits )
				*dstbits = diff;
		}
	}

	float load = (float(globaldiff)/31.0f)/(width*height);
	fDeltaLoadView->AddSlice( load );
	
	if( load >= fRefValue )
//		beep();
		system_beep( "Coffee Alarm" );
}



