// This file was generated by WTL subclass control wizard
// LogListBox.h : Declaration of the CLogListBox

#pragma once
#include <queue>
#include <map>
#include <mutex>
#include <memory>
#include "atlheaders.h"
#include <atltheme.h>
#include <atlmisc.h>

#include "3rdpart/thread.h"
#include "Core/HistoryManager.h"
#include "Core/FileDownloader.h"
#include "Core/3rdpart/FastDelegate.h"
#include "CustomTreeControl.h"

class INetworkClientFactory;

struct HistoryTreeItem
{
    HistoryItem* hi;
    HBITMAP thumbnail;
    bool ThumbnailRequested;
    std::string thumbnailSource;
    HistoryTreeItem() :hi(nullptr), thumbnail(nullptr), ThumbnailRequested(false){
        
    }

    ~HistoryTreeItem() {
        delete hi;
    }
};

class CHistoryTreeControlCallback {
public:
    virtual void OnItemDblClick(TreeItem* item) = 0;
    virtual ~CHistoryTreeControlCallback() = default;
};

class CHistoryTreeControl :
    public CCustomTreeControlImpl<CHistoryTreeControl>,
    public CThemeImpl <CHistoryTreeControl>,
    protected CThreadImpl<CHistoryTreeControl>,
    public TreeItemCallback
{
    public:
        CHistoryTreeControl(std::shared_ptr<INetworkClientFactory> factory);
        ~CHistoryTreeControl();
        DECLARE_WND_SUPERCLASS(_T("CHistoryTreeControl"), CListViewCtrl::GetWndClassName())

        BEGIN_MSG_MAP(CHistoryTreeControl)
            MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
            CHAIN_MSG_MAP(CCustomTreeControlImpl<CHistoryTreeControl>)
        END_MSG_MAP()

        // Handler prototypes:
        //  LRESULT MessageHandler(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
        //  LRESULT CommandHandler(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
        //  LRESULT NotifyHandler(int idCtrl, LPNMHDR pnmh, BOOL& bHandled);
        LRESULT OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
        void DrawTreeItem(HDC dc, RECT rc, UINT itemState,  TreeItem* item) override;
        DWORD OnItemPrePaint(int /*idCtrl*/, LPNMCUSTOMDRAW /*lpNMCustomDraw*/);
        DWORD OnSubItemPrePaint(int /*idCtrl*/, LPNMCUSTOMDRAW /*lpNMCustomDraw*/);
        bool LoadThumbnail(HistoryTreeItem* ItemID);
        LRESULT OnDblClick(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/) override;
        fastdelegate::FastDelegate0<> onThreadsFinished;
        fastdelegate::FastDelegate0<> onThreadsStarted;
        fastdelegate::FastDelegate1<TreeItem*> onItemDblClick;
        void setDownloadingEnabled(bool enabled);
        bool m_bIsRunning;
        int m_thumbWidth;
        bool downloading_enabled_;
        void addSubEntry(TreeItem* res, HistoryItem* it, bool autoExpand);
        TreeItem*  addEntry(CHistorySession* session, const CString& text);
        void Init();
        void Clear();
        bool IsItemAtPos(int x, int y, bool& isRoot);
        TreeItem* selectedItem();
        void TreeItemSize( TreeItem* item, SIZE* sz) override;
        DWORD Run();
        bool isRunning() const;
        void StartLoadingThumbnails();
        void OnTreeItemDelete(TreeItem* item) override;
        void CreateDownloader();
        void abortLoadingThreads();
        LRESULT ReflectContextMenu(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
        void ResetContent();
        static HistoryItem* getItemData(const TreeItem* res);

    private:
        std::map<CString, HICON> m_fileIconCache;
        std::map<CString, HICON> m_serverIconCache;
        HICON getIconForExtension(const CString& fileName);
        HICON getIconForServer(const CString& serverName);
        int CalcItemHeight(TreeItem* item);
        LRESULT OnLButtonDown(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
        void _DrawItem(TreeItem* res, HDC dc, DWORD itemState, RECT invRC, int* outHeight);
        void DrawSubItem(TreeItem* res, HDC dc, DWORD itemState, RECT invRC,  int* outHeight);
        LRESULT OnLButtonDoubleClick(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
        HBITMAP GetItemThumbnail(HistoryTreeItem* item);
        std::deque<HistoryTreeItem*> m_thumbLoadingQueue;
        std::mutex m_thumbLoadingQueueMutex;
        std::unique_ptr<CFileDownloader> m_FileDownloader;
        std::shared_ptr<INetworkClientFactory> networkClientFactory_;
        bool OnFileFinished(bool ok, int statusCode, const CFileDownloader::DownloadFileListItem& it);
        void DownloadThumb(HistoryTreeItem* it);
        int m_SessionItemHeight;
        int m_SubItemHeight;
        void QueueFinishedEvent();
        void threadsFinished();
        void OnConfigureNetworkClient(INetworkClient* nm);
        static void DrawBitmap(HDC hdc, HBITMAP bmp, int x, int y);
};
