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
        void render(Painter* gr) override;
        void setEndPoint(POINT endPoint) override;
        void getAffectedSegments(AffectedSegments* segments) override;
        virtual void createGrips() override;
        virtual bool isItemAtPos(int x, int y) override;
        virtual ElementType getType() const override;
};

class TextElement: public MovableElement{
    public:
        TextElement( Canvas* canvas, InputBox* inputBox, int startX, int startY, int endX,int endY );
        ~TextElement();
        void render(Painter* gr) override;
        void getAffectedSegments(AffectedSegments* segments) override;
        virtual void resize(int width, int height) override;
        void setInputBox(InputBox* inputBox);
        void setFont(LOGFONT font,  DWORD changeMask);
        LOGFONT getFont();
        InputBox* getInputBox() const;
        virtual ElementType getType() const override;
        virtual void setSelected(bool selected) override;
        void beginEdit();
        void endEdit(bool saveToHistory);
        void setRawText(const std::string& rawText);
        virtual void setColor(Gdiplus::Color color) override;
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
    void render(Painter* gr) override;
    void getAffectedSegments(AffectedSegments* segments) override;
    virtual ElementType getType() const override;
    virtual void setPos(int  x, int y) override;
    virtual bool move(int  offsetX, int offsetY) override;
    virtual void resize(int width, int height) override;
};

class CropOverlay: public MovableElement {
public:
    CropOverlay(Canvas* canvas, int startX, int startY, int endX,int endY);
    void render(Painter* gr) override;
};

class BlurringRectangle: public MovableElement {
public:
    BlurringRectangle(Canvas* canvas, float blurRadius, int startX, int startY, int endX,int endY);
    ~BlurringRectangle();
    void setBlurRadius(float radius);
    float getBlurRadius();
    void render(Painter* gr) override;
    virtual ElementType getType() const override;
protected:
    float blurRadius_;

};

class Rectangle: public MovableElement {
public:
    Rectangle(Canvas* canvas, int startX, int startY, int endX,int endY,bool filled = false );
    void render(Painter* gr) override;
    void getAffectedSegments(AffectedSegments* segments) override;
    virtual bool isItemAtPos(int x, int y) override;
    virtual RECT getPaintBoundingRect() override;

    virtual ElementType getType() const override;

protected:
    bool filled_;

};

class FilledRectangle: public Rectangle {
public:
    FilledRectangle(Canvas* canvas, int startX, int startY, int endX,int endY );
    virtual ElementType getType() const override;
};

class RoundedRectangle: public Rectangle {
public:
    RoundedRectangle(Canvas* canvas, int startX, int startY, int endX,int endY,bool filled = false );
    void render(Painter* gr) override;
    virtual ElementType getType() const override;
};

class FilledRoundedRectangle: public RoundedRectangle {
public:
    FilledRoundedRectangle(Canvas* canvas, int startX, int startY, int endX,int endY );
    virtual ElementType getType() const override;
};

class Arrow: public Line {
public:
    Arrow(Canvas* canvas,int startX, int startY, int endX,int endY);
    void render(Painter* gr) override;
    virtual RECT getPaintBoundingRect() override;
    virtual ElementType getType() const override;
};

class Ellipse: public MovableElement {
public:
    Ellipse(Canvas* canvas, bool filled = false );
    void render(Painter* gr) override;
    virtual bool isItemAtPos(int x, int y) override;
    virtual ElementType getType() const override;
    virtual RECT getPaintBoundingRect() override;
protected:
    bool filled_;
    bool ContainsPoint(Gdiplus::Rect ellipse, Gdiplus::Point location);
    virtual void createGrips() override;
};

class FilledEllipse: public Ellipse {
public:
    FilledEllipse(Canvas* canvas );
    virtual ElementType getType() const override;
};

class Selection: public MovableElement {
public:
    Selection(Canvas* canvas, int startX, int startY, int endX,int endY);
    void render(Painter* gr) override;
    virtual ElementType getType() const override;
    virtual void createGrips() override;
};

}

#endif