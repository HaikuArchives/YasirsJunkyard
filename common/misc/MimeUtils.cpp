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
#include <storage/Mime.h>
//-------------------------------------
#include "MimeUtils.h"
//-----------------------------------------------------------------------------

/**
 * Find the mimetype for \a filename by looking at the extension.
 *
 * The mimetype is found by searcing through the BeOS buildin mimetype database.
 */
BMimeType *damn::GuessMimetypeForExtension( const char *filename )
{
	int filename_len = strlen( filename );

	BMessage types;
	BMimeType::GetInstalledTypes( &types );
	
	BMimeType *mimetype = new BMimeType;

	const char *mimestring;
	for( int i=0; types.FindString("types",i,&mimestring)==B_NO_ERROR; i++ )
	{
		mimetype->SetTo( mimestring );

		BMessage extensions;
		mimetype->GetFileExtensions( &extensions );

		const char *extensionstring;
		for( int iext=0; extensions.FindString("extensions",iext,&extensionstring)==B_NO_ERROR; iext++ )
		{
			int extensionstring_len = strlen( extensionstring );
			if( filename_len >= extensionstring_len )
			{
				if( strcmp(extensionstring,filename+filename_len-extensionstring_len) == 0 )
				{
					return mimetype;
				}
			}
		}
	}

	delete mimetype;
	return NULL;
}

//-----------------------------------------------------------------------------
