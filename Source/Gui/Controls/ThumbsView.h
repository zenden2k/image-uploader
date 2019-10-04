/*

    Image Uploader -  free application for uploading images/files to the Internet

    Copyright 2007-2018 Sergey Svistunov (zenden2k@yandex.ru)

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
#ifndef GUI_CONTROLS_THUMBSVIEW_H
#define GUI_CONTROLS_THUMBSVIEW_H

#pragma once

#include <deque>
#include <condition_variable>

#include "atlheaders.h"
#include "3rdpart/thread.h"
#include "Gui/Controls/ImageView.h"


// CThumbsView

struct ThumbsViewItem
{
    CString FileName;
    BOOL ThumbOutDate;
    bool ThumbnailRequested; 
    //CBitmap Image;
    bool ThumbLoaded;
    int Index;
    ThumbsViewItem() {
        ThumbOutDate = true;
        ThumbnailRequested = false;
        Index = -1;
        ThumbLoaded = false;
    }
};



class CThumbsView :
    public CWindowImpl<CThumbsView, CListViewCtrl>, public CThreadImpl<CThumbsView>, public CImageViewCallback
{
public:
    
    CImageList ImageList;
    CThumbsView();
    ~CThumbsView();
    DECLARE_WND_SUPERCLASS(_T("CThumbsView"), CListViewCtrl::GetWndClassName())
    
    BEGIN_MSG_MAP(CThumbsView)
        MESSAGE_HANDLER(WM_MBUTTONUP, OnMButtonUp)
        MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
        MSG_WM_KEYDOWN(OnKeyDown)
        MSG_WM_LBUTTONDBLCLK(OnLButtonDblClk)
        REFLECTED_NOTIFY_CODE_HANDLER(LVN_BEGINDRAG, OnLvnBeginDrag)
        REFLECTED_NOTIFY_CODE_HANDLER(LVN_DELETEITEM, OnDeleteItem)
        REFLECTED_NOTIFY_CODE_HANDLER(LVN_DELETEALLITEMS, OnDeleteItem)
        REFLECTED_NOTIFY_CODE_HANDLER(LVN_ITEMCHANGED, OnItemChanged)
        REFLECTED_NOTIFY_CODE_HANDLER(NM_CUSTOMDRAW, OnCustomDraw)
    END_MSG_MAP()

    // Handler prototypes:
    //  LRESULT MessageHandler(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    //  LRESULT CommandHandler(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
    //  LRESULT NotifyHandler(int idCtrl, LPNMHDR pnmh, BOOL& bHandled);
    LRESULT OnLvnBeginDrag(int /*idCtrl*/, LPNMHDR /*pnmh*/, BOOL& /*bHandled*/);
    LRESULT OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT OnItemChanged(int /*idCtrl*/, LPNMHDR /*pnmh*/, BOOL& /*bHandled*/);
    LRESULT OnCustomDraw(int idCtrl, LPNMHDR pnmh, BOOL& bHandled);
    typedef std::function<void(CThumbsView*, bool)> ItemCountChangedCallback;
    void SetOnItemCountChanged(ItemCountChangedCallback&& callback);
    CAutoCriticalSection ImageListCS;
    LRESULT OnMButtonUp(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
    int maxwidth,maxheight;
    void Init(bool Extended=false);
    int AddImage(LPCTSTR FileName, LPCTSTR Title, bool ensureVisible =false, Gdiplus::Image* Img=NULL);
    bool MyDeleteItem(int ItemIndex);
    int DeleteSelected();
    void UpdateImageIndexes(int StartIndex = 0);
    void MyDeleteAllItems();
    bool SimpleDelete(int ItemIndex, bool DeleteThumb = true, bool deleteFile = false);
    LPCTSTR GetFileName(int ItemIndex);
    LRESULT OnKeyDown(TCHAR vk, UINT cRepeat, UINT flags);
    bool LoadThumbnail(int ItemID, ThumbsViewItem* tvi, Gdiplus::Image *Img = NULL);
    int GetImageIndex(int ItemIndex) const;
    CImageViewWindow ImageView;
    LRESULT OnLButtonDblClk(UINT Flags, CPoint Pt);
    DWORD Run();
    void ViewSelectedImage();
    bool ExtendedView;
    void OutDateThumb(int nIndex);
    bool StopBackgroundThread(bool wait = false);
    void SelectLastItem();
    bool CopySelectedItemsToClipboard() const;
    LRESULT OnDeleteItem(int idCtrl, LPNMHDR pnmh, BOOL& bHandled);
    void SetDeletePhysicalFiles(bool doDelete);
    CImageViewItem getNextImgViewItem(const CImageViewItem& currentItem) override;
    CImageViewItem getPrevImgViewItem(const CImageViewItem& currentItem) override;
    void getThumbnail(int itemIndex);
    void clearImageList();
protected:
    ItemCountChangedCallback callback_;
    DWORD callbackLastCallTime_;
    bool deletePhysicalFiles_;
    CBitmap defaultImage_;
    void NotifyItemCountChanged(bool selected = true);
    std::deque<int> thumbQueue_;
    std::mutex thumbQueueMutex_;
    std::condition_variable thumbQueueCondition_;
};



#endif // THUMBSVIEW_H