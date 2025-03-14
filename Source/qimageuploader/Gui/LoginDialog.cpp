#include "LoginDialog.h"

#include "ui_LoginDialog.h"
#include <QMessageBox>
#include "Core/Upload/UploadEngine.h"
#include "Core/CommonDefs.h"
#include "Core/Settings/BasicSettings.h"
#include "Core/ServiceLocator.h"

LoginDialog::LoginDialog(ServerProfile& serverProfile, bool createNew, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::LoginDialog),
	serverProfile_(serverProfile),
	createNew_(createNew)
{
    ui->setupUi(this);

    BasicSettings* Settings = ServiceLocator::instance()->basicSettings();
    if (!Settings) {
        return;
    }
    ServerSettingsStruct* serverSettings = Settings->getServerSettings(serverProfile_);

	LoginInfo li = serverSettings ? serverSettings->authData: LoginInfo();
	auto ued = serverProfile_.uploadEngineData();
    if (ued) {
        QString loginLabelText = ued->LoginLabel.empty() ? tr("Login:") : U2Q(ued->LoginLabel) + ":";
        ui->loginLabel->setText(loginLabelText);
        QString passwordLabelText = ued->PasswordLabel.empty() ? tr("Password:") : U2Q(ued->PasswordLabel) +":";
        ui->passwordLabel->setText(passwordLabelText);
        ui->passwordLabel->setEnabled(ued->NeedPassword);
        ui->passwordEdit->setEnabled(ued->NeedPassword);
    }
	
	accountName_ = U2Q(li.Login);

	ui->loginEdit->setText(accountName_);
	ui->passwordEdit->setText(U2Q(li.Password));


	connect(ui->buttonBox, &QDialogButtonBox::accepted, this, &LoginDialog::onAccept);
}


LoginDialog::~LoginDialog()
{
}

QString LoginDialog::accountName() const
{
	return accountName_;
}

void LoginDialog::onAccept() {
	LoginInfo li;
	QString buffer = ui->loginEdit->text();

	li.Login = Q2U(buffer);

	if (li.Login.empty()) {
		QMessageBox::critical(this, tr("Error"), tr("Login cannot be empty"));
		return;
	}
	std::string serverNameA = serverProfile_.serverName();
    BasicSettings& Settings = *ServiceLocator::instance()->basicSettings();
	// /* !ignoreExistingAccount_ &&  */
	if (createNew_ && Settings.ServersSettings[serverNameA].find(li.Login) != Settings.ServersSettings[serverNameA].end()) {
		QMessageBox::critical(this, tr("Error"), tr("Account with such name already exists."));
		return;
	}

	if (li.Login != Q2U(accountName_)) {
		serverProfile_.clearFolderInfo();
	}

	accountName_ = buffer;
    serverProfile_.setProfileName(Q2U(buffer));
    li.Login = Q2U(buffer);
	li.Password = Q2U(ui->passwordEdit->text());
	li.DoAuth = true;
	//uploadEngineManager_->resetAuthorization(serverProfile_);

    ServerSettingsStruct* serverSettings = Settings.getServerSettings(serverProfile_, true);
    if(serverSettings) {
        serverSettings->authData = li;
    }
	accept();
}
