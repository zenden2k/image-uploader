#include "ScreenRecorderWindow.h"

#include <ctime>

#include "Core/Logging.h"
#include "Core/i18n/Translator.h"
#include "Gui/GuiTools.h"
#include "Core/Images/Utils.h"
#include "Core/Settings/WtlGuiSettings.h"
#include "ImageEditor/Gui/ImageEditorWindow.h"
#include "Core/TaskDispatcher.h"
#include "ScreenCapture/ScreenRecorder/FFmpegScreenRecorder.h"
#include "ScreenCapture/ScreenRecorder/DXGIScreenRecorder.h"

constexpr auto PANEL_HEIGHT = 40;

/*
namespace {

// {45CE8BB0-4973-4560-88DF-72A7CC407FF9}
static const GUID ScreenRecorderBaseGUID = { 0x45ce8bb0, 0x4973, 0x4560, { 0x88, 0xdf, 0x72, 0xa7, 0xcc, 0x40, 0x7f, 0xf9 } };

}
*/

ScreenRecorderWindow::ScreenRecorderWindow():
        dialogResult_(drCancel),
        toolbar_(ImageEditor::Toolbar::orHorizontal, false) {
    transparentColor_ = RGB(255, 50, 56);
    memset(&trayIconGuid_, 0, sizeof(GUID));
}

ScreenRecorderWindow::~ScreenRecorderWindow() {
    
}

LRESULT ScreenRecorderWindow::onCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/) {
    GuiTools::SetWindowPointer(m_hWnd, this);
    SetLayeredWindowAttributes(m_hWnd, transparentColor_, 0, LWA_COLORKEY);

    CWindowDC hdc(nullptr);
    int dpiX = GetDeviceCaps(hdc, LOGPIXELSX);
    int dpiY = GetDeviceCaps(hdc, LOGPIXELSY);

    CRect clientRect;
    GetClientRect(clientRect);

    int toolbarHeight = MulDiv(PANEL_HEIGHT, dpiX, USER_DEFAULT_SCREEN_DPI);

    if (toolbar_.Create(m_hWnd, false, true, transparentColor_, false)) {
        toolbar_.setShowButtonText(true);

        auto loadToolbarIcon = [](int resource)
        {
            return std::shared_ptr<Gdiplus::Bitmap>(ImageUtils::BitmapFromResource(GetModuleHandle(0), MAKEINTRESOURCE(resource), _T("PNG")));
        };
        toolbar_.addButton(ImageEditor::Toolbar::Item(TR("Finish"), loadToolbarIcon(IDB_ICONOK), ID_STOP, TR("Finish")));
        iconPause_ = loadToolbarIcon(IDB_ICONPAUSE);
        iconResume_ = loadToolbarIcon(IDB_ICONPLAY);
        toolbar_.addButton(ImageEditor::Toolbar::Item(TR("Pause"), iconPause_, ID_PAUSE, TR("Pause")));

        int index = toolbar_.addButton(ImageEditor::Toolbar::Item(CString(TR("Cancel")), loadToolbarIcon(IDB_ICONCANCEL), IDCANCEL, TR("Cancel")));

        ImageEditor::Toolbar::Item timeLabel(CString(), loadToolbarIcon(IDB_ICONUNDOPNG), IDCANCEL, CString(), ImageEditor::Toolbar::itLabel, false);

        timeDelegate_ = std::make_unique<TimeDelegate>(&toolbar_, index + 1);
        timeLabel.itemDelegate = timeDelegate_.get();
        toolbar_.addButton(timeLabel);
        toolbar_.SetWindowPos(nullptr, clientRect.left, clientRect.bottom - toolbarHeight, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
        toolbar_.AutoSize();
        toolbar_.ShowWindow(SW_SHOW);
    }
    auto* settings = ServiceLocator::instance()->settings<WtlGuiSettings>();

    icon_.LoadIconMetric(IDI_ICONRECORD, LIM_SMALL);
    GUID* guid = nullptr;
    
    // Use GUID for uniquely identifying the system tray icon.
    // Generating fake GUID ensures that the icon is consistently identified across sessions, even if the app is restarted from a different folder.
    // Note: On Windows 7, using the same GUID but from a different path (folder) might fail to add the icon due to strict path checks.
    // On Windows 10/11, GUID is considered the unique identifier, and the icon can be added even from a different path.
    // trayIconGuid_ = WinUtils::GenerateFakeUUIDv4();
    // guid = &trayIconGuid_;

    CString iconTitle = TR("Image Uploader (screen recording)");
    if (!InstallIcon(iconTitle, icon_, NULL, guid)) {
        LOG(WARNING) << "Failed to create tray icon!";
        /*guid = nullptr;
        if (!InstallIcon(iconTitle, icon_, NULL)) {
            LOG(WARNING) << "Failed to create tray icon again. I give up.";
        }*/
    }

    NOTIFYICONDATA nid;
    ZeroMemory(&nid, sizeof(nid));
    nid.cbSize = sizeof(NOTIFYICONDATA);
    nid.hWnd = m_hWnd;
    nid.uVersion = NOTIFYICON_VERSION_4;
    if (guid){
        nid.uFlags = NIF_GUID;
        nid.guidItem = trayIconGuid_;
    }
    Shell_NotifyIcon(NIM_SETVERSION, &nid);

    CString folder = U2W(settings->ScreenRecordingSettings.OutDirectory);

    if (folder.IsEmpty()) {
        folder = WinUtils::GetSystemSpecialPath(CSIDL_MYVIDEO);
    }

    if (!WinUtils::IsDirectory(folder)) {
        LOG(INFO) << "Creating folder " << folder;
        if (!WinUtils::CreateFolder(folder)) {
            std::wstring msg = str(IuStringUtils::FormatWideNoExcept(TR("Failed to create the directory '%s'.")) % folder);
            GuiTools::LocalizedMessageBox(m_hWnd, msg.c_str(), TR("Error"), MB_ICONERROR);
        }
    }

    time_t now = time(nullptr);
    tm timeStruct;
    localtime_s(&timeStruct, &now);

    CString fileName;
    fileName.Format(
        _T("%s\\capture %04d-%02d-%02d %02d-%02d-%02d"),
        folder.GetString(),
        timeStruct.tm_year + 1900, 
        timeStruct.tm_mon + 1, 
        timeStruct.tm_mday,
        timeStruct.tm_hour, 
        timeStruct.tm_min, 
        timeStruct.tm_sec
    );

    if (settings->ScreenRecordingSettings.Backend == ScreenRecordingStruct::ScreenRecordingBackendFFmpeg) {
        std::string ffmpegCLIPath = settings->ScreenRecordingSettings.FFmpegSettings.FFmpegCLIPath;
        if (ffmpegCLIPath.empty()) {
            ffmpegCLIPath = FFMpegOptionsManager::findFFmpegExecutable();
        }
        if (!ffmpegCLIPath.empty()) {
            FFmpegOptions options;
            const auto& ffmpegSettings = settings->ScreenRecordingSettings.FFmpegSettings;

            options.framerate = settings->ScreenRecordingSettings.FrameRate;
            options.source = ffmpegSettings.VideoSourceId;
            options.codec = ffmpegSettings.VideoCodecId;
            options.preset = ffmpegSettings.VideoPresetId;
            options.quality = ffmpegSettings.VideoQuality;
            options.bitrate = ffmpegSettings.VideoBitrate;
            options.useQuality = ffmpegSettings.UseQuality;
            options.audioSource = ffmpegSettings.AudioSourceId;
            options.audioCodec = ffmpegSettings.AudioCodecId;
            options.audioQuality = ffmpegSettings.AudioQuality;

            /* if (options.codec == NvencVideoCodec::H264_CODEC_ID) {
                options.source = DDAGrabSource::SOURCE_ID;
            }*/

            screenRecorder_ = std::make_shared<FFmpegScreenRecorder>(ffmpegCLIPath, W2U(fileName), captureRect_, std::move(options));
        } else {
            GuiTools::LocalizedMessageBox(m_hWnd, TR("Could not find ffmpeg executable!"), TR("Error"), MB_ICONERROR);
        }
    } else if (settings->ScreenRecordingSettings.Backend == ScreenRecordingStruct::ScreenRecordingBackendDirectX) {  
            DXGIOptions options;
            const auto& dxgiSettings = settings->ScreenRecordingSettings.DXGISettings;

            options.framerate = settings->ScreenRecordingSettings.FrameRate;
            options.codec = dxgiSettings.VideoCodecId;
            options.preset = dxgiSettings.VideoPresetId;
            options.quality = dxgiSettings.VideoQuality;
            options.bitrate = dxgiSettings.VideoBitrate;
            options.useQuality = dxgiSettings.UseQuality;
            options.audioSource = dxgiSettings.AudioSourceId;
            options.audioCodec = dxgiSettings.AudioCodecId;
            options.audioBitrate = dxgiSettings.AudioBitrate;

            screenRecorder_ = std::make_shared<DXGIScreenRecorder>(W2U(fileName), captureRect_, std::move(options));
    }
    if (!screenRecorder_) {
        return -1;
    }

    screenRecorder_->addStatusChangeCallback(
        ScreenRecorder::StatusChangeSignal::slot_type(
            std::bind(&ScreenRecorderWindow::statusChangeCallback, this, std::placeholders::_1)
        )
        .track(shared_from_this())
    );

    screenRecorder_->start();

    updateTimeLabel();
    SetTimer(kTimer, 100);
    return 0;
}

LRESULT ScreenRecorderWindow::onDestroy(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/) {
    GuiTools::ClearWindowPointer(m_hWnd);
    return 0;
}

ScreenRecorderWindow::DialogResult ScreenRecorderWindow::doModal(HWND parent, CRect captureRect) {
    if (captureRect.IsRectEmpty()) {
        LOG(ERROR) << "Capture rectangle is empty";
        return drCancel;
    }
    CWindowDC hdc(nullptr);
    int dpiX = GetDeviceCaps(hdc, LOGPIXELSX);
    int dpiY = GetDeviceCaps(hdc, LOGPIXELSY);

    captureRect_ = captureRect;

    DWORD initialWindowStyle = WS_POPUP | WS_CLIPCHILDREN;
    CRect windowRect = captureRect;
    windowRect.InflateRect(1, 1, 1, 1);
    windowRect.bottom += MulDiv(PANEL_HEIGHT, dpiX, USER_DEFAULT_SCREEN_DPI);
    CRgn exclude;

    // Parent window should be null here!
    if (Create(nullptr, windowRect, _T("Screen Recorder"), initialWindowStyle, WS_EX_TOPMOST | WS_EX_TOOLWINDOW | WS_EX_NOACTIVATE  | WS_EX_LAYERED) == NULL) {
        LOG(ERROR) << "Screen Recorder window creation failed!\n";
        return drCancel;
    }

    icon_ = GuiTools::LoadBigIcon(IDR_MAINFRAME);
    iconSmall_ = GuiTools::LoadSmallIcon(IDR_MAINFRAME);

    SetIcon(icon_, TRUE);
    SetIcon(iconSmall_, FALSE);
    
    ShowWindow(SW_SHOW);
    SetForegroundWindow(m_hWnd);
    //BringWindowToTop();
    //m_view.Invalidate(false);

    if (parent) {
        ::EnableWindow(parent, false);
    }

    CMessageLoop loop;
    loop.AddMessageFilter(this);
    loop.Run();

    if (parent) {
        ::EnableWindow(parent, true);
    }
    ShowWindow(SW_HIDE);
    if (parent) {
        ::SetForegroundWindow(parent);
        ::SetActiveWindow(parent); 
        ::SetFocus(parent); 
    }

    DestroyWindow();
    loop.RemoveMessageFilter(this);

    return dialogResult_;
}

void ScreenRecorderWindow::createToolbar() {

}

void ScreenRecorderWindow::endDialog(DialogResult dr) {
    dialogResult_ = dr;
    PostQuitMessage(0);
}

BOOL ScreenRecorderWindow::PreTranslateMessage(MSG* pMsg) {
    /*if (TranslateAccelerator(m_hWnd, accelerators_, pMsg)) {
        return TRUE;
    }*/
    return FALSE;
}

LRESULT ScreenRecorderWindow::onPaint(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/) {
    CRect clientRect;
    GetClientRect(clientRect);
    CPaintDC dc(m_hWnd);
    int dpiX = dc.GetDeviceCaps(LOGPIXELSX);
    int dpiY = dc.GetDeviceCaps(LOGPIXELSY);
    CBrush br;
    br.CreateSolidBrush(transparentColor_);

    dc.FillRect(clientRect, br);
    int panelHeight = MulDiv(PANEL_HEIGHT, dpiX, USER_DEFAULT_SCREEN_DPI);
    CPen pen;
    pen.CreatePen(PS_DASH, 1, RGB(0, 255, 0));
    HPEN oldPen = dc.SelectPen(pen);
    HBRUSH oldBr = dc.SelectBrush(br);
    dc.SetBkMode(TRANSPARENT);
    dc.Rectangle(clientRect.left, clientRect.top, clientRect.right, clientRect.bottom - panelHeight);
    dc.SelectBrush(oldBr);
    dc.SelectPen(oldPen);
    return 0;
}

LRESULT ScreenRecorderWindow::onEraseBkgnd(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled) {
    bHandled = true;
    return 1;
}

LRESULT ScreenRecorderWindow::onCancel(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
    if (screenRecorder_->status() == ScreenRecorder::Status::Canceled 
        || screenRecorder_->status() == ScreenRecorder::Status::Invalid
        || screenRecorder_->status() == ScreenRecorder::Status::Failed
        || screenRecorder_->status() == ScreenRecorder::Status::Finished
        //|| !screenRecorder_->isRunning() 
    ) {
        endDialog(drCancel);
    } else {
        cancelRequested_ = true;
        screenRecorder_->cancel();
    }

    return 0;
}

LRESULT ScreenRecorderWindow::onTimer(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/) {
    elapsedTime_++;
    updateTimeLabel();
    return 0;
}

LRESULT ScreenRecorderWindow::onPause(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
    CRect clientRect;
    GetClientRect(&clientRect);
    ClientToScreen(&clientRect);

    screenRecorder_->setOffset(clientRect.left + 1, clientRect.top + 1);

    if (screenRecorder_->status() == ScreenRecorder::Status::Recording) {
        //LOG(ERROR) << "screenRecorder_->pause()";
        screenRecorder_->pause();
    } else if (screenRecorder_->status() == ScreenRecorder::Status::Paused) {
        //LOG(ERROR) << "screenRecorder_->start()";
        screenRecorder_->start();
    }

    return 0;
}

void ScreenRecorderWindow::updateTimeLabel() {
    CString timeLabel;
    timeLabel.Format(_T("%02d:%02d:%02d.%d"), (int)(elapsedTime_ / 36000), (int)(elapsedTime_ / 600 % 60), elapsedTime_ / 10 % 60, elapsedTime_ % 10);
    timeDelegate_->setText(timeLabel);
}

LRESULT ScreenRecorderWindow::onStop(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
    screenRecorder_->stop();
    return 0;
}

LRESULT ScreenRecorderWindow::onNcHitTest(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled) {
    bHandled = true;
    return HTCAPTION;
}


LRESULT ScreenRecorderWindow::onTrayIcon(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/) {
    int msg = LOWORD(lParam);
    if (msg == WM_CONTEXTMENU) {
        CPoint pos(GET_X_LPARAM(wParam), GET_Y_LPARAM(wParam)); 
        
        CMenu trayIconMenu;
        trayIconMenu.CreatePopupMenu();
        trayIconMenu.AppendMenu(MF_STRING, ID_STOP, TR("Finish"));
        CString msg;
        bool enablePauseItem = true;
        msg = TR("Pause");
        switch (screenRecorder_->status()) {
        case ScreenRecorder::Status::Recording:
            break;
        case ScreenRecorder::Status::Paused:
            msg = TR("Resume");
            break;
        default:
            enablePauseItem = false;
        }

        trayIconMenu.AppendMenu(MF_STRING, ID_PAUSE, msg);
        trayIconMenu.EnableMenuItem(ID_PAUSE, MF_BYCOMMAND | ( enablePauseItem ? MF_ENABLED : MF_DISABLED));
        trayIconMenu.AppendMenu(MF_STRING, IDCANCEL, TR("Cancel"));
        trayIconMenu.SetMenuDefaultItem(ID_STOP);

        trayIconMenu.TrackPopupMenu( TPM_LEFTALIGN | TPM_RIGHTBUTTON, pos.x, pos.y, m_hWnd);
    } else if (msg == WM_LBUTTONDBLCLK || msg == WM_LBUTTONUP) {
        SendMessage(WM_COMMAND, MAKEWPARAM(ID_STOP, 0));
    }
    return 0;
}

void ScreenRecorderWindow::statusChangeCallback(ScreenRecorder::Status status) {
    auto self = shared_from_this();
    ServiceLocator::instance()->taskRunner()->runInGuiThread([this, self, wnd = this->m_hWnd, status] {
        if (!GuiTools::CheckWindowPointer(wnd, this)) {
            return;
        }
        if (status == ScreenRecorder::Status::Recording) {
            SetTimer(kTimer, 100);
        } else {
            KillTimer(kTimer);
        }

        toolbar_.setMoveParent(status == ScreenRecorder::Status::Paused);

        if (status == ScreenRecorder::Status::Finished) {
            outFileName_ = U2W(screenRecorder_->outFileName());
            endDialog(drSuccess);
        } else if (
            status == ScreenRecorder::Status::Canceled
            || status == ScreenRecorder::Status::Failed
            || status == ScreenRecorder::Status::Paused
        ) {
            if (status == ScreenRecorder::Status::Failed) {
                GuiTools::LocalizedMessageBox(wnd, TR("An error occurred during screen recording. For details, please check the error log."), TR("Error"), MB_ICONERROR);
            }
            if (cancelRequested_) {
                endDialog(drCancel);
            }
        }

        int pauseItemIndex = toolbar_.getItemIndexByCommand(ID_PAUSE);
        auto item = toolbar_.getItem(pauseItemIndex);
        CString newTitle = status == ScreenRecorder::Status::Paused ? TR("Resume") : TR("Pause");
        if (item->title != newTitle) {
            item->title = newTitle;
            item->hint = newTitle;
            item->icon = status == ScreenRecorder::Status::Paused ? iconResume_ : iconPause_;
            toolbar_.update();
        }

    }, true);
    previousStatus_ = status;
}

CString ScreenRecorderWindow::outFileName() const {
    return outFileName_;
}

TimeDelegate::TimeDelegate(ImageEditor::Toolbar* toolbar, int itemIndex):  toolbarItemIndex_(itemIndex),
                                                                           toolbar_(toolbar)
{
    CFont systemFont = GuiTools::GetSystemDialogFont();
    CWindowDC dc(toolbar->m_hWnd);
    font_ = std::make_unique<Gdiplus::Font>(dc, systemFont);
}

void TimeDelegate::setText(CString text) {
    text_ = text;
    if (toolbar_->m_hWnd) {
        toolbar_->repaintItem(toolbarItemIndex_);
    }
}

bool TimeDelegate::needClick() {
    return false;
}

SIZE TimeDelegate::CalcItemSize(ImageEditor::Toolbar::Item& item, int x, int y, float dpiScaleX, float dpiScaleY) {
    SIZE sz = { static_cast<LONG>(120 * dpiScaleX), static_cast<LONG>(25 * dpiScaleY) };
    return sz;
}

void TimeDelegate::DrawItem(ImageEditor::Toolbar::Item& item, Gdiplus::Graphics* gr, int x, int y, float dpiScaleX, float dpiScaleY) {
    using namespace Gdiplus;
    SolidBrush brush(Gdiplus::Color(0, 0, 0));
    SIZE sz = CalcItemSize(item, x, y, dpiScaleX, dpiScaleY);
    StringFormat format;
    format.SetLineAlignment(StringAlignmentCenter);
    format.SetAlignment(StringAlignmentCenter);
    gr->DrawString(text_, text_.GetLength(), font_.get(), RectF(x, y, sz.cx, sz.cy), &format, &brush);
}
