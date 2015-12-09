#ifndef IMAGEEDITOR_MOVABLEELEMENTS_H
#define IMAGEEDITOR_MOVABLEELEMENTS_H

#include "3rdpart/GdiplusH.h"
#include "DrawingElement.h"
#include "MovableElement.h"

namespace ImageEditor {
class InputBox;
class Line: public MovableElement {
    public:
        Line(Canvas* canvas,int startX, int startY, int endX,int endY);
        void render(Painter* gr);
        void setEndPoint(POINT endPoint);
        void getAffectedSegments( AffectedSegments* segments );
        virtual void createGrips();
        virtual bool isItemAtPos(int x, int y);
        virtual ElementType getType() const;
};

class TextElement: public MovableElement{
    public:
        TextElement( Canvas* canvas, InputBox* inputBox, int startX, int startY, int endX,int endY );
        ~TextElement();
        void render(Painter* gr);
        void getAffectedSegments( AffectedSegments* segments );
        virtual void resize(int width, int height);
        void setInputBox(InputBox* inputBox);
        void setFont(LOGFONT font,  DWORD changeMask);
        LOGFONT getFont();
        InputBox* getInputBox() const;
        virtual ElementType getType() const;
        virtual void setSelected(bool selected);
        void beginEdit();
        void endEdit(bool saveToHistory);
        void setRawText(const std::string& rawText);
        virtual void setColor(Gdiplus::Color color);
protected:
    InputBox *inputBox_;
    LOGFONT font_;
    bool isEditing_;
    std::string originalRawText_;
    void onTextChanged(TCHAR *text);
    void onEditCanceled();
    void onEditFinished();
    void onControlResized(int w, int h);
    void setTextColor();
    void onSelectionChanged(int min, int max, LOGFONT font);
    

};

class Crop: public MovableElement {
public:
    Crop(Canvas* canvas, int startX, int startY, int endX,int endY);
    void render(Painter* gr);
    void getAffectedSegments( AffectedSegments* segments );
    virtual ElementType getType() const;
    virtual void setPos(int  x, int y) override;
    virtual bool move(int  offsetX, int offsetY) override;

};

class CropOverlay: public MovableElement {
public:
    CropOverlay(Canvas* canvas, int startX, int startY, int endX,int endY);
    void render(Painter* gr);
};

class BlurringRectangle: public MovableElement {
public:
    BlurringRectangle(Canvas* canvas, float blurRadius, int startX, int startY, int endX,int endY);
    ~BlurringRectangle();
    void setBlurRadius(float radius);
    float getBlurRadius();
    void render(Painter* gr);
    virtual ElementType getType() const;
protected:
    float blurRadius_;

};

class Rectangle: public MovableElement {
public:
    Rectangle(Canvas* canvas, int startX, int startY, int endX,int endY,bool filled = false );
    void render(Painter* gr);
    void getAffectedSegments( AffectedSegments* segments );
    virtual bool isItemAtPos(int x, int y);
    virtual RECT getPaintBoundingRect();

    virtual ElementType getType() const;

protected:
    bool filled_;

};

class FilledRectangle: public Rectangle {
public:
    FilledRectangle(Canvas* canvas, int startX, int startY, int endX,int endY );
    virtual ElementType getType() const;
};

class RoundedRectangle: public Rectangle {
public:
    RoundedRectangle(Canvas* canvas, int startX, int startY, int endX,int endY,bool filled = false );
    void render(Painter* gr);
    virtual ElementType getType() const;
};

class FilledRoundedRectangle: public RoundedRectangle {
public:
    FilledRoundedRectangle(Canvas* canvas, int startX, int startY, int endX,int endY );
    virtual ElementType getType() const;
};

class Arrow: public Line {
public:
    Arrow(Canvas* canvas,int startX, int startY, int endX,int endY);
    void render(Painter* gr);
    virtual RECT getPaintBoundingRect();
    virtual ElementType getType() const;
};

class Ellipse: public MovableElement {
public:
    Ellipse(Canvas* canvas, bool filled = false );
    void render(Painter* gr);
    virtual bool isItemAtPos(int x, int y);
    virtual ElementType getType() const;
    virtual RECT getPaintBoundingRect();
protected:
    bool filled_;
    bool ContainsPoint(Gdiplus::Rect ellipse, Gdiplus::Point location);
    virtual void createGrips();
};

class FilledEllipse: public Ellipse {
public:
    FilledEllipse(Canvas* canvas );
    virtual ElementType getType() const;
};

class Selection: public MovableElement {
public:
    Selection(Canvas* canvas, int startX, int startY, int endX,int endY);
    void render(Painter* gr);
    virtual ElementType getType() const;
    virtual void createGrips();

};

}

#endif