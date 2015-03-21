#include "Document.h"

#include <cassert>

#include <GdiPlus.h>
#include "DrawingElement.h"
#include <Core/Logging.h>

namespace ImageEditor {
	using namespace Gdiplus;
Document::Document(int width, int height) {
	Gdiplus::Bitmap* bitmap = new Gdiplus::Bitmap( width, height, PixelFormat32bppARGB );
	currentImage_ = bitmap;
	init();
}

Document::Document(const wchar_t* fileName) {
	currentImage_ = new Gdiplus::Bitmap( fileName );
	//LOG(INFO) << "Last status " << (int)currentImage_->GetLastStatus();
	init();
}

Document::Document(Gdiplus::Bitmap *sourceImage) {
	currentImage_ = sourceImage;
	init();
}

void Document::init() {
	assert( currentImage_ );
	drawStarted_ = false;
	originalImage_ = NULL;
	currentCanvas_ = new Gdiplus::Graphics( currentImage_ );
	changedSegments_ = AffectedSegments(getWidth(), getHeight());
	//currentCanvas_->Clear( Gdiplus::Color( 150, 0, 0 ) );

	/*
	рисуем сетку*/ 
	/*for ( int i = 0; i < currentImage_->GetWidth() /  AffectedSegments::kSegmentSize; i++ ) {
		Pen pen(Color::DarkGray);
		currentCanvas_->DrawLine(&pen, i * AffectedSegments::kSegmentSize, 0, i * AffectedSegments::kSegmentSize, currentImage_->GetHeight() );
		currentCanvas_->DrawLine(&pen, 0, i * AffectedSegments::kSegmentSize, currentImage_->GetWidth(),  i * AffectedSegments::kSegmentSize  );
	}*/
}

void Document::beginDrawing(bool cloneImage) {
	drawStarted_ = true;
	if ( cloneImage ) {
		originalImage_ = currentImage_->Clone(0, 0, currentImage_->GetWidth(), currentImage_->GetHeight(), PixelFormat32bppARGB );
	}
	changedSegments_.clear();
	// saving current image
}

void Document::addDrawingElement(DrawingElement *element) {
	currentCanvas_->SetSmoothingMode( Gdiplus::SmoothingModeAntiAlias );
	currentCanvas_->SetInterpolationMode( Gdiplus::InterpolationModeHighQualityBicubic );
	AffectedSegments segments;
	element->getAffectedSegments( &segments );
	changedSegments_ += segments;
	if( !drawStarted_ ) {
		saveDocumentState( );
		changedSegments_.clear();
	}
	element->render( currentCanvas_ );
}

void Document::endDrawing() {
	// save document state
	saveDocumentState( );
	drawStarted_ = false;
	changedSegments_.clear();

}

void Document::addAffectedSegments(const AffectedSegments& segments)
{
	changedSegments_ += segments;
}

Gdiplus::Bitmap* Document::getBitmap()
{
	return currentImage_;
}

void Document::saveDocumentState( /*DrawingElement* element*/ ) {
	int pixelSize = 4;
	typedef std::deque<RECT>::iterator iter;
	std::deque<RECT> rects;
	Bitmap *srcImage = ( originalImage_ ) ? originalImage_ : currentImage_;
	int srcImageWidth = srcImage->GetWidth();
	int srcImageHeight = srcImage->GetHeight();

	changedSegments_.getRects( rects, srcImageWidth, srcImageHeight ); // may contain invalid segments!
	unsigned int pixels = 0;
	
	pixels = rects.size() * AffectedSegments::kSegmentSize * AffectedSegments::kSegmentSize * pixelSize;
	
	unsigned int dataSize    = pixels * pixelSize;
	unsigned char* imageData = new unsigned char[dataSize];
	
	Gdiplus::BitmapData bdSrc;
	Gdiplus::Rect r ( 0,0, currentImage_->GetWidth(), currentImage_->GetHeight() );
	
	if ( srcImage->LockBits( &r,  ImageLockModeRead, PixelFormat32bppARGB, &bdSrc) != Gdiplus::Ok ) {
		return ;
	}
	
	BYTE* bpSrc = (BYTE*)bdSrc.Scan0;
	unsigned int segmentSize = AffectedSegments::kSegmentSize * AffectedSegments::kSegmentSize * pixelSize;

	unsigned char* pImageData = imageData;
	AffectedSegments outSegments(srcImageWidth, srcImageHeight);
	
	for ( iter it = rects.begin(); it != rects.end(); ++it ) {
		int x = it->left;
		int y = it->top;
		if ( x < 0 || y < 0 ) {
			continue;;
		}
		int rectWidth  = min(it->right - it->left, srcImageWidth - x);
		int rectHeight = min(it->bottom - it->top, srcImageHeight - y);
		if ( rectWidth <= 0 || rectHeight <= 0) {
			// invalid rectangle. Out of bounds;
			continue;
		}
		outSegments.markRect(x,y, rectWidth,rectHeight);
		//LOG(INFO) << "Saving segment ("<<x<<","<<y<<"," << rectWidth << ","<< rectHeight << ")";
		for( int j = 0; j < rectHeight; j++ ) {
			unsigned int dataOffset = (r.Width * (y + j) + x) * pixelSize;
			unsigned int rowSize = rectWidth * pixelSize;
			memcpy ( pImageData,  bpSrc + dataOffset, rowSize );
			//memset( pImageData, 255, rowSize );
			pImageData += rowSize;
		}

	}
	currentImage_->UnlockBits( &bdSrc );
	HistoryItem item;
	item.data     = imageData;
	item.segments = changedSegments_;
	item.size     = dataSize;
	history_.push_back( item );
	delete originalImage_;
	originalImage_ = 0;
	changedSegments_.clear();
}

void Document::render(Gdiplus::Graphics *gr) {
	if (!gr || !currentImage_ ) return;
	
	gr->DrawImage( currentImage_, 0, 0, currentImage_->GetWidth(), currentImage_->GetHeight());
}

bool  Document::undo() {
	if ( history_.empty() ) {
		return false;
	}
	typedef std::deque<RECT>::iterator iter;
	HistoryItem undoItem = history_.back();
	history_.pop_back();
	std::deque<RECT> rects;
	undoItem.segments.getRects( rects, currentImage_->GetWidth(),currentImage_->GetHeight() );
	
	Gdiplus::BitmapData bdSrc;
	Gdiplus::Rect r ( 0,0, currentImage_->GetWidth(), currentImage_->GetHeight() );
	if ( currentImage_->LockBits( &r,  ImageLockModeWrite, PixelFormat32bppARGB, &bdSrc) != Gdiplus::Ok ) {
		return false ;
	}
	BYTE* bpSrc = (BYTE*)bdSrc.Scan0;
	unsigned char* pdata = undoItem.data;
	int pixelSize = 4;

	for ( iter it = rects.begin(); it != rects.end(); ++it ) {
		int x = it->left;
		int y = it->top;
		int rectWidth  = it->right - it->left;
		int rectHeight = it->bottom - it->top;

		//LOG(INFO) << "Restoring segment ("<<x<<","<<y<<"," << rectWidth << ","<< rectHeight << ")";
		for( int j = 0; j < rectHeight; j++ ) {
			unsigned int dstDataOffset = (r.Width * (y + j) + x) * pixelSize;
			unsigned int rowSize    = rectWidth * pixelSize;
			memcpy ( bpSrc + dstDataOffset,  pdata, rowSize );
			//memset( bpSrc + dstDataOffset, 255, rowSize );
			pdata += rowSize;
		}
		

	}
	delete[] undoItem.data;
	currentImage_->UnlockBits( &bdSrc );
	return true;
}


int Document::getWidth()
{
	return currentImage_->GetWidth();
}

int Document::getHeight()
{
	return currentImage_->GetHeight();
}

Painter* Document::getGraphicsObject() {
	return currentCanvas_;
}
}
