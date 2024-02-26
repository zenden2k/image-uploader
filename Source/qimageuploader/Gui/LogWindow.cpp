#include "LogWindow.h"
#include <QStringListModel>
#include "Gui/controls/LogListItemDelegate.h"
#include "ui_LogWindow.h"
#include "models/LogListModel.h"

LogWindow::LogWindow(QWidget *parent) :
    QDialog(parent),
	ui(new Ui::LogWindow) 
{
	ui->setupUi(this);
    ui->plainTextEdit->setReadOnly(true);
    logModel_ = new LogListModel(this);
    ui->listView->setModel(logModel_);
    setWindowFlags(windowFlags() | Qt::Tool | Qt::WindowCloseButtonHint);
    setAttribute(Qt::WA_QuitOnClose, false);
     ui->listView->setItemDelegate(new LogListItemDelegate(this));
    logModel_->addItem(ILogger::logError, "Main window", "Hello wo", "SOme info");

}

LogWindow::~LogWindow()
{
}

void LogWindow::writeLog(ILogger::LogMsgType MsgType, QString Sender, QString Msg, QString Info)
{
    QString line;
    line.fill('-', 70);
	QString msg = "Sender:" + Sender + "\r\n" + Info + "\r\n" + Msg + "\r\n" + line + "\r\n";
    QMetaObject::invokeMethod(this, "writeLogInMainThread", MsgType, Sender,  msg, Info);
}

void LogWindow::writeLogInMainThread(ILogger::LogMsgType MsgType, QString Sender, QString Msg, QString Info) {
    logModel_->addItem(MsgType, Sender, Msg, Info);
    //ui->plainTextEdit->appendPlainText(msg);
}

