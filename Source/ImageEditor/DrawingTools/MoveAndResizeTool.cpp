#include "MoveAndResizeTool.h"

#include "../Canvas.h"
#include "../Document.h"
#include "../MovableElements.h"

#include <Core/Utils/CoreUtils.h>
#include <Core/Logging.h>

#include <math.h>
#include <cassert>
#include <gdiplus.h>
#include <math.h>

namespace ImageEditor {

CropOverlay* MoveAndResizeTool::cropOverlay_ = 0;

MoveAndResizeTool::MoveAndResizeTool( Canvas* canvas, ElementType type ) : AbstractDrawingTool( canvas ) {
	currentElement_       = NULL;
	elementType_          = type;
//	draggedBoundary_. = btNone;
	isMoving_ = false;
	allowCreatingElements_ = true;
	prevPaintBoundingRect_.left = -1;
	prevPaintBoundingRect_.right = -1;
	prevPaintBoundingRect_.top = -1;
	prevPaintBoundingRect_.bottom = -1;

}

void MoveAndResizeTool::beginDraw( int x, int y ) {
	draggedBoundary_ = checkElementsBoundaries(x,y, &currentElement_);
	if ( draggedBoundary_.bt!= btNone ) {
		canvas_->unselectAllElements();
		currentElement_->setSelected(true);
		currentElement_->beginMove();
		originalStartPoint_ = currentElement_->getStartPoint();
		originalEndPoint_ = currentElement_->getEndPoint();
		prevPaintBoundingRect_ = currentElement_->getPaintBoundingRect();
		return;
	}
	MovableElement* el = canvas_->getElementAtPosition(x, y);
	if ( el && ( elementType_ == el->getType() ||  ( elementType_== etNone  && el->getType()  != etCrop )) ) {
		//currentElement_->setSelected(true);
		canvas_->unselectAllElements();
		el->setSelected(true);
		originalStartPoint_ = el->getStartPoint();
		originalEndPoint_ = el->getEndPoint();
		isMoving_ = true;
		//LOG(INFO) << "Starting moving!";
		currentElement_ = el;
		currentElement_->beginMove();
		prevPaintBoundingRect_ = currentElement_->getPaintBoundingRect();
		startPoint_.x = x;
		startPoint_.y = y;
		//canvas_->updateView(el->getPaintBoundingRect());
		return;
	}
	if ( elementType_== etCrop  ) {
		canvas_->deleteElementsByType(elementType_);
	}
	canvas_->unselectAllElements();

	if ( allowCreatingElements_ ) {
		POINT pt = { x, y };
		//LOG(INFO) << "beginDraw createElement"  ;
		createElement();
		
		if ( currentElement_ ) {
			startPoint_.x = x;
			startPoint_.y = y;
			currentElement_->setStartPoint( pt );
			currentElement_->setEndPoint( pt );
			prevPaintBoundingRect_ = currentElement_->getPaintBoundingRect();
			canvas_->addMovableElement( currentElement_ );
			currentElement_->beginMove();
			
		}
		if ( !currentElement_ || currentElement_->getType() == etCrop ) {
			//canvas_->updateView(); // update the whole canvas
		} else {
			canvas_->updateView(currentElement_->getPaintBoundingRect());
		}
	} else {
		currentElement_ = 0;
		if ( canvas_->unselectAllElements() ) {
			//canvas_->updateView();
		}
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
		//LOG(INFO) << "draggedBoundary_.gpt="<<draggedBoundary_.gpt;
		int elWidth  = 0;
		int elHeight = 0;
		int elX = 0;
		int elY  = 0;
		if ( elementBasePoint ) {
			elementBasePoint->x = x;
			elementBasePoint->y = y;
			elWidth = currentElement_->getWidth();
			elHeight = currentElement_->getHeight();
			elX = currentElement_->getX();
			elY  = currentElement_->getY();
			//LOG(INFO) << "Resizing object to " << elWidth << " "<<elHeight;
		} else {
			elWidth = currentElement_->getWidth();
			elHeight = currentElement_->getHeight();
			elX = currentElement_->getX();
			elY  = currentElement_->getY();
			switch ( draggedBoundary_.bt ) {
				case btBottomRight:
					elWidth = x - elX+1;
					elHeight = y - elY+1;
					break;
				case btBottom:
					elHeight = y - elY+1;
					break;
				case btRight:
					elWidth = x - elX+1;
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
					elHeight = y - elY+1;
					elX = x;
					break;
				case btTopRight:
					
					elWidth =  x - elX+1;
					elHeight = elY - y + elHeight;
					elY = y;

					//currentElement_->setEndPoint()
			}
			//LOG(INFO) << "x=" << x << " y="<<y<<" object.x = "<<currentElement_->getX()<<" object.y = "<< currentElement_->getY();
			//LOG(INFO) << "Resizing object to " << elX  << " "<< elY << " " << elWidth << " "<<elHeight;
			currentElement_->resize( elWidth,elHeight);
			currentElement_->setX(elX);
			currentElement_->setY(elY);
		}
		
		if ( currentElement_ && currentElement_->getType() == etCrop && canvas_->onCropChanged ) {
			//LOG(INFO) << "onCropChanged";
			canvas_->onCropChanged(currentElement_->getX(), currentElement_->getY(), currentElement_->getWidth(), currentElement_->getHeight());
		}
		RECT paintBoundingRect = currentElement_->getPaintBoundingRect();
		RECT updateRect;
		UnionRect(&updateRect, &paintBoundingRect, &prevPaintBoundingRect_);

		canvas_->updateView(updateRect);
		prevPaintBoundingRect_ = paintBoundingRect;

		return;
	}
	
	if ( isMoving_ && currentElement_ ) {
		int newX = currentElement_->getX() + x - startPoint_.x;
		int newY  = currentElement_->getY() + y - startPoint_.y;
		//LOG(INFO) << "Moving object to new position " << newX << " "<<newY;
		currentElement_->setX(newX);
		currentElement_->setY(newY);
		startPoint_.x = x;
		startPoint_.y = y;
		
		RECT paintBoundingRect = currentElement_->getPaintBoundingRect();
		RECT updateRect;
		UnionRect(&updateRect, &paintBoundingRect, &prevPaintBoundingRect_);
		canvas_->updateView(updateRect);
		if ( currentElement_ && currentElement_->getType() == etCrop && canvas_->onCropChanged ) {
			//LOG(INFO) << "onCropChanged";
			canvas_->onCropChanged(currentElement_->getX(), currentElement_->getY(), currentElement_->getWidth(), currentElement_->getHeight());
		}
		prevPaintBoundingRect_ = currentElement_->getPaintBoundingRect();

		return;
	}

	if ( currentElement_ ) {
		POINT pt = { x, y };
		currentElement_->setEndPoint( pt );
		/*if ( currentElement_ && currentElement_->getType() == etCrop && canvas_->onCropChanged ) {
			//LOG(INFO) << "onCropChanged";
			canvas_->onCropChanged(currentElement_->getX(), currentElement_->getY(), currentElement_->getWidth(), currentElement_->getHeight());
		}*/
		RECT paintBoundingRect = currentElement_->getPaintBoundingRect();
		RECT updateRect;
		UnionRect(&updateRect, &paintBoundingRect, &prevPaintBoundingRect_);
		
		canvas_->updateView(updateRect);
		prevPaintBoundingRect_ = currentElement_->getPaintBoundingRect();
	}
}

void MoveAndResizeTool::endDraw( int x, int y ) {
	if ( currentElement_ ) {
		currentElement_->endMove();
		if ( currentElement_->getType() == etCrop && canvas_->onCropFinished ) {
			//LOG(INFO) << "onCropChanged";
			canvas_->onCropFinished(currentElement_->getX(), currentElement_->getY(), currentElement_->getWidth(), currentElement_->getHeight());
		}
		if ( currentElement_->getType() != etCrop) {
			currentElement_->setDrawDashedRectangle(false);
		}

		POINT newStartPoint_ = currentElement_->getStartPoint();
		POINT newEndPoint_ = currentElement_->getEndPoint();
		if ( !memcmp(&newStartPoint_,&originalStartPoint_, sizeof(newStartPoint_)) || memcmp(&newEndPoint_,&originalEndPoint_, sizeof(newEndPoint_)) ) {
			Canvas::UndoHistoryItem uhi;
			uhi.type = Canvas::uitElementPositionChanged;
			Canvas::UndoHistoryItemElement uhie;
			uhie.startPoint = originalStartPoint_;
			uhie.endPoint = originalEndPoint_;
			uhie.movableElement = currentElement_;
			uhi.elements.push_back(uhie);
			canvas_->addUndoHistoryItem(uhi);
		}

		RECT paintBoundingRect = currentElement_->getPaintBoundingRect();
		RECT updateRect;
		UnionRect(&updateRect, &paintBoundingRect, &prevPaintBoundingRect_);
		currentElement_= 0;
		prevPaintBoundingRect_.bottom = -1;
		prevPaintBoundingRect_.left = -1;
		prevPaintBoundingRect_.right = -1;
		prevPaintBoundingRect_.top = -1;
		canvas_->updateView(updateRect);
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
		//currentElement_->render( gr );
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
			/*if ( !cropOverlay_ ) {
				cropOverlay_ = new CropOverlay(canvas_, 0,0, canvas_->getWidth(), canvas_->getHeigth());
				
				atexit(&cleanUp);
			}*/
			canvas_->showOverlay(true);
			currentElement_ = new Crop(canvas_, 0, 0, 0, 0 );
			break;
		case etRectangle:
			currentElement_ = new Rectangle(canvas_, startPoint_.x,startPoint_.y, endPoint_.x, endPoint_.y);
			break;
		case etRoundedRectangle:
			currentElement_ = new RoundedRectangle(canvas_, startPoint_.x,startPoint_.y, endPoint_.x, endPoint_.y);
			break;
		case etFilledRoundedRectangle:
			currentElement_ = new FilledRoundedRectangle(canvas_, startPoint_.x,startPoint_.y, endPoint_.x, endPoint_.y);
			break;
		case etEllipse:
			currentElement_ = new Ellipse(canvas_);
			break;
		case etFilledEllipse:
			currentElement_ = new FilledEllipse(canvas_);
			break;
		case etBlurringRectangle:
			currentElement_ = new BlurringRectangle(canvas_, canvas_->getBlurRadius(), startPoint_.x,startPoint_.y, endPoint_.x, endPoint_.y);
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

}