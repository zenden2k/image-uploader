/*

    Image Uploader -  free application for uploading images/files to the Internet

    Copyright 2007-2015 Sergey Svistunov (zenden2k@gmail.com)

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

#include "Func/Settings.h"
#include "Gui/GuiTools.h"
#include "Gui/Components/NewStyleFolderDialog.h"

#define CheckBounds(n, a, b, d) {if ((n < a) || (n > b)) n = d; }

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

    SetWindowText(TR("Настройки внешнего вида"));
    TRC(IDCANCEL, "Отмена");
    TRC(IDC_COLUMNSEDITLABEL, "Количество колонок:");
    TRC(IDC_PREVIEWWIDTHLABEL, "Ширина превьюшки:");
    TRC(IDC_INTERVALHORLABEL, "Горизонтальный промежуток:");
    TRC(IDC_INTERVALVERTLABEL, "Вертикальный промежуток:");
    TRC(IDC_APPEARANCEGROUP, "Параметры компоновки кадров");
    TRC(IDC_MEDIAINFOONIMAGE, "Отобразить информацию о видеофайле на картинке");
    TRC(IDC_MEDIAINFOFONT, "Шрифт...");
    TRC(IDC_TEXTCOLORLABEL, "Цвет текста:");
    TRC(IDC_PARAMETERSHINTLABEL, "%f% - имя видео файла без расширения, \r\n%fe% - имя видео файла с расширением\r\n%ext% - расширение видео файла,\r\n%y% - год, %m% - месяц, %d% - день\n%h% - час, %n% - минута, %s% - секунда\n %i% - порядковый номер,\n%cx% - ширина,  %cy% - высота изображения");
    TRC(IDC_SNAPSHOTFILENAMELABEL, "Формат имени файла");

    TRC(IDC_VIDEOSNAPSHOTSFOLDERLABEL, "Папка для сохранения кадров:");
    TRC(IDC_VIDEOSNAPSHOTSFOLDERBUTTON, "Выбрать...");
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
    Settings.VideoSettings.Columns = GetDlgItemInt(IDC_COLUMNSEDIT);
    Settings.VideoSettings.TileWidth = GetDlgItemInt(IDC_TILEWIDTH);
    Settings.VideoSettings.GapWidth = GetDlgItemInt(IDC_GAPWIDTH);
    Settings.VideoSettings.GapHeight = GetDlgItemInt(IDC_GAPHEIGHT);
    Settings.VideoSettings.UseAviInfo = SendDlgItemMessage(IDC_USEAVIINFO, BM_GETCHECK);
    Settings.VideoSettings.ShowMediaInfo = SendDlgItemMessage(IDC_MEDIAINFOONIMAGE, BM_GETCHECK);
    Settings.VideoSettings.Font = m_Font;
    Settings.VideoSettings.SnapshotsFolder = GuiTools::GetDlgItemText(m_hWnd, IDC_VIDEOSNAPSHOTSFOLDEREDIT);
    Settings.VideoSettings.SnapshotFileTemplate = GuiTools::GetDlgItemText(m_hWnd, IDC_SNAPSHOTFILENAMEEDIT);

    CheckBounds(Settings.VideoSettings.TileWidth, 10, 1024, 200);
    CheckBounds(Settings.VideoSettings.GapWidth, 0, 200, 2);
    CheckBounds(Settings.VideoSettings.GapHeight, 0, 200, 2);
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

    CNewStyleFolderDialog fd(m_hWnd, path, TR("Выбор папки") );
   
    if ( fd.DoModal(m_hWnd) == IDOK ) {
        SetDlgItemText(IDC_VIDEOSNAPSHOTSFOLDEREDIT,fd.GetFolderPath());
        return true;
    }
    return 0;
}