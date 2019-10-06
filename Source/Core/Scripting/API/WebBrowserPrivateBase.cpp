#include "WebBrowserPrivateBase.h"
#include "ScriptAPI.h"

namespace ScriptAPI {

WebBrowserPrivateBase::WebBrowserPrivateBase() : timerInterval_(0)
{
    AddBrowserToVM(GetCurrentThreadVM(), this);
}

WebBrowserPrivateBase::~WebBrowserPrivateBase()
{
    RemoveBrowserToVM(GetCurrentThreadVM(), this);
}

void WebBrowserPrivateBase::abort()
{
    clearCallbacks();
}

void WebBrowserPrivateBase::clearCallbacks()
{
    onUrlChangedCallback_.Release();
    onUrlChangedCallbackContext_.Release();
    onNavigateErrorCallback_.Release();
    onNavigateErrorCallbackContext_.Release();
    onLoadFinishedCallback_.Release();
    onLoadFinishedCallbackContext_.Release();
    onTimerCallback_.Release();
    onTimerCallbackContext_.Release();
    onFileFieldFilledCallback_.Release();
    onFileFieldFilledCallbackContext_.Release();
}

}