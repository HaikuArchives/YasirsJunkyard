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
#ifndef DAMN_DLE_GROUP_H
#define DAMN_DLE_GROUP_H
//-----------------------------------------------------------------------------
//-------------------------------------
//-------------------------------------
#include "misc/List.h"

#include "Core.h"
//-----------------------------------------------------------------------------

namespace dle
{
	class Group : public Object
	{
	public:
		Group( BView *view );
		virtual ~Group() =0;
		
//		void SetSpacing( float spacing );
//		void SetHSpacing( float spacing );
//		void SetVSpacing( float spacing );

		void SetDefaultAlign( align_t align );

		void AddObject( Object *child, float weight ) { AddObject(child,DEFAULT,weight); }
		virtual	void AddObject( Object *child, align_t align=DEFAULT, float weight=1.0f );

	protected:
		struct childinfo
		{
			Object	*object;
			bool	objectisgroup;
			float	weight;
			align_t	align;
		};

//		float					fHSpacing;
//		float					fVSpacing;

		align_t					fDefaultAlign;

		damn::List<childinfo>	fChilds;
	};
}

//-----------------------------------------------------------------------------
#endif

