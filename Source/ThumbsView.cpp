/*
    Image Uploader - program for uploading images/files to Internet
    Copyright (C) 2007-2010 ZendeN <zenden2k@gmail.com>
	 
    HomePage:    http://zenden.ws/imageuploader

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "stdafx.h"
#include "ThumbsView.h"
#include "mydropsource.h"
#include "mydataobject.h"
#include "common.h"

#define THUMBNAIL_WIDTH 170   // constants
#define THUMBNAIL_HEIGHT 120



bool NewBytesToString(__int64 nBytes, LPTSTR szBuffer, int nBufSize);

// CThumbsView
CThumbsView::CThumbsView()
{
	maxwidth = 0;
	maxheight = 0;
	m_NeedUpdate  = FALSE;
}

CThumbsView::~CThumbsView()
{
	ImageList.Destroy();
}

void CThumbsView::Init(bool Extended)
{
	ExtendedView = Extended;
	ImageView.Create(m_hWnd);
	ImageList.Create(THUMBNAIL_WIDTH, THUMBNAIL_HEIGHT + (ExtendedView?20:0), ILC_COLOR24, 0, 3);
	SetImageList(ImageList, LVSIL_NORMAL);
	DWORD style = GetExtendedListViewStyle();
	style = style | LVS_EX_DOUBLEBUFFER | LVS_EX_BORDERSELECT ;
	SetExtendedListViewStyle(style);
	SendMessage(LVM_SETICONSPACING, 0, MAKELONG(THUMBNAIL_WIDTH+5, THUMBNAIL_HEIGHT+25+(ExtendedView?20:0)));
}

int CThumbsView::AddImage(LPCTSTR FileName, LPCTSTR Title, Image* Img)
{
	if( !FileName ) return false;

	int n = GetItemCount();

	RECT rc;
	GetClientRect(&rc);

	// Если ImageList пустой, создаем дефолтную картинку
	if(ImageList.GetImageCount() < 1)
		LoadThumbnail(-1, 0);

	AddItem(n, 0, Title, 0);

	ThumbsViewItem * TVI = new ThumbsViewItem;

	TVI->ThumbOutDate = FALSE;
	TVI->FileName = FileName;
	SetItemData(n, (DWORD)TVI);

	// Если уже есть загруженная картинка, генерируем эскиз немедленно
	// Это нужно для Video Grabber-a
	bool IsRun = IsRunning();

	if(Img && !IsRun) LoadThumbnail(n, Img);

	// Упорядочиваем картинки
	Arrange(LVA_ALIGNTOP);

	RedrawItems(n, n);
	return 0;
}

bool CThumbsView::MyDeleteItem(int ItemIndex)
{
	if( ItemIndex < 0 || ItemIndex > GetItemCount()-1) return false;

	SimpleDelete(ItemIndex); // Удаляем превьюшку из Imagelist
	DeleteItem(ItemIndex);	// Удаляем непостредственно из контрола

	Arrange(LVA_ALIGNTOP);

	// Показываем ту картинку, которая шла после удаленной
	EnsureVisible(ItemIndex,true);

	return true;
}

LRESULT CThumbsView::OnMButtonUp(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& bHandled)
{
	// Получаем координаты курсора
	POINT p = {LOWORD(lParam),HIWORD(lParam)};

	int ItemIndex = HitTest(p, 0); //Getting the index of item was clicked (by middle button)
	if(ItemIndex < 0) return 0;

	if(GetItemState(ItemIndex, LVIS_SELECTED) != LVIS_SELECTED)
	{
		// Удаляем только элемент под указателем
		bool NeedRestart = StopAndWait();
		MyDeleteItem(ItemIndex);
		UpdateImageIndexes(ItemIndex);
		if(NeedRestart)
			LoadThumbnails();
	}
	else if(GetNextItem(-1,LVNI_SELECTED)>=0)
	{
		// Если под указателем выбранная картинка, удаляем все выбранные
		DeleteSelected();
	}

	bHandled = true; //Не даем обработать соообщение стандартной процедуре
	return 0;
}

int CThumbsView::DeleteSelected(void)
{
	bool IsRun = IsRunning();
	if(IsRun)
	{
		SignalStop();
		MsgWaitForSingleObject(m_hThread, INFINITE);
		ATLTRACE(_T("MsgWait stopped!!!\r\n"));
	}

	if(GetItemCount() < 1) return 0;

	SetRedraw(false);
	int nItem=0;
	int FirstItem = GetNextItem(nItem-1,LVNI_SELECTED	);
	do
	{
		nItem = GetNextItem(nItem-1,LVNI_SELECTED	);
		if(nItem == -1) break;
		SimpleDelete(nItem);
		DeleteItem(nItem);
	}
	while(nItem!=-1);


	UpdateImageIndexes(FirstItem);
	SetRedraw(true);
	Arrange(LVA_ALIGNTOP);
	if(IsRun )
		LoadThumbnails();
	return 0;
}

void CThumbsView::UpdateImageIndexes(int StartIndex)
{
	if(StartIndex==-1) return;
	int n = GetItemCount();
	LV_ITEM item;
	for (int i=StartIndex;i<n;i++)
	{
		if(GetImageIndex(i)>0)
			SetItem(i, 0, LVIF_IMAGE, 0,i+1, 0, 0, 0);
	}
}

void CThumbsView::MyDeleteAllItems()
{
	bool IsRun = IsRunning();
	if(IsRun)
	{
		SignalStop();
		MsgWaitForSingleObject(m_hThread, INFINITE);
		ATLTRACE(_T("MsgWait stopped!!!\r\n"));
	}
	int n = GetItemCount();
	for(int i=0; i<n; i++)
		SimpleDelete(i, false);
	DeleteAllItems();
	ImageList.RemoveAll();

}

bool CThumbsView::SimpleDelete(int ItemIndex, bool DeleteThumb)
{
	if(DeleteThumb) 
		ImageList.Remove(ItemIndex + 1);

	ThumbsViewItem *TVI = (ThumbsViewItem *)GetItemData(ItemIndex);
	SetItemData(ItemIndex, 0);
	if(TVI) delete TVI;

	return true;
}

LPCTSTR CThumbsView::GetFileName(int ItemIndex)
{
	ThumbsViewItem *TVI = (ThumbsViewItem *)GetItemData(ItemIndex);
	if(!TVI){return _T("");} 
	return TVI->FileName;
}

HICON GetAssociatedIcon (LPCTSTR filename, bool Small)
{
	SHFILEINFO Info;
	DWORD Flags;


	if (Small) 
		Flags = SHGFI_ICON | SHGFI_SMALLICON | SHGFI_USEFILEATTRIBUTES;
	else
		Flags = SHGFI_ICON | SHGFI_LARGEICON | SHGFI_USEFILEATTRIBUTES|SHGFI_ADDOVERLAYS;
	SHGetFileInfo (filename, FILE_ATTRIBUTE_NORMAL, &Info, sizeof(Info), Flags);
	return Info.hIcon;
}

LRESULT CThumbsView::OnKeyDown(TCHAR vk, UINT cRepeat, UINT flags)
{
	if(vk == VK_DELETE)
		DeleteSelected();
	else if(vk == _T('A'))
	{
		if(GetKeyState(VK_CONTROL)) //Check if Ctrl key is hold
		{
			SetRedraw(FALSE); // Turn off control redraw
			int n = GetItemCount();
			for (int i=0; i<n; i++)
			{
				SetItemState(i, LVIS_SELECTED, 2);
			}
			SetRedraw(TRUE);
		}
	}
	return 0;
}


bool CThumbsView::LoadThumbnail(int ItemID, Image *Img)
{
	if(ItemID>GetItemCount()-1) 
	{
		return false;
	}
	Bitmap *ImgBuffer=NULL;
	Image *bm=NULL;
	CString filename;
	if(ItemID>=0) 
	{
		filename  = GetFileName(ItemID);
	}

	if(IsImage(filename))
	{


		if(Img)
			bm=Img;
		else 
			if(ItemID>=0) 
			{
				bm=new Image(filename);
			}
	}

	int width,height,imgwidth,imgheight,newwidth,newheight;
	width=THUMBNAIL_WIDTH/*rc.right-2*/;
	height=THUMBNAIL_HEIGHT/*rc.bottom-16*/;
	int thumbwidth=THUMBNAIL_WIDTH;
	int  thumbheight=THUMBNAIL_HEIGHT;

	if(bm)
	{
		imgwidth=bm->GetWidth();
		imgheight=bm->GetHeight();
		if(imgwidth>maxwidth) maxwidth=imgwidth;
		if(imgheight>maxheight) maxheight=imgheight;

		if((float)imgwidth/imgheight>(float)width/height)
		{
			if(imgwidth<=width)
			{
				newwidth=imgwidth;
				newheight=imgheight;
			}
			else
			{
				newwidth=width;
				newheight=(float)newwidth/imgwidth*imgheight;}
		}
		else
		{
			if(imgheight<=height)
			{
				newwidth=imgwidth;
				newheight=imgheight;
			}
			else
			{
				newheight=height;
				newwidth=(float)newheight/imgheight*imgwidth;
			}
		}
	}

	Graphics g(m_hWnd,true);
	ImgBuffer = new Bitmap(thumbwidth, thumbheight+30, &g);


	Graphics gr(ImgBuffer);
	gr.SetInterpolationMode(InterpolationModeHighQualityBicubic );
	gr.Clear(Color(255,255,255,255));

	RectF bounds(1, 1, float(width), float(height));

	if(bm && !bm->GetWidth() && ItemID>=0) 
	{
		LinearGradientBrush 
			brush(bounds, Color(130, 255, 0, 0), Color(255, 0, 0, 0), 
			LinearGradientModeBackwardDiagonal); 

		StringFormat format;
		format.SetAlignment(StringAlignmentCenter);
		format.SetLineAlignment(StringAlignmentCenter);
		Font font(L"Arial", 12, FontStyleBold);
		WriteLog(logWarning, TR("Список изображений"),TR("Невозможно загрузить миниатюру к файлу."), CString(TR("Файл:"))+_T(" ")+GetFileName(ItemID));
		gr.DrawString(TR("Невозможно загрузить изображение"), -1, &font, bounds, &format, &brush);
	}

	// 

	else {
		LinearGradientBrush 
			br(bounds, Color(255, 255, 255, 255), Color(255, 210, 210, 210), 
			LinearGradientModeBackwardDiagonal/* LinearGradientModeVertical*/); 

		if(IsImage(filename))
			gr.FillRectangle(&br,1, 1, width-1,height-1);
		gr.SetInterpolationMode(InterpolationModeHighQualityBicubic );

		if(ItemID>=0 && !Img && !IsImage(filename))
		{


			WORD id;
			HICON ggg =//GetAssociatedIcon (filename,false);
				ExtractAssociatedIcon(
				GetModuleHandle(0),
				(LPWSTR)(LPCTSTR)filename,
				&id);
			if(ggg)
			{
				HDC dc=GetDC();
				HDC memDC = CreateCompatibleDC(dc);
				HBITMAP memBm = CreateCompatibleBitmap(dc,32,32);
				SelectObject(memDC, memBm);
				RECT r={0,0,32,32};
				FillRect(memDC,	// handle to device context 
					&r,	// pointer to structure with rectangle  
					GetSysColorBrush(COLOR_WINDOW)	// handle to brush 
					);


				DrawIcon(memDC, 0,0,ggg);
				Bitmap *bitmap=Bitmap::FromHBITMAP(memBm,0);
				;
				gr.DrawImage(/*backBuffer*/bitmap, (int)((width-bitmap->GetWidth())/2)+1, (int)((height-bitmap->GetHeight())/2), (int)bitmap->GetWidth(),(int)bitmap->GetHeight());
				delete bitmap;
				DeleteObject(ggg);
				DeleteObject(memDC);
				DeleteObject(memBm);
				ReleaseDC(dc);
			}
		}

		try{
			if(bm)
				gr.DrawImage(/*backBuffer*/bm, (int)((width-newwidth)/2)+1, (int)((height-newheight)/2), (int)newwidth,(int)newheight);
		}
		catch(...)
		{
		}

		RectF bounds(0, float(height), float(width), float(20));

		if (ExtendedView)
		{
			LinearGradientBrush 
				br2(bounds, Color(240, 255, 255, 255), Color(200, 210, 210, 210), 
				LinearGradientModeBackwardDiagonal /*LinearGradientModeVertical*/); 
			gr.FillRectangle(&br2,(float)1, (float)height+1, (float)width-2, (float)height+20-1);



		}

		if(ItemID>=0)
		{
			SolidBrush brush(Color(255,0,0,0));
			StringFormat format;
			format.SetAlignment(StringAlignmentCenter);
			format.SetLineAlignment(StringAlignmentCenter);
			Font font(L"Tahoma", 8, FontStyleRegular );
			LPCTSTR Filename = GetFileName(ItemID);
			WCHAR Buffer[256];
			int f = MyGetFileSize(GetFileName(ItemID));
			WCHAR buf2[25];
			NewBytesToString(f, buf2, 25);
			WCHAR FileExt[25];
			lstrcpy(FileExt, GetFileExt(Filename));
			if(!lstrcmpi(FileExt, _T("jpg"))) 
				lstrcpy(FileExt,_T("JPEG"));
			if(IsImage(filename) && bm)
			{
				wsprintf(Buffer,_T("%s %dx%d (%s)"),FileExt,(int)bm->GetWidth(),(int)bm->GetHeight(), buf2 );
			}
			else
			{
				wsprintf(Buffer,_T("%s")/*,FileExt*/, buf2 );

			}
			gr.DrawString(Buffer, -1, &font, bounds, &format, &brush);
		}
	}

	HBITMAP bmp=NULL;
	ImgBuffer->GetHBITMAP(Color(255,255,255), &bmp);

	if(ImageList.GetImageCount()>ItemID+1)
	{
		ImageList.Replace(ItemID+1, bmp,0);
		RedrawItems(ItemID,ItemID);
	}
	else 
	{
		ImageList.Add(bmp, (COLORREF)0);
		SetItem(ItemID, 0, LVIF_IMAGE	, 0,ItemID+1, 0, 0, 0);
	}
	DeleteObject(bmp);
	if(!Img && bm) delete bm;
	if(ImgBuffer) delete ImgBuffer;
	return true;
}

int CThumbsView::GetImageIndex(int ItemIndex)
{
	LV_ITEM item;

	item.iItem = ItemIndex;
	item.iSubItem = 0;
	item.mask = LVIF_IMAGE;
	GetItem(&item);
	return item.iImage;
}

LRESULT CThumbsView::OnLButtonDblClk(UINT Flags, CPoint Pt)
{
	ViewSelectedImage();
	return 0;
}

// The Thread which loads Thumbnails in ImageList
DWORD CThumbsView::Run()
{
	int i, item; 
	item = -1;

	for(i=0; i<GetItemCount(); i++) // FIX THIS 
	{
		ThumbsViewItem *TVI =(ThumbsViewItem *) GetItemData(i);
		if(ShouldStop()) goto finish;
		if(GetImageIndex(i) < 1 || (m_NeedUpdate && TVI &&  TVI->ThumbOutDate))
		{
			item = i; 
			break;
		}
	}
	if(item==-1) goto finish;

	for(i=item; i<GetItemCount(); i++) // FIX THIS 
	{
		ThumbsViewItem *TVI =(ThumbsViewItem *) GetItemData(i);
		LockImagelist();
		if(!TVI) break;
		if(ShouldStop()) break;
		TVI->ThumbOutDate = FALSE;
		LockImagelist(false);
		LoadThumbnail(i);
	}
finish:
	LockImagelist(false);

	m_NeedUpdate = false;
	UpdateImageIndexes();
	ATLTRACE(_T("Thumbs loading thread stopped\r\n"));
	return 0;
}

void CThumbsView::LoadThumbnails()
{
	if(!IsRunning())  
	{
		Start(THREAD_PRIORITY_BELOW_NORMAL);
	}
}

void CThumbsView::StopLoadingThumbnails()
{
	if(IsRunning())  SignalStop();
}

void CThumbsView::ViewSelectedImage()
{
	int nCurItem;
	if ((nCurItem = GetNextItem(-1, LVNI_ALL|LVNI_SELECTED)) < 0) return;

	LPCTSTR FileName = GetFileName(nCurItem);

	if(!FileName || !IsImage(FileName)) return;
	ImageView.ViewImage(FileName);
}

LRESULT CThumbsView::OnLvnBeginDrag(int /*idCtrl*/, LPNMHDR pnmh, BOOL& /*bHandled*/)
{
	if(GetItemCount() < 1) return 0;

	NM_LISTVIEW *pnmv = (NM_LISTVIEW FAR *) pnmh;  
	DWORD dwEffect=0;

	CMyDropSource *pDropSource = new CMyDropSource();
	CMyDataObject *pDataObject = new CMyDataObject();

	int nItem=-1;
	do
	{
		nItem = GetNextItem(nItem,LVNI_SELECTED	);
		if(nItem == -1) break;
		pDataObject->AddFile(GetFileName(nItem));
	}
	while(nItem!=-1);

	DWORD dwResult = ::DoDragDrop(pDataObject, pDropSource, DROPEFFECT_COPY, &dwEffect);
	pDropSource->Release();
	pDataObject->Release();

	return 0;
}

void CThumbsView::OutDateThumb(int nIndex)
{
	ThumbsViewItem *TVI = (ThumbsViewItem *)GetItemData(nIndex);
	if(!TVI) return;
	TVI->ThumbOutDate=TRUE;
}

void CThumbsView::UpdateOutdated()
{
	m_NeedUpdate = true;
	if(!IsRunning()) LoadThumbnails();
}

void CThumbsView::LockImagelist(bool bLock)
{
	/*if(bLock)
	ImageListCS.Lock();
	else ImageListCS.Unlock();*/
}

bool CThumbsView::StopAndWait()
{	
	bool IsRun = IsRunning();
	if(IsRun)
	{
		SignalStop();
		MsgWaitForSingleObject(m_hThread, INFINITE);
		ATLTRACE(_T("StopAndWait() finished!!!\r\n"));
	}
	return IsRun;
}

void CThumbsView::SelectLastItem()
{
	int nItem = -1;
	do
	{
		nItem = GetNextItem(nItem, LVNI_SELECTED);
		if(nItem == -1) break;
		SetItemState(nItem, 0, LVIS_SELECTED	);
	}
	while(nItem != -1);
	SetItemState(GetItemCount() - 1, LVIS_SELECTED, LVIS_SELECTED	);
	EnsureVisible( GetItemCount() - 1, FALSE);

}