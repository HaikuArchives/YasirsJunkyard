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
#ifndef DAMN_DLE_CORE_H
#define DAMN_DLE_CORE_H
//-----------------------------------------------------------------------------
//-------------------------------------
#include <interface/GraphicsDefs.h>
#if DEBUG
#include <support/Debug.h>
#endif
//-------------------------------------
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
			
#if DEBUG
			void SetTo( float min_, float max_ ) { ASSERT_WITH_MESSAGE(max_>=min_, "Min must be >= max"); min=min_; max=max_; }
#else
			void SetTo( float min_, float max_ ) { min=min_; max=max_; }
#endif
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

			void PrintToStream() const;
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
		
		void InnerSpacing( bool spacing ) { fWantInnerSpacing=spacing; }
		bool WantInnerSpacing() const { return fWantInnerSpacing; }

	protected:
		bool				fHaveColor;
		rgb_color			fColor;

		bool				fWantInnerSpacing;
		
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

	class Root : public Settings
	{
	public:
		Root();
		virtual ~Root() =0;

		virtual	void ReLayout() =0;

		const Settings *GetParentSettings() const { return NULL; }

	private:
	};

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
		
		bool SendMouseEventToParent() const;

	protected:
		BView	*fView;

	private:
		void ReLayoutParent();
	};

	BRect AlignRect( const BRect &fitrect, const MinMax2 &minmax, align_t align );
	MinMax1	SpreadCalcSize( const float *min, const float *max, int count );
	void Spread( const float *min, const float *max, float *dstsize, const float *weight, int count, float totsize );
}

//-----------------------------------------------------------------------------
#endif

