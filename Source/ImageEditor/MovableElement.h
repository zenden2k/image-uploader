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
		enum GripPointType {gptNone, gptStartPoint, gptEndPoint};
		struct Grip {
			POINT pt;
			BoundaryType bt;
			GripPointType gpt; 
			Grip() {
				pt.x = -1;
				pt.y = -1;
				bt = btNone;
				gpt = gptNone;
			}
		};
		MovableElement(Canvas* canvas_);
		void render(Painter* gr);
		void renderGrips(Painter* gr);
		virtual void setSelected(bool selected);
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