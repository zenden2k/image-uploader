#include "BlurTool.h"


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

#if GDIPVER >= 0x0110 
BlurTool::BlurTool(Canvas* canvas) : BrushTool(canvas)
{

}

void BlurTool::drawLine(int x0, int y0, int x1, int y1)
{
	using namespace Gdiplus;
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
	using namespace Gdiplus;
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