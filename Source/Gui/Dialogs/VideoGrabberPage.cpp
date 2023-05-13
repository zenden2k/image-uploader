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

#include "VideoGrabberPage.h"

#include <boost/format.hpp>

#include "Core/CommonDefs.h"
#include "Video/VideoGrabber.h"
#include "Core/Images/GdiPlusImage.h"
#include "Gui/Dialogs/SettingsDlg.h"
#include "Func/MediaInfoHelper.h"
#include "Func/WinUtils.h"
#include "MediaInfoDlg.h"
#include "Core/Settings/WtlGuiSettings.h"
#include "Gui/GuiTools.h"
#include "Core/Utils/CryptoUtils.h"
#include "Core/Logging.h"
#include "Core/Utils/StringUtils.h"
#include "Core/Images/Utils.h"
#include "Core/ServiceLocator.h"
#include "Gui/Dialogs/WizardDlg.h"
#include "Gui/Components/MyFileDialog.h"
#include "Core/AppParams.h"
#include "Core/Video/VideoUtils.h"
#include "Func/ImageGenerator.h"
#include "Func/MyUtils.h"

CVideoGrabberPage::CVideoGrabberPage(UploadEngineManager* uploadEngineManager)
{
	Terminated = true;
    grabbedFramesCount = 0;
    originalGrabInfoLabelWidth_ = 0;
    uploadEngineManager_ = uploadEngineManager;
    ThumbsView.SetDeletePhysicalFiles(true);
    NumOfFrames = 0;
    CanceledByUser = false;
    MainDlg = nullptr;
}

bool CVideoGrabberPage::SetFileName(LPCTSTR FileName)
{
    fileName_ = FileName;
    fileEdit_.SetWindowText(fileName_);
    return false;
}

LRESULT CVideoGrabberPage::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    WtlGuiSettings& Settings = *ServiceLocator::instance()->settings<WtlGuiSettings>();
    PageWnd = m_hWnd;
    DoDataExchange(FALSE);

    frameCountUpDownCtrl_.SetRange(1, 100);

    SetDlgItemInt(IDC_NUMOFFRAMESEDIT, Settings.VideoSettings.NumOfFrames);

    TRC(IDC_EXTRACTFRAMES, "Extracting frames from video");
    TRC(IDC_SELECTVIDEO, "Browse...");

    TRC(IDC_PATHTOFILELABEL, "Filename:");
    TRC(IDCANCEL, "Stop");
    TRC(IDC_DEINTERLACE, "Deinterlace");
    TRC(IDC_FRAMELABEL, "Number of frames:");
    TRC(IDC_MULTIPLEFILES, "Multiple files");
    TRC(IDC_SAVEASONE, "Single file");
    TRC(IDC_SAVEAS, "Save as:");
    TRC(IDC_GRAB, "Grab");
    TRC(IDC_GRABBERPARAMS, "Settings...");
    TRC(IDC_FILEINFOBUTTON, "Information about file");
    openInFolderLink_.SetLabel(TR("Open containing folder"));
    openInFolderLink_.SubclassWindow(GetDlgItem(IDC_OPENFOLDER));
    openInFolderLink_.m_dwExtendedStyle |= HLINK_COMMANDBUTTON | HLINK_UNDERLINEHOVER; 
    openInFolderLink_.m_clrLink = WtlGuiSettings::DefaultLinkColor;

    videoEngineCombo_.AddString(WtlGuiSettings::VideoEngineAuto);
    videoEngineCombo_.AddString(WtlGuiSettings::VideoEngineDirectshow);
    videoEngineCombo_.AddString(WtlGuiSettings::VideoEngineDirectshow2);
    if (WtlGuiSettings::IsFFmpegAvailable()) {
        videoEngineCombo_.AddString(WtlGuiSettings::VideoEngineFFmpeg);
    }

    int itemIndex = videoEngineCombo_.FindStringExact(0, Settings.VideoSettings.Engine);
    if ( itemIndex == CB_ERR){
        itemIndex = 0;
    }
    
    videoEngineCombo_.SetCurSel(itemIndex);
    fileEdit_.SetWindowText(fileName_);

    bool check = true;
    SendDlgItemMessage(IDC_MULTIPLEFILES, BM_SETCHECK, check);
    SendDlgItemMessage(IDC_SAVEASONE, BM_SETCHECK, !check);

    ::ShowWindow(GetDlgItem(IDC_STOP), SW_HIDE);
    ::ShowWindow(GetDlgItem(IDC_PROGRESSBAR), SW_HIDE);
    SavingMethodChanged();
    ThumbsView.SubclassWindow(GetDlgItem(IDC_THUMBLIST));
    ThumbsView.Init();

    toolTipCtrl_ = GuiTools::CreateToolTipForWindow(GetDlgItem(IDC_VIDEOENGINECOMBO),
        TR("Video engine which is used to extract frames from a video file."));

    GuiTools::AddToolTip(toolTipCtrl_, GetDlgItem(IDC_DEINTERLACE), TR("Deinterlace video"));
    return 1;  // Let the system set the focus
}

LRESULT CVideoGrabberPage::OnClickedCancel(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
    if (!Terminated) {
    
        if(videoGrabber_) {
            videoGrabber_->abort();
        }

        
        CanceledByUser = true;  
    }

    return 0;
}

LRESULT CVideoGrabberPage::OnBnClickedGrab(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
    WtlGuiSettings* settings = ServiceLocator::instance()->settings<WtlGuiSettings>();
    CString fileName = GuiTools::GetDlgItemText(m_hWnd, IDC_FILEEDIT);
    if ( fileName.IsEmpty() ) {
        return 0;
    }

    if ( !WinUtils::FileExists(fileName ) ) {
        LOG(ERROR) << str(boost::wformat(TR("File doesn't exist:\n%s")) % fileName.GetString());
        return 0;
    }

    WizardDlg->setLastVideoFile(fileName);
    grabbedFramesCount = 0;
    Terminated = false;

    ::ShowWindow(GetDlgItem(IDC_FRAMELABEL), SW_HIDE);
    ::ShowWindow(GetDlgItem(IDC_DEINTERLACE), SW_HIDE);
    ::ShowWindow(GetDlgItem(IDC_NUMOFFRAMESEDIT), SW_HIDE);
    ::ShowWindow(GetDlgItem(IDC_UPDOWN), SW_HIDE);
    ::ShowWindow(GetDlgItem(IDC_VIDEOENGINECOMBO), SW_HIDE);

    ::ShowWindow(GetDlgItem(IDCANCEL), SW_SHOW);

    ::EnableWindow(GetDlgItem(IDC_GRAB), 0);
    ::EnableWindow(GetDlgItem(IDC_FILEEDIT), 0);
    ::EnableWindow(GetDlgItem(IDC_SELECTVIDEO), 0);
    ::ShowWindow(GetDlgItem(IDC_PROGRESSBAR), SW_SHOW);

    int videoEngineIndex = videoEngineCombo_.GetCurSel();
    CString buf;
    videoEngineCombo_.GetLBText(videoEngineIndex, buf);
    settings->VideoSettings.Engine = buf;

    SetNextCaption(TR("Next >"));
    EnableNext(false);
    EnablePrev(false);
    EnableExit(false);
    TRC(IDC_STOP, "Stop");

    NumOfFrames = GetDlgItemInt(IDC_NUMOFFRAMESEDIT);
    if (NumOfFrames < 1)
        NumOfFrames = 5;
    SendDlgItemMessage(IDC_PROGRESSBAR, PBM_SETPOS, 0);
    
    SendDlgItemMessage(IDC_PROGRESSBAR, PBM_SETRANGE, 0, MAKELPARAM(0, NumOfFrames*10));
    CanceledByUser = false;

    openInFolderLink_.ShowWindow(SW_HIDE);

    if ( originalGrabInfoLabelWidth_ ) {
        RECT grabInfoLabelRect;
        ::GetClientRect(GetDlgItem(IDC_GRABINFOLABEL), &grabInfoLabelRect);
        SetDlgItemText(IDC_GRABINFOLABEL, L"");
        ::SetWindowPos(GetDlgItem(IDC_GRABINFOLABEL), nullptr, 0, 0, originalGrabInfoLabelWidth_, grabInfoLabelRect.bottom, SWP_NOMOVE|SWP_NOZORDER);
        ::InvalidateRect(GetDlgItem(IDC_GRABINFOLABEL), 0, true);
    }

    snapshotsFolder.Empty();
    openInFolderLink_.SetToolTipText(_T(""));

    GrabBitmaps(fileName);

    return 0;
}

bool CVideoGrabberPage::OnAddImage(Gdiplus::Bitmap *bm, CString title)
{
    using namespace Gdiplus;
    auto* settings = ServiceLocator::instance()->settings<WtlGuiSettings>();
    CString fileNameBuffer;

    SetGrabbingStatusText(CString(TR("Extracting frame ")) + title);

    if (SendDlgItemMessage(IDC_DEINTERLACE, BM_GETCHECK) == BST_CHECKED)
    {
        int iwidth = bm->GetWidth();
        int iheight = bm->GetHeight();
        int halfheight = iheight / 2;
        Graphics g(m_hWnd, true);
        Bitmap BackBuffer (iwidth,  halfheight, &g);
        Graphics gr(&BackBuffer);
        gr.SetInterpolationMode(InterpolationModeHighQualityBicubic );
        gr.DrawImage(bm, 0, 0, iwidth, halfheight);
        Graphics gr2(bm);
        gr2.SetInterpolationMode(InterpolationModeHighQualityBicubic );
        gr2.DrawImage(&BackBuffer, 0, 0, iwidth, iheight);
    }

    CString videoFile = GuiTools::GetDlgItemText(m_hWnd, IDC_FILEEDIT);

    if ( snapshotsFolder.IsEmpty() ) {
        if ( !settings->VideoSettings.SnapshotsFolder.IsEmpty() ) {
            CString path = settings->VideoSettings.SnapshotsFolder + "\\" + settings->VideoSettings.SnapshotFileTemplate;
            CString snapshotsFolderTemplate = Utf8ToWCstring( IuCoreUtils::ExtractFilePath(WCstringToUtf8(path)) );
            snapshotsFolder = GenerateFileNameFromTemplate(snapshotsFolderTemplate, 1, CPoint(bm->GetWidth(),bm->GetHeight()), videoFile);
            std::string snapshotsFolderUtf8 = WCstringToUtf8(snapshotsFolder);

            if ( !IuCoreUtils::DirectoryExists(snapshotsFolderUtf8) ) {
                if ( !IuCoreUtils::CreateDir(snapshotsFolderUtf8) ) {
                    CString logMessage;
                    CString lastError = WinUtils::GetLastErrorAsString();
                    logMessage.Format(_T("Could not create folder '%s'.\r\n%s"), (LPCTSTR)snapshotsFolder, (LPCTSTR)lastError);
                    ServiceLocator::instance()->logger()->write(ILogger::logError, _T("Video Grabber"), logMessage, L"", videoFile);
                    snapshotsFolder = AppParams::instance()->tempDirectoryW();
                }
            }
        }
        if (snapshotsFolder.IsEmpty()) {
            snapshotsFolder = AppParams::instance()->tempDirectoryW();
        }
        ServiceLocator::instance()->taskRunner()->runInGuiThread([this] { openInFolderLink_.SetToolTipText(snapshotsFolder); });
    }

    CString wOutDir;
    if ( IuCoreUtils::DirectoryExists(WCstringToUtf8(snapshotsFolder)) ) {
        wOutDir = snapshotsFolder;
    }
    CString snapshotFileTemplate = Utf8ToWCstring( IuCoreUtils::ExtractFileNameNoExt(WCstringToUtf8(settings->VideoSettings.SnapshotFileTemplate)) );

    
    CString outFilename = GenerateFileNameFromTemplate(snapshotFileTemplate, grabbedFramesCount + 1,CPoint(bm->GetWidth(),bm->GetHeight()), videoFile);
    /*CString fullOutFileName = Settings.VideoSettings.SnapshotsFolder + "\\" + outFilename;
    std::string outDir = IuCoreUtils::ExtractFilePath(WCstringToUtf8(fullOutFileName));
    CString fileNameNoExt = Utf8ToWCstring(IuCoreUtils::ExtractFileNameNoExt(WCstringToUtf8(fullOutFileName)));*/

    try {
        if (ImageUtils::MySaveImage(bm, outFilename, fileNameBuffer, ImageUtils::sifPNG, 100, !wOutDir.IsEmpty() ? static_cast<LPCTSTR>(wOutDir) : NULL)) {
            ThumbsView.AddImage(fileNameBuffer, title, false, bm);
            grabbedFramesCount++;
        }
    } catch (const std::exception& ex) {
        LOG(ERROR) << "Failed to save image " << outFilename << std::endl << ex.what();
    }

    return true;
}

bool CVideoGrabberPage::SetGrabbingStatusText(LPCTSTR String)
{
    ErrorStr = String;
    SetDlgItemText(IDC_GRABINFOLABEL, String);
    return false;
}

int CVideoGrabberPage::ThreadTerminated()
{
    int left = GuiTools::GetWindowLeft(openInFolderLink_.m_hWnd);
    RECT grabInfoLabelRect;
    HWND grabInfoLabelHwnd = GetDlgItem(IDC_GRABINFOLABEL);
    ::GetClientRect(grabInfoLabelHwnd, &grabInfoLabelRect);
    ::SetWindowPos(grabInfoLabelHwnd, NULL, 0,0,left,grabInfoLabelRect.bottom, SWP_NOMOVE|SWP_NOZORDER);
    if ( !originalGrabInfoLabelWidth_ ) {
        originalGrabInfoLabelWidth_ = grabInfoLabelRect.right;
    }
    ::InvalidateRect(grabInfoLabelHwnd, 0, true);

    if (IuCoreUtils::DirectoryExists(W2U(snapshotsFolder))) {
        openInFolderLink_.ShowWindow(SW_SHOW);
    }

    Terminated = true;

    ::EnableWindow(GetDlgItem(IDC_GRAB), 1);

    ::ShowWindow(GetDlgItem(IDC_STOP), SW_HIDE);
    ::ShowWindow(GetDlgItem(IDC_PROGRESSBAR), SW_HIDE);
    if (CanceledByUser)
        SetGrabbingStatusText(TR("Extracting video frames was stopped by user."));

    ::ShowWindow(GetDlgItem(IDC_FRAMELABEL), SW_SHOW);
    ::ShowWindow(GetDlgItem(IDC_DEINTERLACE), SW_SHOW);
    ::ShowWindow(GetDlgItem(IDC_NUMOFFRAMESEDIT), SW_SHOW);
    ::ShowWindow(GetDlgItem(IDC_UPDOWN), SW_SHOW);
    ::ShowWindow(GetDlgItem(IDC_VIDEOENGINECOMBO), SW_SHOW);
    CheckEnableNext();
    ::EnableWindow(GetDlgItem(IDC_FILEEDIT), 1);
    ::EnableWindow(GetDlgItem(IDC_SELECTVIDEO), 1);

    EnablePrev();
    EnableExit();
    return 0;
}

LRESULT CVideoGrabberPage::OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
    return 0;
}

LRESULT CVideoGrabberPage::OnBnClickedGrabberparams(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
    CSettingsDlg dlg(CSettingsDlg::spVideo, uploadEngineManager_);
    dlg.DoModal(m_hWnd);
    return 0;
}

LRESULT CVideoGrabberPage::OnBnClickedMultiplefiles(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
    SavingMethodChanged();
    return 0;
}

void CVideoGrabberPage::SavingMethodChanged(void)
{
    BOOL check = SendDlgItemMessage(IDC_MULTIPLEFILES, BM_GETCHECK);
    ::EnableWindow(GetDlgItem(IDC_GRABBERPARAMS), !check);
}

int CVideoGrabberPage::GenPicture(CString& outFileName)
{
	CMainDlg* mainDlg = WizardDlg->getPage<CMainDlg>(CWizardDlg::wpMainPage);
	TCHAR buf[256] = _T("\0");
	int n = ThumbsView.GetItemCount();
	std::vector<ImageGeneratorTask::FileItem> items;
	for (int i = 0; i < n; i++) {
		buf[0] = _T('\0');
		CString fileName = ThumbsView.GetFileName(i);
		ThumbsView.GetItemText(i, 0, buf, 256);
		items.emplace_back(fileName, buf);
	}
	CString mediaFile = GuiTools::GetDlgItemText(m_hWnd, IDC_FILEEDIT);
	auto task = std::make_shared<ImageGeneratorTask>(
		m_hWnd, items, ThumbsView.maxwidth, ThumbsView.maxheight, mediaFile);
	
	task->onTaskFinished.connect([task, mainDlg](BackgroundTask*, BackgroundTaskResult taskResult) {
		if (taskResult == BackgroundTaskResult::Success && !task->outFileName().IsEmpty()) {
			mainDlg->AddToFileList(task->outFileName(), L"", true, nullptr, true);
		}

	});
	CStatusDlg dlg(task);
	if (dlg.DoModal(m_hWnd) == IDOK) {
		return 1;
	}

	return 0;
}

LRESULT CVideoGrabberPage::OnBnClickedBrowseButton(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
    auto* settings = ServiceLocator::instance()->settings<WtlGuiSettings>();
    IMyFileDialog::FileFilterArray filters = {
        { CString(TR("Video files")) + _T(" (avi, mpg, vob, wmv ...)"), PrepareVideoDialogFilters(), },
        { TR("All files"), _T("*.*") }
    };

    auto dlg = MyFileDialogFactory::createFileDialog(m_hWnd, settings->VideoFolder, TR("Choose video file"), filters, false);
    if (dlg->DoModal(m_hWnd) != IDOK) {
        return 0;
    }

    fileEdit_.SetWindowText(dlg->getFile());
    settings->VideoFolder = dlg->getFolderPath();

    return 0;
}

int CVideoGrabberPage::GrabBitmaps(const CString& szFile )
{
    auto* settings = ServiceLocator::instance()->settings<WtlGuiSettings>();
    CString videoEngine = settings->VideoSettings.Engine;

    if (videoEngine == WtlGuiSettings::VideoEngineAuto) {
        if ( !settings->IsFFmpegAvailable() ) {
            videoEngine = WtlGuiSettings::VideoEngineDirectshow;
        } else {
            videoEngine = WtlGuiSettings::VideoEngineFFmpeg;
            std::string ext = IuStringUtils::toLower( IuCoreUtils::ExtractFileExt( W2U(szFile) ) );
            if ( ext == "wmv" || ext == "asf" ) {
                videoEngine = WtlGuiSettings::VideoEngineDirectshow;
            }
        }
    }

    settings->VideoSettings.NumOfFrames = NumOfFrames;

    videoGrabber_ = std::make_unique<VideoGrabber>();
    videoGrabber_->setFrameCount(NumOfFrames);
    using namespace std::placeholders;
    videoGrabber_->setOnFrameGrabbed(std::bind(&CVideoGrabberPage::OnFrameGrabbed, this, _1, _2, _3));
    videoGrabber_->setOnFinished(std::bind(&CVideoGrabberPage::OnFrameGrabbingFinished, this, _1));
    VideoGrabber::VideoEngine engine = VideoGrabber::veAuto;
#ifdef IU_ENABLE_FFMPEG
    if (videoEngine == WtlGuiSettings::VideoEngineFFmpeg) {
        engine = VideoGrabber::veAvcodec;
    } else
#endif
    if (videoEngine == WtlGuiSettings::VideoEngineDirectshow) {
        engine = VideoGrabber::veDirectShow;
    } else if (videoEngine == WtlGuiSettings::VideoEngineDirectshow2) {
        engine = VideoGrabber::veDirectShow2;
    }

    videoGrabber_->setVideoEngine(engine);
    videoGrabber_->grab(W2U(szFile));
    return 0;
}

bool CVideoGrabberPage::OnShow()
{
    SetNextCaption(TR("Grab"));
    fileEdit_.SetWindowText(fileName_);
    ::ShowWindow(GetDlgItem(IDC_FILEINFOBUTTON), MediaInfoHelper::IsMediaInfoAvailable() ? SW_SHOW : SW_HIDE);
    SetGrabbingStatusText(_T(""));
    EnableNext(true);
    ShowPrev();
    ShowNext();
    EnablePrev();
    EnableExit();
    ThumbsView.MyDeleteAllItems();
    ::SetFocus(GetDlgItem(IDC_GRAB));
    return true;
}

void CVideoGrabberPage::OnFrameGrabbed(const std::string& timeStr, int64_t, std::shared_ptr<AbstractImage> img )
{
    auto image = std::dynamic_pointer_cast<GdiPlusImage>(img);
    if ( !image ) {
        LOG(ERROR) << "image is null";
        return;
    }
    Gdiplus::Bitmap *bm = image->getBitmap();
    if ( bm ) {
        SendDlgItemMessage(IDC_PROGRESSBAR, PBM_SETPOS, (grabbedFramesCount + 1) * 10 );
        OnAddImage(bm, Utf8ToWCstring(timeStr));
    }

}

void CVideoGrabberPage::OnFrameGrabbingFinished(bool success)
{
    ThreadTerminated();

    if (!CanceledByUser) {
        SetGrabbingStatusText(success ? TR("Extracting video frames was finished."): TR("An error occured while extracting video frames."));
    }
}

void CVideoGrabberPage::CheckEnableNext()
{
    EnableNext(ThumbsView.GetItemCount() > 1);
}

bool CVideoGrabberPage::OnNext()
{
    int n = ThumbsView.GetItemCount();
    if (n < 1)
    {
        SendDlgItemMessage(IDC_GRAB, BM_CLICK);
        return false;
    }
    
    WizardDlg->CreatePage(CWizardDlg::wpMainPage);
    CMainDlg* mainDlg = WizardDlg->getPage<CMainDlg>(CWizardDlg::wpMainPage);

    BOOL check = SendDlgItemMessage(IDC_MULTIPLEFILES, BM_GETCHECK);

    if (check) // If option "Multiple files" turn on
    {
        for (int i = 0; i < n; i++)
        {
            LPCTSTR filename = ThumbsView.GetFileName(i);
            if (!filename)
                continue;
            mainDlg->AddToFileList(filename);
        }
    }
    else
    {
        CString outFileName;
        if (!GenPicture(outFileName)) {
            return false;
        }
    }

    // Reset selection in the thumbs view
    mainDlg->ThumbsView.SelectItem(-1);

    ThumbsView.MyDeleteAllItems();
    auto* settings = ServiceLocator::instance()->settings<WtlGuiSettings>();
    settings->VideoSettings.NumOfFrames = GetDlgItemInt(IDC_NUMOFFRAMESEDIT);

    return true;
}

LRESULT CVideoGrabberPage::OnLvnItemDelete(int /*idCtrl*/, LPNMHDR pNMHDR, BOOL& /*bHandled*/)
{
    CheckEnableNext();
    return 0;
}

LRESULT CVideoGrabberPage::OnBnClickedFileinfobutton(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
    CMediaInfoDlg dlg;
    CString fileName;
    fileEdit_.GetWindowText(fileName);
    dlg.ShowInfo(m_hWnd, fileName);
    return 0;
}

CString CVideoGrabberPage::GenerateFileNameFromTemplate(const CString& templateStr, int index, const CPoint& size, const CString& originalName)
{
    std::mt19937 mt_{ std::random_device{}() };
    std::uniform_int_distribution<int> dist(0, 100);
	
    CString result = templateStr;
    time_t t = time(0);
    tm* timeinfo = localtime ( &t );
    CString indexStr;
    CString day, month, year;
    CString hours, seconds, minutes;
    std::string originalNameUtf8  = WCstringToUtf8(originalName);
    CString fileName = Utf8ToWCstring(IuCoreUtils::ExtractFileName(originalNameUtf8));
    CString fileNameNoExt = Utf8ToWCstring(IuCoreUtils::ExtractFileNameNoExt(originalNameUtf8));
    indexStr.Format(_T("%03d"), index);

    CString md5 = Utf8ToWstring(IuCoreUtils::CryptoUtils::CalcMD5HashFromString(WCstringToUtf8(WinUtils::IntToStr(GetTickCount() + dist(mt_))))).c_str();
    CString uid = md5.Mid(5,6);
    result.Replace(_T("%md5%"), md5);
    result.Replace(_T("%uid%"), uid);
    result.Replace(_T("%cx%"), WinUtils::IntToStr(size.x));
    result.Replace(_T("%cy%"), WinUtils::IntToStr(size.y));
    year.Format(_T("%04d"), (int)1900 + timeinfo->tm_year);
    month.Format(_T("%02d"), timeinfo->tm_mon + 1);
    day.Format(_T("%02d"), timeinfo->tm_mday);
    hours.Format(_T("%02d"), timeinfo->tm_hour);
    seconds.Format(_T("%02d"), timeinfo->tm_sec);
    minutes.Format(_T("%02d"), timeinfo->tm_min);
    result.Replace(_T("%y%"), year);
    result.Replace(_T("%m%"), month);
    result.Replace(_T("%d%"), day);
    result.Replace(_T("%h%"), hours);
    result.Replace(_T("%n%"), minutes);
    result.Replace(_T("%s%"), seconds);
    result.Replace(_T("%i%"), indexStr);
    result.Replace(_T("%fe%"),fileName);
    result.Replace(_T("%f%"), fileNameNoExt);
    return result;
}

LRESULT CVideoGrabberPage::OnOpenFolder(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
    WinUtils::ShellOpenFileOrUrl(snapshotsFolder, m_hWnd);
    return 0;
}