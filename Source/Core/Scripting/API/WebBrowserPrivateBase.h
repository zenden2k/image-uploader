#ifndef IU_CORE_WEBBROWSER_PRIVATEBASE_H
#define IU_CORE_WEBBROWSER_PRIVATEBASE_H

#pragma once

#include "ScriptAPI.h"
#include "Core/Scripting/Squirrelnc.h"

namespace ScriptAPI {
    class WebBrowserPrivateBase: public Stoppable {
    public:
        WebBrowserPrivateBase();
        virtual ~WebBrowserPrivateBase();
        void setOnUrlChangedCallback(Sqrat::Function callBack, Sqrat::Object context) {
            onUrlChangedCallback_ = callBack;
            onUrlChangedCallbackContext_ = context;
        }

        void setOnNavigateErrorCallback(Sqrat::Function callBack, Sqrat::Object context) {
            onNavigateErrorCallback_ = callBack;
            onNavigateErrorCallbackContext_ = context;
        }

        void setOnLoadFinishedCallback(Sqrat::Function callBack, Sqrat::Object context) {
            onLoadFinishedCallback_ = callBack;
            onLoadFinishedCallbackContext_ = context;
        }

        void setOnTimerCallback(int timerInterval, Sqrat::Function callBack, Sqrat::Object context) {
            onTimerCallback_ = callBack;
            onTimerCallbackContext_ = context;
            timerInterval_ = timerInterval;
        }

        void setOnFileInputFilledCallback(Sqrat::Function callBack, Sqrat::Object context) {
            onFileFieldFilledCallback_ = callBack;
            onFileFieldFilledCallbackContext_ = context;
        }

        virtual void close() = 0;
        virtual void abort();
        void stop() override;
        void clearCallbacks();

    protected:
        Sqrat::Function onUrlChangedCallback_;
        Sqrat::Object onUrlChangedCallbackContext_;
        Sqrat::Function onNavigateErrorCallback_;
        Sqrat::Object onNavigateErrorCallbackContext_;
        Sqrat::Function onLoadFinishedCallback_;
        Sqrat::Object onLoadFinishedCallbackContext_;
        Sqrat::Function onTimerCallback_;
        Sqrat::Object onTimerCallbackContext_;
        Sqrat::Function onFileFieldFilledCallback_;
        Sqrat::Object onFileFieldFilledCallbackContext_;
        int timerInterval_;
    };
}
#endif