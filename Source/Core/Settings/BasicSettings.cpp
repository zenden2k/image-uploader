#include "BasicSettings.h"
#include "EncodedPassword.h"


BasicSettings::BasicSettings()
{
    LastUpdateTime = 0;
    UploadBufferSize = 1024 * 1024;
    FileRetryLimit = 3;
    ActionRetryLimit = 2;
    ExecuteScript = false;
    loadFromRegistry_ = false;
    MaxThreads = 3;
}

BasicSettings::~BasicSettings() {

}

void BasicSettings::setEngineList(CUploadEngineList_Base* engineList) {
    engineList_ = engineList;
}

bool BasicSettings::LoadAccounts(SimpleXmlNode root)
{
    std::vector<SimpleXmlNode> servers;
    root.GetChilds("Server", servers);

    for (size_t i = 0; i < servers.size(); i++) {
        std::string server_name = servers[i].Attribute("Name");
        std::vector<std::string> attribs;
        servers[i].GetAttributes(attribs);
        ServerSettingsStruct tempSettings;

        for (size_t j = 0; j < attribs.size(); j++) {
            std::string attribName = attribs[j];

            if (attribName.empty())
                continue;
            if (attribName.substr(0, 1) == "_") {
                std::string value = servers[i].Attribute(attribName);
                attribName = attribName.substr(1, attribName.size() - 1);
                if (!value.empty())
                    tempSettings.params[attribName] = value;
            }
        }
        tempSettings.authData.DoAuth = servers[i].AttributeBool("Auth");
#if !defined  (IU_CLI) && !defined(IU_SHELLEXT)


        std::string encodedLogin = servers[i].Attribute("Login");
        CEncodedPassword login;
        login.fromEncodedData(encodedLogin.c_str());
        tempSettings.authData.Login = login;

        std::string encodedPass = servers[i].Attribute("Password");
        CEncodedPassword pass;
        pass.fromEncodedData(encodedPass.c_str());
        tempSettings.authData.Password = pass;
#else
        tempSettings.authData.Login = servers[i].Attribute("Login");
#endif

        tempSettings.defaultFolder.setId(servers[i].Attribute("DefaultFolderId"));
        tempSettings.defaultFolder.viewUrl = servers[i].Attribute("DefaultFolderUrl");
        tempSettings.defaultFolder.setTitle(servers[i].Attribute("DefaultFolderTitle"));

        ServersSettings[server_name][tempSettings.authData.Login] = tempSettings;
    }
    return true;
}

bool BasicSettings::SaveAccounts(SimpleXmlNode root)
{
    ServerSettingsMap::iterator it1;
    for (it1 = ServersSettings.begin(); it1 != ServersSettings.end(); ++it1) {
        std::map <std::string, ServerSettingsStruct>::iterator it;
        for (it = it1->second.begin(); it != it1->second.end(); ++it) {
            ServerSettingsStruct & sss = it->second;
            if (sss.isEmpty()) {
                continue;
            }
            SimpleXmlNode serverNode = root.CreateChild("Server");

            serverNode.SetAttribute("Name", it1->first);

            std::map <std::string, std::string>::iterator param;
            for (param = it->second.params.begin(); param != sss.params.end(); ++param) {
                if (param->first == "FolderID" || param->first == "FolderUrl" || param->first == "FolderTitle") {
                    continue;
                }
                serverNode.SetAttribute("_" + param->first, param->second);
            }
            serverNode.SetAttributeBool("Auth", sss.authData.DoAuth);

#if !defined  (IU_CLI) && !defined(IU_SHELLEXT)
            CEncodedPassword login(sss.authData.Login);
            serverNode.SetAttribute("Login", login.toEncodedData());

            CUploadEngineData* ued = engineList_->byName(it->first);
            if (!ued || ued->NeedPassword) {
                CEncodedPassword pass(it->second.authData.Password);
                serverNode.SetAttribute("Password", pass.toEncodedData());
            }
#else
            serverNode.SetAttribute("Login", sss.authData.Login);
#endif
            if (!it->second.defaultFolder.getId().empty()) {
                serverNode.SetAttributeString("DefaultFolderId", sss.defaultFolder.getId());
                serverNode.SetAttributeString("DefaultFolderUrl", sss.defaultFolder.viewUrl);
                serverNode.SetAttributeString("DefaultFolderTitle", sss.defaultFolder.getTitle());
            }

        }
    }
    return true;
}

void BasicSettings::BindToManager()
{
    SettingsNode& upload = mgr_["Uploading"];
    upload.n_bind(MaxThreads);
}

bool BasicSettings::PostLoadSettings(SimpleXml &xml)
{
    return true;
}

bool BasicSettings::PostSaveSettings(SimpleXml& xml)
{
    return true;
}

void BasicSettings::notifyChange() {
    for (size_t i = 0; i < changeCallbacks_.size(); i++) {
        changeCallbacks_[i](this);
    }
}

bool BasicSettings::LoadSettings(std::string szDir, std::string fileName, bool LoadFromRegistry) {
    loadFromRegistry_ = LoadFromRegistry;
    fileName_ = !szDir.empty() ? szDir + ((!fileName.empty()) ? fileName : "Settings.xml")
        : SettingsFolder + (!fileName.empty() ? fileName : "Settings.xml");
    //std::cout<< fileName_;
    //MessageBoxA(0,fileName_.c_str(),0,0);
    if (!IuCoreUtils::FileExists(fileName_)) {
        return true;
    }
    SimpleXml xml;
    xml.LoadFromFile(fileName_);
    mgr_.loadFromXmlNode(xml.getRoot("ImageUploader").GetChild("Settings"));
    LoadAccounts(xml.getRoot("ImageUploader").GetChild("Settings").GetChild("ServersParams"));
    PostLoadSettings(xml);
    notifyChange();
    return true;
}


bool BasicSettings::SaveSettings()
{
    SimpleXml xml;
    mgr_.saveToXmlNode(xml.getRoot("ImageUploader").GetChild("Settings"));
    PostSaveSettings(xml);
    SaveAccounts(xml.getRoot("ImageUploader").GetChild("Settings").GetChild("ServersParams"));
    //std::cerr << "Saving setting to "<< IuCoreUtils::WstringToUtf8((LPCTSTR)fileName_);
    xml.SaveToFile(fileName_);
    notifyChange();
    return true;
}

void BasicSettings::addChangeCallback(const ChangeCallback& callback)
{
    changeCallbacks_.push_back(callback);
}

ServerSettingsStruct* BasicSettings::getServerSettings(const ServerProfile& profile)
{
    std::lock_guard<std::mutex> lock(serverSettingsMutex_);
    return &ServersSettings[profile.serverName()][profile.profileName()];
}