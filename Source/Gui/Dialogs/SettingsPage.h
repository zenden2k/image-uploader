#ifndef SETTINGSPAGE_H
#define SETTINGSPAGE_H

#pragma once 

#include "atlheaders.h"
#include <windows.h>
#include <stdexcept>
#include "Func/Library.h"


class CWizardDlg;
class CSettingsDlg;


class ValidationException : public std::exception {
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
    ValidationException(CString Message, HWND Control = nullptr) : std::exception("Form validation error") {
        try {
            errors_.emplace_back(Message, Control);
        } catch (const std::exception&) {
            
        }
    }
    ValidationException(std::vector<ValidationError> errors) : std::exception("Form validation error") {
        errors_ = std::move(errors);
    }
    ValidationException(const ValidationException& ex) : std::exception(ex), errors_(ex.errors_) {}
    ValidationException& operator=(const ValidationException& ex) {
        this->errors_ = ex.errors_;
        return *this;
    }
    std::vector<ValidationError> errors_;
};

class CSettingsPage
{
    public:
        CSettingsPage();
        virtual ~CSettingsPage() = default;
        CWizardDlg *WizardDlg = nullptr;
        HBITMAP HeadBitmap = nullptr;
        virtual bool OnShow();
        virtual bool OnHide();
        virtual bool OnNext();
        void EnableNext(bool Enable = true);
        void EnablePrev(bool Enable = true);
        void EnableExit(bool Enable = true);
        void SetNextCaption(LPTSTR Caption);
        HWND PageWnd = nullptr;
        void ShowNext(bool Show = true);
        void ShowPrev(bool Show = true);

        void FixBackground() const;
        static void TabBackgroundFix(HWND hwnd);
        
        virtual bool Apply();
};

#endif // SETTINGSPAGE_H