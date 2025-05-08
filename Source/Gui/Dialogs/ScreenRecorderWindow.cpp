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

constexpr auto PANEL_HEIGHT = 40;
ScreenRecorderWindow::ScreenRecorderWindow():
        dialogResult_(drCancel),
        toolbar_(ImageEditor::Toolbar::orHorizontal, false) {
    transparentColor_ = RGB(255, 50, 56);
}

ScreenRecorderWindow::~ScreenRecorderWindow() {
    
}

LRESULT ScreenRecorderWindow::onCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/) {
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
        toolbar_.addButton(ImageEditor::Toolbar::Item(CString(TR("Stop")), loadToolbarIcon(IDB_ICONADDPNG), ID_STOP, TR("Stop")));
        toolbar_.addButton(ImageEditor::Toolbar::Item(CString(TR("Pause")), loadToolbarIcon(IDB_ICONADDPNG), ID_PAUSE, TR("Pause")));
        int index = toolbar_.addButton(ImageEditor::Toolbar::Item(CString(TR("Cancel")), loadToolbarIcon(IDB_ICONADDPNG), IDCANCEL, TR("Cancel")));

        ImageEditor::Toolbar::Item timeLabel(CString(), loadToolbarIcon(IDB_ICONUNDOPNG), IDCANCEL, CString(), ImageEditor::Toolbar::itLabel, false);

        timeDelegate_ = std::make_unique<TimeDelegate>(&toolbar_, index + 1);
        timeLabel.itemDelegate = timeDelegate_.get();
        toolbar_.addButton(timeLabel);
        toolbar_.SetWindowPos(nullptr, clientRect.left, clientRect.bottom - toolbarHeight, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
        toolbar_.AutoSize();
        toolbar_.ShowWindow(SW_SHOW);
    }
    auto* settings = ServiceLocator::instance()->settings<WtlGuiSettings>();
    CString folder = U2W(settings->ScreenRecordingSettings.OutDirectory);

    if (folder.IsEmpty()) {
        folder = WinUtils::GetSystemSpecialPath(CSIDL_MYVIDEO);
    }

    time_t now = time(nullptr);
    tm timeStruct;
    localtime_s(&timeStruct, &now);

    CString fileName;
    fileName.Format(
        _T("%s\\capture %04d-%02d-%02d %02d-%02d-%02d.mp4"),
        folder.GetString(),
        timeStruct.tm_year + 1900, 
        timeStruct.tm_mon + 1, 
        timeStruct.tm_mday,
        timeStruct.tm_hour, 
        timeStruct.tm_min, 
        timeStruct.tm_sec
    );

    if (settings->ScreenRecordingSettings.Backend == ScreenRecordingStruct::ScreenRecordingBackendFFmpeg) {
        screenRecorder_ = std::make_unique<FFmpegScreenRecorder>(settings->ScreenRecordingSettings.FFmpegCLIPath, W2U(fileName), captureRect_);
    }
    screenRecorder_->start();
    screenRecorder_->addStatusChangeCallback([this](auto status) { statusChangeCallback(status); });
    updateTimeLabel();
    SetTimer(kTimer, 100);
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
        ::SetActiveWindow(parent);
    }
    loop.RemoveMessageFilter(this);
    DestroyWindow();
   
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
        || screenRecorder_->status() == ScreenRecorder::Status::Finished
        ) {
        endDialog(drCancel);
    } else {
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

    if (screenRecorder_->isRunning()) {
        screenRecorder_->pause();
    } else {
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

void ScreenRecorderWindow::statusChangeCallback(ScreenRecorder::Status status) {
    ServiceLocator::instance()->taskRunner()->runInGuiThread([this, status] {
        if (status == ScreenRecorder::Status::Recording) {
            //toolbar_.setMoveParent(false);
            SetTimer(kTimer, 100);
        } else {
            KillTimer(kTimer);
        }

        toolbar_.setMoveParent(status == ScreenRecorder::Status::Paused);

        if (status == ScreenRecorder::Status::Finished) {
            outFileName_ = U2W(screenRecorder_->outFileName());
            endDialog(drSuccess);
        } else if (status == ScreenRecorder::Status::Canceled) {
            endDialog(drCancel);
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

SIZE TimeDelegate::CalcItemSize(ImageEditor::Toolbar::Item& item, int x, int y, float dpiScaleX, float dpiScaleY) {
    SIZE sz = { 120 * dpiScaleX, 25 * dpiScaleY };
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
