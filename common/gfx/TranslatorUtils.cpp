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

TranslatorBitmap damn::ConvertToBigEndian( const TranslatorBitmap &native )
{
	TranslatorBitmap bendian;
	bendian.magic			= B_HOST_TO_BENDIAN_INT32( native.magic );
	bendian.bounds.left		= B_HOST_TO_BENDIAN_FLOAT( native.bounds.left );
	bendian.bounds.top		= B_HOST_TO_BENDIAN_FLOAT( native.bounds.top );
	bendian.bounds.right	= B_HOST_TO_BENDIAN_FLOAT( native.bounds.right );
	bendian.bounds.bottom	= B_HOST_TO_BENDIAN_FLOAT( native.bounds.bottom );
	bendian.rowBytes		= B_HOST_TO_BENDIAN_INT32( native.rowBytes );
	bendian.colors			= (color_space)B_HOST_TO_BENDIAN_INT32( (uint32)native.colors );
	bendian.dataSize		= B_HOST_TO_BENDIAN_INT32( native.dataSize );
	return bendian;
}

TranslatorBitmap damn::ConvertToNative( const TranslatorBitmap &bendian )
{
	TranslatorBitmap native;
	native.magic			= B_BENDIAN_TO_HOST_INT32( bendian.magic );
	native.bounds.left		= B_BENDIAN_TO_HOST_FLOAT( bendian.bounds.left );
	native.bounds.top		= B_BENDIAN_TO_HOST_FLOAT( bendian.bounds.top );
	native.bounds.right		= B_BENDIAN_TO_HOST_FLOAT( bendian.bounds.right );
	native.bounds.bottom	= B_BENDIAN_TO_HOST_FLOAT( bendian.bounds.bottom );
	native.rowBytes			= B_BENDIAN_TO_HOST_INT32( bendian.rowBytes );
	native.colors			= (color_space)B_BENDIAN_TO_HOST_INT32( (uint32)bendian.colors );
	native.dataSize			= B_BENDIAN_TO_HOST_INT32( bendian.dataSize );
	return native;
}

bool damn::IsTranslatorBitmap( BPositionIO *io )
{
	TranslatorBitmap header;
	
	if( io->Read(&header, sizeof(header)) != sizeof(header) )
		return B_IO_ERROR;
	
	header = ConvertToNative( header );
	
	if( header.magic != B_TRANSLATOR_BITMAP )
		return B_ERROR;
	
	if( header.bounds.left>header.bounds.right || header.bounds.top>header.bounds.bottom )
		return B_ERROR;

	if( header.dataSize != header.rowBytes * (header.bounds.IntegerHeight()+1) )
		return B_ERROR;
		
	return B_OK;
}

//-----------------------------------------------------------------------------
