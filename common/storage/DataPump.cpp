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
//-------------------------------------
#include "DataPump.h"
//-----------------------------------------------------------------------------

damn::DataPump::DataPump() :
	fDataLock( "datapump - class lock", true )
{
	fDataReadySem = create_sem( 0, "datapump - data ready" );
	fDataWantedSem = create_sem( 0, "datapump - data wanted" );
	fDataReadyLen = 0;
	fClosed = false;
	fFlush = false;
	fTransfered = 0;
}

damn::DataPump::~DataPump()
{
	Close();
}

void damn::DataPump::Close()
{
	fClosed = true;
	delete_sem( fDataReadySem );
	delete_sem( fDataWantedSem );
}

ssize_t damn::DataPump::Read( void *buffer, size_t size )
{
	size_t readlen = 0;
	while( (readlen != size) )
	{
		if( fClosed || !fDataLock.Lock() )
			return readlen>0?readlen:B_ERROR;
		if( fDataReadyLen )
		{
			int copylen = min_c( fDataReadyLen, size-readlen );
			memcpy( (uint8*)buffer+readlen, (const void*)fDataReady, copylen );
			fDataReady = (uint8*)fDataReady + copylen;
			fDataReadyLen -= copylen;
			readlen += copylen;
			fTransfered += copylen;
		}
		bool flush = fFlush;
		fDataLock.Unlock();

		if( flush && readlen>0 )
			break;

		if( readlen != size )
		{
			release_sem( fDataWantedSem );
			acquire_sem( fDataReadySem );
		}
	}

	return readlen;
}

ssize_t damn::DataPump::Write( const void *buffer, size_t size )
{
	if( fClosed || !fDataLock.Lock() )
		return B_ERROR;

	assert( fDataReadyLen == 0 );

	fDataReady = buffer;
	fDataReadyLen = size;
	fFlush = false;
	fDataLock.Unlock();
	release_sem( fDataReadySem );

	while( 1 )
	{
		if( fClosed || !fDataLock.Lock() )
			return B_ERROR;

		size_t datalen = fDataReadyLen;
		fDataLock.Unlock();

		if( datalen == 0 )
			break;

		release_sem( fDataReadySem );
		acquire_sem( fDataWantedSem );
	}

	return size;
}

void damn::DataPump::Flush()
{
	if( fClosed || !fDataLock.Lock() )
		return;

	fFlush = true;
	release_sem( fDataReadySem );

	fDataLock.Unlock();
}

size_t damn::DataPump::Transfered()
{
	return fTransfered;
}

//-----------------------------------------------------------------------------
