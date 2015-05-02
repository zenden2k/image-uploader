#ifndef IMAGEEDITOR_GUI_TOOLBAR_H
#define IMAGEEDITOR_GUI_TOOLBAR_H

#include "atlheaders.h"

#include <vector>
#include "3rdpart/GdiplusH.h"
#include "Core/Utils/CoreTypes.h"
#define MTBM_DROPDOWNCLICKED  WM_USER + 400
namespace ImageEditor {


class Toolbar : public CWindowImpl<Toolbar> {
public:
    typedef CWindowImpl<Toolbar> TParent;
    enum Orientation { orHorizontal, orVertical };
    enum ItemState { isNormal, isHover, isDown, isDropDown };
    enum ItemType { itButton, itComboButton, itTinyCombo };
    enum { kTinyComboDropdownTimer = 42, kSubpanelWidth = 300 };
    class ToolbarItemDelegate;
    struct Item {
        CString title;
        int command;
        std::shared_ptr<Gdiplus::Bitmap> icon;
        CString hint;
        ItemState state;
        bool checkable;
        RECT rect;
        ItemType type;
        int group;
        bool isChecked;
        HWND tooltipWnd;
        ToolbarItemDelegate* itemDelegate;
        Item(CString title, std::shared_ptr<Gdiplus::Bitmap> icon, int command, CString hint = CString(), ItemType type = itButton, bool checkable = false, int group = -1) {
            this->title = title;
            this->icon = icon;
            this->command = command;
            state = isNormal;
            memset(&rect, 0, sizeof(rect));
            this->type = type;
            this->hint = hint;
            this->checkable = checkable;
            isChecked = false;
            this->group = group;
            tooltipWnd = 0;
            itemDelegate = 0;
        }
    };

    class ToolbarItemDelegate {
    public:
        virtual ~ToolbarItemDelegate() {}
        virtual SIZE CalcItemSize(Item& item, float dpiScaleX, float dpiScaleY) = 0;
        virtual void DrawItem(Item& item, Gdiplus::Graphics* gr, int, int y, float dpiScaleX, float dpiScaleY) = 0;
        virtual void OnClick(int x, int y, float dpiScaleX, float dpiScaleY){};
    };

    Toolbar(Orientation orientation); 
    ~Toolbar();
    bool Create(HWND parent, bool child = false);
    int addButton(Item item);
    DECLARE_WND_CLASS(L"ImageEditor_Toolbar");
    int getItemAtPos(int clientX, int clientY);
    int getItemIndexByCommand(int command);
    Item* getItem(int index);
    void repaintItem(int index);
    void clickButton(int index);

    BEGIN_MSG_MAP(Toolbar)
        MESSAGE_HANDLER( WM_CREATE, OnCreate )
        MESSAGE_HANDLER( WM_PAINT, OnPaint )
        MESSAGE_HANDLER( WM_MOUSEMOVE, OnMouseMove )
        MESSAGE_HANDLER( WM_MOUSELEAVE, OnMouseLeave ) 
        MESSAGE_HANDLER( WM_NCHITTEST, OnNcHitTest )
        MESSAGE_HANDLER( WM_LBUTTONDOWN, OnLButtonDown )
        MESSAGE_HANDLER( WM_LBUTTONUP, OnLButtonUp )
        MESSAGE_HANDLER( WM_RBUTTONUP, OnRButtonUp )
        MESSAGE_HANDLER( WM_ERASEBKGND, OnEraseBackground )
        MESSAGE_HANDLER( WM_KEYDOWN, OnKeyDown )
        MESSAGE_HANDLER( WM_KEYUP, OnKeyUp )
        MESSAGE_HANDLER( WM_ACTIVATE, OnActivate )
        MESSAGE_HANDLER(WM_CTLCOLORSTATIC, OnColorStatic)
        MESSAGE_HANDLER(WM_HSCROLL , OnHScroll)
        MESSAGE_HANDLER(WM_TIMER , OnTimer)
        
        REFLECT_NOTIFICATIONS ()

    END_MSG_MAP()


    // Handler prototypes (uncomment arguments if needed):
    //    LRESULT MessageHandler(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
    //    LRESULT CommandHandler(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
    //    LRESULT NotifyHandler(int /*idCtrl*/, LPNMHDR /*pnmh*/, BOOL& /*bHandled*/)
    LRESULT OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
    LRESULT OnHScroll(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
    LRESULT OnColorStatic(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
    LRESULT OnActivate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
    LRESULT OnPaint(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
    LRESULT OnMouseMove(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
    LRESULT OnMouseLeave(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
    LRESULT OnLButtonDown(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
    LRESULT OnLButtonUp(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
    LRESULT OnRButtonUp(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
    LRESULT OnEraseBackground(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
    LRESULT OnNcHitTest(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
    LRESULT OnTimer(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
    LRESULT OnKeyDown(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
    LRESULT OnKeyUp(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);

    SIZE CalcItemSize(int index);
    int AutoSize();
    void CreateToolTipForItem(int index);
    CTrackBarCtrl  penSizeSlider_;
    CTrackBarCtrl  roundRadiusSlider_;
    CStatic pixelLabel_;
    CStatic roundRadiusLabel_;
protected:
    Orientation orientation_;
    std::vector<Item> buttons_;
    int selectedItemIndex_;
    void drawItem(int itemIndex, Gdiplus::Graphics* gr, int, int y);
    
    bool trackMouse_;
    float dpiScaleX_;
    float dpiScaleY_;
    Gdiplus::Bitmap* dropDownIcon_;
    int itemMargin_;
    int itemHorPadding_;
    int itemVertPadding_;
    int iconSizeX_;
    int iconSizeY_;
    Gdiplus::Font* font_;
    Gdiplus::Color transparentColor_;
    CFont systemFont_;
    RECT buttonsRect_;
    int subpanelHeight_;
    int subpanelLeftOffset_;
    Gdiplus::Color  subpanelColor_;
    CBrush subpanelBrush_;
    Gdiplus::TextRenderingHint textRenderingHint_;
    void createHintForSliders(HWND slider, CString text);
};

}
#endif