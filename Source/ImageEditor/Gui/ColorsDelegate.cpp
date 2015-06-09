#include "ColorsDelegate.h"
#include "Gui/GuiTools.h"
#include "Func/WinUtils.h"

namespace ImageEditor {

ColorsDelegate::ColorsDelegate(Toolbar* toolbar, int itemIndex, Canvas* canvas) {
    toolbar_ = toolbar;
    toolbarItemIndex_ = itemIndex;
    canvas_ = canvas;
    RECT rc = {0,0,1,1};
    font_ = GuiTools::GetSystemDialogFont();
    foregroundButton_.Create(toolbar->m_hWnd, rc, 0,WS_CHILD|WS_VISIBLE|BS_PUSHBUTTON);
    foregroundButton_.SetFont(font_);
    foregroundColorButton_.SubclassWindow(foregroundButton_.m_hWnd);
    foregroundColorButton_.OnSelChange.bind(this, &ColorsDelegate::OnForegroundButtonSelChanged);
    foregroundColorButton_.SetCustomText(TR("More colors..."));
    backgroundButton_.Create(toolbar->m_hWnd, rc, 0,WS_CHILD|WS_VISIBLE|BS_PUSHBUTTON);
    backgroundButton_.SetFont(font_);
    backgroundColorButton_.SubclassWindow(backgroundButton_.m_hWnd);
    backgroundColorButton_.OnSelChange.bind(this, &ColorsDelegate::OnBackgroundButtonSelChanged);
    backgroundColorButton_.SetCustomText(TR("More colors..."));
}

SIZE ColorsDelegate::CalcItemSize(Toolbar::Item& item, float dpiScaleX, float dpiScaleY) {
    SIZE res = { static_cast<LONG>((kSquareSize + kOffset + kPadding)* dpiScaleX), static_cast<LONG>((kSquareSize + kOffset + 4)* dpiScaleY ) };
    return res;
}

void ColorsDelegate::DrawItem(Toolbar::Item& item, Gdiplus::Graphics* gr, int x, int y, float dpiScaleX, float dpiScaleY) {
    using namespace Gdiplus;
    Pen borderPen(Color(0,0,0));

    SolidBrush backgroundBrush(backgroundColor_);
    SolidBrush foregroundBrush(foregroundColor_);

    backgroundRect_ = Rect(static_cast<int>(x+(kPadding+kOffset)*dpiScaleX), static_cast<int>(y+kOffset*dpiScaleY), 
                        static_cast<int>(kSquareSize * dpiScaleX), static_cast<int>(kSquareSize * dpiScaleY));
    gr->FillRectangle(&backgroundBrush, backgroundRect_);
    gr->DrawRectangle(&borderPen, backgroundRect_);

    foregroundRect_ = Rect(static_cast<int>(kPadding*dpiScaleX + x), y, static_cast<int>(kSquareSize * dpiScaleX), static_cast<int>(kSquareSize *  dpiScaleY));
    gr->FillRectangle(&foregroundBrush, foregroundRect_);
    gr->DrawRectangle(&borderPen, foregroundRect_);

    POINT pt = {foregroundRect_.X,foregroundRect_.Y + foregroundRect_.Height};
    //toolbar_->ClientToScreen(&pt);
    foregroundColorButton_.SetWindowPos(0, pt.x, pt.y,0,0,/*SWP_NOSIZE*/0);

    POINT pt2 = {backgroundRect_.X,backgroundRect_.Y + backgroundRect_.Height};
    //toolbar_->ClientToScreen(&pt2);
    backgroundColorButton_.SetWindowPos(0, pt2.x, pt2.y,0,0,/*SWP_NOSIZE*/0);
}

void ColorsDelegate::setForegroundColor(Gdiplus::Color color ) {
    foregroundColor_ = color;
}

void ColorsDelegate::setBackgroundColor(Gdiplus::Color color) {
    backgroundColor_ = color;
}

Gdiplus::Color ColorsDelegate::getForegroundColor() const {
    return foregroundColor_;
}

Gdiplus::Color ColorsDelegate::getBackgroundColor() const {
    return backgroundColor_;
}

int ColorsDelegate::itemIndex() {
    return toolbarItemIndex_;
}

void ColorsDelegate::OnClick(int x, int y, float dpiScaleX, float dpiScaleY){
    if ( foregroundRect_.Contains(x,y) ) {
        //MessageBox(0,0,0,0);
        foregroundColorButton_.Click();
    } else if ( backgroundRect_.Contains(x,y)) {
        backgroundColorButton_.Click();
    }

    //MessageBox(0,0,0,0);
}

void ColorsDelegate::OnForegroundButtonSelChanged(COLORREF color, BOOL valid ) {
    foregroundColor_ = Gdiplus::Color(GetRValue(color), GetGValue(color), GetBValue(color));
    toolbar_->repaintItem(toolbarItemIndex_);
    canvas_->setForegroundColor(foregroundColor_);

}

void ColorsDelegate::OnBackgroundButtonSelChanged(COLORREF color, BOOL valid ) {
    backgroundColor_ = Gdiplus::Color(GetRValue(color), GetGValue(color), GetBValue(color));
    toolbar_->repaintItem(toolbarItemIndex_);
    canvas_->setBackgroundColor(backgroundColor_);
}

}