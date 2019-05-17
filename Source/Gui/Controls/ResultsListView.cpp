#include "ResultsListView.h"

CResultsListView::CResultsListView() : bmpOld_(nullptr) {
}

bool CResultsListView::AttachToDlgItem(HWND parent, UINT dlgID) {

    HWND hWnd = ::GetDlgItem(parent,dlgID);
    bool res =  SubclassWindow(hWnd)!=FALSE;
    Init();
    return res;
}

void CResultsListView::Init() {
    CreateDoubleBuffer();
}

LRESULT CResultsListView::OnPaint(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    CRect rcClient;
    GetClientRect(rcClient);
    CPaintDC dc(m_hWnd);

    this->DefWindowProc(WM_PAINT, reinterpret_cast<WPARAM>(dcMem_.m_hDC), 0);

    dc.BitBlt(0, 0, rcClient.Width(), rcClient.Height(), dcMem_, 0, 0, SRCCOPY);

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
    CRect rcClient;
    GetClientRect(rcClient);
    dcMem_.FillSolidRect(rcClient, ::GetSysColor(COLOR_WINDOW));
    return TRUE;
}

LRESULT CResultsListView::OnSize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
    int width = LOWORD(lParam);
    int height = HIWORD(lParam);

    CreateDoubleBuffer();
    return 0;
}

void CResultsListView::CreateDoubleBuffer() {
    if (dcMem_.m_hDC) {
        SelectObject(dcMem_, bmpOld_);
        dcMem_.DeleteDC();
        bmMem_.DeleteObject();
    }
    CRect rcClient;
    GetClientRect(rcClient);
    CWindowDC dc(m_hWnd);

    dcMem_.CreateCompatibleDC(dc);

    bmMem_.CreateCompatibleBitmap(dc, rcClient.Width(), rcClient.Height());
    bmpOld_ = dcMem_.SelectBitmap(bmMem_);

    dcMem_.FillSolidRect(rcClient, ::GetSysColor(COLOR_WINDOW));
}