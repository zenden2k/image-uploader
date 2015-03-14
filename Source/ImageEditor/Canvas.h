#ifndef IMAGEEDITOR_CANVAS_H
#define IMAGEEDITOR_CANVAS_H

#include <GdiPlus.h>
#include "InputBox.h"
#include "Gui/InputBoxControl.h"
#include "MovableElement.h"
#include <vector>
#include <Core/3rdpart/FastDelegate.h>

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
			dtPen, dtBrush, dtLine, dtRectangle, dtText, dtCrop
		};

		

		Canvas( HWND parent );
		~Canvas();
		void setDocument( Document *doc );
		void setSize( int x, int y );
		void mouseMove( int x, int y, DWORD flags);
		void mouseDown( int button, int x, int y );
		void mouseUp( int button, int x, int y );
		Document* currentDocument() const;
		void render(Painter* gr, const RECT& rect);
		void setCallback(Callback * callback);
		void setPenSize(int size);
		void setDrawingToolType(DrawingToolType tool);
		void addMovableElement(MovableElement* element);
		void deleteMovableElement(MovableElement* element);
		void deleteMovableElements(ElementType elementType);
		void getElementsByType(ElementType elementType, std::vector<MovableElement*>& out);
		void setOverlay(MovableElement* overlay);
		void setZoomFactor(float zoomFactor);
		float getZoomFactor() const;
		MovableElement* getElementAtPosition(int x, int y);
		int deleteElementsByType(ElementType elementType);
		int getWidth() const;
		int getHeigth() const;
		CursorType getCursor() const;
		bool undo();
		InputBox* getInputBox( const RECT& rect ); 

		fastdelegate::FastDelegate4<int,int,int,int> onCropChanged;
		friend class AbstractDrawingTool;
		friend class VectorElementTool;
		friend class PenTool;
		friend class BrushTool;
		friend class MovableElementTool;
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
		std::vector<MovableElement*> elementsOnCanvas_;
		CursorType currentCursor_;
		MovableElement* overlay_;
		float zoomFactor_;
		
		HWND parentWindow_;
		
		int penSize_;
		void init();
		void updateView( RECT boundingRect );
		void updateView( const CRgn& region );
		void updateView();
		void createDoubleBuffer();
		void setCursor(CursorType cursor);
		
		InputBoxControl* inputBox_;
};

}

#endif