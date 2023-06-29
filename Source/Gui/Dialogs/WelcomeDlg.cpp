/*

    Image Uploader -  free application for uploading images/files to the Internet

    Copyright 2007-2018 Sergey Svistunov (zenden2k@gmail.com)

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

        http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.

*/
#include "WelcomeDlg.h"

#include "HistoryWindow.h"
#include "SettingsDlg.h"
#include "Func/MyUtils.h"
#ifdef IU_ENABLE_MEDIAINFO
#include "Func/MediaInfoHelper.h"
#endif
#include "WizardDlg.h"
#include "Core/Images/Utils.h"

// CWelcomeDlg
CWelcomeDlg::CWelcomeDlg()
{
    br = CreateSolidBrush(RGB(255, 255, 255));
    QuickRegionPrint = false;
}

LRESULT CWelcomeDlg::OnEraseBkg(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled)
{
    bHandled = true;
    return 1;
}
    
LRESULT CWelcomeDlg::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    DoDataExchange(FALSE);
    using namespace std::placeholders;
    CClientDC dc(m_hWnd);
    float dpiScaleX_ = dc.GetDeviceCaps(LOGPIXELSX) / 96.0f;
    float dpiScaleY_ = dc.GetDeviceCaps(LOGPIXELSY) / 96.0f;

    WizardDlg->addLastRegionAvailabilityChangeCallback(std::bind(&CWelcomeDlg::lastRegionAvailabilityChanged, this, _1));
    auto leftImage = createLeftImage();
    LeftImage.loadImage(0, leftImage.get(), 1, false, RGB(255,255,255), true);

    LogoImage.SetWindowPos(0, 0,0, roundf(dpiScaleX_ * 32), roundf(dpiScaleY_ * 32), SWP_NOMOVE | SWP_NOZORDER);
    LogoImage.loadImage(0, 0, IDR_ICONMAINNEW, false, RGB(255,255,255), true);

    TRC(IDC_SELECTOPTION, "Select action:");
    TRC(IDC_SOVET, "Advice:");
    TRC(IDC_SOVET2, "Just drag-n-drop your files on Image Uploader's window and it will process them.");
    TRC(IDC_WELCOMEMSG, "Welcome to pictures Publishing Wizard, that will help you to upload your images, photos, video frames on Internet!");
    SetDlgItemText(IDC_TITLE, APPNAME);

    const int iconWidth = GetSystemMetrics(SM_CXSMICON);
    const int iconHeight = GetSystemMetrics(SM_CYSMICON);
    const int iconBigWidth = GetSystemMetrics(SM_CXICON);
    const int iconBigHeight = GetSystemMetrics(SM_CYICON);

    auto loadSmallIcon = [&](int resourceId) -> HICON {
        CIconHandle icon;
        icon.LoadIconWithScaleDown(MAKEINTRESOURCE(resourceId), iconWidth, iconHeight);
        return icon.m_hIcon;
    };

    auto loadBigIcon = [&](int resourceId) -> HICON {
        CIconHandle icon;
        icon.LoadIconWithScaleDown(MAKEINTRESOURCE(resourceId), iconBigWidth, iconBigHeight);
        return icon.m_hIcon;
    };

    ListBox.Init();
    ListBox.AddString(TR("Add Images"), TR("JPEG, PNG, GIF, BMP or any other file"), IDC_ADDIMAGES, loadBigIcon(IDI_IMAGES));
    
    ListBox.AddString(TR("Add Files..."), 0, IDC_ADDFILES, loadSmallIcon(IDI_ICONADD));
    
    ListBox.AddString(TR("From Web"), 0, IDC_DOWNLOADIMAGES, loadSmallIcon(IDI_ICONWEB), true);

    ListBox.AddString(TR("Add Folder..."), 0, IDC_ADDFOLDER, loadSmallIcon(IDI_ICONADDFOLDER),true,0,true);

    ListBox.AddString(TR("From Clipboard"), 0, IDC_CLIPBOARD, loadSmallIcon(IDI_CLIPBOARD),true);
    
    ListBox.AddString(TR("Reupload"), 0, IDC_REUPLOADIMAGES, loadSmallIcon(IDI_ICONRELOAD), true, 0, true);
    ListBox.AddString(TR("Shorten a link"), 0, IDC_SHORTENURL, loadSmallIcon(IDI_ICONLINK), true, 0, false);

    ListBox.AddString(TR("Screen Capture"), TR("a pic of the whole screen or selected region"), IDC_SCREENSHOT, loadBigIcon(IDI_SCREENSHOT));
    ListBox.AddString(TR("Select Region..."), 0, IDC_REGIONPRINT, loadSmallIcon(IDI_ICONREGION));
    ListBox.AddString(TR("Last Region"), 0, IDC_LASTREGIONSCREENSHOT, loadSmallIcon(IDI_ICONLASTREGION));
    
    ListBox.AddString(TR("Import Video File"), TR("Extracting frames from video"), IDC_ADDVIDEO, loadBigIcon(IDI_GRAB));
#ifdef IU_ENABLE_MEDIAINFO
    if(MediaInfoHelper::IsMediaInfoAvailable())
        ListBox.AddString(TR("View Media File Information"), 0, IDC_MEDIAFILEINFO, loadSmallIcon(IDI_ICONINFO));
#endif
    ListBox.AddString(TR("Settings"), TR("a tool for advanced users"), IDC_SETTINGS, loadBigIcon(IDI_ICONSETTINGS));
    ListBox.AddString(TR("History"), nullptr, ID_VIEWHISTORY, loadSmallIcon(IDI_ICONHISTORY));
    
    HFONT font = GetFont();
    LOGFONT alf;
    PageWnd = m_hWnd;

    bool ok = ::GetObject(font, sizeof(LOGFONT), &alf) == sizeof(LOGFONT);

    if(ok)
    {
        alf.lfWeight = FW_BOLD;

        NewFont=CreateFontIndirect(&alf);

        SendDlgItemMessage(IDC_SELECTOPTION,WM_SETFONT,(WPARAM)(HFONT)NewFont,MAKELPARAM(false, 0));
        HDC dc = ::GetDC(nullptr);
        alf.lfHeight  =  - MulDiv(13, GetDeviceCaps(dc, LOGPIXELSY), 72);
        ::ReleaseDC(nullptr, dc);
        NewFont = CreateFontIndirect(&alf);
        SendDlgItemMessage(IDC_TITLE,WM_SETFONT,(WPARAM)(HFONT)NewFont,MAKELPARAM(false, 0));
    }

    ShowNext(false);
    ShowPrev(false);    

    AddClipboardFormatListener(m_hWnd);

    //ListBox.SetFocus();
    ShowWindow(SW_HIDE);
    clipboardUpdated();
    lastRegionAvailabilityChanged(WizardDlg->hasLastScreenshotRegion());

    return FALSE;  // Let the system set the focus
}

std::unique_ptr<Gdiplus::Bitmap> CWelcomeDlg::createLeftImage() {
    CClientDC dc(m_hWnd);
    int dpiX = dc.GetDeviceCaps(LOGPIXELSX);
    int dpiY = dc.GetDeviceCaps(LOGPIXELSY);

    CRect controlRect;
    LeftImage.GetClientRect(controlRect);

    using namespace Gdiplus;
    auto bm = std::make_unique<Bitmap>(controlRect.Width(), controlRect.Height(), PixelFormat32bppARGB);
    Graphics gr(bm.get());

    gr.SetInterpolationMode(InterpolationModeHighQualityBicubic);
    gr.SetPixelOffsetMode(PixelOffsetModeHalf);
    ImageAttributes attr;
    attr.SetWrapMode(WrapModeTileFlipXY);
    Rect bounds(0, 0, controlRect.Width(), controlRect.Height());
    LinearGradientBrush brush(bounds, Color(71, 124, 155), Color(104, 178, 112),
            LinearGradientModeVertical);

    gr.FillRectangle(&brush, bounds); 

    auto logo = ImageUtils::BitmapFromResource(_Module.GetResourceInstance(), MAKEINTRESOURCE(IDR_PNG2), _T("PNG"));

    if (logo) {
        int horMargin = MulDiv(5, dpiX, USER_DEFAULT_SCREEN_DPI);
        int topMargin = MulDiv(80, dpiY, USER_DEFAULT_SCREEN_DPI);
        int w = static_cast<int>(logo->GetWidth());
        int h = static_cast<int>(logo->GetHeight());
        Size sz = ImageUtils::ProportionalSize(Size(w, h), Size(controlRect.Width() - horMargin * 2, controlRect.Height()));
        
        Rect dst((controlRect.Width() - sz.Width) / 2, topMargin, sz.Width, sz.Height);
        gr.DrawImage(logo.get(), dst, 0, 0, w, h, UnitPixel, &attr);
    }
    return bm;
}

LRESULT CWelcomeDlg::OnBnClickedScreenshot(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
    WizardDlg->executeFunc(_T("screenshotdlg"));
    return 0;
}

LRESULT CWelcomeDlg::OnBnClickedAddvideo(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
    WizardDlg->executeFunc(_T("importvideo"));
    return 0;
}

LRESULT CWelcomeDlg::OnBnClickedAddimages(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
    WizardDlg->executeFunc(_T("addimages"));

    return 0;
}

LRESULT CWelcomeDlg::OnCtlColorMsgDlg(HDC hdc, HWND hwndChild)
{
    return reinterpret_cast<LRESULT>(static_cast<HBRUSH>(br)); // Returning brush solid filled with COLOR_WINDOW color
}

bool CWelcomeDlg::OnShow()
{
    ShowPrev(false);
    EnableNext();
    CMainDlg* mainPage = WizardDlg->getPage<CMainDlg>(CWizardDlg::wpMainPage);
    ShowNext(mainPage && mainPage->FileList.GetCount() > 0);
    EnableExit();
    
    return 0;
}

LRESULT CWelcomeDlg::OnBnClickedSettings(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
    WizardDlg->executeFunc(_T("settings"));
    return 0;
}

LRESULT CWelcomeDlg::OnBnClickedRegionPrint(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
    WizardDlg->executeFunc(_T("regionscreenshot"));    
    return 0;
}

LRESULT CWelcomeDlg::OnBnClickedLastRegionScreenshot(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
    WizardDlg->executeFunc(_T("lastregionscreenshot"));
    return 0;
}
    
LRESULT CWelcomeDlg::OnBnClickedMediaInfo(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
    WizardDlg->executeFunc(_T("mediainfo"));
    return 0;
}
    

void CWelcomeDlg::clipboardUpdated()
{
    // Checking if there is an bitmap in clipboard
    bool IsClipboard = WizardDlg->IsClipboardDataAvailable();

    HyperLinkControlItem* item = ListBox.getItemByCommand(IDC_CLIPBOARD);

    if (item->Visible != IsClipboard)
    {
        item->Visible = IsClipboard;
        ListBox.InvalidateRect(&item->ItemRect, false);
    }
    else item->Visible = IsClipboard;
}

void CWelcomeDlg::lastRegionAvailabilityChanged(bool available) {
    HyperLinkControlItem* item = ListBox.getItemByCommand(IDC_LASTREGIONSCREENSHOT);
    if (item->Visible != available)
    {
        item->Visible = available;
        ListBox.InvalidateRect(&item->ItemRect, false); 
    }
}

LRESULT CWelcomeDlg::OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    bHandled = FALSE;
    LeftImage.UnsubclassWindow();
    LogoImage.UnsubclassWindow();
    return 0;
}

LRESULT CWelcomeDlg::OnClipboardClick(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
    SendMessage(GetParent(), WM_COMMAND, MAKELONG(ID_PASTE,1), 0); // Sending "Ctrl+V" to parent window (WizardDlg)
    return 0;
}
    
LRESULT CWelcomeDlg::OnAddFolderClick(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
    WizardDlg->executeFunc(_T("addfolder"));
    return 0;
}

LRESULT CWelcomeDlg::OnBnClickedAddFiles(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
    WizardDlg->executeFunc(_T("addfiles"));
    return 0;
}

LRESULT CWelcomeDlg::OnBnClickedDownloadImages(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
    WizardDlg->executeFunc(_T("downloadimages"));
    return 0;
}

LRESULT CWelcomeDlg::OnViewHistoryClick(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
    CHistoryWindow dlg(WizardDlg);
    dlg.DoModal(m_hWnd);
    return 0;
}
    
LRESULT CWelcomeDlg::OnBnClickedReuploadImages(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
    WizardDlg->executeFunc(_T("reuploadimages"));
    return 0;
}

LRESULT CWelcomeDlg::OnBnClickedShortenUrl(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
    WizardDlg->executeFunc(_T("shortenurl"));
    return 0;
}

LRESULT CWelcomeDlg::OnClipboardUpdate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    clipboardUpdated();
    return 0;
}

void CWelcomeDlg::SetInitialFocus() {
    ListBox.SetFocus();
}
