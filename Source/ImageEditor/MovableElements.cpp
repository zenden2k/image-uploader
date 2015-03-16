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
		Gdiplus::Pen pen( Color( 10, 10, 10), penSize_ );
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

		int x = std::min<>( startPoint_.x, endPoint_.x );
		int y = std::min<>( startPoint_.y, endPoint_.y );
		int width = std::max<>( startPoint_.x, endPoint_.x ) - x;
		int height = std::max<>( startPoint_.y, endPoint_.y ) - y;

		Grip grip1;
		Grip grip2;
		grip1.pt.x = startPoint_.x;
		grip1.pt.y = startPoint_.y;
		grip2.pt.x = endPoint_.x;
		grip2.pt.y =  endPoint_.y;

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
		// Line equation
		if ( endPoint_.x - startPoint_.x != 0 ) {
			float k = float(endPoint_.y - startPoint_.y)/float(endPoint_.x - startPoint_.x);
			float b = float(endPoint_.x* startPoint_.y - startPoint_.x*endPoint_.y)/float(endPoint_.x - startPoint_.x);
			//LOG(INFO) << "k="<<k <<" b="<<b;
			if(abs(y - (k*x + b)) <= kSelectRadius && x >= itemX-kSelectRadius && x <= itemX + itemWidth +  kSelectRadius ) {
				return true;
			}
		}

		if ( endPoint_.y - startPoint_.y != 0 ) {
			float k = float(endPoint_.x - startPoint_.x)/float(endPoint_.y - startPoint_.y);
			float b = float(endPoint_.y* startPoint_.x - startPoint_.y*endPoint_.x)/float(endPoint_.y - startPoint_.y);
			//LOG(INFO) << "k="<<k <<" b="<<b;
			if(abs(x - (k*y + b)) <= kSelectRadius && y >= itemY-kSelectRadius && y <= itemY + itemHeight +  kSelectRadius) {
				return true;
			}
		}
		return false;
	}

	TextElement::TextElement( Canvas* canvas, int startX, int startY, int endX,int endY ) :MovableElement(canvas) {
	startPoint_.x = startX;
	startPoint_.y = startY;
	endPoint_.x   = endX;
	endPoint_.y   = endY;
}

void TextElement::render(Painter* gr) {
	using namespace Gdiplus;
	if ( !gr ) {
		return;
	}
	Gdiplus::Pen pen( Color( 10, 10, 10) );
	int x = std::min<>( startPoint_.x, endPoint_.x );
	int y = std::min<>( startPoint_.y, endPoint_.y );
	int width = std::max<>( startPoint_.x, endPoint_.x ) - x;
	int height = std::max<>( startPoint_.y, endPoint_.y ) - y;
	gr->DrawRectangle( &pen, x, y, width, height );

	PrintRichEdit(canvas_->getRichEditControl(), gr, Rect(x,y,width,height));
}


void TextElement::getAffectedSegments( AffectedSegments* segments ) {
	int x = std::min<>( startPoint_.x, endPoint_.x );
	int y = std::min<>( startPoint_.y, endPoint_.y );
	int width = std::max<>( startPoint_.x, endPoint_.x ) - x;
	int height = std::max<>( startPoint_.y, endPoint_.y ) - y;
	segments->markRect( x, y, width, height ); // top
}


Crop::Crop(Canvas* canvas, int startX, int startY, int endX, int endY):MovableElement(canvas)  {
	startPoint_.x = startX;
	startPoint_.y = startY;
	endPoint_.x   = endX;
	endPoint_.y   = endY;
}

void Crop::render(Painter* gr) {
	using namespace Gdiplus;
	if ( !gr ) {
		return;
	}
	
	MovableElement::render(gr);
	
//	LOG(INFO)<<x<<" "<<y;
}


void Crop::getAffectedSegments( AffectedSegments* segments ) {
	int x = std::min<>( startPoint_.x, endPoint_.x );
	int y = std::min<>( startPoint_.y, endPoint_.y );
	int width = std::max<>( startPoint_.x, endPoint_.x ) - x;
	int height = std::max<>( startPoint_.y, endPoint_.y ) - y;

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
	Gdiplus::SolidBrush brush(Gdiplus::Color( 120, 0, 0, 0) );
	
	std::vector<MovableElement*> crops;
	canvas_->getElementsByType(etCrop, crops);
	Region rgn(0,0, canvas_->getWidth(), canvas_->getHeigth());
	/*if ( crops.size() ) {
		rgn =  Region(crops[0]->getX(),crops[0]->getY(),crops[0]->getWidth(),crops[0]->getHeight());
	}*/
	//Region rgn(300,30, 500, 400);
	for ( int i = 0; i < crops.size(); i++ ) {
	//	LOG(INFO) << "Substracting region "<<crops[i]->getX()<< " "<< crops[i]->getY() << " "<<crops[i]->getWidth() << " "<<crops[i]->getHeight();
		rgn = rgn.subtracted(Region(crops[i]->getX(),crops[i]->getY(),crops[i]->getWidth()+1,crops[i]->getHeight()+1));
	}

	try {
		gr->SetClip(rgn.toNativeRegion());
	}
	catch ( std::exception ex ) {
		LOG(ERROR) << ex.what();
	} catch ( ... ) {
		LOG(ERROR) << "Other exception";
	}
	gr->FillRectangle( &brush, getX(), getY(), getWidth(), getHeight() );
	gr->SetClip(Region(0,0,canvas_->getWidth(), canvas_->getHeigth()).toNativeRegion());


}


// Rectangle
//
Rectangle::Rectangle(Canvas* canvas, int startX, int startY, int endX, int endY):MovableElement(canvas) {
	startPoint_.x = startX;
	startPoint_.y = startY;
	endPoint_.x   = endX;
	endPoint_.y   = endY;
	drawDashedRectangle_   = false;
}

void Rectangle::render(Painter* gr) {
	using namespace Gdiplus;
	if ( !gr ) {
		return;
	}
	Gdiplus::Pen pen( Color( 10, 10, 10) );
	int x = std::min<>( startPoint_.x, endPoint_.x );
	int y = std::min<>( startPoint_.y, endPoint_.y );
	int width = std::max<>( startPoint_.x, endPoint_.x ) - x;
	int height = std::max<>( startPoint_.y, endPoint_.y ) - y;
	gr->DrawRectangle( &pen, x, y, width, height );
}


void Rectangle::getAffectedSegments( AffectedSegments* segments ) {
	int x = std::min<>( startPoint_.x, endPoint_.x );
	int y = std::min<>( startPoint_.y, endPoint_.y );
	int width = std::max<>( startPoint_.x, endPoint_.x ) - x;
	int height = std::max<>( startPoint_.y, endPoint_.y ) - y;

	//segments->markRect( x, y, width, height );
	segments->markRect( x, y, width, penSize_ ); // top
	segments->markRect( x, y, penSize_, height ); // left
	segments->markRect( x, y + height - penSize_, width, penSize_ ); // bottom
	segments->markRect( x + width - penSize_, y, penSize_, height ); // right*/
}

bool Rectangle::isItemAtPos(int x, int y)
{
	int elementX = getX();
	int elementY = getY();
	int elementWidth = getWidth();
	int elementHeight = getHeight();
	return 
		((( x >= elementX - kSelectRadius && x  <= elementX  + kSelectRadius )  || ( x >= elementX +elementWidth - kSelectRadius && x  <= elementX  +elementWidth+ kSelectRadius ) ) 
		&& y>= elementY - kSelectRadius && y <= elementY + elementHeight + kSelectRadius )

		|| 
		((( y >= elementY - kSelectRadius && y  <= elementY  + kSelectRadius )  || ( y >= elementY +elementHeight - kSelectRadius && y  <= elementY  +elementHeight+ kSelectRadius ) ) 
		
		&& x>= elementX - kSelectRadius && x <= elementX + elementWidth + kSelectRadius );
}

}
