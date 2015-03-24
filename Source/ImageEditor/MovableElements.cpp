#include "MovableElements.h"

#include <algorithm>
#include <GdiPlus.h>
#include <Core/Logging.h>
#include <exception>

#include "Region.h"
#include "Canvas.h"
#include <Core/Images/Utils.h> 

namespace ImageEditor {

Line::Line(Canvas* canvas, int startX, int startY, int endX, int endY) :MovableElement(canvas) {
	startPoint_.x = startX;
	startPoint_.y = startY;
	endPoint_.x   = endX;
	endPoint_.y   = endY;
	drawDashedRectangle_ = false;
	grips_.resize(2);

}


void Line::setEndPoint(POINT endPoint) {
	int kAccuracy = 7;
	if (abs (endPoint.y-startPoint_.y) < kAccuracy ) {
		endPoint.y = startPoint_.y;
	} else if (abs (endPoint.x-startPoint_.x) < kAccuracy ) {
		endPoint.x = startPoint_.x;
	}
	DrawingElement::setEndPoint( endPoint );
}

void Line::render(Painter* gr) {
	using namespace Gdiplus;
	if ( !gr ) {
		return;
	}
	Gdiplus::Pen pen( color_, penSize_ );
	pen.SetStartCap(LineCapRound);
	pen.SetEndCap(LineCapRound);
	//CustomLineCap cap()
	//pen.SetCustomStartCap()
	gr->DrawLine( &pen, startPoint_.x, startPoint_.y, endPoint_.x, endPoint_.y );
}

void Line::getAffectedSegments( AffectedSegments* segments ) {
	int x0 = startPoint_.x;
	int y0 = startPoint_.y;
	int x1 = endPoint_.x;
	int y1 = endPoint_.y;

	int xStart = ( min(x0, x1) / AffectedSegments::kSegmentSize +1 ) * AffectedSegments::kSegmentSize;
	int xEnd   = ( max(x0,x1) / AffectedSegments::kSegmentSize ) * AffectedSegments::kSegmentSize;

	x0 = startPoint_.x;
	x1 = endPoint_.x;
	y0 = startPoint_.y;
	y1 = endPoint_.y;
	segments->markPoint( x0, y0 );
	segments->markPoint( x1, y1 );

	for( int x = xStart; x <= xEnd; x += AffectedSegments::kSegmentSize ) {
		int y = -1;
		if ( x1-x0 != 0) {
			y = y0 + (x-x0)*(y1-y0)/(x1-x0);
		}

		segments->markRect( x - penSize_, y - penSize_, penSize_ * 2, penSize_ * 2  );
	}

	int yStart = ( min( y0, y1 ) / AffectedSegments::kSegmentSize + 1) * AffectedSegments::kSegmentSize;
	int yEnd   = ( max( y0, y1 ) / AffectedSegments::kSegmentSize ) * AffectedSegments::kSegmentSize;

	for( int y = yStart; y <= yEnd; y += AffectedSegments::kSegmentSize ) {
		int x = -1;
		if ( y1-y0 != 0) {
			x = x0 + ( y - y0 ) * ( x1 - x0 ) / ( y1-y0 );
		}

		segments->markRect( x - penSize_, y - penSize_, penSize_ * 2, penSize_ * 2  );
	}
}

void Line::createGrips()
{
	int rectSize = kGripSize;

	int x = getX();
	int y = getY();
	int width = getWidth();
	int height = getHeight();

	Grip grip1;
	Grip grip2;
	grip1.pt.x = startPoint_.x;
	grip1.pt.y = startPoint_.y;
	grip1.gpt = MovableElement::gptStartPoint;
	grip2.pt.x = endPoint_.x;
	grip2.pt.y =  endPoint_.y;
	grip2.gpt = MovableElement::gptEndPoint;

	if ( grip1.pt.x <= grip2.pt.x ) {
		if (  grip1.pt.y <= grip2.pt.y  ) {
			grip1.bt = btTopLeft;
			grip2.bt = btBottomRight;
		} else {
			grip1.bt = btBottomLeft;
			grip2.bt = btTopRight;
		}
	} else {
		if (  grip2.pt.y > grip1.pt.y  ) {
			grip1.bt = btTopRight;
			grip2.bt = btBottomLeft;
		} else {
			grip1.bt = btBottomRight;
			grip2.bt = btTopLeft;
		}
	}
	grips_[0] = grip1;	
	grips_[1] = grip2;	
}

bool Line::isItemAtPos(int x, int y)
{
	int itemX = getX();
	int itemY = getY();
	int itemWidth = getWidth();
	int itemHeight = getHeight();
	int selectRadius = max(kSelectRadius, penSize_);
	// Line equation
	if ( endPoint_.x - startPoint_.x != 0 ) {
		float k = float(endPoint_.y - startPoint_.y)/float(endPoint_.x - startPoint_.x);
		float b = float(endPoint_.x* startPoint_.y - startPoint_.x*endPoint_.y)/float(endPoint_.x - startPoint_.x);
		//LOG(INFO) << "k="<<k <<" b="<<b;
		if(abs(y - (k*x + b)) <= selectRadius && x >= itemX-selectRadius && x <= itemX + itemWidth +  selectRadius ) {
			return true;
		}
	}

	if ( endPoint_.y - startPoint_.y != 0 ) {
		float k = float(endPoint_.x - startPoint_.x)/float(endPoint_.y - startPoint_.y);
		float b = float(endPoint_.y* startPoint_.x - startPoint_.y*endPoint_.x)/float(endPoint_.y - startPoint_.y);
		//LOG(INFO) << "k="<<k <<" b="<<b;
		if(abs(x - (k*y + b)) <= selectRadius && y >= itemY-selectRadius && y <= itemY + itemHeight +  selectRadius) {
			return true;
		}
	}
	return false;
}

ImageEditor::ElementType Line::getType() const
{
	return etLine;
}

TextElement::TextElement( Canvas* canvas, InputBox* inputBox, int startX, int startY, int endX,int endY ) :MovableElement(canvas) {
	startPoint_.x = startX;
	startPoint_.y = startY;
	endPoint_.x   = endX;
	endPoint_.y   = endY;
	inputBox_ = inputBox;
}

void TextElement::render(Painter* gr) {
	using namespace Gdiplus;
	if ( !gr ) {
		return;
	}
	/*Gdiplus::Pen pen( Color( 10, 10, 10) );
	int x = std::min<>( startPoint_.x, endPoint_.x );
	int y = std::min<>( startPoint_.y, endPoint_.y );
	int width = std::max<>( startPoint_.x, endPoint_.x ) - x;
	int height = std::max<>( startPoint_.y, endPoint_.y ) - y;*/
	drawDashedRectangle_ = isSelected() || !inputBox_;
	//gr->DrawRectangle( &pen, x, y, width, height );
	if ( inputBox_  && !inputBox_->isVisible()) {
		inputBox_->render(gr, canvas_->getBufferBitmap(), Rect(getX(),getY(),getWidth(),getHeight()));
	}
}


void TextElement::getAffectedSegments( AffectedSegments* segments ) {
	int x = getX();
	int y = getY();
	int width = getWidth();
	int height = getHeight();
	segments->markRect( x, y, width, height ); // top
}


void TextElement::resize(int width, int height)
{
	MovableElement::resize(width,height);
	if ( inputBox_ ) {
		inputBox_->resize(getX()+1, getY()+1, width,height, grips_);
		inputBox_->invalidate();
		canvas_->updateView();
	}
}

void TextElement::setInputBox(InputBox* inputBox)
{
	inputBox_ = inputBox;
	setTextColor();
	inputBox_->onTextChanged.bind(this, &TextElement::onTextChanged);
	inputBox_->onEditCanceled.bind(this, &TextElement::onEditCanceled);
	inputBox_->onEditFinished.bind(this, &TextElement::onEditFinished);
	inputBox_->onResized.bind(this, &TextElement::onControlResized);
}

InputBox* TextElement::getInputBox() const
{
	return inputBox_;
}

void TextElement::onTextChanged(TCHAR *text)
{
	canvas_->updateView(getPaintBoundingRect());
}

void TextElement::onEditCanceled()
{
	inputBox_->show(false);
}

void TextElement::onEditFinished()
{
	inputBox_->show(false);
}

void TextElement::onControlResized(int w, int h)
{
	resize(w, h);
}

void TextElement::setTextColor()
{
	if ( inputBox_ ) {
		inputBox_->setTextColor(color_);
	}

}

void TextElement::setColor(Gdiplus::Color color)
{
	MovableElement::setColor(color);
	setTextColor();
}

ImageEditor::ElementType TextElement::getType() const
{
	return etText;
}

void TextElement::setSelected(bool selected)
{
	MovableElement::setSelected(selected);
	if ( inputBox_ && !selected ) {
		inputBox_->show(false);
	}
}

Crop::Crop(Canvas* canvas, int startX, int startY, int endX, int endY):MovableElement(canvas)  {
	startPoint_.x = startX;
	startPoint_.y = startY;
	endPoint_.x   = endX;
	endPoint_.y   = endY;
}

void Crop::render(Painter* gr) {
	MovableElement::render(gr);
}


void Crop::getAffectedSegments( AffectedSegments* segments ) {
	int x = getX();
	int y = getY();
	int width = getWidth();
	int height = getHeight();

	//segments->markRect( x, y, width, height );
	segments->markRect( x, y, width, penSize_ ); // top
	segments->markRect( x, y, penSize_, height ); // left
	segments->markRect( x, y + height - penSize_, width, penSize_ ); // bottom
	segments->markRect( x + width - penSize_, y, penSize_, height ); // right
}

ImageEditor::ElementType Crop::getType() const
{
	return etCrop;
}

CropOverlay::CropOverlay(Canvas* canvas, int startX, int startY, int endX,int endY):MovableElement(canvas)
{
	startPoint_.x = startX;
	startPoint_.y = startY;
	endPoint_.x = endX;
	endPoint_.y = endY;
}

void CropOverlay::render(Painter* gr)
{
	using namespace Gdiplus;
	Gdiplus::SolidBrush brush(Gdiplus::Color( 120, 0, 0, 0) );
	
	std::vector<MovableElement*> crops;
	canvas_->getElementsByType(etCrop, crops);
	Rect rc (0,0, canvas_->getWidth(), canvas_->getHeigth());
	rc.Intersect(canvas_->currentRenderingRect());
	Region rgn(rc.X,rc.Y, rc.Width, rc.Height);
	for ( int i = 0; i < crops.size(); i++ ) {
		rgn = rgn.subtracted(Region(crops[i]->getX(),crops[i]->getY(),crops[i]->getWidth(),crops[i]->getHeight()));
	}

	Gdiplus::Region oldRegion;
	gr->GetClip(&oldRegion);
	gr->SetClip(rgn.toNativeRegion(), Gdiplus::CombineModeIntersect);
	
	
	gr->FillRectangle( &brush, getX(), getY(), getWidth(), getHeight() );
	gr->SetClip(&oldRegion);


}


// Rectangle
//
Rectangle::Rectangle(Canvas* canvas, int startX, int startY, int endX, int endY, bool filled):MovableElement(canvas) {
	startPoint_.x = startX;
	startPoint_.y = startY;
	endPoint_.x   = endX;
	endPoint_.y   = endY;
	drawDashedRectangle_   = false;
	filled_ = filled;
}

void Rectangle::render(Painter* gr) {
	using namespace Gdiplus;
	if ( !gr ) {
		return;
	}
	SmoothingMode prevSmoothingMode = gr->GetSmoothingMode();
	gr->SetSmoothingMode(SmoothingModeNone);
	Gdiplus::Pen pen( color_, penSize_ );
	int x = getX()+penSize_/2/*-(1-penSize_%2)*/;
	int y = getY()+penSize_/2/*-(1-penSize_%2)*/;
	int width = getWidth()-penSize_;
	int height = getHeight()-penSize_;
	if ( filled_ ) {
		SolidBrush br(backgroundColor_);
		gr->FillRectangle(&br, x, y, width, height);
	}
	gr->DrawRectangle( &pen, x, y, width, height );
	gr->SetSmoothingMode(prevSmoothingMode);
	
}


void Rectangle::getAffectedSegments( AffectedSegments* segments ) {
	int x = getX();
	int y = getY();
	int width = getWidth();
	int height = getHeight();

	//segments->markRect( x, y, width, height );
	segments->markRect( x, y, width, penSize_ ); // top
	segments->markRect( x, y, penSize_, height ); // left
	segments->markRect( x, y + height - penSize_, width, penSize_ ); // bottom
	segments->markRect( x + width - penSize_, y, penSize_, height ); // right*/
}

bool Rectangle::isItemAtPos(int x, int y)
{
	if ( filled_ ) {
		return MovableElement::isItemAtPos(x,y);
	}
	int elementX = getX();
	int elementY = getY();
	int elementWidth = getWidth();
	int elementHeight = getHeight();
	int selectRadius = max(kSelectRadius, penSize_);
	return 
		((( x >= elementX - selectRadius && x  <= elementX  + selectRadius )  || ( x >= elementX +elementWidth - selectRadius && x  <= elementX  +elementWidth+ selectRadius ) ) 
		&& y>= elementY - selectRadius && y <= elementY + elementHeight + selectRadius )

		|| 
		((( y >= elementY - selectRadius && y  <= elementY  + selectRadius )  || ( y >= elementY +elementHeight - selectRadius && y  <= elementY  +elementHeight+ selectRadius ) ) 
		
		&& x>= elementX - selectRadius && x <= elementX + elementWidth + selectRadius );
}

RECT Rectangle::getPaintBoundingRect()
{
	RECT rc = MovableElement::getPaintBoundingRect();
	/*rc.left -= penSize_;
	rc.top -= penSize_;
	rc.right += penSize_;
	rc.bottom+= penSize_;*/
	return rc;
}

ImageEditor::ElementType Rectangle::getType() const
{
	return etRectangle;
}

Arrow::Arrow(Canvas* canvas,int startX, int startY, int endX,int endY) : Line(canvas, startX, startY, endX, endY)
{

}

void Arrow::render(Painter* gr)
{
	using namespace Gdiplus;
	Gdiplus::Pen pen(/* color_*/color_, penSize_ );
	// Create two AdjustableArrowCap objects
	AdjustableArrowCap cap1(/*penSize_/2*/3, /*penSize_/2*/3, true);
	//AdjustableArrowCap cap2 = new AdjustableArrowCap(2, 1);

	// Set cap properties
	cap1.SetBaseCap(/*LineCapRound*/LineCapTriangle);
	//cap1.SetBaseInset(5);
	cap1.SetStrokeJoin(/*LineJoinBevel*/LineJoinRound);
	/*cap2.WidthScale = 3;
	cap2.BaseCap = LineCap.Square;
	cap2.Height = 1;*/


	// Set CustomStartCap and CustomEndCap properties
	//blackPen.CustomStartCap = cap1;
	pen.SetCustomEndCap(&cap1);

	gr->DrawLine( &pen, startPoint_.x, startPoint_.y, endPoint_.x, endPoint_.y );
}

RECT Arrow::getPaintBoundingRect()
{
	RECT res = MovableElement::getPaintBoundingRect();
	res.left -= penSize_ * 2;
	res.top -= penSize_ * 2;
	res.bottom += penSize_*2;
	res.right += penSize_*2;
	return res;
}

ImageEditor::ElementType Arrow::getType() const
{
	return etArrow;
}

Selection::Selection(Canvas* canvas, int startX, int startY, int endX,int endY) : MovableElement(canvas)
{

}

void Selection::render(Painter* gr)
{

}

ImageEditor::ElementType Selection::getType() const
{
	return etSelection;
}

void Selection::createGrips()
{
	throw std::logic_error("The method or operation is not implemented.");
}

FilledRectangle::FilledRectangle(Canvas* canvas, int startX, int startY, int endX,int endY):Rectangle(canvas, startX, startY, endX, endY, true)
{
}


ImageEditor::ElementType FilledRectangle::getType() const
{
	return etBlurringRectangle;
}

BlurringRectangle::BlurringRectangle(Canvas* canvas, float blurRadius, int startX, int startY, int endX,int endY) : MovableElement(canvas)
{
	blurRadius_ = blurRadius;
} 

BlurringRectangle::~BlurringRectangle()
{
	BlurCleanup();
}

void BlurringRectangle::setBlurRadius(float radius)
{
	blurRadius_ = radius;
}

float BlurringRectangle::getBlurRadius()
{
	return blurRadius_;
}

void BlurringRectangle::render(Painter* gr)
{
	drawDashedRectangle_ = isSelected();
	using namespace Gdiplus;
	Bitmap* background = canvas_->getBufferBitmap();
	drawDashedRectangle_ = isMoving_;
	Rect currentRenderingRect = canvas_->currentRenderingRect();
	Rect elRect(getX(), getY(), getWidth(), getHeight());
	elRect.Intersect(currentRenderingRect);
	if ( elRect.Width < 1 || elRect.Height < 1 ) {
		return;
	}
	if ( !isMoving_ ) { // Optimization: do not apply blur while moving or resizing, can hang on slow CPUs
		#if GDIPVER >= 0x0110 

		Blur blur;
		BlurParams blurParams;
		blurParams.radius = blurRadius_;
		blur.SetParameters(&blurParams);
		Matrix matrix;
		Status st ;
		RectF sourceRect(elRect.X, elRect.Y, elRect.Width, elRect.Height);

		st = gr->DrawImage(background,  &sourceRect, &matrix, &blur, 0, Gdiplus::UnitPixel);
		#else
		ApplyGaussianBlur(background, elRect.X, elRect.Y, elRect.Width, elRect.Height, blurRadius_);
		#endif
	}
}

ImageEditor::ElementType BlurringRectangle::getType() const
{
	return etBlurringRectangle;
}


RoundedRectangle::RoundedRectangle(Canvas* canvas, int startX, int startY, int endX,int endY,bool filled /*= false */) 
				: Rectangle(canvas, startX, startY, endX,endY, filled)
{

}

void RoundedRectangle::render(Painter* gr)
{
	using namespace Gdiplus;
	Gdiplus::Pen pen( color_, penSize_ );
	int x = getX() + penSize_/2 /*- (1-penSize_%2)*/;
	int y = getY() + penSize_/2 /*- (1-penSize_%2)*/;
	int width = getWidth()-penSize_;
	int height = getHeight()-penSize_;
	SolidBrush br(backgroundColor_);
	PixelOffsetMode oldPOM = gr->GetPixelOffsetMode();
	gr->SetPixelOffsetMode(PixelOffsetModeHalf);
	DrawRoundedRectangle(gr, Rect(x,y,width,height), penSize_*2, &pen, filled_ ? &br : 0);
	gr->SetPixelOffsetMode(oldPOM);
}

ImageEditor::ElementType RoundedRectangle::getType() const
{
	return etRoundedRectangle;
}

FilledRoundedRectangle::FilledRoundedRectangle(Canvas* canvas, int startX, int startY, int endX,int endY) : RoundedRectangle(canvas, startX, startY, endX,endY, true)
{

}

ImageEditor::ElementType FilledRoundedRectangle::getType() const
{
	return etFilledRoundedRectangle;
}

Ellipse::Ellipse(Canvas* canvas, bool filled /*= false */) : MovableElement(canvas)
{
	filled_ = filled;
}

void Ellipse::render(Painter* gr)
{
	using namespace Gdiplus;
	Gdiplus::Pen pen( color_, penSize_ );
	int x = getX() + penSize_/2 /*- (1-penSize_%2)*/;
	int y = getY() + penSize_/2 /*- (1-penSize_%2)*/;
	int width = getWidth()-penSize_;
	int height = getHeight()-penSize_;
	SolidBrush br(backgroundColor_);
	if ( filled_ ) {
		gr->FillEllipse(&br, x,y,width,height);
	}
	gr->DrawEllipse(&pen, x,y,width,height);
	
}

bool Ellipse::ContainsPoint(Gdiplus::Rect ellipse, Gdiplus::Point location) {
	using namespace Gdiplus;
	Point center(
                ellipse.GetLeft() + (ellipse.Width / 2),
				  ellipse.GetTop() + (ellipse.Height / 2));

            double _xRadius = ellipse.Width / 2;
            double _yRadius = ellipse.Height / 2;


            if (_xRadius <= 0.0 || _yRadius <= 0.0)
                return false;
            /* This is a more general form of the circle equation
             *
             * X^2/a^2 + Y^2/b^2 <= 1
             */

            Point normalized(location.X - center.X,
                                         location.Y - center.Y);

            return ((double)(normalized.X * normalized.X)
                     / (_xRadius * _xRadius)) + ((double)(normalized.Y * normalized.Y) / (_yRadius * _yRadius))
                <= 1.0;
        }


void Ellipse::createGrips()
{
	grips_.resize(8);
	MovableElement::createGrips();
	//btBottomRight, btBottom,  btBottomLeft,  btRight,  btLeft,  btTopLeft, btTop, btTopRight, 
		grips_.erase(grips_.begin() + btTopRight);
		grips_.erase(grips_.begin() + btTopLeft);
			grips_.erase(grips_.begin() + btBottomLeft);
	grips_.erase(grips_.begin() + btBottomRight);
}

ImageEditor::ElementType Ellipse::getType() const
{
	return etEllipse;
}

bool Ellipse::isItemAtPos(int x, int y)
{
	using namespace Gdiplus;
	int elX = getX();
	int elY = getY();
	int width = getWidth();
	int height = getHeight();
	int selectRadius = max(penSize_, kSelectRadius);
	if ( filled_ ) {
		return ContainsPoint(Rect(elX,elY, width, height), Point(x,y));
	} else {
		return ContainsPoint(Rect(elX-selectRadius,elY-selectRadius, width+selectRadius*2, height+selectRadius*2), Point(x,y))
			&& !ContainsPoint(Rect(elX+selectRadius,elY+selectRadius, width-selectRadius*2, height-selectRadius*2), Point(x,y) );
	}
}

FilledEllipse::FilledEllipse(Canvas* canvas) : Ellipse(canvas, true)
{

}

ImageEditor::ElementType FilledEllipse::getType() const
{
	return etFilledEllipse;
}

}
