#ifndef IMAGEEDITOR_CANVAS_H
#define IMAGEEDITOR_CANVAS_H

#include <GdiPlus.h>
#include "InputBox.h"
#include "Gui/InputBoxControl.h"
#include "MovableElement.h"
#include <vector>
#include <Core/3rdpart/FastDelegate.h>
#include <stack>

namespace ImageEditor {

class Document;
class AbstractDrawingTool;
class DrawingTool;
class TextElement;

class Canvas {
	public:
		class Callback {
		public:
			virtual void updateView( Canvas* canvas, const CRgn& region ) = NULL;
		};
		
		enum DrawingToolType {
			dtPen, dtBrush, dtLine, dtArrow, dtRectangle, dtText, dtCrop, dtMove, dtSelection
		};

		enum UndoHistoryItemType { uitDocumentChanged, uitElementAdded, uitElementRemoved, uitElementPositionChanged};
		struct UndoHistoryItem {
			UndoHistoryItemType type;
			MovableElement * element;
		};
		

		Canvas( HWND parent );
		~Canvas();
		void setDocument( Document *doc );
		void setSize( int x, int y );
		void mouseMove( int x, int y, DWORD flags);
		void mouseDown( int button, int x, int y );
		void mouseUp( int button, int x, int y );
		void mouseDoubleClick( int button, int x, int y );

		Document* currentDocument() const;
		void render(Painter* gr, const RECT& rect);
		void setCallback(Callback * callback);
		void setPenSize(int size);
		void setForegroundColor(Gdiplus::Color color);
		void setBackgroundColor(Gdiplus::Color color);
		void setDrawingToolType(DrawingToolType tool);
		AbstractDrawingTool* getCurrentDrawingTool();
		void addMovableElement(MovableElement* element);
		void deleteMovableElement(MovableElement* element);
		void deleteMovableElements(ElementType elementType);
		void getElementsByType(ElementType elementType, std::vector<MovableElement*>& out);
		void setOverlay(MovableElement* overlay);
		void setZoomFactor(float zoomFactor);
		Gdiplus::Bitmap* getBufferBitmap();
	
		float getZoomFactor() const;
		MovableElement* getElementAtPosition(int x, int y);
		int deleteElementsByType(ElementType elementType);
		int getWidth() const;
		int getHeigth() const;
		CursorType getCursor() const;
		bool undo();
		InputBox* getInputBox( const RECT& rect ); 
		TextElement* getCurrentlyEditedTextElement();
		void setCurrentlyEditedTextElement(TextElement* textElement);
		void unselectAllElements();
		HWND getRichEditControl();
		void updateView();
		bool addDrawingElementToDoc(DrawingElement* element);
		fastdelegate::FastDelegate4<int,int,int,int> onCropChanged;
		friend class AbstractDrawingTool;
		friend class VectorElementTool;
		friend class PenTool;
		friend class BrushTool;
		friend class MoveAndResizeTool;
		friend class CropTool;
	private:
		void init();
		void updateView( RECT boundingRect );
		void updateView( const CRgn& region );

		void createDoubleBuffer();
		void setCursor(CursorType cursor);

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
		MovableElement* selection_;
		float zoomFactor_;
		TextElement* currentlyEditedTextElement_;
		Gdiplus::Color foregroundColor_;
		Gdiplus::Color backgroundColor_;
		std::stack<UndoHistoryItem> undoHistory_;
		std::vector<MovableElement*> elementsToDelete_;
		int penSize_;
		
		HWND parentWindow_;
		InputBoxControl* inputBox_;
};

}


#endif