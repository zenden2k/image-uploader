#include "QtGuiSettings.h"

QtGuiSettings::QtGuiSettings() : CommonGuiSettings() {
    BindToManager();
}

#ifdef _WIN32

void ImageUploadParams::bind(SettingsNode& n) {
}
#endif

ThumbCreatingParams ImageUploadParams::getThumb()
{
    return Thumb;
}

ThumbCreatingParams& ImageUploadParams::getThumbRef()
{
    return Thumb;
}


