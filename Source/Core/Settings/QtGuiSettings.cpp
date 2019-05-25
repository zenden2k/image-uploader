#include "QtGuiSettings.h"

QtGuiSettings::QtGuiSettings() : BasicSettings() {
    BindToManager();
}

#ifdef _WIN32

void ImageUploadParams::bind(SettingsNode& n) {
}

ThumbCreatingParams ImageUploadParams::getThumb()
{
    return Thumb;
}

ThumbCreatingParams& ImageUploadParams::getThumbRef()
{
    return Thumb;
}

#endif
