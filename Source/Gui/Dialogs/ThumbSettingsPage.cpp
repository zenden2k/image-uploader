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
#include "atlheaders.h"
#include "3rdpart/GdiplusH.h"
#include <GdiPlusPixelFormats.h>
#include "ThumbSettingsPage.h"

#include <uxtheme.h>
#include "LogWindow.h"
#include "Func/LangClass.h"
#include "Core/Settings.h"
#include "Gui/GuiTools.h"
#include "Core/Images/Thumbnail.h"
#include "Func/MyUtils.h"
#include "ThumbEditor.h"
#include "InputDialog.h"
#include "Func/IuCommonFunctions.h"
#include "Func/WinUtils.h"
#include "Core/Images/Utils.h"
#include "Core/Video/GdiPlusImage.h"


#pragma comment( lib, "uxtheme.lib" )
#include <Core/Video/GdiPlusImage.h>
#include <Core/ServiceLocator.h>

// CThumbSettingsPage
CThumbSettingsPage::CThumbSettingsPage()
{
    params_ = Settings.imageServer.getImageUploadParams().getThumb();
   m_CatchFormChanges = false;
}

CThumbSettingsPage::~CThumbSettingsPage()
{
    for(std::map<std::string, Thumbnail*>::const_iterator it = thumb_cache_.begin(); it!= thumb_cache_.end(); ++it)
    {
        delete it->second;
    }
}

LRESULT CThumbSettingsPage::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    TabBackgroundFix(m_hWnd);
    // Translating controls

    TRC(IDC_TEXTONIMAGEGROUP, "Текст на картинке");
    TRC(IDC_ENTERYOURTEXTLABEL, "Введите текст:");
    TRC(IDC_THUMBFONT, "Шрифт...");
    TRC(IDC_THUMBTEXTCHECKBOX, "Надпись на миниатюре:");
    TRC(IDC_THUMBBACKGROUNDLABEL, "Цвет фона:");
    TRC(IDC_WIDTHRADIO, "Ширина:");
    TRC(IDC_HEIGHTRADIO, "Высота:");
    TRC(IDC_THUMBSCOMBOLABEL, "Шаблон миниатюры:");
    TRC(IDC_EDITTHUMBNAILPRESET, "Редактировать");
    TRC(IDC_NEWTHUMBNAIL, "Создать копию");
    TRC(IDC_THUMBFORMATLABEL,"Формат:");
    TRC(IDC_THUMBQUALITYLABEL,"Качество:");
    
    ThumbBackground.SubclassWindow(GetDlgItem(IDC_THUMBBACKGROUND));
    RECT rc = {13, 170, 290, 400};
    img.SubclassWindow(GetDlgItem(IDC_COMBOPREVIEW));
    //img.Create(m_hWnd, rc);
    img.LoadImage(0);

    SendDlgItemMessage(IDC_THUMBQUALITYSPIN, UDM_SETRANGE, 0, (LPARAM) MAKELONG((short)100, (short)1) );    
    SetDlgItemText(IDC_THUMBTEXT, params_.Text);

    GuiTools::AddComboBoxItems(m_hWnd, IDC_THUMBFORMATLIST, 4, TR("Как у изображения"),
        _T("JPEG"), _T("PNG"), _T("GIF"));

    std::vector<CString> files;
    CString folder = IuCommonFunctions::GetDataFolder()+_T("\\Thumbnails\\");
    WinUtils::GetFolderFileList(files, folder , _T("*.xml"));
    for(size_t i=0; i<files.size(); i++)
        GuiTools::AddComboBoxItems(m_hWnd, IDC_THUMBSCOMBO, 1, Utf8ToWCstring(IuCoreUtils::ExtractFileNameNoExt( WCstringToUtf8( files[i]))) );
    
    
    SendDlgItemMessage(IDC_THUMBTEXTCHECKBOX, BM_SETCHECK, params_.AddImageSize);
    SendDlgItemMessage(IDC_THUMBSCOMBO, CB_SELECTSTRING, -1, (LPARAM) (LPCTSTR) params_.TemplateName);
    SetDlgItemText(IDC_THUMBTEXT, params_.Text);
    SetDlgItemInt(IDC_THUMBQUALITYEDIT,  params_.Quality);
    SendDlgItemMessage(IDC_THUMBFORMATLIST, CB_SETCURSEL, params_.Format);
    SendDlgItemMessage(IDC_WIDTHRADIO, BM_SETCHECK,  params_.ResizeMode == ThumbCreatingParams::trByWidth);
     SendDlgItemMessage(IDC_HEIGHTRADIO, BM_SETCHECK,  params_.ResizeMode == ThumbCreatingParams::trByHeight);
    SetDlgItemInt(IDC_WIDTHEDIT,params_.Size);
   SetDlgItemInt(IDC_HEIGHTEDIT,params_.Size);
    BOOL b;
    ThumbBackground.SetColor(params_.BackgroundColor);
    OnThumbComboChanged(0,0,0,b);
    ::EnableWindow(GetDlgItem(IDC_WIDTHEDIT), params_.ResizeMode == ThumbCreatingParams::trByWidth);
   ::EnableWindow(GetDlgItem(IDC_HEIGHTEDIT), ThumbCreatingParams::trByHeight);
 m_CatchFormChanges = true;
    return 1;  // Let the system set the focus
}

bool CThumbSettingsPage::Apply()
{
    params_.AddImageSize =  SendDlgItemMessage(IDC_THUMBTEXTCHECKBOX, BM_GETCHECK) == BST_CHECKED;
    TCHAR buf[256] =_T("\0");
    GetDlgItemText(IDC_THUMBSCOMBO, buf, 255);
     params_.TemplateName =buf;
     params_.Format = static_cast<ThumbCreatingParams::ThumbFormatEnum>(SendDlgItemMessage(IDC_THUMBFORMATLIST, CB_GETCURSEL ));
    params_.Quality = GetDlgItemInt(IDC_THUMBQUALITYEDIT);
    params_.ResizeMode = SendDlgItemMessage(IDC_WIDTHRADIO, BM_GETCHECK) == FALSE ? ThumbCreatingParams::trByHeight : ThumbCreatingParams::trByWidth;
    params_.Text = GuiTools::GetWindowText(GetDlgItem(IDC_THUMBTEXT));
    params_.Size  = params_.ResizeMode == ThumbCreatingParams::trByWidth ?  GetDlgItemInt(IDC_WIDTHEDIT) : GetDlgItemInt(IDC_HEIGHTEDIT);
    params_.BackgroundColor = ThumbBackground.GetColor();
    ImageUploadParams iup = Settings.imageServer.getImageUploadParamsRef();
    iup.setThumb(params_);
    Settings.imageServer.setImageUploadParams(iup);

   for(std::map<std::string, Thumbnail*>::const_iterator it = thumb_cache_.begin(); it!= thumb_cache_.end(); ++it)
    {
        it->second->SaveToFile();
    }
    return TRUE;
}

LRESULT  CThumbSettingsPage::OnBnClickedNewThumbnail(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
    std::string fileName = getSelectedThumbnailFileName();
    if(fileName.empty())
        return 0;
    std::string newName = "copy_" + IuCoreUtils::ExtractFileNameNoExt(fileName);
    CInputDialog dlg(TR("Окно ввода"), TR("Введите имя нового шаблона миниатюры:"), Utf8ToWCstring(newName));
    if(dlg.DoModal() == IDOK)
    {
        newName = WCstringToUtf8(dlg.getValue());
    }
    else return 0;
    std::string srcFolder = IuCoreUtils::ExtractFilePath(fileName) +"/";
    std::string destination = srcFolder + newName + ".xml";
    if(IuCoreUtils::FileExists(destination))
    {
        MessageBox(TR("Шаблон с таким именем уже существует!"), APPNAME, MB_ICONERROR);
        return 0;
    }
    Thumbnail * thumb = 0;
    if(thumb_cache_.count(fileName)) {
    
        thumb =  thumb_cache_[fileName];
    } else {
        thumb = new Thumbnail();
        thumb->LoadFromFile(fileName);
    }
    std::string sprite = thumb->getSpriteFileName();
    thumb->setSpriteFileName(newName + '.' + IuCoreUtils::ExtractFileExt(sprite));
    if ( ( sprite.empty() || IuCoreUtils::copyFile(sprite,srcFolder + newName + '.' + IuCoreUtils::ExtractFileExt(sprite))) && IuCoreUtils::copyFile(fileName, destination) && thumb->SaveToFile(destination) ) 
    {
        GuiTools::AddComboBoxItems(m_hWnd, IDC_THUMBSCOMBO, 1, Utf8ToWCstring(newName) );
        
    }
    SendDlgItemMessage(IDC_THUMBSCOMBO, CB_SELECTSTRING, -1, (LPARAM) (LPCTSTR)Utf8ToWCstring( newName));
    GuiTools::EnableDialogItem(m_hWnd, IDC_EDITTHUMBNAILPRESET, true);
    showSelectedThumbnailPreview();
    return 0;
}

LRESULT CThumbSettingsPage::OnThumbComboChanged(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
    std::string fileName = getSelectedThumbnailName();
    bool isStandartThumbnail = ( fileName == "default" || fileName == "classic" || fileName == "classic2" || fileName == "flat"
        || fileName == "transp");
    GuiTools::EnableDialogItem(m_hWnd, IDC_EDITTHUMBNAILPRESET, !isStandartThumbnail);

    showSelectedThumbnailPreview();
    return 0;
}

LRESULT  CThumbSettingsPage::OnEditThumbnailPreset(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
    std::string fileName = getSelectedThumbnailFileName();
    std::auto_ptr<Thumbnail> autoPtrThumb;
    Thumbnail * thumb = 0;
    if(thumb_cache_.count(fileName))
    thumb  = thumb_cache_[fileName];
    if(!thumb)
    { 
        thumb = new Thumbnail();
        autoPtrThumb.reset(thumb);
        if(!thumb->LoadFromFile(fileName))
        {
            MessageBox(TR("Не могу загрузить файл миниатюры!"));
            return 0;
        }
    }
    CThumbEditor dlg(thumb);
    if(dlg.DoModal(m_hWnd) == IDOK)
    {
        thumb_cache_[fileName] = thumb;
        autoPtrThumb.release();
        
        showSelectedThumbnailPreview();
    }
    return 0;
}

std::string CThumbSettingsPage::getSelectedThumbnailFileName()
{
    TCHAR buf[256];
    int index = SendDlgItemMessage(IDC_THUMBSCOMBO, CB_GETCURSEL);
    if(index < 0) return "";
    SendDlgItemMessage(IDC_THUMBSCOMBO, CB_GETLBTEXT, index, (WPARAM)buf);
    CString thumbFileName = buf;
    Thumbnail thumb;
    CString folder = IuCommonFunctions::GetDataFolder()+_T("\\Thumbnails\\");
    return WCstringToUtf8(folder + thumbFileName+".xml");
}

std::string CThumbSettingsPage::getSelectedThumbnailName()
{
    TCHAR buf[256];
    int index = SendDlgItemMessage(IDC_THUMBSCOMBO, CB_GETCURSEL);
    if(index < 0) return "";
    SendDlgItemMessage(IDC_THUMBSCOMBO, CB_GETLBTEXT, index, (WPARAM)buf);
    return WCstringToUtf8(buf);
}

void CThumbSettingsPage::showSelectedThumbnailPreview()
{
    using namespace Gdiplus;
    std::string fileName = getSelectedThumbnailFileName();
    if(fileName.empty())
        return ;
    CString folder = IuCommonFunctions::GetDataFolder()+_T("\\Thumbnails\\");
    
    std::auto_ptr<Thumbnail> autoPtrThumb;
    Thumbnail * thumb = 0;
    if(thumb_cache_.count(fileName))
        thumb  = thumb_cache_[fileName];
    if(!thumb)
    { 
        thumb = new Thumbnail();
        autoPtrThumb.reset(thumb);
        if(!thumb->LoadFromFile(fileName))
        {
            ServiceLocator::instance()->logger()->write(logError, _T("CThumbSettingsPage"), TR("Не могу загрузить файл миниатюры!"));
            return;
        }
    }
    ImageConverter conv;
    conv.setThumbCreatingParams(params_);
    conv.setThumbnail(thumb);
    Bitmap * bm = BitmapFromResource(GetModuleHandle(0), MAKEINTRESOURCE(IDR_PNG2),_T("PNG"));
    if(!bm) 
    {
        MessageBox(TR("Не могу загрузить файл миниатюры!"));
        return ;
    }
    
    Bitmap *toUse = bm->Clone(0,300, bm->GetWidth(), bm->GetHeight()-300, PixelFormatDontCare);
    GdiPlusImage source(toUse);
    std::shared_ptr<AbstractImage> result = conv.createThumbnail(&source,  50 * 1024, 0);
    if(result)    
        img.LoadImage(0, dynamic_cast<GdiPlusImage*>(result.get())->getBitmap());

    //delete toUse;
    delete bm;
}

LRESULT CThumbSettingsPage::OnThumbTextCheckboxClick(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
    ThumbTextCheckboxChange();
    showSelectedThumbnailPreview();
    return 0;
}

void CThumbSettingsPage::ThumbTextCheckboxChange()
{
    bool bChecked = SendDlgItemMessage(IDC_THUMBTEXTCHECKBOX, BM_GETCHECK)==BST_CHECKED;
    ::EnableWindow(GetDlgItem(IDC_THUMBTEXT), bChecked);
    params_.AddImageSize = bChecked;
    
}

LRESULT CThumbSettingsPage::OnThumbTextChange(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
   if(!m_CatchFormChanges) return 0;
    params_.Text = GuiTools::GetWindowText(GetDlgItem(IDC_THUMBTEXT));
    showSelectedThumbnailPreview();
    return 0;
}

LRESULT CThumbSettingsPage::OnWidthEditChange(WORD wNotifyCode, WORD wID, HWND hWndCtl)
{
   if(!m_CatchFormChanges) return 0;
   params_.ResizeMode = (SendDlgItemMessage(IDC_WIDTHRADIO, BM_GETCHECK) == FALSE)?ThumbCreatingParams::trByHeight : ThumbCreatingParams::trByWidth ;
   ::EnableWindow(GetDlgItem(IDC_WIDTHEDIT), params_.ResizeMode == ThumbCreatingParams::trByWidth);
   ::EnableWindow(GetDlgItem(IDC_HEIGHTEDIT),  params_.ResizeMode == ThumbCreatingParams::trByHeight);
    params_.Size = GetDlgItemInt(IDC_WIDTHEDIT);
    params_.Size = GetDlgItemInt(IDC_HEIGHTEDIT);
   showSelectedThumbnailPreview();
   return 0;
}
