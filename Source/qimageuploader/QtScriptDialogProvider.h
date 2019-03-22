#ifndef IU_FUNC_WTLSCRIPTDIALOGPROVIDER_H
#define IU_FUNC_WTLSCRIPTDIALOGPROVIDER_H

#include <QObject>
#include "Core/Scripting/DialogProvider.h"
#include <mutex>


class QtScriptDialogProvider : public QObject, public IDialogProvider {
    Q_OBJECT
public:
	QtScriptDialogProvider();
    std::string askUserCaptcha(NetworkClient *nm, const std::string& url) override;
    std::string inputDialog(const std::string& text, const std::string& defaultValue) override;
protected:
	std::mutex mutex_;
	std::string value_;
	int dialogResult_;
	std::string showDialog(const std::string& text, const std::string& defaultValue);
    Q_INVOKABLE void showDialogInMainThread(QString text, QString defaultValue);
};
#endif