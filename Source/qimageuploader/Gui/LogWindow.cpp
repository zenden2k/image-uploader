#include "LogWindow.h"

#include "ui_LogWindow.h"

LogWindow::LogWindow(QWidget *parent) :
    QDialog(parent),
	ui(new Ui::LogWindow) 
{
	ui->setupUi(this);
}


LogWindow::~LogWindow()
{
}

void LogWindow::writeLog(LogMsgType MsgType, QString Sender, QString Msg, QString Info)
{
	QString msg = "Sender:" + Sender + "\r\n" + Info + "\r\n" + Msg + "\r\n------------------------------------\r\n";
	QMetaObject::invokeMethod(this, [&, msg] {
		ui->plainTextEdit->appendPlainText(msg);
	});
	
}

