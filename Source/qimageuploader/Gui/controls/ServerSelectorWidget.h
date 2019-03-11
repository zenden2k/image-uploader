#ifndef QIMAGEUPLOADER_GUI_CONTROLS_SERVERSELECTORWIDGET_H
#define QIMAGEUPLOADER_GUI_CONTROLS_SERVERSELECTORWIDGET_H

#include <QListWidget>
#include <QKeyEvent>
#include <QComboBox>
#include <QLabel>
#include <QGroupBox>
#include "Core/Upload/ServerProfile.h"

class UploadEngineManager;

class ServerSelectorWidget : public QGroupBox
{
    Q_OBJECT
    private:

    public:
    ServerSelectorWidget(UploadEngineManager* uploadEngineManager, bool defaultServer = false, QWidget* parent = 0);
	void setServerProfile(ServerProfile serverProfile);
	void setShowDefaultServerItem(bool show);
	void setServersMask(int mask);
	void setShowFilesizeLimits(bool show);
	void updateServerList();
	ServerProfile serverProfile() const;
	enum ServerMaskEnum { smAll = 0xffff, smImageServers = 0x1, smFileServers = 0x2, smUrlShorteners = 0x4 };

public slots:
	void comboBoxIndexChanged(int index);
protected:
	QLabel* titleLabel, *accountLabel;
	QComboBox* serverListComboBox;
	UploadEngineManager* uploadEngineManager;
	ServerProfile serverProfile_;
	bool showDefaultServerItem;
	int serversMask;
	bool showFileSizeLimits;
	void serverChanged();

	


};

#endif