#include "ImageEditorConfigurationProvider.h"
#include "Settings.h"

ImageEditorConfigurationProvider::ImageEditorConfigurationProvider()
{
	penSize_ = Settings.ImageEditorSettings.PenSize;
	foregroundColor_ = Settings.ImageEditorSettings.ForegroundColor;
	backgroundColor_ = Settings.ImageEditorSettings.BackgroundColor;
}


void ImageEditorConfigurationProvider::saveConfiguration()
{
	Settings.ImageEditorSettings.PenSize = penSize_;
	Settings.ImageEditorSettings.ForegroundColor = foregroundColor_;
	Settings.ImageEditorSettings.BackgroundColor = backgroundColor_;
}

