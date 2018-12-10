#ifndef IMAGEEDITOR_CANVAS_H
#define IMAGEEDITOR_CANVAS_H

#include "3rdpart/GdiPlusH.h"
#include "InputBox.h"
#include <ImageEditor/Gui/InputBoxControl.h>
#include "MovableElement.h"
#include <vector>
#include "Core/3rdpart/FastDelegate.h"
#include <stack>
#include "Core/Utils/CoreTypes.h"

namespace ImageEditor {

class Document;
class AbstractDrawingTool;
class DrawingTool;
class TextElement;
class InputBoxControl;

class Canvas {
    public:
        class Callback {
        public:
            virtual void updateView( Canvas* canvas, Gdiplus::Rect rect ) = NULL;
        };
        
        enum DrawingToolType {
            dtNone, dtPen, dtBrush, dtLine, dtArrow, dtRectangle, dtFilledRectangle, dtText, dtCrop, dtMove, dtSelection, dtBlur, dtBlurrringRectangle, dtColorPicker,
            dtRoundedRectangle, dtEllipse, dtFilledRoundedRectangle, dtFilledEllipse, dtMarker
        };

        enum UndoHistoryItemType { uitDocumentChanged, uitElementAdded, uitElementRemoved, 
            uitElementPositionChanged, uitElementForegroundColorChanged, uitElementBackgroundColorChanged,
            uitPenSizeChanged, uitFontChanged, uitTextChanged, uitRoundingRadiusChanged
        };
        enum { kMaxPenSize = 50, kMaxRoundingRadius = 50 };
        struct UndoHistoryItemElement {
            MovableElement * movableElement;
            int pos;
            POINT startPoint;
            POINT endPoint;
            Gdiplus::Color color;
            int penSize; // pensize or rounding radius
            std::string rawText;
            UndoHistoryItemElement() {
                pos = -1;
                startPoint.x = -1;
                startPoint.y = -1;
                endPoint.x = -1;
                endPoint.y = -1;
                penSize = -1;
                movableElement = nullptr;
            }
        };
        struct UndoHistoryItem {
            UndoHistoryItemType type;
            
            std::vector<UndoHistoryItemElement> elements;
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
        //void render(Painter* gr, const RECT& rect, POINT scrollOffset, SIZE size);
        void render(HDC dc, const RECT& rect, POINT scrollOffset, SIZE size);
        void setCallback(Callback * callback);
        void setPenSize(int size);
        int getPenSize() const;
        void beginPenSizeChanging();
        void endPenSizeChanging(int penSize);

        void setRoundingRadius(int radius);
        int getRoundingRadius() const;
        void beginRoundingRadiusChanging();
        void endRoundingRadiusChanging(int radius);


        void setForegroundColor(Gdiplus::Color color);
        void setBackgroundColor(Gdiplus::Color color);
        Gdiplus::Color getForegroundColor() const;
        Gdiplus::Color getBackgroundColor() const;
        void setFont(LOGFONT font, DWORD changeMask = CFM_FACE | CFM_SIZE | CFM_CHARSET 
            | CFM_BOLD | CFM_ITALIC | CFM_UNDERLINE | CFM_STRIKEOUT | CFM_OFFSET);
        LOGFONT getFont();
        AbstractDrawingTool* setDrawingToolType(DrawingToolType tool, bool notify = false);
        void setPreviousDrawingTool();
        AbstractDrawingTool* getCurrentDrawingTool();
        void addMovableElement(MovableElement* element);
        void deleteMovableElement(MovableElement* element);
        void deleteMovableElements(ElementType elementType);
        void getElementsByType(ElementType elementType, std::vector<MovableElement*>& out);
        //void setOverlay(MovableElement* overlay);
        void setZoomFactor(float zoomFactor);
        Gdiplus::Bitmap* getBufferBitmap();
        void addUndoHistoryItem(const UndoHistoryItem& item);
        std::shared_ptr<Gdiplus::Bitmap> getBitmapForExport();
    
        float getZoomFactor() const;
        MovableElement* getElementAtPosition(int x, int y, ElementType et = etNone);
        int deleteElementsByType(ElementType elementType);
        int getWidth() const;
        int getHeigth() const;
        CursorType getCursor() const;
        bool undo();
        InputBox* getInputBox( const RECT& rect ); 
        TextElement* getCurrentlyEditedTextElement();
        void setCurrentlyEditedTextElement(TextElement* textElement);
        int unselectAllElements();
        HWND getRichEditControl();
        void updateView();
        void updateView( RECT boundingRect );
        bool addDrawingElementToDoc(DrawingElement* element);
        void endDocDrawing();
        int deleteSelectedElements();
        float getBlurRadius();
        void setBlurRadius(float radius);
        bool hasBlurRectangles();
        void showOverlay(bool show);
        void selectionChanged();
        Gdiplus::Rect currentRenderingRect();
        bool isRoundingRectangleSelected(); 
        bool isDocumentModified();
        void setDocumentModified(bool modified);
        fastdelegate::FastDelegate4<int,int,int,int> onCropChanged;
        fastdelegate::FastDelegate4<int,int,int,int> onCropFinished;
        fastdelegate::FastDelegate1<DrawingToolType> onDrawingToolChanged;
        fastdelegate::FastDelegate1<Gdiplus::Color> onForegroundColorChanged;
        fastdelegate::FastDelegate1<Gdiplus::Color> onBackgroundColorChanged;
        fastdelegate::FastDelegate1<LOGFONT> onFontChanged;
        fastdelegate::FastDelegate1<TextElement*> onTextEditStarted;
        fastdelegate::FastDelegate1<TextElement*> onTextEditFinished;
        fastdelegate::FastDelegate0<void> onSelectionChanged;

        friend class AbstractDrawingTool;
        friend class VectorElementTool;
        friend class PenTool;
        friend class BrushTool;
        friend class MoveAndResizeTool;
        friend class CropTool;
        POINT GetScrollOffset() const;
private:
        void init();
        
        //void updateView( const CRgn& region );

        void createDoubleBuffer();
        void setCursor(CursorType cursor);
        void renderInBuffer(Gdiplus::Rect  rect, bool forExport =false);

        std::shared_ptr<Gdiplus::Bitmap> buffer_;
        Document* doc_;
        Gdiplus::Graphics* bufferedGr_;
        float blurRadius_;
        int canvasWidth_, canvasHeight_;
        POINT oldPoint_;
        POINT leftMouseDownPoint_;
        POINT leftMouseUpPoint_;
        bool isDocumentModified_;
        Callback* callback_;
        DrawingToolType drawingToolType_;
        DrawingToolType previousDrawingTool_;
        AbstractDrawingTool* currentDrawingTool_;
        std::vector<MovableElement*> elementsOnCanvas_;
        CursorType currentCursor_;
        MovableElement* overlay_;
        MovableElement* selection_;
        bool showOverlay_;
        float zoomFactor_;
        TextElement* currentlyEditedTextElement_;
        Gdiplus::Color foregroundColor_;
        Gdiplus::Color backgroundColor_;
        Gdiplus::Rect currentRenderingRect_;
        std::stack<UndoHistoryItem> undoHistory_;
        std::vector<MovableElement*> elementsToDelete_;
        int penSize_;
        int originalPenSize_;
        int roundingRadius_;
        int originalRoundingRadius_;
        bool canvasChanged_;
        bool fullRender_;
        int blurRectanglesCount_;
        
        Gdiplus::Rect updatedRect_;
        LOGFONT font_;
        POINT scrollOffset_;
        
        HWND parentWindow_;
        InputBoxControl* inputBox_;
};

}


#endif