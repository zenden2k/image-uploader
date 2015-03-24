#ifndef IMAGEEDITOR_DOCUMENT_H
#define IMAGEEDITOR_DOCUMENT_H

#include <windows.h>
#include <GdiPlus.h>
#include <queue>
#include <vector>
#include "BasicTypes.h"
#include "DrawingElement.h"

namespace ImageEditor {

class DrawingElement;

class Document {
	public:

		struct HistoryItem {
			unsigned char *data;
			unsigned int size;
			AffectedSegments segments;
		};
		Document(int width, int height);
		Document(const wchar_t* fileName);
		Document(Gdiplus::Bitmap *sourceImage, bool hasTransparentPixels = false);
		virtual ~Document();
		
		Painter* getGraphicsObject();
		void beginDrawing( bool cloneImage = true );
		void addDrawingElement(DrawingElement *element);
		void endDrawing();

		void addAffectedSegments(const AffectedSegments& segments);
		Gdiplus::Bitmap* getBitmap();
		void render(Painter* gr, Gdiplus::Rect rc);
		bool undo();
		int getWidth();
		int getHeight();
		bool hasTransparentPixels() const;
	private:
		Gdiplus::Bitmap* currentImage_;
		Gdiplus::Bitmap* originalImage_;
		Painter* currentCanvas_;
		std::vector<HistoryItem> history_;
		bool drawStarted_;
		AffectedSegments changedSegments_;
		bool hasTransparentPixels_;
		void init();
		void saveDocumentState( );
		void checkTransparentPixels();
};

}

#endif