/*

    Image Uploader -  free application for uploading images/files to the Internet

    Copyright 2007-2015 Sergey Svistunov (zenden2k@gmail.com)

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

#include "atlheaders.h"
#include "ImageView.h"

// CImageView
CImageView::CImageView()
{
}

CImageView::~CImageView()
{ 
}

LRESULT CImageView::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    RECT rc = {380, 37, 636, 240};
    Img.Create(m_hWnd, rc);
    Img.HideParent = true;
    SetFocus();
    return 0;  // Let the system set the focus
}

LRESULT CImageView::OnClickedOK(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
    return ShowWindow(SW_HIDE);
}


LRESULT CImageView::OnKillFocus(HWND hwndNewFocus)
{
    return ShowWindow(SW_HIDE);
}

bool CImageView::ViewImage(LPCTSTR FileName,HWND Parent)
{
    Gdiplus::Image img(FileName);
    
    float width = static_cast<float>(GetSystemMetrics(SM_CXSCREEN)-12);
    float height = static_cast<float>(GetSystemMetrics(SM_CYSCREEN)-50);
    float imgwidth = static_cast<float>(img.GetWidth());
    float imgheight = static_cast<float>(img.GetHeight());
    float newheight, newwidth;
    newwidth = imgwidth;
    newheight = imgheight;
    
    if(newwidth>width || newheight>height)
    {
        float k1 = imgwidth/imgheight;
        float k2 = width/height;
        if(k1 >= k2) 
        {
            newwidth = width;
            newheight = newwidth/imgwidth*imgheight;
        }
        else
        {
            newheight = height;
            newwidth = (newheight/imgheight)*imgwidth;
        }
    }
    
    int realwidth = static_cast<int>(newwidth + 2);
    int realheight = static_cast<int>(newheight + 2);

    if(realwidth<200) realwidth = 200;
    if(realheight<180) realheight = 180;
    ShowWindow(SW_HIDE);
    if(realwidth && realheight)
    {
        MoveWindow(0, 0, realwidth, realheight);
        Img.MoveWindow(0, 0, realwidth, realheight);
        Img.LoadImage(FileName, &img);
    }
    CenterWindow(Parent);
    ShowWindow(SW_SHOW);
    SetForegroundWindow(m_hWnd);
    return false;
}

LRESULT CImageView::OnActivate(UINT state, BOOL fMinimized, HWND hwndActDeact)
{
    if (state == WA_INACTIVE) 
        return ShowWindow(SW_HIDE);
    return 0;
}

LRESULT CImageView::OnKeyDown(TCHAR vk, UINT cRepeat, UINT flags)
{
    return ShowWindow(SW_HIDE);
}