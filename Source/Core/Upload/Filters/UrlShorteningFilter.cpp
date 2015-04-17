#include "UrlShorteningFilter.h"

bool UrlShorteningFilter::PreUpload(UploadTask* task)
{
	return true;
}

bool UrlShorteningFilter::PostUpload(UploadTask* task)
{
	return true;
}