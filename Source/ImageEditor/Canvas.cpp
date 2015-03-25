#include "Canvas.h"


#include "DrawingElement.h"
#include "Document.h"
#include "DrawingTool.h"
#include "InputBox.h"
#include "MovableElements.h"
#include "Gui/InputBoxControl.h"
#include <Core/Logging.h>
#include <math.h>
#include <cassert>
#include <algorithm>
#include <GdiPlus.h>

namespace ImageEditor {
	
Canvas::Canvas( HWND parent ) {
	parentWindow_		    = parent;
	oldPoint_.x           = -1;
	oldPoint_.y           = -1;
	callback_             = 0;
	drawingToolType_	  = dtNone;
	previousDrawingTool_ = dtNone; 
	leftMouseDownPoint_.x = -1;
	leftMouseDownPoint_.y = -1;
//	buffer_               = NULL;
	inputBox_             = NULL;
	currentCursor_    = ctDefault;
	scrollOffset_.x = 0;
	scrollOffset_.y = 0;
	overlay_ = 0;
	showOverlay_ = false;
	zoomFactor_ = 1;
	currentlyEditedTextElement_ = 0;
	foregroundColor_ = Gdiplus::Color(255,0,0);
	backgroundColor_ = Gdiplus::Color(255,255,255);
	penSize_ = 12;
	selection_ = 0;
	canvasChanged_ = true;
	fullRender_ = true;
	blurRadius_ = 5;
	blurRectanglesCount_ = 0;
	currentDrawingTool_ = 0;
	bufferedGr_ = 0;
	memset(&font_, 0, sizeof(font_));
	createDoubleBuffer();
}

Canvas::~Canvas() {
//	delete buffer_;
	delete bufferedGr_;
	delete currentDrawingTool_;
	delete overlay_;
	for ( int i = 0; i < elementsToDelete_.size(); i++ ) {
		delete elementsToDelete_[i];
	}
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
	updateView();
}

void Canvas::mouseMove( int x, int y, DWORD flags) {
	bool isLButtonDown = flags & MK_LBUTTON;
	POINT point        = { x, y };
	/*if ( x > canvasWidth_ || y > canvasHeight_ || x <0 || y <0) {
		return;
	}*/
	if ( isLButtonDown  ) {
		assert( currentDrawingTool_ );
		currentDrawingTool_->continueDraw( x,  y, flags );
	}
	CursorType ct = currentDrawingTool_->getCursor(x ,y);
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
	if ( !currentDrawingTool_ ) {
		return;
	}
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
	if ( !currentDrawingTool_ ) {
		return;
	}
	if ( button == 0 ) {
		currentDrawingTool_->endDraw( x, y );
	} else {
		currentDrawingTool_->rightButtonClick(x,y);
	}
	/*if (currentDrawingTool_ != dtPen ) {
		if ( currentElement_ != NULL) {
			doc_->addDrawingElement( currentElement_ );
		}
		currentElement_ = NULL;
		updateView();
	}
	currentElement_ = 0;*/
}

void Canvas::mouseDoubleClick(int button, int x, int y)
{
	if ( !currentDrawingTool_ ) {
		return;
	}

	currentDrawingTool_->mouseDoubleClick( x, y );
}

void Canvas::render(HDC dc, const RECT& rectInWindowCoordinates, POINT scrollOffset, SIZE size) { 
	using namespace Gdiplus;
	scrollOffset_ = scrollOffset;
	// Updating rect in canvas coordinates
	RECT rect = {rectInWindowCoordinates.left+scrollOffset.x, rectInWindowCoordinates.top+scrollOffset.y,
		/*size.cx*/rectInWindowCoordinates.right - rectInWindowCoordinates.left, /*size.cy*/rectInWindowCoordinates.bottom - rectInWindowCoordinates.top};
	rect.right += rect.left;
	rect.bottom += rect.top;

	if ( fullRender_ ) {
		rect.left = 0;
		rect.right = 0;
		rect.bottom = getHeigth();
		rect.right = getWidth();
	}
	if ( canvasChanged_ || fullRender_ ) {
		//LOG(INFO) << "Canvas::re-render rect"<< rect.left << " " << rect.top<< " "<< rect.right - rect.left<< " "<< rect.bottom - rect.top;
		renderInBuffer(updatedRect_);	
	}
	BitmapData bitmapData;
	Rect lockRect(0,0,  getWidth(),getHeigth());
	// I hope Gdiplus does not copy data in LockBits
	if ( buffer_->LockBits(&lockRect, ImageLockModeRead, PixelFormat32bppARGB, &bitmapData) == Ok ) {
		//LOG(INFO) << "bitmap locked";
		uint8_t * source = (uint8_t *) bitmapData.Scan0;
		unsigned int stride;
		if ( bitmapData.Stride > 0) { 
			stride = bitmapData.Stride;
		} else {
			stride = - bitmapData.Stride;
		}
		BITMAPINFO bi;
		ZeroMemory(&bi, sizeof(bi));
		bi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
		bi.bmiHeader.biPlanes = 1;
		bi.bmiHeader.biCompression = BI_RGB;

		bi.bmiHeader.biWidth = buffer_->GetWidth();
		bi.bmiHeader.biHeight = -buffer_->GetHeight();
		bi.bmiHeader.biBitCount = 32;
		// Faster than Graphics::DrawImage and there is no tearing!
		int res = SetDIBitsToDevice (dc, rectInWindowCoordinates.left,rectInWindowCoordinates.top,rect.right - rect.left, rect.bottom - rect.top, rect.left, rect.top, rect.top, rect.bottom - rect.top, source + rect.top * stride, &bi, DIB_RGB_COLORS );
		buffer_->UnlockBits(&bitmapData);
	}
	//gr->DrawImage( &*buffer_, rectInWindowCoordinates.left, rectInWindowCoordinates.top, rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top, UnitPixel );

}

Gdiplus::Rect Canvas::currentRenderingRect()
{
	return currentRenderingRect_;
}

void  Canvas::setCallback(Callback * callback) {
	callback_ = callback;
}

void Canvas::setPenSize(int size) {
	if ( size < 1 || size > kMaxPenSize ) {
		return;
	}
	penSize_ = size;
	if ( currentDrawingTool_ ) {
		currentDrawingTool_->setPenSize(size);
	}
	for ( int i =0; i < elementsOnCanvas_.size(); i++ ) {
		if ( elementsOnCanvas_[i]->isSelected()) {
			RECT paintRect = elementsOnCanvas_[i]->getPaintBoundingRect();
			elementsOnCanvas_[i]->setPenSize(size);
			RECT newPaintRect = elementsOnCanvas_[i]->getPaintBoundingRect();
			UnionRect(&paintRect, &paintRect, &newPaintRect);
			updateView(paintRect);
		}
	}

}

int Canvas::getPenSize() const
{
	return penSize_;
}

void Canvas::beginPenSizeChanging()
{
	if ( !originalPenSize_ ) {
		originalPenSize_ = penSize_;
	}
}

void Canvas::endPenSizeChanging(int penSize) {
	penSize_ = penSize_;
	if ( originalPenSize_ == 0 || originalPenSize_ == penSize_ ) {
		return ;
	}
	int updatedElementsCount = 0;
	UndoHistoryItem uhi;
	uhi.type = uitPenSizeChanged;
	for ( int i =0; i < elementsOnCanvas_.size(); i++ ) {
		if ( elementsOnCanvas_[i]->isSelected()) {
			UndoHistoryItemElement uhie;
			uhie.penSize = originalPenSize_;
			RECT paintRect = elementsOnCanvas_[i]->getPaintBoundingRect();
			uhie.movableElement = elementsOnCanvas_[i];
			elementsOnCanvas_[i]->setPenSize(penSize);
			RECT newPaintRect = elementsOnCanvas_[i]->getPaintBoundingRect();
			uhi.elements.push_back(uhie);
			UnionRect(&paintRect, &paintRect, &newPaintRect);
			updatedElementsCount++;
			updateView(paintRect);
		}
	}
	if ( updatedElementsCount ) {
		undoHistory_.push(uhi);
	}
	originalPenSize_= 0;
}

void Canvas::setForegroundColor(Gdiplus::Color color)
{
	foregroundColor_ = color;
	if ( currentDrawingTool_ ) {
		currentDrawingTool_->setForegroundColor(color);
	}
	int updatedElementsCount = 0;
	UndoHistoryItem uhi;
	uhi.type = uitElementForegroundColorChanged;
	for ( int i =0; i < elementsOnCanvas_.size(); i++ ) {
		if ( elementsOnCanvas_[i]->isSelected()) {
			UndoHistoryItemElement uhie;
			uhie.color = elementsOnCanvas_[i]->getColor();
			uhie.pos = i;
			uhie.movableElement = elementsOnCanvas_[i];
			elementsOnCanvas_[i]->setColor(color);
			if ( elementsOnCanvas_[i]->getType() != etText ) { // TextElements saves it's color by itself
				uhi.elements.push_back(uhie);
				updatedElementsCount++;
			}
		}
	}
	if ( updatedElementsCount ) {
		undoHistory_.push(uhi);
		updateView();
	}
}

void Canvas::setBackgroundColor(Gdiplus::Color color)
{
	backgroundColor_ = color;
	if ( currentDrawingTool_ ) {
		currentDrawingTool_->setBackgroundColor(color);
	}
	int updatedElementsCount = 0;
	UndoHistoryItem uhi;
	uhi.type = uitElementBackgroundColorChanged;
	for ( int i =0; i < elementsOnCanvas_.size(); i++ ) {
		if ( elementsOnCanvas_[i]->isSelected()) {
			UndoHistoryItemElement uhie;
			uhie.color = elementsOnCanvas_[i]->getBackgroundColor();
			uhie.pos = i;
			uhie.movableElement = elementsOnCanvas_[i];
			elementsOnCanvas_[i]->setBackgroundColor(color);
			uhi.elements.push_back(uhie);
			updatedElementsCount++;
		}
	}
	if ( updatedElementsCount ) {
		undoHistory_.push(uhi);
		updateView();
	}
}

Gdiplus::Color Canvas::getForegroundColor() const
{
	return foregroundColor_;
}

Gdiplus::Color Canvas::getBackgroundColor() const
{
	return backgroundColor_;
}

void Canvas::setFont(LOGFONT font, DWORD changeMask)
{
	font_ = font;
	/*if ( currentDrawingTool_ ) {
		currentDrawingTool_->setForegroundColor(color);
	}*/
	int updatedElementsCount = 0;
/*	UndoHistoryItem uhi;
	uhi.type = uitFontChanged;*/
	for ( int i =0; i < elementsOnCanvas_.size(); i++ ) {
		if ( elementsOnCanvas_[i]->isSelected() && elementsOnCanvas_[i]->getType() == etText ) {
			/*UndoHistoryItemElement uhie;
			uhie.color = elementsOnCanvas_[i]->getColor();
			uhie.pos = i;
			uhie.movableElement = elementsOnCanvas_[i];*/
			static_cast<TextElement*>(elementsOnCanvas_[i])->setFont(font, changeMask);
			/*uhi.elements.push_back(uhie);
			updatedElementsCount++;*/
		}
	}
	/*if ( updatedElementsCount ) {
		undoHistory_.push(uhi);
		updateView();
	}*/
}

LOGFONT Canvas::getFont()
{
	return font_;
}

AbstractDrawingTool* Canvas::setDrawingToolType(DrawingToolType toolType, bool notify ) {
	previousDrawingTool_ = drawingToolType_;
	drawingToolType_ = toolType;
	unselectAllElements();
	updateView();
	ElementType type;
	delete currentDrawingTool_;
	currentDrawingTool_ = 0;
	if ( toolType == dtPen) {
		currentDrawingTool_ = new PenTool( this );
	} else if ( toolType == dtBrush) {
		currentDrawingTool_ = new BrushTool( this );
	} else if ( toolType == dtMarker) {
		currentDrawingTool_ = new MarkerTool( this );
	}else if ( toolType == dtBlur) {
		#if GDIPVER >= 0x0110 
		currentDrawingTool_ = new BlurTool( this );
		#else
		LOG(ERROR) << "Blur effect is not supported by current version of GdiPlus.";
		#endif
	}else if ( toolType == dtColorPicker) {
		currentDrawingTool_ = new ColorPickerTool( this );
	}else if ( toolType == dtText) {
		currentDrawingTool_ = new TextTool( this );
	} else if ( toolType == dtCrop ) {
		currentDrawingTool_ = new CropTool( this );
		showOverlay(true);
	} 
	else {
		ElementType type;
		if ( toolType == dtLine ) {
			type = etLine;
		} if ( toolType == dtArrow ) {
			type = etArrow;
		}else if ( toolType == dtCrop ) {
			type = etCrop;
		} else if ( toolType == dtRectangle ) {
			type = etRectangle;
		} else if ( toolType == dtBlurrringRectangle ) {
			type = etBlurringRectangle;
		}else if ( toolType == dtFilledRectangle ) {
			type = etFilledRectangle;
		} else if ( toolType == dtRoundedRectangle ) {
			type = etRoundedRectangle;
		} else if ( toolType == dtEllipse ) {
			type = etEllipse;
		} else if ( toolType == dtFilledRoundedRectangle ) {
			type = etFilledRoundedRectangle;
		} else if ( toolType == dtFilledEllipse ) {
			type = etFilledEllipse;
		}

		else if ( toolType == dtMove ) {
			currentDrawingTool_ = new MoveAndResizeTool( this, etNone );
			return currentDrawingTool_;
		} else if ( toolType == dtSelection ) {
			currentDrawingTool_ = new SelectionTool( this );
			return currentDrawingTool_;
		} else if ( toolType == dtLine ) {
			type = etLine;
		}
		else {
			LOG(ERROR) << "createElement for toolType="<<toolType<<" not implemented.";
			return 0;
		}

		currentDrawingTool_ = new VectorElementTool( this, type );


		//currentDrawingTool_ = new VectorElementTool( this, type );
	}
	
	currentDrawingTool_->setPenSize(penSize_);
	currentDrawingTool_->setForegroundColor(foregroundColor_);
	currentDrawingTool_->setBackgroundColor(backgroundColor_);
	if ( notify && onDrawingToolChanged ) {
		onDrawingToolChanged(toolType);
	}
	return currentDrawingTool_;
}

void Canvas::setPreviousDrawingTool()
{
	if ( previousDrawingTool_ != dtNone ) {
		setDrawingToolType(previousDrawingTool_, true);
	}
}	

AbstractDrawingTool* Canvas::getCurrentDrawingTool()
{
	return currentDrawingTool_;
}

void Canvas::addMovableElement(MovableElement* element)
{
	if ( element->getType() == etSelection ) {
		delete selection_;
		selection_ = element;
		return;
	}

	std::vector<MovableElement*>::iterator it;
	it = find (elementsOnCanvas_.begin(), elementsOnCanvas_.end(), element);
	if (it == elementsOnCanvas_.end()) {

		elementsOnCanvas_.push_back(element);
		if ( element->getType() == etBlurringRectangle ) {
			blurRectanglesCount_ ++;
		}
		UndoHistoryItem historyItem;
		historyItem.type = uitElementAdded;
		UndoHistoryItemElement uhie;
		uhie.pos = elementsOnCanvas_.size();
		uhie.movableElement = element;
		historyItem.elements.push_back(uhie);
		undoHistory_.push(historyItem);
		elementsToDelete_.push_back(element);
	}
}

bool Canvas::addDrawingElementToDoc(DrawingElement* element)
{
	currentDocument()->addDrawingElement(element);
	return true;
}


void Canvas::endDocDrawing()
{
	currentDocument()->endDrawing();
	UndoHistoryItem historyItem;
	historyItem.type = uitDocumentChanged;
	undoHistory_.push(historyItem);
}

int Canvas::deleteSelectedElements()
{
	int deletedCount = 0;
	UndoHistoryItem uhi;
	uhi.type = uitElementRemoved;

	for ( int i = 0; i < elementsOnCanvas_.size(); i++ ) {
		if ( elementsOnCanvas_[i]->isSelected() && elementsOnCanvas_[i]->getType() != etCrop ) {
			UndoHistoryItemElement uhie;
			uhie.movableElement = elementsOnCanvas_[i];
			uhie.pos = i;
			uhi.elements.push_back(uhie );
			elementsOnCanvas_.erase(elementsOnCanvas_.begin() + i);
			i--;
			deletedCount++;
		}
	}
	if ( deletedCount ) {
		undoHistory_.push(uhi);
		updateView();
	}
	return deletedCount;
}

float Canvas::getBlurRadius()
{
	return blurRadius_;
}

void Canvas::setBlurRadius(float radius)
{
	blurRadius_ = radius;
}

bool Canvas::hasBlurRectangles()
{
	return blurRectanglesCount_!=0;
}

void Canvas::showOverlay(bool show)
{
	if ( !overlay_ ) {
		overlay_ = new CropOverlay(this, 0,0, getWidth(),getHeigth());
	}
	showOverlay_ = show;
	updateView();
}

void Canvas::deleteMovableElement(MovableElement* element)
{
	for ( int i = 0; i < elementsOnCanvas_.size(); i++ ) {
		if ( elementsOnCanvas_[i] == element ) {

			elementsOnCanvas_.erase(elementsOnCanvas_.begin() + i);
			if ( element->getType() == etBlurringRectangle ) {
				blurRectanglesCount_--;
			}
			if ( element->getType() == etCrop ) {
				showOverlay(false);
			}
			//delete element;
			break;
		}
	}
}

void Canvas::updateView() {
	fullRender_ = true;
	RECT rc = { 0, 0, getWidth(), getHeigth() };
	updateView( rc );
}
/*
void Canvas::updateView( const CRgn& region ) {
	canvasChanged_ = true;
	if ( callback_ ) {
		callback_->updateView( this, region );
	}
}*/

void Canvas::updateView( RECT boundingRect ) {
	using namespace Gdiplus;
	CRgn region;
	canvasChanged_ = true;
	Rect newRect(boundingRect.left, boundingRect.top, boundingRect.right - boundingRect.left, boundingRect.bottom - boundingRect.top );
	Rect::Union(updatedRect_,newRect,updatedRect_);
	region.CreateRectRgnIndirect( &boundingRect );
	if ( callback_ ) {
		callback_->updateView( this, region );
	}

}


void Canvas::createDoubleBuffer() {
//	delete buffer_;
	buffer_ = std_tr::shared_ptr<Gdiplus::Bitmap>(new Gdiplus::Bitmap( canvasWidth_, canvasHeight_, PixelFormat32bppARGB  ));
	bufferedGr_ = new Gdiplus::Graphics( &*buffer_ );
}

void Canvas::setCursor(CursorType cursorType)
{
	if ( currentCursor_ == cursorType ) {
		return;
	}

	currentCursor_ = cursorType ;
}


void Canvas::renderInBuffer(Gdiplus::Rect rc,bool forExport)
{
	using namespace Gdiplus;
	if ( fullRender_ ) {
		//LOG(INFO) << "canvas full render";
	}
	currentRenderingRect_ = rc;
	if (!fullRender_ && !forExport) {
		Gdiplus::Region reg(rc);
		bufferedGr_->SetClip(&reg);
	} else {
		Gdiplus::Region reg;
		bufferedGr_->SetClip(&reg);
	}
	bufferedGr_->SetPageUnit(Gdiplus::UnitPixel);
	bufferedGr_->SetSmoothingMode(SmoothingModeAntiAlias);
	
	if ( doc_->hasTransparentPixels() ) {
		if (  !forExport ) {
			SolidBrush whiteBrush(Color(255,255,255));
			bufferedGr_->FillRectangle(&whiteBrush, rc);
			/*int kSquareSize = 40;
			SolidBrush dark(Color(50,50,50));
			SolidBrush light(Color(100,100,100));
			int startX = rc.X - rc.X % kSquareSize;
			int startY = rc.Y - rc.Y % kSquareSize;
			int xCount = ceil(float(rc.Width) / kSquareSize)+1;
			int yCount = ceil(float(rc.Height) / kSquareSize)+1;
			bool isDark =  !(rc.Y / kSquareSize)%2 ;
			isDark =  (rc.X / kSquareSize)%2 == ( isDark ? 0 : 1);
			for (int j = 0; j < yCount; j++) {
				for (int i = 0; i < xCount; i++)
				{
					bufferedGr_->FillRectangle(isDark ? &dark : &light, startX + i * kSquareSize, startY + j * kSquareSize, kSquareSize, kSquareSize);
					isDark = !isDark;
				}
				isDark = !isDark;
			}*/
		} else {
			bufferedGr_->Clear(Color(0,0,0,0));
		}
	}

	doc_->render( bufferedGr_, rc );

	if ( currentDrawingTool_ != NULL ) {
		currentDrawingTool_->render( bufferedGr_ );
	}

	/*if ( !fullRender_ ) {
			for ( int i=0; i< elementsOnCanvas_.size(); i++) {
				if ( elementsOnCanvas_[i]->getType() != etBlurringRectangle ) {
					continue;
				}
				RECT paintRect = elementsOnCanvas_[i]->getPaintBoundingRect();
				RECT intersection;
				IntersectRect(&intersection, &paintRect, &rect);
				if ( !(intersection.left == 0 && intersection.right ==0 && intersection.top == 0 && intersection.bottom == 0) ) {
					UnionRect(&rect, &rect, &paintRect);
				}
			}
		}*/

	for ( int i=0; i< elementsOnCanvas_.size(); i++) {
		RECT paintRect = elementsOnCanvas_[i]->getPaintBoundingRect();
		RECT intersection;
		RECT rect = { rc.X, rc.Y, rc.GetRight(), rc.GetBottom()};
		IntersectRect(&intersection, &paintRect, &rect);
		if ( !fullRender_ && intersection.left == 0 && intersection.right ==0 && intersection.top == 0 && intersection.bottom == 0 ) {
			//LOG(INFO) << "Skipping element " << i << " out of bounds";
			continue;
		}
		elementsOnCanvas_[i]->render(bufferedGr_);
	}
	if ( !forExport ) {
		if ( overlay_ && showOverlay_ ) {
			//LOG(INFO) << "rendering overlay";
			overlay_->render(bufferedGr_);
		}

		for ( int i=0; i< elementsOnCanvas_.size(); i++) {
			elementsOnCanvas_[i]->renderGrips(bufferedGr_);
		}
	}
	canvasChanged_ = false;
	fullRender_ = false;
	updatedRect_ = Rect();
	//delete bufferedGr_;
}

void Canvas::getElementsByType(ElementType elementType, std::vector<MovableElement*>& out)
{
	int count = elementsOnCanvas_.size();
	for ( int i = 0; i < count; i++ ) {
		if ( elementType == etNone || elementsOnCanvas_[i]->getType() == elementType ) {
			out.push_back(elementsOnCanvas_[i]);
		}
	}
}

/*void Canvas::setOverlay(MovableElement* overlay)
{
	overlay_ = overlay;
	updateView();
}*/

void Canvas::setZoomFactor(float zoomFactor)
{
	zoomFactor_ = zoomFactor;
}

Gdiplus::Bitmap* Canvas::getBufferBitmap()
{
	return &*buffer_;
}

void Canvas::addUndoHistoryItem(const UndoHistoryItem& item)
{
	undoHistory_.push(item);
}

std_tr::shared_ptr<Gdiplus::Bitmap> Canvas::getBitmapForExport()
{
	using namespace Gdiplus;
	Rect rc(0,0, getWidth(), getHeigth());
	fullRender_ = true;
	renderInBuffer(rc, true);
	Crop * crop = 0;
	for ( int i=0; i< elementsOnCanvas_.size(); i++) {
		if ( elementsOnCanvas_[i]->getType() == etCrop ) {
			crop = dynamic_cast<Crop*>(elementsOnCanvas_[i]);
			break;
		}
	}

	if ( !crop )  {
		return buffer_;
	}

	int cropX = crop->getX();
	int cropY = crop->getY();
	int cropWidth = crop->getWidth();
	int cropHeight = crop->getHeight();
	Bitmap* bm = new Bitmap(cropWidth, cropHeight);
	Graphics gr(bm);
	gr.DrawImage( &*buffer_, 0, 0, cropX, cropY, cropWidth, cropHeight, UnitPixel );
	return std_tr::shared_ptr<Gdiplus::Bitmap>(bm);
}

float Canvas::getZoomFactor() const
{
	return zoomFactor_;
}

MovableElement* Canvas::getElementAtPosition(int x, int y, ElementType et)
{
	int count = elementsOnCanvas_.size();
	for ( int i = count-1; i >=0 ; i-- ) {
		if ( elementsOnCanvas_[i]->getType() != etCrop && ( et == etNone || et == elementsOnCanvas_[i]->getType()) ) {	
			if ( elementsOnCanvas_[i]->isItemAtPos(x,y) ) {
				return  elementsOnCanvas_[i];
			}
		}
	}

	for ( int i = count-1; i >=0; i-- ) {
		if ( elementsOnCanvas_[i]->getType() == etCrop ) {
			int elementX = elementsOnCanvas_[i]->getX();
			int elementY = elementsOnCanvas_[i]->getY();
			int elementWidth = elementsOnCanvas_[i]->getWidth();
			int elementHeight = elementsOnCanvas_[i]->getHeight();
			if ( x >= elementX && x <= elementX + elementWidth && y>= elementY && y <= elementY + elementHeight ) {
				return  elementsOnCanvas_[i];
			}
		}
	}


	return 0;
}

int Canvas::deleteElementsByType(ElementType elementType)
{
	int count = 0;
	for ( int i = 0; i < elementsOnCanvas_.size(); i++ ) {
		if ( elementsOnCanvas_[i]->getType() == elementType ) {
			elementsOnCanvas_.erase(elementsOnCanvas_.begin() + i);
			i--;
			count++;
		}
	}
	return count;
}

int Canvas::getWidth() const
{
	return canvasWidth_;
}

int Canvas::getHeigth() const
{
	return canvasHeight_;
}

CursorType Canvas::getCursor() const
{
	return currentCursor_;
}

bool Canvas::undo() {
	if ( !undoHistory_.size() ) {
		return false;
	}
	bool result = false;
	UndoHistoryItem item = undoHistory_.top();
	if ( item.type == uitDocumentChanged ){
		result =  doc_->undo();
	} else if ( item.type == uitElementAdded ) {
		for ( int i = 0; i< item.elements.size(); i++ ) {
			deleteMovableElement(item.elements[i].movableElement);
		}
		result = true;
	} else if  ( item.type == uitElementRemoved ) {
		int itemCount = item.elements.size();
		// Insert elements in their initial positions
		for ( int i = itemCount-1; i>=0; i-- ) {
			elementsOnCanvas_.insert(elementsOnCanvas_.begin()+ item.elements[i].pos, item.elements[i].movableElement);
			if ( item.elements[i].movableElement->getType() == etBlurringRectangle ) {
				blurRectanglesCount_++;
			}
		}
		result = true;
	} else if ( item.type == uitElementForegroundColorChanged ) {
		int itemCount = item.elements.size();
		for ( int i = itemCount-1; i>=0; i-- ) {
			item.elements[i].movableElement->setColor(item.elements[i].color);
		}
		result = true;
	} else if ( item.type == uitElementBackgroundColorChanged ) {
		int itemCount = item.elements.size();
		for ( int i = itemCount-1; i>=0; i-- ) {
			item.elements[i].movableElement->setBackgroundColor(item.elements[i].color);
		}
		result = true;
	} else if ( item.type == uitPenSizeChanged ) {
		int itemCount = item.elements.size();
		for ( int i = itemCount-1; i>=0; i-- ) {
			item.elements[i].movableElement->setPenSize(item.elements[i].penSize);
		}
		result = true;
	}else if ( item.type == uitElementPositionChanged ) {
		int itemCount = item.elements.size();
		// Insert elements in their initial positions
		for ( int i = itemCount-1; i>=0; i-- ) {
			item.elements[i].movableElement->setStartPoint(item.elements[i].startPoint);
			item.elements[i].movableElement->setEndPoint(item.elements[i].endPoint);
		}
		result = true;
	} else if ( item.type == uitTextChanged ) {
		int itemCount = item.elements.size();
		// Insert elements in their initial positions
		for ( int i = itemCount-1; i>=0; i-- ) {
			static_cast<TextElement*>(item.elements[i].movableElement)->setRawText(item.elements[i].rawText);
		}
		result = true;
	}
	if ( result ) {
		undoHistory_.pop();
	}
	updateView();
	return result;
}

InputBox* Canvas::getInputBox( const RECT& rect ) {
	inputBox_ = new InputBoxControl();
	RECT rc = rect;
	rc.left++;
	rc.top++;

	rc.top -= scrollOffset_.x;
	rc.left -= scrollOffset_.y;
		
	HWND wnd = inputBox_->Create( parentWindow_, rc, _T(""), WS_CHILD |ES_MULTILINE|/*ES_AUTOHSCROLL|*/ES_AUTOVSCROLL|  ES_WANTRETURN
			|  ES_NOHIDESEL /*| ES_LEFT */,WS_EX_TRANSPARENT );

	inputBox_->SetWindowPos(HWND_TOP,0,0,0,0, SWP_NOSIZE|SWP_NOMOVE);
	inputBox_->setFont(font_,  CFM_FACE | CFM_SIZE | CFM_CHARSET 
		| CFM_BOLD | CFM_ITALIC | CFM_UNDERLINE | CFM_STRIKEOUT | CFM_OFFSET);
	inputBox_->SetFocus();
	return inputBox_;
}

TextElement* Canvas::getCurrentlyEditedTextElement()
{
	return currentlyEditedTextElement_;
}

void Canvas::setCurrentlyEditedTextElement(TextElement* textElement)
{
	currentlyEditedTextElement_ = textElement;
}

int Canvas::unselectAllElements()
{
	int count = 0;
	for ( int i = 0; i < elementsOnCanvas_.size(); i++ ) {
		if ( elementsOnCanvas_[i]->isSelected() ) {
			elementsOnCanvas_[i]->setSelected(false);
			updateView(elementsOnCanvas_[i]->getPaintBoundingRect());
			count++;
		}
	}
	return count;
}

HWND Canvas::getRichEditControl()
{
	return inputBox_?inputBox_->m_hWnd : 0;
}

}
