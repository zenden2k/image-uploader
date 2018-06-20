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

// MainDlg.h : interface of the CMainDlg class
//
/////////////////////////////////////////////////////////////////////////////

#include <map>
#include "atlheaders.h"
#include "resource.h"
#include "3rdpart/thread.h"
#include "Core/Upload/UploadEngine.h"
#include "Core/FileDownloader.h"
#include "Func/MyEngineList.h"
#include "Core/Utils/SimpleXml.h"

class UploadManager;
class UploadEngineManager;
class NetworkClient;

struct ServerData
{
    int stars[3];
    COLORREF color;
    int fileToCheck;
    int filesChecked;
    int timeElapsed;
};

struct UploadTaskUserData {
    int rowIndex;
    DWORD startTime;
    UploadTaskUserData() {
        rowIndex = 0;
        startTime = 0;
    }
};
struct MyFileInfo
{
    int width;
    int height;
    CString mimeType;
    int size;
};
class CMainDlg :
    public CDialogImpl<CMainDlg>, public CThreadImpl<CMainDlg>, 
     public  CDialogResize <CMainDlg>
{
public:
    enum { IDD = IDD_MAINDLG };
    CMainDlg(UploadEngineManager *uploadEngineManager, UploadManager* uploadManager, CMyEngineList* engineList);
    BEGIN_MSG_MAP(CMainDlg)
        MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
        COMMAND_ID_HANDLER(ID_APP_ABOUT, OnAppAbout)
        COMMAND_ID_HANDLER(IDOK, OnOK)
        COMMAND_ID_HANDLER(IDC_BUTTONSKIP, OnSkip)
        COMMAND_ID_HANDLER(IDC_BUTTONSKIPALL, OnSkipAll)
        COMMAND_ID_HANDLER(IDCANCEL, OnCancel)
        COMMAND_ID_HANDLER(IDC_ERRORLOGBUTTON, OnErrorLogButtonClicked)
        COMMAND_ID_HANDLER(IDC_TOOLBROWSEBUTTON, OnBrowseButton)
        COMMAND_HANDLER(IDC_STOPBUTTON, BN_CLICKED, OnBnClickedStopbutton)
        NOTIFY_HANDLER(IDC_TOOLSERVERLIST, NM_CUSTOMDRAW, OnListViewNMCustomDraw)
        CHAIN_MSG_MAP(CDialogResize<CMainDlg>) 
    END_MSG_MAP()

    BEGIN_DLGRESIZE_MAP(CMainDlg)
        DLGRESIZE_CONTROL(IDC_TOOLSERVERLIST, DLSZ_SIZE_X|DLSZ_SIZE_Y)
        DLGRESIZE_CONTROL(ID_APP_ABOUT, DLSZ_MOVE_X)
        DLGRESIZE_CONTROL(IDOK, DLSZ_MOVE_X|DLSZ_MOVE_Y)
        DLGRESIZE_CONTROL(IDC_BUTTONSKIP, DLSZ_MOVE_Y)
        DLGRESIZE_CONTROL(IDC_BUTTONSKIPALL, DLSZ_MOVE_Y)
        DLGRESIZE_CONTROL(IDC_ERRORLOGBUTTON, DLSZ_MOVE_Y)
        DLGRESIZE_CONTROL(IDCANCEL, DLSZ_MOVE_X|DLSZ_MOVE_Y)
        DLGRESIZE_CONTROL(IDC_STOPBUTTON, DLSZ_MOVE_X | DLSZ_MOVE_Y)
    END_DLGRESIZE_MAP()

// Handler prototypes (uncomment arguments if needed):
//    LRESULT MessageHandler(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
//    LRESULT CommandHandler(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
//    LRESULT NotifyHandler(int /*idCtrl*/, LPNMHDR /*pnmh*/, BOOL& /*bHandled*/)

    LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
    LRESULT OnAppAbout(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
    LRESULT OnOK(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
    LRESULT OnCancel(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
    LRESULT OnSkip(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
    LRESULT OnBrowseButton(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
    LRESULT OnErrorLogButtonClicked(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
    LRESULT OnSkipAll(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);

    CListViewCtrl m_ListView;
    DWORD Run();
    void stop();
    bool m_NeedStop;
    bool m_bIsRunning;
    bool isRunning();
    std::map<int,ServerData> m_CheckedServersMap;
    std::map<int, bool> m_skipMap;
    CImageList m_ImageList;
    CString m_srcFileHash;
    MyFileInfo m_sourceFileInfo;
    void OnConfigureNetworkClient(NetworkClient* nm);
    void OnConfigureNetworkClient(CUploader* uploader, NetworkClient* nm);
    void onTaskStatusChanged(UploadTask* task);
    void onTaskFinished(UploadTask* task, bool ok);
    void onSessionFinished(UploadSession* session);
    virtual bool OnFileFinished(bool ok, int statusCode,  CFileDownloader::DownloadFileListItem it);
    void MarkServer(int id);
    CFileDownloader m_FileDownloader;
    LRESULT OnListViewNMCustomDraw(int idCtrl, LPNMHDR pnmh, BOOL& bHandled);
    bool OnNeedStop();
    void processFinished();
    void checkShortUrl(UploadTask* task);
    UploadEngineManager *uploadEngineManager_;
    UploadManager* uploadManager_;
    CMyEngineList* engineList_;
    std::shared_ptr<UploadSession> uploadSession_;
    SimpleXml xml;
    LRESULT OnBnClickedStopbutton(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
};
