#include "ServerSelectorWidget.h"
#include <QGridLayout>
#include <QDir>
#include "Core/Upload/UploadEngineManager.h"
#include "Core/Upload/ServerProfile.h"
#include "Core/ServiceLocator.h"
#include "Core/CommonDefs.h"
#include "Core/AppParams.h"

ServerSelectorWidget::ServerSelectorWidget(UploadEngineManager* uploadEngineManager, bool defaultServer, QWidget* parent) :QGroupBox(parent) {
	this->uploadEngineManager = uploadEngineManager;
	showDefaultServerItem = false;
	showFileSizeLimits = false;
	serversMask = smImageServers | smFileServers;


	QGridLayout * grid = new QGridLayout(this);
	serverListComboBox = new QComboBox(this);
	/*QFont f = font();
	f.setBold(true);
	setFont(f);*/
	accountLabel = new QLabel(this);
	accountLabel->setText(tr("Account:"));
	grid->addWidget(accountLabel, 0, 1);
	grid->addWidget(serverListComboBox, 0,0);
	connect(serverListComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(comboBoxIndexChanged(int)));
	updateServerList();
}

/*void ServerSelectorWidget::setTitle(QString title)
{
	titleLabel->setText(title);
}*/

void ServerSelectorWidget::setServerProfile(ServerProfile serverProfile)
{
	serverProfile_ = serverProfile;
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
	serverChanged();
}

ServerProfile ServerSelectorWidget::serverProfile() const
{
	return serverProfile_;
}

void ServerSelectorWidget::serverChanged() {
	QVariant data = serverListComboBox->currentData();
	QString serverName = data.toString();
	serverProfile_.setServerName(Q2U(serverName));
}
