#if !defined(AFX_THREAD_H__20000927_7992_09B2_A0EF_0080AD509054__INCLUDED_)
#define AFX_THREAD_H__20000927_7992_09B2_A0EF_0080AD509054__INCLUDED_

#pragma once

/////////////////////////////////////////////////////////////////////////////
// Thread - wrapper for Thread API
//
// Written by Bjarke Viksoe (bjarke@viksoe.dk)
//
// This code may be used in compiled form in any way you desire. This
// file may be redistributed by any means PROVIDING it is 
// not sold for profit without the authors written consent, and 
// providing that this notice and the authors name is included. 
//
// This file is provided "as is" with no expressed or implied warranty.
// The author accepts no liability if it causes any damage to you or your
// computer whatsoever. It's free, so don't hassle me about it.
//
// Beware of bugs.
//

#include <Windows.h>
#include <crtdbg.h>
#if defined(_MT) || defined(_DLL)
// Due to the nature of the multithreaded C runtime lib we
// need to use _beginthreadex() and _endthreadex() instead
// of CreateThread() and ExitThread(). See Q104641.
#include <process.h>
#endif

#ifndef ATLTRY
   #define ATLTRY(x) (x)
#endif


/////////////////////////////////////////////////////////////////////////////
// CThread

template< bool t_bManaged >
class CThreadT
{
public:
   HANDLE m_hThread;
   bool m_bSuspended;

   CThreadT(HANDLE hThread = NULL) : m_hThread(hThread), m_bSuspended(false)
   {
   }
   virtual ~CThreadT()
   {
      if( t_bManaged ) Release();
   }
   BOOL Creates(LPTHREAD_START_ROUTINE pThreadProc, 
               LPVOID pParam = NULL, 
               int iPriority = THREAD_PRIORITY_NORMAL)
   {
      _ASSERTE(m_hThread==NULL);
      _ASSERTE(pThreadProc);
      DWORD dwThreadID;
#if defined(_MT) || defined(_DLL)
      m_hThread = (HANDLE) _beginthreadex(NULL, 0, (UINT (WINAPI*)(void*)) pThreadProc, pParam, CREATE_SUSPENDED, (UINT*) &dwThreadID);
#else
      m_hThread = ::CreateThread(NULL, 0, pThreadProc, pParam, CREATE_SUSPENDED, &dwThreadID);
#endif
      if( m_hThread == NULL ) return FALSE;
      if( iPriority != THREAD_PRIORITY_NORMAL ) {
         if( !::SetThreadPriority(m_hThread, iPriority) ) {
            _ASSERTE(!"Couldn't set thread priority");
         }
      }
      ::ResumeThread(m_hThread);
      return TRUE;
   }
   BOOL Release()
   {
      if( m_hThread == NULL ) return TRUE;
      if( ::CloseHandle(m_hThread) == FALSE ) return FALSE;
      m_hThread = NULL;
      return TRUE;
   }
   void Attach(HANDLE hThread)
   {
      _ASSERTE(m_hThread==NULL);
      m_hThread = hThread;
   }
   HANDLE Detach()
   {
      HANDLE hThread = m_hThread;
      m_hThread = NULL;
      return hThread;
   }
   BOOL SetPriority(int iPriority) const
   {
      _ASSERTE(m_hThread);
      return ::SetThreadPriority(m_hThread, iPriority);
   }
   int GetPriority() const
   {
      _ASSERTE(m_hThread);
      return ::GetThreadPriority(m_hThread);
   }
   BOOL Suspend()
   {
      _ASSERTE(m_hThread);
      if( m_bSuspended ) return TRUE;
      if( ::SuspendThread(m_hThread) == (DWORD) -1 ) return FALSE;
      m_bSuspended = true;
      return TRUE;
   }
   BOOL Resume()
   {
      _ASSERTE(m_hThread);
      if( !m_bSuspended ) return TRUE;
      if( ::ResumeThread(m_hThread) == (DWORD) -1 ) return FALSE;
      m_bSuspended = false;
      return TRUE;
   }
   bool IsSuspended() const
   {
      _ASSERTE(m_hThread);
      return m_bSuspended == true;
   }
   bool IsRunning() const
   {
      if( m_hThread == NULL ) return FALSE;
      DWORD dwCode = 0;
      ::GetExitCodeThread(m_hThread, &dwCode);
      return dwCode == STILL_ACTIVE;
   }
   BOOL WaitForThread(DWORD dwTimeout = INFINITE) const
   {
      _ASSERTE(m_hThread);
      return ::WaitForSingleObject(m_hThread, dwTimeout) == WAIT_OBJECT_0;
   }
   BOOL Terminate(DWORD dwExitCode = 0) const
   {
      // See Q254956 why calling this could be a bad idea!
      _ASSERTE(m_hThread);
      return ::TerminateThread(m_hThread, dwExitCode);
   }
   BOOL GetExitCode(DWORD* pExitCode) const
   {
      _ASSERTE(m_hThread);
      _ASSERTE(pExitCode);
      return ::GetExitCodeThread(m_hThread, pExitCode);
   }
#if(WINVER >= 0x0500)
   BOOL GetThreadTimes(LPFILETIME lpCreationTime, LPFILETIME lpExitTime, LPFILETIME lpKernelTime, LPFILETIME lpUserTime) const
   {
      _ASSERTE(m_hThread);
      _ASSERTE(lpExitTime!=NULL && lpKernelTime!=NULL && lpUserTime!=NULL);
      return ::GetThreadTimes(m_hThread, lpExitTime, lpKernelTime, lpUserTime);
   }
#endif
   operator HANDLE() const { return m_hThread; }
};

typedef CThreadT<false> CThreadHandle;
typedef CThreadT<true> CThread;


/////////////////////////////////////////////////////////////////////////////
// CThreadImpl

template< class T >
class CThreadImpl : public CThread
{
public:
   bool /*volatile*/ m_bStopped; // Signals when thread should stop (thread loop should exit)

   CThreadImpl() : m_bStopped(false)
   {
   }
   virtual ~CThreadImpl()
   {
      // NOTE: Remember destructors cannot call overrides!
      Stop();
   }
   virtual BOOL Start( int iPriority = THREAD_PRIORITY_NORMAL)
   {
      m_bStopped = false;
      if( !Creates(ThreadProc, (LPVOID) static_cast<T*>(this), iPriority ) ) return FALSE;
      return TRUE;
   }
   void Stop()
   {
      if( !SignalStop() ) return;
      WaitForThread();
      Release();
   }
   BOOL SignalStop()
   {
      if( m_hThread == NULL ) return FALSE;
      m_bStopped = true;
      if( m_bSuspended ) Resume();
      return TRUE;
   }
   BOOL ShouldStop() const
   {
      _ASSERTE(m_hThread);
      return m_bStopped == true;
   }

   static DWORD WINAPI ThreadProc(LPVOID pData)
   {
      T* pThis = static_cast<T*>(pData);
#if defined(_MT) || defined(_DLL)
      ATLTRY(_endthreadex( pThis->Run() ));
      return 0;
#else
      DWORD dwRet = 0;
      ATLTRY(dwRet = pThis->Run());
      return dwRet;
#endif
   }
   DWORD Run()
   {
      _ASSERTE(false); // must override this
/*
      // Sample thread code...
      while( !ShouldStop() ) {
         ...
      }
*/
      return 0;
   }
};


/////////////////////////////////////////////////////////////////////////////
// CEvent

class CEvent
{
public:
   HANDLE m_hEvent;

   CEvent(HANDLE hEvent = INVALID_HANDLE_VALUE) : m_hEvent(hEvent)
   { 
   }
   ~CEvent()
   {
      Close();
   }
   BOOL Create(LPCTSTR pstrName = NULL, BOOL bManualReset = FALSE, BOOL bInitialState = FALSE, LPSECURITY_ATTRIBUTES pEventAttributes = NULL)
   {
       _ASSERTE(pstrName == NULL || !::IsBadStringPtr(pstrName, static_cast<UINT_PTR>(-1)));
      _ASSERTE(m_hEvent==INVALID_HANDLE_VALUE);
      m_hEvent = ::CreateEvent(pEventAttributes, bManualReset, bInitialState, pstrName);
      _ASSERTE(m_hEvent!=INVALID_HANDLE_VALUE);
      return m_hEvent != INVALID_HANDLE_VALUE;
   }
   BOOL Open(LPCTSTR pstrName, DWORD dwDesiredAccess = EVENT_ALL_ACCESS, BOOL bInheritHandle = TRUE)
   {
      _ASSERTE(!::IsBadStringPtr(pstrName, static_cast<UINT_PTR>(-1)));
      _ASSERTE(m_hEvent==INVALID_HANDLE_VALUE);
      m_hEvent = ::OpenEvent(dwDesiredAccess, bInheritHandle, pstrName);
      _ASSERTE(m_hEvent!=INVALID_HANDLE_VALUE);
      return m_hEvent != INVALID_HANDLE_VALUE;
   }
   BOOL IsOpen() const
   {
      return m_hEvent != INVALID_HANDLE_VALUE;
   }
   void Close()
   {
      if( m_hEvent == INVALID_HANDLE_VALUE ) return;
      ::CloseHandle(m_hEvent);
      m_hEvent = INVALID_HANDLE_VALUE;
   }
   void Attach(HANDLE hEvent)
   {
      _ASSERTE(m_hEvent==INVALID_HANDLE_VALUE);
      m_hEvent= hEvent;
   }  
   HANDLE Detach()
   {
      HANDLE hEvent = m_hEvent;
      m_hEvent = INVALID_HANDLE_VALUE;
      return hEvent;
   }
   BOOL ResetEvent()
   {
      _ASSERTE(m_hEvent!=INVALID_HANDLE_VALUE);
      return ::ResetEvent(m_hEvent);
   }
   BOOL SetEvent()
   {
      _ASSERTE(m_hEvent!=INVALID_HANDLE_VALUE);
      return ::SetEvent(m_hEvent);
   }
   BOOL PulseEvent()
   {
      _ASSERTE(m_hEvent!=INVALID_HANDLE_VALUE);
      return ::PulseEvent(m_hEvent);
   }
   BOOL WaitForEvent(DWORD dwTimeout = INFINITE)
   {
      _ASSERTE(m_hEvent!=INVALID_HANDLE_VALUE);
      return ::WaitForSingleObject(m_hEvent, dwTimeout) == WAIT_OBJECT_0;
   }
   operator HANDLE() const { return m_hEvent; }
};


/////////////////////////////////////////////////////////////////////////////
// CCriticalSection

#ifndef __ATLBASE_H__

class CCriticalSection
{
public:
   CRITICAL_SECTION m_sec;

   void Init() 
   {
      ::InitializeCriticalSection(&m_sec);
   }
   void Term() 
   {
      ::DeleteCriticalSection(&m_sec);
   }
   void Lock() 
   {
      ::EnterCriticalSection(&m_sec);
   }
   void Unlock() 
   {
      ::LeaveCriticalSection(&m_sec);
   }
};

#endif

class CAutoCriticalSection
{
public:
   CRITICAL_SECTION m_sec;

   CAutoCriticalSection() 
   {
      ::InitializeCriticalSection(&m_sec);
   }
   ~CAutoCriticalSection() 
   {
      ::DeleteCriticalSection(&m_sec);
   }
   void Lock() 
   {
      ::EnterCriticalSection(&m_sec);
   }
   void Unlock() 
   {
      ::LeaveCriticalSection(&m_sec);
   }
};


#endif // !defined(AFX_THREAD_H__20000927_7992_09B2_A0EF_0080AD509054__INCLUDED_)

