#include "MarkerTool.h"

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
using namespace Gdiplus;


MarkerTool::MarkerTool( Canvas* canvas ) : AbstractDrawingTool( canvas ) {
	circleStride_ = 0;
	circleData_ = 0;
	createCircle();
}

MarkerTool::~MarkerTool()
{
	delete[] circleData_;
}

void MarkerTool::beginDraw( int x, int y ) {
	Gdiplus::Region exclRegion(Gdiplus::Rect(0,0,canvas_->getWidth(), canvas_->getHeigth()));
	affectedRegion_.Exclude(&exclRegion);
	canvas_->currentDocument()->beginDrawing();
	oldPoint_.x = x;
	oldPoint_.y = y;
}

void MarkerTool::continueDraw( int x, int y, DWORD flags ) {
	if ( flags & MK_CONTROL ) {
		y = oldPoint_.y;
	}
	drawLine( oldPoint_.x, oldPoint_.y, x, y) ;

	//line->setPenSize(25 );
	//_canvas->currentDocument()->addDrawingElement( line );

	oldPoint_.x = x;
	oldPoint_.y = y;


}

void MarkerTool::endDraw( int x, int y ) {
	canvas_->currentDocument()->addAffectedSegments(segments_);
	canvas_->endDocDrawing();
	segments_.clear();
}

void MarkerTool::render( Painter* gr ) {

}


ImageEditor::CursorType MarkerTool::getCursor(int x, int y)
{
	return ctCross;
}

void MarkerTool::drawLine(int x0, int y0, int x1, int y1) {
	using namespace Gdiplus;
	Graphics *gr = canvas_->currentDocument()->getGraphicsObject();
	gr->SetSmoothingMode(SmoothingModeAntiAlias);
	Pen pen(foregroundColor_,penSize_);
	pen.SetStartCap(LineCapRound);
	pen.SetEndCap(LineCapRound);
	//gr->DrawLine(&pen, x0,y0, x1, y1);

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
			RECT rc = {x - penSize_, y - penSize_, penSize_*2, penSize_*2 };
			rc.right += rc.left;
			rc.bottom += rc.top;
			UnionRect(&updatedRect, &updatedRect, &rc);
			highlightRegion(rc);

			
			/*RectF sourceRect(rc.left, rc.top, rc.right-rc.left, rc.bottom - rc.top);
			GraphicsPath path;
			path.AddEllipse(sourceRect);
			path.SetFillMode(FillModeAlternate);
			Gdiplus::Region reg(&path);

			affectedRegion_.Union(&reg);*/
		
			UnionRect(&updatedRect, &updatedRect, &rc);
			//delete circle;
		} 
	} else if ( y1 == y0 ) {
		for( int x = xStart; x <= xEnd; x++ ) {
			int y = y0;
			RECT rc = {x - penSize_, y - penSize_, penSize_ * 2, penSize_ * 2 };
			rc.right += rc.left;
			rc.bottom += rc.top;
			UnionRect(&updatedRect, &updatedRect, &rc);
			highlightRegion(rc);
			segments_.markRect( rc );
		} 
	} else {
		for( int a = 0; a <= len; a++ ) {
			x = x0 + a * sinA;
			y = y0 + a * cosA;



			RECT rc = {x - penSize_, y - penSize_, penSize_ * 2, penSize_ * 2 };
			rc.right += rc.left;
			rc.bottom += rc.top;
			UnionRect(&updatedRect, &updatedRect, &rc);
			highlightRegion(rc);
			segments_.markRect( rc );
		} 
	}  

	canvas_->updateView(updatedRect);
}


void MarkerTool::highlightRegion(RECT rc)
{
	Bitmap* canvasBm = canvas_->currentDocument()->getBitmap();
	BitmapData canvasData;
	int w = min(canvasBm->GetWidth()-rc.left,rc.right - rc.left);
	int h = min(canvasBm->GetHeight()-rc.top, rc.bottom - rc.top);
	rc.left = max(0,rc.left);
	rc.top = max(0,rc.top);
	Rect rc2 (rc.left , rc.top, w, h);
	segments_.markRect( rc );
	if (canvasBm->LockBits(& rc2, ImageLockModeRead|ImageLockModeWrite, PixelFormat32bppARGB, & canvasData) == Ok) {
		UINT stride;
		uint8_t * source= (uint8_t *) canvasData.Scan0;
		uint8_t * brSource= (uint8_t *) circleData_;
		if (canvasData.Stride > 0) {
			stride = canvasData.Stride;
		} else {
			stride = - canvasData.Stride;
		}
		/*int lum = 0;
		int disp = 0;
		for ( int i =0; i < h; i++ ) {
		for ( int j = 0; j < w; j++ ) {
		int offset = i*stride+j*4;
		int Y = 0.299 * source[offset] + 0.587 * source[offset+1] + 0.114 * source[offset+2];
		lum += Y;
		}
		}

		lum = float(lum) / ( w * h);
		for ( int i =0; i < h; i++ ) {
		for ( int j = 0; j < w; j++ ) {
		int offset = i*stride+j*4;
		int Y = 0.299 * source[offset] + 0.587 * source[offset+1] + 0.114 * source[offset+2];
		if ( abs(Y-lum) > disp ) {
		disp = abs(Y-lum);
		}
		}
		}*/

		for ( int i =0; i < h; i++ ) {
			for ( int j = 0; j < w; j++ ) {
				/*if ( affectedRegion_.IsVisible(i+rc.top, j+rc.left) ) {
				continue;
				}*/
				int offset = i*stride+j*4;
				int circleOffset = i * circleStride_ + j* 4;
				int Y = 0.299 * source[offset] + 0.587 * source[offset+1] + 0.114 * source[offset+2];

				float srcA =  pow(brSource[circleOffset+3]/255.0 * (Y/255.0),15); // why pow 15 ?? I don't know
				uint8_t srcR=  brSource[circleOffset];
				uint8_t srcG=  brSource[circleOffset+1];
				uint8_t srcB=  brSource[circleOffset+2];
				if ( Y != 255 ) {
					srcA = srcA;
				}

				if ( i+j % 40 ) {
					//LOG(INFO) << "srcA" << srcA;
				}

				float dstA =  source[offset+3]/255.0;
				uint8_t dstR=  source[offset];
				uint8_t dstG=  source[offset+1];
				uint8_t dstB=  source[offset+2];
				float outA = srcA + dstA*(1-srcA);
				uint8_t outR=  (srcR * srcA + dstR * dstA * ( 1 - srcA))/ outA;
				uint8_t outG=  (srcG * srcA + dstG * dstA* ( 1 - srcA))/ outA;
				uint8_t outB=  (srcB * srcA + dstB * dstA* ( 1 - srcA))/ outA;
				source[offset] = outR;
				source[offset+1] = outG ;
				source[offset+2] = outB;
				source[offset+3] = outA * 255; 

			}
		}

		canvasBm->UnlockBits(&canvasData);
	}

}
		
void MarkerTool::setPenSize(int size)
{
	AbstractDrawingTool::setPenSize(size);
	createCircle();
}


void MarkerTool::createCircle()
{
	using namespace Gdiplus;
	delete[] circleData_;
	circleData_ = 0;
	circleStride_ = 0;
	Bitmap * circle = new Bitmap(penSize_*2, penSize_*2, PixelFormat32bppARGB);
	Graphics gr2(circle);
	SolidBrush br(Color(255,255,0));
	gr2.FillEllipse( &br, 0, 0, circle->GetWidth(), circle->GetHeight());

	BitmapData circleData;

	Rect lc(0,0,circle->GetWidth(),circle->GetHeight());
	if ( circle->LockBits(&lc, ImageLockModeRead, PixelFormat32bppARGB, & circleData) == Ok)
	{
		if (circleData.Stride > 0) { 
			circleStride_ = circleData.Stride;
		} else {
			circleStride_ = - circleData.Stride;
		}
		size_t dataSize = circleStride_ * circle->GetHeight();
		circleData_ = new uint8_t[dataSize];
		memcpy(circleData_, circleData.Scan0, dataSize);
		circle->UnlockBits(&circleData);
	}
	
	delete circle;
}

}