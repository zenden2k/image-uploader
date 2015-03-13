#include "MovableElements.h"

#include <algorithm>
#include <GdiPlus.h>
#include <Core/Logging.h>
#include <exception>

#include "Region.h"
#include "Canvas.h"

namespace ImageEditor {

TextElement::TextElement( int startX, int startY, int endX,int endY ) {
	startPoint_.x = startX;
	startPoint_.y = startY;
	endPoint_.x   = endX;
	endPoint_.y   = endY;
}

void TextElement::render(Gdiplus::Graphics* gr) {
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


void TextElement::getAffectedSegments( AffectedSegments* segments ) {
	int x = std::min<>( startPoint_.x, endPoint_.x );
	int y = std::min<>( startPoint_.y, endPoint_.y );
	int width = std::max<>( startPoint_.x, endPoint_.x ) - x;
	int height = std::max<>( startPoint_.y, endPoint_.y ) - y;
	segments->markRect( x, y, width, height ); // top
}


// Rectangle
//
Crop::Crop(int startX, int startY, int endX, int endY) {
	startPoint_.x = startX;
	startPoint_.y = startY;
	endPoint_.x   = endX;
	endPoint_.y   = endY;
}

void Crop::render(Gdiplus::Graphics* gr) {
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
	segments->markRect( x + width - penSize_, y, penSize_, height ); // right*/
}

ImageEditor::ElementType Crop::getType() const
{
	return etCrop;
}

CropOverlay::CropOverlay(int startX, int startY, int endX,int endY)
{
	startPoint_.x = startX;
	startPoint_.y = startY;
	endPoint_.x = endX;
	endPoint_.y = endY;
}

void CropOverlay::render(Gdiplus::Graphics* gr)
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

}
