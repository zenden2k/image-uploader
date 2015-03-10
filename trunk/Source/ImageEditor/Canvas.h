#ifndef IMAGEEDITOR_CANVAS_H
#define IMAGEEDITOR_CANVAS_H

#include <GdiPlus.h>
#include "InputBox.h"
#include "Gui/InputBoxControl.h"

namespace ImageEditor {

class Document;
class AbstractDrawingTool;
class DrawingTool;


class Canvas {
	public:
		class Callback {
		public:
			virtual void updateView( Canvas* canvas, const CRgn& region ) = NULL;
		};
		
		enum DrawingToolType {
			dtPen, dtBrush, dtLine, dtRectangle, dtText
		};

		Canvas( HWND parent );
		~Canvas();
		void setDocument( Document *doc );
		void setSize( int x, int y );
		void mouseMove( int x, int y, DWORD flags);
		void mouseDown( int button, int x, int y );
		void mouseUp( int button, int x, int y );
		Document* currentDocument() const;
		void render(Gdiplus::Graphics* gr, const RECT& rect);
		void setCallback(Callback * callback);
		void setPenSize(int size);
		void setDrawingToolType(DrawingToolType tool);
		bool undo();
		InputBox* getInputBox( const RECT& rect ); 
		friend class AbstractDrawingTool;
		friend class VectorElementTool;
		friend class PenTool;
		friend class BrushTool;
	private:
		Gdiplus::Bitmap* buffer_;
		Document* doc_;
		int canvasWidth_, canvasHeight_;
		POINT oldPoint_;
		POINT leftMouseDownPoint_;
		POINT leftMouseUpPoint_;
		Callback* callback_;
		DrawingToolType drawingToolType_;
		AbstractDrawingTool* currentDrawingTool_;
		
		HWND parentWindow_;
		int penSize_;
		void init();
		void updateView( RECT boundingRect );
		void updateView( const CRgn& region );
		void updateView();
		void createDoubleBuffer();
		InputBoxControl* inputBox_;
};

}

#endif