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
#include "AutoPtr.h"
#include "StorageUtils.h"
//-----------------------------------------------------------------------------

status_t damn_Copy( BDataIO *in, BDataIO *out, ssize_t *copiedbytes, size_t buffersize )
{
	return damn::Copy( in, out, copiedbytes, buffersize );
}

status_t damn::Copy( BDataIO *in, BDataIO *out, ssize_t *copiedbytes, size_t buffersize )
{
	assert( in != NULL );
	assert( out != NULL );
	assert( buffersize > 0 );

	damn::AutoArray<uint8> buffer( buffersize );
	
	if( copiedbytes ) *copiedbytes = 0;

	ssize_t readsize;
	while( (readsize=in->Read(buffer, buffersize)) > 0 )
	{
		ssize_t writesize = out->Write( buffer, readsize );
		if( writesize>0 && copiedbytes ) *copiedbytes += writesize;
		if( writesize != readsize )
			return B_IO_ERROR;
	}
	if( readsize < 0 )
		return B_IO_ERROR;

	return B_OK;
}

//-----------------------------------------------------------------------------
