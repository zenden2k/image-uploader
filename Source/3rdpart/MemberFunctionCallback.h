#ifndef IU_MEMBER_FUNCTION_CALLBACK_H
#define IU_MEMBER_FUNCTION_CALLBACK_H

/*
Source: http://p-nand-q.com/programming/cplusplus/using_member_functions_with_c_function_pointers.html
*/
#pragma once
#include <windows.h>
#include "Core/3rdpart/fastdelegate.h"


typedef LRESULT (CALLBACK *LPFN_CBTHookCallback)(int nCode, WPARAM wParam, LPARAM lParam);

class CBTHookCallbackBase;
enum { kMaxCallbacks = 5};
extern  CBTHookCallbackBase* AvailableCallbackSlots[kMaxCallbacks];
typedef fastdelegate::FastDelegate3<int, WPARAM, LPARAM, LRESULT> HookCallback;
// this object holds the state for a C++ member function callback in memory
class CBTHookCallbackBase
{
public:
    // input: pointer to a unique C callback. 
    CBTHookCallbackBase(LPFN_CBTHookCallback pCCallback):
        m_pCCallback( pCCallback )
    {
    }

    // when done, remove allocation of the callback
    void Free()
    {
        m_pMethod = NULL;
        // not clearing m_pMethod: it won't be used, since m_pClass is NULL and so this entry is marked as free
    }

    // when free, allocate this callback
    LPFN_CBTHookCallback Reserve(const HookCallback& method)
    {
        if (m_pMethod)
            return NULL;

        m_pMethod = method;
        return m_pCCallback;
    }

protected:
    static LRESULT StaticInvoke(int context, int a, WPARAM b, LPARAM c) {
        return AvailableCallbackSlots[context]->m_pMethod(a, b,c);
    }

private:
    LPFN_CBTHookCallback m_pCCallback;
    HookCallback m_pMethod;
};


template <int context> class DynamicCBTHookCallback : public CBTHookCallbackBase
{
public:
    DynamicCBTHookCallback()
        :    CBTHookCallbackBase(&DynamicCBTHookCallback<context>::GeneratedStaticFunction)
    {
    }

private:
    static LRESULT CALLBACK GeneratedStaticFunction(int a, WPARAM b, LPARAM c)
    {
        int ab = context;
        ab;
        return StaticInvoke(context, a, b,c);
    }
};

class CBTHookMemberFunctionCallback
{
public:
    CBTHookMemberFunctionCallback(const HookCallback& method);
    ~CBTHookMemberFunctionCallback();

public:
    operator LPFN_CBTHookCallback() const
    {
        return m_cbCallback;
    }

    bool IsValid() const
    {
        return m_cbCallback != NULL;
    }

private:
    LPFN_CBTHookCallback m_cbCallback;
    int m_nAllocIndex;

private:
    CBTHookMemberFunctionCallback( const CBTHookMemberFunctionCallback& os );
    CBTHookMemberFunctionCallback& operator=( const CBTHookMemberFunctionCallback& os );
};



#endif
