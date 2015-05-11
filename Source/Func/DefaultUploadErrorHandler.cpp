#include "DefaultUploadErrorHandler.h"

#include "Gui/Dialogs/LogWindow.h"
#include "Gui/Dialogs/IntegrationSettings.h"
#include "Gui/Dialogs/TextViewDlg.h"


DefaultUploadErrorHandler::DefaultUploadErrorHandler(ILogger* logger) {
    logger_ = logger;
}


void DefaultUploadErrorHandler::ErrorMessage(ErrorInfo errorInfo)
{
    LogMsgType type = errorInfo.messageType == (ErrorInfo::mtWarning) ? logWarning : logError;
    CString errorMsg;

    CString infoText;
    if (!errorInfo.FileName.empty())
        infoText += TR("Файл: ") + Utf8ToWCstring(errorInfo.FileName) + _T("\n");

    if (!errorInfo.ServerName.empty()) {
        CString serverName = Utf8ToWCstring(errorInfo.ServerName);
        if (!errorInfo.sender.empty())
            serverName += _T("(") + Utf8ToWCstring(errorInfo.sender) + _T(")");
        infoText += TR("Сервер: ") + serverName + _T("\n");
    }

    if (!errorInfo.Url.empty())
        infoText += _T("URL: ") + Utf8ToWCstring(errorInfo.Url) + _T("\n");

    if (errorInfo.ActionIndex != -1)
        infoText += _T("Действие:") + CString(_T(" #")) + WinUtils::IntToStr(errorInfo.ActionIndex);

    if (infoText.Right(1) == _T("\n"))
        infoText.Delete(infoText.GetLength() - 1);
    if (!errorInfo.error.empty()) {
        errorMsg += Utf8ToWCstring(errorInfo.error);
    } else {
        if (errorInfo.errorType == etRepeating) {
            errorMsg.Format(TR("Загрузка на сервер не удалась. Повторяю (%d)..."), errorInfo.RetryIndex);
        } else if (errorInfo.errorType == etRetriesLimitReached) {
            errorMsg = TR("Загрузка не сервер удалась! (лимит попыток исчерпан)");
        }
    }

    CString sender = TR("Модуль загрузки");
    if (!errorMsg.IsEmpty())
        logger_->write(type, sender, errorMsg, infoText);
}

int responseFileIndex = 1;

void DefaultUploadErrorHandler::DebugMessage(const std::string& msg, bool isResponseBody)
{
    if (!isResponseBody)
        MessageBox(0, Utf8ToWCstring(msg.c_str()), _T("Uploader"), MB_ICONINFORMATION);
    else {
        CTextViewDlg TextViewDlg(Utf8ToWstring(msg).c_str(), CString(_T("Server reponse")), CString(_T("Server reponse:")),
            _T("Save to file?"));

        if (TextViewDlg.DoModal(GetActiveWindow()) == IDOK) {
            CFileDialog fd(false, 0, 0, 4 | 2, _T("*.html\0*.html\0\0"), GetActiveWindow());
            CString fileName;
            fileName.Format(_T("response_%02d.html"), responseFileIndex++);
            lstrcpy(fd.m_szFileName, fileName);
            if (fd.DoModal() == IDOK) {
                FILE* f = _tfopen(fd.m_szFileName, _T("wb"));
                if (f) {
                    // WORD BOM = 0xFEFF;
                    // fwrite(&BOM, sizeof(BOM),1,f);
                    fwrite(msg.c_str(), msg.size(), sizeof(char), f);
                    fclose(f);
                }
            }
        }
    }
}
