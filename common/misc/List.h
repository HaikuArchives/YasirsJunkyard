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
#ifndef DAMN_LIST_H
#define DAMN_LIST_H
//-----------------------------------------------------------------------------
//-------------------------------------
#include <support/List.h>
//-------------------------------------
//-----------------------------------------------------------------------------

namespace damn
{

	template<class T>
	class List
	{
	public:
				List(int32 itemsPerBlock = 20)						: fList(itemsPerBlock) {};
				List(const List<T> &newItems)						: fList(newItems) {};
	virtual		~List()												{};
	
		List<T>	&operator=(const List<T> &from)						{ return	fList=from; }
	
		// Adding and removing items.
		bool	AddItem(T *item)									{ return	fList.AddItem((void*)item); }
		bool	AddItem(T *item, int32 atIndex)						{ return	fList.AddItem((void*)item,atIndex); }
		bool	AddList(List<T> *newItems)							{ return	fList.AddList(newItems); }
		bool	AddList(List<T> *newItems, int32 atIndex)			{ return	fList.AddList(newItems,atIndex); }
		bool	RemoveItem(T *item)									{ return	fList.RemoveItem(item); }
		T		*RemoveItem(int32 index)							{ return	(T*)fList.RemoveItem(index); }
		bool	RemoveItems(int32 index, int32 count)				{ return	fList.RemoveItems(index,count); }
		bool	ReplaceItem(int32 index, T *newItem)				{ return	fList.ReplaceItem(index,newItem); }
		void	MakeEmpty()											{ 			fList.MakeEmpty(); }
	
		// Reordering items.
		void	SortItems(int (*cmp)(const T *, const T *))			{ 			fList.SortItems(cmp); }
		bool	SwapItems(int32 indexA, int32 indexB)				{ return	fList.SwapItems(indexA,indexB); }
		bool	MoveItem(int32 fromIndex, int32 toIndex)			{ return	fList.MoveItems(fromIndex,toIndex); }
	
		// Retrieving items.
		T		*ItemAt(int32 index) const							{ return	(T*)fList.ItemAt(index); }
		T		*ItemAtFast(int32 index) const						{ return	(T*)fList.ItemAtFast(index); }
		T		*FirstItem() const									{ return	(T*)fList.FirstItem(); }
		T		*LastItem() const									{ return	(T*)fList.LastItem(); }
		T		*Items() const										{ return	(T*)fList.Items(); }
	
		// Querying the list.
		bool	HasItem(T *item) const								{ return	fList.hasItem(item); }
		int32	IndexOf(T *item) const								{ return	fList.IndexOf(item); }
		int32	CountItems() const									{ return	fList.CountItems(); }
		bool	IsEmpty() const										{ return	fList.IsEmpty(); }
	
		// Iterating over the list.
		void	DoForEach(bool (*func)(T *))						{ return	fList.DoForEach(func); }
		void	DoForEach(bool (*func)(void *, void *), void *user)	{ return	fList.DoForEach(func,user); }
		
	private:
		BList	fList;
	};

}

//-----------------------------------------------------------------------------
#endif
