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
#ifndef DAMN_AUTOPTR_H
#define DAMN_AUTOPTR_H
//-----------------------------------------------------------------------------
#include <assert.h>
//-------------------------------------
#include <support/SupportDefs.h>
//-------------------------------------
//-----------------------------------------------------------------------------

namespace damn
{
	template<class T>
	class AutoPtr
	{
		class Link
		{
		public:
			Link( T *data=NULL ) : fData(data), fRefCnt(1) {};
			
			Link *AddRef() { fRefCnt++; return this; }
			void Release() { if(--fRefCnt==0) delete this; }
			
			T		*fData;
			uint	fRefCnt;
		private:
			~Link() { assert(fRefCnt==0); delete fData; }
		};
	
	public:
		AutoPtr( T *data=NULL ) : fLink(new Link(data)) {}
		AutoPtr( const AutoPtr &other ) : fLink(const_cast<Link<T>*>(other.fLink)->AddRef()) {};
	//	template<class TO> AutoPtr( AutoPtr<TO> &other) : fData(other.Release()) {}
		~AutoPtr() { fLink->Release(); }
		
		T &operator*() const { assert(fLink->fData!=NULL); return fLink->fData; }
		T *operator->() const { return fLink->fData; }
		T *Get() const { return fLink->fData; }
	
		operator T*() { return fLink->fData; };
		
		AutoPtr &operator=( T *data ) { fLink->Release(); fLink=new Link(data); return *this; }
		AutoPtr &operator=( const AutoPtr &other ) { fLink->Release(); fLink=const_cast<Link<T>*>(other.fLink)->AddRef(); return *this; }

		T *Release() { T *tmp=fLink->fData; fLink->fData=NULL; return tmp; }
		
	private:
		Link	*fLink;
	};

//-----------------------------------------------------------------------------

	// TODO: add ref counting
	template<class T>
	class AutoArray
	{
	public:
		AutoArray( T data[]=NULL ) : fData(data) {}
		AutoArray( AutoArray<T> &other ) : fData(other.Release()) {}
		AutoArray( int items ) : fData(new T[items]) {}
	//	template<class TO> AutoArray( AutoArray<TO> &other) : fData(other.Release()) {}
		~AutoArray() { delete[] fData; }
		
		T &operator*() const { assert(fData!=NULL); return *fData; }
		T *operator->() const { return fData; }
		T *Get() const { return fData; }
		
		operator T*() { return fData; };
		
		AutoArray &operator=( T data[] ) { delete[] fData; fData=data; return *this; }
		AutoArray &operator=( AutoArray &other ) { delete[] fData; fData=other.Release(); return *this; }
		
		T *Release() { T *tmp=fData; fData=NULL; return tmp; }
	
	private:
		T	*fData;
	};

}

//-----------------------------------------------------------------------------
#endif

