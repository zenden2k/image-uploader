/*!
 * \file	RyeolStringFunc.cpp
 * \brief	Implementations of functions related to string handling.
 * \author	Jo Hyeong-ryeol
 * \since	2003.07.07
 * \version	$LastChangedRevision: 99 $
 *			$LastChangedDate: 2006-02-03 23:16:57 +0900 (금, 03 2 2006) $
 *
 * This file contains implementations of functions related to string handling.
 * \n\n\n
 * Copyright &copy; 2006 by <a href="mailto:hyeongryeol@gmail.com">Jo Hyeong-ryeol</a>\n
 * Permission to copy, use, modify, sell and distribute this software is
 * granted provided this copyright notice appears in all copies.
 * This software is provided "as is" without express or implied warranty,
 * and with no claim as to its suitability for any purpose.
 */
#include "stdafx.h"
#include "RyeolStringFunc.h"

/*!
 * \brief	The namespace of the Ryeol's library
 *
 * This is the namespace for source codes written by Jo Hyeong-ryeol.
 */
namespace Ryeol {

/*!
 * This function returns the number of bytes which is required
 * to convert an UNICODE string into an ANSI string.
 * The returned value does not include the terminating NULL character.
 * If an error occurrs, you can use the ::GetLastError function for error information.
 *
 * \param[in] wszSrc		An UNICODE string.
 * \param[out] pcbNeeded	The number of bytes required.
 *							(Not including the terminating NULL character)
 * \return					A HRESULT value
 */
HRESULT
NeededCharsForUnicode2Ansi (LPCWSTR wszSrc, int * pcbNeeded)
{
	// Checks the input parameters
	_ASSERTE ( (wszSrc != NULL) && (pcbNeeded != NULL) ) ;

	size_t		cchSrc ;
	if ( FAILED (::StringCchLengthW (wszSrc, STRSAFE_MAX_CCH, &cchSrc)) ) {
		::SetLastError (ERROR_INVALID_PARAMETER) ;
		return E_INVALIDARG ;
	}

	// If the length of the string is zero
	if ( cchSrc == 0 ) {
		*pcbNeeded = 0 ;
		return S_OK ;
	}
	
	// Calculates the required memory
	if ( 0 == (*pcbNeeded = ::WideCharToMultiByte (
			CP_ACP
			, 0
			, wszSrc
			, static_cast<int> (cchSrc + 1)
			, NULL
			, 0
			, NULL
			, NULL)) )
		return E_FAIL ;

	*pcbNeeded -= 1 ;
	return S_OK ;
}

/*!
 * This function converts an UNICODE string into an ANSI string.
 * If an error occurrs, you can use the ::GetLastError function for error information.
 * The buffer pointed to by the szDest must be enough to copy the converted string.
 *
 * \param[in] szDest		A buffer to save the converted string.
 * \param[in] cbDest		The number of bytes the buffer can hold.
 * \param[in] wszSrc		An UNICODE string to convert.
 * \return					A HRESULT value
 */
HRESULT
Unicode2Ansi (LPSTR szDest, int cbDest, LPCWSTR wszSrc)
{
	_ASSERTE ( !(szDest == NULL || cbDest == 0 || wszSrc == NULL) ) ;

	// Converts into ANSI format.
	if ( 0 == ::WideCharToMultiByte (
			CP_ACP
			, 0
			, wszSrc
			, -1
			, szDest
			, cbDest
			, NULL
			, NULL) )
			return E_FAIL ;
	return S_OK ;
}

/*!
 * This function converts an UNICODE string into an ANSI string.
 * If an error occurrs, you can use the ::GetLastError function for error information.
 * The string returned by the pszDest must be released by using the ::HeapFree function.
 *
 * \param[out] pszDest		A pointer to receive the converted string.
 * \param[in] wszSrc		An UNICODE string to convert.
 * \param[in] hHeap			A heap handle which is used to allocate memory.
 *							If this is NULL, the process heap is used.
 * \return					A HRESULT value
 */
HRESULT
Unicode2Ansi (LPSTR * pszDest, LPCWSTR wszSrc, HANDLE hHeap)
{
	// Checks the input parameters
	_ASSERTE ( !(pszDest == NULL || wszSrc == NULL) ) ;

	*pszDest = NULL ;

	if ( hHeap == NULL && (hHeap = ::GetProcessHeap ()) == NULL )
		return E_FAIL ;

	HRESULT			hResult ;

	int				cchNeeded ;
	if ( FAILED (hResult = NeededCharsForUnicode2Ansi  (wszSrc, &cchNeeded)) )
		return hResult ;

	// Allocates memory
	*pszDest = static_cast<LPSTR> (::HeapAlloc (hHeap, 0, cchNeeded + 1)) ;
	if ( *pszDest == NULL ) {
		::SetLastError (ERROR_NOT_ENOUGH_MEMORY) ;
		return E_OUTOFMEMORY ;
	}

	// Converts into ANSI format
	if ( FAILED (hResult = Unicode2Ansi (*pszDest, cchNeeded + 1, wszSrc)) ) {
		DWORD		dwLastErr = ::GetLastError () ;
		::HeapFree (hHeap, 0, *pszDest) ;
		::SetLastError (dwLastErr) ;
		return hResult ;
	}

	return S_OK ;
}


/*!
 * This function returns the number of bytes which is required
 * to convert an UNICODE string into an UTF-8 string.
 * The returned value does not include the terminating NULL character.
 * If an error occurrs, you can use the ::GetLastError function for error information.
 *
 * \param[in] wszSrc		An UNICODE string.
 * \param[out] pcbNeeded	The number of bytes required.
 *							(Not including the terminating NULL character)
 * \return					A HRESULT value
 */
HRESULT
NeededCharsForUnicode2UTF8 (LPCWSTR wszSrc, int * pcbNeeded)
{
	// Checks the input parameters
	_ASSERTE ( (wszSrc != NULL) && (pcbNeeded != NULL) ) ;

	size_t		cchSrc ;
	if ( FAILED (::StringCchLengthW (wszSrc, STRSAFE_MAX_CCH, &cchSrc)) ) {
		::SetLastError (ERROR_INVALID_PARAMETER) ;
		return E_INVALIDARG ;
	}

	// If the length of the string is zero
	if ( cchSrc == 0 ) {
		*pcbNeeded = 0 ;
		return S_OK ;
	}

	// Calculates the required memory
	if ( 0 == (*pcbNeeded = ::WideCharToMultiByte (
			CP_UTF8
			, 0
			, wszSrc
			, static_cast<int> (cchSrc + 1)
			, NULL
			, 0
			, NULL
			, NULL)) )
		return E_FAIL ;

	*pcbNeeded -= 1 ;
	return S_OK ;
}

/*!
 * This function converts an UNICODE string into an UTF-8 string.
 * If an error occurrs, you can use the ::GetLastError function for error information.
 * The buffer pointed to by the szDest must be enough to copy the converted string.
 *
 * \param[in] szDest		A buffer to save the converted string.
 * \param[in] cbDest		The number of bytes the buffer can hold.
 * \param[in] wszSrc		An UNICODE string to convert.
 * \return					A HRESULT value
 */
HRESULT
Unicode2UTF8 (LPSTR szDest, int cbDest, LPCWSTR wszSrc)
{
	_ASSERTE ( !(szDest == NULL || cbDest == 0 || wszSrc == NULL) ) ;

	// Converts into UTF-8 format.
	if ( 0 == ::WideCharToMultiByte (
			CP_UTF8
			, 0
			, wszSrc
			, -1
			, szDest
			, cbDest
			, NULL
			, NULL) )
			return E_FAIL ;
	return S_OK ;
}

/*!
 * This function converts an UNICODE string into an UTF-8 string.
 * If an error occurrs, you can use the ::GetLastError function for error information.
 * The string returned by the pszDest must be released by using the ::HeapFree function.
 *
 * \param[out] pszDest		A pointer to receive the converted string.
 * \param[in] wszSrc		An UNICODE string to convert.
 * \param[in] hHeap			A heap handle which is used to allocate memory.
 *							If this is NULL, the process heap is used.
 * \return					A HRESULT value
 */
HRESULT
Unicode2UTF8 (LPSTR * pszDest, LPCWSTR wszSrc, HANDLE hHeap)
{
	// Checks the input parameters
	_ASSERTE ( !(pszDest == NULL || wszSrc == NULL) ) ;

	*pszDest = NULL ;

	if ( hHeap == NULL && (hHeap = ::GetProcessHeap ()) == NULL )
		return E_FAIL ;

	HRESULT			hResult ;

	int				cchNeeded ;
	if ( FAILED (hResult = NeededCharsForUnicode2UTF8  (wszSrc, &cchNeeded)) )
		return hResult ;

	// Allocates memory
	*pszDest = static_cast<LPSTR> (::HeapAlloc (hHeap, 0, cchNeeded + 1)) ;
	if ( *pszDest == NULL ) {
		::SetLastError (ERROR_NOT_ENOUGH_MEMORY) ;
		return E_OUTOFMEMORY ;
	}

	// Converts into UTF-8 format
	if ( FAILED (hResult = Unicode2UTF8 (*pszDest, cchNeeded + 1, wszSrc)) ) {
		DWORD		dwLastErr = ::GetLastError () ;
		::HeapFree (hHeap, 0, *pszDest) ;
		::SetLastError (dwLastErr) ;
		return hResult ;
	}

	return S_OK ;
}


/*!
 * This function returns the number of characters which is required
 * to convert an ANSI string into an UNICODE string.
 * The returned value does not include the terminating NULL character.
 * If an error occurrs, you can use the ::GetLastError function for error information.
 *
 * \param[in] szSrc			An ANSI string.
 * \param[out] pcchNeeded	The number of characters required.
 *							(Not including the terminating NULL character)
 * \return					A HRESULT value
 */
HRESULT
NeededCharsForAnsi2Unicode (LPCSTR szSrc, int * pcchNeeded)
{
	// Checks the input parameters
	_ASSERTE ( (szSrc != NULL) && (pcchNeeded != NULL) ) ;

	size_t		cchSrc ;
	if ( FAILED (::StringCchLengthA (szSrc, STRSAFE_MAX_CCH, &cchSrc)) ) {
		::SetLastError (ERROR_INVALID_PARAMETER) ;
		return E_INVALIDARG ;
	}

	// If the length of the string is zero
	if ( cchSrc == 0 ) {
		*pcchNeeded = 0 ;
		return S_OK ;
	}

	// Calculates the required memory
	if ( 0 == (*pcchNeeded = ::MultiByteToWideChar (
			CP_ACP
			, 0
			, szSrc
			, static_cast<int> (cchSrc + 1)
			, NULL
			, 0)) )
		return E_FAIL ;

	*pcchNeeded -= 1 ;
	return S_OK ;
}

/*!
 * This function converts an ANSI string into an UNICODE string.
 * If an error occurrs, you can use the ::GetLastError function for error information.
 * The buffer pointed to by the wszDest must be enough to copy the converted string.
 *
 * \param[in] wszDest		A buffer to save the converted string.
 * \param[in] cchDest		The number of characters the buffer can hold.
 * \param[in] szSrc			An ANSI string to convert.
 * \return					A HRESULT value
 */
HRESULT
Ansi2Unicode (LPWSTR wszDest, int cchDest, LPCSTR szSrc)
{
	_ASSERTE ( !(wszDest == NULL || cchDest == 0 || szSrc == NULL) ) ;

	// Converts into UNICODE format.
	if ( 0 == ::MultiByteToWideChar (
			CP_ACP
			, 0
			, szSrc
			, -1
			, wszDest
			, cchDest
			) )
		return E_FAIL ;

	return S_OK ;
}

/*!
 * This function converts an ANSI string into an UNICODE string.
 * If an error occurrs, you can use the ::GetLastError function for error information.
 * The string returned by the pwszDest must be released by using the ::HeapFree function.
 *
 * \param[out] pwszDest		A pointer to receive the converted string.
 * \param[in] szSrc			An ANSI string to convert.
 * \param[in] hHeap			A heap handle which is used to allocate memory.
 *							If this is NULL, the process heap is used.
 * \return					A HRESULT value
 */
HRESULT
Ansi2Unicode (LPWSTR * pwszDest, LPCSTR szSrc, HANDLE hHeap)
{
	// Checks the input parameters
	_ASSERTE ( !(pwszDest == NULL || szSrc == NULL) ) ;

	*pwszDest = NULL ;

	if ( hHeap == NULL && (hHeap = ::GetProcessHeap ()) == NULL )
		return E_FAIL ;

	HRESULT		hResult ;
	int			cchNeeded ;

	if ( FAILED (hResult = NeededCharsForAnsi2Unicode (szSrc, &cchNeeded)) )
		return hResult ;

	// Allocates memory
	*pwszDest = static_cast<LPWSTR> (::HeapAlloc (hHeap, 0, (cchNeeded + 1) * sizeof (WCHAR))) ;
	if ( *pwszDest == NULL ) {
		::SetLastError (ERROR_NOT_ENOUGH_MEMORY) ;
		return E_OUTOFMEMORY ;
	}

	// Converts into UNICODE format
	if ( FAILED (hResult = Ansi2Unicode (*pwszDest, cchNeeded + 1, szSrc)) ) {
		DWORD		dwLastErr = ::GetLastError () ;
		::HeapFree (hHeap, 0, *pwszDest) ;
		::SetLastError (dwLastErr) ;
		return E_FAIL ;
	}

	return S_OK ;
}


}
