#include "BrushTool.h"

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
	using namespace Gdiplus;
	Graphics *gr = canvas_->currentDocument()->getGraphicsObject();
	gr->SetSmoothingMode(SmoothingModeAntiAlias);
	Pen pen(foregroundColor_,penSize_);
	pen.SetStartCap(LineCapRound);
	pen.SetEndCap(LineCapRound);
	gr->DrawLine(&pen, x0,y0, x1, y1);

	RECT updatedRect = {0,0,0,0};

	if ( y1 < y0 ) {
		std::swap( y0, y1 );
		std::swap( x0, x1 );
	}
	int xStart = min( x0, x1 )/*( min(x0, x1) / AffectedSegments::kSegmentSize +1 ) * AffectedSegments::kSegmentSize*/;
	int xEnd   = max( x0, x1 )/*( max(x0, x1) / AffectedSegments::kSegmentSize ) * AffectedSegments::kSegmentSize*/;

	int yStart = min( y0, y1 );
	int yEnd   = max( y0, y1 );





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
			RECT rc = {x - penSize_, y - penSize_, penSize_ * 2, penSize_ * 2 };
			rc.right += rc.left;
			rc.bottom += rc.top;
			UnionRect(&updatedRect, &updatedRect, &rc);
			//gr->FillEllipse( &br, (int)x, y, penSize_, penSize_ );
			segments_.markRect( rc );
		} 
	} else if ( y1 == y0 ) {
		for( int x = xStart; x <= xEnd; x++ ) {
			int y = y0;
			RECT rc = {x - penSize_, y - penSize_, penSize_ * 2, penSize_ * 2 };
			rc.right += rc.left;
			rc.bottom += rc.top;
			UnionRect(&updatedRect, &updatedRect, &rc);
			//gr->FillEllipse( &br, (int)x, y, penSize_, penSize_ );
			segments_.markRect( rc );
		} 
	} else {
		// Why not simple draw line ? O_o
		for( int a = 0; a <= len; a++ ) {
			x = x0 + a * sinA;
			y = y0 + a * cosA;



			RECT rc = {x - penSize_, y - penSize_, penSize_ * 2, penSize_ * 2 };
			rc.right += rc.left;
			rc.bottom += rc.top;
			UnionRect(&updatedRect, &updatedRect, &rc);
			//gr->FillEllipse( &br, (int)x, y, penSize_, penSize_ );
			segments_.markRect( rc );
		} 
	}  

	canvas_->updateView(updatedRect);
}

}