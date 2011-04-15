/********************************************************************************
	Copyright 2004 Sjaak Priester	

	This file is part of Tinter.

	Tinter is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.

	Tinter is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with Tinter; if not, write to the Free Software
	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
********************************************************************************/

// QColorQuantizer
//
// MFC class to reduce colors of GDI+ Gdiplus::Bitmap, using the Octree algorithm or
// the Median Cut algorithm.
// Use at your own risk. Comments welcome.
//
// Version 1.0 (c) 2004, Sjaak Priester, Amsterdam.
// mailto:sjaak@sjaakpriester.nl

#pragma once

#pragma warning (disable: 4201)	// nameless struct/union

// Special windows message.
// After MakeQuantized() is finished, QColorQuantizer posts this message to the
// associated window pMsgWnd.
// LPARAM contains a Gdiplus::Bitmap *, pointing to a new 8 bpp indixed Gdiplus::Bitmap. The window
// gets ownership of the Gdiplus::Bitmap and is responsible for it's destruction.

class QColorQuantizer
{
public:
	enum Mode
	{
		HalfTone,
		WebSafe = HalfTone,
		Octree
	};
	QColorQuantizer(void);
	virtual ~QColorQuantizer(void);

	Gdiplus::Bitmap * GetQuantized(Gdiplus::Image * pSource, Mode mode, UINT nMaxColors = 256);
	// Return an 8bpp indexed Gdiplus::Bitmap, based on pSource.
	//
	// Parameters:
	// - pSource	points to source image, not necessarily Gdiplus::Bitmap;
	// - mode		one of the values of enum Mode;
	// - nMaxColors	maximum number of colors in returned Gdiplus::Bitmap.
	//
	// Return:
	// - non-NULL	new 8bpp indexed Gdiplus::Bitmap, user takes ownership and should delete;
	// - NULL		something went wrong, probably lack of memory.
	//
	// Remarks for different modes:
	// - WebSafe, HalfTone	Two names for the same mode. Returned Gdiplus::Bitmap has 216 colors of the
	//						GDI/GDI+ standard halftone palette. nMaxColors not used. Ultrafast.
	// - Octree				nMaxColors >= 8 and nMaxColors <= 256. Returned Gdiplus::Bitmap might have
	//						less than nMaxColors. Good result, relatively fast.

	void MakeQuantized(Gdiplus::Image * pSource, Mode mode, UINT nMaxColors = 256);
	// Creates an 8bpp indexed Gdiplus::Bitmap in a separate working thread. Returns immediately.
	//
	// Parameters:
	// - pSource	points to source image, not necessarily Gdiplus::Bitmap;
	// - mode		one of the values of enum Mode;
	// - pMsgWnd	points to CWnd that will receive message when Gdiplus::Bitmap is completed;
	// - nMaxColors	maximum number of colors in returned Gdiplus::Bitmap;
	// - message	Windows message sent to pMsgWnd.

	void Stop(void);	// Stop MakeQuantized's working thread.
//	bool IsCalculating()	{ return m_pThread != NULL; }

protected:
	bool QuantizeWebSafe(Gdiplus::Bitmap * pSource , Gdiplus::Bitmap * pDest);
	bool QuantizeOctree(Gdiplus::Bitmap * pSource, Gdiplus::Bitmap * pDest, UINT nMaxColors = 256);
	// Worker functions GetQuantized() delegates to.
	// - pDest should point to a new 8 bpp indexed Gdiplus::Bitmap with size of source.

	// Helper class for QuantizeOctree
	class OctreeNode
	{
	public:
		OctreeNode(UINT level, OctreeNode * pHead, UINT& nLeafs);
		~OctreeNode();
		void SetColor(UINT r, UINT g, UINT b, OctreeNode * pHead, UINT& nLeafs);
		void GetQuantized(UINT& nLeafs);
		UINT ChildIndex(UINT r, UINT g, UINT b) const;
		UINT GetPaletteIndex(UINT r, UINT g, UINT b) const;
		void FillPalette(Gdiplus::ColorPalette * pPal, UINT& index);

		bool ReduceBefore(const OctreeNode& other) const
		{
			// true if other should be reduced before we are
			if (m_nLevel == other.m_nLevel) return m_nPixels < other.m_nPixels;
			return m_nLevel < other.m_nLevel;
		}
		void UpdatePosition();
		void Remove();
		void InsertBefore(OctreeNode * pNext);
		void InsertAfter(OctreeNode * pPrev)	{ InsertBefore(pPrev->m_pNext); }

		bool m_bLeaf;
		UINT m_nLevel;
		OctreeNode * m_pChild[8];	// pointers in octree
		union	// spare some memory by declaring variables not used together as union
		{
			UINT m_nPixels;
			UINT m_iPaletteIndex;	// only used in leaf, after FillPalette()
		};
		union
		{
			struct
			{
				OctreeNode * m_pNext;	// pointers in priority list,
				OctreeNode * m_pPrev;	// only used in non-leaf nodes
			};
			struct
			{
				UINT m_totalR;		// color totals, only used in leaf nodes
				UINT m_totalG;
				UINT m_totalB;
			};
		};
	};

protected:
	Gdiplus::Bitmap * Calculate(void);
	void Notify(Gdiplus::Bitmap * pResult);
	bool m_bStop;
	UINT m_Message;
	Gdiplus::Image *m_pSource;
	Mode m_Mode;
	UINT m_nMaxColors;

	static UINT ThreadProc(LPVOID pParam);
};
