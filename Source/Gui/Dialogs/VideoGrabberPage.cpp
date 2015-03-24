/*
    Image Uploader - program for uploading images/files to Internet
    Copyright (C) 2007-2015 ZendeN <zenden2k@gmail.com>

    HomePage:    http://zenden.ws/imageuploader

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "VideoGrabberPage.h"


#include <Core/Video/VideoGrabber.h>
#include <Core/Video/GdiPlusImage.h>
#include "Gui/Controls/MyImage.h"
#include "Gui/Dialogs/SettingsDlg.h"
#include "Func/fileinfohelper.h"
#include "Core/ImageConverter.h"
#include "LogWindow.h"
#include "mediainfodlg.h"
#include "Func/Settings.h"
#include "Gui/GuiTools.h"
#include <Core/Utils/CryptoUtils.h>
#include <Core/Logging.h>
#include <Core/Utils/StringUtils.h>
#include <Func/IuCommonFunctions.h>
#include "atlheaders.h"

CVideoGrabberPage::CVideoGrabberPage()
{
	Terminated = true;
	grabbedFramesCount = 0;
	originalGrabInfoLabelWidth_ = 0;
	videoGrabber_ = 0;
}

CVideoGrabberPage::~CVideoGrabberPage()
{
	*m_szFileName = 0;
	delete videoGrabber_;
}

//  ��������� ��� ����� �� �������� ����
//
bool CVideoGrabberPage::SetFileName(LPCTSTR FileName)
{
	lstrcpy(m_szFileName, FileName);
	// ������� � ��������� ���� ��� �����, ���������� �� �������� ����
	SetDlgItemText(IDC_FILEEDIT, m_szFileName);
	return false;
}

LRESULT CVideoGrabberPage::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	PageWnd = m_hWnd;

	// ��������� ���������� UpDown ���������
	SendDlgItemMessage(IDC_UPDOWN, UDM_SETRANGE, 0, (LPARAM) MAKELONG((short)100, (short)1) );
	SendDlgItemMessage(IDC_QUALITYSPIN, UDM_SETRANGE, 0, (LPARAM) MAKELONG((short)100, (short)1) );

	SetDlgItemInt(IDC_NUMOFFRAMESEDIT, Settings.VideoSettings.NumOfFrames);
	SetDlgItemInt(IDC_QUALITY, Settings.VideoSettings.JPEGQuality);

	TRC(IDC_EXTRACTFRAMES, "���������� ����� ������");
	TRC(IDC_SELECTVIDEO, "�����...");

	TRC(IDC_PATHTOFILELABEL, "��� �����:");
	TRC(IDCANCEL, "����������");
	TRC(IDC_DEINTERLACE, "��������������");
	TRC(IDC_FRAMELABEL, "���-�� ������:");
	TRC(IDC_MULTIPLEFILES, "��������� ������");
	TRC(IDC_SAVEASONE, "���� ����");
	TRC(IDC_SAVEAS, "��������� ���:");
	TRC(IDC_GRAB, "�������");
	TRC(IDC_QUALITYLABEL, "��������:");
	TRC(IDC_GRABBERPARAMS, "���������...");
	TRC(IDC_FILEINFOBUTTON, "���������� � �����");
	openInFolderLink_.SetLabel(TR("������� ����� � �������������"));
	openInFolderLink_.SubclassWindow(GetDlgItem(IDC_OPENFOLDER));
	openInFolderLink_.m_dwExtendedStyle |= HLINK_COMMANDBUTTON | HLINK_UNDERLINEHOVER; 
	openInFolderLink_.m_clrLink = CSettings::DefaultLinkColor;

	

	GuiTools::AddComboBoxItems(m_hWnd, IDC_VIDEOENGINECOMBO, 3, CSettings::VideoEngineAuto, CSettings::VideoEngineDirectshow,CSettings::VideoEngineFFmpeg);
	int itemIndex = SendDlgItemMessage( IDC_VIDEOENGINECOMBO, CB_FINDSTRING, 0, (LPARAM)(LPCTSTR) Settings.VideoSettings.Engine );
	if ( itemIndex == CB_ERR){
		itemIndex = 0;
	}
	if ( !CSettings::IsFFmpegAvailable() ){
		::EnableWindow( GetDlgItem(IDC_VIDEOENGINECOMBO), false);
	}
	SendDlgItemMessage(IDC_VIDEOENGINECOMBO, CB_SETCURSEL, itemIndex );
	// ������� � ��������� ���� ��� �����, ���������� �� �������� ����
	SetDlgItemText(IDC_FILEEDIT, m_szFileName);

	bool check = true;
	// ��������� ������ ����������
	SendDlgItemMessage(IDC_MULTIPLEFILES, BM_SETCHECK, check);
	SendDlgItemMessage(IDC_SAVEASONE, BM_SETCHECK, !check);

	::ShowWindow(GetDlgItem(IDC_STOP), SW_HIDE);
	::ShowWindow(GetDlgItem(IDC_PROGRESSBAR), SW_HIDE);
	SavingMethodChanged();
	ThumbsView.SubclassWindow(GetDlgItem(IDC_THUMBLIST));
	ThumbsView.Init();

	return 1;  // Let the system set the focus
}

LRESULT CVideoGrabberPage::OnClickedCancel(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	if (!Terminated) {
	
		if(videoGrabber_) {
			videoGrabber_->abort();
		}
		//SignalStop();           //  �������� ������ ��������� ������ ��������
		if (!IsStopTimer)
		{
			TimerInc = 8;           // ���� 8 ������, ������ ��� ������� �����
			SetTimer(1, 1000, NULL);
			IsStopTimer = true;
		}
		else
		{
			CanceledByUser = true;
			//Terminate();      // ������� �����
			//ThreadTerminated();
		}
	}

	return 0;
}

LRESULT CVideoGrabberPage::OnBnClickedGrab(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	WizardDlg->LastVideoFile = GuiTools::GetWindowText(GetDlgItem(IDC_FILEEDIT));
	grabbedFramesCount = 0;
	Terminated = false;
	IsStopTimer = false;

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

	int videoEngineIndex = SendDlgItemMessage(IDC_VIDEOENGINECOMBO, CB_GETCURSEL );
	TCHAR buf[256];
	SendDlgItemMessage(IDC_VIDEOENGINECOMBO, CB_GETLBTEXT, videoEngineIndex, (LPARAM)buf );
	Settings.VideoSettings.Engine = buf;
	/*if (  videoEngineIndex == 0) {
	Settings.VideoSettings.Engine = CSettings::VideoEngineFFmpeg;
	}else if (  videoEngineIndex == 1) {
		Settings.VideoSettings.Engine = CSettings::VideoEngineFFmpeg;
	}else if (  videoEngineIndex == 2) {
		Settings.VideoSettings.Engine = CSettings::VideoEngineFFmpeg;
	}*/
	//videoEngineIndex ? CSettings::VideoEngineFFmpeg : CSettings::VideoEngineDirectshow;

	SetNextCaption(TR("����� >"));
	EnableNext(false);
	EnablePrev(false);
	EnableExit(false);
	TRC(IDC_STOP, "����������");

	NumOfFrames = GetDlgItemInt(IDC_NUMOFFRAMESEDIT);
	if (!NumOfFrames)
		NumOfFrames = 5;
	SendDlgItemMessage(IDC_PROGRESSBAR, PBM_SETPOS, 0);
	
	SendDlgItemMessage(IDC_PROGRESSBAR, PBM_SETRANGE, 0, MAKELPARAM(0, NumOfFrames*10));
	CanceledByUser = false;

	
	openInFolderLink_.ShowWindow(SW_HIDE);

	if ( originalGrabInfoLabelWidth_ ) {
		RECT grabInfoLabelRect;
		::GetClientRect(GetDlgItem(IDC_GRABINFOLABEL), &grabInfoLabelRect);
		SetDlgItemText(IDC_GRABINFOLABEL, L"");
		::SetWindowPos(GetDlgItem(IDC_GRABINFOLABEL), NULL, 0,0,originalGrabInfoLabelWidth_,grabInfoLabelRect.bottom, SWP_NOMOVE);
		::InvalidateRect(GetDlgItem(IDC_GRABINFOLABEL), 0, true);
	}
	Run();

	return 0;
}

DWORD CVideoGrabberPage::Run()
{
	snapshotsFolder.Empty();

	CString fileName = GuiTools::GetDlgItemText(m_hWnd, IDC_FILEEDIT);
	if ( fileName.IsEmpty() ) {
		return 0;
	}

	GrabBitmaps((LPCTSTR)fileName);

	return 0;
}

bool CVideoGrabberPage::OnAddImage(Gdiplus::Bitmap *bm, CString title)
{
	using namespace Gdiplus;
	CString fileNameBuffer;

	GrabInfo(CString(TR("�������� ���� ")) + title);


	if (SendDlgItemMessage(IDC_DEINTERLACE, BM_GETCHECK) == BST_CHECKED)
	{
		// Genial deinterlace realization ;)
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
		CString snapshotsFolderTemplate;
		if ( !Settings.VideoSettings.SnapshotsFolder.IsEmpty() ) {
			CString path = Settings.VideoSettings.SnapshotsFolder + "\\" + Settings.VideoSettings.SnapshotFileTemplate;
			snapshotsFolderTemplate = Utf8ToWCstring( IuCoreUtils::ExtractFilePath(WCstringToUtf8(path)) );
			snapshotsFolder = GenerateFileNameFromTemplate(snapshotsFolderTemplate, 1, CPoint(bm->GetWidth(),bm->GetHeight()), videoFile);
			std::string snapshotsFolderUtf8 = WCstringToUtf8(snapshotsFolder);

			if ( !IuCoreUtils::DirectoryExists(snapshotsFolderUtf8) ) {
				if ( !IuCoreUtils::createDirectory(snapshotsFolderUtf8) ) {
					CString logMessage;
					logMessage.Format(_T("Could not create folder '%s'."), (LPCTSTR)snapshotsFolder);
					WriteLog(logError, _T("Video Grabber"), logMessage);
					snapshotsFolder = IuCommonFunctions::IUTempFolder;

				}
			}
		}
	}
	CString wOutDir;
	if ( IuCoreUtils::DirectoryExists(WCstringToUtf8(snapshotsFolder)) ) {
		wOutDir = snapshotsFolder;
	}
	CString snapshotFileTemplate =  Utf8ToWCstring( IuCoreUtils::ExtractFileNameNoExt(WCstringToUtf8(Settings.VideoSettings.SnapshotFileTemplate)) );

	
	CString outFilename = GenerateFileNameFromTemplate(snapshotFileTemplate, grabbedFramesCount + 1,CPoint(bm->GetWidth(),bm->GetHeight()), videoFile);
	/*CString fullOutFileName = Settings.VideoSettings.SnapshotsFolder + "\\" + outFilename;
	std::string outDir = IuCoreUtils::ExtractFilePath(WCstringToUtf8(fullOutFileName));
	CString fileNameNoExt = Utf8ToWCstring(IuCoreUtils::ExtractFileNameNoExt(WCstringToUtf8(fullOutFileName)));*/
	
	MySaveImage(bm, outFilename, fileNameBuffer, 1, 100, !wOutDir.IsEmpty() ? (LPCTSTR)wOutDir : NULL);
	ThumbsView.AddImage(fileNameBuffer, title, bm);
	grabbedFramesCount++;
	return true;
}

bool CVideoGrabberPage::GrabInfo(LPCTSTR String)
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
	::SetWindowPos(grabInfoLabelHwnd, NULL, 0,0,left,grabInfoLabelRect.bottom, SWP_NOMOVE);
	if ( !originalGrabInfoLabelWidth_ ) {
		originalGrabInfoLabelWidth_ = grabInfoLabelRect.right;
	}
	::InvalidateRect(grabInfoLabelHwnd, 0, true);

	openInFolderLink_.ShowWindow(SW_SHOW);
	Terminated = true;
	KillTimer(1);
	IsStopTimer = false;

	::EnableWindow(GetDlgItem(IDC_GRAB), 1);

	::ShowWindow(GetDlgItem(IDC_STOP), SW_HIDE);
	::ShowWindow(GetDlgItem(IDC_PROGRESSBAR), SW_HIDE);
	if (CanceledByUser)
		GrabInfo(TR("���������� ������ ���� ����������� �������������."));

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

LRESULT CVideoGrabberPage::OnTimer(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	TimerInc--;
	TCHAR szBuffer[256];
	if (TimerInc > 0)
	{
		wsprintf(szBuffer, CString(TR("����������")) + _T(" (%d)"), TimerInc);
		SetDlgItemText(IDC_STOP, szBuffer);
	}
	else
	{
		//Terminate();
		ThreadTerminated();
	}
	return 0;
}

LRESULT CVideoGrabberPage::OnBnClickedGrabberparams(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	CSettingsDlg dlg(CSettingsDlg::spVideo);
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
	using namespace Gdiplus;
	RectF TextRect;
	int infoHeight = 0;
	CString Report;

	if (Settings.VideoSettings.ShowMediaInfo)
	{
		TCHAR buffer[256];
		GetDlgItemText(IDC_FILEEDIT, buffer, 256);
		bool bMediaInfoResult = GetMediaFileInfo(buffer, Report);

		Graphics g1(m_hWnd);

		CString s;

		Font font(::GetDC(0), &Settings.VideoSettings.Font);

		FontFamily ff;
		font.GetFamily(&ff);
		g1.SetPageUnit(UnitPixel);
		g1.MeasureString(Report, -1, &font, PointF(0, 0), &TextRect);
		infoHeight = int(TextRect.Height);
	}

	int n = ThumbsView.GetItemCount();
	int ncols = min(Settings.VideoSettings.Columns, n);
	int nstrings = n / ncols + ((n % ncols) ? 1 : 0);
	int maxwidth = ThumbsView.maxwidth;
	int maxheight = ThumbsView.maxheight;
	int gapwidth = Settings.VideoSettings.GapWidth;
	int gapheight = Settings.VideoSettings.GapHeight;
	infoHeight += gapheight;
	int tilewidth = Settings.VideoSettings.TileWidth;
	int tileheight = int(((float)tilewidth) / ((float)maxwidth) * ((float)maxheight));
	int needwidth = gapwidth + ncols * (tilewidth + gapwidth);
	int needheight = gapheight + nstrings * (tileheight + gapheight) + infoHeight;

	RECT rc;
	GetClientRect(&rc);
	Bitmap* BackBuffer;
	Graphics g(m_hWnd, true);
	BackBuffer = new Bitmap(needwidth, needheight, &g);
	Graphics gr(BackBuffer);
	Image* bm = NULL;
	Rect r(0, 0, needwidth, needheight);
	gr.Clear(Color(255, 180, 180, 180));
	LinearGradientBrush br(r, Color(255, 224, 224, 224), Color(255, 243, 243, 243),
	                       LinearGradientModeBackwardDiagonal);
	gr.FillRectangle(&br, r);
	int x, y;
	Pen Framepen(Color(90, 90, 90));
	TCHAR buf[256] = _T("\0");
	Font font(L"Tahoma", 10, FontStyleBold);
	Color ColorText(140, 255, 255, 255);
	Color ColorStroke(120, 0, 0, 0);

	for (int i = 0; i < n; i++)
	{
		bm = new Image(ThumbsView.GetFileName(i));
		x = gapwidth + (i % ncols) * (tilewidth + gapwidth);
		y = infoHeight + (infoHeight ? gapheight : 0) + ((i / ncols)) * (tileheight + gapheight);
		ThumbsView.GetItemText(i, 0, buf, 256);
		gr.DrawImage(bm, (int)(x /*(tilewidth-newwidth)/2*/), (int)y, (int)tilewidth, (int)tileheight);
		DrawStrokedText(gr, buf, RectF(float(x), float(y), float(tilewidth),
		                               float(tileheight)), font, ColorText, ColorStroke, 3, 3);
		gr.DrawRectangle(&Framepen, Rect(x /*(tilewidth-newwidth)/2*/, (int)y, (int)tilewidth, (int)tileheight));
		delete bm;
	}

	if (infoHeight)
	{
		StringFormat format;
		format.SetAlignment(StringAlignmentNear);
		format.SetLineAlignment(StringAlignmentNear);
		Font font(::GetDC(0), &Settings.VideoSettings.Font);
		// Font font(L"Arial", 12, FontStyleBold);
		SolidBrush br(/*Settings.ThumbSettings.ThumbTextColor*/ MYRGB(255, Settings.VideoSettings.TextColor));
		RectF textBounds(float(gapwidth), float(gapheight), float(needwidth - gapwidth), float(infoHeight - gapheight));
		gr.DrawString(Report, -1, &font, textBounds, &format, &br);
		// /DrawStrokedText(gr, Report,textBounds,font,ColorText,ColorStroke,3,3);
	}

	MySaveImage(BackBuffer, _T("grab_custom"), outFileName, 1, 100);
	delete BackBuffer;
	return 0;
}

LRESULT CVideoGrabberPage::OnBnClickedButton1(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	TCHAR Buf[MAX_PATH*4];
	GuiTools::SelectDialogFilter(Buf, sizeof(Buf)/sizeof(TCHAR),2, 
		CString(TR("����� �����"))+ _T(" (avi, mpg, vob, wmv, mkv ...)"),
		Settings.prepareVideoDialogFilters(),
		TR("��� �����"),
		_T("*.*"));

	CFileDialog fd(true,0,0,4|2,Buf,m_hWnd);

	if (fd.DoModal() != IDOK || !fd.m_szFileName[0])
		return 0;

	SetDlgItemText(IDC_FILEEDIT, fd.m_szFileName);

	return 0;
}

int CVideoGrabberPage::GrabBitmaps(const CString& szFile )
{

	CString videoEngine = Settings.VideoSettings.Engine;

	if ( videoEngine == CSettings::VideoEngineAuto) {
		if ( !Settings.IsFFmpegAvailable() ) {
			videoEngine = CSettings::VideoEngineDirectshow;
		} else {
			videoEngine = CSettings::VideoEngineFFmpeg;
			Utf8String ext = IuStringUtils::toLower( IuCoreUtils::ExtractFileExt( IuCoreUtils::WstringToUtf8((LPCTSTR)szFile) ) );
			if ( ext == "wmv" || ext == "asf" ) {
				videoEngine = CSettings::VideoEngineDirectshow;
			}
		}
	}

	delete videoGrabber_;
	Settings.VideoSettings.NumOfFrames = NumOfFrames;

	videoGrabber_ = new VideoGrabber();
	videoGrabber_->setFrameCount(NumOfFrames);
	videoGrabber_->onFrameGrabbed.bind(this, &CVideoGrabberPage::OnFrameGrabbed);
	videoGrabber_->onFinished.bind(this, &CVideoGrabberPage::OnFrameGrabbingFinished);
	VideoGrabber::VideoEngine engine = VideoGrabber::veAuto;
	if ( videoEngine == CSettings::VideoEngineFFmpeg ) {
		engine = VideoGrabber::veAvcodec;
	} else if ( videoEngine ==CSettings::VideoEngineDirectshow ) {
		engine = VideoGrabber::veDirectShow;
	}

	videoGrabber_->setVideoEngine(engine);


	videoGrabber_->grab(WCstringToUtf8(szFile));

		//SendDlgItemMessage(IDC_PROGRESSBAR, PBM_SETPOS, (i + 1)*10);
	

	return 0;
}

bool CVideoGrabberPage::OnShow()
{
	SetNextCaption(TR("�������"));
	SetDlgItemText(IDC_FILEEDIT, m_szFileName);
	::ShowWindow(GetDlgItem(IDC_FILEINFOBUTTON), (*MediaInfoDllPath) ? SW_SHOW : SW_HIDE);
	EnableNext(true);
	ShowPrev();
	ShowNext();
	EnablePrev();
	EnableExit();
	ThumbsView.MyDeleteAllItems();
	::SetFocus(GetDlgItem(IDC_GRAB));
	return true;
}


void CVideoGrabberPage::OnFrameGrabbed(const Utf8String& timeStr, int64_t, AbstractImage* img )
{
	GdiPlusImage *image = dynamic_cast<GdiPlusImage*>(img);
	if ( !image ) {
		LOG(ERROR) << "image is null";
		return;
	}
	Gdiplus::Bitmap *bm = image->getBitmap();
	if ( bm ) {
		SendDlgItemMessage(IDC_PROGRESSBAR, PBM_SETPOS, (grabbedFramesCount + 1) * 10 );
		OnAddImage(bm, Utf8ToWCstring(timeStr));
		delete image;
	}

}

void CVideoGrabberPage::OnFrameGrabbingFinished()
{
	ThreadTerminated();
	if (!CanceledByUser)
	{
		GrabInfo(TR("���������� ������ ���� ���������."));
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
	LPCTSTR filename;

	WizardDlg->CreatePage(2);
	CMainDlg* MainDlg = (CMainDlg*)WizardDlg->Pages[2];

	BOOL check = SendDlgItemMessage(IDC_MULTIPLEFILES, BM_GETCHECK);

	if (check) // If option "Multiple files" turn on
	{
		for (int i = 0; i < n; i++)
		{
			filename = ThumbsView.GetFileName(i);
			if (!filename)
				continue;
			MainDlg->AddToFileList(filename);
		}
	}
	else
	{
		CString outFileName;
		GenPicture(outFileName);
		if (!outFileName.IsEmpty())
			MainDlg->AddToFileList(outFileName);
	}

	ThumbsView.MyDeleteAllItems();
	BOOL scheck = SendDlgItemMessage(IDC_MULTIPLEFILES, BM_GETCHECK);

	Settings.VideoSettings.NumOfFrames = GetDlgItemInt(IDC_NUMOFFRAMESEDIT);

	return true;
}

LRESULT CVideoGrabberPage::OnLvnItemDelete(int /*idCtrl*/, LPNMHDR pNMHDR, BOOL& /*bHandled*/)
{
	// � ������ ����������� ������ ������������ ������ "�����"
	CheckEnableNext();
	return 0;
}

LRESULT CVideoGrabberPage::OnBnClickedFileinfobutton(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	CMediaInfoDlg dlg;
	TCHAR buffer[256];
	GetDlgItemText(IDC_FILEEDIT, buffer, 256);

	dlg.ShowInfo(buffer);
	return 0;
}


CString CVideoGrabberPage::GenerateFileNameFromTemplate(const CString& templateStr, int index, const CPoint size, const CString& originalName)
{
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
	CString md5 = Utf8ToWstring(IuCoreUtils::CryptoUtils::CalcMD5HashFromString(WCstringToUtf8(IntToStr(GetTickCount() + random(100))))).c_str();
	CString uid = md5.Mid(5,6);
	result.Replace(_T("%md5%"), (LPCTSTR)md5);
	result.Replace(_T("%uid%"), (LPCTSTR)uid);
	result.Replace(_T("%cx%"), IntToStr(size.x));
	result.Replace(_T("%cy%"), IntToStr(size.y));
	year.Format(_T("%04d"), (int)1900 + timeinfo->tm_year);
	month.Format(_T("%02d"), (int) timeinfo->tm_mon + 1);
	day.Format(_T("%02d"), (int) timeinfo->tm_mday);
	hours.Format(_T("%02d"), (int)timeinfo->tm_hour);
	seconds.Format(_T("%02d"), (int)timeinfo->tm_sec);
	minutes.Format(_T("%02d"), (int)timeinfo->tm_min);

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
	ShellExecute(NULL, L"open", snapshotsFolder, 0, 0, SW_SHOWDEFAULT);
	return 0;
}