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
//-------------------------------------
#include <app/Message.h>
#include <translation/TranslatorRoster.h>
//-------------------------------------
#include "TranslatorUtils.h"
//-----------------------------------------------------------------------------

status_t damn::FindTranslatorsFor( BTranslatorRoster *roster, uint32 intype, uint32 ingroup, const char *outmimetype, BMessage *result )
{
	assert( roster != NULL );
	assert( outmimetype != NULL );

	int32 translators_cnt = 0;
	translator_id *translators = NULL;
	roster->GetAllTranslators( &translators, &translators_cnt );
	for( int i=0; i<translators_cnt; i++ )
	{
		const char *translator_name;
		const char *translator_info;
		int32 translator_version;
		roster->GetTranslatorInfo( translators[i], &translator_name, &translator_info, &translator_version );

		bool supportstype = false;
		int32 informats_cnt = 0;
		const translation_format *informats = NULL;
		roster->GetInputFormats( translators[i], &informats, &informats_cnt );
		for( int iin=0; iin<informats_cnt; iin++ )
		{
			if( informats[iin].type==intype && informats[iin].group==ingroup )
			{
				supportstype = true;
				break;
			}
		}
		
		if( supportstype )
		{
			int32 outformats_cnt = 0;
			const translation_format *outformats = NULL;
			roster->GetOutputFormats( translators[i], &outformats, &outformats_cnt );
			for( int iout=0; iout<informats_cnt; iout++ )
			{
				if( strcmp(outformats[iout].MIME,outmimetype) == 0 )
				{
					result->AddInt32( "translators", translators[i] );
					result->AddInt32( "types", outformats[iout].type );
				}
			}
		}
	}

	return B_NO_ERROR;
}

//-----------------------------------------------------------------------------

