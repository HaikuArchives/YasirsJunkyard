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
//-------------------------------------
#include <app/Application.h>
#include <interface/Bitmap.h>
#include <interface/Screen.h>
#include <interface/ScrollView.h>
#include <storage/Directory.h>
#include <storage/File.h>
#include <translation/BitmapStream.h>
#include <translation/TranslatorRoster.h>
//-------------------------------------
#include "SSWin.h"
#include "BitmapView.h"
//-----------------------------------------------------------------------------

template<class T>
T min( T a, T b )
{
	if( a<b ) return a;
	return b;
}

int bitmapcnt = 0;

//-----------------------------------------------------------------------------

SSWin::SSWin() :
	BWindow( BRect(10,40,10+400-1,40+300-1), "SimpleSlide", B_DOCUMENT_WINDOW/*B_TITLED_WINDOW*/, B_NOT_ZOOMABLE )
{
	fNextRef = 0;
	fCurrRef = 0;

	fWorkListChanged = false;
	fWorkThreadQuitReq = false;
	fWorkThread = spawn_thread( _ThreadFunc, "Bitmap loader", B_NORMAL_PRIORITY, this );

//	BRect bounds = Bounds();
//	fFileListView = new FileListView( BRect(0,menu_height,bounds.Width()-B_V_SCROLL_BAR_WIDTH, bounds.Height()) );
//	AddChild( new BScrollView("", fFileListView, B_FOLLOW_ALL, 0, false, true, B_NO_BORDER) );

	BRect bounds = Bounds();
	fBitmapView = new BitmapView( BRect(0,0,bounds.Width()-B_V_SCROLL_BAR_WIDTH, bounds.Height()-B_H_SCROLL_BAR_HEIGHT) );
//	fBitmapView = new BitmapView( BRect(0,0,bounds.Width()-B_V_SCROLL_BAR_WIDTH, bounds.Height()-B_H_SCROLL_BAR_HEIGHT) );
	AddChild( new BScrollView("", fBitmapView, B_FOLLOW_ALL, 0, true, true, B_NO_BORDER) );

	Show();
}

SSWin::~SSWin()
{
}

//--------------------------------------------------------------------------------

bool SSWin::QuitRequested()
{
	be_app->PostMessage( B_QUIT_REQUESTED );
	return true;
}

void SSWin::MessageReceived( BMessage *msg )
{
	switch( msg->what )
	{
		case 'UPD!':
			{
				printf( "<<<<<<<<<<<<<<<%d\n", bitmapcnt );
				printf( "Win: list update\n" );
				if( fWorkListLock.Lock() )
				{
					entry_ref *ref = (entry_ref*)fRefs.ItemAt( fCurrRef );
					WorkInfo *wi = NULL;
					for( int i=0; i<fWorkList.CountItems(); i++ )
					{
						wi = (WorkInfo*)fWorkList.ItemAt( i );
						if( wi->fRef == *ref )
							break;
						wi = NULL;
					}
					if( wi )
					{
						printf( "Win: got new bitmap: %s\n", wi->fRef.name );
						if( wi->fState == WorkInfo::S_READY )
						{
							BBitmap *bitmap = wi->fBitmap;
							fWorkListLock.Unlock();
							
							if( fBitmapView->GetBitmap() != bitmap )
							{
								Lock();
								BScreen screen( this );
								BRect frame = Frame();
								float width = min( screen.Frame().Width()-frame.left-10, float(bitmap->Bounds().Width()+B_V_SCROLL_BAR_WIDTH) );
								float height = min( screen.Frame().Height()-frame.top-10, float(bitmap->Bounds().Height()+B_H_SCROLL_BAR_HEIGHT) );
								ResizeTo( width, height );	
								Unlock();
								fBitmapView->SetOwnership( false );
								fBitmapView->SetBitmap( bitmap );
							}
						}
						else if( wi->fState == WorkInfo::S_ERROR )
						{
							printf( "Win: got new (defect) bitmap\n" );
							fWorkList.RemoveItem( wi );
							fWorkListLock.Unlock();
	
							fBitmapView->SetOwnership( false );
							fBitmapView->SetBitmap( NULL );
						}
						else
						{
							printf( "Win: got new bitmap, but with state: %d\n", wi->fState );
							fWorkListLock.Unlock();
						}
					}
					else
					{
						fWorkListLock.Unlock();
					}
				}
				// remove gunk:
				if( fWorkListLock.Lock() )
				{
					int i;
					WorkInfo *wi;
					for( i=0; i<fWorkList.CountItems(); i++ )
					{
						wi = (WorkInfo*)fWorkList.ItemAt( i );
						bool found = false;
						bool remove = false;
						for( int j=0; j<fRefs.CountItems(); j++ )
						{
							entry_ref *ref = (entry_ref*)fRefs.ItemAt( j );
							if( *ref == wi->fRef )
							{
								found = true;
								if( j<fCurrRef-1 || j>fCurrRef+2 )
								{
									printf( "Old done: %s %d/%d\n", wi->fRef.name, j, fCurrRef );
									remove = true;
									break;
								}
							}
						}
						if( !found || remove )
						{
							delete wi->fBitmap; if(wi->fBitmap) bitmapcnt--;
							fWorkList.RemoveItem( wi );
						}
					}
					fWorkListLock.Unlock();
				}
			}
			break;
			
		case B_REFS_RECEIVED:
			{
				entry_ref ref;
				for( int i=0; msg->FindRef("refs",i,&ref)>=B_NO_ERROR; i++ )
				{
					entry_ref *refp = new entry_ref(ref);
					fRefs.AddItem( refp );
				}
			}
			if( fRefs.CountItems() > 0 )
			{
				fCurrRef = 0;
				fNextRef = 0;
				
				SetWindowTitle( fCurrRef );
				ShowPicture( fCurrRef );
			}
			break;
			
		case B_KEY_DOWN:
			{
//				msg->PrintToStream();
				int32 raw_char = -1;
				const char *bytes;
				msg->FindInt32( "raw_char", &raw_char );
				msg->FindString( "bytes", &bytes );
				if( raw_char == B_RIGHT_ARROW )
				{
					if( fNextRef+1 < fRefs.CountItems() )
					{
						fNextRef++;
						SetWindowTitle( fNextRef );
					}
				}
				else if( raw_char == B_LEFT_ARROW )
				{
					if( fNextRef > 0 )
					{
						fNextRef--;
						SetWindowTitle( fNextRef );
					}
				}
				else if( raw_char == B_HOME )
				{
					if( fRefs.CountItems() )
					{
						fNextRef = 0;
						SetWindowTitle( fNextRef );
					}
				}
				else if( raw_char == B_END )
				{
					if( fRefs.CountItems() )
					{
						fNextRef = fRefs.CountItems()-1;
						SetWindowTitle( fNextRef );
					}
				}
				else if( raw_char == B_DELETE )
				{
					char newname[B_FILE_NAME_LENGTH];
					strcpy( newname, "!D_" );
					entry_ref *refp = (entry_ref*)fRefs.ItemAt( fCurrRef );
					BEntry entry( refp );
					entry.GetName( newname+strlen(newname) );
					entry.Rename( newname );

					fRefs.RemoveItem( fCurrRef );
					if( fCurrRef >= fRefs.CountItems() )
						fCurrRef = fRefs.CountItems()-1;
					if( fCurrRef < 0 )
						fCurrRef = 0;
					else
					{
						fNextRef = fCurrRef;
						SetWindowTitle( fCurrRef );
						ShowPicture( fCurrRef );
					}
				}
				else if( bytes[0]>='0' && bytes[0]<='9' )
				{
					char dirname[16];
					sprintf( dirname, "!_%c", bytes[0] );
					
					entry_ref *refp = (entry_ref*)fRefs.ItemAt( fCurrRef );
					BEntry entry( refp );
					
					BDirectory filedir;
					entry.GetParent( &filedir );
					
					BEntry newdirentry;
					BDirectory newdir;
					if( filedir.FindEntry(dirname,&newdirentry) >= B_OK )
					{
						newdir.SetTo( &newdirentry );
					}
					else
					{
						filedir.CreateDirectory( dirname, &newdir );
					}

					if( entry.MoveTo(&newdir) >= B_OK )
					{
						fRefs.RemoveItem( fCurrRef );
						if( fCurrRef >= fRefs.CountItems() )
							fCurrRef = fRefs.CountItems()-1;
						if( fCurrRef < 0 )
							fCurrRef = 0;
						else
						{
							fNextRef = fCurrRef;
							SetWindowTitle( fCurrRef );
							ShowPicture( fCurrRef );
						}
					}
				}
			}
			break;

		case B_KEY_UP:
			if( fNextRef != fCurrRef )
			{
				fCurrRef = fNextRef;
				ShowPicture( fNextRef );
			}
			break;
			
		default:
			msg->PrintToStream();
	}
}

//--------------------------------------------------------------------------------

void SSWin::SetWindowTitle( int index )
{
	char title[256];

	entry_ref *refp = (entry_ref*)fRefs.ItemAt( index );
	sprintf( title, "%s (%d/%ld)", refp->name, index+1, fRefs.CountItems() );

	Lock();
	SetTitle( title );
	Unlock();
}

void SSWin::AddPicture( int index, int pri, bool sendmsg )
{
	if( index<0 || index>=fRefs.CountItems() )
		return;

	entry_ref *ref = (entry_ref*)fRefs.ItemAt( index );
	for( int i=0; i<fWorkList.CountItems(); i++ )
	{
		WorkInfo *wi = (WorkInfo*)fWorkList.ItemAt( i );
		if( wi->fRef == *ref )
		{
			wi->fPriority = pri;
			if( sendmsg )
				PostMessage( 'UPD!' );
			return;
		}
	}

	WorkInfo *wi = new WorkInfo;
	wi->fPriority = pri;
	wi->fBitmap = NULL;
	wi->fRef = *(entry_ref*)fRefs.ItemAt( index );
	wi->fState = WorkInfo::S_NEW;
	fWorkList.AddItem( wi );
}

void SSWin::ShowPicture( int index )
{
	fWorkListLock.Lock();

	for( int i=0; i<fWorkList.CountItems(); i++ )
	{
		WorkInfo *wi = (WorkInfo*)fWorkList.ItemAt( i );
		wi->fPriority = 1000;
	}
	AddPicture( index, 0, true );
	AddPicture( index+1, 1 );
	AddPicture( index+2, 2 );
	AddPicture( index-1, 3 );
	
	fWorkList.SortItems( WorkListCmp );
	fWorkListChanged = true;
	fWorkListLock.Unlock();
	resume_thread( fWorkThread );
}


#if 0
status_t SSWin::ShowPicture( int index )
{
//	SetWindowTitle( index );

	entry_ref *refp = (entry_ref*)fRefs.ItemAt( index );

	return ShowPicture( *refp );
}

status_t SSWin::ShowPicture( const entry_ref &ref )
{
	BFile file;
	if( file.SetTo(&ref, B_READ_WRITE)<B_NO_ERROR )
	{
		return B_ERROR;
	}

	return ShowPicture( file, ref.name );
}

status_t SSWin::ShowPicture( BFile &file, char *name )
{
#if 0
	if( !file.IsFile() )
	{
		FDERROR(( "'%s' is not a file\n", name ));
		return B_ERROR;
	}

	if( !file.IsReadable() )
	{
		FDERROR(( "'%s' is not readeable\n", name ));
		return B_ERROR;
	}

	BitmapStream output;
	if( DATATranslate(file, NULL, NULL, output, DATA_BITMAP) < B_NO_ERROR )
	{
		FDERROR(( "'%s' is not of a known type\n", name ));
		return B_ERROR;
	}

	BBitmap *bitmap;
	if( output.DetachBitmap(bitmap)<B_NO_ERROR || bitmap==NULL )
	{
		FDERROR(( "'%s' is not detach bitmap\n", name ));
		return B_ERROR;
	}
#else
	BTranslatorRoster *roster = BTranslatorRoster::Default();
	BBitmapStream stream;
	BBitmap *bitmap = NULL;
	if( roster->Translate(&file, NULL, NULL, &stream, B_TRANSLATOR_BITMAP) < B_OK )
		return B_ERROR;
	stream.DetachBitmap( &bitmap );
#endif

	Lock();
	BScreen screen( this );
	BRect frame = Frame();
	float width = min( screen.Frame().Width()-frame.left-10, float(bitmap->Bounds().Width()+B_V_SCROLL_BAR_WIDTH) );
	float height = min( screen.Frame().Height()-frame.top-10, float(bitmap->Bounds().Height()+B_H_SCROLL_BAR_HEIGHT) );
	ResizeTo( width, height );	
//	ResizeTo( bitmap->Bounds().Width()+B_V_SCROLL_BAR_WIDTH, bitmap->Bounds().Height()+B_H_SCROLL_BAR_HEIGHT );
	Unlock();
	fBitmapView->SetBitmap( bitmap );

	return B_NO_ERROR;
}
#endif

//--------------------------------------------------------------------------------

BBitmap *SSWin::ReadBitmap( const entry_ref &ref )
{
	BFile file;
	if( file.SetTo(&ref, B_READ_WRITE)<B_NO_ERROR )
	{
		return NULL;
	}

	BTranslatorRoster *roster = BTranslatorRoster::Default();
	BBitmapStream stream;
	BBitmap *bitmap = NULL;
	if( roster->Translate(&file, NULL, NULL, &stream, B_TRANSLATOR_BITMAP) < B_OK )
		return NULL;
	stream.DetachBitmap( &bitmap );
	return bitmap;
}

int32 SSWin::_ThreadFunc( void *data )
{
	SSWin *win = (SSWin*)data;
	win->ThreadFunc();
	return 0;
}

void SSWin::ThreadFunc()
{
	printf( "Worker started\n" );
	while( !fWorkThreadQuitReq )
	{
		bool done = false;
		
		if( fWorkListChanged )
		{
			if( fWorkListLock.Lock() )
			{
				WorkInfo *wi = NULL;
				for( int i=0; i<fWorkList.CountItems(); i++ )
				{
					wi = (WorkInfo*)fWorkList.ItemAt( i );
					if( wi->fState == WorkInfo::S_NEW )
						break;
					wi = NULL;
				}
				if( wi )
				{
					entry_ref ref = wi->fRef;
					printf( "New work: %s\n", ref.name );

					wi->fState = WorkInfo::S_LOADING;
					fWorkListLock.Unlock();

					BBitmap *bitmap = ReadBitmap( ref );
					if( bitmap ) bitmapcnt++;

					if( fWorkListLock.Lock() )
					{
						WorkInfo *wi = NULL;
						for( int j=0; j<fWorkList.CountItems(); j++ )
						{
							wi = (WorkInfo*)fWorkList.ItemAt( j );
							if( wi->fRef == ref )
								break;
							wi = NULL;
						}
						if( !wi )
						{
							delete bitmap; bitmapcnt--;
							fWorkListLock.Unlock();
						}
						else
						{
							wi->fBitmap = bitmap;
							wi->fState = bitmap?WorkInfo::S_READY:WorkInfo::S_ERROR;
							fWorkListLock.Unlock();
							PostMessage( 'UPD!' );
						}
					}
					else
					{
						printf( "Worker: bitmap no longer in list\n" );
						delete bitmap; bitmapcnt--;
					}
				}
				else
				{
					done = true;
					fWorkListChanged = false;
					fWorkListLock.Unlock();
				}
			}
			else
			{
				printf( "Worker could not lock the job list\n" );
			}
		}
		else
		{
			done = true;
		}

		if( done )
		{
			printf( "Worker waiting\n" );
			suspend_thread( find_thread(NULL) );
			printf( "Worker restarted\n" );
		}
	}
}

int SSWin::WorkListCmp( const void *a, const void *b )
{
	const WorkInfo *wa = (const WorkInfo*)a;
	const WorkInfo *wb = (const WorkInfo*)b;
	return wb->fPriority - wa->fPriority;
}

//-----------------------------------------------------------------------------
