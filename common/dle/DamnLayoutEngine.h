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
#ifndef DAMN_DAMNLAYOUTENGINE_H
#define DAMN_DAMNLAYOUTENGINE_H
//-----------------------------------------------------------------------------
#include <assert.h>
#include <stdio.h>
//-------------------------------------
#include <interface/Button.h>
#include <interface/Point.h>
#include <interface/Rect.h>
#include <interface/Button.h>
#include <interface/CheckBox.h>
#include <interface/MenuField.h>
#include <interface/View.h>
#include <interface/ScrollBar.h>
#include <interface/StringView.h>
#include <interface/Window.h>
//-------------------------------------
#include "misc/List.h"
//-----------------------------------------------------------------------------

namespace damn
{

	enum align_t {
		HMASK=0x0f, LEFT=0x01, HCENTER=0x00, RIGHT=0x02,  HDEFAULT=0x03, 
		VMASK=0xf0, TOP=0x10,  VCENTER=0x00, BOTTOM=0x20, VDEFAULT=0x30,
		CENTER=HCENTER|VCENTER, DEFAULT=HDEFAULT|VDEFAULT };

	//-------------------------------------

	class MinMax1
	{
		public:
			float min, max;

			MinMax1( float min_, float max_ ) { min=min_; max=max_; }
	};

	class MinMax2
	{
		public:
			float hmin, hmax;
			float vmin, vmax;

						MinMax2() {};
						MinMax2( float hmin_, float hmax_, float vmin_, float vmax_ ) { assert(hmax_>=hmin_); assert(vmax_>=vmin_); hmin=hmin_; hmax=hmax_; vmin=vmin_; vmax=vmax_; }

		void			PrintToStream() const { printf( "MinMax2(hmin:%.1f, hmax:%.1f, vmin:%.1f, vmax:%.1f)\n",hmin,hmax,vmin,vmax); }
	};

	//-------------------------------------

	BRect	AlignRect( const BRect &fitrect, const MinMax2 &minmax, align_t align );
	MinMax1	SpreadCalcSize( const float *min, const float *max, int count );
	void	Spread( float *min, float *max, float *dstsize, float *weight, int count, float totsize );

	//-------------------------------------

	class Object
	{
	public:
						Object( BView *view );
		virtual 		~Object() =0;
		
		virtual MinMax2	GetMinMaxSize() =0;
		virtual void	SetSize( const BRect &size );
		
				BView	*GetView() const;

				Object	*GetParent() const;

		virtual	void	ReLayout();

	protected:
				void	ReLayoutParent();

		BView	*fView;
	};

	//-------------------------------------

	class Group : public Object
	{
	public:
						Group( BView *view );
		virtual			~Group() =0;
		
				void	SetSpacing( float spacing );
				void	SetHSpacing( float spacing );
				void	SetHSpacing( float left, float mid, float right );
				void	SetVSpacing( float spacing );
				void	SetVSpacing( float top, float mid, float bottom );
		
	protected:
		float	fHSpacingLeft, fHSpacingMid, fHSpacingRight;
		float	fVSpacingTop, fVSpacingMid, fVSpacingBottom;
	};

	//-------------------------------------

//	class Grid : public Group
//	{
//	};

	//-------------------------------------

//	class Table : public Group
//	{
//	};

	//-------------------------------------

	class Split : public BView, public Group
	{
	public:
						Split( float spacing=0.0f, align_t align=CENTER );
		virtual			~Split() =0;

				void	AttachedToWindow();

				void	AddObject( Object *child, float weight ) { AddObject(child,DEFAULT,weight); }
		virtual	void	AddObject( Object *child, align_t align=DEFAULT, float weight=1.0f );

	protected:
		struct childinfo
		{
			Object	*object;
			float	weight;
			align_t	align;
		};

		align_t			fDefaultAlign;

		damn::List<childinfo>	fChilds;
	};

	//-------------------------------------

	class HSplit : public Split
	{
	public:
						HSplit( float spacing=0, align_t align=CENTER );
						~HSplit();

				MinMax2	GetMinMaxSize();
				void	SetSize( const BRect &size );
	private:
	};

	//-------------------------------------

	class VSplit : public Split
	{
	public:
						VSplit( float spacing=0, align_t align=CENTER );
						~VSplit();

				MinMax2	GetMinMaxSize();
				void	SetSize( const BRect &size );
	private:
	};

	//-------------------------------------

	class DebugView : public BView, public Object
	{
	public:
						DebugView( const char *name, uchar r=255, uchar g=0, uchar b=0, uchar a = 255 );

				void	SetMinMaxSize( const MinMax2 &mm );

				MinMax2	GetMinMaxSize();
				void	SetSize( const BRect &size );
	private:
		MinMax2		fMinMax;
		char		fName[128];
	};

	//-------------------------------------

	class AutoScrollView : public BView, public Object
	{
	public:
						AutoScrollView();

				void	AddObject( Object *object );

				MinMax2	GetMinMaxSize();
				void	SetSize( const BRect &size );
	private:
		Object		*fObject;

		BScrollBar	*fHorScrollBar;
		BScrollBar	*fVerScrollBar;
		BView		*fScrollView;
	};

	//-------------------------------------

	class Space : public BView, public Object
	{
	public:
						Space();

				void	AttachedToWindow() { if(Parent()) SetViewColor(Parent()->ViewColor()); }

				void	SetMinMaxSize( const MinMax2 &mm );

				MinMax2	GetMinMaxSize();
				void	SetSize( const BRect &size );
	private:
		MinMax2		fMinMax;
	};

	class HSpace : public Space
	{
	public:
						HSpace( float min=0, float max=1000000 ) { SetMinMaxSize(MinMax2(min,max,0,0)); }
	};

	class VSpace : public Space
	{
	public:
						VSpace( float min=0, float max=1000000 ) { SetMinMaxSize(MinMax2(0,0,min,max)); }
	};

	//-------------------------------------

	class BButton : public ::BButton, public Object
	{
	public:
						BButton( const char *name, const char *label, BMessage *message, uint32 flags=B_WILL_DRAW|B_NAVIGABLE/*|B_FULL_UPDATE_ON_RESIZE*/ );
						~BButton();

				MinMax2	GetMinMaxSize();
				void	SetSize( const BRect &size );
	private:
	};

	//-------------------------------------

	class BCheckBox : public ::BCheckBox, public Object
	{
	public:
						BCheckBox( const char *name, const char *label, BMessage *message, uint32 flags=B_WILL_DRAW );
						~BCheckBox();

				MinMax2	GetMinMaxSize();
				void	SetSize( const BRect &size );
				
//				void	AttachedToWindow() { SetViewColor(0,255,255); }
	private:
	};
	
	//-------------------------------------

	class BMenuField : public ::BMenuField, public Object
	{
	public:
						BMenuField( const char *name, BMenu *menu, uint32 flags=B_WILL_DRAW|B_NAVIGABLE );
						~BMenuField();

				void	FrameResized( float new_width, float new_height );
						
				MinMax2	GetMinMaxSize();
				void	SetSize( const BRect &size );
	private:
	};
	
	//-------------------------------------

	class BStringView : public ::BStringView, public Object
	{
	public:
						BStringView( const char *name, const char *text, uint32 flags=B_WILL_DRAW );
						~BStringView();

				MinMax2	GetMinMaxSize();
				void	SetSize( const BRect &size );
				
//				void	AttachedToWindow() { SetViewColor(0,255,255); }
	private:
	};
	
	//-------------------------------------

	class Root
	{
	public:
		virtual	void	ReLayout() =0;
	private:
	};

	class RootView : public Root, public BView
	{
	public:
						RootView( BRect frame, uint32 resizeMask, uint32 flags );
						~RootView();
		
				void	FrameResized( float new_width, float new_height );

				void	AddObject( Object *object );

				void	ReLayout();
	private:
				void	ResizeChild();

		Object		*fRoot;
	};

	class Window : /*public Root,*/ public BWindow
	{
	public:
						Window( BRect frame, const char *title, window_look look, window_feel feel, uint32 flags, uint32 workspace = B_CURRENT_WORKSPACE );
		virtual			~Window();
		
				void	FrameResized( float new_width, float new_height );

				void	AddObject( Object *object );

				void	ReLayout() { assert(0); }
	private:
		Object		*fRoot;
	};

}

//-----------------------------------------------------------------------------
#endif

