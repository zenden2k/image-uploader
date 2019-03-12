#ifndef IU_FUNC_WTLSCRIPTDIALOGPROVIDER_H
#define IU_FUNC_WTLSCRIPTDIALOGPROVIDER_H

#include "Core/Scripting/DialogProvider.h"
#include <mutex>

class QtScriptDialogProvider : public IDialogProvider {
public:
	QtScriptDialogProvider();
    std::string askUserCaptcha(NetworkClient *nm, const std::string& url) override;
    std::string inputDialog(const std::string& text, const std::string& defaultValue) override;
protected:
	std::mutex mutex_;
	std::string value_;
	int dialogResult_;
	std::string showDialog(const std::string& text, const std::string& defaultValue);
};
#endif