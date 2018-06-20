#include "QtGuiSettings.h"

QtGuiSettings Settings;

QtGuiSettings::QtGuiSettings() : CommonGuiSettings() {
    BindToManager();
}
