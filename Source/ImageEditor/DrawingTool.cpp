#include "DrawingTool.h"

#include "Canvas.h"
#include "Document.h"
#include "MovableElements.h"

#include <Core/Utils/CoreUtils.h>
#include <Core/Logging.h>

#include <math.h>
#include <cassert>
#include <gdiplus.h>
#include <math.h>



void DebugWrite(const TCHAR* szFormat, ...)
{
	TCHAR szBuff[1024];
	va_list arg;
	va_start(arg, szFormat);
	_vsntprintf(szBuff, sizeof(szBuff), szFormat, arg);
	va_end(arg);

	OutputDebugString(szBuff);
}

namespace ImageEditor {

using namespace Gdiplus;

AbstractDrawingTool::AbstractDrawingTool( Canvas *canvas ) {
	startPoint_.x = 0;
	startPoint_.y = 0;
	endPoint_.x   = 0;
	endPoint_.y   = 0;
	assert( canvas );
	canvas_ = canvas;
}

void AbstractDrawingTool::beginDraw( int x, int y ) {
	startPoint_.x = x;
	startPoint_.y = y;
}

void AbstractDrawingTool::endDraw( int x, int y ) {
	endPoint_.x = x;
	endPoint_.y = y;
}

void AbstractDrawingTool::mouseDoubleClick(int x, int y)
{

}

CursorType AbstractDrawingTool::getCursor(int x, int y)
{
	return ctDefault;
}

void AbstractDrawingTool::setPenSize(int size)
{
	penSize_ = size;
}

void AbstractDrawingTool::setForegroundColor(Gdiplus::Color color)
{
	foregroundColor_ = color;
}

void AbstractDrawingTool::setBackgroundColor(Gdiplus::Color color)
{
	backgroundColor_ = color;
}

VectorElementTool::VectorElementTool( Canvas* canvas, ElementType type ) : MoveAndResizeTool( canvas, type ) {
	currentElement_       = NULL;
}


ImageEditor::CursorType VectorElementTool::getCursor(int x, int y)
{
	return ctCross;
}



/*
 Pen Tool
*/

PenTool::PenTool( Canvas* canvas ): AbstractDrawingTool( canvas )  {

}

void PenTool::beginDraw( int x, int y ) {
	canvas_->currentDocument()->beginDrawing();
	oldPoint_.x = x;
	oldPoint_.y = y;
}

void PenTool::continueDraw( int x, int y, DWORD flags ) {
	if ( flags & MK_CONTROL ) {
		y = oldPoint_.y;
	}
	//LOG(INFO) <<"x=" << x <<"y=" << y;
	Line * line =  new Line( canvas_, oldPoint_.x, oldPoint_.y, x, y) ;

	line->setPenSize(1 );
	line->setColor(foregroundColor_);
	line->setBackgroundColor(backgroundColor_);

	line->setCanvas(canvas_);
	canvas_->currentDocument()->addDrawingElement( line );

	oldPoint_.x = x;
	oldPoint_.y = y;
	canvas_->updateView();
}

void PenTool::endDraw( int x, int y ) {
	canvas_->endDocDrawing();
}

void PenTool::render( Painter* gr ) {

}


ImageEditor::CursorType PenTool::getCursor(int x, int y)
{
	return ctCross;
}

BrushTool::BrushTool( Canvas* canvas ) : AbstractDrawingTool( canvas ) {

}

void BrushTool::beginDraw( int x, int y ) {
	Gdiplus::Region exclRegion(Gdiplus::Rect(0,0,canvas_->getWidth(), canvas_->getHeigth()));
	affectedRegion_.Exclude(&exclRegion);
	canvas_->currentDocument()->beginDrawing();
	oldPoint_.x = x;
	oldPoint_.y = y;
}

void BrushTool::continueDraw( int x, int y, DWORD flags ) {
	if ( flags & MK_CONTROL ) {
		y = oldPoint_.y;
	}
	drawLine( oldPoint_.x, oldPoint_.y, x, y) ;

	//line->setPenSize(25 );
	//_canvas->currentDocument()->addDrawingElement( line );

	oldPoint_.x = x;
	oldPoint_.y = y;
	canvas_->updateView();

}

void BrushTool::endDraw( int x, int y ) {
	canvas_->currentDocument()->addAffectedSegments(segments_);
	canvas_->endDocDrawing();
	segments_.clear();
}

void BrushTool::render( Painter* gr ) {

}

ImageEditor::CursorType BrushTool::getCursor(int x, int y)
{
	return ctCross;
}

void BrushTool::drawLine(int x0, int y0, int x1, int y1) {


	if ( y1 < y0 ) {
		std::swap( y0, y1 );
		std::swap( x0, x1 );
	}
	int xStart = min( x0, x1 )/*( min(x0, x1) / AffectedSegments::kSegmentSize +1 ) * AffectedSegments::kSegmentSize*/;
	int xEnd   = max( x0, x1 )/*( max(x0, x1) / AffectedSegments::kSegmentSize ) * AffectedSegments::kSegmentSize*/;

	int yStart = min( y0, y1 );
	int yEnd   = max( y0, y1 );
	

	Graphics *gr = canvas_->currentDocument()->getGraphicsObject();



	float len = sqrt(  pow((float)x1-x0, 2) + pow ((float) y1- y0, 2 ) );
	if ( ((int)len) == 0 ) {
		len = 1;
	}
	float sinA = ( x1 - x0 ) / len;
	float cosA = sqrt( 1 - sinA * sinA );

	float x = x0;
	float y = y0;
		SolidBrush br( foregroundColor_ );
	if ( x1 == x0 ) {
		//	MessageBox(0,0,0,0);
		for( int y = yStart; y <= yEnd; y++ ) {
			x = x0;
			gr->FillEllipse( &br, (int)x, y, penSize_, penSize_ );
			segments_.markRect( x - penSize_, y - penSize_, penSize_ * 2, penSize_ * 2  );
		} 
	} else if ( y1 == y0 ) {
		for( int x = xStart; x <= xEnd; x++ ) {
			int y = y0;
			gr->FillEllipse( &br, x, y, penSize_, penSize_ );
			segments_.markRect( x - penSize_, y - penSize_, penSize_ * 2, penSize_ * 2  );
		} 
	} else {
		// Why not simple draw line ? O_o
		for( int a = 0; a <= len; a++ ) {
			x = x0 + a * sinA;
			y = y0 + a * cosA;
				

			
			gr->FillEllipse( &br, (int)x, (int)y, penSize_, penSize_ );
			segments_.markRect( x - penSize_, y - penSize_, penSize_ * 2, penSize_ * 2  );
		 } 
	}  
}




CropOverlay* MoveAndResizeTool::cropOverlay_ = 0;

MoveAndResizeTool::MoveAndResizeTool( Canvas* canvas, ElementType type ) : AbstractDrawingTool( canvas ) {
	currentElement_       = NULL;
	elementType_          = type;
//	draggedBoundary_. = btNone;
	isMoving_ = false;
	allowCreatingElements_ = true;
}

void MoveAndResizeTool::beginDraw( int x, int y ) {
	draggedBoundary_ = checkElementsBoundaries(x,y, &currentElement_);
	if ( draggedBoundary_.bt!= btNone ) {
		canvas_->unselectAllElements();
		currentElement_->setSelected(true);
		return;
	}
	MovableElement* el = canvas_->getElementAtPosition(x,y);
	if ( el && ( elementType_ == el->getType() ||  ( elementType_== etNone  && el->getType()  != etCrop )) ) {
		//currentElement_->setSelected(true);
		canvas_->unselectAllElements();
		el->setSelected(true);
		isMoving_ = true;
		LOG(INFO) << "Starting moving!";
		currentElement_ = el;
		startPoint_.x = x;
		startPoint_.y = y;
		canvas_->updateView();
		return;
	}
	if ( elementType_== etCrop  ) {
		canvas_->deleteElementsByType(elementType_);
	}
	canvas_->unselectAllElements();

	if ( allowCreatingElements_ ) {
		POINT pt = { x, y };
		LOG(INFO) << "beginDraw createElement"  ;
		createElement();
		
		if ( currentElement_ ) {
			startPoint_.x = x;
			startPoint_.y = y;
			currentElement_->setStartPoint( pt );
			currentElement_->setEndPoint( pt );
			canvas_->addMovableElement( currentElement_ );
			
		}
		canvas_->updateView();
	} else {
		currentElement_ = 0;
		canvas_->unselectAllElements();
		canvas_->updateView();
	}
}

void MoveAndResizeTool::continueDraw( int x, int y, DWORD flags ) {

	if ( currentElement_ && draggedBoundary_.bt!= btNone ) {
		POINT* elementBasePoint = 0;
		if ( draggedBoundary_.gpt == MovableElement::gptStartPoint ) {
			elementBasePoint = &currentElement_->startPoint_;
		} else if ( draggedBoundary_.gpt == MovableElement::gptEndPoint ) {
			elementBasePoint = &currentElement_->endPoint_;
		}

		if ( elementBasePoint ) {
			elementBasePoint->x = x;
			elementBasePoint->y = y;
		} else {
			int elWidth = currentElement_->getWidth();
			int elHeight = currentElement_->getHeight();
			int elX = currentElement_->getX();
			int elY  = currentElement_->getY();
			switch ( draggedBoundary_.bt ) {
				case btBottomRight:
					elWidth = x - elX;
					elHeight = y - elY;
					break;
				case btBottom:
					elHeight = y - elY;
					break;
				case btRight:
					elWidth = x - elX;
					break;
				case btTopLeft:
					
					elWidth =  elX - x + elWidth;
					elHeight = elY - y + elHeight;
					elX = x;
					elY = y;
				case btLeft:
					
					elWidth = elX - x + elWidth;
					elX = x;
					break;
				case btTop:
					elHeight =  elY - y + elHeight;
					elY = y;
					break;
				case btBottomLeft:
					
					elWidth = elX - x + elWidth;
					elHeight = y - elY;
					elX = x;
					break;
				case btTopRight:
					
					elWidth =  x - elX;
					elHeight = elY - y + elHeight;
					elY = y;

					//currentElement_->setEndPoint()
			}
			//LOG(INFO) << "x=" << x << " y="<<y<<" object.x = "<<currentElement_->getX()<<" object.y = "<< currentElement_->getY();
			LOG(INFO) << "Resizing object to " << elX  << " "<< elY << " " << elWidth << " "<<elHeight;
			currentElement_->resize( elWidth,elHeight);
			currentElement_->setX(elX);
			currentElement_->setY(elY);
		}
		if ( currentElement_ && currentElement_->getType() == etCrop && canvas_->onCropChanged ) {
			LOG(INFO) << "onCropChanged";
			canvas_->onCropChanged(currentElement_->getX(), currentElement_->getY(), currentElement_->getWidth(), currentElement_->getHeight());
		}
		canvas_->updateView();
		return;
	}
	
	if ( isMoving_ && currentElement_ ) {
		int newX = currentElement_->getX() + x - startPoint_.x;
		int newY  = currentElement_->getY() + y - startPoint_.y;
		LOG(INFO) << "Moving object to new position " << newX << " "<<newY;
		currentElement_->setX(newX);
		currentElement_->setY(newY);
		startPoint_.x = x;
		startPoint_.y = y;
		if ( currentElement_ && currentElement_->getType() == etCrop && canvas_->onCropChanged ) {
			LOG(INFO) << "onCropChanged";
			canvas_->onCropChanged(currentElement_->getX(), currentElement_->getY(), currentElement_->getWidth(), currentElement_->getHeight());
		}
		canvas_->updateView();

		return;
	}

	if ( currentElement_ ) {
		POINT pt = { x, y };
		currentElement_->setEndPoint( pt );
		/*if ( currentElement_ && currentElement_->getType() == etCrop && canvas_->onCropChanged ) {
			LOG(INFO) << "onCropChanged";
			canvas_->onCropChanged(currentElement_->getX(), currentElement_->getY(), currentElement_->getWidth(), currentElement_->getHeight());
		}*/
		canvas_->updateView();
	}
}

void MoveAndResizeTool::endDraw( int x, int y ) {
	if ( currentElement_ ) {
		if ( currentElement_->getType() == etCrop && canvas_->onCropChanged ) {
			LOG(INFO) << "onCropChanged";
			canvas_->onCropChanged(currentElement_->getX(), currentElement_->getY(), currentElement_->getWidth(), currentElement_->getHeight());
		}
		currentElement_->setDrawDashedRectangle(false);
		currentElement_= 0;
		
		canvas_->updateView();
		//currentElement_->setSelected(true);
	}
	if ( draggedBoundary_.bt!= btNone ) {
		
		draggedBoundary_.bt = btNone;
		return;
	}
	if ( isMoving_ ) {
		isMoving_ = false;
		return;
	}
}

void MoveAndResizeTool::render( Painter* gr ) {
	if ( currentElement_ ) {
		currentElement_->render( gr );
	}
}

void MoveAndResizeTool::createElement() {
	//delete currentElement_;
	currentElement_ = 0;
	switch( elementType_ ) {
		case etArrow:
			currentElement_ = new Arrow(canvas_, startPoint_.x,startPoint_.y, endPoint_.x, endPoint_.y);
			break;
		case etLine:
			currentElement_ = new Line(canvas_, startPoint_.x,startPoint_.y, endPoint_.x, endPoint_.y);
			break;
		case etText:
			currentElement_ = new TextElement(canvas_, 0, startPoint_.x,startPoint_.y, endPoint_.x, endPoint_.y);
			break;
		case etSelection:
			currentElement_ = new Selection(canvas_, startPoint_.x,startPoint_.y, endPoint_.x, endPoint_.y);
			break;
		case etCrop:
			if ( !cropOverlay_ ) {
				cropOverlay_ = new CropOverlay(canvas_, 0,0, canvas_->getWidth(), canvas_->getHeigth());
				
				atexit(&cleanUp);
			}
			canvas_->setOverlay(cropOverlay_);
			currentElement_ = new Crop(canvas_, 0, 0, 0, 0 );
			break;
		case etRectangle:
			currentElement_ = new Rectangle(canvas_, startPoint_.x,startPoint_.y, endPoint_.x, endPoint_.y);
			break;
		case etBlurringRectangle:
			#if GDIPVER >= 0x0110 
			currentElement_ = new BlurringRectangle(canvas_, startPoint_.x,startPoint_.y, endPoint_.x, endPoint_.y);
			#else
				LOG(ERROR) << "Blur effect is not supported by current version of GdiPlus.";
			#endif
			break;
		case etFilledRectangle:
			currentElement_ = new FilledRectangle(canvas_, startPoint_.x,startPoint_.y, endPoint_.x, endPoint_.y);
			break;
	}
	if ( currentElement_ ) {
		currentElement_->setPenSize(penSize_);
		currentElement_->setColor(foregroundColor_);
		currentElement_->setBackgroundColor(backgroundColor_);
	}

}

MovableElement::Grip MoveAndResizeTool::checkElementsBoundaries( int x, int y, MovableElement** elem)
{
	std::vector<MovableElement*> cropElements;
	canvas_->getElementsByType(elementType_, cropElements);
	int count = cropElements.size();
	for( int i  = count-1; i>= 0; i-- ) {
		if ( !cropElements[i]->isSelected() && cropElements[i]->getType() != etCrop ) {
			continue;
		}
		MovableElement::Grip grip = checkElementBoundaries(cropElements[i], x , y);
		if ( grip.bt != btNone ) {
			if ( elem ) {
				*elem = cropElements[i];
			}
			return grip;
		}
	}

	return MovableElement::Grip();
}	

MovableElement::Grip  MoveAndResizeTool::checkElementBoundaries(MovableElement* element, int x, int y)
{
	for ( int i = 0 ; i < element->grips_.size(); i++ ) {
		if ( abs (x - element->grips_[i].pt.x) <= MovableElement::kGripSize+2 &&  abs (y - element->grips_[i].pt.y) <= MovableElement::kGripSize+2 ) {
			return element->grips_[i];
		}
	}
	return MovableElement::Grip();
}

void MoveAndResizeTool::cleanUp()
{
	delete cropOverlay_;
}

CursorType MoveAndResizeTool::getCursor(int x, int y)
{
	CursorType  ct = ctDefault;
	
	switch( elementType_ ) {
		case etArrow:
			//currentElement_ = new Line( 0, 0, 0, 0 );
			break;
		case etCrop:
			ct = ctCross;
			break;
	}
	MovableElement::Grip grip = checkElementsBoundaries( x, y); 
	if ( grip.bt != btNone ) {
		return MovableElement::GetCursorForBoundary(grip.bt);
	}
	MovableElement* el = canvas_->getElementAtPosition(x,y);
	if ( el &&  (el->getType() != etCrop || elementType_ == etCrop)  ) {

		return ctMove;
	}
	return ct;
}

void MoveAndResizeTool::mouseDoubleClick(int x, int y)
{
	MovableElement* el = canvas_->getElementAtPosition(x,y);
	if ( el &&  el->getType() == etText ) {
		canvas_->setDrawingToolType(Canvas::dtText);	
		AbstractDrawingTool* dtool = canvas_->getCurrentDrawingTool();
		if ( dtool ) {
			dtool->beginDraw(x,y);
		}
	
	}
}

CropTool::CropTool(Canvas* canvas) : MoveAndResizeTool(canvas, etCrop) {

}

void CropTool::beginDraw(int x, int y)
{
	MoveAndResizeTool::beginDraw(x,y);
}

void CropTool::continueDraw(int x, int y, DWORD flags)
{
	MoveAndResizeTool::continueDraw(x,y,flags);
	
}

void CropTool::endDraw(int x, int y)
{
	MoveAndResizeTool::endDraw(x, y);
	if ( currentElement_ ) {
		POINT pt = { x, y };
		if ( x == startPoint_.x && y == startPoint_.y ) {
			canvas_->deleteMovableElement(currentElement_);
			currentElement_ = 0;
			canvas_->setOverlay(0);
		}
		canvas_->updateView();
		currentElement_ = 0;
	}
}




TextTool::TextTool( Canvas* canvas ) : MoveAndResizeTool( canvas, etText ) {

}

void TextTool::beginDraw( int x, int y ) {
	if ( currentElement_ && dynamic_cast<TextElement*>(currentElement_)->getInputBox()->isVisible() ) {
		allowCreatingElements_ = false;
	}
	MoveAndResizeTool::beginDraw( x, y );
	allowCreatingElements_ = true;
	if ( currentElement_ ) {
		
		InputBox* input = dynamic_cast<TextElement*>(currentElement_)->getInputBox();
		if ( input ) {
			input->show(true);
		}
	}

}

void TextTool::continueDraw( int x, int y, DWORD flags ) {
	MoveAndResizeTool::continueDraw( x, y ,flags);
}

void TextTool::endDraw( int x, int y ) {
	if (!currentElement_) {
		return;
	}
	int xStart = min( startPoint_.x, x );
	int xEnd   = max( startPoint_.x, x );

	int yStart = min( startPoint_.y, y );
	int yEnd   = max( startPoint_.y, y );

	int width = currentElement_->getWidth();
	int height = currentElement_->getHeight();
	if (  width < 100 ) {
		width = 100;
	}

	if ( height < 25 ) {
		height = 25;
	}
	currentElement_->resize(width, height);
	int elX = currentElement_->getX();
	int elY = currentElement_->getY();
	RECT inputRect = {elX, elY, elX + currentElement_->getWidth(), elY + currentElement_->getHeight() };
	
	TextElement * textElement = dynamic_cast<TextElement*>(currentElement_);
	InputBox * inputBox = textElement->getInputBox();
	if ( !inputBox ) {
		inputBox = canvas_->getInputBox( inputRect );
	}
	//	currentElement_ = new TextElement(canvas_,inputBox, xStart,yStart, xEnd, yEnd);
	textElement->setInputBox(inputBox);
	canvas_->setCurrentlyEditedTextElement(textElement);
	textElement->setSelected(true);
	inputBox->invalidate();
	textElement->setDrawDashedRectangle(false);
	//currentElement_ = new TextElement(canvas_,inputBox, xStart,yStart, xEnd, yEnd);
	//canvas_->addMovableElement(currentElement_);
	//MoveAndResizeTool::endDraw( x, y);
	canvas_->updateView();

}

void TextTool::render( Painter* gr ) {

}

ImageEditor::CursorType TextTool::getCursor(int x, int y)
{
	CursorType ct = MoveAndResizeTool::getCursor(x,y);
	InputBox* inputBox = currentElement_ ? dynamic_cast<TextElement*>(currentElement_)->getInputBox(): NULL;
	if ( (ct == ctDefault || ( ct == ctMove && canvas_->getElementAtPosition(x,y)!= currentElement_)) && 
		( !inputBox || !inputBox->isVisible() )) {
		ct = ctEdit;
	}
	return ct;
}

SelectionTool::SelectionTool(Canvas* canvas) : MoveAndResizeTool(canvas, etSelection)
{

}
#if GDIPVER >= 0x0110 
BlurTool::BlurTool(Canvas* canvas) : BrushTool(canvas)
{

}

void BlurTool::drawLine(int x0, int y0, int x1, int y1)
{
	Graphics *gr = canvas_->currentDocument()->getGraphicsObject();
	gr->SetClip(&affectedRegion_, CombineModeExclude);
	if ( y1 < y0 ) {
		std::swap( y0, y1 );
		std::swap( x0, x1 );
	}
	int xStart = min( x0, x1 )/*( min(x0, x1) / AffectedSegments::kSegmentSize +1 ) * AffectedSegments::kSegmentSize*/;
	int xEnd   = max( x0, x1 )/*( max(x0, x1) / AffectedSegments::kSegmentSize ) * AffectedSegments::kSegmentSize*/;

	int yStart = min( y0, y1 );
	int yEnd   = max( y0, y1 );


	

	Gdiplus::Bitmap * background =  canvas_->currentDocument()->getBitmap();

	float len = sqrt(  pow((float)x1-x0, 2) + pow ((float) y1- y0, 2 ) );
	if ( ((int)len) == 0 ) {
		len = 1;
	}
	float sinA = ( x1 - x0 ) / len;
	float cosA = sqrt( 1 - sinA * sinA );

	float x = x0;
	float y = y0;
	Blur blur;
	BlurParams blurParams;
	blurParams.radius = 1.5;
	blur.SetParameters(&blurParams);
	Matrix matrix;
	Status st ;
	


	SolidBrush br( foregroundColor_ );
	if ( x1 == x0 ) {
		RectF sourceRect(x - penSize_, y - penSize_, penSize_ * 2, penSize_ * 2);
		GraphicsPath path;
		path.AddEllipse(sourceRect);
		path.SetFillMode(FillModeAlternate);
		/*Gdiplus::Region reg(&path);
		//reg.Exclude(&affectedRegion_);
		gr->SetClip(&reg);
		st = gr->DrawImage(background,  &sourceRect, &matrix, &blur, 0, Gdiplus::UnitPixel);
		affectedRegion_.Union(&reg);*/
		
		//	MessageBox(0,0,0,0);
		for( int y = yStart; y <= yEnd; y++ ) {
			x = x0;
			RectF sourceRect(x - penSize_, y - penSize_, penSize_ * 2, penSize_ * 2);
			//gr->FillEllipse( &br, (int)x, y, penSize_, penSize_ );
			GraphicsPath path;
			path.AddEllipse(sourceRect);
			path.SetFillMode(FillModeAlternate);
			Gdiplus::Region reg(&path);
			//reg.Exclude(&affectedRegion_);
			gr->SetClip(&reg, CombineModeIntersect);
			
			segments_.markRect( x - penSize_, y - penSize_, penSize_ * 2, penSize_ * 2  );
			st = gr->DrawImage(background,  &sourceRect, &matrix, &blur, 0, Gdiplus::UnitPixel);
			affectedRegion_.Union(&reg);
		} 
	} else if ( y1 == y0 ) {
		for( int x = xStart; x <= xEnd; x++ ) {
			int y = y0;
			RectF sourceRect(x - penSize_, y - penSize_, penSize_ * 2, penSize_ * 2 );
			GraphicsPath path;
			path.AddEllipse(sourceRect);
			path.SetFillMode(FillModeAlternate);
			Gdiplus::Region reg(&path);
			//reg.Exclude(&affectedRegion_);
			gr->SetClip(&reg, CombineModeIntersect);

			gr->FillEllipse( &br, x, y, penSize_, penSize_ );
			segments_.markRect( x - penSize_, y - penSize_, penSize_ * 2, penSize_ * 2  );
			st = gr->DrawImage(background,  &sourceRect, &matrix, &blur, 0, Gdiplus::UnitPixel);
			affectedRegion_.Union(&reg);
		} 
	} else {
		// Why not simple draw line ? O_o
		for( int a = 0; a <= len; a++ ) {
			x = x0 + a * sinA;
			y = y0 + a * cosA;

			RectF sourceRect( x - penSize_, y - penSize_, penSize_ * 2, penSize_ * 2);
			GraphicsPath path;
			path.AddEllipse(sourceRect);
			path.SetFillMode(FillModeAlternate);
			Gdiplus::Region reg(&path);
			//reg.Exclude(&affectedRegion_);
			gr->SetClip(&reg, CombineModeIntersect);

			//gr->FillEllipse( &br, (int)x, (int)y, penSize_, penSize_ );
			st = gr->DrawImage(background,  &sourceRect, &matrix, &blur, 0, Gdiplus::UnitPixel);
			segments_.markRect( x - penSize_, y - penSize_, penSize_ * 2, penSize_ * 2  );
			affectedRegion_.Union(&reg);
		} 
	}  
}

void BlurTool::endDraw(int x, int y)
{
	Graphics *gr = canvas_->currentDocument()->getGraphicsObject();

	gr->SetClip(Rect(0,0,canvas_->getWidth(), canvas_->getHeigth()));
	/*SolidBrush br(Color(100,255,0,0));
	gr->SetClip(&affectedRegion_);
	gr->FillRectangle(&br, 0,0, canvas_->getWidth(), canvas_->getHeigth());
	BrushTool::endDraw(x,y);
	canvas_->updateView();*/
}

#endif
}