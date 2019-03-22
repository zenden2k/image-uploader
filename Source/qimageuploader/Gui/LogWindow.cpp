#include "LogWindow.h"

#include "ui_LogWindow.h"

LogWindow::LogWindow(QWidget *parent) :
    QDialog(parent),
	ui(new Ui::LogWindow) 
{
	ui->setupUi(this);
    ui->plainTextEdit->setReadOnly(true);
}


LogWindow::~LogWindow()
{
}

void LogWindow::writeLog(LogMsgType MsgType, QString Sender, QString Msg, QString Info)
{
	QString msg = "Sender:" + Sender + "\r\n" + Info + "\r\n" + Msg + "\r\n------------------------------------\r\n";
    QMetaObject::invokeMethod(this, "writeLogInMainThread", Q_ARG(QString, msg));	
}

void LogWindow::writeLogInMainThread(QString msg) {
    ui->plainTextEdit->appendPlainText(msg);
}

