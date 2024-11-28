#ifndef IU_FUNC_WTLSCRIPTDIALOGPROVIDER_H
#define IU_FUNC_WTLSCRIPTDIALOGPROVIDER_H

#include <mutex>

#include <QObject>

#include "Core/Scripting/DialogProvider.h"


class QtScriptDialogProvider : public QObject, public IDialogProvider {
    Q_OBJECT
public:
	QtScriptDialogProvider();
    std::string askUserCaptcha(INetworkClient *nm, const std::string& url) override;
    std::string inputDialog(const std::string& text, const std::string& defaultValue) override;
    std::string messageBox(const std::string& message, const std::string& title, const std::string& buttons, const std::string& type) override;
protected:
	std::mutex mutex_;
	std::string value_;
	int dialogResult_;
	std::string showDialog(const std::string& text, const std::string& defaultValue);
    Q_INVOKABLE void showDialogInMainThread(QString text, QString defaultValue);
    std::string messageBoxInMainThread(QString message, QString title, QString buttons, QString type);

};
#endif
