#ifndef IMAGEEDITOR_MOVABLEELEMENT_H
#define IMAGEEDITOR_MOVABLEELEMENT_H

#include <GdiPlus.h>
#include <map>
#include <deque>
#include "BasicTypes.h"
#include "DrawingElement.h"

namespace ImageEditor {

class AffectedSegments;

class MovableElement: public DrawingElement {
	public:
		
		enum { kGripSize = 6 };
		MovableElement();
		void render(Painter* gr);
		void renderGrips(Painter* gr);
		void setSelected(bool selected);
		bool isSelected() const;
		virtual ElementType getType() const;
		static CursorType GetCursorForBoundary(BoundaryType bt);
		friend class MovableElementTool;
		int getX();
		int getY();
		void setX(int  x);
		void setY(int y);

		virtual void resize(int width, int height);


	protected:
		bool isSelected_;
		POINT grips_[8];
		
		POINT* getMaxPoint(Axis axis);
		POINT* getMinPoint(Axis axis);
};

}

#endif