#include "ImageEditorConfigurationProvider.h"

#include "Core/Settings.h"

ImageEditorConfigurationProvider::ImageEditorConfigurationProvider()
{
    penSize_ = Settings.ImageEditorSettings.PenSize;
    foregroundColor_ = Settings.ImageEditorSettings.ForegroundColor;
    backgroundColor_ = Settings.ImageEditorSettings.BackgroundColor;
    font_ =  Settings.ImageEditorSettings.Font;
    roundingRadius_ = Settings.ImageEditorSettings.RoundingRadius;
    allowAltTab_ = Settings.ImageEditorSettings.AllowAltTab;
}

void ImageEditorConfigurationProvider::saveConfiguration()
{
    Settings.ImageEditorSettings.PenSize = penSize_;
    Settings.ImageEditorSettings.ForegroundColor = foregroundColor_;
    Settings.ImageEditorSettings.BackgroundColor = backgroundColor_;
    Settings.ImageEditorSettings.Font = font_;
    Settings.ImageEditorSettings.RoundingRadius = roundingRadius_;
    Settings.ImageEditorSettings.AllowAltTab = allowAltTab_;
}

