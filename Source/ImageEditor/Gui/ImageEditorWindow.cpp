// MainFrm.cpp : implmentation of the CMainFrame class
//
/////////////////////////////////////////////////////////////////////////////


#include "ImageEditorWindow.h"

#include "ImageEditor/resource.h"
#include "ImageEditorView.h"
#include "ImageEditor/BasicElements.h"

LRESULT ImageEditorWindow::OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{

	RECT rc = {0,0,1024,720};
	GetClientRect(&rc);
	HWND m_hWndClient = m_view.Create(m_hWnd, rc, _T("ImageEditor_Canvas"), WS_CHILD | WS_VISIBLE /*| WS_CLIPSIBLINGS | WS_CLIPCHILDREN*/, 0 );
		
		//Create(m_hWnd, rcDefault, NULL, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN, WS_EX_CLIENTEDGE);

	currentDoc_ = new ImageEditor::Document(L"screenshot.png");

	
	//ImageEditor::Line line( 0, 0, 50, 50 );
	//line.resize( Gdiplus::Rect( 0, 0, 50, 50 ) );
	//currentDoc_->addDrawingElement( &line );
	ImageEditor::Canvas *canvas = new ImageEditor::Canvas( m_hWnd );
	canvas->setSize( currentDoc_->getWidth(), currentDoc_->getHeight());
	canvas->setDocument( currentDoc_ );

	m_view.setCanvas( canvas );
	canvas->setDrawingToolType(ImageEditor::Canvas::dtCrop);
	// register object for message filtering and idle updates
	CMessageLoop* pLoop = _Module.GetMessageLoop();
	ATLASSERT(pLoop != NULL);
	//pLoop->AddMessageFilter(this);
	//pLoop->AddIdleHandler(this);

	return 0;
}

LRESULT ImageEditorWindow::OnDestroy(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled)
{
	// unregister message filtering and idle updates


	bHandled = FALSE;
	return 1;
}

LRESULT ImageEditorWindow::OnClose(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled)
{
	PostQuitMessage(0);
	return 0;
}

LRESULT ImageEditorWindow::OnFileExit(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	PostMessage(WM_CLOSE);
	return 0;
}

LRESULT ImageEditorWindow::OnFileNew(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	// TODO: add code to initialize document

	return 0;
}

LRESULT ImageEditorWindow::OnViewToolBar(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	/*BOOL bVisible = !::IsWindowVisible(m_hWndToolBar);
	::ShowWindow(m_hWndToolBar, bVisible ? SW_SHOWNOACTIVATE : SW_HIDE);
	UISetCheck(ID_VIEW_TOOLBAR, bVisible);
	UpdateLayout();*/
	return 0;
}

LRESULT ImageEditorWindow::OnAppAbout(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	return 0;
}
