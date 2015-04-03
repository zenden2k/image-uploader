#include "Region.h"

#include <3rdpart/GdiplusH.h>

namespace ImageEditor {

Region::Region(int x,int y, int w, int h)
{
	rgn_.reset(new Gdiplus::Region(Gdiplus::Rect(x,y,w,h)));
}

Region::Region(Gdiplus::Region* rgn)
{
	rgn_.reset(rgn);
}

Region::~Region()
{
	//delete rgn_;
}

Region Region::intersected(const Region & r) const
{
	Gdiplus::Region* rgn = rgn_->Clone();
	rgn->Intersect(r.toNativeRegion().get());
	return rgn;
}

Region Region::subtracted(const Region & r) const
{
	Gdiplus::Region* rgn = rgn_->Clone();

	rgn->Exclude(r.toNativeRegion().get());
	return rgn;
}

Region Region::united(const Region & r) const
{
	Gdiplus::Region* rgn = rgn_->Clone();

	rgn->Union(r.toNativeRegion().get());
	return rgn;
}

std_tr::shared_ptr<Gdiplus::Region> Region::toNativeRegion() const
{
	return rgn_;
}

}
