#ifndef IMAGEEDITOR_REGION_H
#define IMAGEEDITOR_REGION_H

#include <GdiPlus.h>

#include "atlheaders.h"

namespace ImageEditor 
{
	class Region {
	public:
		Region(int x,int y, int w, int h);
		~Region();
		#ifndef QT_VERSION
		Region(Gdiplus::Region* rgn );
		#endif
		Region intersected(const Region & r) const;
		Region subtracted(const Region & r) const;
		Region united(const Region & r) const;
		Gdiplus::Region* toNativeRegion() const;
	private:
#ifndef QT_VERSION
		Gdiplus::Region* rgn_;
#endif
	};
}
#endif