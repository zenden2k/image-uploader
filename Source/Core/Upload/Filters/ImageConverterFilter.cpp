#include "ImageConverterFilter.h"

#include "Func/IuCommonFunctions.h"
#include "Core/Logging.h"
#include "Core/Images/ImageConverter.h"
#include "Core/Upload/FileUploadTask.h"
#include "Core/CommonDefs.h"
#include "Core/i18n/Translator.h"
#include "Core/Settings/WtlGuiSettings.h"

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
    if (!IuCommonFunctions::IsImage(IuCoreUtils::Utf8ToWstring(fileTask->getFileName()).c_str()))
    {
        return true;
    }
    ImageConverter imageConverter;
    Thumbnail thumb;
    ImageUploadParams imageUploadParams = task->serverProfile().getImageUploadParams();
    CString templateName = U2W(imageUploadParams.getThumb().TemplateName);
    if (templateName.IsEmpty()) {
        templateName = _T("default");
    }
    CString thumbTemplateFileName = IuCommonFunctions::GetDataFolder() + _T("\\Thumbnails\\") + templateName +
        _T(".xml");

    if (!thumb.LoadFromFile(IuCoreUtils::WstringToUtf8((LPCTSTR)thumbTemplateFileName)))
    {
        LOG(ERROR) << TR("Couldn't load thumbnail preset!") + CString(_T("\r\n")) + thumbTemplateFileName;
        return false;
    }
    task->setStatusText(_("Preparing image..."));
    imageConverter.setEnableProcessing(imageUploadParams.ProcessImages);
    WtlGuiSettings& Settings = *ServiceLocator::instance()->settings<WtlGuiSettings>();
    imageConverter.setImageConvertingParams(Settings.ConvertProfiles[U2W(imageUploadParams.ImageProfileName)]);
    imageConverter.setThumbCreatingParams(imageUploadParams.getThumb());
    bool GenThumbs = imageUploadParams.CreateThumbs &&
        ((!imageUploadParams.UseServerThumbs) || (!task->serverProfile().uploadEngineData()->SupportThumbnails));
    imageConverter.setThumbnail(&thumb);
    imageConverter.setGenerateThumb(GenThumbs);
    if (imageConverter.Convert(fileTask->getFileName())) {
        
        std::string convertedImageFileName = imageConverter.getImageFileName();
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
            std::string thumbFileName = imageConverter.getThumbFileName();
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
                //thumbTask->setStatus(UploadTask::StatusPostponed);
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