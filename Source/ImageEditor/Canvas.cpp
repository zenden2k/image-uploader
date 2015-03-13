#include "Canvas.h"

#include <cassert>
#include <algorithm>
#include <GdiPlus.h>
#include "DrawingElement.h"
#include "Document.h"
#include "BasicElements.h"
#include "DrawingTool.h"
#include "InputBox.h"
#include "Gui/InputBoxControl.h"
#include <Core/Logging.h>


namespace ImageEditor {
	
Canvas::Canvas( HWND parent ) {
	parentWindow_		    = parent;
	oldPoint_.x           = -1;
	oldPoint_.y           = -1;
	callback_             = 0;
	penSize_              = 1;
	drawingToolType_	    = dtPen;
	leftMouseDownPoint_.x = -1;
	leftMouseDownPoint_.y = -1;
	buffer_               = NULL;
	inputBox_             = NULL;
	currentCursor_    = ctDefault;
	createDoubleBuffer();
}

Canvas::~Canvas() {
	delete buffer_;
}

void Canvas::setSize( int x, int y ) {
	if ( x < 1 || x > 10000) {
		x = 1;
	}
	if ( y <1 || y > 10000) {
		y = 1;
	}
	canvasWidth_ = x;
	canvasHeight_ = y;
	createDoubleBuffer();
}

Document* Canvas::currentDocument() const {
	return doc_;
}

void Canvas::setDocument( Document *doc ) {
	doc_ = doc;
}

void Canvas::mouseMove( int x, int y, DWORD flags) {
	bool isLButtonDown = flags & MK_LBUTTON;
	POINT point        = { x, y };
	if ( x > canvasWidth_ || y > canvasHeight_ || x <0 || y <0) {
		return;
	}
	if ( isLButtonDown  ) {
		assert( currentDrawingTool_ );
		currentDrawingTool_->continueDraw( x,  y, flags );
	}
	CursorType ct = currentDrawingTool_->getCursor();
	setCursor(ct);
	/*if (isLButtonDown && currentElement_ != NULL ) {
		AffectedSegments prevSegments;
		currentElement_->getAffectedSegments( &prevSegments );
		CRgn prevRegion = prevSegments.createRegionFromSegments();
		currentElement_->setEndPoint( point );
		AffectedSegments imageSegments;
		currentElement_->getAffectedSegments( &imageSegments );
		CRgn affectedRegion = imageSegments.createRegionFromSegments();
		affectedRegion.CombineRgn( prevRegion, RGN_OR );
		updateView( affectedRegion );
	}
	*/

	/*if ( isLButtonDown && oldPoint_.x != -1 && currentDrawingTool_ == dtPen ) {
		
		int xStart = oldPoint_.x;
		int yStart = oldPoint_.y;
		int xEnd   = point.x;
		int yEnd   = point.y;

		ImageEditor::Line line( xStart, yStart, xEnd, yEnd );
		doc_->addDrawingElement( &line );
		RECT updateRect =  { std::min<>( xStart, xEnd), std::min<>( yStart, yEnd ), 
									std::max<>( xStart, xEnd) + 1, std::max<>( yStart, yEnd) + 1 };
		updateView( updateRect );
		
	} else if ( currentDrawingTool_ == dtLine ) {

	}*/
	oldPoint_ = point;
}

void Canvas::mouseDown( int button, int x, int y ) {
	leftMouseDownPoint_.x = x;
	leftMouseDownPoint_.y = y;
	assert( currentDrawingTool_ );
	currentDrawingTool_->beginDraw( x, y );
	/*if ( currentElement_ == NULL) {
		if ( currentDrawingTool_ == dtLine) {
			currentElement_ = new Line(x, y, x, y);
		} else if ( currentDrawingTool_ == dtRectangle ) {
			currentElement_ = new Rectangle(x, y, x, y);
		}
	}*/
}

void Canvas::mouseUp( int button, int x, int y ) {
	assert( currentDrawingTool_ );
	currentDrawingTool_->endDraw( x, y );
	/*if (currentDrawingTool_ != dtPen ) {
		if ( currentElement_ != NULL) {
			doc_->addDrawingElement( currentElement_ );
		}
		currentElement_ = NULL;
		updateView();
	}
	currentElement_ = 0;*/
}

void Canvas::render(Gdiplus::Graphics* gr, const RECT& rect) {
	Gdiplus::Graphics bufferedGr( buffer_ );
	//gr->SetClip( Gdiplus::Rect( rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top ) );
	//bufferedGr.SetClip( region );
	doc_->render( &bufferedGr );

	if ( currentDrawingTool_ != NULL ) {
		currentDrawingTool_->render( &bufferedGr );
	}

	for ( int i=0; i< elementsOnCanvas_.size(); i++) {
		elementsOnCanvas_[i]->render(&bufferedGr);
	}

	gr->DrawImage( buffer_, 0, 0 );
}

void Canvas::updateView( RECT boundingRect ) {
	CRgn region;
	region.CreateRectRgnIndirect( &boundingRect );
	updateView( region );
}

void  Canvas::setCallback(Callback * callback) {
	callback_ = callback;
}

void Canvas::setPenSize(int size) {
	penSize_ = size;
}

void Canvas::setDrawingToolType(DrawingToolType toolType) {
	drawingToolType_ = toolType;

	VectorElementTool::ElementType type;

	if ( toolType == dtPen) {
		currentDrawingTool_ = new PenTool( this );
	} else if ( toolType == dtBrush) {
		currentDrawingTool_ = new BrushTool( this );
	} else if ( toolType == dtText) {
		currentDrawingTool_ = new TextTool( this );
	} else {
		
		if ( toolType == dtLine )
			type = VectorElementTool::etLine;
		else if ( toolType == dtRectangle ) {
			type = VectorElementTool::etRectangle;
		} else {
			 MovableElement::ElementType type;
		   if ( toolType == dtCrop ) {
			  
				type = MovableElement::etCrop;
			}

		   currentDrawingTool_ = new MovableElementTool( this, type );

			return;
		}
		
		currentDrawingTool_ = new VectorElementTool( this, type );
	}
	
}

void Canvas::addMovableElement(MovableElement* element)
{
	elementsOnCanvas_.push_back(element);
}

void Canvas::deleteMovableElement(MovableElement* element)
{

}

void Canvas::updateView() {
	RECT rc = { 0, 0, 1280, 720 };
	updateView( rc );
}

void Canvas::updateView( const CRgn& region ) {
	if ( callback_ ) {
		callback_->updateView( this, region );
	}
}

void Canvas::createDoubleBuffer() {
	delete buffer_;
	buffer_ = new Gdiplus::Bitmap( canvasWidth_, canvasHeight_ );
}

void Canvas::setCursor(CursorType cursorType)
{
	if ( currentCursor_ == cursorType ) {
		return;
	}

	currentCursor_ = cursorType ;
}


void Canvas::getElementsByType(MovableElement::ElementType elementType, std::vector<MovableElement*>& out)
{
	int count = elementsOnCanvas_.size();
	for ( int i = 0; i < count; i++ ) {
		if ( elementsOnCanvas_[i]->getType() == elementType ) {
			out.push_back(elementsOnCanvas_[i]);
		}
	}
}

Canvas::CursorType Canvas::getCursor() const
{
	return currentCursor_;
}

bool Canvas::undo() {
	if ( !doc_ ) {
		return false;
	}
	bool result = doc_->undo();
	updateView();
	return result;
}

InputBox* Canvas::getInputBox( const RECT& rect ) {
	if ( inputBox_ == NULL ) {
		inputBox_ = new InputBoxControl();
		RECT rc = rect;
		HWND wnd = inputBox_->Create( parentWindow_, rc, _T("ololo"), WS_VISIBLE | WS_CHILD | WS_BORDER|ES_MULTILINE|ES_AUTOHSCROLL|ES_AUTOVSCROLL|  ES_WANTRETURN
			|  ES_NOHIDESEL | ES_LEFT  );
		inputBox_->SetWindowPos(HWND_TOP,0,0,0,0, SWP_NOSIZE|SWP_NOMOVE);
		
	} else {
		inputBox_->SetWindowPos(HWND_TOP, rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top , 0);
		inputBox_->ShowWindow( SW_SHOW );
	}
	inputBox_->SetWindowText( _T("") );
	inputBox_->SetFocus();
	//DebugWrite2( _T( "%d ,parent=%d, bounds = ( %d %d %d %d )" ), wnd, parentWindow_, rect);
	//MessageBox ( 0, 0, 0, 0);
	return inputBox_;
}

}
