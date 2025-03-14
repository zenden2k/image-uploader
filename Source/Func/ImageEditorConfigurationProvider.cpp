#include "ImageEditorConfigurationProvider.h"

#include "Core/Settings/WtlGuiSettings.h"

ImageEditorConfigurationProvider::ImageEditorConfigurationProvider()
{
    WtlGuiSettings* settings = ServiceLocator::instance()->settings<WtlGuiSettings>();
    penSize_ = settings->ImageEditorSettings.PenSize;
    foregroundColor_ = settings->ImageEditorSettings.ForegroundColor;
    backgroundColor_ = settings->ImageEditorSettings.BackgroundColor;
    stepForegroundColor_ = settings->ImageEditorSettings.StepForegroundColor;
    stepBackgroundColor_ = settings->ImageEditorSettings.StepBackgroundColor;
    font_ =  settings->ImageEditorSettings.Font;
    roundingRadius_ = settings->ImageEditorSettings.RoundingRadius;
    blurRadius_ = settings->ImageEditorSettings.BlurRadius;
    allowAltTab_ = settings->ImageEditorSettings.AllowAltTab;
    searchEngine_ = settings->imageSearchServer;
    fillTextBackground_ = settings->ImageEditorSettings.FillTextBackground;
    arrowMode_ = settings->ImageEditorSettings.ArrowType;
    invertSelection_ = settings->ImageEditorSettings.InvertSelection;
}

void ImageEditorConfigurationProvider::saveConfiguration()
{
    WtlGuiSettings* settings = ServiceLocator::instance()->settings<WtlGuiSettings>();
    settings->ImageEditorSettings.PenSize = penSize_;
    settings->ImageEditorSettings.ForegroundColor = foregroundColor_;
    settings->ImageEditorSettings.BackgroundColor = backgroundColor_;
    settings->ImageEditorSettings.Font = font_;
    settings->ImageEditorSettings.RoundingRadius = roundingRadius_;
    settings->ImageEditorSettings.BlurRadius = blurRadius_;
    settings->ImageEditorSettings.AllowAltTab = allowAltTab_;
    settings->imageSearchServer = searchEngine_;
    settings->ImageEditorSettings.FillTextBackground = fillTextBackground_;
    settings->ImageEditorSettings.StepForegroundColor = stepForegroundColor_;
    settings->ImageEditorSettings.StepBackgroundColor = stepBackgroundColor_;
    settings->ImageEditorSettings.ArrowType = arrowMode_;
    settings->ImageEditorSettings.InvertSelection = invertSelection_;
}

