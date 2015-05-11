#ifndef IU_CORE_SETTINGS_SETTINGS_H
#define IU_CORE_SETTINGS_SETTINGS_H

#pragma once
#ifdef IU_WTL
    #include "Core/Settings/WtlGuiSettings.h"
    typedef WtlGuiSettings CSettings;
#elif defined(IU_CLI)
    #include "Core/Settings/CliSettings.h"
    typedef CliSettings CSettings;
#elif defined(IU_QT)
    #include "Core/Settings/QtGuiSettings.h"
    typedef QtGuiSettings CSettings;
#endif
    extern CSettings Settings;
#endif