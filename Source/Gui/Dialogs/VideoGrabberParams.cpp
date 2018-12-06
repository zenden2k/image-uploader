/*

    Image Uploader -  free application for uploading images/files to the Internet

    Copyright 2007-2018 Sergey Svistunov (zenden2k@yandex.ru)

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

#include "VideoGrabberParams.h"

#include "Core/Settings.h"
#include "Gui/GuiTools.h"
#include "Gui/Components/NewStyleFolderDialog.h"

//#define CheckBounds(n, a, b, d) {if ((n < a) || (n > b)) n = d; }

// CVideoGrabberParams
CVideoGrabberParams::CVideoGrabberParams()
{
}

CVideoGrabberParams::~CVideoGrabberParams()
{
}

LRESULT CVideoGrabberParams::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    TabBackgroundFix(m_hWnd);

    SetDlgItemInt(IDC_COLUMNSEDIT, Settings.VideoSettings.Columns);
    SetDlgItemInt(IDC_TILEWIDTH, Settings.VideoSettings.TileWidth);
    SetDlgItemInt(IDC_GAPWIDTH, Settings.VideoSettings.GapWidth);
    SetDlgItemInt(IDC_GAPHEIGHT, Settings.VideoSettings.GapHeight);
    m_Font = Settings.VideoSettings.Font;
    SendDlgItemMessage(IDC_USEAVIINFO, BM_SETCHECK, Settings.VideoSettings.UseAviInfo);
    SendDlgItemMessage(IDC_MEDIAINFOONIMAGE, BM_SETCHECK, Settings.VideoSettings.ShowMediaInfo);

    SetWindowText(TR("Appearance options"));
    TRC(IDC_COLUMNSEDITLABEL, "Number of columns:");
    TRC(IDC_PREVIEWWIDTHLABEL, "Thumbnail width:");
    TRC(IDC_INTERVALHORLABEL, "Horizontal interval:");
    TRC(IDC_INTERVALVERTLABEL, "Vertical interval:");
    TRC(IDC_APPEARANCEGROUP, "Frames layout");
    TRC(IDC_MEDIAINFOONIMAGE, "Display video file information on resulting picture");
    TRC(IDC_MEDIAINFOFONT, "Font...");
    TRC(IDC_TEXTCOLORLABEL, "Text color:");
    TRC(IDC_PARAMETERSHINTLABEL, "%f% - name of videofile without extension, \r\n%fe% -  name of videofile, \r\n%ext% - extension, \r\n%y% - year, %m% - month, %d% - day\r\n%h% - hour, %n% - minute, %s% - second\r\n %i% - file counter,\r\n%cx% - width,  %cy% - height");
    TRC(IDC_SNAPSHOTFILENAMELABEL, "Filename format:");

    TRC(IDC_VIDEOSNAPSHOTSFOLDERLABEL, "Folder for snapshots:");
    TRC(IDC_VIDEOSNAPSHOTSFOLDERBUTTON, "Select...");
    Color1.SubclassWindow(GetDlgItem(IDC_TEXTCOLOR));
    Color1.SetColor(Settings.VideoSettings.TextColor);
    SetDlgItemText(IDC_VIDEOSNAPSHOTSFOLDEREDIT, Settings.VideoSettings.SnapshotsFolder);
    SetDlgItemText(IDC_SNAPSHOTFILENAMEEDIT, Settings.VideoSettings.SnapshotFileTemplate);

    BOOL b;
    OnShowMediaInfoTextBnClicked(IDC_MEDIAINFOONIMAGE, 0, 0, b);
    return 1;  // Let the system set the focus
}

bool CVideoGrabberParams::Apply()
{
    int columns = GetDlgItemInt(IDC_COLUMNSEDIT);
    if (columns <= 0) {
        CString fieldName = GuiTools::GetDlgItemText(m_hWnd, IDC_COLUMNSEDITLABEL);
        CString message;
        message.Format(TR("Error in the field '%s': value should be greater than zero"), fieldName);
        throw ValidationException(message, GetDlgItem(IDC_COLUMNSEDIT));
    }
    CheckBounds(IDC_TILEWIDTH, 10, 1024, IDC_PREVIEWWIDTHLABEL);
    CheckBounds(IDC_GAPWIDTH, 0, 200, IDC_INTERVALHORLABEL);
    CheckBounds(IDC_GAPHEIGHT, 0, 200, IDC_INTERVALVERTLABEL);

    Settings.VideoSettings.Columns = columns;
    Settings.VideoSettings.TileWidth = GetDlgItemInt(IDC_TILEWIDTH);
    Settings.VideoSettings.GapWidth = GetDlgItemInt(IDC_GAPWIDTH);
    Settings.VideoSettings.GapHeight = GetDlgItemInt(IDC_GAPHEIGHT);
    Settings.VideoSettings.UseAviInfo = SendDlgItemMessage(IDC_USEAVIINFO, BM_GETCHECK);
    Settings.VideoSettings.ShowMediaInfo = SendDlgItemMessage(IDC_MEDIAINFOONIMAGE, BM_GETCHECK);
    Settings.VideoSettings.Font = m_Font;
    Settings.VideoSettings.SnapshotsFolder = GuiTools::GetDlgItemText(m_hWnd, IDC_VIDEOSNAPSHOTSFOLDEREDIT);
    Settings.VideoSettings.SnapshotFileTemplate = GuiTools::GetDlgItemText(m_hWnd, IDC_SNAPSHOTFILENAMEEDIT);

  
    Settings.VideoSettings.TextColor = Color1.GetColor();

    return true;
}

LRESULT CVideoGrabberParams::OnMediaInfoFontClicked(WORD wNotifyCode, WORD wID, HWND hWndCtl)
{
    // Font selection dialog
    CFontDialog dlg(&m_Font);
    dlg.DoModal(m_hWnd);
    return 0;
}

LRESULT CVideoGrabberParams::OnShowMediaInfoTextBnClicked(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
    bool bChecked = SendDlgItemMessage(wID, BM_GETCHECK) == BST_CHECKED;
    GuiTools::EnableNextN(GetDlgItem(wID), 3, bChecked);
    return 0;
}

LRESULT CVideoGrabberParams::OnVideoSnapshotsFolderButtonClicked(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled) {
    CString path = GuiTools::GetWindowText(GetDlgItem(IDC_VIDEOSNAPSHOTSFOLDEREDIT));

    CNewStyleFolderDialog fd(m_hWnd, path, TR("Select folder") );

    fd.SetFolder(path);

    if ( fd.DoModal(m_hWnd) == IDOK ) {
        SetDlgItemText(IDC_VIDEOSNAPSHOTSFOLDEREDIT,fd.GetFolderPath());
        return true;
    }
    return 0;
}

void CVideoGrabberParams::CheckBounds(int controlId, int minValue, int maxValue, int labelId) {
    int value = GetDlgItemInt(controlId);
    if (value < minValue || value > maxValue) {
        CString fieldName = labelId !=-1 ? GuiTools::GetDlgItemText(m_hWnd, labelId) : _T("Unknown field");
        CString message;
        message.Format(TR("Error in the field '%s': value should be between %d and %d."), fieldName, minValue, maxValue);
        throw ValidationException(message, GetDlgItem(controlId));
    }
}