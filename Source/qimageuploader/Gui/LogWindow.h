#ifndef QIMAGEUPLOADER_GUI_LOGWINDOW_H
#define QIMAGEUPLOADER_GUI_LOGWINDOW_H

#include <memory>
#include <QDialog>
#include "Core/Logging/Logger.h"

class LogListModel;

namespace Ui {
class LogWindow;
}

class LogWindow : public QDialog
{
    Q_OBJECT
 
public:
    explicit LogWindow(QWidget *parent = 0);
    ~LogWindow();
	void writeLog(ILogger::LogMsgType MsgType, QString Sender, QString  Msg, QString Info);
protected:
	std::unique_ptr<Ui::LogWindow> ui;
    LogListModel* logModel_;
    Q_INVOKABLE void writeLogInMainThread(ILogger::LogMsgType MsgType, QString Sender, QString Msg, QString Info);

};

#endif // FRAMEGRABBERDLG_H
