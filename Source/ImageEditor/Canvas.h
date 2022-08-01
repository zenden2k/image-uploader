#ifndef IMAGEEDITOR_CANVAS_H
#define IMAGEEDITOR_CANVAS_H

#include <vector>
#include <stack>

#include <boost/signals2.hpp>

#include "3rdpart/GdiplusH.h"
#include "InputBox.h"
#include "ImageEditor/Gui/InputBoxControl.h"
#include "MovableElement.h"
#include "Core/Utils/CoreTypes.h"
#include "MovableElements.h"

namespace ImageEditor {

class Document;
class AbstractDrawingTool;
class DrawingTool;
class TextElement;
class InputBoxControl;

enum class DrawingToolType {
    dtNone, dtPen, dtBrush, dtLine, dtArrow, dtRectangle, dtFilledRectangle, dtText, dtCrop, dtMove, dtSelection,
    dtBlur, dtBlurrringRectangle, dtPixelateRectangle, dtColorPicker,dtRoundedRectangle, dtEllipse,
    dtFilledRoundedRectangle, dtFilledEllipse, dtMarker, dtStepNumber
};

class Canvas {
    public:
        class Callback {
            public:
                virtual void updateView(Canvas* canvas, Gdiplus::Rect rect) = 0;
                virtual ~Callback(){}
        }; 

        enum class UndoHistoryItemType { uitDocumentChanged, uitElementAdded, uitElementRemoved, 
            uitElementPositionChanged, uitElementForegroundColorChanged, uitElementBackgroundColorChanged,
            uitPenSizeChanged, uitFontChanged, uitTextChanged, uitRoundingRadiusChanged, uitFillBackgroundChanged
        };
        enum { kMaxPenSize = 50, kMaxRoundingRadius = 50, kDefaultStepFontSize = 14 };

        struct UndoHistoryItemElement {
            MovableElement* movableElement;
            int pos;
            POINT startPoint{};
            POINT endPoint{};
            Gdiplus::Color color;
            int penSize; // pen size or rounding radius or fill background
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

        explicit Canvas( HWND parent );
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

        Gdiplus::Color getStepForegroundColor() const;
        Gdiplus::Color getStepBackgroundColor() const;
        void setStepColors(Gdiplus::Color fgColor, Gdiplus::Color bgColor);
        bool isStepColorSet() const;
        void setFont(LOGFONT font, DWORD changeMask = CFM_FACE | CFM_SIZE | CFM_CHARSET 
            | CFM_BOLD | CFM_ITALIC | CFM_UNDERLINE | CFM_STRIKEOUT | CFM_OFFSET);
        LOGFONT getFont() const;
        AbstractDrawingTool* setDrawingToolType(DrawingToolType tool, bool notify = false);

        // This method is used by ColorPickerTool
        void setPreviousDrawingTool();

        AbstractDrawingTool* getCurrentDrawingTool() const;
        void addMovableElement(MovableElement* element);
        void deleteMovableElement(MovableElement* element);
        //void deleteMovableElements(ElementType elementType);
        void getElementsByType(ElementType elementType, std::vector<MovableElement*>& out) const;
        void setZoomFactor(float zoomFactor);
        Gdiplus::Bitmap* getBufferBitmap() const;
        void addUndoHistoryItem(const UndoHistoryItem& item);
        std::shared_ptr<Gdiplus::Bitmap> getBitmapForExport();
    
        float getZoomFactor() const;
        MovableElement* getElementAtPosition(int x, int y, ElementType et = ElementType::etNone);
        int deleteElementsByType(ElementType elementType);
        int getWidth() const;
        int getHeigth() const;
        CursorType getCursor() const;
        bool undo();
        InputBox* getInputBox( const RECT& rect ); 
        TextElement* getCurrentlyEditedTextElement() const;
        void setCurrentlyEditedTextElement(TextElement* textElement);
        int unselectAllElements();
        bool unselectElement(MovableElement* element);
        HWND getRichEditControl() const;
        void updateView();
        void updateView( RECT boundingRect );
        bool addDrawingElementToDoc(DrawingElement* element);
        void endDocDrawing();
        int deleteSelectedElements();
        float getBlurRadius() const;
        void setBlurRadius(float radius);
        bool hasBlurRectangles() const;
        int getPixelateBlockSize() const;
        void showOverlay(bool show);
        void selectionChanged();
        Gdiplus::Rect currentRenderingRect() const;
        bool isRoundingRectangleSelected() const; 
        bool isDocumentModified() const;
        void setDocumentModified(bool modified);
        int getNextNumber();

        void setStepFontSize(int fontSize);
        int getStepFontSize() const;

        void setStepInitialValue(int value);

        void setFillTextBackground(bool fill);
        bool getFillTextBackground() const;

        void setArrowMode(Arrow::ArrowMode arrowMode);
        Arrow::ArrowMode getArrowMode() const;
        Gdiplus::Graphics* getGraphicsDevice() const;

        Gdiplus::Rect lastAppliedCrop() const;

        boost::signals2::signal<void(int,int,int,int)> onCropChanged;
        boost::signals2::signal<void(int,int,int,int)> onCropFinished;
        boost::signals2::signal<void(DrawingToolType)> onDrawingToolChanged;
        boost::signals2::signal<void(Gdiplus::Color)> onForegroundColorChanged;
        boost::signals2::signal<void(Gdiplus::Color)> onBackgroundColorChanged;
        boost::signals2::signal<void(LOGFONT)> onFontChanged;
        boost::signals2::signal<void(TextElement*)> onTextEditStarted;
        boost::signals2::signal<void(TextElement*)> onTextEditFinished;
        boost::signals2::signal<void()> onSelectionChanged;
        boost::signals2::signal<void()> onDocumentModified;
	
        friend class AbstractDrawingTool;
        friend class VectorElementTool;
        friend class PenTool;
        friend class BrushTool;
        friend class MoveAndResizeTool;
        friend class CropTool;
        POINT GetScrollOffset() const;
private:
        void createDoubleBuffer();
        void setCursor(CursorType cursor);
        void renderInBuffer(Gdiplus::Rect rc, bool forExport =false);
        void recalcStepNumbers();

        std::shared_ptr<Gdiplus::Bitmap> buffer_;
        Document* doc_;
        Gdiplus::Graphics* bufferedGr_;
        float blurRadius_;
        int pixelateBlockSize_;
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
        Gdiplus::Color foregroundColor_, backgroundColor_,
            stepForegroundColor_, stepBackgroundColor_;
        bool stepColorsSet_;
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
        int stepInitialValue_;
        int nextNumber_; // next number for element StepNumber
        int stepFontSize_;
        bool fillTextBackground_;
        Arrow::ArrowMode arrowMode_;
        
        Gdiplus::Rect updatedRect_;
        LOGFONT font_;
        POINT scrollOffset_;
        
        HWND parentWindow_;
        InputBoxControl* inputBox_;
        Gdiplus::Rect lastAppliedCrop_;
};

}

#endif