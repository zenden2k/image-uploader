#ifndef IMAGEEDITOR_MOVABLEELEMENT_H
#define IMAGEEDITOR_MOVABLEELEMENT_H

#include <GdiPlus.h>
#include <map>
#include <deque>
#include <vector>
#include "BasicTypes.h"
#include "DrawingElement.h"

namespace ImageEditor {
class Canvas;
class AffectedSegments;

class MovableElement: public DrawingElement {
	public:
		
		enum { kGripSize = 6 ,kSelectRadius = 5};
		struct Grip {
			POINT pt;
			BoundaryType bt;
		};
		MovableElement(Canvas* canvas_);
		void render(Painter* gr);
		void renderGrips(Painter* gr);
		void setSelected(bool selected);
		bool isSelected() const;
		virtual ElementType getType() const;
		static CursorType GetCursorForBoundary(BoundaryType bt);
		friend class MoveAndResizeTool;
		int getX();
		int getY();
		void setX(int  x);
		void setY(int y);
		virtual bool isItemAtPos(int x, int y);

		virtual void resize(int width, int height);
		virtual void createGrips();


	protected:
		bool isSelected_;
		bool drawDashedRectangle_;
		std::vector<Grip> grips_;
		
		POINT* getMaxPoint(Axis axis);
		POINT* getMinPoint(Axis axis);
};

}

#endif