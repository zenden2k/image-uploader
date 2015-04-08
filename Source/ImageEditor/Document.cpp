/*
     Image Uploader - program for uploading images/files to the Internet

     Copyright 2007-2015 Sergey Svistunov (zenden2k@gmail.com)

     Licensed under the Apache License, Version 2.0 (the "License");
     you may not use this file except in compliance with the License.
     You may obtain a copy of the License at

     http://www.apache.org/licenses/LICENSE-2.0

     Unless required by applicable law or agreed to in writing, software
     distributed under the License is distributed on an "AS IS" BASIS,
     WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
     See the License for the specific language governing permissions and
     limitations under the License.
*/

#include "Document.h"

#include <cassert>

#include <3rdpart/GdiplusH.h>
#include "DrawingElement.h"
#include <Core/Logging.h>
#include <stdint.h>
#include <Core/Images/Utils.h>

namespace ImageEditor {
	using namespace Gdiplus;
Document::Document(int width, int height) {
	hasTransparentPixels_ = false;
	currentImage_.reset(new Gdiplus::Bitmap( width, height, PixelFormat32bppARGB ));
	init();
}

Document::Document(const wchar_t* fileName) {
	currentImage_.reset(LoadImageFromFileWithoutLocking(fileName));
	init();
	if ( currentImage_ ) {
		checkTransparentPixels();
	}
}

Document::Document(std_tr::shared_ptr<Gdiplus::Bitmap> sourceImage,  bool hasTransparentPixels ) {
	currentImage_ = sourceImage;
	hasTransparentPixels_ = hasTransparentPixels;
	init();
}

Document::~Document()
{
	for( int i = 0; i < history_.size(); i++ ) {
		delete[] history_[i].data;
	}
	delete currentCanvas_;
}

void Document::init() {
	drawStarted_ = false;
	originalImage_ = NULL;
	if ( currentImage_ ) {
		currentCanvas_ = new Gdiplus::Graphics( currentImage_.get() );
		changedSegments_ = AffectedSegments(getWidth(), getHeight());
	}
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
	return currentImage_.get();
}

void Document::saveDocumentState( /*DrawingElement* element*/ ) {
	int pixelSize = 4;
	typedef std::deque<RECT>::iterator iter;
	std::deque<RECT> rects;
	Bitmap *srcImage = ( originalImage_ ) ? originalImage_: currentImage_.get();
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

void Document::checkTransparentPixels()
{
	using namespace Gdiplus;
	BitmapData bitmapData;
	Rect lockRect(0,0, min(10, currentImage_->GetWidth()), min(10, currentImage_->GetHeight()));
	if ( currentImage_->LockBits(&lockRect, ImageLockModeRead, PixelFormat32bppARGB, &bitmapData) == Ok) {
		uint8_t * source = (uint8_t *) bitmapData.Scan0;
		unsigned int stride;
		if ( bitmapData.Stride > 0) { 
			stride = bitmapData.Stride;
		} else {
			stride = - bitmapData.Stride;
		}
		for( int i = 0; i < lockRect.Height; i++ ) {
			for ( int j = 0; j < lockRect.Width; j++ ) {
				if ( source[i * stride + j * 4 + 3 ] != 255 ) {
					hasTransparentPixels_ = true;
					currentImage_->UnlockBits(&bitmapData);
					return;
				}

			}
		}
		currentImage_->UnlockBits(&bitmapData);
	}
}

void Document::render(Gdiplus::Graphics *gr, Gdiplus::Rect rc) {
	if (!gr || !currentImage_ ) return;
	gr->DrawImage( currentImage_.get(),rc.X, rc.Y, rc.X, rc.Y, rc.Width, rc.Height, Gdiplus::UnitPixel);
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

bool Document::isNull()
{
	return !currentImage_;
}

bool Document::hasTransparentPixels() const
{
	return hasTransparentPixels_;
}

Painter* Document::getGraphicsObject() {
	return currentCanvas_;
}
}
