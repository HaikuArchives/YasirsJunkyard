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
#include <interface/Window.h>
#include <storage/Entry.h>
#include <support/Locker.h>
//-------------------------------------
class BitmapView;
//-----------------------------------------------------------------------------

class SSWin : public BWindow
{
public:
	SSWin();
	~SSWin();

	bool QuitRequested();

	void MessageReceived( BMessage *msg );

private:
	void SetWindowTitle( int index );

	void AddPicture( int index, int pri, bool sendmsg=false );
	void ShowPicture( int index );

	BBitmap *ReadBitmap( const entry_ref &ref );
	static int32 _ThreadFunc( void *data );
	void ThreadFunc();
	static int WorkListCmp( const void *a, const void *b );

	BitmapView	*fBitmapView;

	struct WorkInfo
	{
		enum state_t { S_NEW, S_LOADING, S_READY, S_ERROR };
		int			fPriority;
		BBitmap		*fBitmap;
		entry_ref	fRef;
		state_t		fState;
	};
	
	BLocker			fWorkListLock;
	bool			fWorkListChanged;
	BList			fWorkList;
	volatile bool	fWorkThreadQuitReq;
	thread_id		fWorkThread;

	int			fCurrRef;
	int			fNextRef;
	BList		fRefs;
};

//-----------------------------------------------------------------------------
