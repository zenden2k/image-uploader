#pragma once

#include "atlheaders.h"
#include <exception>

class Win32Exception : public std::exception {
    CString m_message;
public:
    Win32Exception(const CString& message) : m_message(message) {}
    
    virtual const char* what() const noexcept override
    {
        return CT2A(m_message);
    }
    
    const CString& getMessage() const { return m_message; }
};
