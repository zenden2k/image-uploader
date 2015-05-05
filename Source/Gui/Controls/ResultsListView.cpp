#include "ResultsListView.h"

CResultsListView::CResultsListView() {
}

bool CResultsListView::AttachToDlgItem(HWND parent, UINT dlgID) {

    HWND hWnd = ::GetDlgItem(parent,dlgID);
    return SubclassWindow(hWnd)!=FALSE;
}
/*
LRESULT CResultsListView::OnKeyDown(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
    WPARAM vk = wParam;
    if ( vk == _T('A') && GetKeyState(VK_CONTROL) & 0x80 ) { // Ctrl + A 
        SendMessage(EM_SETSEL, 0, -1);
    }
    bHandled = false;
    return 0;
}*/

LRESULT CResultsListView::OnPaint(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    CRect rcClient;
    GetClientRect(rcClient);

    CPaintDC dc(m_hWnd);
    CDC dcMem;
    dcMem.CreateCompatibleDC(dc);

    CBitmap bmMem;
    bmMem.CreateCompatibleBitmap(dc, rcClient.Width(), rcClient.Height());
    HBITMAP pbmOld = dcMem.SelectBitmap(bmMem);

    dcMem.FillSolidRect(rcClient, ::GetSysColor(COLOR_WINDOW));

    this->DefWindowProc(WM_PAINT, (WPARAM)dcMem.m_hDC, (LPARAM)0);

    dc.BitBlt(0, 0, rcClient.Width(), rcClient.Height(), dcMem, 0, 0, SRCCOPY);
    dcMem.SelectBitmap(pbmOld);
    CHeaderCtrl pCtrl = GetHeader();

    if (::IsWindow(pCtrl.m_hWnd) )
    {
        CRect  aHeaderRect;
        pCtrl.GetClientRect(&aHeaderRect);
        pCtrl.RedrawWindow(&aHeaderRect);
    }
    return 0;
}

LRESULT CResultsListView::OnEraseBackground(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    bHandled = true;
    return TRUE;
}