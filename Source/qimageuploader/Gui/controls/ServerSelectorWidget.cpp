#include "ServerSelectorWidget.h"
#include <QGridLayout>
#include <QDir>
#include <QMenu>

#include "Core/Upload/UploadEngineManager.h"
#include "Core/Upload/ServerProfile.h"
#include "Core/ServiceLocator.h"
#include "Core/CommonDefs.h"
#include "Core/AppParams.h"
#include "Core/Settings.h"
#include "Gui/LoginDialog.h"

ServerSelectorWidget::ServerSelectorWidget(UploadEngineManager* uploadEngineManager, bool defaultServer, QWidget* parent) :QGroupBox(parent) {
	this->uploadEngineManager = uploadEngineManager;
	showDefaultServerItem = false;
	showFileSizeLimits = false;
	serversMask = smImageServers | smFileServers;

	setStyleSheet("QGroupBox {font-weight: bold;}");
	QGridLayout * grid = new QGridLayout(this);
	serverListComboBox = new QComboBox(this);
	/*QFont f = font();
	f.setBold(true);
	setFont(f);*/
	/*accountLabel = new QLabel(this);
	accountLabel->setStyleSheet("QLabel { color : blue; }");
	//accountLabel->setText("<a href=\"#\">account</a>");
	accountLabel->setText("Account");
	accountLabel->setCursor(Qt::PointingHandCursor);*/

	accountButton = new QToolButton( this);
	accountButton->setIcon(QIcon(":/res/icon-user.ico"));
	accountButton->setText(tr("<without account>"));
	accountButton->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
	accountButton->setPopupMode(QToolButton::MenuButtonPopup);
	//accountButton->setFlat(true);
	accountButton->setCursor(Qt::PointingHandCursor);
	accountButton->setStyleSheet("QPushButton { color : blue; }");
	/*accountIcon = new QLabel(this);
	accountIcon->setPixmap(QIcon(":/res/icon-user.ico").pixmap(16,16));*/
	grid->addWidget(serverListComboBox, 0,0);
	accountLayout = new QHBoxLayout(this);
	//accountLayout->addWidget(accountIcon);
	accountLayout->addWidget(accountButton);
	QSpacerItem* horizontalSpacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

	accountLayout->addItem(horizontalSpacer);



	grid->addLayout(accountLayout, 0, 1);
	connect(serverListComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(comboBoxIndexChanged(int)));
	connect(accountButton, &QPushButton::clicked, this, &ServerSelectorWidget::accountButtonClicked);
	updateServerList();
	updateAccountButtonMenu();
}

/*void ServerSelectorWidget::setTitle(QString title)
{
	titleLabel->setText(title);
}*/

void ServerSelectorWidget::setServerProfile(ServerProfile serverProfile)
{
	serverProfile_ = serverProfile;
	int itemIndex = serverListComboBox->findData(U2Q(serverProfile.serverName()));
	if (itemIndex != -1) {
		serverListComboBox->setCurrentIndex(itemIndex);
	}
}

void ServerSelectorWidget::setShowDefaultServerItem(bool show)
{
	showDefaultServerItem = show;
}

void ServerSelectorWidget::setServersMask(int mask)
{
	serversMask = mask;
}

void ServerSelectorWidget::setShowFilesizeLimits(bool show)
{
	showFileSizeLimits = show;
}

void ServerSelectorWidget::updateServerList()
{
	serverListComboBox->clear();
	auto* myEngineList = ServiceLocator::instance()->engineList();
	
	int selectedIndex = 0;
	int addedItems = 0;
	std::string selectedServerName = serverProfile_.serverName();
	TCHAR line[40];
	for (int i = 0; i < ARRAY_SIZE(line) - 1; i++) {
		line[i] = '-';
	}
	line[ARRAY_SIZE(line) - 1] = 0;

	AppParams* params = AppParams::instance();
	QString dataDir = U2Q(params->dataDirectory());


	for (int mask = 1; mask <= 4; mask *= 2) {
		int currentLoopMask = mask & serversMask;
		if (!currentLoopMask) {
			continue;
		}
		if (addedItems) {
			serverListComboBox->addItem(QString::fromWCharArray(line));
		}
		for (int i = 0; i < myEngineList->count(); i++) {
			CUploadEngineData * ue = myEngineList->byIndex(i);

			if (serversMask != smUrlShorteners && !ue->hasType(CUploadEngineData::TypeFileServer) && !ue->hasType(CUploadEngineData::TypeImageServer)) {
				continue;
			}
			if (!ue->hasType(CUploadEngineData::TypeImageServer) && ((currentLoopMask & smImageServers) == smImageServers)) {
				continue;
			}
			if (!ue->hasType(CUploadEngineData::TypeFileServer) && ((currentLoopMask & smFileServers) == smFileServers)) {
				continue;
			}

			if (!ue->hasType(CUploadEngineData::TypeUrlShorteningServer) && (currentLoopMask & smUrlShorteners)) {
				continue;
			}

			QString iconPath = QDir(dataDir).filePath("Favicons/" + U2Q(ue->Name) + ".ico");
			QIcon ico;
			if (QFile::exists(iconPath)) {
				ico = QIcon(iconPath);

			}
			/*HICON hImageIcon = myEngineList->getIconForServer(ue->Name);
			int nImageIndex = -1;
			if (hImageIcon) {
				nImageIndex = comboBoxImageList_.AddIcon(hImageIcon);
			}*/

			std::string displayName = ue->Name;
			if (showFileSizeLimits && ue->MaxFileSize > 0) {
				displayName += " (" + IuCoreUtils::fileSizeToString(ue->MaxFileSize) + ")";
			}
			serverListComboBox->addItem(ico, U2Q(displayName), U2Q(ue->Name));
			/*if (ue->Name == selectedServerName) {
				selectedIndex = itemIndex;
			}*/
			addedItems++;
		}
	}
	if (serversMask != smUrlShorteners) {
		serverListComboBox->addItem(QString::fromWCharArray(line));
		//serverListComboBox->addItem(tr("Add FTP server..."), reinterpret_cast<LPARAM>(kAddFtpServer));
		//serverListComboBox->addItem(tr("Add local folder..."), reinterpret_cast<LPARAM>(kAddDirectoryAsServer));
	}

	//serverComboBox_.SetImageList(comboBoxImageList_);
	//serverComboBox_.SetCurSel(selectedIndex);

}

void ServerSelectorWidget::comboBoxIndexChanged(int index) {
	if ( serverListComboBox->itemData(index).isNull()) {
		serverListComboBox->setCurrentIndex(0);
	}
	else {
		serverChanged();
	}
}

ServerProfile ServerSelectorWidget::serverProfile() const
{
	return serverProfile_;
}

void ServerSelectorWidget::serverChanged() {
	QVariant data = serverListComboBox->currentData();
	QString serverName = data.toString();
	serverProfile_.setServerName(Q2U(serverName));
	auto ued = serverProfile_.uploadEngineData();
	if (ued) {
		accountButton->setVisible(ued->NeedAuthorization != CUploadEngineData::naNotAvailable);
	}
	updateAccountButtonMenu();
}

void ServerSelectorWidget::accountButtonClicked(bool /*checked*/) {
	/*QRect widgetRect = accountButton->geometry();
	QMenu* contextMenu = new QMenu(accountButton);
	QAction* viewCodeAction = new QAction(tr("<without account>"), contextMenu);

	contextMenu->addAction(viewCodeAction);
	contextMenu->setDefaultAction(viewCodeAction);
	contextMenu->exec(accountButton->parentWidget()->mapToGlobal(widgetRect.bottomLeft()));*/
	if (serverProfile_.profileName() == "") {
		addAccountClicked();
	} else {
		LoginDialog dlg(serverProfile_, false, this);
		if (dlg.exec() == QDialog::Accepted) {
			accountButton->setText(dlg.accountName());
			updateAccountButtonMenu();
		}
	}
}

void ServerSelectorWidget::updateAccountButtonMenu() {
	if (!accountButtonMenu_) {
		accountButtonMenu_.reset(new QMenu(accountButton));
	}
	accountButtonMenu_->clear();
	auto& serverUsers = Settings.ServersSettings[serverProfile_.serverName()];
	for (const auto& user: serverUsers) {
		std::string accountName = user.first;
		if (accountName.empty()) {
			continue;
		}
		QAction* userAction = new QAction(U2Q(accountName), accountButtonMenu_.get());
		//userAction->setData(U2Q(user.first));

		connect(userAction, &QAction::triggered, [accountName, this]{
			serverProfile_.setProfileName(accountName);
			accountButton->setText(U2Q(accountName));
		});
		accountButtonMenu_->addAction(userAction);
	}

	QAction* viewCodeAction = new QAction(tr("<without account>"), accountButtonMenu_.get());
	connect(viewCodeAction, &QAction::triggered, this, &ServerSelectorWidget::noAccountSelected);
	accountButtonMenu_->addAction(viewCodeAction);

	accountButtonMenu_->addSeparator();
	QAction* addAccountAction = new QAction(tr("Add account..."), accountButtonMenu_.get());
	connect(addAccountAction, &QAction::triggered, this, &ServerSelectorWidget::addAccountClicked);
	accountButtonMenu_->addAction(addAccountAction);
	accountButton->setMenu(accountButtonMenu_.get());
}

void ServerSelectorWidget::noAccountSelected() {
	serverProfile_.setProfileName(std::string());
	accountButton->setText(tr("<without account>"));
}

void ServerSelectorWidget::addAccountClicked() {
	ServerProfile serverProfileCopy = serverProfile_;
	serverProfileCopy.setProfileName(std::string());

	LoginDialog dlg(serverProfileCopy, true,this);
	if(dlg.exec() == QDialog::Accepted) {
		serverProfileCopy.setProfileName(Q2U(dlg.accountName()));
		serverProfileCopy.setFolderId(std::string());
		serverProfileCopy.setFolderTitle(std::string());
		serverProfileCopy.setFolderUrl(std::string());

		serverProfile_ = serverProfileCopy;

		accountButton->setText(dlg.accountName());
		updateAccountButtonMenu();
	}

}
