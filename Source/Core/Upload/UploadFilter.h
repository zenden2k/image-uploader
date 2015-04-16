#ifndef IU_CORE_UPLOAD_UPLOADFILTER
#define IU_CORE_UPLOAD_UPLOADFILTER

#pragma once

class UploadTask;

class UploadFilter
{
public:
	virtual ~UploadFilter() {}
	virtual bool PreUpload(UploadTask* task) = 0;
	virtual bool PostUpload(UploadTask* task) = 0;	
};

#endif