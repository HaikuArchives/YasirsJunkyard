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
#include <interface/Box.h>
#include <interface/Button.h>
#include <interface/Point.h>
#include <interface/Rect.h>
#include <interface/Button.h>
#include <interface/CheckBox.h>
#include <interface/MenuBar.h>
#include <interface/MenuField.h>
#include <interface/View.h>
#include <interface/ScrollBar.h>
#include <interface/Slider.h>
#include <interface/StringView.h>
#include <interface/TextControl.h>
#include <interface/Window.h>
#include <support/Debug.h>
#include <support/String.h>
//-------------------------------------
#include "misc/List.h"
//-----------------------------------------------------------------------------

namespace dle
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

			MinMax1() {};
			MinMax1( float min_, float max_ ) { SetTo(min_,max_); }
			
			void SetTo( float min_, float max_ ) { ASSERT_WITH_MESSAGE(max_>=min_, "Min must be >= max"); min=min_; max=max_; }
	};

	class MinMax2
	{
		public:
//			float hmin, hmax;
//			float vmin, vmax;
			MinMax1	horz;
			MinMax1	vert;

			MinMax2() {};
			MinMax2( float hmin_, float hmax_, float vmin_, float vmax_ ) { horz.SetTo(hmin_,hmax_); vert.SetTo(vmin_,vmax_); }
			MinMax2( const MinMax1 &horz_, const MinMax1 &vert_ ) { SetTo(horz_,vert_); }

			void SetTo( float hmin_, float hmax_, float vmin_, float vmax_ ) { horz.SetTo(hmin_,hmax_); vert.SetTo(vmin_,vmax_); }
			void SetTo( const MinMax1 &horz_, const MinMax1 &vert_ ) { horz=horz_; vert=vert_; }

			MinMax1 &Horz() { return horz; }
			MinMax1 &Vert() { return vert; }
			const MinMax1 &Horz() const { return horz; }
			const MinMax1 &Vert() const { return vert; }

			void PrintToStream() const { printf( "MinMax2(horz:%.1f-%.1f vert:%.1f-%.1f)\n",horz.min,horz.max,vert.min,vert.max); }
	};

	class Settings
	{
	public:
		Settings();
		virtual ~Settings() {}
		
		// these are recursive:
		void SetColor( const rgb_color &color );
		void UnsetColor();
		const rgb_color &GetColor() const;
		
		void SetInner( float inner );
		void SetInnerLeft( float inner );
		void SetInnerRight( float inner );
		void SetInnerTop( float inner );
		void SetInnerBottom( float inner );
		void UnsetInner();
		void UnsetInnerLeft();
		void UnsetInnerRight();
		void UnsetInnerTop();
		void UnsetInnerBottom();
		float GetInnerLeft() const;
		float GetInnerRight() const;
		float GetInnerTop() const;
		float GetInnerBottom() const;
		
		virtual const Settings *GetParentSettings() const =0;

		// these are not recursive:
		void ForceMinWidth( float minwidth ) { fHaveForcedMinWidth=true; fForcedSize.horz.min=minwidth; }
		void ForceMaxWidth( float maxwidth ) { fHaveForcedMaxWidth=true; fForcedSize.horz.max=maxwidth; }
		void ForceMinHeight( float minheight ) { fHaveForcedMinHeight=true; fForcedSize.vert.min=minheight; }
		void ForceMaxHeight( float maxheight) { fHaveForcedMaxHeight=true; fForcedSize.vert.max=maxheight; }
		void UnforceMinWidth() { fHaveForcedMinWidth=false; }
		void UnforceMinHeight() { fHaveForcedMaxWidth=false; }
		void UnforceMaxWidth() { fHaveForcedMinHeight=false; }
		void UnforceMaxHeight() { fHaveForcedMaxHeight=false; }

		void ForceWidth( const MinMax1 &minmax );
		void ForceHeight( const MinMax1 &minmax );
		void ForceSize( const MinMax2 &minmax );

		void ForceWidth( float width );
		void ForceHeight( float height );

		const MinMax2 &GetForcedSize() const;
		
		float ForcedClipHorz( float width );
		float ForcedClipVert( float height );
		MinMax1 ForcedClipHorz( const MinMax1 &minmax );
		MinMax1 ForcedClipVert( const MinMax1 &minmax );
		MinMax2 ForcedClip( const MinMax2 &minmax );

	protected:
		bool				fHaveColor;
		rgb_color			fColor;
		
		bool				fHaveInnerLeft;
		bool				fHaveInnerRight;
		bool				fHaveInnerTop;
		bool				fHaveInnerBottom;
		float				fInnerLeft;
		float				fInnerRight;
		float				fInnerTop;
		float				fInnerBottom;
		
		bool				fHaveForcedMinWidth;
		bool				fHaveForcedMaxWidth;
		bool				fHaveForcedMinHeight;
		bool				fHaveForcedMaxHeight;
		MinMax2				fForcedSize;
	};
	
	class DefaultSettings : public Settings
	{
	public:
		DefaultSettings();

		const Settings *GetParentSettings() const;
	};


	//-------------------------------------

	BRect	AlignRect( const BRect &fitrect, const MinMax2 &minmax, align_t align );
	MinMax1	SpreadCalcSize( const float *min, const float *max, int count );
	void	Spread( const float *min, const float *max, float *dstsize, const float *weight, int count, float totsize );

	//-------------------------------------

	class Object : public Settings
	{
		friend class RootView;
		friend class Window;
		friend class Group;
	public:
		static const int kMaxSize=10000;
	
		Object( BView *view );
		virtual ~Object() =0;
		
		void SetInnerSpacing( const BRect &innerspacing );
		
		BView *GetView() const;

		virtual void ReLayout();

		virtual void SetSize( const BRect &size );
		virtual MinMax2 GetMinMaxSize() =0;

		const Settings *GetParentSettings() const;

	protected:

		bool	fNeedInnerSpacing;
		BView	*fView;

	private:

		void ReLayoutParent();
	};

	//-------------------------------------

	class Group : public Object
	{
	public:
		Group( BView *view );
		virtual ~Group() =0;
		
		void SetSpacing( float spacing );
		void SetHSpacing( float spacing );
		void SetVSpacing( float spacing );

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

		float					fHSpacing;
		float					fVSpacing;

		align_t					fDefaultAlign;

		damn::List<childinfo>	fChilds;
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

	class HSplit : public BView, public Group
	{
	public:
		HSplit();
		virtual ~HSplit();

	protected:
		MinMax2 GetMinMaxSize();
		void SetSize( const BRect &size );

	private:
		void AttachedToWindow();
	};

	//-------------------------------------

	class VSplit : public BView, public Group
	{
	public:
		VSplit();
		virtual ~VSplit();

	private:
		void AttachedToWindow();

	protected:
		MinMax2 GetMinMaxSize();
		void SetSize( const BRect &size );

	private:
	};

	//-------------------------------------

	class DebugView : public BView, public Object
	{
	public:
		DebugView( const char *name, uchar r=255, uchar g=0, uchar b=0, uchar a = 255 );

	protected:
		MinMax2	GetMinMaxSize();
		void SetSize( const BRect &size );

		void Draw( BRect updateRect );

	private:
		void AttachedToWindow();

		rgb_color	fColor;
		BString		fName;
	};

	//-------------------------------------

	class AutoScrollView : public BView, public Object
	{
	public:
						AutoScrollView();

				void	AddObject( Object *object );

	protected:
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

				void	SetMinMaxSize( const MinMax2 &mm );

	protected:
				void	AttachedToWindow() { if(Parent()) SetViewColor(Parent()->ViewColor()); }

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

	class BBox : public ::BBox, public Group
	{
	public:
		BBox( const char *title, border_style border=B_FANCY_BORDER );
		virtual ~BBox();

	protected:
		MinMax2 GetMinMaxSize();
		void SetSize( const BRect &size );

	private:
		void GetFrameSize( BRect *frame, BPoint *label );
	};

	//-------------------------------------

	class BButton : public ::BButton, public Object
	{
	public:
		BButton( const char *name, const char *label, BMessage *message, uint32 flags=B_WILL_DRAW|B_NAVIGABLE/*|B_FULL_UPDATE_ON_RESIZE*/ );
		virtual ~BButton();

	protected:
		MinMax2 GetMinMaxSize();
		void SetSize( const BRect &size );

	private:
	};

	//-------------------------------------

	class BCheckBox : public ::BCheckBox, public Object
	{
	public:
		BCheckBox( const char *name, const char *label, BMessage *message, uint32 flags=B_WILL_DRAW );
		virtual ~BCheckBox();

	protected:
		MinMax2 GetMinMaxSize();
		void SetSize( const BRect &size );
				
//		void	AttachedToWindow() { SetViewColor(0,255,255); }
	private:
	};
	
	//-------------------------------------

	class BMenuBar : public ::BMenuBar, public Object
	{
	public:
		BMenuBar( const char *title, menu_layout layout=B_ITEMS_IN_ROW/*, bool resizeToFit=true*/ );
		virtual ~BMenuBar();

	protected:
		MinMax2 GetMinMaxSize();
		void SetSize( const BRect &size );

	private:
	};

	//-------------------------------------

	class BMenuField : public ::BMenuField, public Object
	{
	public:
		BMenuField( const char *name, BMenu *menu, uint32 flags=B_WILL_DRAW|B_NAVIGABLE );
		virtual ~BMenuField();

	protected:
		void FrameResized( float new_width, float new_height );
						
		MinMax2 GetMinMaxSize();
		void SetSize( const BRect &size );
	private:
	};
	
	//-------------------------------------

	class BSlider : public ::BSlider, public Object
	{
	public:
		BSlider( const char *name, const char *label, BMessage *message, int32 minvalue, int maxvalue, thumb_style thumbtype=B_BLOCK_THUMB, uint32 flags=B_NAVIGABLE|B_WILL_DRAW|B_FRAME_EVENTS );
		virtual ~BSlider();

	protected:
		MinMax2 GetMinMaxSize();
		void SetSize( const BRect &size );
		
//		void	AttachedToWindow() { SetViewColor(0,255,255); }

	private:
	};

	//-------------------------------------

	class BStringView : public ::BStringView, public Object
	{
	public:
		BStringView( const char *name, const char *text, uint32 flags=B_WILL_DRAW );
		virtual ~BStringView();

	protected:
		MinMax2 GetMinMaxSize();
		void SetSize( const BRect &size );
				
//		void	AttachedToWindow() { SetViewColor(0,255,255); }
	private:
	};
	
	//-------------------------------------

	class BTextControl : public ::BTextControl, public Object
	{
	public:
		BTextControl( const char *name, const char *initial_text, BMessage *message, uint32 flags=B_WILL_DRAW|B_NAVIGABLE );
		virtual ~BTextControl();

	protected:
		MinMax2 GetMinMaxSize();
		void SetSize( const BRect &size );
				
//		void	AttachedToWindow() { SetViewColor(0,255,255); }
	private:
	};

	//-------------------------------------

	class Root : public Settings
	{
	public:
		Root();
		virtual ~Root() =0;

		virtual	void ReLayout() =0;

		const Settings *GetParentSettings() const { return NULL; }

	private:
	};

	class RootView : public BView, public Root
	{
	public:
		RootView( BRect frame, uint32 resizeMask, uint32 flags );
		virtual ~RootView();
		
		void AddObject( Object *object );

		void ReLayout();

	protected:
		void FrameResized( float new_width, float new_height );

	private:
		void ResizeChild();

		Object		*fRoot;
	};

	class Window : public BWindow, public Root
	{
	public:
		Window( BRect frame, const char *title, window_look look, window_feel feel, uint32 flags, uint32 workspace = B_CURRENT_WORKSPACE );
		virtual ~Window();
		
		void AddObject( Object *object );

		void ReLayout();

	protected:
		void FrameResized( float new_width, float new_height );

	private:
		Object		*fRoot;
	};

}

//-----------------------------------------------------------------------------
#endif

