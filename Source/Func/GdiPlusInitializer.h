#pragma once

#include <Windows.h>
#include <boost/noncopyable.hpp>

class GdiPlusInitializer : boost::noncopyable
{
public:
	GdiPlusInitializer();

	~GdiPlusInitializer();

private:
	ULONG_PTR token_;
};
