/////////////////////////////////////////////////////////////////////
// PictureExWnd.cpp : Implementation of CPictureExWnd
//
// Picture displaying control with support for the following formats:
// GIF (including animated GIF87a and GIF89a), JPEG, BMP, WMF, ICO, CUR
// 
// Written by Oleg Bykov (oleg_bykoff@rsdn.ru)
// Copyright (c) 2001
//
// To use CPictureExWnd, follow these steps:
//   - place a static control on your dialog (either a text or a bitmap)
//   - change its identifier to something else (e.g. IDC_MYPIC)
//   - add a member variable of type CPictureExWnd in your dialog class
//     (don't forget to #include "PictureEx.h" and add 
//     PictureEx.h and PictureEx.cpp to your project)
//   - in OnInitDialog() call the variable's SubclassWindow functions
//     (like this:
//         HWND hWnd = GetDlgItem(IDC_MYPIC);
//         if (hWnd) m_wndBanner.SubclassWindow(hWnd);
//     )
//   - call one of the overloaded CPictureEx::Load() functions somewhere
//     (OnInitDialog is a good place to do so)
//   - if the preceding Load() succeeded call Draw()
//  
// If your parent window is not a dialog, just add a member variable of type 
// CPictureExWnd to your main window class, call CPictureExWnd::Create()
// (say, in WM_CREATE handler function), then CPictureExWnd::Load() and 
// CPictureExWnd::Draw().
//
// By default, the control initializes its background to COLOR_3DFACE
// (see CPictureEx::PrepareDC()). You can change the background by
// calling CPictureEx::SetBkColor(COLORREF) after CPictureEx::Load().
//
// I decided to leave in the class the functions to write separate frames from 
// animated GIF to disk. If you want to use them, uncomment #define GIF_TRACING 
// and an appropriate section in CPictureEx::Load(HGLOBAL, DWORD). These functions 
// won't be compiled and linked to your project unless you uncomment #define GIF_TRACING,
// so you don't have to worry.
// 
// Warning: this code hasn't been subject to a heavy testing, so
// use it on your own risk. The author accepts no liability for the 
// possible damage caused by this code.
//
//
// Version 1.0  7 Aug 2001
//              Initial release
//
// Version 1.1  6 Sept 2001
//              ATL version of the class
//
// Version 1.2  14 Oct 2001
//              - Fixed a problem with loading GIFs from resources
//                in MFC-version of the class for multi-modules apps.
//                Thanks to Ruben Avila-Carretero for finding this out.
//
//              - Got rid of waitable timer in ThreadAnimation()
//                Now CPictureEx[Wnd] works in Win95 too.
//                Thanks to Alex Egiazarov and Wayne King for the idea.
//
//              - Fixed a visual glitch of using SetBkColor.
//                Thanks to Kwangjin Lee for finding this out.
//
// Version 1.3  10 Nov 2001
//              - Fixed a DC leak. One DC leaked per each UnLoad()
//                (forgot to put a ReleaseDC() in the end of 
//                CPictureExWnd::PrepareDC() function).
//
//              - Now it is possible to set a clipping rectangle using
//                CPictureEx[Wnd]::SetPaintRect(const LPRECT) function.
//                The LPRECT parameter tells the class what portion of
//                a picture should it display. If the clipping rect is 
//                not set, the whole picture is shown.
//                Thanks to Fabrice Rodriguez for the idea.
//
//              - Added support for Stop/Draw. Now you can Stop() an
//                animated GIF, then Draw() it again, it will continue
//                animation from the frame it was stopped on. You can 
//                also know if a GIF is currently playing with the 
//                IsPlaying() function.
//             
//              - Got rid of math.h and made m_bExitThread volatile. 
//                Thanks to Piotr Sawicki for the suggestion.
//
/////////////////////////////////////////////////////////////////////
#include "../../atlheaders.h"
#include "PictureExWnd.h"
#include <math.h>
#include <process.h>

CPictureExWnd::CPictureExWnd()
{
	// check structures size
	ATLASSERT(sizeof(TGIFImageDescriptor) == 10);
	ATLASSERT(sizeof(TGIFAppExtension)    == 14);
	ATLASSERT(sizeof(TGIFPlainTextExt)    == 15);
	ATLASSERT(sizeof(TGIFLSDescriptor)    ==  7);
	ATLASSERT(sizeof(TGIFControlExt)	   ==  8);
	ATLASSERT(sizeof(TGIFCommentExt)	   ==  2);
	ATLASSERT(sizeof(TGIFHeader)		   ==  6);

	m_pGIFLSDescriptor = NULL;
	m_pGIFHeader	   = NULL;
	m_pPicture		   = NULL;
	m_pRawData		   = NULL;
	m_hThread		   = NULL;
	m_hBitmap          = NULL;
	m_hMemDC		   = NULL;

	m_hDispMemDC       = NULL;
	m_hDispMemBM       = NULL;
	m_hDispOldBM       = NULL;

	m_bIsInitialized   = FALSE;
	m_bExitThread	   = FALSE;
	m_bIsPlaying       = FALSE;
	m_bIsGIF		   = FALSE;
	m_clrBackground    = RGB(255,255,255); // white by default
	m_nGlobalCTSize    = 0;
	m_nCurrOffset	   = 0;
	m_nCurrFrame	   = 0;
	m_nDataSize		   = 0;
	m_PictureSize.cx = m_PictureSize.cy = 0;
	SetRect(&m_PaintRect,0,0,0,0);

	m_hExitEvent = CreateEvent(NULL,TRUE,FALSE,NULL);
}

CPictureExWnd::~CPictureExWnd()
{
	UnLoad();
	CloseHandle(m_hExitEvent);
}

//////////////////////////////////////////////////////////////////////
// Nested structures member functions
//////////////////////////////////////////////////////////////////////

inline int CPictureExWnd::TGIFControlExt::GetPackedValue(enum ControlExtValues Value)
{
	int nRet = (int)m_cPacked;
	switch (Value)
	{
	case GCX_PACKED_DISPOSAL:
		nRet = (nRet & 28) >> 2;
		break;

	case GCX_PACKED_USERINPUT:
		nRet = (nRet & 2) >> 1;
		break;

	case GCX_PACKED_TRANSPCOLOR:
		nRet &= 1;
		break;
	};

	return nRet;
}

inline int CPictureExWnd::TGIFLSDescriptor::GetPackedValue(enum LSDPackedValues Value)
{
	int nRet = (int)m_cPacked;

	switch (Value)
	{
	case LSD_PACKED_GLOBALCT:
		nRet = nRet >> 7;
		break;

	case LSD_PACKED_CRESOLUTION:
		nRet = ((nRet & 0x70) >> 4) + 1;
		break;

	case LSD_PACKED_SORT:
		nRet = (nRet & 8) >> 3;
		break;

	case LSD_PACKED_GLOBALCTSIZE:
		nRet &= 7;
		break;
	};

	return nRet;
}

inline int CPictureExWnd::TGIFImageDescriptor::GetPackedValue(enum IDPackedValues Value)
{
	int nRet = (int)m_cPacked;

	switch (Value)
	{
	case ID_PACKED_LOCALCT:
		nRet >>= 7;
		break;

	case ID_PACKED_INTERLACE:
		nRet = ((nRet & 0x40) >> 6);
		break;

	case ID_PACKED_SORT:
		nRet = (nRet & 0x20) >> 5;
		break;

	case ID_PACKED_LOCALCTSIZE:
		nRet &= 7;
		break;
	};

	return nRet;
}


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

BOOL CPictureExWnd::Load(HGLOBAL hGlobal, DWORD dwSize)
{
	IStream *pStream = NULL;
	UnLoad();

	if (!(m_pRawData = reinterpret_cast<unsigned char*> (GlobalLock(hGlobal))) )
	{
		ATLTRACE(_T("Load: Error locking memory\n"));
		return FALSE;
	};

	m_nDataSize = dwSize;
	m_pGIFHeader = reinterpret_cast<TGIFHeader *> (m_pRawData);

	if ((memcmp(&m_pGIFHeader->m_cSignature,"GIF",3) != 0) &&
		((memcmp(&m_pGIFHeader->m_cVersion,"87a",3) != 0) ||
		 (memcmp(&m_pGIFHeader->m_cVersion,"89a",3) != 0)) )
	{
	// it's neither GIF87a nor GIF89a
	// do the default processing

		// clear GIF variables
		m_pRawData = NULL;
		GlobalUnlock(hGlobal);

		// don't delete memory on object's release
		if (CreateStreamOnHGlobal(hGlobal,FALSE,&pStream) != S_OK)
			return FALSE;

		if (OleLoadPicture(pStream,dwSize,FALSE,IID_IPicture,
			reinterpret_cast<LPVOID *>(&m_pPicture)) != S_OK)
		{
			pStream->Release();
			return FALSE;
		};
		pStream->Release();

		// store picture's size

		long hmWidth;
		long hmHeight;
		m_pPicture->get_Width(&hmWidth);
		m_pPicture->get_Height(&hmHeight);

		HDC hDC = ::GetDC(m_hWnd);
		m_PictureSize.cx = MulDiv(hmWidth, GetDeviceCaps(hDC,LOGPIXELSX), 2540);
		m_PictureSize.cy = MulDiv(hmHeight, GetDeviceCaps(hDC,LOGPIXELSY), 2540);
		::ReleaseDC(m_hWnd,hDC);
	}
	else
	{
		// it's a GIF
		m_bIsGIF = TRUE;
		m_pGIFLSDescriptor = reinterpret_cast<TGIFLSDescriptor *>
			(m_pRawData + sizeof(TGIFHeader));
		if (m_pGIFLSDescriptor->GetPackedValue(LSD_PACKED_GLOBALCT) == 1)
		{
			// calculate the globat color table size
			m_nGlobalCTSize = static_cast<int>
				(3*pow(2.0,m_pGIFLSDescriptor->GetPackedValue(LSD_PACKED_GLOBALCTSIZE)+1));
			// get the background color if GCT is present
			unsigned char *pBkClr = m_pRawData + sizeof(TGIFHeader) + 
				sizeof(TGIFLSDescriptor) + 3*m_pGIFLSDescriptor->m_cBkIndex;
			m_clrBackground = RGB(pBkClr[0],pBkClr[1],pBkClr[2]);
		};

		// store the picture's size
		m_PictureSize.cx = m_pGIFLSDescriptor->m_wWidth;
		m_PictureSize.cy = m_pGIFLSDescriptor->m_wHeight;

		// determine frame count for this picture
		UINT nFrameCount=0;
		ResetDataPointer();
		while (SkipNextGraphicBlock())
			nFrameCount++;

#ifdef GIF_TRACING
		ATLTRACE(
			_T(" -= GIF encountered\n"
			   "Logical Screen dimensions = %dx%d\n"
			   "Global color table = %d\n"
			   "Color depth = %d\n"
			   "Sort flag = %d\n"
			   "Size of Global Color Table = %d\n"
			   "Background color index = %d\n"
			   "Pixel aspect ratio = %d\n"
			   "Frame count = %d\n"
			   "Background color = %06Xh\n\n"
			  ),
			m_pGIFLSDescriptor->m_wWidth,
			m_pGIFLSDescriptor->m_wHeight,
			m_pGIFLSDescriptor->GetPackedValue(LSD_PACKED_GLOBALCT),
			m_pGIFLSDescriptor->GetPackedValue(LSD_PACKED_CRESOLUTION),
			m_pGIFLSDescriptor->GetPackedValue(LSD_PACKED_SORT),
			m_pGIFLSDescriptor->GetPackedValue(LSD_PACKED_GLOBALCTSIZE),
			m_pGIFLSDescriptor->m_cBkIndex,
			m_pGIFLSDescriptor->m_cPixelAspect,
			nFrameCount,
			m_clrBackground
			);
		EnumGIFBlocks();
#endif

		if (nFrameCount == 0) // it's an empty GIF!
		{
			m_pRawData = NULL;
			GlobalUnlock(hGlobal);
			return FALSE;
		};

		// now check the frame count
		// if there's only one frame, no need to animate this GIF
		// therefore, treat it like any other pic

		if (nFrameCount == 1)
		{
			// clear GIF variables
			m_pRawData = NULL;
			GlobalUnlock(hGlobal);

			// don't delete memory on object's release
			if (CreateStreamOnHGlobal(hGlobal,FALSE,&pStream) != S_OK)
				return FALSE;

			if (OleLoadPicture(pStream,dwSize,FALSE,IID_IPicture,
				(LPVOID *)&m_pPicture) != S_OK)
			{
				pStream->Release();
				return FALSE;
			};

			pStream->Release();
		}
		else
		{
		// if, on the contrary, there are several frames
		// then store separate frames in an array

			TFrame frame;
			UINT nBlockLen;
			HGLOBAL hFrameData;
			UINT nCurFrame = 0;

			ResetDataPointer();
			while (hFrameData = GetNextGraphicBlock(&nBlockLen,
				&frame.m_nDelay, &frame.m_frameSize,
				&frame.m_frameOffset, &frame.m_nDisposal) )
			{
				#ifdef GIF_TRACING
				//////////////////////////////////////////////
				// uncomment the following strings if you want 
				// to write separate frames on disk
				//
				//TCHAR szName[_MAX_PATH];
				//sprintf(szName,_T("%.4d.gif"),nCurFrame);
				//WriteDataOnDisk(szName,hFrameData,nBlockLen);
				//nCurFrame++;
				#endif // GIF_TRACING

				IStream *pStream = NULL;

				// delete memory on object's release
				if (CreateStreamOnHGlobal(hFrameData,TRUE,&pStream) != S_OK)
				{
					GlobalFree(hFrameData);
					continue;
				};

				if (OleLoadPicture(pStream,nBlockLen,FALSE,
					IID_IPicture,
					reinterpret_cast<LPVOID *>(&frame.m_pPicture)) != S_OK)
				{
					pStream->Release();
					continue;
				};
				pStream->Release();
			
				// everything went well, add this frame
				m_arrFrames.push_back(frame);
			};

			// clean after ourselves
			m_pRawData = NULL;
			GlobalUnlock(hGlobal);

			if (m_arrFrames.empty()) // couldn't load any frames
				return FALSE;
		};
	}; // if (!IsGIF...

	return PrepareDC(m_PictureSize.cx,m_PictureSize.cy);
}

void CPictureExWnd::UnLoad()
{
	Stop();
	if (m_pPicture)
	{
		m_pPicture->Release();
		m_pPicture = NULL;
	};
	
	std::vector<TFrame>::iterator it;
	for (it=m_arrFrames.begin();it<m_arrFrames.end();it++)
		(*it).m_pPicture->Release();
	m_arrFrames.clear();

	if (m_hMemDC)
	{
		SelectObject(m_hMemDC,m_hOldBitmap);
		::DeleteDC(m_hMemDC);
		::DeleteObject(m_hBitmap);
		m_hMemDC  = NULL;
		m_hBitmap = NULL;
	};

	if (m_hDispMemDC)
	{
		SelectObject(m_hDispMemDC,m_hDispOldBM);
		::DeleteDC(m_hDispMemDC);
		::DeleteObject(m_hDispMemBM);
		m_hDispMemDC  = NULL;
		m_hDispMemBM = NULL;
	};

	SetRect(&m_PaintRect,0,0,0,0);
	m_pGIFLSDescriptor = NULL;
	m_pGIFHeader	   = NULL;
	m_pRawData		   = NULL;
	m_hThread		   = NULL;
	m_bIsInitialized   = FALSE;
	m_bExitThread	   = FALSE;
	m_bIsGIF		   = FALSE;
	m_clrBackground    = RGB(255,255,255); // white by default
	m_nGlobalCTSize	   = 0;
	m_nCurrOffset	   = 0;
	m_nCurrFrame	   = 0;
	m_nDataSize		   = 0;
}

BOOL CPictureExWnd::Draw()
{
	if (!m_bIsInitialized)
	{
		ATLTRACE(_T("Call of one the CPictureExWnd::Load() member functions before calling Draw()\n"));
		return FALSE;
	};

	if (IsAnimatedGIF())
	{
	// the picture needs some animation
	// we'll start the thread that will handle it for us
	
		unsigned int nDummy;
		m_hThread = (HANDLE) _beginthreadex(NULL,0,_ThreadAnimation,this,
			CREATE_SUSPENDED,&nDummy);
		if (!m_hThread)
		{
			ATLTRACE(_T("Draw: Couldn't start a GIF animation thread\n"));
			return FALSE;
		} 
		else 
			ResumeThread(m_hThread);
	} 
	else
	{
		if (m_pPicture)
		{
			long hmWidth;
			long hmHeight;
			m_pPicture->get_Width(&hmWidth);
			m_pPicture->get_Height(&hmHeight);
			if (m_pPicture->Render(m_hMemDC, 0, 0, m_PictureSize.cx, m_PictureSize.cy, 
				0, hmHeight, hmWidth, -hmHeight, NULL) == S_OK)
			{
				Invalidate(FALSE);
				return TRUE;
			};
		};
	};

	return FALSE;	
}

SIZE CPictureExWnd::GetSize() const
{
	return m_PictureSize;
}

BOOL CPictureExWnd::Load(LPCTSTR szFileName)
{
	ATLASSERT(szFileName);

	FILE *file;
	HGLOBAL hGlobal;
	DWORD dwSize;
	
	if (!(file = _tfopen(szFileName,_T("rb"))))
	{
		ATLTRACE(_T("Load (file): Error opening file %s\n"),szFileName);
		return FALSE;
	};

	fseek(file,0,SEEK_END);
	dwSize = ftell(file);
	fseek(file,0,SEEK_SET);

	hGlobal = GlobalAlloc(GMEM_MOVEABLE | GMEM_NODISCARD,dwSize);
	if (!hGlobal)
	{
		ATLTRACE(_T("Load (file): Error allocating memory\n"));
		fclose(file);
		return FALSE;
	};
	
	char *pData = reinterpret_cast<char*>(GlobalLock(hGlobal));
	if (!pData)
	{
		ATLTRACE(_T("Load (file): Error locking memory\n"));
		GlobalFree(hGlobal);
		fclose(file);
		return FALSE;
	};

	if (fread(pData,1,dwSize,file) != dwSize)
	{
		ATLTRACE(_T("Load (file): Error reading the file %s\n"),
			szFileName);
		GlobalFree(hGlobal);
		fclose(file);
		return FALSE;
	};

	GlobalUnlock(hGlobal);
	fclose(file);

	BOOL bRetValue = Load(hGlobal,dwSize);
	GlobalFree(hGlobal);
	return bRetValue;
}

BOOL CPictureExWnd::Load(LPCTSTR szResourceName, LPCTSTR szResourceType)
{
	ATLASSERT(szResourceName);
	ATLASSERT(szResourceType);

	HRSRC hPicture = FindResource(_Module.GetResourceInstance(),szResourceName,szResourceType);
	HGLOBAL hResData;
	if (!hPicture || !(hResData = LoadResource(_Module.GetResourceInstance(),hPicture)))
	{
		ATLTRACE(_T("Load (resource): Error loading resource %s\n"),szResourceName);
		return FALSE;
	};
	DWORD dwSize = SizeofResource(_Module.GetResourceInstance(),hPicture);

	// hResData is not the real HGLOBAL (we can't lock it)
	// let's make it real

	HGLOBAL hGlobal = GlobalAlloc(GMEM_MOVEABLE | GMEM_NODISCARD,dwSize);
	if (!hGlobal)
	{
		ATLTRACE("Load (resource): Error allocating memory\n");
		FreeResource(hResData);
		return FALSE;
	};
	
	char *pDest = reinterpret_cast<char *> (GlobalLock(hGlobal));
	char *pSrc = reinterpret_cast<char *> (LockResource(hResData));
	if (!pSrc || !pDest)
	{
		ATLTRACE(_T("Load (resource): Error locking memory\n"));
		GlobalFree(hGlobal);
		FreeResource(hResData);
		return FALSE;
	};
	CopyMemory(pDest,pSrc,dwSize);
	FreeResource(hResData);
	GlobalUnlock(hGlobal);

	BOOL bRetValue = Load(hGlobal,dwSize);
	GlobalFree(hGlobal);
	return bRetValue;
}

void CPictureExWnd::ResetDataPointer()
{
	// skip header and logical screen descriptor
	m_nCurrOffset = 
		sizeof(TGIFHeader)+sizeof(TGIFLSDescriptor)+m_nGlobalCTSize;
}

BOOL CPictureExWnd::SkipNextGraphicBlock()
{
	if (!m_pRawData) return FALSE;

	// GIF header + LSDescriptor [+ GCT] [+ Control block] + Data

	enum GIFBlockTypes nBlock;

	nBlock = GetNextBlock();

	while ((nBlock != BLOCK_CONTROLEXT) &&
		   (nBlock != BLOCK_IMAGE) &&
		   (nBlock != BLOCK_PLAINTEXT) &&
		   (nBlock != BLOCK_UNKNOWN) &&
		   (nBlock != BLOCK_TRAILER) )
	{
		if (!SkipNextBlock()) return NULL;
		nBlock = GetNextBlock();
	};

	if ((nBlock == BLOCK_UNKNOWN) ||
		(nBlock == BLOCK_TRAILER))
		return FALSE;

	// it's either a control ext.block, an image or a plain text

	if (GetNextBlockLen() <= 0) return FALSE;

	if (nBlock == BLOCK_CONTROLEXT)
	{
		if (!SkipNextBlock()) return FALSE;
		nBlock = GetNextBlock();

		// skip everything until we meet an image block or a plain-text block
		while ((nBlock != BLOCK_IMAGE) &&
			   (nBlock != BLOCK_PLAINTEXT) &&
			   (nBlock != BLOCK_UNKNOWN) &&
			   (nBlock != BLOCK_TRAILER) )
		{
			if (!SkipNextBlock()) return NULL;
			nBlock = GetNextBlock();
		};

		if ((nBlock == BLOCK_UNKNOWN) ||
			(nBlock == BLOCK_TRAILER))
			return FALSE;
	};

	// skip the found data block (image or plain-text)
	if (!SkipNextBlock()) return FALSE;

	return TRUE;
}

UINT CPictureExWnd::GetSubBlocksLen(UINT nStartingOffset) const
{
	UINT nRet = 0;
	UINT nCurOffset = nStartingOffset;
	
	while (m_pRawData[nCurOffset] != 0)
	{
		nRet += m_pRawData[nCurOffset]+1;
		nCurOffset += m_pRawData[nCurOffset]+1;
	};

	return nRet+1;
}

enum CPictureExWnd::GIFBlockTypes CPictureExWnd::GetNextBlock() const
{
	switch(m_pRawData[m_nCurrOffset])
	{
	case 0x21:
	// extension block
		switch(m_pRawData[m_nCurrOffset+1])
		{
		case 0x01:
		// plain text extension
			return BLOCK_PLAINTEXT;
			break;

		case 0xF9:
		// graphic control extension
			return BLOCK_CONTROLEXT;
			break;

		case 0xFE:
		// comment extension
			return BLOCK_COMMEXT;
			break;

		case 0xFF:
		// application extension
			return BLOCK_APPEXT;
			break;
		};
		break;
	
	case 0x3B:
	// trailer
		return BLOCK_TRAILER;
		break;

	case 0x2C:
	// image data
		return BLOCK_IMAGE;
		break;
	};

	return BLOCK_UNKNOWN;
}

BOOL CPictureExWnd::SkipNextBlock()
{
	if (!m_pRawData) return FALSE;

	int nLen = GetNextBlockLen();
	if ((nLen <= 0) || ((m_nCurrOffset+nLen) > m_nDataSize))
		return FALSE;

	m_nCurrOffset += nLen;
	return TRUE;
}

int CPictureExWnd::GetNextBlockLen() const
{
	GIFBlockTypes nBlock = GetNextBlock();

	int nTmp;

	switch(nBlock)
	{
	case BLOCK_UNKNOWN:
		return -1;
		break;

	case BLOCK_TRAILER:
		return 1;
		break;

	case BLOCK_APPEXT:
		nTmp = GetSubBlocksLen(m_nCurrOffset+sizeof(TGIFAppExtension));
		if (nTmp > 0)
			return sizeof(TGIFAppExtension)+nTmp;
		break;

	case BLOCK_COMMEXT:
		nTmp = GetSubBlocksLen(m_nCurrOffset+sizeof(TGIFCommentExt));
		if (nTmp > 0)
			return sizeof(TGIFCommentExt)+nTmp;
		break;

	case BLOCK_CONTROLEXT:
		return sizeof(TGIFControlExt);
		break;

	case BLOCK_PLAINTEXT:
		nTmp = GetSubBlocksLen(m_nCurrOffset+sizeof(TGIFPlainTextExt));
		if (nTmp > 0)
			return sizeof(TGIFPlainTextExt)+nTmp;
		break;

	case BLOCK_IMAGE:
		TGIFImageDescriptor *pIDescr = 
			reinterpret_cast<TGIFImageDescriptor *> (&m_pRawData[m_nCurrOffset]);
		int nLCTSize = (int)
			(pIDescr->GetPackedValue(ID_PACKED_LOCALCT)*3*
			pow(2.0,pIDescr->GetPackedValue(ID_PACKED_LOCALCTSIZE)+1));

		int nTmp = GetSubBlocksLen(m_nCurrOffset+
			sizeof(TGIFImageDescriptor) + nLCTSize + 1);
		if (nTmp > 0)
			return sizeof(TGIFImageDescriptor) + nLCTSize + 1 + nTmp;
		break;
	};

	return 0;
}

UINT WINAPI CPictureExWnd::_ThreadAnimation(LPVOID pParam)
{
	ATLASSERT(pParam);
	CPictureExWnd *pPic = reinterpret_cast<CPictureExWnd *> (pParam);

	pPic->m_bIsPlaying = TRUE;
	pPic->ThreadAnimation();
	pPic->m_bIsPlaying = FALSE;

	// this thread has finished its work so we close the handle
	CloseHandle(pPic->m_hThread); 
	// and init the handle to zero (so that Stop() doesn't Wait on it)
	pPic->m_hThread = 0;
	return 0;
}

void CPictureExWnd::ThreadAnimation()
{
	// first, restore background (for stop/draw support)
	// disposal method #2
	if (m_arrFrames[m_nCurrFrame].m_nDisposal == 2)
	{
		HBRUSH hBrush = CreateSolidBrush(m_clrBackground);
		if (hBrush)
		{
			RECT rect = {
				m_arrFrames[m_nCurrFrame].m_frameOffset.cx,
				m_arrFrames[m_nCurrFrame].m_frameOffset.cy,
				m_arrFrames[m_nCurrFrame].m_frameOffset.cx + m_arrFrames[m_nCurrFrame].m_frameSize.cx,
				m_arrFrames[m_nCurrFrame].m_frameOffset.cy + m_arrFrames[m_nCurrFrame].m_frameSize.cy };
			FillRect(m_hMemDC,&rect,hBrush);
			DeleteObject(hBrush);
		};
	} 
	else
		// disposal method #3
		if (m_hDispMemDC && (m_arrFrames[m_nCurrFrame].m_nDisposal == 3) )
		{
			// put it back
			BitBlt(m_hMemDC,
				m_arrFrames[m_nCurrFrame].m_frameOffset.cx,
				m_arrFrames[m_nCurrFrame].m_frameOffset.cy,
				m_arrFrames[m_nCurrFrame].m_frameSize.cx,
				m_arrFrames[m_nCurrFrame].m_frameSize.cy,
				m_hDispMemDC,0,0, SRCCOPY);
			// init variables
			SelectObject(m_hDispMemDC,m_hDispOldBM);
			DeleteDC(m_hDispMemDC); m_hDispMemDC = NULL;
			DeleteObject(m_hDispMemBM); m_hDispMemBM = NULL;
		};

	while (!m_bExitThread)
	{
		if (m_arrFrames[m_nCurrFrame].m_pPicture)
		{
		///////////////////////////////////////////////////////
		// Before rendering a frame we should take care of what's 
		// behind that frame. TFrame::m_nDisposal will be our guide:
		//   0 - no disposal specified (do nothing)
		//   1 - do not dispose (again, do nothing)
		//   2 - restore to background color (m_clrBackground)
		//   3 - restore to previous

			//////// disposal method #3
			if (m_arrFrames[m_nCurrFrame].m_nDisposal == 3)
			{
				// prepare a memory DC and store the background in it
				m_hDispMemDC = CreateCompatibleDC(m_hMemDC);
				m_hDispMemBM = CreateCompatibleBitmap(m_hMemDC,
							m_arrFrames[m_nCurrFrame].m_frameSize.cx,
							m_arrFrames[m_nCurrFrame].m_frameSize.cy);
				
				if (m_hDispMemDC && m_hDispMemBM)
				{
					m_hDispOldBM = reinterpret_cast<HBITMAP> (SelectObject(m_hDispMemDC,m_hDispMemBM));
					BitBlt(m_hDispMemDC,0,0,
						m_arrFrames[m_nCurrFrame].m_frameSize.cx,
						m_arrFrames[m_nCurrFrame].m_frameSize.cy,
						m_hMemDC,
						m_arrFrames[m_nCurrFrame].m_frameOffset.cx,
						m_arrFrames[m_nCurrFrame].m_frameOffset.cy,
						SRCCOPY);
				};
			};
			///////////////////////

			long hmWidth;
			long hmHeight;
			m_arrFrames[m_nCurrFrame].m_pPicture->get_Width(&hmWidth);
			m_arrFrames[m_nCurrFrame].m_pPicture->get_Height(&hmHeight);

			if (m_arrFrames[m_nCurrFrame].m_pPicture->Render(m_hMemDC, 
				m_arrFrames[m_nCurrFrame].m_frameOffset.cx, 
				m_arrFrames[m_nCurrFrame].m_frameOffset.cy, 
				m_arrFrames[m_nCurrFrame].m_frameSize.cx, 
				m_arrFrames[m_nCurrFrame].m_frameSize.cy, 
				0, hmHeight, hmWidth, -hmHeight, NULL) == S_OK)
			{
				Invalidate(FALSE);
			};
			
			if (m_bExitThread) break;

			// if the delay time is too short (like in old GIFs), wait for 100ms
			if (m_arrFrames[m_nCurrFrame].m_nDelay < 5) 
				WaitForSingleObject(m_hExitEvent, 100);
			else
				WaitForSingleObject(m_hExitEvent, 10*m_arrFrames[m_nCurrFrame].m_nDelay);

			if (m_bExitThread) break;

			// disposal method #2
			if (m_arrFrames[m_nCurrFrame].m_nDisposal == 2)
			{
				HBRUSH hBrush = CreateSolidBrush(m_clrBackground);
				if (hBrush)
				{
					RECT rect = {
						m_arrFrames[m_nCurrFrame].m_frameOffset.cx,
						m_arrFrames[m_nCurrFrame].m_frameOffset.cy,
						m_arrFrames[m_nCurrFrame].m_frameOffset.cx + m_arrFrames[m_nCurrFrame].m_frameSize.cx,
						m_arrFrames[m_nCurrFrame].m_frameOffset.cy + m_arrFrames[m_nCurrFrame].m_frameSize.cy };
					FillRect(m_hMemDC,&rect,hBrush);
					DeleteObject(hBrush);
				};
			} 
			else
				if (m_hDispMemDC && (m_arrFrames[m_nCurrFrame].m_nDisposal == 3) )
				{
					// put it back
					BitBlt(m_hMemDC,
						m_arrFrames[m_nCurrFrame].m_frameOffset.cx,
						m_arrFrames[m_nCurrFrame].m_frameOffset.cy,
						m_arrFrames[m_nCurrFrame].m_frameSize.cx,
						m_arrFrames[m_nCurrFrame].m_frameSize.cy,
						m_hDispMemDC,0,0, SRCCOPY);
					// init variables
					SelectObject(m_hDispMemDC,m_hDispOldBM);
					DeleteDC(m_hDispMemDC); m_hDispMemDC = NULL;
					DeleteObject(m_hDispMemBM); m_hDispMemBM = NULL;
				};
		};
		m_nCurrFrame++;
		if (m_nCurrFrame == m_arrFrames.size())
		{
			m_nCurrFrame
				= 0; 
		// init the screen for the first frame,
			HBRUSH hBrush = CreateSolidBrush(m_clrBackground);
			if (hBrush)
			{
				RECT rect = {0,0,m_PictureSize.cx,m_PictureSize.cy};
				FillRect(m_hMemDC,&rect,hBrush);
				DeleteObject(hBrush);
			};
		};
	};
}

void CPictureExWnd::Stop()
{
	m_bIsPlaying = FALSE;
	m_bExitThread = TRUE;
	SetEvent(m_hExitEvent);
	if (m_hThread)
	{
		// we'll wait for 5 seconds then continue execution
		// I hate hangups :P
		WaitForSingleObject(m_hThread,5000);
		CloseHandle(m_hThread);
		m_hThread = NULL;
	}

	// make it possible to Draw() again
	ResetEvent(m_hExitEvent);
	m_bExitThread = FALSE;
}

HGLOBAL CPictureExWnd::GetNextGraphicBlock(UINT *pBlockLen, 
	UINT *pDelay, SIZE *pBlockSize, SIZE *pBlockOffset, 
	UINT *pDisposal)
{
	if (!m_pRawData) return NULL;

	// GIF header + LSDescriptor [+ GCT] [+ Control block] + Data

	*pDisposal = 0;
	enum GIFBlockTypes nBlock;
	nBlock = GetNextBlock();

	while (
		(nBlock != BLOCK_CONTROLEXT) &&
		(nBlock != BLOCK_IMAGE) &&
		(nBlock != BLOCK_PLAINTEXT) &&
		(nBlock != BLOCK_UNKNOWN) &&
		(nBlock != BLOCK_TRAILER)
		)
	{
		if (!SkipNextBlock()) return NULL;
		nBlock = GetNextBlock();
	};

	if ((nBlock == BLOCK_UNKNOWN) ||
		(nBlock == BLOCK_TRAILER))
		return NULL;

	// it's either a control ext.block, an image or a plain text

	int nStart = m_nCurrOffset;
	int nBlockLen = GetNextBlockLen();

	if (nBlockLen <= 0) return NULL;

	if (nBlock == BLOCK_CONTROLEXT)
	{
		// get the following data
		TGIFControlExt *pControl = 
			reinterpret_cast<TGIFControlExt *> (&m_pRawData[m_nCurrOffset]);
		// store delay time
		*pDelay = pControl->m_wDelayTime;
		// store disposal method
		*pDisposal = pControl->GetPackedValue(GCX_PACKED_DISPOSAL);

		if (!SkipNextBlock()) return NULL;
		nBlock = GetNextBlock();
		
		// skip everything until we find data to display 
		// (image block or plain-text block)
		
		while (
			(nBlock != BLOCK_IMAGE) &&
			(nBlock != BLOCK_PLAINTEXT) &&
			(nBlock != BLOCK_UNKNOWN) &&
			(nBlock != BLOCK_TRAILER)
			)
		{
			if (!SkipNextBlock()) return NULL;
			nBlock = GetNextBlock();
			nBlockLen += GetNextBlockLen();
		};

		if ((nBlock == BLOCK_UNKNOWN) || (nBlock == BLOCK_TRAILER))
			return NULL;
		nBlockLen += GetNextBlockLen();
	}
	else
		*pDelay = -1; // to indicate that there was no delay value

	if (nBlock == BLOCK_IMAGE)
	{
		// store size and offsets
		TGIFImageDescriptor *pImage = 
			reinterpret_cast<TGIFImageDescriptor *> (&m_pRawData[m_nCurrOffset]);
		pBlockSize->cx = pImage->m_wWidth;
		pBlockSize->cy = pImage->m_wHeight;
		pBlockOffset->cx = pImage->m_wLeftPos;
		pBlockOffset->cy = pImage->m_wTopPos;
	};

	if (!SkipNextBlock()) return NULL;

	HGLOBAL hGlobal = GlobalAlloc(GMEM_FIXED,
		sizeof(TGIFHeader) +
		sizeof(TGIFLSDescriptor) +
		m_nGlobalCTSize +
		nBlockLen + 
		1);  // for the trailer

	if (!hGlobal) return NULL;

	int nOffset = 0; 

	// GMEM_FIXED means we get a pointer
	unsigned char *pGlobal = reinterpret_cast<unsigned char *> (hGlobal);

	CopyMemory(pGlobal,m_pRawData, 
		sizeof(TGIFHeader)+sizeof(TGIFLSDescriptor)+m_nGlobalCTSize);
	nOffset += sizeof(TGIFHeader)+sizeof(TGIFLSDescriptor)+m_nGlobalCTSize;

	CopyMemory(pGlobal + nOffset,&m_pRawData[nStart], nBlockLen);
	nOffset += nBlockLen;

	pGlobal[nOffset] = 0x3B; // trailer
	nOffset++;

	*pBlockLen = nOffset;

	return hGlobal;
}

BOOL CPictureExWnd::IsGIF() const
{
	return m_bIsGIF;
}

BOOL CPictureExWnd::IsPlaying() const
{
	return m_bIsPlaying;
}

BOOL CPictureExWnd::IsAnimatedGIF() const
{
	return (m_bIsGIF && (m_arrFrames.size() > 1));
}

int CPictureExWnd::GetFrameCount() const
{
	if (!IsAnimatedGIF())
		return 0;

	return m_arrFrames.size();
}

COLORREF CPictureExWnd::GetBkColor() const
{
	return m_clrBackground;
}

LRESULT CPictureExWnd::OnPaint(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) 
{
	RECT rcUpdate;
	if (!GetUpdateRect(&rcUpdate))
		return S_OK;

	PAINTSTRUCT paint;
	HDC hDC = BeginPaint(&paint);
	if (!hDC) return E_FAIL;

	LONG nPaintWidth = m_PaintRect.right-m_PaintRect.left;

	if (nPaintWidth > 0)
	{
		LONG nPaintHeight = m_PaintRect.bottom - m_PaintRect.top;
		::BitBlt(hDC, 0, 0, nPaintWidth, nPaintHeight,	
			m_hMemDC, m_PaintRect.left, m_PaintRect.top, SRCCOPY);
	}
	else
	{
		::BitBlt(hDC, 0, 0, m_PictureSize.cx, m_PictureSize.cy,
			m_hMemDC, 0, 0, SRCCOPY);
	};

	EndPaint(&paint);
	bHandled = TRUE;
	return S_OK;
}

BOOL CPictureExWnd::PrepareDC(int nWidth, int nHeight)
{
	SetWindowPos(NULL,0,0,nWidth,nHeight,SWP_NOMOVE | SWP_NOZORDER);

	HDC hWinDC = ::GetDC(m_hWnd);
	if (!hWinDC) return FALSE;
	
	m_hMemDC = CreateCompatibleDC(hWinDC);
	if (!m_hMemDC) 
	{
		::ReleaseDC(m_hWnd,hWinDC);
		return FALSE;
	};

	m_hBitmap  = CreateCompatibleBitmap(hWinDC,nWidth,nHeight);
	if (!m_hBitmap) 
	{
		::ReleaseDC(m_hWnd,hWinDC);
		::DeleteDC(m_hMemDC);
		return FALSE;
	};

	m_hOldBitmap = reinterpret_cast<HBITMAP> 
						(SelectObject(m_hMemDC,m_hBitmap));
	
	// fill the background
	m_clrBackground = GetSysColor(COLOR_3DFACE);
	RECT rect = {0,0,nWidth,nHeight};
	FillRect(m_hMemDC,&rect,(HBRUSH)(COLOR_WINDOW));

	::ReleaseDC(m_hWnd,hWinDC);
	m_bIsInitialized = TRUE;
	return TRUE;
}

LRESULT CPictureExWnd::OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) 
{
	Stop();	
	return S_OK;
}

void CPictureExWnd::SetBkColor(COLORREF clr)
{
	if (!m_bIsInitialized) return;

	m_clrBackground = clr;

	HBRUSH hBrush = CreateSolidBrush(clr);
	if (hBrush)
	{
		RECT rect = {0,0,m_PictureSize.cx,m_PictureSize.cy};
		FillRect(m_hMemDC,&rect,hBrush);
		DeleteObject(hBrush);
	};
}

#ifdef GIF_TRACING
void CPictureExWnd::WriteDataOnDisk(LPCTSTR szFileName, HGLOBAL hData, DWORD dwSize)
{
	FILE *file;

	if (!(file = fopen(szFileName,_T("wb"))) )
	{
		ATLTRACE(_T("WriteData: Error creating file %s\n"),szFileName);
		return;
	};

	char *pData = reinterpret_cast<char *> (GlobalLock(hData));
	if (!pData)
	{
		ATLTRACE(_T("WriteData: Error locking memory\n"));
		fclose(file);
		return;
	};

	if (fwrite(pData,1,dwSize,file) != dwSize)
	{
		ATLTRACE(_T("WriteData: Error writing to the file %s\n"), szFileName);
		GlobalUnlock(hData);
		fclose(file);
		return;
	}
	
	GlobalUnlock(hData);
	fclose(file);
}

void CPictureExWnd::EnumGIFBlocks()
{
	enum GIFBlockTypes nBlock;

	ResetDataPointer();
	while(m_nCurrOffset < m_nDataSize)
	{
		nBlock = GetNextBlock();
		switch(nBlock)
		{
		case BLOCK_UNKNOWN:
			ATLTRACE(_T("- Unknown block\n"));
			return;
			break;

		case BLOCK_TRAILER:
			ATLTRACE(_T("- Trailer block\n"));
			break;

		case BLOCK_APPEXT:
			ATLTRACE(_T("- Application extension block\n"));
			break;

		case BLOCK_COMMEXT:
			ATLTRACE(_T("- Comment extension block\n"));
			break;

		case BLOCK_CONTROLEXT:
			{
			TGIFControlExt *pControl = 
				reinterpret_cast<TGIFControlExt *> (&m_pRawData[m_nCurrOffset]);
			ATLTRACE(_T("- Graphic control extension block (delay %d, disposal %d)\n"),
					pControl->m_wDelayTime, pControl->GetPackedValue(GCX_PACKED_DISPOSAL));
			};
			break;

		case BLOCK_PLAINTEXT:
			ATLTRACE(_T("- Plain text extension block\n"));
			break;

		case BLOCK_IMAGE:
			TGIFImageDescriptor *pIDescr = 
				reinterpret_cast<TGIFImageDescriptor *> (&m_pRawData[m_nCurrOffset]);
			ATLTRACE(_T("- Image data block (%dx%d  %d,%d)\n"),
					pIDescr->m_wWidth,
					pIDescr->m_wHeight,
					pIDescr->m_wLeftPos,
					pIDescr->m_wTopPos);
			break;
		};

		SkipNextBlock();	
	};

	ATLTRACE(_T("\n"));
}
#endif // GIF_TRACING


BOOL CPictureExWnd::SetPaintRect(const RECT *lpRect)
{
	return CopyRect(&m_PaintRect, lpRect);
}

BOOL CPictureExWnd::GetPaintRect(RECT *lpRect)
{
	return CopyRect(lpRect, &m_PaintRect);
}
