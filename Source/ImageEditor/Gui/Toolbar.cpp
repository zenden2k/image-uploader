#include "Toolbar.h"
#include <Core/Logging.h>
#include <GdiPlus.h>
#include <Gui/GuiTools.h>
#include <3rdpart/RoundRect.h>
#include "resource.h"
#include <Core/Images/Utils.h>
#include <Func/WinUtils.h>
namespace ImageEditor {

Toolbar::Toolbar(Toolbar::Orientation orientation)
{
	orientation_ = orientation;
	selectedItemIndex_ = -1;
	trackMouse_ = false;
	m_hWnd = 0;
	dropDownIcon_ = BitmapFromResource(GetModuleHandle(0), MAKEINTRESOURCE(IDB_DROPDOWNICONPNG),_T("PNG")); //(HICON)LoadImage(GetModuleHandle(0),  MAKEINTRESOURCE(IDI_DROPDOWN), IMAGE_ICON, 16,16,0);
	dpiScaleX = 1.0f;
	dpiScaleY = 1.0f;
	transparentColor_ = Color(255,50,56);
	font_ = 0;
}

Toolbar::~Toolbar()
{
	delete font_;
}

bool Toolbar::Create(HWND parent, bool child )
{
	RECT rc = {0, 0, 1,1};
	HWND wnd = TParent::Create(parent, rc, _T("test"), ( child ? WS_CHILD :WS_POPUP) ,child?0:( WS_EX_LAYERED|  WS_EX_NOACTIVATE) /*|WS_EX_TOOLWINDOW*/);
	if ( !wnd ) {
		LOG(ERROR) << WinUtils::GetLastErrorAsString();
		return false;
	}
	if ( child ) {
		transparentColor_ = Color(255,255,255);
	}
	LONG lStyle = ::GetWindowLong(m_hWnd, GWL_STYLE);
	//lStyle &= ~(WS_CAPTION /*| WS_THICKFRAME | WS_MINIMIZE | WS_MAXIMIZE | WS_SYSMENU*/);
	::SetWindowLong(m_hWnd, GWL_STYLE, lStyle);
	SetLayeredWindowAttributes(m_hWnd, RGB(transparentColor_.GetR(),transparentColor_.GetG(),transparentColor_.GetB()),0,LWA_COLORKEY);
	HDC hdc = GetDC();
	dpiScaleX = GetDeviceCaps(hdc, LOGPIXELSX) / 96.0f;
	dpiScaleY = GetDeviceCaps(hdc, LOGPIXELSY) / 96.0f;
	ReleaseDC(hdc);

	enum {kItemMargin = 3, kItemHorPadding = 5, kItemVertPadding = 3, kIconSize = 20};
	itemMargin_ = kItemMargin *dpiScaleX;
	LOG(INFO) << itemMargin_ << " dpi="<< dpiScaleX;
	itemHorPadding_ = kItemHorPadding * dpiScaleX;
	itemVertPadding_ = kItemVertPadding * dpiScaleY;
	iconSizeX_ = kIconSize * dpiScaleX;
	iconSizeY_ = kIconSize * dpiScaleY;
	font_ = new Gdiplus::Font(L"Arial",9, FontStyleRegular);
	return true;
}

int Toolbar::addButton(Item item)
{
	buttons_.push_back(item);
	return buttons_.size()-1;
}

int Toolbar::getItemAtPos(int clientX, int clientY)
{
	POINT pt = {clientX, clientY};
	for ( int i  = 0; i < buttons_.size(); i++ ) {
		if ( PtInRect(&buttons_[i].rect,pt ) ) {
			return i;
		}
	}
	return -1;
}

int Toolbar::getItemIndexByCommand(int command)
{
	for ( int i  = 0; i < buttons_.size(); i++ ) {
		if (buttons_[i].command == command ) {
			return i;
		}
	}
	return -1;
}

void Toolbar::repaintItem(int index)
{
	Item item = buttons_[index];
	InvalidateRect(&item.rect, false);
}

void Toolbar::clickButton(int index)
{
	if ( index < 0 || index >= buttons_.size() ) {
		LOG(ERROR) << "Out of range";
	}
	Item& item = buttons_[index];
	selectedItemIndex_ = index;
	if ( item.checkable ) {

		item.isChecked = item.group!=-1|| !item.isChecked;
		//item.state = item.isChecked ? isChecked : isNormal;
	} else {
		//item.state = isNormal;
	}
	if ( item.group != -1 ) {

		// Uncheck all other buttons with same group id
		for( int i = 0; i < buttons_.size(); i++ ) {
			//LOG(INFO) << "buttons_[i].group=" << buttons_[i].group;
			if ( i != index & buttons_[i].group == item.group && buttons_[i].checkable && buttons_[i].isChecked ) {
				buttons_[i].isChecked  = false;
				buttons_[i].state = isNormal;
				InvalidateRect(&buttons_[i].rect, FALSE);
			}
		}
	}

	InvalidateRect(&item.rect, FALSE);
	
}

LRESULT Toolbar::OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	RECT rc2;
	GetClientRect(&rc2);
	LOG(INFO) << rc2.right<< " " << rc2.bottom;
	return 0;
}

void DrawRoundedRectangle(Gdiplus::Graphics* gr, Gdiplus::Rect r, int d, Gdiplus::Pen* p, Gdiplus::Brush*br){
	using namespace Gdiplus;
	GraphicsPath gp;

	gp.AddArc(r.X, r.Y, d, d, 180, 90);
	gp.AddArc(r.X + r.Width - d, r.Y, d, d, 270, 90);
	gp.AddArc(r.X + r.Width - d, r.Y + r.Height - d, d, d, 0, 90);
	gp.AddArc(r.X, r.Y + r.Height - d, d, d, 90, 90);
	gp.AddLine(r.X, r.Y + r.Height - d, r.X, r.Y + d/2);
	if ( br ) {
		gr->FillPath(br, &gp);
	}
	gr->DrawPath(p, &gp);

}
LRESULT Toolbar::OnPaint(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled)
{
	//LOG(INFO) << "Toolbar WM_PAINT";
	using namespace Gdiplus;
	CPaintDC dc(m_hWnd);
	RECT clientRect;
	bHandled = true;
	GetClientRect(&clientRect);
	Gdiplus::Graphics gr(dc);
	gr.SetInterpolationMode(InterpolationModeHighQualityBicubic );
	gr.SetPageUnit(Gdiplus::UnitPixel);
	
	//*gr.SetPixelOffsetMode(PixelOffsetModeHalf);
	//dc.FillSolidRect(&clientRect, RGB(255,0,0));
	Rect rect( 0, 0, clientRect.right-1, clientRect.bottom-1);
	SolidBrush br1(transparentColor_);
	gr.FillRectangle(&br1, rect);
	LinearGradientBrush br (RectF(float(0), float(-0.5 ), float(clientRect.right),
		/*rect.top+*/ float(clientRect.bottom) ), Color(252,252,252), Color(
		200,200,200), orientation_ == orHorizontal ? LinearGradientModeVertical : LinearGradientModeHorizontal);
	//Gdiplus::SolidBrush br(Color(200,2,146,209));
	Pen p(Color(1,87,124));
	/*gr.FillRectangle(&br, rect);
	gr.DrawRectangle(&p, rect);*/
	/*CRoundRect roundRect;
	roundRect.FillRoundRect(&gr,&br,rect,Color(198,196,197),5);*/
	//roundRect.DrawRoundRect(&gr,rect,Color(198,196,197),8, 1);
	gr.SetSmoothingMode(SmoothingModeHighSpeed);

	DrawRoundedRectangle(&gr,rect,8,&p, &br);
	gr.SetSmoothingMode(SmoothingModeAntiAlias);


	//gr.FillRectangle(&br,rect);
	int x = itemMargin_;
	int y = itemMargin_;
	for ( int i =0; i < buttons_.size(); i++ ) {
		SIZE s = CalcItemSize(i);
		drawItem(i, &gr, x, y);

		if ( orientation_ == orHorizontal ) {
			x += s.cx + itemMargin_;
		} else {
			y += s.cy + itemMargin_;
		}
		//y+= s.cy;
	}

		//SetLayeredWindowAttributes(m_hWnd, RGB(255,255,255),0,LWA_COLORKEY);
	//RECT rct;
	//rgn.GetRgnBox( &rct );
	/*
	
	//
	//gr.DrawRo*/

	return 0;
}

LRESULT Toolbar::OnMouseMove(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& /*bHandled*/)
{
	int xPos = GET_X_LPARAM(lParam); 
	int yPos = GET_Y_LPARAM(lParam); 
	
	if(!trackMouse_) // Capturing mouse
	{
		TRACKMOUSEEVENT tme;
		tme.cbSize = sizeof(TRACKMOUSEEVENT);
		tme.dwFlags = TME_LEAVE;
		tme.hwndTrack = m_hWnd;
		::_TrackMouseEvent(&tme); // We want to receive WM_MOUSELEAVE message
		trackMouse_ = true;
	}

	int oldSelectedIndex  = selectedItemIndex_;
	
	selectedItemIndex_ = getItemAtPos(xPos, yPos);

	if (  oldSelectedIndex != selectedItemIndex_  ) {
		if ( selectedItemIndex_ != -1 ) {
			buttons_[selectedItemIndex_].state = isHover;
			//LOG(INFO) << "selectedItemIndex_=" << selectedItemIndex_;
			if ( selectedItemIndex_ != -1 ) {
				buttons_[selectedItemIndex_].state = isHover;
				InvalidateRect(&buttons_[selectedItemIndex_].rect, false);
				//Invalidate();
			}
		}
		if ( oldSelectedIndex != -1 ) {
			buttons_[oldSelectedIndex].state = isNormal;
			InvalidateRect(&buttons_[oldSelectedIndex].rect, false);
		}
	} 
		
	
	
	return 0;
}

LRESULT Toolbar::OnMouseLeave(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& /*bHandled*/)
{
	int xPos = GET_X_LPARAM(lParam); 
	int yPos = GET_Y_LPARAM(lParam); 
	//LOG(INFO) << "OnMouseLeave";
	trackMouse_ = false;
	if ( selectedItemIndex_ != -1 ) {
		buttons_[selectedItemIndex_].state = isNormal;
		
		InvalidateRect(&buttons_[selectedItemIndex_].rect, false);
		selectedItemIndex_ = -1;
		trackMouse_ = false;
	}

	return 0;
}

LRESULT Toolbar::OnLButtonDown(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	int xPos = GET_X_LPARAM(lParam); 
	int yPos = GET_Y_LPARAM(lParam); 
	if ( selectedItemIndex_ != -1 ) {
		Item& item = buttons_[selectedItemIndex_];
		if ( item.type == Toolbar::itComboButton && xPos >  item.rect.right - dropDownIcon_->GetWidth() - itemMargin_  ) {
			item.state = isDropDown;
		} else if ( item.type == Toolbar::itTinyCombo && xPos >  item.rect.right - 6*dpiScaleX - itemMargin_ && yPos >   item.rect.bottom - 6*dpiScaleY - itemMargin_  ) {
			item.state = isDropDown;
		}else {
			item.state = isDown;
		}
		
		InvalidateRect(&item.rect, false);
		
	}
	return 0;
}

LRESULT Toolbar::OnLButtonUp(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	int xPos = GET_X_LPARAM(lParam); 
	int yPos = GET_Y_LPARAM(lParam); 
	if ( selectedItemIndex_ != -1 ) {
		
		selectedItemIndex_ = getItemAtPos(xPos, yPos);
		Item& item = buttons_[selectedItemIndex_];

		clickButton(selectedItemIndex_);


		if ( item.itemDelegate ) {
			item.itemDelegate->OnClick(xPos, yPos, dpiScaleX, dpiScaleY);
		} else {
			HWND parent = GetParent();
			int command = item.command;
			::PostMessage(parent, WM_COMMAND, MAKEWPARAM(command,BN_CLICKED),(LPARAM)m_hWnd);
			selectedItemIndex_ = -1;
			OnMouseMove(WM_MOUSEMOVE, wParam, lParam, bHandled);
		}
	}
	return 0;
}

LRESULT Toolbar::OnEraseBackground(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	return 0;
}

SIZE Toolbar::CalcItemSize(int index)
{
	using namespace Gdiplus;
	SIZE res={0,0};
	Item &item = buttons_[index];
	
	if ( item.itemDelegate ) {
		return item.itemDelegate->CalcItemSize(item,dpiScaleX, dpiScaleY);
	}

	if (  item.title.GetLength()) {
		CWindowDC dc(m_hWnd);
		Gdiplus::Graphics gr(dc);
		PointF origin(0,0);
		RectF textBoundingBox;
		if (  gr.MeasureString(item.title, item.title.GetLength(), font_, origin, &textBoundingBox) == Ok ) {
			res.cx = textBoundingBox.Width;
			res.cy = textBoundingBox.Height;
		}
	}

	if ( item.icon ) {
		res.cx += iconSizeX_  + (item.title.IsEmpty() ? 0 :itemHorPadding_ ) ;
		res.cy = max(iconSizeY_, res.cy);
	}

	if ( item.type == itComboButton ) {
		res.cx += dropDownIcon_ ->GetWidth()*dpiScaleX + itemHorPadding_;
	}/*else if ( item.type == itTinyCombo ) {
		res.cx += 5*dpiScaleX;
	}*/

	res.cx += itemHorPadding_ * 2;
	res.cy  += itemVertPadding_ * 2;
	RECT rc;
	GetClientRect(&rc);
	if ( orientation_ == orVertical ) {
		res.cx =  max( res.cx, rc.right - itemMargin_*2);
	} else {
		res.cy =  max( res.cy, rc.bottom - itemMargin_*2);
	}
	return res;
}


int Toolbar::AutoSize()
{
	int x = itemMargin_;
	int y = itemMargin_;
	int width = 0;
	int height = 0;
	for ( int j = 0; j < 1; j ++ ) {
		x = itemMargin_;
		y = itemMargin_;
		for ( int i =0; i < buttons_.size(); i++ ) {
			SIZE s = CalcItemSize(i);
			Item& item = buttons_[i];
			RectF bounds(x, y, float(s.cx), float(s.cy));
			item.rect.left = x;
			item.rect.top = y;
			item.rect.right = s.cx + x;
			item.rect.bottom = s.cy + y;

			if ( orientation_ == orHorizontal ) {
				x+= s.cx + itemMargin_;
				width = x;
				height = max(s.cy + itemMargin_*2, height);
			} else {
				y+= s.cy + itemMargin_;
				height = y;
				width = max(s.cx+itemMargin_*2, width);
			}
		}
		if ( j == 0 ) {
			SetWindowPos(0, 0,0,width,height,SWP_NOMOVE);
		}
	}

	for ( int i = 0; i < buttons_.size(); i++ ) {
		CreateToolTipForItem(i);
	}
	//x += itemMargin_ * 2;
	
	return 1;
}

void Toolbar::drawItem(int itemIndex, Gdiplus::Graphics* gr, int x, int y)
{
	using namespace Gdiplus;
	SIZE size = CalcItemSize(itemIndex);
	
	Item& item = buttons_[itemIndex];

	if ( item.itemDelegate ) {
		item.itemDelegate->DrawItem(item, gr, x, y, dpiScaleX, dpiScaleY);
		return;
	}

	RectF bounds(x, y, float(size.cx), float(size.cy));
	item.rect.left = x;
	item.rect.top = y;
	item.rect.right = size.cx + x;
	item.rect.bottom = size.cy + y;
	SolidBrush brush(Color(0,0,0));
	
	if ( item.state == isHover ||  item.state == isDown ||  item.state == isDropDown || item.isChecked) {
			Pen p(Color(198,196,197));
			Color gradientColor1 = item.isChecked ?  Color(200,200,200) : Color(232,232,232);
			Color gradientColor2 = item.isChecked ? Color(140,140,140) : Color(170,170,170);
			LinearGradientBrush br (RectF(float(x), float(y ), float( x+size.cx),
				/*rect.top+*/ float(y+size.cy ) ), gradientColor1, gradientColor2, LinearGradientModeVertical);
			//LOG(INFO) << "item.isChecked " << item.isChecked;
		//	gr->FillRectangle( &brush, Rect(x, y, size.cx, size.cy));
			 //br.TranslateTransform(x,y);
			//br.SetWrapMode(WrapModeTile);
			CRoundRect roundRect;
			roundRect.FillRoundRect(gr,&br,Rect(x, y, size.cx, size.cy),Color(198,196,197),4);
			//roundRect.DrawRoundRect(gr,Rect(x, y, size.cx, size.cy),Color(198,196,197),7, 1);
			//DrawRoundedRectangle(gr,Rect(x, y, size.cx, size.cy),8,&p, 0);
			if ( item.type == itComboButton ) {
				//LOG(INFO) <<  "GetWidth "<< dropDownIcon_->GetWidth() << " " << dropDownIcon_->GetHeight();
				gr->DrawLine(&p, bounds.X + bounds.Width - dropDownIcon_->GetWidth()-3 ,  bounds.Y+1 , bounds.X + bounds.Width - dropDownIcon_->GetWidth()-3, bounds.Y + bounds.Height -1 );
			}
		
	} /*else if ( item.state == isChecked ) {
		Pen p(Color(198,196,197));
		Color gradientColor1 = Color(200,200,200);
		Color gradientColor2 = Color(140,140,140);
		LinearGradientBrush br (RectF(float(0), float(0.5 ), float( size.cx),
			/*rect.top+* float(size.cy ) ), gradientColor1, gradientColor2, LinearGradientModeVertical);
		//	gr->FillRectangle( &brush, Rect(x, y, size.cx, size.cy));
		//br.TranslateTransform(x,y);
		//br.SetWrapMode(WrapModeTile);
		/*CRoundRect roundRect;
		roundRect.FillRoundRect(gr,&br,Rect(x, y, size.cx, size.cy),Color(198,196,197),4);
		//roundRect.DrawRoundRect(gr,Rect(x, y, size.cx, size.cy),Color(198,196,197),7, 1);
		//DrawRoundedRectangle(gr,Rect(x, y, size.cx, size.cy),8,&p, 0);
		/*if ( item.type == itComboButton ) {
			//LOG(INFO) <<  "GetWidth "<< dropDownIcon_->GetWidth() << " " << dropDownIcon_->GetHeight();
			gr->DrawLine(&p, bounds.X + bounds.Width - dropDownIcon_->GetWidth()-3 ,  bounds.Y+1 , bounds.X + bounds.Width - dropDownIcon_->GetWidth()-3, bounds.Y + bounds.Height -1 );
		}*/

//	}

	if ( item.type == itComboButton ) {
		//LOG(INFO) <<  "GetWidth "<< dropDownIcon_->GetWidth() << " " << dropDownIcon_->GetHeight();
		gr->DrawImage(dropDownIcon_, bounds.X + bounds.Width - 16*dpiScaleX+ (item.state == isDropDown ? 1 : 0), bounds.Y + (bounds.Height -16*dpiScaleY )/2 + (item.state == isDropDown ? 1 : 0), (int)16*dpiScaleX, (int)16*dpiScaleY);
	} else if ( item.type == itTinyCombo ) {
		gr->DrawImage(dropDownIcon_, bounds.X + bounds.Width - 8*dpiScaleX+ (item.state == isDropDown ? 1 : 0), bounds.Y + bounds.Height - 8*dpiScaleY + (item.state == isDropDown ? 1 : 0), (int)10*dpiScaleX, (int)10*dpiScaleY);
	}

	if (  item.icon ) {
		gr->DrawImage(item.icon,(int) (itemHorPadding_ + bounds.X+ (item.state == isDown ? 1 : 0)), (int)(bounds.Y+ (item.state == isDown ? 1 : 0)+(bounds.Height -iconSizeY_)/2),iconSizeX_, iconSizeY_);

	}

	StringFormat format;
	format.SetAlignment(StringAlignmentCenter);
	format.SetLineAlignment(StringAlignmentCenter);
	gr->SetTextRenderingHint(TextRenderingHintClearTypeGridFit);
	
	int iconWidth = (item.icon ? iconSizeX_/*+ itemHorPadding_*/:0 ) ;
	int textOffsetX =  iconWidth+ itemHorPadding_;
	RectF textBounds( textOffsetX + bounds.X + (item.state == isDown ? 1 : 0), bounds.Y+ (item.state == isDown ? 1 : 0), bounds.Width - textOffsetX - (item.type == itComboButton ? 16*dpiScaleX : 0), bounds.Height);
	if ( orientation_ == orVertical ) {
		//LOG(INFO) << "textBounds x="<<textBounds.X << " y = " << textBounds.Y << "   "<<item.title;
	}
	gr->DrawString(item.title, -1, font_, textBounds, &format, &brush);
}


void Toolbar::CreateToolTipForItem(int index)
{
	Item& item = buttons_[index];
	if ( item.tooltipWnd ) {
		::DestroyWindow(item.tooltipWnd);
		item.tooltipWnd = 0;
	}
	if ( item.hint.IsEmpty() ) {
		return;
	}
	// Create a tooltip.
	HWND hwndTT = ::CreateWindowEx(WS_EX_TOPMOST, TOOLTIPS_CLASS, NULL, 
		WS_POPUP | TTS_NOPREFIX | TTS_ALWAYSTIP, 
		CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, 
		m_hWnd, NULL, _Module.GetModuleInstance(),NULL);

	::SetWindowPos(hwndTT, HWND_TOPMOST, 0, 0, 0, 0, 
		SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);

	// Set up "tool" information. In this case, the "tool" is the entire parent window.

	TOOLINFO ti = { 0 };
	ti.cbSize   = sizeof(TOOLINFO);
	ti.uFlags   = TTF_SUBCLASS;
	ti.hwnd     = m_hWnd;
	ti.hinst    = _Module.GetModuleInstance();
	TCHAR* textBuffer = new TCHAR[item.hint.GetLength()+1];
	lstrcpy(textBuffer, item.hint);
	ti.lpszText = textBuffer;
	ti.rect  = item.rect;

	// Associate the tooltip with the "tool" window.
	SendMessage(hwndTT, TTM_ADDTOOL, 0, (LPARAM) (LPTOOLINFO) &ti);	
	delete[] textBuffer;
} 

}