#ifndef IMAGEEDITOR_DOCUMENT_H
#define IMAGEEDITOR_DOCUMENT_H

#include <windows.h>
#include <GdiPlus.h>
#include <queue>
#include <vector>
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
		Document(Gdiplus::Bitmap *sourceImage);
		
		Gdiplus::Graphics* getGraphicsObject();
		void beginDrawing( bool cloneImage = true );
		void addDrawingElement(DrawingElement *element);
		void endDrawing();

		void render(Gdiplus::Graphics* gr);
		bool undo();
	private:
		Gdiplus::Bitmap* currentImage_;
		Gdiplus::Bitmap* originalImage_;
		Gdiplus::Graphics* currentCanvas_;
		std::vector<HistoryItem> history_;
		bool drawStarted_;
		AffectedSegments changedSegments_;
		void init();
		void saveDocumentState( );
		
};

}

#endif