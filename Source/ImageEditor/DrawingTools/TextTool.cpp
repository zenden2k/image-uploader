#include "TextTool.h"

#include "../Canvas.h"
#include "../Document.h"
#include "../MovableElements.h"
#include <Core/Utils/CoreUtils.h>
#include <Core/Logging.h>

#include <math.h>
#include <cassert>
#include <gdiplus.h>
#include <math.h>

namespace ImageEditor {

TextTool::TextTool( Canvas* canvas ) : MoveAndResizeTool( canvas, etText ) {

}

void TextTool::beginDraw( int x, int y ) {
	if ( currentElement_ && dynamic_cast<TextElement*>(currentElement_)->getInputBox()->isVisible() ) {
		allowCreatingElements_ = false;
	}
	MoveAndResizeTool::beginDraw( x, y );
	allowCreatingElements_ = true;
	if ( currentElement_ ) {

		InputBox* input = dynamic_cast<TextElement*>(currentElement_)->getInputBox();
		if ( input ) {
			input->show(true);
		}
	}

}

void TextTool::continueDraw( int x, int y, DWORD flags ) {
	MoveAndResizeTool::continueDraw( x, y ,flags);
}

void TextTool::endDraw( int x, int y ) {
	if (!currentElement_) {
		return;
	}
	int xStart = min( startPoint_.x, x );
	int xEnd   = max( startPoint_.x, x );

	int yStart = min( startPoint_.y, y );
	int yEnd   = max( startPoint_.y, y );

	int width = currentElement_->getWidth();
	int height = currentElement_->getHeight();
	if (  width < 300 ) {
		width = 300;
	}

	if ( height < 30 ) {
		height = 30;
	}
	currentElement_->resize(width, height);
	int elX = currentElement_->getX();
	int elY = currentElement_->getY();
	RECT inputRect = {elX, elY, elX + currentElement_->getWidth(), elY + currentElement_->getHeight() };

	TextElement * textElement = dynamic_cast<TextElement*>(currentElement_);
	InputBox * inputBox = textElement->getInputBox();
	if ( !inputBox ) {
		inputBox = canvas_->getInputBox( inputRect );
	}
	//	currentElement_ = new TextElement(canvas_,inputBox, xStart,yStart, xEnd, yEnd);
	textElement->setInputBox(inputBox);
	canvas_->setCurrentlyEditedTextElement(textElement);
	textElement->setSelected(true);
	inputBox->invalidate();
	textElement->setDrawDashedRectangle(false);
	//currentElement_ = new TextElement(canvas_,inputBox, xStart,yStart, xEnd, yEnd);
	//canvas_->addMovableElement(currentElement_);
	//MoveAndResizeTool::endDraw( x, y);
	canvas_->updateView();

}

void TextTool::render( Painter* gr ) {

}

ImageEditor::CursorType TextTool::getCursor(int x, int y)
{
	CursorType ct = MoveAndResizeTool::getCursor(x,y);
	InputBox* inputBox = currentElement_ ? dynamic_cast<TextElement*>(currentElement_)->getInputBox(): NULL;
	if ( (ct == ctDefault || ( ct == ctMove && canvas_->getElementAtPosition(x,y)!= currentElement_)) && 
		( !inputBox || !inputBox->isVisible() )) {
			ct = ctEdit;
	}
	return ct;
}

}