/*!
 * \file	RyeolErrMgmtFunc.cpp
 * \brief	Implementations of functions related to error handling.
 * \author	Jo Hyeong-ryeol
 * \since	2003.07.07
 * \version	$LastChangedRevision: 89 $
 *			$LastChangedDate: 2006-01-30 11:20:19 +0900 (월, 30 1 2006) $
 *
 * This file contains implementations of functions related to error handling.
 * \n\n\n
 * Copyright &copy; 2006 by <a href="mailto:hyeongryeol@gmail.com">Jo Hyeong-ryeol</a>\n
 * Permission to copy, use, modify, sell and distribute this software is
 * granted provided this copyright notice appears in all copies.
 * This software is provided "as is" without express or implied warranty,
 * and with no claim as to its suitability for any purpose.
 */
#include "stdafx.h"
#include "RyeolErrMgmtFunc.h"

#pragma warning (disable: 4996)	// avoids 'This function or variable may be unsafe' message


/*!
 * \brief	The namespace of the Ryeol's library
 *
 * This is the namespace for source codes written by Jo Hyeong-ryeol.
 */
namespace Ryeol {


/*!
 * \internal
 * This function creates a custom error message which has an appended system error message.
 * This function first copies pszErrDesc to pszBuff and then copies a system
 * error message of an error code specified by dwErrCode.
 * If the buffer is not enough to copy the system error message, the system error message
 * is not copied. If the buffer is not enough to copy pszErrDesc, a truncated pszErrDesc
 * is copied to the buffer. This function always succeed.
 *
 * \param szBuff		[out] A buffer to save the result error message. The buffer can not be NULL.
 * \param cchBuff		[in] Number of characters of the buffer, including the terminating NULL character.
 * \param dwErrCode		[in] A system error code. If the bUseLastErr is TRUE, this parameter is ignored.
 * \param pszErrDesc	[in] An user's custom error message. The system error message is appended after it.
 *                           This parameter is ignored if the value is NULL.
 * \param bUseLastErr	[in] If this is TRUE, an error code which is returned by ::GetLastError ()
 *                           is used as the system error code, otherwise dwErrCode is used.
 * \param hModule		[in] A module to get a message table. 
 *                           This parameter is ignored if the value is NULL.
 * \sa					::FormatMessage
 * \sa					::GetLastError
 */
static void
GetSystemErrMsgA_Impl (LPSTR pszBuff, DWORD cchBuff, DWORD dwErrCode
							, LPCSTR pszErrDesc, BOOL bUseLastErr, HMODULE hModule)
{
	// Check parameters
	_ASSERTE ( !(pszBuff == NULL || cchBuff == 0) ) ;

	if ( cchBuff == 1 ) {
		pszBuff[0] = NULL ;
		return ;
	}
	
	if ( bUseLastErr )
		dwErrCode = ::GetLastError () ;

	size_t		cchUsedBuff = 0 ;
	pszBuff[0] = NULL ;

	// Copies the user specified message
	if ( (pszErrDesc != NULL) && (::strlen (pszErrDesc) > 0) ) {
		::_mbsnbcpy (
			(unsigned char *) (pszBuff + cchUsedBuff)
			, (unsigned char *) (pszErrDesc)
			, cchBuff - cchUsedBuff - 1) ;
		pszBuff[cchBuff - 1] = NULL ;
		cchUsedBuff = ::strlen (pszBuff) ;
	}

	// If the buffer is full, then return.
	if ( cchUsedBuff >= cchBuff - 1 )
		return ;


	DWORD			dwFmtMsgFlag ;
	dwFmtMsgFlag = FORMAT_MESSAGE_FROM_SYSTEM ;

	// If the hModule is not NULL,
	if ( hModule )
		dwFmtMsgFlag |= FORMAT_MESSAGE_FROM_HMODULE ;

	// Copies the system error message.
	// If the buffer is not enough to copy the system error message,
	// the system error message is not copied.
	DWORD			dwWrittenChrs ;
	dwWrittenChrs = ::FormatMessageA (
		dwFmtMsgFlag
		, hModule						// lpSource
		, dwErrCode						// Error Code
		, 0								// dwLanguageID
		, pszBuff + cchUsedBuff			// lpBuffer
		, cchBuff - cchUsedBuff			// nSize
		, NULL) ;						// Arguments

	// If it fails to get a system error message,
	// copies the error code itself.
	// Errors caused by StringCbPrintf is ignored.
	if ( dwWrittenChrs == 0 ) {
		::StringCbPrintfA (
			pszBuff + cchUsedBuff
			, (cchBuff - cchUsedBuff) * sizeof (CHAR)
			, "Error Code is %u, FormatMessage Error Code is %u"
			, dwErrCode, ::GetLastError ()
		) ;
	}

}

/*!
 * \internal
 * This function creates a custom error message which has an appended system error message.
 * This function first copies pszErrDesc to pszBuff and then copies a system
 * error message of an error code specified by dwErrCode.
 * If the buffer is not enough to copy the system error message, the system error message
 * is not copied. If the buffer is not enough to copy pszErrDesc, a truncated pszErrDesc
 * is copied to the buffer. This function always succeed.
 *
 * \param szBuff		[out] A buffer to save the result error message. The buffer can not be NULL.
 * \param cchBuff		[in] Number of characters of the buffer, including the terminating NULL character.
 * \param dwErrCode		[in] A system error code. If the bUseLastErr is TRUE, this parameter is ignored.
 * \param pszErrDesc	[in] An user's custom error message. The system error message is appended after it.
 *                           This parameter is ignored if the value is NULL.
 * \param bUseLastErr	[in] If this is TRUE, an error code which is returned by ::GetLastError ()
 *                           is used as the system error code, otherwise dwErrCode is used.
 * \param hModule		[in] A module to get a message table resources. 
 *                           This parameter is ignored if the value is NULL.
 * \sa					::FormatMessage
 * \sa					::GetLastError
 */
static void
GetSystemErrMsgW_Impl (LPWSTR pszBuff, DWORD cchBuff, DWORD dwErrCode
							, LPCWSTR pszErrDesc, BOOL bUseLastErr, HMODULE hModule)
{
	// Check parameters
	_ASSERTE ( !(pszBuff == NULL || cchBuff == 0) ) ;

	if ( cchBuff == 1 ) {
		pszBuff[0] = NULL ;
		return ;
	}
	
	if ( bUseLastErr )
		dwErrCode = ::GetLastError () ;

	size_t		cchUsedBuff = 0 ;
	pszBuff[0] = NULL ;

	// Copies the user specified message
	if ( (pszErrDesc != NULL) && (::wcslen (pszErrDesc) > 0) ) {
		::StringCbCopyW (
			pszBuff + cchUsedBuff
			, (cchBuff - cchUsedBuff) * sizeof (WCHAR)
			, pszErrDesc) ;
		cchUsedBuff = ::wcslen (pszBuff) ;
	}

	// If the buffer is full, then return.
	if ( cchUsedBuff >= cchBuff - 1 )
		return ;


	DWORD			dwFmtMsgFlag ;
	dwFmtMsgFlag = FORMAT_MESSAGE_FROM_SYSTEM ;

	// If the hModule is not NULL,
	if ( hModule )
		dwFmtMsgFlag |= FORMAT_MESSAGE_FROM_HMODULE ;

	// Copies the system error message.
	// If the buffer is not enough to copy the system error message,
	// the system error message is not copied.
	DWORD			dwWrittenChrs ;
	dwWrittenChrs = ::FormatMessageW (
		dwFmtMsgFlag
		, hModule						// lpSource
		, dwErrCode						// Error Code
		, 0								// dwLanguageID
		, pszBuff + cchUsedBuff			// lpBuffer
		, cchBuff - cchUsedBuff			// nSize
		, NULL) ;						// Arguments


	// If it fails to get a system error message,
	// copies the error code itself.
	// Errors caused by StringCbPrintf is ignored.
	if ( dwWrittenChrs == 0 ) {
		::StringCbPrintfW (
			pszBuff + cchUsedBuff
			, (cchBuff - cchUsedBuff) * sizeof (WCHAR)
			, L"Error Code is %u, FormatMessage Error Code is %u"
			, dwErrCode, ::GetLastError ()
		) ;
	}

}


/*!
 * This function creates a custom error message which has an appended system error message.
 * This function first copies pszErrDesc to pszBuff and then copies a system
 * error message of an error code returned by ::GetLastError ().
 * If the buffer is not enough to copy the system error message, the system error message
 * is not copied. If the buffer is not enough to copy pszErrDesc, a truncated pszErrDesc
 * is copied to the buffer. This function always succeed.
 *
 * \param szBuff		[out] A buffer to save the result error message. The buffer can not be NULL.
 * \param cchBuff		[in] Number of characters of the buffer, including the terminating NULL character.
 * \param pszErrDesc	[in] An user's custom error message. The system error message is appended after it.
 *                           This parameter is ignored if the value is NULL.
 * \sa					::FormatMessage
 * \sa					::GetLastError
 */
void
GetSystemErrMsgA (LPSTR pszBuff, DWORD cchBuff, LPCSTR pszErrDesc)
{
	GetSystemErrMsgA_Impl (pszBuff, cchBuff, 0, pszErrDesc, TRUE, NULL) ;
}

/*!
 * This function creates a custom error message which has an appended system error message.
 * This function first copies pszErrDesc to pszBuff and then copies a system
 * error message of an error code returned by ::GetLastError ().
 * If the buffer is not enough to copy the system error message, the system error message
 * is not copied. If the buffer is not enough to copy pszErrDesc, a truncated pszErrDesc
 * is copied to the buffer. This function always succeed.
 *
 * \param szBuff		[out] A buffer to save the result error message. The buffer can not be NULL.
 * \param cchBuff		[in] Number of characters of the buffer, including the terminating NULL character.
 * \param pszErrDesc	[in] An user's custom error message. The system error message is appended after it.
 *                           This parameter is ignored if the value is NULL.
 * \sa					::FormatMessage
 * \sa					::GetLastError
 */
void
GetSystemErrMsgW (LPWSTR pszBuff, DWORD cchBuff, LPCWSTR pszErrDesc)
{
	GetSystemErrMsgW_Impl (pszBuff, cchBuff, 0, pszErrDesc, TRUE, NULL) ;
}


/*!
 * This function creates a custom error message which has an appended system error message.
 * This function first copies pszErrDesc to pszBuff and then copies a system
 * error message of an error code specified by dwErrCode.
 * If the buffer is not enough to copy the system error message, the system error message
 * is not copied. If the buffer is not enough to copy pszErrDesc, a truncated pszErrDesc
 * is copied to the buffer. This function always succeed.
 *
 * \param szBuff		[out] A buffer to save the result error message. The buffer can not be NULL.
 * \param cchBuff		[in] Number of characters of the buffer, including the terminating NULL character.
 * \param dwErrCode		[in] A system error code.
 * \param pszErrDesc	[in] An user's custom error message. The system error message is appended after it.
 *                           This parameter is ignored if the value is NULL.
 * \sa					::FormatMessage
 * \sa					::GetLastError
 */
void
GetSystemErrMsgA (LPSTR pszBuff, DWORD cchBuff, DWORD dwErrCode, LPCSTR pszErrDesc)
{
	GetSystemErrMsgA_Impl (pszBuff, cchBuff, dwErrCode, pszErrDesc, FALSE, NULL) ;
}

/*!
 * This function creates a custom error message which has an appended system error message.
 * This function first copies pszErrDesc to pszBuff and then copies a system
 * error message of an error code specified by dwErrCode.
 * If the buffer is not enough to copy the system error message, the system error message
 * is not copied. If the buffer is not enough to copy pszErrDesc, a truncated pszErrDesc
 * is copied to the buffer. This function always succeed.
 *
 * \param szBuff		[out] A buffer to save the result error message. The buffer can not be NULL.
 * \param cchBuff		[in] Number of characters of the buffer, including the terminating NULL character.
 * \param dwErrCode		[in] A system error code.
 * \param pszErrDesc	[in] An user's custom error message. The system error message is appended after it.
 *                           This parameter is ignored if the value is NULL.
 * \sa					::FormatMessage
 * \sa					::GetLastError
 */
void
GetSystemErrMsgW (LPWSTR pszBuff, DWORD cchBuff, DWORD dwErrCode, LPCWSTR pszErrDesc)
{
	GetSystemErrMsgW_Impl (pszBuff, cchBuff, dwErrCode, pszErrDesc, FALSE, NULL) ;
}


/*!
 * This function creates a custom error message which has an appended WinInet error message.
 * This function first copies pszErrDesc to pszBuff and then copies a system
 * error message of an error code returned by ::GetLastError ().
 * It uses the message table resources of wininet.dll.
 * If the buffer is not enough to copy the system error message, the WinInet error message
 * is not copied. If the buffer is not enough to copy pszErrDesc, a truncated pszErrDesc
 * is copied to the buffer. This function always succeed.
 *
 * \param szBuff		[out] A buffer to save the result error message. The buffer can not be NULL.
 * \param cchBuff		[in] Number of characters of the buffer, including the terminating NULL character.
 * \param pszErrDesc	[in] An user's custom error message. The system error message is appended after it.
 *                           This parameter is ignored if the value is NULL.
 * \sa					::FormatMessage
 * \sa					::GetLastError
 */
void
GetWinInetErrMsgA (LPSTR pszBuff, DWORD cchBuff, LPCSTR pszErrDesc)
{
	// Caches an error code
	DWORD		dwLastErrCode = ::GetLastError () ;
	GetSystemErrMsgA_Impl (pszBuff, cchBuff, dwLastErrCode, pszErrDesc, FALSE
		, ::GetModuleHandle (TEXT ("wininet.dll"))) ;
}

/*!
 * This function creates a custom error message which has an appended WinInet error message.
 * This function first copies pszErrDesc to pszBuff and then copies a system
 * error message of an error code returned by ::GetLastError ().
 * If the buffer is not enough to copy the system error message, the WinInet error message
 * is not copied. If the buffer is not enough to copy pszErrDesc, a truncated pszErrDesc
 * is copied to the buffer. This function always succeed.
 *
 * \param szBuff		[out] A buffer to save the result error message. The buffer can not be NULL.
 * \param cchBuff		[in] Number of characters of the buffer, including the terminating NULL character.
 * \param pszErrDesc	[in] An user's custom error message. The system error message is appended after it.
 *                           This parameter is ignored if the value is NULL.
 * \sa					::FormatMessage
 * \sa					::GetLastError
 */
void
GetWinInetErrMsgW (LPWSTR pszBuff, DWORD cchBuff, LPCWSTR pszErrDesc)
{
	// Caches an error code
	DWORD		dwLastErrCode = ::GetLastError () ;
	GetSystemErrMsgW_Impl (pszBuff, cchBuff, dwLastErrCode, pszErrDesc, FALSE
		, ::GetModuleHandle (TEXT ("wininet.dll"))) ;
}

/*!
 * This function creates a custom error message which has an appended WinInet error message.
 * This function first copies pszErrDesc to pszBuff and then copies a system
 * error message of an error code specified by dwErrCode.
 * It uses the message table resources of wininet.dll.
 * If the buffer is not enough to copy the system error message, the WinInet error message
 * is not copied. If the buffer is not enough to copy pszErrDesc, a truncated pszErrDesc
 * is copied to the buffer. This function always succeed.
 *
 * \param szBuff		[out] A buffer to save the result error message. The buffer can not be NULL.
 * \param cchBuff		[in] Number of characters of the buffer, including the terminating NULL character.
 * \param dwErrCode		[in] A system error code.
 * \param pszErrDesc	[in] An user's custom error message. The system error message is appended after it.
 *                           This parameter is ignored if the value is NULL.
 * \sa					::FormatMessage
 * \sa					::GetLastError
 */
void
GetWinInetErrMsgA (LPSTR pszBuff, DWORD cchBuff, DWORD dwErrCode, LPCSTR pszErrDesc)
{
	GetSystemErrMsgA_Impl (pszBuff, cchBuff, dwErrCode, pszErrDesc, FALSE
		, ::GetModuleHandle (TEXT ("wininet.dll"))) ;
}

/*!
 * This function creates a custom error message which has an appended WinInet error message.
 * This function first copies pszErrDesc to pszBuff and then copies a system
 * error message of an error code specified by dwErrCode.
 * It uses the message table resources of wininet.dll.
 * If the buffer is not enough to copy the system error message, the WinInet error message
 * is not copied. If the buffer is not enough to copy pszErrDesc, a truncated pszErrDesc
 * is copied to the buffer. This function always succeed.
 *
 * \param szBuff		[out] A buffer to save the result error message. The buffer can not be NULL.
 * \param cchBuff		[in] Number of characters of the buffer, including the terminating NULL character.
 * \param dwErrCode		[in] A system error code.
 * \param pszErrDesc	[in] An user's custom error message. The system error message is appended after it.
 *                           This parameter is ignored if the value is NULL.
 * \sa					::FormatMessage
 * \sa					::GetLastError
 */
void
GetWinInetErrMsgW (LPWSTR pszBuff, DWORD cchBuff, DWORD dwErrCode, LPCWSTR pszErrDesc)
{
	GetSystemErrMsgW_Impl (pszBuff, cchBuff, dwErrCode, pszErrDesc, FALSE
		, ::GetModuleHandle (TEXT ("wininet.dll"))) ;
}


/*!
 * \internal
 * This function creates a custom error message which has an appended COM error message.
 * This function first copies pszErrDesc to pszBuff and then copies a COM
 * error message which is returned by using pIErrorInfo.
 * If pIErrorInfo is NULL, ::GetErrorInfo () is used to get IErrorInfo interface.
 * If the buffer is not enough to copy the system error message, the COM error message
 * is not copied. If the buffer is not enough to copy pszErrDesc, a truncated pszErrDesc
 * is copied to the buffer. If the function fails to get a IErrorInfo interface or
 * to get an error message from the IErrorInfo interface, it will return FALSE
 * and an error message of the failure is copied to the buffer.
 *
 * \param szBuff		[out] A buffer to save the result error message. The buffer can not be NULL.
 * \param cchBuff		[in] Number of characters of the buffer, including the terminating NULL character.
 * \param pIErrorInfo	[in] A IErrorInfo interface to get an error message. If this value is NULL,
 *                           ::GetErrorInfo () is used to get IErrorInfo interface.
 * \param pszErrDesc	[in] An user's custom error message. The COM error message is appended after it.
 *                           This parameter is ignored if the value is NULL.
 * \return				TRUE if the function succeeds to get an error message. otherwise, FALSE.
 * \sa					::GetErrorInfo
 */
static BOOL
GetCOMErrMsgA_Impl (LPSTR pszBuff, DWORD cchBuff, IErrorInfo * pIErrorInfo, LPCSTR pszErrDesc)
{
	_ASSERTE ( !(pszBuff == NULL || cchBuff == 0) ) ;

	CComBSTR			bstrErrMsg ;
	LPSTR				szErrMsg = NULL ;
	BOOL				bReturn = FALSE ;

	// If pIErrorInfo is NULL, get it by using ::GetErrorInfo ().
	if ( pIErrorInfo == NULL ) {
		HRESULT				hResult ;
		hResult = ::GetErrorInfo (NULL, &pIErrorInfo) ;

		if ( FAILED (hResult) )
			szErrMsg = "Failed to get a IErrorInfo interface." ;

		if ( hResult == S_FALSE )
			szErrMsg = "Object does not support the ISupportErrorInfo interface." ;

	} else {
		pIErrorInfo->AddRef () ;
	}

	// Gets a COM error message
	if ( szErrMsg == NULL ) {
		if ( FAILED (pIErrorInfo->GetDescription (&bstrErrMsg)) )
			szErrMsg = "Failed to get a error message from IErrorInfo" ;

		// release the interface
		pIErrorInfo->Release () ;
	}

	if ( cchBuff == 1 ) {
		pszBuff[0] = NULL ;
		// If no error is occured,
		if ( szErrMsg == NULL )
			return TRUE ;
		return bReturn ;
	}

	size_t		cchUsedBuff = 0 ;
	pszBuff[0] = NULL ;

	// Copies the user's error message
	if ( (pszErrDesc != NULL) && (::strlen (pszErrDesc) > 0) ) {
		::_mbsnbcpy (
			(unsigned char *) (pszBuff + cchUsedBuff)
			, (unsigned char *) (pszErrDesc)
			, cchBuff - cchUsedBuff - 1) ;
		pszBuff[cchBuff - 1] = NULL ;
		cchUsedBuff = ::strlen (pszBuff) ;
	}

	// If the buffer is full, 
	if ( cchUsedBuff >= cchBuff - 1 ) {
		// If no error is occured,
		if ( szErrMsg == NULL )
			return TRUE ;
		return bReturn ;
	}

	LPSTR				szUsingBuff = NULL ;
	BOOL				bFreeUsingBuff = FALSE ;

	if ( szErrMsg == NULL && bstrErrMsg.m_str != NULL ) {
		int			cbNeeded ;

		// Calculates required memory
		if ( 0 == (cbNeeded = ::WideCharToMultiByte (
				CP_ACP
				, 0
				, static_cast <LPWSTR> (bstrErrMsg.m_str)
				, -1
				, NULL
				, 0
				, NULL
				, NULL)) )
			szErrMsg = "This COM error message is a invalid unicode string." ;

		if ( szErrMsg == NULL ) {
			if ( ((cchBuff - cchUsedBuff) * sizeof (CHAR)) >= static_cast<DWORD> (cbNeeded) )
				szUsingBuff = pszBuff + cchUsedBuff ;
			else {
				// Allocate memory
				szUsingBuff = (LPSTR) ::malloc (cbNeeded) ;
				if ( szUsingBuff == NULL )
					szErrMsg = "Failed to allocate memory" ;
				else
					bFreeUsingBuff = TRUE ;
			}
		}

		if ( szErrMsg == NULL ) {
			// Converts Unicode to ANSI
			if ( 0 == ::WideCharToMultiByte (
					CP_ACP
					, 0
					, static_cast <LPWSTR> (bstrErrMsg.m_str)
					, -1
					, szUsingBuff
					, cbNeeded
					, NULL
					, NULL) )
				szErrMsg = "Failed to convert a COM error message to ANSI" ;
			else {
				bReturn = TRUE ;
				szErrMsg = szUsingBuff ;
			}
		}
	}

	// Copies the error message
	if ( szErrMsg ) {
		::_mbsnbcpy (
			(unsigned char *) (pszBuff + cchUsedBuff)
			, (unsigned char *) szErrMsg
			, cchBuff - cchUsedBuff - 1
		) ;
		pszBuff[cchBuff - 1] = NULL ;
	}

	if ( bFreeUsingBuff )
		::free (szUsingBuff) ;

	return bReturn ;
}

/*!
 * \internal
 * This function creates a custom error message which has an appended COM error message.
 * This function first copies pszErrDesc to pszBuff and then copies a COM
 * error message which is returned by using pIErrorInfo.
 * If pIErrorInfo is NULL, ::GetErrorInfo () is used to get IErrorInfo interface.
 * If the buffer is not enough to copy the system error message, the COM error message
 * is not copied. If the buffer is not enough to copy pszErrDesc, a truncated pszErrDesc
 * is copied to the buffer. If the function fails to get a IErrorInfo interface or
 * to get an error message from the IErrorInfo interface, it will return FALSE
 * and an error message of the failure is copied to the buffer.
 *
 * \param szBuff		[out] A buffer to save the result error message. The buffer can not be NULL.
 * \param cchBuff		[in] Number of characters of the buffer, including the terminating NULL character.
 * \param pIErrorInfo	[in] A IErrorInfo interface to get an error message. If this value is NULL,
 *                           ::GetErrorInfo () is used to get IErrorInfo interface.
 * \param pszErrDesc	[in] An user's custom error message. The COM error message is appended after it.
 *                           This parameter is ignored if the value is NULL.
 * \return				TRUE if the function succeeds to get an error message. otherwise, FALSE.
 * \sa					::GetErrorInfo
 */
static BOOL
GetCOMErrMsgW_Impl (LPWSTR pszBuff, DWORD cchBuff, IErrorInfo * pIErrorInfo, LPCWSTR pszErrDesc)
{
	_ASSERTE ( !(pszBuff == NULL || cchBuff == 0) ) ;

	CComBSTR			bstrErrMsg ;
	LPCWSTR				szErrMsg = NULL ;
	BOOL				bReturn = FALSE ;

	// If pIErrorInfo is NULL, get it by using ::GetErrorInfo ().
	if ( pIErrorInfo == NULL ) {
		HRESULT				hResult ;
		hResult = ::GetErrorInfo (NULL, &pIErrorInfo) ;

		if ( FAILED (hResult) )
			szErrMsg = L"Failed to get a IErrorInfo interface." ;

		if ( hResult == S_FALSE )
			szErrMsg = L"Object does not support the ISupportErrorInfo interface." ;

	} else {
		pIErrorInfo->AddRef () ;
	}

	// Gets a COM error message
	if ( szErrMsg == NULL ) {
		if ( FAILED (pIErrorInfo->GetDescription (&bstrErrMsg)) )
			szErrMsg = L"Failed to get a error message from IErrorInfo" ;	
		else {
			szErrMsg = static_cast<LPWSTR> (bstrErrMsg.m_str) ;
			bReturn = TRUE ;
		}
		pIErrorInfo->Release () ;
	}

	if ( cchBuff == 1 ) {
		pszBuff[0] = NULL ;
		return bReturn ;
	}

	size_t		cchUsedBuff = 0 ;
	pszBuff[0] = NULL ;

	// Copies the user's error message
	if ( (pszErrDesc != NULL) && (::wcslen (pszErrDesc) > 0) ) {
		::StringCbCopyW (
			pszBuff + cchUsedBuff
			, (cchBuff - cchUsedBuff) * sizeof (WCHAR)
			, pszErrDesc) ;
		cchUsedBuff = ::wcslen (pszBuff) ;
	}

	// If the buffer is full, 
	if ( cchUsedBuff >= cchBuff - 1 )
		return bReturn ;

	// Copies the error message
	if ( szErrMsg )
		::StringCchCopyW (pszBuff + cchUsedBuff
				, cchBuff - cchUsedBuff
				, szErrMsg) ;

	return bReturn ;
}


/*!
 * This function creates a custom error message which has an appended COM error message.
 * This function first copies pszErrDesc to pszBuff and then copies a COM
 * error message which is returned by using pIErrorInfo.
 * If pIErrorInfo is NULL, ::GetErrorInfo () is used to get IErrorInfo interface.
 * If the buffer is not enough to copy the system error message, the COM error message
 * is not copied. If the buffer is not enough to copy pszErrDesc, a truncated pszErrDesc
 * is copied to the buffer. If the function fails to get a IErrorInfo interface or
 * to get an error message from the IErrorInfo interface, it will return FALSE
 * and an error message of the failure is copied to the buffer.
 *
 * \param szBuff		[out] A buffer to save the result error message. The buffer can not be NULL.
 * \param cchBuff		[in] Number of characters of the buffer, including the terminating NULL character.
 * \param pIErrorInfo	[in] A IErrorInfo interface to get an error message. If this value is NULL,
 *                           ::GetErrorInfo () is used to get IErrorInfo interface.
 * \param pszErrDesc	[in] An user's custom error message. The COM error message is appended after it.
 *                           This parameter is ignored if the value is NULL.
 * \return				TRUE if the function succeeds to get an error message. otherwise, FALSE.
 * \sa					::GetErrorInfo
 */
BOOL
GetCOMErrMsgA (LPSTR pszBuff, DWORD cchBuff, IErrorInfo * pIErrorInfo, LPCSTR pszErrDesc)
{
	return GetCOMErrMsgA_Impl (pszBuff, cchBuff, pIErrorInfo, pszErrDesc) ;
}

/*!
 * This function creates a custom error message which has an appended COM error message.
 * This function first copies pszErrDesc to pszBuff and then copies a COM
 * error message which is returned by using pIErrorInfo.
 * If pIErrorInfo is NULL, ::GetErrorInfo () is used to get IErrorInfo interface.
 * If the buffer is not enough to copy the system error message, the COM error message
 * is not copied. If the buffer is not enough to copy pszErrDesc, a truncated pszErrDesc
 * is copied to the buffer. If the function fails to get a IErrorInfo interface or
 * to get an error message from the IErrorInfo interface, it will return FALSE
 * and an error message of the failure is copied to the buffer.
 *
 * \param szBuff		[out] A buffer to save the result error message. The buffer can not be NULL.
 * \param cchBuff		[in] Number of characters of the buffer, including the terminating NULL character.
 * \param pIErrorInfo	[in] A IErrorInfo interface to get an error message. If this value is NULL,
 *                           ::GetErrorInfo () is used to get IErrorInfo interface.
 * \param pszErrDesc	[in] An user's custom error message. The COM error message is appended after it.
 *                           This parameter is ignored if the value is NULL.
 * \return				TRUE if the function succeeds to get an error message. otherwise, FALSE.
 * \sa					::GetErrorInfo
 */
BOOL
GetCOMErrMsgW (LPWSTR pszBuff, DWORD cchBuff, IErrorInfo * pIErrorInfo, LPCWSTR pszErrDesc)
{
	return GetCOMErrMsgW_Impl (pszBuff, cchBuff, pIErrorInfo, pszErrDesc) ;
}


/*!
 * This function creates a custom error message which has an appended COM error message.
 * This function first copies pszErrDesc to pszBuff and then copies a COM
 * error message which is returned by using IErrorInfo.
 * ::GetErrorInfo () is used to get a IErrorInfo interface.
 * If the buffer is not enough to copy the system error message, the COM error message
 * is not copied. If the buffer is not enough to copy pszErrDesc, a truncated pszErrDesc
 * is copied to the buffer. If the function fails to get a IErrorInfo interface or
 * to get an error message from the IErrorInfo interface, it will return FALSE
 * and an error message of the failure is copied to the buffer.
 *
 * \param szBuff		[out] A buffer to save the result error message. The buffer can not be NULL.
 * \param cchBuff		[in] Number of characters of the buffer, including the terminating NULL character.
 * \param pszErrDesc	[in] An user's custom error message. The COM error message is appended after it.
 *                           This parameter is ignored if the value is NULL.
 * \return				TRUE if the function succeeds to get an error message. otherwise, FALSE.
 * \sa					::GetErrorInfo
 */
BOOL
GetCOMErrMsgA (LPSTR pszBuff, DWORD cchBuff, LPCSTR pszErrDesc)
{
	return GetCOMErrMsgA_Impl (pszBuff, cchBuff, NULL, pszErrDesc) ;
}

/*!
 * This function creates a custom error message which has an appended COM error message.
 * This function first copies pszErrDesc to pszBuff and then copies a COM
 * error message which is returned by using IErrorInfo.
 * ::GetErrorInfo () is used to get a IErrorInfo interface.
 * If the buffer is not enough to copy the system error message, the COM error message
 * is not copied. If the buffer is not enough to copy pszErrDesc, a truncated pszErrDesc
 * is copied to the buffer. If the function fails to get a IErrorInfo interface or
 * to get an error message from the IErrorInfo interface, it will return FALSE
 * and an error message of the failure is copied to the buffer.
 *
 * \param szBuff		[out] A buffer to save the result error message. The buffer can not be NULL.
 * \param cchBuff		[in] Number of characters of the buffer, including the terminating NULL character.
 * \param pszErrDesc	[in] An user's custom error message. The COM error message is appended after it.
 *                           This parameter is ignored if the value is NULL.
 * \return				TRUE if the function succeeds to get an error message. otherwise, FALSE.
 * \sa					::GetErrorInfo
 */
BOOL
GetCOMErrMsgW (LPWSTR pszBuff, DWORD cchBuff, LPCWSTR pszErrDesc)
{
	return GetCOMErrMsgW_Impl (pszBuff, cchBuff, NULL, pszErrDesc) ;
}





/*!
 * \internal
 * This function set a COM IErrorInfo.
 *
 * \param pGUID			[in] GUID of the interface that defined the error, or GUID_NULL if the error was defined by the operating system. 
 * \param szSource		[in] Null-terminated string that contains the programmatic identifier in the form progname.objectname.
 * \param szDescription	[in] Brief, null-terminated string that describes the error.
 * \param szHelpFile	[in] Null-terminated string that contains the fully qualified path of the Help file that describes the error.
 * \param dwHelpContext	[in] Specifies the Help context identifier for the errorr.
 * \return				The standard COM HRESULTs.
 * \sa					::CreateErrorInfo
 * \sa					::SetErrorInfo
 */
static HRESULT
SetCOMErrInfo_Impl (const GUID * pGUID, LPCOLESTR szSource, LPCOLESTR szDescription, LPCOLESTR szHelpFile, DWORD dwHelpContext)
{
	CComPtr<ICreateErrorInfo>	spCEI ;
	HRESULT						hResult ;

	hResult = ::CreateErrorInfo (&spCEI) ;
	if ( FAILED (hResult) )
		return hResult ;

	if ( pGUID && FAILED (hResult = spCEI->SetGUID (*pGUID)) )
		return hResult ;

	if ( szSource && FAILED (hResult = spCEI->SetSource (const_cast <LPOLESTR> (szSource))) )
		return hResult ;

	if ( szDescription && FAILED (hResult = spCEI->SetDescription (const_cast <LPOLESTR> (szDescription))) )
		return hResult ;

	if ( szHelpFile && FAILED (hResult = spCEI->SetHelpFile (const_cast <LPOLESTR> (szHelpFile))) )
		return hResult ;

	if ( FAILED (hResult = spCEI->SetHelpContext (dwHelpContext)) )
		return hResult ;

	CComPtr<IErrorInfo>			spEI ;
	if ( FAILED (hResult = spCEI->QueryInterface (IID_IErrorInfo, (void **) (&spEI))) )
		return hResult ;

	if ( FAILED (hResult = ::SetErrorInfo (0, spEI)) )
		return hResult ;

	return S_OK ;
}

/*!
 * This function set a COM IErrorInfo.
 *
 * \param pGUID			[in] GUID of the interface that defined the error, or GUID_NULL if the error was defined by the operating system. 
 * \param szSource		[in] Null-terminated string that contains the programmatic identifier in the form progname.objectname.
 * \param szDescription	[in] Brief, null-terminated string that describes the error.
 * \param szHelpFile	[in] Null-terminated string that contains the fully qualified path of the Help file that describes the error.
 * \param dwHelpContext	[in] Specifies the Help context identifier for the errorr.
 * \return				The standard COM HRESULTs.
 * \sa					::CreateErrorInfo
 * \sa					::SetErrorInfo
 */
HRESULT
SetCOMErrInfoA (const GUID * pGUID, LPCSTR szSource, LPCSTR szDescription, LPCSTR szHelpFile, DWORD dwHelpContext)
{
	HANDLE			hProcHeap ;

	if ( NULL == (hProcHeap = ::GetProcessHeap ()) )
		return E_FAIL ;

	LPWSTR			wszSource = NULL ;
	LPWSTR			wszDescription = NULL ;
	LPWSTR			wszHelpFile = NULL ;
	HRESULT			hResult ;

	if ( szSource && 
			(FAILED (hResult = Ansi2Unicode (&wszSource, szSource, hProcHeap))) )
		return hResult ;

	if ( szDescription && 
		(FAILED (hResult = Ansi2Unicode (&wszDescription, szDescription, hProcHeap))) ) {
		::HeapFree (hProcHeap, 0, wszSource) ;
		return hResult ;
	}

	if ( szHelpFile && 
		(FAILED (hResult = Ansi2Unicode (&wszHelpFile, szHelpFile, hProcHeap))) ) {
		::HeapFree (hProcHeap, 0, wszSource) ;
		::HeapFree (hProcHeap, 0, wszDescription) ;
		return hResult ;
	}

	hResult = SetCOMErrInfo_Impl (pGUID, wszSource, wszDescription, wszHelpFile, dwHelpContext) ;
	::HeapFree (hProcHeap, 0, wszSource) ;
	::HeapFree (hProcHeap, 0, wszDescription) ;
	::HeapFree (hProcHeap, 0, wszHelpFile) ;

	return hResult ;
}

/*!
 * This function set a COM IErrorInfo.
 *
 * \param pGUID			[in] GUID of the interface that defined the error, or GUID_NULL if the error was defined by the operating system. 
 * \param szSource		[in] Null-terminated string that contains the programmatic identifier in the form progname.objectname.
 * \param szDescription	[in] Brief, null-terminated string that describes the error.
 * \param szHelpFile	[in] Null-terminated string that contains the fully qualified path of the Help file that describes the error.
 * \param dwHelpContext	[in] Specifies the Help context identifier for the errorr.
 * \return				The standard COM HRESULTs.
 * \sa					::CreateErrorInfo
 * \sa					::SetErrorInfo
 */
HRESULT
SetCOMErrInfoW (const GUID * pGUID, LPCWSTR szSource, LPCWSTR szDescription, LPCWSTR wszHelpFile, DWORD dwHelpContext)
{
	return SetCOMErrInfo_Impl (pGUID, szSource, szDescription, wszHelpFile, dwHelpContext) ;
}


}
