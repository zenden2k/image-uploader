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
#include "Func/MyEngineList.h"
#include "Core/Utils/SimpleXml.h"
#include "ServerListTool/ServersCheckerModel.h"
#include "ServersChecker.h"
#include "Helpers.h"
#include "ServerListView.h"
#include "Core/TaskDispatcher.h"

class UploadManager;
class UploadEngineManager;
class NetworkClient;

namespace ServersListTool {

class ServersChecker;

class CMainDlg :
    public CDialogImpl<CMainDlg>, public CDialogResize<CMainDlg>, public CWinDataExchange<CMainDlg>, public ITaskDispatcher {
public:
    enum { IDD = IDD_MAINDLG };
    enum {
        ID_COPYDIRECTURL = 13000, ID_COPYTHUMBURL, ID_COPYVIEWURL, WM_TASKDISPATCHERMSG = WM_USER + 100
    };
    CMainDlg(UploadEngineManager *uploadEngineManager, UploadManager* uploadManager, CMyEngineList* engineList, std::shared_ptr<INetworkClientFactory> factory);
    BEGIN_MSG_MAP(CMainDlg)
        MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
        MESSAGE_HANDLER(WM_CONTEXTMENU, OnContextMenu)
        MESSAGE_HANDLER(WM_TASKDISPATCHERMSG, OnTaskDispatcherMsg)
        COMMAND_ID_HANDLER(ID_APP_ABOUT, OnAppAbout)
        COMMAND_ID_HANDLER(IDOK, OnOK)
        COMMAND_ID_HANDLER(IDC_BUTTONSKIP, OnSkip)
        COMMAND_ID_HANDLER(IDC_BUTTONSKIPALL, OnSkipAll)
        COMMAND_ID_HANDLER(IDCANCEL, OnCancel)
        COMMAND_ID_HANDLER(IDC_ERRORLOGBUTTON, OnErrorLogButtonClicked)
        COMMAND_ID_HANDLER(IDC_TOOLBROWSEBUTTON, OnBrowseButton)
        COMMAND_ID_HANDLER(ID_COPYDIRECTURL, OnCopyDirectUrl)
        COMMAND_ID_HANDLER(ID_COPYTHUMBURL, OnCopyThumbUrl)
        COMMAND_ID_HANDLER(ID_COPYVIEWURL, OnCopyViewUrl)
        COMMAND_HANDLER(IDC_STOPBUTTON, BN_CLICKED, OnBnClickedStopbutton)
        //NOTIFY_HANDLER(IDC_TOOLSERVERLIST, NM_CUSTOMDRAW, OnListViewNMCustomDraw)
        CHAIN_MSG_MAP(CDialogResize<CMainDlg>)
        REFLECT_NOTIFICATIONS()
    END_MSG_MAP()

    BEGIN_DLGRESIZE_MAP(CMainDlg)
        DLGRESIZE_CONTROL(IDC_TOOLSERVERLIST, DLSZ_SIZE_X | DLSZ_SIZE_Y)
        DLGRESIZE_CONTROL(ID_APP_ABOUT, DLSZ_MOVE_X)
        DLGRESIZE_CONTROL(IDOK, DLSZ_MOVE_X | DLSZ_MOVE_Y)
        DLGRESIZE_CONTROL(IDC_BUTTONSKIP, DLSZ_MOVE_Y)
        DLGRESIZE_CONTROL(IDC_BUTTONSKIPALL, DLSZ_MOVE_Y)
        DLGRESIZE_CONTROL(IDC_ERRORLOGBUTTON, DLSZ_MOVE_Y)
        DLGRESIZE_CONTROL(IDCANCEL, DLSZ_MOVE_X | DLSZ_MOVE_Y)
        DLGRESIZE_CONTROL(IDC_STOPBUTTON, DLSZ_MOVE_X | DLSZ_MOVE_Y)
    END_DLGRESIZE_MAP()

    BEGIN_DDX_MAP(CMainDlg)
        DDX_CONTROL_HANDLE(IDC_RADIOWITHACCS, withAccountsRadioButton_)
        DDX_CONTROL_HANDLE(IDC_RADIOALWAYSACCS, alwaysWithAccountsRadioButton_)
        DDX_CONTROL_HANDLE(IDC_CHECKIMAGESERVERS, checkImageServersCheckBox_)
        DDX_CONTROL_HANDLE(IDC_CHECKFILESERVERS, checkFileServersCheckBox_)
        DDX_CONTROL_HANDLE(IDC_CHECKURLSHORTENERS, checkUrlShortenersCheckBox_)
        DDX_CONTROL(IDC_TOOLSERVERLIST, m_ListView)
    END_DDX_MAP()

    // Handler prototypes (uncomment arguments if needed):
    //    LRESULT MessageHandler(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
    //    LRESULT CommandHandler(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
    //    LRESULT NotifyHandler(int /*idCtrl*/, LPNMHDR /*pnmh*/, BOOL& /*bHandled*/)

    LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
    LRESULT OnTaskDispatcherMsg(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
    LRESULT OnContextMenu(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
    LRESULT OnAppAbout(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
    LRESULT OnOK(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
    LRESULT OnCancel(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
    LRESULT OnSkip(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
    LRESULT OnBrowseButton(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
    LRESULT OnErrorLogButtonClicked(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
    LRESULT OnSkipAll(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
    LRESULT OnCopyDirectUrl(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
    LRESULT OnCopyThumbUrl(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
    LRESULT OnCopyViewUrl(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
    LRESULT OnBnClickedStopbutton(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);

    void runInGuiThread(TaskDispatcherTask&& task, bool async = false) override;
    //LRESULT OnListViewNMCustomDraw(int idCtrl, LPNMHDR pnmh, BOOL& bHandled);

    int contextMenuItemId;
    CImageList m_ImageList;
    CString m_srcFileHash;
    Helpers::MyFileInfo m_sourceFileInfo;
    UploadEngineManager *uploadEngineManager_;
    UploadManager* uploadManager_;
    CMyEngineList* engineList_;
    ServersCheckerModel model_;
    CServerListView m_ListView;

    SimpleXml xml;
    CIcon icon_, iconSmall_;
    CButton withAccountsRadioButton_, alwaysWithAccountsRadioButton_, checkImageServersCheckBox_,
        checkFileServersCheckBox_, checkUrlShortenersCheckBox_;

    std::unique_ptr<ServersChecker> serversChecker_;
    std::shared_ptr<INetworkClientFactory> networkClientFactory_;
    bool OnNeedStop();
    void processFinished();
    int Run();
    void stop();
    bool m_NeedStop;
    bool isRunning();
};

}
