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
#ifndef DAMN_ENUMMAPPER_H
#define DAMN_ENUMMAPPER_H
//-----------------------------------------------------------------------------
//-------------------------------------
//#include <app/Message.h>
//#include <support/String.h>
//-------------------------------------
#include "List.h"
//-----------------------------------------------------------------------------

namespace damn
{
	template<class T>
	class EnumMapping
	{
	public:
		EnumMapping() {};
		~EnumMapping()
		{
			for( int i=0; i<fKeyList.CountItems(); i++ )
				delete fKeyList.ItemAtFast( i );
		}
		
		void AddEnum( int e_num, T val )
		{
			keyval *k = new keyval( e_num, val );
			fKeyList.AddItem( k );
		}
	
		status_t GetVal( int e_num, T *val, int *index )
		{
			for( int i=0; i<fKeyList.CountItems(); i++ )
			{
				keyval *k = fKeyList.ItemAtFast( i );
				if( k->key == e_num )
				{
					if( val )
						*val = k->val;
					if( index )
						*index = i;
					return B_OK;
				}
			}
			return B_ERROR;
		}
	
		status_t GetEnum( T val, int *e_num, int *index )
		{
			for( int i=0; i<fKeyList.CountItems(); i++ )
			{
				keyval *k = fKeyList.ItemAtFast( i );
				if( k->val == val )
				{
					if( e_num )
						*e_num = k->key;
					if( index )
						*index = i;
					return B_OK;
				}
			}
			return B_ERROR;
		}
	
		status_t GetNearestEnum( T val, int *e_num )
		{
			assert( e_num != NULL );
	
			if( fKeyList.CountItems() == 0 )
				return B_ERROR;
	
			keyval *best = fKeyList.ItemAtFast( 0 );
			T best_dist = best->val - val;
			if( best_dist < 0 ) best_dist = -best_dist;
			
			for( int i=1; i<fKeyList.CountItems(); i++ )
			{
				keyval *k = fKeyList.ItemAtFast( i );
				T dist = k->val - val;
				if( dist < 0 ) dist = -dist;
				if( dist < best_dist )
				{
					best = k;
					best_dist = dist;
				}
			}
	
			*e_num = best->key;
			return B_OK;
		}
	
	private:
		struct keyval { keyval(int k,T v) : key(k), val(v) {}; int key; T val;  };
		damn::List<keyval>	fKeyList;
	};
}

//-----------------------------------------------------------------------------
#endif
