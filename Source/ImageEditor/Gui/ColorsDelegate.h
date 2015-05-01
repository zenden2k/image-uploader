
#include "atlheaders.h"
#include "Toolbar.h"
#include "../Canvas.h"
#include "3rdpart/GdiplusH.h"
#include "3rdpart/ColorButton.h"

namespace ImageEditor {
    class ColorsDelegate: public Toolbar::ToolbarItemDelegate {
    public:
        enum {kOffset = 7, kSquareSize = 16, kPadding = 3};

        ColorsDelegate(Toolbar* toolbar, int itemIndex, Canvas* canvas);
        virtual SIZE CalcItemSize(Toolbar::Item& item, float dpiScaleX, float dpiScaleY);
        virtual void DrawItem(Toolbar::Item& item, Gdiplus::Graphics* gr, int x, int y, float dpiScaleX, float dpiScaleY);
        void setForegroundColor(Gdiplus::Color color );
        void setBackgroundColor(Gdiplus::Color color);
        Gdiplus::Color getForegroundColor() const;
        Gdiplus::Color getBackgroundColor() const;
        int itemIndex();
        virtual void OnClick(int x, int y, float dpiScaleX, float dpiScaleY);

    protected:
        Gdiplus::Color foregroundColor_;
        Gdiplus::Color backgroundColor_;
        Toolbar* toolbar_;
        CColorButton foregroundColorButton_;
        CColorButton backgroundColorButton_;
        CButton foregroundButton_;
        CButton backgroundButton_;
        Gdiplus::Rect backgroundRect_;
        Gdiplus::Rect foregroundRect_;
        int toolbarItemIndex_;
        Canvas* canvas_;
        CFont font_;

        void OnForegroundButtonSelChanged(COLORREF color, BOOL valid );

        void OnBackgroundButtonSelChanged(COLORREF color, BOOL valid );

    };
}