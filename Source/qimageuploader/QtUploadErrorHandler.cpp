#include "QtUploadErrorHandler.h"

#include <QString>
#include <QDebug>
#include "Core/CommonDefs.h"
#include "Core/i18n/Translator.h"

QtUploadErrorHandler::QtUploadErrorHandler(ILogger* logger) {
    logger_ = logger;
    responseFileIndex_ = 0;
}

void QtUploadErrorHandler::ErrorMessage(const ErrorInfo& errorInfo)
{
    LogMsgType type = errorInfo.messageType == (ErrorInfo::mtWarning) ? logWarning : logError;
    QString errorMsg;

	QString infoText;
    if (!errorInfo.FileName.empty())
        infoText += tr("File: ").c_str() + U2Q(errorInfo.FileName) + "\n";

    if (!errorInfo.ServerName.empty()) {
		QString serverName = U2Q(errorInfo.ServerName);
        if (!errorInfo.sender.empty())
            serverName += "(" + U2Q(errorInfo.sender) +")";
        infoText += tr("Server: ").c_str() + serverName + "\n";
    }

    if (!errorInfo.Url.empty())
        infoText += QString::fromUtf8(tr("URL: ").c_str()) + U2Q(errorInfo.Url) + "\n";

    if (errorInfo.ActionIndex != -1)
        infoText += QString::fromUtf8(tr("Action:").c_str()) + " #" + QString::number(errorInfo.ActionIndex);

    /*if (infoText.Right(1) == _T("\n"))
        infoText.Delete(infoText.GetLength() - 1);*/
    if (!errorInfo.error.empty()) {
        errorMsg += U2Q(errorInfo.error);
    } else {
        /*if (errorInfo.errorType == etRepeating) {
            errorMsg.Format(TR("Upload failed. Another retry (%d)..."), errorInfo.RetryIndex);
        } else if (errorInfo.errorType == etRetriesLimitReached) {
            errorMsg = TR("Upload failed! (retry limit reached)");
        }*/
    }

    QString sender = QString::fromUtf8(tr("Uploading module").c_str());
	if (!errorMsg.isEmpty())
		qDebug() << errorMsg;
        //logger_->write(type, sender, errorMsg, infoText);
}

void QtUploadErrorHandler::DebugMessage(const std::string& msg, bool isResponseBody)
{

}
