#ifndef IMAGEEDITOR_DOCUMENT_H
#define IMAGEEDITOR_DOCUMENT_H

#include <memory>
#include <vector>

#include <Windows.h>
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
            bool full = false;
            int width = 0;
            int height = 0;
            std::shared_ptr<Gdiplus::Bitmap> bmp;
        };
        Document(int width, int height);
        explicit Document(const wchar_t* fileName);
        explicit Document(std::shared_ptr<Gdiplus::Bitmap> sourceImage, bool hasTransparentPixels = false);
        virtual ~Document();
        
        Painter* getGraphicsObject() const;
        void beginDrawing( bool cloneImage = true );
        void addDrawingElement(DrawingElement *element);
        void endDrawing();

        void addAffectedSegments(const AffectedSegments& segments);
        void applyCrop(int cropX, int cropY, int cropWidth, int cropHeight);
        Gdiplus::Bitmap* getBitmap() const;
        void render(Painter* gr, Gdiplus::Rect rc);
        bool undo();
        int getWidth() const;
        int getHeight() const;
        bool isNull() const;
        bool hasTransparentPixels() const;
        bool isSrcMultiFrame() const;
    private:
        std::shared_ptr<Gdiplus::Bitmap> currentImage_;
        Gdiplus::Bitmap* originalImage_;
        Painter* currentCanvas_;
        std::vector<HistoryItem> history_;
        bool drawStarted_;
        AffectedSegments changedSegments_;
        bool hasTransparentPixels_;
        bool isSrcMultiFrame_ = false;
        void init();
        void saveDocumentState(bool full = false);
        void checkTransparentPixels();
        
        DISALLOW_COPY_AND_ASSIGN(Document);
};

}

#endif