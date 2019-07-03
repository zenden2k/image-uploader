#ifndef IMAGEEDITOR_DOCUMENT_H
#define IMAGEEDITOR_DOCUMENT_H

#include <vector>
#include <windows.h>
#include "3rdpart/GdiplusH.h"
#include "BasicTypes.h"
#include "DrawingElement.h"
#include "Core/Utils/CoreTypes.h"

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
        explicit Document(const wchar_t* fileName);
        Document(std::shared_ptr<Gdiplus::Bitmap> sourceImage, bool hasTransparentPixels = false);
        virtual ~Document();
        
        Painter* getGraphicsObject() const;
        void beginDrawing( bool cloneImage = true );
        void addDrawingElement(DrawingElement *element);
        void endDrawing();

        void addAffectedSegments(const AffectedSegments& segments);
        Gdiplus::Bitmap* getBitmap() const;
        void render(Painter* gr, Gdiplus::Rect rc);
        bool undo();
        int getWidth() const;
        int getHeight() const;
        bool isNull() const;
        bool hasTransparentPixels() const;
    private:
        std::shared_ptr<Gdiplus::Bitmap> currentImage_;
        Gdiplus::Bitmap* originalImage_;
        Painter* currentCanvas_;
        std::vector<HistoryItem> history_;
        bool drawStarted_;
        AffectedSegments changedSegments_;
        bool hasTransparentPixels_;
        void init();
        void saveDocumentState( );
        void checkTransparentPixels();
        DISALLOW_COPY_AND_ASSIGN(Document);
};

}

#endif