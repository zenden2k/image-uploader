#include "ImageConverterFilter.h"
#include <Func/IuCommonFunctions.h>
#include <Func/Settings.h>
#include <Core/Logging.h>
#include <Core/Images/ImageConverter.h>
#include <Core/Upload/FileUploadTask.h>
#include <Func/myutils.h>

bool ImageConverterFilter::PreUpload(UploadTask* task)
{
	FileUploadTask* fileTask = dynamic_cast<FileUploadTask*>(task);
	if (!fileTask) {
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
		fileTask->setFileName(convertedImageFileName);
		std::string virtualName = IuCoreUtils::ExtractFileNameNoExt(fileTask->getFileName()) + "." + IuCoreUtils::ExtractFileExt(convertedImageFileName);
		fileTask->setDisplayName(virtualName);
		std::shared_ptr<FileUploadTask> thumbTask(new FileUploadTask(IuCoreUtils::WstringToUtf8(static_cast<LPCTSTR>(imageConverter.getThumbFileName())), "", task));
		thumbTask->OnFileFinished.bind(this, &ImageConverterFilter::OnFileFinished);
		fileTask->addChildTask(thumbTask);
	}
	return true;
}

bool ImageConverterFilter::PostUpload(UploadTask* task)
{
	return true;
}

void ImageConverterFilter::OnFileFinished(std::shared_ptr<UploadTask> task, bool ok)
{
	std::shared_ptr<FileUploadTask> thumbTask = std::static_pointer_cast<FileUploadTask>(task);
	//TODO: keep server thumbnail url
	thumbTask->parentTask()->uploadResult()->thumbUrl = task->uploadResult()->directUrl;
}