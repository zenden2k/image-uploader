#ifndef QIMAGEUPLOADER_GUI_LOGWINDOW_H
#define QIMAGEUPLOADER_GUI_LOGWINDOW_H

#include <QDialog>
#include "Core/Logging/Logger.h"

namespace Ui {
class LogWindow;
}

class LogWindow : public QDialog
{
    Q_OBJECT
 
public:
    explicit LogWindow(QWidget *parent = 0);
    ~LogWindow();
	void writeLog(LogMsgType MsgType, QString Sender, QString  Msg, QString Info);
protected:
	std::unique_ptr<Ui::LogWindow> ui;

};

#endif // FRAMEGRABBERDLG_H
