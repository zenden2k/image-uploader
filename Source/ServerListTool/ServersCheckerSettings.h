#ifndef SERVERSCHECKER_SERVERSCHECKERSETTINGS_H
#define SERVERSCHECKER_SERVERSCHECKERSETTINGS_H

#include <string>
#include "atlheaders.h"
#include "Core/Settings/BasicSettings.h"

class BasicSettings;

namespace ServersListTool {

class ServersCheckerSettings: public BasicSettings {
public:
    std::string testFileName, testUrl;

    ServersCheckerSettings();
    
    bool LoadSettings(const std::string& szDir, const std::string& fileName);

protected:
    void BindToManager();
    bool PostLoadSettings(SimpleXml &root) override;
};

}
#endif