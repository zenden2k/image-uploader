#ifndef IMAGEEDITOR_MOVABLEELEMENT_H
#define IMAGEEDITOR_MOVABLEELEMENT_H

#include <GdiPlus.h>
#include <map>
#include <deque>
#include "DrawingElement.h"

namespace ImageEditor {

class AffectedSegments;

class MovableElement: public DrawingElement {
	public:
		enum ElementType { etUnknown, etCrop , etArrow,  };
		MovableElement();
		void render(Gdiplus::Graphics* gr);
		void setSelected(bool selected);
		bool isSelected() const;
		ElementType getType() const;
	protected:
		bool isSelected_;
};

}

#endif