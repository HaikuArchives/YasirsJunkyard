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
#ifndef DAMN_SETTINGS_H
#define DAMN_SETTINGS_H
//-----------------------------------------------------------------------------
//-------------------------------------
#include <app/Message.h>
#include <support/String.h>
//-------------------------------------
//-----------------------------------------------------------------------------

namespace damn
{
	class Settings
	{
	public:
					Settings( const char *filename, bool writeondestruct=false );
					Settings( const BMessage &message );
					Settings( const Settings &settings );
					~Settings();
					
		status_t	Write( const char *filename=NULL );

		const char	*GetString( const char *name, const char *defaultvalue=NULL );
		bool		GetBool( const char *name, bool defaultvalue );
		int32		GetInt32( const char *name, int32 defaultvalue );

		void		SetBool( const char *name, bool value );
		void		SetInt32( const char *name, int32 value );

	private:
		Settings 	&operator=( const Settings &settings );

		bool		fWriteOnDestruct;
		BString		fFilename;
				
		BMessage	fMessage;
	};
}

//-----------------------------------------------------------------------------
#endif
