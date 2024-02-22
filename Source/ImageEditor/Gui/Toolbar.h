#ifndef IMAGEEDITOR_GUI_TOOLBAR_H
#define IMAGEEDITOR_GUI_TOOLBAR_H

#include "atlheaders.h"

#include <memory>
#include <vector>

#include "CustomTrackBarControl.h"
#include "3rdpart/GdiplusH.h"
#include "Core/Utils/CoreTypes.h"

constexpr UINT MTBM_DROPDOWNCLICKED = WM_USER + 400;
constexpr UINT MTBM_FONTSIZECHANGE = WM_USER + 401;
constexpr UINT MTBM_STEPINITIALVALUECHANGE = WM_USER + 402;
constexpr UINT MTBM_FILLBACKGROUNDCHANGE = WM_USER + 403;
constexpr UINT MTBM_ARROWTYPECHANGE = WM_USER + 404;
constexpr UINT MTBM_APPLY = WM_USER + 405; // Crop
constexpr UINT MTBM_CANCEL = WM_USER + 406;
constexpr UINT MTBM_INVERTSELECTIONCHANGE = WM_USER + 407;

namespace ImageEditor {

class Toolbar : public CWindowImpl<Toolbar> {
public:
    typedef CWindowImpl<Toolbar> TParent;
    enum Orientation { orHorizontal, orVertical };
    enum ItemState { isNormal, isHover, isDown, isDropDown };
    enum ItemType { itButton, itComboButton, itTinyCombo };
    enum { kTinyComboDropdownTimer = 42, kSubpanelWidth = 300 };
    enum {ID_FONTSIZEEDITCONTROL = 12001, ID_STEPINITIALVALUE, ID_FILLBACKGROUNDCHECKBOX, ID_ARROWTYPECOMBOBOX, ID_APPLYBUTTON,
        ID_CANCELOPERATIONBUTTON, ID_INVERTSELECTIONCHECKBOX
    };
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

    explicit Toolbar(Orientation orientation); 
    ~Toolbar() override;
    bool Create(HWND parent, bool topMost = false, bool child = false);
    int addButton(const Item& item);
    DECLARE_WND_CLASS(L"ImageEditor_Toolbar");
    int getItemAtPos(int clientX, int clientY) const;
    int getItemIndexByCommand(int command) const;
    Item* getItem(int index);
    void repaintItem(int index);
    void clickButton(int index);
    int getFontSize() const;
    void setStepFontSize(int fontSize);
    void showStepFontSize(bool show);
    void showPenSize(bool show);
    void showApplyButtons(bool show);
    void setStepInitialValue(int value);
    int getStepInitialValue() const;
    void showFillBackgroundCheckbox(bool show);
    void showInvertSelectionCheckbox(bool show);
    void setFillBackgroundCheckbox(bool fill);
    void showArrowTypeCombo(bool show);
    bool isFillBackgroundChecked() const;
    bool isInvertSelectionChecked() const;
    void setInvertSelectionCheckbox(bool invert);
    int getArrowType() const;
    void setArrowType(int type);
    void setMovable(bool value);
    void setShowButtonText(bool show);
    Orientation orientation() const;

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
        MESSAGE_HANDLER( WM_ACTIVATE, OnActivate )
        MESSAGE_HANDLER(WM_CTLCOLORSTATIC, OnColorStatic)
        MESSAGE_HANDLER(WM_HSCROLL, OnHScroll)
        MESSAGE_HANDLER(WM_TIMER, OnTimer)
        MESSAGE_HANDLER(WM_CLOSE, OnClose)
        COMMAND_HANDLER(ID_FONTSIZEEDITCONTROL, EN_CHANGE, OnFontSizeEditControlChange)
        COMMAND_HANDLER(ID_STEPINITIALVALUE, EN_CHANGE, OnStepInitialValueChange)
        COMMAND_HANDLER(ID_FILLBACKGROUNDCHECKBOX, BN_CLICKED, OnFillBackgroundCheckboxClicked)
        COMMAND_HANDLER(ID_INVERTSELECTIONCHECKBOX, BN_CLICKED, OnInvertSelectionCheckboxClicked)
        COMMAND_HANDLER(ID_ARROWTYPECOMBOBOX, CBN_SELCHANGE, OnArrowTypeComboChange)
        COMMAND_ID_HANDLER(ID_APPLYBUTTON, OnApplyButtonClicked)
        COMMAND_ID_HANDLER(ID_CANCELOPERATIONBUTTON, OnCancelOperationButtonClicked)
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
    LRESULT OnFontSizeEditControlChange(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
    LRESULT OnStepInitialValueChange(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
    LRESULT OnClose(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
    LRESULT OnFillBackgroundCheckboxClicked(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
    LRESULT OnInvertSelectionCheckboxClicked(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
    LRESULT OnArrowTypeComboChange(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
    LRESULT OnApplyButtonClicked(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
    LRESULT OnCancelOperationButtonClicked(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
    SIZE CalcItemSize(Gdiplus::Graphics* gr, int index);
    int AutoSize();
    void CreateToolTipForItem(size_t index);
    void updateTooltipForItem(size_t index);
    CCustomTrackBarControl penSizeSlider_;
    CTrackBarCtrl  roundRadiusSlider_;
    CStatic pixelLabel_;
    CStatic roundRadiusLabel_;
    CStatic fontSizeLabel_;
    CEdit fontSizeEdit_;
    CUpDownCtrl fontSizeUpDownCtrl_;
    CStatic initialValueLabel_;
    CEdit initialValueEdit_;
    CButton testButton_;
    CButton fillBackgroundCheckbox_;
    CButton invertSelectionCheckbox_;
    CComboBox arrowTypeCombobox_;
    CButton applyButton_;
    CButton cancelOperationButton_;
    CToolTipCtrl tooltip_;
protected:
    Orientation orientation_;
    std::vector<Item> buttons_;
    int selectedItemIndex_;
    void drawItem(int itemIndex, Gdiplus::Graphics* gr, int, int y);
    
    bool trackMouse_;
    float dpiScaleX_;
    float dpiScaleY_;
    std::unique_ptr<Gdiplus::Bitmap> dropDownIcon_;
    int itemMargin_;
    int itemHorPadding_;
    int itemVertPadding_;
    int iconSizeX_;
    int iconSizeY_;
    std::unique_ptr<Gdiplus::Font> font_;
    Gdiplus::Color transparentColor_;
    CFont systemFont_;
    RECT buttonsRect_;
    int subpanelHeight_;
    int subpanelLeftOffset_;
    Gdiplus::Color  subpanelColor_;
    CBrush subpanelBrush_;
    CBitmap backBuffer_;
    CDC backBufferDc_;
    HBITMAP oldSelectedBm_;
    Gdiplus::TextRenderingHint textRenderingHint_;
    bool movable_;
    bool showButtonText_;
    void createHintForSliders(HWND slider, CString hint);
};

}
#endif
