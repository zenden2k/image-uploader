#ifndef SETTINGSPAGE_H
#define SETTINGSPAGE_H

#pragma once 

#include "atlheaders.h"
#include <windows.h>

#include "Func/Library.h"

class CWizardDlg;
class CSettingsDlg;


class ValidationException : public std::runtime_error {
public:
    struct ValidationError {
        CString Message;
        HWND Control;

        ValidationError() {
            Control = nullptr;
        }
        ValidationError(CString message, HWND control) : Message(message), Control(control) {
        }
    };
    ValidationException(CString Message, HWND Control = nullptr) : std::runtime_error("Form validation error") {
        errors_.push_back(ValidationError(Message, Control));
    }
    ValidationException(std::vector<ValidationError> errors) : std::runtime_error("Form validation error") {
        errors_ = errors;
    }
    ValidationException(const ValidationException& ex) : std::runtime_error(ex), errors_(ex.errors_) {}
    ValidationException& operator=(const ValidationException& ex) {
        this->errors_ = ex.errors_;
        return *this;
    }
    std::vector<ValidationError> errors_;
};

class CSettingsPage
{
    public:
        virtual ~CSettingsPage()=0;
        CWizardDlg *WizardDlg;
        HBITMAP HeadBitmap;
        virtual bool OnShow();
        virtual bool OnHide();
        virtual bool OnNext();
        void EnableNext(bool Enable = true);
        void EnablePrev(bool Enable = true);
        void EnableExit(bool Enable = true);
        void SetNextCaption(LPTSTR Caption);
        HWND PageWnd;
        void ShowNext(bool Show = true);
        void ShowPrev(bool Show = true);

        void FixBackground();
        static void TabBackgroundFix(HWND hwnd);
        
        virtual bool Apply();
};

#endif // SETTINGSPAGE_H