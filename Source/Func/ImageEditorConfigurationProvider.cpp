#include "ImageEditorConfigurationProvider.h"
#include "Settings.h"

ImageEditorConfigurationProvider::ImageEditorConfigurationProvider()
{
	penSize_ = Settings.ImageEditorSettings.PenSize;
	foregroundColor_ = Settings.ImageEditorSettings.ForegroundColor;
	backgroundColor_ = Settings.ImageEditorSettings.BackgroundColor;
	font_ =  Settings.ImageEditorSettings.Font;
}


void ImageEditorConfigurationProvider::saveConfiguration()
{
	Settings.ImageEditorSettings.PenSize = penSize_;
	Settings.ImageEditorSettings.ForegroundColor = foregroundColor_;
	Settings.ImageEditorSettings.BackgroundColor = backgroundColor_;
	Settings.ImageEditorSettings.Font = font_;
}

