#if !defined(AFX_SPLITTER_H__20030708_C845_A999_1D3A_0080AD509054__INCLUDED_)
#define AFX_SPLITTER_H__20030708_C845_A999_1D3A_0080AD509054__INCLUDED_

#pragma once

/////////////////////////////////////////////////////////////////////////////
// CSplitterBar - Splitter control (not a container)
//
// Written by Bjarke Viksoe (bjarke@viksoe.dk)
// Copyright (c) 2003 Bjarke Viksoe.
//
// This code may be used in compiled form in any way you desire. This
// source file may be redistributed by any means PROVIDING it is 
// not sold for profit without the authors written consent, and 
// providing that this notice and the authors name is included. 
//
// This file is provided "as is" with no expressed or implied warranty.
// The author accepts no liability if it causes any damage to you or your
// computer whatsoever. It's free, so don't hassle me about it.
//
// Beware of bugs.
//

#ifndef __cplusplus
  #error ATL requires C++ compilation (use a .cpp suffix)
#endif

#ifndef __ATLAPP_H__
  #error This control requires atlapp.h to be included first
#endif

#ifndef __ATLCTRLS_H__
  #error This control requires atlctrls.h to be included first
#endif


template< class T, bool t_bVertical = true, class TBase = CWindow, class TWinTraits = CControlWinTraits >
class ATL_NO_VTABLE CSplitterBarImpl : 
   public CWindowImpl< T, TBase, TWinTraits >
{
public:
   DECLARE_WND_SUPERCLASS(NULL, TBase::GetWndClassName())

   typedef CSplitterBarImpl<T, t_bVertical, TBase, TWinTraits> thisClass;

   CWindow m_wndTop;
   CWindow m_wndBottom;
   BOOL m_bFullDrag;
   SIZE m_szGutter;
   inline static HCURSOR s_hCursor = NULL;
   RECT m_rcSplitterPos;
   int m_xySplitterPos;
   SIZE m_szLimits;

   // Operations

   BOOL SubclassWindow(HWND hWnd)
   {
      ATLASSERT(m_hWnd==NULL);
      ATLASSERT(::IsWindow(hWnd));
      BOOL bRet = CWindowImpl< T, TBase, TWinTraits >::SubclassWindow(hWnd);
      if( bRet ) _Init();
      return bRet;
   }

   void SetSplitterPanes(HWND hWndTop, HWND hWndBottom)
   {
      ATLASSERT(::IsWindow(m_hWnd));
      ATLASSERT(::IsWindow(hWndTop));
      ATLASSERT(::IsWindow(hWndBottom));
      m_wndTop = hWndTop;
      m_wndBottom = hWndBottom;
      RECT rcTop;
      RECT rcBottom;
      m_wndTop.GetWindowRect(&rcTop);
      m_wndBottom.GetWindowRect(&rcBottom);
      RECT rcSplitter = { 0 };
      if( t_bVertical ) ::SetRect(&rcSplitter, rcTop.right, rcTop.top, rcBottom.left, rcBottom.bottom);
      else ::SetRect(&rcSplitter, rcTop.left, rcTop.bottom, rcTop.right, rcBottom.top);
      ::MapWindowPoints(NULL, GetParent(), (LPPOINT) &rcSplitter, 2);
      MoveWindow(&rcSplitter);
   }
   void SetGutterSize(int cxTop, int cxBottom)
   {
      m_szGutter.cx = cxTop;
      m_szGutter.cy = cxBottom:
   }
   void SetSplitterSize(int cx)
   {
      ATLASSERT(::IsWindow(m_hWnd));
      ResizeClient(t_bVertical ? cx : -1, t_bVertical ? -1 : cx);
      T* pT = static_cast<T*>(this);
      pT->UpdateSplitterLayout();
   }

   void UpdateSplitterLayout()
   {
      ATLASSERT(::IsWindow(m_hWnd));
      RECT rcClient;
      RECT rcTop;
      RECT rcBottom;
      GetWindowRect(&rcClient);
      m_wndTop.GetWindowRect(&rcTop);
      m_wndBottom.GetWindowRect(&rcBottom);
      if( t_bVertical ) {
         rcTop.right = rcClient.left;
         rcBottom.left = rcClient.right;
      }
      else {
         rcTop.bottom = rcClient.top;
         rcBottom.top = rcClient.bottom;
      }
      ::MapWindowPoints(NULL, GetParent(), (LPPOINT) &rcTop, 2);
      ::MapWindowPoints(NULL, GetParent(), (LPPOINT) &rcBottom, 2);
      m_wndTop.SetWindowPos(NULL, &rcTop, SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOMOVE);
      m_wndBottom.SetWindowPos(NULL, &rcBottom, SWP_NOACTIVATE | SWP_NOZORDER);
      ::UpdateWindow(GetParent());
   }

   // Implementation

   void _Init()
   {
      ATLASSERT(::IsWindow(m_hWnd));

      if( s_hCursor == NULL ) {
         ::EnterCriticalSection(&_Module.m_csStaticDataInit);
         if( s_hCursor == NULL ) s_hCursor = ::LoadCursor(NULL, t_bVertical ? IDC_SIZEWE : IDC_SIZENS);
         ::LeaveCriticalSection(&_Module.m_csStaticDataInit);
      }

      // HACK: If subclassed as label, we need mouse notifications!
      ModifyStyle(0, SS_NOTIFY);

      ::SystemParametersInfo(SPI_GETDRAGFULLWINDOWS, 0, &m_bFullDrag, 0);

      m_szGutter.cx = 14;
      m_szGutter.cy = 14;
   }

   void _DrawGhostBar()
   {
      if( m_bFullDrag ) return;
      RECT rect;
      GetClientRect(&rect);
      // Invert the brush pattern (looks just like frame window sizing)
      T* pT = static_cast<T*>(this);
      CWindowDC dc(pT->m_hWnd);
      CBrush brush = CDCHandle::GetHalftoneBrush();
      if( !brush.IsNull() ) {
         CBrushHandle brushOld = dc.SelectBrush(brush);
         dc.PatBlt(rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top, PATINVERT);
         dc.SelectBrush(brushOld);
      }
   }

   // Message map and handlers

   BEGIN_MSG_MAP( thisClass )
      MESSAGE_HANDLER(WM_CREATE, OnCreate)
      MESSAGE_HANDLER(WM_PAINT, OnPaint)
      MESSAGE_HANDLER(WM_PRINTCLIENT, OnPaint)
      MESSAGE_HANDLER(WM_SETCURSOR, OnSetCursor)
      MESSAGE_HANDLER(WM_MOUSEMOVE, OnMouseMove)
      MESSAGE_HANDLER(WM_LBUTTONDOWN, OnLButtonDown)
      MESSAGE_HANDLER(WM_LBUTTONUP, OnLButtonUp)
   END_MSG_MAP()

   LRESULT OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/)
   {
      LRESULT lRes = DefWindowProc(uMsg, wParam, lParam);
      _Init();
      return lRes;
   }
   LRESULT OnPaint(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
   {
      T* pT = static_cast<T*>(this);
      CPaintDC dc(pT->m_hWnd);
      RECT rcClient;
      GetClientRect(&rcClient);
      if( (GetStyle() & SS_TYPEMASK) == SS_OWNERDRAW ) {
         DRAWITEMSTRUCT dis = { 0 };
         dis.CtlID = GetDlgCtrlID();
         dis.CtlType = ODT_STATIC;
         dis.itemAction = ODA_DRAWENTIRE;
         dis.hwndItem = m_hWnd;
         dis.hDC = dc;
         dis.itemID = 0;
         dis.itemState = 0;
         if( GetCapture() == m_hWnd ) dis.itemState |= ODS_SELECTED;
         dis.rcItem = rcClient;
         ::SendMessage(GetParent(), WM_DRAWITEM, (WPARAM) dis.CtlID, (LPARAM) &dis);
      }
      else
      {
         dc.FillRect(&rcClient, ::GetSysColorBrush(COLOR_3DFACE));
      }
      return 0;
   }
   LRESULT OnSetCursor(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
   {
      T* pT = static_cast<T*>(this);
      if( (HWND) wParam == pT->m_hWnd && LOWORD(lParam) == HTCLIENT ) return 1;
      bHandled = FALSE;
      return 0;
   }
   LRESULT OnMouseMove(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
   {
      T* pT = static_cast<T*>(this);
      if( (wParam & MK_LBUTTON) != 0 && ::GetCapture() == pT->m_hWnd ) {
         lParam = ::GetMessagePos();
         int xPos = GET_X_LPARAM(lParam);
         int yPos = GET_Y_LPARAM(lParam);
         int xyNewSplitPos = t_bVertical ? xPos : yPos;
         int cxySplitter = t_bVertical ? (m_rcSplitterPos.right - m_rcSplitterPos.left) : (m_rcSplitterPos.bottom - m_rcSplitterPos.top);
         xyNewSplitPos = std::max<int>(xyNewSplitPos, m_szLimits.cx);
         xyNewSplitPos = std::min<int>(xyNewSplitPos, m_szLimits.cy - cxySplitter);
         if( m_xySplitterPos != xyNewSplitPos ) {
            _DrawGhostBar();
            int iDiff = xyNewSplitPos - m_xySplitterPos;
            RECT rcClient = m_rcSplitterPos;
            ::OffsetRect(&rcClient, t_bVertical ? iDiff : 0, t_bVertical ? 0 : iDiff);
            ::MapWindowPoints(NULL, GetParent(), (LPPOINT) &rcClient, 2);
            Invalidate();
            SetWindowPos(HWND_TOP, &rcClient, SWP_NOACTIVATE);
            Invalidate();
            if( m_bFullDrag ) pT->UpdateSplitterLayout();
            _DrawGhostBar();
         }
      }
      ::SetCursor(s_hCursor);
      bHandled = FALSE;
      return 1;
   }
   LRESULT OnLButtonDown(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& bHandled)
   {
      T* pT = static_cast<T*>(this);
      pT->SetCapture();
      ::SetCursor(s_hCursor);
      _DrawGhostBar();
      lParam = ::GetMessagePos();
      int xPos = GET_X_LPARAM(lParam);
      int yPos = GET_Y_LPARAM(lParam);
      GetWindowRect(&m_rcSplitterPos);
      m_xySplitterPos = t_bVertical ? xPos : yPos;
      RECT rcTop;
      RECT rcBottom;
      m_wndTop.GetWindowRect(&rcTop);
      m_wndBottom.GetWindowRect(&rcBottom);
      if( t_bVertical ) {
         m_szLimits.cx = rcTop.left + m_szGutter.cx;
         m_szLimits.cy = rcBottom.right - m_szGutter.cy;
      }
      else {
         m_szLimits.cx = rcTop.top + m_szGutter.cx;
         m_szLimits.cy = rcBottom.bottom - m_szGutter.cy;
      }
      bHandled = FALSE;
      return 1;
   }
   LRESULT OnLButtonUp(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled)
   {
      _DrawGhostBar();
      if( !m_bFullDrag ) {
         T* pT = static_cast<T*>(this);
         pT->UpdateSplitterLayout();
         ::UpdateWindow(GetParent());
      }
      ::ReleaseCapture();
      bHandled = FALSE;
      return 1;
   }
};

//template< class T, bool t_bVertical, class TBase, class TWinTraits > HCURSOR CSplitterBarImpl< T, t_bVertical>::s_hCursor = NULL;

class CVertSplitterCtrl : public CSplitterBarImpl<CVertSplitterCtrl, true>
{
public:
   DECLARE_WND_SUPERCLASS(_T("WTL_VertSplitterBar"), GetWndClassName())  
};

class CHorSplitterCtrl : public CSplitterBarImpl<CHorSplitterCtrl, false>
{
public:
   DECLARE_WND_SUPERCLASS(_T("WTL_HorSplitterBar"), GetWndClassName())  
};


#endif // !defined(AFX_SPLITTER_H__20030708_C845_A999_1D3A_0080AD509054__INCLUDED_)

