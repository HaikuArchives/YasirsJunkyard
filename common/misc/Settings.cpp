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
#include <errno.h>
#include <stdio.h>
#include <string.h>
//-------------------------------------
#include <storage/File.h>
#include <storage/FindDirectory.h>
#include <storage/Path.h>
//-------------------------------------
#include "Settings.h"
//-----------------------------------------------------------------------------

damn::Settings::Settings( const char *filename, bool writeondestruct )
{
	status_t status;


	BPath pathname;
	status = find_directory( B_USER_SETTINGS_DIRECTORY, &pathname );
	if( status != B_NO_ERROR )
	{
		fprintf( stderr, "Settings: could not find user config path: %s: %s\n", filename, strerror(errno) );
	}
	else
	{
		pathname.Append( filename );
	
		BFile file( pathname.Path(), B_READ_ONLY );
		if( !file.IsReadable() )
		{
			fprintf( stderr, "Settings: could not open file: %s: %s\n", pathname.Path(), strerror(errno) );
		}
		else
		{
			status = fMessage.Unflatten( &file );
			if( status != B_NO_ERROR )
			{
				fprintf( stderr,  "Settings: could not unflatten file: %s: %s\n", pathname.Path(), strerror(status) );
			}
		}
	}
	
	if( writeondestruct )
	{
		fWriteOnDestruct = writeondestruct;
		fFilename = filename;
	}
}

damn::Settings::Settings( const BMessage &message )
{
	fMessage = message;
}

damn::Settings::Settings( const Settings &settings )
{
	fMessage = settings.fMessage;
}

damn::Settings::~Settings()
{
	if( fWriteOnDestruct )
		Write( fFilename.String() );
}

//-----------------------------------------------------------------------------

status_t damn::Settings::Write( const char *filename )
{
	assert( filename!=NULL || fWriteOnDestruct );
	
	if( filename == NULL )
		filename = fFilename.String();

	status_t status;
	
	BPath pathname;

	status = find_directory( B_USER_SETTINGS_DIRECTORY, &pathname );
	if( status != B_NO_ERROR )
	{
		fprintf( stderr, "Settings: could not find user config path: %s: %s\n", filename, strerror(errno) );
		return status;
	}
	pathname.Append( filename );
	
	BFile file( pathname.Path(), B_READ_WRITE|B_CREATE_FILE|B_ERASE_FILE );
	if( !file.IsWritable() )
	{
		fprintf( stderr, "Settings: could not open file: %s: %s\n", pathname.Path(), strerror(errno) );
		return B_ERROR;
	}
	
	status = fMessage.Flatten( &file );
	if( status != B_NO_ERROR )
	{
		fprintf( stderr,  "Settings: could not flatten to file: %s: %s\n", pathname.Path(), strerror(status) );
		return status;
	}
	
	return B_NO_ERROR;
}

//-----------------------------------------------------------------------------

const char *damn::Settings::GetString( const char *name, const char *defaultvalue )
{
	const char *value;
	if( fMessage.FindString(name,&value) == B_NO_ERROR )
		return value;
	return defaultvalue;
}

//-----------------------------------------------------------------------------

bool damn::Settings::GetBool( const char *name, bool defaultvalue )
{
	bool value;
	if( fMessage.FindBool(name,&value) == B_NO_ERROR )
		return value;
	return defaultvalue;
}

void damn::Settings::SetBool( const char *name, bool value )
{
	if( fMessage.ReplaceBool(name, value) != B_NO_ERROR )
		fMessage.AddBool( name, value );
}

//-----------------------------------------------------------------------------

int32 damn::Settings::GetInt32( const char *name, int32 defaultvalue )
{
	int32 value;
	if( fMessage.FindInt32(name,&value) == B_NO_ERROR )
		return value;
	return defaultvalue;
}

void damn::Settings::SetInt32( const char *name, int32 value )
{
	if( fMessage.ReplaceInt32(name, value) != B_NO_ERROR )
		fMessage.AddInt32( name, value );
}

//-----------------------------------------------------------------------------
