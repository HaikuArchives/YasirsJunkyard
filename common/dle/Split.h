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
#ifndef DAMN_DLE_SPLIT_H
#define DAMN_DLE_SPLIT_H
//-----------------------------------------------------------------------------
//-------------------------------------
#include <interface/View.h>
//-------------------------------------
#include "Group.h"
//-----------------------------------------------------------------------------

namespace dle
{
	class HSplit : public BView, public Group
	{
	public:
		HSplit();
		virtual ~HSplit();

	protected:
		MinMax2 GetMinMaxSize();
		void SetSize( const BRect &size );
		
		void MouseDown( BPoint where ) { if(Parent()) Parent()->MouseDown(ConvertToParent(where)); }
		void MouseUp( BPoint where ) { if(Parent()) Parent()->MouseUp(ConvertToParent(where)); }

	private:
		void AttachedToWindow();
	};

	//-------------------------------------

	class VSplit : public BView, public Group
	{
	public:
		VSplit();
		virtual ~VSplit();

	protected:
		MinMax2 GetMinMaxSize();
		void SetSize( const BRect &size );

		void MouseDown( BPoint where ) { if(Parent()) Parent()->MouseDown(ConvertToParent(where)); }
		void MouseUp( BPoint where ) { if(Parent()) Parent()->MouseUp(ConvertToParent(where)); }

	private:
		void AttachedToWindow();
	};
}

//-----------------------------------------------------------------------------
#endif

