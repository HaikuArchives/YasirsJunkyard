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
#include <interface/Bitmap.h>
#include <support/String.h>
#include <storage/Directory.h>
#include <storage/Mime.h>
#include <storage/NodeInfo.h>
//-------------------------------------
#include "misc/AutoPtr.h"
#include "MakeIcon.h"
#include "MIWin.h"
#include "MIApp.h"
//-----------------------------------------------------------------------------

MIApp::MIApp() :
	BApplication( APP_MIME )
{
	fWindow = new MIWin();

	fWindow->Lock();
	fWindow->SetProgress( "Setting up" B_UTF8_ELLIPSIS, 0.0f );
	fWindow->Unlock();
	
	BMessage imagesubtypes;
	status_t status = BMimeType::GetInstalledTypes( "image", &imagesubtypes );
	if( status == B_NO_ERROR )
	{
		const char *subname;
		for( uint i=0; imagesubtypes.FindString("types",i,&subname) == B_NO_ERROR; i++ )
		{
//			printf( "%s\n", subname );
			BMimeType subtype( subname );
			if( !subtype.IsValid() ) continue;
			
			BMessage attrinfo;
			subtype.GetAttrInfo( &attrinfo );
			const char *attrname;
			bool foundattr = false;
			for( uint j=0; attrinfo.FindString("attr:name",j,&attrname) == B_NO_ERROR; j++ )
			{
				if( strcmp(attrname,"Image:Size") == 0 )
				{
					foundattr = true;
					break;
				}
			}
			if( !foundattr )
			{
//				printf( "=====================================\n" );
//				attrinfo.PrintToStream();
				attrinfo.AddString( "attr:public_name", "Size" );
				attrinfo.AddString( "attr:name", "Image:Size" );
				attrinfo.AddInt32( "attr:type", B_STRING_TYPE );
				attrinfo.AddBool( "attr:editable", false );
				attrinfo.AddBool( "attr:viewable", true );
				attrinfo.AddBool("attr:extra", false );
				attrinfo.AddInt32( "attr:alignment", B_ALIGN_RIGHT );
				attrinfo.AddInt32( "attr:width", 50 );
//				printf( "=====================================\n" );
//				attrinfo.PrintToStream();
				subtype.SetAttrInfo( &attrinfo );
			}
		}
	}
	
	fWindow->Lock();
	fWindow->SetProgress( "Gathering files" B_UTF8_ELLIPSIS, 0.0f );
	fWindow->Unlock();
}

MIApp::~MIApp()
{
}

void MIApp::ReadyToRun()
{
	fWindow->Lock();
	fWindow->SetProgress( "Ready" B_UTF8_ELLIPSIS, 0.0f );
	fWindow->Unlock();
	
	uint items = fRefs.CountItems();
	for( uint i=0; i<items; i++ )
	{
		entry_ref *ref;
		ref = (entry_ref*)fRefs.ItemAt( i );
	
		fWindow->Lock();
		fWindow->SetProgress( ref->name, float(i)/float(items) );
		fWindow->Unlock();

		BFile file( ref, B_READ_WRITE );
		MakeIcon( &file );
	}
	
	PostMessage( B_QUIT_REQUESTED );
}

void MIApp::RefsReceived( BMessage *message )
{
#if 0
	type_code type;
	int32 cnt;
	if( message->GetInfo("refs", &type, &cnt) == B_NO_ERROR )
	{
		for( int i=cnt-1; i>=0; i-- )
		{
			entry_ref ref;
			if( message->FindRef("refs",i,&ref) == B_NO_ERROR )
				ProcessEntry( ref );
		}
	}
#else
	for( int i=0; message->HasRef("refs",i); i++ )
	{
		entry_ref ref;
		if( message->FindRef("refs",i,&ref) == B_NO_ERROR )
		{
			BString path; path << ref.name;
			ProcessEntry( ref, path );
		}
	}
#endif
}

void MIApp::ArgvReceived( int32 argc, char **argv )
{
	for( int i=1; i<argc; i++ )
	{
		entry_ref ref;
		if( !get_ref_for_path(argv[i], &ref) )
		{
			BString path; path << ref.name;
			ProcessEntry( ref, path );
		}
	}
}


status_t MIApp::ProcessEntry( const entry_ref &ref, const BString &path )
{
	fWindow->Lock();
	BString prog; prog << "Preparing: " << path << B_UTF8_ELLIPSIS;
	fWindow->SetProgress( prog.String(), 0.0f );
	fWindow->Unlock();

	BEntry entry( &ref, true );
	status_t status = entry.InitCheck();
	if( status != B_NO_ERROR ) return status;

	if( entry.IsFile() )
	{
		// add to list
		fRefs.AddItem( new entry_ref(ref) );
	}
	else if( entry.IsDirectory() )
	{
		// We do now follow symlinks to dirs...
		entry.SetTo( &ref, false );
		status_t status = entry.InitCheck();
		if( status != B_NO_ERROR ) return status;
		if( entry.IsSymLink() ) return B_NO_ERROR;
	
		BDirectory dir( &entry );
		status = dir.InitCheck();
		if( status != B_NO_ERROR ) return status;

		entry_ref entryref;
		while( dir.GetNextRef(&entryref) >= B_OK )
		{
			BString subpath; subpath << path << "/" << entryref.name;
			ProcessEntry( entryref, subpath );
		}
	}

	return B_NO_ERROR;
}

status_t MIApp::MakeIcon( BFile *file )
{
	if( file->InitCheck() < B_OK )
	{
		return B_ERROR;
	}

	time_t modtime;
	file->GetModificationTime( &modtime );

	time_t icontime;
	if( file->ReadAttr("IconModificationTime",B_TIME_TYPE,0,&icontime,sizeof(icontime)) == sizeof(icontime) )
	{
		if( icontime >= modtime )
		{
			return B_NO_ERROR;
		}
	}

	damn::AutoPtr<BBitmap> bitmap = ReadBitmap( file );
	if( !bitmap ) return B_ERROR;

	BNodeInfo nodeinfo( file );
	status_t status = ::MakeIcon( bitmap, &nodeinfo );
	if( status!=B_NO_ERROR ) return B_ERROR;

	file->WriteAttr( "IconModificationTime", B_TIME_TYPE, 0, &modtime, sizeof(modtime) );

	char sizestring[32];
	sprintf( sizestring, "%ldx%ld", bitmap->Bounds().IntegerWidth()+1, bitmap->Bounds().IntegerHeight()+1 );
	file->WriteAttr( "Image:Size", B_STRING_TYPE, 0, &sizestring, strlen(sizestring)+1 );

	return B_NO_ERROR;
}

//-----------------------------------------------------------------------------

