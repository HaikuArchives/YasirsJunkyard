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
#ifndef DAMN_NETSTRINGENDPOINT_H
#define DAMN_NETSTRINGENDPOINT_H
//-----------------------------------------------------------------------------
#include <deque>
//-------------------------------------
#include <net/NetEndpoint.h>
#include <support/String.h>
//-------------------------------------
//-------------------------------------
#ifdef __MWERKS__
#define std
#endif
//-----------------------------------------------------------------------------

namespace damn
{
	class NetStringEndpoint
	{
	public:
		NetStringEndpoint( BNetEndpoint *endpoint );
		~NetStringEndpoint();

		void Close() { fEndpoint->Close(); }

		void SetTimeout( bigtime_t timeout ) { fTimeout=timeout; }

		BString ReceiveString();
		void SendData( const void *data, int32 len );
		void SendString( const char *string, int32 len=-1 );
		void SendString( const BString &string ) { SendString(string.String(),string.Length()); }

		BNetEndpoint *GetEndpoint() { return fEndpoint; }

	private:
		BNetEndpoint		*fEndpoint;
		bigtime_t			fTimeout;
		std::deque<uint8>	fData;
	};
}

//-----------------------------------------------------------------------------
#endif

