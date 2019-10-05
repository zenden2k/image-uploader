#include "DefaultUploadErrorHandler.h"

#ifdef IU_WTL_APP
#include "Gui/Dialogs/TextViewDlg.h"
#endif
#include "Func/WinUtils.h"
#include "Core/i18n/Translator.h"

DefaultUploadErrorHandler::DefaultUploadErrorHandler(std::shared_ptr<ILogger> logger):
    logger_(logger)
{
    responseFileIndex_ = 0;
}

void DefaultUploadErrorHandler::ErrorMessage(const ErrorInfo& errorInfo)
{
    ILogger::LogMsgType type;

    switch (errorInfo.messageType) {
        case ErrorInfo::mtWarning:
            type = ILogger::logWarning;
            break;
        case ErrorInfo::mtError:
            type = ILogger::logError;
            break;
        case ErrorInfo::mtInformation:
            type = ILogger::logInformation;
            break;
        default:
            type = ILogger::logError;
    }

    CString errorMsg, infoText;
    if (!errorInfo.FileName.empty())
        infoText += TR("File: ") + Utf8ToWCstring(errorInfo.FileName) + _T("\n");

    if (!errorInfo.ServerName.empty()) {
        CString serverName = Utf8ToWCstring(errorInfo.ServerName);
        if (!errorInfo.sender.empty())
            serverName += _T(" (") + Utf8ToWCstring(errorInfo.sender) + _T(")");
        infoText += TR("Server: ") + serverName + _T("\n");
    }

    if (!errorInfo.Script.empty()) {
        CString script = Utf8ToWCstring(errorInfo.Script);
        infoText += TR("Script: ") + script;
        if (errorInfo.ThreadId != std::thread::id()) {
            infoText += CString(_T(" [TID=")) + IuCoreUtils::ThreadIdToString(errorInfo.ThreadId).c_str() + _T("]");
        }
        infoText += _T("\n");
    }

    if (!errorInfo.Url.empty())
        infoText += _T("URL: ") + Utf8ToWCstring(errorInfo.Url) + _T("\n");

    if (errorInfo.ActionIndex != -1)
        infoText += TR("Action:") + CString(_T(" #")) + WinUtils::IntToStr(errorInfo.ActionIndex);

    if (infoText.Right(1) == _T("\n"))
        infoText.Delete(infoText.GetLength() - 1);
    if (!errorInfo.error.empty()) {
        errorMsg += Utf8ToWCstring(errorInfo.error);
    } else {
        if (errorInfo.errorType == etRepeating) {
            errorMsg.Format(TR("Upload failed. Another retry (%d)..."), errorInfo.RetryIndex);
        } else if (errorInfo.errorType == etRetriesLimitReached) {
            errorMsg = TR("Upload failed! (retry limit reached)");
        }
    }

    CString sender = TR("Uploading module");
    CString topLevelFileName = U2W(errorInfo.TopLevelFileName);
    if (!errorMsg.IsEmpty())
        logger_->write(type, sender, errorMsg, infoText, topLevelFileName);
}

void DefaultUploadErrorHandler::DebugMessage(const std::string& msg, bool isResponseBody)
{
    if (!isResponseBody)
        MessageBox(0, Utf8ToWCstring(msg), _T("Uploader"), MB_ICONINFORMATION);
    else {
#ifdef IU_WTL_APP
        CTextViewDlg TextViewDlg(Utf8ToWstring(msg).c_str(), CString(_T("Server reponse")), CString(_T("Server reponse:")),
            _T("Save to file?"));

        if (TextViewDlg.DoModal(GetActiveWindow()) == IDOK) {
            CFileDialog fd(false, 0, 0, 4 | 2, _T("*.html\0*.html\0\0"), GetActiveWindow());
            CString fileName;
            fileName.Format(_T("response_%02d.html"), responseFileIndex_++);
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
#endif
    }
}
