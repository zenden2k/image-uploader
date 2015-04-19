#include "ImageConverterFilter.h"
#include "Func/IuCommonFunctions.h"
#include "Func/Settings.h"
#include "Core/Logging.h"
#include "Core/Images/ImageConverter.h"
#include "Core/Upload/FileUploadTask.h"
#include "Func/myutils.h"

bool ImageConverterFilter::PreUpload(UploadTask* task)
{
	FileUploadTask* fileTask = dynamic_cast<FileUploadTask*>(task);
	if (!fileTask) {
		return true;
	}
	if (task->parentTask())
	{
		return true;
	}
	if (!IsImage(IuCoreUtils::Utf8ToWstring(fileTask->getFileName()).c_str()))
	{
		return true;
	}
	CImageConverter imageConverter;
	Thumbnail thumb;
	ImageUploadParams imageUploadParams = task->serverProfile().getImageUploadParams();
	CString templateName = imageUploadParams.getThumb().TemplateName;
	if (templateName.IsEmpty()) {
		templateName = _T("default");
	}
	CString thumbTemplateFileName = IuCommonFunctions::GetDataFolder() + _T("\\Thumbnails\\") + templateName +
		_T(".xml");

	if (!thumb.LoadFromFile(IuCoreUtils::WstringToUtf8((LPCTSTR)thumbTemplateFileName)))
	{
		LOG(ERROR) << TR("Не могу загрузить файл миниатюры!") + CString(_T("\r\n")) + thumbTemplateFileName;
		return false;
	}
	imageConverter.setEnableProcessing(imageUploadParams.ProcessImages);
	imageConverter.setImageConvertingParams(Settings.ConvertProfiles[imageUploadParams.ImageProfileName]);
	imageConverter.setThumbCreatingParams(imageUploadParams.getThumb());
	bool GenThumbs = imageUploadParams.CreateThumbs &&
		((!imageUploadParams.UseServerThumbs) || (!task->serverProfile().uploadEngineData()->SupportThumbnails));
	imageConverter.setThumbnail(&thumb);
	imageConverter.setGenerateThumb(GenThumbs);
	if (imageConverter.Convert(IuCoreUtils::Utf8ToWstring(fileTask->getFileName()).c_str())) {
		
		std::string convertedImageFileName = IuCoreUtils::WstringToUtf8(static_cast<LPCTSTR>(imageConverter.getImageFileName()));
		if (!convertedImageFileName.empty()) {
			
			if (convertedImageFileName != fileTask->getFileName())
			{
				TempFileDeleter* deleter = fileTask->tempFileDeleter();
				deleter->addFile(convertedImageFileName);
			}
			std::string virtualName = IuCoreUtils::ExtractFileNameNoExt(fileTask->getFileName()) + "." + IuCoreUtils::ExtractFileExt(convertedImageFileName);
			fileTask->setDisplayName(virtualName);
			fileTask->setFileName(convertedImageFileName);
		} 
		if (GenThumbs) {
			std::string thumbFileName = IuCoreUtils::WstringToUtf8(static_cast<LPCTSTR>(imageConverter.getThumbFileName()));
			if (!thumbFileName.empty())
			{
				std::shared_ptr<FileUploadTask> thumbTask(new FileUploadTask(thumbFileName, "", task));

				TempFileDeleter* deleter = fileTask->tempFileDeleter();
				deleter->addFile(thumbFileName);
				
				//thumbTask->OnFileFinished.bind(this, &ImageConverterFilter::OnFileFinished);
				ServerProfile thumbServerProfile = fileTask->serverProfile().deepCopy();
				thumbServerProfile.getImageUploadParamsRef().CreateThumbs = false;
				thumbServerProfile.getImageUploadParamsRef().ProcessImages = false;
				thumbTask->setServerProfile(thumbServerProfile);
				thumbTask->setRole(UploadTask::ThumbRole);
				fileTask->addChildTask(thumbTask);
			}
			
		}
	}
	return true;
}

bool ImageConverterFilter::PostUpload(UploadTask* task)
{
	return true;
}
/*
void ImageConverterFilter::OnFileFinished(UploadTask* task, bool ok)
{
	FileUploadTask* thumbTask = dynamic_cast<FileUploadTask*>(task);
	//TODO: keep server thumbnail url
	thumbTask->parentTask()->uploadResult()->thumbUrl = task->uploadResult()->directUrl;
}*/