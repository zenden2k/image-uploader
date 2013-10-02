/*
    Image Uploader - program for uploading images/files to Internet
    Copyright (C) 2007-2011 ZendeN <zenden2k@gmail.com>
	 
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

#include "HistoryTreeControl.h"
#include "Func/Myutils.h"
#include "Core/Utils/CoreUtils.h"
#include "Func/Common.h"
#include "Func/Base.h"
#include "Gui/GuiTools.h"

// CHistoryTreeControl
CHistoryTreeControl::CHistoryTreeControl()
{
	m_thumbWidth = 56;
	m_SessionItemHeight = 0;
	m_SubItemHeight = 0;
	m_FileDownloader = 0;
	downloading_enabled_ = true;
	m_bIsRunning = false;
}	

CHistoryTreeControl::~CHistoryTreeControl()
{
	CWindow::Detach();
}

void CHistoryTreeControl::Init()
{
}

void CHistoryTreeControl::CreateDownloader()
{
	//delete m_FileDownloader;
	//m_FileDownloader = 0;
	if(!m_FileDownloader)
	{
		m_FileDownloader = new CFileDownloader();
		m_FileDownloader->onConfigureNetworkManager.bind(this, &CHistoryTreeControl::OnConfigureNetworkManager);
		m_FileDownloader->onFileFinished.bind(this, &CHistoryTreeControl::OnFileFinished);
		m_FileDownloader->onQueueFinished.bind(this, &CHistoryTreeControl::QueueFinishedEvent);
	}
}

LRESULT CHistoryTreeControl::OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam,BOOL& bHandled)
{
	abortLoadingThreads();
	return 0;
}

void CHistoryTreeControl::abortLoadingThreads()
{
	if(IsRunning())
	{
		SignalStop();
	}
	if(m_FileDownloader && m_FileDownloader->IsRunning())
	{
		
		m_FileDownloader->stop();
		//m_FileDownloader->onFileFinished.clear();
		//m_FileDownloader->onQueueFinished.clear();
		/*if(!m_FileDownloader->waitForFinished(4000))
		{
			m_FileDownloader->kill();
			m_FileDownloader->waitForFinished(-1);
			delete m_FileDownloader;
			m_FileDownloader = 0;
		}*/
	}
	
	if(IsRunning())
	{
		SignalStop();
		/*if(!WaitForThread(2100))
		{
			Terminate();
			WaitForThread();
		}*/
	}
}

void CHistoryTreeControl::Clear()
{
	/*SetRedraw(false);

	int n = GetCount();
	for(int i= 0; i<n; i++)
	{
			HistoryTreeControlItem * item =(HistoryTreeControlItem *)GetItemDataPtr(i);
			delete item;
	}
	ResetContent();
	SetRedraw(true);*/
}

void CHistoryTreeControl::addSubEntry(TreeItem* res, HistoryItem it, bool autoExpand)
{
	HistoryTreeItem * it2 = new HistoryTreeItem();
	TreeItem *item = AddSubItem(Utf8ToWCstring(IuCoreUtils:: timeStampToString(it.timeStamp)+ " "+ it.localFilePath), res, it2, autoExpand);
	item->setCallback(this);
	it2->hi = it;
	it2->thumbnail = 0;
	it2->ThumbnailRequested = false;
}

DWORD CHistoryTreeControl::OnItemPrePaint(int /*idCtrl*/, LPNMCUSTOMDRAW lpNMCustomDraw)
{
	return 0;
}

DWORD CHistoryTreeControl::OnSubItemPrePaint(int /*idCtrl*/, LPNMCUSTOMDRAW /*lpNMCustomDraw*/)
{
	return CDRF_DODEFAULT;
}
HistoryItem* CHistoryTreeControl::getItemData(TreeItem* res)
{
	return reinterpret_cast<HistoryItem*> (res->userData());
}

HICON CHistoryTreeControl::getIconForExtension(const CString& ext)
{
	if(m_fileIconCache[ext] != 0)
	{
		return m_fileIconCache[ext];
	}
	HICON res = GetAssociatedIcon(ext, false);
	if(!res) return 0;
	m_fileIconCache[ext]=res;
	return res;
}

TreeItem*  CHistoryTreeControl::addEntry(CHistorySession* session, const CString& text)
{
	TreeItem *item = AddItem(text, session);
	return item;
}

void CHistoryTreeControl::_DrawItem(TreeItem* item, HDC hdc, DWORD itemState, RECT invRC, int *outHeight)
{
	int curY = 0;
	bool draw = !outHeight;

	bool isSelected = (itemState & CDIS_SELECTED) || (itemState&CDIS_FOCUS);
	CRect clientRect;
	GetClientRect(clientRect);
	//HistoryItem * it2 = new HistoryItem(it);
	CHistorySession* ses = reinterpret_cast<CHistorySession*>(item->userData());
	std::string label = "["+ IuCoreUtils::timeStampToString(ses->timeStamp()) +"]";
	std::string serverName = ses->serverName();
	if(ses->entriesCount())
	{
		serverName  = ses->entry(0).serverName;
	}
		if(serverName.empty()) serverName = "uknown server";
	std::string lowText = 
	serverName+ " (" + IuCoreUtils::toString(ses->entriesCount())+" files)"; 
	
	CString text = Utf8ToWCstring(label);
	
	CRect rc = invRC;
	CRect calcRect;

	CDC dc (hdc);
	CBrush br;
	
	dc.SetBkMode(TRANSPARENT);
	dc.SetTextColor(RGB(0,0,0));
	CBrush backgroundBrush;

	DWORD color=RGB(255,255,255);
	if(itemState & CDIS_SELECTED)
		color=0x9fd5ff;
	backgroundBrush.CreateSolidBrush(color);
	if(draw)
		dc.FillRect(&invRC, backgroundBrush);
	DrawText(dc.m_hDC, text, text.GetLength(), &calcRect, DT_CALCRECT);
	calcRect.OffsetRect(rc.left, rc.top);	

	curY += 1;

	if(draw)
	{
		CRect dateRect = rc;
		dateRect.right -= 20;
		//dateRect.OffsetRect(400,0);		
		dc.SetTextColor(0x909090);
		DrawText(dc.m_hDC, text, text.GetLength(), &dateRect, DT_RIGHT);
		dc.SetTextColor(0);
	}

	int curX= 0;

	RECT gradientLineRect = invRC;
	gradientLineRect.bottom--;
	gradientLineRect.top = gradientLineRect.bottom;
	if(draw) GuiTools::FillRectGradient(hdc, gradientLineRect,0xc8c8c8, 0xFFFFFF, true);
	
	calcRect = rc;
	DrawText(dc.m_hDC, Utf8ToWCstring(lowText), lowText.length(), &calcRect, DT_CALCRECT);
	
	if(draw)
	{
		bool isItemExpanded = item->IsExpanded();
			//(GetItemState(item,TVIS_EXPANDED)&TVIS_EXPANDED);	
		CRect plusIconRect;
		SIZE plusIconSize = {9,9};
		HTHEME theme = OpenThemeData(_T("treeview"));
		if(theme)
		{	
			GetThemePartSize(dc.m_hDC, TVP_GLYPH, isItemExpanded?GLPS_OPENED: GLPS_CLOSED, 0, TS_DRAW, &plusIconSize);
		}

		int iconOffsetX = 3;
		int iconOffsetY = (rc.Height() - plusIconSize.cy)/2;
		plusIconRect.left = rc.left + iconOffsetX;
		plusIconRect.top = rc.top + iconOffsetY;
		plusIconRect.right = plusIconRect.left + plusIconSize.cx;
		plusIconRect.bottom = plusIconRect.top + plusIconSize.cy;
		curX  += iconOffsetX + plusIconSize.cx;
		if(theme)
		{
			DrawThemeBackground( dc.m_hDC, TVP_GLYPH, isItemExpanded?GLPS_OPENED: GLPS_CLOSED,&plusIconRect);  // закрытая папка
			CloseThemeData();
		}
		else
		{
            CRect FrameRect(plusIconRect);
            dc.FillSolidRect(FrameRect, 0x808080);
            FrameRect.DeflateRect(1, 1, 1, 1);
            dc.FillSolidRect(FrameRect, 0xFFFFFF);

				CRect  MinusRect(2,4,7,5);
				MinusRect.OffsetRect(plusIconRect.TopLeft());
            dc.FillSolidRect(MinusRect, 0x000000);
            
            if (! isItemExpanded)
            {
                CRect PlusRect(4,2,5, 7);
				PlusRect.OffsetRect(plusIconRect.TopLeft());
                dc.FillSolidRect(PlusRect, 0x000000);
            }
		}
	}

	CRect drawRect;
	int serverIconOffsetY = (rc.Height()-1 - 16)/2;
	drawRect.top = rc.top + serverIconOffsetY;
	drawRect.bottom = drawRect.top + calcRect.Height();
	drawRect.left = rc.left + curX+4;
	drawRect.right = drawRect.left + calcRect.Width();
	HICON ico = getIconForServer(Utf8ToWCstring(ses->serverName()));
	if(ico)
	{
		dc.DrawIconEx(drawRect.left, drawRect.top, ico, 16, 16);
	}
	drawRect.OffsetRect(16 +3 , 0);;
	if(draw)
		DrawText(dc.m_hDC,  Utf8ToWCstring(lowText), lowText.length(), &drawRect, DT_LEFT|DT_VCENTER);

	curY += max(calcRect.Height(), /* server icon height */16);
	dc.Detach();
	curY += 3;
	if(outHeight)
		*outHeight = curY;
}

int CHistoryTreeControl::CalcItemHeight(TreeItem* item)
{
	bool isRootItem = (item->level()==0);
	if(isRootItem && m_SessionItemHeight)
	{
		return m_SessionItemHeight;
	}
	else if(!isRootItem && m_SubItemHeight)
	{
		return m_SubItemHeight;
	}

	int res = 0;
	HDC dc =  GetDC();
	RECT rc;
	GetItemRect(FindVisibleItemIndex(item), &rc);
	if( !isRootItem)
	{
		DrawSubItem(item,  dc, 0, rc, &res);
		m_SubItemHeight = res;
	}
	else 
	{
		_DrawItem(item,  dc, 0, rc, &res);
		m_SessionItemHeight = res;
	}

	ReleaseDC(dc);
	return res;
}

void DrawBitmap(HDC hdc, HBITMAP bmp, int x, int y)
{
	HDC hdcMem = CreateCompatibleDC(hdc);
	BITMAP bm;
	HBITMAP hbmOld = (HBITMAP)SelectObject(hdcMem, bmp);
	GetObject(bmp, sizeof(bm), &bm);
	BitBlt(hdc, x, y, bm.bmWidth, bm.bmHeight, hdcMem, 0, 0, SRCCOPY);
	SelectObject(hdcMem, hbmOld);
	DeleteDC(hdcMem);
}
void CHistoryTreeControl::DrawSubItem(TreeItem* item, HDC hdc, DWORD itemState, RECT invRC, int* outHeight)
{
		RECT rc = invRC;
		bool draw = !outHeight;
		CDC dc (hdc);
		CBrush br;
		if(draw)
		{
			dc.SetBkMode(TRANSPARENT);
			dc.SetTextColor(RGB(0,0,0));
		}

		int indent = 24;

		RECT fullRect = invRC;

		CBrush backgroundBrush;
		backgroundBrush.CreateSolidBrush(RGB(255, 255, 255));
		
		CBrush selectedBrush;
		selectedBrush.CreateSolidBrush(0x9fd5ff);
		
		if(draw)
		{
			CRect rc2 = rc;
			
			if(!(itemState & CDIS_SELECTED))
			{
				
				rc2.left-= CATEGORY_INDENT;
				dc.FillRect(&rc2, backgroundBrush);	
			}
			else
			{
				CRect rc3 = rc2;
				dc.FillRect(&rc3, selectedBrush);	
				rc3.left -= CATEGORY_INDENT;
				rc3.right = rc3.left + CATEGORY_INDENT;
				dc.FillRect(&rc3, backgroundBrush);	
			}
			//dc.FillRect(&rc2, backgroundBrush);
		}
		
		br.CreateSolidBrush(0x9fd5ff);
		RECT thumbRect;
		thumbRect.left = rc.left+10;
		thumbRect.top = rc.top+2;
		thumbRect.bottom = thumbRect.top + m_thumbWidth;
		thumbRect.right = thumbRect.left + m_thumbWidth;

		if(draw)
			dc.FrameRect(&thumbRect, br);
		HistoryItem * it2 = getItemData(item);
		std::string fileName = IuCoreUtils::ExtractFileName(it2->localFilePath);
		CString ext = Utf8ToWstring(IuCoreUtils::ExtractFileExt(fileName)).c_str();
		
		CString iconSourceFileName = Utf8ToWCstring(it2->localFilePath);
		if(iconSourceFileName.IsEmpty())
			iconSourceFileName = Utf8ToWCstring(it2->directUrl);
		HICON ico = getIconForExtension(iconSourceFileName);
		CString text = Utf8ToWstring(fileName).c_str();
		int iconWidth= 32;
		int iconHeight = 32;
		HistoryTreeItem * hti  = reinterpret_cast<HistoryTreeItem*>(item->userData());
		
		if(draw)
		{
			HBITMAP bm = GetItemThumbnail(hti);
			if(hti->thumbnail)
			{
				DrawBitmap(dc, hti->thumbnail,  thumbRect.left+1,thumbRect.top+1);
			}
			else if(ico!=0)
				dc.DrawIcon(thumbRect.left+1 + (m_thumbWidth-iconWidth)/2 , thumbRect.top+1+(m_thumbWidth-iconHeight)/2, ico);
		}

		CRect calcRect = invRC;
		calcRect.left = thumbRect.right+5;
		DrawText(dc.m_hDC, text, text.GetLength(), &calcRect, DT_CALCRECT);
		int filenameHeight = calcRect.Height();
		if(draw)
		DrawText(dc.m_hDC, text, text.GetLength(), &calcRect, DT_LEFT);
		
		CRect urlRect = invRC;
		urlRect.left = calcRect.left;
		urlRect.top += filenameHeight +3;

		CString url =  Utf8ToWCstring(it2->directUrl.length()?it2->directUrl:it2->viewUrl);
		dc.SetTextColor(0xa6a6a6);
		if(draw)
		DrawText(dc.m_hDC, url, url.GetLength(), &urlRect, DT_LEFT);
		
		dc.Detach();
		if(outHeight) *outHeight = m_thumbWidth+3;
}

HICON CHistoryTreeControl::getIconForServer(const CString& serverName)
{
	HICON ico = 0;
	
	if(m_serverIconCache[serverName]!=0)
		return m_serverIconCache[serverName];
	else

	ico = (HICON)LoadImage(0,IU_GetDataFolder()+_T("Favicons\\")+serverName+_T(".ico"),IMAGE_ICON	,16,16,LR_LOADFROMFILE);
	m_serverIconCache[serverName] = ico;
	return ico;		
}

LRESULT CHistoryTreeControl::OnLButtonDoubleClick(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	return 0;
}

LRESULT CHistoryTreeControl::OnLButtonDown(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	int xPos = LOWORD(lParam);  // horizontal position of cursor 
	int yPos = HIWORD(lParam);
	UINT flags = 0;
		if(xPos >50) xPos = 50;
	POINT pt = {xPos, yPos};
	bHandled = false;
	return 0;
}

LRESULT CHistoryTreeControl::ReflectContextMenu(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	return 0;
}

bool CHistoryTreeControl::IsItemAtPos(int x, int y, bool &isRoot)
{
	if(x > 50) x = 50;
	UINT flags = 0;
	POINT pt = {x, y};
	BOOL out;
	int index = ItemFromPoint(pt, out);
	if(out) return false;
	TreeItem* item = GetItem(index);

	isRoot = item->level()==0;
	return true;
}

void CHistoryTreeControl::DrawTreeItem(HDC dc, RECT rc, UINT itemState,  TreeItem *item)
{
	if( item->level()>0)
	{
		DrawSubItem(item,  dc, itemState, rc, 0);
	}
	else 
	{
		_DrawItem(item,  dc, itemState, rc, 0);
	}
}


void CHistoryTreeControl::TreeItemSize( TreeItem *item, SIZE *sz)
{
	int height = CalcItemHeight(item);
	sz->cy = height;
}

TreeItem * CHistoryTreeControl::selectedItem()
{
	int idx = GetCurSel();
	if( idx != -1 ) 
	{
		TreeItem* prop = GetItem(idx);
		return prop;
	}
	return 0;
}

#define THUMBNAIL_WIDTH 54
#define THUMBNAIL_HEIGHT 54
bool CHistoryTreeControl::LoadThumbnail(HistoryTreeItem * ItemID)
{
	using namespace Gdiplus;
	Bitmap *ImgBuffer=NULL;
	Image *bm=NULL;

	CString filename ;
	if(!ItemID->thumbnailSource.empty())
		filename = Utf8ToWCstring(ItemID->thumbnailSource); 
		
	else  
		filename = Utf8ToWCstring(ItemID->hi.localFilePath);
	bool error = false;
	if(IsImage(filename))
	{
		bm=new Image(filename);
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
//		if(imgwidth>maxwidth) maxwidth=imgwidth;
		//if(imgheight>maxheight) maxheight=imgheight;

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
				newheight=int((float)newwidth/imgwidth*imgheight);}
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
				newwidth=int((float)newheight/imgheight*imgwidth);
			}
		}
	}

	Graphics g(m_hWnd,true);
	ImgBuffer = new Bitmap(thumbwidth, thumbheight, &g);


	Graphics gr(ImgBuffer);
	gr.SetInterpolationMode(InterpolationModeHighQualityBicubic );
	gr.Clear(Color(255,255,255,255));

	RectF bounds(1, 1, float(width), float(height));

	if(bm && !bm->GetWidth() && ItemID>=0) 
	{
		error = true;
	}
	else 
	{
		LinearGradientBrush 
			br(bounds, Color(255, 255, 255, 255), Color(255, 210, 210, 210), 
			LinearGradientModeBackwardDiagonal/* LinearGradientModeVertical*/); 

		if(IsImage(filename))
			gr.FillRectangle(&br,1, 1, width-1,height-1);
		gr.SetInterpolationMode(InterpolationModeHighQualityBicubic );

		if(bm)
				gr.DrawImage(/*backBuffer*/bm, (int)((width-newwidth)/2)+1, (int)((height-newheight)/2), (int)newwidth,(int)newheight);
	

		RectF bounds(0, float(height), float(width), float(20));

//		if (ExtendedView)
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
			LPCTSTR Filename = filename;
			WCHAR Buffer[256];
			int f = MyGetFileSize(filename);
			WCHAR buf2[25];
			NewBytesToString(f, buf2, 25);
			WCHAR FileExt[25];
			lstrcpy(FileExt, GetFileExt(Filename));
			if(!lstrcmpi(FileExt, _T("jpg"))) 
				lstrcpy(FileExt,_T("JPEG"));
			if(IsImage(filename) && bm)
			{
				wsprintf(Buffer,_T("%s %dx%d (%s)"),(LPCTSTR)FileExt,(int)bm->GetWidth(),(int)bm->GetHeight(), (LPCTSTR)buf2 );
			}
			else
			{
				wsprintf(Buffer,_T("%s"), (LPCTSTR)buf2 );

			}
			gr.DrawString(Buffer, -1, &font, bounds, &format, &brush);
		}
	}

	HBITMAP bmp=NULL;
	ImgBuffer->GetHBITMAP(Color(255,255,255), &bmp);
	ItemID->thumbnail = error?0:bmp;
	delete bm;
	if(ImgBuffer) delete ImgBuffer;
	return !error;
}

HBITMAP CHistoryTreeControl::GetItemThumbnail(HistoryTreeItem* item)
{
	if(item->thumbnail!=0)
		return item->thumbnail;
	if(m_bStopped) return 0;
	if(item->ThumbnailRequested) return 0;

	item->ThumbnailRequested = true;

	
	std::string stdLocalFileName = item->hi.localFilePath;
	CString localFileName = Utf8ToWCstring(stdLocalFileName);
	if(!IsImage(localFileName))
	{
		return 0;
	}

	if(IuCoreUtils::FileExists(stdLocalFileName))
	{
		m_thumbLoadingQueue.push_back(item);
		StartLoadingThumbnails();
	}
	else
	{
		DownloadThumb(item);
	}
	return 0;
}

void CHistoryTreeControl::DownloadThumb(HistoryTreeItem * it)
{
	if(m_bStopped) return;
	if(it->thumbnailSource.empty())
	{
		std::string thumbUrl = it->hi.thumbUrl;
		if(thumbUrl.empty()) return ;
		std::string cacheFile = 	ZBase::get()->getFromCache(thumbUrl);
		if(!cacheFile.empty() && IuCoreUtils::FileExists(cacheFile))
		{
			it->thumbnailSource = cacheFile;
			m_thumbLoadingQueue.push_back(it);
			StartLoadingThumbnails();
			return;
		}
		if(downloading_enabled_)
		{
			CreateDownloader();
			m_FileDownloader->AddFile(thumbUrl, it);
			if(onThreadsStarted)	
				onThreadsStarted();
			m_FileDownloader->start();
		}
	}
}
DWORD CHistoryTreeControl::Run()
{
	while(!m_thumbLoadingQueue.empty())
	{
		if(m_bStopped) break;
		HistoryTreeItem * it = m_thumbLoadingQueue.front();
		m_thumbLoadingQueue.pop_front();
		if(!LoadThumbnail(it) && it->thumbnailSource.empty())
		{
			// Try downloading it
			DownloadThumb(it);	
		}
		else
		{
			Invalidate();
		}
	}
	m_bIsRunning = false;
	if(!m_FileDownloader || !m_FileDownloader->IsRunning()) 
		threadsFinished();
	return 0;
}

void CHistoryTreeControl::StartLoadingThumbnails()
{
	if(!IsRunning())
	{
		if(onThreadsStarted)	
			onThreadsStarted();
		m_bIsRunning = true;
		this->Start();
	}
}

bool CHistoryTreeControl::OnFileFinished(bool ok, CFileDownloader::DownloadFileListItem it)
{
	if(ok && !it.fileName.empty())
	{
		HistoryTreeItem * hit = reinterpret_cast<HistoryTreeItem*> (it.id);
		if(!hit)
		{
			//MessageBox(Utf8ToWCstring(it.url));
		}
		hit->thumbnailSource = it.fileName;
		ZBase::get()->addToGlobalCache(it.fileName, it.url);
		m_thumbLoadingQueue.push_back(hit);
		StartLoadingThumbnails();
	}
	return true;
}

void CHistoryTreeControl::OnTreeItemDelete(TreeItem* item)
{
	HistoryTreeItem* hti = reinterpret_cast<HistoryTreeItem*> (item->userData());
	HBITMAP bm = hti->thumbnail;
	if(bm) DeleteObject(bm);
	delete hti;
}

void CHistoryTreeControl::threadsFinished()
{
	m_thumbLoadingQueue.clear();
	if(onThreadsFinished)
	onThreadsFinished();
}

void CHistoryTreeControl::QueueFinishedEvent()
{
	if(!IsRunning())
		threadsFinished();
}

bool CHistoryTreeControl::isRunning() const
{
	return (m_bIsRunning || (m_FileDownloader && m_FileDownloader->IsRunning()) );
}

void CHistoryTreeControl::setDownloadingEnabled(bool enabled)
{
	downloading_enabled_ = enabled;
}

void CHistoryTreeControl::ResetContent()
{
	if(m_bIsRunning || (m_FileDownloader && m_FileDownloader->IsRunning()))
	{
		//MessageBox(_T("Cannot reset list while threads are still running!"));
		return;
	}
	CCustomTreeControlImpl<CHistoryTreeControl>::ResetContent();
}

void CHistoryTreeControl::OnConfigureNetworkManager(NetworkManager* nm)
{
	//MessageBox(_T("OnConfigureNetworkManager"),0,0);
	IU_ConfigureProxy(*nm);
}