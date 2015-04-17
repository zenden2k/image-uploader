#ifndef IU_CORE_UPLOAD_UPLOADRESULT_H
#define IU_CORE_UPLOAD_UPLOADRESULT_H
#include "Core/Utils/CoreTypes.h"

#pragma once

struct UploadResult
{
public:
	std::string directUrl;
	std::string directUrlShortened;
	std::string thumbUrl;
	std::string downloadUrl;
	std::string downloadUrlShortened;

	std::string serverName;

	UploadResult()
	{
	}
};

#endif
