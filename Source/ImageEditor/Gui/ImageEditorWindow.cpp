// MainFrm.cpp : implmentation of the CMainFrame class
//
/////////////////////////////////////////////////////////////////////////////


#include "ImageEditorWindow.h"

#include "ImageEditorView.h"
#include "ImageEditor/Gui/Toolbar.h"
#include "ColorsDelegate.h"
#include "Core/Logging.h"
#include "resource.h"
#include "Core/Images/Utils.h"
#include "Gui/GuiTools.h"
#include "Func/WinUtils.h"
#include "Func/MyUtils.h"
#include "ImageEditor/MovableElements.h"

namespace ImageEditor {
    
ImageEditorWindow::ImageEditorWindow(std::shared_ptr<Gdiplus::Bitmap> bitmap, bool hasTransparentPixels, ConfigurationProvider* configurationProvider ):horizontalToolbar_(Toolbar::orHorizontal),verticalToolbar_(Toolbar::orVertical) 
{
    currentDoc_ =  new ImageEditor::Document(bitmap, hasTransparentPixels);
    configurationProvider_ = configurationProvider;
    askBeforeClose_ = false;
    allowAltTab_ = false;
    init();
    
}

ImageEditorWindow::ImageEditorWindow(CString imageFileName, ConfigurationProvider* configurationProvider ):horizontalToolbar_(Toolbar::orHorizontal),verticalToolbar_(Toolbar::orVertical) 
{
    if (!imageFileName.IsEmpty()) {
        CString ext = WinUtils::GetFileExt(imageFileName);
        if (ext == "webp") {
            ::MessageBox(nullptr, _T("Webp format is not supported by image editor"), TR("Image Editor"), MB_ICONERROR);
        }
    }
    currentDoc_ = new ImageEditor::Document(imageFileName);
    sourceFileName_ = imageFileName;
    configurationProvider_ = configurationProvider;
    askBeforeClose_ = true;
    allowAltTab_ = false;
    suggestedFileName_ = myExtractFileName(sourceFileName_);
    init();
}

void ImageEditorWindow::init()
{
//    resultingBitmap_ = 0;
    canvas_ = 0;
    cropToolTip_ = 0;
    showUploadButton_ = true;
    showAddToWizardButton_ = true;
    prevPenSize_ = 0;
    prevRoundingRadius_ = 0;
    colorsDelegate_ = 0;
    imageQuality_ = 85;
    currentDrawingTool_ = Canvas::dtNone;
    initialDrawingTool_ = Canvas::dtBrush;
    menuItems_[ID_PEN]             = Canvas::dtPen; 
    menuItems_[ID_LINE]            = Canvas::dtLine;
    menuItems_[ID_BRUSH]           = Canvas::dtBrush;
    menuItems_[ID_MARKER]          = Canvas::dtMarker;
    menuItems_[ID_RECTANGLE]       = Canvas::dtRectangle;
    menuItems_[ID_FILLEDRECTANGLE] = Canvas::dtFilledRectangle;
    menuItems_[ID_CROP]            = Canvas::dtCrop;
    menuItems_[ID_MOVE]            = Canvas::dtMove;
    menuItems_[ID_ARROW]           = Canvas::dtArrow;
    menuItems_[ID_SELECTION]       = Canvas::dtSelection;
    menuItems_[ID_BLUR]            = Canvas::dtBlur;
    menuItems_[ID_BLURRINGRECTANGLE]   = Canvas::dtBlurrringRectangle;
    menuItems_[ID_COLORPICKER]     = Canvas::dtColorPicker;
    menuItems_[ID_ROUNDEDRECTANGLE]     = Canvas::dtRoundedRectangle;
    menuItems_[ID_ELLIPSE]     = Canvas::dtEllipse;
    menuItems_[ID_FILLEDROUNDEDRECTANGLE]     = Canvas::dtFilledRoundedRectangle;
    menuItems_[ID_FILLEDELLIPSE]     = Canvas::dtFilledEllipse;
    menuItems_[ID_TEXT]     = Canvas::dtText;

    SubMenuItem item;
    item.parentCommand = ID_RECTANGLE;
    item.icon = loadToolbarIcon(IDB_ICONTOOLRECTANGLEPNG);
    item.command = ID_RECTANGLE;
    item.hint = TR("Rectangle");
    subMenuItems_[Canvas::dtRectangle] = item;

    item.icon = loadToolbarIcon(IDB_ICONTOOLROUNDEDRECTANGLE);
    item.command = ID_ROUNDEDRECTANGLE;
    item.hint = TR("Rounded rectangle");
    subMenuItems_[Canvas::dtRoundedRectangle] = item;

    item.icon = loadToolbarIcon(IDB_ICONTOOLELLIPSE);
    item.command = ID_ELLIPSE;
    item.hint = TR("Ellipse");
    subMenuItems_[Canvas::dtEllipse] = item;

    item.parentCommand = ID_FILLEDRECTANGLE;
    item.command = ID_FILLEDRECTANGLE;
    item.hint = TR("Filled rectangle");
    item.icon = loadToolbarIcon(IDB_ICONTOOLFILLEDRECTANGLE);
    subMenuItems_[Canvas::dtFilledRectangle] = item;


    item.icon = loadToolbarIcon(IDB_ICONTOOLFILLEDROUNDEDRECTANGLE);
    item.command = ID_FILLEDROUNDEDRECTANGLE;
    item.hint = TR("Rounded rectangle");
    subMenuItems_[Canvas::dtFilledRoundedRectangle] = item;


    item.icon = loadToolbarIcon(IDB_ICONTOOLFILLEDELLIPSE);
    item.command = ID_FILLEDELLIPSE;
    item.hint =  TR("Filled ellipse");
    subMenuItems_[Canvas::dtFilledEllipse] = item;
    selectedSubMenuItems_[ID_RECTANGLE] = ID_RECTANGLE;
    selectedSubMenuItems_[ID_FILLEDRECTANGLE] = ID_FILLEDRECTANGLE;

    drawingToolsHotkeys_[kMoveKey] = Canvas::dtMove;
    drawingToolsHotkeys_[kBrushKey] = Canvas::dtBrush;
    drawingToolsHotkeys_[kTextKey] = Canvas::dtText;
    drawingToolsHotkeys_[kRectangleKey] = Canvas::dtRectangle;
    drawingToolsHotkeys_[kColorPickerKey] = Canvas::dtColorPicker;
    drawingToolsHotkeys_[kCropKey] = Canvas::dtCrop;
    drawingToolsHotkeys_[kMarkerKey] = Canvas::dtMarker;
    drawingToolsHotkeys_[kBlurringRectangleKey] = Canvas::dtBlurrringRectangle;
    drawingToolsHotkeys_[kArrowKey] = Canvas::dtArrow;
    drawingToolsHotkeys_[kLineKey] = Canvas::dtLine;
    drawingToolsHotkeys_[kFilledRectangle] = Canvas::dtFilledRectangle;
    
    dialogResult_ = drCancel;
}

bool ImageEditorWindow::saveDocument(ClipboardFormat clipboardFormat)
{
    resultingBitmap_ = canvas_->getBitmapForExport();
    canvas_->setDocumentModified(false);
    if ( !resultingBitmap_ ) {
        LOG(ERROR) << "canvas_->getBitmapForExport() returned NULL";
        return false;
    }
    if (clipboardFormat == ClipboardFormat::None) {
        if ( !outFileName_.IsEmpty() ) {
            SaveImage(resultingBitmap_.get(), outFileName_, sifDetectByExtension, imageQuality_);
            canvas_->updateView();
            return true;
        } else {
            CString outFileName;
            if (MySaveImage(resultingBitmap_.get(), "screeenshot001.png", outFileName, 0, 95)) {
                outFileName_ = outFileName;
                canvas_->updateView();
                return true;
            }
            return false;
        }
    } else if (clipboardFormat  == ClipboardFormat::Bitmap ) {
        CDC dc = GetDC();
        CopyBitmapToClipboard(m_hWnd, dc, resultingBitmap_.get(), true);
        return true;
    } else if (clipboardFormat == ClipboardFormat::DataUri || clipboardFormat == ClipboardFormat::DataUriHtml) {
        CopyBitmapToClipboardInDataUriFormat(resultingBitmap_.get(), 1, 85, clipboardFormat == ClipboardFormat::DataUriHtml);
        return true;
    }
    return false;
}

void ImageEditorWindow::updateToolbarDrawingTool(Canvas::DrawingToolType dt)
{
    if ( currentDrawingTool_ == dt ) {
        return;
    }

    currentDrawingTool_ = dt;
    updateRoundingRadiusSlider();

    std::map<Canvas::DrawingToolType, SubMenuItem>::iterator submenuIter = subMenuItems_.find(dt);
    if ( submenuIter != subMenuItems_.end() ) {
        int buttonIndex = verticalToolbar_.getItemIndexByCommand(selectedSubMenuItems_[submenuIter->second.parentCommand]);
        if ( buttonIndex != -1 ) {
            Toolbar::Item* item = verticalToolbar_.getItem(buttonIndex);
            if ( item ) {
                item->icon = submenuIter->second.icon;
                item->command = submenuIter->second.command;
                item->hint = submenuIter->second.hint;
                verticalToolbar_.CreateToolTipForItem(buttonIndex);
                selectedSubMenuItems_[submenuIter->second.parentCommand] = item->command;
            }
            
            verticalToolbar_.clickButton(buttonIndex);
            return;
        }
    }

    //std::map<int, Canvas::DrawingToolType>::iterator it = std::find(menuItems_.begin(), menuItems_.end(), dt);
    std::map<int, Canvas::DrawingToolType>::iterator it;
    for( it = menuItems_.begin(); it != menuItems_.end(); ++it ) {
        if ( it->second == dt ) {
            int buttonIndex = verticalToolbar_.getItemIndexByCommand(it->first);
            if ( buttonIndex != -1 ) {
                verticalToolbar_.clickButton(buttonIndex);
                break;
            }
        }
    }
    if ( dt == Canvas::dtCrop ) {
        createTooltip();
    } else if ( cropToolTip_ ) {
        ::DestroyWindow(cropToolTip_);
        cropToolTip_ = 0;
    }
    
}

void ImageEditorWindow::OnForegroundColorChanged(Gdiplus::Color color)
{
    colorsDelegate_->setForegroundColor(color);
    verticalToolbar_.repaintItem(colorsDelegate_->itemIndex());
}

void ImageEditorWindow::OnBackgroundColorChanged(Gdiplus::Color color)
{
    colorsDelegate_->setBackgroundColor(color);
    verticalToolbar_.repaintItem(colorsDelegate_->itemIndex());
}

void ImageEditorWindow::onFontChanged(LOGFONT font)
{
    textParamsWindow_.setFont(font);
}

ImageEditorWindow::~ImageEditorWindow()
{
    delete currentDoc_;
    delete canvas_;
    delete colorsDelegate_;
}

void ImageEditorWindow::setInitialDrawingTool(Canvas::DrawingToolType dt)
{
    initialDrawingTool_ = dt;
}

void ImageEditorWindow::showUploadButton(bool show)
{
    showUploadButton_ = show;
}

void ImageEditorWindow::showAddToWizardButton(bool show)
{
    showAddToWizardButton_ = show;
}

void ImageEditorWindow::setSuggestedFileName(CString string)
{
    suggestedFileName_ = string;
}

std::shared_ptr<Gdiplus::Bitmap> ImageEditorWindow::getResultingBitmap()
{
    return resultingBitmap_;
}

void ImageEditorWindow::setServerName(const CString & serverName)
{
    serverName_ = serverName;
}

void ImageEditorWindow::setAskBeforeClose(bool ask)
{
    askBeforeClose_ = ask;
}

ImageEditorWindow::DialogResult ImageEditorWindow::DoModal(HWND parent, WindowDisplayMode mode)
{
    if (currentDoc_->isNull()) {
        ::MessageBox(0, _T("Invalid image file."), APPNAME, MB_ICONERROR);
        return drCancel;
    }
    int screenWidth = GetSystemMetrics(SM_CXSCREEN);
    int screenHeight = GetSystemMetrics(SM_CYSCREEN);
    //    displayMode_ = wdmWindowed;

    CRect displayBounds;
    GuiTools::GetScreenBounds(displayBounds);


    CDC hdc = ::GetDC(0);
    float dpiScaleX = GetDeviceCaps(hdc, LOGPIXELSX) / 96.0f;
    float dpiScaleY = GetDeviceCaps(hdc, LOGPIXELSY) / 96.0f;
    int desiredClientWidth = currentDoc_->getWidth() + static_cast<int>(50 * dpiScaleX); // with toolbars
    int desiredClientHeight = currentDoc_->getHeight() + static_cast<int>(50 * dpiScaleX);
    CRect rc(rcDefault);
    DWORD initialWindowStyle = WS_OVERLAPPED | WS_POPUP | WS_CAPTION | WS_SYSMENU | WS_SIZEBOX | WS_MAXIMIZEBOX |
        WS_MINIMIZEBOX | WS_CLIPCHILDREN;
    if (Create(0, rc, _T("Image Editor"), initialWindowStyle, 0) == NULL) {
        LOG(ERROR) << "Main window creation failed!\n";
        return drCancel;
    }

    if (mode == wdmAuto) {
        RECT clientRect;
        RECT windowRect;
        GetClientRect(&clientRect);
        GetWindowRect(&windowRect);
        int paddingX = windowRect.right - windowRect.left - clientRect.right;
        int paddingY = windowRect.bottom - windowRect.top - clientRect.bottom;

        if (desiredClientWidth + paddingX >= screenWidth &&   currentDoc_->getWidth() <= displayBounds.Width()
            && desiredClientHeight + paddingY >= screenHeight &&   currentDoc_->getHeight() <= displayBounds.Height()) {
            mode = wdmFullscreen;
        } else {
            mode = wdmWindowed;
        }
    }
    displayMode_ = mode;

    if (displayMode_ == wdmFullscreen) {
        //windowStyle = GetStyle();
        DWORD windowStyle = WS_POPUP | WS_CLIPCHILDREN;
        SetWindowLong(GWL_STYLE, windowStyle);
        HWND insertAfter = nullptr;
#ifndef _DEBUG
        SetWindowLong(GWL_EXSTYLE, WS_EX_TOPMOST);
        insertAfter = HWND_TOPMOST;
#endif
        SetWindowPos(insertAfter, displayBounds.left, displayBounds.top, displayBounds.right - displayBounds.left, displayBounds.bottom - displayBounds.top, 0);
    }

    if (displayMode_ == wdmWindowed) {
        CenterWindow();
    }

    icon_ = GuiTools::LoadBigIcon(IDR_MAINFRAME);
    iconSmall_ = GuiTools::LoadSmallIcon(IDR_MAINFRAME);

    SetIcon(icon_, TRUE);
    SetIcon(iconSmall_, FALSE);
    ACCEL accels[] = {
        { FVIRTKEY | FCONTROL, 'Z', ID_UNDO },
        { FVIRTKEY | FCONTROL, 'D', ID_UNSELECTALL },
        { FVIRTKEY | FCONTROL, 'S', ID_SAVE }
    };

    accelerators_ = CreateAcceleratorTable(accels, ARRAY_SIZE(accels));

    //RECT rc;
    GetClientRect(&rc);
    /*HWND m_hWndClient = */m_view.Create(m_hWnd, rc, _T("ImageEditor_Canvas"), WS_CHILD | WS_VISIBLE /*| WS_CLIPSIBLINGS | WS_CLIPCHILDREN*/, 0);

    canvas_ = new ImageEditor::Canvas(m_view);
    canvas_->setSize(currentDoc_->getWidth(), currentDoc_->getHeight());
    canvas_->setDocument(currentDoc_);
    textParamsWindow_.Create(m_hWnd);

    if (configurationProvider_) {
        canvas_->setPenSize(configurationProvider_->penSize());
        canvas_->setForegroundColor(configurationProvider_->foregroundColor());
        canvas_->setBackgroundColor(configurationProvider_->backgroundColor());
        canvas_->setFont(configurationProvider_->font());
        canvas_->setRoundingRadius(configurationProvider_->roundingRadius());
        allowAltTab_ = configurationProvider_->allowAltTab();
        textParamsWindow_.setFont(configurationProvider_->font());
    }
    m_view.setCanvas(canvas_);
    createToolbars();
    RECT horToolbarRect;
    RECT vertToolbarRect;
    horizontalToolbar_.GetClientRect(&horToolbarRect);
    verticalToolbar_.GetClientRect(&vertToolbarRect);
    rc.left = displayMode_ == wdmWindowed ? vertToolbarRect.right + kCanvasMargin : 0;
    rc.top = displayMode_ == wdmWindowed ? horToolbarRect.bottom + kCanvasMargin : 0;

    if (displayMode_ == wdmWindowed) {
        horizontalToolbar_.SetWindowPos(0, rc.left, 0, 0, 0, SWP_NOSIZE);
        verticalToolbar_.SetWindowPos(0, 0, rc.top, 0, 0, SWP_NOSIZE);
    }

    m_view.SetWindowPos(0, &rc, SWP_NOSIZE);

    canvas_->onDrawingToolChanged.bind(this, &ImageEditorWindow::OnDrawingToolChanged);
    canvas_->onForegroundColorChanged.bind(this, &ImageEditorWindow::OnForegroundColorChanged);
    canvas_->onBackgroundColorChanged.bind(this, &ImageEditorWindow::OnBackgroundColorChanged);
    canvas_->onCropChanged.bind(this, &ImageEditorWindow::OnCropChanged);
    canvas_->onFontChanged.bind(this, &ImageEditorWindow::onFontChanged);
    canvas_->onTextEditStarted.bind(this, &ImageEditorWindow::OnTextEditStarted);
    canvas_->onTextEditFinished.bind(this, &ImageEditorWindow::OnTextEditFinished);
    canvas_->onSelectionChanged.bind(this, &ImageEditorWindow::OnSelectionChanged);
    if (displayMode_ != wdmWindowed) {
        canvas_->onCropFinished.bind(this, &ImageEditorWindow::OnCropFinished);
    }

    if (initialDrawingTool_ != Canvas::dtCrop) {
        verticalToolbar_.ShowWindow(SW_SHOW);
        horizontalToolbar_.ShowWindow(SW_SHOW);
    }

    updatePixelLabels();

    canvas_->setDrawingToolType(initialDrawingTool_);
    updateToolbarDrawingTool(initialDrawingTool_);
    if (displayMode_ == wdmWindowed) {
        CRect newClientRect(0, 0, desiredClientWidth, desiredClientHeight);
        AdjustWindowRect(newClientRect, GetStyle(), false);
        int newWidth = max(static_cast<LONG>(500 * dpiScaleX), newClientRect.right - newClientRect.left);
        int newHeight = max(static_cast<LONG>(500 * dpiScaleY), newClientRect.bottom - newClientRect.top);
        ResizeClient(newWidth, newHeight);
        CenterWindow(parent);
    }
    canvas_->updateView();

    //displayMode_ = wdmWindowed;
    ShowWindow(SW_SHOW);
    SetForegroundWindow(m_hWnd);
    //BringWindowToTop();
    //m_view.Invalidate(false);
    
    if ( parent ) {
        ::EnableWindow(parent, false);
    }

    CMessageLoop loop;
    loop.AddMessageFilter(this);
    loop.Run();
    saveSettings();
    if ( parent ) {
        ::EnableWindow(parent, true);
    }
    ShowWindow(SW_HIDE);
    if ( parent ) {
        ::SetActiveWindow(parent);
    }
    DestroyWindow();
    if ( dialogResult_ == drCancel  ) {
//        delete resultingBitmap_;
        //resultingBitmap_ = 0;
    }
    
    return dialogResult_;
}

LRESULT ImageEditorWindow::OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
    SetWindowText(TR("Image Editor"));
    LoadLibrary(CRichEditCtrl::GetLibraryName());


    return 0;
}

LRESULT ImageEditorWindow::OnDestroy(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled)
{
    // unregister message filtering and idle updates


    bHandled = FALSE;
    return 1;
}

LRESULT ImageEditorWindow::OnClose(UINT uMsg, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled)
{
    OnClickedClose(ID_CLOSE,0,0, bHandled);
    return 0;
}

LRESULT ImageEditorWindow::OnSize(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled)
{
    if ( horizontalToolbar_.m_hWnd ) { // It is possible that toolbars are not created yet
        RECT rc;
        GetClientRect(&rc);
        RECT horToolbarRect;
        RECT vertToolbarRect;
        horizontalToolbar_.GetClientRect(&horToolbarRect);
        verticalToolbar_.GetClientRect(&vertToolbarRect);
        
        rc.right -=  (displayMode_ == wdmWindowed ? vertToolbarRect.right+kCanvasMargin : 0);
        rc.bottom -=  (displayMode_ == wdmWindowed ? horToolbarRect.bottom+kCanvasMargin : 0);

        m_view.SetWindowPos(0, 0,0, rc.right, rc.bottom, SWP_NOMOVE);
        //m_view.Invalidate(TRUE);
    }
    return 0;
}

LRESULT ImageEditorWindow::OnPaint(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled)
{
    CPaintDC dc(m_hWnd);
    if ( displayMode_ == wdmWindowed ) {
        
        CRect clientRect;
        GetClientRect(&clientRect);
        CRgn rgn;
        rgn.CreateRectRgn(clientRect.left,clientRect.top, clientRect.right, clientRect.bottom);

        RECT horToolbarRect;
        RECT vertToolbarRect;
        RECT viewRect;
        horizontalToolbar_.GetClientRect(&horToolbarRect);
        horizontalToolbar_.ClientToScreen(&horToolbarRect);
        ScreenToClient(&horToolbarRect);
        verticalToolbar_.GetClientRect(&vertToolbarRect);
        verticalToolbar_.ClientToScreen(&vertToolbarRect);
        ScreenToClient(&vertToolbarRect);
        m_view.GetClientRect(&viewRect);
        m_view.ClientToScreen(&viewRect);
        ScreenToClient(&viewRect);
        
        CRgn horToolRgn;
        CRgn vertToolRgn;
        CRgn viewRgn;
        horToolRgn.CreateRectRgn(horToolbarRect.left, horToolbarRect.top, horToolbarRect.right, horToolbarRect.bottom);
        vertToolRgn.CreateRectRgn(vertToolbarRect.left, vertToolbarRect.top, vertToolbarRect.right, vertToolbarRect.bottom);
        viewRgn.CreateRectRgn(viewRect.left, viewRect.top, viewRect.right, viewRect.bottom);
        rgn.CombineRgn(horToolRgn, RGN_DIFF);
        rgn.CombineRgn(vertToolRgn, RGN_DIFF);
        rgn.CombineRgn(viewRgn, RGN_DIFF);

        CBrush br;
        br.CreateSolidBrush(GetSysColor(COLOR_APPWORKSPACE)); // MDI background
        dc.FillRgn(rgn, br);

        /*CRect viewRect;
        m_view.GetClientRect(&viewRect);*/
        
        /*RECT topRect = {0, 0, clientRect.right,horToolbarRect.bottom + kCanvasMargin};
        dc.FillRect(&topRect, br);
        RECT leftRect = {0, topRect.bottom, vertToolbarRect.right + kCanvasMargin, clientRect.bottom};
        dc.FillRect(&leftRect, br);*/
    }

    return 0;
}

LRESULT ImageEditorWindow::OnEraseBackground(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled)
{
    bHandled = true;
    return TRUE;
}

LRESULT ImageEditorWindow::OnGetMinMaxInfo(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& bHandled)
{
    if ( !horizontalToolbar_.m_hWnd ) {
        return 0;
    }
    MINMAXINFO* mmi = reinterpret_cast<MINMAXINFO*>(lParam);
    RECT horRc, vertRc;
    horizontalToolbar_.GetClientRect(&horRc);
    verticalToolbar_.GetClientRect(&vertRc);
    RECT clientRect;
    RECT windowRect;
    GetClientRect(&clientRect);
    GetWindowRect(&windowRect);
    int paddingX = windowRect.right -  windowRect.left - clientRect.right;
    int paddingY = windowRect.bottom -  windowRect.top - clientRect.bottom;

    mmi->ptMinTrackSize.y = paddingY + kToolbarOffset+ horRc.bottom + vertRc.bottom;
    mmi->ptMinTrackSize.x = paddingX + kToolbarOffset + horRc.right + vertRc.right;
    return 0;
}

LRESULT ImageEditorWindow::OnKeyDown(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
     std::map<DrawingToolHotkey, Canvas::DrawingToolType>::iterator it;
     //HKL englishLayout = LoadKeyboardLayout(_T("00000409"),0);
     if ( wParam == VK_ESCAPE ) {
        EndDialog(drCancel);
        return 0;
     } else if ( wParam == VK_RETURN && (showUploadButton_ || showAddToWizardButton_) ) {
         if ( !sourceFileName_.IsEmpty() ) {
             outFileName_ = sourceFileName_;
         }
         if ( saveDocument() ) {
             DialogResult dr = showUploadButton_ ? drUpload : drAddToWizard;
             EndDialog(dr);
         }
     }
     else if (wParam == VK_OEM_6) { // ']'
         canvas_->beginPenSizeChanging();
         canvas_->setPenSize(canvas_->getPenSize()+1);
         horizontalToolbar_.penSizeSlider_.SetPos(canvas_->getPenSize());
         updatePixelLabels();
         m_view.SendMessage(WM_SETCURSOR, (LPARAM)m_view.m_hWnd, 0);
     }
     else if (wParam == VK_OEM_4 ) { //'['
         canvas_->beginPenSizeChanging();
         canvas_->setPenSize(canvas_->getPenSize()-1);
         horizontalToolbar_.penSizeSlider_.SetPos(canvas_->getPenSize());
         updatePixelLabels();
         m_view.SendMessage(WM_SETCURSOR, reinterpret_cast<LPARAM>(m_view.m_hWnd), 0);
     } else if ( (it = drawingToolsHotkeys_.find((DrawingToolHotkey)wParam)) != drawingToolsHotkeys_.end() 
         && !(GetKeyState(VK_CONTROL) & 0x8000) 
         && !(GetKeyState(VK_SHIFT) & 0x8000)
        && !(GetKeyState(VK_MENU) & 0x8000) ) {
         canvas_->setDrawingToolType(it->second);
         updateToolbarDrawingTool(it->second);
     } else if ( wParam == VK_DELETE ) {
         canvas_->deleteSelectedElements();
     } 
    return 0;
}

LRESULT ImageEditorWindow::OnKeyUp(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/)
{
    if (wParam == VK_OEM_4 || wParam == VK_OEM_6) {
        canvas_->endPenSizeChanging(canvas_->getPenSize());
    } 
    return 0;
}

LRESULT ImageEditorWindow::OnActivateApp(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
    if ( !allowAltTab_ ) {
        return 0;
    }
    if ( displayMode_ == wdmFullscreen ) {
        if ( !wParam ) { // if the window is being deactivated
            SetWindowLong(GWL_EXSTYLE, 0);
            SetWindowPos(HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE );
            SetWindowPos(HWND_BOTTOM, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE );
            HWND foregroundWindow = GetForegroundWindow();
            if ( foregroundWindow != m_hWnd ) {
                ::SetWindowPos(foregroundWindow, m_hWnd, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE );
            }
        } else {
            SetWindowPos(HWND_TOPMOST, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE );
            SetWindowLong(GWL_EXSTYLE, WS_EX_TOPMOST);
        }
    }
    return 0;
}

LRESULT ImageEditorWindow::OnDropDownClicked(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{    
    Toolbar::Item* item = reinterpret_cast<Toolbar::Item*>(wParam);
    if ( item->command == ID_RECTANGLE || item->command == ID_ROUNDEDRECTANGLE || item->command == ID_ELLIPSE) {
        CMenu rectangleMenu;
        RECT rc = item->rect;
        verticalToolbar_.ClientToScreen(&rc);
        rectangleMenu.CreatePopupMenu();
        rectangleMenu.AppendMenu(MF_STRING, ID_RECTANGLE, TR("Rectangle"));
        rectangleMenu.AppendMenu(MF_STRING, ID_ROUNDEDRECTANGLE, TR("Rounded rectangle"));
        rectangleMenu.AppendMenu(MF_STRING, ID_ELLIPSE, TR("Ellipse"));
        TPMPARAMS excludeArea;
        ZeroMemory(&excludeArea, sizeof(excludeArea));
        excludeArea.cbSize = sizeof(excludeArea);
        excludeArea.rcExclude = rc;
        rectangleMenu.TrackPopupMenuEx(TPM_LEFTALIGN | TPM_LEFTBUTTON | TPM_VERTICAL, rc.left, rc.bottom, m_hWnd,&excludeArea);
    } else if ( item->command == ID_FILLEDRECTANGLE || item->command ==ID_FILLEDROUNDEDRECTANGLE || item->command ==ID_FILLEDELLIPSE) {
        CMenu rectangleMenu;
        RECT rc = item->rect;
        verticalToolbar_.ClientToScreen(&rc);
        rectangleMenu.CreatePopupMenu();
        rectangleMenu.AppendMenu(MF_STRING, ID_FILLEDRECTANGLE, TR("Filled rectangle"));
        rectangleMenu.AppendMenu(MF_STRING, ID_FILLEDROUNDEDRECTANGLE, TR("Rounded rectangle"));
        rectangleMenu.AppendMenu(MF_STRING, ID_FILLEDELLIPSE, TR("Filled ellipse"));
        TPMPARAMS excludeArea;
        ZeroMemory(&excludeArea, sizeof(excludeArea));
        excludeArea.cbSize = sizeof(excludeArea);
        excludeArea.rcExclude = rc;
        rectangleMenu.TrackPopupMenuEx(TPM_LEFTALIGN | TPM_LEFTBUTTON | TPM_VERTICAL, rc.left, rc.bottom, m_hWnd, &excludeArea);
    } else if ( item->command == ID_SAVE ) {
        CMenu rectangleMenu;
        RECT rc = item->rect;
        horizontalToolbar_.ClientToScreen(&rc);
        rectangleMenu.CreatePopupMenu();
        rectangleMenu.AppendMenu(MF_STRING, ID_SAVEAS, TR("Save as"));
        TPMPARAMS excludeArea;
        ZeroMemory(&excludeArea, sizeof(excludeArea));
        excludeArea.cbSize = sizeof(excludeArea);
        excludeArea.rcExclude = rc;
        rectangleMenu.TrackPopupMenuEx(TPM_LEFTALIGN | TPM_LEFTBUTTON | TPM_VERTICAL, rc.left, rc.bottom, m_hWnd, &excludeArea);
    } else if (item->command == ID_COPYBITMAPTOCLIBOARD) {
        CMenu rectangleMenu;
        RECT rc = item->rect;
        horizontalToolbar_.ClientToScreen(&rc);
        rectangleMenu.CreatePopupMenu();
        rectangleMenu.AppendMenu(MF_STRING, ID_COPYBITMAPTOCLIBOARDASDATAURI, TR("Copy in data:URI format"));
        rectangleMenu.AppendMenu(MF_STRING, ID_COPYBITMAPTOCLIBOARDASDATAURIHTML, TR("Copy in data:URI format (html)"));
        TPMPARAMS excludeArea;
        ZeroMemory(&excludeArea, sizeof(excludeArea));
        excludeArea.cbSize = sizeof(excludeArea);
        excludeArea.rcExclude = rc;
        rectangleMenu.TrackPopupMenuEx(TPM_LEFTALIGN | TPM_LEFTBUTTON | TPM_VERTICAL, rc.left, rc.bottom, m_hWnd, &excludeArea);
    }
    return 0;
}

LRESULT ImageEditorWindow::OnFileExit(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
    PostMessage(WM_CLOSE);
    return 0;
}

LRESULT ImageEditorWindow::OnFileNew(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
    // TODO: add code to initialize document

    return 0;
}

LRESULT ImageEditorWindow::OnViewToolBar(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
    /*BOOL bVisible = !::IsWindowVisible(m_hWndToolBar);
    ::ShowWindow(m_hWndToolBar, bVisible ? SW_SHOWNOACTIVATE : SW_HIDE);
    UISetCheck(ID_VIEW_TOOLBAR, bVisible);
    UpdateLayout();*/
    return 0;
}

LRESULT ImageEditorWindow::OnAppAbout(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
    return 0;
}


void ImageEditorWindow::createToolbars()
{
    if ( !horizontalToolbar_.Create(m_hWnd, displayMode_ == wdmWindowed) ) {
        LOG(ERROR) << "Failed to create horizontal toolbar";
        return;
    }
    if ( showAddToWizardButton_ ) {
        CString buttonHint=  (showUploadButton_ ? _T("") : _T(" (Enter)"));
        horizontalToolbar_.addButton(Toolbar::Item(CString(TR("Add to the list")), loadToolbarIcon(IDB_ICONADDPNG),ID_ADDTOWIZARD, buttonHint));
    }
    if ( showUploadButton_ ) {
        CString uploadButtonText;
        if ( serverName_.IsEmpty() ) {
            uploadButtonText = TR("Upload to Web");
        } else {
            uploadButtonText.Format(TR("Upload to %s"), serverName_);
        }
        horizontalToolbar_.addButton(Toolbar::Item(uploadButtonText, loadToolbarIcon(IDB_ICONUPLOADPNG),ID_UPLOAD, _T(" (Enter)"), Toolbar::itButton));
    }
    //horizontalToolbar_.addButton(Toolbar::Item(TR("Share"),0,ID_SHARE, CString(),Toolbar::itComboButton));
    horizontalToolbar_.addButton(Toolbar::Item(TR("Save"),loadToolbarIcon(IDB_ICONSAVEPNG), ID_SAVE, CString(_T("(Ctrl+S)")),sourceFileName_.IsEmpty() ? Toolbar::itButton : Toolbar::itComboButton));
    horizontalToolbar_.addButton(Toolbar::Item(TR("Copy to clipboard"), loadToolbarIcon(IDB_ICONCLIPBOARDPNG), ID_COPYBITMAPTOCLIBOARD, CString(), Toolbar::itComboButton));
    horizontalToolbar_.addButton(Toolbar::Item(TR("Print..."), std::shared_ptr<Gdiplus::Bitmap>(), ID_PRINTIMAGE, CString(), Toolbar::itButton));
    horizontalToolbar_.addButton(Toolbar::Item(TR("Close"),std::shared_ptr<Gdiplus::Bitmap> () ,ID_CLOSE, CString(_T("(Esc)"))));
    horizontalToolbar_.AutoSize();
    if ( displayMode_ != wdmFullscreen ) {
        horizontalToolbar_.ShowWindow(SW_SHOW);
    }

    if ( !verticalToolbar_.Create(m_hWnd, displayMode_ == wdmWindowed) ) {
        LOG(ERROR) << "Failed to create horizontal toolbar";

    }
    /* enum DrawingToolHotkey {kMoveKey = 'V', kBrushKey = 'B', kTextKey = 'T', kRectangleKey = 'U', kColorPickerKey = 'I', kCropKey = 'C', // photoshop keys
    kMarkerKey = 'H', kBlurringRectangleKey = 'R', kArrowKey = 'A', kLineKey = 'L'
    };
    */ 
    verticalToolbar_.addButton(Toolbar::Item(CString(),  loadToolbarIcon(IDB_TOOLMOVEICONPNG),ID_MOVE,TR("Move") + CString(_T(" (")) + (char)kMoveKey  + CString(_T(")")), Toolbar::itButton, true, 1));
    //verticalToolbar_.addButton(Toolbar::Item(CString(),  loadToolbarIcon(IDB_ICONTOOLSELECTION),ID_SELECTION,TR("Selection"), Toolbar::itButton, true, 1));

    verticalToolbar_.addButton(Toolbar::Item(CString(),  loadToolbarIcon(IDB_TOOLCROPPING), ID_CROP,TR("Crop")+ CString(_T(" (")) + (char)kCropKey  + CString(_T(")")), Toolbar::itButton, true,1));
    //verticalToolbar_.addButton(Toolbar::Item(CString(),  loadToolbarIcon(IDB_ICONTOOLPENCIL), ID_PEN,T_R("Карандаш"), Toolbar::itButton, true,1));
    verticalToolbar_.addButton(Toolbar::Item(CString(),  loadToolbarIcon(IDB_ICONTOOLBRUSHPNG), ID_BRUSH,TR("Brush")+ CString(_T(" (")) + (char)kBrushKey  + CString(_T(")")), Toolbar::itButton, true,1));
    verticalToolbar_.addButton(Toolbar::Item(CString(),  loadToolbarIcon(IDB_ICONTOOLMARKER), ID_MARKER,TR("Highlight")+ CString(_T(" (")) + (char)kMarkerKey  + CString(_T(")")), Toolbar::itButton, true,1));
    verticalToolbar_.addButton(Toolbar::Item(CString(),  loadToolbarIcon(IDB_ICONTOOLLINE), ID_LINE,TR("Line")+ CString(_T(" (")) + (char)kLineKey  + CString(_T(")")), Toolbar::itButton, true,1));
    verticalToolbar_.addButton(Toolbar::Item(CString(),  loadToolbarIcon(IDB_ICONTOOLARROWPNG), ID_ARROW,TR("Arrow")+ CString(_T(" (")) + (char)kArrowKey  + CString(_T(")")), Toolbar::itButton, true,1));

    verticalToolbar_.addButton(Toolbar::Item(CString(),  loadToolbarIcon(IDB_ICONTOOLRECTANGLEPNG), ID_RECTANGLE,TR("Rectangle")+ CString(_T(" (")) + (char)kRectangleKey  + CString(_T(")")), Toolbar::itTinyCombo, true,1));
    verticalToolbar_.addButton(Toolbar::Item(CString(),  loadToolbarIcon(IDB_ICONTOOLFILLEDRECTANGLE), ID_FILLEDRECTANGLE,TR("Filled rectangle")+ CString(_T(" (")) + (char)kFilledRectangle  + CString(_T(")")), Toolbar::itTinyCombo, true,1));
    verticalToolbar_.addButton(Toolbar::Item(CString(),  loadToolbarIcon(IDB_ICONTOOLTEXTPNG), ID_TEXT,TR("Text")+ CString(_T(" (")) + (char)kTextKey  + CString(_T(")")), Toolbar::itButton, true,1));

    verticalToolbar_.addButton(Toolbar::Item(CString(),  loadToolbarIcon(IDB_ICONTOOLBLURINGRECTANGLEPNG), ID_BLURRINGRECTANGLE,TR("Blurring rectangle")+ CString(_T(" (")) + (char)kBlurringRectangleKey  + CString(_T(")")), Toolbar::itButton, true,1));

    //verticalToolbar_.addButton(Toolbar::Item(CString(),  loadToolbarIcon(IDB_ICONTOOLBRUSHPNG), ID_BLUR,TR("Blur"), Toolbar::itButton, true,1));
    verticalToolbar_.addButton(Toolbar::Item(CString(),  loadToolbarIcon(IDB_ICONCOLORPICKERPNG), ID_COLORPICKER,TR("Color chooser")+ CString(_T(" (")) + (char)kColorPickerKey  + CString(_T(")")), Toolbar::itButton, true,1));
    int index = verticalToolbar_.addButton(Toolbar::Item(CString(),  loadToolbarIcon(IDB_ICONUNDOPNG), ID_UNDO,TR("Undo") + CString(L" (Ctrl+Z)"), Toolbar::itButton, false));

    Toolbar::Item colorsButton(CString(),  loadToolbarIcon(IDB_ICONUNDOPNG), ID_UNDO,CString(), Toolbar::itButton, false);
    colorsDelegate_ = new ColorsDelegate(&verticalToolbar_, index+1, canvas_);
    colorsButton.itemDelegate = colorsDelegate_;
    colorsDelegate_->setBackgroundColor(canvas_->getBackgroundColor());
    colorsDelegate_->setForegroundColor(canvas_->getForegroundColor());
    verticalToolbar_.addButton(colorsButton);

    verticalToolbar_.AutoSize();
    if ( displayMode_ != wdmFullscreen ) {
        verticalToolbar_.ShowWindow(SW_SHOW);
    }
    horizontalToolbar_.penSizeSlider_.SetPos(canvas_->getPenSize());
    horizontalToolbar_.roundRadiusSlider_.SetPos(canvas_->getRoundingRadius());
}  


void ImageEditorWindow::OnCropChanged(int x, int y, int w, int h)
{
    if ( cropToolTip_ ) {
        ::DestroyWindow(cropToolTip_);
        cropToolTip_ = 0;
    }
    if ( displayMode_ != wdmFullscreen ) {
        return;
    }
    enum ToolbarPosition { pBottomRight, pBottomLeft, pTopRight, pTopLeft, pBottomRightInner };
    
    POINT scrollOffset;
    m_view.GetScrollOffset(scrollOffset);
    x -= scrollOffset.x;
    y -= scrollOffset.y;
    RECT horRc, vertRc;
    RECT clientRect;
    GetClientRect(&clientRect);
    horizontalToolbar_.GetClientRect(&horRc);
    verticalToolbar_.GetClientRect(&vertRc);

    ToolbarPosition pos = pBottomRight ;
    if ( y + h + horRc.bottom <= canvas_->getHeigth()  && x + w + vertRc.right <= clientRect.right  ) {
        pos = pBottomRight ;
    } else if ( y >= horRc.bottom && x >= vertRc.right )  {
         pos = pTopLeft;
    } else if ( y >= horRc.bottom && x + w + vertRc.right <= clientRect.right ) {
        pos = pTopRight;
    } else if ( x >= vertRc.right && y +h+horRc.bottom  <= clientRect.bottom) {
        pos = pBottomLeft;
    }
    POINT horToolbarPos = {0, 0};
    POINT vertToolbarPos = {0, 0};
    
    if ( pos == pBottomRight ) {
        horToolbarPos.x = x + w - horRc.right;
        horToolbarPos.y =  y + h + kToolbarOffset;

        vertToolbarPos.x = x + w + kToolbarOffset ;
        vertToolbarPos.y = y + h - vertRc.bottom;

        horToolbarPos.x = min(max<LONG>( horToolbarPos.x, 0),clientRect.right - vertRc.right - horRc.right);
        horToolbarPos.y = min( max( horToolbarPos.y, vertRc.bottom + kToolbarOffset ) , clientRect.bottom - horRc.bottom - kToolbarOffset);
        vertToolbarPos.x = min(max( vertToolbarPos.x, horRc.right + kToolbarOffset ), clientRect.right - vertRc.right);
        vertToolbarPos.y = min( max<LONG>( vertToolbarPos.y, 0), clientRect.bottom - horRc.bottom - kToolbarOffset - vertRc.bottom);

    } else if ( pos == pTopLeft ) {
        horToolbarPos.x = x;
        horToolbarPos.y =  y - horRc.bottom - kToolbarOffset;

        vertToolbarPos.x = x - vertRc.right - kToolbarOffset;
        vertToolbarPos.y = y ;

        horToolbarPos.x = min(max( horToolbarPos.x,  vertRc.right), clientRect.right - horRc.right - kToolbarOffset);
        horToolbarPos.y = min(max<LONG>( horToolbarPos.y, 0), clientRect.bottom - vertRc.bottom- horRc.bottom - kToolbarOffset);
        vertToolbarPos.x = min(max<LONG>( vertToolbarPos.x, 0), clientRect.right - horRc.right- vertRc.right - kToolbarOffset);
        vertToolbarPos.y = min(max( vertToolbarPos.y, horRc.bottom), clientRect.bottom - vertRc.bottom - kToolbarOffset);
    } else if ( pos == pTopRight ) {
        horToolbarPos.x = x + w - horRc.right - kToolbarOffset;
        horToolbarPos.y =  y - horRc.bottom - kToolbarOffset;

        vertToolbarPos.x = x + w + kToolbarOffset;
        vertToolbarPos.y = y ;

        horToolbarPos.x = /*min*(*/max<LONG>( horToolbarPos.x,  0)/*, clientRect.right - horRc.right - kToolbarOffset)*/;
        horToolbarPos.y = min(max<LONG>( horToolbarPos.y, 0), clientRect.bottom - vertRc.bottom- horRc.bottom - kToolbarOffset);
        vertToolbarPos.x = /*min(*/max( vertToolbarPos.x, horRc.right + kToolbarOffset)/*, clientRect.right - horRc.right- vertRc.right - kToolbarOffset)*/;
        vertToolbarPos.y = min(max( vertToolbarPos.y, horRc.bottom), clientRect.bottom - vertRc.bottom - kToolbarOffset);
    } else if ( pos == pBottomLeft ) {
        horToolbarPos.x = x;
        horToolbarPos.y =  y + h + kToolbarOffset;

        vertToolbarPos.x = x - vertRc.right - kToolbarOffset;
        vertToolbarPos.y = y + h - vertRc.bottom;

        horToolbarPos.x = min(max( horToolbarPos.x,  vertRc.right), clientRect.right - horRc.right - kToolbarOffset);
    
        vertToolbarPos.x = min(max<LONG>( vertToolbarPos.x, 0), clientRect.right - horRc.right- vertRc.right - kToolbarOffset);
    
        //horToolbarPos.y = max( horToolbarPos.y, vertRc.bottom + kToolbarOffset );
        horToolbarPos.y = max( horToolbarPos.y,  vertRc.bottom + kToolbarOffset);
        vertToolbarPos.y = max<LONG>( vertToolbarPos.y, 0);
    }
    //SIZE toolbarRect = {  horRc.right + vertRc.right, vertRc.bottom + horRc.bottom}


    ClientToScreen(&horToolbarPos);
    ClientToScreen(&vertToolbarPos);

    horizontalToolbar_.SetWindowPos(0, horToolbarPos.x, horToolbarPos.y, 0, 0, SWP_NOSIZE|SWP_NOZORDER|SWP_NOACTIVATE);
    //horizontalToolbar_.SetWindowPos(0, horToolbarPos.x, horToolbarPos.y, 0, 0, SWP_NOSIZE);
    verticalToolbar_.SetWindowPos(0, vertToolbarPos.x, vertToolbarPos.y, 0, 0, SWP_NOSIZE|SWP_NOZORDER|SWP_NOACTIVATE);
    SetActiveWindow();

}



void ImageEditorWindow::OnCropFinished(int x, int y, int w, int h)
{
    OnCropChanged(x,y,w,h);
    if ( !verticalToolbar_.IsWindowVisible() ) {
        verticalToolbar_.ShowWindow(SW_SHOW);
        horizontalToolbar_.ShowWindow(SW_SHOW);
    }
}

void ImageEditorWindow::OnDrawingToolChanged(Canvas::DrawingToolType drawingTool)
{

    updateToolbarDrawingTool(drawingTool);
    SendMessage(WM_SETCURSOR,0,0);
}

void ImageEditorWindow::OnTextEditStarted(ImageEditor::TextElement * textElement)
{
    InputBoxControl * control = static_cast<InputBoxControl*>(textElement->getInputBox());
    RECT inputControlRect;
    control->GetWindowRect(&inputControlRect);
    int kOffset = 30;
    RECT viewRect;
    RECT textParamsWindowRect;
    textParamsWindow_.GetWindowRect(&textParamsWindowRect);
    m_view.GetWindowRect(&viewRect);
    int x = inputControlRect.left;
    int y = 0;
    if ( inputControlRect.top > viewRect.top + (textParamsWindowRect.bottom - textParamsWindowRect.top) + kOffset ) {
        y = inputControlRect.top - (textParamsWindowRect.bottom - textParamsWindowRect.top) - kOffset;
    } else {
        y = inputControlRect.bottom + kOffset;
    }
    textParamsWindow_.SetWindowPos(0,x,y, 0,0, SWP_NOSIZE|SWP_NOACTIVATE|SWP_NOZORDER);

    textParamsWindow_.ShowWindow(SW_SHOW);
    control->SetFocus();
}

void ImageEditorWindow::OnTextEditFinished(ImageEditor::TextElement * textElement)
{
    textParamsWindow_.ShowWindow(SW_HIDE);
}

void ImageEditorWindow::OnSelectionChanged()
{
    updateRoundingRadiusSlider();
}

void ImageEditorWindow::updateRoundingRadiusSlider()
{
    bool showRoundingRadiusSlider = currentDrawingTool_ == Canvas::dtRoundedRectangle || currentDrawingTool_ == Canvas::dtFilledRoundedRectangle || canvas_->isRoundingRectangleSelected();
    horizontalToolbar_.roundRadiusLabel_.ShowWindow( showRoundingRadiusSlider ? SW_SHOW: SW_HIDE );
    horizontalToolbar_.roundRadiusSlider_.ShowWindow( showRoundingRadiusSlider ? SW_SHOW: SW_HIDE );
}

std::shared_ptr<Gdiplus::Bitmap>  ImageEditorWindow::loadToolbarIcon(int resource)
{
    return std::shared_ptr<Gdiplus::Bitmap>(BitmapFromResource(GetModuleHandle(0), MAKEINTRESOURCE(resource),_T("PNG")) );
}

void ImageEditorWindow::EndDialog(DialogResult dr)
{
    allowAltTab_ = false;
    dialogResult_ = dr;
    PostQuitMessage(0);
}

LRESULT ImageEditorWindow::OnMenuItemClick(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
    if (GetFocus() != m_view.m_hWnd)
    {
        ::SetFocus(m_view.m_hWnd);
    }
    Canvas::DrawingToolType toolId =  static_cast<Canvas::DrawingToolType>(menuItems_[wID]);
    canvas_->setDrawingToolType( toolId  );
    updateToolbarDrawingTool(toolId);
    return 0;
}

LRESULT ImageEditorWindow::OnUndoClick(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
    canvas_->undo();
    return 0;
}


LRESULT ImageEditorWindow::OnTextClick(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {

    canvas_->setDrawingToolType( Canvas::dtText );
    updateToolbarDrawingTool(Canvas::dtText);
    return 0;
}

LRESULT ImageEditorWindow::OnClickedClose(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& bHandled)
{
    DialogResult dr = drCancel;
    if ( askBeforeClose_ && canvas_->isDocumentModified() ) {
        int msgBoxResult = MessageBox(TR("Save changes?"), APPNAME, MB_YESNOCANCEL|MB_ICONQUESTION);
        if ( msgBoxResult == IDYES ) {
            dr = outFileName_.IsEmpty() ? drCancel : drSave;
            OnClickedSave();
        } else if ( msgBoxResult == IDCANCEL ) {
            return 0;
        }
    }
    EndDialog(dr);
    return 0;
}

LRESULT ImageEditorWindow::OnClickedAddToWizard(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
    resultingBitmap_ = canvas_->getBitmapForExport();
    canvas_->setDocumentModified(false);

    if (!resultingBitmap_) {
        LOG(ERROR) << "canvas_->getBitmapForExport() returned NULL";
        return 0;
    }

    EndDialog(drAddToWizard);
    
    return 0;
}

LRESULT ImageEditorWindow::OnClickedUpload(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
    resultingBitmap_ = canvas_->getBitmapForExport();
    canvas_->setDocumentModified(false);

    if (!resultingBitmap_) {
        LOG(ERROR) << "canvas_->getBitmapForExport() returned NULL";
        return 0;
    }

    EndDialog(drUpload);
    return 0;
}

LRESULT ImageEditorWindow::OnClickedShare(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
    if ( saveDocument() ) {
        EndDialog(drShare);
    }
    return 0;
}

LRESULT ImageEditorWindow::OnClickedSave(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
    OnClickedSave();
    return 0;
}

LRESULT ImageEditorWindow::OnClickedSaveAs(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
    OnSaveAs();
    return 0;
}

LRESULT ImageEditorWindow::OnClickedCopyToClipboard(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
    CopyBitmapToClipboardAndClose(ClipboardFormat::Bitmap);
   
    return 0;
}

LRESULT ImageEditorWindow::OnClickedCopyToClipboardAsDataUri(WORD, WORD, HWND, BOOL&) {
    CopyBitmapToClipboardAndClose(ClipboardFormat::DataUri);
    return 0;
}

LRESULT ImageEditorWindow::OnClickedCopyToClipboardAsDataUriHtml(WORD, WORD, HWND, BOOL&) {
    CopyBitmapToClipboardAndClose(ClipboardFormat::DataUriHtml);
    return 0;
}

LRESULT ImageEditorWindow::OnPrintImage(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled) {
    if (GetFocus() != m_view.m_hWnd) {
        ::SetFocus(m_view.m_hWnd);
    }
    bool res = saveDocument(); // Save screenshot in default format
    if (res) {
        std::vector<CString> files = { outFileName_ };
        WinUtils::DisplaySystemPrintDialogForImage(files, m_hWnd);
        if (displayMode_ == wdmFullscreen &&  !(GetKeyState(VK_SHIFT) & 0x8000)) {
            EndDialog(drPrintRequested);
        }
    }
    return res;
}

bool ImageEditorWindow::createTooltip() {
    // Create a tooltip.
    cropToolTip_ = ::CreateWindowEx(WS_EX_TOPMOST, TOOLTIPS_CLASS, NULL, 
        WS_POPUP | TTS_NOPREFIX | TTS_ALWAYSTIP, 
        CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, 
        m_view.m_hWnd, NULL, _Module.GetModuleInstance(),NULL);

    ::SetWindowPos(cropToolTip_, HWND_TOPMOST, 0, 0, 0, 0, 
        SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);

    // Set up "tool" information. In this case, the "tool" is the entire parent window.


    TOOLINFO ti = { 0 };
    ti.cbSize   = sizeof(TOOLINFO);
    ti.uFlags   = TTF_SUBCLASS;
    ti.hwnd     = m_view.m_hWnd;
    ti.hinst    = _Module.GetModuleInstance();
    ti.lpszText = TR_CONST("Select region");;
    m_view.GetClientRect(&ti.rect);

    // Associate the tooltip with the "tool" window.
    SendMessage(cropToolTip_, TTM_ADDTOOL, 0, (LPARAM) (LPTOOLINFO) &ti);    
    return true;
}

void ImageEditorWindow::updatePixelLabels()
{
    horizontalToolbar_.pixelLabel_.SetWindowText(WinUtils::IntToStr(canvas_->getPenSize()) + L" px");
    horizontalToolbar_.roundRadiusLabel_.SetWindowText(WinUtils::IntToStr(canvas_->getRoundingRadius()) + L" px");
}

void ImageEditorWindow::OnSaveAs()
{
    TCHAR Buf[MAX_PATH*4];
    GuiTools::SelectDialogFilter(Buf, sizeof(Buf)/sizeof(TCHAR), 2,
        _T("PNG"), CString(_T("*.png")),
        _T("JPEG"), CString(_T("*.jpg;*.jpeg")),
        TR("All files"),_T("*.*"));
    CFileDialog fd(false, GetFileExt(suggestedFileName_), suggestedFileName_ ,OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,Buf,m_hWnd);
    if(fd.DoModal()!=IDOK || !fd.m_szFileName[0]) return;

    outFileName_ = fd.m_szFileName;
    saveDocument();
}

void ImageEditorWindow::saveSettings()
{
    if ( configurationProvider_ ) {
        configurationProvider_->setPenSize(canvas_->getPenSize());
        configurationProvider_->setForegroundColor(canvas_->getForegroundColor());
        configurationProvider_->setBackgroundColor(canvas_->getBackgroundColor());
        configurationProvider_->setFont(canvas_->getFont());
        configurationProvider_->setRoundingRadius(canvas_->getRoundingRadius());
        configurationProvider_->saveConfiguration();
    }
}

bool ImageEditorWindow::CopyBitmapToClipboardAndClose(ClipboardFormat format) {
    if (GetFocus() != m_view.m_hWnd) {
        ::SetFocus(m_view.m_hWnd);
    }
    bool res = saveDocument(format);
    if (res && displayMode_ == wdmFullscreen &&  !(GetKeyState(VK_SHIFT) & 0x8000)) {
        EndDialog(drCopiedToClipboard);
    }
    return res;
}

BOOL ImageEditorWindow::PreTranslateMessage(MSG* pMsg) {
    if (TranslateAccelerator(m_hWnd, accelerators_, pMsg)) {
        return TRUE;
    }
    return FALSE;
}

void ImageEditorWindow::OnClickedSave() {
    if (GetFocus() != m_view.m_hWnd) {
        ::SetFocus(m_view.m_hWnd);
    }
    if (!sourceFileName_.IsEmpty()) {
        outFileName_ = sourceFileName_;
        saveDocument();
    } else {
        OnSaveAs();
    }
}

LRESULT ImageEditorWindow::OnHScroll(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/)
{
    HWND hwndSender = reinterpret_cast<HWND>(lParam);
    if ( hwndSender == horizontalToolbar_.penSizeSlider_.m_hWnd  ) {
        int penSize = HIWORD(wParam);
        switch( LOWORD(wParam ) ) {
        case TB_ENDTRACK:
            penSize = horizontalToolbar_.penSizeSlider_.GetPos();
        case TB_THUMBPOSITION:
            canvas_->endPenSizeChanging(penSize);
            updatePixelLabels();
            prevPenSize_ = 0;
            break;
        case TB_PAGEDOWN:
        case TB_PAGEUP:
        case TB_TOP:
        case TB_LINEDOWN:
        case TB_LINEUP:
        case TB_BOTTOM:
            penSize = horizontalToolbar_.penSizeSlider_.GetPos();
        case TB_THUMBTRACK:
            if ( !prevPenSize_ ) {
                prevPenSize_ = canvas_->getPenSize();
                canvas_->beginPenSizeChanging();
            }
            canvas_->setPenSize(penSize);
            updatePixelLabels();
            break;
        }
    } else if ( hwndSender == horizontalToolbar_.roundRadiusSlider_.m_hWnd ) {
        int roundingRadius = HIWORD(wParam);
        switch( LOWORD(wParam ) ) {
        case TB_ENDTRACK:
            roundingRadius = horizontalToolbar_.roundRadiusSlider_.GetPos();
        case TB_THUMBPOSITION:
            canvas_->endRoundingRadiusChanging(roundingRadius);
            updatePixelLabels();
            prevRoundingRadius_ = 0;
            break;
        case TB_PAGEDOWN:
        case TB_PAGEUP:
        case TB_TOP:
        case TB_LINEDOWN:
        case TB_LINEUP:
        case TB_BOTTOM:
            roundingRadius = horizontalToolbar_.roundRadiusSlider_.GetPos();
        case TB_THUMBTRACK:
            if ( !prevRoundingRadius_ ) {
                prevRoundingRadius_ = canvas_->getRoundingRadius();
                canvas_->beginRoundingRadiusChanging();
            }
            canvas_->setRoundingRadius(roundingRadius);
            updatePixelLabels();
            break;
        }
    }
    
    return 0;
}

LRESULT ImageEditorWindow::OnTextParamWindowFontChanged(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& /*bHandled*/)
{
    DWORD changeMask = lParam;
    canvas_->setFont(textParamsWindow_.getFont(), changeMask);
    return 0;
}

LRESULT ImageEditorWindow::OnUnselectAll(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled) {
    canvas_->unselectAllElements();
    canvas_->updateView();
    return 0;
}
}