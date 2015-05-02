/**
@file
@section UploadFilters Upload filter script
Functions of this script are being called for each upload task and for each child task (e.g. file upload, thumbnail upload, url shortening)

@since version 1.3.2

Example:
@include uploadfilter.nut
*/

/**
This function is called before upload
@return true - success, false - error (will abort upload)
*/
bool PreUpload(ScriptAPI::UploadTaskWrapperBase task, int reserved);

/**
This function is called after upload, but child tasks (url shortening, thumbnail upload) of this task can be still running
*/
bool PostUpload(ScriptAPI::UploadTaskWrapperBase task, int reserved);
