#include "LogWindow.h"

#include "ui_LogWindow.h"

LogWindow::LogWindow(QWidget *parent) :
    QDialog(parent),
	ui(new Ui::LogWindow) 
{
	ui->setupUi(this);
    ui->plainTextEdit->setReadOnly(true);
    setWindowFlags(windowFlags() | Qt::Tool | Qt::WindowCloseButtonHint);
    setAttribute(Qt::WA_QuitOnClose, false);
}


LogWindow::~LogWindow()
{
}

void LogWindow::writeLog(LogMsgType MsgType, QString Sender, QString Msg, QString Info)
{
    QString line;
    line.fill('-', 70);
	QString msg = "Sender:" + Sender + "\r\n" + Info + "\r\n" + Msg + "\r\n" + line + "\r\n";
    QMetaObject::invokeMethod(this, "writeLogInMainThread", Q_ARG(QString, msg));	
}

void LogWindow::writeLogInMainThread(QString msg) {
    ui->plainTextEdit->appendPlainText(msg);
}

