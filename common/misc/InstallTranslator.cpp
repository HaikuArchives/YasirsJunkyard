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
#include <interface/Alert.h>
#include <kernel/fs_attr.h>
#include <storage/AppFileInfo.h>
#include <storage/File.h>
#include <storage/FindDirectory.h>
#include <storage/Path.h>
#include <support/String.h>
//-------------------------------------
#include "InstallTranslator.h"
#include "misc/AutoPtr.h" // HUH??!? if this line is not after InstallTranslator.h, then the app does not link...
//-----------------------------------------------------------------------------

static status_t GetAppVersion( BAppFileInfo *appfile, version_info *version )
{
	return appfile->GetVersionInfo( version, B_APP_VERSION_KIND );
}

static status_t GetAppVersion( BFile *file, version_info *version )
{
	BAppFileInfo appfile( file );
	status_t status = file->InitCheck();
	if( status != B_NO_ERROR ) return status;
	return GetAppVersion( &appfile, version );
}

static status_t GetAppVersion( const entry_ref &entry, version_info *version )
{
	BFile file( &entry, B_READ_ONLY );
	status_t status = file.InitCheck();
	if( status != B_NO_ERROR ) return status;
	if( !file.IsReadable() ) return B_IO_ERROR;
	return GetAppVersion( &file, version );
}
	
static status_t GetAppVersion( const char *path, version_info *version )
{
	BFile file( path, B_READ_ONLY );
	status_t status = file.InitCheck();
	if( status != B_NO_ERROR ) return status;
	if( !file.IsReadable() ) return B_IO_ERROR;
	return GetAppVersion( &file, version );
}

static int CompareVersionInfo( const version_info &info1, const version_info &info2 )
{
	if( info1.major < info2.major )				return -1;
	else if( info1.major > info2.major )		return 1;

	if( info1.middle < info2.middle )			return -1;
	else if( info1.middle > info2.middle )		return 1;

	if( info1.minor < info2.minor )				return -1;
	else if( info1.minor > info2.minor )		return 1;

	if( info1.internal < info2.internal )		return -1;
	else if( info1.internal > info2.internal )	return 1;
	
	return 0;
}

static status_t CopyFile( const entry_ref &src, const char *dst )
{
	status_t status;

	BFile srcfile( &src, B_READ_ONLY );
	status = srcfile.InitCheck();
	if( status != B_NO_ERROR ) return status;
	if( !srcfile.IsReadable() ) return B_IO_ERROR;
	
	// This is omportant: if somebody is using the old file, while we overwrite it, bad things might happend...
	remove( dst );
	
	BFile dstfile( dst, B_WRITE_ONLY|B_CREATE_FILE|B_ERASE_FILE );
	status = dstfile.InitCheck();
	if( status != B_NO_ERROR ) return status;
	if( !dstfile.IsWritable() ) return B_IO_ERROR;
//	dstfile.Lock(); // Make sure that nobody tries to use the file befoes we're done with it.

	// Copy data:
	int buffersize = 64*1024;
	damn::AutoArray<uint8> buffer( buffersize );
	ssize_t readlen;
	while( (readlen=srcfile.Read(buffer,buffersize)) > 0 )
	{
		ssize_t writelen = dstfile.Write( buffer, readlen );
		if( writelen != readlen )
			return B_IO_ERROR;
	}
	if( readlen < 0 )
		return B_IO_ERROR;

	// Copy attributes:
	srcfile.RewindAttrs();
	char attrname[B_ATTR_NAME_LENGTH];
	while( srcfile.GetNextAttrName(attrname) == B_NO_ERROR )
	{
		attr_info attrinfo;
		status = srcfile.GetAttrInfo( attrname, &attrinfo );
		if( status != B_NO_ERROR ) return status;
#if 0	
		off_t offset=0;
		ssize_t readlen;
		while( (readlen=srcfile.ReadAttr(attrname, attrinfo.type, offset, buffer, buffersize)) > 0 )
		{
			ssize_t writelen = dstfile.WriteAttr( attrname, attrinfo.type, offset, buffer, readlen );
			printf( "%s>>>%ld/%ld\n", attrname, readlen, writelen );
			printf( "%d\n", offset );
			if( readlen != writelen )
				return B_IO_ERROR;
			offset += readlen;
		}
#else
		damn::AutoArray<uint8> attrbuffer( attrinfo.size );
		ssize_t readlen = srcfile.ReadAttr( attrname, attrinfo.type, 0, attrbuffer, attrinfo.size );
		if( readlen != attrinfo.size ) return B_IO_ERROR;
		ssize_t writelen = dstfile.WriteAttr( attrname, attrinfo.type, 0, attrbuffer, attrinfo.size );
		if( writelen != attrinfo.size ) return B_IO_ERROR;
#endif
	}

	return B_NO_ERROR;
}
	
status_t damn::InstallTranslator( const char *translatorname, const entry_ref &fileentry )
{
	status_t status;
	
	BPath dstpath;
	status = find_directory( B_USER_TRANSLATORS_DIRECTORY, &dstpath, true );
	if( status != B_NO_ERROR ) return status;
	status = dstpath.Append( fileentry.name );
	if( status != B_NO_ERROR ) return status;
	
	version_info srcinfo = {0,0,0,0,0};
	version_info dstinfo = {0,0,0,0,0};
	GetAppVersion( fileentry, &srcinfo );
	GetAppVersion( dstpath.Path(), &dstinfo );
	
//	printf( ">%ld.%ld.%ld.%ld.%ld\n", srcinfo.major, srcinfo.middle, srcinfo.minor, srcinfo.variety, srcinfo.internal );
//	printf( "<%ld.%ld.%ld.%ld.%ld\n", dstinfo.major, dstinfo.middle, dstinfo.minor, dstinfo.variety, dstinfo.internal );
	
	if( CompareVersionInfo(srcinfo,dstinfo) < 0 )
	{
		BString string;
		string << "Do you want to overwrite the existing translator?\n\n";
		string << "Version of this translator: " << srcinfo.major << "." << srcinfo.middle << "." << srcinfo.minor << "." << srcinfo.internal << "\n";
		string << "Version of installed translator: " << dstinfo.major << "." << dstinfo.middle << "." << dstinfo.minor << "." << dstinfo.internal << "\n";
		
		if( (new BAlert( translatorname, string.String(), "Yes", "No" ))->Go() == 1 )
			return B_NO_ERROR;
	}

	return CopyFile( fileentry, dstpath.Path() );
}

//-----------------------------------------------------------------------------
