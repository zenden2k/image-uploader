#ifndef IU_GUI_DIALOGS_SCREENRECORDERWINDOW_H
#define IU_GUI_DIALOGS_SCREENRECORDERWINDOW_H

#pragma once

#include <memory>

#include "atlheaders.h"
#include "resource.h"
#include "ImageEditor/Gui/Toolbar.h"
#include "ScreenCapture/ScreenRecorder/ScreenRecorder.h"

class TimeDelegate : public ImageEditor::Toolbar::ToolbarItemDelegate {
public:
   // enum { kOffset = 7, kSquareSize = 16, kPadding = 3 };

    TimeDelegate(ImageEditor::Toolbar* toolbar, int itemIndex);
    SIZE CalcItemSize(ImageEditor::Toolbar::Item& item, float dpiScaleX, float dpiScaleY) override;
    void DrawItem(ImageEditor::Toolbar::Item& item, Gdiplus::Graphics* gr, int x, int y, float dpiScaleX, float dpiScaleY) override;
    void setText(CString text);
protected:
    int toolbarItemIndex_;
    ImageEditor::Toolbar* toolbar_;
    CString text_;
    std::unique_ptr<Gdiplus::Font> font_;
};

class ScreenRecorderWindow : public CWindowImpl<ScreenRecorderWindow>, CMessageFilter
{
public:
    DECLARE_WND_CLASS(_T("ScreenRecorderWindow"))

    ScreenRecorderWindow();
    ~ScreenRecorderWindow() override;

    BEGIN_MSG_MAP(ScreenRecorderWindow)
        MESSAGE_HANDLER(WM_CREATE, onCreate)
        MESSAGE_HANDLER(WM_PAINT, onPaint)
        MESSAGE_HANDLER(WM_ERASEBKGND, onEraseBkgnd)
        MESSAGE_HANDLER(WM_TIMER, onTimer)
        COMMAND_ID_HANDLER(IDCANCEL, onCancel)
        //REFLECT_NOTIFICATIONS()
    END_MSG_MAP()

    enum DialogResult
    {
        drCancel
    };

    const int kTimer = 1;

    DialogResult doModal(HWND parent, CRect captureRect);
    BOOL PreTranslateMessage(MSG* pMsg) override;
private:

    // Handler prototypes (uncomment arguments if needed):
    //    LRESULT MessageHandler(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
    //    LRESULT CommandHandler(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
    //    LRESULT NotifyHandler(int /*idCtrl*/, LPNMHDR /*pnmh*/, BOOL& /*bHandled*/)


    LRESULT onCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
    LRESULT onPaint(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
    LRESULT onEraseBkgnd(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
    LRESULT onTimer(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
    LRESULT onCancel(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);

    DialogResult dialogResult_;
    void endDialog(DialogResult dr);
    void createToolbar();
    void updateTimeLabel();

    CIcon icon_, iconSmall_;
    CRect captureRect_;
    COLORREF transparentColor_;
    ImageEditor::Toolbar toolbar_;
    std::unique_ptr<TimeDelegate> timeDelegate_;
    std::unique_ptr<ScreenRecorder> screenRecorder_;

    unsigned int elapsedTime_ = 0;
}; 


#endif