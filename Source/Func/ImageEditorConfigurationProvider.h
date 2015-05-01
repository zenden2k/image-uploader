#ifndef IU_FUNCT_IMAGEEDITORCONFIGURATIONPROVIDER_H
#define IU_FUNCT_IMAGEEDITORCONFIGURATIONPROVIDER_H

#include <ImageEditor/Gui/ImageEditorWindow.h>

class ImageEditorConfigurationProvider : public ImageEditor::ConfigurationProvider {
public:
    ImageEditorConfigurationProvider();
    virtual void saveConfiguration();
};
#endif