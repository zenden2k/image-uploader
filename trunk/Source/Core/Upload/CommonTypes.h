#ifndef IU_COMMONTYPES_H
#define IU_COMMONTYPES_H

#include "../Utils/CoreUtils.h"
struct InfoProgress
{
	unsigned long Uploaded,Total;
	bool IsUploading;
	void clear()
	{
		Uploaded = 0;
		Total = 0;
		IsUploading = false;
	}
};

enum StatusType 
{
	stNone = 0, stUploading, stWaitingAnswer,  stAuthorization, stPerformingAction, stCreatingFolder, stUserDescription 
};

enum MessageType
{
	mtError, mtWarning
};
enum ErrorType
{
	etNone, etOther, etRepeating, etRetriesLimitReached, etActionRepeating, etActionRetriesLimitReached, etRegularExpressionError, etNetworkError, etUserError
};

struct ErrorInfo
{
	Utf8String error;
	Utf8String Url;
	Utf8String ServerName;
	Utf8String FileName;
	int ActionIndex; 
	MessageType messageType;
	ErrorType errorType;
	int RetryIndex;
	Utf8String sender;
	ErrorInfo()
	{
		RetryIndex = -1;
		ActionIndex = -1;
	}

	void Clear()
	{	
		error.clear();
		Url.clear();
		ServerName.clear();
		FileName.clear();
		sender.clear();
		ActionIndex = -1;
		//messageType = mtNone;
		errorType = etNone;
		RetryIndex = -1;
	}
};
#endif
