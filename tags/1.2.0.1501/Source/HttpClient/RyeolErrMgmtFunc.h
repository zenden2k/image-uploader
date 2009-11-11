/*!
 * \file	RyeolErrMgmtFunc.h
 * \brief	Functions related to error handling.
 * \author	Jo Hyeong-ryeol
 * \since	2003.07.07
 * \version	$LastChangedRevision: 102 $
 *			$LastChangedDate: 2006-02-05 00:33:27 +0900 (일, 05 2 2006) $
 *
 * This file contains functions related to error handling.
 * \n\n\n
 * Copyright &copy; 2006 by <a href="mailto:hyeongryeol@gmail.com">Jo Hyeong-ryeol</a>\n
 * Permission to copy, use, modify, sell and distribute this software is
 * granted provided this copyright notice appears in all copies.
 * This software is provided "as is" without express or implied warranty,
 * and with no claim as to its suitability for any purpose.
 */
#pragma once

#include <windows.h>
#include <atlbase.h>
#include <mbstring.h>			// for mbsnbcpy 
#define STRSAFE_NO_DEPRECATE
#include <strsafe.h>			// for String.. functions
#include <crtdbg.h>				// for _ASSERTE 

#include "RyeolStringFunc.h"	// for Ansi2Unicode function

/*!
 * \brief	The namespace of the Ryeol's library
 *
 * This is the namespace for source codes written by Jo Hyeong-ryeol.
 */
namespace Ryeol {

	/*! \brief	Creates a custom error message which has an appended system error message (ANSI Version) */
	void GetSystemErrMsgA (LPSTR pszBuff, DWORD cchBuff, LPCSTR pszErrDesc = NULL) ;
	/*! \brief	Creates a custom error message which has an appended system error message (Unicode Version) */
	void GetSystemErrMsgW (LPWSTR pszBuff, DWORD cchBuff, LPCWSTR pszErrDesc = NULL) ;
	/*! \brief	Creates a custom error message which has an appended system error message (Generic Version) */
	inline
	void GetSystemErrMsg (LPTSTR pszBuff, DWORD cchBuff, LPCTSTR pszErrDesc = NULL) {

#ifdef UNICODE
		GetSystemErrMsgW (pszBuff, cchBuff, pszErrDesc) ;
#else
		GetSystemErrMsgA (pszBuff, cchBuff, pszErrDesc) ;
#endif

	}

	/*! \brief	Creates a custom error message which has an appended system error message (ANSI Version) */
	void GetSystemErrMsgA (LPSTR pszBuff, DWORD cchBuff, DWORD dwErrCode, LPCSTR pszErrDesc = NULL) ;
	/*! \brief	Creates a custom error message which has an appended system error message (Unicode Version) */
	void GetSystemErrMsgW (LPWSTR pszBuff, DWORD cchBuff, DWORD dwErrCode, LPCWSTR pszErrDesc = NULL) ;
	/*! \brief	Creates a custom error message which has an appended system error message (Generic Version) */
	inline
	void GetSystemErrMsg (LPTSTR pszBuff, DWORD cchBuff, DWORD dwErrCode, LPCTSTR pszErrDesc = NULL) {

#ifdef UNICODE
		GetSystemErrMsgW (pszBuff, cchBuff, dwErrCode, pszErrDesc) ;
#else
		GetSystemErrMsgA (pszBuff, cchBuff, dwErrCode, pszErrDesc) ;
#endif

	}

	/*! \brief	Creates a custom error message which has an appended WinInet error message (ANSI Version) */
	void GetWinInetErrMsgA (LPSTR pszBuff, DWORD cchBuff, LPCSTR pszErrDesc = NULL) ;
	/*! \brief	Creates a custom error message which has an appended WinInet error message (Unicode Version) */
	void GetWinInetErrMsgW (LPWSTR pszBuff, DWORD cchBuff, LPCWSTR pszErrDesc = NULL) ;
	/*! \brief	Creates a custom error message which has an appended WinInet error message (Generic Version) */
	inline
	void GetWinInetErrMsg (LPTSTR pszBuff, DWORD cchBuff, LPCTSTR pszErrDesc = NULL) {

#ifdef UNICODE
		GetWinInetErrMsgW (pszBuff, cchBuff, pszErrDesc) ;
#else
		GetWinInetErrMsgA (pszBuff, cchBuff, pszErrDesc) ;
#endif

	}

	/*! \brief	Creates a custom error message which has an appended WinInet error message (ANSI Version) */
	void GetWinInetErrMsgA (LPSTR pszBuff, DWORD cchBuff, DWORD dwErrCode, LPCSTR pszErrDesc = NULL) ;
	/*! \brief	Creates a custom error message which has an appended WinInet error message (Unicode Version) */
	void GetWinInetErrMsgW (LPWSTR pszBuff, DWORD cchBuff, DWORD dwErrCode, LPCWSTR pszErrDesc = NULL) ;
	/*! \brief	Creates a custom error message which has an appended WinInet error message (Generic Version) */
	inline
	void GetWinInetErrMsg (LPTSTR pszBuff, DWORD cchBuff, DWORD dwErrCode, LPCTSTR pszErrDesc = NULL) {

#ifdef UNICODE
		GetWinInetErrMsgW (pszBuff, cchBuff, dwErrCode, pszErrDesc) ;
#else
		GetWinInetErrMsgA (pszBuff, cchBuff, dwErrCode, pszErrDesc) ;
#endif

	}

	/*! \brief	Creates a custom error message which has an appended COM error message (ANSI Version) */
	BOOL GetCOMErrMsgA (LPSTR pszBuff, DWORD cchBuff, IErrorInfo * pIErrorInfo, LPCSTR pszErrDesc = NULL) ;
	/*! \brief	Creates a custom error message which has an appended COM error message (Unicode Version) */
	BOOL GetCOMErrMsgW (LPWSTR pszBuff, DWORD cchBuff, IErrorInfo * pIErrorInfo, LPCWSTR pszErrDesc = NULL) ;
	/*! \brief	Creates a custom error message which has an appended COM error message (Generic Version) */
	inline
	BOOL GetCOMErrMsg (LPTSTR pszBuff, DWORD cchBuff, IErrorInfo * pIErrorInfo, LPCTSTR pszErrDesc = NULL) {

#ifdef UNICODE
		return GetCOMErrMsgW (pszBuff, cchBuff, pIErrorInfo, pszErrDesc) ;
#else
		return GetCOMErrMsgA (pszBuff, cchBuff, pIErrorInfo, pszErrDesc) ;
#endif

	}

	/*! \brief	Creates a custom error message which has an appended COM error message (ANSI Version) */
	BOOL GetCOMErrMsgA (LPSTR pszBuff, DWORD cchBuff, LPCSTR pszErrDesc = NULL) ;
	/*! \brief	Creates a custom error message which has an appended COM error message (Unicode Version) */
	BOOL GetCOMErrMsgW (LPWSTR pszBuff, DWORD cchBuff, LPCWSTR pszErrDesc = NULL) ;
	/*! \brief	Creates a custom error message which has an appended COM error message (Generic Version) */
	inline
	BOOL GetCOMErrMsg (LPTSTR pszBuff, DWORD cchBuff, LPCTSTR pszErrDesc = NULL) {

#ifdef UNICODE
		return GetCOMErrMsgW (pszBuff, cchBuff, pszErrDesc) ;
#else
		return GetCOMErrMsgA (pszBuff, cchBuff, pszErrDesc) ;
#endif

	}


	/*! \brief	Set a COM IErrorInfo (ANSI Version) */
	HRESULT SetCOMErrInfoA (const GUID * pGUID, LPCSTR szSource, LPCSTR szDescription, LPCSTR szHelpFile, DWORD dwHelpContext) ;
	/*! \brief	Set a COM IErrorInfo (Unicode Version) */
	HRESULT SetCOMErrInfoW (const GUID * pGUID, LPCWSTR szSource, LPCWSTR szDescription, LPCWSTR szHelpFile, DWORD dwHelpContext) ;
	/*! \brief	Set a COM IErrorInfo (Generic Version) */
	inline
	HRESULT	SetCOMErrInfo (const GUID * pGUID, LPCTSTR szSource, LPCTSTR szDescription, LPCTSTR szHelpFile, DWORD dwHelpContext) {

#ifdef UNICODE
		return SetCOMErrInfoW (pGUID, szSource, szDescription, szHelpFile, dwHelpContext) ;
#else
		return SetCOMErrInfoA (pGUID, szSource, szDescription, szHelpFile, dwHelpContext) ;
#endif

	}

	/*! \brief	Set a COM error message (ANSI Version)
	 *
	 * This function set a COM error message.
	 *
	 * \param szDescription	[in] Brief, null-terminated string that describes the error.
	 * \return				The standard COM HRESULTs.
	 * \sa					::CreateErrorInfo
	 * \sa					::SetErrorInfo
	 */
	inline
	HRESULT	SetCOMErrMsgA (LPCSTR szDescription) {
		return SetCOMErrInfoA (NULL, NULL, szDescription, NULL, 0) ;
	}

	/*! \brief	Set a COM error message (Unicode Version)
	 *
	 * This function set a COM error message.
	 *
	 * \param szDescription	[in] Brief, null-terminated string that describes the error.
	 * \return				The standard COM HRESULTs.
	 * \sa					::CreateErrorInfo
	 * \sa					::SetErrorInfo
	 */
	inline
	HRESULT	SetCOMErrMsgW (LPCWSTR wszDescription) {
		return SetCOMErrInfoW (NULL, NULL, wszDescription, NULL, 0) ;
	}

	/*! \brief	Set a COM error message (Generic Version)
	 *
	 * This function set a COM error message.
	 *
	 * \param szDescription	[in] Brief, null-terminated string that describes the error.
	 * \return				The standard COM HRESULTs.
	 * \sa					::CreateErrorInfo
	 * \sa					::SetErrorInfo
	 */
	inline
	HRESULT	SetCOMErrMsg (LPCTSTR szDescription) {

#ifdef UNICODE
		return SetCOMErrMsgW (szDescription) ;
#else
		return SetCOMErrMsgA (szDescription) ;
#endif

	}


#ifndef SAFEFREE
#	define SAFEFREE(x) if(x){ ::free((void *)x); x = NULL; }
#endif

#ifndef INVALID_SET_FILE_POINTER
#	define INVALID_SET_FILE_POINTER ((DWORD)-1)
#endif

}

