#include "ImageEditorWindow.h"

#include <boost/format.hpp>

#include "ImageEditorView.h"
#include "ImageEditor/Gui/Toolbar.h"
#include "ColorsDelegate.h"
#include "Core/Logging.h"
#include "Core/Images/Utils.h"
#include "Gui/GuiTools.h"
#include "Func/WinUtils.h"
#include "ImageEditor/MovableElements.h"
#include "Gui/Dialogs/SearchByImageDlg.h"
#include "Gui/Components/MyFileDialog.h"
#include "Core/ScreenCapture/MonitorEnumerator.h"

namespace ImageEditor {
    
ImageEditorWindow::ImageEditorWindow(std::shared_ptr<Gdiplus::Bitmap> bitmap, bool hasTransparentPixels, ConfigurationProvider* configurationProvider ):horizontalToolbar_(Toolbar::orHorizontal),verticalToolbar_(Toolbar::orVertical) 
{
    currentDoc_ =  std::make_unique<ImageEditor::Document>(std::move(bitmap), hasTransparentPixels);
    configurationProvider_ = configurationProvider;
    askBeforeClose_ = true;
    allowAltTab_ = false;

    init();
}

ImageEditorWindow::ImageEditorWindow(CString imageFileName, ConfigurationProvider* configurationProvider ):horizontalToolbar_(Toolbar::orHorizontal),verticalToolbar_(Toolbar::orVertical) 
{
    currentDoc_ = std::make_unique<ImageEditor::Document>(imageFileName);
    
    sourceFileName_ = imageFileName;
    configurationProvider_ = configurationProvider;
    askBeforeClose_ = true;
    allowAltTab_ = false;
    suggestedFileName_ = WinUtils::myExtractFileName(sourceFileName_);
    CString fileExt = WinUtils::GetFileExt(suggestedFileName_);
    fileExt.MakeLower();
    if (fileExt != _T("png") && fileExt != _T("jpg") && fileExt != _T("jpeg") && fileExt != _T("webp")) {
        suggestedFileName_ = WinUtils::GetOnlyFileName(suggestedFileName_) + _T(".png");
    }
    init();
}

void ImageEditorWindow::init()
{
    richeditLib_ = LoadLibrary(CRichEditCtrl::GetLibraryName());
//    resultingBitmap_ = 0;
    cropToolTip_ = 0;
    showUploadButton_ = true;
    showAddToWizardButton_ = true;
    prevPenSize_ = 0;
    prevRoundingRadius_ = 0;
    imageQuality_ = 85;
    searchEngine_ = SearchByImage::SearchEngine::seGoogle;
    currentDrawingTool_ = DrawingToolType::dtNone;
    initialDrawingTool_ = DrawingToolType::dtBrush;
    displayMode_ = wdmAuto;
    accelerators_ = nullptr;
    menuItems_[ID_PEN]             = DrawingToolType::dtPen;
    menuItems_[ID_LINE]            = DrawingToolType::dtLine;
    menuItems_[ID_BRUSH]           = DrawingToolType::dtBrush;
    menuItems_[ID_MARKER]          = DrawingToolType::dtMarker;
    menuItems_[ID_RECTANGLE]       = DrawingToolType::dtRectangle;
    menuItems_[ID_FILLEDRECTANGLE] = DrawingToolType::dtFilledRectangle;
    menuItems_[ID_CROP]            = DrawingToolType::dtCrop;
    menuItems_[ID_MOVE]            = DrawingToolType::dtMove;
    menuItems_[ID_ARROW]           = DrawingToolType::dtArrow;
    menuItems_[ID_SELECTION]       = DrawingToolType::dtSelection;
    menuItems_[ID_BLUR]            = DrawingToolType::dtBlur;
    menuItems_[ID_BLURRINGRECTANGLE]   = DrawingToolType::dtBlurrringRectangle;
    menuItems_[ID_PIXELATERECTANGLE]   = DrawingToolType::dtPixelateRectangle;
    menuItems_[ID_COLORPICKER]     = DrawingToolType::dtColorPicker;
    menuItems_[ID_ROUNDEDRECTANGLE]     = DrawingToolType::dtRoundedRectangle;
    menuItems_[ID_ELLIPSE]     = DrawingToolType::dtEllipse;
    menuItems_[ID_FILLEDROUNDEDRECTANGLE]     = DrawingToolType::dtFilledRoundedRectangle;
    menuItems_[ID_FILLEDELLIPSE]     = DrawingToolType::dtFilledEllipse;
    menuItems_[ID_TEXT]     = DrawingToolType::dtText;
    menuItems_[ID_STEPNUMBER]     = DrawingToolType::dtStepNumber;

    SubMenuItem item;
    item.parentCommand = ID_RECTANGLE;
    item.icon = loadToolbarIcon(IDB_ICONTOOLRECTANGLEPNG);
    item.command = ID_RECTANGLE;
    item.hint = TR("Rectangle");
    subMenuItems_[DrawingToolType::dtRectangle] = item;

    item.icon = loadToolbarIcon(IDB_ICONTOOLROUNDEDRECTANGLE);
    item.command = ID_ROUNDEDRECTANGLE;
    item.hint = TR("Rounded rectangle");
    subMenuItems_[DrawingToolType::dtRoundedRectangle] = item;

    item.icon = loadToolbarIcon(IDB_ICONTOOLELLIPSE);
    item.command = ID_ELLIPSE;
    item.hint = TR("Ellipse");
    subMenuItems_[DrawingToolType::dtEllipse] = item;

    item.parentCommand = ID_FILLEDRECTANGLE;
    item.command = ID_FILLEDRECTANGLE;
    item.hint = TR("Filled rectangle");
    item.icon = loadToolbarIcon(IDB_ICONTOOLFILLEDRECTANGLE);
    subMenuItems_[DrawingToolType::dtFilledRectangle] = item;

    item.icon = loadToolbarIcon(IDB_ICONTOOLFILLEDROUNDEDRECTANGLE);
    item.command = ID_FILLEDROUNDEDRECTANGLE;
    item.hint = TR("Rounded rectangle");
    subMenuItems_[DrawingToolType::dtFilledRoundedRectangle] = item;

    item.icon = loadToolbarIcon(IDB_ICONTOOLFILLEDELLIPSE);
    item.command = ID_FILLEDELLIPSE;
    item.hint =  TR("Filled ellipse");
    subMenuItems_[DrawingToolType::dtFilledEllipse] = item;

    SubMenuItem item2;
    item2.parentCommand = ID_BLURRINGRECTANGLE;

    item2.icon = loadToolbarIcon(IDB_ICONTOOLBLURINGRECTANGLEPNG);
    item2.command = ID_BLURRINGRECTANGLE;
    item2.hint = TR("Blurring rectangle");
    subMenuItems_[DrawingToolType::dtBlurrringRectangle] = item2;

    item2.icon = loadToolbarIcon(IDB_ICONTOOLPIXELATE);
    item2.command = ID_PIXELATERECTANGLE;
    item2.hint = TR("Pixelation");
    subMenuItems_[DrawingToolType::dtPixelateRectangle] = item2;

    selectedSubMenuItems_[ID_RECTANGLE] = ID_RECTANGLE;
    selectedSubMenuItems_[ID_FILLEDRECTANGLE] = ID_FILLEDRECTANGLE;
    selectedSubMenuItems_[ID_BLURRINGRECTANGLE] = ID_BLURRINGRECTANGLE;

    drawingToolsHotkeys_[kMoveKey] = DrawingToolType::dtMove;
    drawingToolsHotkeys_[kBrushKey] = DrawingToolType::dtBrush;
    drawingToolsHotkeys_[kTextKey] = DrawingToolType::dtText;
    drawingToolsHotkeys_[kRectangleKey] = DrawingToolType::dtRectangle;
    drawingToolsHotkeys_[kColorPickerKey] = DrawingToolType::dtColorPicker;
    drawingToolsHotkeys_[kCropKey] = DrawingToolType::dtCrop;
    drawingToolsHotkeys_[kMarkerKey] = DrawingToolType::dtMarker;
    drawingToolsHotkeys_[kBlurringRectangleKey] = DrawingToolType::dtBlurrringRectangle;
    drawingToolsHotkeys_[kArrowKey] = DrawingToolType::dtArrow;
    drawingToolsHotkeys_[kLineKey] = DrawingToolType::dtLine;
    drawingToolsHotkeys_[kFilledRectangle] = DrawingToolType::dtFilledRectangle;
    drawingToolsHotkeys_[kStepNumber] = DrawingToolType::dtStepNumber;
    
    dialogResult_ = drCancel;
}

bool ImageEditorWindow::saveDocument(ClipboardFormat clipboardFormat, bool saveAs)
{
    resultingBitmap_ = canvas_->getBitmapForExport();
	if (!saveAs) {
        canvas_->setDocumentModified(false);
	}

    if ( !resultingBitmap_ ) {
        LOG(ERROR) << "canvas_->getBitmapForExport() returned NULL";
        return false;
    }
    if (clipboardFormat == ClipboardFormat::None) {
        if ( !outFileName_.IsEmpty() ) {
            try {
                ImageUtils::SaveImageToFile(resultingBitmap_.get(), outFileName_, nullptr, ImageUtils::sifDetectByExtension, imageQuality_);
            } catch (const std::exception& ex) {
                ServiceLocator::instance()->logger()->write(ILogger::logError, _T("Image Editor"), U2W(ex.what()));
            }
            canvas_->updateView();
            return true;
        } else {
            CString outFileName;
            try {
                if (ImageUtils::MySaveImage(resultingBitmap_.get(), "screeenshot001.png", outFileName, ImageUtils::sifJPEG, 95)) {
                    outFileName_ = outFileName;
                    canvas_->updateView();
                    return true;
                }
            } catch (const std::exception& ex) {
                ServiceLocator::instance()->logger()->write(ILogger::logError, _T("Image Editor"), U2W(ex.what()));
            }
            return false;
        }
    } else if (clipboardFormat  == ClipboardFormat::Bitmap ) {
        CWindowDC dc(m_hWnd);
        ImageUtils::CopyBitmapToClipboard(m_hWnd, dc, resultingBitmap_.get(), true);
        return true;
    } else if (clipboardFormat == ClipboardFormat::DataUri || clipboardFormat == ClipboardFormat::DataUriHtml) {
        try {
            ImageUtils::CopyBitmapToClipboardInDataUriFormat(resultingBitmap_.get(), ImageUtils::sifPNG, 85, clipboardFormat == ClipboardFormat::DataUriHtml);
        } catch (const std::exception& ex) {
            ServiceLocator::instance()->logger()->write(ILogger::logError, _T("Image Editor"), U2W(ex.what()));
        }
        return true;
    }
    return false;
}

CString ImageEditorWindow::saveToTempFile() {
    const std::shared_ptr<Gdiplus::Bitmap> bitmap = canvas_->getBitmapForExport();
    CString result;
    try {
        if (ImageUtils::MySaveImage(bitmap.get(), "image.png", result, ImageUtils::sifPNG, 95)) {
            return result;
        }
    } catch (const std::exception& ex) {
        LOG(ERROR) << ex.what();
    }
    return CString();
}

void ImageEditorWindow::updateToolbarDrawingTool(DrawingToolType dt)
{
    if ( currentDrawingTool_ == dt ) {
        return;
    }

    currentDrawingTool_ = dt;
    updateRoundingRadiusSlider();
    updateFontSizeControls();

    const auto submenuIter = subMenuItems_.find(dt);
    if ( submenuIter != subMenuItems_.end() ) {
        const int buttonIndex = verticalToolbar_.getItemIndexByCommand(selectedSubMenuItems_[submenuIter->second.parentCommand]);
        if ( buttonIndex != -1 ) {
            Toolbar::Item* item = verticalToolbar_.getItem(buttonIndex);
            if ( item ) {
                item->icon = submenuIter->second.icon;
                item->command = submenuIter->second.command;
                item->hint = submenuIter->second.hint;
                verticalToolbar_.updateTooltipForItem(buttonIndex);
                selectedSubMenuItems_[submenuIter->second.parentCommand] = item->command;
            }
            
            verticalToolbar_.clickButton(buttonIndex);
            return;
        }
    }

    //std::map<int, Canvas::DrawingToolType>::iterator it = std::find(menuItems_.begin(), menuItems_.end(), dt);
    std::map<int, DrawingToolType>::iterator it;
    for( it = menuItems_.begin(); it != menuItems_.end(); ++it ) {
        if ( it->second == dt ) {
            int buttonIndex = verticalToolbar_.getItemIndexByCommand(it->first);
            if ( buttonIndex != -1 ) {
                verticalToolbar_.clickButton(buttonIndex);
                break;
            }
        }
    }
    if ( dt == DrawingToolType::dtCrop ) {
        createTooltip();
    } else if ( cropToolTip_ ) {
        ::DestroyWindow(cropToolTip_);
        cropToolTip_ = nullptr;
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
    FreeLibrary(richeditLib_);
}

void ImageEditorWindow::setInitialDrawingTool(DrawingToolType dt)
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

void ImageEditorWindow::setSuggestedFileName(CString fileName)
{
    suggestedFileName_ = fileName;
}

std::shared_ptr<Gdiplus::Bitmap> ImageEditorWindow::getResultingBitmap() const
{
    return resultingBitmap_;
}

Gdiplus::Rect ImageEditorWindow::lastAppliedCrop() const {
    return canvas_ ? canvas_->lastAppliedCrop() : Gdiplus::Rect();
}

void ImageEditorWindow::setServerName(const CString & serverName)
{
    serverName_ = serverName;
}

void ImageEditorWindow::setAskBeforeClose(bool ask)
{
    askBeforeClose_ = ask;
}

ImageEditorWindow::DialogResult ImageEditorWindow::DoModal(HWND parent, HMONITOR screenshotsMonitor, WindowDisplayMode mode)
{
    if (currentDoc_->isNull()) {
        GuiTools::LocalizedMessageBox(nullptr, _T("Invalid image file."), APPNAME, MB_ICONERROR);
        return drCancel;
    }

    static BOOL verificationFlagChecked = FALSE;
    if (currentDoc_ && currentDoc_->isSrcMultiFrame() && !verificationFlagChecked) {
        int buttonPressed;
        CTaskDialog dlg;
        CString verText = TR("Do not ask again");
        dlg.SetVerificationText(verText);
        CString contentText = TR("Image Editor cannot deal with animated images. If you edit this file, animation will be lost. Do you wish to continue?");
        dlg.SetContentText(contentText);
        CString windowTitle = TR("Image Editor");
        dlg.SetWindowTitle(windowTitle);
        dlg.SetCommonButtons(TDCBF_YES_BUTTON | TDCBF_NO_BUTTON);
        dlg.SetMainIcon(TD_WARNING_ICON);
        DWORD flags = TDF_POSITION_RELATIVE_TO_WINDOW | TDF_ALLOW_DIALOG_CANCELLATION;
        if (ServiceLocator::instance()->translator()->isRTL()) {
            flags |= TDF_RTL_LAYOUT;
        }
        dlg.ModifyFlags(0, flags);
        
        int res = dlg.DoModal(parent, &buttonPressed, nullptr, &verificationFlagChecked);
        if (SUCCEEDED(res) && buttonPressed != IDYES) {
            return drCancel;
        }
    }
    //int screenWidth = GetSystemMetrics(SM_CXSCREEN);
    //int screenHeight = GetSystemMetrics(SM_CYSCREEN);
    //    displayMode_ = wdmWindowed;

    CRect displayBounds;
    GuiTools::GetScreenBounds(displayBounds);

    CWindowDC hdc(nullptr);
    float dpiScaleX = GetDeviceCaps(hdc, LOGPIXELSX) / 96.0f;
    float dpiScaleY = GetDeviceCaps(hdc, LOGPIXELSY) / 96.0f;
    int scrollbarWidth = GetSystemMetrics(SM_CXVSCROLL);
    int scrollbarHeight = GetSystemMetrics(SM_CYVSCROLL);
    int desiredClientWidth = currentDoc_->getWidth() + static_cast<int>(40 * dpiScaleX) + kCanvasMargin + scrollbarWidth; // with toolbars
    int desiredClientHeight = currentDoc_->getHeight() + static_cast<int>(60 * dpiScaleY) + kCanvasMargin + scrollbarHeight;
    CRect rc(rcDefault);
    DWORD initialWindowStyle = WS_OVERLAPPED | WS_POPUP | WS_CAPTION | WS_SYSMENU | WS_SIZEBOX | WS_MAXIMIZEBOX |
        WS_MINIMIZEBOX | WS_CLIPCHILDREN;
    // Parent window should be null here!
    if (Create(nullptr, rc, _T("Image Editor"), initialWindowStyle, 0) == NULL) {
        LOG(ERROR) << "Main window creation failed!\n";
        return drCancel;
    }

    CRect workArea;
    MONITORINFO mi;
    mi.cbSize = sizeof(mi);
    ::GetMonitorInfo(::MonitorFromWindow(m_hWnd, MONITOR_DEFAULTTONEAREST), &mi);
    workArea = mi.rcWork;

    CRect screenshotMonitorBounds;
    if (screenshotsMonitor) {
        memset(&mi, 0, sizeof(mi));
        mi.cbSize = sizeof(mi);
        if (!GetMonitorInfo(screenshotsMonitor, &mi)) {
            LOG(ERROR) << "Error getting information about monitor";
        }
        screenshotMonitorBounds = mi.rcMonitor;
    }

    if (mode == wdmAuto) {
        /*CRect newClientRect(0, 0, desiredClientWidth, desiredClientHeight);
        AdjustWindowRect(newClientRect, GetStyle(), false);*/

        /*LOG(ERROR) << "NewClienRect ajusted =" << newClientRect.Width() << "x" << newClientRect.Height() << std::endl
            
            << " displayBounds = " << displayBounds.Width() << "x" << displayBounds.Height() << std::endl
            << " currentDoc = " << currentDoc_->getWidth() << "x" << currentDoc_->getHeight();*/

        // newClientRect.Width() >= workArea.Width()  || newClientRect.Height()  >= workArea.Height()
        if ( currentDoc_->getWidth() == displayBounds.Width() &&  currentDoc_->getHeight() == displayBounds.Height()) {
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
//#ifndef _DEBUG
        if (!allowAltTab_) {
            SetWindowLong(GWL_EXSTYLE, WS_EX_TOPMOST);
        }
        insertAfter = HWND_TOPMOST;
//#endif
        if (screenshotsMonitor) {
            SetWindowPos(insertAfter, screenshotMonitorBounds.left, screenshotMonitorBounds.top, screenshotMonitorBounds.right - screenshotMonitorBounds.left, screenshotMonitorBounds.bottom - screenshotMonitorBounds.top, 0);
        } else {
            SetWindowPos(insertAfter, displayBounds.left, displayBounds.top, displayBounds.right - displayBounds.left, displayBounds.bottom - displayBounds.top, 0);
        }
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
        { FVIRTKEY | FCONTROL, 'S', ID_SAVE },
        { FVIRTKEY | FCONTROL, 'C', ID_COPYBITMAPTOCLIBOARD },
        { FVIRTKEY | FCONTROL, 'F', ID_SEARCHBYIMAGE },
        { FVIRTKEY | FCONTROL, 'P', ID_PRINTIMAGE },
    };

    accelerators_ = CreateAcceleratorTable(accels, ARRAY_SIZE(accels));

    //RECT rc;
    GetClientRect(&rc);
    /*HWND m_hWndClient = */m_view.Create(m_hWnd, rc, _T("ImageEditor_Canvas"), WS_CHILD | WS_VISIBLE /*| WS_CLIPSIBLINGS | WS_CLIPCHILDREN*/, 0);

    canvas_ = std::make_unique<ImageEditor::Canvas>(m_view);
    canvas_->setSize(currentDoc_->getWidth(), currentDoc_->getHeight());
    canvas_->setDocument(currentDoc_.get());
    canvas_->setCropOnExport(displayMode_ == wdmFullscreen);

    updateWindowTitle();

    textParamsWindow_.Create(m_hWnd);

    if (configurationProvider_) {
        canvas_->setPenSize(configurationProvider_->penSize());
        canvas_->setForegroundColor(configurationProvider_->foregroundColor());
        canvas_->setBackgroundColor(configurationProvider_->backgroundColor());
        canvas_->setFont(configurationProvider_->font());
        canvas_->setRoundingRadius(configurationProvider_->roundingRadius());
        canvas_->setFillTextBackground(configurationProvider_->fillTextBackground());
        canvas_->setStepColors(configurationProvider_->stepForegroundColor(), configurationProvider_->stepBackgroundColor());
        canvas_->setArrowMode(static_cast<Arrow::ArrowMode>(configurationProvider_->getArrowMode()));
        allowAltTab_ = configurationProvider_->allowAltTab();
        textParamsWindow_.setFont(configurationProvider_->font());
        searchEngine_ = configurationProvider_->searchEngine();
    }
    m_view.setCanvas(canvas_.get());
    createToolbars();
    RECT horToolbarRect;
    RECT vertToolbarRect;
    horizontalToolbar_.GetClientRect(&horToolbarRect);
    verticalToolbar_.GetClientRect(&vertToolbarRect);
    rc.left = displayMode_ == wdmWindowed ? vertToolbarRect.right + kCanvasMargin : 0;
    rc.top = displayMode_ == wdmWindowed ? horToolbarRect.bottom + kCanvasMargin : 0;

    if (displayMode_ == wdmWindowed) {
        horizontalToolbar_.SetWindowPos(0, rc.left, 0, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
        verticalToolbar_.SetWindowPos(0, 0, rc.top, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
    } else {
        repositionToolbar(horizontalToolbar_, vertToolbarRect);
        repositionToolbar(verticalToolbar_, horToolbarRect);
    }

    m_view.SetWindowPos(0, &rc, SWP_NOSIZE);

    using namespace std::placeholders;
    canvas_->onDrawingToolChanged.connect(std::bind(&ImageEditorWindow::OnDrawingToolChanged, this, _1));
    canvas_->onForegroundColorChanged.connect(std::bind(&ImageEditorWindow::OnForegroundColorChanged, this, _1));
    canvas_->onBackgroundColorChanged.connect(std::bind(&ImageEditorWindow::OnBackgroundColorChanged, this, _1));
    canvas_->onCropChanged.connect(std::bind(&ImageEditorWindow::OnCropChanged, this, _1, _2, _3, _4));
    canvas_->onFontChanged.connect(std::bind(&ImageEditorWindow::onFontChanged, this,  _1));
    canvas_->onTextEditStarted.connect(std::bind(&ImageEditorWindow::OnTextEditStarted, this, _1));
    canvas_->onTextEditFinished.connect(std::bind(&ImageEditorWindow::OnTextEditFinished, this, _1));
    canvas_->onSelectionChanged.connect(std::bind(&ImageEditorWindow::OnSelectionChanged, this));
    /*if (displayMode_ != wdmWindowed)*/ {
        canvas_->onCropFinished.connect(std::bind(&ImageEditorWindow::OnCropFinished, this, _1, _2, _3, _4));
    }

    canvas_->onDocumentModified.connect([this] { updateWindowTitle();  });

    if (initialDrawingTool_ != DrawingToolType::dtCrop) {
        verticalToolbar_.ShowWindow(SW_SHOW);
        horizontalToolbar_.ShowWindow(SW_SHOW);
    }

    updatePixelLabels();

    canvas_->setDrawingToolType(initialDrawingTool_);
    updateToolbarDrawingTool(initialDrawingTool_);
    if (displayMode_ == wdmWindowed) {
        MONITORINFO mi;
        mi.cbSize = sizeof(mi);
        ::GetMonitorInfo(::MonitorFromWindow(m_hWnd, MONITOR_DEFAULTTONEAREST), &mi);
        workArea = mi.rcWork;
        CRect newClientRect(0, 0, desiredClientWidth, desiredClientHeight);
        AdjustWindowRect(newClientRect, GetStyle(), false);
        int newWidth = std::max(static_cast<LONG>(500 * dpiScaleX), newClientRect.right - newClientRect.left);
        int newHeight = std::max(static_cast<LONG>(500 * dpiScaleY), newClientRect.bottom - newClientRect.top);
        newWidth = std::min(newWidth, workArea.Width()-4);
        newHeight= std::min(newHeight, workArea.Height()-4);

        SetWindowPos(0, 0, 0, newWidth, newHeight, SWP_NOMOVE | SWP_NOZORDER);
        //ResizeClient(newWidth, newHeight);
        CenterWindow(parent);
    }
    canvas_->updateView();
    m_view.SetFocus();
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
    loop.RemoveMessageFilter(this);
    DestroyWindow();
    DestroyAcceleratorTable(accelerators_);
    accelerators_ = nullptr;
    if ( dialogResult_ == drCancel  ) {
//        delete resultingBitmap_;
        //resultingBitmap_ = 0;
    }
    
    return dialogResult_;
}

LRESULT ImageEditorWindow::OnNcCreate(UINT, WPARAM, LPARAM, BOOL&) {
    SetWindowLong(GWL_EXSTYLE, GetWindowLong(GWL_EXSTYLE) & ~WS_EX_LAYOUTRTL);
    return TRUE;
}

LRESULT ImageEditorWindow::OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
    SetWindowText(TR("Image Editor"));
    return 0;
}

LRESULT ImageEditorWindow::OnDestroy(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled)
{
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
     std::map<DrawingToolHotkey, DrawingToolType>::iterator it;
     //HKL englishLayout = LoadKeyboardLayout(_T("00000409"),0);
     if ( wParam == VK_ESCAPE ) {
        onClose();
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
    if (allowAltTab_ && displayMode_ == wdmFullscreen) {
        if ( !wParam ) { // if the window is being deactivated
            LONG exStyle = GetWindowLong(GWL_EXSTYLE);
            SetWindowLong(GWL_EXSTYLE, exStyle & ~(WS_EX_TOPMOST));
            SetWindowPos(HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE );
            SetWindowPos(HWND_BOTTOM, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE );
            HWND foregroundWindow = GetForegroundWindow();
            if ( foregroundWindow != m_hWnd ) {
                ::SetWindowPos(foregroundWindow, m_hWnd, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE );
            }
        } else {
            SetWindowPos(HWND_TOPMOST, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE );
            LONG exStyle = GetWindowLong(GWL_EXSTYLE);
            SetWindowLong(GWL_EXSTYLE, exStyle | WS_EX_TOPMOST);
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
    }
    else if (item->command == ID_BLURRINGRECTANGLE || item->command == ID_PIXELATERECTANGLE) {
        CMenu blurMenu;
        RECT rc = item->rect;
        verticalToolbar_.ClientToScreen(&rc);
        blurMenu.CreatePopupMenu();
        blurMenu.AppendMenu(MF_STRING, ID_BLURRINGRECTANGLE, TR("Blurring rectangle"));
        blurMenu.AppendMenu(MF_STRING, ID_PIXELATERECTANGLE, TR("Pixelation"));
        TPMPARAMS excludeArea;
        ZeroMemory(&excludeArea, sizeof(excludeArea));
        excludeArea.cbSize = sizeof(excludeArea);
        excludeArea.rcExclude = rc;
        blurMenu.TrackPopupMenuEx(TPM_LEFTALIGN | TPM_LEFTBUTTON | TPM_VERTICAL, rc.left, rc.bottom, m_hWnd, &excludeArea);
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
    else if (item->command == ID_SEARCHBYIMAGE) {
        CMenu rectangleMenu;
        RECT rc = item->rect;
        horizontalToolbar_.ClientToScreen(&rc);
        rectangleMenu.CreatePopupMenu();
        CString itemText;
        itemText.Format(TR("Search by image (%s)"), _T("Google"));
        rectangleMenu.AppendMenu(MF_STRING, ID_SEARCHBYIMAGEINGOOGLE, itemText);
        itemText.Format(TR("Search by image (%s)"), _T("Yandex"));
        rectangleMenu.AppendMenu(MF_STRING, ID_SEARCHBYIMAGEINYANDEX, itemText);
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

void ImageEditorWindow::createToolbars()
{
    CRect rc;
    GetClientRect(&rc);
    if ( !horizontalToolbar_.Create(m_hWnd, !allowAltTab_, displayMode_ == wdmWindowed) ) {
        LOG(ERROR) << "Failed to create horizontal toolbar";
        return;
    }
    int dpiX;
    {
        CWindowDC dc(m_hWnd);
        dpiX = dc.GetDeviceCaps(LOGPIXELSX);
    }
    
    if (displayMode_ ==  wdmFullscreen && rc.Width() < MulDiv(800, dpiX, USER_DEFAULT_SCREEN_DPI)) {
        horizontalToolbar_.setShowButtonText(false);
    }
    if ( showAddToWizardButton_ ) {
        CString buttonHint= CString(TR("Add to the list")) + (showUploadButton_ ? _T("") : _T(" (Enter)"));
        horizontalToolbar_.addButton(Toolbar::Item(CString(TR("Add to the list")), loadToolbarIcon(IDB_ICONADDPNG),ID_ADDTOWIZARD, buttonHint));
    }
    if ( showUploadButton_ ) {
        CString fullUploadButtonText, uploadButtonText;
        if ( serverName_.IsEmpty() ) {
            fullUploadButtonText = uploadButtonText = TR("Upload to Web");
        } else {
            fullUploadButtonText.Format(TR("Upload to %s"), static_cast<LPCTSTR>(serverName_));
            uploadButtonText = WinUtils::TrimStringEnd(fullUploadButtonText, 35);
        }
        horizontalToolbar_.addButton(Toolbar::Item(uploadButtonText, loadToolbarIcon(IDB_ICONUPLOADPNG), ID_UPLOAD, fullUploadButtonText + _T(" (Enter)"), Toolbar::itButton));
    }
    //horizontalToolbar_.addButton(Toolbar::Item(TR("Share"),0,ID_SHARE, CString(),Toolbar::itComboButton));
    horizontalToolbar_.addButton(Toolbar::Item(TR("Save"),loadToolbarIcon(IDB_ICONSAVEPNG), ID_SAVE, TR("Save") + CString(_T(" (Ctrl+S)")),sourceFileName_.IsEmpty() ? Toolbar::itButton : Toolbar::itComboButton));
    std::wstring copyButtonHint = str(boost::wformat(TR("Copy to clipboard and close (%s)")) % L"Ctrl+C");
    horizontalToolbar_.addButton(Toolbar::Item(TR("Copy"), loadToolbarIcon(IDB_ICONCLIPBOARDPNG), ID_COPYBITMAPTOCLIBOARD, copyButtonHint.c_str(), Toolbar::itComboButton));
    
    CString itemText;
    CString searchEngineName = U2W(SearchByImage::getSearchEngineDisplayName(searchEngine_));
    itemText.Format(TR("Search on %s"), searchEngineName.GetString());

    horizontalToolbar_.addButton(Toolbar::Item(itemText, loadToolbarIcon(IDB_ICONSEARCH), ID_SEARCHBYIMAGE, itemText + CString(_T(" (Ctrl+F)")), Toolbar::itComboButton));
    
    horizontalToolbar_.addButton(Toolbar::Item(TR("Print..."), loadToolbarIcon(IDB_ICONPRINT), ID_PRINTIMAGE, TR("Print...") + CString(_T(" (Ctrl+P)")), Toolbar::itButton));
    horizontalToolbar_.addButton(Toolbar::Item(TR("Close"),std::shared_ptr<Gdiplus::Bitmap> () ,ID_CLOSE, TR("Close") + CString(_T(" (Esc)"))));
    horizontalToolbar_.AutoSize();
    if ( displayMode_ != wdmFullscreen ) {
        horizontalToolbar_.ShowWindow(SW_SHOW);
    }

    if ( !verticalToolbar_.Create(m_hWnd, !allowAltTab_, displayMode_ == wdmWindowed) ) {
        LOG(ERROR) << "Failed to create vertical toolbar";

    }
    /* enum DrawingToolHotkey {kMoveKey = 'V', kBrushKey = 'B', kTextKey = 'T', kRectangleKey = 'U', kColorPickerKey = 'I', kCropKey = 'C', // photoshop keys
    kMarkerKey = 'H', kBlurringRectangleKey = 'R', kArrowKey = 'A', kLineKey = 'L'
    };
    */ 
    verticalToolbar_.addButton(Toolbar::Item(CString(),  loadToolbarIcon(IDB_TOOLMOVEICONPNG),ID_MOVE,TR("Move") + CString(_T(" (")) + (char)kMoveKey  + CString(_T(")")), Toolbar::itButton, true, 1));
    //verticalToolbar_.addButton(Toolbar::Item(CString(),  loadToolbarIcon(IDB_ICONTOOLSELECTION),ID_SELECTION,TR("Selection"), Toolbar::itButton, true, 1));

    verticalToolbar_.addButton(Toolbar::Item(CString(),  loadToolbarIcon(IDB_TOOLCROPPING), ID_CROP,TR("Crop")+ CString(_T(" (")) + (char)kCropKey  + CString(_T(")")), Toolbar::itButton, true,1));
    verticalToolbar_.addButton(Toolbar::Item(CString(),  loadToolbarIcon(IDB_ICONTOOLBRUSHPNG), ID_BRUSH,TR("Brush")+ CString(_T(" (")) + (char)kBrushKey  + CString(_T(")")), Toolbar::itButton, true,1));
    verticalToolbar_.addButton(Toolbar::Item(CString(),  loadToolbarIcon(IDB_ICONTOOLMARKER), ID_MARKER,TR("Highlight")+ CString(_T(" (")) + (char)kMarkerKey  + CString(_T(")")), Toolbar::itButton, true,1));
    verticalToolbar_.addButton(Toolbar::Item(CString(),  loadToolbarIcon(IDB_ICONTOOLLINE), ID_LINE,TR("Line")+ CString(_T(" (")) + (char)kLineKey  + CString(_T(")")), Toolbar::itButton, true,1));
    verticalToolbar_.addButton(Toolbar::Item(CString(),  loadToolbarIcon(IDB_ICONTOOLARROWPNG), ID_ARROW,TR("Arrow")+ CString(_T(" (")) + (char)kArrowKey  + CString(_T(")")), Toolbar::itButton, true,1));

    verticalToolbar_.addButton(Toolbar::Item(CString(),  loadToolbarIcon(IDB_ICONTOOLRECTANGLEPNG), ID_RECTANGLE,TR("Rectangle")+ CString(_T(" (")) + (char)kRectangleKey  + CString(_T(")")), Toolbar::itTinyCombo, true,1));
    verticalToolbar_.addButton(Toolbar::Item(CString(),  loadToolbarIcon(IDB_ICONTOOLFILLEDRECTANGLE), ID_FILLEDRECTANGLE,TR("Filled rectangle")+ CString(_T(" (")) + (char)kFilledRectangle  + CString(_T(")")), Toolbar::itTinyCombo, true,1));
    verticalToolbar_.addButton(Toolbar::Item(CString(),  loadToolbarIcon(IDB_ICONTOOLTEXTPNG), ID_TEXT,TR("Text")+ CString(_T(" (")) + (char)kTextKey  + CString(_T(")")), Toolbar::itButton, true,1));
    verticalToolbar_.addButton(Toolbar::Item(CString(),  loadToolbarIcon(IDB_ICONTOOLSTEP), ID_STEPNUMBER,TR("Step")+ CString(_T(" (")) + (char)kStepNumber  + CString(_T(")")), Toolbar::itButton, true,1));

    verticalToolbar_.addButton(Toolbar::Item(CString(),  loadToolbarIcon(IDB_ICONTOOLBLURINGRECTANGLEPNG), ID_BLURRINGRECTANGLE,TR("Blurring rectangle")+ CString(_T(" (")) + (char)kBlurringRectangleKey  + CString(_T(")")), Toolbar::itTinyCombo, true,1));

    //verticalToolbar_.addButton(Toolbar::Item(CString(),  loadToolbarIcon(IDB_ICONTOOLBRUSHPNG), ID_BLUR,TR("Blur"), Toolbar::itButton, true,1));
    verticalToolbar_.addButton(Toolbar::Item(CString(),  loadToolbarIcon(IDB_ICONCOLORPICKERPNG), ID_COLORPICKER,TR("Color chooser")+ CString(_T(" (")) + (char)kColorPickerKey  + CString(_T(")")), Toolbar::itButton, true,1));
    int index = verticalToolbar_.addButton(Toolbar::Item(CString(),  loadToolbarIcon(IDB_ICONUNDOPNG), ID_UNDO,TR("Undo") + CString(L" (Ctrl+Z)"), Toolbar::itButton, false));

    Toolbar::Item colorsButton(CString(),  loadToolbarIcon(IDB_ICONUNDOPNG), ID_UNDO,CString(), Toolbar::itButton, false);
    colorsDelegate_ = std::make_unique<ColorsDelegate>(&verticalToolbar_, index+1, canvas_.get());
    colorsButton.itemDelegate = colorsDelegate_.get();
    colorsDelegate_->setBackgroundColor(canvas_->getBackgroundColor());
    colorsDelegate_->setForegroundColor(canvas_->getForegroundColor());
    verticalToolbar_.addButton(colorsButton);

    verticalToolbar_.AutoSize();
    if ( displayMode_ != wdmFullscreen ) {
        verticalToolbar_.ShowWindow(SW_SHOW);
    }
    horizontalToolbar_.penSizeSlider_.SetPos(canvas_->getPenSize());
    horizontalToolbar_.roundRadiusSlider_.SetPos(canvas_->getRoundingRadius());
    horizontalToolbar_.setStepFontSize(canvas_->getStepFontSize());
    horizontalToolbar_.setStepInitialValue(1);
    horizontalToolbar_.setArrowType(static_cast<int>(canvas_->getArrowMode()));
    horizontalToolbar_.setFillBackgroundCheckbox(canvas_->getFillTextBackground());
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

        horToolbarPos.x = std::min(std::max<LONG>( horToolbarPos.x, 0),clientRect.right - vertRc.right - horRc.right);
        horToolbarPos.y = std::min( std::max( horToolbarPos.y, vertRc.bottom + kToolbarOffset ) , clientRect.bottom - horRc.bottom - kToolbarOffset);
        vertToolbarPos.x = std::min(std::max( vertToolbarPos.x, horRc.right + kToolbarOffset ), clientRect.right - vertRc.right);
        vertToolbarPos.y = std::min(std::max<LONG>( vertToolbarPos.y, 0), clientRect.bottom - horRc.bottom - kToolbarOffset - vertRc.bottom);

    } else if ( pos == pTopLeft ) {
        horToolbarPos.x = x;
        horToolbarPos.y =  y - horRc.bottom - kToolbarOffset;

        vertToolbarPos.x = x - vertRc.right - kToolbarOffset;
        vertToolbarPos.y = y ;

        horToolbarPos.x = std::min(std::max(horToolbarPos.x, vertRc.right), clientRect.right - horRc.right - kToolbarOffset);
        horToolbarPos.y = std::min(std::max<LONG>(horToolbarPos.y, 0), clientRect.bottom - vertRc.bottom - horRc.bottom - kToolbarOffset);
        vertToolbarPos.x = std::min(std::max<LONG>(vertToolbarPos.x, 0), clientRect.right - horRc.right - vertRc.right - kToolbarOffset);
        vertToolbarPos.y = std::min(std::max(vertToolbarPos.y, horRc.bottom), clientRect.bottom - vertRc.bottom - kToolbarOffset);
    } else if ( pos == pTopRight ) {
        horToolbarPos.x = x + w - horRc.right - kToolbarOffset;
        horToolbarPos.y =  y - horRc.bottom - kToolbarOffset;

        vertToolbarPos.x = x + w + kToolbarOffset;
        vertToolbarPos.y = y ;

        horToolbarPos.x = /*min*(*/std::max<LONG>(horToolbarPos.x, 0)/*, clientRect.right - horRc.right - kToolbarOffset)*/;
        horToolbarPos.y = std::min(std::max<LONG>(horToolbarPos.y, 0), clientRect.bottom - vertRc.bottom - horRc.bottom - kToolbarOffset);
        vertToolbarPos.x = /*min(*/std::max(vertToolbarPos.x, horRc.right + kToolbarOffset)/*, clientRect.right - horRc.right- vertRc.right - kToolbarOffset)*/;
        vertToolbarPos.y = std::min(std::max(vertToolbarPos.y, horRc.bottom), clientRect.bottom - vertRc.bottom - kToolbarOffset);
    } else if ( pos == pBottomLeft ) {
        horToolbarPos.x = x;
        horToolbarPos.y =  y + h + kToolbarOffset;

        vertToolbarPos.x = x - vertRc.right - kToolbarOffset;
        vertToolbarPos.y = y + h - vertRc.bottom;

        horToolbarPos.x = std::min(std::max(horToolbarPos.x, vertRc.right), clientRect.right - horRc.right - kToolbarOffset);
    
        vertToolbarPos.x = std::min(std::max<LONG>(vertToolbarPos.x, 0), clientRect.right - horRc.right - vertRc.right - kToolbarOffset);
    
        //horToolbarPos.y = max( horToolbarPos.y, vertRc.bottom + kToolbarOffset );
        horToolbarPos.y = std::max(horToolbarPos.y, vertRc.bottom + kToolbarOffset);
        vertToolbarPos.y = std::max<LONG>(vertToolbarPos.y, 0);
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
    showApplyButtons();

    OnCropChanged(x,y,w,h);
    if (displayMode_ != wdmFullscreen) {
        return;
    }
    if ( !verticalToolbar_.IsWindowVisible() ) {
        verticalToolbar_.ShowWindow(SW_SHOW);
        horizontalToolbar_.ShowWindow(SW_SHOW);
    }
}

void ImageEditorWindow::OnDrawingToolChanged(DrawingToolType drawingTool)
{
    updateToolbarDrawingTool(drawingTool);
    SendMessage(WM_SETCURSOR,0,0);
}

void ImageEditorWindow::OnTextEditStarted(ImageEditor::TextElement * textElement)
{
    auto* control = dynamic_cast<InputBoxControl*>(textElement->getInputBox());
    if (!control) {
        return;
    }
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
    updateFontSizeControls();
}

void ImageEditorWindow::updateRoundingRadiusSlider()
{
    bool showLineWidth = ( currentDrawingTool_ != DrawingToolType::dtStepNumber 
        && currentDrawingTool_ != DrawingToolType::dtText 
        && currentDrawingTool_ != DrawingToolType::dtCrop
    );
    horizontalToolbar_.showPenSize(showLineWidth);

    bool showRoundingRadiusSlider = currentDrawingTool_ == DrawingToolType::dtRoundedRectangle || currentDrawingTool_ == DrawingToolType::dtFilledRoundedRectangle || canvas_->isRoundingRectangleSelected();
    horizontalToolbar_.roundRadiusLabel_.ShowWindow( showRoundingRadiusSlider ? SW_SHOW: SW_HIDE );
    horizontalToolbar_.roundRadiusSlider_.ShowWindow( showRoundingRadiusSlider ? SW_SHOW: SW_HIDE );

    bool showFillBackgound = currentDrawingTool_ == DrawingToolType::dtText;
    horizontalToolbar_.showFillBackgroundCheckbox(showFillBackgound);

    horizontalToolbar_.showArrowTypeCombo(currentDrawingTool_ == DrawingToolType::dtArrow);

    showApplyButtons();
}

void ImageEditorWindow::updateFontSizeControls() {
    bool showFontSizeControls = currentDrawingTool_ == DrawingToolType::dtStepNumber;
    horizontalToolbar_.showStepFontSize(showFontSizeControls);
}

void ImageEditorWindow::updateSearchButton() {
    int buttonIndex = horizontalToolbar_.getItemIndexByCommand(ID_SEARCHBYIMAGE);
    if (buttonIndex != -1) {
        Toolbar::Item* item = horizontalToolbar_.getItem(buttonIndex);
        CString searchEngineName = U2W(SearchByImage::getSearchEngineDisplayName(searchEngine_));
        item->title.Format(TR("Search on %s"), searchEngineName.GetString());
        horizontalToolbar_.AutoSize();
        //horizontalToolbar_.Invalidate(FALSE);
    }
}

std::shared_ptr<Gdiplus::Bitmap> ImageEditorWindow::loadToolbarIcon(int resource)
{
    return std::shared_ptr<Gdiplus::Bitmap>(ImageUtils::BitmapFromResource(GetModuleHandle(0), MAKEINTRESOURCE(resource),_T("PNG")) );
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
    DrawingToolType toolId =  static_cast<DrawingToolType>(menuItems_[wID]);
    canvas_->setDrawingToolType(toolId);
    updateToolbarDrawingTool(toolId);
    return 0;
}

LRESULT ImageEditorWindow::OnUndoClick(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
    canvas_->undo();
    return 0;
}

LRESULT ImageEditorWindow::OnTextClick(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
    canvas_->setDrawingToolType(DrawingToolType::dtText );
    updateToolbarDrawingTool(DrawingToolType::dtText);
    return 0;
}

LRESULT ImageEditorWindow::OnClickedClose(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& bHandled)
{
    onClose();
    return 0;
}

void ImageEditorWindow::onClose() {
    DialogResult dr = drCancel;
    if (askBeforeClose_ && canvas_->isDocumentModified()) {
        int msgBoxResult = GuiTools::LocalizedMessageBox(m_hWnd, TR("Save changes?"), APPNAME, MB_YESNOCANCEL | MB_ICONQUESTION);
        if (msgBoxResult == IDYES) {
            dr = outFileName_.IsEmpty() ? drCancel : drSave;
            if(!OnClickedSave()) {
                return;
            }
        }
        else if (msgBoxResult == IDCANCEL) {
            return;
        }
    }
    EndDialog(dr);
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
    const bool res = saveDocument(); // Save screenshot in default format
    if (res) {
        const std::vector<CString> files = { outFileName_ };
        WinUtils::DisplaySystemPrintDialogForImage(files, m_hWnd);
        /*if (displayMode_ == wdmFullscreen && !(GetKeyState(VK_SHIFT) & 0x8000)) {
            EndDialog(drPrintRequested);
        }*/
    }
    return res;
}

LRESULT ImageEditorWindow::OnSearchByImage(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled) {
    if (wID == ID_SEARCHBYIMAGEINGOOGLE) {
        searchEngine_ = SearchByImage::SearchEngine::seGoogle;
    } else if (wID == ID_SEARCHBYIMAGEINYANDEX) {
        searchEngine_ = SearchByImage::SearchEngine::seYandex;
    }
    const bool shiftPressed = (GetKeyState(VK_SHIFT) & 0x8000)!=0;
    updateSearchButton();
    const CString fileName = saveToTempFile();
    if (fileName.IsEmpty()) {
        LOG(ERROR) << "Unable to create temporary file";
        return 0;
    }
    auto uploadManager = ServiceLocator::instance()->uploadManager();
    CSearchByImageDlg dlg(uploadManager, searchEngine_, fileName);
    if (dlg.DoModal(m_hWnd) == IDOK) {
        if (displayMode_ == wdmFullscreen &&  !shiftPressed) {
            EndDialog(drSearch);
        }
    }
    return 0;
}

LRESULT ImageEditorWindow::OnFontSizeChanged(UINT, WPARAM, LPARAM, BOOL&) {
    int fontSize = horizontalToolbar_.getFontSize();
    if (!fontSize) {
        fontSize = Canvas::kDefaultStepFontSize;
    }
    canvas_->setStepFontSize(fontSize);
    return 0;
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


    TOOLINFO ti = {};
    CString title = TR("Select region");
    ti.cbSize   = sizeof(TOOLINFO);
    ti.uFlags   = TTF_SUBCLASS;
    ti.hwnd     = m_view.m_hWnd;
    ti.hinst    = _Module.GetModuleInstance();
    ti.lpszText = const_cast<LPWSTR>(title.GetString());
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

bool ImageEditorWindow::OnSaveAs()
{    
    IMyFileDialog::FileFilterArray filters = {
        { _T("PNG"), CString(_T("*.png")) },
        { _T("JPEG"), CString(_T("*.jpg;*.jpeg")) },
        { _T("WEBP"), CString(_T("*.webp")) },
        { TR("All files"), _T("*.*") }
    };

    auto dlg = MyFileDialogFactory::createFileDialog(m_hWnd, CString(), CString(), filters, false, false);
    dlg->setFileName(suggestedFileName_);
    CString ext = WinUtils::GetFileExt(suggestedFileName_);
    ext.MakeLower();
    dlg->setDefaultExtension(ext);
    int index = 1; // png
    if (ext == "jpg") {
        index = 2;
    } else if (ext == "webp") {
        index = 3;
    }
    dlg->setFileTypeIndex(index);
    enableToolbarsIfNecessary(false);
    if (dlg->DoModal(m_hWnd) != IDOK) {
        enableToolbarsIfNecessary(true);
        return false;
    }
    enableToolbarsIfNecessary(true);
    outFileName_ = dlg->getFile();
    saveDocument(ClipboardFormat::None, true);
    return true;
}

void ImageEditorWindow::saveSettings()
{
    if ( configurationProvider_ ) {
        configurationProvider_->setPenSize(canvas_->getPenSize());
        configurationProvider_->setForegroundColor(canvas_->getForegroundColor());
        configurationProvider_->setBackgroundColor(canvas_->getBackgroundColor());
        configurationProvider_->setFont(canvas_->getFont());
        configurationProvider_->setRoundingRadius(canvas_->getRoundingRadius());
        configurationProvider_->setSearchEngine(searchEngine_);
        configurationProvider_->setFillTextBackground(canvas_->getFillTextBackground());
        configurationProvider_->setStepForegroundColor(canvas_->getStepForegroundColor());
        configurationProvider_->setStepBackgroundColor(canvas_->getStepBackgroundColor());
        configurationProvider_->setArrowMode(static_cast<int>(canvas_->getArrowMode()));

        configurationProvider_->saveConfiguration();
    }
}

bool ImageEditorWindow::CopyBitmapToClipboardAndClose(ClipboardFormat format) {
    if (GetFocus() != m_view.m_hWnd) {
        ::SetFocus(m_view.m_hWnd);
    }
    const bool res = saveDocument(format);
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

bool ImageEditorWindow::OnClickedSave() {
    if (GetFocus() != m_view.m_hWnd) {
        ::SetFocus(m_view.m_hWnd);
    }
    if (!sourceFileName_.IsEmpty()) {
        //CString ext = WinUtils::GetFileExt(sourceFileName_);
        outFileName_ = sourceFileName_;
        saveDocument();
    } else {
        return OnSaveAs();
    }
    return true;
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

LRESULT ImageEditorWindow::OnStepInitialValueChange(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/) {
    int stepInitialValue = horizontalToolbar_.getStepInitialValue();
    if (!stepInitialValue) {
        stepInitialValue = 1;
    }
    canvas_->setStepInitialValue(stepInitialValue);
    return 0;
}


LRESULT ImageEditorWindow::OnFillBackgroundChange(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/) {
    canvas_->setFillTextBackground(horizontalToolbar_.isFillBackgroundChecked());
    return 0;
}

LRESULT ImageEditorWindow::OnArrowTypeChange(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/) {
    canvas_->setArrowMode(static_cast<Arrow::ArrowMode>(horizontalToolbar_.getArrowType()));
    return 0;
}

void ImageEditorWindow::enableToolbarsIfNecessary(bool enable) {
    if (displayMode_ == wdmFullscreen) {
        horizontalToolbar_.EnableWindow(enable);
        verticalToolbar_.EnableWindow(enable);
    }
}

LRESULT ImageEditorWindow::OnEnable(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& /*bHandled*/) {
    enableToolbarsIfNecessary(static_cast<BOOL>(wParam));
    return 0;
}

void ImageEditorWindow::updateWindowTitle() {
    std::wstring windowTitle;
    if (sourceFileName_.IsEmpty()) {
        windowTitle = str(boost::wformat(TR("Image Editor  (%1%x%2%)")) % currentDoc_->getWidth() % currentDoc_->getHeight());
    }
    else {
        CString fileNameStr = WinUtils::TrimString(WinUtils::myExtractFileName(sourceFileName_), 60);
        std::wstring fileNameWstring{ fileNameStr };

        windowTitle = str(boost::wformat(TR("Image Editor - %1% (%2%x%3%)")) % fileNameWstring % currentDoc_->getWidth() % currentDoc_->getHeight());
    }

	if (canvas_->isDocumentModified()) {
        windowTitle = L"* " + windowTitle;
	}
    if (windowTitle != windowTitle_) {
        windowTitle_ = windowTitle;
        SetWindowText(windowTitle.c_str());
    }
}

LRESULT ImageEditorWindow::OnApplyOperation(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/) {
    canvas_->applyCurrentOperation();
    showApplyButtons();
    return 0;
}

LRESULT ImageEditorWindow::OnCancelOperation(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/) {
    canvas_->cancelCurrentOperation();
    showApplyButtons();
    return 0;
}

void ImageEditorWindow::showApplyButtons() {
    horizontalToolbar_.showApplyButtons(currentDrawingTool_ == DrawingToolType::dtCrop && displayMode_ == wdmWindowed && canvas_->hasElementOfType(ElementType::etCrop));
}

void ImageEditorWindow::repositionToolbar(Toolbar& toolbar, const CRect& otherToolbarRect) {
    if (displayMode_ != wdmFullscreen) {
        return;
    }
    MONITORINFO mi = {};
    mi.cbSize = sizeof(mi);
    ::GetMonitorInfo(::MonitorFromWindow(m_hWnd, MONITOR_DEFAULTTONEAREST), &mi);
    CRect workArea = mi.rcWork;

    MonitorEnumerator enumerator;
    CRect toolbarRc;
    toolbar.GetClientRect(toolbarRc);
    toolbar.ClientToScreen(toolbarRc);

    bool needRepos = false;

    if (enumerator.enumDisplayMonitors(nullptr, nullptr)) {
        needRepos = true;
        for (const auto& mon : enumerator) {
            CRect r = mon.rect;
            CRect intersected;
            intersected.IntersectRect(r, toolbarRc);
            if (intersected == toolbarRc) {
                needRepos = false;
                break;
            }
        }
    }

    if (needRepos) {
        CPoint newPos;
        if (toolbar.orientation() == Toolbar::orHorizontal) {
            newPos = CPoint(workArea.left + otherToolbarRect.Width() + kCanvasMargin, workArea.top);
        } else {
            newPos = CPoint(workArea.left, workArea.top + otherToolbarRect.Height() + kCanvasMargin);
        }
        toolbar.SetWindowPos(nullptr, newPos.x, newPos.y, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
    }
}

}
