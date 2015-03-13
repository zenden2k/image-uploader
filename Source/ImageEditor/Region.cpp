#include "Region.h"

#include <GdiPlus.h>

namespace ImageEditor {

Region::Region(int x,int y, int w, int h)
{
	rgn_ = new Gdiplus::Region(Gdiplus::Rect(x,y,w,h));
}

Region::Region(Gdiplus::Region* rgn)
{
	rgn_ = rgn;
}

Region::~Region()
{
	//delete rgn_;
}

Region Region::intersected(const Region & r) const
{
	Gdiplus::Region* rgn = rgn_->Clone();
	rgn->Intersect(r.toNativeRegion());
	return rgn;
}

Region Region::subtracted(const Region & r) const
{
	Gdiplus::Region* rgn = rgn_->Clone();

	rgn->Exclude(r.toNativeRegion());
	return rgn;
}

Region Region::united(const Region & r) const
{
	Gdiplus::Region* rgn = rgn_->Clone();

	rgn->Union(r.toNativeRegion());
	return rgn;
}

Gdiplus::Region* Region::toNativeRegion() const
{
	return rgn_;
}

}
