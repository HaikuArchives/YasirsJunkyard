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
//-------------------------------------
//#include <add-ons/tracker/TrackerAddOn.h>
//#include <app/Roster.h>
//#include <storage/Entry.h>
#include <add-ons/screen_saver/ScreenSaver.h>
#include <add-ons/graphics/Accelerant.h>
//-------------------------------------
class Feedback;
//-----------------------------------------------------------------------------

#define USE_WINSCR

//-----------------------------------------------------------------------------

class FeedbackSaver : public BScreenSaver
{
public:
	FeedbackSaver( BMessage *message, image_id id );
	~FeedbackSaver();
	
	status_t InitCheck();

	status_t StartSaver( BView *view, bool preview );
	void StopSaver();

	void Draw( BView *view, int32 frame );

	void DirectConnected( direct_buffer_info *info );
	void DirectDraw( int32 frame );

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

private:

#ifdef USE_WINSCR
	BWindow	*fWindow;
#else
	display_mode		fDisplayMode;
	display_mode		fOldDisplayMode;

	direct_buffer_info	fDirectInfo;
	Feedback			*fFeedback;
#endif

	Feedback			*fPreviewFeedback;
	BBitmap				*fPreviewBitmap;
	

};

//-----------------------------------------------------------------------------