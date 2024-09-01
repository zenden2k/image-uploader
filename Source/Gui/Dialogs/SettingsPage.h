#ifndef SETTINGSPAGE_H
#define SETTINGSPAGE_H

#pragma once 

#include "atlheaders.h"
#include <windows.h>
#include <stdexcept>
#include "Func/Library.h"


class CWizardDlg;
class CSettingsDlg;

struct ValidationError {
    CString Message;
    HWND Control;

    ValidationError()
    {
        Control = nullptr;
    }
    ValidationError(CString message, HWND control)
        : Message(message)
        , Control(control)
    {
    }
};

class ValidationException : public std::exception {
public:
    struct ValidationError {
        CString Message;
        HWND Control;

        ValidationError()
        {
            Control = nullptr;
        }
        ValidationError(CString message, HWND control)
            : Message(message)
            , Control(control)
        {
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
        virtual bool onShow();
        virtual bool onHide();
        HWND PageWnd = nullptr;
        void fixBackground() const;

        void addError(CString message, HWND control = NULL);
        virtual bool apply();
        virtual bool validate();
        void clearErrors();
        const std::vector<ValidationError>& errors() const;
    protected:
         std::vector<ValidationError> errors_;
};

#endif // SETTINGSPAGE_H
