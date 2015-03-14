#include "Toolbar.h"
#include <Core/Logging.h>
#include <GdiPlus.h>
namespace ImageEditor {

Toolbar::Toolbar(Toolbar::Orientation orientation)
{
	orientation_ = orientation;
}

bool Toolbar::Create(HWND parent)
{
	RECT rc = {0, 0, 300,40};
	TParent::Create(parent, rc, _T("test"),WS_VISIBLE | /*WS_POPUPWINDOW*/WS_POPUP |WS_SYSMENU /*, WS_EX_LAYERED*/);
	if ( !m_hWnd ) {
		return false;
	}
	LONG lStyle = ::GetWindowLong(m_hWnd, GWL_STYLE);
	//lStyle &= ~(WS_CAPTION /*| WS_THICKFRAME | WS_MINIMIZE | WS_MAXIMIZE | WS_SYSMENU*/);
	::SetWindowLong(m_hWnd, GWL_STYLE, lStyle);
	SetLayeredWindowAttributes(m_hWnd, RGB(5,5,5),195,LWA_ALPHA);
	return true;
}

int Toolbar::addButton(Item item)
{
	buttons_.push_back(item);
	return buttons_.size()-1;
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
	gr->FillPath(br, &gp);
	gr->DrawPath(p, &gp);

}
LRESULT Toolbar::OnPaint(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled)
{
	using namespace Gdiplus;
	CPaintDC dc(m_hWnd);
	RECT clientRect;
	bHandled = true;
	GetClientRect(&clientRect);
	Gdiplus::Graphics gr(dc);
	//dc.FillSolidRect(&clientRect, RGB(255,0,0));
	Rect rect( 0, 0, clientRect.right-1, clientRect.bottom-1);
	Gdiplus::SolidBrush br(Color(200,2,146,209));
	Pen p(Color(1,87,124));
	gr.FillRectangle(&br, rect);
	gr.DrawRectangle(&p, rect);
	//DrawRoundedRectangle(&gr,rect,8,&p, &br);
		//gr.FillRectangle(&br,rect);
	//RECT rct;
	//rgn.GetRgnBox( &rct );
	/*
	
	//
	//gr.DrawRo*/

	return 0;
}

LRESULT Toolbar::OnMouseMove(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	return 0;
}

LRESULT Toolbar::OnLButtonDown(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	return 0;
}

LRESULT Toolbar::OnLButtonUp(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	return 0;
}

LRESULT Toolbar::OnEraseBackground(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	return 0;
}

SIZE Toolbar::CalcItemSize(int index)
{
	SIZE res;
	Item item = buttons_[index];
	CWindowDC dc(m_hWnd);
	dc.GetTextExtentExPoint(item.title, -1, &res, 0);
	return res;
}

}