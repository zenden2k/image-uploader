/***************************************************************************
 * 
 * $Id$
 * 
 **************************************************************************/

/**
 * @file $HeadURL$
 * @author $Author$(hoping@baimashi.com)
 * @date $Date$
 * @version $Revision$
 * @brief 
 *  
 **/

#include "Object.h"
#include <stdexcept>

CObject::CObject()
{
	mReferences = 1;
}

CObject::~CObject()
{
	/*
    Raising exception inside the destructor is illegal. 
    if (mReferences != 1)
	{
		throw std::runtime_error("something wrong, reference count not zero");
	}*/
}

void CObject::retain()
{
	mReferences++;
}

void CObject::release()
{
	/*if (mReferences < 0)
	{
        throw std::runtime_error("something wrong, reference count is negative");
	}*/

	if (mReferences == 1)
	{
		delete this;
	}
	else
	{
		mReferences--;
	}
}

unsigned int CObject::references()
{
	return mReferences;
}

/* vim: set ts=4 sw=4 sts=4 tw=100 noet: */

