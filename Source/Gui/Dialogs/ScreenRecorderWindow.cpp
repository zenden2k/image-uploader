#include "ScreenRecorderWindow.h"
#include "Core/Logging.h"
#include "Core/i18n/Translator.h"
#include "Gui/GuiTools.h"
#include "Core/Images/Utils.h"
#include "Core/Settings/WtlGuiSettings.h"
#include "ImageEditor/Gui/ImageEditorWindow.h"

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
        toolbar_.addButton(ImageEditor::Toolbar::Item(CString(TR("Stop")), loadToolbarIcon(IDB_ICONADDPNG), 0, TR("Stop")));
        toolbar_.addButton(ImageEditor::Toolbar::Item(CString(TR("Pause")), loadToolbarIcon(IDB_ICONADDPNG), 0, TR("Pause")));
        int index = toolbar_.addButton(ImageEditor::Toolbar::Item(CString(TR("Cancel")), loadToolbarIcon(IDB_ICONADDPNG), IDCANCEL, TR("Cancel")));

        ImageEditor::Toolbar::Item timeLabel(CString(), loadToolbarIcon(IDB_ICONUNDOPNG), IDCANCEL, CString(), ImageEditor::Toolbar::itButton, false);

        timeDelegate_ = std::make_unique<TimeDelegate>(&toolbar_, index + 1);
        timeLabel.itemDelegate = timeDelegate_.get();
        toolbar_.addButton(timeLabel);
        timeDelegate_->setText(_T("Test"));

        toolbar_.SetWindowPos(nullptr, clientRect.left, clientRect.bottom - toolbarHeight, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
        toolbar_.AutoSize();
        toolbar_.ShowWindow(SW_SHOW);
    }
    auto* settings = ServiceLocator::instance()->settings<WtlGuiSettings>();
    screenRecorder_ = std::make_unique<ScreenRecorder>(settings->ScreenRecordingSettings.FFmpegCLIPath, captureRect_);
    screenRecorder_->start();
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
    endDialog(drCancel);
    return 0;
}

LRESULT ScreenRecorderWindow::onTimer(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/) {
    elapsedTime_++;
    updateTimeLabel();
    return 0;
}


void ScreenRecorderWindow::updateTimeLabel() {
    CString timeLabel;
    timeLabel.Format(_T("%02d:%02d:%02d.%d"), (int)(elapsedTime_ / 36000), (int)(elapsedTime_ / 600 % 60), elapsedTime_ / 10 % 60, elapsedTime_ % 10);
    timeDelegate_->setText(timeLabel);
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

SIZE TimeDelegate::CalcItemSize(ImageEditor::Toolbar::Item& item, float dpiScaleX, float dpiScaleY) {
    SIZE sz = { 120 * dpiScaleX, 25 * dpiScaleY };
    return sz;
}

void TimeDelegate::DrawItem(ImageEditor::Toolbar::Item& item, Gdiplus::Graphics* gr, int x, int y, float dpiScaleX, float dpiScaleY) {
    using namespace Gdiplus;
    SolidBrush brush(Gdiplus::Color(0, 0, 0));
    SIZE sz = CalcItemSize(item, dpiScaleX, dpiScaleY);
    StringFormat format;
    format.SetLineAlignment(StringAlignmentCenter);
    format.SetAlignment(StringAlignmentCenter);
    gr->DrawString(text_, text_.GetLength(), font_.get(), RectF(x, y, sz.cx, sz.cy), &format, &brush);
}
