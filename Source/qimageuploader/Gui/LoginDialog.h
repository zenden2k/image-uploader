#ifndef QIMAGEUPLOADER_GUI_LOGINDIALOG_H
#define QIMAGEUPLOADER_GUI_LOGINDIALOG_H

#include <QDialog>
#include "Core/Upload/UploadEngine.h"

namespace Ui {
class LoginDialog;
}

class LoginDialog : public QDialog
{
    Q_OBJECT
    
public:
    explicit LoginDialog(ServerProfile& serverProfile, bool createNew, QWidget *parent = 0);
    ~LoginDialog();
	QString accountName() const;
private:
	std::unique_ptr<Ui::LoginDialog> ui;
	ServerProfile& serverProfile_;
	QString accountName_;
	bool createNew_;
	void onAccept();

};

#endif 
