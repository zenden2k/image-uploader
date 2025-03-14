/*

Image Uploader -  free application for uploading images/files to the Internet

Copyright 2007-2018 Sergey Svistunov (zenden2k@gmail.com)

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.

*/

#include "TabListBox.h"

LRESULT CTabListBox::OnDrawitem(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    auto* lpdis = reinterpret_cast<LPDRAWITEMSTRUCT>(lParam);
    int iItemIndex = lpdis->itemID;
    CDCHandle dc(lpdis->hDC);
    TCHAR buf[256]=_T("");
    GetText(iItemIndex, buf);
    dc.SetBkMode(TRANSPARENT);
    if(lpdis->itemState & ODS_SELECTED )
    {
        CRect roundedRect(lpdis->rcItem);
        roundedRect.DeflateRect(3,3);
        HPEN oldPen = 0;
        CPen pen;
        dc.FillRect(&lpdis->rcItem, COLOR_WINDOW);

        /* if (lpdis->itemState & ODS_FOCUS) {
            dc.DrawFocusRect(roundedRect);
        } else {*/
            pen.CreatePen(PS_SOLID, 1, 0xC5C5C5);
            oldPen = dc.SelectPen(pen);
            dc.RoundRect(roundedRect, CPoint(2, 2));
        //}
        CRect rc(lpdis->rcItem);
        rc.DeflateRect(4,4);

        TRIVERTEX vertexFocused[2] = { {rc.left, rc.top, 0xF500, 0xF200, 0xE200, 0x0000},   
                                {rc.right, rc.bottom, 0xEF00, 0xD000, 0x7700, 0x0000}};
        TRIVERTEX vertexUnfocused[2] = { { rc.left, rc.top, 0xF800, 0xF600, 0xEB00, 0x0000 },
            { rc.right, rc.bottom, 0xF400, 0xDE00, 0xA000, 0x0000 } };

        GRADIENT_RECT gradientrc = {0, 1};
        dc.GradientFill(lpdis->itemState & ODS_FOCUS ? vertexFocused : vertexUnfocused, 2, &gradientrc, 1, GRADIENT_FILL_RECT_V);
        dc.SelectPen(oldPen);
    }
    else
    {
        CBrush br;
        br.CreateSolidBrush(GetSysColor(COLOR_WINDOW));
        dc.FillRect(&lpdis->rcItem,br);
    }

    DrawText(dc, buf, lstrlen(buf), &lpdis->rcItem, DT_CENTER | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX);
    return 0;
}

LRESULT CTabListBox::OnMeasureItem(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    auto* lpmis = reinterpret_cast<LPMEASUREITEMSTRUCT>(lParam);
    CClientDC dc(m_hWnd);
    int dpiY = dc.GetDeviceCaps(LOGPIXELSY);
    int iItemIndex = lpmis->itemID;
    CString buf;
    RECT r = { 0, 0, 0, 0 };
    GetText(iItemIndex, buf);
    dc.DrawText(buf, buf.GetLength(), &r, DT_CALCRECT | DT_NOPREFIX | DT_SINGLELINE);
    lpmis->itemHeight = r.bottom - r.top + MulDiv(15, dpiY, USER_DEFAULT_SCREEN_DPI);
    bHandled = true;
    return TRUE;
}


